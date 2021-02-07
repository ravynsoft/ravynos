/*
   GSAnimator.m
 
   Author: Xavier Glattard (xgl) <xavier.glattard@online.fr>
 
   Copyright (c) 2007 Free Software Foundation, Inc.
  
   This file used to be part of the mySTEP Library.
   This file now is part of the GNUstep GUI Library.
 
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

#import <Foundation/NSDebug.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSString.h>
#import <Foundation/NSTimer.h>

#import "AppKit/NSEvent.h"
#import "GNUstepGUI/GSAnimator.h"

@interface GSAnimator (private)
- (void) _animationBegin;
- (void) _animationLoop;
- (void) _animationEnd;
@end

@implementation GSAnimator

+ (GSAnimator*) animatorWithAnimation: (id<GSAnimation>)anAnimation
                            frameRate: (float)fps
                                 zone: (NSZone*)aZone
{
  return AUTORELEASE([[self allocWithZone: aZone]
                         initWithAnimation: anAnimation
                         frameRate: fps]);
}

+ (GSAnimator*) animatorWithAnimation: (id<GSAnimation>)anAnimation
                            frameRate: (float)fps
{
  return [self animatorWithAnimation: anAnimation
               frameRate: fps
               zone: NULL];
}

+ (GSAnimator*) animatorWithAnimation: (id<GSAnimation>)anAnimation
{
  return [self animatorWithAnimation: anAnimation
               frameRate: 0.0];
}

- (GSAnimator*) initWithAnimation: (id<GSAnimation>)anAnimation
                        frameRate: (float)fps
{
  if ((self = [super init]))
    {
      _running = NO;
      
      _animation = anAnimation;
      ASSIGN(_runLoopModes, [NSArray arrayWithObject: NSDefaultRunLoopMode]);
      _timerInterval = (fps == 0.0) ? 0.0 : (1.0 / fps);

      [self resetCounters];
    }
  return self;
}

- (GSAnimator*) initWithAnimation: (id<GSAnimation>)anAnimation
{
  return [self initWithAnimation: anAnimation
               frameRate: 0.0];
}

- (void) dealloc
{
  [self stopAnimation];
  TEST_RELEASE(_runLoopModes);
  [super dealloc];
}

- (unsigned int) frameCount
{
  return _frameCount;
}

- (void) resetCounters
{
  _elapsed = 0.0;
  _frameCount = 0;
  _lastFrame = [NSDate timeIntervalSinceReferenceDate];
}

- (float) frameRate 
{
  return ((float)[self frameCount]) / ((float)_elapsed);
}

- (NSArray*) runLoopModesForAnimating
{
  return _runLoopModes;
}

- (void) setRunLoopModesForAnimating: (NSArray*)modes
{
  ASSIGN(_runLoopModes, modes);
}

- (void) startAnimation
{
  if (!_running)
    {
      _running = YES;
      [self resetCounters];
      [_animation animatorDidStart];
      [self _animationBegin];
      [self _animationLoop];
      NSDebugMLLog(@"GSAnimator", @"Started !");
    }
}

- (void) stopAnimation
{
  if (_running)
    {
      _running = NO;
      [self _animationEnd];
      [_animation animatorDidStop];
    }
}

- (void) startStopAnimation
{
  if (_running)
    [self stopAnimation];
  else
    [self startAnimation];
}

- (BOOL) isAnimationRunning
{
  return _running;
}

- (void) stepAnimation
{
  NSTimeInterval thisFrame = [NSDate timeIntervalSinceReferenceDate];
  NSTimeInterval sinceLastFrame = (thisFrame - _lastFrame);

  _elapsed += sinceLastFrame;
  _lastFrame = thisFrame;

  [_animation animatorStep: _elapsed];
  _frameCount++;
}

@end

static NSTimer *_GSTimerBasedAnimator_timer = nil;
static NSMutableSet *_GSTimerBasedAnimator_animators = nil;
static GSAnimator *_GSTimerBasedAnimator_the_one_animator = nil;
static int _GSTimerBasedAnimator_animator_count = 0;

@implementation GSAnimator (private)

+ (void) loopsAnimators
{
  switch (_GSTimerBasedAnimator_animator_count)
    {
    case 0:
      break;
    case 1:
      [_GSTimerBasedAnimator_the_one_animator _animationLoop];
      break;
    default:
      [_GSTimerBasedAnimator_animators 
        makeObjectsPerform: @selector(_animationLoop)];
    }
}

- (void) _animationBegin
{
  NSTimer *newTimer = nil;

  if (_timerInterval == 0.0)
    {
      NSDebugMLLog(@"GSAnimator", @"AFAP start");
      if (!_GSTimerBasedAnimator_animator_count++)
        _GSTimerBasedAnimator_the_one_animator = self;

      if (nil == _GSTimerBasedAnimator_animators)
        _GSTimerBasedAnimator_animators = [[NSMutableSet alloc] initWithCapacity: 5];
      [_GSTimerBasedAnimator_animators addObject: self];
    
      if (nil == _GSTimerBasedAnimator_timer)
        {
          newTimer = [NSTimer timerWithTimeInterval: 0.0
                              target: [self class]
                              selector: @selector(loopsAnimators)
                              userInfo: nil
                              repeats: YES];
          ASSIGN(_GSTimerBasedAnimator_timer, newTimer);
        }
    }
  else
    {
      NSDebugMLLog(@"GSAnimator", @"Fixed frame rate start");
      newTimer = [NSTimer timerWithTimeInterval: _timerInterval
                          target: self
                          selector: @selector(_animationLoop)
                          userInfo: nil
                          repeats: YES];
      ASSIGN(_timer, newTimer);
    }

  if (newTimer != nil)
    {
      unsigned i,c;

      for (i = 0, c = [_runLoopModes count]; i < c; i++)
        {
          [[NSRunLoop currentRunLoop]
              addTimer: newTimer
              forMode: [_runLoopModes objectAtIndex:i]];
        }
      NSDebugMLLog(@"GSAnimator",@"addTimer in %d mode(s)", c);
    }
}

- (void) _animationLoop
{
  NSDebugMLLog(@"GSAnimator", @"Loop");
  [self stepAnimation];
}

- (void) _animationEnd
{
  if (_timerInterval == 0.0)
    {
      NSDebugMLLog(@"GSAnimator", @"AFAP end");
      [_GSTimerBasedAnimator_animators removeObject: self];
        
      if (!--_GSTimerBasedAnimator_animator_count)
        {
          [_GSTimerBasedAnimator_timer invalidate];
          DESTROY(_GSTimerBasedAnimator_timer);
          _GSTimerBasedAnimator_the_one_animator = nil;
        }
      else
        if (_GSTimerBasedAnimator_the_one_animator == self)
          _GSTimerBasedAnimator_the_one_animator
            = [_GSTimerBasedAnimator_animators anyObject];
    }
  else
    {
      NSDebugMLLog(@"GSAnimator", @"Fixed frame rate end");
      if (_timer != nil)
        {
          [_timer invalidate];
          DESTROY(_timer);
        }
    }
}

@end // implementation GSAnimator (private)
