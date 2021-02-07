/*
   NSAnimation.m
 
   Created by Dr. H. Nikolaus Schaller on Sat Mar 06 2006.
   Copyright (c) 2007 Free Software Foundation, Inc.
 
   Author: Xavier Glattard (xgl) <xavier.glattard@online.fr>
 
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

#import <Foundation/NSDate.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSValue.h>
#import <GNUstepBase/GSLock.h>

#import "AppKit/NSAnimation.h"
#import "AppKit/NSApplication.h"
// needed by NSViewAnimation
#import "AppKit/NSView.h"
#import "AppKit/NSWindow.h"

#include <math.h>

NSString* NSAnimationBlockingRunLoopMode = @"NSAnimationBlockingRunLoopMode";

/*===================*
 * NSAnimation class *
 *===================*/
#define	GSI_ARRAY_NO_RETAIN
#define	GSI_ARRAY_NO_RELEASE
#define	GSIArrayItem NSAnimationProgress
#include <GNUstepBase/GSIArray.h>

// 'reasonable value' ?
#define GS_ANIMATION_DEFAULT_FRAME_RATE 25.0

static NSArray* _NSAnimationDefaultRunLoopModes;

static inline void 
_GSBezierComputeCoefficients(_GSBezierDesc *b)
{
  b->a[0] =     b->p[0];
  b->a[1] =-3.0*b->p[0] + 3.0*b->p[1];
  b->a[2] = 3.0*b->p[0] - 6.0*b->p[1] + 3.0*b->p[2];
  b->a[3] =-    b->p[0] + 3.0*b->p[1] - 3.0*b->p[2] + b->p[3];
  b->areCoefficientsComputed = YES;
}

static inline float 
_GSBezierEval(_GSBezierDesc *b, float t)
{
  if (!b->areCoefficientsComputed)
    _GSBezierComputeCoefficients(b);
  return b->a[0] + t * (b->a[1] + t * (b->a[2] + t * b->a[3]));
}

static inline float 
_GSBezierDerivEval(_GSBezierDesc *b, float t)
{
  if (!b->areCoefficientsComputed)
    _GSBezierComputeCoefficients(b);
  return b->a[1] + t * (2.0 * b->a[2] + t * 3.0 * b->a[3]);
}

static inline void 
_GSRationalBezierComputeBezierDesc(_GSRationalBezierDesc *rb)
{
  unsigned i;

  for (i = 0; i < 4; i++)
    rb->n.p[i] = (rb->d.p[i] = rb->w[i]) * rb->p[i];
  _GSBezierComputeCoefficients(&rb->n);
  _GSBezierComputeCoefficients(&rb->d);
  rb->areBezierDescComputed = YES;
}
 
static inline float
_GSRationalBezierEval(_GSRationalBezierDesc *rb, float t)
{
  if (!rb->areBezierDescComputed)
    _GSRationalBezierComputeBezierDesc(rb);
  return _GSBezierEval(&(rb->n), t) / _GSBezierEval(&(rb->d), t);
}

static inline float
_GSRationalBezierDerivEval(_GSRationalBezierDesc *rb, float t)
{
  float h;
  if (!rb->areBezierDescComputed)
    _GSRationalBezierComputeBezierDesc(rb);
  h = _GSBezierEval(&(rb->d), t);
  return (_GSBezierDerivEval(&(rb->n), t) * h 
          - _GSBezierEval   (&(rb->n), t) * _GSBezierDerivEval(&(rb->d), t))
    / (h*h);
}

static
_NSAnimationCurveDesc _gs_animationCurveDesc[] =
{
  // easeInOut : endGrad = startGrad & startGrad <= 1/3
  { 0.0,1.0,  1.0/3,1.0/3 ,  {{2.0,2.0/3,2.0/3,2.0}} },
  // easeIn    : endGrad = 1/startGrad & startGrad >= 1/6
  { 0.0,1.0,  0.25,4.0 ,  {{4.0,3.0,2.0,1.0}} },
  // easeOut   : endGrad = 1/startGrad & startGrad <= 6
  { 0.0,1.0,  4.0 ,0.25,  {{1.0,2.0,3.0,4.0}} },
  // linear (not used)
  { 0.0,1.0,  1.0 ,1.0 ,  {{1.0,1.0,1.0,1.0}} },
  // speedInOut: endGrad = startGrad & startGrad >=3
  { 0.0,1.0,  3.0 ,3.0 ,  {{2.0/3,2.0,2.0,2.0/3}} }
};

/* Translate the NSAnimationCurveDesc data (start/end points and start/end
 * gradients) to GSRBezier data (4 control points), then evaluate it.
 */
static inline float
_gs_animationValueForCurve(_NSAnimationCurveDesc *c, float t, float t0)
{
  if (!c->isRBezierComputed)
    {
      c->rb.p[0] = c->s;
      c->rb.p[1] = c->s + (c->sg*c->rb.w[0]) / (3*c->rb.w[1]);
      c->rb.p[2] = c->e - (c->eg*c->rb.w[3]) / (3*c->rb.w[2]);
      c->rb.p[3] = c->e;
      _GSRationalBezierComputeBezierDesc (&c->rb);
      c->isRBezierComputed = YES;
    }
  return _GSRationalBezierEval ( &(c->rb), (t-t0) / (1.0-t0) );
}

@interface NSAnimation (PrivateNotificationCallbacks)
- (void) _gs_startAnimationReachesProgressMark: (NSNotification*)notification;
- (void) _gs_stopAnimationReachesProgressMark: (NSNotification*)notification;
@end

@interface NSAnimation (Private)
- (void) _gs_didReachProgressMark: (NSAnimationProgress)progress;
- (void) _gs_startAnimationInOwnLoop;
- (void) _gs_startThreadedAnimation;
- (_NSAnimationCurveDesc*) _gs_curveDesc;
- (NSAnimationProgress) _gs_curveShift;
@end

NSComparisonResult
nsanimation_progressMarkSorter(NSAnimationProgress first, NSAnimationProgress second)
{
  float diff = first - second;
  return (NSComparisonResult)(diff / fabs(diff));
}

