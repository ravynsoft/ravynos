#import "ObjectTesting.h"
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSAutoreleasePool.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id testObj;
  
  PASS([NSRunLoop new] == nil, "run loop initialises to nil");
  testObj = [NSRunLoop currentRunLoop];
  test_NSObject(@"NSRunLoop", [NSArray arrayWithObject:testObj]);

  PASS([NSTimer new] == nil, "timer initialises to nil");
  ASSIGN(testObj, [[NSTimer alloc] initWithFireDate: 0 interval: 0 target: [NSObject class] selector: @selector(description) userInfo: nil repeats: NO]);
  test_NSObject(@"NSTimer", [NSArray arrayWithObject:testObj]);
  
  [arp release]; arp = nil;
  return 0;
}
