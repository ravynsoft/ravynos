/* Implementation for NSSortDescriptor for GNUStep
   Copyright (C) 2005-2011 Free Software Foundation, Inc.

   Written by:  Saso Kiselkov <diablos@manga.sk>
   Date: 2005

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

#define	EXPOSE_NSSortDescriptor_IVARS	1
#import "Foundation/NSSortDescriptor.h"

#import "Foundation/NSCoder.h"
#import "Foundation/NSException.h"
#import "Foundation/NSKeyValueCoding.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSUserDefaults.h"

#import "GNUstepBase/GSObjCRuntime.h"
#import "GSPrivate.h"
#import "GSSorting.h"

static BOOL     initialized = NO;

#ifdef  __clang__
#pragma clang diagnostic ignored "-Wreceiver-forward-class"
#endif

@interface GSTimSortPlaceHolder : NSObject
+ (void) setUnstable;
@end
@interface GSQuickSortPlaceHolder : NSObject
+ (void) setUnstable;
@end
@interface GSShellSortPlaceHolder : NSObject
+ (void) setUnstable;
@end

@implementation NSSortDescriptor

+ (void) defaultsChanged: (NSNotification*)n
{
  NSUserDefaults        *defs = (NSUserDefaults*)[n object];
  NSString              *algorithm;

  algorithm = [defs stringForKey: @"GSSortAlgorithm"];
  if ([algorithm isEqual: @"QuickSort"])
    {
      [GSQuickSortPlaceHolder setUnstable];
    }
  else if ([algorithm isEqual: @"ShellSort"])
    {
      [GSShellSortPlaceHolder setUnstable];
    }
  else if ([algorithm isEqual: @"TimSort"])
    {
      [GSTimSortPlaceHolder setUnstable];
    }
  else
    {
      [GSTimSortPlaceHolder setUnstable];
      if (nil != algorithm)
        {
          NSLog(@"GSSortAlgorithm default unknown value (%@)", algorithm);
        }
    }
}

+ (void) initialize
{
  if (NO == initialized)
    {
      NSNotificationCenter      *nc;
      NSUserDefaults            *defs;

      [GSTimSortPlaceHolder class];     // default stable sort
      nc = [NSNotificationCenter defaultCenter];
      defs = [NSUserDefaults standardUserDefaults];
      [nc addObserver: self
             selector: @selector(defaultsChanged:)
                 name: NSUserDefaultsDidChangeNotification
               object: defs];
      [self defaultsChanged: nil];      // set unstable sort
      initialized = YES;
    }
}

- (BOOL) ascending
{
  return _ascending;
}

- (NSComparisonResult) compareObject: (id) object1 toObject: (id) object2
{
  NSComparisonResult result;
  id comparedKey1 = [object1 valueForKeyPath: _key];
  id comparedKey2 = [object2 valueForKeyPath: _key];

  if (_comparator == NULL)
    {
      result = (NSComparisonResult) [comparedKey1 performSelector: _selector
                                                       withObject: comparedKey2];
    }
  else
    {
      result = CALL_BLOCK(((NSComparator)_comparator), comparedKey1, comparedKey2);
    }
  
  if (_ascending == NO)
    {
      if (result == NSOrderedAscending)
	{
	  result = NSOrderedDescending;
	}
      else if (result == NSOrderedDescending)
	{
	  result = NSOrderedAscending;
	}
    }

  return result;
}

- (id) copyWithZone: (NSZone*)zone
{
  NSSortDescriptor *copy = nil;
  if (NSShouldRetainWithZone(self, zone))
    {
      return RETAIN(self);
    }

  if (_comparator == NULL)
    {
      copy = [[NSSortDescriptor allocWithZone: zone]
               initWithKey: _key ascending: _ascending selector: _selector];
    }
  else
    {
      copy = [[NSSortDescriptor allocWithZone: zone]
               initWithKey: _key ascending: _ascending comparator: _comparator];
    }
  return copy;
}

- (void) dealloc
{
  TEST_RELEASE(_key);
  TEST_RELEASE(_comparator);
  [super dealloc];
}

- (NSUInteger) hash
{
  const char	*sel = sel_getName(_selector);

  return _ascending + GSPrivateHash(0, sel, strlen(sel)) + [_key hash];
}

+ (id) sortDescriptorWithKey: (NSString *)aKey ascending: (BOOL)ascending
{
  return AUTORELEASE([[self alloc] initWithKey: aKey ascending: ascending]);
}

+ (id) sortDescriptorWithKey: (NSString *)aKey 
                   ascending: (BOOL)ascending 
                    selector: (SEL)aSelector
{
  return AUTORELEASE([[self alloc] initWithKey: aKey 
                                     ascending: ascending 
                                      selector: aSelector]);
}

+ (id)sortDescriptorWithKey: (NSString *)key 
                  ascending: (BOOL)ascending 
                 comparator: (NSComparator)cmptr
{
  return AUTORELEASE([[self alloc] initWithKey: key
                                     ascending: ascending
                                    comparator: cmptr]);
  
}

- (id) initWithKey: (NSString *) key ascending: (BOOL) ascending
{
  return [self initWithKey: key ascending: ascending selector: NULL];
}

- (id) initWithKey: (NSString *) key
         ascending: (BOOL) ascending
        comparator: (NSComparator) cmptr
{
  if ([self init])
    {
      if (key == nil)
        {
          [NSException raise: NSInvalidArgumentException
                      format: @"%@", _(@"Passed nil key when initializing "
                                       @"an NSSortDescriptor.")];
        }
      if (cmptr == NULL)
        {
          [NSException raise: NSInvalidArgumentException
                      format: @"%@", _(@"Passed NULL comparator when initializing "
                                       @"an NSSortDescriptor.")];
        }

      ASSIGN(_key, key);
      _ascending = ascending;
      ASSIGN(_comparator, (id)cmptr);

      return self;
    }
  else
    {
      return nil;
    }
}

- (id) initWithKey: (NSString *) key
         ascending: (BOOL) ascending
          selector: (SEL) selector
{
  if ([self init])
    {
      if (key == nil)
        {
          [NSException raise: NSInvalidArgumentException
                      format: @"%@", _(@"Passed nil key when initializing "
            @"an NSSortDescriptor.")];
        }
      if (selector == NULL)
        {
          selector = @selector(compare:);
        }

      ASSIGN(_key, key);
      _ascending = ascending;
      _selector = selector;

      return self;
    }
  else
    {
      return nil;
    }
}

- (BOOL) isEqual: (id)other
{
  if (other == self)
    {
      return YES;
    }
  if ([other isKindOfClass: [NSSortDescriptor class]] == NO)
    {
      return NO;
    }
  if (((NSSortDescriptor*)other)->_ascending != _ascending)
    {
      return NO;
    }
  if (!sel_isEqual(((NSSortDescriptor*)other)->_selector, _selector))
    {
      return NO;
    }
  return [((NSSortDescriptor*)other)->_key isEqualToString: _key];
}

- (NSString *) key
{
  return _key;
}

- (id) reversedSortDescriptor
{
  return AUTORELEASE([[NSSortDescriptor alloc]
    initWithKey: _key ascending: !_ascending selector: _selector]);
}

- (SEL) selector
{
  return _selector;
}

- (void) encodeWithCoder: (NSCoder *) coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: _key forKey: @"NSKey"];
      [coder encodeBool: _ascending forKey: @"NSAscending"];
      [coder encodeObject: NSStringFromSelector(_selector)
                   forKey: @"NSSelector"];
    }
  else
    {
      [coder encodeObject: _key];
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_ascending];
      [coder encodeValueOfObjCType: @encode(SEL) at: &_selector];
    }
}

- (id) initWithCoder: (NSCoder *)decoder
{
  if ((self = [super init]) != nil)
    {
      if ([decoder allowsKeyedCoding])
        {
          ASSIGN(_key, [decoder decodeObjectForKey: @"NSKey"]);
          _ascending = [decoder decodeBoolForKey: @"NSAscending"];
          _selector = NSSelectorFromString([decoder
            decodeObjectForKey: @"NSSelector"]);
        }
      else
        {
          ASSIGN(_key, [decoder decodeObject]);
          [decoder decodeValueOfObjCType: @encode(BOOL) at: &_ascending];
          [decoder decodeValueOfObjCType: @encode(SEL) at: &_selector];
        }
    }
  return self;
}

@end


/* Symbols for the sorting functions, the actual algorithms fill these. */
void
(*_GSSortUnstable)(id* buffer, NSRange range,
  id comparisonEntity, GSComparisonType cmprType, void *context) = NULL;

