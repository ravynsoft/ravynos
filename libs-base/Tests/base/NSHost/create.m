#import "ObjectTesting.h"
#import <Foundation/NSHost.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSHost *current;
  NSHost *localh;
  NSHost *tmp; 

  current = [NSHost currentHost];
  PASS(current != nil && [current isKindOfClass: [NSHost class]],
       "NSHost understands +currentHost");
 
#if	defined(GNUSTEP_BASE_LIBRARY)
  localh = [NSHost localHost];
  PASS(localh != nil && [localh isKindOfClass: [NSHost class]],
       "NSHost understands +localHost");
#else
  localh = current;
#endif

  tmp = [NSHost hostWithName: @"::1"];
  PASS([[tmp address] isEqual: @"::1"], "+hostWithName: works for IPV6 addr");

  tmp = [NSHost hostWithName: [current name]];
  PASS([tmp isEqualToHost: current], "NSHost understands +hostWithName:");
  
  tmp = [NSHost hostWithAddress: [current address]];
  PASS([tmp isEqualToHost: current], "NSHost understands +hostWithAddress:");
  
  tmp = [NSHost hostWithName: @"127.0.0.1"];
  PASS(tmp != nil && [tmp isEqualToHost: localh], 
       "NSHost understands [+hostWithName: 127.0.0.1]");
  
  [arp release]; arp = nil;
  return 0;
}
