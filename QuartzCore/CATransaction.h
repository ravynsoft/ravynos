#import <Foundation/NSObject.h>
#import <QuartzCore/CABase.h>

@class CAMediaTimingFunction;

CA_EXPORT NSString *const kCATransactionAnimationDuration;
CA_EXPORT NSString *const kCATransactionDisableActions;
CA_EXPORT NSString *const kCATransactionAnimationTimingFunction;
CA_EXPORT NSString *const kCATransactionCompletionBlock;

@interface CATransaction : NSObject

+ (BOOL)disableActions;
+ (CFTimeInterval)animationDuration;
+ (CAMediaTimingFunction *)animationTimingFunction;
//+(void (^)(void))completionBlock;
+ valueForKey:(NSString *)key;

+ (void)setAnimationDuration:(CFTimeInterval)value;
+ (void)setAnimationTimingFunction:(CAMediaTimingFunction *)value;
//+(void)setCompletionBlock:(void (^)(void))value;
+ (void)setDisableActions:(BOOL)value;
+ (void)setValue:value forKey:(NSString *)key;

+ (void)begin;
+ (void)commit;
+ (void)flush;

+ (void)lock;
+ (void)unlock;

@end
