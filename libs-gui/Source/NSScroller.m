/** <title>NSScroller</title>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   A completely rewritten version of the original source by Scott Christley.
   Date: July 1997
   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: August 1998
   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: Mar 1999 - Use flipped views and make conform to spec

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

#include <math.h>

#import <Foundation/NSDate.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSDebug.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSButtonCell.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSScroller.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSWindow.h"

#import "GNUstepGUI/GSTheme.h"

NSString *NSPreferredScrollerStyleDidChangeNotification =
  @"NSPreferredScrollerStyleDidChangeNotification";

/**<p>TODO Description</p>
 */
@implementation NSScroller

/*
 * Class variables
 */

/* button cells used by scroller instances to draw scroller buttons and knob. */
static NSButtonCell	*upCell = nil;
static NSButtonCell	*downCell = nil;
static NSButtonCell	*leftCell = nil;
static NSButtonCell	*rightCell = nil;
static NSCell	*horizontalKnobCell = nil;
static NSCell	*verticalKnobCell = nil;
static NSCell	*horizontalKnobSlotCell = nil;
static NSCell	*verticalKnobSlotCell = nil;
static CGFloat	scrollerWidth = 0.0;
/**
 * This is the amount (in userspace points) by which the knob slides over the
 * button ends of the track. Typical use would be to set it to 1 when both
 * the knob and the buttons have a 1-point border, so that when the knob is
 * at its maximum, it overlaps the button by 1 point giving a resulting
 * 1-point wide border.
 */
static CGFloat  scrollerKnobOvershoot = 0.0;

/* This is the distance by which buttons are offset inside the scroller slot.
 */
static float	buttonsOffset = 1.0; // buttonsWidth = sw - 2*buttonsOffset

+ (void) _themeWillDeactivate: (NSNotification*)n
{
  /* Clear cached information from the old theme ... will get info from
   * the new theme as required.
   */
  scrollerWidth = 0.0;
  upCell = nil;
  downCell = nil;
  leftCell = nil;
  rightCell = nil;
  horizontalKnobCell = nil;
  verticalKnobCell = nil;
  horizontalKnobSlotCell = nil;
  verticalKnobSlotCell = nil;
}

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSScroller class])
    {
      [self setVersion: 1];
      [[NSNotificationCenter defaultCenter] addObserver: self
	selector: @selector(_themeWillDeactivate:)
	name: GSThemeWillDeactivateNotification
	object: nil];
    }
}

/**<p>Returns the NSScroller's width. By default 18.</p>
   <p>Subclasses can override this to provide different scrollbar width.  But
   you may need to also override -drawParts .</p>
 */
+ (CGFloat) scrollerWidth
{
  return [self scrollerWidthForControlSize: NSRegularControlSize];
}

+ (CGFloat) scrollerWidthForControlSize: (NSControlSize)controlSize
{
  // FIXME
  if (scrollerWidth == 0.0)
    {
      scrollerWidth = [[GSTheme theme] defaultScrollerWidth];  
    }
  return scrollerWidth;
}

+ (NSScrollerStyle)preferredScrollerStyle
{
  // FIXME: a theme should define this?
  return NSScrollerStyleLegacy;
}

- (BOOL) isFlipped
{
  return YES;
}

- (BOOL) acceptsFirstMouse: (NSEvent *)theEvent
{
  return YES;
}

- (BOOL) acceptsFirstResponder
{
  return NO;
}

/** <p>Returns the position of the NSScroller's arrows used for scrolling 
    By default the arrow position is set to <ref type="type" 
    id="NSScrollArrowPosition">NSScrollerArrowsMinEnd</ref> if the 
    scrolletr is a horizontal scroller and <ref type="type" 
    id="NSScrollArrowPosition">NSScrollerArrowsMaxEnd</ref> if the scroller
    is a vertical scroller. See <ref type="type" id="NSScrollArrowPosition">
    NSScrollArrowPosition</ref> for more informations.</p>
    <p>See Also: -arrowsPosition</p>
 */
- (NSScrollArrowPosition) arrowsPosition
{
  return _arrowsPosition;
}

- (NSUsableScrollerParts) usableParts
{
  return _usableParts;
}

/**<p>Returns a float value ( between 0.0 and 1.0 ) indicating the ratio
   between the NSScroller length and the knob length</p>
 */
- (CGFloat) knobProportion
{
  return _knobProportion;
}

/**<p>Returns the part of the NSScroller that have been hit ( mouse down )
   See <ref type="type" id="NSScrollerPart">NSScrollerPart</ref> for more 
   information </p><p>See Also: -highlight: [NSResponder-mouseDown:]</p>
 */
- (NSScrollerPart) hitPart
{
  return _hitPart;
}

- (double) doubleValue
{
  return _doubleValue;
}

- (float) floatValue
{
  return _doubleValue;
}

