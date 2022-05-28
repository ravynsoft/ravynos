#define _LIBCPP_NO_EXCEPTIONS 1
#define TSL_NO_EXCEPTIONS 1
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "third_party/robin-map/include/tsl/robin_map.h"
#import "lock.h"
#import "objc/runtime.h"
#import "objc/blocks_runtime.h"
#import "nsobject.h"
#import "class.h"
#import "selector.h"
#import "visibility.h"
#import "objc/hooks.h"
#import "objc/objc-arc.h"
#import "objc/blocks_runtime.h"

extern "C" id (*_objc_weak_load)(id object);

#if defined(_WIN32)
// We're using the Fiber-Local Storage APIs on Windows
// because the TLS APIs won't pass app certification.
// Additionally, the FLS API surface is 1:1 mapped to
// the TLS API surface when fibers are not in use.
#	include "safewindows.h"
#	define arc_tls_store FlsSetValue
#	define arc_tls_load FlsGetValue
#	define TLS_CALLBACK(name) void WINAPI name

typedef DWORD arc_tls_key_t;
typedef void WINAPI(*arc_cleanup_function_t)(void*);
static inline arc_tls_key_t arc_tls_key_create(arc_cleanup_function_t cleanupFunction)
{
	return FlsAlloc(cleanupFunction);
}

#else // if defined(_WIN32)

#	ifndef NO_PTHREADS
#		include <pthread.h>
#		define arc_tls_store pthread_setspecific
#		define arc_tls_load pthread_getspecific
#		define TLS_CALLBACK(name) void name

typedef pthread_key_t arc_tls_key_t;
typedef void (*arc_cleanup_function_t)(void*);
static inline arc_tls_key_t arc_tls_key_create(arc_cleanup_function_t cleanupFunction)
{
	pthread_key_t key;
	pthread_key_create(&key, cleanupFunction);
	return key;
}
#	endif
#endif

#ifdef arc_tls_store
arc_tls_key_t ARCThreadKey;
#endif

extern "C"
{
	extern struct objc_class _NSConcreteMallocBlock;
	extern struct objc_class _NSConcreteStackBlock;
	extern struct objc_class _NSConcreteGlobalBlock;
}

@interface NSAutoreleasePool
+ (Class)class;
+ (id)new;
- (void)release;
@end

#define POOL_SIZE (4096 / sizeof(void*) - (2 * sizeof(void*)))
/**
 * Structure used for ARC-managed autorelease pools.  This structure should be
 * exactly one page in size, so that it can be quickly allocated.  This does
 * not correspond directly to an autorelease pool.  The 'pool' returned by
 * objc_autoreleasePoolPush() may be an interior pointer to one of these
 * structures.
 */
struct arc_autorelease_pool
{
	/**
	 * Pointer to the previous autorelease pool structure in the chain.  Set
	 * when pushing a new structure on the stack, popped during cleanup.
	 */
	struct arc_autorelease_pool *previous;
	/**
	 * The current insert point.
	 */
	id *insert;
	/**
	 * The remainder of the page, an array of object pointers.  
	 */
	id pool[POOL_SIZE];
};

struct arc_tls
{
	struct arc_autorelease_pool *pool;
	id returnRetained;
};

/**
 * Type-safe wrapper around calloc.
 */
template<typename T>
static inline T* new_zeroed()
{
	return static_cast<T*>(calloc(sizeof(T), 1));
}

#ifdef _LIBCPP_BEGIN_NAMESPACE_STD
//_LIBCPP_BEGIN_NAMESPACE_STD
/**
 * If we're compiling with libc++, we need to instantiate the vector base class
 * to avoid linker errors.  Libc++ defines the functions that throw exceptions
 * in the headers but makes them extern templates to avoid code duplication in
 * the slow paths.
 */
//template class __attribute__((visibility("hidden")))  __vector_base_common<true>;
//_LIBCPP_END_NAMESPACE_STD
#endif


