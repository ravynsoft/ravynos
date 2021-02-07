/* 
   NSTokenField.h

   Token field control class for text entry

   Copyright (C) 2008 Free Software Foundation, Inc.

   Author: Gregory Casamento <greg.casamento@gmail.com>
   Date: 2008
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/ 

#ifndef _GNUstep_H_NSTokenField
#define _GNUstep_H_NSTokenField
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSDate.h>
#import <AppKit/NSTextField.h>
#import <AppKit/NSTokenFieldCell.h>

@class NSCharacterSet;

@interface NSTokenField : NSTextField
// Style...
- (NSTokenStyle)tokenStyle;
- (void)setTokenStyle:(NSTokenStyle)style;

// Completion delay...
+ (NSTimeInterval)defaultCompletionDelay;
- (NSTimeInterval)completionDelay;
- (void)setCompletionDelay:(NSTimeInterval)delay;

// Character set...
+ (NSCharacterSet *)defaultTokenizingCharacterSet;
- (void)setTokenizingCharacterSet:(NSCharacterSet *)characterSet;
- (NSCharacterSet *)tokenizingCharacterSet;
@end;

#endif // _GNUstep_H_NSTokenField
