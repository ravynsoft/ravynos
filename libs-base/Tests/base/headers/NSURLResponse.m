#include <Foundation/NSURLResponse.h>
#include "ObjectTesting.h"

@class NSAutoreleasePool;
int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];

  PASS (1, "include of Foundation/NSURLResponse.h works");
  [arp release];
  return 0;
}
