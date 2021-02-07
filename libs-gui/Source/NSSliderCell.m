/** <title>NSSliderCell</title>

   Copyright (C) 1996,1999 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: September 1997
   Rewrite: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: 1999
  
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

#include <math.h>                  // (float)rintf(float x)
#include "config.h"
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#import <Foundation/NSValue.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSControl.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSSliderCell.h"
#import "AppKit/NSTextFieldCell.h"
#import "AppKit/NSWindow.h"
#import <GNUstepGUI/GSTheme.h>

#import "GSGuiPrivate.h"

#ifndef HAVE_ATAN2F
#define atan2f atan2
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626434
#endif

static inline
double _doubleValueForMousePoint (NSPoint point, NSRect knobRect,
				NSRect slotRect, BOOL isVertical, 
				double minValue, double maxValue, 
				NSSliderCell *theCell, BOOL flipped, 
				BOOL isCircular)
{
  double doubleValue = 0;
  double position;

  if (isCircular)
    {
      NSPoint slotCenter = NSMakePoint(NSMidX(slotRect), NSMidY(slotRect));
      NSPoint pointRelativeToKnobCenter = NSMakePoint(point.x - slotCenter.x,
						      point.y - slotCenter.y);
      if (flipped)
	{
	  pointRelativeToKnobCenter.y *= -1.0;
	}
      doubleValue = atan2f(pointRelativeToKnobCenter.x,
			  pointRelativeToKnobCenter.y) / (2.0 * M_PI);
      if (doubleValue < 0)
	{
	  doubleValue += 1.0;
	}
      // doubleValue is 0 for up, 0.25 for right, 0.5 for down, 0.75 for left, etc.
    }
  else
    {
      // Adjust the point to lie inside the knob slot. We don't
      // have to worry whether the view is flipped or not.
      if (isVertical)
	{
	  if (point.y < slotRect.origin.y + knobRect.size.height / 2)
	    {
	      position = slotRect.origin.y + knobRect.size.height / 2;
	    }
	  else if (point.y > slotRect.origin.y + slotRect.size.height
		   - knobRect.size.height / 2)
	    {
	      position = slotRect.origin.y + slotRect.size.height
		- knobRect.size.height / 2;
	    }
	  else
	    position = point.y;
	  // Compute the double value
	  doubleValue = (position - (slotRect.origin.y + knobRect.size.height/2))
	    / (slotRect.size.height - knobRect.size.height);
	  if (flipped)
	    doubleValue = 1 - doubleValue;
	}
      else
	{
	  if (point.x < slotRect.origin.x + knobRect.size.width / 2)
	    {
	      position = slotRect.origin.x + knobRect.size.width / 2;
	    }
	  else if (point.x > slotRect.origin.x + slotRect.size.width
		   - knobRect.size.width / 2)
	    {
	      position = slotRect.origin.x + slotRect.size.width
		- knobRect.size.width / 2;
	    }
	  else
	    position = point.x;

	  // Compute the double value given the knob size
	  doubleValue = (position - (slotRect.origin.x + knobRect.size.width / 2))
	    / (slotRect.size.width - knobRect.size.width);
	}
    }

  return doubleValue * (maxValue - minValue) + minValue;
}

/**
  <unit>
  <heading>Class Description</heading>

  <p>
  An NSSliderCell controls the behaviour and appearance of an
  associated NSSlider, or a single slider in an NSMatrix.  Tick marks
  are defined in the official standard, but are not implemented in
  GNUstep.
  </p>
  <p> 
  An NSSliderCell can be customized through its
  <code>set...</code> methods.  If these do not provide enough
  customization, a subclass can be created, which overrides any of the
  follwing methods: <code>knobRectFlipped:</code>,
  <code>drawBarInside:flipped:</code>, <code>drawKnob:</code>, or
  <code>prefersTrackingUntilMouseUp</code>.
  </p>
  </unit> 
*/
@implementation NSSliderCell

