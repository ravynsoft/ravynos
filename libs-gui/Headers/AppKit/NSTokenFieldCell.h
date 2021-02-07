/* 
   NSTokenFieldCell.h

   Cell class for the token field entry control

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

#ifndef _GNUstep_H_NSTokenFieldCell
#define _GNUstep_H_NSTokenFieldCell
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSDate.h>
#import <AppKit/NSTextFieldCell.h>

@class NSCharacterSet;

typedef enum _NSTokenStyle
{  
  NSDefaultTokenStyle = 0,
  NSPlainTextTokenStyle,
  NSRoundedTokenStyle
} NSTokenStyle;


@interface NSTokenFieldCell : NSTextFieldCell <NSCoding>
{
  NSTokenStyle tokenStyle;
  NSTimeInterval completionDelay;
  NSCharacterSet *tokenizingCharacterSet;
}

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
@end

#endif // _GNUstep_H_NSTokenFieldCell
