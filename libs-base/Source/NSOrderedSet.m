/** Implementation for NSOrderedSet, NSMutableOrderedSet for GNUStep
   Copyright (C) 2019 Free Software Foundation, Inc.

   Written by: Gregory John Casamento <greg.casamento@gmail.com>
   Created: May 17 2019

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

*/
#import "common.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSKeyedArchiver.h"
#import "Foundation/NSKeyValueCoding.h"
#import "Foundation/NSKeyValueObserving.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSOrderedSet.h"
#import "Foundation/NSPredicate.h"
#import "Foundation/NSValue.h"

#import <GNUstepBase/GSBlocks.h>
#import "GSPrivate.h"
#import "GSFastEnumeration.h"
#import "GSDispatch.h"
#import "GSSorting.h"

@class	GSOrderedSet;
@interface GSOrderedSet : NSObject	// Help the compiler
@end

@class	GSMutableOrderedSet;
@interface GSMutableOrderedSet : NSObject	// Help the compiler
@end

@interface NSOrderedSet (Private)
- (void) _raiseRangeExceptionWithIndex: (NSUInteger)index from: (SEL)sel;
@end

@implementation NSOrderedSet

static Class NSOrderedSet_abstract_class;
static Class NSMutableOrderedSet_abstract_class;
static Class NSOrderedSet_concrete_class;
static Class NSMutableOrderedSet_concrete_class;

static SEL	eqSel;
static SEL	oaiSel;
static SEL	remSel;

+ (id) allocWithZone: (NSZone*)z
{
  if (self == NSOrderedSet_abstract_class)
    {
      return NSAllocateObject(NSOrderedSet_concrete_class, 0, z);
    }
  else
    {
      return NSAllocateObject(self, 0, z);
    }
}

+ (void) initialize
{
  if (self == [NSOrderedSet class])
    {
      [self setVersion: 1];

      eqSel = @selector(isEqual:);
      oaiSel = @selector(objectAtIndex:);
      remSel = @selector(removeObjectAtIndex:);

      NSOrderedSet_abstract_class = self;
      NSOrderedSet_concrete_class = [GSOrderedSet class];
      [NSMutableSet class];
    }
}


- (void) _raiseRangeExceptionWithIndex: (NSUInteger)index from: (SEL)sel
{
  NSDictionary *info;
  NSException  *exception;
  NSString     *reason;
  NSUInteger    count = [self count];

  info = [NSDictionary dictionaryWithObjectsAndKeys:
    [NSNumber numberWithUnsignedInteger: index], @"Index",
    [NSNumber numberWithUnsignedInteger: count], @"Count",
    self, @"GSMutableSet", nil, nil];

  reason = [NSString stringWithFormat:
    @"Index %"PRIuPTR" is out of range %lu (in '%@')",
    index, count, NSStringFromSelector(sel)];

  exception = [NSException exceptionWithName: NSRangeException
		                      reason: reason
                                    userInfo: info];
  [exception raise];
}

- (Class) classForCoder
{
  return NSOrderedSet_abstract_class;
}

// NSCoding
- (instancetype) initWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      id	array;

      array = [(NSKeyedUnarchiver*)coder _decodeArrayOfObjectsForKey:
						@"NS.objects"];
      if (array == nil)
	{
	  unsigned	i = 0;
	  NSString	*key;
	  id		val;

	  array = [NSMutableArray arrayWithCapacity: 2];
	  key = [NSString stringWithFormat: @"NS.object.%u", i];
	  val = [(NSKeyedUnarchiver*)coder decodeObjectForKey: key];

	  while (val != nil)
	    {
	      [array addObject: val];
	      i++;
	      key = [NSString stringWithFormat: @"NS.object.%u", i];
	      val = [(NSKeyedUnarchiver*)coder decodeObjectForKey: key];
	    }
	}
      self = [self initWithArray: array];
    }
  else
    {
      unsigned	count;

      [coder decodeValueOfObjCType: @encode(unsigned) at: &count];
      if (count > 0)
        {
	  unsigned	i;
	  GS_BEGINIDBUF(objs, count);

	  for (i = 0; i < count; i++)
	    {
	      [coder decodeValueOfObjCType: @encode(id) at: &objs[i]];
	    }
	  self = [self initWithObjects: objs count: count];
	  while (count-- > 0)
	    {
	      [objs[count] release];
	    }
	  GS_ENDIDBUF();
	}
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)aCoder
{
   if ([aCoder allowsKeyedCoding])
    {
      if ([aCoder class] == [NSKeyedArchiver class])
	{
          /* HACK ... MacOS-X seems to code differently if the coder is an
           * actual instance of NSKeyedArchiver
           */
          NSArray *array = [self array];

	  [(NSKeyedArchiver*)aCoder _encodeArrayOfObjects: array
						   forKey: @"NS.objects"];
	}
      else
	{
	  unsigned	i = 0;
	  NSEnumerator	*e = [self objectEnumerator];
	  id		o;

	  while ((o = [e nextObject]) != nil)
	    {
	      NSString	*key;

	      key = [NSString stringWithFormat: @"NS.object.%u", i++];
	      [(NSKeyedArchiver*)aCoder encodeObject: o forKey: key];
	    }
	}
    }
  else
    {
      unsigned		count = [self count];
      NSEnumerator	*e = [self objectEnumerator];
      id		o;

      [aCoder encodeValueOfObjCType: @encode(unsigned) at: &count];
      while ((o = [e nextObject]) != nil)
	{
	  [aCoder encodeValueOfObjCType: @encode(id) at: &o];
	}
    }
}

