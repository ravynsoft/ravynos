/* Implementation of class NSItemProvider
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

#include <Foundation/NSItemProvider.h>
#include <Foundation/NSString.h>

@implementation NSItemProvider

- (instancetype) init
{
  return nil;
}

- (void) registerDataRepresentationForTypeIdentifier: (NSString *)typeIdentifier
                                          visibility: (NSItemProviderRepresentationVisibility)visibility
                                         loadHandler: (GSProgressHandler)loadHandler
{
}

- (void) registerFileRepresentationForTypeIdentifier: (NSString *)typeIdentifier
                                         fileOptions: (NSItemProviderFileOptions)fileOptions
                                          visibility: (NSItemProviderRepresentationVisibility)visibility
                                         loadHandler: (GSProgressURLBOOLHandler)loadHandler
{
}

- (NSArray *) registeredTypeIdentifiers
{
  return nil;
}

- (NSArray *) registeredTypeIdentifiersWithFileOptions: (NSItemProviderFileOptions)fileOptions
{
  return nil;
}

- (BOOL) hasItemConformingToTypeIdentifier: (NSString *)typeIdentifier
{
  return NO;
}

- (BOOL) hasRepresentationConformingToTypeIdentifier: (NSString *)typeIdentifier
                                         fileOptions: (NSItemProviderFileOptions)fileOptions
{
  return NO;
}

- (NSProgress *) loadDataRepresentationForTypeIdentifier: (NSString *)typeIdentifier
                                       completionHandler: (GSProviderCompletionHandler)completionHandler
{
  return nil;
}

- (NSProgress *) loadFileRepresentationForTypeIdentifier: (NSString *)typeIdentifier
                                       completionHandler: (GSProviderURLCompletionHandler)completionHandler
{
  return nil;
}

- (NSProgress *) loadInPlaceFileRepresentationForTypeIdentifier: (NSString *)typeIdentifier
                                              completionHandler: (GSProviderURLBOOLCompletionHandler)completionHandler
{
  return nil;
}

- (NSString *) suggestedName
{
  return nil;
}

- (void) setSuggestedName: (NSString *)suggestedName
{
}

- (instancetype) initWithObject: (id<NSItemProviderWriting>)object
{
  return nil;
}

- (void) registerObject: (id<NSItemProviderWriting>)object visibility: (NSItemProviderRepresentationVisibility)visibility
{
}

- (void) registerObjectOfClass: (Class<NSItemProviderWriting>)aClass  // NSItemProviderWriting conforming class...
                    visibility: (NSItemProviderRepresentationVisibility)visibility
                   loadHandler: (GSItemProviderWritingHandler)loadHandler
{
}

- (BOOL) canLoadObjectOfClass: (Class<NSItemProviderReading>)aClass
{
  return NO;
}

- (NSProgress *) loadObjectOfClass: (Class<NSItemProviderReading>)aClass // NSItemProviderReading conforming class...
                 completionHandler: (GSItemProviderReadingHandler)completionHandler
{
  return nil;
}

- (instancetype) initWithItem: (id<NSSecureCoding>)item typeIdentifier: (NSString *)typeIdentifier // designated init
{
  return nil;
}

- (instancetype) initWithContentsOfURL: (NSURL *)fileURL
{
  return nil;
}

- (void) registerItemForTypeIdentifier: (NSString *)typeIdentifier loadHandler: (NSItemProviderLoadHandler)loadHandler
{
}

- (void)loadItemForTypeIdentifier: (NSString *)typeIdentifier
                          options: (NSDictionary *)options
                completionHandler: (NSItemProviderCompletionHandler)completionHandler
{
}

- (instancetype) copyWithZone: (NSZone*)zone
{
  return nil;
}
@end

// Preview support
NSString * const NSItemProviderPreferredImageSizeKey = @"NSItemProviderPreferredImageSizeKey";

@implementation NSItemProvider (NSPreviewSupport)

- (NSItemProviderLoadHandler) previewImageHandler
{
  return (NSItemProviderLoadHandler)0;
}

- (void) setPreviewImageHandler: (NSItemProviderLoadHandler) previewImageHandler
{
}
  
- (void) loadPreviewImageWithOptions: (NSDictionary *)options
                   completionHandler: (NSItemProviderCompletionHandler)completionHandler
{
}

@end

NSString * const NSExtensionJavaScriptPreprocessingResultsKey = @"NSExtensionJavaScriptPreprocessingResultsKey"; 

NSString * const NSExtensionJavaScriptFinalizeArgumentKey = @"NSExtensionJavaScriptFinalizeArgumentKey";

NSString * const NSItemProviderErrorDomain = @"NSItemProviderErrorDomain" ;


