/* Definition of class NSTabViewController
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: 23-07-2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSTabViewController_h_GNUSTEP_GUI_INCLUDE
#define _NSTabViewController_h_GNUSTEP_GUI_INCLUDE

#import <AppKit/NSViewController.h>
#import <AppKit/NSToolbar.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSArray, NSTabViewItem, NSTabView, NSMutableArray;

enum
{
   NSTabViewControllerTabStyleSegmentedControlOnTop,
   NSTabViewControllerTabStyleSegmentedControlOnBottom,
   NSTabViewControllerTabStyleToolbar,
   NSTabViewControllerTabStyleUnspecified
};
typedef NSUInteger NSTabViewControllerTabStyle;
  
@interface NSTabViewController : NSViewController
{
  NSTabViewControllerTabStyle _tabStyle;
  NSViewControllerTransitionOptions _transitionOptions;
  BOOL _canPropagateSelectedChildViewControllerTitle;
}
  
- (NSTabViewControllerTabStyle) tabStyle;
- (void) setTabStyle: (NSTabViewControllerTabStyle)ts;

- (NSTabView *) tabView;
- (void) setTabView: (NSTabView *)tv;

- (NSViewControllerTransitionOptions) transitionOptions;
- (void) setTransitionOptions: (NSViewControllerTransitionOptions)options;

- (BOOL) canPropagateSelectedChildViewControllerTitle;
- (void) setCanPropagateSelectedChildViewControllerTitle: (BOOL)flag;

// Managing tabViewItems...
- (NSArray *) tabViewItems;
- (void) setTabViewItems: (NSArray *)items;
- (NSTabViewItem *) tabViewItemForViewController: (NSViewController *)controller;
- (void) addTabViewItem: (NSTabViewItem *)item;
- (void) insertTabViewItem: (NSTabViewItem *)item 
                   atIndex: (NSInteger)index;
- (void) removeTabViewItem: (NSTabViewItem *)item;
- (NSInteger) selectedTabViewItemIndex;
- (void) setSelectedTabViewItemIndex: (NSInteger)idx;

// Responding to tabview actions...
- (BOOL)tabView:(NSTabView *)tabView 
  shouldSelectTabViewItem:(NSTabViewItem *)tabViewItem;
- (void)tabView:(NSTabView *)tabView
  willSelectTabViewItem:(NSTabViewItem *)tabViewItem;
- (void)tabView:(NSTabView *)tabView 
  didSelectTabViewItem:(NSTabViewItem *)tabViewItem;

// Responding to toolbar actions...
- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar 
     itemForItemIdentifier:(NSToolbarItemIdentifier)itemIdentifier 
 willBeInsertedIntoToolbar:(BOOL)flag;
- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar;
- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar;
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSTabViewController_h_GNUSTEP_GUI_INCLUDE */

