
/**
 * Garbage collection operations.
 */
struct gc_ops
{
	/**
	 * Initialises this collector.
	 */
	void (*init)(void);
	/**
	 * Allocates enough space for a class, followed by some extra bytes.
	 */
	id (*allocate_class)(Class, size_t);
	/**
	 * Frees an object.
	 */
	void (*free_object)(id);
	/**
	 * Allocates some memory that can be used to store pointers.  This must be
	 * used instead of malloc() for internal data structures that will store
	 * pointers passed in from outside.  The function is expected to zero the
	 * memory that it returns.
	 */
	void* (*malloc)(size_t);
	/**
	 * Frees some memory that was previously used to store pointers.
	 */
	void (*free)(void*);
};

/**
 * Enables garbage collection, if it isn't already enabled.
 *
 * If the exclusive flag is set, then this will ensure that all -retain /
 * -release / -autorelease messages become no-ops.
 */
void enableGC(BOOL exclusive);

/**
 * The mode for garbage collection
 */
enum objc_gc_mode
{
	/** This module neither uses, nor supports, garbage collection. */
	GC_None     = 0,
	/**
	 * This module uses garbage collection, but also sends retain / release
	 * messages.  It can be used with or without GC.
	 */
	GC_Optional = 1,
	/**
	 * This module expects garbage collection and will break without it.
	 */
	GC_Required = 2,
	/**
	 * This module was compiled with automatic reference counting.  This
	 * guarantees the use of the non-fragile ABI and means that we could
	 * potentially support GC, although we don't currently.
	 */
	GC_ARC = 3
};

/**
 * The current Objective-C garbage collection mode.
 */
extern enum objc_gc_mode current_gc_mode;
/**
 * Have we loaded any code that triggers the ObjC GC support?
 */
extern BOOL isGCEnabled;

/**
 * The current set of garbage collector operations to use.
 */
extern struct gc_ops *gc;

extern struct gc_ops gc_ops_boehm;
extern struct gc_ops gc_ops_none;
