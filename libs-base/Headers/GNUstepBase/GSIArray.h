/* A fast (Inline) array implementation without objc method overhead.
 * Copyright (C) 1998,1999  Free Software Foundation, Inc.
 * 
 * Author:	Richard Frith-Macdonald <richard@brainstorm.co.uk>
 * Created:	Nov 1998
 * 
 * This file is part of the GNUstep Base Library.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02111 USA. */

#import	<GNUstepBase/GSVersionMacros.h>

#if	OS_API_VERSION(GS_API_NONE,GS_API_LATEST)


#if	defined(GNUSTEP_BASE_INTERNAL)
#import "Foundation/NSObject.h"
#import "Foundation/NSException.h"
#import "Foundation/NSGarbageCollector.h"
#import "Foundation/NSZone.h"
#else
#import <Foundation/NSObject.h>
#import <Foundation/NSException.h>
#import <Foundation/NSGarbageCollector.h>
#import <Foundation/NSZone.h>
#endif

/* To turn assertions on, define GSI_ARRAY_CHECKS */
#define GSI_ARRAY_CHECKS 1

#if	defined(__cplusplus)
extern "C" {
#endif

/* To easily un-inline functions for debugging */
#ifndef GS_STATIC_INLINE
#define GS_STATIC_INLINE static inline
#endif


#ifdef	GSI_ARRAY_CHECKS
#define	GSI_ARRAY_CHECK NSCAssert(array->count <= array->cap && array->old <= array->cap, NSInternalInconsistencyException)
#else
#define	GSI_ARRAY_CHECK
#endif

/*
 *      NB. This file is intended for internal use by the GNUstep libraries
 *      and may change siugnificantly between releases.
 *      While it is unlikley to be removed from the distributiuon any time
 *      soon, its use by other software is not officially supported.
 *
 *      This file should be INCLUDED in files wanting to use the GSIArray
 *	functions - these are all declared inline for maximum performance.
 *
 *	The file including this one may predefine some macros to alter
 *	the behaviour (default macros assume the items are NSObjects
 *	that are to be retained in the array) ...
 *
 *	GSI_ARRAY_RETAIN()
 *		Macro to retain an array item
 *
 *	GSI_ARRAY_RELEASE()
 *		Macro to release the item.
 *
 *	The next two values can be defined in order to let us optimise
 *	even further when either retain or release operations are not needed.
 *
 *	GSI_ARRAY_NO_RELEASE
 *		Defined if no release operation is needed for an item
 *	GSI_ARRAY_NO_RETAIN
 *		Defined if no retain operation is needed for a an item
 *		 
 *	The value GSI_ARRAY_EXTRA may be defined as the type of an extra
 *	field produced in every array.  The name of this field is 'extra'.
 *
 *	The value GSI_ARRAY_TYPE may be defined as an additional type
 *	which must be valid as an array element.  Otherwise the types are
 *	controlled by the mechanism in GSUnion.h
 */


#ifdef	GSI_ARRAY_NO_RETAIN
#ifdef	GSI_ARRAY_RETAIN
#undef	GSI_ARRAY_RETAIN
#endif
#define	GSI_ARRAY_RETAIN(A, X)	
#else
#ifndef	GSI_ARRAY_RETAIN
#define	GSI_ARRAY_RETAIN(A, X)	[(X).obj retain]
#endif
#endif

#ifdef	GSI_ARRAY_NO_RELEASE
#ifdef	GSI_ARRAY_RELEASE
#undef	GSI_ARRAY_RELEASE
#endif
#define	GSI_ARRAY_RELEASE(A, X)	
#else
#ifndef	GSI_ARRAY_RELEASE
#define	GSI_ARRAY_RELEASE(A, X)	[(X).obj release]
#endif
#endif

/*
 *	If there is no bitmask defined to supply the types that
 *	may be stored in the array, default to none.
 */
#ifndef	GSI_ARRAY_TYPES
#define	GSI_ARRAY_TYPES	0
#endif

#ifndef GSIArrayItem

/*
 *	Set up the name of the union to store array elements.
 */
#ifdef	GSUNION
#undef	GSUNION
#endif
#define	GSUNION	GSIArrayItem

/*
 *	Set up the types that will be storable in the union.
 *	See 'GSUnion.h' for details.
 */
#ifdef	GSUNION_TYPES
#undef	GSUNION_TYPES
#endif
#define	GSUNION_TYPES	GSI_ARRAY_TYPES
#ifdef	GSUNION_EXTRA
#undef	GSUNION_EXTRA
#endif

/*
 * Override extra type used in array value
 */
#ifdef	GSI_ARRAY_TYPE
#define	GSUNION_EXTRA	GSI_ARRAY_TYPE
#endif

/*
 *	Generate the union typedef
 */
#if	defined(GNUSTEP_BASE_INTERNAL)
#include "GNUstepBase/GSUnion.h"
#else
#include <GNUstepBase/GSUnion.h>
#endif

#endif /* #ifndef GSIArrayItem */

struct	_GSIArray {
  GSIArrayItem	*ptr;
  unsigned	count;
  unsigned	cap;
  unsigned	old;
  NSZone	*zone;
#ifdef	GSI_ARRAY_EXTRA
  GSI_ARRAY_EXTRA	extra;
#endif
};
typedef	struct	_GSIArray	GSIArray_t;
typedef	struct	_GSIArray	*GSIArray;

GS_STATIC_INLINE unsigned
GSIArrayCapacity(GSIArray array)
{
  return array->cap;
}

GS_STATIC_INLINE unsigned
GSIArrayCount(GSIArray array)
{
  return array->count;
}

GS_STATIC_INLINE NSUInteger
GSIArraySize(GSIArray array)
{
  NSUInteger	size = sizeof(GSIArray);

  size += array->cap * sizeof(GSIArrayItem);
  return size;
}

GS_STATIC_INLINE void
GSIArrayGrow(GSIArray array)
{
  unsigned int	next;
  unsigned int	size;
  GSIArrayItem	*tmp;

  if (array->old == 0)
    {
      /*
       * Statically initialised buffer ... copy into new heap buffer.
       */
      array->old = array->cap / 2;
      if (array->old < 1)
	{
	  array->old = 1;
	  array->cap = 1;
	}
      next = array->cap + array->old;
      size = next*sizeof(GSIArrayItem);
      tmp = NSZoneMalloc(array->zone, size);
      memcpy(tmp, array->ptr, array->count * sizeof(GSIArrayItem));
    }
  else
    {
      next = array->cap + array->old;
      size = next*sizeof(GSIArrayItem);
      tmp = NSZoneRealloc(array->zone, array->ptr, size);
    }

  if (tmp == 0)
    {
      [NSException raise: NSMallocException
		  format: @"failed to grow GSIArray"];
    }
  array->ptr = tmp;
  array->old = array->cap;
  array->cap = next;
}

GS_STATIC_INLINE void
GSIArrayGrowTo(GSIArray array, unsigned next)
{
  unsigned int	size;
  GSIArrayItem	*tmp;

  if (next < array->count)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"attempt to shrink below count"];
    }
  size = next*sizeof(GSIArrayItem);
  if (array->old == 0)
    {
      /*
       * Statically initialised buffer ... copy into new heap buffer.
       */
      tmp = NSZoneMalloc(array->zone, size);
      memcpy(tmp, array->ptr, array->count * sizeof(GSIArrayItem));
    }
  else
    {
      tmp = NSZoneRealloc(array->zone, array->ptr, size);
    }

  if (tmp == 0)
    {
      [NSException raise: NSMallocException
		  format: @"failed to grow GSIArray"];
    }
  array->ptr = tmp;
  array->old = (array->cap > 0 ? array->cap : 1);
  array->cap = next;
}

