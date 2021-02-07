/** Implementation for NSIndexPath for GNUStep
   Copyright (C) 2006 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Created: Feb 2006
   
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
#define	EXPOSE_NSIndexPath_IVARS	1
#import	"Foundation/NSByteOrder.h"
#import	"Foundation/NSData.h"
#import	"Foundation/NSException.h"
#import	"Foundation/NSHashTable.h"
#import	"Foundation/NSIndexPath.h"
#import	"Foundation/NSKeyedArchiver.h"
#import	"Foundation/NSLock.h"
#import	"GNUstepBase/GSLock.h"

static	NSRecursiveLock	*lock = nil;
static	NSHashTable	*shared = 0;
static	Class		myClass = 0;
static	NSIndexPath	*empty = nil;
static	NSIndexPath	*dummy = nil;

@implementation	NSIndexPath

+ (id) allocWithZone: (NSZone*)aZone
{
  if (self == myClass)
    {
      return empty;
    }
  return [super allocWithZone: aZone];
}

+ (id) indexPathWithIndex: (NSUInteger)anIndex
{
  return [self indexPathWithIndexes: &anIndex length: 1];
}

+ (id) indexPathWithIndexes: (NSUInteger*)indexes length: (NSUInteger)length
{
  id	o = [self allocWithZone: NSDefaultMallocZone()];

  o = [o initWithIndexes: indexes length: length];
  return AUTORELEASE(o);
}

+ (void) initialize
{
  if (empty == nil)
    {
      myClass = self;
      empty = (NSIndexPath*)NSAllocateObject(self, 0, NSDefaultMallocZone());
      [[NSObject leakAt: &empty] release];
      dummy = (NSIndexPath*)NSAllocateObject(self, 0, NSDefaultMallocZone());
      [[NSObject leakAt: &dummy] release];
      shared = NSCreateHashTable(NSNonRetainedObjectHashCallBacks, 1024);
      [[NSObject leakAt: &shared] release];
      NSHashInsert(shared, empty);
      lock = [NSRecursiveLock new];
      [[NSObject leakAt: &lock] release];
    }
}

- (NSComparisonResult) compare: (NSIndexPath*)other
{
  if (other != self)
    {
      NSUInteger	olength = other->_length;
      NSUInteger	*oindexes = other->_indexes;
      NSUInteger	end = (_length > olength) ? _length : olength;
      NSUInteger	pos;

      for (pos = 0; pos < end; pos++)
	{
	  if (pos >= _length)
	    {
	      return NSOrderedDescending;
	    }
	  else if (pos >= olength)
	    {
	      return NSOrderedAscending;
	    }
	  if (oindexes[pos] < _indexes[pos])
	    {
	      return NSOrderedDescending;
	    }
	  if (oindexes[pos] > _indexes[pos])
	    {
	      return NSOrderedAscending;
	    }
	}
      /*
       * Should never get here.
       */
      NSLog(@"Argh ... two identical index paths exist!");
    }
  return NSOrderedSame;
}

- (id) copyWithZone: (NSZone*)aZone
{
  return RETAIN(self);
}

- (void) dealloc
{
  if (self != empty)
    {
      [lock lock];
      if (shared != nil)
        {
          NSHashRemove(shared, self);
        }
      [lock unlock];
      if (_indexes != 0)
        {
          NSZoneFree(NSDefaultMallocZone(), _indexes);
        }
      [super dealloc];
    }
  GSNOSUPERDEALLOC;
}