/* Thread locking/unlocking support macros.
 * _isThreaded flag is an ivar that records whether the
 * NSAnimation is running in thread mode.
 * __gs_isLocked flag is local to each method and records
 * whether the thread is locked and must be locked before
 * the method exits.
 * Both are needed because _isThreaded is reset when the
 * NSAnimation stops : that may happen at any time between
 * a lock/unlock pair.
 */
#define _NSANIMATION_LOCKING_SETUP  \
  BOOL __gs_isLocked = NO;

#define _NSANIMATION_LOCK           \
  if (_isThreaded)                  \
  {                                 \
    NSAssert(__gs_isLocked == NO, NSInternalInconsistencyException); \
    NSDebugMLLog(@"NSAnimationLock",\
                 @"LOCK %@", [NSThread currentThread]);\
    [_isAnimatingLock lock];        \
    __gs_isLocked = YES;            \
   }

#define _NSANIMATION_UNLOCK         \
  if (__gs_isLocked)                \
  {                                 \
    /* NSAssert(__gs_isLocked == YES, NSInternalInconsistencyException); */ \
    NSDebugMLLog(@"NSAnimationLock",\
                 @"UNLOCK %@", [NSThread currentThread]);\
    __gs_isLocked = NO;             \
    [_isAnimatingLock unlock];      \
  }

@implementation NSAnimation

+ (void) initialize
{
  unsigned i;

  for (i = 0; i < 5; i++) // compute Bezier curve parameters...
    _gs_animationValueForCurve(&_gs_animationCurveDesc[i], 0.0, 0.0);
  _NSAnimationDefaultRunLoopModes
    = [[NSArray alloc] initWithObjects:
        NSDefaultRunLoopMode,
        NSModalPanelRunLoopMode,
        NSEventTrackingRunLoopMode,
        nil];
}

- (void) addProgressMark: (NSAnimationProgress)progress
{
  _NSANIMATION_LOCKING_SETUP;
  
  if (progress < 0.0) progress = 0.0;
  if (progress > 1.0) progress = 1.0;

  _NSANIMATION_LOCK;
  if (GSIArrayCount(_progressMarks) == 0)
    { // First mark
      GSIArrayAddItem (_progressMarks,progress);
      NSDebugMLLog (@"NSAnimationMark",
                    @"Insert 1st mark for %f (next:#%d)",
                    progress, _nextMark);
      _nextMark = (progress >= [self currentProgress])? 0 : 1;
    }
  else 
    {
      unsigned index;
      index = GSIArrayInsertionPosition (_progressMarks,
                                         progress,
                                         &nsanimation_progressMarkSorter);
      if (_nextMark < GSIArrayCount(_progressMarks)) 
        if (index <= _nextMark 
            && progress < GSIArrayItemAtIndex(_progressMarks,_nextMark))
          _nextMark++;
      GSIArrayInsertItem (_progressMarks,progress,index);
      NSDebugMLLog (@"NSAnimationMark",
                    @"Insert mark #%d/%d for %f (next:#%d)",
                    index,GSIArrayCount(_progressMarks),progress,_nextMark);
    }
  _isCachedProgressMarkNumbersValid = NO;

  _NSANIMATION_UNLOCK;
}

- (NSAnimationBlockingMode) animationBlockingMode
{
  NSAnimationBlockingMode m;
  _NSANIMATION_LOCKING_SETUP;
  
  _NSANIMATION_LOCK;
  m = _blockingMode;
  _NSANIMATION_UNLOCK;
  return m;
}

- (NSAnimationCurve) animationCurve
{
  NSAnimationCurve c;
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  c = _curve;
  _NSANIMATION_UNLOCK;
  return c;
}

- (void) clearStartAnimation
{
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  [[NSNotificationCenter defaultCenter]
    removeObserver: self
	      name: NSAnimationProgressMarkNotification
	    object: _startAnimation];
  [_startAnimation removeProgressMark: _startMark];
  _startAnimation = nil;
  _NSANIMATION_UNLOCK;
}

- (void) clearStopAnimation
{
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  [[NSNotificationCenter defaultCenter]
    removeObserver: self
	      name: NSAnimationProgressMarkNotification
	    object: _stopAnimation];
  [_stopAnimation removeProgressMark: _stopMark];
  _stopAnimation = nil;
  _NSANIMATION_UNLOCK;
}

- (NSAnimationProgress) currentProgress
{
  NSAnimationProgress p;
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  p = _currentProgress;
  _NSANIMATION_UNLOCK;
  return p;
}

- (float) currentValue
{
  float value;
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;

  if (_delegate_animationValueForProgress)
    { // method is cached (the animation is running)
      NSDebugMLLog (@"NSAnimationDelegate",
                    @"[delegate animationValueForProgress] (cached)");
      value = (*_delegate_animationValueForProgress)
                (_currentDelegate,
                 @selector (animation:valueForProgress:),
                 self, _currentProgress);
    }
  else // method is not cached (the animation did not start yet)
    if ( _delegate != nil
         && [_delegate respondsToSelector:
               @selector (animation:valueForProgress:)] )
      {
        NSDebugMLLog (@"NSAnimationDelegate",
                      @"[delegate animationValueForProgress]");
        value = [_delegate animation: self
                                   valueForProgress: _currentProgress];
      }
    else // default -- FIXME ??
      /*    switch (_curve)
            {
            case NSAnimationEaseInOut:
               case NSAnimationEaseIn:
              case NSAnimationEaseOut:
           case NSAnimationSpeedInOut:*/
      value = _gs_animationValueForCurve ( 
                &_curveDesc, _currentProgress, _curveProgressShift
                );
  /*	break;
        case NSAnimationLinear:
        value = _currentProgress; break;
        }*/

  _NSANIMATION_UNLOCK;

  return value;
}

- (id) delegate
{
  id d;
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  d = (_delegate == nil)? nil : _delegate;
  _NSANIMATION_UNLOCK;
  return d;
}

