/**
   Implementation of the GSToolTips class

   Copyright (C) 2006 Free Software Foundation, Inc.

   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: 2006

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

#import <Foundation/NSGeometry.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSString.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSIndexSet.h>
#import <Foundation/NSUserDefaults.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSScreen.h"
#import "AppKit/NSStringDrawing.h"
#import "AppKit/NSView.h"
#import "AppKit/NSPanel.h"
#import "GNUstepGUI/GSTrackingRect.h"
#import "GSToolTips.h"
#import "GSFastEnumeration.h"

@interface NSWindow (GNUstepPrivate)

+ (void) _setToolTipVisible: (GSToolTips*)t;
+ (GSToolTips*) _toolTipVisible;

@end


@interface	NSObject (ToolTips)
- (NSString*) view: (NSView*)v stringForToolTip: (NSToolTipTag)t
 point: (NSPoint)p userData: (void*)d;
@end

/* A trivial class to hold information about the provider of the tooltip
 * string.  Instance allocation/deallocation is managed by GSToolTip and
 * our instances are stored in the user data field of tracking rectangles.
 */
@interface	GSTTProvider : NSObject
{
  id		object;
  void		*data;
}
- (void*) data;
- (id) initWithObject: (id)o userData: (void*)d;
- (id) object;
- (void) setObject: (id)o;
@end

@implementation	GSTTProvider
- (void*) data
{
  return data;
}
- (void) dealloc
{
  [self setObject: nil];
  [super dealloc];
}
- (id) initWithObject: (id)o userData: (void*)d
{
  data = d;
  [self setObject: o];
  return self;
}
- (id) object
{
  return object;
}
- (void) setObject: (id)o
{
  /* Experimentation on MacOS-X shows that the object is not retained.
   * However, if the object does not provide a string, we must use a
   * copy of its description ... and we have to retain that until we
   * are done with it.
   */
  if ([object respondsToSelector:
    @selector(view:stringForToolTip:point:userData:)] == NO)
    {
      /* Object must be a string rather than something which provides one */
      RELEASE(object);
    }
  object = o;
  if ([object respondsToSelector:
    @selector(view:stringForToolTip:point:userData:)] == NO)
    {
      /* Object does not provide a string ... so we take a copy of it
       * as the string to be used.
       */
      object = [[object description] copy];
    }
}
@end

@interface GSTTView : NSView
{
  NSAttributedString *_text;
}

- (void)setText: (NSAttributedString *)text;
@end
 	  	 
@implementation GSTTView
- (id) initWithFrame: (NSRect)frameRect
{
  self = [super initWithFrame: frameRect];
  if (self)
    {
      [self setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
    }
  return self;
}

- (void) setText: (NSAttributedString *)text
{
  if (_text != text)
    {
      ASSIGN(_text, text);
      [self setNeedsDisplay: YES];
    }
}
 	  	 
- (void) drawRect: (NSRect)dirtyRect
{
  if (_text)
    {
      NSRectEdge sides[] = {NSMinXEdge, NSMaxYEdge, NSMaxXEdge, NSMinYEdge};
      NSColor *black = [NSColor blackColor];
      NSColor *colors[] = {black, black, black, black};
      NSRect bounds = [self bounds];
      NSRect frame = [self frame];
      NSRect textRect = NSInsetRect(frame, 2, 2);

      NSDrawColorTiledRects(bounds, bounds, sides, colors, 4);
      [_text drawInRect: textRect];
    }
}
@end

@interface GSTTPanel : NSPanel
// Tooltip panel that will not try to become main or key
- (BOOL) canBecomeKeyWindow;
- (BOOL) canBecomeMainWindow;

@end

@implementation GSTTPanel

- (id) initWithContentRect: (NSRect)contentRect
                 styleMask: (NSUInteger)aStyle
                   backing: (NSBackingStoreType)bufferingType
                     defer: (BOOL)flag;
{
  self = [super initWithContentRect: contentRect
                          styleMask: aStyle
                            backing: bufferingType
                              defer: flag];
  if (self)
    {
      [self setContentView: [[[GSTTView alloc] initWithFrame: contentRect] autorelease]];
    }
  return self;
}

- (BOOL) canBecomeKeyWindow 
{
  return NO;
}

- (BOOL) canBecomeMainWindow 
{
  return NO;
}

@end


@interface	GSToolTips (Private)
- (void) _endDisplay;
- (void) _endDisplay: (NSTrackingRectTag)tag;
- (void) _timedOut: (NSTimer *)timer;
@end
/*
typedef struct NSView_struct
{
  @defs(NSView)
} *NSViewPtr;
*/
typedef NSView* NSViewPtr;

@implementation GSToolTips

static NSMapTable	*viewsMap = 0;
static NSTimer		*timer = nil;
static GSToolTips       *timedObject = nil;
static NSTrackingRectTag timedTag = NSNotFound;
// Having a single stored panel for tooltips greatly reduces callback interaction from MS-Windows
static GSTTPanel	*window = nil;
// Prevent Windows callback API from attempting to dismiss tooltip as its in the process of appearing
static BOOL   isOpening = NO;
static NSSize		offset;
static BOOL		restoreMouseMoved;

+ (void) initialize
{
  viewsMap = NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,
			     NSObjectMapValueCallBacks, 8);
           
  window = [[GSTTPanel alloc] initWithContentRect: NSMakeRect(0,0,100,25)
				       styleMask: NSBorderlessWindowMask
					 backing: NSBackingStoreRetained
					   defer: YES];
    
  [window setBackgroundColor: [NSColor toolTipColor]];
  [window setReleasedWhenClosed: NO];
  [window setExcludedFromWindowsMenu: YES];
  [window setLevel: NSPopUpMenuWindowLevel];
  [window setAutodisplay: NO];
}

