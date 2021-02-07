/* 
   <title>GSToolbarView.h</title>

   <abstract>The private toolbar class which draws the actual toolbar.</abstract>
   
   Copyright (C) 2002 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>,
            Fabien Vallon <fabien.vallon@fr.alcove.com>,
	    Quentin Mathe <qmathe@club-internet.fr>
   Date: May 2002
   
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

#ifndef _GSToolbarView_h_INCLUDE
#define _GSToolbarView_h_INCLUDE

#import <Foundation/NSGeometry.h>
#import <AppKit/NSView.h>
#import <AppKit/NSColor.h>

@class NSMutableArray;
@class NSClipView;
@class NSToolbar;
@class NSToolbarItem;
@class GSToolbarClippedItemsButton;

typedef enum {
  GSToolbarViewNoBorder = 0,
  GSToolbarViewRightBorder = 2,
  GSToolbarViewLeftBorder = 4,
  GSToolbarViewTopBorder = 8,
  GSToolbarViewBottomBorder = 16
} GSToolbarViewBorder;

@interface GSToolbarView : NSView
{
  NSToolbar *_toolbar;
  NSClipView *_clipView;
  GSToolbarClippedItemsButton *_clippedItemsMark;
  unsigned int _borderMask;
  NSRect _rectAvailable;
  CGFloat _heightFromLayout;
}

+ (NSUInteger) draggedItemIndex;
+ (void) setDraggedItemIndex:(NSUInteger)sourceIndex;

- (id) initWithFrame: (NSRect)frame;

// Accessors
- (NSToolbar *) toolbar;
- (void) setToolbar: (NSToolbar *)toolbar;
- (unsigned int) borderMask;
- (void) setBorderMask: (unsigned int)borderMask;

@end

// Toolbar related NSColor methods
@interface NSColor (GSToolbarViewAdditions)
+ (NSColor *) toolbarBackgroundColor;
+ (NSColor *) toolbarBorderColor;
@end

#endif
