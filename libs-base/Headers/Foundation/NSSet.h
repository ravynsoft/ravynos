/** Interface for NSSet, NSMutableSet, NSCountedSet for GNUStep
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

   AutogsdocSource: NSSet.m
   AutogsdocSource: NSCountedSet.m

   */

#ifndef _NSSet_h_GNUSTEP_BASE_INCLUDE
#define _NSSet_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>
#import <Foundation/NSEnumerator.h>
#import <GNUstepBase/GSBlocks.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class GS_GENERIC_CLASS(NSArray, ElementT);
@class GS_GENERIC_CLASS(NSEnumerator, ElementT);
@class GS_GENERIC_CLASS(NSDictionary, KeyT:id<NSCopying>, ValT);
@class NSString;

GS_EXPORT_CLASS
@interface GS_GENERIC_CLASS(NSSet, __covariant ElementT) : NSObject <NSCoding,
                                                             NSCopying,
                                                             NSMutableCopying,
                                                             NSFastEnumeration>

+ (instancetype) set;
+ (instancetype) setWithArray: (GS_GENERIC_CLASS(NSArray, ElementT)*)objects;
+ (instancetype) setWithObject: (GS_GENERIC_TYPE(ElementT))anObject;
+ (instancetype) setWithObjects: (GS_GENERIC_TYPE(ElementT))firstObject, ...;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (instancetype) setWithObjects: (const GS_GENERIC_TYPE(ElementT)[])objects
		                  count: (NSUInteger)count;
#endif
+ (instancetype) setWithSet: (GS_GENERIC_CLASS(NSSet, ElementT)*)aSet;

- (GS_GENERIC_CLASS(NSArray, ElementT)*) allObjects;
- (GS_GENERIC_TYPE(ElementT)) anyObject;
- (BOOL) containsObject: (GS_GENERIC_TYPE(ElementT))anObject;
- (NSUInteger) count;
- (NSString*) description;
- (NSString*) descriptionWithLocale: (id)locale;

- (instancetype) init;
- (instancetype) initWithArray: (GS_GENERIC_CLASS(NSArray, ElementT)*)other;
- (instancetype) initWithObjects: (GS_GENERIC_TYPE(ElementT))firstObject, ...;
- (instancetype) initWithObjects: (const GS_GENERIC_TYPE(ElementT)[])objects
		                   count: (NSUInteger)count;
- (instancetype) initWithSet: (GS_GENERIC_CLASS(NSSet, ElementT)*)other;
- (instancetype) initWithSet: (GS_GENERIC_CLASS(NSSet, ElementT)*)other
                   copyItems: (BOOL)flag;

- (BOOL) intersectsSet: (GS_GENERIC_CLASS(NSSet, ElementT)*)otherSet;
- (BOOL) isEqualToSet: (GS_GENERIC_CLASS(NSSet, ElementT)*)other;
- (BOOL) isSubsetOfSet: (GS_GENERIC_CLASS(NSSet, ElementT)*)otherSet;

- (void) makeObjectsPerform: (SEL)aSelector;
- (void) makeObjectsPerform: (SEL)aSelector withObject: (id)argument;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) makeObjectsPerformSelector: (SEL)aSelector;
- (void) makeObjectsPerformSelector: (SEL)aSelector withObject: (id)argument;
#endif
- (GS_GENERIC_TYPE(ElementT)) member: (GS_GENERIC_TYPE(ElementT))anObject;
- (GS_GENERIC_CLASS(NSEnumerator, ElementT)*) objectEnumerator;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)

DEFINE_BLOCK_TYPE(GSSetEnumeratorBlock, void, GS_GENERIC_TYPE(ElementT), BOOL*);
DEFINE_BLOCK_TYPE(GSSetFilterBlock, BOOL, GS_GENERIC_TYPE(ElementT), BOOL*);

/**
 * Enumerate over the collection using a given block.  The first argument is
 * the object.  The second argument is a pointer to a BOOL indicating
 * whether the enumeration should stop.  Setting this to YES will interupt
 * the enumeration.
 */
- (void) enumerateObjectsUsingBlock:(GSSetEnumeratorBlock)aBlock;

/**
 * Enumerate over the collection using the given block.  The first argument is
 * the object.  The second argument is a pointer to a BOOL indicating whether
 * the enumeration should stop.  Setting  this to YES will interrupt the
 * enumeration.
 *
 * The opts argument is a bitfield.  Setting the NSNSEnumerationConcurrent flag
 * specifies that it is thread-safe.  The NSEnumerationReverse bit specifies
 * that it should be enumerated in reverse order.
 */
- (void) enumerateObjectsWithOptions: (NSEnumerationOptions)opts
                          usingBlock: (GSSetEnumeratorBlock)aBlock;

