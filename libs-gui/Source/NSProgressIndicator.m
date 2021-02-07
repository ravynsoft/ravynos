/** <title>NSProgressIndicator</title>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Gerrit van Dyk <gerritvd@decimax.com>
   Date: 1999
   Author:  Fred Kiefer <fredkiefer@gmx.de>
   Date: 2009

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSTimer.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSProgressIndicator.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSWindow.h"
#import "GNUstepGUI/GSTheme.h"
#import "GNUstepGUI/GSNibLoading.h"

@implementation NSProgressIndicator

+ (void) initialize
{
  if (self == [NSProgressIndicator class])
    {
      [self setVersion: 1];
    }
}
 
- (id) initWithFrame: (NSRect)frameRect
{
  self = [super initWithFrame: frameRect];
  if (!self)
    return nil;

  _isIndeterminate = YES;
  _isDisplayedWhenStopped = YES;
  _isBezeled = YES;
  _animationDelay = 5.0 / 60.0;  // 1 twelfth a a second
  _doubleValue = 0.0;
  _minValue = 0.0;
  _maxValue = 100.0;
  _controlTint = NSDefaultControlTint;
  _controlSize = NSRegularControlSize;
  [self setStyle: NSProgressIndicatorBarStyle];
  //_isVertical = NO;
  //_usesThreadedAnimation = NO;

  return self;
}

- (void) dealloc
{
  [self stopAnimation: self];
  [super dealloc];
}

- (BOOL) isFlipped
{
  return YES;
}

- (void) animate: (id)sender
{
  if (!_isIndeterminate && (_style == NSProgressIndicatorBarStyle))
    return;

  // Let this value overflow when it reachs the limit
  _count++;

  [self setNeedsDisplay: YES];
}

- (NSTimeInterval) animationDelay
{
  return _animationDelay;
}

- (void) setAnimationDelay: (NSTimeInterval)delay
{
  _animationDelay = delay;
  if (_isRunning && (_isIndeterminate 
                     || (_style == NSProgressIndicatorSpinningStyle)))
    {
      [self stopAnimation: self];
      [self startAnimation: self];
    }
}

- (void) _animationLoop
{
  while (_isRunning)
    {
      CREATE_AUTORELEASE_POOL(pool);

      [self animate: self];
      [NSThread sleepForTimeInterval: _animationDelay];
      [pool drain];
    }
}

- (void) startAnimation: (id)sender
{
  if (_isRunning || (!_isIndeterminate 
                     && (_style == NSProgressIndicatorBarStyle)))
    return;

  _isRunning = YES;
  if (!_usesThreadedAnimation)
    {
      ASSIGN(_timer, [NSTimer scheduledTimerWithTimeInterval: _animationDelay 
                              target: self 
                              selector: @selector(animate:)
                              userInfo: nil
                              repeats: YES]);
      [[NSRunLoop currentRunLoop] addTimer: _timer forMode: NSModalPanelRunLoopMode];
    }
  else
    {
      [NSThread detachNewThreadSelector: @selector(_animationLoop) 
                toTarget: self 
                withObject: nil];
    }
}

- (void) stopAnimation: (id)sender
{
  if (!_isRunning || (!_isIndeterminate 
                      && (_style == NSProgressIndicatorBarStyle)))
    return;

  if (!_usesThreadedAnimation)
    {
      [_timer invalidate];
      DESTROY(_timer);
    }
  else
    {
      // Done automatically
    }

  _isRunning = NO;
  _count = 0;
  [self setNeedsDisplay: YES];
}

- (BOOL) isHidden
{
  if (!_isRunning && !_isDisplayedWhenStopped)
    {
      return YES;
    }
  return [super isHidden];
}

- (BOOL) usesThreadedAnimation
{
  return _usesThreadedAnimation;
}

- (void) setUsesThreadedAnimation: (BOOL)flag
{
  if (_usesThreadedAnimation != flag)
    {
      BOOL wasRunning = _isRunning;

      if (wasRunning)
        [self stopAnimation: self];

      _usesThreadedAnimation = flag;

      if (wasRunning)
        [self startAnimation: self];
    }
}

- (void) incrementBy: (double)delta
{
  [self setDoubleValue: _doubleValue + delta];
}

- (double) doubleValue
{
  return _doubleValue;
}

- (void) setDoubleValue: (double)aValue
{
  if (aValue > _maxValue)
    aValue = _maxValue;
  else if (aValue < _minValue)
    aValue = _minValue;

  if (_doubleValue != aValue)
    {
      _doubleValue = aValue;
      [self setNeedsDisplay: YES];
    }
}

- (double) minValue
{
  return _minValue;
}

- (void) setMinValue: (double)newMinimum
{
  if (_minValue != newMinimum)
    {
      _minValue = newMinimum;
      if (_minValue > _doubleValue)
        _doubleValue = _minValue;
      [self setNeedsDisplay: YES];
    }
}

- (double) maxValue
{
  return _maxValue;
}

- (void) setMaxValue: (double)newMaximum
{
  if (_maxValue != newMaximum)
    {
      _maxValue = newMaximum;
      if (_maxValue < _doubleValue)
        _doubleValue = _maxValue;
      [self setNeedsDisplay: YES];
    }
}

- (BOOL)isBezeled
{
  return _isBezeled;
}

- (void) setBezeled: (BOOL)flag
{
  if (_isBezeled != flag)
    {
      _isBezeled = flag;
      [self setNeedsDisplay: YES];
    }
}

- (BOOL) isIndeterminate
{
  return _isIndeterminate;
}

- (void) setIndeterminate: (BOOL)flag
{
  /* Note: We must stop a running animation before setting _isIndeterminate
     because -stopAnimation: has no effect when _isIndeterminate is NO. */
  if (flag == NO && _isRunning)
    [self stopAnimation: self];

  _isIndeterminate = flag;
   // Maybe we need more functionality here when we implement indeterminate

  [self setNeedsDisplay: YES];
}

