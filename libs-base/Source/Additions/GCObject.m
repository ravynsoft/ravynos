/* Implementation of garbage collecting classe framework

   Copyright (C) 2002 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Inspired by gc classes of  Ovidiu Predescu and Mircea Oancea

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

   AutogsdocSource: Additions/GCObject.m
   AutogsdocSource: Additions/GCArray.m
   AutogsdocSource: Additions/GCDictionary.m

*/

#import "common.h"
#ifndef NeXT_Foundation_LIBRARY
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSThread.h"
#endif

#import "GNUstepBase/GCObject.h"

#include <pthread.h>

/*
 * The head of a linked list of all garbage collecting objects  is a
 * special object which is never deallocated.
 */
@interface _GCObjectList : GCObject
@end
@implementation _GCObjectList
- (void) dealloc
{
  GSNOSUPERDEALLOC;
}
@end


/**
 * <p>The GCObject class is both the base class for all garbage collected
 * objects, and an infrastructure for handling garbage collection.
 * </p>
 * <p>It maintains a list of all garbage collectable objects and provides
 * a method to run a garbage collection pass on those objects.
 * </p>
 */
@implementation GCObject

static GCObject	*allObjects = nil;
static BOOL	isCollecting = NO;

#ifdef NeXT_RUNTIME
static void *allocationLock = NULL;
#define pthread_mutex_lock(lock)
#define pthread_mutex_unlock(lock)
#else
static pthread_mutex_t *allocationLock = NULL;
#endif

+ (void) _becomeMultiThreaded: (NSNotification *)aNotification
{
  if (allocationLock == NULL)
    {
#     ifndef NeXT_RUNTIME
      allocationLock = malloc(sizeof(pthread_mutex_t));
	  if (allocationLock == NULL)
        {
	      abort();
		}
	  
	  pthread_mutex_init(allocationLock, NULL);
#     endif
    }
}

/**
 * Allocates an instance of the class and links it into the list of
 * all garbage collectable objects.  Returns the new instance.<br />
 */
+ (id) allocWithZone: (NSZone*)zone
{
  GCObject	*o = [super allocWithZone: zone];

  if (allocationLock != 0)
    {
      pthread_mutex_lock(allocationLock);
    }
  o->gc.next = allObjects;
  o->gc.previous = allObjects->gc.previous;
  allObjects->gc.previous->gc.next = o;
  allObjects->gc.previous = o;
  o->gc.flags.refCount = 1;
  if (allocationLock != 0)
    {
      pthread_mutex_unlock(allocationLock);
    }

  return o;
}

/**
 * <p>This method runs a garbage collection, causing unreferenced objects to
 * be deallocated.  This is done using a simple three pass algorithm -
 * </p>
 * <deflist>
 *   <term>Pass 1</term>
 *   <desc>
 *     All the garbage collectable objects are sent a
 *     -gcDecrementRefCountOfContainedObjects message.
 *   </desc>
 *   <term>Pass 2</term>
 *   <desc>
 *      All objects having a refCount greater than 0 are sent an
 *     -gcIncrementRefCountOfContainedObjects message.
 *   </desc>
 *   <term>Pass 3</term>
 *   <desc>
 *      All the objects that still have the refCount of 0
 * 	are part of cyclic graphs and none of the objects from this graph
 * 	are held by some object outside graph. These objects receive the
 * 	-dealloc message. In this method they should send the -dealloc message
 * 	to any garbage collectable (GCObject and subclass) instances they
 *      contain.
 *   </desc>
 * </deflist>
 * <p>During garbage collection, the +gcIsCollecting method returns YES.
 * </p>
 */
