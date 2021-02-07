#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSSet.h>


int main()
{  
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSMutableSet *testObj = [NSMutableSet new];
  test_NSObject(@"NSMutableSet", [NSArray arrayWithObject:testObj]);
  test_NSCoding([NSArray arrayWithObject:testObj]);
  test_NSCopying(@"NSSet",@"NSMutableSet",
                 [NSArray arrayWithObject:testObj],NO,NO);
  test_NSMutableCopying(@"NSSet",@"NSMutableSet",
                        [NSArray arrayWithObject:testObj]);
  [arp release]; arp = nil;
  return 0;
}
