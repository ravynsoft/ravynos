/* Definition of class NSFontAssetRequest
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Tue Apr  7 08:06:56 EDT 2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSFontAssetRequest_h_GNUSTEP_GUI_INCLUDE
#define _NSFontAssetRequest_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSProgress.h>
#import <Foundation/NSError.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_13, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

enum {
  NSFontAssetRequestOptionUsesStandardUI = 1 << 0, // Use standard system UI for downloading.
};
typedef NSUInteger NSFontAssetRequestOptions;

DEFINE_BLOCK_TYPE(GSFontAssetCompletionHandler, BOOL, NSError*);

@interface NSFontAssetRequest : NSObject <NSProgressReporting>

- (instancetype) initWithFontDescriptors: (NSArray *)fontDescriptors
                                 options: (NSFontAssetRequestOptions)options;

- (NSArray *) downloadedFontDescriptors;

- (NSProgress *) progress;

- (void)downloadFontAssetsWithCompletionHandler: (GSFontAssetCompletionHandler)completionHandler;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSFontAssetRequest_h_GNUSTEP_GUI_INCLUDE */

