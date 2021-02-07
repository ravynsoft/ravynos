/* Definition of class NSExtensionContext
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Sun Nov 10 03:59:38 EST 2019

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

#ifndef _NSExtensionContext_h_GNUSTEP_BASE_INCLUDE
#define _NSExtensionContext_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

DEFINE_BLOCK_TYPE(GSExtensionContextReturningItemsCompletionHandler, void, BOOL);
DEFINE_BLOCK_TYPE(GSOpenURLCompletionHandler, void, BOOL);

@class NSError, NSArray, NSURL;

GS_EXPORT_CLASS
@interface NSExtensionContext : NSObject
{
  NSArray *_inputItems;
}

- (void) setInputItems: (NSArray *)inputItems;
- (NSArray *) inputItems;
  
- (void)completeRequestReturningItems: (NSArray *)items completionHandler: (GSExtensionContextReturningItemsCompletionHandler)completionHandler;

- (void)cancelRequestWithError:(NSError *)error;

- (void)openURL: (NSURL *)URL completionHandler: (GSOpenURLCompletionHandler)completionHandler;

@end

GS_EXPORT NSString* const NSExtensionItemsAndErrorsKey;

GS_EXPORT NSString* const NSExtensionHostWillEnterForegroundNotification;

GS_EXPORT NSString* const NSExtensionHostDidEnterBackgroundNotification;

GS_EXPORT NSString* const NSExtensionHostWillResignActiveNotification;

GS_EXPORT NSString* const NSExtensionHostDidBecomeActiveNotification;

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSExtensionContext_h_GNUSTEP_BASE_INCLUDE */

