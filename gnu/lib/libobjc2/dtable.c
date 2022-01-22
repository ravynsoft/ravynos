#define __BSD_VISIBLE 1
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "objc/runtime.h"
#include "objc/hooks.h"
#include "sarray2.h"
#include "selector.h"
#include "class.h"
#include "lock.h"
#include "method.h"
#include "dtable.h"
#include "visibility.h"
#include "asmconstants.h"

_Static_assert(__builtin_offsetof(struct objc_class, dtable) == DTABLE_OFFSET,
		"Incorrect dtable offset for assembly");
_Static_assert(__builtin_offsetof(SparseArray, shift) == SHIFT_OFFSET,
		"Incorrect shift offset for assembly");
_Static_assert(__builtin_offsetof(SparseArray, data) == DATA_OFFSET,
		"Incorrect data offset for assembly");
// Slots are now a public interface to part of the method structure, so make
// sure that it's safe to use method and slot structures interchangeably.
_Static_assert(__builtin_offsetof(struct objc_slot2, method) == SLOT_OFFSET,
		"Incorrect slot offset for assembly");
_Static_assert(__builtin_offsetof(struct objc_method, imp) == SLOT_OFFSET,
		"Incorrect slot offset for assembly");

PRIVATE dtable_t uninstalled_dtable;
#if defined(WITH_TRACING) && defined (__x86_64)
PRIVATE dtable_t tracing_dtable;
#endif
#ifndef ENOTSUP
#	define ENOTSUP -1
#endif

/** Head of the list of temporary dtables.  Protected by initialize_lock. */
PRIVATE InitializingDtable *temporary_dtables;
/** Lock used to protect the temporary dtables list. */
PRIVATE mutex_t initialize_lock;
/** The size of the largest dtable.  This is a sparse array shift value, so is
 * 2^x in increments of 8. */
static uint32_t dtable_depth = 8;

_Atomic(uint64_t) objc_method_cache_version;

/**
 * Starting at `cls`, finds the class that provides the implementation of the
 * method identified by `sel`.
 */
static Class ownerForMethod(Class cls, SEL sel)
{
	struct objc_slot2 *slot = objc_get_slot2(cls, sel, NULL);
	if (slot == NULL)
	{
		return Nil;
	}
	if (cls->super_class == NULL)
	{
		return cls;
	}
	if (objc_get_slot2(cls->super_class, sel, NULL) == slot)
	{
		return ownerForMethod(cls->super_class, sel);
	}
	return cls;
}

/**
 * Returns YES if the class implements a method for the specified selector, NO
 * otherwise.
 */
static BOOL ownsMethod(Class cls, SEL sel)
{
	return ownerForMethod(cls, sel) == cls;
}


#ifdef DEBUG_ARC_COMPAT
#define ARC_DEBUG_LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define ARC_DEBUG_LOG(...) do {} while(0)
#endif

/**
 * Checks whether the class implements memory management methods, and whether
 * they are safe to use with ARC.
 */
static void checkARCAccessors(Class cls)
{
	static SEL retain, release, autorelease, isARC;
	if (NULL == retain)
	{
		retain = sel_registerName("retain");
		release = sel_registerName("release");
		autorelease = sel_registerName("autorelease");
		isARC = sel_registerName("_ARCCompliantRetainRelease");
	}
	Class owner = ownerForMethod(cls, retain);
	if ((NULL != owner) && !ownsMethod(owner, isARC))
	{
		ARC_DEBUG_LOG("%s does not support ARC correctly (implements retain)\n", cls->name);
		objc_clear_class_flag(cls, objc_class_flag_fast_arc);
		return;
	}
	owner = ownerForMethod(cls, release);
	if ((NULL != owner) && !ownsMethod(owner, isARC))
	{
		ARC_DEBUG_LOG("%s does not support ARC correctly (implements release)\n", cls->name);
		objc_clear_class_flag(cls, objc_class_flag_fast_arc);
		return;
	}
	owner = ownerForMethod(cls, autorelease);
	if ((NULL != owner) && !ownsMethod(owner, isARC))
	{
		ARC_DEBUG_LOG("%s does not support ARC correctly (implements autorelease)\n", cls->name);
		objc_clear_class_flag(cls, objc_class_flag_fast_arc);
		return;
	}
	objc_set_class_flag(cls, objc_class_flag_fast_arc);
}

