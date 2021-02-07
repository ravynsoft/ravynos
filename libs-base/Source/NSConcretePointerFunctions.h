/**Interface for NSConcretePointerFunctions for GNUStep
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

#import	"Foundation/NSPointerFunctions.h"

#ifdef __GNUSTEP_RUNTIME__
#  include <objc/capabilities.h>
#endif

#if defined(OBJC_CAP_ARC)
#    include <objc/objc-arc.h>
#    define ARC_WEAK_READ(x) objc_loadWeak((id*)x)
#    define ARC_WEAK_WRITE(addr, x) objc_storeWeak((id*)addr, (id)x)
#    define WEAK_READ(x) (*x)
#    define WEAK_WRITE(addr, x) (*(addr) =  x)
#    define STRONG_WRITE(addr, x) objc_storeStrong((id*)addr, (id)x)
#    define STRONG_ACQUIRE(x) objc_retain(x)
#else
#  define WEAK_READ(x) (*x)
#  define WEAK_WRITE(addr, x) (*(addr) =  x)
#  define STRONG_WRITE(addr, x) ASSIGN(*((id*)addr), ((id)x))
#  define STRONG_ACQUIRE(x) RETAIN(((id)x))
#endif
#ifndef ARC_WEAK_WRITE
#  define ARC_WEAK_WRITE(addr, x) WEAK_WRITE(addr, x)
#endif
#ifndef ARC_WEAK_READ
#  define ARC_WEAK_READ(x) WEAK_READ(x)
#endif


/* Declare a structure type to copy pointer functions information 
 * around easily.
 */
typedef struct
{
  void* (*acquireFunction)(const void *item,
    NSUInteger (*size)(const void *item), BOOL shouldCopy);

  NSString *(*descriptionFunction)(const void *item);

  NSUInteger (*hashFunction)(const void *item,
    NSUInteger (*size)(const void *item));

  BOOL (*isEqualFunction)(const void *item1, const void *item2,
    NSUInteger (*size)(const void *item));

  void (*relinquishFunction)(const void *item,
    NSUInteger (*size)(const void *item));

  NSUInteger (*sizeFunction)(const void *item);

  NSPointerFunctionsOptions	options;

} PFInfo;

inline static BOOL memoryType(int options, int flag)
{
  return (options & 0xff) == flag;
}

/* Declare the concrete pointer functions class as a wrapper around
 * an instance of the PFInfo structure.
 */
@interface NSConcretePointerFunctions : NSPointerFunctions
{
@public
  PFInfo	_x;
}
@end

/* Wrapper functions to make use of the pointer functions.
 */

/**
 * Reads the pointer from the specified address, inserting a read barrier if
 * required.
 */
static inline void *pointerFunctionsRead(PFInfo *PF, void **addr)
{
  if (memoryType(PF->options, NSPointerFunctionsWeakMemory))
    {
      return ARC_WEAK_READ((id*)addr);
    }
  if (memoryType(PF->options, NSPointerFunctionsZeroingWeakMemory))
    {
      return WEAK_READ((id*)addr);
    }
  return *addr;
}

/**
 * Assigns a pointer, inserting the correct write barrier if required.
 */
static inline void pointerFunctionsAssign(PFInfo *PF, void **addr, void *value)
{
  if (memoryType(PF->options, NSPointerFunctionsWeakMemory))
    {
      ARC_WEAK_WRITE(addr, value);
    }
  else if (memoryType(PF->options, NSPointerFunctionsZeroingWeakMemory))
    {
      WEAK_WRITE(addr, value);
    }
  else if (memoryType(PF->options, NSPointerFunctionsStrongMemory))
    {
      STRONG_WRITE(addr, value);
    }
  else
    {
      *addr = value;
    }
}

/* Acquire the pointer value to store for the specified item.
 */
static inline void *
pointerFunctionsAcquire(PFInfo *PF, void **dst, void *src)
{
  if (PF->acquireFunction != 0)
    src = (*PF->acquireFunction)(src, PF->sizeFunction,
    PF->options & NSPointerFunctionsCopyIn ? YES : NO);
  // FIXME: This shouldn't be here.  Acquire and assign are separate
  // operations.  Acquire is for copy-in operations (i.e. retain / copy),
  // assign is for move operations of already-owned pointers.  Combining them
  // like this is Just Plain Wrong™
  pointerFunctionsAssign(PF, dst, src);
  return src;
}


/**
 * Moves a pointer from location to another.
 */
static inline void pointerFunctionsMove(PFInfo *PF, void **new, void **old)
{
  pointerFunctionsAssign(PF, new, pointerFunctionsRead(PF, old));
}


/* Generate an NSString description of the item
 */
static inline NSString *
pointerFunctionsDescribe(PFInfo *PF, void *item)
{
  if (PF->descriptionFunction != 0)
    return (*PF->descriptionFunction)(item);
  return nil;
}


/* Generate the hash of the item
 */
static inline NSUInteger
pointerFunctionsHash(PFInfo *PF, void *item)
{
  if (PF->hashFunction != 0)
    return (*PF->hashFunction)(item, PF->sizeFunction);
  return (NSUInteger)(uintptr_t)item;
}


/* Compare two items for equality
 */
static inline BOOL
pointerFunctionsEqual(PFInfo *PF, void *item1, void *item2)
{
  if (PF->isEqualFunction != 0)
    return (*PF->isEqualFunction)(item1, item2, PF->sizeFunction);
  if (item1 == item2)
    return YES;
  return NO;
}


/* Relinquish the specified item and set it to zero.
 */
static inline void
pointerFunctionsRelinquish(PFInfo *PF, void **itemptr)
{
  
  if (PF->relinquishFunction != 0)
    (*PF->relinquishFunction)(*itemptr, PF->sizeFunction);
  if (memoryType(PF->options, NSPointerFunctionsWeakMemory))
    ARC_WEAK_WRITE(itemptr, 0);
  else if (memoryType(PF->options, NSPointerFunctionsZeroingWeakMemory))
    WEAK_WRITE(itemptr, (void*)0);
  else
    *itemptr = 0;
}


static inline void
pointerFunctionsReplace(PFInfo *PF, void **dst, void *src)
{
  if (src != *dst)
    {
      if (PF->acquireFunction != 0)
	src = (*PF->acquireFunction)(src, PF->sizeFunction,
          PF->options & NSPointerFunctionsCopyIn ? YES : NO);
      if (PF->relinquishFunction != 0)
	(*PF->relinquishFunction)(*dst, PF->sizeFunction);
      if (memoryType(PF->options, NSPointerFunctionsWeakMemory))
        ARC_WEAK_WRITE(dst, 0);
      else if (memoryType(PF->options, NSPointerFunctionsZeroingWeakMemory))
        WEAK_WRITE(dst, (void*)0);
      else
	*dst = src;
    }
}
