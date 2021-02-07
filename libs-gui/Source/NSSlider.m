/** <title>NSSlider</title>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: September 1997
   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: August 1998

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

#include "math.h" // fabs
#import <Foundation/NSString.h>

#import "AppKit/NSEvent.h"
#import "AppKit/NSSlider.h"
#import "AppKit/NSSliderCell.h"

/**
  <unit>
  <heading>Class Description</heading>
  
  <p>An NSSlider displays, and allows control of, some value in the
  application.  It represents a continuous stream of values of type
  <code>float</code>, which can be retrieved by the method
  <code>floatValue</code> and set by the method
  <code>setFloatValue:</code>.</p>

  <p>This control is a continuous control.  It sends its action
  message as long as the user is manipulating it.  This can be changed
  by passing <code>NO</code> to the <code>setContinuous:</code>
  message of a given NSSlider.</p>

  <p>Although methods for adding and managing a title are provided,
  the slider's knob can cover this title, so it is recommended that a
  label be added near the slider, for identification.</p>

  <p>As with many controls, NSSlider relies on its cell counterpart,
  NSSliderCell.  For more information, please see the specification
  for NSSliderCell.</p>

  <p>Use of an NSSlider to do the role of an NSScroller is not
  recommended.  A scroller is intended to represent the visible
  portion of a view, whereas a slider is intended to represent some
  value.</p>

  </unit>
*/
@implementation NSSlider

static Class cellClass;

+ (void) initialize
{
  if (self == [NSSlider class])
    {
      // Initial version
      [self setVersion: 1];

      // Set our cell class to NSSliderCell
      [self setCellClass: [NSSliderCell class]];
    }
}

+ (void) setCellClass: (Class)class
{
  cellClass = class;
}

+ (Class) cellClass
{
  return cellClass;
}

- (id) initWithFrame: (NSRect)frameRect
{
  self = [super initWithFrame: frameRect];
  if (self == nil)
    return nil;

  [_cell setState: 1];
  [_cell setContinuous: YES];
  return self;
}

- (BOOL) isFlipped
{
  return YES;
}

/**<p>Returns the value by which the slider will be incremented if the
  user holds down the ALT key.</p><p>See Also: -setAltIncrementValue:</p> */
- (double) altIncrementValue
{
  return [_cell altIncrementValue];
}

/**<p>  Returns the image drawn in the slider's track.  Returns
  <code>nil</code> if this has not been set. </p><p>See Also: -setImage:</p> */
- (NSImage *) image
{
  return [_cell image];
}

/**
  Returns whether or not the slider is vertical.  If, for some reason,
  this cannot be determined, for such reasons as the slider is not yet
  displayed, this method returns -1.  Generally, a slider is
  considered vertical if its height is greater than its width.  */
- (NSInteger) isVertical
{
  return [_cell isVertical];
}

/**
  Returns the thickness of the slider's knob.  This value is in
  pixels, and is the size of the knob along the slider's track.  */
- (CGFloat) knobThickness
{
  return [_cell knobThickness];
}

/**<p> Sets the value by which the slider will be incremented, when the
  ALT key is held down, to <var>increment</var>.</p>
  <p>See Also: -altIncrementValue</p>
*/
- (void) setAltIncrementValue: (double)increment
{
  [_cell setAltIncrementValue: increment];
}

/** <p>Sets the image to be displayed in the slider's track
    to <var>barImage</var>.</p><p>See Also: -image</p>
 */
- (void) setImage: (NSImage *)backgroundImage
{
  [_cell setImage: backgroundImage];
}

/**<p>Sets the thickness of the knob to <var>aFloat</var>, in pixels.
  This value sets the amount of space which the knob takes up in the
  slider's track.</p><p>See Also: -knobThickness</p> 
 */
- (void) setKnobThickness: (CGFloat)aFloat
{
  [_cell setKnobThickness: aFloat];
}

/** <p>Sets the title of the slider to <var>aString</var>.  
    This title is displayed  on the slider's track, behind the knob.</p>
    <p>See Also: -title</p>
*/
- (void) setTitle: (NSString *)aString
{
  [_cell setTitle: aString];
}

/** <p>Sets the cell used to draw the title to <var>aCell</var>.</p>
 <p>See Also: -titleCell</p>*/
- (void) setTitleCell: (NSCell *)aCell
{
  [_cell setTitleCell: aCell];
}

/** <p>Sets the colour with which the title will be drawn to
    <var>aColor</var>.</p><p>See Also -titleColor</p>
*/
- (void) setTitleColor: (NSColor *)aColor
{
  [_cell setTitleColor: aColor];
}

/** <p>Sets the font with which the title will be drawm to 
    <var>fontObject</var>.</p>
 <p>See Also: -titleFont</p>*/
- (void) setTitleFont: (NSFont *)fontObject
{
  [_cell setTitleFont: fontObject];
}

/** <p>Returns the title of the slider as an <code>NSString</code>.</p>
 <p>See Also: -setTitle:</p>*/
- (NSString *) title
{
  return [_cell title];
}