static inline struct arc_tls* getARCThreadData(void)
{
#ifndef arc_tls_store
	return NULL;
#else // !defined arc_tls_store
	auto tls = static_cast<struct arc_tls*>(arc_tls_load(ARCThreadKey));
	if (NULL == tls)
	{
		tls = new_zeroed<struct arc_tls>();
		arc_tls_store(ARCThreadKey, tls);
	}
	return tls;
#endif
}
static inline void release(id obj);

/**
 * Empties objects from the autorelease pool, stating at the head of the list
 * specified by pool and continuing until it reaches the stop point.  If the stop point is NULL then 
 */
static void emptyPool(struct arc_tls *tls, void *stop)
{
	struct arc_autorelease_pool *stopPool = NULL;
	if (NULL != stop)
	{
		stopPool = tls->pool;
		while (1)
		{
			// Invalid stop location
			if (NULL == stopPool)
			{
				return;
			}
			// NULL is the placeholder for the top-level pool
			if (NULL == stop && stopPool->previous == NULL)
			{
				break;
			}
			// Stop location was found in this pool
			if ((stop >= stopPool->pool) && (stop < &stopPool->pool[POOL_SIZE]))
			{
				break;
			}
			stopPool = stopPool->previous;
		}
	}
	while (tls->pool != stopPool)
	{
		while (tls->pool->insert > tls->pool->pool)
		{
			tls->pool->insert--;
			// This may autorelease some other objects, so we have to work in
			// the case where the autorelease pool is extended during a -release.
			release(*tls->pool->insert);
		}
		void *old = tls->pool;
		tls->pool = tls->pool->previous;
		free(old);
	}
	if (NULL != tls->pool)
	{
		while ((stop == NULL || (tls->pool->insert > stop)) &&
		       (tls->pool->insert > tls->pool->pool))
		{
			tls->pool->insert--;
			release(*tls->pool->insert);
		}
	}
	//fprintf(stderr, "New insert: %p.  Stop: %p\n", tls->pool->insert, stop);
}

#ifdef arc_tls_store
static TLS_CALLBACK(cleanupPools)(struct arc_tls* tls)
{
	if (tls->returnRetained)
	{
		release(tls->returnRetained);
		tls->returnRetained = nil;
	}
	if (NULL != tls->pool)
	{
		emptyPool(tls, NULL);
		assert(NULL == tls->pool);
	}
	if (tls->returnRetained)
	{
		cleanupPools(tls);
	}
	free(tls);
}
#endif


static Class AutoreleasePool;
static IMP NewAutoreleasePool;
static IMP DeleteAutoreleasePool;
static IMP AutoreleaseAdd;

extern BOOL FastARCRetain;
extern BOOL FastARCRelease;
extern BOOL FastARCAutorelease;

static BOOL useARCAutoreleasePool;

static const long refcount_shift = 1;
/**
 * We use the top bit of the reference count to indicate whether an object has
 * ever had a weak reference taken.  This lets us avoid acquiring the weak
 * table lock for most objects on deallocation.
 */
static const size_t weak_mask = ((size_t)1)<<((sizeof(size_t)*8)-refcount_shift);
/**
 * All of the bits other than the top bit are the real reference count.
 */
static const size_t refcount_mask = ~weak_mask;

extern "C" OBJC_PUBLIC size_t object_getRetainCount_np(id obj)
{
	uintptr_t *refCount = ((uintptr_t*)obj) - 1;
	uintptr_t refCountVal = __sync_fetch_and_add(refCount, 0);
	return (((size_t)refCountVal) & refcount_mask)/* + 1*/;
}