+ (void) initialize
{
  if (self == [NSSliderCell class])
    {
      /* Set the class version to 2, as tick information is now 
	 stored in the encoding */
      [self setVersion: 2];
    }
}

- (id) init
{
  self = [self initImageCell: nil];
  if (self == nil)
    return nil;

  _altIncrementValue = -1;
  _isVertical = -1;
  [self setDoubleValue: 0];
  [self setMinValue: 0];
  [self setMaxValue: 1];
  _cell.is_bordered = YES;
  _cell.is_bezeled = NO;
  [self setContinuous: YES];
  [self setSliderType: NSLinearSlider];
      
  _knobCell = [NSCell new];
  _titleCell = [NSTextFieldCell new];
  [_titleCell setTextColor: [NSColor controlTextColor]];
  [_titleCell setStringValue: @""];
  [_titleCell setAlignment: NSCenterTextAlignment];

  return self;
}

- (void) dealloc
{
  RELEASE(_titleCell);
  RELEASE(_knobCell);
  [super dealloc];
}

- (id) copyWithZone:(NSZone *)zone
{
  NSSliderCell *cpy = [super copyWithZone: zone];

  if (cpy != nil)
    {
      /* since NSCells call to NSCopyObject only copies object addresses */
      cpy->_titleCell = [_titleCell copyWithZone: zone];
      cpy->_knobCell = [_knobCell copyWithZone: zone];
    }

  return cpy;
}

- (BOOL) isContinuous
{
  return (_action_mask & NSLeftMouseDraggedMask) != 0;
}

- (void) setContinuous: (BOOL)flag
{
  if (flag)
    {
      _action_mask |= NSLeftMouseDraggedMask;
    }
  else
    {
      _action_mask &= ~NSLeftMouseDraggedMask;
    }
}

- (void) _drawBorderAndBackgroundWithFrame: (NSRect)cellFrame 
                                    inView: (NSView*)controlView
{
  NSBorderType aType;
  
  if (_cell.is_bordered)
    aType = NSLineBorder;
  else if (_cell.is_bezeled)
    aType = NSBezelBorder;
  else
    aType = NSNoBorder;

  [[GSTheme theme] drawSliderBorderAndBackground: aType 
					   frame: cellFrame
					  inCell: self
				    isHorizontal: ![self isVertical]];
}


/** <p>Draws the slider's track, not including the bezel, in <var>aRect</var>
  <var>flipped</var> indicates whether the control view has a flipped 
   coordinate system.</p>

  <p>Do not call this method directly, it is provided for subclassing
  only.</p> */
- (void) drawBarInside: (NSRect)rect flipped: (BOOL)flipped
{
  [[GSTheme theme] drawBarInside: rect
		   inCell: self
		   flipped: flipped];
}

/**<p>Returns the rect in which to draw the knob, based on the
  coordinate system of the NSSlider or NSMatrix this NSSliderCell is
  associated with.  <var>flipped</var> indicates whether or not that
  coordinate system is flipped, which can be determined by sending the
  <code>isFlipped</code> message to the associated NSSlider or
  NSMatrix.</p>

  <p>Do not call this method directly.  It is included for subclassing
  only.</p> */
- (NSRect) knobRectFlipped: (BOOL)flipped
{
  NSImage *image = [_knobCell image];
  NSSize size;
  NSPoint origin;
  double doubleValue = _value;
  
  // FIXME: this method needs to be refactored out to GSTheme
  if (_isVertical && flipped)
    {
      doubleValue = _maxValue + _minValue - doubleValue;
    }

  doubleValue = (doubleValue - _minValue) / (_maxValue - _minValue);

  if (image != nil)
    {
      size = [image size];
    }
  else
    {
      size = NSZeroSize;
    }

  if (_isVertical == YES)
    {
      origin = _trackRect.origin;
      origin.x += (_trackRect.size.width - size.width) / 2.0; // center horizontally
      origin.y += (_trackRect.size.height - size.height) * doubleValue;
    }
  else
    {
      origin = _trackRect.origin;
      origin.x += (_trackRect.size.width - size.width) * doubleValue;
      origin.y += (_trackRect.size.height - size.height) / 2.0; // center vertically
    }

  {
    NSRect result = NSMakeRect (origin.x, origin.y, size.width, size.height);

    if ([self controlView])
      {
	result = [[self controlView] centerScanRect: result];
      }

    return result;
  } 
}

