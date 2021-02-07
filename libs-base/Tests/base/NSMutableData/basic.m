#import "ObjectTesting.h"
#import <Foundation/NSData.h>
#import <Foundation/NSAutoreleasePool.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id testObject = [NSMutableData new];
  test_alloc(@"NSMutableData");
  test_NSObject(@"NSData",[NSArray arrayWithObject:testObject]);
  test_NSCoding([NSArray arrayWithObject:testObject]);
  test_NSCopying(@"NSData",
                 @"NSMutableData",
		 [NSArray arrayWithObject:testObject], NO, NO);
  test_NSMutableCopying(@"NSData",
                        @"NSMutableData",
		        [NSArray arrayWithObject:testObject]);

  [arp release]; arp = nil;
  return 0;
}

