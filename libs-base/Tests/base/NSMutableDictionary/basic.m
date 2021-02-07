#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDictionary.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSMutableDictionary *testObj;
  
  testObj = [NSMutableDictionary new];
  
  test_NSObject(@"NSMutableDictionary", [NSArray arrayWithObject:testObj]);
  
  test_NSCoding([NSArray arrayWithObject:testObj]);

  test_keyed_NSCoding([NSArray arrayWithObject:testObj]);
  
  test_NSCopying(@"NSDictionary",@"NSMutableDictionary", 
                 [NSArray arrayWithObject:testObj], NO, NO);
  
  test_NSMutableCopying(@"NSDictionary",@"NSMutableDictionary", 
                        [NSArray arrayWithObject:testObj]);
  [arp release]; arp = nil;
  return 0;
}
