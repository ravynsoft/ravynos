/* Interface of class NSSplitViewItem
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Mon 20 Jul 2020 12:56:20 AM EDT

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

#ifndef _NSSplitViewItem_h_GNUSTEP_GUI_INCLUDE
#define _NSSplitViewItem_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

enum
{
 NSSplitViewItemBehaviorDefault,
 NSSplibViewItemBehaviorSidebar,
 NSSplitViewItemBehaviorContentList
};
typedef NSInteger NSSplitViewItemBehavior;

enum
{
 NSSplitViewItemCollapseBehaviorDefault,
 NSSplitViewItemCollapseBehaviorPreferResizingSiblingsWithFixedSplitView,
 NSSplitViewItemCollapseBehaviorPreferResizingSplitViewWithFixedSiblings,
 NSSplitViewItemCollapseBehaviorUseConstraints
};
typedef NSInteger NSSplitViewItemCollapseBehavior;

enum
{
 NSTitlebarSeparatorStyleAutomatic,
 NSTitlebarSeparatorStyleLine,
 NSTitlebarSeparatorStyleNone,
 NSTitlebarSeparatorStyleShadow
};
typedef NSInteger NSTitlebarSeparatorStyle;

@class NSViewController;
  
@interface NSSplitViewItem : NSObject <NSCoding>
{
  CGFloat _automaticMaximumThickness;
  CGFloat _preferredThicknessFraction;
  CGFloat _minimumThickness;
  CGFloat _maximumThickness;
  BOOL _springLoaded;
  BOOL _allowsFullHeightLayout;
  BOOL _canCollapse;
  CGFloat /*NSLayoutPriority*/ _holdingPriority;
  NSSplitViewItemCollapseBehavior _collapseBehavior;
  NSViewController *_viewController;
  NSTitlebarSeparatorStyle _titlebarSeparatorStyle;
}
  
+ (instancetype)contentListWithViewController:(NSViewController *)viewController;
+ (instancetype)sidebarWithViewController:(NSViewController *)viewController;
+ (instancetype)splitViewItemWithViewController:(NSViewController *)viewController;

- (CGFloat) automaticMaximumThickness;
- (void) setAutomaticMaximumThickness: (CGFloat)f;
- (CGFloat) preferredThicknessFraction;
- (void) setPreferredThicknessFraction: (CGFloat)f;
- (CGFloat) minimumThickness;
- (void) setMinimumThickness: (CGFloat)f;
- (CGFloat) maximumThickness;
- (void) setMaximumThickness: (CGFloat)f;

- (/* NSLayoutPriority */ CGFloat) holdingPriority;

- (BOOL) canCollapse;
- (NSSplitViewItemCollapseBehavior) collapseBehavior;
- (BOOL) isSpringLoaded;
- (void) setSpringLoaded: (BOOL)flag;

- (BOOL) allowsFullHeightLayout;
- (void) setAllowsFullHeightLayout: (BOOL)flag;

- (NSTitlebarSeparatorStyle) titlebarSeparatorStyle;
- (void) setTitlebarSeparatorStyle: (NSTitlebarSeparatorStyle)style;

- (NSViewController *) viewController;
- (void) setViewController: (NSViewController *)vc;
@end

APPKIT_EXPORT const CGFloat NSSplitViewItemUnspecifiedDimension;
  
#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSSplitViewItem_h_GNUSTEP_GUI_INCLUDE */

