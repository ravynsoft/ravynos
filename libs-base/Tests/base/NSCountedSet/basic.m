#import "ObjectTesting.h"
#import <Foundation/NSSet.h>
#import <Foundation/NSAutoreleasePool.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id testObject = [NSCountedSet new];
  test_alloc(@"NSCountedSet");
  test_NSObject(@"NSCountedSet",[NSArray arrayWithObject:testObject]);
  test_NSCoding([NSArray arrayWithObject:testObject]);
  test_NSCopying(@"NSCountedSet",
                 @"NSCountedSet",
		 [NSArray arrayWithObject:testObject], NO, YES);
  test_NSMutableCopying(@"NSCountedSet",
                        @"NSCountedSet",
		        [NSArray arrayWithObject:testObject]);

  [arp release]; arp = nil;
  return 0;
}

