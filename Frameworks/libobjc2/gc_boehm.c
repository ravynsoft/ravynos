#define GNUSTEP_LIBOBJC_NO_LEGACY
#include "objc/runtime.h"
#include "objc/toydispatch.h"
#include "class.h"
#include "ivar.h"
#include "lock.h"
#include "objc/objc-auto.h"
#include "visibility.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include "gc_ops.h"
#define I_HIDE_POINTERS


/**
 * Dispatch queue used to invoke finalizers.
 */
static dispatch_queue_t finalizer_queue;
/**
 * Should finalizers be invoked in their own thread?
 */
static BOOL finalizeThreaded;
/**
 * Should we do some (not 100% reliable) buffer overflow checking.
 */
static size_t canarySize;
/**
 * The canary value.  Used to check for overruns.  When an allocation is
 * finalized, we check whether it ends with this value.
 */
static uint32_t canary;
/**
 * Destination to write allocation log to.  This can be used to implement the
 * equivalent of malloc_history
 */
static FILE *allocationLog;

struct objc_slot* objc_get_slot(Class cls, SEL selector);

/*
 * Citing boehm-gc's README.linux:
 *
 * 3a) Every file that makes thread calls should define GC_LINUX_THREADS and
 *  _REENTRANT and then include gc.h.  Gc.h redefines some of the
 *  pthread primitives as macros which also provide the collector with
 *  information it requires.
 */
#ifdef __linux__
#	define GC_LINUX_THREADS

#	ifndef _REENTRANT
#		define _REENTRANT
#	endif

#endif
#include <gc/gc.h>
#include <gc/gc_typed.h>

#ifndef __has_builtin
#	define __has_builtin(x) 0
#endif
#if !__has_builtin(__sync_swap)
#define __sync_swap __sync_lock_test_and_set
#endif

void call_cxx_destruct(id obj);

#ifdef NO_EXECINFO
static inline void dump_stack(char *msg, void *addr) {}
#else 
#include <execinfo.h>
static inline void dump_stack(char *msg, void *addr)
{
	if (NULL == allocationLog) { return; }
	void *array[30];
	int frames = backtrace(array, 30);
	fprintf(allocationLog, "%s %p\n", msg, addr);
	fflush(allocationLog);
	backtrace_symbols_fd(array, frames, fileno(allocationLog));
	fflush(allocationLog);
}
#endif

Class dead_class;

Class objc_lookup_class(const char*);

GC_descr gc_typeForClass(Class cls);
void gc_setTypeForClass(Class cls, GC_descr type);

static unsigned long collectionType(unsigned options)
{
	// Low 2 bits in GC options are used for the
	return options & 3;
}

static size_t CollectRatio     = 0x10000;
static size_t CollectThreshold = 0x10000;

void objc_set_collection_threshold(size_t threshold)
{
	CollectThreshold = threshold;
}
void objc_set_collection_ratio(size_t ratio)
{
	CollectRatio = ratio;
}

void objc_collect(unsigned long options)
{
	size_t newAllocations = GC_get_bytes_since_gc();
	// Skip collection if we haven't allocated much memory and this is a
	// collect if needed collection
	if ((options & OBJC_COLLECT_IF_NEEDED) && (newAllocations < CollectThreshold))
	{
		return;
	}
	switch (collectionType(options))
	{
		case OBJC_RATIO_COLLECTION:
			if (newAllocations >= CollectRatio)
			{
				GC_gcollect();
			}
			else
			{
				GC_collect_a_little();
			}
			break;
		case OBJC_GENERATIONAL_COLLECTION:
			GC_collect_a_little();
			break;
		case OBJC_FULL_COLLECTION:
			GC_gcollect();
			break;
		case OBJC_EXHAUSTIVE_COLLECTION:
		{
			size_t freeBytes = 0;
			while (GC_get_free_bytes() != freeBytes)
			{
				freeBytes = GC_get_free_bytes();
				GC_gcollect();
			}
		}
	}
}

BOOL objc_collectingEnabled(void)
{
	return GC_dont_gc == 0;
}

void objc_gc_disable(void)
{
	GC_disable();
}
void objc_gc_enable(void)
{
	GC_enable();
}
void* objc_gc_collectable_address(void* ptr)
{
	return GC_base(ptr);
}

