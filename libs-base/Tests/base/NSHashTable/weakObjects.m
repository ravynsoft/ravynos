#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSHashTable.h>
#if __has_include(<objc/capabilities.h>)
#include <objc/capabilities.h>
#endif

int main()
{
  START_SET("NSHashTable weak objects")
#ifdef OBJC_CAP_ARC
  if (!objc_test_capability(OBJC_CAP_ARC))
#endif
  {
    SKIP("ARC support unavailable")
  }
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  NSHashTable *hashTable = [NSHashTable weakObjectsHashTable];

  NSAutoreleasePool *arp2 = [NSAutoreleasePool new];

  id testObj = [[[NSObject alloc] init] autorelease];
  [hashTable addObject:testObj];
  PASS([[hashTable allObjects] count] == 1, "Table retains active weak reference");

  [arp2 release]; arp2 = nil;

  PASS([[hashTable allObjects] count] == 0, "Table removes dead weak reference");

  [arp release]; arp = nil;
  END_SET("NSHashTable weak objects")
  return 0;
}
