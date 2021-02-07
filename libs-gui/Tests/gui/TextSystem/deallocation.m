/*
  (C) Copyright 2012 Wolfgang Lux

  Check that no dangling pointers are left when elements of a text network
  are deallocated.

  FIXME In its present form, this test is unlikely to yield correct results
  with a garbage collected runtime.
*/

#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSLayoutManager.h>
#import <AppKit/NSTextContainer.h>
#import <AppKit/NSTextStorage.h>
#import <AppKit/NSTextView.h>

int
main(int argc, char **argv)
{
  NSLayoutManager *lm;
  NSTextStorage *ts;
  NSTextContainer *tc;
  NSTextView *tv;

  START_SET("TextSystem GNUstep deallocation")
  CREATE_AUTORELEASE_POOL(arp);

  NS_DURING
  {
    // Create shared application object (required by NSTextView)
    [NSApplication sharedApplication];
  }
  NS_HANDLER
  {
    if ([[localException name] isEqualToString: NSInternalInconsistencyException ])
       SKIP("It looks like GNUstep backend is not yet installed")
  }
  NS_ENDHANDLER

  // Set up text network retaining all elements
  ts = [NSTextStorage new];
  lm = [NSLayoutManager new];
  [ts addLayoutManager: lm];
  tc = [[NSTextContainer alloc] initWithContainerSize: NSMakeSize(100, 100)];
  [lm addTextContainer: tc];
  tv =
    [[NSTextView alloc] initWithFrame: NSMakeRect(0, 0, 100, 100)
			textContainer: tc];

  // Check text view returns the expected elements
  pass([tv textContainer] == tc,
       "NSTextView -textContainer returns text container");
  pass([tv layoutManager] == lm,
       "NSTextView -layoutManager returns layout manager");
  pass([tv textStorage] == ts,
       "NSTextView -textStorage returns text storage");

  // Release text storage
  [ts release];
  RECREATE_AUTORELEASE_POOL(arp);
  pass([tv textContainer] == tc,
       "NSTextView -textContainer returns text container");
  pass([tv layoutManager] == lm,
       "NSTextView -layoutManager returns layout manager");
  pass([tv textStorage] == nil, "NSTextView -textStorage returns nil");

  // Release layout manager
  [lm release];
  RECREATE_AUTORELEASE_POOL(arp);
  pass([tv textContainer] == tc,
       "NSTextView -textContainer returns text container");
  pass([tv layoutManager] == nil, "NSTextView -layoutManager returns nil");
  pass([tv textStorage] == nil, "NSTextView -textStorage returns nil");

  // Release text container
  [tc release];
  RECREATE_AUTORELEASE_POOL(arp);
  pass([tv textContainer] == nil, "NSTextView -textContainer returns nil");
  pass([tv layoutManager] == nil, "NSTextView -layoutManager returns nil");
  pass([tv textStorage] == nil, "NSTextView -textStorage returns nil");

  DESTROY(arp);
  END_SET("TextSystem GNUstep deallocation")

  return 0;
}
