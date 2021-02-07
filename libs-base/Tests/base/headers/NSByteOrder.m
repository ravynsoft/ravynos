#include <Foundation/NSByteOrder.h>
#include "ObjectTesting.h"

@class NSAutoreleasePool;
int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];

  PASS (1, "include of Foundation/NSByteOrder.h works");
  [arp release];
  return 0;
}