extern "C" OBJC_PUBLIC id objc_retain_fast_np(id obj)
{
	uintptr_t *refCount = ((uintptr_t*)obj) - 1;
	uintptr_t refCountVal = __sync_fetch_and_add(refCount, 0);
	uintptr_t newVal = refCountVal;
	do {
		refCountVal = newVal;
		size_t realCount = refCountVal & refcount_mask;
		// If this object's reference count is already less than 0, then
		// this is a spurious retain.  This can happen when one thread is
		// attempting to acquire a strong reference from a weak reference
		// and the other thread is attempting to destroy it.  The
		// deallocating thread will decrement the reference count with no
		// locks held and will then acquire the weak ref table lock and
		// attempt to zero the weak references.  The caller of this will be
		// `objc_loadWeakRetained`, which will also hold the lock.  If the
		// serialisation is such that the locked retain happens after the
		// decrement, then we return nil here so that the weak-to-strong
		// transition doesn't happen and the object is actually destroyed.
		// If the serialisation happens the other way, then the locked
		// check of the reference count will happen after we've referenced
		// this and we don't zero the references or deallocate.
		if (realCount < 0)
		{
			return nil;
		}
		// If the reference count is saturated, don't increment it.
		if (realCount == refcount_mask)
		{
			return obj;
		}
		realCount++;
		realCount |= refCountVal & weak_mask;
		uintptr_t updated = (uintptr_t)realCount;
		newVal = __sync_val_compare_and_swap(refCount, refCountVal, updated);
	} while (newVal != refCountVal);
	return obj;
}

__attribute__((always_inline))
static inline BOOL isPersistentObject(id obj)
{
	// No reference count manipulations on nil objects.
	if (obj == nil)
	{
		return YES;
	}
	// Small objects are never accessibly by reference
	if (isSmallObject(obj))
	{
		return YES;
	}
	// Persistent objects are persistent.  Safe to access isa directly here
	// because we've already handled the small object case separately.
	return objc_test_class_flag(obj->isa, objc_class_flag_permanent_instances);
}

static inline id retain(id obj)
{
	if (isPersistentObject(obj)) { return obj; }
	Class cls = obj->isa;
	if ((Class)&_NSConcreteMallocBlock == cls ||
	    (Class)&_NSConcreteStackBlock == cls)
	{
		return Block_copy(obj);
	}
	if (objc_test_class_flag(cls, objc_class_flag_fast_arc))
	{
		return objc_retain_fast_np(obj);
	}
	return [obj retain];
}

extern "C" OBJC_PUBLIC void object_incrementExternalRefCount(id value)
{
	objc_retain_fast_np(value);
// 	fprintf(stderr,"object_incrementExternalRefCount %p = %d\n",value,object_getRetainCount_np(value));
}


extern "C" OBJC_PUBLIC void object_decrementExternalRefCount(id value)
{
	objc_release_fast_np(value);
// 	fprintf(stderr,"object_decrementExternalRefCount %p = %d\n",value,object_getRetainCount_np(value));
}

#define objc_DecrementExtraRefCountWasZero(object) objc_release_fast_no_destroy_np(object)
extern "C" OBJC_PUBLIC BOOL objc_release_fast_no_destroy_np(id obj)
{
	uintptr_t *refCount = ((uintptr_t*)obj) - 1;
	uintptr_t refCountVal = __sync_fetch_and_add(refCount, 0);
	uintptr_t newVal = refCountVal;
	bool isWeak;
	bool shouldFree;
	do {
		refCountVal = newVal;
		size_t realCount = refCountVal & refcount_mask;
		// If the reference count is saturated, don't decrement it.
		if (realCount == refcount_mask)
		{
			return NO;
		}
		realCount--;
		isWeak = (refCountVal & weak_mask) == weak_mask;
		shouldFree = realCount == -1;
		realCount |= refCountVal & weak_mask;
		uintptr_t updated = (uintptr_t)realCount;
		newVal = __sync_val_compare_and_swap(refCount, refCountVal, updated);
	} while (newVal != refCountVal);
	// We allow refcounts to run into the negative, but should only
	// deallocate once.
	if (shouldFree)
	{
		if (isWeak)
		{
			if (!objc_delete_weak_refs(obj))
			{
				return NO;
			}
		}
		return YES;
	}
	return NO;
}

