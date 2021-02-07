#include <Foundation/NSIndexPath.h>
#include "ObjectTesting.h"

@class NSAutoreleasePool;
int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];

  PASS (1, "include of Foundation/NSIndexPath.h works");
  [arp release];
  return 0;
}
