/** Concrete implementation of NSSet based on GNU Set class
   Copyright (C) 1995, 1996, 1998, 2000 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Created: September 1995
   Rewrite by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>

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
#import "Foundation/NSSet.h"
#import "GNUstepBase/GSObjCRuntime.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSPortCoder.h"
// For private method _decodeArrayOfObjectsForKey:
#import "Foundation/NSKeyedArchiver.h"
#import "GSPrivate.h"

#define	GSI_MAP_HAS_VALUE	0
#define	GSI_MAP_KTYPES		GSUNION_OBJ


#include "GNUstepBase/GSIMap.h"

static SEL	memberSel;
static SEL      privateCountOfSel;
@interface GSSet : NSSet
{
@public
  GSIMapTable_t	map;
}
@end

@interface GSMutableSet : NSMutableSet
{
@public
  GSIMapTable_t	map;
@private
  unsigned long	_version;
}
@end

@interface GSSetEnumerator : NSEnumerator
{
  GSSet			*set;
  GSIMapEnumerator_t	enumerator;
}
@end

@implementation GSSetEnumerator

- (id) initWithSet: (NSSet*)d
{
  self = [super init];
  if (self != nil)
    {
      set = (GSSet*)RETAIN(d);
      enumerator = GSIMapEnumeratorForMap(&set->map);
    }
  return self;
}

- (id) nextObject
{
  GSIMapNode node = GSIMapEnumeratorNextNode(&enumerator);

  if (node == 0)
    {
      return nil;
    }
  return node->key.obj;
}

- (void) dealloc
{
  GSIMapEndEnumerator(&enumerator);
  RELEASE(set);
  [super dealloc];
}

@end


@implementation GSSet

static Class	arrayClass;
static Class	setClass;
static Class	mutableSetClass;

- (NSUInteger) sizeOfContentExcluding: (NSHashTable*)exclude
{
  NSUInteger    	size = GSIMapSize(&map) - sizeof(GSIMapTable);
  GSIMapEnumerator_t	enumerator = GSIMapEnumeratorForMap(&map);
  GSIMapNode		node = GSIMapEnumeratorNextNode(&enumerator);

  while (node != 0)
    {
      size += [node->key.obj sizeInBytesExcluding: exclude];
      node = GSIMapEnumeratorNextNode(&enumerator);
    }
  GSIMapEndEnumerator(&enumerator);
  return size;
}

+ (void) initialize
{
  if (self == [GSSet class])
    {
      arrayClass = [NSArray class];
      setClass = [GSSet class];
      mutableSetClass = [GSMutableSet class];
      memberSel = @selector(member:);
      privateCountOfSel = @selector(_countForObject:);
    }
}

- (NSArray*) allObjects
{
  GSIMapEnumerator_t	enumerator = GSIMapEnumeratorForMap(&map);
  GSIMapNode 		node = GSIMapEnumeratorNextNode(&enumerator);
  NSUInteger		i = 0;
  NSArray		*result;
  GS_BEGINIDBUF(objects, map.nodeCount);

  while (node != 0)
    {
      objects[i++] = node->key.obj;
      node = GSIMapEnumeratorNextNode(&enumerator);
    }
  GSIMapEndEnumerator(&enumerator);
  result = AUTORELEASE([[arrayClass allocWithZone: NSDefaultMallocZone()]
    initWithObjects: objects count: i]);
  GS_ENDIDBUF();
  return result;
}

- (id) anyObject
{
  if (map.nodeCount > 0)
    {
      GSIMapBucket bucket = map.buckets;

      while (1)
	{
	  if (bucket->firstNode)
	    {
	      return bucket->firstNode->key.obj;
	    }
	  else
	    {
	      bucket++;
	    }
	}
    }
  else
    {
      return nil;
    }
}

- (id) copyWithZone: (NSZone*)z
{
  return RETAIN(self);
}

- (NSUInteger) count
{
  return map.nodeCount;
}

- (void) dealloc
{
  GSIMapEmptyMap(&map);
  [super dealloc];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [super encodeWithCoder: aCoder];
    }
  else
    {
      unsigned		count = map.nodeCount;
      SEL			sel = @selector(encodeObject:);
      IMP			imp = [aCoder methodForSelector: sel];
      GSIMapEnumerator_t	enumerator = GSIMapEnumeratorForMap(&map);
      GSIMapNode 		node = GSIMapEnumeratorNextNode(&enumerator);

      [aCoder encodeValueOfObjCType: @encode(unsigned) at: &count];
      while (node != 0)
	{
	  (*imp)(aCoder, sel, node->key.obj);
	  node = GSIMapEnumeratorNextNode(&enumerator);
	}
      GSIMapEndEnumerator(&enumerator);
    }
}

- (NSUInteger) hash
{
  return map.nodeCount;
}

- (id) init
{
  return [self initWithObjects: 0 count: 0];
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      self = [super initWithCoder: aCoder];
    }
  else
    {
      unsigned	count;
      id		value;
      SEL		sel = @selector(decodeValueOfObjCType:at:);
      IMP		imp = [aCoder methodForSelector: sel];
      const char	*type = @encode(id);

      (*imp)(aCoder, sel, @encode(unsigned), &count);

      GSIMapInitWithZoneAndCapacity(&map, [self zone], count);
      while (count-- > 0)
        {
	  (*imp)(aCoder, sel, type, &value);
	  GSIMapAddKeyNoRetain(&map, (GSIMapKey)value);
	}
    }
  return self;
}

/* Designated initialiser */
- (id) initWithObjects: (const id*)objs count: (NSUInteger)c
{
  NSUInteger i;

  GSIMapInitWithZoneAndCapacity(&map, [self zone], c);
  for (i = 0; i < c; i++)
    {
      GSIMapNode     node;

      if (objs[i] == nil)
	{
	  DESTROY(self);
	  [NSException raise: NSInvalidArgumentException
		      format: @"Tried to init set with nil value"];
	}
      node = GSIMapNodeForKey(&map, (GSIMapKey)objs[i]);
      if (node == 0)
	{
	  GSIMapAddKey(&map, (GSIMapKey)objs[i]);
        }
    }
  return self;
}

