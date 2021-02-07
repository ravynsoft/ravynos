/* Definition of class NSFilePresenter
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

#ifndef __NSFilePresenter_h_GNUSTEP_BASE_INCLUDE
#define __NSFilePresenter_h_GNUSTEP_BASE_INCLUDE

#import <Foundation/NSObject.h>

@class NSError, NSFileVersion, NSOperationQueue, NSSet;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7,GS_API_LATEST)

DEFINE_BLOCK_TYPE_NO_ARGS(GSFilePresenterCompletionHandler, void);
DEFINE_BLOCK_TYPE(GSFilePresenterSubitemDeletionHandler, void, NSError*);
DEFINE_BLOCK_TYPE_NO_ARGS(GSFilePresenterReacquirer, void); 
DEFINE_BLOCK_TYPE(GSFilePresentedItemChangesWithCompletionHandler, void, NSError*);

@protocol NSFilePresenter <NSObject>

- (NSURL *) presentedItemURL;
- (NSOperationQueue *) presentedItemOperationQueue;

#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSFilePresenter)
#endif

- (NSURL *) primaryPresentedItemURL;
- (NSString *) observedPresentedItemUbiquityAttributes;

- (void) accommodatePresentedItemDeletionWithCompletionHandler: (GSFilePresenterCompletionHandler)completionHandler;
- (void) accommodatePresentedSubitemDeletionAtURL:(NSURL *)url completionHandler: (GSFilePresenterSubitemDeletionHandler)completionHandler;
- (void) presentedItemDidChange;
- (void) presentedItemDidChangeUbiquityAttributes: (NSSet *)attributes; // 10.13
- (void) presentedItemDidGainVersion: (NSFileVersion *)version;
- (void) presentedItemDidLoseVersion: (NSFileVersion *)version;
- (void) presentedItemDidMoveToURL: (NSURL *)newURL;
- (void) presentedItemDidResolveConflictVersion: (NSFileVersion *)version;
- (void) presentedSubitemAtURL: (NSURL *)oldURL didMoveToURL: (NSURL *)newURL;
- (void) presentedSubitemAtURL: (NSURL *)url didGainVersion: (NSFileVersion *)version;
- (void) presentedSubitemAtURL: (NSURL *)url didLoseVersion: (NSFileVersion *)version;
- (void) presentedSubitemAtURL: (NSURL *)url didResolveConflictVersion: (NSFileVersion *)version;
- (void) presentedSubitemDidAppearAtURL: (NSURL *)url;
- (void) presentedSubitemDidChangeAtURL: (NSURL *)url;
- (void) relinquishPresentedItemToReader: (GSFilePresenterReacquirer)reader;
- (void) relinquishPresentedItemToWriter: (GSFilePresenterReacquirer)writer;
- (void) savePresentedItemChangesWithCompletionHandler: (GSFilePresentedItemChangesWithCompletionHandler)completionHandler;

@end

#endif
#endif