- (NSTimeInterval) duration
{
  NSTimeInterval d;
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  d = _duration;
  _NSANIMATION_UNLOCK;
  return d;
}

- (float) frameRate
{
  float f;
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  f = _frameRate;
  _NSANIMATION_UNLOCK;
  return f;
}

- (id) initWithDuration: (NSTimeInterval)duration
	 animationCurve: (NSAnimationCurve)curve
{
  if ((self = [super init]))
    {
      if (duration<=0.0)
        [NSException raise: NSInvalidArgumentException                
                    format: @"%@ Duration must be > 0.0 (passed: %f)",self,duration];
      _duration = duration;
      _frameRate = GS_ANIMATION_DEFAULT_FRAME_RATE;
      _curve = curve;
      _curveDesc = _gs_animationCurveDesc[_curve];
      _curveProgressShift = 0.0;

      _currentProgress = 0.0;
      _progressMarks = NSZoneMalloc ([self zone], sizeof(GSIArray_t));
      GSIArrayInitWithZoneAndCapacity (_progressMarks, [self zone], 16);
      _cachedProgressMarkNumbers = NULL;
      _cachedProgressMarkNumberCount = 0;
      _isCachedProgressMarkNumbersValid = NO;
      _nextMark = 0;

      _startAnimation = _stopAnimation = nil;
      _startMark = _stopMark = 0.0;
      
      _blockingMode = NSAnimationBlocking;
      _animator = nil;
      _isANewAnimatorNeeded = YES;

      _delegate = nil;
      _delegate_animationDidReachProgressMark =
        (void (*)(id,SEL,NSAnimation*,NSAnimationProgress)) NULL;
      _delegate_animationValueForProgress =
        (float (*)(id,SEL,NSAnimation*,NSAnimationProgress)) NULL;
      _delegate_animationDidEnd =
        (void (*)(id,SEL,NSAnimation*)) NULL;
      _delegate_animationDidStop =
        (void (*)(id,SEL,NSAnimation*)) NULL;
      _delegate_animationShouldStart =
        (BOOL (*)(id,SEL,NSAnimation*)) NULL;
      
      _isThreaded = NO;
      _isAnimatingLock = [GSLazyRecursiveLock new];
    }
  return self;
}

- (id) copyWithZone: (NSZone*)zone
{
  NSAnimation *c = (NSAnimation*)NSCopyObject (self, 0, zone);

  c->_progressMarks = GSIArrayCopyWithZone(_progressMarks, zone);
  c->_animator = nil;
  c->_isANewAnimatorNeeded = YES;
  c->_isAnimatingLock = [GSLazyRecursiveLock new];
  return c;
}

- (void) dealloc
{
  [self stopAnimation];

  GSIArrayEmpty(_progressMarks);
  NSZoneFree([self zone], _progressMarks);
  if (_cachedProgressMarkNumbers != NULL)
    {
      unsigned i;

      for (i = 0; i < _cachedProgressMarkNumberCount; i++)
        RELEASE(_cachedProgressMarkNumbers[i]);
      NSZoneFree([self zone], _cachedProgressMarkNumbers);
    }

  [self clearStartAnimation];
  [self clearStopAnimation];

  TEST_RELEASE(_animator);
  RELEASE(_isAnimatingLock);

  [super dealloc];
}

- (BOOL) isAnimating
{
  BOOL f;
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  f = (_animator != nil) ? [_animator isAnimationRunning] : NO;
  _NSANIMATION_UNLOCK;
  return f;
}

- (NSArray*) progressMarks
{
  NSNumber **cpmn;
  unsigned count;
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;

  count = GSIArrayCount(_progressMarks);

  if (!_isCachedProgressMarkNumbersValid)
    {
      unsigned i;

      if (_cachedProgressMarkNumbers != NULL)
        {   
          for (i = 0; i < _cachedProgressMarkNumberCount; i++)
            RELEASE(_cachedProgressMarkNumbers[i]);
          _cachedProgressMarkNumbers =
           (NSNumber**)NSZoneRealloc([self zone], _cachedProgressMarkNumbers,
                                     count * sizeof(NSNumber*));
        }
      else
        {
          _cachedProgressMarkNumbers =
           (NSNumber**)NSZoneMalloc([self zone], count * sizeof(NSNumber*));
        }

      for (i = 0; i < count; i++)
        {
          _cachedProgressMarkNumbers[i] =
           [NSNumber numberWithFloat: GSIArrayItemAtIndex (_progressMarks,i)];
        }
      _cachedProgressMarkNumberCount = count;
      _isCachedProgressMarkNumbersValid = YES;
    }

  cpmn = _cachedProgressMarkNumbers;
  _NSANIMATION_UNLOCK;

  return [NSArray arrayWithObjects: cpmn count: count];
}

- (void) removeProgressMark: (NSAnimationProgress)progress
{
  NSUInteger index;
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;

  index = GSIArraySearch(_progressMarks, progress, 
                         nsanimation_progressMarkSorter);
  if (index < GSIArrayCount(_progressMarks)
      && progress == GSIArrayItemAtIndex (_progressMarks,index))
    {
      GSIArrayRemoveItemAtIndex(_progressMarks,index);
      _isCachedProgressMarkNumbersValid = NO;
      if (_nextMark > index) _nextMark--;
      NSDebugMLLog(@"NSAnimationMark",@"Remove mark #%lu (%f) for (next:#%d)",
                   (unsigned long)index, progress, _nextMark);
    }
  else
    NSWarnMLog(@"Unexistent progress mark");

  _NSANIMATION_UNLOCK;
}

- (NSArray*) runLoopModesForAnimating
{
  return nil;
}

- (void) setAnimationBlockingMode: (NSAnimationBlockingMode)mode
{
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  _isANewAnimatorNeeded |= (_blockingMode != mode);
  _blockingMode = mode;
  _NSANIMATION_UNLOCK;
}