static BOOL selEqualUnTyped(SEL expected, SEL untyped)
{
	return (expected->index == untyped->index)
#ifdef TYPE_DEPENDENT_DISPATCH
		|| (get_untyped_idx(expected) == untyped->index)
#endif
		;
}

PRIVATE void checkARCAccessorsSlow(Class cls)
{
	if (cls->dtable != uninstalled_dtable)
	{
		return;
	}
	static SEL retain, release, autorelease, isARC;
	if (NULL == retain)
	{
		retain = sel_registerName("retain");
		release = sel_registerName("release");
		autorelease = sel_registerName("autorelease");
		isARC = sel_registerName("_ARCCompliantRetainRelease");
	}
	BOOL superIsFast = YES;
	if (cls->super_class != Nil)
	{
		checkARCAccessorsSlow(cls->super_class);
		superIsFast = objc_test_class_flag(cls->super_class, objc_class_flag_fast_arc);
	}
	BOOL selfImplementsRetainRelease = NO;
	for (struct objc_method_list *l=cls->methods ; l != NULL ; l= l->next)
	{
		for (int i=0 ; i<l->count ; i++)
		{
			SEL s = method_at_index(l, i)->selector;
			if (selEqualUnTyped(s, retain) ||
			    selEqualUnTyped(s, release) ||
			    selEqualUnTyped(s, autorelease))
			{
				selfImplementsRetainRelease = YES;
			}
			else if (selEqualUnTyped(s, isARC))
			{
				objc_set_class_flag(cls, objc_class_flag_fast_arc);
				return;
			}
		}
	}
	if (superIsFast && !selfImplementsRetainRelease)
	{
		objc_set_class_flag(cls, objc_class_flag_fast_arc);
	}
}

static void collectMethodsForMethodListToSparseArray(
		struct objc_method_list *list,
		SparseArray *sarray,
		BOOL recurse)
{
	if (recurse && (NULL != list->next))
	{
		collectMethodsForMethodListToSparseArray(list->next, sarray, YES);
	}
	for (unsigned i=0 ; i<list->count ; i++)
	{
		SparseArrayInsert(sarray, method_at_index(list, i)->selector->index,
				(void*)method_at_index(list, i));
	}
}


PRIVATE void init_dispatch_tables ()
{
	INIT_LOCK(initialize_lock);
	uninstalled_dtable = SparseArrayNewWithDepth(dtable_depth);
#if defined(WITH_TRACING) && defined (__x86_64)
	tracing_dtable = SparseArrayNewWithDepth(dtable_depth);
#endif
}

#if defined(WITH_TRACING) && defined (__x86_64)
static int init;

static void free_thread_stack(void* x)
{
	free(*(void**)x);
}
static pthread_key_t thread_stack_key;
static void alloc_thread_stack(void)
{
	pthread_key_create(&thread_stack_key, free_thread_stack);
	init = 1;
}

PRIVATE void* pushTraceReturnStack(void)
{
	static pthread_once_t once_control = PTHREAD_ONCE_INIT;
	if (!init)
	{
		pthread_once(&once_control, alloc_thread_stack);
	}
	void **stack = pthread_getspecific(thread_stack_key);
	if (stack == 0)
	{
		stack = malloc(4096*sizeof(void*));
	}
	pthread_setspecific(thread_stack_key, stack + 5);
	return stack;
}

PRIVATE void* popTraceReturnStack(void)
{
	void **stack = pthread_getspecific(thread_stack_key);
	stack -= 5;
	pthread_setspecific(thread_stack_key, stack);
	return stack;
}
#endif

int objc_registerTracingHook(SEL aSel, objc_tracing_hook aHook)
{
#if defined(WITH_TRACING) && defined (__x86_64)
	// If this is an untyped selector, register it for every typed variant
	if (sel_getType_np(aSel) == 0)
	{
		SEL buffer[16];
		SEL *overflow = 0;
		int count = sel_copyTypedSelectors_np(sel_getName(aSel), buffer, 16);
		if (count > 16)
		{
			overflow = calloc(count, sizeof(SEL));
			sel_copyTypedSelectors_np(sel_getName(aSel), buffer, 16);
			for (int i=0 ; i<count ; i++)
			{
				SparseArrayInsert(tracing_dtable, overflow[i]->index, aHook);
			}
			free(overflow);
		}
		else
		{
			for (int i=0 ; i<count ; i++)
			{
				SparseArrayInsert(tracing_dtable, buffer[i]->index, aHook);
			}
		}
	}
	SparseArrayInsert(tracing_dtable, aSel->index, aHook);
	return 0;
#else
	return ENOTSUP;
#endif
}

