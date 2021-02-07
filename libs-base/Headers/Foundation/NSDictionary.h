/* Interface for NSDictionary for GNUStep
   Copyright (C) 1995, 1996, 1999 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Date: 1995

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

#ifndef _NSDictionary_h_GNUSTEP_BASE_INCLUDE
#define _NSDictionary_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>
#import <GNUstepBase/GSBlocks.h>
#import	<Foundation/NSObject.h>
#import	<Foundation/NSEnumerator.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class GS_GENERIC_CLASS(NSArray, ElementT);
@class GS_GENERIC_CLASS(NSSet, ElementT);
@class NSString, NSURL;

GS_EXPORT_CLASS
@interface GS_GENERIC_CLASS(NSDictionary,
  __covariant KeyT:id<NSCopying>, __covariant ValT)
  : NSObject <NSCoding, NSCopying, NSMutableCopying, NSFastEnumeration>
+ (instancetype) dictionary;
+ (instancetype) dictionaryWithContentsOfFile: (NSString*)path;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (instancetype) dictionaryWithContentsOfURL: (NSURL*)aURL;
#endif
+ (instancetype) dictionaryWithDictionary: (NSDictionary*)otherDictionary;
+ (instancetype) dictionaryWithObject: (GS_GENERIC_TYPE(ValT))object
                               forKey: (GS_GENERIC_TYPE(KeyT))key;
+ (instancetype) dictionaryWithObjects: (GS_GENERIC_CLASS(NSArray,ValT)*)objects
                               forKeys: (GS_GENERIC_CLASS(NSArray,KeyT)*)keys;
+ (instancetype) dictionaryWithObjects: (const GS_GENERIC_TYPE(ValT)[])objects
  forKeys: (const GS_GENERIC_TYPE_F(KeyT,id<NSCopying>)[])keys
  count: (NSUInteger)count;
+ (instancetype) dictionaryWithObjectsAndKeys: (id)firstObject, ...;

- (GS_GENERIC_CLASS(NSArray,KeyT)*) allKeys;
- (GS_GENERIC_CLASS(NSArray,KeyT)*) allKeysForObject:
  (GS_GENERIC_TYPE(ValT))anObject;
- (GS_GENERIC_CLASS(NSArray,ValT)*) allValues;
- (NSUInteger) count;						// Primitive
- (NSString*) description;
- (NSString*) descriptionInStringsFileFormat;
- (NSString*) descriptionWithLocale: (id)locale;
- (NSString*) descriptionWithLocale: (id)locale
			     indent: (NSUInteger)level;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
DEFINE_BLOCK_TYPE(GSKeysAndObjectsEnumeratorBlock, void, GS_GENERIC_TYPE_F(KeyT,id<NSCopying>), GS_GENERIC_TYPE(ValT), BOOL*);
- (void) enumerateKeysAndObjectsUsingBlock:
  (GSKeysAndObjectsEnumeratorBlock)aBlock;
- (void) enumerateKeysAndObjectsWithOptions: (NSEnumerationOptions)opts
  usingBlock: (GSKeysAndObjectsEnumeratorBlock)aBlock;
#endif

- (void) getObjects: (__unsafe_unretained GS_GENERIC_TYPE(ValT)[])objects
  andKeys: (__unsafe_unretained GS_GENERIC_TYPE_F(KeyT,id<NSCopying>)[])keys;
- (instancetype) init;
- (instancetype) initWithContentsOfFile: (NSString*)path;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (instancetype) initWithContentsOfURL: (NSURL*)aURL;
#endif

- (instancetype) initWithDictionary:
    (GS_GENERIC_CLASS(NSDictionary,KeyT, ValT)*)otherDictionary;
- (id) initWithDictionary: (GS_GENERIC_CLASS(NSDictionary,KeyT, ValT)*)other
                copyItems: (BOOL)shouldCopy;
- (id) initWithObjects: (GS_GENERIC_CLASS(NSArray,KeyT)*)objects
               forKeys: (GS_GENERIC_CLASS(NSArray,ValT)*)keys;
- (id) initWithObjectsAndKeys: (GS_GENERIC_TYPE(ValT))firstObject, ...;
- (id) initWithObjects: (const GS_GENERIC_TYPE(ValT)[])objects
	       forKeys: (const GS_GENERIC_TYPE_F(KeyT,id<NSCopying>)[])keys
		 count: (NSUInteger)count; // Primitive
- (BOOL) isEqualToDictionary: (GS_GENERIC_CLASS(NSDictionary,KeyT, ValT)*)other;

- (GS_GENERIC_CLASS(NSEnumerator,KeyT)*) keyEnumerator;	// Primitive

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
DEFINE_BLOCK_TYPE(GSKeysAndObjectsPredicateBlock, BOOL, GS_GENERIC_TYPE_F(KeyT,id<NSCopying>), GS_GENERIC_TYPE(ValT), BOOL*);
- (GS_GENERIC_CLASS(NSSet,KeyT)*) keysOfEntriesPassingTest:
    (GSKeysAndObjectsPredicateBlock)aPredicate;
- (GS_GENERIC_CLASS(NSSet,KeyT)*) keysOfEntriesWithOptions:
  (NSEnumerationOptions)opts
  passingTest: (GSKeysAndObjectsPredicateBlock)aPredicate;
#endif

- (GS_GENERIC_CLASS(NSArray,ValT)*) keysSortedByValueUsingSelector: (SEL)comp;
- (GS_GENERIC_CLASS(NSArray,ValT)*) keysSortedByValueUsingComparator:(NSComparator)cmptr;
- (GS_GENERIC_CLASS(NSArray,ValT)*) keysSortedByValueWithOptions:(NSSortOptions)opts usingComparator:(NSComparator)cmptr;  
- (GS_GENERIC_CLASS(NSEnumerator,ValT)*) objectEnumerator;	// Primitive
- (GS_GENERIC_TYPE(ValT)) objectForKey:
  (GS_GENERIC_TYPE(KeyT))aKey;				// Primitive
- (GS_GENERIC_CLASS(NSArray,ValT)*) objectsForKeys:
  (GS_GENERIC_CLASS(NSArray,KeyT)*)keys
  notFoundMarker: (GS_GENERIC_TYPE(ValT))marker;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (GS_GENERIC_TYPE(ValT)) valueForKey: (NSString*)key;
#endif

- (BOOL) writeToFile: (NSString*)path atomically: (BOOL)useAuxiliaryFile;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (BOOL) writeToURL: (NSURL*)url atomically: (BOOL)useAuxiliaryFile;
#endif
/**
 * Method called by array subscripting.
 */