+ (void) gcCollectGarbage
{
  GCObject	*object;
  GCObject	*last;

  if (allocationLock != 0)
    {
      pthread_mutex_lock(allocationLock);
    }
  if (isCollecting == YES)
    {
      if (allocationLock != 0)
	{
	  pthread_mutex_unlock(allocationLock);
	}
      return;	// Don't allow recursion.
    }
  isCollecting = YES;

  // Pass 1
  object = allObjects->gc.next;
  while (object != allObjects)
    {
      [object gcDecrementRefCountOfContainedObjects];
      // object->gc.flags.visited = 0;
      // object = object->gc.next;
      [object gcSetVisited: NO];
      object = [object gcNextObject];
    }

  // Pass 2
  object = allObjects->gc.next;
  while (object != allObjects)
    {
      if ([object retainCount] > 0)
	{
	  [object gcIncrementRefCountOfContainedObjects];
	}
      // object = object->gc.next;
      object = [object gcNextObject];
    }

  last = allObjects;
  object = last->gc.next;
  while (object != allObjects)
    {
      if ([object retainCount] == 0)
	{
	  GCObject	*next;

	  // next = object->gc.next;
	  // next->gc.previous = last;
	  // last->gc.next = next;
	  // object->gc.next = object;
	  // object->gc.previous = object;
	  next = [object gcNextObject];
	  [next gcSetPreviousObject: last];
	  [last gcSetNextObject: next];
	  [object gcSetNextObject: object];
	  [object gcSetPreviousObject: object];
	  [object dealloc];
	  object = next;
	}
      else
	{
	  last = object;
	  // object = object->gc.next;
	  object = [object gcNextObject];
	}
    }
  isCollecting = NO;
  if (allocationLock != 0)
    {
      pthread_mutex_unlock(allocationLock);
    }
}

+ (void) initialize
{
  if (self == [GCObject class])
    {
      allObjects = (_GCObjectList*)
	NSAllocateObject([_GCObjectList class], 0, NSDefaultMallocZone());
      [[NSObject leakAt: &allObjects] release];
      allObjects->gc.next = allObjects;
      allObjects->gc.previous = allObjects;
      if ([NSThread isMultiThreaded] == YES)
	{
	  [self _becomeMultiThreaded: nil];
	}
      else
	{
	  [[NSNotificationCenter defaultCenter]
	    addObserver: self
	       selector: @selector(_becomeMultiThreaded:)
		   name: NSWillBecomeMultiThreadedNotification
		 object: nil];
	}
    }
}

/**
 * Returns a flag to indicate whether a garbage collection is in progress.
 */
+ (BOOL) gcIsCollecting
{
  return isCollecting;
}

/**
 * Called to remove anObject from the list of garbage collectable objects.<br />
 * This method is provided so that classes which are not subclasses of
 * GCObject (but which have the same initial instance variable layout) can
 * use multiple inheritance (behaviors) to act as GCObject instances, but
 * can have their own -dealloc methods.<br />
 * These classes should call this in their own -dealloc methods.
 */
+ (void) gcObjectWillBeDeallocated: (GCObject*)anObject
{
  GCObject	*p;
  GCObject	*n;

  if (allocationLock != 0)
    {
      pthread_mutex_lock(allocationLock);
    }
  // p = anObject->gc.previous;
  // n = anObject->gc.next;
  // p->gc.next = n;
  // n->gc.previous = p;
  p = [anObject gcPreviousObject];
  n = [anObject gcNextObject];
  [p gcSetNextObject: n];
  [n gcSetPreviousObject: p];
  if (allocationLock != 0)
    {
      pthread_mutex_unlock(allocationLock);
    }
}

/**
 * Copies the receiver (using the NSCopyObject() function) and links
 * the resulting object into the list of all garbage collactable
 * objects.  Returns the newly created object.
 */
- (id) copyWithZone: (NSZone*)zone
{
  GCObject	*o = (GCObject*)NSCopyObject(self, 0, zone);

  if (allocationLock != 0)
    {
      pthread_mutex_lock(allocationLock);
    }
  o->gc.next = allObjects;
  o->gc.previous = allObjects->gc.previous;
  allObjects->gc.previous->gc.next = o;
  allObjects->gc.previous = o;
  o->gc.flags.refCount = 1;
  if (allocationLock != 0)
    {
      pthread_mutex_unlock(allocationLock);
    }
  return o;
}

/**
 * Removes the receiver from the list of garbage collectable objects and
 * then calls the superclass implementation to complete deallocation of
 * th receiver and freeing of the memory it uses.<br />
 * Subclasses should call this at the end of their -dealloc methods as usual.
 */
- (void) dealloc
{
  GCObject	*p;
  GCObject	*n;

  if (allocationLock != 0)
    {
      pthread_mutex_lock(allocationLock);
    }
  // p = anObject->gc.previous;
  // n = anObject->gc.next;
  // p->gc.next = n;
  // n->gc.previous = p;
  p = [self gcPreviousObject];
  n = [self gcNextObject];
  [p gcSetNextObject: n];
  [n gcSetPreviousObject: p];
  if (allocationLock != 0)
    {
      pthread_mutex_unlock(allocationLock);
    }
  [super dealloc];
}

