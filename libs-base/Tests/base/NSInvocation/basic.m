#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSInvocation.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSMethodSignature *testSig = [NSMethodSignature signatureWithObjCTypes:"@@::"];
  NSInvocation *testObj = [NSInvocation invocationWithMethodSignature:testSig];
  test_alloc(@"NSInvocation");
  test_NSObject(@"NSInvocation", [NSArray arrayWithObject:testObj]);
  [arp release]; arp = nil;
  return 0;
}
