#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "objc/runtime.h"
#include "objc/objc-arc.h"
#include "nsobject.h"
#include "spinlock.h"
#include "class.h"
#include "dtable.h"
#include "selector.h"
#include "lock.h"
#include "gc_ops.h"

/**
 * A single associative reference.  Contains the key, value, and association
 * policy.
 */
struct reference
{
	/**
	 * The key used for identifying this object.  Opaque pointer, should be set
	 * to 0 when this slot is unused.
	 */
	const void *key;
	/**
	 * The associated object.  Note, if the policy is assign then this may be
	 * some other type of pointer...
	 */
	void *object;
	/**
	 * Association policy.
	 */
	uintptr_t policy;
};

#define REFERENCE_LIST_SIZE 10

/**
 * Linked list of references associated with an object.  We assume that there
 * won't be very many, so we don't bother with a proper hash table, and just
 * iterate over a list.
 */
struct reference_list
{
	/**
	 * Next group of references.  This is only ever used if we have more than
	 * 10 references associated with an object, which seems highly unlikely.
	 */
	struct reference_list *next;
	/**
	 * Mutex.  Only set for the first reference list in a chain.  Used for
	 * @syncronize().
	 */
	mutex_t lock;
	/**
	 * Garbage collection type.  This stores the location of all of the
	 * instance variables in the object that may contain pointers.
	 */
	void *gc_type;
	/**
	 * Array of references.
	 */
	struct reference list[REFERENCE_LIST_SIZE];
};
enum
{
	OBJC_ASSOCIATION_ATOMIC = 0x300,
};

static BOOL isAtomic(uintptr_t policy)
{
	return (policy & OBJC_ASSOCIATION_ATOMIC) == OBJC_ASSOCIATION_ATOMIC;
}

static struct reference* findReference(struct reference_list *list, const void *key)
{
	while (list)
	{
		for (int i=0 ; i<REFERENCE_LIST_SIZE ; i++)
		{
			if (list->list[i].key == key)
			{
				return &list->list[i];
			}
		}
		list = list->next;
	}
	return NULL;
}
static void cleanupReferenceList(struct reference_list *list)
{
	if (NULL == list) { return; }

	cleanupReferenceList(list->next);

	for (int i=0 ; i<REFERENCE_LIST_SIZE ; i++)
	{
		struct reference *r = &list->list[i];
		if (0 != r->key)
		{
			r->key = 0;
			if (OBJC_ASSOCIATION_ASSIGN != r->policy)
			{
				// Full barrier - ensure that we've zero'd the key before doing
				// this!
				__sync_synchronize();
				objc_release(r->object);
			}
			r->object = 0;
			r->policy = 0;
		}
	}
}

static void freeReferenceList(struct reference_list *l)
{
	if (NULL == l) { return; }
	freeReferenceList(l->next);
	gc->free(l);
}

static void setReference(struct reference_list *list,
                         const void *key,
                         void *obj,
                         uintptr_t policy)
{
	switch (policy)
	{
		// Ignore any unknown association policies
		default: return;
		case OBJC_ASSOCIATION_COPY_NONATOMIC:
		case OBJC_ASSOCIATION_COPY:
			obj = [(id)obj copy];
			break;
		case OBJC_ASSOCIATION_RETAIN_NONATOMIC:
		case OBJC_ASSOCIATION_RETAIN:
			obj = objc_retain(obj);
		case OBJC_ASSOCIATION_ASSIGN:
			break;
	}
	// While inserting into the list, we need to lock it temporarily.
	volatile int *lock = lock_for_pointer(list);
	lock_spinlock(lock);
	struct reference *r = findReference(list, key);
	// If there's an existing reference, then we can update it, otherwise we
	// have to install a new one
	if (NULL == r)
	{
		// Search for an unused slot
		r = findReference(list, 0);
		if (NULL == r)
		{
			struct reference_list *l = list;

			while (NULL != l->next) { l = l->next; }

			l->next = gc->malloc(sizeof(struct reference_list));
			r = &l->next->list[0];
		}
		r->key = key;
	}
	unlock_spinlock(lock);
	// Now we only need to lock if the old or new property is atomic
	BOOL needLock = isAtomic(r->policy) || isAtomic(policy);
	if (needLock)
	{
		lock = lock_for_pointer(r);
		lock_spinlock(lock);
	}
	@try
	{
		if (OBJC_ASSOCIATION_ASSIGN != r->policy)
		{
			objc_release(r->object);
		}
	}
	@finally
	{
		r->policy = policy;
		r->object = obj;
	}
	if (needLock)
	{
		unlock_spinlock(lock);
	}
}