- (void) setAction: (SEL)action
{
  _action = action;
}

- (SEL) action
{
  return _action;
}

- (void) setTarget: (id)target
{
  _target = target;
}

- (id) target
{
  return _target;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];

  if ([aCoder allowsKeyedCoding])
    {
      if (_target)
        {
          [aCoder encodeObject: _target forKey: @"NSTarget"];
        }
      if (_action != NULL)
        {
          [aCoder encodeObject: NSStringFromSelector(_action) forKey: @"NSAction"];
        }
      [aCoder encodeFloat: [self floatValue] forKey: @"NSCurValue"];
      [aCoder encodeFloat: [self knobProportion] * 100.0 forKey: @"NSPercent"];
    }
  else
    {
      BOOL flag;

      [aCoder encodeValueOfObjCType: @encode(NSUInteger) at: &_arrowsPosition];
      flag = _scFlags.isEnabled;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      [aCoder encodeConditionalObject: _target];
      [aCoder encodeValueOfObjCType: @encode(SEL) at: &_action];
      /* We do not save float value, knob proportion. */
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if(nil == self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      if (_frame.size.width > _frame.size.height)
        {
	  _scFlags.isHorizontal = YES;
	}
      else
        {
	  _scFlags.isHorizontal = NO;
	}

      if (_scFlags.isHorizontal)
        {
	  _doubleValue = 0.0;
	}
      else
        {
	  _doubleValue = 1.0;
	}

      if ([aDecoder containsValueForKey: @"NSAction"])
        {
          NSString *action = [aDecoder decodeObjectForKey: @"NSAction"];
          if (action != nil)
            {
              [self setAction: NSSelectorFromString(action)];
            }
        }
      if ([aDecoder containsValueForKey: @"NSTarget"])
        {
          id target = [aDecoder decodeObjectForKey: @"NSTarget"];
          [self setTarget: target];
        }
      if ([aDecoder containsValueForKey: @"NSCurValue"])
        {
	  float value = [aDecoder decodeFloatForKey: @"NSCurValue"];
          [self setFloatValue: value];
	}
      if ([aDecoder containsValueForKey: @"NSPercent"])
        {
	  float percent = [aDecoder decodeFloatForKey: @"NSPercent"];
          [self setKnobProportion: percent / 100.0];
	}

      if ([aDecoder containsValueForKey: @"NSsFlags"])
        {
          int flags;

	  flags = [aDecoder decodeIntForKey: @"NSsFlags"];
	  // is horiz is set above...
          [self setControlTint: ((flags >> 16) & 7)];
          [self setArrowsPosition: ((flags >> 29) & 3)];
          _usableParts = ((flags >> 27) & 3);
	}
      if ([aDecoder containsValueForKey: @"NSsFlags2"])
        {
          int flags2;

	  flags2 = [aDecoder decodeIntForKey: @"NSsFlags2"];
          [self setControlSize: ((flags2 >> 26) & 3)];
	}

      // setup...
      _hitPart = NSScrollerNoPart;

      [self drawParts];
      [self checkSpaceForParts];
    }
  else
    {
      BOOL flag;

      if (_frame.size.width > _frame.size.height)
        {
	  _scFlags.isHorizontal = YES;
	}
      else
        {
	  _scFlags.isHorizontal = NO;
	}

      if (_scFlags.isHorizontal)
        {
	  _doubleValue = 0.0;
	}
      else
        {
	  _doubleValue = 1.0;
	}
      
      [aDecoder decodeValueOfObjCType: @encode(NSUInteger)
				   at: &_arrowsPosition];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _scFlags.isEnabled = flag;
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_target];
      // Undo RETAIN by decoder
      TEST_RELEASE(_target);
      [aDecoder decodeValueOfObjCType: @encode(SEL) at: &_action];

      _hitPart = NSScrollerNoPart;

      [self drawParts];
      [self checkSpaceForParts];
    }

  return self;
}

- (BOOL) isOpaque
{
  return YES;
}

- (id) initWithFrame: (NSRect)frameRect
{
  BOOL isHorizontal;

  /*
   * determine the orientation of the scroller and adjust it's size accordingly
   */
  if (frameRect.size.width > frameRect.size.height)
    {
      isHorizontal = YES;
      frameRect.size.height = [[self class] scrollerWidth];
    }
  else
    {
      isHorizontal = NO;
      frameRect.size.width = [[self class] scrollerWidth];
    }

  self = [super initWithFrame: frameRect];
  if (!self)
    return nil;

  _scFlags.isHorizontal = isHorizontal;
  if (_scFlags.isHorizontal)
    {
      _arrowsPosition = NSScrollerArrowsMinEnd;
      _doubleValue = 0.0;
    }
  else
    {
      _arrowsPosition = NSScrollerArrowsMaxEnd;
      _doubleValue = 1.0;
    }

  _hitPart = NSScrollerNoPart;
  [self setEnabled: NO];
  [self drawParts];
  [self checkSpaceForParts];

  return self;
}