/**
 * Installs a new method in the dtable for `class`.  If `replaceMethod` is
 * `YES` then this will replace any dtable entry where the original is
 * `method_to_replace`.  This is used when a superclass method is replaced, to
 * replace all subclass dtable entries that are inherited, but not ones that
 * are overridden.
 */
static BOOL installMethodInDtable(Class class,
                                  SparseArray *dtable,
                                  struct objc_method *method,
                                  struct objc_method *method_to_replace,
                                  BOOL replaceExisting)
{
	ASSERT(uninstalled_dtable != dtable);
	uint32_t sel_id = method->selector->index;
	struct objc_method *oldMethod = SparseArrayLookup(dtable, sel_id);
	// If we're being asked to replace an existing method, don't if it's the
	// wrong one.
	if ((replaceExisting) && (method_to_replace != oldMethod))
	{
		return NO;
	}
	// If we're not being asked to replace existing methods and there is an
	// existing one, don't replace it.
	if (!replaceExisting && (oldMethod != NULL))
	{
		return NO;
	}
	// If this method is the one already installed, pretend to install it again.
	if (NULL != oldMethod && (oldMethod->imp == method->imp))
	{
		return NO;
	}
	SparseArrayInsert(dtable, sel_id, method);
	// In TDD mode, we also register the first typed method that we
	// encounter as the untyped version.
#ifdef TYPE_DEPENDENT_DISPATCH
	uint32_t untyped_idx = get_untyped_idx(method->selector);
	SparseArrayInsert(dtable, untyped_idx, method);
#endif

	static SEL cxx_construct, cxx_destruct;
	if (NULL == cxx_construct)
	{
		cxx_construct = sel_registerName(".cxx_construct");
		cxx_destruct = sel_registerName(".cxx_destruct");
	}
	if (selEqualUnTyped(method->selector, cxx_construct))
	{
		class->cxx_construct = method->imp;
	}
	else if (selEqualUnTyped(method->selector, cxx_destruct))
	{
		class->cxx_destruct = method->imp;
	}

	for (struct objc_class *subclass=class->subclass_list ; 
		Nil != subclass ; subclass = subclass->sibling_class)
	{
		// Don't bother updating dtables for subclasses that haven't been
		// initialized yet
		if (!classHasDtable(subclass)) { continue; }

		// Recursively install this method in all subclasses
		installMethodInDtable(subclass,
		                      dtable_for_class(subclass),
		                      method,
		                      oldMethod,
		                      YES);
	}

	// Invalidate the old slot, if there is one.
	if (NULL != oldMethod)
	{
		objc_method_cache_version++;
	}
	return YES;
}

static void installMethodsInClass(Class cls,
                                  SparseArray *methods_to_replace,
                                  SparseArray *methods,
                                  BOOL replaceExisting)
{
	SparseArray *dtable = dtable_for_class(cls);
	assert(uninstalled_dtable != dtable);

	uint32_t idx = 0;
	struct objc_method *m;
	while ((m = SparseArrayNext(methods, &idx)))
	{
		struct objc_method *method_to_replace = methods_to_replace
			?  SparseArrayLookup(methods_to_replace, m->selector->index)
			: NULL;
		if (!installMethodInDtable(cls, dtable, m, method_to_replace, replaceExisting))
		{
			// Remove this method from the list, if it wasn't actually installed
			SparseArrayInsert(methods, idx, 0);
		}
	}
}

Class class_getSuperclass(Class);

PRIVATE void objc_update_dtable_for_class(Class cls)
{
	// Only update real dtables
	if (!classHasDtable(cls)) { return; }

	LOCK_RUNTIME_FOR_SCOPE();

	SparseArray *methods = SparseArrayNewWithDepth(dtable_depth);
	collectMethodsForMethodListToSparseArray((void*)cls->methods, methods, YES);
	SparseArray *super_dtable = cls->super_class ? dtable_for_class(cls->super_class)
	                                             : NULL;
	installMethodsInClass(cls, super_dtable, methods, YES);
	SparseArrayDestroy(methods);
	checkARCAccessors(cls);
}