extern "C" OBJC_PUBLIC void objc_release_fast_np(id obj)
{
	if (objc_release_fast_no_destroy_np(obj))
	{
		[obj dealloc];
	}
}

static inline void release(id obj)
{
	if (isPersistentObject(obj)) { return; }
	Class cls = obj->isa;
	if (cls == static_cast<Class>(&_NSConcreteMallocBlock))
	{
		_Block_release(obj);
		return;
	}
	if (cls == static_cast<Class>(&_NSConcreteStackBlock))
	{
		return;
	}
	if (objc_test_class_flag(cls, objc_class_flag_fast_arc))
	{
		objc_release_fast_np(obj);
		return;
	}
	[obj release];
}

static inline void initAutorelease(void)
{
	if (Nil == AutoreleasePool)
	{
		AutoreleasePool = objc_getClass("NSAutoreleasePool");
		if (Nil == AutoreleasePool)
		{
			useARCAutoreleasePool = YES;
		}
		else
		{
			useARCAutoreleasePool = (0 != class_getInstanceMethod(AutoreleasePool,
			                                                      SELECTOR(_ARCCompatibleAutoreleasePool)));
//             printf("useARCAutoreleasePool = %d\n",useARCAutoreleasePool);
			if (!useARCAutoreleasePool)
			{
				[AutoreleasePool class];
				NewAutoreleasePool = class_getMethodImplementation(object_getClass(AutoreleasePool),
				                                                   SELECTOR(new));
				DeleteAutoreleasePool = class_getMethodImplementation(AutoreleasePool,
				                                                      SELECTOR(release));
				AutoreleaseAdd = class_getMethodImplementation(object_getClass(AutoreleasePool),
				                                               SELECTOR(addObject:));
			}
		}
	}
}

extern "C" {
  void objc_autoreleasePoolAdd(struct arc_autorelease_pool *pool, id object)
  {
// 	fprintf(stderr, "autoreleasePoolAdd %p %d\n", pool, object);
  }
}

static inline id autorelease(id obj)
{
// 	fprintf(stderr, "Autoreleasing %p (ARC %d)\n", obj, useARCAutoreleasePool);
	if (useARCAutoreleasePool)
	{
		struct arc_tls *tls = getARCThreadData();
		if (NULL != tls)
		{
			struct arc_autorelease_pool *pool = tls->pool;
			if (NULL == pool || (pool->insert >= &pool->pool[POOL_SIZE]))
			{
				pool = new_zeroed<struct arc_autorelease_pool>();
				pool->previous = tls->pool;
				pool->insert = pool->pool;
				tls->pool = pool;
			}
			*pool->insert = obj;
			pool->insert++;
			return obj;
		}
	}
	if (objc_test_class_flag(classForObject(obj), objc_class_flag_fast_arc))
	{
		initAutorelease();
		if (0 != AutoreleaseAdd)
		{
			AutoreleaseAdd(AutoreleasePool, SELECTOR(addObject:), obj);
		}
		return obj;
	}
	//return [obj autorelease];
	return obj;
}

extern "C" OBJC_PUBLIC unsigned long objc_arc_autorelease_count_np(void)
{
	struct arc_tls* tls = getARCThreadData();
	unsigned long count = 0;
	if (!tls) { return 0; }

	for (struct arc_autorelease_pool *pool=tls->pool ;
	     NULL != pool ;
	     pool = pool->previous)
	{
		count += (((intptr_t)pool->insert) - ((intptr_t)pool->pool)) / sizeof(id);
	}
	return count;
}
extern "C" OBJC_PUBLIC unsigned long objc_arc_autorelease_count_for_object_np(id obj)
{
	struct arc_tls* tls = getARCThreadData();
	unsigned long count = 0;
	if (!tls) { return 0; }

	for (struct arc_autorelease_pool *pool=tls->pool ;
	     NULL != pool ;
	     pool = pool->previous)
	{
		for (id* o = pool->insert-1 ; o >= pool->pool ; o--)
		{
			if (*o == obj)
			{
				count++;
			}
		}
	}
	return count;
}