/**
 *  Cache images for scroll arrows and knob.  If you override +scrollerWidth
 *  you may need to override this as well (to provide images for the new
 *  width).  However, if you do so, you must currently also override
 *  -drawArrow:highlight: and -drawKnob: .
 */
- (void) drawParts
{
  NSUserDefaults	*defs;
  GSTheme 		*theme;

  if (upCell)
    return;

  theme = [GSTheme theme];
  defs = [NSUserDefaults standardUserDefaults];
  if ([defs objectForKey: @"GSScrollerButtonsOffset"] != nil)
    {
      buttonsOffset = [defs floatForKey: @"GSScrollerButtonsOffset"];
    }
  else
    {
      buttonsOffset = 1.0;
    }
  if ([defs objectForKey: @"GSScrollerKnobOvershoot"] != nil)
    {
      scrollerKnobOvershoot = [defs floatForKey: @"GSScrollerKnobOvershoot"];
    }
  else
    {
      scrollerKnobOvershoot = 0.0;
    }

  upCell
    = [theme cellForScrollerArrow: NSScrollerDecrementArrow horizontal:NO];
  downCell
    = [theme cellForScrollerArrow: NSScrollerIncrementArrow horizontal:NO];
  leftCell
    = [theme cellForScrollerArrow: NSScrollerDecrementArrow horizontal:YES];
  rightCell
    = [theme cellForScrollerArrow: NSScrollerIncrementArrow horizontal:YES];
  verticalKnobCell = [theme cellForScrollerKnob: NO];
  horizontalKnobCell = [theme cellForScrollerKnob: YES];
  verticalKnobSlotCell = [theme cellForScrollerKnobSlot: NO];
  horizontalKnobSlotCell = [theme cellForScrollerKnobSlot: YES];

  [downCell setContinuous: YES];
  [downCell sendActionOn: (NSLeftMouseDownMask | NSPeriodicMask)];
  [downCell setPeriodicDelay: 0.3 interval: 0.03];
           
  [leftCell setContinuous: YES];
  [leftCell sendActionOn: (NSLeftMouseDownMask | NSPeriodicMask)];
  [leftCell setPeriodicDelay: 0.3 interval: 0.03];
           
  [rightCell setContinuous: YES];
  [rightCell sendActionOn: (NSLeftMouseDownMask | NSPeriodicMask)];
  [rightCell setPeriodicDelay: 0.3 interval: 0.03];
  
  [upCell setContinuous: YES];
  [upCell sendActionOn: (NSLeftMouseDownMask | NSPeriodicMask)];
  [upCell setPeriodicDelay: 0.3 interval: 0.03];
}         
          
- (void) _setTargetAndActionToCells
{
  [upCell setTarget: _target];
  [upCell setAction: _action];

  [downCell setTarget: _target];
  [downCell setAction: _action];

  [leftCell setTarget: _target];
  [leftCell setAction: _action];

  [rightCell setTarget: _target];
  [rightCell setAction: _action];

  [horizontalKnobCell setTarget: _target];
  [horizontalKnobCell setAction: _action];
  
  [verticalKnobCell setTarget:_target];
  [horizontalKnobCell setTarget:_target];
}

- (void) checkSpaceForParts
{
  NSSize frameSize = _frame.size;
  CGFloat size = (_scFlags.isHorizontal ? frameSize.width : frameSize.height);
  CGFloat buttonsWidth = [[self class] scrollerWidth] - 2*buttonsOffset;

  if (_arrowsPosition == NSScrollerArrowsNone)
    {
      if (size >= buttonsWidth + 3)
	{
	  _usableParts = NSAllScrollerParts;
	}
      else
	{
	  _usableParts = NSNoScrollerParts;
	}
    }
  else
    {
      if (size >= 4 /* spacing */ + 1 /* min. scroll area */ + buttonsWidth * 3)
	{
	  _usableParts = NSAllScrollerParts;
	}
      else if (size >= 3 /* spacing */ + buttonsWidth * 2)
	{
	  _usableParts = NSOnlyScrollerArrows;
	}
      else
	{
	  _usableParts = NSNoScrollerParts;
	}
    }
}

- (void) setEnabled: (BOOL)flag
{
  if (_scFlags.isEnabled == flag)
    {
      return;
    }

  _scFlags.isEnabled = flag;
  [self setNeedsDisplay: YES];
}