static void rebaseDtableRecursive(Class cls, Class newSuper)
{
	dtable_t parentDtable = dtable_for_class(newSuper);
	// Collect all of the methods for this class:
	dtable_t temporaryDtable = SparseArrayNewWithDepth(dtable_depth);

	for (struct objc_method_list *list = cls->methods ; list != NULL ; list = list->next)
	{
		for (unsigned i=0 ; i<list->count ; i++)
		{
			struct objc_method *m = method_at_index(list, i);
			uint32_t idx = m->selector->index;
			// Don't replace existing methods - we're doing the traversal
			// pre-order so we'll see methods from categories first.
			if (SparseArrayLookup(temporaryDtable, idx) == NULL)
			{
				SparseArrayInsert(temporaryDtable, idx, m);
			}
		}
	}


	dtable_t dtable = dtable_for_class(cls);
	uint32_t idx = 0;
	struct objc_method *method;
	// Install all methods from the parent that aren't overridden here.
	while ((method = SparseArrayNext(parentDtable, &idx)))
	{
		if (SparseArrayLookup(temporaryDtable, idx) == NULL)
		{
			SparseArrayInsert(dtable, idx, method);
			SparseArrayInsert(temporaryDtable, idx, method);
		}
	}
	idx = 0;
	// Now look at all of the methods in the dtable.  If they're not ones from
	// the dtable that we've just created, then they must have come from the
	// original superclass, so remove them by replacing them with NULL.
	while ((method = SparseArrayNext(dtable, &idx)))
	{
		if (SparseArrayLookup(temporaryDtable, idx) == NULL)
		{
			SparseArrayInsert(dtable, idx, NULL);
		}
	}
	SparseArrayDestroy(temporaryDtable);

	// merge can make a class ARC-compatible.
	checkARCAccessors(cls);

	// Now visit all of our subclasses and propagate the changes downwards.
	for (struct objc_class *subclass=cls->subclass_list ;
	     Nil != subclass ; subclass = subclass->sibling_class)
	{
		// Don't bother updating dtables for subclasses that haven't been
		// initialized yet
		if (!classHasDtable(subclass)) { continue; }
		rebaseDtableRecursive(subclass, cls);
	}

}

PRIVATE void objc_update_dtable_for_new_superclass(Class cls, Class newSuper)
{
	// Only update real dtables
	if (!classHasDtable(cls)) { return; }

	LOCK_RUNTIME_FOR_SCOPE();
	rebaseDtableRecursive(cls, newSuper);
	// Invalidate all caches after this operation.
	objc_method_cache_version++;

	return;
}

PRIVATE void add_method_list_to_class(Class cls,
                                      struct objc_method_list *list)
{
	// Only update real dtables
	if (!classHasDtable(cls)) { return; }

	LOCK_RUNTIME_FOR_SCOPE();

	SparseArray *methods = SparseArrayNewWithDepth(dtable_depth);
	SparseArray *super_dtable = cls->super_class ? dtable_for_class(cls->super_class)
	                                             : NULL;
	collectMethodsForMethodListToSparseArray(list, methods, NO);
	installMethodsInClass(cls, super_dtable, methods, YES);
	// Methods now contains only the new methods for this class.
	SparseArrayDestroy(methods);
	checkARCAccessors(cls);
}

PRIVATE dtable_t create_dtable_for_class(Class class, dtable_t root_dtable)
{
	// Don't create a dtable for a class that already has one
	if (classHasDtable(class)) { return dtable_for_class(class); }

	LOCK_RUNTIME_FOR_SCOPE();

	// Make sure that another thread didn't create the dtable while we were
	// waiting on the lock.
	if (classHasDtable(class)) { return dtable_for_class(class); }

	Class super = class_getSuperclass(class);
	dtable_t dtable;
	dtable_t super_dtable = NULL;

	if (Nil == super)
	{
		dtable = SparseArrayNewWithDepth(dtable_depth);
	}
	else
	{
		super_dtable = dtable_for_class(super);
		if (super_dtable == uninstalled_dtable)
		{
			if (super->isa == class)
			{
				super_dtable = root_dtable;
			}
			else
			{
				abort();
			}
		}
		dtable = SparseArrayCopy(super_dtable);
	}

	// When constructing the initial dtable for a class, we iterate along the
	// method list in forward-traversal order.  The first method that we
	// encounter is always the one that we want to keep, so we instruct
	// installMethodInDtable() to replace only methods that are inherited from
	// the superclass.
	struct objc_method_list *list = (void*)class->methods;

	while (NULL != list)
	{
		for (unsigned i=0 ; i<list->count ; i++)
		{
			struct objc_method *super_method = super_dtable
				? SparseArrayLookup(super_dtable, method_at_index(list, i)->selector->index)
				: NULL;
			installMethodInDtable(class, dtable, method_at_index(list, i), super_method, YES);
		}
		list = list->next;
	}

	return dtable;
}


