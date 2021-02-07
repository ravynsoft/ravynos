//
//  NSView_frame_bounds.m
//
//  Created by Fred Kiefer on 13.11.08.
//
#include "Testing.h"

#include <math.h>

#include <Foundation/NSAutoreleasePool.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSView.h>
#include <AppKit/NSWindow.h>


int CHECK(NSView *view, NSRect frame, NSRect bounds)
{
	NSRect r;

	r = [view frame];
	if (fabs(r.origin.x - frame.origin.x)>0.001
	 || fabs(r.origin.y - frame.origin.y)>0.001
	 || fabs(r.size.width - frame.size.width)>0.001
	 || fabs(r.size.height - frame.size.height)>0.001)
	{
		printf("(1) expected frame (%g %g)+(%g %g), got (%g %g)+(%g %g)\n",
			frame.origin.x, frame.origin.y, frame.size.width, frame.size.height,
			r.origin.x, r.origin.y, r.size.width, r.size.height);

		return 0;
	}

	r = [view bounds];
	if (fabs(r.origin.x - bounds.origin.x)>0.001
	 || fabs(r.origin.y - bounds.origin.y)>0.001
	 || fabs(r.size.width - bounds.size.width)>0.001
	 || fabs(r.size.height - bounds.size.height)>0.001)
	{
		printf("(2) expected bounds (%g %g)+(%g %g), got (%g %g)+(%g %g)\n",
			bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height,
			r.origin.x, r.origin.y, r.size.width, r.size.height);

		return 0;
	}

	return 1;
}

int main(int argc, char **argv)
{
	CREATE_AUTORELEASE_POOL(arp);

	NSWindow *window;
	NSView *view1;
	int passed = 1;

	START_SET("NView GNUstep frame_bounds")

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
	view1 = [[NSView alloc] initWithFrame: NSMakeRect(20,20,100,100)];

	[[window contentView] addSubview: view1];

	passed = CHECK(view1, NSMakeRect(20,20,100,100),NSMakeRect(0,0,100,100)) && passed;

	[view1 setFrameOrigin: NSMakePoint(10, 10)];
	passed = CHECK(view1, NSMakeRect(10,10,100,100),NSMakeRect(0,0,100,100)) && passed;

	[view1 setFrameSize: NSMakeSize(80, 80)];
	passed = CHECK(view1, NSMakeRect(10,10,80,80),NSMakeRect(0,0,80,80)) && passed;

	[view1 setFrameRotation: 45.0];
	passed = CHECK(view1, NSMakeRect(10,10,80,80),NSMakeRect(0,0,80,80)) && passed;

	[view1 setBoundsRotation: -45.0];
	passed = CHECK(view1, NSMakeRect(10,10,80,80),NSMakeRect(-56.5685,0,113.137,113.137)) && passed;

	[view1 setFrameSize: NSMakeSize(100, 100)];
	passed = CHECK(view1, NSMakeRect(10,10,100,100),NSMakeRect(-70.7107,0,141.421,141.421)) && passed;

	[view1 setFrameOrigin: NSMakePoint(20, 20)];
	passed = CHECK(view1, NSMakeRect(20,20,100,100),NSMakeRect(-70.7107,0,141.421,141.421)) && passed;

	[view1 setBoundsOrigin: NSMakePoint(20, 20)];
	passed = CHECK(view1, NSMakeRect(20,20,100,100),NSMakeRect(-50.7107,20,141.421,141.421)) && passed;

	[view1 setBoundsSize: NSMakeSize(100, 100)];
	passed = CHECK(view1, NSMakeRect(20,20,100,100),NSMakeRect(-50.7107,20,141.421,141.421)) && passed;

	[view1 setBoundsSize: NSMakeSize(10, 10)];
	passed = CHECK(view1, NSMakeRect(20,20,100,100),NSMakeRect(-5.07107,2,14.1421,14.1421)) && passed;
 
	[view1 setBoundsRotation: 0.0];
	passed = CHECK(view1, NSMakeRect(20,20,100,100),NSMakeRect(2.82843, 0, 10, 10)) && passed;
	
	[view1 setBoundsSize: NSMakeSize(1, 1)];
	passed = CHECK(view1, NSMakeRect(20,20,100,100),NSMakeRect(0.282843, 0, 1, 1)) && passed;
	
	[view1 setBoundsRotation: -45.0];
	[view1 setBounds: NSMakeRect(10, 10, 100, 100)];
	passed = CHECK(view1, NSMakeRect(20,20,100,100),NSMakeRect(-60.7107,10,141.421,141.421)) && passed;

	[view1 translateOriginToPoint: NSMakePoint(20, 20)];
	passed = CHECK(view1, NSMakeRect(20,20,100,100),NSMakeRect(-80.7107,-10,141.421,141.421)) && passed;

	[view1 scaleUnitSquareToSize: NSMakeSize(2, 3)];
	passed = CHECK(view1, NSMakeRect(20,20,100,100),NSMakeRect(-40.3553,-3.33333,70.7107,47.1405)) && passed;

	pass(passed,"NSView -frame and -bounds work");

	END_SET("NView GNUstep frame_bounds")

	DESTROY(arp);
	return 0;
}
