#import <QuartzCore/CAPropertyAnimation.h>

@interface CABasicAnimation : CAPropertyAnimation {
    id _fromValue;
    id _toValue;
    id _byValue;
}

@property(retain) id fromValue;
@property(retain) id toValue;
@property(retain) id byValue;

@end
