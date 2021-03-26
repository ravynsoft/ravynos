#import <QuartzCore/CAAnimationGroup.h>

@implementation CAAnimationGroup

-(NSArray *)animations {
   return _animations;
}

-(void)setAnimations:(NSArray *)value {
   value=[value copy];
   [_animations release];
   _animations=value;
}

@end
