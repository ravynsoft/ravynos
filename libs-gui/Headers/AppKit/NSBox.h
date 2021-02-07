/* 
   NSBox.h

   Simple box view that can display a border and title

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

#ifndef _GNUstep_H_NSBox
#define _GNUstep_H_NSBox
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSView.h>

@class NSString;
@class NSColor;
@class NSFont;

/** Title positioning of an NSBox:
 * <list>
 *  <item>NSNoTitle</item>
 *  <item>NSAboveTop</item>
 *  <item>NSAtTop</item>
 *  <item>NSBelowTop</item>
 *  <item>NSAboveBottom</item>
 *  <item>NSAtBottom</item>
 *  <item>NSBelowBottom</item>
 * </list>
 */
typedef enum _NSTitlePosition {
  NSNoTitle,
  NSAboveTop,
  NSAtTop,
  NSBelowTop,
  NSAboveBottom,
  NSAtBottom,
  NSBelowBottom
} NSTitlePosition;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
typedef enum _NSBoxType
{
  NSBoxPrimary=0,
  NSBoxSecondary,
  NSBoxSeparator,
  NSBoxOldStyle
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
  , NSBoxCustom
#endif
} NSBoxType;
#endif

@interface NSBox : NSView <NSCoding>
{
  // Attributes
  id _cell;
  id _content_view;
  NSSize _offsets;
  NSRect _border_rect;
  NSRect _title_rect;
  NSBorderType _border_type;
  NSTitlePosition _title_position;
  NSBoxType _box_type;
  // Only used when the type is NSBoxCustom
  NSColor *_fill_color;
  NSColor *_border_color;
  CGFloat _border_width;
  CGFloat _corner_radius;
  BOOL _transparent;
}

//
// Getting and Modifying the Border and Title 
//
- (NSRect)borderRect;
- (NSBorderType)borderType;
- (void)setBorderType:(NSBorderType)aType;
- (void)setTitle:(NSString *)aString;
- (void)setTitleFont:(NSFont *)fontObj;
- (void)setTitlePosition:(NSTitlePosition)aPosition;
- (NSString *)title;
- (id)titleCell;
- (NSFont *)titleFont;
- (NSTitlePosition)titlePosition;
- (NSRect)titleRect;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)setTitleWithMnemonic:(NSString *)aString;
- (NSBoxType)boxType;
- (void)setBoxType:(NSBoxType)aType;
#endif

//
// Setting and Placing the Content View 
//
- (id)contentView;
- (NSSize)contentViewMargins;
- (void)setContentView:(NSView *)aView;
- (void)setContentViewMargins:(NSSize)offsetSize;

//
// Resizing the Box 
//
- (void)setFrameFromContentFrame:(NSRect)contentFrame;
- (void)sizeToFit;

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
-(NSSize) minimumSize;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (NSColor*)fillColor;
- (void)setFillColor:(NSColor*)newFillColor;
- (NSColor*)borderColor;
- (void)setBorderColor:(NSColor*)newBorderColor;
- (CGFloat)borderWidth;
- (void)setBorderWidth:(CGFloat)borderWidth;
- (CGFloat)cornerRadius;
- (void)setCornerRadius:(CGFloat)cornerRadius;
- (BOOL)isTransparent;
- (void)setTransparent:(BOOL)transparent;
#endif
@end

#endif // _GNUstep_H_NSBox

