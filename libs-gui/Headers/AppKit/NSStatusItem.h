/*
   NSStatusItem.h

   The status item class

   Copyright (C) 2013 Free Software Foundation, Inc.

   Author:  Gregory Casamento <greg.casamento@gmail.com>
   Date: 2013

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

// FIXME: This class is currently a placeholder to allow compilation of
// apps which require NSStatusItem.  Currently there is not a clean,
// cross-platform way to implement this functionality.

#ifndef _GNUstep_H_NSStatusItem
#define _GNUstep_H_NSStatusItem
#import <GNUstepBase/GSVersionMacros.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSAttributedString;
@class NSString;
@class NSStatusBar;
@class NSView;
@class NSImage;
@class NSMenu;
@class NSMenuItem;

@interface NSStatusItem : NSObject
{
  @private
  NSMenuItem *_menuItem;
  NSStatusBar *_statusBar;
  NSView *_view;
  CGFloat _length;
  BOOL _highlightMode;
}

- (SEL) action;
- (NSAttributedString*) attributedTitle;
- (SEL) doubleAction;
- (void) drawStatusBarBackgroundInRect: (NSRect)rect withHighlight: (BOOL)flag; 
- (BOOL) highlightMode;
- (NSImage*) image;
- (BOOL) isEnabled;
- (CGFloat) length;
- (NSMenu*) menu;
- (void) popUpStatusItemMenu: (NSMenu*)menu;
- (NSInteger) sendActionOn: (NSInteger)mask;
- (void) setAction: (SEL)action;
- (void) setAttributedTitle: (NSAttributedString*)title;
- (void) setDoubleAction: (SEL)sel;
- (void) setEnabled: (BOOL)flag;
- (void) setHighlightMode: (BOOL)highlightMode;
- (void) setImage: (NSImage*)image;
- (void) setLength: (CGFloat)length;
- (void) setMenu: (NSMenu*)menu;
- (void) setTarget: (id)target;
- (void) setTitle: (NSString*)title;
- (void) setToolTip: (NSString*)toolTip;
- (void) setView: (NSView*)view;
- (NSStatusBar*) statusBar;
- (id) target;
- (NSString*) title;
- (NSString*) toolTip;
- (NSView*) view;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST) 
- (NSImage*) alternateImage;
- (void) setAlternateImage: (NSImage*)img;
#endif

@end

#endif // _GNUstep_H_NSStatusItem
