/** <title>NSStepperCell</title>

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author: Pierre-Yves Rivaille <pyrivail@ens-lyon.fr>
   Date: 2001
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: August 2006
   
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

#include "config.h"

#import <Foundation/NSValue.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSControl.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSStepperCell.h"
#import "AppKit/NSWindow.h"
#import "GNUstepGUI/GSTheme.h"
#import "GSGuiPrivate.h"

@interface NSStepperCell (Private)
- (void) _increment;
- (void) _decrement;
- (void) setHighlighted: (BOOL)highlight
	       upButton: (BOOL)upButton
	      withFrame: (NSRect)frame
		 inView: (NSView*)controlView;
@end

@implementation NSStepperCell

+ (void) initialize
{
  if (self == [NSStepperCell class])
    {
      [self setVersion: 1];
    }
}

+ (BOOL) prefersTrackingUntilMouseUp
{
  return YES;
}

//
// Initialization
//
- (id) init
{
  self = [super init];
  if (!self)
    return nil;

  [self setIntValue: 0];
  [self setAlignment: NSRightTextAlignment];
  [self setWraps: NO];
  _autorepeat = YES;
  _valueWraps = YES;
  [self setMaxValue: 59];
  [self setMinValue: 0];
  _increment = 1;

  highlightUp = NO;
  highlightDown = NO;

  return self;
}

- (double) maxValue
{
  return _maxValue;
}

- (void) setMaxValue: (double)maxValue
{
  _maxValue = maxValue;
  if ([self doubleValue] > _maxValue)
    [self setDoubleValue: _maxValue];
}

- (double) minValue
{
  return _minValue;
}

- (void) setMinValue: (double)minValue
{
  _minValue = minValue;
  if ([self doubleValue] < _minValue)
    [self setDoubleValue: _minValue];
}

- (void) setObjectValue: (id)anObject
{
  // NOTE: valueWraps has no effect on setObjectValue:
  // FIXME: Copied from NSSliderCell.. can we share the code somehow?

  // If the provided object doesn't respond to doubeValue, or our minValue
  // is greater than our maxValue, we set our value to our minValue
  // (this arbitrary choice matches OS X)
  if ([anObject respondsToSelector: @selector(doubleValue)] == NO ||
      _minValue > _maxValue)
    [super setObjectValue: [NSNumber numberWithDouble: _minValue]];
  else
    {
      double aDouble = [anObject doubleValue];
      if (aDouble < _minValue)
        [super setObjectValue: [NSNumber numberWithDouble: _minValue]];
      else if (aDouble > _maxValue)
        [super setObjectValue: [NSNumber numberWithDouble: _maxValue]];
      else
        [super setObjectValue: anObject];
    }
}

- (double) increment
{
  return _increment;
}

- (void) setIncrement: (double)increment
{
  _increment = increment;
}

- (BOOL) autorepeat
{
  return _autorepeat;
}

- (void) setAutorepeat: (BOOL)autorepeat
{
  _autorepeat = autorepeat;
}

- (BOOL) valueWraps
{
  return _valueWraps;
}

- (void) setValueWraps: (BOOL)valueWraps
{
  _valueWraps = valueWraps;
}

- (void) drawInteriorWithFrame: (NSRect)cellFrame
			inView: (NSView*)controlView
{
  [[GSTheme theme] drawStepperCell: self
                   withFrame: cellFrame
                   inView: controlView
                   highlightUp: highlightUp
                   highlightDown: highlightDown];
}

- (void) getPeriodicDelay: (float*)delay interval: (float*)interval
{
  *delay = 0.5;
  *interval = 0.025;
}

- (BOOL) trackMouse: (NSEvent*)theEvent
	     inRect: (NSRect)cellFrame
	     ofView: (NSView*)controlView
       untilMouseUp: (BOOL)flag
{
  NSPoint location = [theEvent locationInWindow];
  NSPoint point = [controlView convertPoint: location fromView: nil];
  NSPoint last_point = point;
  NSRect upRect;
  NSRect downRect;
  NSRect rect;
  float delay;
  float	interval;
  BOOL overButton = YES;
  unsigned int event_mask = NSLeftMouseUpMask | NSLeftMouseDraggedMask;
  unsigned int periodCount = 0;
  BOOL isDirectionUp;
  BOOL autorepeat = [self autorepeat];
  BOOL mouseWentUp = NO;
  BOOL tracking;

  _mouse_down_flags = [theEvent modifierFlags];

  if ([self isEnabled] == NO)
    return NO;

  if (![controlView mouse: point inRect: cellFrame])
    return NO;	// point is not in cell

  tracking = [self startTrackingAt: point inView: controlView];

  if ([theEvent type] != NSLeftMouseDown)
    return NO;

  upRect = [[GSTheme theme] stepperUpButtonRectWithFrame: cellFrame];
  downRect = [[GSTheme theme] stepperDownButtonRectWithFrame: cellFrame];
  
  // Did the mouse go down in the up or in the down part?
  if (NSMouseInRect(point, upRect, NO))
    {
      isDirectionUp = YES;
      rect = upRect;
    }
  else if (NSMouseInRect(point, downRect, NO))
    {
      isDirectionUp = NO;
      rect = downRect;
    }
  else
    {
      return NO;
    }

  [self setHighlighted: YES
	upButton: isDirectionUp
	withFrame: cellFrame
	inView: controlView];

  if (autorepeat)
    {
      [self getPeriodicDelay: &delay interval: &interval];
      [NSEvent startPeriodicEventsAfterDelay: delay withPeriod: interval];
      event_mask |= NSPeriodicMask;
    }

  while (YES)
    {
      NSEventType eventType;

      theEvent = [NSApp nextEventMatchingMask: event_mask
			untilDate: [NSDate distantFuture]
			inMode: NSEventTrackingRunLoopMode
			dequeue: YES];
      eventType = [theEvent type];

      // Did the mouse go up?
      if (eventType == NSLeftMouseUp)
	{
	  mouseWentUp = YES;
	  break;
	}
      else if (eventType == NSPeriodic)
        {
	  if (periodCount == 4) 
            {
              if (isDirectionUp)
		[self _increment];
	      else
		[self _decrement];

	      [(NSControl*)controlView sendAction: [self action] to: [self target]];
              location = [[controlView window] mouseLocationOutsideOfEventStream];
              last_point = point;
              point = [controlView convertPoint: location fromView: nil];
              periodCount = 0;
	    }
          else
            {
              periodCount++;
            }
	}
      else
        {
	  location = [theEvent locationInWindow];
          last_point = point;
          point = [controlView convertPoint: location fromView: nil];
	}

      if (!flag && ![controlView mouse: point inRect: cellFrame])
	{
          break;
	}

      if (tracking)
        {
          tracking = [self continueTracking: last_point
                                         at: point
                                     inView: controlView];
         }

       if (NSMouseInRect(point, rect, NO) != overButton)
        {
	  overButton = !overButton;
	  if (overButton && autorepeat)
	    {
	      [NSEvent startPeriodicEventsAfterDelay: delay withPeriod: interval];
	      periodCount = 0;
	    }
	  else
	    {
	      [NSEvent stopPeriodicEvents];
	    }
	  [self setHighlighted: overButton
		upButton: isDirectionUp
		withFrame: cellFrame
		inView: controlView];
	}
    }

  if (tracking)
    {
      [self stopTracking: last_point
                      at: point
                  inView: controlView
               mouseIsUp: mouseWentUp];
    }

  if (overButton && autorepeat)
    {
      [NSEvent stopPeriodicEvents];
    }

  if (overButton)
    {
      if (isDirectionUp)
	[self _increment];
      else
	[self _decrement];
      [(NSControl*)controlView sendAction: [self action] to: [self target]];
    }
  
  [self setHighlighted: NO
	upButton: isDirectionUp
	withFrame: cellFrame
	inView: controlView];

  return mouseWentUp;
}

- (NSSize) cellSize
{
  NSRect upRect = [[GSTheme theme]
                    stepperUpButtonRectWithFrame: [_control_view bounds]];
  NSRect downRect = [[GSTheme theme]
                      stepperDownButtonRectWithFrame: [_control_view bounds]];
  NSSize size;

  if (upRect.size.width > downRect.size.width)
    size.width = upRect.size.width;
  else
    size.width = downRect.size.width;

  size.height = upRect.size.height + downRect.size.height;

  return size;
}

//
// NSCoding protocol
//
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];

  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeDouble: [self increment] forKey: @"NSIncrement"];
      [aCoder encodeDouble: [self maxValue] forKey: @"NSMaxValue"];
      [aCoder encodeDouble: [self minValue] forKey: @"NSMinValue"];
      [aCoder encodeBool: [self autorepeat] forKey: @"NSAutorepeat"];
      [aCoder encodeBool: [self valueWraps] forKey: @"NSValueWraps"];
    }
  else
  {
    NSInteger tmp;

    [aCoder encodeValueOfObjCType: @encode(double)
	    at: &_maxValue];
    [aCoder encodeValueOfObjCType: @encode(double)
	    at: &_minValue];
    [aCoder encodeValueOfObjCType: @encode(double)
	    at: &_increment];
    tmp = _autorepeat;
    encode_NSInteger(aCoder, &tmp);
    tmp = _valueWraps;
    encode_NSInteger(aCoder, &tmp);
  }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (nil == self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      _autorepeat = [aDecoder decodeBoolForKey: @"NSAutorepeat"];
      _valueWraps = [aDecoder decodeBoolForKey: @"NSValueWraps"];
      _increment = [aDecoder decodeDoubleForKey: @"NSIncrement"];
      _maxValue = [aDecoder decodeDoubleForKey: @"NSMaxValue"];
      if ([aDecoder containsValueForKey: @"NSMinValue"])
	{
	  _minValue = [aDecoder decodeDoubleForKey: @"NSMinValue"];
	}
    }
  else
    {
      NSInteger tmp;

      [aDecoder decodeValueOfObjCType: @encode(double)
		at: &_maxValue];
      [aDecoder decodeValueOfObjCType: @encode(double)
		at: &_minValue];
      [aDecoder decodeValueOfObjCType: @encode(double)
		at: &_increment];
      decode_NSInteger(aDecoder, &tmp);
      _autorepeat = (BOOL)tmp;
      decode_NSInteger(aDecoder, &tmp);
      _valueWraps = (BOOL)tmp;
    }

  return self;
}

@end

@implementation NSStepperCell (Private)

- (void) _increment
{
  double newValue;
  double maxValue = [self maxValue];
  double minValue = [self minValue];
  double increment = [self increment];

  newValue = [self doubleValue] + increment;
  if ([self valueWraps])
    {
      if (newValue > maxValue)
	newValue = newValue - maxValue + minValue - 1;
      else if (newValue < minValue)
	newValue = newValue + maxValue - minValue + 1;
    }
  [self setDoubleValue: newValue];
}

- (void) _decrement
{
  double newValue;
  double maxValue = [self maxValue];
  double minValue = [self minValue];
  double increment = [self increment];

  newValue = [self doubleValue] - increment;
  if ([self valueWraps])
    {
      if (newValue > maxValue)
	newValue = newValue - maxValue + minValue - 1;
      else if (newValue < minValue)
	newValue = newValue + maxValue - minValue + 1;
    }
  [self setDoubleValue: newValue];
}

- (void) setHighlighted: (BOOL)highlight
	       upButton: (BOOL)upButton
	      withFrame: (NSRect)frame
		 inView: (NSView*)controlView
{
  if (upButton)
    {  
      highlightUp = highlight;
    }
  else
    {
      highlightDown = highlight;
    }

  [controlView setNeedsDisplayInRect: frame];
}

@end
