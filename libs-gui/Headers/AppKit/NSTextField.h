/* 
   NSTextField.h

   Text field control class for text entry

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

#ifndef _GNUstep_H_NSTextField
#define _GNUstep_H_NSTextField
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSControl.h>
// For NSTextFieldBezelStyle
#import <AppKit/NSTextFieldCell.h>

@class NSNotification;
@class NSColor;
@class NSText;
@class NSCursor;

@protocol NSTextFieldDelegate <NSControlTextEditingDelegate>
@end

@interface NSTextField : NSControl
{
  // Attributes
  id _delegate;
  SEL _error_action;
  NSText *_text_object;
}

//
// Setting User Access to Text 
//
- (BOOL)isEditable;
- (BOOL)isSelectable;
- (void)setEditable:(BOOL)flag;
- (void)setSelectable:(BOOL)flag;

//
// Editing Text 
//
- (void)selectText:(id)sender;

//
// Setting Tab Key Behavior 
//
- (id)nextText;
- (id)previousText;
- (void)setNextText:(id)anObject;
- (void)setPreviousText:(id)anObject;

//
// Assigning a Delegate 
//
- (void)setDelegate:(id<NSTextFieldDelegate>)anObject;
- (id<NSTextFieldDelegate>)delegate;

//
// Modifying Graphic Attributes 
//
- (NSColor *)backgroundColor;
- (BOOL)drawsBackground;
- (BOOL)isBezeled;
- (BOOL)isBordered;
- (void)setBackgroundColor:(NSColor *)aColor;
- (void)setBezeled:(BOOL)flag;
- (void)setBordered:(BOOL)flag;
- (void)setDrawsBackground:(BOOL)flag;
- (void)setTextColor:(NSColor *)aColor;
- (NSColor *)textColor;

//
// Target and Action 
//
- (SEL)errorAction;
- (void)setErrorAction:(SEL)aSelector;

//
// Handling Events 
//
- (BOOL)acceptsFirstResponder;
- (void)textDidBeginEditing:(NSNotification *)aNotification;
- (void)textDidChange:(NSNotification *)aNotification;
- (void)textDidEndEditing:(NSNotification *)aNotification;
- (BOOL)textShouldBeginEditing:(NSText *)textObject;
- (BOOL)textShouldEndEditing:(NSText *)textObject;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
//
// Rich Text
//
- (void)setAllowsEditingTextAttributes:(BOOL)flag;
- (BOOL)allowsEditingTextAttributes;
- (void)setImportsGraphics:(BOOL)flag;
- (BOOL)importsGraphics;

- (void)setTitleWithMnemonic:(NSString *)aString;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (void)setBezelStyle:(NSTextFieldBezelStyle)style;
- (NSTextFieldBezelStyle)bezelStyle;
#endif 

@end

#endif // _GNUstep_H_NSTextField
