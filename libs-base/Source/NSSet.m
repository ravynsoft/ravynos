/** NSSet - Set object to store key/value pairs
   Copyright (C) 1995, 1996, 1998 Free Software Foundation, Inc.

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

   <title>NSSet class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSSet.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSKeyValueCoding.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSException.h"
// For private method _decodeArrayOfObjectsForKey:
#import "Foundation/NSKeyedArchiver.h"
#import "GSPrivate.h"
#import "GSFastEnumeration.h"
#import "GSDispatch.h"

@class	GSSet;
@interface GSSet : NSObject	// Help the compiler
@end
@class	GSMutableSet;
@interface GSMutableSet : NSObject	// Help the compiler
@end

/**
 *  <code>NSSet</code> maintains an unordered collection of unique objects
 *  (according to [NSObject-isEqual:]).  When a duplicate object is added
 *  to the set, it replaces its old copy.
 */
@implementation NSSet

static Class NSSet_abstract_class;
static Class NSMutableSet_abstract_class;
static Class NSSet_concrete_class;
static Class NSMutableSet_concrete_class;

+ (id) allocWithZone: (NSZone*)z
{
  if (self == NSSet_abstract_class)
    {
      return NSAllocateObject(NSSet_concrete_class, 0, z);
    }
  else
    {
      return NSAllocateObject(self, 0, z);
    }
}

+ (void) initialize
{
  if (self == [NSSet class])
    {
      NSSet_abstract_class = self;
      NSSet_concrete_class = [GSSet class];
      [NSMutableSet class];
    }
}

/**
 *  New autoreleased empty set.
 */
+ (id) set
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()] init]);
}

/**
 *  New set containing (unique elements of) objects.
 */
+ (id) setWithArray: (NSArray*)objects
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithArray: objects]);
}

/**
 *  New set containing single object anObject.
 */
+ (id) setWithObject: anObject
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithObjects: &anObject count: 1]);
}

/**
 *  New set containing (unique elements of) objects.
 */
+ (id) setWithObjects: (const id[])objects
	        count: (NSUInteger)count
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithObjects: objects count: count]);
}

/**
 *  New set with objects in given nil-terminated list.
 */
+ (id) setWithObjects: firstObject, ...
{
  id	set;

  GS_USEIDLIST(firstObject,
    set = [[self allocWithZone: NSDefaultMallocZone()]
      initWithObjects: __objects count: __count]);
  return AUTORELEASE(set);
}

/**
 *  Copy constructor.
 */
+ (id) setWithSet: (NSSet*)aSet
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithSet: aSet]);
}

- (Class) classForCoder
{
  return NSSet_abstract_class;
}

/**
 * Returns a new copy of the receiver.<br />
 * The default abstract implementation of a copy is to use the
 * -initWithSet:copyItems: method with the flag set to YES.<br />
 * Concrete subclasses generally simply retain and return the receiver.
 */
- (id) copyWithZone: (NSZone*)z
{
  NSSet	*copy = [NSSet_concrete_class allocWithZone: z];

  return [copy initWithSet: self copyItems: YES];
}

/**
 * Returns the number of objects stored in the set.
 */