static void deallocHiddenClass(id obj, SEL _cmd);

static inline Class findHiddenClass(id obj)
{
	Class cls = obj->isa;
	while (Nil != cls && 
	       !objc_test_class_flag(cls, objc_class_flag_assoc_class))
	{
		cls = class_getSuperclass(cls);
	}
	return cls;
}

static Class allocateHiddenClass(Class superclass)
{
	Class newClass =
		calloc(1, sizeof(struct objc_class) + sizeof(struct reference_list));

	if (Nil == newClass) { return Nil; }

	// Set up the new class
	newClass->isa = superclass->isa;
	newClass->name = superclass->name;
	// Uncomment this for debugging: it makes it easier to track which hidden
	// class is which
	// static int count;
	//asprintf(&newClass->name, "%s%d", superclass->name, count++);
	newClass->info = objc_class_flag_resolved | objc_class_flag_user_created |
		objc_class_flag_hidden_class | objc_class_flag_assoc_class;
	newClass->super_class = superclass;
	newClass->dtable = uninstalled_dtable;
	newClass->instance_size = superclass->instance_size;

	LOCK_RUNTIME_FOR_SCOPE();
	newClass->sibling_class = superclass->subclass_list;
	superclass->subclass_list = newClass;

	return newClass;
}

static inline Class initHiddenClassForObject(id obj)
{
	Class hiddenClass = allocateHiddenClass(obj->isa); 
	assert(!class_isMetaClass(obj->isa));
	static SEL cxx_destruct;
	if (NULL == cxx_destruct)
	{
		cxx_destruct = sel_registerName(".cxx_destruct");
	}
	const char *types = sizeof(void*) == 4 ? "v8@0:4" : "v16@0:8";
	class_addMethod(hiddenClass, cxx_destruct,
		(IMP)deallocHiddenClass, types);
	obj->isa = hiddenClass;
	return hiddenClass;
}

static void deallocHiddenClass(id obj, SEL _cmd)
{
	LOCK_RUNTIME_FOR_SCOPE();
	Class hiddenClass = findHiddenClass(obj);
	// After calling [super dealloc], the object will no longer exist.
	// Free the hidden class.
	struct reference_list *list = object_getIndexedIvars(hiddenClass);
	DESTROY_LOCK(&list->lock);
	cleanupReferenceList(list);
	freeReferenceList(list->next);
	//fprintf(stderr, "Deallocating dtable %p\n", hiddenClass->dtable);
	free_dtable(hiddenClass->dtable);
	// We shouldn't have any subclasses left at this point
	assert(hiddenClass->subclass_list == 0);
	// Remove the class from the subclass list of its superclass
	Class sub = hiddenClass->super_class->subclass_list;
	if (sub == hiddenClass)
	{
		hiddenClass->super_class->subclass_list = hiddenClass->sibling_class;
	}
	else
	{
		while (sub != NULL)
		{
			if ((Class)sub->sibling_class == hiddenClass)
			{
				sub->sibling_class = hiddenClass->sibling_class;
				break;
			}
			sub = sub->sibling_class;
		}
	}
	obj->isa = hiddenClass->super_class;
	// Free the introspection structures:
	freeMethodLists(hiddenClass);
	freeIvarLists(hiddenClass);
	// Free the class
	free(hiddenClass);
}

static struct reference_list* referenceListForObject(id object, BOOL create)
{
	if (class_isMetaClass(object->isa))
	{
		Class cls = (Class)object;
		if ((NULL == cls->extra_data) && create)
		{
			volatile int *lock = lock_for_pointer(cls);
			struct reference_list *list = gc->malloc(sizeof(struct reference_list));
			lock_spinlock(lock);
			if (NULL == cls->extra_data)
			{
				INIT_LOCK(list->lock);
				cls->extra_data = list;
				unlock_spinlock(lock);
			}
			else
			{
				unlock_spinlock(lock);
				gc->free(list);
			}
		}
		return cls->extra_data;
	}
	Class hiddenClass = findHiddenClass(object);
	if ((NULL == hiddenClass) && create)
	{
		volatile int *lock = lock_for_pointer(object);
		lock_spinlock(lock);
		hiddenClass = findHiddenClass(object);
		if (NULL == hiddenClass)
		{
			hiddenClass = initHiddenClassForObject(object);
			struct reference_list *list = object_getIndexedIvars(hiddenClass);
			INIT_LOCK(list->lock);
		}
		unlock_spinlock(lock);
	}
	return hiddenClass ? object_getIndexedIvars(hiddenClass) : NULL;
}

