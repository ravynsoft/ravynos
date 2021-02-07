/* 
   NSTextFieldCell.h

   Cell class for the text field entry control

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   
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

#ifndef _GNUstep_H_NSTextFieldCell
#define _GNUstep_H_NSTextFieldCell
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSActionCell.h>

@class NSColor;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
typedef enum _NSTextFieldBezelStyle
{
	NSTextFieldSquareBezel = 0,
	NSTextFieldRoundedBezel
} NSTextFieldBezelStyle;
#endif 

@interface NSTextFieldCell : NSActionCell <NSCoding>
{
  // Attributes
  NSColor *_background_color;
  NSColor *_text_color;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
  NSTextFieldBezelStyle _bezelStyle;
#else
  unsigned int _bezelStyle;
#endif 

  // Think of the following ones as of two BOOL ivars
#define _textfieldcell_draws_background _cell.subclass_bool_one
#define _textfieldcell_placeholder_is_attributed_string _cell.subclass_bool_three
  id _placeholder;
}

//
// Modifying Graphic Attributes 
//
- (void)setTextColor:(NSColor *)aColor;
- (NSColor *)textColor;

- (void)setDrawsBackground:(BOOL)flag;
- (BOOL)drawsBackground;

- (void)setBackgroundColor:(NSColor *)aColor;
- (NSColor *)backgroundColor;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (void)setBezelStyle:(NSTextFieldBezelStyle)style;
- (NSTextFieldBezelStyle)bezelStyle;
#endif 

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void)setPlaceholderString:(NSString *)string;
- (NSString *)placeholderString;
- (void)setPlaceholderAttributedString:(NSAttributedString *)string;
- (NSAttributedString *)placeholderAttributedString;
#endif 

@end

// 
// Methods that are private GNUstep extensions
//
@interface NSTextFieldCell (PrivateMethods)

- (void) _drawBackgroundWithFrame: (NSRect)cellFrame 
                           inView: (NSView*)controlView;
@end

#endif // _GNUstep_H_NSTextFieldCell
