#include "Testing.h"

#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSValue.h>

#include <AppKit/NSApplication.h>
#include <AppKit/NSCell.h>
#include <AppKit/NSImage.h>

int main()
{
  CREATE_AUTORELEASE_POOL(arp);
  NSCell *cell;
  NSNumber *num;

  START_SET("NSCell GNUstep objectValue")
  
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
  cell = [[NSCell alloc] init];
  num = [NSNumber numberWithFloat:55.0]; 
  [cell setObjectValue:num];
  pass([[cell objectValue] isEqual:num], 
       "-objectValue with NSNumber works");
  pass([cell floatValue] == 55.0, "-floatValue works");
  pass([cell intValue] == 55, "-intValue works");
  pass([cell doubleValue] == 55.0, "-doubleValue works");
  
  [cell setObjectValue:@"foo"];

  pass ([[cell objectValue] isEqual:@"foo"], "-objectValue with NSString works");
  
  [cell setObjectValue:[NSImage imageNamed:@"GNUstep"]];
  pass ([[cell objectValue] isEqual:[NSImage imageNamed:@"GNUstep"]],
        "-objectValue with NSImage works");
 
  END_SET("NSCell GNUstep objectValue")

  DESTROY(arp);
  return 0;
}

