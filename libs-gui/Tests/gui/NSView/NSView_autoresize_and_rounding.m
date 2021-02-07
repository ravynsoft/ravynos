//
//  NSView_autoresize_and_rounding.m
//
//  Created by Eric Wasylishen on 06.07.11
//
#include "Testing.h"

#include <math.h>

#include <Foundation/NSAutoreleasePool.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSView.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSTextView.h>

BOOL rects_almost_equal(NSRect r1, NSRect r2) {
  if (fabs(r1.origin.x - r2.origin.x)>0.001
      || fabs(r1.origin.y - r2.origin.y)>0.001
      || fabs(r1.size.width - r2.size.width)>0.001
      || fabs(r1.size.height - r2.size.height)>0.001)
    {
      printf("(1) expected frame (%g %g)+(%g %g), got (%g %g)+(%g %g)\n",
	     r2.origin.x, r2.origin.y, r2.size.width, r2.size.height,
	     r1.origin.x, r1.origin.y, r1.size.width, r1.size.height);

      return NO;
    }
  else
    {
      return YES;
    }
}

int CHECK(NSView *view, NSRect frame)
{
	NSRect r = [view frame];
	return rects_almost_equal(r, frame);
}

@interface TestView : NSView
@end
@implementation TestView
-(BOOL) isFlipped
{
  return YES;
}
@end