BOOL objc_atomicCompareAndSwapPtr(id predicate, id replacement, volatile id *objectLocation)
{
	return __sync_bool_compare_and_swap(objectLocation, predicate, replacement);
}
BOOL objc_atomicCompareAndSwapPtrBarrier(id predicate, id replacement, volatile id *objectLocation)
{
	return __sync_bool_compare_and_swap(objectLocation, predicate, replacement);
}

BOOL objc_atomicCompareAndSwapGlobal(id predicate, id replacement, volatile id *objectLocation)
{
	return objc_atomicCompareAndSwapPtr(predicate, replacement, objectLocation);
}
BOOL objc_atomicCompareAndSwapGlobalBarrier(id predicate, id replacement, volatile id *objectLocation)
{
	return objc_atomicCompareAndSwapPtr(predicate, replacement, objectLocation);
}
BOOL objc_atomicCompareAndSwapInstanceVariable(id predicate, id replacement, volatile id *objectLocation)
{
	return objc_atomicCompareAndSwapPtr(predicate, replacement, objectLocation);
}
BOOL objc_atomicCompareAndSwapInstanceVariableBarrier(id predicate, id replacement, volatile id *objectLocation)
{
	return objc_atomicCompareAndSwapPtr(predicate, replacement, objectLocation);
}

id objc_assign_strongCast(id val, id *ptr)
{
	*ptr = val;
	return val;
}

id objc_assign_global(id val, id *ptr)
{
	if (isGCEnabled)
	{
		GC_add_roots(ptr, ptr+1);
	}
	*ptr = val;
	return val;
}

id objc_assign_ivar(id val, id dest, ptrdiff_t offset)
{
	*(id*)((char*)dest+offset) = val;
	return val;
}

struct memmove_args
{
	void *dst;
	const void *src;
	size_t size;
};

static void* callMemmove(void *args)
{
	struct memmove_args *a = args;
	memmove(a->dst, a->src, a->size);
	return a->dst;
}

void *objc_memmove_collectable(void *dst, const void *src, size_t size)
{
	// For small copies, we copy onto the stack then copy from the stack.  This
	// ensures that pointers are always present in a scanned region.  For
	// larger copies, we just lock the GC to prevent it from freeing the memory
	// while the system memmove() does the real copy.  The first case avoids
	// the need to acquire the lock, but will perform worse for very large
	// copies since we are copying the data twice (and because stack space is
	// relatively scarce).
	if (size < 128)
	{
		char buffer[128];
		memcpy(buffer, src, size);
		__sync_synchronize();
		memcpy(dst, buffer, size);
		// In theory, we should zero the on-stack buffer here to prevent the GC
		// from seeing spurious pointers, but it's not really important because
		// the contents of the buffer is duplicated on the heap and overwriting
		// it will typically involve another copy to this function.  This will
		// not be the case if we are storing in a dead object, but that's
		// probably sufficiently infrequent that we shouldn't worry about
		// optimising for that case.
		return dst;
	}
	struct memmove_args args = {dst, src, size};
	return GC_call_with_alloc_lock(callMemmove, &args);
}
/**
 * Weak Pointers:
 *
 * To implement weak pointers, we store the hidden pointer (bits all flipped)
 * in the real address.  We tell the GC to zero the pointer when the associated
 * object is finalized.  The read barrier locks the GC to prevent it from
 * freeing anything, deobfuscates the pointer (at which point it becomes a
 * GC-visible on-stack pointer), and then returns it.
 */

static void *readWeakLocked(void *ptr)
{
	void *val = *(void**)ptr;
	return 0 == val ? val : REVEAL_POINTER(val);
}

id objc_read_weak(id *location)
{
	if (!isGCEnabled)
	{
		return *location;
	}
	return GC_call_with_alloc_lock(readWeakLocked, location);
}

id objc_assign_weak(id value, id *location)
{
	if (!isGCEnabled)
	{
		*location = value;
		return value;
	}
	// Temporarily zero this pointer and get the old value
	id old = __sync_swap(location, 0);
	if (0 != old)
	{
		GC_unregister_disappearing_link((void**)location);
	}
	// If the value is not GC'd memory (e.g. a class), the collector will crash
	// trying to collect it when you add it as the target of a disappearing
	// link.
	if (0 != GC_base(value))
	{
		GC_GENERAL_REGISTER_DISAPPEARING_LINK((void**)location, value);
	}
	// If some other thread has modified this, then we may have two different
	// objects registered to make this pointer 0 if either is destroyed.  This
	// would be bad, so we need to make sure that we unregister them and
	// register the correct one.
	if (!__sync_bool_compare_and_swap(location, old, (id)HIDE_POINTER(value)))
	{
		return objc_assign_weak(value, location);
	}
	return value;
}

