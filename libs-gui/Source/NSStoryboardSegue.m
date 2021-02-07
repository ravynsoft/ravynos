/* Implementation of class NSStoryboardSegue
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory Casamento
   Date: Mon Jan 20 15:57:31 EST 2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import <Foundation/NSString.h>
#import <Foundation/NSGeometry.h>

#import "AppKit/NSStoryboardSegue.h"
#import "AppKit/NSWindowController.h"
#import "AppKit/NSViewController.h"
#import "AppKit/NSSplitViewController.h"
#import "AppKit/NSSplitViewItem.h"
#import "AppKit/NSSplitView.h"
#import "AppKit/NSTabViewController.h"
#import "AppKit/NSTabViewItem.h"
#import "AppKit/NSTabView.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSView.h"
#import "AppKit/NSPopover.h"

@implementation NSStoryboardSegue

- (id) sourceController
{
  return _sourceController;
}

- (id) destinationController
{
  return _destinationController;
}

- (NSStoryboardSegueIdentifier)identifier
{
  return _identifier;
}

- (void) _setHandler: (GSStoryboardSeguePerformHandler)handler
{
  ASSIGN(_handler, handler);
}

- (void) _setDestinationController: (id)controller
{
  _destinationController = controller;
}

- (void) _setSourceController: (id)controller
{
  _sourceController = controller;
}

+ (instancetype) segueWithIdentifier: (NSStoryboardSegueIdentifier)identifier 
                              source: (id)sourceController 
                         destination: (id)destinationController 
                      performHandler: (GSStoryboardSeguePerformHandler)performHandler
{
  NSStoryboardSegue *segue = [[NSStoryboardSegue alloc] initWithIdentifier: identifier
                                                                    source: sourceController
                                                               destination: destinationController];
  AUTORELEASE(segue);
  [segue _setHandler: performHandler];

  return segue;
}

- (instancetype) initWithIdentifier: (NSStoryboardSegueIdentifier)identifier 
                             source: (id)sourceController 
                        destination: (id)destinationController
{
  self = [super init];
  if (self != nil)
    {
      ASSIGN(_sourceController, sourceController);
      ASSIGN(_destinationController, destinationController);
      ASSIGN(_identifier, identifier);
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_sourceController);
  RELEASE(_destinationController);
  RELEASE(_identifier);
  RELEASE(_kind);
  RELEASE(_relationship);
  RELEASE(_handler);
  [super dealloc];
}

- (void) perform
{
  // Perform segue based on it's kind...
  if ([_kind isEqualToString: @"relationship"])
    {
      if ([_relationship isEqualToString: @"window.shadowedContentViewController"])
        {
          NSWindow *w = [_sourceController window];
          NSView *v = [_destinationController view];
          [w setContentView: v];
          [w setTitle: [_destinationController title]];
          [_sourceController showWindow: self];
        }
      else if ([_relationship isEqualToString: @"splitItems"])
        {
          NSView *v = [_destinationController view];
          NSSplitViewController *svc = (NSSplitViewController *)_sourceController;
          [[svc splitView] addSubview: v];
          NSUInteger idx = [[[svc splitView] subviews] count] - 1;
          NSSplitViewItem *item = [[svc splitViewItems] objectAtIndex: idx];
          [item setViewController: _destinationController];
        }
      else if ([_relationship isEqualToString: @"tabItems"])
        {
          NSTabViewController *tvc = (NSTabViewController *)_sourceController;
          NSTabViewItem *item = [NSTabViewItem tabViewItemWithViewController: _destinationController];
          [tvc addTabViewItem: item]; 
        }
    }
  else if ([_kind isEqualToString: @"modal"])
    {
      NSWindow *w = nil;
      if ([_destinationController isKindOfClass: [NSWindowController class]])
        {
          w = [_destinationController window];
        }
      else
        {
          w = [NSWindow windowWithContentViewController: _destinationController];
          [w setTitle: [_destinationController title]];
        }
      RETAIN(w);
      [w center];
      [NSApp runModalForWindow: w];
    }
  else if ([_kind isEqualToString: @"show"])
    {
      if ([_destinationController isKindOfClass: [NSWindowController class]])
        {
          [_destinationController showWindow: _sourceController];
        }
      else
        {
          NSWindow *w = [NSWindow windowWithContentViewController: _destinationController];
          [w setTitle: [_destinationController title]];
          [w center];
          [w orderFrontRegardless];
          RETAIN(w);
        }
    }
  else if ([_kind isEqualToString: @"popover"])
    {
      NSPopover *po = [[NSPopover alloc] init];
      NSRect rect = [_popoverAnchorView frame];

      [po setBehavior: _popoverBehavior];
      [po setContentViewController: _destinationController];
      [po showRelativeToRect: rect
                      ofView: _popoverAnchorView
               preferredEdge: _preferredEdge];      
    }
  else if ([_kind isEqualToString: @"sheet"])
    {
    }
  else if ([_kind isEqualToString: @"custom"])
    {
    }

  if (_handler != nil)
    {
      CALL_BLOCK_NO_ARGS(_handler);
    }
}

@end