- (id) copyWithZone: (NSZone*)zone
{
  NSOrderedSet	*copy = [NSOrderedSet_concrete_class allocWithZone: zone];
  return [copy initWithOrderedSet: self copyItems: YES];
}

- (id) mutableCopyWithZone: (NSZone*)zone
{
   NSMutableOrderedSet	*copy = [NSMutableOrderedSet_concrete_class allocWithZone: zone];
   return [copy initWithOrderedSet: self copyItems: NO];
}

// NSFastEnumeration
- (NSUInteger) countByEnumeratingWithState: (NSFastEnumerationState *)state
				   objects: (__unsafe_unretained id[])stackbuf
				     count: (NSUInteger)len
{
  NSInteger count;

  /* In a mutable subclass, the mutationsPtr should be set to point to a
   * value (unsigned long) which will be changed (incremented) whenever
   * the container is mutated (content added, removed, re-ordered).
   * This is cached in the caller at the start and compared at each
   * iteration.   If it changes during the iteration then
   * objc_enumerationMutation() will be called, throwing an exception.
   * The abstract base class implementation points to a fixed value
   * (the enumeration state pointer should exist and be unchanged for as
   * long as the enumeration process runs), which is fine for enumerating
   * an immutable array.
   */
  state->mutationsPtr = (unsigned long *)&state->mutationsPtr;
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

// class methods
+ (instancetype) orderedSet
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()] init]);
}

+ (instancetype) orderedSetWithArray: (NSArray *)objects
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
			 initWithArray: objects]);
}

+ (instancetype) orderedSetWithArray: (NSArray *)objects
                               range: (NSRange)range
                           copyItems: (BOOL)flag
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
			 initWithArray: objects
				 range: range
			     copyItems: flag]);
}

+ (instancetype) orderedSetWithObject: (id)anObject
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
			 initWithObject: anObject]);
}

+ (instancetype) orderedSetWithObjects: (id)firstObject, ...
{
  id	set;
  GS_USEIDLIST(firstObject,
	       set = [[self allocWithZone: NSDefaultMallocZone()]
		       initWithObjects: __objects count: __count]);
  return AUTORELEASE(set);
 }

+ (instancetype) orderedSetWithObjects: (const id [])objects
                                 count: (NSUInteger) count
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
		       initWithObjects: objects count: count]);
}

+ (instancetype) orderedSetWithOrderedSet: (NSOrderedSet *)aSet
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
		       initWithOrderedSet: aSet]);
}


+ (instancetype) orderedSetWithSet: (NSSet *)aSet
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
		       initWithSet: aSet]);
}

+ (instancetype) orderedSetWithSet: (NSSet *)aSet
                         copyItems: (BOOL)flag
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
			initWithSet: aSet
			  copyItems: flag]);
}

// instance methods
- (instancetype) initWithArray: (NSArray *)other
{
  unsigned	count = [other count];

  if (count == 0)
    {
      return [self init];
    }
  else
    {
      GS_BEGINIDBUF(objs, count);

      if ([other isProxy])
	{
	  unsigned	i;

	  for (i = 0; i < count; i++)
	    {
	      objs[i] = [other objectAtIndex: i];
	    }
	}
      else
	{
          [other getObjects: objs];
	}
      self = [self initWithObjects: objs count: count];
      GS_ENDIDBUF();
      return self;
    }

  return nil;
}

- (instancetype) initWithArray: (NSArray *)other copyItems: (BOOL)flag
{
  unsigned	count = [other count];
  unsigned      j = count;

  if (count == 0)
    {
      return [self init];
    }

  GS_BEGINIDBUF(objs, count);
  {
    unsigned	i;

    for (i = 0; i < count; i++)
      {
	if (flag == NO)
	  {
	    objs[i] = [other objectAtIndex: i];
	  }
	else // copy the items.
	  {
	    objs[i] = [[other objectAtIndex: i] copy];
	  }
      }
  }

  self = [self initWithObjects: objs count: count];

  if (flag == YES)
    {
      while(j--)
	{
	  [objs[j] release];
	}
    }
  GS_ENDIDBUF();
  return self;
}

- (instancetype) initWithArray: (NSArray *)other
                         range: (NSRange)range
                     copyItems: (BOOL)flag
{
  unsigned	count = [other count];
  unsigned      i = 0, j = 0;

  if (count == 0)
    {
      return [self init];
    }

  GS_BEGINIDBUF(objs, range.length);
  {
    unsigned      loc = range.location;
    unsigned      len = range.length;

    for (i = 0; i < count; i++)
      {
	if (i >= loc && j < len)
	  {
	    if (flag == YES)
	      {
		objs[i] = [[other objectAtIndex: i] copy];
	      }
	    else
	      {
		objs[i] = [other objectAtIndex: i];
	      }
	    j++;
	  }

	if (j >= len)
	  {
	    break;
	  }
      }
  }
  self = [self initWithObjects: objs count: count];

  if (flag == YES)
    {
      while(j--)
	{
	  [objs[j] release];
	}
    }
  GS_ENDIDBUF();

  return self;
}

- (instancetype) initWithObject: (id)obj
{
  id objs[] = {obj};

  self = [self initWithObjects: objs count: 1];
  return self;
}

- (instancetype) initWithObjects: (id)firstObject, ...
{
  GS_USEIDLIST(firstObject,
    self = [self initWithObjects: __objects count: __count]);
  return self;
}

/** <init /> <override-subclass />
 * Initialize to contain (unique elements of) objects.<br />
 * Calls -init (which does nothing but maintain MacOS-X compatibility),
 * and needs to be re-implemented in subclasses in order to have all
 * other initialisers work.
 */
