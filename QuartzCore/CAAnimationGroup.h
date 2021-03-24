#import <QuartzCore/CAAnimation.h>

@class NSArray;

@interface CAAnimationGroup : CAAnimation {
    NSArray *_animations;
}

@property(copy) NSArray *animations;

@end
