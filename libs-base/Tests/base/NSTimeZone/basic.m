#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSTimeZone.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSArray *testObj = [NSTimeZone defaultTimeZone];

  test_NSObject(@"NSTimeZone", [NSArray arrayWithObject:testObj]); 

  [arp release]; arp = nil;
  return 0;
}