- (instancetype) initWithObjects: (const id [])objects // required override.
                           count: (NSUInteger)count
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (instancetype) initWithOrderedSet: (NSOrderedSet *)aSet
{
  return [self initWithOrderedSet: aSet copyItems: NO];
}

- (instancetype) initWithOrderedSet: (NSOrderedSet *)other
                          copyItems: (BOOL)flag
{
  unsigned	c = [other count];
  id		o, e = [other objectEnumerator];
  unsigned	i = 0;
  GS_BEGINIDBUF(os, c);

  while ((o = [e nextObject]))
    {
      if (flag)
        {
          os[i] = [o copy];
        }
      else
        {
          os[i] = o;
        }
      i++;
    }
  self = [self initWithObjects: os count: c];
  if (flag)
    {
      while (i--)
        {
          [os[i] release];
        }
    }
  GS_ENDIDBUF();
  return self;
}

- (instancetype) initWithOrderedSet: (NSOrderedSet *)other
                              range: (NSRange)range
                          copyItems: (BOOL)flag
{
  unsigned	c = [other count];
  id		o, e = [other objectEnumerator];
  unsigned	i = 0, j = 0;
  unsigned      loc = range.location;
  unsigned      len = range.length;
  GS_BEGINIDBUF(os, c);

  while ((o = [e nextObject]))
    {
      if (i >= loc && j < len)
	{
	  if (flag)
            {
              os[i] = [o copy];
            }
	  else
            {
              os[i] = o;
            }
	  j++;
	}
      i++;

      if (j >= len)
	{
	  break;
	}
    }

  self = [self initWithObjects: os count: c];
  if (flag)
    {
      while (i--)
        {
          [os[i] release];
        }
    }
  GS_ENDIDBUF();
  return self;
}

- (instancetype) initWithSet: (NSSet *)aSet
{
  return [self initWithSet: aSet copyItems: NO];
}

- (instancetype) initWithSet: (NSSet *)other copyItems: (BOOL)flag
{
  unsigned	c = [other count];
  id		o, e = [other objectEnumerator];
  unsigned	i = 0;
  GS_BEGINIDBUF(os, c);

  while ((o = [e nextObject]))
    {
      if (flag)
        {
          os[i] = [o copy];
        }
      else
        {
          os[i] = o;
        }
      i++;
    }
  self = [self initWithObjects: os count: c];
  if (flag)
    {
      while (i--)
        {
          [os[i] release];
        }
    }
  GS_ENDIDBUF();
  return self;
}

- (instancetype) init
{
  return [self initWithObjects: NULL count:0];
}

- (NSUInteger) count // required override
{
  [self subclassResponsibility: _cmd];
  return 0;
}

- (BOOL) containsObject: (id)anObject
{
  NSUInteger i = [self indexOfObject: anObject];
  if (i == NSNotFound)
    {
      return NO;
    }
  return YES;
}

- (void) enumerateObjectsAtIndexes: (NSIndexSet *)indexSet
                           options: (NSEnumerationOptions)opts
                        usingBlock: (GSEnumeratorBlock)aBlock
{
  [[self objectsAtIndexes: indexSet] enumerateObjectsWithOptions: opts
						      usingBlock: aBlock];
}

- (void) enumerateObjectsUsingBlock: (GSEnumeratorBlock)aBlock
{
  [self enumerateObjectsWithOptions: 0 usingBlock: aBlock];
}

- (void) enumerateObjectsWithOptions: (NSEnumerationOptions)opts
                          usingBlock: (GSEnumeratorBlock)aBlock
{
  NSUInteger count = 0;
  BLOCK_SCOPE BOOL shouldStop = NO;
  BOOL isReverse = (opts & NSEnumerationReverse);
  id<NSFastEnumeration> enumerator = self;

  /* If we are enumerating in reverse, use the reverse enumerator for fast
   * enumeration. */
  if (isReverse)
    {
      enumerator = [self reverseObjectEnumerator];
      count = ([self count] - 1);
    }

  {
  GS_DISPATCH_CREATE_QUEUE_AND_GROUP_FOR_ENUMERATION(enumQueue, opts)
  FOR_IN (id, obj, enumerator)
    GS_DISPATCH_SUBMIT_BLOCK(enumQueueGroup, enumQueue, if (shouldStop == NO) {, }, aBlock, obj, count, &shouldStop);
      if (isReverse)
        {
          count--;
        }
      else
        {
          count++;
        }

      if (shouldStop)
        {
          break;
        }
    END_FOR_IN(enumerator)
    GS_DISPATCH_TEARDOWN_QUEUE_AND_GROUP_FOR_ENUMERATION(enumQueue, opts)
  }
}

- (id) firstObject
{
  NSUInteger count = [self count];
  if (count == 0)
    {
      return nil;
    }
  return [self objectAtIndex: 0];
}

- (id) lastObject
{
   NSUInteger count = [self count];
  if (count == 0)
    {
      return nil;
    }
  return [self objectAtIndex: count - 1];
}

- (id) objectAtIndex: (NSUInteger)index  // required override...
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (id) objectAtIndexedSubscript: (NSUInteger)index
{
  return [self objectAtIndex: index];
}

- (NSArray *) objectsAtIndexes: (NSIndexSet *)indexes
{
  NSMutableArray *group = [NSMutableArray arrayWithCapacity: [indexes count]];

  NSUInteger i = [indexes firstIndex];
  while (i != NSNotFound)
    {
      [group addObject: [self objectAtIndex: i]];
      i = [indexes indexGreaterThanIndex: i];
    }

  return GS_IMMUTABLE(group);
}

