/*
copyright 2005 Alexander Malmberg <alexander@malmberg.org>

Test that the file lists in NSSavePanel are reloaded properly when the
delegate changes.
*/

#include "Testing.h"

#include <AppKit/AppKit.h>

@interface NSSavePanel (TestDelegate)
- (NSMatrix *)lastColumnMatrix;
@end

@implementation NSSavePanel (TestDelegate)

- (NSMatrix *)lastColumnMatrix
{
  return [_browser matrixInColumn: [_browser lastColumn]];
}

@end

@interface Delegate : NSObject
@end

@implementation Delegate


+ (BOOL) panel: (NSSavePanel *)p
shouldShowFilename: (NSString *)fname
{
  if ([[fname lastPathComponent] isEqual: @"B"])
    {
      return NO;
    }
  return YES;
}

@end

int main(int argc, char **argv)
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  NSSavePanel *p;
  NSMatrix *m;

  START_SET("NSSavePanel GNUstep setDelegate")

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

  p = [NSSavePanel savePanel];
  [p setShowsHiddenFiles: NO];
  [p setDirectory: [[[[[NSBundle mainBundle] bundlePath]
                       stringByDeletingLastPathComponent] stringByDeletingLastPathComponent]
                     stringByAppendingPathComponent: @"dummy"]];
  
  m = [p lastColumnMatrix];
  pass([m numberOfRows] == 2
       && [[[m cellAtRow: 0 column: 0] stringValue] isEqual: @"A"]
       && [[[m cellAtRow: 1 column: 0] stringValue] isEqual: @"B"],
       "browser initially contains all files");
  
  [p setDelegate: [Delegate self]];
  m = [p lastColumnMatrix];
  pass([m numberOfRows] == 1
       && [[[m cellAtRow: 0 column: 0] stringValue] isEqual: @"A"],
       "browser is reloaded after -setDelegate:");
  
  /* Not really a -setDelegate: issue, but the other methods involved are
     documented as doing the wrong thing.  */
  [p setDelegate: nil];
  m = [p lastColumnMatrix];
  testHopeful = YES;
  pass([m numberOfRows] == 2
       && [[[m cellAtRow: 0 column: 0] stringValue] isEqual: @"A"]
       && [[[m cellAtRow: 1 column: 0] stringValue] isEqual: @"B"],
       "browser contains all files after resetting delegate");
  testHopeful = NO;
  
  [p setDelegate: [Delegate self]];
  m = [p lastColumnMatrix];
  pass([m numberOfRows] == 1
       && [[[m cellAtRow: 0 column: 0] stringValue] isEqual: @"A"],
       "browser is reloaded after -setDelegate: (2)");

  END_SET("NSSavePanel GNUstep setDelegate")

  [arp release];
  return 0;
}
