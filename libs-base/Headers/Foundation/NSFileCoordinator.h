/* Definition of class NSFileCoordinator
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

#ifndef __NSFileCoordinator_h_GNUSTEP_BASE_INCLUDE
#define __NSFileCoordinator_h_GNUSTEP_BASE_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSURL.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7,GS_API_LATEST)

@class NSArray, NSError, NSMutableDictionary, NSOperationQueue, NSSet; 

@protocol NSFilePresenter;

enum {
    NSFileCoordinatorReadingWithoutChanges = 1 << 0,
    NSFileCoordinatorReadingResolvesSymbolicLink = 1 << 1,
    NSFileCoordinatorReadingImmediatelyAvailableMetadataOnly = 1 << 2,
    NSFileCoordinatorReadingForUploading = 1 << 3,
};
typedef NSUInteger NSFileCoordinatorReadingOptions;

enum {
    NSFileCoordinatorWritingForDeleting = 1 << 0,
    NSFileCoordinatorWritingForMoving = 1 << 1,
    NSFileCoordinatorWritingForMerging = 1 << 2,
    NSFileCoordinatorWritingForReplacing = 1 << 3,
    NSFileCoordinatorWritingContentIndependentMetadataOnly = 1 << 4,
};
typedef NSUInteger NSFileCoordinatorWritingOptions;

GS_EXPORT_CLASS
@interface NSFileAccessIntent : NSObject
{
  NSURL *_url;
  BOOL _isRead;
  NSInteger _options;
}
+ (instancetype) readingIntentWithURL: (NSURL *)url
                              options: (NSFileCoordinatorReadingOptions)options;
+ (instancetype) writingIntentWithURL: (NSURL *)url
                              options: (NSFileCoordinatorWritingOptions)options;
- (NSURL *) URL;
@end

DEFINE_BLOCK_TYPE(GSNoEscapeReadWriteHandler, void, NSURL*, NSURL*);
DEFINE_BLOCK_TYPE(GSNoEscapeNewURLHandler, void, NSURL*);
DEFINE_BLOCK_TYPE(GSAccessorCallbackHandler, void, NSError*);
DEFINE_BLOCK_TYPE(GSDualWriteURLCallbackHandler, void, NSURL*, NSURL*);
DEFINE_BLOCK_TYPE_NO_ARGS(GSBatchAccessorCompletionHandler, void);
DEFINE_BLOCK_TYPE(GSBatchAccessorCompositeBlock, void, GSBatchAccessorCompletionHandler);

GS_EXPORT_CLASS
@interface NSFileCoordinator : NSObject
{
  id _purposeIdentifier;
  BOOL _isCancelled;
}

+ (NSArray *) filePresenters;

- (NSString *) purposeIdentifier;

- (void) setPurposeIdentifier: (NSString *)ident;  // copy
                 
- (void)cancel;
                 
- (void)coordinateAccessWithIntents: (NSArray *)intents
                              queue: (NSOperationQueue *)queue
                         byAccessor: (GSAccessorCallbackHandler)accessor;
                 
- (void)coordinateReadingItemAtURL: (NSURL *)readingURL
                           options: (NSFileCoordinatorReadingOptions)readingOptions
                  writingItemAtURL: (NSURL *)writingURL
                           options: (NSFileCoordinatorWritingOptions)writingOptions
                             error: (NSError **)outError
                        byAccessor: (GSNoEscapeReadWriteHandler)readerWriter;
                 
- (void)coordinateReadingItemAtURL: (NSURL *)url
                           options: (NSFileCoordinatorReadingOptions)options
                             error: (NSError **)outError
                        byAccessor: (GSNoEscapeNewURLHandler)reader;
                 
- (void)coordinateWritingItemAtURL: (NSURL *)url
                           options: (NSFileCoordinatorWritingOptions)options error:(NSError **)outError
                        byAccessor: (GSNoEscapeNewURLHandler)writer;

- (void)coordinateWritingItemAtURL: (NSURL *)url1
                           options: (NSFileCoordinatorWritingOptions)options1
                  writingItemAtURL: (NSURL *)url2
                           options: (NSFileCoordinatorWritingOptions)options2
                             error: (NSError **)outError
                        byAccessor: (GSDualWriteURLCallbackHandler)writer;

- (void)itemAtURL: (NSURL *)oldURL didMoveToURL: (NSURL *)newURL;

- (void)itemAtURL: (NSURL *)oldURL willMoveToURL: (NSURL *)newURL; 

- (void)itemAtURL: (NSURL *)url didChangeUbiquityAttributes: (NSSet *)attributes;

- (void)prepareForReadingItemsAtURLs: (NSArray *)readingURLs
                             options: (NSFileCoordinatorReadingOptions)readingOptions
                  writingItemsAtURLs: (NSArray *)writingURLs
                             options: (NSFileCoordinatorWritingOptions)writingOptions
                               error: (NSError **)outError
                          byAccessor: (GSBatchAccessorCompositeBlock)batchAccessor;                 
@end
 
#endif
#endif