extern "C" OBJC_PUBLIC void *objc_autoreleasePoolPush(void)
{
	initAutorelease();
	struct arc_tls* tls = getARCThreadData();
	// If there is an object in the return-retained slot, then we need to
	// promote it to the real autorelease pool BEFORE pushing the new
	// autorelease pool.  If we don't, then it may be prematurely autoreleased.
	if ((NULL != tls) && (nil != tls->returnRetained))
	{
		autorelease(tls->returnRetained);
		tls->returnRetained = nil;
	}
	if (useARCAutoreleasePool)
	{
		if (NULL != tls)
		{
			struct arc_autorelease_pool *pool = tls->pool;
			if (NULL == pool || (pool->insert >= &pool->pool[POOL_SIZE]))
			{
				pool = new_zeroed<struct arc_autorelease_pool>();
				pool->previous = tls->pool;
				pool->insert = pool->pool;
				tls->pool = pool;
			}
			// If there is no autorelease pool allocated for this thread, then
			// we lazily allocate one the first time something is autoreleased.
			return (NULL != tls->pool) ? tls->pool->insert : NULL;
		}
	}
	initAutorelease();
	if (0 == NewAutoreleasePool) { return NULL; }
	return NewAutoreleasePool(AutoreleasePool, SELECTOR(new));
}
extern "C" OBJC_PUBLIC void objc_autoreleasePoolPop(void *pool)
{
	if (useARCAutoreleasePool)
	{
		struct arc_tls* tls = getARCThreadData();
		if (NULL != tls)
		{
			if (NULL != tls->pool)
			{
				emptyPool(tls, pool);
			}
			return;
		}
	}
	DeleteAutoreleasePool(static_cast<id>(pool), SELECTOR(release));
	struct arc_tls* tls = getARCThreadData();
	if (tls && tls->returnRetained)
	{
		release(tls->returnRetained);
		tls->returnRetained = nil;
	}
}

extern "C" OBJC_PUBLIC id objc_autorelease(id obj)
{
	if (nil != obj)
	{
		obj = autorelease(obj);
	}
	return obj;
}

extern "C" OBJC_PUBLIC id objc_autoreleaseReturnValue(id obj)
{
	if (!useARCAutoreleasePool) 
	{
		struct arc_tls* tls = getARCThreadData();
		if (NULL != tls)
		{
			objc_autorelease(tls->returnRetained);
			tls->returnRetained = obj;
			return obj;
		}
	}
	return objc_autorelease(obj);
}

extern "C" OBJC_PUBLIC id objc_retainAutoreleasedReturnValue(id obj)
{
	// If the previous object was released  with objc_autoreleaseReturnValue()
	// just before return, then it will not have actually been autoreleased.
	// Instead, it will have been stored in TLS.  We just remove it from TLS
	// and undo the fake autorelease.
	//
	// If the object was not returned with objc_autoreleaseReturnValue() then
	// we actually autorelease the fake object. and then retain the argument.
	// In tis case, this is equivalent to objc_retain().
	struct arc_tls* tls = getARCThreadData();
	if (NULL != tls)
	{
		// If we're using our own autorelease pool, just pop the object from the top
		if (useARCAutoreleasePool)
		{
			if ((NULL != tls->pool) &&
			    (*(tls->pool->insert-1) == obj))
			{
				tls->pool->insert--;
				return obj;
			}
		}
		else if (obj == tls->returnRetained)
		{
			tls->returnRetained = NULL;
			return obj;
		}
	}
	return objc_retain(obj);
}