/**
 * Decrements the garbage collection reference count for the receiver.<br />
 */
- (void) gcDecrementRefCount
{
  /*
   * No locking needed since this is only called when garbage collecting
   * and the collection method handles locking.
   */
  gc.flags.refCount--;
}

/**
 * <p>Marks the receiver as not having been visited in the current garbage
 * collection process (first pass of collection).
 * </p>
 * <p>All container subclasses should override this method to call the super
 * implementation then decrement the ref counts of their contents as well as
 * sending the -gcDecrementRefCountOfContainedObjects
 * message to each of them.
 * </p>
 */
- (void) gcDecrementRefCountOfContainedObjects
{
  /*
   * No locking needed since this is only called when garbage collecting
   * and the collection method handles locking.
   */
  gc.flags.visited = 0;
}

/**
 * Increments the garbage collection reference count for the receiver.<br />
 */
- (void) gcIncrementRefCount
{
  /*
   * No locking needed since this is only called when garbage collecting
   * and the collection method handles locking.
   */
  gc.flags.refCount++;
}

/**
 * <p>Checks to see if the receiver has already been visited in the
 * current garbage collection process, and either marks the receiver as
 * visited (and returns YES) or returns NO to indicate that it had already
 * been visited.
 * </p>
 * <p>All container subclasses should override this method to call the super
 * implementation then, if the method returns YES, increment the reference
 * count of any contained objects and send the
 * -gcIncrementRefCountOfContainedObjects
 * to each of the contained objects too.
 * </p>
 */
- (BOOL) gcIncrementRefCountOfContainedObjects
{
  /*
   * No locking needed since this is only called when garbage collecting
   * and the collection method handles locking.
   */
  if (gc.flags.visited == 1)
    {
      return NO;
    }
  gc.flags.visited = 1;
  return YES;
}

/**
 * Decrements the receivers reference count, and if zero, rmoveis it
 * from the list of garbage collectable objects and deallocates it.
 */
- (oneway void) release
{
  if (allocationLock != 0)
    {
      pthread_mutex_lock(allocationLock);
    }
  if (gc.flags.refCount > 0 && gc.flags.refCount-- == 1)
    {
      [GCObject gcObjectWillBeDeallocated: self];
      [self dealloc];
    }
  if (allocationLock != 0)
    {
      pthread_mutex_unlock(allocationLock);
    }
}

/**
 * Increments the receivers reference count and returns the receiver.
 */
- (id) retain
{
  if (allocationLock != 0)
    {
      pthread_mutex_lock(allocationLock);
    }
  gc.flags.refCount++;
  if (allocationLock != 0)
    {
      pthread_mutex_unlock(allocationLock);
    }
  return self;
}

/**
 * Returns the receivers reference count.
 */
- (NSUInteger) retainCount
{
  return gc.flags.refCount;
}

@end

/**
 * This category implements accessor methods for the instance variables
 * used for garbage collecting.  If/when we can ensure that all garbage
 * collecting classes use the same initial ivar layout, we can remove
 * these methods and the garbage collector can access the ivars directly,
 * making a pretty big performance improvement during collecting.<br />
 * NB. These methods must *only* be used by the garbage collecting process
 * or in methods called from the garbage collector.  Anything else is not
 * thread-safe.
 */
@implementation GCObject (Extra)

- (BOOL) gcAlreadyVisited
{
  if (gc.flags.visited == 1)
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

- (GCObject*) gcNextObject
{
  return gc.next;
}

- (GCObject*) gcPreviousObject
{
  return gc.previous;
}

- (GCObject*) gcSetNextObject: (GCObject*)anObject
{
  gc.next = anObject;
  return self;
}

- (GCObject*) gcSetPreviousObject: (GCObject*)anObject
{
  gc.previous = anObject;
  return self;
}

- (void) gcSetVisited: (BOOL)flag
{
  if (flag == YES)
    {
      gc.flags.visited = 1;
    }
  else
    {
      gc.flags.visited = 0;
    }
}

@end

