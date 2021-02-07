/* -*-objc-*-
   NSDrawer.h

   The drawer class

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author:  Fred Kiefer <FredKiefer@gmx.de>
   Date: 2001
   
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

#ifndef _GNUstep_H_NSDrawer
#define _GNUstep_H_NSDrawer
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSGeometry.h>
#import <AppKit/NSResponder.h>

@class NSWindow;
@class NSView;
@class NSNotification;

enum {
  NSDrawerClosedState  = 0,
  NSDrawerOpeningState = 1,
  NSDrawerOpenState    = 2,
  NSDrawerClosingState = 3
};

@interface NSDrawer : NSResponder
{
  // Attributes
  id _delegate;
  id _drawerWindow;
  NSRectEdge _preferredEdge;
  NSRectEdge _currentEdge;
  NSSize _maxContentSize;
  NSSize _minContentSize;
  NSSize _contentSize;
  CGFloat _leadingOffset;
  CGFloat _trailingOffset;
  NSInteger _state;
}

// Creation
- (id) initWithContentSize: (NSSize)contentSize 
	     preferredEdge: (NSRectEdge)edge;

// Opening and Closing
- (void) close;
- (void) close: (id)sender;
- (void) open;
- (void) open: (id)sender;
- (void) openOnEdge: (NSRectEdge)edge;
- (void) toggle: (id)sender;

// Managing Size
- (NSSize) contentSize;
- (CGFloat) leadingOffset;
- (NSSize) maxContentSize;
- (NSSize) minContentSize;
- (void) setContentSize: (NSSize)size;
- (void) setLeadingOffset: (CGFloat)offset;
- (void) setMaxContentSize: (NSSize)size;
- (void) setMinContentSize: (NSSize)size;
- (void) setTrailingOffset: (CGFloat)offset;
- (CGFloat) trailingOffset;

// Managing Edge
- (NSRectEdge) edge;
- (NSRectEdge) preferredEdge;
- (void) setPreferredEdge: (NSRectEdge)preferredEdge;

// Managing Views
- (NSView *) contentView;
- (NSWindow *) parentWindow;
- (void) setContentView: (NSView *)aView;
- (void) setParentWindow: (NSWindow *)parent;
 
// Delegation and State
- (id) delegate;
- (void) setDelegate: (id)anObject;
- (NSInteger) state;

@end

@interface NSDrawerDelegate
- (BOOL) drawerShouldClose: (NSDrawer *)sender;
- (BOOL) drawerShouldOpen: (NSDrawer *)sender;
- (NSSize) drawerWillResizeContents: (NSDrawer *)sender 
			    toSize: (NSSize)contentSize;
- (void) drawerDidClose: (NSNotification *)notification;
- (void) drawerDidOpen: (NSNotification *)notification;
- (void) drawerWillClose: (NSNotification *)notification;
- (void) drawerWillOpen: (NSNotification *)notification;
@end

// Notifications
APPKIT_EXPORT NSString *NSDrawerDidCloseNotification;
APPKIT_EXPORT NSString *NSDrawerDidOpenNotification;
APPKIT_EXPORT NSString *NSDrawerWillCloseNotification;
APPKIT_EXPORT NSString *NSDrawerWillOpenNotification;

#endif // _GNUstep_H_NSDrawer

