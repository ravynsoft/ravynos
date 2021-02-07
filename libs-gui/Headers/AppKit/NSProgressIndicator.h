/*
   NSProgressIndicator.h

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Gerrit van Dyk <gerritvd@decimax.com>
   Date: 1999

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

#ifndef _GNUstep_H_NSProgressIndicator
#define _GNUstep_H_NSProgressIndicator
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSView.h>

/* For NSControlTint */
#import <AppKit/NSColor.h>

/* For NSControlSize */
#import <AppKit/NSCell.h>

@class NSTimer;
@class NSThread;

typedef enum _NSProgressIndicatorThickness
{
  NSProgressIndicatorPreferredThickness = 14,
  NSProgressIndicatorPreferredSmallThickness = 10,
  NSProgressIndicatorPreferredLargeThickness = 18,
  NSProgressIndicatorPreferredAquaThickness = 12
} NSProgressIndicatorThickness;

typedef enum _NSProgressIndicatorStyle
{
  NSProgressIndicatorBarStyle = 0,
  NSProgressIndicatorSpinningStyle = 1
} NSProgressIndicatorStyle;

@interface NSProgressIndicator : NSView
{
  double _doubleValue;
  double _minValue;
  double _maxValue;
  NSTimeInterval _animationDelay;
  NSProgressIndicatorStyle _style;
  BOOL _isIndeterminate;
  BOOL _isBezeled;
  BOOL _usesThreadedAnimation;
  BOOL _isDisplayedWhenStopped;
  NSControlTint _controlTint;
  NSControlSize _controlSize;
@private
  BOOL _isVertical;
  BOOL _isRunning;
  int _count;  
  NSTimer *_timer;
  id _reserved;
}

//
// Animating the progress indicator
//

#if OS_API_VERSION(GS_API_LATEST, MAC_OS_X_VERSION_10_5)
- (void)animate:(id)sender;
- (NSTimeInterval)animationDelay;
- (void)setAnimationDelay:(NSTimeInterval)delay;
#endif

- (void)startAnimation:(id)sender;
- (void)stopAnimation:(id)sender;
- (BOOL)usesThreadedAnimation;
- (void)setUsesThreadedAnimation:(BOOL)flag;

// 
// Advancing the progress bar
//
- (void)incrementBy:(double)delta;
- (double)doubleValue;
- (void)setDoubleValue:(double)aValue;
- (double)minValue;
- (void)setMinValue:(double)newMinimum;
- (double)maxValue;
- (void)setMaxValue:(double)newMaximum;

//
// Setting the appearance
//
- (BOOL)isBezeled;
- (void)setBezeled:(BOOL)flag;
- (BOOL)isIndeterminate;
- (void)setIndeterminate:(BOOL)flag;

//
// Standard control layout
//
- (NSControlSize)controlSize;
- (void)setControlSize:(NSControlSize)size;
- (NSControlTint)controlTint;
- (void)setControlTint:(NSControlTint)tint;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (BOOL)isDisplayedWhenStopped;
- (void)setDisplayedWhenStopped:(BOOL)flag;
- (void)setStyle:(NSProgressIndicatorStyle)style;
- (NSProgressIndicatorStyle)style;
- (void)sizeToFit;
#endif

@end

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
@interface NSProgressIndicator (GNUstepExtensions)

/*
 * Enables Vertical ProgressBar
 *
 * If isVertical = YES, Progress is from the bottom to the top
 * If isVertical = NO, Progress is from the left to the right
 */
- (BOOL)isVertical;
- (void)setVertical:(BOOL)flag;

@end
#endif

#endif /* _GNUstep_H_NSProgressIndicator */
