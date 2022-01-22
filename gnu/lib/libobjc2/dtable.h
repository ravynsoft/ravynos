#include "lock.h"
#include "class.h"
#include "sarray2.h"
#include "objc/slot.h"
#include "visibility.h"
#include <stdint.h>
#include <stdio.h>

#ifdef __OBJC_LOW_MEMORY__
typedef struct objc_dtable* dtable_t;
struct objc_slot* objc_dtable_lookup(dtable_t dtable, uint32_t uid);
#else
typedef SparseArray* dtable_t;
#	define objc_dtable_lookup SparseArrayLookup
#endif

/**
 * Pointer to the sparse array representing the pretend (uninstalled) dtable.
 */
PRIVATE extern dtable_t uninstalled_dtable;
/**
 * Structure for maintaining a linked list of temporary dtables.  When sending
 * an +initialize message to a class, we create a temporary dtables and store
 * it in a linked list.  This is then used when sending other messages to
 * instances of classes in the middle of initialisation.
 */
typedef struct _InitializingDtable
{
	/** The class that owns the dtable. */
	Class class;
	/** The dtable for this class. */
	dtable_t dtable;
	/** The next uninstalled dtable in the list. */
	struct _InitializingDtable *next;
} InitializingDtable;

/** Head of the list of temporary dtables.  Protected by initialize_lock. */
extern InitializingDtable *temporary_dtables;
extern mutex_t initialize_lock;

/**
 * Returns whether a class has an installed dtable.
 */
static inline int classHasInstalledDtable(struct objc_class *cls)
{
	return (cls->dtable != uninstalled_dtable);
}

OBJC_PUBLIC
int objc_sync_enter(id object);
OBJC_PUBLIC
int objc_sync_exit(id object);
/**
 * Returns the dtable for a given class.  If we are currently in an +initialize
 * method then this will block if called from a thread other than the one
 * running the +initialize method.  
 */
static inline dtable_t dtable_for_class(Class cls)
{
	if (classHasInstalledDtable(cls))
	{
		return cls->dtable;
	}

	dtable_t dtable = uninstalled_dtable;

	{
		LOCK_FOR_SCOPE(&initialize_lock);
		if (classHasInstalledDtable(cls))
		{
			return cls->dtable;
		}
		/* This is a linear search, and so, in theory, could be very slow.  It
		 * is O(n) where n is the number of +initialize methods on the stack.
		 * In practice, this is a very small number.  Profiling with GNUstep
		 * showed that this peaks at 8. */
		InitializingDtable *buffer = temporary_dtables;
		while (NULL != buffer)
		{
			if (buffer->class == cls)
			{
				dtable = buffer->dtable;
				break;
			}
			buffer = buffer->next;
		}
	}

	if (dtable != uninstalled_dtable)
	{
		// Make sure that we block if +initialize is still running.  We do this
		// after we've released the initialize lock, so that the real dtable
		// can be installed.  This acquires / releases a recursive mutex, so if
		// this mutex is already held by this thread then this will proceed
		// immediately.  If it's held by another thread (i.e. the one running
		// +initialize) then we block here until it's run.  We don't need to do
		// this if the dtable is the uninstalled dtable, because that means
		// +initialize has not yet been sent, so we can wait until something
		// triggers it before needing any synchronisation.
		objc_sync_enter((id)cls);
		objc_sync_exit((id)cls);
	}
	return dtable;
}

/**
 * Returns whether a class has had a dtable created.  The dtable may be
 * installed, or stored in the look-aside buffer.
 */
static inline int classHasDtable(struct objc_class *cls)
{
	return (dtable_for_class(cls) != uninstalled_dtable);
}

/**
 * Updates the dtable for a class and its subclasses.  Must be called after
 * modifying a class's method list.
 */
void objc_update_dtable_for_class(Class);
/**
 * Updates the dtable for a class and its subclasses.  Must be called after
 * changing and initializing a class's superclass.
 */
void objc_update_dtable_for_new_superclass(Class, Class);
/**
 * Adds a single method list to a class.  This is used when loading categories,
 * and is faster than completely rebuilding the dtable.
 */
void add_method_list_to_class(Class cls,
                              struct objc_method_list *list);

/**
 * Destroys a dtable.
 */
void free_dtable(dtable_t dtable);

/**
 * Checks whether the class supports ARC.  This can be used before the dtable
 * is installed.
 */
void checkARCAccessorsSlow(Class cls);