/** <p>Sets the position of the NSScroller arrows used for scrolling to 
    <var>where</var> and marks self for display. By default the arrow position
    is set to <ref type="type" id="NSScrollArrowPosition">
    NSScrollerArrowsMinEnd</ref> if the scroller is a horizontal scroller
    and <ref type="type" id="NSScrollArrowPosition">NSScrollerArrowsMaxEnd
    </ref> if the scroller is a vertical scroller. See <ref type="type"
    id="NSScrollArrowPosition">NSScrollArrowPosition</ref> for more
    informations.</p><p>See Also: -arrowsPosition</p>
 */
- (void) setArrowsPosition: (NSScrollArrowPosition)where
{
  if (_arrowsPosition == where)
    {
      return;
    }
  
  _arrowsPosition = where;
  [self setNeedsDisplay: YES];
}

- (void) setFloatValue: (float)aFloat
{
  [self setDoubleValue: aFloat];
}

- (void) setDoubleValue: (double)aDouble
{
  if (_doubleValue == aDouble)
    {
      /* Most likely our trackKnob method initiated this via NSScrollView */
      return;
    }
  if (aDouble < 0.0)
    {
      _doubleValue = 0.0;
    }
  else if (aDouble > 1.0)
    {
      _doubleValue = 1.0;
    }
  else
    {
      _doubleValue = aDouble;
    }

  [self setNeedsDisplayInRect: [self rectForPart: NSScrollerKnobSlot]];
}

- (void) setKnobProportion: (CGFloat)proportion
{
  if (_knobProportion == proportion)
    {
      /* Most likely our trackKnob method initiated this via NSScrollView */
      return;
    }

  if (proportion < 0.0)
    {
      _knobProportion = 0.0;
    }
  else if (proportion > 1.0)
    {
      _knobProportion = 1.0;
    }
  else
    {
      _knobProportion = proportion;
    }
  [self setNeedsDisplayInRect: [self rectForPart: NSScrollerKnobSlot]];

  // Handle the case when parts should disappear
  if (_knobProportion == 1.0)
    {
      [self setEnabled: NO];
    }
  else
    {
      [self setEnabled: YES];
    }
}

- (void) setFloatValue: (float)aFloat knobProportion: (CGFloat)ratio
{
  if (_hitPart == NSScrollerNoPart)
    {
      [self setKnobProportion: ratio];
    }
  else
    {
      _pendingKnobProportion = ratio;
    }

  // Don't set float value if knob is being dragged
  if (_hitPart != NSScrollerKnobSlot && _hitPart != NSScrollerKnob)
    {
      /* Make sure we mark ourselves as needing redisplay.  */
      _doubleValue = -1;

      [self setFloatValue: aFloat];
    }
}

- (void) setFrame: (NSRect)frameRect
{
  /*
   * determine the orientation of the scroller and adjust it's size accordingly
   */
  if (frameRect.size.width > frameRect.size.height)
    {
      _scFlags.isHorizontal = YES;
      frameRect.size.height = [[self class] scrollerWidth];
    }
  else
    {
      _scFlags.isHorizontal = NO;
      frameRect.size.width = [[self class] scrollerWidth];
    }

  [super setFrame: frameRect];

  if (_arrowsPosition != NSScrollerArrowsNone)
    {
      if (_scFlags.isHorizontal)
	{
	  _arrowsPosition = NSScrollerArrowsMinEnd;
	}
      else
	{
	  _arrowsPosition = NSScrollerArrowsMaxEnd;
	}
    }

  _hitPart = NSScrollerNoPart;
  [self checkSpaceForParts];
}

- (void) setFrameSize: (NSSize)size
{
  /*
   * determine the orientation of the scroller and adjust it's size accordingly
   */
  if (size.width > size.height)
    {
      _scFlags.isHorizontal = YES;
      size.height = [[self class] scrollerWidth];
    }
  else
    {
      _scFlags.isHorizontal = NO;
      size.width = [[self class] scrollerWidth];
    }

  [super setFrameSize: size];

  if (_arrowsPosition != NSScrollerArrowsNone)
    {
      if (_scFlags.isHorizontal)
	{
	  _arrowsPosition = NSScrollerArrowsMinEnd;
	}
      else
	{
	  _arrowsPosition = NSScrollerArrowsMaxEnd;
	}
    }

  _hitPart = NSScrollerNoPart;
  [self checkSpaceForParts];
}

/**<p>Returns the NSScroller's part under the point <var>thePoint</var>.
   See <ref type="type" id="NSScrollerPart">NSScrollerPart</ref> for more 
   informations</p>   
 */