- (void) setAnimationCurve: (NSAnimationCurve)curve
{
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;

  if (_currentProgress <= 0.0f || _currentProgress >= 1.0f)
    {
      _curveDesc = _gs_animationCurveDesc[curve];
    }
  else
    { // FIXME ??
      _GSRationalBezierDesc newrb;

      _GSRationalBezierDesc *rb1 = &(_curveDesc.rb);
      float t1 = (_currentProgress - _curveProgressShift) / (1.0 - _curveProgressShift);
      _GSRationalBezierDesc *rb2 = &(_gs_animationCurveDesc[curve].rb);
      float t2 = _currentProgress;
      float K;
      newrb.p[0] = _GSRationalBezierEval ( rb1,   t1        );
      newrb.w[0] = _GSBezierEval        (&rb1->d,t1        );
      newrb.w[1] = 
        rb1->w[1]
        + t1*(   2*( rb1->w[2]           - rb1->w[1] )
              + t1*( rb1->w[1]           - 2*rb1->w[2]           + rb1->w[3]           ));
      newrb.p[1] = (
        rb1->w[1]*rb1->p[1]
        + t1*(   2*( rb1->w[2]*rb1->p[2] - rb1->w[1]*rb1->p[1] )
              + t1*( rb1->w[1]*rb1->p[1] - 2*rb1->w[2]*rb1->p[2] + rb1->w[3]*rb1->p[3] ))
        ) / newrb.w[1];
      newrb.w[2] = rb2->w[2]           + t2*(rb2->w[3]           - rb2->w[2]          );
      newrb.p[2] = (
                    rb2->w[2]*rb2->p[2] + t2*(rb2->w[3]*rb2->p[3] - rb2->w[2]*rb2->p[2])
                   ) / newrb.w[2];

      // 3rd point is moved to the right by scaling : w3*p3 = w1*p1 + (w1*p1 - w0*p0) 
      K = ( 2*newrb.w[1]*newrb.p[1]-newrb.w[0]*newrb.p[0] ) / (newrb.w[2]*newrb.p[2]);
      newrb.p[3] = rb2->p[3];
      newrb.w[3] = rb2->w[3] * K;
      newrb.w[2] = newrb.w[2]* K;

      _GSRationalBezierComputeBezierDesc (&newrb);
#if 0
      NSLog (@"prgrss = %f shift = %f",_currentProgress,_curveProgressShift);
      switch (curve)
      { case 0:NSLog (@"EaseInOut t=%f - %f",t1,t2);break;
        case 1:NSLog (@"EaseIn    t=%f - %f",t1,t2);break;
        case 2:NSLog (@"EaseOut   t=%f - %f",t1,t2);break;
        case 3:NSLog (@"Linear    t=%f - %f",t1,t2);break;
        default:NSLog (@"???");
      }
      NSLog (@"a=%f b=%f c=%f d=%f",newrb.p[0],newrb.p[1],newrb.p[2],newrb.p[3]);
      NSLog (@"  %f   %f   %f   %f",newrb.w[0],newrb.w[1],newrb.w[2],newrb.w[3]);
#endif
      _curveProgressShift = _currentProgress;
      _curveDesc.rb = newrb;
      _curveDesc.isRBezierComputed = YES;
    }
  _curve = curve;

  _NSANIMATION_UNLOCK;
}

- (void) setCurrentProgress: (NSAnimationProgress)progress
{
  BOOL needSearchNextMark = NO;
  NSAnimationProgress markedProgress;
  _NSANIMATION_LOCKING_SETUP;

  if (progress < 0.0) progress = 0.0;
  if (progress > 1.0) progress = 1.0;

  _NSANIMATION_LOCK;

  // NOTE: In the case of a forward jump the marks between the
  //       previous progress value and the new (excluded) progress 
  //       value are never reached.
  //       In the case of a backward jump (rewind) the marks will 
  //       be reached again !
  if (_nextMark < GSIArrayCount(_progressMarks))
    {
      markedProgress = GSIArrayItemAtIndex (_progressMarks,_nextMark);
      if (markedProgress == progress)
        [self _gs_didReachProgressMark: markedProgress];
      else
        {
          // the following should never happens if the progress
          // is reached during the normal run of the animation
          // (method called from animatorStep)
          if (markedProgress < progress) // forward jump ?
            needSearchNextMark = YES;
        }
    }
  needSearchNextMark |= progress < _currentProgress; // rewind ?

  if (needSearchNextMark)
    {
      _nextMark = GSIArrayInsertionPosition (_progressMarks,progress,&nsanimation_progressMarkSorter);

      if (_nextMark < GSIArrayCount(_progressMarks))
        NSDebugMLLog(@"NSAnimationMark",@"Next mark #%d for %f",
                     _nextMark, GSIArrayItemAtIndex(_progressMarks,_nextMark));
    }

  NSDebugMLLog(@"NSAnimation",@"Progress = %f", progress);
  _currentProgress = progress;

  if (progress >= 1.0 && _animator != nil)
    [_animator stopAnimation];

  _NSANIMATION_UNLOCK;
}

- (void) setDelegate: (id)delegate
{
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  _delegate = (delegate == nil)? nil : delegate;
  _NSANIMATION_UNLOCK;
}

- (void) setDuration: (NSTimeInterval)duration
{
  _NSANIMATION_LOCKING_SETUP;

  if (duration<=0.0)
    [NSException raise: NSInvalidArgumentException                
		format: @"%@ Duration must be > 0.0 (passed: %f)",self,duration];
  _NSANIMATION_LOCK;
  _duration = duration;
  _NSANIMATION_UNLOCK;
}

- (void) setFrameRate: (float)fps
{
  _NSANIMATION_LOCKING_SETUP;

  if (fps<0.0)
    [NSException raise: NSInvalidArgumentException
		format: @"%@ Framerate must be >= 0.0 (passed: %f)",self,fps];
  _NSANIMATION_LOCK;
  _isANewAnimatorNeeded |= (_frameRate != fps);
  if ( _frameRate != fps && [self isAnimating] )
    { // a new animator is needed *now*
      // FIXME : should I have been smarter ?
      [self stopAnimation];
      [self startAnimation];
    }
  _frameRate = fps;
  _NSANIMATION_UNLOCK;
}

