#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSCache.h>

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  NSCache		*testObj;

  testObj = [[NSCache new] autorelease];
  [testObj setObject: @"hello" forKey: @"there"];
  test_alloc(@"NSCache");
  test_NSObject(@"NSCache", [NSArray arrayWithObject: testObj]);

  [arp release]; arp = nil;
  return 0;
}