- (NSScrollerPart) testPart: (NSPoint)thePoint
{
  /*
   * return what part of the scroller the mouse hit
   */
  NSRect rect;

  /* thePoint is in window's coordinate system; convert it to
   * our own coordinate system.  */
  thePoint = [self convertPoint: thePoint fromView: nil];

  if (thePoint.x <= 0 || thePoint.x >= _frame.size.width
    || thePoint.y <= 0 || thePoint.y >= _frame.size.height)
    return NSScrollerNoPart;

  rect = [self rectForPart: NSScrollerDecrementLine];
  if ([self mouse: thePoint inRect: rect])
    return NSScrollerDecrementLine;

  rect = [self rectForPart: NSScrollerIncrementLine];
  if ([self mouse: thePoint inRect: rect])
    return NSScrollerIncrementLine;

  rect = [self rectForPart: NSScrollerKnob];
  if ([self mouse: thePoint inRect: rect])
    return NSScrollerKnob;

  rect = [self rectForPart: NSScrollerDecrementPage];
  if ([self mouse: thePoint inRect: rect])
    return NSScrollerDecrementPage;

  rect = [self rectForPart: NSScrollerIncrementPage];
  if ([self mouse: thePoint inRect: rect])
    return NSScrollerIncrementPage;

  rect = [self rectForPart: NSScrollerKnobSlot];
  if ([self mouse: thePoint inRect: rect])
    return NSScrollerKnobSlot;

  return NSScrollerNoPart;
}

- (double) _doubleValueForMousePoint: (NSPoint)point
{
  NSRect knobRect = [self rectForPart: NSScrollerKnob];
  NSRect slotRect = [self rectForPart: NSScrollerKnobSlot];
  float position;
  float min_pos;
  float max_pos;

  /*
   * Compute limits and mouse position
   */
  if (_scFlags.isHorizontal)
    {
      min_pos = NSMinX(slotRect) + NSWidth(knobRect) / 2;
      max_pos = NSMaxX(slotRect) - NSWidth(knobRect) / 2;
      position = point.x;
    }
  else
    {
      min_pos = NSMinY(slotRect) + NSHeight(knobRect) / 2;
      max_pos = NSMaxY(slotRect) - NSHeight(knobRect) / 2;
      position = point.y;
    }

  /*
   * Compute float value
   */

  if (position <= min_pos)
    return 0;
  if (position >= max_pos)
    return 1;
  return (position - min_pos) / (max_pos - min_pos);
}


- (void) mouseDown: (NSEvent*)theEvent
{
  if (!_scFlags.isEnabled)
    {
      [super mouseDown: theEvent];
      return;
    }

  NSPoint location = [theEvent locationInWindow];
  _hitPart = [self testPart: location];
  [self _setTargetAndActionToCells];

  switch (_hitPart)
    {
      case NSScrollerIncrementLine:
      case NSScrollerDecrementLine:
      case NSScrollerIncrementPage:
      case NSScrollerDecrementPage:
	[self trackScrollButtons: theEvent];
	break;

      case NSScrollerKnob:
	[self trackKnob: theEvent];
	break;

      case NSScrollerKnobSlot:
	{
	  double doubleValue = [self _doubleValueForMousePoint: 
				     [self convertPoint: location
					   fromView: nil]];
	  if (doubleValue != _doubleValue)
	    {
	      const BOOL scrollsToPoint =
		![[GSTheme theme] scrollerScrollsByPageForScroller: self];

	      if (scrollsToPoint)
		{
		  /* NeXTstep style is to scroll to point.
		   */
	          [self setDoubleValue: doubleValue];
	          [self sendAction: _action to: _target];
                  // And then track the knob
                  [self trackKnob: theEvent];
		}
	      else
		{
		  /* Windows style is to scroll by a page.
		   */
		  if (doubleValue > _doubleValue)
		    {
		      _hitPart = NSScrollerIncrementPage;
		    }
		  else
		    {
		      _hitPart = NSScrollerDecrementPage;
		    }
	          [self sendAction: _action to: _target];
                  /* FIXME: Here we should not track the knob but keep on moving it
                     towards the mouse until the button goes up. If the mouse moves,
                     move towards it, but only if this is in the original direction.
                   */
		}
	    }
	  break;
	}

      case NSScrollerNoPart:
	break;
    }

  _hitPart = NSScrollerNoPart;
  if (_pendingKnobProportion)
    {
      [self setKnobProportion: _pendingKnobProportion];
      _pendingKnobProportion = 0.0;
    }
  else
    {
      [self setNeedsDisplay:YES];
    }
}