- (void) setProgressMarks: (NSArray*)marks
{
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  GSIArrayEmpty(_progressMarks);
  _nextMark = 0;
  if (marks != nil)
    {
      unsigned i, count = [marks count];

      for (i = 0; i < count; i++)
        [self addProgressMark: [(NSNumber*)[marks objectAtIndex:i] floatValue]];
    }
  _isCachedProgressMarkNumbersValid = NO;
  _NSANIMATION_UNLOCK;
}

- (void) startAnimation
{
  unsigned i;

  if ([self isAnimating])
    return;

  NSDebugMLLog(@"NSAnimationStart",@"");

  for (i = 0; i < GSIArrayCount(_progressMarks); i++)
    NSDebugMLLog(@"NSAnimationMark", @"Mark #%d : %f",
                 i, GSIArrayItemAtIndex(_progressMarks,i));

  if ([self currentProgress] >= 1.0) 
    {
      [self setCurrentProgress: 0.0];
      _nextMark = 0;
    }

  _curveDesc = _gs_animationCurveDesc[_curve];
  _curveProgressShift = 0.0;

  if (_delegate != nil)
    {
      id delegate;

      NSDebugMLLog(@"NSAnimationDelegate", @"Cache delegation methods");
      // delegation methods are cached while the animation is running
      delegate = _delegate;
      _delegate_animationDidReachProgressMark =
        ([delegate respondsToSelector: @selector (animation:didReachProgressMark:)]) ?
        (void (*)(id,SEL,NSAnimation*,NSAnimationProgress))
        [delegate methodForSelector: @selector (animation:didReachProgressMark:)]
        : NULL;
      _delegate_animationValueForProgress =
        ([delegate respondsToSelector: @selector (animation:valueForProgress:)]) ?
        (float (*)(id,SEL,NSAnimation*,NSAnimationProgress))
        [delegate methodForSelector: @selector (animation:valueForProgress:)]
        : NULL;
      _delegate_animationDidEnd =
        ([delegate respondsToSelector: @selector (animationDidEnd:)]) ?
        (void (*)(id,SEL,NSAnimation*))
        [delegate methodForSelector: @selector (animationDidEnd:)]
        : NULL;
      _delegate_animationDidStop =
        ([delegate respondsToSelector: @selector (animationDidStop:)]) ?
        (void (*)(id,SEL,NSAnimation*))
        [delegate methodForSelector: @selector (animationDidStop:)]
        : NULL;
      _delegate_animationShouldStart =
        ([delegate respondsToSelector: @selector (animationShouldStart:)]) ?
        (BOOL (*)(id,SEL,NSAnimation*))
        [delegate methodForSelector: @selector (animationShouldStart:)]
        : NULL;
      NSDebugMLLog(@"NSAnimationDelegate",
                   @"Delegation methods : %p %p %p %p %p",
                   _delegate_animationDidReachProgressMark,
                   _delegate_animationValueForProgress,
                   _delegate_animationDidEnd,
                   _delegate_animationDidStop,
                   _delegate_animationShouldStart);
      _currentDelegate = _delegate;
    }
  else
    {
      NSDebugMLLog(@"NSAnimationDelegate",
                   @" No delegate : clear delegation methods");
      _delegate_animationDidReachProgressMark =
        (void (*)(id,SEL,NSAnimation*,NSAnimationProgress)) NULL;
      _delegate_animationValueForProgress =
        (float (*)(id,SEL,NSAnimation*,NSAnimationProgress)) NULL;
      _delegate_animationDidEnd =
        (void (*)(id,SEL,NSAnimation*)) NULL;
      _delegate_animationDidStop =
        (void (*)(id,SEL,NSAnimation*)) NULL;
      _delegate_animationShouldStart =
        (BOOL (*)(id,SEL,NSAnimation*)) NULL;
      _currentDelegate = nil;
    }
  
  if (_animator == nil || _isANewAnimatorNeeded)
    {
      TEST_RELEASE(_animator);

      _animator = [[GSAnimator allocWithZone: [self zone]]
                      initWithAnimation: self
                      frameRate: _frameRate];
      NSAssert(_animator,@"Can not create a GSAnimator");
      NSDebugMLLog(@"NSAnimationAnimator", @"New GSAnimator: %@", _animator);
      _isANewAnimatorNeeded = NO;
    }

  switch (_blockingMode)
    {
      case NSAnimationBlocking:
        [self _gs_startAnimationInOwnLoop];
        //[_animator setRunLoopModesForAnimating:
        //  [NSArray arrayWithObject: NSAnimationBlockingRunLoopMode]];
        //[_animator startAnimation];
        break;
      case NSAnimationNonblocking:
        {
          NSArray *runLoopModes;

          runLoopModes = [self runLoopModesForAnimating];
          if (runLoopModes == nil)
            runLoopModes = _NSAnimationDefaultRunLoopModes;
          [_animator setRunLoopModesForAnimating: runLoopModes];
        }
        [_animator startAnimation];
        break;
      case NSAnimationNonblockingThreaded:
        _isThreaded = YES;
        [NSThread
          detachNewThreadSelector: @selector (_gs_startThreadedAnimation)
                         toTarget: self 
                       withObject: nil];
    }
}

- (void) startWhenAnimation: (NSAnimation*)animation
	    reachesProgress: (NSAnimationProgress)start
{
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;

  _startAnimation = animation;
  _startMark = start;

  [_startAnimation addProgressMark: _startMark];
  NSDebugMLLog (@"NSAnimationMark",@"register for progress %f", start);
  [[NSNotificationCenter defaultCenter]
    addObserver: self
       selector: @selector (_gs_startAnimationReachesProgressMark:)
	   name: NSAnimationProgressMarkNotification
	 object: _startAnimation];

  _NSANIMATION_UNLOCK;
}

- (void) stopAnimation
{
  _NSANIMATION_LOCKING_SETUP;

  if ([self isAnimating])
    {
      _NSANIMATION_LOCK;
      [_animator stopAnimation];
      _NSANIMATION_UNLOCK;
    }
}

