#if defined(__clang__) && !defined(__OBJC_RUNTIME_INTERNAL__)
#pragma clang system_header
#endif

/**
 * objc-auto.h - This file provides the interface for Objective-C garbage
 * collection
 */

/**
 * Flags passed to objc_collect.  The low 2 bits specify the type of collection
 * to perform, the remainder provide additional options.
 */
enum
{
	/**
	 * Perform an incremental collection if the collection ratio has not been
	 * exceeded, or a full collection if it has.
	 */
	OBJC_RATIO_COLLECTION        = 0,
	/**
	 * Performs an incremental collection. 
	 */
	OBJC_GENERATIONAL_COLLECTION = 1,
	/**
	 * Performs a full collection.
	 */
	OBJC_FULL_COLLECTION         = 2,
	/**
	 * Repeatedly performs a full collection until collection does not find any
	 * new free memory.
	 */
	OBJC_EXHAUSTIVE_COLLECTION   = 3,
	/**
	 * Only runs the collector (in any mode) if the number of bytes allocated
	 * since the last collection is greater than the threshold.
	 */
	OBJC_COLLECT_IF_NEEDED       = (1 << 3),
	/**
	 * Does not return until the collector has finished running.  
	 */
	OBJC_WAIT_UNTIL_DONE         = (1 << 4),
};

/**
 * Options for objc_clear_stack().
 */
enum
{
	/** Ignored - provided for OS X compatibility. */
	OBJC_CLEAR_RESIDENT_STACK = 1
};


/**
 * Instructs the garbage collector to run.
 */
void objc_collect(unsigned long options);

/**
 * Returns yes if the connector is currently running, i.e. if every call to
 * objc_gc_disable() has been balanced with a corresponding call to
 * objc_gc_enable().
 */
BOOL objc_collectingEnabled(void);

/**
 * Returns YES if running in GC mode, NO otherwise.
 */
BOOL objc_collecting_enabled(void);

/**
 * Starts concurrent collection.  If this has been called, then finalizers will
 * run in a separate thread.
 */
void objc_startCollectorThread(void);

/**
 * Causes all finalizers for instances of the specified class to be run on the
 * main thread.  This is currently unimplemented.
 */
void objc_finalizeOnMainThread(Class cls);

/**
 * Attempts to delete pointers currently stored on unused bits of the stack.  
 */
void objc_clear_stack(unsigned long options);

/**
 * Returns YES if an object has been finalized.  
 */
BOOL objc_is_finalized(void *ptr);

/**
 * Performs an atomic compare and exchange on a pointer value.  Sets the value
 * at objectLocation to replacement, if the current value is predicate.
 */
BOOL objc_atomicCompareAndSwapPtr(id predicate,
                                  id replacement,
                                  volatile id *objectLocation);
/**
 * Performs an atomic compare and exchange on a pointer value.  Sets the value
 * at objectLocation to replacement, if the current value is predicate.
 */
BOOL objc_atomicCompareAndSwapPtrBarrier(id predicate,
                                         id replacement,
                                         volatile id *objectLocation);

/**
 * Performs an atomic compare and exchange on a pointer value.  Sets the value
 * at objectLocation to replacement, if the current value is predicate.
 */
BOOL objc_atomicCompareAndSwapGlobal(id predicate,
                                     id replacement,
                                     volatile id *objectLocation);
/**
 * Performs an atomic compare and exchange on a pointer value.  Sets the value
 * at objectLocation to replacement, if the current value is predicate.
 */
BOOL objc_atomicCompareAndSwapGlobalBarrier(id predicate,
                                            id replacement,
                                            volatile id *objectLocation);
/**
 * Performs an atomic compare and exchange on a pointer value.  Sets the value
 * at objectLocation to replacement, if the current value is predicate.
 */
BOOL objc_atomicCompareAndSwapInstanceVariable(id predicate,
                                               id replacement,
                                               volatile id *objectLocation);
/**
 * Performs an atomic compare and exchange on a pointer value.  Sets the value
 * at objectLocation to replacement, if the current value is predicate.
 */
BOOL objc_atomicCompareAndSwapInstanceVariableBarrier(id predicate,
                                                      id replacement,
                                                      volatile id *objectLocation);

