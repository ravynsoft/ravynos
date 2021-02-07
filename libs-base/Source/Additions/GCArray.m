/* Implementation of garbage collecting array classes.

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

*/

#import "common.h"
#ifndef NeXT_Foundation_LIBRARY
#import "Foundation/NSException.h"
#import "Foundation/NSRange.h"
#endif

#import "GNUstepBase/GSObjCRuntime.h"
#import "GNUstepBase/GCObject.h"

@implementation GCArray

static Class	gcClass = 0;

+ (void) initialize
{
  if (gcClass == 0)
    {
      gcClass = [GCObject class];
      GSObjCAddClassBehavior(self, gcClass);
    }
}

- (Class) classForCoder
{
  return [GCArray class];
}

- (id) copyWithZone: (NSZone*)zone
{
  GCArray *result;
  id *objects;
  NSUInteger i, c = [self count];

  if (NSShouldRetainWithZone(self, zone))
    {
      return [self retain];
    }

  objects = NSZoneMalloc(zone, c * sizeof(id));
  /* FIXME: Check if malloc return 0 */
  [self getObjects: objects];
  for (i = 0; i < c; i++)
    {
      objects[i] = [objects[i] copy];
    }
  result = [[GCArray allocWithZone: zone] initWithObjects: objects count: c];
  NSZoneFree(zone, objects);

  return result;
}

- (NSUInteger) count
{
  return _count;
}

- (void) dealloc
{
  NSUInteger	c = _count;

  [GCObject gcObjectWillBeDeallocated: (GCObject*)self];
  if ([GCObject gcIsCollecting])
    {
      while (c-- > 0)
	{
	  if (_isGCObject[c] == NO)
	    {
	      DESTROY(_contents[c]);
	    }
	}
    }
  else
    {
      while (c-- > 0)
	{
	  DESTROY(_contents[c]);
	}
    }

  NSZoneFree([self zone], _contents);
  [super dealloc];
}

- (void) gcDecrementRefCountOfContainedObjects
{
  NSUInteger	c = _count;

  gc.flags.visited = 0;
  while (c-- > 0)
    {
      if (_isGCObject[c])
	{
	  [_contents[c] gcDecrementRefCount];
	}
    }
}

- (BOOL) gcIncrementRefCountOfContainedObjects
{
  if (gc.flags.visited == 1)
    {
      return NO;
    }
  else
    {
      NSUInteger	c = _count;

      gc.flags.visited = 1;
      while (c-- > 0)
	{
	  if (_isGCObject[c])
	    {
	      [_contents[c] gcIncrementRefCount];
	      [_contents[c] gcIncrementRefCountOfContainedObjects];
	    }
	}
      return YES;
    }
}

- (id) initWithObjects: (const id[])objects count: (NSUInteger)count
{
  _contents = NSZoneMalloc([self zone], count * (sizeof(id) + sizeof(BOOL)));
  _isGCObject = (BOOL*)&_contents[count];
  _count = 0;
  while (_count < count)
    {
      _contents[_count] = RETAIN(objects[_count]);
      if (_contents[_count] == nil)
	{
	  DESTROY(self);
	  [NSException raise: NSInvalidArgumentException
		      format: @"Nil object to be added in array"];
	}
      else
	{
	  _isGCObject[_count] = [objects[_count] isKindOfClass: gcClass];
	}
      _count++;
    }
  return self;
}

- (id) initWithArray: (NSArray*)anotherArray
{
  NSUInteger	count = [anotherArray count];

  _contents = NSZoneMalloc([self zone], count * (sizeof(id) + sizeof(BOOL)));
  _isGCObject = (BOOL*)&_contents[count];
  _count = 0;
  while (_count < count)
    {
      _contents[_count] = RETAIN([anotherArray objectAtIndex: _count]);
      _isGCObject[_count] = [_contents[_count] isKindOfClass: gcClass];
      _count++;
    }
  return self;
}

/**
 * We use the same initial instance variable layout as a GCObject and
 * ue the <em>behavior</em> mechanism to inherit methods from that class
 * to implement a form of multiple inheritance.  We need to implement
 * this method to make this apparent at runtime.
 */
- (BOOL) isKindOfClass: (Class)c
{
  if (c == gcClass)
    {
      return YES;
    }
  return [super isKindOfClass: c];
}

- (id) mutableCopyWithZone: (NSZone*)zone
{
    return [[GCMutableArray allocWithZone: zone] initWithArray: self];
}

- (id) objectAtIndex: (NSUInteger)index
{
  if (index >= _count)
    {
      [NSException raise: NSRangeException
		  format: @"[%@-%@]: index: %"PRIuPTR,
	NSStringFromClass([self class]), NSStringFromSelector(_cmd), index];
    }
  return _contents[index];
}

@end



@implementation GCMutableArray

+ (void)initialize
{
  static BOOL beenHere = NO;

  if (beenHere == NO)
    {
      beenHere = YES;
      GSObjCAddClassBehavior(self, [GCArray class]);
    }
}

- (void) addObject: (id)anObject
{
  [self insertObject: anObject atIndex: _count];
}