- (void) stopWhenAnimation: (NSAnimation*)animation
	   reachesProgress: (NSAnimationProgress)stop
{
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;

  _stopAnimation = animation;
  _stopMark = stop;

  [_stopAnimation addProgressMark: _stopMark];
  NSDebugMLLog (@"NSAnimationMark",@"register for progress %f", stop);
  [[NSNotificationCenter defaultCenter]
    addObserver: self
       selector: @selector (_gs_stopAnimationReachesProgressMark:)
	   name: NSAnimationProgressMarkNotification
	 object: _stopAnimation];

  _NSANIMATION_UNLOCK;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeInt: (int)[self animationCurve] 
                 forKey: @"NSAnimationAnimationCurve"];
      [aCoder encodeDouble: [self duration] 
                    forKey: @"NSAnimationDuration"];
    }
  else
    {
      [self notImplemented: _cmd];
    }
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      NSTimeInterval duration = 1.0;
      NSAnimationCurve animationCurve = 0;

      if ([aCoder containsValueForKey: @"NSAnimationAnimationCurve"])
        {
          animationCurve = [aCoder decodeIntForKey: @"NSAnimationAnimationCurve"];
        }
      if ([aCoder containsValueForKey: @"NSAnimationDuration"])
        {
          duration = [aCoder decodeDoubleForKey: @"NSAnimationDuration"];
        }
      return [self initWithDuration: duration
                     animationCurve: animationCurve];
    }
  else
    {
      [self notImplemented: _cmd];
    }
  return self;
}

/*
 * protocol GSAnimation (callbacks)
 */

- (void) animatorDidStart
{
  id delegate;
  _NSANIMATION_LOCKING_SETUP;

  NSDebugMLLog(@"NSAnimationAnimator",@"");

  _NSANIMATION_LOCK;

  delegate = _currentDelegate;

  if (_delegate_animationShouldStart) // method is cached (the animation is running)
    {
      NSDebugMLLog(@"NSAnimationDelegate",@"[delegate animationShouldStart] (cached)");
      _delegate_animationShouldStart (delegate,@selector(animationShouldStart:),self);
    }
  RETAIN (self);

  _NSANIMATION_UNLOCK;
}

- (void) animatorDidStop
{
  id delegate;
  _NSANIMATION_LOCKING_SETUP;

  NSDebugMLLog(@"NSAnimationAnimator",@"Progress = %f", _currentProgress);

  _NSANIMATION_LOCK;

  delegate = _currentDelegate;
  if (_currentProgress < 1.0)
    {
      if (_delegate_animationDidStop) // method is cached (the animation is running)
        {
          NSDebugMLLog(@"NSAnimationDelegate",@"[delegate animationDidStop] (cached)");
          _delegate_animationDidStop (delegate,@selector(animationDidStop:),self);
        }
    }
  else
    {
      if (_delegate_animationDidEnd) // method is cached (the animation is running)
        {
          NSDebugMLLog(@"NSAnimationDelegate",@"[delegate animationDidEnd] (cached)");
          _delegate_animationDidEnd (delegate,@selector(animationDidEnd:),self);
        }
    }
  RELEASE (self);

  _NSANIMATION_UNLOCK;
}

- (void) animatorStep: (NSTimeInterval) elapsedTime;
{
  NSAnimationProgress progress;
  _NSANIMATION_LOCKING_SETUP;

  NSDebugMLLog(@"NSAnimationAnimator", @"Elapsed time : %f", elapsedTime);

  _NSANIMATION_LOCK;

  progress = (elapsedTime / _duration);

  { // have some marks been passed ?
    // NOTE: the case where progress == markedProgress is
    //       treated in [-setCurrentProgress]
    unsigned count = GSIArrayCount (_progressMarks);
    NSAnimationProgress markedProgress;
    while ( _nextMark < count
            && progress > (markedProgress = GSIArrayItemAtIndex (_progressMarks,_nextMark)) ) // is a mark reached ?
      {
        [self _gs_didReachProgressMark: markedProgress];
      }
  }

  [self setCurrentProgress: progress];

  _NSANIMATION_UNLOCK;
}

@end //implementation NSAnimation

@implementation NSAnimation (PrivateNotificationCallbacks)

- (void) _gs_startAnimationReachesProgressMark: (NSNotification*)notification
{
  NSAnimation *animation;
  NSAnimationProgress mark;
  _NSANIMATION_LOCKING_SETUP;
 
  _NSANIMATION_LOCK;
  animation = [notification object];
  mark = [[[notification userInfo] objectForKey: NSAnimationProgressMark] floatValue];

  NSDebugMLLog(@"NSAnimationMark",
               @"Start Animation %@ reaches %f", animation, mark);

  if ( animation == _startAnimation && mark == _startMark)
    {
  //    [self clearStartAnimation];
      [self startAnimation];
    }

  _NSANIMATION_UNLOCK;
}


- (void) _gs_stopAnimationReachesProgressMark: (NSNotification*)notification
{
  NSAnimation *animation;
  NSAnimationProgress mark;
  _NSANIMATION_LOCKING_SETUP;
 
  _NSANIMATION_LOCK;
  animation = [notification object];
  mark = [[[notification userInfo] objectForKey: NSAnimationProgressMark] floatValue];

  NSDebugMLLog(@"NSAnimationMark",
               @"Stop Animation %@ reaches %f",animation, mark);


  if ( animation == _stopAnimation && mark == _stopMark)
    {
  //    [self clearStopAnimation];
      [self stopAnimation];
    }

  _NSANIMATION_UNLOCK;
}

@end // implementation NSAnimation (PrivateNotificationCallbacks)

@implementation NSAnimation (Private)