int main(int argc, char **argv)
{
	CREATE_AUTORELEASE_POOL(arp);

	NSWindow *window;
	NSView *container;
	NSView *view1;
	int passed = 1;

	START_SET("NSView GNUstep autoresize_and_rounding")

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

	window = [[NSWindow alloc] initWithContentRect: NSMakeRect(100,100,100,100)
		styleMask: NSBorderlessWindowMask
		backing: NSBackingStoreRetained
		defer: YES];

	container = [[NSView alloc] initWithFrame: NSMakeRect(0,0,100,100)];
	[[window contentView] addSubview: container];

	view1 = [[NSView alloc] initWithFrame: NSMakeRect(10,10,10,10)];
	[container addSubview: view1];

	/**
	 * Basic autoresizing test
	 */

	// No autosizing

	[view1 setAutoresizingMask: NSViewNotSizable];
	passed = CHECK(view1, NSMakeRect(10,10,10,10)) && passed;
	[container setFrameSize: NSMakeSize(50, 100)];
	passed = CHECK(view1, NSMakeRect(10,10,10,10)) && passed;
	[container setFrameSize: NSMakeSize(200, 100)];
	passed = CHECK(view1, NSMakeRect(10,10,10,10)) && passed;
	[container setFrameSize: NSMakeSize(100, 100)];	
	passed = CHECK(view1, NSMakeRect(10,10,10,10)) && passed;

	// NSViewWidthSizable

	[view1 setFrame: NSMakeRect(10,10,10,10)];
	[view1 setAutoresizingMask: NSViewWidthSizable];
	passed = CHECK(view1, NSMakeRect(10,10,10,10)) && passed;
	[container setFrameSize: NSMakeSize(96, 100)];
	passed = CHECK(view1, NSMakeRect(10,10,6,10)) && passed;
	[container setFrameSize: NSMakeSize(200, 100)];
	passed = CHECK(view1, NSMakeRect(10,10,110,10)) && passed;
	[container setFrameSize: NSMakeSize(100, 100)];	
	passed = CHECK(view1, NSMakeRect(10,10,10,10)) && passed;

	// NSViewWidthSizable | NSViewMaxXMargin

	[view1 setFrame: NSMakeRect(0,0,33,33)];
	[view1 setAutoresizingMask: NSViewWidthSizable | NSViewMaxXMargin];
	[container setFrameSize: NSMakeSize(200, 100)];
	passed = CHECK(view1, NSMakeRect(0,0,66,33)) && passed;
	[container setFrameSize: NSMakeSize(100, 100)];

	// NSViewWidthSizable | NSViewMinXMargin

	[view1 setFrame: NSMakeRect(50,0,25,25)];
	[view1 setAutoresizingMask: NSViewWidthSizable | NSViewMinXMargin];
	[container setFrameSize: NSMakeSize(175, 100)];
	passed = CHECK(view1, NSMakeRect(100,0,50,25)) && passed;
	[container setFrameSize: NSMakeSize(100, 100)];

	pass(passed,"NSView autoresizing works");
	
	/**
	 * Corner case tests, involving widths of 0, etc.
	 */
	
	// Test that a view with a width of 0 can be expanded
	// if it only has NSViewWidthSizable

	passed = 1;
	[view1 setFrame: NSMakeRect(50,0,0,25)];
	[view1 setAutoresizingMask: NSViewWidthSizable];
	[container setFrameSize: NSMakeSize(133, 100)];
	passed = CHECK(view1, NSMakeRect(50,0,33,25)) && passed;
	[container setFrameSize: NSMakeSize(100, 100)];
	
	// Test that if NSViewWidthSizable | NSViewMinXMargin is set,
	// and the view has width of 0, extra space is given to the margin
	// not the width

	[view1 setFrame: NSMakeRect(50,0,0,35)];
	[view1 setAutoresizingMask: NSViewWidthSizable | NSViewMinXMargin];
	[container setFrameSize: NSMakeSize(133, 100)];
	passed = CHECK(view1, NSMakeRect(83,0,0,35)) && passed;
	[container setFrameSize: NSMakeSize(100, 100)];

	// Test a {0, 0} sized window with NSViewWidthSizable | NSViewHeightSizable

	[container setFrameSize: NSMakeSize(0,0)];
	[view1 setFrame: NSMakeRect(0,0,0,0)];
	[view1 setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
	[container setFrameSize: NSMakeSize(4,4)];
	passed = CHECK(view1, NSMakeRect(0,0,4,4)) && passed;
	[container setFrameSize: NSMakeSize(100, 100)];

	// Test a {0, 0} sized window with NSMinXMargin | NSViewWidthSizable

	[container setFrameSize: NSMakeSize(0,0)];
	[view1 setFrame: NSMakeRect(0,0,0,0)];
	[view1 setAutoresizingMask: NSViewMinXMargin | NSViewWidthSizable];
	[container setFrameSize: NSMakeSize(6,6)];
	passed = CHECK(view1, NSMakeRect(3,0,3,0)) && passed;
	[container setFrameSize: NSMakeSize(100, 100)];

	// Test a {0, 0} sized window with all autoresizing flags set on the view

	[container setFrameSize: NSMakeSize(0,0)];
	[view1 setFrame: NSMakeRect(0,0,0,0)];
	[view1 setAutoresizingMask: NSViewMinXMargin | NSViewWidthSizable | NSViewMaxXMargin |
	       NSViewMinYMargin | NSViewHeightSizable | NSViewMaxYMargin];
	[container setFrameSize: NSMakeSize(9,9)];
	passed = CHECK(view1, NSMakeRect(3,3,3,3)) && passed;
	[container setFrameSize: NSMakeSize(100, 100)];

	pass(passed,"NSView autoresizing corner cases work");


	/**
	 * Rounding test
	 */

	passed = 1;
	[view1 setFrame: NSMakeRect(10, 10, 10, 10)];
	[view1 setAutoresizingMask: NSViewMinXMargin |
	       NSViewWidthSizable |
	       NSViewMaxXMargin |
	       NSViewMinYMargin |
	       NSViewHeightSizable |
	       NSViewMaxYMargin ];

	// All autoresize masks are enabled. Check that halving the
	// width and height of the window works as expected.

	passed = CHECK(view1, NSMakeRect(10,10,10,10)) && passed;
	[container setFrameSize: NSMakeSize(50, 50)]; // reduce to 50%
	passed = CHECK(view1, NSMakeRect(5,5,5,5)) && passed;

	[container setFrameSize: NSMakeSize(100, 100)]; // restore
	passed = CHECK(view1, NSMakeRect(10,10,10,10)) && passed;

	[container setFrameSize: NSMakeSize(33, 33)]; // reduce to 33%
	// NOTE: Frame should be rounded from NSMakeRect(3.3,3.3,3.3,3.3) to
	// NSMakeRect(3,3,3,3)
	passed = CHECK(view1, NSMakeRect(3,3,3,3)) && passed;

	[container setFrameSize: NSMakeSize(100, 100)]; // restore
	// NOTE: The following shows that the precision lost in the rounding
	// shown in the previous test was saved by the view
	passed = CHECK(view1, NSMakeRect(10,10,10,10)) && passed;


	// Now test that we can still set fractional frames

	[view1 setFrame: NSMakeRect(1.5, 1.5, 1.5, 1.5)];
	passed = CHECK(view1, NSMakeRect(1.5, 1.5, 1.5, 1.5)) && passed;

	pass(passed,"NSView autoresize rounding works");

	// Test centerScanRect

	{
	  NSView *view2 = [[NSView alloc] initWithFrame: NSMakeRect(0, 0, 100, 100)];
	  
          testHopeful = YES;
	  PASS(rects_almost_equal([view2 centerScanRect: NSMakeRect(0.5, 0.5, 100, 100)],
				  NSMakeRect(1, 1, 100, 100)),
	       "centerScanRect works 1");
	  PASS(rects_almost_equal([view2 centerScanRect: NSMakeRect(0.9, 0.9, 99.9, 99.9)],
				  NSMakeRect(1, 1, 100, 100)),
	       "centerScanRect works 2");       
	  PASS(rects_almost_equal([view2 centerScanRect: NSMakeRect(0.9, 0.9, 99.4, 99.4)],
				  NSMakeRect(1, 1, 99, 99)),
	       "centerScanRect works 3");
	  PASS(rects_almost_equal([view2 centerScanRect: NSMakeRect(0.4, 0.4, 99.4, 99.4)],
				  NSMakeRect(0, 0, 100, 100)),
	       "centerScanRect works 4");
          testHopeful = NO;

	  [view2 release];
	}

	{
	  NSView *view2 = [[TestView alloc] initWithFrame: NSMakeRect(0, 0, 100, 100)];
	  
          testHopeful = YES;
	  PASS(rects_almost_equal([view2 centerScanRect: NSMakeRect(0.5, 0.5, 100, 100)],
				  NSMakeRect(1, 0, 100, 100)),
	       "centerScanRect works 1");
	  PASS(rects_almost_equal([view2 centerScanRect: NSMakeRect(0.9, 0.9, 99.9, 99.9)],
				  NSMakeRect(1, 1, 100, 100)),
	       "centerScanRect works 2");       
	  PASS(rects_almost_equal([view2 centerScanRect: NSMakeRect(0.9, 0.9, 99.4, 99.4)],
				  NSMakeRect(1, 1, 99, 99)),
	       "centerScanRect works 3");
	  PASS(rects_almost_equal([view2 centerScanRect: NSMakeRect(0.4, 0.4, 99.4, 99.4)],
				  NSMakeRect(0, 0, 100, 100)),
	       "centerScanRect works 4");
          testHopeful = NO;

	  [view2 release];
	}

	END_SET("NSView GNUstep autoresize_and_rounding")

	DESTROY(arp);
	return 0;
}