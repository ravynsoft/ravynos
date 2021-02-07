/* Implementation of class NSTabViewController
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

#import <Foundation/NSArray.h>

#import "AppKit/NSTabViewController.h"
#import "AppKit/NSTabViewItem.h"
#import "AppKit/NSTabView.h"

#import "GSFastEnumeration.h"

@implementation NSTabViewController
- (NSTabViewControllerTabStyle) tabStyle
{
  return _tabStyle;
}

- (void) setTabStyle: (NSTabViewControllerTabStyle)ts
{
  _tabStyle = ts;
}

- (NSTabView *) tabView
{
  return (NSTabView *)[self view];
}

- (void) setTabView: (NSTabView *)tv
{
  [self setView: tv];
  [tv setDelegate: self];
}

- (NSViewControllerTransitionOptions) transitionOptions
{
  return _transitionOptions;
}

- (void) setTransitionOptions: (NSViewControllerTransitionOptions)options
{
  _transitionOptions = options;
}

- (BOOL) canPropagateSelectedChildViewControllerTitle
{
  return _canPropagateSelectedChildViewControllerTitle;
}

- (void) setCanPropagateSelectedChildViewControllerTitle: (BOOL)flag
{
  _canPropagateSelectedChildViewControllerTitle = flag;
}

// Managing tabViewItems...
- (NSArray *) tabViewItems
{
  return [[self tabView] tabViewItems];
}

- (void) setTabViewItems: (NSArray *)items
{
  FOR_IN(NSTabViewItem*, item, items)
    [[self tabView] addTabViewItem: item];
  END_FOR_IN(items);
}

- (NSTabViewItem *) tabViewItemForViewController: (NSViewController *)controller
{
  NSArray *tabViewItems = [[self tabView] tabViewItems];
  FOR_IN(NSTabViewItem*, tvi, tabViewItems)
    if ([tvi viewController] == controller)
      {
        return tvi;
      }
  END_FOR_IN(tabViewItems);
  return nil;
}

- (void) addTabViewItem: (NSTabViewItem *)item
{
  [[self tabView] addTabViewItem: item];
}

- (void) insertTabViewItem: (NSTabViewItem *)item 
                   atIndex: (NSInteger)index
{
  [[self tabView] insertTabViewItem: item atIndex: index];
}

- (void) removeTabViewItem: (NSTabViewItem *)item
{
  [[self tabView] removeTabViewItem: item];
}

- (NSInteger) selectedTabViewItemIndex
{
  return [[self tabView] indexOfTabViewItem: [[self tabView] selectedTabViewItem]];
}

- (void) setSelectedTabViewItemIndex: (NSInteger)idx
{
  [[self tabView] selectTabViewItemAtIndex: idx];
  if (_canPropagateSelectedChildViewControllerTitle)
    {
      NSString *title = [[[self tabView] tabViewItems] objectAtIndex: idx];
      if (title != nil)
        {
          [self setTitle: title];
        }
    }
}

// Responding to tabview actions...
- (BOOL)tabView:(NSTabView *)tabView 
  shouldSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
  return YES;
}

- (void)tabView:(NSTabView *)tabView
  willSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
  // not implemented
}

- (void)tabView:(NSTabView *)tabView 
  didSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
  // not implemented
}

// Responding to toolbar actions...
- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar 
     itemForItemIdentifier:(NSToolbarItemIdentifier)itemIdentifier
     willBeInsertedIntoToolbar:(BOOL)flag
{
  return nil; 
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
  return [NSArray array];
}

- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar
{
  return [NSArray array];
}

// NSCoding
- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super initWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"NSTabView"])
        {
          NSTabView *tv = [coder decodeObjectForKey: @"NSTabView"];
          [self setTabView: tv];

          // Currently we only support the tabs being on the top or the bottom.
          // The rendering code doesn't support anything outside of these two
          // cases.  Here we force the use of the top case, when it is outside
          // of either of the cases we handle... this is temporary.  FIXME
          if ([tv tabViewType] != NSTopTabsBezelBorder &&
              [tv tabViewType] != NSBottomTabsBezelBorder)
            {
              [tv setTabViewType: NSTopTabsBezelBorder];
            }
        }

      if ([coder containsValueForKey: @"NSTabViewControllerCanPropagateSelectedChildViewControllerTitle"])
        {
          BOOL flag = [coder decodeBoolForKey: @"NSTabViewControllerCanPropagateSelectedChildViewControllerTitle"];
          [self setCanPropagateSelectedChildViewControllerTitle: flag];
        }
    }
  else
    {
      BOOL flag;
      [self setTabView: [coder decodeObject]]; // get tabview...
      [coder decodeValueOfObjCType: @encode(BOOL)
                                at: &flag];
      [self setCanPropagateSelectedChildViewControllerTitle: flag];
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      NSTabView *tv = [self tabView];
      [coder encodeObject: tv forKey: @"NSTabView"];
      [coder encodeBool: [self canPropagateSelectedChildViewControllerTitle]
                 forKey: @"NSTabViewControllerCanPropagateSelectedChildViewControllerTitle"];
    }
  else
    {
      BOOL flag = [self canPropagateSelectedChildViewControllerTitle];
      [coder encodeObject: [self tabView]]; // get tabview...
      [coder encodeValueOfObjCType: @encode(BOOL)
                                at: &flag];
    }
}
@end

