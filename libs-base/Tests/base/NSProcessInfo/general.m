#import "Testing.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSProcessInfo *info = [NSProcessInfo processInfo];
  id obj;
  unsigned int val;
  PASS(info != nil, "NSProcessInfo understands [+processInfo]");
  
  obj = [info arguments];
  PASS((info != nil && 
       [obj isKindOfClass:[NSArray class]] &&
       [obj count] != 0),
       "NSProcessInfo has arguments");
  
  obj = [info environment];
  PASS((obj != nil &&
       [obj isKindOfClass:[NSDictionary class]]), 
       "NSProcessInfo has environment");
  
  obj = [info processName]; 
  PASS((obj != nil &&
       [obj isKindOfClass:[NSString class]] &&
       [obj length] > 0),
       "-processName is non-nil");

  obj = [info globallyUniqueString];
  PASS((obj != nil &&
       [obj isKindOfClass:[NSString class]] &&
       [obj length] > 0),
       "-globallyUniqueString works");

  obj = [info operatingSystemName];
  PASS((obj != nil && [obj isKindOfClass:[NSString class]] && [obj length] > 0),
    "-operatingSystemName works");
  NSLog(@"operatingSystemName %@", obj);
  
  val = [info operatingSystem];
  PASS(val != 0, "-operatingSystem works"); 
  
  testHopeful = YES;
  val = [info systemUptime];
  NSLog(@"systemUptime %lu", val);
  PASS(val != 0, "-systemUptime works");
  testHopeful = NO;
  
  obj = [info hostName];
  PASS((obj != nil && [obj isKindOfClass:[NSString class]] && [obj length] > 0),
    "-hostName works"); 
  NSLog(@"hostName %@", obj);
  [arp release]; arp = nil;
  return 0;
}
