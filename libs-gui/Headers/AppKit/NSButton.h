/*
   NSButton.h

   The button class

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
	    Ovidiu Predescu <ovidiu@net-community.com>
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

#ifndef _GNUstep_H_NSButton
#define _GNUstep_H_NSButton
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSControl.h>
#import <AppKit/NSButtonCell.h>

@class NSAttributedString;
@class NSString;
@class NSEvent;

@interface NSButton : NSControl
{
  // Attributes
}

//
// Setting the Button Type 
//
- (void)setButtonType:(NSButtonType)aType;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)setHighlightsBy:(NSInteger)aType;
- (void)setShowsStateBy:(NSInteger)aType;
#endif

//
// Setting the State 
//
- (void)setState:(NSInteger)value;
- (NSInteger)state;
- (BOOL)allowsMixedState;
- (void)setAllowsMixedState: (BOOL)flag;
- (void)setNextState;

//
// Setting the Repeat Interval 
//
- (void)getPeriodicDelay:(float *)delay
		interval:(float *)interval;
- (void)setPeriodicDelay:(float)delay
		interval:(float)interval;

//
// Setting the Titles 
//
- (NSString *)alternateTitle;
- (void)setAlternateTitle:(NSString *)aString;
- (void)setTitle:(NSString *)aString;
- (NSString *)title;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (NSAttributedString *)attributedAlternateTitle;
- (NSAttributedString *)attributedTitle;
- (void)setAttributedAlternateTitle:(NSAttributedString *)aString;
- (void)setAttributedTitle:(NSAttributedString *)aString;
- (void)setTitleWithMnemonic:(NSString *)aString;
#endif

//
// Setting the Images 
//
- (NSImage *)alternateImage;
- (NSImage *)image;
- (NSCellImagePosition)imagePosition;
- (void)setAlternateImage:(NSImage *)anImage;
- (void)setImage:(NSImage *)anImage;
- (void)setImagePosition:(NSCellImagePosition)aPosition;

//
// Modifying Graphic Attributes 
//
- (BOOL)isBordered;
- (BOOL)isTransparent;
- (void)setBordered:(BOOL)flag;
- (void)setTransparent:(BOOL)flag;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (NSBezelStyle)bezelStyle;
- (void)setBezelStyle:(NSBezelStyle)bezelStyle;
- (void)setShowsBorderOnlyWhileMouseInside:(BOOL)show;
- (BOOL)showsBorderOnlyWhileMouseInside;
#endif

//
// Displaying 
//
- (void)highlight:(BOOL)flag;

//
// Setting the Key Equivalent 
//
- (NSString *)keyEquivalent;
- (NSUInteger)keyEquivalentModifierMask;
- (void)setKeyEquivalent:(NSString *)aKeyEquivalent;
- (void)setKeyEquivalentModifierMask:(NSUInteger)mask;

//
// Handling Events and Action Messages 
//
- (BOOL)performKeyEquivalent:(NSEvent *)anEvent;

//
// Sound
//
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)setSound:(NSSound *)aSound;
- (NSSound *)sound;
#endif

@end

#endif // _GNUstep_H_NSButton
