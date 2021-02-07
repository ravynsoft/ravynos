#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSMapTable.h>
#if __has_include(<objc/capabilities.h>)
#include <objc/capabilities.h>
#endif

int main()
{
  START_SET("NSMapTable weak objects")
#ifdef OBJC_CAP_ARC
  if (!objc_test_capability(OBJC_CAP_ARC))
#endif
  {
    SKIP("ARC support unavailable")
  }
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  NSMapTable *mapTable = [NSMapTable strongToWeakObjectsMapTable];

  NSAutoreleasePool *arp2 = [NSAutoreleasePool new];

  id testObj1 = [[[NSObject alloc] init] autorelease];
  id testObj2 = [[[NSObject alloc] init] autorelease];
  
  [mapTable setObject:testObj1 forKey:@"test"];
  PASS([mapTable objectForKey:@"test"] == testObj1, "Table retains first active weak reference");
  
  [mapTable setObject:testObj2 forKey:@"test"];
  PASS([mapTable objectForKey:@"test"] == testObj2, "Table retains second active weak reference");

  [arp2 release]; arp2 = nil;

  PASS([mapTable objectForKey:@"test"] == nil, "Table removes dead weak reference");

  [arp release]; arp = nil;
  END_SET("NSMapTable weak objects")
  return 0;
}