- (NSUInteger) indexOfObject: (id)anObject
{
  NSUInteger	c = [self count];

  if (c > 0 && anObject != nil)
    {
      NSUInteger	i;
      IMP	get = [self methodForSelector: oaiSel];
      BOOL	(*eq)(id, SEL, id)
	= (BOOL (*)(id, SEL, id))[anObject methodForSelector: eqSel];

      for (i = 0; i < c; i++)
	if ((*eq)(anObject, eqSel, (*get)(self, oaiSel, i)) == YES)
	  return i;
    }
  return NSNotFound;
}

- (NSUInteger) indexOfObject: (id)key
               inSortedRange: (NSRange)range
                     options: (NSBinarySearchingOptions)options
             usingComparator: (NSComparator)comparator
{
  if (range.length == 0)
    {
      return options & NSBinarySearchingInsertionIndex
        ? range.location : NSNotFound;
    }
  if (range.length == 1)
    {
      switch (CALL_BLOCK(comparator, key, [self objectAtIndex: range.location]))
        {
          case NSOrderedSame:
            return range.location;
          case NSOrderedAscending:
            return options & NSBinarySearchingInsertionIndex
              ? range.location : NSNotFound;
          case NSOrderedDescending:
            return options & NSBinarySearchingInsertionIndex
              ? (range.location + 1) : NSNotFound;
          default:
            // Shouldn't happen
            return NSNotFound;
        }
    }
  else
    {
      NSUInteger index = NSNotFound;
      NSUInteger count = [self count];
      NSRange range = NSMakeRange(0, [self count]);
      GS_BEGINIDBUF(objects, count);

      [self getObjects: objects range: range];
      // We use the timsort galloping to find the insertion index:
      if (options & NSBinarySearchingLastEqual)
        {
          index = GSRightInsertionPointForKeyInSortedRange(key,
            objects, range, comparator);
        }
      else
        {
          // Left insertion is our default
          index = GSLeftInsertionPointForKeyInSortedRange(key,
            objects, range, comparator);
        }
      GS_ENDIDBUF()

      // If we were looking for the insertion point, we are done here
      if (options & NSBinarySearchingInsertionIndex)
        {
          return index;
        }

      /* Otherwise, we need need another equality check in order to
       * know whether we need return NSNotFound.
       */

      if (options & NSBinarySearchingLastEqual)
        {
          /* For search from the right, the equal object would be
           * the one before the index, but only if it's not at the
           * very beginning of the range (though that might not
           * actually be possible, it's better to check nonetheless).
           */
          if (index > range.location)
            {
              index--;
            }
        }
      if (index >= NSMaxRange(range))
        {
          return NSNotFound;
        }
      /*
       * For a search from the left, we'd have the correct index anyways. Check
       * whether it's equal to the key and return NSNotFound otherwise
       */
      return (NSOrderedSame == CALL_BLOCK(comparator,
        key, [self objectAtIndex: index]) ? index : NSNotFound);
    }
  // Never reached
  return NSNotFound;
}

- (NSUInteger) indexOfObjectAtIndexes: (NSIndexSet *)indexSet
                              options: (NSEnumerationOptions)opts
                          passingTest: (GSPredicateBlock)predicate
{
  return [[self array]
           indexOfObjectAtIndexes: indexSet
                          options: opts
                      passingTest: predicate];
}

- (NSUInteger) indexOfObjectPassingTest: (GSPredicateBlock)predicate
{
  return [self indexOfObjectWithOptions: 0 passingTest: predicate];
}

- (NSUInteger) indexOfObjectWithOptions: (NSEnumerationOptions)opts
                            passingTest: (GSPredicateBlock)predicate
{
   /* TODO: Concurrency. */
  id<NSFastEnumeration> enumerator = self;
  BLOCK_SCOPE BOOL      shouldStop = NO;
  NSUInteger            count = 0;
  BLOCK_SCOPE NSUInteger index = NSNotFound;
  BLOCK_SCOPE NSLock    *indexLock = nil;

  /* If we are enumerating in reverse, use the reverse enumerator for fast
   * enumeration. */
  if (opts & NSEnumerationReverse)
    {
      enumerator = [self reverseObjectEnumerator];
    }

  if (opts & NSEnumerationConcurrent)
    {
      indexLock = [NSLock new];
    }
  {
    GS_DISPATCH_CREATE_QUEUE_AND_GROUP_FOR_ENUMERATION(enumQueue, opts)
    FOR_IN (id, obj, enumerator)
#     if __has_feature(blocks) && (GS_USE_LIBDISPATCH == 1)
      if (enumQueue != NULL)
        {
          dispatch_group_async(enumQueueGroup, enumQueue, ^(void){
            if (shouldStop)
            {
              return;
            }
            if (predicate(obj, count, &shouldStop))
            {
              // FIXME: atomic operation on the shouldStop variable would be nicer,
              // but we don't expose the GSAtomic* primitives anywhere.
              [indexLock lock];
              index =  count;
              // Cancel all other predicate evaluations:
              shouldStop = YES;
              [indexLock unlock];
            }
          });
        }
      else // call block directly
#     endif
      if (CALL_BLOCK(predicate, obj, count, &shouldStop))
        {
          index = count;
          shouldStop = YES;
        }
      if (shouldStop)
        {
          break;
        }
      count++;
    END_FOR_IN(enumerator)
    GS_DISPATCH_TEARDOWN_QUEUE_AND_GROUP_FOR_ENUMERATION(enumQueue, opts);
  }
  RELEASE(indexLock);
  return index;
}