- (BOOL) intersectsSet: (NSSet*) otherSet
{
  Class	c;

  /* If this set is empty, or the other is nil/empty,
   * this method should return NO.
   */
  if (0 == map.nodeCount || nil == otherSet || [otherSet count] == 0)
    {
      return NO;
    }

  // Loop for all members in otherSet
  c = object_getClass(otherSet);
  if (c == setClass || c == mutableSetClass)
    {
      GSIMapTable		m = &((GSSet*)otherSet)->map;
      GSIMapEnumerator_t	enumerator = GSIMapEnumeratorForMap(m);
      GSIMapNode 		node = GSIMapEnumeratorNextNode(&enumerator);

      while (node != 0)
	{
	  if (GSIMapNodeForKey(&map, node->key) != 0)
	    {
	      GSIMapEndEnumerator(&enumerator);
	      return YES;
	    }
	  node = GSIMapEnumeratorNextNode(&enumerator);
	}
      GSIMapEndEnumerator(&enumerator);
    }
  else
    {
      NSEnumerator	*e;
      id		o;

      e = [otherSet objectEnumerator];
      while ((o = [e nextObject])) // 1. pick a member from otherSet.
	{
	  if (GSIMapNodeForKey(&map, (GSIMapKey)o) != 0)
	    {
	      return YES;
	    }
	}
    }
  return NO;
}

- (BOOL) isSubsetOfSet: (NSSet*) otherSet
{
  GSIMapEnumerator_t	enumerator;
  GSIMapNode 		node;
  IMP			imp;

  // -1. members of this set(self) <= that of otherSet
  if (map.nodeCount > [otherSet count])
    {
      return NO;
    }
  if (map.nodeCount == 0)
    {
      return YES;
    }

  imp = [otherSet methodForSelector: memberSel];
  enumerator = GSIMapEnumeratorForMap(&map);
  node = GSIMapEnumeratorNextNode(&enumerator);

  // 0. Loop for all members in this set(self).
  while (node != 0)
    {
      // 1. check the member is in the otherSet.
      if ((*imp)(otherSet, memberSel, node->key.obj) != nil)
	{
	  // 1.1 if true -> continue, try to check the next member.
	  node = GSIMapEnumeratorNextNode(&enumerator);
	}
      else
	{
	  // 1.2 if false -> return NO;
	  GSIMapEndEnumerator(&enumerator);
	  return NO;
	}
    }
  GSIMapEndEnumerator(&enumerator);
  // 2. return YES; all members in this set are also in the otherSet.
  return YES;
}

