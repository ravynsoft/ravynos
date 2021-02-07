/* Implementation of class NSExtensionContext
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

#include <Foundation/NSExtensionContext.h>
#include <Foundation/NSError.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSURL.h>
#include <Foundation/NSString.h>

@implementation NSExtensionContext

- (void) setInputItems: (NSArray *)inputItems
{
  ASSIGNCOPY(_inputItems, inputItems);
}

- (NSArray *) inputItems
{
  return _inputItems;
}
  
- (void)completeRequestReturningItems: (NSArray *)items completionHandler: (GSExtensionContextReturningItemsCompletionHandler)completionHandler
{
}

- (void)cancelRequestWithError:(NSError *)error
{
}

- (void)openURL: (NSURL *)URL completionHandler: (GSOpenURLCompletionHandler)completionHandler
{
}

@end

NSString* const NSExtensionItemsAndErrorsKey = @"NSExtensionItemsAndErrorsKey";

NSString* const NSExtensionHostWillEnterForegroundNotification = @"NSExtensionHostWillEnterForegroundNotification";

NSString* const NSExtensionHostDidEnterBackgroundNotification = @"NSExtensionHostDidEnterBackgroundNotification";

NSString* const NSExtensionHostWillResignActiveNotification = @"NSExtensionHostWillResignActiveNotification";

NSString* const NSExtensionHostDidBecomeActiveNotification = @"NSExtensionHostDidBecomeActiveNotification";

