#import <Foundation/NSObject.h>
#import <QuartzCore/CABase.h>
#import <QuartzCore/CAMediaTiming.h>
#import <QuartzCore/CAAction.h>

@class CAMediaTimingFunction;

CA_EXPORT NSString *const kCATransitionFade;
CA_EXPORT NSString *const kCATransitionMoveIn;
CA_EXPORT NSString *const kCATransitionPush;
CA_EXPORT NSString *const kCATransitionReveal;

CA_EXPORT NSString *const kCATransitionFromLeft;
CA_EXPORT NSString *const kCATransitionFromRight;
CA_EXPORT NSString *const kCATransitionFromTop;
CA_EXPORT NSString *const kCATransitionFromBottom;

@interface CAAnimation : NSObject <NSCopying, CAMediaTiming, CAAction> {
    id _delegate;
    BOOL _removedOnCompletion;
    CAMediaTimingFunction *_timingFunction;
    BOOL _autoreverses;
    CFTimeInterval _beginTime;
    CFTimeInterval _duration;
    NSString *_fillMode;
    float _repeatCount;
    CFTimeInterval _repeatDuration;
    float _speed;
    CFTimeInterval _timeOffset;
}

+ animation;

@property(retain) id delegate;

@property(getter=isRemovedOnCompletion) BOOL removedOnCompletion;

@property(retain) CAMediaTimingFunction *timingFunction;

@end

@interface NSObject (CAAnimationDelegate)
- (void)animationDidStart:(CAAnimation *)animation;
- (void)animationDidStop:(CAAnimation *)animation finished:(BOOL)finished;
@end

#import <QuartzCore/CAPropertyAnimation.h>
#import <QuartzCore/CABasicAnimation.h>
#import <QuartzCore/CATransition.h>
#import <QuartzCore/CAAnimationGroup.h>
