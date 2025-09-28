#import "CATransactionGroup.h"
#import <QuartzCore/CATransaction.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSDictionary.h>

@implementation CATransactionGroup

-init {
   _values=[[NSMutableDictionary alloc] init];
   [_values setObject:[NSNumber numberWithFloat:0.25] forKey:kCATransactionAnimationDuration];
   [_values setObject:[NSNumber numberWithBool:NO] forKey:kCATransactionDisableActions];
   // kCATransactionAnimationTimingFunction default is nil
   // kCATransactionCompletionBlock default is nil
   return self;
}

-valueForKey:(NSString *)key {
   return [_values objectForKey:key];
}

-(void)setValue:value forKey:(NSString *)key {
   [_values setObject:value forKey:key];
}

@end
