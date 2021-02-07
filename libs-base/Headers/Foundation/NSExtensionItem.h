/* Definition of class NSExtensionItem
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Sun Nov 10 03:59:46 EST 2019

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

#ifndef _NSExtensionItem_h_GNUSTEP_BASE_INCLUDE
#define _NSExtensionItem_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSAttributedString, NSArray, NSDictionary;

GS_EXPORT_CLASS
@interface NSExtensionItem : NSObject
{
  NSAttributedString *_attributedTitle;
  NSAttributedString *_attributedContentText;
  NSArray *_attachments;
  NSDictionary *_userInfo;
}
  
- (NSAttributedString *) attributedTitle;
- (void) setAttributedTitle: (NSAttributedString *)string;

- (NSAttributedString *) attributedContentText;
- (void) setAttributedContentText: (NSAttributedString *)string;
  
- (NSArray *) attachments;
- (void) setAttachments: (NSArray *)attachments;

- (NSDictionary *) userInfo;
- (void) setUserInfo: (NSDictionary *) userInfo;
  
@end

GS_EXPORT NSString * const NSExtensionItemAttributedTitleKey;

GS_EXPORT NSString * const NSExtensionItemAttributedContentTextKey;

GS_EXPORT NSString * const NSExtensionItemAttachmentsKey;

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSExtensionItem_h_GNUSTEP_BASE_INCLUDE */

