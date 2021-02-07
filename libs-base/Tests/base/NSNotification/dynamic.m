#import <Foundation/Foundation.h>
#include <objc/runtime.h>
#import "ObjectTesting.h"

static BOOL notifiedCurrent = NO;

@interface Toggle : NSObject
@end

@implementation Toggle
- (void) foo: (NSNotification*)n
{
  notifiedCurrent = NO;
}
- (void) bar: (NSNotification*)n
{
  notifiedCurrent = YES;
}
@end

int main(void)
{
  NSNotificationCenter *nc;
  id t = [Toggle new];

  [NSAutoreleasePool new];
  nc = [NSNotificationCenter new];
  [nc addObserver: t selector: @selector(foo:) name: nil object: nil];
  class_replaceMethod([Toggle class],
    @selector(foo:),
    class_getMethodImplementation([Toggle class], @selector(bar:)),
    "v@:@");
  [nc postNotificationName: @"foo" object: t];
  [t release];
  PASS(YES == notifiedCurrent, "implementation not cached");
  return 0;
}
