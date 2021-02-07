#include <Foundation/NSCoder.h>
#include "ObjectTesting.h"

@class NSAutoreleasePool;
int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];

  PASS (1, "include of Foundation/NSCoder.h works");
  [arp release];
  return 0;
}
