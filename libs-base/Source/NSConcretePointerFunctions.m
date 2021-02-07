/**Implementation for NSConcretePointerFunctions for GNUStep
   Copyright (C) 2009 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date:	2009
   
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
#import	"NSConcretePointerFunctions.h"

static void*
acquireMallocMemory(const void *item,
  NSUInteger (*size)(const void *item), BOOL shouldCopy)
{
  if (shouldCopy == YES)
    {
      NSUInteger	len = (*size)(item);
      void		*newItem = malloc(len);

      memcpy(newItem, item, len);
      item = newItem;
    }
  return (void*)item;
}

static void*
acquireRetainedObject(const void *item,
  NSUInteger (*size)(const void *item), BOOL shouldCopy)
{
  if (shouldCopy == YES)
    {
      return [(NSObject*)item copy];
    }
  return [(NSObject*)item retain];
}

static void*
acquireExistingMemory(const void *item,
  NSUInteger (*size)(const void *item), BOOL shouldCopy)
{
  return (void*)item;
}

static NSString*
describeString(const void *item)
{
  return [NSString stringWithFormat: @"%s", (char*)item];
}

static NSString*
describeInteger(const void *item)
{
  return [NSString stringWithFormat: @"%"PRIdPTR, (intptr_t)item];
}

static NSString*
describeObject(const void *item)
{
  return [(NSObject*)item description];
}

static NSString*
describePointer(const void *item)
{
  return [NSString stringWithFormat: @"%p", item];
}

static BOOL
equalDirect(const void *item1, const void *item2,
  NSUInteger (*size)(const void *item))
{
  return (item1 == item2) ? YES : NO;
}

static BOOL
equalObject(const void *item1, const void *item2,
  NSUInteger (*size)(const void *item))
{
  return [(NSObject*)item1 isEqual: (NSObject*)item2];
}

static BOOL
equalMemory(const void *item1, const void *item2,
  NSUInteger (*size)(const void *item))
{
  NSUInteger	s1 = (*size)(item1);
  NSUInteger	s2 = (*size)(item2);

  return (s1 == s2 && memcmp(item1, item2, s1) == 0) ? YES : NO;
}

static BOOL
equalString(const void *item1, const void *item2,
  NSUInteger (*size)(const void *item))
{
  return (strcmp((const char*)item1, (const char*)item2) == 0) ? YES : NO;
}

static NSUInteger
hashDirect(const void *item, NSUInteger (*size)(const void *item))
{
  return (NSUInteger)(uintptr_t)item;
}

static NSUInteger
hashObject(const void *item, NSUInteger (*size)(const void *item))
{
  return [(NSObject*)item hash];
}

static NSUInteger
hashMemory(const void *item, NSUInteger (*size)(const void *item))
{
  unsigned	len = (*size)(item);
  NSUInteger	hash = 0;

  while (len-- > 0)
    {
      hash = (hash << 5) + hash + *(const uint8_t*)item++;
    }
  return hash;
}

static NSUInteger
hashShifted(const void *item, NSUInteger (*size)(const void *item))
{
  return ((NSUInteger)(uintptr_t)item) >> 2;
}

static NSUInteger
hashString(const void *item, NSUInteger (*size)(const void *item))
{
  NSUInteger	hash = 0;

  while (*(const uint8_t*)item != 0)
    {
      hash = (hash << 5) + hash + *(const uint8_t*)item++;
    }
  return hash;
}

static void
relinquishMallocMemory(const void *item,
  NSUInteger (*size)(const void *item))
{
  free((void*)item);
}

static void
relinquishRetainedMemory(const void *item,
  NSUInteger (*size)(const void *item))
{
  [(NSObject*)item release];
}

@implementation NSConcretePointerFunctions

+ (id) allocWithZone: (NSZone*)zone
{
  return (id) NSAllocateObject(self, 0, zone);
}

- (id) copyWithZone: (NSZone*)zone
{
  return NSCopyObject(self, 0, zone);
}

- (id) initWithOptions: (NSPointerFunctionsOptions)options
{
  _x.options = options;

  /* First we look at the memory management options to see which function
   * should be used to relinquish contents of a container with these
   * options.
   */
  if (options & NSPointerFunctionsZeroingWeakMemory)
    {
      _x.relinquishFunction = 0;
    }
  else if (options & NSPointerFunctionsOpaqueMemory)
    {
      _x.relinquishFunction = 0;
    }
  else if (options & NSPointerFunctionsMallocMemory)
    {
      _x.relinquishFunction = relinquishMallocMemory;
    }
  else if (options & NSPointerFunctionsMachVirtualMemory)
    {
      _x.relinquishFunction = relinquishMallocMemory;
    }
  else
    {
      _x.relinquishFunction = relinquishRetainedMemory;
    }

  /* Now we look at the personality options to determine other functions.
   */
  if (options & NSPointerFunctionsOpaquePersonality)
    {
      _x.acquireFunction = acquireExistingMemory;
      _x.descriptionFunction = describePointer;
      _x.hashFunction = hashShifted;
      _x.isEqualFunction = equalDirect;
    }
  else if (options & NSPointerFunctionsObjectPointerPersonality)
    {
      if (options & NSPointerFunctionsZeroingWeakMemory)
	{
	  _x.acquireFunction = acquireExistingMemory;
	}
      else
	{
	  _x.acquireFunction = acquireRetainedObject;
	}
      _x.descriptionFunction = describeObject;
      _x.hashFunction = hashShifted;
      _x.isEqualFunction = equalDirect;
    }
  else if (options & NSPointerFunctionsCStringPersonality)
    {
      _x.acquireFunction = acquireMallocMemory;
      _x.descriptionFunction = describeString;
      _x.hashFunction = hashString;
      _x.isEqualFunction = equalString;
    }
  else if (options & NSPointerFunctionsStructPersonality)
    {
      _x.acquireFunction = acquireMallocMemory;
      _x.descriptionFunction = describePointer;
      _x.hashFunction = hashMemory;
      _x.isEqualFunction = equalMemory;
    }
  else if (options & NSPointerFunctionsIntegerPersonality)
    {
      _x.acquireFunction = acquireExistingMemory;
      _x.descriptionFunction = describeInteger;
      _x.hashFunction = hashDirect;
      _x.isEqualFunction = equalDirect;
    }
  else		/* objects */
    {
      if (options & NSPointerFunctionsZeroingWeakMemory)
	{
	  _x.acquireFunction = acquireExistingMemory;
	}
      else
	{
          _x.acquireFunction = acquireRetainedObject;
	}
      _x.descriptionFunction = describeObject;
      _x.hashFunction = hashObject;
      _x.isEqualFunction = equalObject;
    }


  return self;
}

