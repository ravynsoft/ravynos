#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSProcessInfo.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSProcessInfo *pi;
  test_alloc(@"NSProcessInfo");
  pi = [NSProcessInfo processInfo];
  test_NSObject(@"NSProcessInfo", [NSArray arrayWithObject:pi]);
  [arp release]; arp = nil;
  return 0;
}
