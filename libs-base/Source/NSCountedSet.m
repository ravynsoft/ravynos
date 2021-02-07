/** NSCountedSet - CountedSet object
   Copyright (C) 1995, 1996 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Created: Sep 1995

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.

   <title>NSCountedSet class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#import "GNUstepBase/GSLock.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSSet.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSThread.h"

@class	GSCountedSet;
@interface GSCountedSet : NSObject	// Help the compiler
@end

/*
 *	Class variables for uniquing objects;
 */
static NSRecursiveLock	*uniqueLock = nil;
static NSCountedSet	*uniqueSet = nil;
static IMP		uniqueImp = 0;
static IMP		lockImp = 0;
static IMP		unlockImp = 0;
static BOOL		uniquing = NO;

/**
 * <p>
 *   The <code>NSCountedSet</code> class is used to maintain a set of objects
 *   where the number of times each object has been added (without a
 *   corresponding removal) is kept track of.
 * </p>
 * <p>
 *   In GNUstep, the <code>purge</code> and <code>unique</code> methods
 *   are provided to make use of a counted set for <em>uniquing</em> objects
 *   easier.
 * </p>
 */
@implementation NSCountedSet

static Class NSCountedSet_abstract_class;
static Class NSCountedSet_concrete_class;

+ (void) initialize
{
  if (self == [NSCountedSet class])
    {
      NSCountedSet_abstract_class = self;
      NSCountedSet_concrete_class = [GSCountedSet class];
      uniqueLock = [NSRecursiveLock new];
      [[NSObject leakAt: &uniqueLock] release];
      lockImp = [uniqueLock methodForSelector: @selector(lock)];
      unlockImp = [uniqueLock methodForSelector: @selector(unlock)];
    }
}

+ (id) allocWithZone: (NSZone*)z
{
  if (self == NSCountedSet_abstract_class)
    {
      return NSAllocateObject(NSCountedSet_concrete_class, 0, z);
    }
  else
    {
      return NSAllocateObject(self, 0, z);
    }
}

- (NSUInteger) _countForObject: (id)anObject
{
  return [self countForObject: anObject];
}

/**
 * Returns the number of times that an object that is equal to the
 * specified object (as determined by the [-isEqual:] method) has
 * been added to the set and not removed from it.
 */
- (NSUInteger) countForObject: (id)anObject
{
  [self subclassResponsibility: _cmd];
  return 0;
}

- (id) copyWithZone: (NSZone*)z
{
  return [[[self class] allocWithZone: z] initWithSet: self copyItems: YES];
}

- (id) mutableCopyWithZone: (NSZone*)z
{
  return [[[self class] allocWithZone: z] initWithSet: self copyItems: NO];
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  unsigned	count;
  Class		c = object_getClass(self);

  if (c == NSCountedSet_abstract_class)
    {
      DESTROY(self);
      self = [NSCountedSet_concrete_class allocWithZone: NSDefaultMallocZone()];
      return [self initWithCoder: aCoder];
    }
  [aCoder decodeValueOfObjCType: @encode(unsigned) at: &count];
  {
    id		objs[count];
    unsigned	refs[count];
    unsigned	i;
    IMP		addImp = [self methodForSelector: @selector(addObject:)];

    for (i = 0; i < count; i++)
      {
	[aCoder decodeValueOfObjCType: @encode(id) at: &objs[i]];
	[aCoder decodeValueOfObjCType: @encode(unsigned) at: &refs[i]];
      }
    self = [self initWithObjects: objs count: count];
    for (i = 0; i < count; i++)
      {
	unsigned	j = refs[i];

	while (j-- > 1)
	  {
	    (*addImp)(self, @selector(addObject:), objs[i]);
	  }
	RELEASE(objs[i]);
      }
  }
  return self;
}

- (Class) classForCoder
{
  return NSCountedSet_abstract_class;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  unsigned	count = [self count];
  NSEnumerator	*e = [self objectEnumerator];
  id		o;

  [aCoder encodeValueOfObjCType: @encode(unsigned) at: &count];
  while ((o = [e nextObject]) != nil)
    {
      [aCoder encodeValueOfObjCType: @encode(id) at: &o];
      count = [self countForObject: o];
      [aCoder encodeValueOfObjCType: @encode(unsigned) at: &count];
    }
}

