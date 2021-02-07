/* Implementation of class NSSplitViewController
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Mon 20 Jul 2020 12:55:02 AM EDT

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
#import <Foundation/NSArchiver.h>

#import "AppKit/NSSplitView.h"
#import "AppKit/NSSplitViewController.h"
#import "AppKit/NSSplitViewItem.h"
#import "AppKit/NSView.h"
#import "NSViewPrivate.h"

#import "GSFastEnumeration.h"

@implementation NSSplitViewController
// return splitview...
- (NSSplitView *) splitView
{
  return (NSSplitView *)[self view];
}

- (void) setSplitView: (NSSplitView *)splitView
{
  [self setView: splitView];
  [splitView setDelegate: self];
}

- (NSSplitViewItem *) splitViewItemForViewController: (NSViewController *)vc
{
  FOR_IN (NSSplitViewItem*, svi, _splitViewItems)
    if ([svi viewController] == vc)
      {
        return svi;
      }
  END_FOR_IN (_splitViewItems);
  return nil;
}

- (CGFloat) minimumThicknessForInlineSidebars
{
  return _minimumThicknessForInlineSidebars;
}

- (void) setMinimumThicknessForInlineSidebars: (CGFloat)value
{
  _minimumThicknessForInlineSidebars = value;
}
  
// manage splitview items...
- (NSArray *) splitViewItems
{
  return _splitViewItems;
}

- (void) setSplitViewItems: (NSArray *)items
{
  NSMutableArray *mutableItems = [items mutableCopy];
  ASSIGN(_splitViewItems, mutableItems);
}

- (void) addSplitViewItem: (NSSplitViewItem *)item
{
  [self insertSplitViewItem: item atIndex: [_splitViewItems count]];
}

- (void) insertSplitViewItem: (NSSplitViewItem *)item atIndex: (NSInteger)index
{
  NSSplitView *sv = [self splitView];
  NSViewController *vc = [item viewController];

  if (vc != nil)
    {
      NSView *v = [vc view];
      if (v != nil)
        {
          [sv _insertSubview: v atIndex: index];
        }
    }       
  [_splitViewItems insertObject: item atIndex: index];     
}

- (void) removeSplitViewItem: (NSSplitViewItem *)item
{
  NSViewController *vc = [item viewController];
  if (vc != nil)
    {
      NSView *v = [vc view];
      if (v != nil)
        {
          [[self splitView] removeSubview: v];  
        }
    }
  [_splitViewItems removeObject: item];
}

- (void) dealloc
{
  RELEASE(_splitViewItems);
  [super dealloc];
}

// instance methods...
- (NSRect) splitView: (NSSplitView *)splitView additionalEffectiveRectOfDividerAtIndex: (NSInteger)dividerIndex
{
  return [splitView frame];
}

- (BOOL) splitView: (NSSplitView *)splitView canCollapseSubview: (NSView *)subview
{
  return YES;
}

- (NSRect) splitView: (NSSplitView *)splitView effectiveRect: (NSRect)proposedEffectiveRect forDrawnRect: (NSRect)drawnRect
    ofDividerAtIndex: (NSInteger)dividerIndex
{
  return proposedEffectiveRect;
}

- (BOOL) splitView:(NSSplitView *)splitView shouldCollapseSubview: (NSView *)subview forDoubleClickOnDividerAtIndex: (NSInteger)dividerIndex
{
  return YES;
}

- (BOOL) splitView: (NSSplitView *)splitView shouldHideDividerAtIndex: (NSInteger)dividerIndex
{
  return YES;
}

- (IBAction)toggleSidebar:(id)sender
{
  NSLog(@"Toggle");
}

// NSCoding
- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super initWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"NSSplitView"])
        {
          NSSplitView *sv = [coder decodeObjectForKey: @"NSSplitView"];
          [self setSplitView: sv];
        }
      if ([coder containsValueForKey: @"NSSplitViewItems"])
        {
          NSArray *items = [coder decodeObjectForKey: @"NSSplitViewItems"];
          [_splitViewItems addObjectsFromArray: items];
        }
      if ([coder containsValueForKey: @"NSMinimumThicknessForInlineSidebars"])
        {
          _minimumThicknessForInlineSidebars =
            [coder decodeFloatForKey: @"NSMinimumThicknessForInlineSidebars"];
        }
    }
  else
    {
      NSSplitView *sv = [coder decodeObject];
      [self setSplitView: sv];
      NSArray *items = [coder decodeObject];
      [self setSplitViewItems: items];
      [coder decodeValueOfObjCType: @encode(CGFloat) at: &_minimumThicknessForInlineSidebars];
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      NSSplitView *sv = [coder decodeObjectForKey: @"NSSplitView"];
      [self setSplitView: sv];
      NSArray *items = [coder decodeObjectForKey: @"NSSplitViewItems"];
      [_splitViewItems addObjectsFromArray: items];
      _minimumThicknessForInlineSidebars =
        [coder decodeFloatForKey: @"NSMinimumThicknessForInlineSidebars"];
    }
  else
    {
      [coder encodeObject: [self splitView]];
      [coder encodeObject: [self splitViewItems]];
      [coder encodeValueOfObjCType: @encode(CGFloat)
                                at: &_minimumThicknessForInlineSidebars];
    }
}
@end
