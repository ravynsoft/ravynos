#import <QuartzCore/CABase.h>
#import <CoreFoundation/CoreFoundation.h>

@protocol CAMediaTiming

@property BOOL autoreverses;

@property CFTimeInterval beginTime;

@property CFTimeInterval duration;

@property(copy) NSString *fillMode;

@property float repeatCount;

@property CFTimeInterval repeatDuration;

@property float speed;

@property CFTimeInterval timeOffset;

@end
