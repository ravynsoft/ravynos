/*
copyright 2011 Fred Kiefer on 18.07.11.
*/
#include "Testing.h"

#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSGeometry.h>
#include <AppKit/NSView.h>

int main(int argc, char **argv)
{
  CREATE_AUTORELEASE_POOL(arp);
  NSRect f = NSMakeRect(0,0,100,100);
  NSView *v = [[NSView alloc] initWithFrame: f];
  int passed = 1;

  if (!NSEqualRects([v visibleRect], f))
    {
      passed = 0;
    }

  testHopeful = YES;
  pass(passed, "NSView -visibleRect works");
  testHopeful = NO;

  DESTROY(arp);
  return 0;
}