////////////////////////////////////////////////////////////////////////////////
// The next group of functions are intended to be called automatically by the
// compiler.  Normal user code will not call them.
////////////////////////////////////////////////////////////////////////////////

/**
 * Performs a strong assignment.  Stores val in *ptr, ensuring that the
 * assignment is visible to the collector.
 */
id objc_assign_strongCast(id val, id *ptr);

/**
 * Assigns val to the global pointed to by ptr, ensuring that the assignment is
 * visible to the collector.
 */
id objc_assign_global(id val, id *ptr);
/**
 * Assigns val to the instance variable offset bytes from dest.
 */
id objc_assign_ivar(id val, id dest, ptrdiff_t offset);
/**
 * Performs a memmove() operation, ensuring that the copied bytes are always
 * visible to the collector.
 */
void *objc_memmove_collectable(void *dst, const void *src, size_t size);
/**
 * Reads a weak pointer value.  All reads of pointers declared __weak MUST be
 * via this call.
 */
id objc_read_weak(id *location);
/**
 * Assigns a value to location, which MUST have been declared __weak.  All
 * assignments to weak pointers must go via this function.
 */
id objc_assign_weak(id value, id *location);


/**
 * Registers the current thread with the garbage collector.  Should be done as
 * soon as a thread is created.  Until this is called, the thread's stack will
 * be invisible to the collector.
 */
void objc_registerThreadWithCollector(void);
/**
 * Unregisters the current thread.  The thread's stack becomes invisible to the
 * collector.  This should be called just before the thread exits.
 */
void objc_unregisterThreadWithCollector(void);
/**
 * Registers the current thread with the garbage collector and aborts if the
 * registration failed.  The thread is expected to have already been
 * registered.  This will print a warning message if it has not been.
 */
void objc_assertRegisteredThreadWithCollector();

////////////////////////////////////////////////////////////////////////////////
// Functions below this line are extensions to the OS X GC API, intended to
// allow implementation of the higher-level public APIs
////////////////////////////////////////////////////////////////////////////////

/**
 * Disables the garbage collector until it is reenabled with objc_gc_enable.
 */
void objc_gc_disable(void);
/**
 * Enables the garbage collector, if it has been previously disabled with a
 * call to objc_gc_disable().  These calls store an internal count.  If
 * objc_gc_disable() is called twice, then collection will not resume until
 * objc_gc_enable() has also been called twice.
 */
void objc_gc_enable(void);

/**
 * Increments the reference count of objects.  This is intended to be used to
 * implement CFRetain().  Reference counts should only be used when storing
 * pointers to objects in untracked allocations (e.g. malloc() memory).
 *
 * This function is intended to be used to implement CFRetain().
 */
id objc_gc_retain(id object);
/**
 * Decrements the reference count on an object.  An object becomes eligible for
 * automatic collection when its reference count reaches zero.  New objects
 * have a reference count of zero, so they are eligible for collection as soon
 * as the last pointer to them vanishes.
 *
 * This function is intended to be used to implement CFRelease().
 */
void objc_gc_release(id object);
/**
 * Returns the retain count of an object.  This is 0 for objects that have not
 * had objc_gc_retain() called on them, which should be most objects in a
 * garbage-collected program.
 */
int objc_gc_retain_count(id object);
/**
 * Allocates a buffer of memory, which will be automatically deallocated by the
 * collector.  If isScanned is true, then this memory may contain pointers.  If
 * not, then pointers stored in the returned region will be ignored.
 *
 * This function is intended to be used to implement NSAllocateCollectable().
 */
void* objc_gc_allocate_collectable(size_t size, BOOL isScanned);

/**
 * Reallocates a block of collectable memory.
 */
void* objc_gc_reallocate_collectable(void *ptr, size_t size, BOOL isScanned);

/**
 * If the pointer lies in a collectible memory region, returns the address at
 * the start of this region.  Otherwise, returns NULL.
 */
void* objc_gc_collectable_address(void* ptr);

/**
 * Registers a class that should be copied on assignment to heap locations.
 * For performance reasons, the runtime only permits a small number of classes
 * to be registered.  These will always be copied when they are assigned, using
 * the function specified in the second argument.
 */
BOOL objc_register_stack_class(Class cls, IMP copyFunction);