Class class_table_next(void **e);

PRIVATE void objc_resize_dtables(uint32_t newSize)
{
	// If dtables already have enough space to store all registered selectors, do nothing
	if (1<<dtable_depth > newSize) { return; }

	LOCK_RUNTIME_FOR_SCOPE();

	if (1<<dtable_depth > newSize) { return; }

	dtable_depth += 8;

	uint32_t oldShift = uninstalled_dtable->shift;
	dtable_t old_uninstalled_dtable = uninstalled_dtable;

	uninstalled_dtable = SparseArrayExpandingArray(uninstalled_dtable, dtable_depth);
#if defined(WITH_TRACING) && defined (__x86_64)
	tracing_dtable = SparseArrayExpandingArray(tracing_dtable, dtable_depth);
#endif
	{
		LOCK_FOR_SCOPE(&initialize_lock);
		for (InitializingDtable *buffer = temporary_dtables ; NULL != buffer ; buffer = buffer->next)
		{
			buffer->dtable = SparseArrayExpandingArray(buffer->dtable, dtable_depth);
		}
	}
	// Resize all existing dtables
	void *e = NULL;
	struct objc_class *next;
	while ((next = class_table_next(&e)))
	{
		if (next->dtable == old_uninstalled_dtable)
		{
			next->dtable = uninstalled_dtable;
			next->isa->dtable = uninstalled_dtable;
			continue;
		}
		if (NULL != next->dtable &&
		    ((SparseArray*)next->dtable)->shift == oldShift)
		{
			next->dtable = SparseArrayExpandingArray((void*)next->dtable, dtable_depth);
			next->isa->dtable = SparseArrayExpandingArray((void*)next->isa->dtable, dtable_depth);
		}
	}
}

PRIVATE dtable_t objc_copy_dtable_for_class(dtable_t old, Class cls)
{
	return SparseArrayCopy(old);
}

PRIVATE void free_dtable(dtable_t dtable)
{
	SparseArrayDestroy(dtable);
}

LEGACY void update_dispatch_table_for_class(Class cls)
{
	static BOOL warned = NO;
	if (!warned)
	{
		fprintf(stderr, 
			"Warning: Calling deprecated private ObjC runtime function %s\n", __func__);
		warned = YES;
	}
	objc_update_dtable_for_class(cls);
}

void objc_resolve_class(Class);

__attribute__((unused)) static void objc_release_object_lock(id *x)
{
	objc_sync_exit(*x);
}
/**
 * Macro that is equivalent to @synchronize, for use in C code.
 */
#define LOCK_OBJECT_FOR_SCOPE(obj) \
	__attribute__((cleanup(objc_release_object_lock)))\
	__attribute__((unused)) id lock_object_pointer = obj;\
	objc_sync_enter(obj);

/**
 * Remove a buffer from an entry in the initializing dtables list.  This is
 * called as a cleanup to ensure that it runs even if +initialize throws an
 * exception.
 */
static void remove_dtable(InitializingDtable* meta_buffer)
{
	LOCK(&initialize_lock);
	InitializingDtable *buffer = meta_buffer->next;
	// Install the dtable:
	meta_buffer->class->dtable = meta_buffer->dtable;
	buffer->class->dtable = buffer->dtable;
	// Remove the look-aside buffer entry.
	if (temporary_dtables == meta_buffer)
	{
		temporary_dtables = buffer->next;
	}
	else
	{
		InitializingDtable *prev = temporary_dtables;
		while (prev->next->class != meta_buffer->class)
		{
			prev = prev->next;
		}
		prev->next = buffer->next;
	}
	UNLOCK(&initialize_lock);
}

/**
 * Send a +initialize message to the receiver, if required.  
 */