/** <p>Calculates the rect in which to draw the knob, then calls
  <code>drawKnob:</code> Before calling this method, a
  <code>lockFocus</code> message must be sent to the cell's control
  view.</p>

  <p>When subclassing NSSliderCell, do not override this method.
  Override <code>drawKnob:</code> instead.</p> <p>See Also: -drawKnob:</p>
*/
- (void) drawKnob
{
  [[GSTheme theme] drawKnobInCell: self];
}

/**<p>Draws the knob in <var>knobRect</var>.  Before calling this
  method, a <code>lockFocus</code> message must be sent to the cell's
  control view.</p>

  <p>Do not call this method directly.  It is included for subclassing
  only.</p> <p>See Also: -drawKnob</p>
*/
- (void) drawKnob: (NSRect)knobRect
{
  [_knobCell drawInteriorWithFrame: knobRect inView: _control_view];
}

- (void) drawInteriorWithFrame: (NSRect)cellFrame inView: (NSView*)controlView
{
  cellFrame = [self drawingRectForBounds: cellFrame];
  _trackRect = cellFrame;

  if (_type == NSCircularSlider)
    {
      NSBezierPath *circle;
      NSPoint knobCenter;
      NSPoint point;
      NSRect knobRect;
      double fraction, angle, radius;
      NSImage *image;

      if (cellFrame.size.width > cellFrame.size.height)
	{
	  knobRect = NSMakeRect(cellFrame.origin.x + ((cellFrame.size.width -
						       cellFrame.size.height) / 2.0),
				cellFrame.origin.y,
				cellFrame.size.height,
				cellFrame.size.height);
	}
      else
	{
	  knobRect = NSMakeRect(cellFrame.origin.x,
				cellFrame.origin.y + ((cellFrame.size.height -
						       cellFrame.size.width) / 2.0),
				cellFrame.size.width,
				cellFrame.size.width);
	}

      if ([self controlView])
	knobRect = [[self controlView] centerScanRect: knobRect];

      image = [NSImage imageNamed: @"common_CircularSlider"];
      if (image != nil)
	{
	  [image drawInRect: knobRect
		   fromRect: NSZeroRect
		  operation: NSCompositeSourceOver
		   fraction: 1.0
	     respectFlipped: YES
		      hints: nil];
	}
      else
	{
	  knobRect = NSInsetRect(knobRect, 1, 1);
	  circle = [NSBezierPath bezierPathWithOvalInRect: knobRect];
	  [[NSColor controlBackgroundColor] set];    
	  [circle fill];
	  [[NSColor blackColor] set];
	  [circle stroke];
	}

      knobCenter = NSMakePoint(NSMidX(knobRect), NSMidY(knobRect));

      fraction = ([self doubleValue] - [self minValue]) /
	([self maxValue] - [self minValue]);
      angle = (fraction * (2.0 * M_PI)) - (M_PI / 2.0);
      radius = (knobRect.size.height / 2) - 6;
      point = NSMakePoint((radius * cos(angle)) + knobCenter.x,
			  (radius * sin(angle)) + knobCenter.y);
          
      image = [NSImage imageNamed: @"common_Dimple"];
	{
	  NSSize size = [image size];
	  NSRect dimpleRect = NSMakeRect(point.x - (size.width / 2.0),
					 point.y - (size.height / 2.0),
					 size.width,
					 size.height);

	  if ([self controlView])
	    dimpleRect = [[self controlView] centerScanRect: dimpleRect];

	  [image drawInRect: dimpleRect
		   fromRect: NSZeroRect
		  operation: NSCompositeSourceOver
		   fraction: 1.0
	     respectFlipped: YES
		      hints: nil];
	}
    }
  else if (_type == NSLinearSlider)
    {
      BOOL vertical = (cellFrame.size.height > cellFrame.size.width);

      if (vertical != _isVertical)
	{
	  NSImage *image;
	  if (vertical == YES)
	    {
	      image = [NSImage imageNamed: @"common_SliderVert"];
	    }
	  else
	    {
	      image = [NSImage imageNamed: @"common_SliderHoriz"];
	    }
	  [_knobCell setImage: image];
	}
      _isVertical = vertical;

      [self drawBarInside: cellFrame flipped: [controlView isFlipped]];

      /* Draw title - Uhmmm - shouldn't this better go into
	 drawBarInside:flipped: ? */
      if (_isVertical == NO)
	{
	  [_titleCell drawInteriorWithFrame: cellFrame inView: controlView];
	}

      [self drawKnob];
    }
}

