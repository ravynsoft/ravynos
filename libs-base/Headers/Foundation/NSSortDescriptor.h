/* Interface for NSSortDescriptor for GNUStep
   Copyright (C) 2005 Free Software Foundation, Inc.

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

#ifndef __NSSortDescriptor_h_GNUSTEP_BASE_INCLUDE
#define __NSSortDescriptor_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3,GS_API_LATEST)

#import	<Foundation/NSObject.h>
#import	<Foundation/NSArray.h>
#import	<Foundation/NSSet.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSString;

/**
 * Instances of this class are used to perform multi-level sorts of
 * arrays containging collections or other objects whose properties
 * can be obtained using key names.
 */
GS_EXPORT_CLASS
@interface NSSortDescriptor : NSObject <NSCopying, NSCoding>
{
#if	GS_EXPOSE(NSSortDescriptor)
@private
  NSString *_key;
  BOOL	_ascending;
  SEL	_selector;
  NSComparator _comparator;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}

/** Returns a flag indicating whether the sort descriptor sorts objects
 * in ascending order (YES) or descending order (NO).
 */
- (BOOL) ascending;

/** Returns the result of comparing object1 to object2 using the property
 * whose key is defined in the receiver and using the selector of the
 * receiver.  If the receiver performs a descending order sort, the
 * result of this comparison is the opposite of that prroduced by
 * applying the selector.
 */
- (NSComparisonResult) compareObject: (id)object1 toObject: (id)object2;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST) 
/** <p>Returns an autoreleased sort descriptor for comparisons using the 
 * 'compare:' selector and the specified key and ordering.</p>
 *
 * <p>See also -initWithKey:ascending:.</p>
 */
+ (id) sortDescriptorWithKey: (NSString *)aKey ascending: (BOOL)ascending;

/** <p>Returns an autoreleased sort descriptor initialized to perform 
 * comparisons in the specified order using aSelector to compare the property 
 * aKey of each object.</p>
 *
 * <p>See also -initWithKey:ascending:selector:.</p>
 */
+ (id) sortDescriptorWithKey: (NSString *)aKey 
                   ascending: (BOOL)ascending 
                    selector: (SEL)aSelector;

/** <p>Returns an autoreleased sort descriptor initialized to perform 
 * comparisons in the specified order using the comparator to compare 
 * the property aKey of each object.</p>
 *
 * <p>See also -initWithKey:ascending:selector:.</p>
 */
+ (id)sortDescriptorWithKey: (NSString *)key 
                  ascending: (BOOL)ascending 
                 comparator: (NSComparator)cmptr;

/** <init />
 * Initialises the receiver to perform comparisons in the specified order
 * using the comparator to compare the property key of each object.
 */
- (id) initWithKey: (NSString *)key
         ascending: (BOOL)ascending
        comparator: (NSComparator)cmptr;
#endif

/** Initialises the receiver for comparisons using the 'compare:' selector
 * and the specified key and ordering.
 */
- (id) initWithKey: (NSString *)key
	 ascending: (BOOL)ascending;

/** <init />
 * Initialises the receiver to perform comparisons in the specified order
 * using selector to compare the property key of each object.
 */
- (id) initWithKey: (NSString *)key
         ascending: (BOOL)ascending
          selector: (SEL)selector;


/** Returns the key used to obtain the property on which comparisons are based.
 */
- (NSString *) key;

/** Returns the selector used to compare the properties of objects.
 */
- (SEL) selector;

/** Returns a copy of the receiver which compares and sorts in reversed
 * order.
 */
- (id) reversedSortDescriptor;
@end

@interface NSArray (NSSortDescriptorSorting)

/**
 * Produces a sorted array using the mechanism described for
 * [NSMutableArray-sortUsingDescriptors:]
 */
- (NSArray *) sortedArrayUsingDescriptors: (NSArray *)sortDescriptors;

@end

@interface NSMutableArray (NSSortDescriptorSorting)

/**
 * This method works like this: first, it sorts the entire
 * contents of the array using the first sort descriptor. Then,
 * after each sort-run, it looks whether there are sort
 * descriptors left to process, and if yes, looks at the partially
 * sorted array, finds all portions in it which are equal
 * (evaluate to NSOrderedSame) and applies the following
 * descriptor onto them. It repeats this either until all
 * descriptors have been applied or there are no more equal
 * portions (equality ranges) left in the array.
 */
- (void) sortUsingDescriptors: (NSArray *)sortDescriptors;

@end

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) 
@interface NSSet (NSSortDescriptorSorting)
 /**
 * Produces a sorted array from using the mechanism described for
 * [NSMutableArray-sortUsingDescriptors:]
 */
- (NSArray *) sortedArrayUsingDescriptors: (NSArray *)sortDescriptors;
@end
#endif

#if	defined(__cplusplus)
}
#endif

#endif	/* 100400 */

#endif /* __NSSortDescriptor_h_GNUSTEP_BASE_INCLUDE */