- (BOOL) isEqualToSet: (NSSet*)other
{
  if (other == nil)
    {
      return NO;
    }
  else if (other == self)
    {
      return YES;
    }
  else
    {
      Class	c = object_getClass(other);

      if (c == setClass || c == mutableSetClass)
	{
	  if (map.nodeCount != ((GSSet*)other)->map.nodeCount)
	    {
	      return NO;
	    }
	  else if (map.nodeCount == 0)
	    {
	      return YES;
	    }
	  else
	    {
	      GSIMapEnumerator_t	enumerator;
	      GSIMapNode 		node;

	      enumerator = GSIMapEnumeratorForMap(&map);
	      node = GSIMapEnumeratorNextNode(&enumerator);

	      while (node != 0)
		{
		  if (GSIMapNodeForKey(&(((GSSet*)other)->map), node->key) == 0)
		    {
		      GSIMapEndEnumerator(&enumerator);
		      return NO;
		    }
		  node = GSIMapEnumeratorNextNode(&enumerator);
		}
	      GSIMapEndEnumerator(&enumerator);
	    }
	}
      else
	{
	  if (map.nodeCount != [other count])
	    {
	      return NO;
	    }
	  else if (map.nodeCount == 0)
	    {
	      return YES;
	    }
	  else
	    {
	      GSIMapEnumerator_t	enumerator;
	      GSIMapNode 		node;
	      IMP			imp;
              IMP                       countImp;

	      imp = [other methodForSelector: memberSel];
              countImp = [other methodForSelector: privateCountOfSel];
	      enumerator = GSIMapEnumeratorForMap(&map);
	      node = GSIMapEnumeratorNextNode(&enumerator);

	      while (node != 0)
		{
		  if ((*imp)(other, memberSel, node->key.obj) == nil)
		    {
		      GSIMapEndEnumerator(&enumerator);
		      return NO;
		    }
                  else
                    {
                      NSUInteger c = (NSUInteger)
                       countImp(other,privateCountOfSel,node->key.obj);
                      // GSSet does not have duplicate entries
                      if (c != 1)
                        {
                          return NO;
                        }
                    }
		  node = GSIMapEnumeratorNextNode(&enumerator);
		}
	      GSIMapEndEnumerator(&enumerator);
	    }
	}
      return YES;
    }
}

- (BOOL) makeImmutable
{
  return YES;
}

- (void) makeObjectsPerform: (SEL)aSelector
{
  GSIMapEnumerator_t	enumerator = GSIMapEnumeratorForMap(&map);
  GSIMapNode 		node = GSIMapEnumeratorNextNode(&enumerator);

  while (node != 0)
    {
      [node->key.obj performSelector: aSelector];
      node = GSIMapEnumeratorNextNode(&enumerator);
    }
  GSIMapEndEnumerator(&enumerator);
}

- (void) makeObjectsPerformSelector: (SEL)aSelector
{
  GSIMapEnumerator_t	enumerator = GSIMapEnumeratorForMap(&map);
  GSIMapNode 		node = GSIMapEnumeratorNextNode(&enumerator);

  while (node != 0)
    {
      [node->key.obj performSelector: aSelector];
      node = GSIMapEnumeratorNextNode(&enumerator);
    }
  GSIMapEndEnumerator(&enumerator);
}

- (void) makeObjectsPerformSelector: (SEL)aSelector withObject: argument
{
  GSIMapEnumerator_t	enumerator = GSIMapEnumeratorForMap(&map);
  GSIMapNode 		node = GSIMapEnumeratorNextNode(&enumerator);

  while (node != 0)
    {
      [node->key.obj performSelector: aSelector withObject: argument];
      node = GSIMapEnumeratorNextNode(&enumerator);
    }
  GSIMapEndEnumerator(&enumerator);
}

- (void) makeObjectsPerform: (SEL)aSelector withObject: argument
{
  GSIMapEnumerator_t	enumerator = GSIMapEnumeratorForMap(&map);
  GSIMapNode 		node = GSIMapEnumeratorNextNode(&enumerator);

  while (node != 0)
    {
      [node->key.obj performSelector: aSelector withObject: argument];
      node = GSIMapEnumeratorNextNode(&enumerator);
    }
  GSIMapEndEnumerator(&enumerator);
}

