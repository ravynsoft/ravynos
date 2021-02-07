#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSCharacterSet.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id testObj = [NSCharacterSet alphanumericCharacterSet];
  test_alloc(@"NSCharacterSet"); 
  test_NSObject(@"NSCharacterSet", [NSArray arrayWithObject:testObj]); 
  test_NSCoding([NSArray arrayWithObject:testObj]);
  test_NSCopying(@"NSCharacterSet", @"NSMutableCharacterSet", [NSArray arrayWithObject:testObj], NO, NO);
  test_NSMutableCopying(@"NSCharacterSet",@"NSMutableCharacterSet", [NSArray arrayWithObject:testObj]);

  
  [arp release]; arp = nil;
  return 0;
}