void
(*_GSSortStable)(id* buffer, NSRange range,
  id comparisonEntity, GSComparisonType cmprType, void *context) = NULL;

void
(*_GSSortUnstableConcurrent)(id* buffer, NSRange range,
  id comparisonEntity, GSComparisonType cmprType, void *context) = NULL;

void
(*_GSSortStableConcurrent)(id* buffer, NSRange range,
  id comparisonEntity, GSComparisonType cmprType, void *context) = NULL;


// Sorting functions that select the adequate algorithms
void
GSSortUnstable(id* buffer, NSRange range, id descriptorOrComparator,
  GSComparisonType type, void* context)
{
  if (NO == initialized) [NSSortDescriptor class];
  if (NULL != _GSSortUnstable)
    {
      _GSSortUnstable(buffer, range, descriptorOrComparator, type, context);
    }
  else if (NULL != _GSSortStable)
    {
      _GSSortStable(buffer, range, descriptorOrComparator, type, context);
    }
  else
    {
      [NSException raise: @"NSInternalInconsistencyException" format:
        @"The GNUstep-base library was compiled without sorting support."];
    }
}

void
GSSortStable(id* buffer, NSRange range, id descriptorOrComparator,
  GSComparisonType type, void* context)
{
  if (NO == initialized) [NSSortDescriptor class];
  if (NULL != _GSSortStable)
    {
      _GSSortStable(buffer, range, descriptorOrComparator, type, context);
    }
  else
    {
      [NSException raise: @"NSInternalInconsistencyException" format:
        @"The GNUstep-base library was compiled without a"
        @" stable sorting algorithm."];
    }
}

