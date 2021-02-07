#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSConnection.h>


int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id testObject = nil;
  test_alloc(@"NSConnection");
  testObject = [NSConnection new];
  test_NSObject(@"NSConnection",[NSArray arrayWithObject:testObject]); 
  testObject = [NSConnection defaultConnection];
  PASS(testObject != nil && [testObject isKindOfClass:[NSConnection class]],
       "NSConnection +defaultConnection works");
  
  [arp release]; arp = nil;
  return 0;
}
