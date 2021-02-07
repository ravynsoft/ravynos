#import <Foundation/Foundation.h>
#import "Testing.h"

static int      called = 0;

@interface NSMessageTest : NSObject
@end

@implementation NSMessageTest

- (void) methodToCall
{
  called++;
}

@end



int main(void)
{
  NSAutoreleasePool* pool = [NSAutoreleasePool new];
  NSMessageTest* test = [NSMessageTest new];

  [NS_MESSAGE(test, methodToCall) invoke];

  PASS(called > 0, "NS_MESSAGE worked");

  [pool release];
  return 0;
}



