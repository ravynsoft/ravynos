/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>
#import <Foundation/NSDate.h>
#import <AppKit/AppKitExport.h>

@class NSMutableArray, NSArray, NSTimer;

typedef enum {
    NSAnimationEaseInOut,
    NSAnimationEaseIn,
    NSAnimationEaseOut,
    NSAnimationLinear,
} NSAnimationCurve;

typedef enum {
    NSAnimationBlocking,
    NSAnimationNonblocking,
    NSAnimationNonblockingThreaded
} NSAnimationBlockingMode;

typedef float NSAnimationProgress;

APPKIT_EXPORT NSString *const NSAnimationProgressMarkNotification;

APPKIT_EXPORT NSString *NSAnimationTriggerOrderIn;
APPKIT_EXPORT NSString *NSAnimationTriggerOrderOut;

@protocol NSAnimatablePropertyContainer

+ defaultAnimationForKey:(NSString *)key;

- animator;
- (NSDictionary *)animations;

- animationForKey:(NSString *)key;

- (void)setAnimations:(NSDictionary *)dictionary;

@end

@interface NSAnimation : NSObject <NSCopying> {
    NSTimeInterval _duration;
    NSAnimationCurve _curve;
    float _frameRate;
    NSAnimationBlockingMode _blockingMode;
    id _delegate;
    NSMutableArray *_progressMarks;
    float _currentValue;
    NSAnimationProgress _currentProgress;
    NSArray *_runLoopModes;
    BOOL _isAnimating;
    NSTimer *_timer;
    NSTimeInterval _startTime;
}

- initWithDuration:(NSTimeInterval)duration animationCurve:(NSAnimationCurve)curve;

- (NSTimeInterval)duration;
- (NSAnimationCurve)animationCurve;
- (float)frameRate;
- (NSAnimationBlockingMode)animationBlockingMode;
- delegate;
- (NSArray *)progressMarks;

- (void)setDuration:(NSTimeInterval)interval;
- (void)setAnimationCurve:(NSAnimationCurve)curve;
- (void)setFrameRate:(float)fps;
- (void)setAnimationBlockingMode:(NSAnimationBlockingMode)mode;
- (void)setDelegate:delegate;
- (void)setProgressMarks:(NSArray *)marks;
- (void)addProgressMark:(NSAnimationProgress)mark;
- (void)removeProgressMark:(NSAnimationProgress)mark;

- (NSAnimationProgress)currentProgress;
- (float)currentValue;
- (BOOL)isAnimating;
- (NSArray *)runLoopModesForAnimating;

- (void)setCurrentProgress:(NSAnimationProgress)progress;

- (void)clearStartAnimation;
- (void)clearStopAnimation;

- (void)startAnimation;
- (void)stopAnimation;

- (void)startWhenAnimation:(NSAnimation *)animation reachesProgress:(NSAnimationProgress)progress;
- (void)stopWhenAnimation:(NSAnimation *)animation reachesProgress:(NSAnimationProgress)progress;

@end

@interface NSObject (NSAnimationDelegate)
- (BOOL)animationShouldStart:(NSAnimation *)animation;
- (float)animation:(NSAnimation *)animation valueForProgress:(NSAnimationProgress)progress;
- (void)animation:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress)progress;
- (void)animationDidEnd:(NSAnimation *)animation;
- (void)animationDidStop:(NSAnimation *)animation;
@end

#import <AppKit/NSViewAnimation.h>
