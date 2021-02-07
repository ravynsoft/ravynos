/* Definition of class NSItemProvider
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: heron
   Date: Sun Nov 10 04:00:17 EST 2019

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

#ifndef _NSItemProvider_h_GNUSTEP_BASE_INCLUDE
#define _NSItemProvider_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSItemProvider, NSProgress, NSData, NSError, NSURL, NSProgress, NSDictionary;;

DEFINE_BLOCK_TYPE(GSProviderCompletionHandler, void, NSData*, NSError**);
DEFINE_BLOCK_TYPE(GSProviderURLCompletionHandler, void, NSURL*, NSError**);
DEFINE_BLOCK_TYPE(GSProviderURLBOOLCompletionHandler, void, NSURL*, BOOL, NSError**);
DEFINE_BLOCK_TYPE(GSProgressHandler, NSProgress*, GSProviderCompletionHandler);
DEFINE_BLOCK_TYPE(GSProgressURLBOOLHandler, NSProgress*, GSProviderURLBOOLCompletionHandler);

DEFINE_BLOCK_TYPE(GSItemProviderWritingHandler, id, NSError**);
DEFINE_BLOCK_TYPE(GSItemProviderReadingHandler, id, NSError**);
DEFINE_BLOCK_TYPE(GSProgressItemProviderWritingLoadHandler, NSProgress*, GSItemProviderWritingHandler);
DEFINE_BLOCK_TYPE(GSProgressItemProviderReadingLoadHandler, NSProgress*, GSItemProviderReadingHandler);

DEFINE_BLOCK_TYPE(NSItemProviderCompletionHandler, void, id, NSError**);
DEFINE_BLOCK_TYPE(NSItemProviderLoadHandler, NSItemProviderCompletionHandler, Class, NSDictionary*); 

enum
{
    NSItemProviderRepresentationVisibilityAll = 0,
    NSItemProviderRepresentationVisibilityTeam = 1,
    NSItemProviderRepresentationVisibilityGroup = 2 ,
    NSItemProviderRepresentationVisibilityOwnProcess = 3,
}; 
typedef NSInteger NSItemProviderRepresentationVisibility;

enum
{
    NSItemProviderFileOptionOpenInPlace = 1,
}; 
typedef NSInteger NSItemProviderFileOptions;

@protocol NSItemProviderWriting <NSObject>

- (NSArray *) writableTypeIdentifiersForItemProvider;
- (void) setWritableTypeIdentifiersForItemProvider: (NSArray *)anArray;
  
+ (NSItemProviderRepresentationVisibility) itemProviderVisibilityForRepresentationWithTypeIdentifier: (NSString *)typeIdentifier;

- (NSItemProviderRepresentationVisibility) itemProviderVisibilityForRepresentationWithTypeIdentifier: (NSString *)typeIdentifier;

- (NSProgress *) loadDataWithTypeIdentifier: (NSString *)typeIdentifier
           forItemProviderCompletionHandler: (GSProviderCompletionHandler)completionHandler;

@end


@protocol NSItemProviderReading <NSObject>

- (NSArray *) readableTypeIdentifiersForItemProvider;
- (void) setReadableTypeIdentifiersForItemProvider: (NSArray *)array;

+ (instancetype) objectWithItemProviderData: (NSData *)data
                             typeIdentifier: (NSString *)typeIdentifier
                                      error: (NSError **)outError;

@end

GS_EXPORT_CLASS
@interface NSItemProvider : NSObject <NSCopying>

- (instancetype) init;

- (void) registerDataRepresentationForTypeIdentifier: (NSString *)typeIdentifier
                                          visibility: (NSItemProviderRepresentationVisibility)visibility
                                         loadHandler: (GSProgressHandler)loadHandler;

- (void) registerFileRepresentationForTypeIdentifier: (NSString *)typeIdentifier
                                         fileOptions: (NSItemProviderFileOptions)fileOptions
                                          visibility: (NSItemProviderRepresentationVisibility)visibility
                                         loadHandler: (GSProgressURLBOOLHandler)loadHandler;

- (NSArray *) registeredTypeIdentifiers;

- (NSArray *) registeredTypeIdentifiersWithFileOptions: (NSItemProviderFileOptions)fileOptions;

- (BOOL) hasItemConformingToTypeIdentifier: (NSString *)typeIdentifier;

- (BOOL) hasRepresentationConformingToTypeIdentifier: (NSString *)typeIdentifier
                                         fileOptions: (NSItemProviderFileOptions)fileOptions;

- (NSProgress *) loadDataRepresentationForTypeIdentifier: (NSString *)typeIdentifier
                                       completionHandler: (GSProviderCompletionHandler)completionHandler;

- (NSProgress *) loadFileRepresentationForTypeIdentifier: (NSString *)typeIdentifier
                                       completionHandler: (GSProviderURLCompletionHandler)completionHandler;

- (NSProgress *) loadInPlaceFileRepresentationForTypeIdentifier: (NSString *)typeIdentifier
                                              completionHandler: (GSProviderURLBOOLCompletionHandler)completionHandler;

- (NSString *) suggestedName;
- (void) setSuggestedName: (NSString *)suggestedName;

- (instancetype) initWithObject: (id<NSItemProviderWriting>)object;

- (void) registerObject: (id<NSItemProviderWriting>)object visibility: (NSItemProviderRepresentationVisibility)visibility;

- (void) registerObjectOfClass: (Class<NSItemProviderWriting>)aClass  // NSItemProviderWriting conforming class...
                    visibility: (NSItemProviderRepresentationVisibility)visibility
                   loadHandler: (GSItemProviderWritingHandler)loadHandler;

- (BOOL) canLoadObjectOfClass: (Class<NSItemProviderReading>)aClass;

- (NSProgress *) loadObjectOfClass: (Class<NSItemProviderReading>)aClass // NSItemProviderReading conforming class...
                 completionHandler: (GSItemProviderReadingHandler)completionHandler;

- (instancetype) initWithItem: (id<NSSecureCoding>)item typeIdentifier: (NSString *)typeIdentifier; // designated init
- (instancetype) initWithContentsOfURL: (NSURL *)fileURL;

- (void) registerItemForTypeIdentifier: (NSString *)typeIdentifier loadHandler: (NSItemProviderLoadHandler)loadHandler;

- (void)loadItemForTypeIdentifier: (NSString *)typeIdentifier
                          options: (NSDictionary *)options
                completionHandler: (NSItemProviderCompletionHandler)completionHandler;
@end

// Preview support
GS_EXPORT NSString * const NSItemProviderPreferredImageSizeKey;

@interface NSItemProvider (NSPreviewSupport)

- (NSItemProviderLoadHandler) previewImageHandler;
- (void) setPreviewImageHandler: (NSItemProviderLoadHandler) previewImageHandler;
  
- (void) loadPreviewImageWithOptions: (NSDictionary *)options
                   completionHandler: (NSItemProviderCompletionHandler)completionHandler;

@end

GS_EXPORT NSString * const NSExtensionJavaScriptPreprocessingResultsKey; 

GS_EXPORT NSString * const NSExtensionJavaScriptFinalizeArgumentKey;

GS_EXPORT NSString * const NSItemProviderErrorDomain;

enum {
  NSItemProviderUnknownError                                      = -1,
  NSItemProviderItemUnavailableError                              = -1000,
  NSItemProviderUnexpectedValueClassError                         = -1100,
  NSItemProviderUnavailableCoercionError                          = -1200 
}; 
typedef NSInteger NSItemProviderErrorCode; 


#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSItemProvider_h_GNUSTEP_BASE_INCLUDE */

