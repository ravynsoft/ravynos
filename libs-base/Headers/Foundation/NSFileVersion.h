/* Definition of class NSFileVersion
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   Implemented by: 	Gregory Casamento <greg.casamento@gmail.com>
   Date: 	Sep 2019
   Original File by: Daniel Ferreira

   This file is part of the GNUstep Library.
   
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

#ifndef __NSFileVersion_h_GNUSTEP_BASE_INCLUDE
#define __NSFileVersion_h_GNUSTEP_BASE_INCLUDE

#import <Foundation/NSObject.h>

@class NSArray, NSDate, NSDictionary, NSError, NSString, NSURL, NSPersonNameComponents;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7,GS_API_LATEST)

enum {
    NSFileVersionReplacingByMoving = 1 << 0
};
typedef NSUInteger NSFileVersionReplacingOptions;

enum {
    NSFileVersionAddingByMoving = 1 << 0
};
typedef NSUInteger NSFileVersionAddingOptions;

GS_EXPORT_CLASS
@interface NSFileVersion : NSObject
{
@private
    NSURL *_fileURL;
    NSURL *_contentsURL;
    id _persistentIdentifier;
    id _nonLocalVersion;
    BOOL _isResolved;
    BOOL _isDiscardable;
    BOOL _hasThumbnail;
    BOOL _hasLocalContents;
    BOOL _conflict;
    NSString *_localizedName;
    NSString *_localizedNameOfSavingComputer;
    NSDate *_modificationDate;
}

// Initializers
+ (NSFileVersion *)currentVersionOfItemAtURL: (NSURL *)url;
+ (NSArray *)otherVersionsOfItemAtURL: (NSURL *)url;
+ (NSFileVersion *)versionOfItemAtURL: (NSURL *)url 
              forPersistentIdentifier: (id)persistentIdentifier;
+ (NSURL *)temporaryDirectoryURLForNewVersionOfItemAtURL: (NSURL *)url;
+ (NSFileVersion *)addVersionOfItemAtURL: (NSURL *)url 
                       withContentsOfURL: (NSURL *)contentsURL 
                                 options: (NSFileVersionAddingOptions)options 
                                   error: (NSError **)outError;
+ (NSArray *)unresolvedConflictVersionsOfItemAtURL: (NSURL *)url;
+ (BOOL)removeOtherVersionsOfItemAtURL: (NSURL *)url 
                                 error: (NSError **)outError;

// instance methods...
- (BOOL) isDiscardable;
- (void) setDiscardable: (BOOL)flag;
- (BOOL) isResolved;
- (void) setResolved: (BOOL)flag;
- (NSDate *) modificationDate;
- (NSPersonNameComponents *) originatorNameComponents; 
- (NSString *) localizedName;
- (NSString *) localizedNameOfSavingComputer;
- (BOOL) hasLocalContents; 
- (BOOL) hasThumbnail; 
- (NSURL *) URL;
- (BOOL) conflict;
- (id<NSCoding>) persistentIdentifier;

- (BOOL) removeAndReturnError: (NSError **)outError;
- (NSURL *) replaceItemAtURL: (NSURL *)url
                     options: (NSFileVersionReplacingOptions)options
                       error: (NSError **)error;

@end

#endif
#endif