void
GSSortStableConcurrent(id* buffer, NSRange range, id descriptorOrComparator,
  GSComparisonType type, void* context)
{
  if (NO == initialized) [NSSortDescriptor class];
  if (NULL != _GSSortStableConcurrent)
    {
      _GSSortStableConcurrent(buffer, range, descriptorOrComparator,
        type, context);
    }
  else
    {
      GSSortStable(buffer, range, descriptorOrComparator, type, context);
    }
}

void
GSSortUnstableConcurrent(id* buffer, NSRange range, id descriptorOrComparator,
  GSComparisonType type, void* context)
{
  if (NO == initialized) [NSSortDescriptor class];
  if (NULL != _GSSortUnstableConcurrent)
    {
      _GSSortUnstableConcurrent(buffer, range, descriptorOrComparator,
        type, context);
    }
  else if (NULL != _GSSortStableConcurrent)
    {
      _GSSortStableConcurrent(buffer, range, descriptorOrComparator,
        type, context);
    }
  else
    {
      GSSortUnstable(buffer, range, descriptorOrComparator, type, context);
    }
}



@implementation NSArray (NSSortDescriptorSorting)

- (NSArray *) sortedArrayUsingDescriptors: (NSArray *)sortDescriptors
{
  NSMutableArray *sortedArray = [GSMutableArray arrayWithArray: self];

  [sortedArray sortUsingDescriptors: sortDescriptors];

  return GS_IMMUTABLE(sortedArray);
}

@end

/* Sort the objects in range using the first descriptor and, if there
 * are more descriptors, recursively call the function to sort each range
 * of adhacent equal objects using the remaining descriptors.
 */
static void
SortRange(id *objects, NSRange range, id *descriptors,
  NSUInteger numDescriptors)
{
  NSSortDescriptor	*sd = (NSSortDescriptor*)descriptors[0];

  GSSortUnstable(objects, range, sd, GSComparisonTypeSortDescriptor, NULL);
  if (numDescriptors > 1)
    {
      NSUInteger	start = range.location;
      NSUInteger	finish = NSMaxRange(range);

      while (start < finish)
	{
	  NSUInteger	pos = start + 1;

	  /* Find next range of adjacent objects.
	   */
	  while (pos < finish
	    && [sd compareObject: objects[start]
	      toObject: objects[pos]] == NSOrderedSame)
	    {
	      pos++;
	    }

	  /* Sort the range using remaining descriptors.
	   */
	  if (pos - start > 1)
	    {
	      SortRange(objects, NSMakeRange(start, pos - start),
		descriptors + 1, numDescriptors - 1);
	    }
	  start = pos;
	}
    }
}

@implementation NSMutableArray (NSSortDescriptorSorting)

- (void) sortUsingDescriptors: (NSArray *)sortDescriptors
{
  NSUInteger	count = [self count];
  NSUInteger	numDescriptors = [sortDescriptors count];

  if (count > 1 && numDescriptors > 0)
    {
      id	descriptors[numDescriptors];
      NSArray	*a;
      GS_BEGINIDBUF(objects, count);

      [self getObjects: objects];
      if ([sortDescriptors isProxy])
	{
	  NSUInteger	i;

	  for (i = 0; i < numDescriptors; i++)
	    {
	      descriptors[i] = [sortDescriptors objectAtIndex: i];
	    }
	}
      else
	{
	  [sortDescriptors getObjects: descriptors];
	}
      SortRange(objects, NSMakeRange(0, count), descriptors, numDescriptors);
      a = [[NSArray alloc] initWithObjects: objects count: count];
      [self setArray: a];
      RELEASE(a);
      GS_ENDIDBUF();
    }
}

@end

@implementation GSMutableArray (NSSortDescriptorSorting)

- (void) sortUsingDescriptors: (NSArray *)sortDescriptors
{
  NSUInteger	dCount = [sortDescriptors count];

  if (_count > 1 && dCount > 0)
    {
      GS_BEGINIDBUF(descriptors, dCount);

      if ([sortDescriptors isProxy])
	{
	  NSUInteger	i;

	  for (i = 0; i < dCount; i++)
	    {
	      descriptors[i] = [sortDescriptors objectAtIndex: i];
	    }
	}
      else
	{
	  [sortDescriptors getObjects: descriptors];
	}
      SortRange(_contents_array, NSMakeRange(0, _count), descriptors, dCount);

      GS_ENDIDBUF();
    }
}

@end

@implementation NSSet (NSSortDescriptorSorting) 

- (NSArray *) sortedArrayUsingDescriptors: (NSArray *)sortDescriptors
{
	return [[self allObjects] sortedArrayUsingDescriptors: sortDescriptors];
}

@end

