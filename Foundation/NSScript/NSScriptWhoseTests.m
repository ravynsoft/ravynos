#import <Foundation/NSScriptWhoseTests.h>

@implementation NSObject(NSScriptWhoseTests)

-(BOOL)isEqualTo:other {
   return [self isEqual:other];
}

@end
