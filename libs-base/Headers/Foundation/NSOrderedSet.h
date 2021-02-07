/** Interface for NSOrderedSet, NSMutableOrderedSet for GNUStep
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

#ifndef _NSOrderedSet_h_GNUSTEP_BASE_INCLUDE
#define _NSOrderedSet_h_GNUSTEP_BASE_INCLUDE

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7,GS_API_LATEST)

#import <GNUstepBase/GSVersionMacros.h>
#import <GNUstepBase/GSBlocks.h>

#import <Foundation/NSObject.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSIndexSet.h>
#import <Foundation/NSKeyedArchiver.h>

#if	defined(__cplusplus)
extern "C" {
#endif
  
@class GS_GENERIC_CLASS(NSArray, ElementT);
@class GS_GENERIC_CLASS(NSEnumerator, ElementT);
@class GS_GENERIC_CLASS(NSSet, ElementT);
@class GS_GENERIC_CLASS(NSDictionary, KeyT:id<NSCopying>, ValT);
@class NSString;
@class NSPredicate;

GS_EXPORT_CLASS
@interface GS_GENERIC_CLASS(NSOrderedSet, __covariant ElementT) : NSObject <NSCoding,
  NSCopying,
  NSMutableCopying,
  NSFastEnumeration>

// class methods
+ (instancetype) orderedSet;
+ (instancetype) orderedSetWithArray:(GS_GENERIC_CLASS(NSArray, ElementT)*)objects;
+ (instancetype) orderedSetWithArray:(GS_GENERIC_CLASS(NSArray, ElementT)*)objects
                               range: (NSRange)range
                           copyItems:(BOOL)flag;
+ (instancetype) orderedSetWithObject:(GS_GENERIC_TYPE(ElementT))anObject;
+ (instancetype) orderedSetWithObjects:(GS_GENERIC_TYPE(ElementT))firstObject, ...;
+ (instancetype) orderedSetWithObjects:(const GS_GENERIC_TYPE(ElementT)[])objects
                                 count:(NSUInteger) count;
+ (instancetype) orderedSetWithOrderedSet:(GS_GENERIC_CLASS(NSOrderedSet, ElementT)*)aSet;
+ (instancetype) orderedSetWithSet:(GS_GENERIC_CLASS(NSSet, ElementT)*)aSet;
+ (instancetype) orderedSetWithSet:(GS_GENERIC_CLASS(NSSet, ElementT)*)aSet
                         copyItems:(BOOL)flag;

// instance methods
- (instancetype) initWithArray:(GS_GENERIC_CLASS(NSArray, ElementT)*)array;
- (instancetype) initWithArray:(GS_GENERIC_CLASS(NSArray, ElementT)*)array copyItems:(BOOL)flag;
- (instancetype) initWithArray:(GS_GENERIC_CLASS(NSArray, ElementT)*)array
                         range:(NSRange)range
                     copyItems:(BOOL)flag;
- (instancetype) initWithObject:(id)object;
- (instancetype) initWithObjects:(GS_GENERIC_TYPE(ElementT))firstObject, ...;
- (instancetype) initWithObjects:(const GS_GENERIC_TYPE(ElementT)[])objects
                           count:(NSUInteger)count;
- (instancetype) initWithOrderedSet:(GS_GENERIC_CLASS(NSOrderedSet, ElementT)*)aSet;
- (instancetype) initWithOrderedSet:(GS_GENERIC_CLASS(NSOrderedSet, ElementT)*)aSet
                          copyItems:(BOOL)flag;
- (instancetype) initWithOrderedSet:(GS_GENERIC_CLASS(NSOrderedSet, ElementT)*)aSet
                              range: (NSRange)range
                          copyItems:(BOOL)flag;
- (instancetype) initWithSet:(GS_GENERIC_CLASS(NSSet, ElementT)*)aSet;
- (instancetype) initWithSet:(GS_GENERIC_CLASS(NSSet, ElementT)*)aSet copyItems:(BOOL)flag;
- (instancetype) init;
- (NSUInteger) count;
- (BOOL)containsObject:(GS_GENERIC_TYPE(ElementT))anObject;
- (void) enumerateObjectsAtIndexes:(NSIndexSet *)indexSet
                           options:(NSEnumerationOptions)opts
                        usingBlock:(GSEnumeratorBlock)aBlock;
- (void) enumerateObjectsUsingBlock: (GSEnumeratorBlock)aBlock;
- (void) enumerateObjectsWithOptions:(NSEnumerationOptions)opts
                          usingBlock:(GSEnumeratorBlock)aBlock;
- (GS_GENERIC_TYPE(ElementT)) firstObject;
- (GS_GENERIC_TYPE(ElementT)) lastObject;
- (GS_GENERIC_TYPE(ElementT)) objectAtIndex: (NSUInteger)index;
  - (GS_GENERIC_TYPE(ElementT)) objectAtIndexedSubscript:(NSUInteger)index;
- (GS_GENERIC_CLASS(NSArray, ElementT)*) objectsAtIndexes:(NSIndexSet *)indexes;
- (NSUInteger) indexOfObject:(GS_GENERIC_TYPE(ElementT))objects;  
- (NSUInteger) indexOfObject: (id)key
               inSortedRange: (NSRange)range
                     options: (NSBinarySearchingOptions)options
             usingComparator: (NSComparator)comparator;

- (NSUInteger) indexOfObjectAtIndexes:(NSIndexSet *)indexSet
                              options:(NSEnumerationOptions)opts
                          passingTest:(GSPredicateBlock)predicate;
- (NSUInteger) indexOfObjectPassingTest:(GSPredicateBlock)predicate;
- (NSUInteger) indexOfObjectWithOptions:(NSEnumerationOptions)opts
                            passingTest:(GSPredicateBlock)predicate;
- (NSIndexSet *) indexesOfObjectsAtIndexes:(NSIndexSet *)indexSet
                              options:(NSEnumerationOptions)opts
                          passingTest:(GSPredicateBlock)predicate;

- (NSIndexSet *)indexesOfObjectsPassingTest:(GSPredicateBlock)predicate;
- (NSIndexSet *) indexesOfObjectsWithOptions:(NSEnumerationOptions)opts
                            passingTest:(GSPredicateBlock)predicate;
- (GS_GENERIC_CLASS(NSEnumerator, ElementT)*) objectEnumerator;
- (GS_GENERIC_CLASS(NSEnumerator, ElementT)*) reverseObjectEnumerator;
- (NSOrderedSet *)reversedOrderedSet;
- (void) getObjects: (__unsafe_unretained GS_GENERIC_TYPE(ElementT)[])aBuffer
              range: (NSRange)aRange;

// Key value coding support
- (void) setValue: (id)value forKey: (NSString*)key;
- (id) valueForKey: (NSString*)key; 

// Comparing Sets
- (BOOL) isEqualToOrderedSet: (NSOrderedSet *)aSet;
  
// Set operations
- (BOOL) intersectsOrderedSet: (NSOrderedSet *)aSet;
- (BOOL) intersectsSet: (NSSet *)aSet;
- (BOOL) isSubsetOfOrderedSet: (NSOrderedSet *)aSet;
- (BOOL) isSubsetOfSet:(NSSet *)aSet;

// Creating a Sorted Array
- (GS_GENERIC_CLASS(NSArray, ElementT) *) sortedArrayUsingDescriptors:(NSArray *)sortDescriptors;
- (GS_GENERIC_CLASS(NSArray, ElementT) *) sortedArrayUsingComparator:
    (NSComparator)comparator;
- (GS_GENERIC_CLASS(NSArray, ElementT) *)
    sortedArrayWithOptions: (NSSortOptions)options
           usingComparator: (NSComparator)comparator;

// Filtering Ordered Sets
- (NSOrderedSet *)filteredOrderedSetUsingPredicate: (NSPredicate *)predicate;

// Describing a set
- (NSString *) description;
- (NSString *) descriptionWithLocale: (NSLocale *)locale;
- (NSString *) descriptionWithLocale: (NSLocale *)locale indent: (BOOL)flag;

  // Convert to other types
- (NSArray *) array;
- (NSSet *) set;
@end

// Mutable Ordered Set
GS_EXPORT_CLASS
@interface GS_GENERIC_CLASS(NSMutableOrderedSet, ElementT) : GS_GENERIC_CLASS(NSOrderedSet, ElementT)
// Creating a Mutable Ordered Set
+ (instancetype)orderedSetWithCapacity: (NSUInteger)capacity;
- (instancetype)initWithCapacity: (NSUInteger)capacity;
- (instancetype) init;
- (void)addObject:(GS_GENERIC_TYPE(ElementT))anObject;
- (void)addObjects:(const GS_GENERIC_TYPE(ElementT)[])objects count:(NSUInteger)count;
- (void)addObjectsFromArray:(GS_GENERIC_CLASS(NSArray, ElementT)*)otherArray;
- (void)insertObject:(GS_GENERIC_TYPE(ElementT))object atIndex:(NSUInteger)index;
- (void)setObject:(GS_GENERIC_TYPE(ElementT))object atIndexedSubscript:(NSUInteger)index;
- (void)insertObjects:(GS_GENERIC_CLASS(NSArray, ElementT)*)array atIndexes:(NSIndexSet *)indexes;
- (void)removeObject:(GS_GENERIC_TYPE(ElementT))object;
- (void)removeObjectAtIndex:(NSUInteger)index;
- (void)removeObjectsAtIndexes:(NSIndexSet *)indexes;
- (void)removeObjectsInArray:(GS_GENERIC_CLASS(NSArray, ElementT)*)otherArray;
- (void)removeObjectsInRange:(NSRange)range;
- (void)removeAllObjects;
- (void)replaceObjectAtIndex:(NSUInteger)index
                  withObject:(GS_GENERIC_TYPE(ElementT))object;
- (void) replaceObjectsAtIndexes: (NSIndexSet *)indexes
                     withObjects: (GS_GENERIC_CLASS(NSArray, ElementT)*)objects;
- (void) replaceObjectsInRange:(NSRange)range
                   withObjects:(const GS_GENERIC_TYPE(ElementT)[])objects
                         count: (NSUInteger)count;
- (void)setObject:(GS_GENERIC_TYPE(ElementT))object atIndex:(NSUInteger)index;
- (void)moveObjectsAtIndexes:(NSIndexSet *)indexes toIndex:(NSUInteger)index;
- (void) exchangeObjectAtIndex:(NSUInteger)index withObjectAtIndex:(NSUInteger)otherIndex;
- (void)filterUsingPredicate:(NSPredicate *)predicate;
- (void) sortUsingDescriptors:(NSArray *)descriptors;
- (void) sortUsingComparator: (NSComparator)comparator;
- (void) sortWithOptions: (NSSortOptions)options
         usingComparator: (NSComparator)comparator;  
- (void) sortRange: (NSRange)range
           options:(NSSortOptions)options
   usingComparator: (NSComparator)comparator;
- (void) intersectOrderedSet:(GS_GENERIC_CLASS(NSOrderedSet, ElementT)*)aSet;
- (void) intersectSet:(GS_GENERIC_CLASS(NSSet, ElementT)*)aSet;
- (void) minusOrderedSet:(GS_GENERIC_CLASS(NSOrderedSet, ElementT)*)aSet;
- (void) minusSet:(GS_GENERIC_CLASS(NSSet, ElementT)*)aSet;
- (void) unionOrderedSet:(GS_GENERIC_CLASS(NSOrderedSet, ElementT)*)aSet;
- (void) unionSet:(GS_GENERIC_CLASS(NSSet, ElementT)*)aSet;
- (instancetype) initWithCoder: (NSCoder *)coder;
@end

#if	defined(__cplusplus)
}
#endif

#endif /* OS_API_VERSION check */

#endif /* _NSOrderedSet_h_GNUSTEP_BASE_INCLUDE */
