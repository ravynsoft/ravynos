/* Definition of class NSSplitViewController
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

#ifndef _NSSplitViewController_h_GNUSTEP_GUI_INCLUDE
#define _NSSplitViewController_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSGeometry.h>
#import <AppKit/NSViewController.h>
#import <AppKit/NSUserInterfaceValidation.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSSplitView, NSSplitViewItem, NSArray, NSMutableArray;
  
@interface NSSplitViewController : NSViewController
{
  CGFloat _minimumThicknessForInlineSidebars;
  NSMutableArray *_splitViewItems;
}

// return splitview...
- (NSSplitView *) splitView;
- (void) setSplitView: (NSSplitView *)splitView;
- (NSSplitViewItem *) splitViewItemForViewController: (NSViewController *)vc;
- (CGFloat) minimumThicknessForInlineSidebars;
- (void) setMinimumThicknessForInlineSidebars: (CGFloat)value;
  
// manage splitview items...
- (NSArray *) splitViewItems;
- (void) setSplitViewItems: (NSArray *)items;
- (void) addSplitViewItem: (NSSplitViewItem *)item;
- (void) insertSplitViewItem: (NSSplitViewItem *)item atIndex: (NSInteger)index;
- (void) removeSplitViewItem: (NSSplitViewItem *)item;

// instance methods...
- (NSRect) splitView: (NSSplitView *)splitView additionalEffectiveRectOfDividerAtIndex: (NSInteger)dividerIndex;
- (BOOL) splitView: (NSSplitView *)splitView canCollapseSubview: (NSView *)subview;
- (NSRect) splitView: (NSSplitView *)splitView effectiveRect: (NSRect)proposedEffectiveRect
           forDrawnRect: (NSRect)drawnRect ofDividerAtIndex: (NSInteger)dividerIndex;
- (BOOL) splitView: (NSSplitView *)splitView shouldCollapseSubview: (NSView *)subview
         forDoubleClickOnDividerAtIndex: (NSInteger)dividerIndex;
- (BOOL) splitView: (NSSplitView *)splitView shouldHideDividerAtIndex: (NSInteger)dividerIndex;
- (IBAction)toggleSidebar:(id)sender;
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSSplitViewController_h_GNUSTEP_GUI_INCLUDE */