- (void) _gs_didReachProgressMark: (NSAnimationProgress) progress
{
  _NSANIMATION_LOCKING_SETUP;

  NSDebugMLLog(@"NSAnimationMark", @"progress %f", progress);

  _NSANIMATION_LOCK;

  // calls delegate's method
  if (_delegate_animationDidReachProgressMark) // method is cached (the animation is running)
    {
      NSDebugMLLog(@"NSAnimationDelegate",
                   @"[delegate animationdidReachProgressMark] (cached)");
      _delegate_animationDidReachProgressMark (_currentDelegate,
                                               @selector(animation:didReachProgressMark:),
                                               self,progress);
    }
  else // method is not cached (the animation did not start yet)
    if ( _delegate != nil
         && [_delegate
              respondsToSelector: @selector(animation:didReachProgressMark:)] )
      {
        NSDebugMLLog(@"NSAnimationDelegate",
                     @"[delegate animationdidReachProgressMark]");
        [_delegate animation: self didReachProgressMark: progress];
      }

  // posts a notification
  NSDebugMLLog(@"NSAnimationNotification",
               @"Post NSAnimationProgressMarkNotification : %f", progress);
  [[NSNotificationCenter defaultCenter]
    postNotificationName: NSAnimationProgressMarkNotification
		  object: self
		userInfo: [NSDictionary 
                            dictionaryWithObject: [NSNumber numberWithFloat: progress]
					  forKey: NSAnimationProgressMark
			  ]
  ];

  // skips marks with the same progress value
  while (
    (++_nextMark) < GSIArrayCount(_progressMarks)
    && GSIArrayItemAtIndex(_progressMarks, _nextMark) == progress
    )
  ;

  _NSANIMATION_UNLOCK;

  NSDebugMLLog(@"NSAnimationMark",
               @"Next mark #%d for %f",
               _nextMark, GSIArrayItemAtIndex(_progressMarks, _nextMark - 1));
}

- (void) _gs_startThreadedAnimation
{
  // NSAssert(_isThreaded);
  CREATE_AUTORELEASE_POOL(pool);
  NSDebugMLLog(@"NSAnimationThread",
               @"Start of %@", [NSThread currentThread]);
  [self _gs_startAnimationInOwnLoop];
  NSDebugMLLog(@"NSAnimationThread",
               @"End of %@", [NSThread currentThread]);
  [pool drain];
  _isThreaded = NO;
}


- (void) _gs_startAnimationInOwnLoop
{
  NSRunLoop	*loop;
  NSDate *end;

  [_animator setRunLoopModesForAnimating:
    [NSArray arrayWithObject: NSAnimationBlockingRunLoopMode]];
  [_animator startAnimation];
  loop = [NSRunLoop currentRunLoop];
  end = [NSDate distantFuture];
  for (;;)
    {
      if ([loop runMode: NSAnimationBlockingRunLoopMode beforeDate: end] == NO)
        {
          NSDate	*d;
          CREATE_AUTORELEASE_POOL(pool);

          d = [loop limitDateForMode: NSAnimationBlockingRunLoopMode];
          if (d == nil)
            {
              [pool drain];
              break;	// No inputs and no timers.
            }
          [NSThread sleepUntilDate: d];
          [pool drain];
        }
    }
}

- (_NSAnimationCurveDesc*) _gs_curveDesc
{ return &self->_curveDesc; }

- (NSAnimationProgress) _gs_curveShift
{ return _curveProgressShift; }

@end // implementation NSAnimation (Private)

@implementation NSAnimation (GNUstep)

- (unsigned int) frameCount
{
  unsigned c;
  _NSANIMATION_LOCKING_SETUP;
  
  _NSANIMATION_LOCK;
  c = (_animator != nil)? [_animator frameCount] : 0;
  _NSANIMATION_UNLOCK;
  return c;
}

- (void) resetCounters
{ 
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  if (_animator != nil) [_animator resetCounters];
  _NSANIMATION_UNLOCK;
}

- (float) actualFrameRate;
{ 
  float r;
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  r = (_animator != nil)? [_animator frameRate] : 0.0;
  _NSANIMATION_UNLOCK;
  return r;
}

@end

/*=======================*
 * NSViewAnimation class *
 *=======================*/

@interface _GSViewAnimationBaseDesc : NSObject
{
  id _target;
  NSRect _startFrame;
  NSRect _endFrame;
  NSString* _effect;
}

- (id) initWithProperties: (NSDictionary*)properties;
- (void) setCurrentProgress: (float)progress;
- (void) setTargetFrame: (NSRect) frame;

@end

@interface _GSViewAnimationDesc : _GSViewAnimationBaseDesc
{
  BOOL _shouldHide;
  BOOL _shouldUnhide;
}
@end

@interface _GSWindowAnimationDesc : _GSViewAnimationBaseDesc
{
  float _startAlpha;
}
@end

@implementation _GSViewAnimationBaseDesc

- (id) initWithProperties: (NSDictionary*)properties
{
  if ([self isMemberOfClass: [_GSViewAnimationBaseDesc class]])
    {
      NSZone* zone;
      id target;

      zone = [self zone];
      RELEASE (self);
      target = [properties objectForKey: NSViewAnimationTargetKey];
      if (target != nil)
        {
          if ([target isKindOfClass: [NSView class]])
            self = [[_GSViewAnimationDesc allocWithZone: zone]
                      initWithProperties: properties];
          else if ([target isKindOfClass: [NSWindow class]])
            self = [(_GSWindowAnimationDesc*)[_GSWindowAnimationDesc allocWithZone: zone]
                      initWithProperties: properties];
          else
            [NSException
               raise: NSInvalidArgumentException
              format: @"Invalid viewAnimation property :"
                      @"target is neither a NSView nor a NSWindow"];
        }
      else
        [NSException
           raise: NSInvalidArgumentException
          format: @"Invalid viewAnimation property :"
                  @"target is nil"];
    }
  else
    { // called from a subclass
      if ((self = [super init]))
        {
          NSValue* startValue;
          NSValue*   endValue;
          _target    = [properties objectForKey: NSViewAnimationTargetKey];
          startValue = [properties objectForKey: NSViewAnimationStartFrameKey];
          endValue   = [properties objectForKey: NSViewAnimationEndFrameKey];
          _effect    = [properties objectForKey: NSViewAnimationEffectKey];

          _startFrame = (startValue!=nil) ?
            [startValue rectValue]
            : [_target frame];
          _endFrame = (endValue!=nil) ?
            [endValue rectValue]
            : [_target frame];
        }
    }
  return self;
}