GS_STATIC_INLINE void
GSIArrayInsertItem(GSIArray array, GSIArrayItem item, unsigned index)
{
  unsigned int	i;

  GSI_ARRAY_CHECK;
  GSI_ARRAY_RETAIN(array, item);
  if (array->count == array->cap)
    {
      GSIArrayGrow(array);
    }
  for (i = array->count++; i > index; i--)
    {
      array->ptr[i] = array->ptr[i-1];
    }
  array->ptr[i] = item;
  GSI_ARRAY_CHECK;
}

GS_STATIC_INLINE void
GSIArrayInsertItemNoRetain(GSIArray array, GSIArrayItem item, unsigned index)
{
  unsigned int	i;

  GSI_ARRAY_CHECK;
  if (array->count == array->cap)
    {
      GSIArrayGrow(array);
    }
  for (i = array->count++; i > index; i--)
    {
      array->ptr[i] = array->ptr[i-1];
    }
  array->ptr[i] = item;
  GSI_ARRAY_CHECK;
}

GS_STATIC_INLINE void
GSIArrayAddItem(GSIArray array, GSIArrayItem item)
{
  GSI_ARRAY_CHECK;
  GSI_ARRAY_RETAIN(array, item);
  if (array->count == array->cap)
    {
      GSIArrayGrow(array);
    }
  array->ptr[array->count++] = item;
  GSI_ARRAY_CHECK;
}

GS_STATIC_INLINE void
GSIArrayAddItemNoRetain(GSIArray array, GSIArrayItem item)
{
  GSI_ARRAY_CHECK;
  if (array->count == array->cap)
    {
      GSIArrayGrow(array);
    }
  array->ptr[array->count++] = item;
  GSI_ARRAY_CHECK;
}