extern "C" OBJC_PUBLIC id objc_retain(id obj)
{
	if (nil == obj) { return nil; }
	return retain(obj);
}

extern "C" OBJC_PUBLIC id objc_retainAutorelease(id obj)
{
	return objc_autorelease(objc_retain(obj));
}

extern "C" OBJC_PUBLIC id objc_retainAutoreleaseReturnValue(id obj)
{
	if (nil == obj) { return obj; }
	return objc_autoreleaseReturnValue(retain(obj));
}


extern "C" OBJC_PUBLIC id objc_retainBlock(id b)
{
	return static_cast<id>(_Block_copy(b));
}

extern "C" OBJC_PUBLIC void objc_release(id obj)
{
	if (nil == obj) { return; }
	release(obj);
}

extern "C" OBJC_PUBLIC id objc_storeStrong(id *addr, id value)
{
	value = objc_retain(value);
	id oldValue = *addr;
	*addr = value;
	objc_release(oldValue);
	return value;
}

////////////////////////////////////////////////////////////////////////////////
// Weak references
////////////////////////////////////////////////////////////////////////////////

static int weakref_class;

namespace {

struct WeakRef
{
	void *isa = &weakref_class;
	id obj = nullptr;
	size_t weak_count = 1;
	WeakRef(id o) : obj(o) {}
};

template<typename T>
struct malloc_allocator
{
	typedef T value_type;
	T* allocate(std::size_t n)
	{
		return static_cast<T*>(malloc(sizeof(T) * n));
	}

	void deallocate(T* p, std::size_t)
	{
		free(p);
	}

	template<typename X>
	malloc_allocator &operator=(const malloc_allocator<X>&) const
	{
		return *this;
	}

	bool operator==(const malloc_allocator &) const
	{
		return true;
	}

	template<typename X>
	operator malloc_allocator<X>() const
	{
		return malloc_allocator<X>();
	}
};

using weak_ref_table = tsl::robin_pg_map<const void*,
                                         WeakRef*,
                                         std::hash<const void*>,
                                         std::equal_to<const void*>,
                                         malloc_allocator<std::pair<const void*, WeakRef*>>>;

weak_ref_table &weakRefs()
{
	static weak_ref_table w{128};
	return w;
}

mutex_t weakRefLock;

}

PRIVATE extern "C" void init_arc(void)
{
	INIT_LOCK(weakRefLock);
#ifdef arc_tls_store
	ARCThreadKey = arc_tls_key_create((arc_cleanup_function_t)cleanupPools);
#endif
}

/**
  * Load from a weak pointer and return whether this really was a weak
  * reference or a strong (not deallocatable) object in a weak pointer.  The
  * object will be stored in `obj` and the weak reference in `ref`, if one
  * exists.
  */
__attribute__((always_inline))
static BOOL loadWeakPointer(id *addr, id *obj, WeakRef **ref)
{
	id oldObj = *addr;
	if (oldObj == nil)
	{
		*ref = NULL;
		*obj = nil;
		return NO;
	}
	if (classForObject(oldObj) == (Class)&weakref_class)
	{
		*ref = (WeakRef*)oldObj;
		*obj = (*ref)->obj;
		return YES;
	}
	*ref = NULL;
	*obj = oldObj;
	return NO;
}

__attribute__((always_inline))
static inline BOOL weakRefRelease(WeakRef *ref)
{
	ref->weak_count--;
	if (ref->weak_count == 0)
	{
		weakRefs().erase(ref->obj);
		delete ref;
		return YES;
	}
	return NO;
}

extern "C" void* block_load_weak(void *block);