- (NSString*) description
{
  NSMutableString	*m = [[super description] mutableCopy];
  NSUInteger		i;

  [m appendFormat: @"%"PRIuPTR" indexes [", _length];
  for (i = 0; i < _length; i++)
    {
      if (i > 0)
	{
	  [m appendString: @", "];
	}
      [m appendFormat: @"%"PRIuPTR, _indexes[i]];
    }
  [m appendString: @"]"];
  return AUTORELEASE(m);
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding] == YES)
    {
      [aCoder encodeInt: (NSInteger)_length forKey: @"NSIndexPathLength"];
      if (_length == 1)
	{
	  [aCoder encodeInt: (NSInteger)_indexes[0]
                     forKey: @"NSIndexPathValue"];
	}
      else if (_length > 1)
	{
	  NSMutableData	*m;
	  NSUInteger	*buf;
	  NSUInteger	i;

	  m = [NSMutableData new];
	  [m setLength: _length * sizeof(NSUInteger)];
	  buf = [m mutableBytes];
	  for (i = 0; i < _length; i++)
	    {
	      buf[i] = NSSwapHostIntToBig(_indexes[i]);
	    }
	  [aCoder encodeObject: m forKey: @"NSIndexPathData"];
	  RELEASE(m);
	}
    }
  else
    {
      [aCoder encodeValueOfObjCType: @encode(NSUInteger) at: &_length];
      if (_length > 0)
	{
	  [aCoder encodeArrayOfObjCType: @encode(NSUInteger)
				  count: _length
				     at: _indexes];
	}
    }
}

- (void) getIndexes: (NSUInteger*)aBuffer
{
  memcpy(aBuffer, _indexes, _length * sizeof(NSUInteger));
}

- (NSUInteger) hash
{
  return _hash;
}

- (NSUInteger) indexAtPosition: (NSUInteger)position
{
  if (position >= _length)
    {
      return NSNotFound;
    }
  return _indexes[position];
}

/**
 * Return path formed by adding the index to the receiver.
 */
- (NSIndexPath *) indexPathByAddingIndex: (NSUInteger)anIndex
{
  NSUInteger	buffer[_length + 1];

  [self getIndexes: buffer];
  buffer[_length] = anIndex;
  return [[self class] indexPathWithIndexes: buffer length: _length + 1];
}

- (NSIndexPath *) indexPathByRemovingLastIndex
{
  if (_length <= 1)
    {
      return empty;
    }
  else
    {
      return [[self class] indexPathWithIndexes: _indexes length: _length - 1];
    }
}
 
- (id) initWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding] == YES)
    {
      NSUInteger	length;
      NSUInteger	index;

      length = [aCoder decodeIntegerForKey: @"NSIndexPathLength"];
      if (length == 1)
	{
	  index = [aCoder decodeIntegerForKey: @"NSIndexPathValue"];
	  self = [self initWithIndex: index];
	}
      else if (length > 1)
	{
	  // FIXME ... not MacOS-X
	  NSMutableData	*d = [aCoder decodeObjectForKey: @"NSIndexPathData"];
	  NSUInteger	l = [d length];
	  NSUInteger	s = l / length;
	  NSUInteger	i;
          void          *src = [d mutableBytes];
          NSUInteger    *dst;

	  if (s == sizeof(NSUInteger))
	    {
              dst = (NSUInteger*)src;
            }
	  else
	    {
	      dst = (NSUInteger*)NSZoneMalloc(NSDefaultMallocZone(),
		length * sizeof(NSUInteger));
            }

          if (s == sizeof(long))
            {
              long	*ptr = (long*)src;

              for (i = 0; i < _length; i++)
                {
                  dst[i] = (NSUInteger)NSSwapBigLongToHost(ptr[i]);
                }
            }
          else if (s == sizeof(short))
            {
              short	*ptr = (short*)src;

              for (i = 0; i < _length; i++)
                {
                  dst[i] = (NSUInteger)NSSwapBigShortToHost(ptr[i]);
                }
            }
          else if (s == sizeof(long long))
            {
              long long	*ptr = (long long*)src;

              for (i = 0; i < _length; i++)
                {
                  dst[i] = (NSUInteger)NSSwapBigLongLongToHost(ptr[i]);
                }
            }
          else
            {
              if ((void*)dst != src)
                {
                  NSZoneFree(NSDefaultMallocZone(), dst);
                }
              [NSException raise: NSGenericException format:
                @"Unable to decode unsigned integers of size %"PRIuPTR, s];
            }
          self = [self initWithIndexes: dst length: length];
          if ((void*)dst != src)
            {
	      NSZoneFree(NSDefaultMallocZone(), dst);
	    }
	}
    }
  else
    {
      NSUInteger	length;

      [aCoder decodeValueOfObjCType: @encode(NSUInteger) at: &length];
      if (length == 0)
	{
	  ASSIGN(self, empty);
	}
      else
	{
	  NSUInteger	buf[16];
	  NSUInteger	*indexes = buf;

	  if (length > 16)
	    {
	      indexes = NSZoneMalloc(NSDefaultMallocZone(),
		length * sizeof(NSUInteger));
	    }
	  [aCoder decodeArrayOfObjCType: @encode(NSUInteger)
				  count: length
				     at: indexes];
	  self = [self initWithIndexes: indexes length: length];
	  if (indexes != buf)
	    {
	      NSZoneFree(NSDefaultMallocZone(), indexes);
	    }
	}
    }
  return self;
}