- (BOOL) isDisplayedWhenStopped
{
  return _isDisplayedWhenStopped;
}

- (void) setDisplayedWhenStopped: (BOOL)flag
{
  if (flag != _isDisplayedWhenStopped)
    {
      _isDisplayedWhenStopped = flag;
      [self setNeedsDisplay: YES];
    }
}

- (NSProgressIndicatorStyle) style
{
  return _style;
}

- (void) setStyle: (NSProgressIndicatorStyle)style
{
  _style = style;
  _count = 0;
  [self setDisplayedWhenStopped: (style == NSProgressIndicatorBarStyle)];
  [self setBezeled: (style == NSProgressIndicatorBarStyle)];
  [self sizeToFit];
  [self setNeedsDisplay: YES];
}

- (NSControlSize) controlSize
{
  return _controlSize;
}

- (void) setControlSize: (NSControlSize)size
{
  _controlSize = size;
  [self sizeToFit];
  [self setNeedsDisplay: YES];
}

- (NSControlTint) controlTint
{
  return _controlTint;
}

- (void) setControlTint: (NSControlTint)tint
{
  _controlTint = tint;
  [self setNeedsDisplay: YES];
}

- (void) sizeToFit  
{
  // FIXME
}

- (void) drawRect: (NSRect)rect
{
   double val;

   if (_doubleValue < _minValue)
     val = 0.0;
   else if (_doubleValue > _maxValue)
     val = 1.0;
   else 
     val = (_doubleValue - _minValue) / (_maxValue - _minValue);
   [[GSTheme theme] drawProgressIndicator: self
                    withBounds: _bounds
                    withClip: rect
                    atCount: _count
                    forValue: val];
}

// It does not seem that Gnustep has a copyWithZone: on NSView, it is private
// under openstep

// NSCopying
/* - (id)copyWithZone:(NSZone *)zone
{
   NSProgressIndicator  *newInd;

   newInd = [super copyWithZone:zone];
   [newInd setIndeterminate:_isIndeterminate];
   [newInd setBezeled:_isBezeled];
   [newInd setUsesThreadedAnimation:_usesThreadedAnimation];
   [newInd setAnimimationDelay:_animationDelay];
   [newInd setDoubleValue:_doubleValue];
   [newInd setMinValue:_minValue];
   [newInd setMaxValue:_maxValue];
   [newInd setVertical:_isVertical];
   return newInd;
}
*/

// NSCoding
- (void) encodeWithCoder: (NSCoder *)aCoder
{
   [super encodeWithCoder: aCoder];
   if ([aCoder allowsKeyedCoding])
     {
       unsigned long flags = 0;
       id matrix = AUTORELEASE([[NSPSMatrix alloc] init]);

       [aCoder encodeDouble: _minValue forKey: @"NSMinValue"];
       [aCoder encodeDouble: _maxValue forKey: @"NSMaxValue"];
       [aCoder encodeObject: matrix forKey: @"NSDrawMatrix"];

       // add flag values.
       flags |= (_isIndeterminate)? 2 : 0;
       // Hard coded... 
       flags |= 8;
       flags |= (_controlSize == NSSmallControlSize) ? 0x100 : 0;
       flags |= (_style == NSProgressIndicatorSpinningStyle) ? 0x1000 : 0;
       flags |= _isDisplayedWhenStopped ? 0 : 0x2000;
       [aCoder encodeInt: flags forKey: @"NSpiFlags"];

       // things which Gorm encodes, but IB doesn't care about.
       [aCoder encodeDouble: _doubleValue forKey: @"GSDoubleValue"];
       [aCoder encodeBool: _isBezeled forKey: @"GSIsBezeled"];
       [aCoder encodeBool: _isVertical forKey: @"GSIsVertical"];
       [aCoder encodeBool: _usesThreadedAnimation forKey: @"GSUsesThreadAnimation"];
       [aCoder encodeDouble: _animationDelay forKey: @"GSAnimationDelay"];
     }
   else
     {
       [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_isIndeterminate];
       [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_isBezeled];
       [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_usesThreadedAnimation];
       [aCoder encodeValueOfObjCType: @encode(NSTimeInterval) at: &_animationDelay];
       [aCoder encodeValueOfObjCType: @encode(double) at: &_doubleValue];
       [aCoder encodeValueOfObjCType: @encode(double) at: &_minValue];
       [aCoder encodeValueOfObjCType: @encode(double) at: &_maxValue];
       [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_isVertical];
     }
}

