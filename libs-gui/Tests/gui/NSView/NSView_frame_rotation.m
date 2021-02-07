/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>
*/
#include "Testing.h"

#include <math.h>

#include <Foundation/NSAutoreleasePool.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSView.h>
#include <AppKit/NSWindow.h>

int main(int argc, char **argv)
{
	CREATE_AUTORELEASE_POOL(arp);

	NSWindow *window;
	NSView *view;
	int passed;

	START_SET("NView GNUstep frame_rotation")

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

	window=[[NSWindow alloc] init];
	view=[[NSView alloc] init];

	[[window contentView] addSubview: view];

#define CHECK(f) \
	if (fabs(f-[view frameRotation])>0.0001 \
	 && fabs(f-[view frameRotation]-360.0)>0.0001 \
	 && fabs(f-[view frameRotation]+360.0)>0.0001) \
	{ \
		passed=0; \
		printf("expected rotation %30.25f, got %30.25f\n",f,[view frameRotation]); \
	}

	passed=1;
	CHECK(0.0)
	[view setFrameRotation: 45.0];
	CHECK(45.0)
	[view setFrameRotation: 0.0];
	CHECK(0.0)
	[view setFrameRotation: 90.0];
	CHECK(90.0)
	[view setFrameRotation: 180.0];
	CHECK(180.0)
	[view setFrameRotation: 360.0];
	CHECK(0.0)
	[view setFrameRotation: 0.0];
	CHECK(0.0)
	[view setFrameRotation: -45.0];
	CHECK(315.0)
	[view setFrameRotation: 0.0];
	CHECK(0.0)

	pass(passed,"-frameRotation/-setFrameRotation work");

	END_SET("NView GNUstep frame_rotation")

	DESTROY(arp);
	return 0;
}