- (void) setCurrentProgress: (float)progress
{
  if (progress < 1.0f)
    {
      NSRect r;

      r.origin.x    = _startFrame.origin.x
        + progress*( _endFrame.origin.x - _startFrame.origin.x );
      r.origin.y    = _startFrame.origin.y
        + progress*( _endFrame.origin.y - _startFrame.origin.y );
      r.size.width  = _startFrame.size.width
        + progress*( _endFrame.size.width - _startFrame.size.width );
      r.size.height = _startFrame.size.height
        + progress*( _endFrame.size.height - _startFrame.size.height );

      [self setTargetFrame: r];

      if (_effect == NSViewAnimationFadeOutEffect)
        {
          [self subclassResponsibility: _cmd];
        }
      if (_effect == NSViewAnimationFadeInEffect)
        {
          [self subclassResponsibility: _cmd];
        }
    }
  else
    {
      [self setTargetFrame: _endFrame];
    }
}

- (void) setTargetFrame: (NSRect)frame
{
 [self subclassResponsibility: _cmd];
}

@end // implementation _GSViewAnimationDesc

@implementation _GSViewAnimationDesc

- (id) initWithProperties: (NSDictionary*)properties
{
  if ((self = [super initWithProperties: properties]))
    {
      _shouldHide = ([properties objectForKey: NSViewAnimationEndFrameKey] == nil);
      _shouldUnhide = ( _effect == NSViewAnimationFadeInEffect
                        && [_target isHidden]
                        && !_shouldHide);
    }
  return self;
}

- (void) setCurrentProgress: (float)progress
{
  [super setCurrentProgress: progress];
  if (_effect == NSViewAnimationFadeOutEffect) {}
    /* ??? TODO */;
  if (_effect == NSViewAnimationFadeInEffect) {}
    /* ??? TODO */;

  if (progress>=1.0f)
    {
      if (_shouldHide)
        [_target setHidden:YES];
      else if (_shouldUnhide)
        [_target setHidden:NO];
    }
}

- (void) setTargetFrame: (NSRect)frame
{
  [_target setFrame: frame];
}

@end // implementation _GSViewAnimationDesc

@implementation _GSWindowAnimationDesc

- (id) initWithProperties: (NSDictionary*)properties
{
  if ((self = [super initWithProperties: properties]))
    {
      _startAlpha = [_target alphaValue];
    }
  return self;
}

- (void) setCurrentProgress: (float)progress
{
  [super setCurrentProgress: progress];
  if (_effect == NSViewAnimationFadeOutEffect)
    [_target setAlphaValue: _startAlpha * (1.0f - progress)];
  if (_effect == NSViewAnimationFadeInEffect)
    [_target setAlphaValue: _startAlpha + (1.0f - _startAlpha) * progress];

  if (progress >= 1.0f)
    {
      if (_effect == NSViewAnimationFadeOutEffect)
        [_target orderBack: self];
      if (_effect == NSViewAnimationFadeInEffect)
        [_target orderFront: self];
    }
}

- (void) setTargetFrame: (NSRect) frame
{
  [_target setFrame: frame display: YES];
}

@end // implementation _GSWindowAnimationDesc

@implementation NSViewAnimation

- (id) initWithViewAnimations: (NSArray*)animations
{
  self = [self initWithDuration: 0.5 animationCurve: NSAnimationEaseInOut];
  if (self)
    {
      [self setAnimationBlockingMode: NSAnimationNonblocking];
      [self setViewAnimations: animations];
    }
  return self;
}

- (void) dealloc
{
  DESTROY(_viewAnimations);
  DESTROY(_viewAnimationDesc);
  [super dealloc];
}

- (void) setViewAnimations: (NSArray*)animations
{
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  if (_viewAnimations != animations)
    DESTROY(_viewAnimationDesc);
  ASSIGN(_viewAnimations, animations) ;
  _NSANIMATION_UNLOCK;
}

- (NSArray*) viewAnimations
{
  NSArray *a;
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  a = _viewAnimations;
  _NSANIMATION_UNLOCK;
  return a;
}

- (void) startAnimation
{
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  if (_viewAnimationDesc == nil)
    {
      unsigned int i, c;

      c = [_viewAnimations count];
      _viewAnimationDesc = [[NSMutableArray alloc] initWithCapacity: c];
      for (i = 0; i < c; i++)
        {
          _GSViewAnimationBaseDesc *vabd;

          vabd = [[_GSViewAnimationBaseDesc alloc]
                     initWithProperties: [_viewAnimations objectAtIndex:i]];
          [_viewAnimationDesc addObject: vabd];
          RELEASE(vabd);
        }
    }
  [super startAnimation];
  _NSANIMATION_UNLOCK;
}

- (void) stopAnimation
{
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  [super stopAnimation];
  [self setCurrentProgress: 1.0];
  _NSANIMATION_UNLOCK;
}

- (void) _gs_updateViewsWithValue: (NSNumber*) value
{
  // Runs in main thread : must not call any NSAnimation method to avoid a deadlock
  unsigned int i, c;
  float v;

  v = [value floatValue];
  if (_viewAnimationDesc != nil)
    for (i = 0, c = [_viewAnimationDesc count]; i < c; i++)
      [[_viewAnimationDesc objectAtIndex: i] setCurrentProgress: v];
}


- (void) setCurrentProgress: (NSAnimationProgress)progress
{
  _NSANIMATION_LOCKING_SETUP;

  _NSANIMATION_LOCK;
  [super setCurrentProgress: progress];
  [self performSelectorOnMainThread: @selector (_gs_updateViewsWithValue:)
                         withObject: [NSNumber numberWithFloat:[self currentValue]]
                      waitUntilDone: YES];
  _NSANIMATION_UNLOCK;
}

@end // implementation NSViewAnimation

