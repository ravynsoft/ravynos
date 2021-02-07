/* 
   NSColorPicking.h

   Protocols for picking colors

   Copyright (C) 1997 Free Software Foundation, Inc.

   Author:  Simon Frankau <sgf@frankau.demon.co.uk>
   Date: 1997
   
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

#ifndef _GNUstep_H_NSColorPicking
#define _GNUstep_H_NSColorPicking
#import <GNUstepBase/GSVersionMacros.h>

@class NSColor;
@class NSColorPanel;
@class NSView;
@class NSImage;
@class NSButtonCell;
@class NSColorList;

@protocol NSColorPickingCustom

//
// Getting the Mode
//
- (int)currentMode;
- (BOOL)supportsMode:(int)mode;

//
// Getting the view
//
- (NSView *)provideNewView:(BOOL)firstRequest;

//
// Setting the Current Color
//
- (void)setColor:(NSColor *)aColor;

@end

@protocol NSColorPickingDefault

//
// Initialize a Color Picker
//
- (id)initWithPickerMask:(int)mask
              colorPanel:(NSColorPanel *)colorPanel;

//
// Adding Button Images
//
- (void)insertNewButtonImage:(NSImage *)newImage 
                          in:(NSButtonCell *)newButtonCell;
- (NSImage *)provideNewButtonImage;

//
// Setting the Mode
//
- (void)setMode:(int)mode;

//
// Using Color Lists
//
- (void)attachColorList:(NSColorList *)aColorList;
- (void)detachColorList:(NSColorList *)aColorList;

//
// Showing Opacity Controls
//
- (void)alphaControlAddedOrRemoved:(id)sender;

//
// Responding to a Resized View
//
- (void)viewSizeChanged:(id)sender;

@end

#endif // _GNUstep_H_NSColorPicking
