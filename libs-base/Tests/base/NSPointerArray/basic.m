#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSPointerArray.h>

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  NSPointerArray        *testObj;

  testObj = [[NSPointerArray new] autorelease];
  [testObj addPointer: @"hello"];
  test_alloc(@"NSPointerArray");
  test_NSObject(@"NSPointerArray", [NSArray arrayWithObject: testObj]); 
  test_NSCopying(@"NSPointerArray",@"NSPointerArray", 
   [NSArray arrayWithObject: testObj], NO, YES); 
  // test_NSCoding([NSArray arrayWithObject: testObj]); 

  [arp release]; arp = nil;
  return 0;
}