- (NSIndexSet *) indexesOfObjectsAtIndexes: (NSIndexSet *)indexSet
				   options: (NSEnumerationOptions)opts
			       passingTest: (GSPredicateBlock)predicate
{
  return [[self array]
           indexesOfObjectsAtIndexes: indexSet
                             options: opts
                         passingTest: predicate];
}

- (NSIndexSet *) indexesOfObjectsPassingTest: (GSPredicateBlock)predicate
{
  return [self indexesOfObjectsWithOptions: 0 passingTest: predicate];
}

- (NSIndexSet *) indexesOfObjectsWithOptions: (NSEnumerationOptions)opts
				 passingTest: (GSPredicateBlock)predicate
{
  /* TODO: Concurrency. */
  NSMutableIndexSet     *set = [NSMutableIndexSet indexSet];
  BLOCK_SCOPE BOOL      shouldStop = NO;
  id<NSFastEnumeration> enumerator = self;
  NSUInteger            count = 0;
  BLOCK_SCOPE NSLock    *setLock = nil;

  /* If we are enumerating in reverse, use the reverse enumerator for fast
   * enumeration. */
  if (opts & NSEnumerationReverse)
    {
      enumerator = [self reverseObjectEnumerator];
    }
  if (opts & NSEnumerationConcurrent)
    {
      setLock = [NSLock new];
    }
  {
    GS_DISPATCH_CREATE_QUEUE_AND_GROUP_FOR_ENUMERATION(enumQueue, opts)
    FOR_IN (id, obj, enumerator)
#     if __has_feature(blocks) && (GS_USE_LIBDISPATCH == 1)
      if (enumQueue != NULL)
        {
          dispatch_group_async(enumQueueGroup, enumQueue, ^(void){
            if (shouldStop)
            {
              return;
            }
            if (predicate(obj, count, &shouldStop))
            {
              [setLock lock];
              [set addIndex: count];
              [setLock unlock];
            }
          });
        }
      else // call block directly
#     endif
      if (CALL_BLOCK(predicate, obj, count, &shouldStop))
        {
          /* TODO: It would be more efficient to collect an NSRange and only
           * pass it to the index set when CALL_BLOCK returned NO. */
          [set addIndex: count];
        }
      if (shouldStop)
        {
          break;
        }
      count++;
    END_FOR_IN(enumerator)
    GS_DISPATCH_TEARDOWN_QUEUE_AND_GROUP_FOR_ENUMERATION(enumQueue, opts);
  }
  RELEASE(setLock);
  return set;
}

- (NSEnumerator *) objectEnumerator
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (NSEnumerator *) reverseObjectEnumerator
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (NSOrderedSet *) reversedOrderedSet
{
  NSEnumerator *e = [self reverseObjectEnumerator];
  NSMutableArray *a = [NSMutableArray arrayWithCapacity: [self count]];
  id o = nil;

  // Build the reverse array...
  while ((o = [e nextObject]) != nil)
    {
      [a addObject: o];
    }

  // Create and return reverse ordered set...
  return [NSOrderedSet orderedSetWithArray: a];
}

- (void) getObjects: (__unsafe_unretained id[])aBuffer range: (NSRange)aRange
{
  NSUInteger i, j = 0;
  NSUInteger c = [self count];
  NSUInteger e = NSMaxRange(aRange);
  IMP	get = [self methodForSelector: oaiSel];

  GS_RANGE_CHECK(aRange, c);

  for (i = aRange.location; i < e; i++)
    {
      aBuffer[j++] = (*get)(self, oaiSel, i);
    }
}

// Key-Value Observing Support

- (void) addObserver: (NSObject *)observer
          forKeyPath: (NSString *)keyPath
             options: (NSKeyValueObservingOptions)options
             context: (void *)context
{
  [[NSException exceptionWithName: NSGenericException
                           reason: @"NSOrderedSet does not support KVO"
                         userInfo: nil] raise];
}

- (void) removeObserver: (NSObject *)observer
             forKeyPath: (NSString *)keyPath
{
  [[NSException exceptionWithName: NSGenericException
                           reason: @"NSOrderedSet does not support KVO"
                         userInfo: nil] raise];
}

- (void) removeObserver: (NSObject *)observer
	     forKeyPath: (NSString *)keyPath
		context: (void *)context
{
  [[NSException exceptionWithName: NSGenericException
                           reason: @"NSOrderedSet does not support KVO"
                         userInfo: nil] raise];
}

// Key value coding support
- (void) setValue: (id)value forKey: (NSString*)key
{
  id	object;
  NSEnumerator *e = [self objectEnumerator];

  while ((object = [e nextObject]) != nil)
    {
      [object setValue: value
		forKey: key];
    }
}

- (id) valueForKey: (NSString*)key
{
  NSEnumerator *e = [self objectEnumerator];
  id object;
  NSMutableSet *results = [NSMutableSet setWithCapacity: [self count]];

  while ((object = [e nextObject]) != nil)
    {
      id result = [object valueForKey: key];

      if (result == nil)
        {
          continue;
        }

      [results addObject: result];
    }
  return results;
}

// Comparing Sets
- (BOOL) isEqualToOrderedSet: (NSOrderedSet *)aSet
{
  if ([self count] == 0 &&
      [aSet count] == 0)
    return YES;

  if (self == aSet)
    return YES;

  if ([self count] != [aSet count])
    return NO;

  // if they are equal, then this set will be a subset of aSet.
  return [self isSubsetOfOrderedSet: aSet];
}