- (BOOL) isOpaque
{
  return NO;
}

/**<p> Returns the thickness of the slider's knob.  This value is in
  pixels, and is the size of the knob along the slider's track.</p>
  <p>See Also: -setKnobThickness:</p>
*/
- (CGFloat) knobThickness
{
  NSImage *image = [_knobCell image];
  NSSize size;

  if (image != nil)
    {
      size = [image size];
    }
  else 
    {
      return 0;
    }

  return _isVertical ? size.height : size.width;
}

/**<p>Sets the thickness of the knob to <var>thickness</var>, in pixels.
  This value sets the amount of space which the knob takes up in the
  slider's track.</p><p>See Also: -knobThickness</p> 
 */
- (void) setKnobThickness: (CGFloat)thickness
{
  NSImage *image = [_knobCell image];
  NSSize size;

  if (image != nil)
    {
      size = [image size];
    }
  else 
    {
      return;
    }

  if (_isVertical == YES)
    size.height = thickness;
  else
    size.width = thickness;

  [image setSize: size];

  if ((_control_view != nil) &&  
      ([_control_view isKindOfClass: [NSControl class]]))
    {
      [(NSControl*)_control_view updateCell: self];
    }
}

/**<p> Sets the value by which the slider will be be incremented when with the
    ALT key down to <var>increment</var>.</p>
    <p>See Also: -altIncrementValue</p> 
*/
- (void) setAltIncrementValue: (double)increment
{
  _altIncrementValue = increment;
}

/** <p>Returns the minimum value that the slider represents.</p>
    <p>See Also: -setMinValue:</p>
*/
- (double) minValue
{
  return _minValue;
}

/**<p>Returns the maximum value that the slider represents.</p>
   <p>See Also: -setMaxValue:</p>
*/
- (double) maxValue
{
  return _maxValue;
}

/**<p> Sets the minimum value that the sliders represents to
   <var>maxValue</var>.</p><p>See Also: -minValue</p>
*/
- (void) setMinValue: (double)aDouble
{
  _minValue = aDouble;
  if (_minValue > _maxValue)
    {
      _value = _minValue;
    }
  else if (_value < _minValue)
    {
      _value = _minValue;
    }
}

/** <p>Sets the maximum value that the sliders represents to 
    <var>maxValue</var>.</p><p>See Also: -maxValue</p>
*/
- (void) setMaxValue: (double)aDouble
{
  _maxValue = aDouble;
  if (_minValue > _maxValue)
    {
      _value = _minValue;
    }
  else if (_value > _maxValue)
    {
      _value = _maxValue;
    }
}

- (double) doubleValue
{
  return _value;
}

- (void) setDoubleValue: (double)aDouble
{
  if (_minValue > _maxValue)
    {
      return;
    }
  if (aDouble < _minValue)
    {
      _value = _minValue;
    }
  else if (aDouble > _maxValue)
    {
      _value = _maxValue;
    }
  else
    {
      _value = aDouble;
    }
  if ((_control_view != nil) &&  
      ([_control_view isKindOfClass: [NSControl class]]))
    {
      [(NSControl*)_control_view updateCell: self];
    }
}

