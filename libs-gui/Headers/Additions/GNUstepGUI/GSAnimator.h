/*
   GSAnimator.h
 
   Author: Xavier Glattard (xgl) <xavier.glattard@online.fr>
 
   Copyright (c) 2007 Free Software Foundation, Inc.
 
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

#ifndef _GNUstep_H_GSAnimator_
#define _GNUstep_H_GSAnimator_

#import <Foundation/NSDate.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSZone.h>

@class NSArray;
@class NSEvent;
@class NSRunLoop;
@class NSString;
@class NSTimer;

/**
 * Protocol that needs to be adopted by classes that want to
 * be animated by a GSAnimator.
 */
@protocol GSAnimation
/** Call back method indicating that the GSAnimator did start the
 * animation loop. */
- (void) animatorDidStart;
/** Call back method indicating that the GSAnimator did stop the
 * animation loop. */
- (void) animatorDidStop;
/** Call back method called for each animation loop. */
- (void) animatorStep: (NSTimeInterval)elapsedTime;
@end

/**
 * GSAnimator is the front of a class cluster. Instances of a subclass of 
 * GSAnimator manage the timing of an animation.
 */
@interface GSAnimator : NSObject
{
  id<GSAnimation> _animation; // The Object to be animated
  BOOL _running;              // Indicates that the animator is looping

  NSTimeInterval _elapsed;    // Elapsed time since the animator started
  NSTimeInterval _lastFrame;  // The time of the last animation loop
  unsigned int _frameCount;   // The number of loops since the start

  NSArray *_runLoopModes;
  
  NSTimer *_timer;            // Timer used for looping
  NSTimeInterval _timerInterval;
}

/** Returns a GSAnimator object initialized with the specified object
 * to be animated as fast as possible. */
+ (GSAnimator*) animatorWithAnimation: (id<GSAnimation>)anAnimation;

/** Returns a GSAnimator object initialized with the specified object
 * to be animated. */
+ (GSAnimator*) animatorWithAnimation: (id<GSAnimation>)anAnimation
                            frameRate: (float)fps;

/** Returns a GSAnimator object allocated in the given NSZone and 
 * initialized with the specified object to be animated. */
+ (GSAnimator*) animatorWithAnimation: (id<GSAnimation>)anAnimation
                            frameRate: (float)fps
                                 zone: (NSZone*)aZone;

/** Returns a GSAnimator object initialized with the specified object
 * to be animated. The given NSRunLoop is used in NSDefaultRunLoopMode.*/
- (GSAnimator*) initWithAnimation: (id<GSAnimation>)anAnimation
                        frameRate: (float)aFrameRate;

- (GSAnimator*) initWithAnimation: (id<GSAnimation>)anAnimation;

- (unsigned int) frameCount;
- (void) resetCounters;
- (float) frameRate;
- (NSArray*) runLoopModesForAnimating;
- (void) setRunLoopModesForAnimating: (NSArray*)modes;

- (void) startAnimation;
- (void) stopAnimation;
- (BOOL) isAnimationRunning;
- (void) startStopAnimation;

- (void) stepAnimation;

@end

#endif /* _GNUstep_H_GSAnimator_ */