- (GS_GENERIC_TYPE(ValT)) objectForKeyedSubscript:
  (GS_GENERIC_TYPE(KeyT))aKey;
@end

GS_EXPORT_CLASS
@interface  GS_GENERIC_CLASS(NSMutableDictionary, KeyT:id<NSCopying>, ValT) :
  GS_GENERIC_CLASS(NSDictionary, KeyT, ValT)

+ (instancetype) dictionaryWithCapacity: (NSUInteger)numItems;

- (void) addEntriesFromDictionary:
    (GS_GENERIC_CLASS(NSDictionary, KeyT, ValT)*)otherDictionary;
- (instancetype) initWithCapacity: (NSUInteger)numItems;	// Primitive
- (void) removeAllObjects;
/**
 * Removes the object with the specified key from the receiver. This method
 * is primitive.
 */
- (void) removeObjectForKey: (GS_GENERIC_TYPE(KeyT))aKey;
- (void) removeObjectsForKeys: (GS_GENERIC_CLASS(NSArray, KeyT) *)keyArray;
- (void) setObject: (GS_GENERIC_TYPE(ValT))anObject
            forKey: (GS_GENERIC_TYPE(KeyT))aKey; // Primitive
- (void) setDictionary:
    (GS_GENERIC_CLASS(NSDictionary, KeyT, ValT)*)otherDictionary;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) setValue: (GS_GENERIC_TYPE(ValT))value forKey: (NSString*)key;
- (void) takeStoredValue: (GS_GENERIC_TYPE(ValT))value forKey: (NSString*)key;
- (void) takeValue: (GS_GENERIC_TYPE(ValT))value forKey: (NSString*)key;
#endif
/**
 * Method called by array subscripting.
 */
- (void) setObject: (GS_GENERIC_TYPE(ValT))anObject
 forKeyedSubscript: (GS_GENERIC_TYPE(KeyT))aKey;

@end

#if	defined(__cplusplus)
}
#endif

#endif