- (float) floatValue
{
  return _value;
}

- (void) setFloatValue: (float)aFloat
{
  if (_minValue > _maxValue)
    {
      return;
    }
  if (aFloat < _minValue)
    {
      _value = _minValue;
    }
  else if (aFloat > _maxValue)
    {
      _value = _maxValue;
    }
  else
    {
      _value = aFloat;
    }
  if ((_control_view != nil) &&  
      ([_control_view isKindOfClass: [NSControl class]]))
    {
      [(NSControl*)_control_view updateCell: self];
    }
}

- (int) intValue
{
  return _value;
}

- (void) setIntValue: (int)anInt
{
  if (_minValue > _maxValue)
    {
      return;
    }
  if (anInt < _minValue)
    {
      _value = _minValue;
    }
  else if (anInt > _maxValue)
    {
      _value = _maxValue;
    }
  else
    {
      _value = anInt;
    }
  if ((_control_view != nil) &&  
      ([_control_view isKindOfClass: [NSControl class]]))
    {
      [(NSControl*)_control_view updateCell: self];
    }
}

- (id) objectValue
{
  return [NSNumber numberWithDouble: _value];
}

- (void) setObjectValue: (id)anObject
{
  // If the provided object doesn't respond to doubeValue, or our minValue
  // is greater than our maxValue, we set our value to our minValue
  // (this arbitrary choice matches OS X)
  if ([anObject respondsToSelector: @selector(doubleValue)] == NO ||
      _minValue > _maxValue)
    {
      _value = _minValue;
    }
   else
    {
      double aDouble = [anObject doubleValue];
      if (aDouble < _minValue)
        {
          _value = _minValue;
        }
      else if (aDouble > _maxValue)
        {
          _value = _maxValue;
        }
      else
        {
          _value = aDouble;
        }
    }
  if ((_control_view != nil) &&  
      ([_control_view isKindOfClass: [NSControl class]]))
    {
      [(NSControl*)_control_view updateCell: self];
    }
}

/**<p>Returns the cell used to draw the title.</p>
   <p>See Also: -setTitleCell:</p> */
- (id) titleCell
{
  return _titleCell;
}

/**<p>Returns the colour used to draw the title.</p>
   <p>See Also: -setTitleColor:</p> 
*/
- (NSColor*) titleColor
{
  return [_titleCell textColor];
}

/**<p>Returns the font used to draw the title.</p>
   <p>See Also: -setTitleFont:</p>
*/
- (NSFont*) titleFont
{
  return [_titleCell font];
}

/**<p>Sets the title of the slider to <var>barTitle</var>.  This title is
  displayed on the slider's track, behind the knob.</p>
  <p>See Also: -title</p>*/
- (void) setTitle: (NSString*)title
{
  [_titleCell setStringValue: title];
}

/**<p>Returns the title of the slider. This title is
  displayed on the slider's track, behind the knob.</p>
  <p>See Also: -setTitle:</p>
*/
- (NSString*) title
{
  return [_titleCell stringValue];
}

/**<p>Sets the cell used to draw the title to <var>titleCell</var>.</p>
   <p>See Also: -titleCell</p>*/
- (void) setTitleCell: (NSCell*)aCell
{
  ASSIGN(_titleCell, aCell);
}

/**<p>Sets the colour with which the title will be drawn to <var>color</var>.
   </p><p>See Also: -titleColor</p>
*/
- (void) setTitleColor: (NSColor*)color
{
  [_titleCell setTextColor: color];
}

/**<p> Sets the font with which the title will be drawm to <var>font</var>.
 </p><p>See Also: -titleFont</p>
*/
- (void) setTitleFont: (NSFont*)font
{
  [_titleCell setFont: font];
}

/**<p>Returns the slider type: linear or circular.</p>
   <p>See Also: -setSliderType:</p>
*/
- (NSSliderType)sliderType
{
  return _type;
}

