/* Definition of class NSFontCollection
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Tue Dec 10 11:51:33 EST 2019

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSFontCollection_h_GNUSTEP_GUI_INCLUDE
#define _NSFontCollection_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif
  
@class NSFontDescriptor, NSLocale, NSError, NSArray, NSMutableArray, NSDictionary, NSMutableDictionary;

enum {
    NSFontCollectionVisibilityProcess = (1UL << 0),
    NSFontCollectionVisibilityUser = (1UL << 1),
    NSFontCollectionVisibilityComputer = (1UL << 2)
};
typedef NSUInteger NSFontCollectionVisibility;

typedef NSString* NSFontCollectionMatchingOptionKey; 

APPKIT_EXPORT NSFontCollectionMatchingOptionKey const NSFontCollectionIncludeDisabledFontsOption;
APPKIT_EXPORT NSFontCollectionMatchingOptionKey const NSFontCollectionRemoveDuplicatesOption;
APPKIT_EXPORT NSFontCollectionMatchingOptionKey const NSFontCollectionDisallowAutoActivationOption;

typedef NSString* NSFontCollectionName;

@interface NSFontCollection : NSObject <NSCopying, NSMutableCopying, NSCoding>
{
  NSMutableDictionary *_fontCollectionDictionary;
}
  
// Initializers...
+ (NSFontCollection *) fontCollectionWithDescriptors: (NSArray *)queryDescriptors;
+ (NSFontCollection *) fontCollectionWithAllAvailableDescriptors;
+ (NSFontCollection *) fontCollectionWithLocale: (NSLocale *)locale;

+ (BOOL) showFontCollection: (NSFontCollection *)collection
                   withName: (NSFontCollectionName)name
                 visibility: (NSFontCollectionVisibility)visibility
                      error: (NSError **)error;

+ (BOOL) hideFontCollectionWithName: (NSFontCollectionName)name
                         visibility: (NSFontCollectionVisibility)visibility
                              error: (NSError **)error;

+ (BOOL) renameFontCollectionWithName: (NSFontCollectionName)name
                           visibility: (NSFontCollectionVisibility)visibility
                               toName: (NSFontCollectionName)name
                                error: (NSError **)error;

+ (NSArray *) allFontCollectionNames;

+ (NSFontCollection *) fontCollectionWithName: (NSFontCollectionName)name;

+ (NSFontCollection *) fontCollectionWithName: (NSFontCollectionName)name
                                   visibility: (NSFontCollectionVisibility)visibility;


// Descriptors
- (NSArray *) queryDescriptors; 

- (NSArray *) exclusionDescriptors;

- (NSArray *) matchingDescriptors; 
- (NSArray *) matchingDescriptorsWithOptions: (NSDictionary *)options;

- (NSArray *) matchingDescriptorsForFamily: (NSString *)family;
- (NSArray *) matchingDescriptorsForFamily: (NSString *)family options: (NSDictionary *)options;
  
@end
 

@interface NSMutableFontCollection : NSFontCollection

+ (NSMutableFontCollection *) fontCollectionWithDescriptors: (NSArray *)queryDescriptors;
+ (NSMutableFontCollection *) fontCollectionWithAllAvailableDescriptors;
+ (NSMutableFontCollection *) fontCollectionWithLocale: (NSLocale *)locale;
+ (NSMutableFontCollection *) fontCollectionWithName: (NSFontCollectionName)name;

+ (NSMutableFontCollection *) fontCollectionWithName: (NSFontCollectionName)name
                                          visibility: (NSFontCollectionVisibility)visibility;

- (NSArray *) queryDescriptors; 
- (void) setQueryDescriptors: (NSArray *)queryDescriptors; 

- (NSArray *) exclusionDescriptors;
- (void) setExclusionDescriptors: (NSArray *)exclusionDescriptors;

- (void)addQueryForDescriptors: (NSArray *)descriptors;

- (void)removeQueryForDescriptors: (NSArray *)descriptors;

@end

// NSFontCollectionDidChangeNotification
APPKIT_EXPORT NSString * const NSFontCollectionDidChangeNotification;

// Notification user info dictionary keys
typedef NSString * NSFontCollectionUserInfoKey;
APPKIT_EXPORT NSFontCollectionUserInfoKey const NSFontCollectionActionKey;
APPKIT_EXPORT NSFontCollectionUserInfoKey const NSFontCollectionNameKey;
APPKIT_EXPORT NSFontCollectionUserInfoKey const NSFontCollectionOldNameKey;
APPKIT_EXPORT NSFontCollectionUserInfoKey const NSFontCollectionVisibilityKey;

// Values for NSFontCollectionAction
typedef NSString * NSFontCollectionActionTypeKey;
APPKIT_EXPORT NSFontCollectionActionTypeKey const NSFontCollectionWasShown;
APPKIT_EXPORT NSFontCollectionActionTypeKey const NSFontCollectionWasHidden;
APPKIT_EXPORT NSFontCollectionActionTypeKey const NSFontCollectionWasRenamed;

// Standard named collections
APPKIT_EXPORT NSFontCollectionName const NSFontCollectionAllFonts;
APPKIT_EXPORT NSFontCollectionName const NSFontCollectionUser;
APPKIT_EXPORT NSFontCollectionName const NSFontCollectionFavorites;
APPKIT_EXPORT NSFontCollectionName const NSFontCollectionRecentlyUsed;

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSFontCollection_h_GNUSTEP_GUI_INCLUDE */

