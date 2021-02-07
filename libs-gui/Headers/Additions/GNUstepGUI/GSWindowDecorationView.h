/** <title>GSWindowDecorationView</title>

   Copyright (C) 2004 Free Software Foundation, Inc.

   Author: Alexander Malmberg <alexander@malmberg.org>
   Date: 2004-03-24

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

#ifndef _GNUstep_H_GSWindowDecorationView
#define _GNUstep_H_GSWindowDecorationView

#import <Foundation/NSGeometry.h>
#import <AppKit/NSView.h>

@class NSWindow;

// These are implemented as class methods on GSWindowDecorationView
@protocol GSWindowDecorator
- (id) newWindowDecorationViewWithFrame: (NSRect)frame
				 window: (NSWindow *)window;
- (NSRect) contentRectForFrameRect: (NSRect)aRect
       styleMask: (NSUInteger)aStyle;
- (NSRect) frameRectForContentRect: (NSRect)aRect
			 styleMask: (NSUInteger)aStyle;
- (CGFloat) minFrameWidthWithTitle: (NSString *)aTitle
                         styleMask: (NSUInteger)aStyle;
@end


/*
Abstract superclass for the top-level view in each window. This view is
responsible for managing window decorations. Concrete subclasses may do
this, either directly, or indirectly (by using the backend).
*/
@interface GSWindowDecorationView : NSView
{
  NSWindow *window; /* not retained */
  int windowNumber;
  NSRect contentRect;
  int inputState;
  BOOL documentEdited;
  BOOL hasMenu;
  BOOL hasToolbar;
}
+ (id<GSWindowDecorator>) windowDecorator;

- (id) initWithFrame: (NSRect)frame window: (NSWindow *)w;

- (NSRect) contentRectForFrameRect: (NSRect)aRect
			 styleMask: (NSUInteger)aStyle;
- (NSRect) frameRectForContentRect: (NSRect)aRect
			 styleMask: (NSUInteger)aStyle;

- (void) layout;
- (void) changeWindowHeight: (CGFloat)difference;

- (void) setBackgroundColor: (NSColor *)color;
- (void) setContentView: (NSView *)contentView;
- (void) setDocumentEdited: (BOOL)flag;
- (void) setInputState: (int)state;
- (void) setTitle: (NSString *)title;

/*
Called when the backend window is created or destroyed. When it's destroyed,
windowNumber will be 0.
*/
- (void) setWindowNumber: (int)windowNumber;			  

// Flags controlling if elements are present
- (void) setHasMenu: (BOOL) flag;
- (void) setHasToolbar: (BOOL) flag;
- (BOOL) hasMenu;
- (BOOL) hasToolbar;
@end


/* Manage window decorations by using the backend functions. This only works
 * on backends that can handle window decorations.
 */
@interface GSBackendWindowDecorationView : GSWindowDecorationView
@end


/*
Standard OPENSTEP-ish window decorations.
*/
@class NSButton;

@interface GSStandardWindowDecorationView : GSWindowDecorationView
{
  BOOL hasTitleBar, hasResizeBar, hasCloseButton, hasMiniaturizeButton;
  BOOL isTitled; //, hasToolbar, hasMenu;
  NSRect titleBarRect;
  NSRect resizeBarRect;
  NSRect closeButtonRect;
  NSRect miniaturizeButtonRect;

  NSButton *closeButton, *miniaturizeButton;
}
@end

#endif