- (void) trackKnob: (NSEvent*)theEvent
{
  NSUInteger eventMask = NSLeftMouseDownMask | NSLeftMouseUpMask
			  | NSLeftMouseDraggedMask | NSFlagsChangedMask;
  NSPoint	point;
  float		lastPosition;
  float		newPosition;
  double	doubleValue;
  float		offset;
  float		initialOffset;
  NSEvent	*presentEvent = theEvent;
  NSEventType	eventType = [theEvent type];
  NSRect	knobRect;
  NSUInteger	flags = [theEvent modifierFlags];

  knobRect = [self rectForPart: NSScrollerKnob];

  point = [self convertPoint: [theEvent locationInWindow] fromView: nil];
  if (_scFlags.isHorizontal)
    {
      lastPosition = NSMidX(knobRect);
      offset = lastPosition - point.x;
    }
  else
    {
      lastPosition = NSMidY(knobRect);
      offset = lastPosition - point.y;
    }

  initialOffset = offset; /* Save the initial offset value */
  _hitPart = NSScrollerKnob;

  do
    {
       /* Inner loop that gets and (quickly) handles all events that have
          already arrived. */
       while (theEvent && eventType != NSLeftMouseUp)
         {
           /* Note the event here. Don't do any expensive handling. */
	   if (eventType == NSFlagsChanged)
             flags = [theEvent modifierFlags];
	   presentEvent = theEvent;

           theEvent = [NSApp nextEventMatchingMask: eventMask
                         untilDate: [NSDate distantPast] /* Only get events that have already arrived. */
                         inMode: NSEventTrackingRunLoopMode
                         dequeue: YES];
	   eventType = [theEvent type];
         }

       /* 
        * No more events right now. Do expensive handling, like drawing, 
	* here. 
	*/
       point = [self convertPoint: [presentEvent locationInWindow] 
			 fromView: nil];

       if (_scFlags.isHorizontal)
         newPosition = point.x + offset;
       else
	 newPosition = point.y + offset;

       if (newPosition != lastPosition)
         {
           if (flags & NSAlternateKeyMask)
	     {
	       float	diff;

	       diff = newPosition - lastPosition;
	       diff = diff * 3 / 4;
	       offset -= diff;
	       newPosition -= diff;
	     }
	   else /* Ok, we are no longer doing slow scrolling, lets go back 
		   to our original offset. */
	     {
	       offset = initialOffset;
	     }

           // only one coordinate (X or Y) is used to compute doubleValue.
           point = NSMakePoint(newPosition, newPosition);
	   doubleValue = [self _doubleValueForMousePoint: point];

	   if (doubleValue != _doubleValue)
	     {
	       [self setDoubleValue: doubleValue];
	       [self sendAction: _action to: _target];
	     }
	      
	     lastPosition = newPosition;
         }

       /* 
	* If our current event is actually the mouse up (perhaps the inner 
	* loop got to this point) we want to update with the last info and 
	* then quit.
	*/
       if (eventType == NSLeftMouseUp)
         break;

       /* Get the next event, blocking if necessary. */
       theEvent = [NSApp nextEventMatchingMask: eventMask
                     untilDate: [NSDate distantFuture] /* No limit, block until we get an event. */
                     inMode: NSEventTrackingRunLoopMode
                     dequeue: YES];
       eventType = [theEvent type];
  } while (eventType != NSLeftMouseUp);
}

- (void) trackScrollButtons: (NSEvent*)theEvent
{
  id theCell = nil;

  NSDebugLog (@"trackScrollButtons");

  _hitPart = [self testPart: [theEvent locationInWindow]];

  /*
   * A hit on a scroller button should be a page movement
   * if the alt key is pressed.
   */
  switch (_hitPart)
    {
      case NSScrollerIncrementLine:
        if ([theEvent modifierFlags] & NSAlternateKeyMask)
	  {
	    _hitPart = NSScrollerIncrementPage;
	  }
	/* Fall through to next case */
      case NSScrollerIncrementPage:
	theCell = (_scFlags.isHorizontal ? rightCell : downCell);
	break;

      case NSScrollerDecrementLine:
	if ([theEvent modifierFlags] & NSAlternateKeyMask)
	  {
	    _hitPart = NSScrollerDecrementPage;
	  }
	/* Fall through to next case */
      case NSScrollerDecrementPage:
	theCell = (_scFlags.isHorizontal ? leftCell : upCell);
	break;

      default:
	theCell = nil;
	break;
    }

  /*
   * If we don't find a cell this has been all for naught, but we 
   * shouldn't ever be in that situation.
   */
  if (theCell)
    {
      NSRect rect = [self rectForPart: _hitPart];

      [self lockFocus];

      [theCell highlight: YES withFrame: rect inView: self];
      [_window flushWindow];

      NSDebugLog (@"tracking cell %@", theCell);

      /*
       * The "tracking" in this method actually takes place within 
       * NSCell's trackMouse: method. 
       */
      [theCell trackMouse: theEvent
		   inRect: rect
		   ofView: self
	     untilMouseUp: YES];

      [theCell highlight: NO withFrame: rect inView: self];
      [_window flushWindow];

      [self unlockFocus];
    }

  NSDebugLog (@"return from trackScrollButtons");
}

/*
 *	draw the scroller
 */
