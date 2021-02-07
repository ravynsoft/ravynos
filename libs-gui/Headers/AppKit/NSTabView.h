/*
   NSTabView.h

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

#ifndef _GNUstep_H_NSTabView
#define _GNUstep_H_NSTabView
 
#import <AppKit/NSView.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSCell.h>

typedef enum {
  NSTopTabsBezelBorder, 
  NSLeftTabsBezelBorder,
  NSBottomTabsBezelBorder, 
  NSRightTabsBezelBorder,
  NSNoTabsBezelBorder,
  NSNoTabsLineBorder,
  NSNoTabsNoBorder
} NSTabViewType;

@class NSFont;
@class NSTabViewItem;

@interface NSTabView : NSView <NSCoding>
{
  NSMutableArray *_items;
  NSFont *_font;
  NSTabViewType _type;
  NSTabViewItem *_selected;
  BOOL _draws_background;
  BOOL _truncated_label;
  id _delegate;
  NSView *_original_nextKeyView;
  NSUInteger _selected_item;
}
- (void)addTabViewItem:(NSTabViewItem *)tabViewItem;
- (void)insertTabViewItem:(NSTabViewItem *)tabViewItem
		  atIndex:(NSInteger)index;
- (void)removeTabViewItem:(NSTabViewItem *)tabViewItem;
- (NSInteger)indexOfTabViewItem:(NSTabViewItem *)tabViewItem;
- (NSInteger)indexOfTabViewItemWithIdentifier:(id)identifier;
- (NSInteger)numberOfTabViewItems;

- (NSTabViewItem *)tabViewItemAtIndex:(NSInteger)index;
- (NSArray *)tabViewItems;

- (void)selectFirstTabViewItem:(id)sender;
- (void)selectLastTabViewItem:(id)sender;
- (void)selectNextTabViewItem:(id)sender;
- (void)selectPreviousTabViewItem:(id)sender;
- (void)selectTabViewItem:(NSTabViewItem *)tabViewItem;
- (void)selectTabViewItemAtIndex:(NSInteger)index;
- (void)selectTabViewItemWithIdentifier:(id)identifier;
- (void)takeSelectedTabViewItemFromSender:(id)sender;
- (NSTabViewItem*) selectedTabViewItem;

- (void)setFont:(NSFont *)font;
- (NSFont *)font;

- (void)setTabViewType:(NSTabViewType)tabViewType;
- (NSTabViewType)tabViewType;

- (void)setDrawsBackground:(BOOL)flag;
- (BOOL)drawsBackground;

- (void)setAllowsTruncatedLabels:(BOOL)allowTruncatedLabels;
- (BOOL)allowsTruncatedLabels;

- (void)setDelegate:(id)anObject;
- (id)delegate;

- (NSSize)minimumSize;
- (NSRect)contentRect;

- (NSTabViewItem *)tabViewItemAtPoint:(NSPoint)point;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (NSControlSize)controlSize;
- (NSControlTint)controlTint;
- (void)setControlSize:(NSControlSize)controlSize;
- (void)setControlTint:(NSControlTint)controlTint;
#endif

@end

@interface NSObject(NSTabViewDelegate)
- (BOOL)tabView:(NSTabView *)tabView shouldSelectTabViewItem:(NSTabViewItem *)tabViewItem;
- (void)tabView:(NSTabView *)tabView willSelectTabViewItem:(NSTabViewItem *)tabViewItem;
- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem;
- (void)tabViewDidChangeNumberOfTabViewItems:(NSTabView *)TabView;
@end

#endif // _GNUstep_H_NSTabView

/* Notifications */