static BOOL setObjectHasWeakRefs(id obj)
{
	BOOL isGlobalObject = isPersistentObject(obj);
	Class cls = isGlobalObject ? Nil : obj->isa;
	if (obj && cls && objc_test_class_flag(cls, objc_class_flag_fast_arc))
	{
		uintptr_t *refCount = ((uintptr_t*)obj) - 1;
		if (obj)
		{
			uintptr_t refCountVal = __sync_fetch_and_add(refCount, 0);
			uintptr_t newVal = refCountVal;
			do {
				refCountVal = newVal;
				size_t realCount = refCountVal & refcount_mask;
				// If this object has already been deallocated (or is in the
				// process of being deallocated) then don't bother storing it.
				if (realCount < 0)
				{
					obj = nil;
					cls = Nil;
					break;
				}
				// The weak ref flag is monotonic (it is set, never cleared) so
				// don't bother trying to re-set it.
				if ((refCountVal & weak_mask) == weak_mask)
				{
					break;
				}
				// Set the flag in the reference count to indicate that a weak
				// reference has been taken.
				//
				// We currently hold the weak ref lock, so another thread
				// racing to deallocate this object will have to wait to do so
				// if we manage to do the reference count update first.  This
				// shouldn't be possible, because `obj` should be a strong
				// reference and so it shouldn't be possible to deallocate it
				// while we're assigning it.
				uintptr_t updated = ((uintptr_t)realCount | weak_mask);
				newVal = __sync_val_compare_and_swap(refCount, refCountVal, updated);
			} while (newVal != refCountVal);
		}
	}
	return isGlobalObject;
}

WeakRef *incrementWeakRefCount(id obj)
{
	WeakRef *&ref = weakRefs()[obj];
	if (ref == nullptr)
	{
		ref = new WeakRef(obj);
	}
	else
	{
		assert(ref->obj == obj);
		ref->weak_count++;
	}
	return ref;
}

extern "C" OBJC_PUBLIC id objc_storeWeak(id *addr, id obj)
{
	LOCK_FOR_SCOPE(&weakRefLock);
	WeakRef *oldRef;
	id old;
	loadWeakPointer(addr, &old, &oldRef);
	// If the old and new values are the same, then we don't need to do anything.
	if (old == obj)
	{
		return obj;
	}
	BOOL isGlobalObject = setObjectHasWeakRefs(obj);
	// If we old ref exists, decrement its reference count.  This may also
	// delete the weak reference control block.
	if (oldRef != NULL)
	{
		weakRefRelease(oldRef);
	}
	// If we're storing nil, then just write a null pointer.
	if (nil == obj)
	{
		*addr = obj;
		return nil;
	}
	if (isGlobalObject)
	{
		// If this is a global object, it's never deallocated, so secretly make
		// this a strong reference.
		*addr = obj;
		return obj;
	}
	if (nil != obj)
	{
		*addr = (id)incrementWeakRefCount(obj);
	}
	return obj;
}

extern "C" OBJC_PUBLIC BOOL objc_delete_weak_refs(id obj)
{
	LOCK_FOR_SCOPE(&weakRefLock);
	if (objc_test_class_flag(classForObject(obj), objc_class_flag_fast_arc))
	{
		// If another thread has done a load of a weak reference, then it will
		// have incremented the reference count with the lock held.  It may
		// have done so in between this thread's decrementing the reference
		// count and its acquiring the lock.  In this case, report failure.
		uintptr_t *refCount = ((uintptr_t*)obj) - 1;
		// Reconstruct the sign bit.  We don't need to do this on any other 
		// operations, because even on increment the overflow will be correct
		// after truncation.
		uintptr_t refCountVal = (__sync_fetch_and_add(refCount, 0) & refcount_mask) << refcount_shift;
		if ((((intptr_t)refCountVal) >> refcount_shift) >= 0)
		{
			return NO;
		}
	}
	auto &table = weakRefs();
	auto old = table.find(obj);
	if (old != table.end())
	{
		WeakRef *oldRef = old->second;
		// The address of obj is likely to be reused, so remove it from
		// the table so that we don't accidentally alias weak
		// references
		table.erase(old);
		// Zero the object pointer.  This prevents any other weak
		// accesses from loading from this.  This must be done after
		// removing the ref from the table, because the compare operation
		// tests the obj field.
		oldRef->obj = nil;
		// If the weak reference count is zero, then we should have
		// already removed this.
		assert(oldRef->weak_count > 0);
	}
	return YES;
}