/**<p> Sets the type of the slider: linear or circular.
 </p><p>See Also: -sliderType</p>
*/
- (void) setSliderType: (NSSliderType)type
{
  _type = type;
  if (_type == NSLinearSlider)
    {
      [self setBordered: YES];
      [self setBezeled: NO];
    }
  else if (_type == NSCircularSlider)
    {
      [self setBordered: NO];
      [self setBezeled: NO];
    }
}

/**Returns whether or not the slider is vertical.  If, for some
  reason, this cannot be determined, for such reasons as the slider is
  not yet displayed, this method returns -1.  Generally, a slider is
  considered vertical if its height is greater than its width. 
*/
- (NSInteger) isVertical
{
  return _isVertical;
}

/**<p>Returns the value by which the slider is incremented when the user
    holds down the ALT key.</p><p>See Also: -setAltIncrementValue:</p>  
*/
- (double) altIncrementValue
{
  return _altIncrementValue;
}

/** 
  <p>The default implementation returns <code>YES</code>, so that the
  slider continues to track the user's movement even if the cursor
  leaves the slider's track.</p>

  <p>Do not call this method directly.  Override it in subclasses
  where the tracking behaviour needs to be different.</p>
 */
+ (BOOL) prefersTrackingUntilMouseUp
{
  return YES;
}

/** Returns the rect of the track, minus the bezel. */
- (NSRect) trackRect
{
  return _trackRect;
}

// ticks
- (BOOL) allowsTickMarkValuesOnly
{
  return _allowsTickMarkValuesOnly;
}

/* verified on Cocoa that a circular slider with one tick has two values: 0, 50 */
- (double) closestTickMarkValueToValue: (double)aValue
{
  double d, f;
  int effectiveTicks;

  if (_numberOfTickMarks == 0)
    return aValue;
  
  effectiveTicks = _numberOfTickMarks;
  if (_type == NSCircularSlider)
    effectiveTicks++;

  if (effectiveTicks == 1)
    return (_maxValue + _minValue) / 2;

  if (aValue < _minValue)
    {
      aValue = _minValue;
    }
  else if (aValue > _maxValue)
    {
      aValue = _maxValue; 
    }

  d = _maxValue - _minValue;
  f = ((aValue - _minValue)  * (effectiveTicks - 1)) / d;
  f = ((GSRoundTowardsInfinity(f) * d) / (effectiveTicks - 1)) + _minValue;

  /* never return the maximum value, tested on Apple */
  if (_type == NSCircularSlider && (f >= _maxValue))
    f = _minValue;

  return f;
}

- (NSInteger) indexOfTickMarkAtPoint: (NSPoint)point
{
  NSInteger i;

  for (i = 0; i < _numberOfTickMarks; i++)
    {
      if (NSPointInRect(point, [self rectOfTickMarkAtIndex: i])) 
        {
	  return i;
	}
    }

  return NSNotFound;
}

- (NSInteger) numberOfTickMarks
{
  return _numberOfTickMarks;
}

- (NSRect) rectOfTickMarkAtIndex: (NSInteger)index
{
  NSRect rect = _trackRect;
  CGFloat d;

  if ((index < 0) || (index >= _numberOfTickMarks))
    {
      [NSException raise: NSRangeException
		   format: @"Index of tick mark out of bounds."];
    }

  if (_numberOfTickMarks > 1)
    {
      if (_isVertical)
	{
	  d = NSHeight(rect) / (_numberOfTickMarks - 1);
	  rect.size.height = d;
	  rect.origin.y += d * index;
	}
      else
	{
	  d = NSWidth(rect) / (_numberOfTickMarks - 1);
	  rect.size.width = d;
	  rect.origin.x += d * index;
	}
    }

  return rect;
}

- (void) setAllowsTickMarkValuesOnly: (BOOL)flag
{
  _allowsTickMarkValuesOnly = flag;
}