- (BOOL) isEqual: (id)other
{
  if ([other isKindOfClass: [NSOrderedSet class]])
    {
      return [self isEqualToOrderedSet: other];
    }

  return NO;
}

// Set operations
- (BOOL) intersectsOrderedSet: (NSOrderedSet *)otherSet
{
  id	o;
  NSEnumerator *e;

  // -1. If this set is empty, this method should return NO.
  if ([self count] == 0)
    return NO;

  // 0. Loop for all members in otherSet
  e = [otherSet objectEnumerator];
  while ((o = [e nextObject])) // 1. pick a member from otherSet.
    {
      if ([self containsObject: o])    // 2. check the member is in this set(self).
        return YES;
    }
  return NO;
}

- (BOOL) intersectsSet: (NSSet *)otherSet
{
  id	o = nil;
  NSEnumerator *e = nil;

  // -1. If this set is empty, this method should return NO.
  if ([self count] == 0)
    return NO;

  // 0. Loop for all members in otherSet
  e = [otherSet objectEnumerator];
  while ((o = [e nextObject])) // 1. pick a member from otherSet.
    {
      if ([self containsObject: o])    // 2. check the member is in this set(self).
        return YES;
    }
  return NO;
}

- (BOOL) isSubsetOfOrderedSet: (NSOrderedSet *)otherSet
{
  id    so, oo;
  NSEnumerator *selfEnum = [self objectEnumerator];
  NSEnumerator *otherEnum = [otherSet objectEnumerator];
  NSUInteger l = [self count];

  // -1. If this set is empty, this method should return YES.
  if (l == 0)
    {
      return YES;
    }

  // If count of set is more than otherSet it's not a subset
  if (l > [otherSet count])
    {
      return NO;
    }

  so = [selfEnum nextObject]; // get first object in enum...
  while ((oo = [otherEnum nextObject]) != nil)
    {
      if ([oo isEqual: so])  // if object is equal advance
	{
	  so = [selfEnum nextObject];
	  if (so == nil)
	    {
	      return YES; // if we are done with iterating self, then it's a subset.
	    }
	}
    }

  return NO;
}

- (BOOL) isSubsetOfSet: (NSSet *)otherSet
{
  id	o, e;
  NSUInteger l = [self count];

  // -1. If this set is empty, this method should return YES.
  if (l == 0)
    {
      return YES;
    }

  // If count of set is more than otherSet it's not a subset
  if (l > [otherSet count])
    {
      return NO;
    }

  // 0. Loop for all members in self
  e = [self objectEnumerator];
  while ((o = [e nextObject])) // 1. pick a member from self.
    {
      if ([otherSet containsObject: o] == NO)    // 2. check the member is in otherset.
        {
          return NO;
        }
    }
  return YES; // if all members are in set.
}

// Creating a Sorted Array
- (NSArray *) sortedArrayUsingDescriptors: (NSArray *)sortDescriptors
{
  return [[self array] sortedArrayUsingDescriptors: sortDescriptors];
}

- (NSArray *) sortedArrayUsingComparator: (NSComparator)comparator
{
  return [self sortedArrayWithOptions: 0
		      usingComparator: comparator];
}

- (NSArray *)
    sortedArrayWithOptions: (NSSortOptions)options
           usingComparator: (NSComparator)comparator
{
  return [[self array] sortedArrayWithOptions: options
			      usingComparator: comparator];
}

// Filtering Ordered Sets
- (NSOrderedSet *) filteredOrderedSetUsingPredicate: (NSPredicate *)predicate
{
  NSMutableOrderedSet   *result = nil;
  NSEnumerator		*e = [self objectEnumerator];
  id			object = nil;

  result = [NSMutableOrderedSet orderedSetWithCapacity: [self count]];
  while ((object = [e nextObject]) != nil)
    {
      if ([predicate evaluateWithObject: object] == YES)
        {
          [result addObject: object];  // passes filter
        }
    }
  return GS_IMMUTABLE(result);
}

// Describing a set
- (NSString *) description
{
  return [self descriptionWithLocale: nil];
}

- (NSString *) descriptionWithLocale: (NSLocale *)locale
{
  return [self descriptionWithLocale: locale indent: NO];
}

- (NSString*) descriptionWithLocale: (NSLocale *)locale indent: (BOOL)flag
{
  return [[self array] descriptionWithLocale: locale
                                      indent: flag];
}

- (NSArray *) array
{
  NSMutableArray *result = [NSMutableArray arrayWithCapacity: [self count]];
  id<NSFastEnumeration> enumerator = self;

  FOR_IN(id, o, enumerator)
    [result addObject: o];
  END_FOR_IN(enumerator)

  return GS_IMMUTABLE(result);
}

- (NSSet *) set
{
  NSMutableSet *result = [NSMutableSet setWithCapacity: [self count]];
  id<NSFastEnumeration> enumerator = self;

  FOR_IN(id, o, enumerator)
    [result addObject: o];
  END_FOR_IN(enumerator)

  return GS_IMMUTABLE(result);
}
@end

// Mutable Ordered Set
@implementation NSMutableOrderedSet
// Creating a Mutable Ordered Set
+ (void) initialize
{
  if (self == [NSMutableOrderedSet class])
    {
      NSMutableOrderedSet_abstract_class = self;
      NSMutableOrderedSet_concrete_class = [GSMutableOrderedSet class];
    }
}

+ (id) allocWithZone: (NSZone*)z
{
  if (self == NSMutableOrderedSet_abstract_class)
    {
      return NSAllocateObject(NSMutableOrderedSet_concrete_class, 0, z);
    }
  else
    {
      return NSAllocateObject(self, 0, z);
    }
}

