#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSMapTable.h>

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  NSMapTable		*testObj;

  testObj = [[NSMapTable new] autorelease];
  [testObj setObject: @"hello" forKey: @"there"];
  test_alloc(@"NSMapTable");
  test_NSObject(@"NSMapTable", [NSArray arrayWithObject: testObj]); 
  test_NSCopying(@"NSMapTable",@"NSMapTable", 
   [NSArray arrayWithObject: testObj], NO, YES); 
  // test_NSCoding([NSArray arrayWithObject: testObj]); 

  [arp release]; arp = nil;
  return 0;
}