static SEL finalize;
Class zombie_class;


static void runFinalize(void *addr, void *context)
{
	dump_stack("Freeing Object: ", addr);
	id obj = addr;
	size_t size = (uintptr_t)context;
	if ((canarySize > 0) &&
		(*(uint32_t*)((char*)obj + size) != canary))
	{
		fprintf(stderr, "Something wrote past the end of %p\n", addr);
		if (obj->isa != Nil)
		{
			fprintf(stderr, "Instance of %s\n", obj->isa->name);
		}
		abort();
	}
	//fprintf(stderr, "FINALIZING %p (%s)\n", addr, ((id)addr)->isa->name);
	if (Nil == obj->isa) { return; }
	struct objc_slot *slot = objc_get_slot(obj->isa, finalize);
	if (NULL != slot)
	{
		slot->method(obj, finalize);
	}
	call_cxx_destruct(obj);
	*(void**)addr = zombie_class;
}

static void collectIvarForClass(Class cls, GC_word *bitmap)
{
	for (unsigned i=0 ; (cls->ivars != 0) && (i<cls->ivars->count) ; i++)
	{
		struct objc_ivar *ivar = ivar_at_index(cls->ivars, i);
		size_t start = ivar->offset;
		size_t end = i+1 < cls->ivars->count ? ivar_at_index(cls->ivars, i+1)->offset
		                                     : cls->instance_size;
		switch (ivar->type[0])
		{
			case '[': case '{': case '(':
				// If the structure / array / union type doesn't contain any
				// pointers, then skip it.  We still need to be careful of packed
				if ((strchr(ivar->type, '^') == 0) &&
				    (strchr(ivar->type, '@') == 0))
				{
					break;
				}
			// Explicit pointer types
			case '^': case '@':
				for (unsigned b=(start / sizeof(void*)) ; b<(end/sizeof(void*)) ; b++)
				{
					GC_set_bit(bitmap, b);
				}
		}
	}
	if (cls->super_class)
	{
		collectIvarForClass(cls->super_class, bitmap);
	}
}

static GC_descr descriptor_for_class(Class cls)
{
	GC_descr descr = gc_typeForClass(cls);

	if (0 != descr) { return descr; }

	LOCK_RUNTIME_FOR_SCOPE();

	descr = (GC_descr)gc_typeForClass(cls);
	if (0 != descr) { return descr; }

	size_t size = cls->instance_size / 8 + 1;
	GC_word bitmap[size];
	memset(bitmap, 0, size);
	collectIvarForClass(cls, bitmap);
	// It's safe to round down here - if a class ends with an ivar that is
	// smaller than a pointer, then it can't possibly be a pointer.
	//fprintf(stderr, "Class is %d byes, %d words\n", cls->instance_size, cls->instance_size/sizeof(void*));
	descr = GC_make_descriptor(bitmap, cls->instance_size / sizeof(void*));
	gc_setTypeForClass(cls, descr);
	return descr;
}

static id allocate_class(Class cls, size_t extra)
{
	size_t size = class_getInstanceSize(cls);
	if (canarySize)
	{
		extra += 4;
	}
	id obj = 0;
	// If there are some extra bytes, they may contain pointers, so we ignore
	// the type
	if (extra > 0)
	{
		size += extra;
		// FIXME: Overflow checking!
		obj = GC_MALLOC(size);
	}
	else
	{
		obj = GC_MALLOC_EXPLICITLY_TYPED(size, descriptor_for_class(cls));
	}
	//fprintf(stderr, "Allocating %p (%s + %d).  Base is %p\n", obj, cls->name, extra, GC_base(obj));
	// It would be nice not to register a finaliser if the object didn't
	// implement finalize or .cxx_destruct methods.  Unfortunately, this is not
	// possible, because a class may add a finalize method as it runs.
	GC_REGISTER_FINALIZER_NO_ORDER(obj, runFinalize,
			(void*)(uintptr_t)size-canarySize, 0, 0);
	if (canarySize > 0)
	{
		*(uint32_t*)((char*)obj + size - canarySize) = canary;
	}
	dump_stack("Allocating object", obj);
	return obj;
}

static void free_object(id obj) {}
id objc_allocate_object(Class cls, int extra)
{
	return class_createInstance(cls, extra);
}

