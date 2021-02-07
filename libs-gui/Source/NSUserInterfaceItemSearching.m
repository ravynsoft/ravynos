/* Implementation of class NSUserInterfaceItemSearching
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Tue Apr  7 08:11:46 EDT 2020

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

#import <Foundation/NSObject.h>
#import <Foundation/NSCoder.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSUserInterfaceItemSearching.h"

@implementation NSApplication (NSUserInterfaceItemSearching)

- (void) registerUserInterfaceItemSearchHandler: (id<NSUserInterfaceItemSearching>)handler
{
}

- (void) unregisterUserInterfaceItemSearchHandler: (id<NSUserInterfaceItemSearching>)handler
{
}

- (BOOL) searchString:(NSString *)searchString
         inUserInterfaceItemString:(NSString *)stringToSearch
         searchRange:(NSRange)searchRange
         foundRange:(NSRange *)foundRange
{
  return NO;
}

@end

