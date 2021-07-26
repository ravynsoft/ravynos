#import <QuartzCore/CAAnimation.h>

@interface CATransition : CAAnimation {
    NSString *_type;
    NSString *_subtype;
    float _startProgress;
    float _endProgress;
}

@property(copy) NSString *type;
@property(copy) NSString *subtype;
@property float startProgress;
@property float endProgress;

@end