/** <p>Returns the cell used to draw the title.</p>
    <p>See Also: -setTitleCell:</p> 
*/
- (id) titleCell
{
  return [_cell titleCell];
}

/** <p>Returns the colour used to draw the title.</p>
 <p>See Also: -setTitleColor:</p>*/
- (NSColor *) titleColor
{
  return [_cell titleColor];
}

/**<p> Returns the font used to draw the title.</p>
   <p>See Also: -setTitleFont:</p> */
- (NSFont *) titleFont
{
  return [_cell titleFont];
}

/** <p>Returns the maximum value that the slider represents.</p>
 <p>See Also: -setMaxValue:</p>*/
- (double) maxValue
{
  return [_cell maxValue];
}

/** <p>Returns the minimum value that the slider represents.</p>
 <p>See Also: -setMinValue:</p>
*/
- (double) minValue
{
  return [_cell minValue];
}

/**<p> Sets the maximum value that the sliders represents to
   <var>aDouble</var>.</p><p>See Also: -maxValue</p>
*/
- (void) setMaxValue: (double)aDouble
{
  [_cell setMaxValue: aDouble];
}

/**<p> Sets the minimum value that the slider represents to
   <var>aDouble</var>. </p> <p>See Also: -minValue</p>*/
- (void) setMinValue: (double)aDouble
{
  [_cell setMinValue: aDouble];
}

/** 
  Returns <code>YES</code> by default.  This will allow the first
  click sent to the slider, when in an inactive window, to both bring
  the window into focus and manipulate the slider. */
- (BOOL) acceptsFirstMouse: (NSEvent *)theEvent
{
  return YES;
}

- (void) keyDown: (NSEvent *)ev
{
  NSString *characters = [ev characters];
  int i, length = [characters length];
  double value = [self doubleValue];
  double min = [_cell minValue];
  double max = [_cell maxValue];
  double altValue = [_cell altIncrementValue];
  NSUInteger alt_down = ([ev modifierFlags] & NSAlternateKeyMask);
  BOOL only_ticks = [_cell allowsTickMarkValuesOnly];
  BOOL valueChanged = NO;
  double diff;
  
  
  if (alt_down && altValue != -1)
    {
      diff = altValue;
    }
  else if (only_ticks)
    {
      if ([_cell numberOfTickMarks])
        {
  	  double tick0 = [_cell tickMarkValueAtIndex: 0];
	  double tick1 = [_cell tickMarkValueAtIndex: 1];
          diff = tick1 - tick0;
	}
      else
        {
	  diff = 0.0;
        }
    }
  else
    {
      diff = fabs(min - max) / 20;
    }
  
  for (i = 0; i < length; i++)
    {
      switch ([characters characterAtIndex: i])
        {
	   case NSLeftArrowFunctionKey:
	   case NSDownArrowFunctionKey:
		value -= diff;
		valueChanged = YES;
	     break;
	   case NSUpArrowFunctionKey:
	   case NSRightArrowFunctionKey:
	        value += diff;
		valueChanged = YES;
	     break;
	   case NSPageDownFunctionKey:
		value -= diff * 2;
		valueChanged = YES;
	     break;
	   case NSPageUpFunctionKey:
		value += diff * 2;
		valueChanged = YES;
	     break;
	   case NSHomeFunctionKey:
		value = min;
		valueChanged = YES;
	     break;
	   case NSEndFunctionKey:
		value = max;
		valueChanged = YES;
	     break;
        }
    }
  
  if (valueChanged)
    {
      if (only_ticks)
        value = [_cell closestTickMarkValueToValue: value];
      
      if (value < min)
	{ 
	  value = min;
	}
      else if (value > max)
	{
	  value = max;
	}
      
      [self setDoubleValue: value];
      [self sendAction: [self action] to: [self target]];
      return;
    }

  [super keyDown: ev];
}

// ticks
- (BOOL) allowsTickMarkValuesOnly
{
  return [_cell allowsTickMarkValuesOnly];
}

- (double) closestTickMarkValueToValue: (double)aValue
{
  return [_cell closestTickMarkValueToValue: aValue];
}

- (NSInteger) indexOfTickMarkAtPoint: (NSPoint)point
{
  return [_cell indexOfTickMarkAtPoint: point];
}

- (NSInteger) numberOfTickMarks
{
  return [_cell numberOfTickMarks];
}

- (NSRect) rectOfTickMarkAtIndex: (NSInteger)index
{
  return [_cell rectOfTickMarkAtIndex: index];
}

- (void) setAllowsTickMarkValuesOnly: (BOOL)flag
{
  [_cell setAllowsTickMarkValuesOnly: flag];
}

- (void) setNumberOfTickMarks: (NSInteger)numberOfTickMarks
{
 [_cell setNumberOfTickMarks: numberOfTickMarks];
}

- (void) setTickMarkPosition: (NSTickMarkPosition)position
{
 [_cell setTickMarkPosition: position];
}

- (NSTickMarkPosition) tickMarkPosition
{
  return [_cell tickMarkPosition];
}

- (double) tickMarkValueAtIndex: (NSInteger)index
{
  return [_cell tickMarkValueAtIndex: index];
}

@end