- (GS_GENERIC_CLASS(NSSet, ElementT) *) objectsPassingTest:
    (GSSetFilterBlock)aBlock;

- (GS_GENERIC_CLASS(NSSet, ElementT) *) objectsWithOptions:
    (NSEnumerationOptions)opts
                                    passingTest: (GSSetFilterBlock)aBlock;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST)
- (GS_GENERIC_CLASS(NSSet, ElementT) *) setByAddingObject:
    (GS_GENERIC_TYPE(ElementT))anObject;
- (GS_GENERIC_CLASS(NSSet, ElementT) *) setByAddingObjectsFromSet:
    (GS_GENERIC_CLASS(NSSet, ElementT) *)other;
- (GS_GENERIC_CLASS(NSSet, ElementT) *) setByAddingObjectsFromArray:
    (GS_GENERIC_CLASS(NSArray, ElementT) *)other;
#endif
@end

GS_EXPORT_CLASS
@interface GS_GENERIC_CLASS(NSMutableSet, ElementT):
  GS_GENERIC_CLASS(NSSet, ElementT)

+ (instancetype) setWithCapacity: (NSUInteger)numItems;

- (void) addObject: (GS_GENERIC_TYPE(ElementT))anObject;
- (void) addObjectsFromArray: (GS_GENERIC_CLASS(NSArray, ElementT)*)array;
- (instancetype) initWithCapacity: (NSUInteger)numItems;
- (void) intersectSet: (GS_GENERIC_CLASS(NSSet, ElementT)*)other;
- (void) minusSet: (GS_GENERIC_CLASS(NSSet, ElementT)*)other;
- (void) removeAllObjects;
- (void) removeObject: (GS_GENERIC_TYPE(ElementT))anObject;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) setSet: (GS_GENERIC_CLASS(NSSet, ElementT)*)other;
#endif
- (void) unionSet: (GS_GENERIC_CLASS(NSSet, ElementT)*)other;
@end

GS_EXPORT_CLASS
@interface GS_GENERIC_CLASS(NSCountedSet, ElementT) :
  GS_GENERIC_CLASS(NSMutableSet, ElementT)

- (NSUInteger) countForObject: (GS_GENERIC_TYPE(ElementT))anObject;

@end

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)

/**
 * Utility methods for using a counted set to handle uniquing of objects.
 */
@interface GS_GENERIC_CLASS(NSCountedSet, ElementT) (GNU_Uniquing)
/**
 * <p>
 *   This method removes from the set all objects whose count is
 *   less than or equal to the specified value.
 * </p>
 * <p>
 *   This is useful where a counted set is used for uniquing objects.
 *   The set can be periodically purged of objects that have only
 *   been added once - and are therefore simply wasting space.
 * </p>
 */
- (void) purge: (NSInteger)level;

/**
 * <p>
 *   If the supplied object (or one equal to it as determined by
 *   the [NSObject-isEqual:] method) is already present in the set, the
 *   count for that object is incremented, the supplied object
 *   is released, and the object in the set is retained and returned.
 *   Otherwise, the supplied object is added to the set and returned.
 * </p>
 * <p>
 *   This method is useful for uniquing objects - the init method of
 *   a class need simply end with -
 *   <code>
 *     return [myUniquingSet unique: self];
 *   </code>
 * </p>
 */
- (GS_GENERIC_TYPE(ElementT)) unique:
    (GS_GENERIC_TYPE(ElementT)) NS_CONSUMED anObject NS_RETURNS_RETAINED;
@end

/*
 * Functions for managing a global uniquing set.
 */

/*
 * GSUniquing() turns on/off the action of the GSUnique() function.
 * if uniquing is turned off, GSUnique() simply returns its argument.
 *
 */
void	GSUniquing(BOOL flag);

/*
 * GSUnique() returns an object that is equal to the one passed to it.
 * If the returned object is not the same object as the object passed in,
 * the original object is released and the returned object is retained.
 * Thus, an -init method that wants to implement uniquing simply needs
 * to end with 'return GSUnique(self);'
 */
id	GSUnique(id NS_CONSUMED anObject) NS_RETURNS_RETAINED;

/*
 * Management functions -
 */

/*
 * GSUPurge() can be used to purge infrequently referenced objects from the
 * set by removing any objec whose count is less than or equal to that given.
 *
 */
void	GSUPurge(NSUInteger count);

/*
 * GSUSet() can be used to artificially set the count for a particular object
 * Setting the count to zero will remove the object from the global set.
 */
id	GSUSet(id anObject, NSUInteger count);

#endif	/* GS_API_NONE */

#if	defined(__cplusplus)
}
#endif

#endif
