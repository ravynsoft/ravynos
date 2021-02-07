/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>
*/

#include "Testing.h"

#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSGeometry.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSClipView.h>
#include <AppKit/NSWindow.h>

#define TEST(r,s) \
	if (!NSEqualRects([v visibleRect],r)) \
	{ \
		printf("%s: got %s, expected %s\n", \
			s, \
			[NSStringFromRect([v visibleRect]) lossyCString], \
			[NSStringFromRect(r) lossyCString]); \
		pass(0,s); \
	} \
	else \
		pass(1,s);

int main(int argc, char **argv)
{
  NSWindow *window;
	CREATE_AUTORELEASE_POOL(arp);
	NSClipView *cv=[[NSClipView alloc] initWithFrame: NSMakeRect(0,0,10,10)];
	NSView *v=[[NSView alloc] initWithFrame: NSMakeRect(0,0,100,100)];
	[cv setDocumentView: v];

  START_SET("NView GNUstep scrollRectToVisible")

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

  window = [[NSWindow alloc] initWithContentRect: NSMakeRect(100,100,200,200)
                                       styleMask: NSClosableWindowMask
                                         backing: NSBackingStoreRetained
                                           defer: YES];
  [[window contentView] addSubview: cv];

	/* Initial position. */
	TEST(NSMakeRect(0,0,10,10),"1")

	/* Basic scrolling. */
	[v scrollRectToVisible: NSMakeRect(50,50,10,10)];
	TEST(NSMakeRect(50,50,10,10),"2");

	/* No scrolling necessary. */
	[v scrollRectToVisible: NSMakeRect(55,55,5,5)];
	TEST(NSMakeRect(50,50,10,10),"3");

	/* No scrolling necessary. */
	[v scrollRectToVisible: NSMakeRect(50,50,5,5)];
	TEST(NSMakeRect(50,50,10,10),"4");

	/* No scrolling necessary. */
	[v scrollRectToVisible: NSMakeRect(52,52,5,5)];
	TEST(NSMakeRect(50,50,10,10),"5");

	/* Minimal scrolling means that the "small-coordinate" corner should be
	visible. */
	[v scrollRectToVisible: NSMakeRect(80,80,20,20)];
	TEST(NSMakeRect(80,80,10,10),"6");

	/* And in this case, the "large-coordinate" corner. */
	[v scrollRectToVisible: NSMakeRect(0,0,20,20)];
	TEST(NSMakeRect(10,10,10,10),"7");

	/* If the visible rect is inside the target rect, no scrolling should
	occur. */
	[v scrollRectToVisible: NSMakeRect(5,5,20,20)];
	TEST(NSMakeRect(10,10,10,10),"8");

	/* Nor for a target rect on the edge of the visible rect. */
	[v scrollRectToVisible: NSMakeRect(10,10,20,20)];
	TEST(NSMakeRect(10,10,10,10),"9");

	/* Repeating the call shouldn't cause any scrolling. */
	[v scrollRectToVisible: NSMakeRect(10,10,20,20)];
	TEST(NSMakeRect(10,10,10,10),"10");

	/* Minimal scrolling and partial overlap. */
	[v scrollRectToVisible: NSMakeRect(7,7,5,5)];
	TEST(NSMakeRect(7,7,10,10),"11");

	[v scrollRectToVisible: NSMakeRect(15,15,5,5)];
	TEST(NSMakeRect(10,10,10,10),"12");

	END_SET("NView GNUstep scrollRectToVisible")

	DESTROY(arp);

	return 0;
}