static void registerThread(BOOL errorOnNotRegistered)
{
	struct GC_stack_base base;
	if (GC_get_stack_base(&base) != GC_SUCCESS)
	{
		fprintf(stderr, "Unable to find stack base for new thread\n");
		abort();
	}
	switch (GC_register_my_thread(&base))
	{
		case GC_SUCCESS:
			if (errorOnNotRegistered)
			{
				fprintf(stderr, "Thread should have already been registered with the GC\n");
			}
		case GC_DUPLICATE:
			return;
		case GC_NO_THREADS:
		case GC_UNIMPLEMENTED:
			fprintf(stderr, "Unable to register stack\n");
			abort();
	}
}

void objc_registerThreadWithCollector(void)
{
	registerThread(NO);
}
void objc_unregisterThreadWithCollector(void)
{
	GC_unregister_my_thread();
}
void objc_assertRegisteredThreadWithCollector()
{
	registerThread(YES);
}

/**
 * Structure stored for each GC
 */
static struct gc_refcount
{
	/** Reference count */
	intptr_t refCount;
	/** Strong pointer */
	id ptr;
} null_refcount = {0};

static int refcount_compare(const void *ptr, struct gc_refcount rc)
{
	return ptr == rc.ptr;
}
static uint32_t ptr_hash(const void *ptr)
{
	// Bit-rotate right 4, since the lowest few bits in an object pointer will
	// always be 0, which is not so useful for a hash value
	return ((uintptr_t)ptr >> 4) | ((uintptr_t)ptr << ((sizeof(id) * 8) - 4));
}
static uint32_t refcount_hash(struct gc_refcount rc)
{
	return ptr_hash(rc.ptr);
}
static int isEmpty(struct gc_refcount rc)
{
	return rc.ptr == NULL;
}
#define MAP_TABLE_VALUE_NULL isEmpty
#define MAP_TABLE_NAME refcount
#define MAP_TABLE_COMPARE_FUNCTION refcount_compare
#define MAP_TABLE_HASH_KEY ptr_hash
#define MAP_TABLE_HASH_VALUE refcount_hash
#define MAP_TABLE_VALUE_TYPE struct gc_refcount
#define MAP_TABLE_VALUE_PLACEHOLDER null_refcount
#define MAP_TABLE_TYPES_BITMAP (1<<(offsetof(struct gc_refcount, ptr) / sizeof(void*)))
#define MAP_TABLE_ACCESS_BY_REFERENCE
#include "hash_table.h"

static refcount_table *refcounts;

id objc_gc_retain(id object)
{
	struct gc_refcount *refcount = refcount_table_get(refcounts, object);
	if (NULL == refcount)
	{
		LOCK_FOR_SCOPE(&(refcounts->lock));
		refcount = refcount_table_get(refcounts, object);
		if (NULL == refcount)
		{
			struct gc_refcount rc = { 1, object};
			refcount_insert(refcounts, rc);
			return object;
		}
	}
	__sync_fetch_and_add(&(refcount->refCount), 1);
	return object;
}
void objc_gc_release(id object)
{
	struct gc_refcount *refcount = refcount_table_get(refcounts, object);
	// This object has not been explicitly retained, don't release it
	if (0 == refcount) { return; }

	if (0 == __sync_sub_and_fetch(&(refcount->refCount), 1))
	{
		LOCK_FOR_SCOPE(&(refcounts->lock));
		refcount->ptr = 0;
		__sync_synchronize();
		// If another thread has incremented the reference count while we were
		// doing this, then we need to add the count back into the table,
		// otherwise we can carry on.
		if (!__sync_bool_compare_and_swap(&(refcount->refCount), 0, 0))
		{
			refcount->ptr = object;
		}
	}
}
int objc_gc_retain_count(id object)
{
	struct gc_refcount *refcount = refcount_table_get(refcounts, object);
	return (0 == refcount) ? 0 : refcount->refCount;
}


static void nuke_buffer(void *addr, void *s)
{
	return;
	dump_stack("Freeing allocation: ", addr);
	uintptr_t size = (uintptr_t)s;
	if (canary != *(uint32_t*)((char*)addr + size))
	{
		fprintf(stderr,
		        "Something wrote past the end of memory allocation %p\n",
		        addr);
		abort();
	}
	memset(addr, 0, size);
}

