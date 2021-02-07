/** <title>GSStandardWindowDecorationView</title>

   Copyright (C) 2004 Free Software Foundation, Inc.

   Author: Alexander Malmberg <alexander@malmberg.org>
   Date: 2004-03-24

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

#import <Foundation/NSException.h>
#import <Foundation/NSNotification.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSParagraphStyle.h"
#import "AppKit/NSScreen.h"
#import "AppKit/NSStringDrawing.h"
#import "AppKit/NSWindow.h"
#import "AppKit/PSOperators.h"
#import "GNUstepGUI/GSDisplayServer.h"
#import "GNUstepGUI/GSTheme.h"

#import <GNUstepGUI/GSWindowDecorationView.h>

@interface GSStandardWindowDecorationView (GSTheme)
- (void) _themeDidActivate: (NSNotification*)notification;
@end

@implementation GSStandardWindowDecorationView

+ (void) offsets: (float *)l : (float *)r : (float *)t : (float *)b
    forStyleMask: (NSUInteger)style
{
  GSTheme *theme = [GSTheme theme];

  if (style
    & (NSTitledWindowMask | NSClosableWindowMask
      | NSMiniaturizableWindowMask | NSResizableWindowMask))
    {
      *l = *r = *t = *b = 1.0;
    }
  else
    {
      *l = *r = *t = *b = 0.0;
    }

  if (style
    & (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask))
    {
      *t = [theme titlebarHeight];
    }
  if (style & NSResizableWindowMask)
    {
      *b = [theme resizebarHeight];
    }
}

+ (CGFloat) minFrameWidthWithTitle: (NSString *)aTitle
		       styleMask: (NSUInteger)aStyle
{
  float l, r, t, b, width;

  [self offsets: &l : &r : &t : &b forStyleMask: aStyle];

  width = l + r;

  if (aStyle & NSTitledWindowMask)
    {
      width += [aTitle sizeWithAttributes: nil].width;
    }
  return width;
}

- (void) updateRects
{
  GSTheme *theme = [GSTheme theme];

  if (hasTitleBar)
    {
      CGFloat titleHeight = [theme titlebarHeight];

      titleBarRect = NSMakeRect(0.0, [self bounds].size.height - titleHeight,
	[self bounds].size.width, titleHeight);
    }
  if (hasResizeBar)
    {
      resizeBarRect = NSMakeRect(0.0, 0.0, [self bounds].size.width, [theme resizebarHeight]);
    }
  if (hasCloseButton)
    {
      closeButtonRect = NSMakeRect([self bounds].size.width - [theme titlebarButtonSize] - 
				   [theme titlebarPaddingRight], [self bounds].size.height - 
				   [theme titlebarButtonSize] - [theme titlebarPaddingTop], 
				   [theme titlebarButtonSize], [theme titlebarButtonSize]);
      [closeButton setFrame: closeButtonRect];
    }

  if (hasMiniaturizeButton)
    {
      miniaturizeButtonRect = NSMakeRect([theme titlebarPaddingLeft], [self bounds].size.height - 
					 [theme titlebarButtonSize] - [theme titlebarPaddingTop], 
					 [theme titlebarButtonSize], [theme titlebarButtonSize]);
      [miniaturizeButton setFrame: miniaturizeButtonRect];
    }
}

- (id) initWithFrame: (NSRect)frame
	      window: (NSWindow *)w
{
  NSUInteger styleMask;

  self = [super initWithFrame: frame window: w];
  if (!self) return nil;

  styleMask = [w styleMask];
  if (styleMask
    & (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask))
    {
      hasTitleBar = YES;
    }
  if (styleMask & NSTitledWindowMask)
    {
      isTitled = YES;
    }
  if (styleMask & NSClosableWindowMask)
    {
      hasCloseButton = YES;

      closeButton = [NSWindow standardWindowButton: NSWindowCloseButton 
                              forStyleMask: styleMask];
      [closeButton setTarget: window];
      [self addSubview: closeButton];
    }
  if (styleMask & NSMiniaturizableWindowMask)
    {
      hasMiniaturizeButton = YES;

      miniaturizeButton = [NSWindow standardWindowButton: NSWindowMiniaturizeButton 
                              forStyleMask: styleMask];
      [miniaturizeButton setTarget: window];
      [self addSubview: miniaturizeButton];
    }
  if (styleMask & NSResizableWindowMask)
    {
      hasResizeBar = YES;
    }
  [self updateRects];

  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(_themeDidActivate:)
    name: GSThemeDidActivateNotification
    object: nil];
  return self;
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  [super dealloc];
}

- (void) drawRect: (NSRect)rect
{
  [[GSTheme theme] drawWindowBorder: rect
                   withFrame: [self bounds] 
                   forStyleMask: [window styleMask]
                   state: inputState
                   andTitle: [window title]];

  [super drawRect: rect];
}

- (void) setTitle: (NSString *)newTitle
{
  if (isTitled)
    [self setNeedsDisplayInRect: titleBarRect];
  [super setTitle: newTitle];
}

- (void) setInputState: (int)state
{
  NSAssert(state >= 0 && state <= 2, @"Invalid state!");
  [super setInputState: state];
  if (hasTitleBar)
    [self setNeedsDisplayInRect: titleBarRect];
}

- (void) setDocumentEdited: (BOOL)flag
{
  if (flag)
    {
      [closeButton setImage: [NSImage imageNamed: @"common_CloseBroken"]];
      [closeButton setAlternateImage:
	[NSImage imageNamed: @"common_CloseBrokenH"]];
    }
  else
    {
      [closeButton setImage: [NSImage imageNamed: @"common_Close"]];
      [closeButton setAlternateImage:
	[NSImage imageNamed: @"common_CloseH"]];
    }
  [super setDocumentEdited: flag];
}

- (NSPoint) mouseLocationOnScreenOutsideOfEventStream
{
  int screen = [[window screen] screenNumber];
  return [GSServerForWindow(window) mouseLocationOnScreen: screen
						   window: NULL];
}

- (void) moveWindowStartingWithEvent: (NSEvent *)event
{
  NSUInteger mask = NSLeftMouseDraggedMask | NSLeftMouseUpMask;
  NSEvent *currentEvent = event;
  NSDate *distantPast = [NSDate distantPast];
  NSPoint delta, point;

  delta = [event locationInWindow];

  [window _captureMouse: nil];
  do
    {
      while (currentEvent && [currentEvent type] != NSLeftMouseUp)
	{
	  currentEvent = [_window nextEventMatchingMask: mask
			   untilDate: distantPast
			   inMode: NSEventTrackingRunLoopMode
			   dequeue: YES];
	}

      point = [self mouseLocationOnScreenOutsideOfEventStream];
      [window setFrameOrigin: NSMakePoint(point.x - delta.x,
					  point.y - delta.y)];

      if (currentEvent && [currentEvent type] == NSLeftMouseUp)
	break;

      currentEvent = [_window nextEventMatchingMask: mask
			untilDate: [NSDate distantFuture]
			inMode: NSEventTrackingRunLoopMode
			dequeue: YES];
    } while ([currentEvent type] != NSLeftMouseUp);
  [window _releaseMouse: nil];
}


static NSRect
calc_new_frame(NSRect frame, NSPoint point, NSPoint firstPoint,
  int mode, NSSize minSize, NSSize maxSize)
{
  NSRect newFrame = frame;
  newFrame.origin.y = point.y - firstPoint.y;
  newFrame.size.height = NSMaxY(frame) - newFrame.origin.y;
  if (newFrame.size.height < minSize.height)
    {
      newFrame.size.height = minSize.height;
      newFrame.origin.y = NSMaxY(frame) - newFrame.size.height;
    }

  if (mode == 0)
    {
      newFrame.origin.x = point.x - firstPoint.x;
      newFrame.size.width = NSMaxX(frame) - newFrame.origin.x;

      if (newFrame.size.width < minSize.width)
	{
	  newFrame.size.width = minSize.width;
	  newFrame.origin.x = NSMaxX(frame) - newFrame.size.width;
	}
    }
  else if (mode == 1)
    {
      newFrame.size.width = point.x - frame.origin.x + frame.size.width
			    - firstPoint.x;

      if (newFrame.size.width < minSize.width)
	{
	  newFrame.size.width = minSize.width;
	  newFrame.origin.x = frame.origin.x;
	}
    }
  return newFrame;
}

- (void) resizeWindowStartingWithEvent: (NSEvent *)event
{
  NSUInteger mask = NSLeftMouseDraggedMask | NSLeftMouseUpMask | NSPeriodicMask;
  NSEvent *currentEvent = event;
  NSDate *distantPast = [NSDate distantPast];
  NSDate *distantFuture = [NSDate distantFuture];
  NSPoint firstPoint, point;
  NSRect newFrame, frame;
  NSSize minSize, maxSize;
  int num = 0;

  /*
  0 drag lower left corner
  1 drag lower right corner
  2 drag lower edge
  */
  int mode;

  firstPoint = [event locationInWindow];
  if (resizeBarRect.size.width < 30 * 2
      && firstPoint.x < resizeBarRect.size.width / 2)
    mode = 0;
  else if (firstPoint.x > resizeBarRect.size.width - 29)
    mode = 1;
  else if (firstPoint.x < 29)
    mode = 0;
  else
    mode = 2;

  frame = [window frame];
  minSize = [window minSize];
  maxSize = [window maxSize];

  [window _captureMouse: nil];
  [NSEvent startPeriodicEventsAfterDelay: 0.1 withPeriod: 0.1];
  do
    {
      while (currentEvent && [currentEvent type] != NSLeftMouseUp)
	{
	  currentEvent = [_window nextEventMatchingMask: mask
			   untilDate: distantPast
			   inMode: NSEventTrackingRunLoopMode
			   dequeue: YES];
	}

      point = [self mouseLocationOnScreenOutsideOfEventStream];
      newFrame
	= calc_new_frame(frame, point, firstPoint, mode, minSize, maxSize);

      if (currentEvent && [currentEvent type] == NSLeftMouseUp)
	break;

      num++;
      if (num == 5)
	{
	  [window setFrame: newFrame  display: YES];
	  num = 0;
	}

      currentEvent = [_window nextEventMatchingMask: mask
			untilDate: distantFuture
			inMode: NSEventTrackingRunLoopMode
			dequeue: YES];
    } while ([currentEvent type] != NSLeftMouseUp);
  [NSEvent stopPeriodicEvents];
  [window _releaseMouse: nil];

  [window setFrame: newFrame  display: YES];
}

- (BOOL) acceptsFirstMouse: (NSEvent*)theEvent
{
  return YES;
}

- (void) mouseDown: (NSEvent *)event
{
  NSPoint p = [self convertPoint: [event locationInWindow] fromView: nil];

  if (NSPointInRect(p, contentRect))
    {
      [super mouseDown: event];
      return;
    }

  if (NSPointInRect(p, titleBarRect))
    {
      [self moveWindowStartingWithEvent: event];
      return;
    }

  if (NSPointInRect(p, resizeBarRect))
    {
      [self resizeWindowStartingWithEvent: event];
      return;
    }

  [super mouseDown: event];
}

- (void) setFrame: (NSRect)frameRect
{
  [super setFrame: frameRect];
  [self updateRects];
}

@end

@implementation GSStandardWindowDecorationView (GSTheme)

- (void) _themeDidActivate: (NSNotification*)notification
{
  [self updateRects];
  [self setNeedsDisplay: YES];
}

@end
