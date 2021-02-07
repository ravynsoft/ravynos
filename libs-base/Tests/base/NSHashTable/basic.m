#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSHashTable.h>

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  NSHashTable		*testObj;

  testObj = [[NSHashTable new] autorelease];
  [testObj addObject: @"hello"];
  test_alloc(@"NSHashTable");
  test_NSObject(@"NSHashTable", [NSArray arrayWithObject: testObj]); 
  test_NSCopying(@"NSHashTable",@"NSHashTable", 
   [NSArray arrayWithObject: testObj], NO, YES); 
  // test_NSCoding([NSArray arrayWithObject: testObj]); 

  [arp release]; arp = nil;
  return 0;
}