- (id) initWithCoder: (NSCoder *)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (!self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      // things which Gorm encodes, but IB doesn't care about.
      // process Gorm encodings that IB doesn't care about first
      // otherwise we overwrite settings read in from XIB...
      if ([aDecoder containsValueForKey: @"GSDoubleValue"])
        {
          _doubleValue = [aDecoder decodeDoubleForKey: @"GSDoubleValue"];
        }
      else
        {
          _doubleValue = _minValue;
        }

      if ([aDecoder containsValueForKey: @"GSIsBezeled"])
        {
          _isBezeled = [aDecoder decodeBoolForKey: @"GSIsBezeled"];
        }
      else
        {
          _isBezeled = YES;
        }

      if ([aDecoder containsValueForKey: @"GSIsVertical"])
        {
          _isVertical = [aDecoder decodeBoolForKey: @"GSIsVertical"];
        }
      else
        {
          _isVertical = NO;
        }

      if ([aDecoder containsValueForKey: @"GSUsesThreadAnimation"])
        {
          _usesThreadedAnimation = [aDecoder decodeBoolForKey: @"GSUsesThreadAnimation"];
        }
      else
        {
          _usesThreadedAnimation = NO;
        } 
     
      if ([aDecoder containsValueForKey: @"GSAnimationDelay"])
        {
          _animationDelay = [aDecoder decodeDoubleForKey: @"GSAnimationDelay"];
        }
      else
        {
          _animationDelay = 5.0 / 60.0;  // 1 twelfth a a second
        }

      // id matrix = [aDecoder decodeObjectForKey: @"NSDrawMatrix"];
      if ([aDecoder containsValueForKey: @"NSMaxValue"])
        {
          double max = [aDecoder decodeDoubleForKey: @"NSMaxValue"];
          
          [self setMaxValue: max];
        }
      else
        {
          _maxValue = 100.0;
        }
      if ([aDecoder containsValueForKey: @"NSMinValue"])
        {
          double min = [aDecoder decodeDoubleForKey: @"NSMinValue"];
          
          [self setMinValue: min];
        }
      else
        {
          _minValue = 0.0;
        }

      if ([aDecoder containsValueForKey: @"NSpiFlags"])
        {
          int flags = [aDecoder decodeIntForKey: @"NSpiFlags"];
          
          _isIndeterminate = ((flags & 2) == 2);
          _controlTint = NSDefaultControlTint;
          _controlSize = (flags & 0x100) ? NSSmallControlSize : NSRegularControlSize;
          [self setStyle: (flags & 0x1000) ? NSProgressIndicatorSpinningStyle 
                : NSProgressIndicatorBarStyle];
          _isDisplayedWhenStopped = ((flags & 0x2000) != 0x2000);
          // ignore the rest, since they are not pertinent to GNUstep.
        }
      else
        {
          _isIndeterminate = YES;
          _isDisplayedWhenStopped = YES;
          _controlTint = NSDefaultControlTint;
          _controlSize = NSRegularControlSize;
          [self setStyle: NSProgressIndicatorBarStyle];
        }
    }
  else
    {
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_isIndeterminate];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_isBezeled];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_usesThreadedAnimation];
      [aDecoder decodeValueOfObjCType: @encode(NSTimeInterval)
                at: &_animationDelay];
      [aDecoder decodeValueOfObjCType: @encode(double) at: &_doubleValue];
      [aDecoder decodeValueOfObjCType: @encode(double) at: &_minValue];
      [aDecoder decodeValueOfObjCType: @encode(double) at: &_maxValue];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_isVertical];

      _isDisplayedWhenStopped = YES;
      _controlTint = NSDefaultControlTint;
      _controlSize = NSRegularControlSize;
      [self setStyle: NSProgressIndicatorBarStyle];
    }
   return self;
}

@end

@implementation NSProgressIndicator (GNUstepExtensions)

- (BOOL) isVertical
{
  return _isVertical;
}

- (void) setVertical: (BOOL)flag
{
  if (_isVertical != flag)
    {
      _isVertical = flag;
      [self setNeedsDisplay:YES];
    }
}

@end