- (id) initWithIndex: (NSUInteger)anIndex
{
  return [self initWithIndexes: &anIndex length: 1];
}

/** <init />
 * Initialise the receiver to contain the specified indexes.<br />
 * May return an existing index path.
 */
- (id) initWithIndexes: (NSUInteger*)indexes length: (NSUInteger)length
{
  NSIndexPath	*found;
  NSUInteger	h = 0;
  NSUInteger	i;

  if (_length != 0)
    {
      [NSException raise: NSGenericException
		  format: @"Attempt to re-initialize NSIndexPath"];
    }
  // FIXME ... need better hash function?
  for (i = 0; i < length; i++)
    {
      h = (h << 5) ^ indexes[i];
    }

  [lock lock];
  dummy->_hash = h;
  dummy->_length = length;
  dummy->_indexes = indexes;
  found = NSHashGet(shared, dummy);
  if (found == nil)
    {
      if (self == empty)
	{
          RELEASE(self);
	  self = (NSIndexPath*)NSAllocateObject([self class],
	    0, NSDefaultMallocZone());
	}
      _hash = dummy->_hash;
      _length = dummy->_length;
      _indexes = NSZoneMalloc(NSDefaultMallocZone(),
	_length * sizeof(NSUInteger));
      memcpy(_indexes, dummy->_indexes, _length * sizeof(NSUInteger));
      NSHashInsert(shared, self);
    }
  else
    {
      ASSIGN(self, found);
    }
  dummy->_indexes = 0;  // Don't want static indexes deallocated atExit
  [lock unlock];
  return self;
}

- (BOOL) isEqual: (id)other
{
  if (other == self)
    {
      return YES;
    }
  if (other == nil || GSObjCIsKindOf(object_getClass(other), myClass) == NO)
    {
      return NO;
    }
  if (((NSIndexPath*)other)->_length != _length)
    {
      return NO;
    }
  else
    {
      NSUInteger	*oindexes = ((NSIndexPath*)other)->_indexes;
      NSUInteger	pos = _length;

      while (pos-- > 0)
	{
	  if (_indexes[pos] != oindexes[pos])
	    {
	      return NO;
	    }
	}
    }
  return YES;
}

- (NSUInteger) length
{
  return _length;
}

- (oneway void) release
{
  if (self != empty)
    {
      /* We lock the table while checking, to prevent
       * another thread from grabbing this object while we are
       * checking it.
       * If we are going to deallocate the object, we first remove
       * it from the table so that no other thread will find it
       * and try to use it while it is being deallocated.
       */
      [lock lock];
      if (NSDecrementExtraRefCountWasZero(self))
	{
	  [self dealloc];
	}
      [lock unlock];
    }
}

@end

