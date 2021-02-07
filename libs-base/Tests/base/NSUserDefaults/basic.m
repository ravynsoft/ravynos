#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSUserDefaults.h>

int main()
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSUserDefaults	*defs = [NSUserDefaults new];

  test_NSObject(@"NSUserDefaults", [NSArray arrayWithObject: defs]); 

  defs = [NSUserDefaults standardUserDefaults];
  [defs setDouble: (double)42.42 forKey: @"aDouble"];
  PASS(EQ((double)42.42, [defs doubleForKey: @"aDouble"]),
    "can store double");
  [arp release]; arp = nil;
  return 0;
}