+ (instancetype) orderedSetWithCapacity: (NSUInteger)capacity
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()] initWithCapacity: capacity]);
}

- (Class) classForCoder
{
  return NSMutableOrderedSet_abstract_class;
}

- (instancetype) initWithCapacity: (NSUInteger)capacity
{
  self = [self init];
  return self;
}

- (instancetype) init
{
  self = [super init];
  if (self == nil)
    {
      NSLog(@"Could not init class");
    }
  return self;
}

- (void) addObject: (id)anObject
{
  [self insertObject: anObject atIndex: [self count]];
}

- (void) addObjects: (const id[])objects count: (NSUInteger)count
{
  NSUInteger i = 0;
  for (i = 0; i < count; i++)
    {
      id obj = objects[i];
      [self addObject: obj];
    }
}

- (void) addObjectsFromArray: (NSArray *)otherArray
{
  NSEnumerator *en = [otherArray objectEnumerator];
  id obj = nil;
  while ((obj = [en nextObject]) != nil)
    {
      [self addObject: obj];
    }
}

- (void) insertObject: (id)object atIndex: (NSUInteger)index  // required override
{
  [self subclassResponsibility: _cmd];
}

- (void) setObject: (id)object atIndexedSubscript: (NSUInteger)index
{
  if ([self count] == index)
    {
      [self addObject: object];
    }
  else
    {
      [self replaceObjectAtIndex: index withObject: object];
    }
}

- (void) insertObjects: (NSArray *)array atIndexes: (NSIndexSet *)indexes
{
  NSUInteger	index = [indexes firstIndex];
  NSEnumerator	*enumerator = [array objectEnumerator];
  id		object = [enumerator nextObject];

  while (object != nil && index != NSNotFound)
    {
      [self insertObject: object atIndex: index];
      object = [enumerator nextObject];
      index = [indexes indexGreaterThanIndex: index];
    }
}

- (void) removeObject: (id)anObject
{
  NSUInteger index;

  if (anObject == nil)
    {
      NSWarnMLog(@"attempt to remove nil object");
      return;
    }

  index = [self indexOfObject: anObject];
  if (NSNotFound != index)
    {
      [self removeObjectAtIndex: index];
    }
}

- (void) removeObjectAtIndex: (NSUInteger)index  // required override
{
  [self subclassResponsibility: _cmd];
}

- (void) removeObjectsAtIndexes: (NSIndexSet *)indexes
{
  NSUInteger count = [indexes count];
  NSUInteger indexArray[count];

  [indexes getIndexes: indexArray
             maxCount: count
         inIndexRange: NULL];

  if (count > 0)
    {
      NSUInteger	i;
      IMP	rem = [self methodForSelector: remSel];
      
      for (i = 0; i < count; i++)
	{
	  NSUInteger idx = indexArray[i];
	  (*rem)(self, remSel, idx);
	}
    }
}

- (void) removeObjectsInArray: (NSArray *)otherArray
{
  NSUInteger	c = [otherArray count];

  if (c > 0)
    {
      NSUInteger	i;
      IMP	get = [otherArray methodForSelector: oaiSel];
      IMP	rem = [self methodForSelector: @selector(removeObject:)];

      for (i = 0; i < c; i++)
	(*rem)(self, @selector(removeObject:), (*get)(otherArray, oaiSel, i));
    }
}

- (void) removeObjectsInRange: (NSRange)aRange
{
  NSUInteger	i;
  NSUInteger	s = aRange.location;
  NSUInteger	c = [self count];

  i = aRange.location + aRange.length;

  if (c < i)
    {
      i = c;
    }

  if (i > s)
    {
      IMP	rem = [self methodForSelector: remSel];

      while (i-- > s)
	{
	  (*rem)(self, remSel, i);
	}
    }
}

- (void) removeAllObjects
{
  NSUInteger	c = [self count];

  if (c > 0)
    {
      while (c--)
	{
	  [self removeObjectAtIndex: c];
	}
    }
}

- (void) replaceObjectAtIndex: (NSUInteger)index
                  withObject: (id)object
{
  [self removeObjectAtIndex: index];
  [self insertObject: object atIndex: index];
}

- (void) replaceObjectsAtIndexes: (NSIndexSet *)indexes
                     withObjects: (NSArray *)objects
{
  NSUInteger count = [indexes count], i = 0;
  NSUInteger indexArray[count];

  // Remove the objects
  [self removeObjectsAtIndexes: indexes];

  // Get the indexes
  [indexes getIndexes: indexArray
	     maxCount: count
	 inIndexRange: NULL];

  // Iterate over the indexes and replace the objs
  for (i = 0; i < count; i++)
    {
      id obj = [objects objectAtIndex: i];
      NSUInteger indx = indexArray[i];
      [self insertObject: obj atIndex: indx];
    }
}

- (void) replaceObjectsInRange: (NSRange)aRange
                   withObjects: (const id[])objects
                         count: (NSUInteger)count
{
  id o = nil;
  NSUInteger i = count;

  [self removeObjectsInRange: aRange];

  while (i-- > 0)
    {
      o = objects[i];
      [self insertObject: o atIndex: aRange.location];
    }
}

- (void) setObject: (id)anObject atIndex: (NSUInteger)anIndex
{
  if ([self count] == anIndex)
    {
      [self addObject: anObject];
    }
  else
    {
      [self replaceObjectAtIndex: anIndex withObject: anObject];
    }
}