/*
 *	The comparator function takes two items as arguments, the first is the
 *	item to be added, the second is the item already in the array.
 *      The function should return NSOrderedAscending if the item to be
 *      added is 'less than' the item in the array, NSOrderedDescending
 *      if it is greater, and NSOrderedSame if it is equal.
 */
GS_STATIC_INLINE unsigned
GSIArraySearch(GSIArray array, GSIArrayItem item, 
  NSComparisonResult (*sorter)(GSIArrayItem, GSIArrayItem))
{
  unsigned int	upper = array->count;
  unsigned int	lower = 0;
  unsigned int	index;

  /*
   *	Binary search for an item equal to the one to be inserted.
   *	Only for sorted array !
   */
  for (index = upper/2; upper != lower; index = (upper+lower)/2)
    {
      NSComparisonResult comparison;

      comparison = (*sorter)(item, (array->ptr[index]));
      if (comparison == NSOrderedAscending)
        {
          upper = index;
        }
      else if (comparison == NSOrderedDescending)
        {
          lower = index + 1;
        }
      else
        {
          break;
        }
    }
  return index;
}

GS_STATIC_INLINE unsigned
GSIArrayInsertionPosition(GSIArray array, GSIArrayItem item, 
  NSComparisonResult (*sorter)(GSIArrayItem, GSIArrayItem))
{
  unsigned int	index;

  index = GSIArraySearch(array,item,sorter);
  /*
   *	Now skip past any equal items so the insertion point is AFTER any
   *	items that are equal to the new one.
   */
  while (index < array->count
    && (*sorter)(item, (array->ptr[index])) != NSOrderedAscending)
    {
      index++;
    }
#ifdef	GSI_ARRAY_CHECKS
  NSCAssert(index <= array->count, NSInternalInconsistencyException);
#endif
  return index;
}

#ifdef	GSI_ARRAY_CHECKS
GS_STATIC_INLINE void
GSIArrayCheckSort(GSIArray array, 
  NSComparisonResult (*sorter)(GSIArrayItem, GSIArrayItem))
{
  unsigned int	i;

  for (i = 1; i < array->count; i++)
    {
#ifdef	GSI_ARRAY_CHECKS
      NSCAssert(((*sorter)(array->ptr[i-1], array->ptr[i]) 
        != NSOrderedDescending), NSInvalidArgumentException);
#endif
    }
}
#endif

GS_STATIC_INLINE void
GSIArrayInsertSorted(GSIArray array, GSIArrayItem item, 
  NSComparisonResult (*sorter)(GSIArrayItem, GSIArrayItem))
{
  unsigned int	index;

#ifdef	GSI_ARRAY_CHECKS
  GSIArrayCheckSort(array, sorter);
#endif
  index = GSIArrayInsertionPosition(array, item, sorter);
  GSIArrayInsertItem(array, item, index);
#ifdef	GSI_ARRAY_CHECKS
  GSIArrayCheckSort(array, sorter);
#endif
}

GS_STATIC_INLINE void
GSIArrayInsertSortedNoRetain(GSIArray array, GSIArrayItem item,
  NSComparisonResult (*sorter)(GSIArrayItem, GSIArrayItem))
{
  unsigned int	index;

#ifdef	GSI_ARRAY_CHECKS
  GSIArrayCheckSort(array, sorter);
#endif
  index = GSIArrayInsertionPosition(array, item, sorter);
  GSIArrayInsertItemNoRetain(array, item, index);
#ifdef	GSI_ARRAY_CHECKS
  GSIArrayCheckSort(array, sorter);
#endif
}

GS_STATIC_INLINE void
GSIArrayRemoveItemAtIndex(GSIArray array, unsigned index)
{
#if	defined(GSI_ARRAY_NO_RELEASE)
# ifdef	GSI_ARRAY_CHECKS
  NSCAssert(index < array->count, NSInvalidArgumentException);
# endif
  while (++index < array->count)
    array->ptr[index-1] = array->ptr[index];
  array->count--;
#else
  GSIArrayItem	tmp;
# ifdef	GSI_ARRAY_CHECKS
  NSCAssert(index < array->count, NSInvalidArgumentException);
# endif
  tmp = array->ptr[index];
  while (++index < array->count)
    array->ptr[index-1] = array->ptr[index];
  array->count--;
  GSI_ARRAY_RELEASE(array, tmp);
#endif
}

GS_STATIC_INLINE void
GSIArrayRemoveLastItem(GSIArray array)
{
#ifdef	GSI_ARRAY_CHECKS
  NSCAssert(array->count, NSInvalidArgumentException);
#endif
  array->count--;
#if	!defined(GSI_ARRAY_NO_RELEASE)
  GSI_ARRAY_RELEASE(array, array->ptr[array->count]);
#endif
}

