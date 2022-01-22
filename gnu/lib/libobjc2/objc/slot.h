#if defined(__clang__) && !defined(__OBJC_RUNTIME_INTERNAL__)
#pragma clang system_header
#endif

#ifndef __OBJC_SLOT_H_INCLUDED__
#define __OBJC_SLOT_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The objc_slot structure is used to permit safe IMP caching.  It is returned
 * by the new lookup APIs.  When you call `objc_slot_lookup_version`, the final
 * parameter is used to return either the current value of
 * `objc_method_cache_version` or 0 if the slot is uncacheable.  You can then
 * store this value along with a pointer to the `objc_slot2`.  If the returned
 * value is equal to the current value of `objc_method_cache_version` then it
 * is safe to call the method from the `method` field of the slot directly.
 */
struct objc_slot2
{
	IMP method;
} OBJC_NONPORTABLE;

/**
 * A counter that is incremented whenever one or more cached slots become
 * invalid, for example if a subclass loads a category containing methods that
 * were inherited from the superclass.
 */
OBJC_PUBLIC extern _Atomic(uint64_t) objc_method_cache_version;

/**
 * Legacy cache structure.  This is no longer maintained in the runtime and is
 * now exported only in the compatibility interfaces.  Slots of this form are
 * never cacheable.
 */
struct objc_slot
{
	/** The class to which this slot is attached (used internally).  */
	Class owner;
	/** The class for which this slot was cached.  Note that this can be
	 * modified by different cache owners, in different threads.  Doing so may
	 * cause some cache misses, but if different methods are sending messages
	 * to the same object and sharing a cached slot then it may also improve
	 * cache hits.  Profiling is probably required here. */
	Class cachedFor;
	/** The (typed) selector for the method identified by this slot. */
	const char *types;
	/** The current version.  This changes if the method changes or if a
	 * subclass overrides this method, potentially invalidating this cache. */
	int version;
	/** The method pointer for this method. */
	IMP method;
	/** Selector for this method. */
	SEL selector;
} OBJC_NONPORTABLE;
#ifdef __cplusplus
}
#endif
#endif // __OBJC_SLOT_H_INCLUDED__
