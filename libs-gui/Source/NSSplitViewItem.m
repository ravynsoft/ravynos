/* Implementation of class NSSplitViewItem
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

#import <Foundation/NSArchiver.h>
#import "AppKit/NSSplitViewItem.h"
#import "AppKit/NSViewController.h"

@implementation NSSplitViewItem
- (instancetype) initWithViewController: (NSViewController *)viewController
{
  self = [super init];
  if (self != nil)
    {
      ASSIGN(_viewController, viewController);
    }
  return self;
}

+ (instancetype) contentListWithViewController: (NSViewController *)viewController
{
  return AUTORELEASE([[NSSplitViewItem alloc] initWithViewController: viewController]);
}

+ (instancetype) sidebarWithViewController: (NSViewController *)viewController
{
  return AUTORELEASE([[NSSplitViewItem alloc] initWithViewController: viewController]);
}

+ (instancetype) splitViewItemWithViewController: (NSViewController *)viewController
{
  return AUTORELEASE([[NSSplitViewItem alloc] initWithViewController: viewController]);  
}

- (CGFloat) automaticMaximumThickness
{
  return _automaticMaximumThickness;
}

- (void) setAutomaticMaximumThickness: (CGFloat)f
{
  _automaticMaximumThickness = f;
}

- (CGFloat) preferredThicknessFraction
{
  return _preferredThicknessFraction;
}

- (void) setPreferredThicknessFraction: (CGFloat)f
{
  _preferredThicknessFraction = f;
}

- (CGFloat) minimumThickness
{
  return _minimumThickness;
}

- (void) setMinimumThickness: (CGFloat)f
{
  _minimumThickness = f;
}

- (CGFloat) maximumThickness
{
  return _maximumThickness;
}

- (void) setMaximumThickness: (CGFloat)f
{
  _maximumThickness = f;
}

- (/* NSLayoutPriority */ CGFloat) holdingPriority
{
  return _holdingPriority;
}

- (void) setHoldingPriority: (/*NSLayoutPriority*/ CGFloat)hp
{
  _holdingPriority = hp;
}

- (BOOL) canCollapse
{
  return _canCollapse;
}

- (NSSplitViewItemCollapseBehavior) collapseBehavior
{
  return _collapseBehavior;
}

- (BOOL) isSpringLoaded
{
  return _springLoaded;
}

- (void) setSpringLoaded: (BOOL)flag
{
  _springLoaded = flag;
}

- (BOOL) allowsFullHeightLayout
{
  return _allowsFullHeightLayout;
}

- (void) setAllowsFullHeightLayout: (BOOL)flag
{
  _allowsFullHeightLayout = flag;
}

- (NSTitlebarSeparatorStyle) titlebarSeparatorStyle
{
  return _titlebarSeparatorStyle;
}

- (void) setTitlebarSeparatorStyle: (NSTitlebarSeparatorStyle)style
{
  _titlebarSeparatorStyle = style;
}

- (NSViewController *) viewController
{
  return _viewController;
}

- (void) setViewController: (NSViewController *)vc
{
  _viewController = vc;
}

// NSCoding
- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super init];
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"NSSplitViewItemViewController"])
        {
          _viewController = [coder decodeObjectForKey: @"NSSplitViewItemViewController"];
        }
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: _viewController
                   forKey: @"NSSplitViewItemViewController"];
    }
}

@end