- (id) initWithSet: (NSSet*)other copyItems: (BOOL)flag
{
  unsigned	c = [other count];
  id		os[c], o, e = [other objectEnumerator];
  unsigned	i = 0;
  NSZone	*z = [self zone];
  IMP		next = [e methodForSelector: @selector(nextObject)];

  while ((o = (*next)(e, @selector(nextObject))) != nil)
    {
      if (flag)
	os[i] = [o copyWithZone: z];
      else
	os[i] = o;
      i++;
    }
  self = [self initWithObjects: os count: c];
  if ([other isKindOfClass: NSCountedSet_abstract_class])
    {
      unsigned	j;
      IMP	addImp = [self methodForSelector: @selector(addObject:)];

      for (j = 0; j < i; j++)
	{
          unsigned	extra = [(NSCountedSet*)other countForObject: os[j]];

	  while (extra-- > 1)
	    (*addImp)(self, @selector(addObject:), os[j]);
	}
    }
  if (flag)
    while (i--)
      [os[i] release];
  return self;
}

- (void) purge: (NSInteger)level
{
  if (level > 0)
    {
      NSEnumerator	*enumerator = [self objectEnumerator];

      if (enumerator != nil)
	{
	  id		obj;
	  id		(*nImp)(NSEnumerator*, SEL);
	  unsigned	(*cImp)(NSCountedSet*, SEL, id);
	  void		(*rImp)(NSCountedSet*, SEL, id);

	  nImp = (id (*)(NSEnumerator*, SEL))
	    [enumerator methodForSelector: @selector(nextObject)];
	  cImp = (unsigned (*)(NSCountedSet*, SEL, id))
	    [self methodForSelector: @selector(countForObject:)];
	  rImp = (void (*)(NSCountedSet*, SEL, id))
	    [self methodForSelector: @selector(removeObject:)];
	  while ((obj = (*nImp)(enumerator, @selector(nextObject))) != nil)
	    {
	      unsigned	c = (*cImp)(self, @selector(countForObject:), obj);

	      if (c <= (NSUInteger)level)
		{
		  while (c-- > 0)
		    {
		      (*rImp)(self, @selector(removeObject:), obj);
		    }
		}
	    }
	}
    }
}

- (id) unique: (id)anObject
{
  id	o = [self member: anObject];

  [self addObject: anObject];
  if (o == nil)
    {
      o = anObject;
    }
  if (o != anObject)
    {
      [anObject release];
      [o retain];
    }
  return o;
}
@end

/**
 * This function purges the global [NSCountedSet] object used for
 * uniquing.  It handles locking as necessary.  It can be used to
 * purge the set even when uniquing is turned off.
 */
void
GSUPurge(NSUInteger count)
{
  if (uniqueLock != nil)
    {
      (*lockImp)(uniqueLock, @selector(lock));
    }
  [uniqueSet purge: count];
  if (uniqueLock != nil)
    {
      (*unlockImp)(uniqueLock, @selector(unlock));
    }
}

/**
 * This function sets the count for the specified object.  If the
 * count for the object is set to zero then the object is removed
 * from the global uniquing set.  The object is added to the set
 * if necessary.  The object returned is the one stored in the set.
 * The function handles locking as necessary.  It can be used to
 * alter the set even when uniquing is turned off.
 */
id
GSUSet(id anObject, NSUInteger count)
{
  id		found;
  NSUInteger	i;

  if (uniqueLock != nil)
    {
      (*lockImp)(uniqueLock, @selector(lock));
    }
  found = [uniqueSet member: anObject];
  if (found == nil)
    {
      found = anObject;
      for (i = 0; i < count; i++)
	{
	  [uniqueSet addObject: anObject];
	}
    }
  else
    {
      i = [uniqueSet countForObject: found];
      if (i < count)
	{
	  while (i < count)
	    {
	      [uniqueSet addObject: found];
	      i++;
	    }
	}
      else if (i > count)
	{
	  while (i > count)
	    {
	      [uniqueSet removeObject: found];
	      i--;
	    }
	}
    }
  if (uniqueLock != nil)
    {
      (*unlockImp)(uniqueLock, @selector(unlock));
    }
  return found;
}

/**
 * This function <em>uniques</em> the supplied argument, returning
 * the result.  It works by using the [-unique:] method of a global
 * NSCountedSet object.  It handles locking as necessary.
 * If uniquing is turned off, it simply returns its argument.
 */
id
GSUnique(id anObject)
{
  if (uniquing == YES)
    {
      if (uniqueLock != nil)
	{
	  (*lockImp)(uniqueLock, @selector(lock));
	}
      anObject = (*uniqueImp)(uniqueSet, @selector(unique:), anObject);
      if (uniqueLock != nil)
	{
	  (*unlockImp)(uniqueLock, @selector(unlock));
	}
    }
  return anObject;
}

/**
 * This function sets the state of a flag that determines the
 * behavior of the GSUnique() function.  If the flag is on,
 * uniquing is performed, if it is off the function has no effect.
 * The default is for uniquing to be turned off.
 */
void
GSUniquing(BOOL flag)
{
  if (uniqueSet == nil)
    {
      uniqueSet = [NSCountedSet new];
      uniqueImp = [uniqueSet methodForSelector: @selector(unique:)];
    }
  uniquing = flag;
}

