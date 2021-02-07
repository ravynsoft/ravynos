#include "Testing.h"

#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSString.h>

#include <AppKit/NSApplication.h>
#include <AppKit/NSEvent.h>

int main()
{
  CREATE_AUTORELEASE_POOL(arp);
  NSEvent *ev;

  START_SET("NSEvent GNUstep delta")

  NS_DURING
  {
    [NSApplication sharedApplication];
  }
  NS_HANDLER
  {
    if ([[localException name] isEqualToString: NSInternalInconsistencyException ])
       SKIP("It looks like GNUstep backend is not yet installed")
  }
  NS_ENDHANDLER

  ev = [NSEvent mouseEventWithType: NSLeftMouseDown
                          location: NSMakePoint(0.0, 0.0)
                     modifierFlags: 0
                         timestamp: 0
                      windowNumber: 0
                           context: nil
                       eventNumber: 0
                        clickCount: 0
                          pressure: 0.0];
  pass([ev deltaX] == 0.0, "-deltaX with NSLeftMouseDown event works");
  pass([ev deltaY] == 0.0, "-deltaY with NSLeftMouseDown event works");
  pass([ev deltaZ] == 0.0, "-deltaZ with NSLeftMouseDown event works");

  ev = [NSEvent keyEventWithType: NSKeyDown
                        location: NSMakePoint(0.0, 0.0)
                   modifierFlags: 0
                       timestamp: 0
                    windowNumber: 0
                         context: nil
                      characters: @"a"
                charactersIgnoringModifiers: @"a"
                       isARepeat: NO
                         keyCode: 65];
  pass([ev deltaX] == 0.0, "-deltaX with NSKeyDown event works");
  pass([ev deltaY] == 0.0, "-deltaY with NSKeyDown event works");
  pass([ev deltaZ] == 0.0, "-deltaZ with NSKeyDown event works");

  END_SET("NSEvent GNUstep delta")

  DESTROY(arp);
  return 0;
}
