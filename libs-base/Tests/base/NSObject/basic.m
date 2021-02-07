#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSObject.h>
int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  test_alloc(@"NSObject");
  test_NSObject(@"NSObject", [NSArray arrayWithObject:[[NSObject new] autorelease]]);
  PASS([[[NSObject new] autorelease] methodSignatureForSelector: 0] == nil,
    "a null selector proiduces a nil method signature");
  [arp release]; arp = nil;
  return 0;
}