PRIVATE void objc_send_initialize(id object)
{
	Class class = classForObject(object);
	// If the first message is sent to an instance (weird, but possible and
	// likely for things like NSConstantString, make sure +initialize goes to
	// the class not the metaclass.  
	if (objc_test_class_flag(class, objc_class_flag_meta))
	{
		class = (Class)object;
	}
	Class meta = class->isa;


	// Make sure that the class is resolved.
	objc_resolve_class(class);

	// Make sure that the superclass is initialized first.
	if (Nil != class->super_class)
	{
		objc_send_initialize((id)class->super_class);
	}

	// Lock the runtime while we're creating dtables and before we acquire any
	// other locks.  This prevents a lock-order reversal when
	// dtable_for_class is called from something holding the runtime lock while
	// we're still holding the initialize lock.  We should ensure that we never
	// acquire the runtime lock after acquiring the initialize lock.
	LOCK_RUNTIME();

	// Superclass +initialize might possibly send a message to this class, in
	// which case this method would be called again.  See NSObject and
	// NSAutoreleasePool +initialize interaction in GNUstep.
	if (objc_test_class_flag(class, objc_class_flag_initialized))
	{
		// We know that initialization has started because the flag is set.
		// Check that it's finished by grabbing the class lock.  This will be
		// released once the class has been fully initialized. The runtime
		// lock needs to be released first to prevent a deadlock between the
		// runtime lock and the class-specific lock.
		UNLOCK_RUNTIME();

		objc_sync_enter((id)meta);
		objc_sync_exit((id)meta);
		assert(dtable_for_class(class) != uninstalled_dtable);
		return;
	}

	LOCK_OBJECT_FOR_SCOPE((id)meta);
	LOCK(&initialize_lock);
	if (objc_test_class_flag(class, objc_class_flag_initialized))
	{
		UNLOCK(&initialize_lock);
		UNLOCK_RUNTIME();
		return;
	}
	BOOL skipMeta = objc_test_class_flag(meta, objc_class_flag_initialized);
	// Mark metaclasses as never needing refcount manipulation for their
	// instances (classes).
	if (!skipMeta)
	{
		objc_set_class_flag(meta, objc_class_flag_permanent_instances);
	}

	// Set the initialized flag on both this class and its metaclass, to make
	// sure that +initialize is only ever sent once.
	objc_set_class_flag(class, objc_class_flag_initialized);
	objc_set_class_flag(meta, objc_class_flag_initialized);

	dtable_t class_dtable = create_dtable_for_class(class, uninstalled_dtable);
	dtable_t dtable = skipMeta ? 0 : create_dtable_for_class(meta, class_dtable);
	// Now we've finished doing things that may acquire the runtime lock, so we
	// can hold onto the initialise lock to make anything doing
	// dtable_for_class block until we've finished updating temporary dtable
	// lists.
	// If another thread holds the runtime lock, it can now proceed until it
	// gets into a dtable_for_class call, and then block there waiting for us
	// to finish setting up the temporary dtable.
	UNLOCK_RUNTIME();

	static SEL initializeSel = 0;
	if (0 == initializeSel)
	{
		initializeSel = sel_registerName("initialize");
	}

	struct objc_method *initializeSlot = skipMeta ? 0 :
			objc_dtable_lookup(dtable, initializeSel->index);

	// If there's no initialize method, then don't bother installing and
	// removing the initialize dtable, just install both dtables correctly now
	if (0 == initializeSlot)
	{
		if (!skipMeta)
		{
			meta->dtable = dtable;
		}
		class->dtable = class_dtable;
		checkARCAccessors(class);
		UNLOCK(&initialize_lock);
		return;
	}



	// Create an entry in the dtable look-aside buffer for this.  When sending
	// a message to this class in future, the lookup function will check this
	// buffer if the receiver's dtable is not installed, and block if
	// attempting to send a message to this class.
	InitializingDtable buffer = { class, class_dtable, temporary_dtables };
	__attribute__((cleanup(remove_dtable)))
	InitializingDtable meta_buffer = { meta, dtable, &buffer };
	temporary_dtables = &meta_buffer;
	// We now release the initialize lock.  We'll reacquire it later when we do
	// the cleanup, but at this point we allow other threads to get the
	// temporary dtable and call +initialize in other threads.
	UNLOCK(&initialize_lock);
	// We still hold the class lock at this point.  dtable_for_class will block
	// there after acquiring the temporary dtable.

	checkARCAccessors(class);

	// Store the buffer in the temporary dtables list.  Note that it is safe to
	// insert it into a global list, even though it's a temporary variable,
	// because we will clean it up after this function.
	initializeSlot->imp((id)class, initializeSel);
}