- (void) setNumberOfTickMarks: (NSInteger)numberOfTickMarks
{
  _numberOfTickMarks = numberOfTickMarks;
  if ((_control_view != nil) &&  
      ([_control_view isKindOfClass: [NSControl class]]))
    {
      [(NSControl*)_control_view updateCell: self];
    }
}

- (void) setTickMarkPosition: (NSTickMarkPosition)position
{
  _tickMarkPosition = position;
  if ((_control_view != nil) &&  
      ([_control_view isKindOfClass: [NSControl class]]))
    {
      [(NSControl*)_control_view updateCell: self];
    }
}

- (NSTickMarkPosition) tickMarkPosition
{
  return _tickMarkPosition;
}

- (double) tickMarkValueAtIndex: (NSInteger)index
{
  if ((index < 0) || (index >= _numberOfTickMarks))
    {
      [NSException raise: NSRangeException
		   format: @"Index of tick mark out of bounds."];
    }

  if (_numberOfTickMarks == 1)
    return (_maxValue + _minValue) / 2;
  if (index >= _numberOfTickMarks)
    return _maxValue;
  if (index <= 0)
    return _minValue;

  if (_type == NSCircularSlider)
    return _minValue + index * (_maxValue - _minValue) / _numberOfTickMarks;
  if (_type == NSLinearSlider)
    return _minValue + index * (_maxValue - _minValue) / (_numberOfTickMarks - 1);

  return 0.0;
}

- (BOOL) startTrackingAt: (NSPoint)startPoint inView: (NSView*)controlView
{
  // If the point is in the view then yes start tracking
  if ([controlView mouse: startPoint inRect: [controlView bounds]])
    {
      BOOL isFlipped = [controlView isFlipped];
      NSRect knobRect = [self knobRectFlipped: isFlipped];

      if (![controlView mouse: startPoint inRect: knobRect])
        {
          // Mouse is not on the knob, move the knob to the mouse position
          double doubleValue;
          NSRect slotRect = [self trackRect];
          BOOL isVertical = [self isVertical];
          double minValue = [self minValue];
          double maxValue = [self maxValue];
          
          doubleValue = _doubleValueForMousePoint(startPoint, knobRect, 
                                                slotRect, isVertical, 
                                                minValue, maxValue,
                                                self, isFlipped,
                                                (_type == NSCircularSlider)); 
          if (_allowsTickMarkValuesOnly)
            {
              doubleValue = [self closestTickMarkValueToValue: doubleValue]; 
            }
          [self setDoubleValue: doubleValue];
          if ([self isContinuous])
            {
              [(NSControl*)controlView sendAction: [self action] to: [self target]];
            }
        }

      return YES;
    }
  else
    {
      return NO;
    }
}

- (BOOL) continueTracking: (NSPoint)lastPoint
                       at: (NSPoint)currentPoint
                   inView: (NSView*)controlView
{
  if (currentPoint.x != lastPoint.x || currentPoint.y != lastPoint.y)
    {
      double doubleValue;
      BOOL isFlipped = [controlView isFlipped];
      NSRect knobRect = [self knobRectFlipped: isFlipped];
      double oldDoubleValue = [self doubleValue];
      NSRect slotRect = [self trackRect];
      BOOL isVertical = [self isVertical];
      double minValue = [self minValue];
      double maxValue = [self maxValue];

      doubleValue = _doubleValueForMousePoint(currentPoint, knobRect,
                                              slotRect, isVertical, 
                                              minValue, maxValue, 
                                              self, isFlipped,
                                              (_type == NSCircularSlider)); 
      if (_allowsTickMarkValuesOnly)
        {
          doubleValue = [self closestTickMarkValueToValue: doubleValue]; 
        }
      if (doubleValue != oldDoubleValue)
        {
          [self setDoubleValue: doubleValue];
          // The action gets triggered in trackMouse:...untilMouseUp:
        }
    }
  return YES;
}