extern "C" OBJC_PUBLIC id objc_loadWeakRetained(id* addr)
{
	LOCK_FOR_SCOPE(&weakRefLock);
	id obj;
	WeakRef *ref;
	// If this is really a strong reference (nil, or an non-deallocatable
	// object), just return it.
	if (!loadWeakPointer(addr, &obj, &ref))
	{
		return obj;
	}
	// The object cannot be deallocated while we hold the lock (release
	// will acquire the lock before attempting to deallocate)
	if (obj == nil)
	{
		// If the object is destroyed, drop this reference to the WeakRef
		// struct.
		if (ref != NULL)
		{
			weakRefRelease(ref);
			*addr = nil;
		}
		return nil;
	}
	Class cls = classForObject(obj);
	if (static_cast<Class>(&_NSConcreteMallocBlock) == cls)
	{
		obj = static_cast<id>(block_load_weak(obj));
	}
	else if (objc_test_class_flag(cls, objc_class_flag_permanent_instances))
	{
		return obj;
	}
	else if (!objc_test_class_flag(cls, objc_class_flag_fast_arc))
	{
		obj = _objc_weak_load(obj);
	}
	return objc_retain(obj);
}

extern "C" OBJC_PUBLIC id objc_loadWeak(id* object)
{
	return objc_autorelease(objc_loadWeakRetained(object));
}

extern "C" OBJC_PUBLIC void objc_copyWeak(id *dest, id *src)
{
	// Don't retain or release.
	// `src` is a valid pointer to a __weak pointer or nil.
	// `dest` is a valid pointer to uninitialised memory.
	// After this operation, `dest` should contain whatever `src` contained.
	LOCK_FOR_SCOPE(&weakRefLock);
	id obj;
	WeakRef *srcRef;
	loadWeakPointer(src, &obj, &srcRef);
	*dest = *src;
	if (srcRef)
	{
		srcRef->weak_count++;
	}
}

extern "C" OBJC_PUBLIC void objc_moveWeak(id *dest, id *src)
{
	// Don't retain or release.
	// `dest` is a valid pointer to uninitialized memory.
	// `src` is a valid pointer to a __weak pointer.
	// This operation moves from *src to *dest and must be atomic with respect
	// to other stores to *src via `objc_storeWeak`.
	//
	// Acquire the lock so that we guarantee the atomicity.  We could probably
	// optimise this by doing an atomic exchange of `*src` with `nil` and
	// storing the result in `dest`, but it's probably not worth it unless weak
	// references are a bottleneck.
	LOCK_FOR_SCOPE(&weakRefLock);
	*dest = *src;
	*src = nil;
}

extern "C" OBJC_PUBLIC void objc_destroyWeak(id* obj)
{
	LOCK_FOR_SCOPE(&weakRefLock);
	WeakRef *oldRef;
	id old;
	loadWeakPointer(obj, &old, &oldRef);
	// If the old ref exists, decrement its reference count.  This may also
	// delete the weak reference control block.
	if (oldRef != NULL)
	{
		weakRefRelease(oldRef);
	}
}

extern "C" OBJC_PUBLIC id objc_initWeak(id *addr, id obj)
{
	if (obj == nil)
	{
		*addr = nil;
		return nil;
	}
	LOCK_FOR_SCOPE(&weakRefLock);
	BOOL isGlobalObject = setObjectHasWeakRefs(obj);
	if (isGlobalObject)
	{
		// If this is a global object, it's never deallocated, so secretly make
		// this a strong reference.
		*addr = obj;
		return obj;
	}
	if (nil != obj)
	{
		*(WeakRef**)addr = incrementWeakRefCount(obj);
	}
	return obj;
}