- (void) moveObjectsAtIndexes: (NSIndexSet *)indexes toIndex: (NSUInteger)index
{
  NSUInteger count = [indexes count];
  NSUInteger i = count;
  NSUInteger indexArray[count];
  NSMutableArray *tmpArray = [NSMutableArray arrayWithCapacity: count];
  id o = nil;
  NSEnumerator *e = nil;

  [indexes getIndexes: indexArray
             maxCount: count
         inIndexRange: NULL];

  // Build the temporary array....
  while (i-- > 0)
    {
      NSUInteger index = indexArray[i];
      id obj = [self objectAtIndex: index];
      [tmpArray addObject: obj];
    }

  // Remove the originals...
  for (i = 0; i < count; i++)
    {
      NSUInteger index = indexArray[i];
      [self removeObjectAtIndex: index];
    }

  // Move the objects
  e = [tmpArray objectEnumerator];
  while ((o = [e nextObject]) != nil)
    {
      [self insertObject: o atIndex: index];
    }
}

- (void) exchangeObjectAtIndex: (NSUInteger)index
	     withObjectAtIndex: (NSUInteger)otherIndex
{
  NSUInteger count = [self count];
  if (index >= count)
    {
      [self _raiseRangeExceptionWithIndex: index from: _cmd];
    }
  if (otherIndex >= count)
    {
      [self _raiseRangeExceptionWithIndex: otherIndex from: _cmd];
    }
  if (index != otherIndex)
    {
      NSUInteger low, high;
      id obj1, obj2;

      if (index > otherIndex)
        {
          high = index;
          low = otherIndex;
        }
      else
        {
          high = otherIndex;
          low = index;
        }

      obj1 = [self objectAtIndex: low];
      obj2 = [self objectAtIndex: high];
      [self removeObjectAtIndex: high];
      [self removeObjectAtIndex: low];
      [self insertObject: obj2 atIndex: low];
      [self insertObject: obj1 atIndex: high];
    }
}

- (void) filterUsingPredicate: (NSPredicate *)predicate
{
  unsigned	count = [self count];

  while (count-- > 0)
    {
      id	object = [self objectAtIndex: count];

      if ([predicate evaluateWithObject: object] == NO)
        {
          [self removeObjectAtIndex: count];
        }
    }
}

- (void) sortUsingDescriptors: (NSArray *)descriptors
{
  NSArray *result = [[self array] sortedArrayUsingDescriptors: descriptors];
  [self removeAllObjects];
  [self addObjectsFromArray: result];
}

- (void) sortUsingComparator: (NSComparator)comparator
{
  [self sortWithOptions: 0 usingComparator: comparator];
}

- (void) sortWithOptions: (NSSortOptions)options
         usingComparator: (NSComparator)comparator
{
  NSUInteger count = [self count];

  [self sortRange: NSMakeRange(0, count)
          options: options
        usingComparator: comparator];
}

- (void) sortRange: (NSRange)range
           options: (NSSortOptions)options
   usingComparator: (NSComparator)comparator
{
  NSUInteger count = range.length;

  if ((1 < count) && (NULL != comparator))
    {
      GS_BEGINIDBUF(objects, count);

      [self getObjects: objects range: range];

      if (options & NSSortStable)
        {
          if (options & NSSortConcurrent)
            {
              GSSortStableConcurrent(objects, NSMakeRange(0, count),
                (id)comparator, GSComparisonTypeComparatorBlock, NULL);
            }
          else
            {
              GSSortStable(objects, NSMakeRange(0, count),
                (id)comparator, GSComparisonTypeComparatorBlock, NULL);
            }
        }
      else
        {
          if (options & NSSortConcurrent)
            {
              GSSortUnstableConcurrent(objects, NSMakeRange(0, count),
                (id)comparator, GSComparisonTypeComparatorBlock, NULL);
            }
          else
            {
              GSSortUnstable(objects, NSMakeRange(0, count),
                (id)comparator, GSComparisonTypeComparatorBlock, NULL);
            }
        }
      [self replaceObjectsInRange: range withObjects: objects count: count];

      GS_ENDIDBUF();
    }
}

- (void) intersectOrderedSet: (NSOrderedSet *)other
{
  if (other != self)
    {
      id keys = [self objectEnumerator];
      id key;

      while ((key = [keys nextObject]))
	{
	  if ([other containsObject: key] == NO)
	    {
	      [self removeObject: key];
	    }
	}
    }
}

- (void) intersectSet: (NSSet *)other
{
  id keys = [self objectEnumerator];
  id key;

  while ((key = [keys nextObject]))
    {
      if ([other containsObject: key] == NO)
	{
	  [self removeObject: key];
	}
    }
}

- (void) minusOrderedSet: (NSOrderedSet *)other
{
  if (other == self)
    {
      [self removeAllObjects];
    }
  else
    {
      id keys = [other objectEnumerator];
      id key;

      while ((key = [keys nextObject]))
	{
	  [self removeObject: key];
	}
    }
}

- (void) minusSet: (NSSet *)other
{
  id keys = [other objectEnumerator];
  id key;

  while ((key = [keys nextObject]))
    {
      [self removeObject: key];
    }
}

- (void) unionOrderedSet: (NSOrderedSet *)other
{
    if (other != self)
    {
      id keys = [other objectEnumerator];
      id key;

      while ((key = [keys nextObject]))
	{
	  [self addObject: key];
	}
    }
}

- (void) unionSet: (NSSet *)other
{
  id keys = [other objectEnumerator];
  id key;

  while ((key = [keys nextObject]))
    {
      [self addObject: key];
    }
}

// Needed only to satisfy compiler that NSCoding is implemented.
- (instancetype) initWithCoder: (NSCoder *)coder
{
  return [super initWithCoder: coder];
}
@end