- (id) member: (id)anObject
{
  if (anObject != nil)
    {
      GSIMapNode node = GSIMapNodeForKey(&map, (GSIMapKey)anObject);

      if (node != 0)
	{
	  return node->key.obj;
	}
    }
  return nil;
}

- (NSEnumerator*) objectEnumerator
{
  return AUTORELEASE([[GSSetEnumerator alloc] initWithSet: self]);
}

- (NSUInteger) countByEnumeratingWithState: (NSFastEnumerationState*)state
                                   objects: (id*)stackbuf
                                     count: (NSUInteger)len
{
  state->mutationsPtr = (unsigned long *)self;
  return GSIMapCountByEnumeratingWithStateObjectsCount
    (&map, state, stackbuf, len);
}

@end

@implementation GSMutableSet

- (NSUInteger) sizeOfContentExcluding: (NSHashTable*)exclude
{
  /* Can't safely calculate for mutable object; just buffer size
   */
  return map.nodeCount * sizeof(GSIMapNode);
}

+ (void) initialize
{
  if (self == [GSMutableSet class])
    {
      GSObjCAddClassBehavior(self, [GSSet class]);
    }
}

- (void) addObject: (id)anObject
{
  GSIMapNode node;

  if (anObject == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Tried to add nil to set"];
    }
  node = GSIMapNodeForKey(&map, (GSIMapKey)anObject);
  if (node == 0)
    {
      GSIMapAddKey(&map, (GSIMapKey)anObject);
      _version++;
    }
}

- (void) addObjectsFromArray: (NSArray*)array
{
  NSUInteger	count = [array count];

  while (count-- > 0)
    {
      id	anObject = [array objectAtIndex: count];

      if (anObject == nil)
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"Tried to add nil to set"];
	}
      else
	{
	  GSIMapNode node;

	  node = GSIMapNodeForKey(&map, (GSIMapKey)anObject);
	  if (node == 0)
	    {
	      GSIMapAddKey(&map, (GSIMapKey)anObject);
	      _version++;
	    }
	}
    }
}

/* Override _version from GSSet */
- (id) copyWithZone: (NSZone*)z
{
  NSSet	*copy = [setClass allocWithZone: z];

  return [copy initWithSet: self copyItems: NO];
}

- (id) init
{
  return [self initWithCapacity: 0];
}

/* Designated initialiser */
- (id) initWithCapacity: (NSUInteger)cap
{
  GSIMapInitWithZoneAndCapacity(&map, [self zone], cap);
  return self;
}

- (id) initWithObjects: (const id*)objects
		 count: (NSUInteger)count
{
  self = [self initWithCapacity: count];

  while (count--)
    {
      id	anObject = objects[count];

      if (anObject == nil)
	{
	  NSLog(@"Tried to init a set with a nil object");
	  continue;
	}
      else
	{
	  GSIMapNode node;

	  node = GSIMapNodeForKey(&map, (GSIMapKey)anObject);
	  if (node == 0)
	    {
	      GSIMapAddKey(&map, (GSIMapKey)anObject);
	    }
	}
    }
  return self;
}