- (void) drawRect: (NSRect)rect
{
  if (!_scFlags.isEnabled)
    {
      NSRect rect1 = NSIntersectionRect(rect, NSInsetRect(_bounds,
                                                  buttonsOffset, buttonsOffset));
      [self drawKnobSlotInRect: rect1
                     highlight: NO];
    }
  else
    {
      [[GSTheme theme] drawScrollerRect: rect
                                 inView: self
                                hitPart: _hitPart
                           isHorizontal: _scFlags.isHorizontal];
    }
}

/**<p>(Un)Highlight the button specified by <var>whichButton</var>.
   <var>whichButton</var> should be <ref type="type" id="NSScrollerArrow">
   NSScrollerDecrementArrow</ref> or <ref type="type" id="NSScrollerArrow">
   NSScrollerIncrementArrow</ref></p>
   <p>See Also: [NSCell-setHighlighted:] [NSCell-drawWithFrame:inView:]</p>
 */
- (void) drawArrow: (NSScrollerArrow)whichButton highlight: (BOOL)flag
{
  NSRect rect = [self rectForPart: (whichButton == NSScrollerIncrementArrow
		? NSScrollerIncrementLine : NSScrollerDecrementLine)];
  id theCell = nil;

  NSDebugLLog (@"NSScroller", @"position of %s cell is (%f, %f)",
	(whichButton == NSScrollerIncrementArrow ? "increment" : "decrement"),
	rect.origin.x, rect.origin.y);

  if (upCell == nil)
    {
      [self drawParts];
      [self checkSpaceForParts];
    }
  switch (whichButton)
    {
      case NSScrollerDecrementArrow:
	theCell = (_scFlags.isHorizontal ? leftCell : upCell);
	break;
      case NSScrollerIncrementArrow:
	theCell = (_scFlags.isHorizontal ? rightCell : downCell);
	break;
    }

  [theCell setHighlighted: flag];
  [theCell drawWithFrame: rect  inView: self];
}

/**<p>Draws the knob</p>
 */
- (void) drawKnob
{
  if (!_scFlags.isEnabled)
    {
      return;
    }

  if (upCell == nil)
    {
      [self drawParts];
      [self checkSpaceForParts];
    }
  if (_scFlags.isHorizontal)
    [horizontalKnobCell drawWithFrame: [self rectForPart: NSScrollerKnob]
			       inView: self];
  else
    [verticalKnobCell drawWithFrame: [self rectForPart: NSScrollerKnob]
			     inView: self];
}

- (void) drawKnobSlot
{
  [self drawKnobSlotInRect: [self rectForPart: NSScrollerKnobSlot] 
                 highlight: NO];
}

- (void) drawKnobSlotInRect: (NSRect)slotRect highlight: (BOOL)flag
{
  // FIXME: Not sure whether this method does the right thing

  if (upCell == nil)
    {
      [self drawParts];
      [self checkSpaceForParts];
    }
  if (_scFlags.isHorizontal)
    {
      [horizontalKnobSlotCell setHighlighted: flag];
      [horizontalKnobSlotCell drawWithFrame: slotRect inView: self];
    }
  else
    {
      [verticalKnobSlotCell setHighlighted: flag];
      [verticalKnobSlotCell drawWithFrame: slotRect inView: self];
    }
}

/**<p>Highlights the button whose under the mouse. Does nothing if the mouse
   is not under a button</p><p>See Also: -drawArrow:highlight:</p>
 */
- (void) highlight: (BOOL)flag
{
  switch (_hitPart)
    {
      case NSScrollerIncrementLine:
      case NSScrollerIncrementPage:
	[self drawArrow: NSScrollerIncrementArrow highlight: flag];
	break;

      case NSScrollerDecrementLine:
      case NSScrollerDecrementPage:
	[self drawArrow: NSScrollerDecrementArrow highlight: flag];
	break;

      default:	/* No button currently hit for highlighting. */
	break;
    }
}

/**
 */