+ (void) removeTipsForView: (NSView*)aView
{
  GSToolTips	*tt = (GSToolTips*)NSMapGet(viewsMap, (void*)aView);

  if (tt != nil)
    {
      [tt removeAllToolTips];
      NSMapRemove(viewsMap, (void*)aView);
    }
}

+ (GSToolTips*) tipsForView: (NSView*)aView
{
  GSToolTips	*tt = (GSToolTips*)NSMapGet(viewsMap, (void*)aView);

  if (tt == nil)
    {
      tt = [[GSToolTips alloc] initForView: aView];
      NSMapInsert(viewsMap, (void*)aView, (void*)tt);
      RELEASE(tt);
    }
  return tt;
}



- (NSToolTipTag) addToolTipRect: (NSRect)aRect
                          owner: (id)anObject
                       userData: (void *)data
{
  NSTrackingRectTag	tag;
  GSTTProvider		*provider;

  if (timer != nil)
    {
      return -1;	// A tip is already in progress.
    }
  aRect = NSIntersectionRect(aRect, [view bounds]);
  if (NSEqualRects(aRect, NSZeroRect))
    {
      return -1;	// No rectangle.
    }
  if (anObject == nil)
    {
      return -1;	// No provider object.
    }

  provider = [[GSTTProvider alloc] initWithObject: anObject
					 userData: data];
  tag = [view addTrackingRect: aRect
                        owner: self
                     userData: provider
                 assumeInside: NO];
  return tag;
}

- (unsigned) count
{
  NSEnumerator		*enumerator;
  GSTrackingRect	*rect;
  unsigned		count = 0;

  enumerator = [((NSViewPtr)view)->_tracking_rects objectEnumerator];
  while ((rect = [enumerator nextObject]) != nil)
    {
      if (rect->owner == self)
        {
	  count++;
	}
    }
  return count;
}

- (void) dealloc
{
  [self _endDisplay];
  [self removeAllToolTips];
  [super dealloc];
}

- (id) initForView: (NSView*)aView
{
  view = aView;
  toolTipTag = -1;
  return self;
}

- (void) mouseEntered: (NSEvent *)theEvent
{
  GSTTProvider	*provider;
  NSString	*toolTipString;

  if (timer != nil)
    {
      /* Moved from one tooltip view to another, so reset the timer.
       */
      [timer invalidate];
      timer = nil;
      timedObject = nil;
      timedTag = NSNotFound;
    }

  provider = (GSTTProvider*)[theEvent userData];
  if ([[provider object] respondsToSelector:
    @selector(view:stringForToolTip:point:userData:)] == YES)
    {
      // From testing on OS X, point is in the view's coordinate system
      // The locationInWindow has been converted to this in
      // [NSWindow _checkTrackingRectangles:forEvent:]
      NSPoint p = [theEvent locationInWindow];

      toolTipString = [[provider object] view: view
			     stringForToolTip: [theEvent trackingNumber]
					point: p
				     userData: [provider data]];
    }
  else
    {
      toolTipString = [provider object];
    }

  timer = [NSTimer scheduledTimerWithTimeInterval: 0.5
                                           target: self
                                         selector: @selector(_timedOut:)
                                         userInfo: toolTipString
                                          repeats: YES];
  [[NSRunLoop currentRunLoop] addTimer: timer forMode: NSModalPanelRunLoopMode];
  timedObject = self;
  timedTag = [theEvent trackingNumber];
  if ([[view window] acceptsMouseMovedEvents] == YES)
    {
      restoreMouseMoved = NO;
    }
  else
    {
      restoreMouseMoved = YES;
      [[view window] setAcceptsMouseMovedEvents: YES];
    }
  [NSWindow _setToolTipVisible: self];
}