- (void) intersectSet: (NSSet*)other
{
  if (nil == other)
    {
      GSIMapCleanMap(&map);
    }
  else if (other != self && map.nodeCount > 0)
    {
      GSIMapEnumerator_t	enumerator;
      GSIMapBucket		bucket;
      GSIMapNode 		node;
      Class                     c;

      if (NO == [other isKindOfClass: [NSSet class]])
        {
	  [NSException raise: NSInvalidArgumentException
		      format: @"-intersectSet: other object is not a set"];
        }
      if (0 == map.nodeCount)
        {
          return;                       // Already empty ... nothing to do
        }
      if (0 == [other count])
        {
          GSIMapCleanMap(&map);         // Other empty ... no intersection
          return;
        }

      c = object_getClass(other);
      if (c == setClass || c == mutableSetClass)
        {
          GSSet *o = (GSSet*)other;

          enumerator = GSIMapEnumeratorForMap(&map);
          bucket = GSIMapEnumeratorBucket(&enumerator);
          node = GSIMapEnumeratorNextNode(&enumerator);

          while (node != 0)
            {
              if (0 == GSIMapNodeForKey(&o->map, node->key))
                {
                  GSIMapRemoveNodeFromMap(&map, bucket, node);
                  GSIMapFreeNode(&map, node);
                }
              bucket = GSIMapEnumeratorBucket(&enumerator);
              node = GSIMapEnumeratorNextNode(&enumerator);
            }
          GSIMapEndEnumerator(&enumerator);
        }
      else
        {
	  SEL	sel = @selector(member:);
	  IMP	imp = [other methodForSelector: sel];

          enumerator = GSIMapEnumeratorForMap(&map);
          bucket = GSIMapEnumeratorBucket(&enumerator);
          node = GSIMapEnumeratorNextNode(&enumerator);

          while (node != 0)
            {
              if (nil == (*imp)(other, sel, node->key.obj))
                {
                  GSIMapRemoveNodeFromMap(&map, bucket, node);
                  GSIMapFreeNode(&map, node);
                }
              bucket = GSIMapEnumeratorBucket(&enumerator);
              node = GSIMapEnumeratorNextNode(&enumerator);
            }
          GSIMapEndEnumerator(&enumerator);
        }
    }
}

- (BOOL) makeImmutable
{
  GSClassSwizzle(self, [GSSet class]);
  return YES;
}

- (id) makeImmutableCopyOnFail: (BOOL)force
{
  GSClassSwizzle(self, [GSSet class]);
  return self;
}

- (void) minusSet: (NSSet*) other
{
  if (other == self)
    {
      GSIMapCleanMap(&map);
    }
  else
    {
      NSEnumerator	*e = [other objectEnumerator];
      id		anObject;

      while ((anObject = [e nextObject]) != nil)
	{
	  GSIMapRemoveKey(&map, (GSIMapKey)anObject);
	  _version++;
	}
    }
}

- (void) removeAllObjects
{
  GSIMapCleanMap(&map);
}

- (void) removeObject: (id)anObject
{
  if (anObject == nil)
    {
      NSWarnMLog(@"attempt to remove nil object");
      return;
    }
  GSIMapRemoveKey(&map, (GSIMapKey)anObject);
  _version++;
}

- (void) unionSet: (NSSet*) other
{
  if (other != self)
    {
      NSEnumerator	*e = [other objectEnumerator];

      if (e != nil)
	{
	  id	anObject;
	  SEL	sel = @selector(nextObject);
	  IMP	imp = [e methodForSelector: sel];

	  while ((anObject = (*imp)(e, sel)) != nil)
	    {
	      GSIMapNode node;

	      node = GSIMapNodeForKey(&map, (GSIMapKey)anObject);
	      if (node == 0)
		{
		  GSIMapAddKey(&map, (GSIMapKey)anObject);
		  _version++;
		}
	    }
	}
    }
}

- (NSUInteger) countByEnumeratingWithState: (NSFastEnumerationState*)state
                                   objects: (id*)stackbuf
                                     count: (NSUInteger)len
{
  state->mutationsPtr = &_version;
  return GSIMapCountByEnumeratingWithStateObjectsCount
    (&map, state, stackbuf, len);
}
@end

@interface	NSGSet : NSSet
@end
@implementation	NSGSet
- (id) initWithCoder: (NSCoder*)aCoder
{
  NSLog(@"Warning - decoding archive containing obsolete %@ object - please delete/replace this archive", NSStringFromClass([self class]));
  DESTROY(self);
  self = (id)NSAllocateObject([GSSet class], 0, NSDefaultMallocZone());
  self = [self initWithCoder: aCoder];
  return self;
}
@end

@interface	NSGMutableSet : NSMutableSet
@end
@implementation	NSGMutableSet
- (id) initWithCoder: (NSCoder*)aCoder
{
  NSLog(@"Warning - decoding archive containing obsolete %@ object - please delete/replace this archive", NSStringFromClass([self class]));
  DESTROY(self);
  self = (id)NSAllocateObject([GSMutableSet class], 0, NSDefaultMallocZone());
  self = [self initWithCoder: aCoder];
  return self;
}
@end

