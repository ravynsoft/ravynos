#import <QuartzCore/CAAnimation.h>

@interface CAPropertyAnimation : CAAnimation {
    NSString *_keyPath;
    BOOL _additive;
    BOOL _cumulative;
}

+ animationWithKeyPath:(NSString *)keyPath;
@property(copy) NSString *keyPath;
@property(getter=isAdditive) BOOL additive;
@property(getter=isCumulative) BOOL cumulative;

@end