GS_STATIC_INLINE void
GSIArrayRemoveItemAtIndexNoRelease(GSIArray array, unsigned index)
{
#ifdef	GSI_ARRAY_CHECKS
  NSCAssert(index < array->count, NSInvalidArgumentException);
#endif
  while (++index < array->count)
    array->ptr[index-1] = array->ptr[index];
  array->count--;
}

GS_STATIC_INLINE void
GSIArraySetItemAtIndex(GSIArray array, GSIArrayItem item, unsigned index)
{
#if	defined(GSI_ARRAY_NO_RELEASE)
# ifdef	GSI_ARRAY_CHECKS
  NSCAssert(index < array->count, NSInvalidArgumentException);
# endif
  GSI_ARRAY_RETAIN(array, item);
  array->ptr[index] = item;
#else
  GSIArrayItem	tmp;
# ifdef	GSI_ARRAY_CHECKS
  NSCAssert(index < array->count, NSInvalidArgumentException);
# endif
  tmp = array->ptr[index];
  GSI_ARRAY_RETAIN(array, item);
  array->ptr[index] = item;
  GSI_ARRAY_RELEASE(array, tmp);
#endif
}

/*
 * For direct access ... unsafe if you change the array in any way while
 * examining the contents of this buffer.
 */
GS_STATIC_INLINE GSIArrayItem *
GSIArrayItems(GSIArray array)
{
  return array->ptr;
}

GS_STATIC_INLINE GSIArrayItem
GSIArrayItemAtIndex(GSIArray array, unsigned index)
{
#ifdef	GSI_ARRAY_CHECKS
  NSCAssert(index < array->count, NSInvalidArgumentException);
#endif
  return array->ptr[index];
}

GS_STATIC_INLINE GSIArrayItem
GSIArrayLastItem(GSIArray array)
{
#ifdef	GSI_ARRAY_CHECKS
  NSCAssert(array->count, NSInvalidArgumentException);
#endif
  return array->ptr[array->count-1];
}

GS_STATIC_INLINE void
GSIArrayClear(GSIArray array)
{
  if (array->ptr)
    {
      /*
       * Only free memory if it was dynamically initialised (old > 0)
       */
      if (array->old > 0)
	{
	  NSZoneFree(array->zone, (void*)array->ptr);
	}
      array->ptr = 0;
      array->cap = 0;
    }
}

GS_STATIC_INLINE void
GSIArrayRemoveItemsFromIndex(GSIArray array, unsigned index)
{
  if (index < array->count)
    {
#ifndef	GSI_ARRAY_NO_RELEASE
      while (array->count-- > index)
	{
	  GSI_ARRAY_RELEASE(array, array->ptr[array->count]);
	}
#endif
      array->count = index;
    }
}

GS_STATIC_INLINE void
GSIArrayRemoveAllItems(GSIArray array)
{
#ifndef	GSI_ARRAY_NO_RELEASE
  while (array->count--)
    {
      GSI_ARRAY_RELEASE(array, array->ptr[array->count]);
    }
#endif
  array->count = 0;
}

GS_STATIC_INLINE void
GSIArrayEmpty(GSIArray array)
{
  GSIArrayRemoveAllItems(array);
  GSIArrayClear(array);
}

GS_STATIC_INLINE GSIArray
GSIArrayInitWithZoneAndCapacity(GSIArray array, NSZone *zone, size_t capacity)
{
  unsigned int	size;

  array->zone = zone;
  array->count = 0;
  if (capacity < 2)
    capacity = 2;
  array->cap = capacity;
  array->old = capacity/2;
  size = capacity*sizeof(GSIArrayItem);
  array->ptr = (GSIArrayItem*)NSZoneMalloc(zone, size);
  return array;
}

GS_STATIC_INLINE GSIArray
GSIArrayInitWithZoneAndStaticCapacity(GSIArray array, NSZone *zone,
    size_t capacity, GSIArrayItem *buffer)
{
  array->zone = zone;
  array->count = 0;
  array->cap = capacity;
  array->old = 0;
  array->ptr = buffer;
  return array;
}

GS_STATIC_INLINE GSIArray
GSIArrayCopyWithZone(GSIArray array, NSZone *zone)
{
  unsigned int i;
  GSIArray new;

  new = NSZoneMalloc(zone, sizeof(GSIArray_t));
  GSIArrayInitWithZoneAndCapacity(new, zone, array->count);
  for (i = 0; i < array->count; i++)
    {
      GSI_ARRAY_RETAIN(array, array->ptr[i]);
      new->ptr[new->count++] = array->ptr[i];
    }
  return new;
}

#if	defined(__cplusplus)
}
#endif

#endif	/* OS_API_VERSION(GS_API_NONE,GS_API_NONE) */