void objc_setAssociatedObject(id object,
                              const void *key,
                              id value,
                              objc_AssociationPolicy policy)
{
	if (isSmallObject(object)) { return; }
	struct reference_list *list = referenceListForObject(object, YES);
	setReference(list, key, value, policy);
}

id objc_getAssociatedObject(id object, const void *key)
{
	if (isSmallObject(object)) { return nil; }
	struct reference_list *list = referenceListForObject(object, NO);
	if (NULL == list) { return nil; }
	struct reference *r = findReference(list, key);
	if (NULL != r)
	{
		return r->object;
	}
	if (class_isMetaClass(object->isa))
	{
		return nil;
	}
	Class cls = object->isa;
	while (Nil != cls)
	{
		while (Nil != cls && 
			   !objc_test_class_flag(cls, objc_class_flag_assoc_class))
		{
			cls = class_getSuperclass(cls);
		}
		if (Nil != cls)
		{
			struct reference_list *next_list = object_getIndexedIvars(cls);
			if (list != next_list)
			{
				list = next_list;
				struct reference *r = findReference(list, key);
				if (NULL != r)
				{
					return r->object;
				}
			}
			cls = class_getSuperclass(cls);
		}
	}
	return nil;
}


void objc_removeAssociatedObjects(id object)
{
	if (isSmallObject(object)) { return; }
	cleanupReferenceList(referenceListForObject(object, NO));
}

PRIVATE void *gc_typeForClass(Class cls)
{
	struct reference_list *list = referenceListForObject(cls, YES);
	return list->gc_type;
}
PRIVATE void gc_setTypeForClass(Class cls, void *type)
{
	struct reference_list *list = referenceListForObject(cls, YES);
	list->gc_type = type;
}

OBJC_PUBLIC
int objc_sync_enter(id object)
{
	if ((object == 0) || isSmallObject(object)) { return 0; }
	struct reference_list *list = referenceListForObject(object, YES);
	LOCK(&list->lock);
	return 0;
}

OBJC_PUBLIC
int objc_sync_exit(id object)
{
	if ((object == 0) || isSmallObject(object)) { return 0; }
	struct reference_list *list = referenceListForObject(object, NO);
	if (NULL != list)
	{
		UNLOCK(&list->lock);
		return 0;
	}
	return 1;
}

static Class hiddenClassForObject(id object)
{
	if (isSmallObject(object)) { return nil; }
	if (class_isMetaClass(object->isa))
	{
		return object->isa;
	}
	Class hiddenClass = findHiddenClass(object);
	if (NULL == hiddenClass)
	{
		volatile int *lock = lock_for_pointer(object);
		lock_spinlock(lock);
		hiddenClass = findHiddenClass(object);
		if (NULL == hiddenClass)
		{
			hiddenClass = initHiddenClassForObject(object);
			struct reference_list *list = object_getIndexedIvars(hiddenClass);
			INIT_LOCK(list->lock);
		}
		unlock_spinlock(lock);
	}
	return hiddenClass;
}

BOOL object_addMethod_np(id object, SEL name, IMP imp, const char *types)
{
	return class_addMethod(hiddenClassForObject(object), name, imp, types);
}

IMP object_replaceMethod_np(id object, SEL name, IMP imp, const char *types)
{
	return class_replaceMethod(hiddenClassForObject(object), name, imp, types);
}
static char prototypeKey;

id object_clone_np(id object)
{
	if (isSmallObject(object)) { return object; }
	// Make sure that the prototype has a hidden class, so that methods added
	// to it will appear in the clone.
	referenceListForObject(object, YES);
	id new = class_createInstance(object->isa, 0);
	Class hiddenClass = initHiddenClassForObject(new);
	struct reference_list *list = object_getIndexedIvars(hiddenClass);
	INIT_LOCK(list->lock);
	objc_setAssociatedObject(new, &prototypeKey, object,
			OBJC_ASSOCIATION_RETAIN_NONATOMIC);
	return new;
}

id object_getPrototype_np(id object)
{
	return objc_getAssociatedObject(object, &prototypeKey);
}
