#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSTask.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSArray *testObj = [NSTask new];

  test_NSObject(@"NSTask", [NSArray arrayWithObject:testObj]); 

  [arp release]; arp = nil;
  return 0;
}
