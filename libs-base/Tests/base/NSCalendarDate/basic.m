#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/NSCalendarDate.h>
#import <Foundation/NSAutoreleasePool.h>

int main()
{  
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id testObj = [NSCalendarDate new];

  test_NSObject(@"NSCalendarDate", [NSArray arrayWithObject: testObj]);
  test_NSCoding([NSArray arrayWithObject: testObj]);
  test_NSCopying(@"NSCalendarDate", @"NSCalendarDate",
    [NSArray arrayWithObject: testObj], NO, NO);
  
  [arp release]; arp = nil;
  return 0;
}
