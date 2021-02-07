/** Concrete implementation of GSOrderedSet and GSMutableOrderedSet
    based on GNU NSOrderedSet and NSMutableOrderedSet classes
    Copyright (C) 2019 Free Software Foundation, Inc.

    Written by: Gregory Casamento <greg.casamento@gmail.com>
    Created: May 17 2019

    This file is part of the GNUstep Base Library.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    _version 2 of the License, or (at your option) any later _version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110 USA.
*/

#import "common.h"
#import "Foundation/NSOrderedSet.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSPortCoder.h"
#import "Foundation/NSIndexSet.h"
#import "Foundation/NSKeyedArchiver.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSDictionary.h"
#import "GNUstepBase/GSObjCRuntime.h"
#import "GSPrivate.h"
#import "GSFastEnumeration.h"
#import "GSDispatch.h"
#import "GSSorting.h"

#define GSI_ARRAY_TYPES       GSUNION_OBJ

#import "GNUstepBase/GSIArray.h"

#define GSI_MAP_HAS_VALUE 0
#define GSI_MAP_KTYPES GSUNION_OBJ
#define GSI_MAP_RETAIN_KEY(M, X)
#define GSI_MAP_RELEASE_KEY(M, X)

#include "GNUstepBase/GSIMap.h"

@interface GSOrderedSet : NSOrderedSet
{
@public
  GSIArray_t array;
  GSIMapTable_t	map;
}
@end

@interface GSMutableOrderedSet : NSMutableOrderedSet
{
@public
  GSIArray_t array;
  GSIMapTable_t	map;
@private
  unsigned long	_version;
}
@end

@interface GSOrderedSetEnumerator : NSEnumerator
{
  GSOrderedSet *set;
  unsigned      current;
  unsigned      count;
}
@end

@interface GSOrderedSetEnumeratorReverse : GSOrderedSetEnumerator
@end

@implementation GSOrderedSetEnumerator
- (id) initWithOrderedSet: (NSOrderedSet*)d
{
  self = [super init];
  if (self != nil)
    {
      set = (GSOrderedSet*)RETAIN(d);
      current = 0;
      count = GSIArrayCount(&set->array);
    }
  return self;
}

- (id) nextObject
{
  if (current < count)
    {
      GSIArrayItem item = GSIArrayItemAtIndex(&set->array, current);
      current++;
      return (id)(item.obj);
    }
  return nil;
}

- (void) dealloc
{
  RELEASE(set);
  [super dealloc];
}
@end


@implementation GSOrderedSetEnumeratorReverse
- (id) initWithOrderedSet: (GSOrderedSet*)d
{
  self = [super initWithOrderedSet: d];
  if (self != nil)
    {
      current = GSIArrayCount(&set->array);
    }
  return self;
}

- (id) nextObject
{
  GSIArrayItem item;

  if (current == 0)
    {
      return nil;
    }

  item = GSIArrayItemAtIndex(&set->array, --current);
  return (id)(item.obj);
}
@end

@implementation GSOrderedSet

static Class	setClass;
static Class	mutableSetClass;

+ (void) initialize
{
  if (self == [GSOrderedSet class])
    {
      setClass = [GSOrderedSet class];
      mutableSetClass = [GSMutableOrderedSet class];
    }
}

- (id) copyWithZone: (NSZone*)z
{
  return RETAIN(self);
}

- (void) dealloc
{
  GSIArrayEmpty(&array);
  GSIMapEmptyMap(&map);
  [super dealloc];
}

- (NSUInteger) hash
{
  return [self count];
}

- (instancetype) init
{
  return [self initWithObjects: NULL count: 0];
}

- (NSEnumerator*) objectEnumerator
{
  return AUTORELEASE([[GSOrderedSetEnumerator alloc] initWithOrderedSet: self]);
}

- (NSEnumerator*) reverseObjectEnumerator
{
  return AUTORELEASE([[GSOrderedSetEnumeratorReverse alloc] initWithOrderedSet: self]);
}

- (NSUInteger) sizeInBytesExcluding: (NSHashTable*)exclude
{
  NSUInteger	size = GSPrivateMemorySize(self, exclude);

  if (size > 0)
    {
      NSUInteger count = [self count];
      NSUInteger i = 0;

      for (i = 0; i < count; i++)
	{
	  GSIArrayItem item = GSIArrayItemAtIndex(&array, i);
          size += [item.obj sizeInBytesExcluding: exclude];
        }
    }
  return size;
}