- (void) mouseExited: (NSEvent *)theEvent
{
  [self _endDisplay:[theEvent trackingNumber]];
}

- (void) mouseDown: (NSEvent *)theEvent
{
  [self _endDisplay];
}

- (void) mouseMoved: (NSEvent *)theEvent
{
  NSPoint mouseLocation;
  NSPoint origin;

  if (window == nil)
    {
      return;
    }

  mouseLocation = [NSEvent mouseLocation];

  origin = NSMakePoint(mouseLocation.x + offset.width,
    mouseLocation.y + offset.height);

  [window setFrameOrigin: origin];
}

- (void) removeAllToolTips
{
  NSEnumerator		*enumerator;
  GSTrackingRect	*rect;

  [self _endDisplay];

  enumerator = [((NSViewPtr)view)->_tracking_rects objectEnumerator];
  while ((rect = [enumerator nextObject]) != nil)
    {
      if (rect->owner == self)
        {
	  RELEASE((GSTTProvider*)rect->user_data);
	  rect->user_data = 0;
          [view removeTrackingRect: rect->tag];
	}
    }
  toolTipTag = -1;
}

- (void)removeToolTipsInRect: (NSRect)aRect
{
  NSUInteger idx = 0;
  NSMutableIndexSet *indexes = [NSMutableIndexSet new];
  id tracking_rects = ((NSViewPtr)view)->_tracking_rects;
  FOR_IN(GSTrackingRect*, rect, tracking_rects)
    if ((rect->owner == self) && NSContainsRect(aRect, rect->rectangle))
      {
        RELEASE((GSTTProvider*)rect->user_data);
        rect->user_data = 0;
        [indexes addIndex: idx];
        [rect invalidate];
      }
      idx++;
  END_FOR_IN(tracking_rects)
  [((NSViewPtr)view)->_tracking_rects removeObjectsAtIndexes: indexes];
  if ([((NSViewPtr)view)->_tracking_rects count] == 0)
    {
      ((NSViewPtr)view)->_rFlags.has_trkrects = 0;
    }
  [indexes release];
}

- (void) removeToolTip: (NSToolTipTag)tag
{
  NSEnumerator   	*enumerator;
  GSTrackingRect	*rect;

  enumerator = [((NSViewPtr)view)->_tracking_rects objectEnumerator];
  while ((rect = [enumerator nextObject]) != nil)
    {
      if (rect->tag == tag && rect->owner == self)
	{
	  RELEASE((GSTTProvider*)rect->user_data);
	  rect->user_data = 0;
	  [view removeTrackingRect: tag];
          return;
	}
    }
}

- (void) setToolTip: (NSString *)string
{
  if ([string length] == 0)
    {
      if (toolTipTag != -1)
        {
	  [self _endDisplay];
	  [self removeToolTip: toolTipTag];
	  toolTipTag = -1;
	}
    }
  else
    {
      GSTTProvider	*provider;

      if (toolTipTag == -1)
        {
	  NSRect	rect;

	  rect = [view bounds];
	  provider = [[GSTTProvider alloc] initWithObject: string
						 userData: nil];
	  toolTipTag = [view addTrackingRect: rect
				       owner: self
				    userData: provider
				assumeInside: NO];
	}
      else
        {
	  NSEnumerator   	*enumerator;
	  GSTrackingRect	*rect;

	  enumerator = [((NSViewPtr)view)->_tracking_rects objectEnumerator];
	  while ((rect = [enumerator nextObject]) != nil)
	    {
	      if (rect->tag == toolTipTag && rect->owner == self)
		{
		  [((GSTTProvider*)rect->user_data) setObject: string];
		}
	    }
	}
    }
}

- (NSString *) toolTip
{
  NSEnumerator		*enumerator;
  GSTrackingRect	*rect;

  enumerator = [((NSViewPtr)view)->_tracking_rects objectEnumerator];
  while ((rect = [enumerator nextObject]) != nil)
    {
      if (rect->tag == toolTipTag)
	{
	  return [((GSTTProvider*)rect->user_data) object];
	}
    }
  return nil;
}

@end

@implementation	GSToolTips (Private)

- (void) _endDisplay
{
  [self _endDisplay:NSNotFound];
}