void* objc_gc_allocate_collectable(size_t size, BOOL isScanned)
{
	void *buffer;
	if (isScanned)
	{
		buffer = GC_MALLOC(size+canarySize);
	}
	else
	{
		buffer = GC_MALLOC_ATOMIC(size+canarySize);
		memset(buffer, 0, size);
	}
	if (canarySize > 0)
	{
		*(uint32_t*)((char*)buffer + size) = canary;
		GC_REGISTER_FINALIZER_NO_ORDER(buffer, nuke_buffer,
				(void*)(uintptr_t)size, 0, 0);
	}
	dump_stack("Allocating memory", buffer);
	return buffer;
}
void* objc_gc_reallocate_collectable(void *ptr, size_t size, BOOL isScanned)
{
	if (0 == size) { return 0; }
	void *new = isScanned ? GC_MALLOC(size) : GC_MALLOC_ATOMIC(size);

	if (0 == new) { return 0; }

	if (NULL != ptr)
	{
		size_t oldSize = GC_size(ptr);
		if (oldSize < size)
		{
			size = oldSize;
		}
		memcpy(new, ptr, size);
	}
	dump_stack("New allocation from realloc: ", new);
	return new;
}

static void collectAndDumpStats(int signalNo)
{
	objc_collect(OBJC_EXHAUSTIVE_COLLECTION);
	GC_dump();
}

static void deferredFinalizer(void)
{
	GC_invoke_finalizers();
}

static void runFinalizers(void)
{
	//fprintf(stderr, "RUNNING FINALIZERS\n");
	if (finalizeThreaded)
	{
		dispatch_async_f(finalizer_queue, deferredFinalizer, NULL);
	}
	else
	{
		GC_invoke_finalizers();
	}
}

PRIVATE void init_gc(void)
{
	//GC_no_dls = 1;
	//GC_enable_incremental();
	GC_INIT();
	char *envValue;
	// Dump GC stats on exit - uncomment when debugging.
	if (getenv("LIBOBJC_DUMP_GC_STATUS_ON_EXIT"))
	{
		atexit(GC_dump);
	}
	if ((envValue = getenv("LIBOBJC_LOG_ALLOCATIONS")))
	{
		allocationLog = fopen(envValue, "a");
	}
	if ((envValue = getenv("LIBOBJC_CANARIES")))
	{
		unsigned s = envValue[0] ? strtol(envValue, NULL, 10) : 123;
		srandom(s);
		canarySize = sizeof(uint32_t);
		canary = random();
	}
	if ((envValue = getenv("LIBOBJC_DUMP_GC_STATUS_ON_SIGNAL")))
	{
		int s = envValue[0] ? (int)strtol(envValue, NULL, 10) : SIGUSR2;
		signal(s, collectAndDumpStats);
	}
	//GC_clear_roots();
}

BOOL objc_collecting_enabled(void)
{
	// Lock the GC in the current state once it's been queried.  This prevents
	// the loading of any modules with an incompatible GC mode.
	current_gc_mode = isGCEnabled ? GC_Required : GC_None;
	return isGCEnabled;
}

void objc_startCollectorThread(void)
{
	if (YES == finalizeThreaded) { return; }
	finalizer_queue = dispatch_queue_create("ObjC finalizeation thread", 0);
	finalizeThreaded = YES;
}

void objc_clear_stack(unsigned long options)
{
	// This isn't a very good implementation - we should really be working out
	// how much stack space is left somehow, but this is not possible to do
	// portably.
	int i[1024];
	int *addr = &i[0];
	memset(addr, 0, 1024);
	// Tell the compiler that something that it doesn't know about is touching
	// this memory, so it shouldn't optimise the allocation and memset away.
	__asm__  volatile ("" :  : "m"(addr) : "memory");

}
BOOL objc_is_finalized(void *ptr)
{
	return *(Class*)ptr == zombie_class;
}
// FIXME: Stub implementation that should be replaced with something better
void objc_finalizeOnMainThread(Class cls) {}

static void *debug_malloc(size_t s)
{
	return GC_MALLOC_UNCOLLECTABLE(s);
}
static void debug_free(void *ptr)
{
	GC_FREE(ptr);
}

PRIVATE struct gc_ops gc_ops_boehm =
{
	.allocate_class = allocate_class,
	.free_object    = free_object,
	.malloc         = debug_malloc,
	.free           = debug_free,
};

extern struct objc_class _NSConcreteStackBlock;
void *_Block_copy(void *src);

PRIVATE void enableGC(BOOL exclude)
{
	isGCEnabled = YES;
	gc = &gc_ops_boehm;
	refcount_initialize(&refcounts, 4096);
	finalize = sel_registerName("finalize");
	GC_finalizer_notifier = runFinalizers;
}
