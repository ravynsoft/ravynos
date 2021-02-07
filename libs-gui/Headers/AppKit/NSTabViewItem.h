/*
   NSTabViewItem.h
   
   Copyright (C) 1996 Free Software Foundation, Inc.
  
   Author:  Michael Hanni <mhanni@sprintmail.com>
   Date: 1999
  
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

#ifndef _GNUstep_H_NSTabViewItem
#define _GNUstep_H_NSTabViewItem

#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>

typedef enum {
  NSSelectedTab = 0,
  NSBackgroundTab,
  NSPressedTab
} NSTabState;

@class NSString;
@class NSColor;
@class NSTabView;
@class NSView;
@class NSViewController;

@interface NSTabViewItem : NSObject <NSCoding>
{
  id _ident;
  NSString *_label;
  NSView *_view;
  NSColor *_color;
  NSTabState _state;
  NSView *_first_responder;
  NSTabView *_tabview;
  NSRect _rect; // cached
  NSString *_toolTip;
  NSViewController *_viewController;
}

- (id) initWithIdentifier:(id)identifier;

- (void)setIdentifier:(id)identifier;
- (id)identifier;

- (void)setLabel:(NSString *)label;
- (NSString *)label;
- (NSSize)sizeOfLabel:(BOOL)shouldTruncateLabel;

- (void)setView:(NSView *)view;
- (NSView *)view;

- (void)setColor:(NSColor *)color;
- (NSColor *)color;

- (NSTabState)tabState;
- (NSTabView *)tabView;

- (void)setInitialFirstResponder:(NSView *)view;
- (id)initialFirstResponder;

- (void)drawLabel:(BOOL)shouldTruncateLabel
           inRect:(NSRect)tabRect;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)
- (NSViewController *) viewController;
- (void) setViewController: (NSViewController *)vc;

+ (instancetype) tabViewItemWithViewController: (NSViewController *)vc;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (NSString *)toolTip;
- (void)setToolTip:(NSString *)newToolTip;
#endif
@end

@interface NSTabViewItem (GNUstep)

// Non-spec
- (void)_setTabState:(NSTabState)tabState;
- (void)_setTabView:(NSTabView *)tabView;
- (NSRect) _tabRect;
- (NSString*)_truncatedLabel;
@end

#endif // _GNUstep_H_NSTabViewItem