- (NSRect) rectForPart: (NSScrollerPart)partCode
{
  NSRect scrollerFrame = _frame;
  CGFloat x, y;
  CGFloat width, height;
  CGFloat buttonsWidth;
  CGFloat buttonsSize;  
  BOOL	arrowsSameEnd = [[GSTheme theme] scrollerArrowsSameEndForScroller: self];

  if (upCell == nil)
    {
      [self drawParts];
      [self checkSpaceForParts];
    }
  buttonsWidth = ([[self class] scrollerWidth] - 2 * buttonsOffset);
  x = y = buttonsOffset;
  buttonsSize = 2 * buttonsWidth + 2 * buttonsOffset;

  /*
   * Assign to `width' and `height' values describing
   * the width and height of the scroller regardless
   * of its orientation.
   * but keeps track of the scroller's orientation.
   */
  if (_scFlags.isHorizontal)
    {
      width = scrollerFrame.size.height - 2 * buttonsOffset;
      height = scrollerFrame.size.width - 2 * buttonsOffset;
    }
  else
    {
      width = scrollerFrame.size.width - 2 * buttonsOffset;
      height = scrollerFrame.size.height - 2 * buttonsOffset;
    }

  /*
   * The x, y, width and height values are computed below for the vertical
   * scroller.  The height of the scroll buttons is assumed to be equal to
   * the width.
   */
  switch (partCode)
    {
      case NSScrollerKnob:
	{
	  CGFloat knobHeight, knobPosition, slotHeight;

	  if (_usableParts == NSNoScrollerParts
	    || _usableParts == NSOnlyScrollerArrows)
	    {
	      return NSZeroRect;
	    }

	  /* calc the slot Height */
	  slotHeight = height - (_arrowsPosition == NSScrollerArrowsNone
	    ?  0 : buttonsSize);
	  knobHeight = _knobProportion * slotHeight;
	  knobHeight = floor(knobHeight);
	  if (knobHeight < buttonsWidth)
	    knobHeight = buttonsWidth;

	  /* calc knob's position */

	  {
	    CGFloat knobOvershootAbove = scrollerKnobOvershoot;
	    CGFloat knobOvershootBelow = scrollerKnobOvershoot;
	    if (arrowsSameEnd
		&& _arrowsPosition == NSScrollerArrowsMinEnd)
	      {
		knobOvershootBelow = 0;
	      }
	    else if (arrowsSameEnd
		     && _arrowsPosition == NSScrollerArrowsMaxEnd)
	      {
		knobOvershootAbove = 0;
	      }
	    else if (_arrowsPosition == NSScrollerArrowsNone)
	      {
		knobOvershootAbove = 0;
		knobOvershootBelow = 0;
	      }

	    knobPosition = floor((float)_doubleValue * (slotHeight - knobHeight + knobOvershootAbove + knobOvershootBelow))
	      - knobOvershootAbove;
	  }

	  if (arrowsSameEnd)
	    {
	      if (_arrowsPosition == NSScrollerArrowsMinEnd)
		{
		  y += buttonsSize;
		}
	    }
	  else
	    {
	      y += buttonsWidth + buttonsOffset;
	    }
	  y += knobPosition;
	  width = buttonsWidth;
	  height = knobHeight;
	  break;
	}

      case NSScrollerKnobSlot:
	/*
	 * if the scroller does not have buttons the slot completely
	 * fills the scroller.
	 */
	if (_usableParts == NSNoScrollerParts
	  || _arrowsPosition == NSScrollerArrowsNone)
	  {
	    break;
	  }
	width = buttonsWidth;
	height -= buttonsSize;
	if (arrowsSameEnd)
	  {
            if (_arrowsPosition == NSScrollerArrowsMinEnd)
              {
                y += buttonsSize;
              }
	  }
        else
          {
            y += buttonsWidth + buttonsOffset;
          }
        break;

      case NSScrollerDecrementLine:
      case NSScrollerDecrementPage:
	if (_usableParts == NSNoScrollerParts
	  || _arrowsPosition == NSScrollerArrowsNone)
	  {
	    return NSZeroRect;
	  }
	else if (arrowsSameEnd && _arrowsPosition == NSScrollerArrowsMaxEnd)
	  {
	    y += (height - buttonsSize + buttonsOffset);
	  }
	width = buttonsWidth;
	height = buttonsWidth;
	break;

      case NSScrollerIncrementLine:
      case NSScrollerIncrementPage:
	if (_usableParts == NSNoScrollerParts
	  || _arrowsPosition == NSScrollerArrowsNone)
	  {
	    return NSZeroRect;
	  }
        else if (arrowsSameEnd)
          {
	    if (_arrowsPosition == NSScrollerArrowsMaxEnd)
	      {
	        y += (height - buttonsWidth);
	      }
	    else if (_arrowsPosition == NSScrollerArrowsMinEnd)
	      {
	        y += (buttonsWidth + buttonsOffset);
	      }
          }
        else 
          {
            y += (height - buttonsWidth);
          }
	height = buttonsWidth;
	width = buttonsWidth;
	break;

      case NSScrollerNoPart:
	return NSZeroRect;
    }

  if (_scFlags.isHorizontal)
    {
      return NSMakeRect (y, x, height, width);
    }
  else
    {
      return NSMakeRect (x, y, width, height);
    }
}

- (void) setControlSize: (NSControlSize)controlSize
{
  if (_scFlags.control_size == controlSize)
    return;

  _scFlags.control_size = controlSize;
  [self setNeedsDisplay: YES];
}

- (NSControlSize) controlSize
{
  return _scFlags.control_size;
}

- (void) setControlTint: (NSControlTint)controlTint
{
  if (_scFlags.control_tint == controlTint)
    return;

  _scFlags.control_tint = controlTint;
  [self setNeedsDisplay: YES];
}

- (NSControlTint) controlTint
{
  return _scFlags.control_tint;
}

@end
