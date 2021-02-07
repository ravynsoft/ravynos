#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSCharacterSet.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id testObj = [NSMutableCharacterSet new];
  test_alloc(@"NSMutableCharacterSet");
  test_NSObject(@"NSMutableCharacterSet",[NSArray arrayWithObject:testObj]);
  test_NSCoding([NSArray arrayWithObject:testObj]);
  test_NSCopying(@"NSCharacterSet", @"NSMutableCharacterSet",
                 [NSArray arrayWithObject:testObj], NO, NO);
  test_NSMutableCopying(@"NSCharacterSet", @"NSMutableCharacterSet", 
                        [NSArray arrayWithObject:testObj]);
  [arp release]; arp = nil;
  return 0;
}