- (Class) classForCoder
{
  return [GCMutableArray class];
}

- (id) copyWithZone: (NSZone*)zone
{
  GCArray *result;
  id *objects;
  NSUInteger i, c = [self count];

  objects = NSZoneMalloc(zone, c * sizeof(id));
  /* FIXME: Check if malloc return 0 */
  [self getObjects: objects];
  for (i = 0; i < c; i++)
    {
      objects[i] = [objects[i] copy];
    }
  result = [[GCArray allocWithZone: zone] initWithObjects: objects count: c];
  NSZoneFree(zone, objects);

  return result;
}

- (id) init
{
  return [self initWithCapacity: 1];
}

- (id) initWithArray: (NSArray*)anotherArray
{
  NSUInteger	count = [anotherArray count];

  self = [self initWithCapacity: count];
  if (self != nil)
    {
      while (_count < count)
	{
	  _contents[_count] = RETAIN([anotherArray objectAtIndex: _count]);
	  _isGCObject[_count] = [_contents[_count] isKindOfClass: gcClass];
	  _count++;
	}
    }
  return self;
}

- (id) initWithCapacity: (NSUInteger)aNumItems
{
  if (aNumItems < 1)
    {
      aNumItems = 1;
    }
  _contents = NSZoneMalloc([self zone],
    aNumItems * (sizeof(id) + sizeof(BOOL)));
  _isGCObject = (BOOL*)&_contents[aNumItems];
  _maxCount = aNumItems;
  _count = 0;
  return self;
}

- (id) initWithObjects: (const id [])objects count: (NSUInteger)count
{
  self = [self initWithCapacity: count];
  if (self != nil)
    {
      while (_count < count)
	{
	  _contents[_count] = RETAIN(objects[_count]);
	  if (_contents[_count] == nil)
	    {
	      DESTROY(self);
	      [NSException raise: NSInvalidArgumentException
			  format: @"Nil object to be added in array"];
	    }
	  else
	    {
	      _isGCObject[_count] = [objects[_count] isKindOfClass: gcClass];
	    }
	  _count++;
	}
    }
  return self;
}

- (void) insertObject: (id)anObject atIndex: (NSUInteger)index
{
  NSUInteger i;

  if (anObject == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"[%@-%@]: nil argument",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  if (index > _count)
    {
      [NSException raise: NSRangeException
		  format: @"[%@-%@]: bad index %"PRIuPTR,
	NSStringFromClass([self class]), NSStringFromSelector(_cmd), index];
    }

  if (_count == _maxCount)
    {
      NSUInteger	old = _maxCount;
      BOOL	*optr;

      if (_maxCount > 0)
	{
	  _maxCount += (_maxCount >> 1) ? (_maxCount >> 1) : 1;
	}
      else
	{
	  _maxCount = 1;
	}
      _contents = (id*)NSZoneRealloc([self zone], _contents,
	_maxCount * (sizeof(id) + sizeof(BOOL)));
      optr = (BOOL*)&_contents[old];
      _isGCObject = (BOOL*)&_contents[_maxCount];
      memmove(_isGCObject, optr, sizeof(BOOL)*old);
    }
  for (i = _count; i > index; i--)
    {
      _contents[i] = _contents[i - 1];
      _isGCObject[i] = _isGCObject[i - 1];
    }
  _contents[index] = RETAIN(anObject);
  _isGCObject[index] = [anObject isKindOfClass: gcClass];
  _count++;
}

- (id) mutableCopyWithZone: (NSZone*)zone
{
    return [[GCMutableArray allocWithZone: zone] initWithArray: self];
}

- (void) removeAllObjects
{
  [self removeObjectsInRange: NSMakeRange(0, _count)];
}

- (void) removeObjectAtIndex: (NSUInteger)index
{
  [self removeObjectsInRange: NSMakeRange(index, 1)];
}

- (void) removeObjectsInRange: (NSRange)range
{
  NSUInteger	i;

  if (NSMaxRange(range) > _count)
    {
      [NSException raise: NSRangeException
		  format: @"[%@-%@]: bad range %@",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd),
	NSStringFromRange(range)];
    }
  if (range.length == 0)
    {
      return;
    }
  for (i = range.location; i < NSMaxRange(range); i++)
    {
      RELEASE(_contents[i]);
    }
  for (i = NSMaxRange(range); i < _count; i++, range.location++)
    {
      _contents[range.location] = _contents[i];
      _isGCObject[range.location] = _isGCObject[i];
    }
  _count -= range.length;
}

- (void) replaceObjectAtIndex: (NSUInteger)index  withObject: (id)anObject
{
  if (anObject == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"[%@-%@]: nil argument",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  if (index >= _count)
    {
      [NSException raise: NSRangeException
		  format: @"[%@-%@]: bad index %"PRIuPTR,
	NSStringFromClass([self class]), NSStringFromSelector(_cmd), index];
    }
  ASSIGN(_contents[index], anObject);
  _isGCObject[index] = [anObject isKindOfClass: gcClass];
}

@end