- (NSUInteger) count
{
  [self subclassResponsibility: _cmd];
  return 0;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      /* HACK ... MacOS-X seems to code differently if the coder is an
       * actual instance of NSKeyedArchiver
       */
      if ([aCoder class] == [NSKeyedArchiver class])
	{
	  [(NSKeyedArchiver*)aCoder _encodeArrayOfObjects: [self allObjects]
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
      NSEnumerator     *e = [self objectEnumerator];
      id		o;

      [aCoder encodeValueOfObjCType: @encode(unsigned) at: &count];
      while ((o = [e nextObject]) != nil)
	{
	  [aCoder encodeValueOfObjCType: @encode(id) at: &o];
	}
    }
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  Class		c;

  c = object_getClass(self);
  if (c == NSSet_abstract_class)
    {
      DESTROY(self);
      self = [NSSet_concrete_class allocWithZone: NSDefaultMallocZone()];
      return [self initWithCoder: aCoder];
    }
  else if (c == NSMutableSet_abstract_class)
    {
      DESTROY(self);
      self = [NSMutableSet_concrete_class allocWithZone: NSDefaultMallocZone()];
      return [self initWithCoder: aCoder];
    }

  if ([aCoder allowsKeyedCoding])
    {
      id	array;

      array = [(NSKeyedUnarchiver*)aCoder _decodeArrayOfObjectsForKey:
						@"NS.objects"];
      if (array == nil)
	{
	  unsigned	i = 0;
	  NSString	*key;
	  id		val;

	  array = [NSMutableArray arrayWithCapacity: 2];
	  key = [NSString stringWithFormat: @"NS.object.%u", i];
	  val = [(NSKeyedUnarchiver*)aCoder decodeObjectForKey: key];

	  while (val != nil)
	    {
	      [array addObject: val];
	      i++;
	      key = [NSString stringWithFormat: @"NS.object.%u", i];
	      val = [(NSKeyedUnarchiver*)aCoder decodeObjectForKey: key];
	    }
	}
      self = [self initWithArray: array];
    }
  else
    {
      unsigned	count;

      [aCoder decodeValueOfObjCType: @encode(unsigned) at: &count];
      if (count > 0)
        {
	  unsigned	i;
	  GS_BEGINIDBUF(objs, count);

	  for (i = 0; i < count; i++)
	    {
	      [aCoder decodeValueOfObjCType: @encode(id) at: &objs[i]];
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

/**
 * <p>In MacOS-X class clusters do not have designated initialisers,
 * and there is a general rule that -init is treated as the designated
 * initialiser of the class cluster, but that other intitialisers
 * may not work s expected an would need to be individually overridden
 * in any subclass.
 * </p>
 * <p>GNUstep tries to make it easier to subclass a class cluster,
 * by making class clusters follow the same convention as normal
 * classes, so the designated initialiser is the <em>richest</em>
 * initialiser.  This means that all other initialisers call the
 * documented designated initialiser (which calls -init only for
 * MacOS-X compatibility), and anyone writing a subclass only needs
 * to override that one initialiser in order to have all the other
 * ones work.
 * </p>
 * <p>For MacOS-X compatibility, you may also need to override various
 * other initialisers.  Exactly which ones, you will need to determine
 * by trial on a MacOS-X system ... and may vary between releases of
 * MacOS-X.  So to be safe, on MacOS-X you probably need to re-implement
 * <em>all</em> the class cluster initialisers you might use in conjunction
 * with your subclass.
 * </p>
 */
- (id) init
{
  self = [super init];
  return self;
}

/** <init /> <override-subclass />
 * Initialize to contain (unique elements of) objects.<br />
 * Calls -init (which does nothing but maintain MacOS-X compatibility),
 * and needs to be re-implemented in subclasses in order to have all
 * other initialisers work.
 */
- (id) initWithObjects: (const id[])objects
		 count: (NSUInteger)count
{
  self = [self init];
  return self;
}

/**
 *  If anObject is in set, return it (the copy in the set).
 */
- (id) member: (id)anObject
{
  return [self subclassResponsibility: _cmd];
  return 0;
}

/**
 * Returns a new instance containing the same objects as
 * the receiver.<br />
 * The default implementation does this by calling the
 * -initWithSet:copyItems: method on a newly created object,
 * and passing it NO to tell it just to retain the items.
 */
- (id) mutableCopyWithZone: (NSZone*)z
{
  NSMutableSet	*copy = [NSMutableSet_concrete_class allocWithZone: z];

  return [copy initWithSet: self copyItems: NO];
}

/**
 *  Return enumerator over objects in set.  Order is undefined.
 */
- (NSEnumerator*) objectEnumerator
{
  return [self subclassResponsibility: _cmd];
}

/**
 *  Initialize with (unique elements of) objects in given nil-terminated list.
 */
- (id) initWithObjects: firstObject, ...
{
  GS_USEIDLIST(firstObject,
    self = [self initWithObjects: __objects count: __count]);
  return self;
}

/**
 * Initialises a newly allocated set by adding all the objects
 * in the supplied array to the set.
 */
- (id) initWithArray: (NSArray*)other
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
}

/**
 * Initialises a newly allocated set by adding all the objects
 * in the supplied set.
 */
- (id) initWithSet: (NSSet*)other copyItems: (BOOL)flag
{
  unsigned	c = [other count];
  id		o, e = [other objectEnumerator];
  unsigned	i = 0;
  GS_BEGINIDBUF(os, c);

  while ((o = [e nextObject]))
    {
      if (flag)
	os[i] = [o copy];
      else
	os[i] = o;
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

/**
 *  Initialize with same items as other (items not copied).
 */
- (id) initWithSet: (NSSet*)other
{
  return [self initWithSet: other copyItems: NO];
}

/**
 *  Return array of all objects in set.  Order is undefined.
 */
- (NSArray*) allObjects
{
  id		e = [self objectEnumerator];
  unsigned	i;
  unsigned	c = [self count];
  NSArray	*result = nil;
  GS_BEGINIDBUF(k, c);

  for (i = 0; i < c; i++)
    {
      k[i] = [e nextObject];
    }
  return AUTORELEASE([[NSArray allocWithZone: NSDefaultMallocZone()]
    initWithObjects: k count: c]);
  GS_ENDIDBUF();
  return result;
}

/**
 *  Return an arbitrary object from set, or nil if this is empty set.
 */
- (id) anyObject
{
  if ([self count] == 0)
    return nil;
  else
    {
      id e = [self objectEnumerator];
      return [e nextObject];
    }
}

/**
 *  Return whether set contains an object equal to this one according
 *  to [NSObject-isEqual:].
 */
- (BOOL) containsObject: (id)anObject
{
  return (([self member: anObject]) ? YES : NO);
}

- (NSUInteger) hash
{
  return [self count];
}

/**
 *  Send each object given message (with no arguments).
 *  Identical to [-makeObjectsPerformSelector:].
 */
- (void) makeObjectsPerform: (SEL)aSelector
{
  id	o, e = [self objectEnumerator];

  while ((o = [e nextObject]))
    [o performSelector: aSelector];
}

/**
 *  Send each object given message (with no arguments).
 *  Identical to [-makeObjectsPerform:].
 */
- (void) makeObjectsPerformSelector: (SEL)aSelector
{
  id	o, e = [self objectEnumerator];

  while ((o = [e nextObject]))
    [o performSelector: aSelector];
}

/**
 *  Send each object given message with given argument.
 *  Identical to [-makeObjectsPerform:withObject:].
 */
- (void) makeObjectsPerformSelector: (SEL)aSelector withObject: argument
{
  id	o, e = [self objectEnumerator];

  while ((o = [e nextObject]))
    [o performSelector: aSelector withObject: argument];
}

/**
 *  Send each object given message with given argument.
 *  Identical to [-makeObjectsPerformSelector:withObject:].
 */
- (void) makeObjectsPerform: (SEL)aSelector withObject: argument
{
  id	o, e = [self objectEnumerator];

  while ((o = [e nextObject]))
    [o performSelector: aSelector withObject: argument];
}

/**
 *  Return whether set intersection with otherSet is non-empty.
 */
- (BOOL) intersectsSet: (NSSet*) otherSet
{
  id	o = nil, e = nil;

  // -1. If this set is empty, this method should return NO.
  if ([self count] == 0)
    return NO;

  // 0. Loop for all members in otherSet
  e = [otherSet objectEnumerator];
  while ((o = [e nextObject])) // 1. pick a member from otherSet.
    {
      if ([self member: o])    // 2. check the member is in this set(self).
        return YES;
    }
  return NO;
}

/**
 *  Return whether subset of otherSet.
 */
- (BOOL) isSubsetOfSet: (NSSet*) otherSet
{
  id o = nil, e = nil;

  // -1. members of this set(self) <= that of otherSet
  if ([self count] > [otherSet count])
    return NO;

  // 0. Loop for all members in this set(self).
  e = [self objectEnumerator];
  while ((o = [e nextObject]))
    {
      // 1. check the member is in the otherSet.
      if ([otherSet member: o])
       {
         // 1.1 if true -> continue, try to check the next member.
         continue ;
       }
      else
       {
         // 1.2 if false -> return NO;
         return NO;
       }
    }
  // 2. return YES; all members in this set are also in the otherSet.
  return YES;
}

- (BOOL) isEqual: (id)other
{
  if ([other isKindOfClass: [NSSet class]])
    return [self isEqualToSet: other];
  return NO;
}

- (NSUInteger)_countForObject: (id)object
{
  return 1;
}

/**
 *  Return whether each set is subset of the other.
 */
- (BOOL) isEqualToSet: (NSSet*)other
{
  if ([self count] != [other count])
    return NO;
  else
    {
      id	o, e = [self objectEnumerator];

      while ((o = [e nextObject]))
        {
	  if (![other member: o])
            {
	      return NO;
            }
         else
           {
             if ([self _countForObject: o] != [other _countForObject: o])
               {
                 return NO;
               }
           }
        }
    }
  /* xxx Recheck this. */
  return YES;
}

/**
 *  Returns listing of objects in set.
 */
- (NSString*) description
{
  return [self descriptionWithLocale: nil];
}

/**
 *  Returns listing of objects in set.
 */
- (NSString*) descriptionWithLocale: (id)locale
{
  return [[self allObjects] descriptionWithLocale: locale];
}

- (id) valueForKey: (NSString*)key
{
  NSEnumerator *e = [self objectEnumerator];
  id object = nil;
  NSMutableSet *results = [NSMutableSet setWithCapacity: [self count]];

  while ((object = [e nextObject]) != nil)
    {
      id result = [object valueForKey: key];

      if (result == nil)
        continue;

      [results addObject: result];
    }
  return results;
}

- (id) valueForKeyPath: (NSString*)path
{
  id result = (id) nil;

  if ([path hasPrefix: @"@"])
    {
      NSRange   r;

      r = [path rangeOfString: @"."];
      if (r.length == 0)
        {
          if ([path isEqualToString: @"@count"] == YES)
            {
              result = [NSNumber numberWithUnsignedInteger: [self count]];
            }
          else
            {
              result = [self valueForKey: path];
            }
        }
      else
        {
          NSString      *op = [path substringToIndex: r.location];
          NSString      *rem = [path substringFromIndex: NSMaxRange(r)];
          unsigned      count = [self count];

          if ([op isEqualToString: @"@count"] == YES)
            {
              result = [NSNumber numberWithUnsignedInteger: count];
            }
          else if ([op isEqualToString: @"@avg"] == YES)
            {
              double        d = 0;

              if (count > 0)
                {
                  NSEnumerator  *e = [self objectEnumerator];
                  id            o;

                  while ((o = [e nextObject]) != nil)
                    {
                      d += [[o valueForKeyPath: rem] doubleValue];
                    }
                  d /= count;
                }
              result = [NSNumber numberWithDouble: d];
            }
          else if ([op isEqualToString: @"@max"] == YES)
            {
              if (count > 0)
                {
                  NSEnumerator  *e = [self objectEnumerator];
                  id            o;

                  while ((o = [e nextObject]) != nil)
                    {
                      o = [o valueForKeyPath: rem];
                      if (result == nil
                        || [result compare: o] == NSOrderedAscending)
                        {
                          result = o;
                        }
                    }
                }
            }
          else if ([op isEqualToString: @"@min"] == YES)
            {
              if (count > 0)
                {
                  NSEnumerator  *e = [self objectEnumerator];
                  id            o;

                  while ((o = [e nextObject]) != nil)
                    {
                      o = [o valueForKeyPath: rem];
                      if (result == nil
                        || [result compare: o] == NSOrderedDescending)
                        {
                          result = o;
                        }
                    }
                }
            }
          else if ([op isEqualToString: @"@sum"] == YES)
            {
              double        d = 0;

              if (count > 0)
                {
                  NSEnumerator  *e = [self objectEnumerator];
                  id            o;

                  while ((o = [e nextObject]) != nil)
                    {
                      d += [[o valueForKeyPath: rem] doubleValue];
                    }
                }
              result = [NSNumber numberWithDouble: d];
            }
          else if ([op isEqualToString: @"@distinctUnionOfArrays"] == YES)
            {
              if (count > 0)
                {
                  NSEnumerator  *e = [self objectEnumerator];
                  id            o;

                  result = [NSMutableSet set];
                  while ((o = [e nextObject]) != nil)
                    {
                      o = [o valueForKeyPath: rem];
                      [result addObjectsFromArray: o];
                    }
                  result = [result allObjects];
                }
              else
                {
                  result = [NSArray array];
                }
            }
          else if ([op isEqualToString: @"@distinctUnionOfObjects"] == YES)
            {
              if (count > 0)
                {
                  NSEnumerator  *e = [self objectEnumerator];
                  id            o;

                  result = [NSMutableSet set];
                  while ((o = [e nextObject]) != nil)
                    {
                      o = [o valueForKeyPath: rem];
                      [result addObject: o];
                    }
                  result = [result allObjects];
                }
              else
                {
                  result = [NSArray array];
                }
            }
          else if ([op isEqualToString: @"@distinctUnionOfSets"] == YES)
            {
              if (count > 0)
                {
                  NSEnumerator  *e = [self objectEnumerator];
                  id            o;

                  result = [NSMutableSet set];
                  while ((o = [e nextObject]) != nil)
                    {
                      o = [o valueForKeyPath: rem];
                      [result addObjectsFromArray: [o allObjects]];
                    }
                  result = [result allObjects];
                }
              else
                {
                  result = [NSArray array];
                }
            }
          else if ([op isEqualToString: @"@unionOfArrays"] == YES)
            {
              if (count > 0)
                {
                  NSEnumerator  *e = [self objectEnumerator];
                  id            o;

                  result = [GSMutableArray array];
                  while ((o = [e nextObject]) != nil)
                    {
                      o = [o valueForKeyPath: rem];
                      [result addObjectsFromArray: o];
                    }
                  result = GS_IMMUTABLE(result);
                }
              else
                {
                  result = [NSArray array];
                }
            }
          else if ([op isEqualToString: @"@unionOfObjects"] == YES)
            {
              if (count > 0)
                {
                  NSEnumerator  *e = [self objectEnumerator];
                  id            o;

                  result = [GSMutableArray array];
                  while ((o = [e nextObject]) != nil)
                    {
                      o = [o valueForKeyPath: rem];
                      [result addObject: o];
                    }
                  result = GS_IMMUTABLE(result);
                }
              else
                {
                  result = [NSArray array];
                }
            }
          else if ([op isEqualToString: @"@unionOfSets"] == YES)
            {
              if (count > 0)
                {
                  NSEnumerator  *e = [self objectEnumerator];
                  id            o;

                  result = [GSMutableArray array];
                  while ((o = [e nextObject]) != nil)
                    {
                      o = [o valueForKeyPath: rem];
                      [result addObjectsFromArray: [o allObjects]];
                    }
                  result = GS_IMMUTABLE(result);
                }
              else
                {
                  result = [NSArray array];
                }
            }
          else
            {
              result = [super valueForKeyPath: path];
            }
        }
    }
  else
    {
      result = [super valueForKeyPath: path];
    }

  return result;
}

- (void) enumerateObjectsUsingBlock: (GSSetEnumeratorBlock)aBlock
{
  [self enumerateObjectsWithOptions: 0 usingBlock: aBlock];
}

- (void) enumerateObjectsWithOptions: (NSEnumerationOptions)opts
                          usingBlock: (GSSetEnumeratorBlock)aBlock
{
  BLOCK_SCOPE BOOL shouldStop = NO;
  id<NSFastEnumeration> enumerator = self;

  GS_DISPATCH_CREATE_QUEUE_AND_GROUP_FOR_ENUMERATION(enumQueue, opts)
  FOR_IN (id, obj, enumerator)
  {
    GS_DISPATCH_SUBMIT_BLOCK(enumQueueGroup,enumQueue, if (shouldStop == NO) {, }, aBlock, obj, &shouldStop);
    if (shouldStop)
      {
	break;
      }
  }
  END_FOR_IN(enumerator)
  GS_DISPATCH_TEARDOWN_QUEUE_AND_GROUP_FOR_ENUMERATION(enumQueue, opts)
}

- (NSSet *) objectsPassingTest: (GSSetFilterBlock)aBlock
{
  return [self objectsWithOptions: 0 passingTest: aBlock];
}

- (NSSet *) objectsWithOptions: (NSEnumerationOptions)opts
                   passingTest: (GSSetFilterBlock)aBlock
{
  BOOL                  shouldStop = NO;
  id<NSFastEnumeration> enumerator = self;
  NSMutableSet          *resultSet;

  resultSet = [NSMutableSet setWithCapacity: [self count]];
    
  FOR_IN (id, obj, enumerator)
    {
      BOOL include = CALL_BLOCK(aBlock, obj, &shouldStop);

      if (include)
        {
          [resultSet addObject:obj];
        }
      if (shouldStop)
        {
          break;
        }
    }
  END_FOR_IN(enumerator)
    
  return GS_IMMUTABLE(resultSet);
}

/** Return a set formed by adding anObject to the receiver.
 */
- (NSSet *) setByAddingObject: (id)anObject
{
  NSMutableSet  *m;
  NSSet         *s;

  m = [self mutableCopy];
  [m addObject: anObject];
  s = [m copy];
  [m release];
  return [s autorelease];
}

/** Return a set formed by adding the contents of other to the receiver.
 */
- (NSSet *) setByAddingObjectsFromArray: (NSArray *)other
{
  NSMutableSet  *m;
  NSSet         *s;

  m = [self mutableCopy];
  [m addObjectsFromArray: other];
  s = [m copy];
  [m release];
  return [s autorelease];
}

/** Return a set formed as a union of the receiver and other.
 */
- (NSSet *) setByAddingObjectsFromSet: (NSSet *)other
{
  NSMutableSet  *m;
  NSSet         *s;

  m = [self mutableCopy];
  [m unionSet: other];
  s = [m copy];
  [m release];
  return [s autorelease];
}

- (NSUInteger) countByEnumeratingWithState: (NSFastEnumerationState*)state
                                   objects: (id*)stackbuf
                                     count: (NSUInteger)len
{
    [self subclassResponsibility: _cmd];
    return 0;
}

@end


/**
 *  Mutable version of [NSSet].
 */
@implementation NSMutableSet

+ (void) initialize
{
  if (self == [NSMutableSet class])
    {
      NSMutableSet_abstract_class = self;
      NSMutableSet_concrete_class = [GSMutableSet class];
    }
}

/**
 *  New autoreleased instance with given capacity.
 */
+ (id) setWithCapacity: (NSUInteger)numItems
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithCapacity: numItems]);
}

+ (id) allocWithZone: (NSZone*)z
{
  if (self == NSMutableSet_abstract_class)
    {
      return NSAllocateObject(NSMutableSet_concrete_class, 0, z);
    }
  else
    {
      return NSAllocateObject(self, 0, z);
    }
}

- (Class) classForCoder
{
  return NSMutableSet_abstract_class;
}

/** <init /> <override-subclass />
 * Initialises a newly allocated set to contain no objects but
 * to have space available to hold the specified number of items.<br />
 * Additions of items to a set initialised
 * with an appropriate capacity will be more efficient than addition
 * of items otherwise.<br />
 * Calls -init (which does nothing but maintain MacOS-X compatibility),
 * and needs to be re-implemented in subclasses in order to have all
 * other initialisers work.
 */
- (id) initWithCapacity: (NSUInteger)numItems
{
  self = [self init];
  return self;
}

/**
 * Adds anObject to the set.<br />
 * The object is retained by the set.
 */
- (void) addObject: (id)anObject
{
  [self subclassResponsibility: _cmd];
}

/**
 * Removes the anObject from the receiver.
 */
- (void) removeObject: (id)anObject
{
  [self subclassResponsibility: _cmd];
}

- (id) initWithObjects: (const id[])objects
		 count: (NSUInteger)count
{
  self = [self initWithCapacity: count];
  if (self != nil)
    {
      while (count--)
	{
	  [self addObject: objects[count]];
	}
    }
  return self;
}

/**
 * Adds all the objects in the array to the receiver.
 */
- (void) addObjectsFromArray: (NSArray*)array
{
  unsigned	i, c = [array count];

  for (i = 0; i < c; i++)
    {
      [self addObject: [array objectAtIndex: i]];
    }
}

/**
 * Removes from the receiver all the objects it contains
 * which are not also in other.
 */
- (void) intersectSet: (NSSet*) other
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

/**
 * Removes from the receiver all the objects that are in
 * other.
 */
- (void) minusSet: (NSSet*) other
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

/**
 * Removes all objects from the receiver.
 */
- (void) removeAllObjects
{
  [self subclassResponsibility: _cmd];
}

/**
 * Removes all objects from the receiver then adds the
 * objects from other.  If the receiver <em>is</em>
 * other, the method has no effect.
 */
- (void) setSet: (NSSet*)other
{
  if (other == self)
    {
      return;
    }
  if (other == nil)
    {
      NSWarnMLog(@"Setting mutable set to nil");
      [self removeAllObjects];
    }
  else
    {
      IF_NO_GC([other retain];)	// In case it's held by us
      [self removeAllObjects];
      [self unionSet: other];
      RELEASE(other);
    }
}

/**

 * Adds all the objects from other to the receiver.
 */
- (void) unionSet: (NSSet*) other
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

@end