// Put required overrides here...
- (BOOL) containsObject: (id)anObject
{
  if (anObject != nil)
    {
      GSIMapNode node = GSIMapNodeForKey(&map, (GSIMapKey)anObject);

      if (node != 0)
	{
	  return YES;
	}
    }
  return NO;
}

- (NSUInteger) count
{
  return GSIArrayCount(&array);
}

- (id) objectAtIndex: (NSUInteger)index
{
  GSIArrayItem item = GSIArrayItemAtIndex(&array, index);
  return item.obj;
}

- (void) getObjects: (__unsafe_unretained id[])aBuffer range: (NSRange)aRange
{
  NSUInteger i, j = 0;
  NSUInteger c = GSIArrayCount(&array);
  NSUInteger e = NSMaxRange(aRange);

  GS_RANGE_CHECK(aRange, c);

  for (i = aRange.location; i < e; i++)
    {
      GSIArrayItem item = GSIArrayItemAtIndex(&array, i);
      aBuffer[j++] = item.obj;
    }
}

/* Designated initialiser */
- (id) initWithObjects: (const id*)objs count: (NSUInteger)c
{
  NSUInteger i = 0;

  // Initialize and fill the set.
  GSIArrayInitWithZoneAndCapacity(&array, [self zone], c);
  GSIMapInitWithZoneAndCapacity(&map, [self zone], c);
  for (i = 0; i < c; i++)
    {
      id obj = objs[i];

      if (obj == nil)
	{
	  DESTROY(self);
	  [NSException raise: NSInvalidArgumentException
		      format: @"Tried to init set with nil value"];
	}

      if (![self containsObject: obj])
	{
          GSIArrayItem item;

          item.obj = obj;
	  GSIArrayAddItem(&array, item);
	  GSIMapAddKey(&map, (GSIMapKey)obj);
	}
     }
  return self;
}

@end

@implementation GSMutableOrderedSet

+ (void) initialize
{
  if (self == [GSMutableOrderedSet class])
    {
      GSObjCAddClassBehavior(self, [GSOrderedSet class]);
    }
}

- (void) insertObject: (id)object atIndex: (NSUInteger)index
{
  if (object == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Tried to add nil to set"];
    }
  else
    {
      if ([self containsObject: object] == NO)
	{
          GSIArrayItem item;

	  _version++;
	  item.obj = object;
	  GSIArrayInsertItem(&array, item, index);
          GSIMapAddKey(&map, (GSIMapKey)object);
	  _version++;
	}
    }
}

- (void) removeObjectAtIndex: (NSUInteger)index
{
  GSIArrayItem item = GSIArrayItemAtIndex(&array, index);

  _version++;
  GSIArrayRemoveItemAtIndex(&array, index);
  GSIMapRemoveKey(&map, (GSIMapKey)item.obj);
  _version++;
}

- (id) init
{
  return [self initWithCapacity: 0];
}

/* Designated initialiser */
- (id) initWithCapacity: (NSUInteger)cap
{
  GSIArrayInitWithZoneAndCapacity(&array, [self zone], cap);
  GSIMapInitWithZoneAndCapacity(&map, [self zone], cap);
  return self;
}

- (id) initWithObjects: (const id*)objects
		 count: (NSUInteger)count
{
  NSUInteger i = 0;

  // Init and fill set
  self = [self initWithCapacity: count];
  if (self != nil)
    {
      for (i = 0; i < count; i++)
	{
	  id	anObject = objects[i];
	  [self addObject: anObject];
	}
    }
  return self;
}

- (BOOL) makeImmutable
{
  GSClassSwizzle(self, [GSOrderedSet class]);
  return YES;
}

- (id) makeImmutableCopyOnFail: (BOOL)force
{
  GSClassSwizzle(self, [GSOrderedSet class]);
  return self;
}

- (NSUInteger) countByEnumeratingWithState: (NSFastEnumerationState *)state
				   objects: (__unsafe_unretained id[])stackbuf
				     count: (NSUInteger)len
{
  NSInteger count;

  /* This is cached in the caller at the start and compared at each
   * iteration.   If it changes during the iteration then
   * objc_enumerationMutation() will be called, throwing an exception.
   */
  state->mutationsPtr = &_version;
  count = MIN(len, [self count] - state->state);
  /* If a mutation has occurred then it's possible that we are being asked to
   * get objects from after the end of the array.  Don't pass negative values
   * to memcpy.
   */
  if (count > 0)
    {
      [self getObjects: stackbuf range: NSMakeRange(state->state, count)];
      state->state += count;
    }
  else
    {
      count = 0;
    }
  state->itemsPtr = stackbuf;
  return count;
}

@end