- (void* (*)(const void *item,
  NSUInteger (*size)(const void *item), BOOL shouldCopy)) acquireFunction
{
  return _x.acquireFunction;
}

- (NSString *(*)(const void *item)) descriptionFunction
{
  return _x.descriptionFunction;
}

- (NSUInteger (*)(const void *item,
  NSUInteger (*size)(const void *item))) hashFunction
{
  return _x.hashFunction;
}

- (BOOL (*)(const void *item1, const void *item2,
  NSUInteger (*size)(const void *item))) isEqualFunction
{
  return _x.isEqualFunction;
}

- (void (*)(const void *item,
  NSUInteger (*size)(const void *item))) relinquishFunction
{
  return _x.relinquishFunction;
}

- (void) setAcquireFunction: (void* (*)(const void *item,
  NSUInteger (*size)(const void *item), BOOL shouldCopy))func
{
  _x.acquireFunction = func;
}

- (void) setDescriptionFunction: (NSString *(*)(const void *item))func
{
  _x.descriptionFunction = func;
}

- (void) setHashFunction: (NSUInteger (*)(const void *item,
  NSUInteger (*size)(const void *item)))func
{
  _x.hashFunction = func;
}

- (void) setIsEqualFunction: (BOOL (*)(const void *item1, const void *item2,
  NSUInteger (*size)(const void *item)))func
{
  _x.isEqualFunction = func;
}

- (void) setRelinquishFunction: (void (*)(const void *item,
  NSUInteger (*size)(const void *item))) func
{
  _x.relinquishFunction = func;
}

- (void) setSizeFunction: (NSUInteger (*)(const void *item))func
{
  _x.sizeFunction = func;
}

- (void) setUsesStrongWriteBarrier: (BOOL)flag
{
  _x.options &=
    ~(NSPointerFunctionsZeroingWeakMemory
    |NSPointerFunctionsOpaqueMemory
    |NSPointerFunctionsMallocMemory
    |NSPointerFunctionsMachVirtualMemory);
}

- (void) setUsesWeakReadAndWriteBarriers: (BOOL)flag
{
  _x.options |= NSPointerFunctionsZeroingWeakMemory;
  _x.options &=
    ~(NSPointerFunctionsOpaqueMemory
    |NSPointerFunctionsMallocMemory
    |NSPointerFunctionsMachVirtualMemory);
}

- (NSUInteger (*)(const void *item)) sizeFunction
{
  return _x.sizeFunction;
}

- (BOOL) usesStrongWriteBarrier
{
  if ((_x.options &
    (NSPointerFunctionsZeroingWeakMemory
    |NSPointerFunctionsOpaqueMemory
    |NSPointerFunctionsMallocMemory
    |NSPointerFunctionsMachVirtualMemory)) == NSPointerFunctionsStrongMemory)
    return YES;
  return NO;
}

- (BOOL) usesWeakReadAndWriteBarriers
{
  if (_x.options & NSPointerFunctionsZeroingWeakMemory)
    return YES;
  return NO;
}

@end