- (void) _endDisplay: (NSTrackingRectTag)tag
{
  if (isOpening)
    return;
  if ([NSWindow _toolTipVisible] == self)
    {
      [NSWindow _setToolTipVisible: nil];
    }
  /* If there is currently a timer running for this object and it is the target tag,
   * cancel it. Always remove if the target tag is NSNotFound
   */
  if (timer != nil && timedObject == self && (timedTag == tag || tag == NSNotFound))
    {
      if ([timer isValid])
	{
	  [timer invalidate];
	}
      timer = nil;
      timedObject = nil;
      timedTag = NSNotFound;
    }
  if (window != nil)
    {
      [window setFrame: NSZeroRect display: NO];
      [window orderOut:self];
    }
  if (restoreMouseMoved == YES)
    {
      restoreMouseMoved = NO;
      [[view window] setAcceptsMouseMovedEvents: NO];
    }
}

/* The delay timed out -- display the tooltip */
- (void) _timedOut: (NSTimer *)aTimer
{
  CGFloat               size;
  NSString		*toolTipString;
  NSAttributedString	*toolTipText = nil;
  NSSize		textSize;
  NSPoint		mouseLocation = [NSEvent mouseLocation];
  NSRect		visible;
  NSRect		rect;
  NSMutableDictionary	*attributes;

  // retain and autorelease the timer's userinfo because we
  // may  invalidate the timer (which releases the userinfo),
  // but need the userinfo object to remain valid for the
  // remainder of this method.
  toolTipString = [[[aTimer userInfo] retain] autorelease];
  if ( (nil == toolTipString) ||
       ([toolTipString isEqualToString: @""]) )
    {
      return;
    }

  if (timer != nil)
    {
      if ([timer isValid])
	{
	  [timer invalidate];
	}
      timer = nil;
      timedObject = nil;
      timedTag = NSNotFound;
    }

  if ([window isVisible])
    {
      /* Moved from one tooltip view to another ... so stop displaying
       * the old tool tip before we start the new one.
       * This is similar to the case in -mouseEntered: where we cancel
       * the timer for one tooltip view because we have entered another
       * one.
       * To think about ... if we entered a tooltip rectangle without
       * having left the previous one, then when we leave this rectangle
       * we are probably back in the other one and should really restart
       * the timer for the original view.  However, this is a rare case
       * so it's probably better to ignore it than add a lot of code to
       * keep track of all entry and exit.
       */
      [self _endDisplay];
    }

  size = [[NSUserDefaults standardUserDefaults]
	   floatForKey: @"NSToolTipsFontSize"];

  if (size <= 0)
    {
      size = 10.0;
    }

  attributes = [NSMutableDictionary dictionary];
  [attributes setObject: [NSFont toolTipsFontOfSize: size]
		 forKey: NSFontAttributeName];
  [attributes setObject: [NSColor toolTipTextColor]
                  forKey: NSForegroundColorAttributeName];
  toolTipText =
    [[NSAttributedString alloc] initWithString: toolTipString
				    attributes: attributes];
  textSize = [toolTipText size];
  if (textSize.width > 300)
    {
      NSRect rect;
      rect = [toolTipText boundingRectWithSize: NSMakeSize(300, 1e7)
                                       options: 0];
      textSize = rect.size;
      // This extra pixel is needed, otherwise the last line gets cut off.
      textSize.height += 1;
    }

  /* Create window just off the current mouse position
   * Constrain it to be on screen, shrinking if necessary.
   */
  rect = NSMakeRect(mouseLocation.x + 8,
    mouseLocation.y - 16 - (textSize.height+3),
    textSize.width + 4, textSize.height + 4);
  visible = [[NSScreen mainScreen] visibleFrame];
  if (NSMaxY(rect) > NSMaxY(visible))
    {
      rect.origin.y -= (NSMaxY(rect) - NSMaxY(visible));
    }
  if (NSMinY(rect) < NSMinY(visible))
    {
      rect.origin.y += (NSMinY(visible) - NSMinY(rect));
    }
  if (NSMaxY(rect) > NSMaxY(visible))
    {
      rect.origin.y = visible.origin.y;
      rect.size.height = visible.size.height;
    }
    
  if (NSMaxX(rect) > NSMaxX(visible))
    {
      rect.origin.x -= (NSMaxX(rect) - NSMaxX(visible));
    }
  if (NSMinX(rect) < NSMinX(visible))
    {
      rect.origin.x += (NSMinX(visible) - NSMinX(rect));
    }
  if (NSMaxX(rect) > NSMaxX(visible))
    {
      rect.origin.x = visible.origin.x;
      rect.size.width = visible.size.width;
    }
  offset.height = rect.origin.y - mouseLocation.y;
  offset.width = rect.origin.x - mouseLocation.x;

  isOpening = YES;
  [(GSTTView*)([window contentView]) setText: toolTipText];
  [window setFrame: rect display: NO];
  [window orderFront: nil];
  isOpening = NO;

  RELEASE(toolTipText);
}

@end

