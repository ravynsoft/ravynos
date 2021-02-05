/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSDistributedNotificationCenter.h>
#import <Foundation/NSRaise.h>

NSString * const NSLocalNotificationCenterType=@"NSLocalNotificationCenterType";

@implementation NSDistributedNotificationCenter

+(NSDistributedNotificationCenter *)defaultCenter {
   return [self notificationCenterForType:NSLocalNotificationCenterType];
}

+(NSDistributedNotificationCenter *)notificationCenterForType:(NSString *)type {
   NSUnimplementedMethod();
   return nil;
}

-(BOOL)suspended {
   NSUnimplementedMethod();
   return NO;
}

-(void)setSuspended:(BOOL)value {
   NSUnimplementedMethod();
}

-(void)addObserver:observer selector:(SEL)selector name:(NSString *)name object:(NSString *)object {
   NSUnimplementedMethod();
}

-(void)addObserver:observer selector:(SEL)selector name:(NSString *)name object:(NSString *)object suspensionBehavior:(NSNotificationSuspensionBehavior)behavior {
   NSUnimplementedMethod();
}

-(void)removeObserver:observer name:(NSString *)name object:(NSString *)object {
   NSUnimplementedMethod();
}

-(void)postNotificationName:(NSString *)name object:(NSString *)object {
   NSUnimplementedMethod();
}

-(void)postNotificationName:(NSString *)name object:(NSString *)object userInfo:(NSDictionary *)userInfo {
   NSUnimplementedMethod();
}

-(void)postNotificationName:(NSString *)name object:(NSString *)object userInfo:(NSDictionary *)userInfo deliverImmediately:(BOOL)immediately {
   NSUnimplementedMethod();
}

-(void)postNotificationName:(NSString *)name object:(NSString *)object userInfo:(NSDictionary *)userInfo options:(NSUInteger)options {
   NSUnimplementedMethod();
}

@end