- (id) initWithCoder: (NSCoder*)decoder
{
  self = [super initWithCoder: decoder];
  if (self == nil)
    return nil;

  if ([decoder allowsKeyedCoding])
    {
      _allowsTickMarkValuesOnly = [decoder decodeBoolForKey: @"NSAllowsTickMarkValuesOnly"];
      _numberOfTickMarks = [decoder decodeIntForKey: @"NSNumberOfTickMarks"];
      _tickMarkPosition = [decoder decodeIntForKey: @"NSTickMarkPosition"];
      [self setMinValue: [decoder decodeDoubleForKey: @"NSMinValue"]];
      [self setMaxValue: [decoder decodeDoubleForKey: @"NSMaxValue"]];
      [self setDoubleValue: [decoder decodeDoubleForKey: @"NSValue"]];
      _altIncrementValue = [decoder decodeDoubleForKey: @"NSAltIncValue"];
      [self setSliderType: [decoder decodeIntForKey: @"NSSliderType"]];
	  
      // do these here, since the Cocoa version of the class does not save these values...
      _knobCell = [NSCell new];
      _titleCell = [NSTextFieldCell new];
      [_titleCell setTextColor: [NSColor controlTextColor]];
      [_titleCell setStringValue: @""];
      [_titleCell setAlignment: NSCenterTextAlignment];

      _isVertical = -1;
    }
  else
    {
      float tmp_float;
      NSInteger tmp_int;

      [self setDoubleValue: 0];
      [decoder decodeValueOfObjCType: @encode(float) at: &tmp_float];
      [self setMinValue: tmp_float];
      [decoder decodeValueOfObjCType: @encode(float) at: &tmp_float];
      [self setMaxValue: tmp_float];
      [decoder decodeValueOfObjCType: @encode(float) at: &tmp_float];
      [self setAltIncrementValue: tmp_float];
      decode_NSInteger(decoder, &tmp_int);
      _isVertical = tmp_int;
      [decoder decodeValueOfObjCType: @encode(id) at: &_titleCell];
      [decoder decodeValueOfObjCType: @encode(id) at: &_knobCell];
      if ([decoder versionForClassName: @"NSSliderCell"] >= 2)
	{
	  [decoder decodeValueOfObjCType: @encode(BOOL) at: &_allowsTickMarkValuesOnly];
          decode_NSInteger(decoder, &_numberOfTickMarks);
          decode_NSInteger(decoder, &tmp_int);
          _tickMarkPosition = tmp_int;
	}
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder*)coder
{
  [super encodeWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      [coder encodeBool: _allowsTickMarkValuesOnly forKey: @"NSAllowsTickMarkValuesOnly"];
      [coder encodeInt: _numberOfTickMarks forKey: @"NSNumberOfTickMarks"];
      [coder encodeInt: _tickMarkPosition forKey: @"NSTickMarkPosition"];
      [coder encodeDouble: _minValue forKey: @"NSMinValue"];
      [coder encodeDouble: _maxValue forKey: @"NSMaxValue"];
      [coder encodeDouble: _value forKey: @"NSValue"];
      [coder encodeDouble: _altIncrementValue forKey: @"NSAltIncValue"];
      [coder encodeInt: _type forKey: @"NSSliderType"];
    }
  else
    {
      float tmp_float;
      NSInteger tmp_int;

      tmp_float = _minValue;
      [coder encodeValueOfObjCType: @encode(float) at: &tmp_float];
      tmp_float = _maxValue;
      [coder encodeValueOfObjCType: @encode(float) at: &tmp_float];
      tmp_float = _altIncrementValue;
      [coder encodeValueOfObjCType: @encode(float) at: &tmp_float];
      tmp_int = _isVertical;
      encode_NSInteger(coder, &tmp_int);
      [coder encodeValueOfObjCType: @encode(id) at: &_titleCell];
      [coder encodeValueOfObjCType: @encode(id) at: &_knobCell];
      // New for version 2
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_allowsTickMarkValuesOnly];
      encode_NSInteger(coder, &_numberOfTickMarks);
      tmp_int = _tickMarkPosition;
      encode_NSInteger(coder, &tmp_int);
    }
}
  
@end
