/* Implementation of class NSExtensionItem
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: heron
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

#include <Foundation/NSExtensionItem.h>
#include <Foundation/NSAttributedString.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSArray.h>

@implementation NSExtensionItem

- (NSAttributedString *) attributedTitle
{
  return _attributedTitle;
}

- (void) setAttributedTitle: (NSAttributedString *)string
{
  ASSIGNCOPY(_attributedTitle, string);
}

- (NSAttributedString *) attributedContentText
{
  return _attributedContentText;
}

- (void) setAttributedContentText: (NSAttributedString *)string
{
  ASSIGNCOPY(_attributedContentText, string);
}
  
- (NSArray *) attachments
{
  return _attachments;
}

- (void) setAttachments: (NSArray *)attachments
{
  ASSIGNCOPY(_attachments, attachments);
}

- (NSDictionary *) userInfo
{
  return _userInfo;
}

- (void) setUserInfo: (NSDictionary *) userInfo
{
  ASSIGNCOPY(_userInfo, userInfo);
}
  
@end

NSString * const NSExtensionItemAttributedTitleKey = @"NSExtensionItemAttributedTitleKey";

NSString * const NSExtensionItemAttributedContentTextKey = @"NSExtensionItemAttributedContentTextKey";

NSString * const NSExtensionItemAttachmentsKey = @"NSExtensionItemAttachmentsKey";



