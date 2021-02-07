/*
   NSPopover.m

   The popover class

   Copyright (C) 2013 Free Software Foundation, Inc.

   Author:  Gregory Casamento <greg.casamento@gmail.com>
   Date: 2013

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

#import <Foundation/NSArchiver.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSNotification.h>

#import "AppKit/NSPopover.h"
#import "AppKit/NSViewController.h"
#import "AppKit/NSView.h"
#import "AppKit/NSWindow.h"


/* Class */
@implementation NSPopover

/* Properties */
- (void) setAnimates: (BOOL)flag
{
  _animates = flag;
}

- (BOOL) animates
{
  return _animates;
}

- (void) setAppearance: (NSPopoverAppearance)value
{
  _appearance = value;
}

- (NSPopoverAppearance) appearance
{
  return _appearance;
}

- (void) setBehavior: (NSPopoverBehavior)value
{
  _behavior = value;
}

- (NSPopoverBehavior) behavior
{
  return _behavior;
}

- (void) setContentSize: (NSSize)value
{
  _contentSize = value;
}

- (NSSize) contentSize
{
  return _contentSize;
}

- (void) setContentViewController: (NSViewController *)controller
{
  ASSIGN(_contentViewController, controller);
}

- (NSViewController *) contentViewController
{
  return _contentViewController;
}

- (void) setDelegate: (id)value
{
  _delegate = value;
}

- (id) delegate
{
  return _delegate;
}

- (void) setPositioningRect: (NSRect)value
{
  _positioningRect = value;
}

- (NSRect) positioningRect
{
  return _positioningRect;
}

- (BOOL) isShown
{
  return _shown;
}

/* Methods */
- (void) close
{
  [_realWindow close];
  [_realWindow setDelegate:nil];
}

- (IBAction) performClose: (id)sender
{
  [_realWindow performClose:sender];
  [_realWindow setDelegate:nil];
}

- (void) showRelativeToRect: (NSRect)positioningRect
                     ofView: (NSView *)positioningView 
              preferredEdge: (NSRectEdge)preferredEdge
{
  NSView *view = nil;
  NSRect screenRect;
  NSRect windowFrame;
  NSRect viewFrame;

  [_contentViewController loadView];
  view = [_contentViewController view];
  viewFrame = [view frame];

  _realWindow = [[NSWindow alloc] initWithContentRect: viewFrame
					    styleMask: NSBorderlessWindowMask
					      backing: NSBackingStoreRetained
						defer: NO];

  screenRect = [[positioningView window] convertRectToScreen:positioningRect];
  windowFrame = [_realWindow frame];
  windowFrame.origin = screenRect.origin;

  if(NSMinXEdge == preferredEdge)
    {
      windowFrame.origin.y -= viewFrame.size.height;
    }
  else if(NSMaxXEdge == preferredEdge)
    {
      windowFrame.origin.y += viewFrame.size.height;
    }
  else if(NSMinYEdge == preferredEdge)
    {
      windowFrame.origin.x -= viewFrame.size.width;
    }
  else if(NSMaxYEdge == preferredEdge)
    {
      windowFrame.origin.x += viewFrame.size.width;
    }

  [_realWindow setFrame: windowFrame display: YES];

  NSLog(@"Showing relative to in window %@",NSStringFromRect(positioningRect));
  NSLog(@"Showing relative to in screen %@",NSStringFromRect(screenRect));
  // [_realWindow setBackgroundColor:[NSColor clearColor]];
  // [_realWindow setOpaque:NO];
  // [_realWindow setLevel:NSFloatingWindowLevel];
  // [_realWindow setAlphaValue:0.0];

  [[_realWindow contentView] addSubview: view];
  [_realWindow setDelegate: self];
  [_realWindow makeKeyAndOrderFront:self];
}

- (BOOL) windowShouldClose: (id)sender
{
  return [_delegate popoverShouldClose:self];
}

- (void) windowDidClose: (NSNotification *)notification
{
  [[NSNotificationCenter defaultCenter] postNotificationName:NSPopoverDidCloseNotification
						      object:self
						    userInfo:nil];
}

- (void) windowWillClose: (NSNotification *)notification
{
  [[NSNotificationCenter defaultCenter] postNotificationName:NSPopoverWillCloseNotification
						      object:self
						    userInfo:nil];
}

- (id) initWithCoder: (NSCoder *)coder
{
  if (nil != (self = [super initWithCoder:coder]))
    {
      if (YES == [coder allowsKeyedCoding])
	{
	  _appearance = [coder decodeIntForKey: @"NSAppearance"];
	  _behavior   = [coder decodeIntForKey: @"NSBehavior"];
	  _animates   = [coder decodeBoolForKey: @"NSAnimates"];
	  _contentSize.width = [coder decodeDoubleForKey: @"NSContentWidth"];
	  _contentSize.height = [coder decodeDoubleForKey: @"NSContentHeight"];
	  [self setContentViewController:[coder decodeObjectForKey:@"NSContentViewController"]];
	}
      else
	{
	  [coder decodeValueOfObjCType: @encode(NSInteger) at: &_appearance];
	  [coder decodeValueOfObjCType: @encode(NSInteger) at: &_behavior];
	  [coder decodeValueOfObjCType: @encode(BOOL) at: &_animates];
	  [coder decodeValueOfObjCType: @encode(CGFloat) at: &_contentSize.width];
	  [coder decodeValueOfObjCType: @encode(CGFloat) at: &_contentSize.height];
	  [self setContentViewController:[coder decodeObject]];
	}
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder:coder];
  if (YES == [coder allowsKeyedCoding])
    {
      [coder encodeInt: _appearance forKey: @"NSAppearance"];
      [coder encodeInt: _behavior forKey: @"NSBehavior"];
      [coder encodeBool: _animates forKey: @"NSAnimates"];
      [coder encodeDouble: _contentSize.width forKey: @"NSContentWidth"];
      [coder encodeDouble: _contentSize.height forKey: @"NSContentHeight"];
      [coder encodeObject:_contentViewController forKey:@"NSContentViewController"];
    }
  else
    {
      [coder encodeValueOfObjCType: @encode(NSInteger) at: &_appearance];
      [coder encodeValueOfObjCType: @encode(NSInteger) at: &_behavior];
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_animates];
      [coder encodeValueOfObjCType: @encode(CGFloat) at: &_contentSize.width];
      [coder encodeValueOfObjCType: @encode(CGFloat) at: &_contentSize.height];
      [coder encodeObject:_contentViewController];
    } 
}
@end
