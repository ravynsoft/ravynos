/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>
*/
#include "Testing.h"

#include <math.h>

#include <Foundation/NSAutoreleasePool.h>
#include <AppKit/PSOperators.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSView.h>
#include <AppKit/NSWindow.h>

#if 0
NSView *view1,*view2;

@interface TestView1 : NSView
@end
@implementation TestView1
-(void) drawRect: (NSRect)dr
{
	NSRect r;
	PSsetrgbcolor(1,0,0);
	PSrectfill(0,0,100,100);

	PSsetrgbcolor(1,1,1);
	PSsetlinewidth(3.0);
//	printf("********* convert\n");
	r=[self convertRect: NSMakeRect(0,0,50,50) fromView: view2];
	printf("got rect (%g %g)+(%g %g)\n",r.origin.x,r.origin.y,r.size.width,r.size.height);
	PSrectstroke(r.origin.x,r.origin.y,r.size.width,r.size.height);
	PSsetrgbcolor(0,1,0);
	PSrectfill(r.origin.x-4,r.origin.y-4,8,8);
}
-(BOOL) isFlipped
{
	return YES;
}
@end

@interface TestView2 : NSView
@end
@implementation TestView2
-(void) drawRect: (NSRect)r
{
	PSsetrgbcolor(0,0,1);
	PSrectfill(0,0,50,50);
}
-(BOOL) isFlipped
{
	return YES;
}
@end
#endif


int check(NSView *from,NSView *to,NSRect rfrom,NSRect rto)
{
	NSRect r;

	r=[to convertRect: rfrom  fromView: from];
	if (fabs(r.origin.x - rto.origin.x)>0.001
	 || fabs(r.origin.y - rto.origin.y)>0.001
	 || fabs(r.size.width - rto.size.width)>0.001
	 || fabs(r.size.height - rto.size.height)>0.001)
	{
		printf("(1) expected (%g %g)+(%g %g) to convert to (%g %g)+(%g %g), got (%g %g)+(%g %g)\n",
			rfrom.origin.x,rfrom.origin.y,rfrom.size.width,rfrom.size.height,
			rto.origin.x,rto.origin.y,rto.size.width,rto.size.height,
			r.origin.x,r.origin.y,r.size.width,r.size.height);

		return 0;
	}

	r=[from convertRect: rfrom  toView: to];
	if (fabs(r.origin.x - rto.origin.x)>0.001
	 || fabs(r.origin.y - rto.origin.y)>0.001
	 || fabs(r.size.width - rto.size.width)>0.001
	 || fabs(r.size.height - rto.size.height)>0.001)
	{
		printf("(2) expected (%g %g)+(%g %g) to convert to (%g %g)+(%g %g), got (%g %g)+(%g %g)\n",
			rfrom.origin.x,rfrom.origin.y,rfrom.size.width,rfrom.size.height,
			rto.origin.x,rto.origin.y,rto.size.width,rto.size.height,
			r.origin.x,r.origin.y,r.size.width,r.size.height);

		return 0;
	}

	return 1;
}

int main(int argc, char **argv)
{
	CREATE_AUTORELEASE_POOL(arp);

	NSWindow *window;
	NSView *view1,*view2;
	int passed=1;

	START_SET("NView GNUstep converRect")

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

	window=[[NSWindow alloc] initWithContentRect: NSMakeRect(100,100,200,200)
		styleMask: NSClosableWindowMask
		backing: NSBackingStoreRetained
		defer: YES];
	view1=[[NSView alloc] initWithFrame: NSMakeRect(20,20,100,100)];
	view2=[[NSView alloc] initWithFrame: NSMakeRect(25,25,50,50)];

	[view1 addSubview: view2];
	[[window contentView] addSubview: view1];

	passed=check(view2,view1,NSMakeRect(0,0,10,10),NSMakeRect(25,25,10,10)) && passed;
	passed=check(view1,view2,NSMakeRect(25,25,10,10),NSMakeRect(0,0,10,10)) && passed;

	[view2 setFrameRotation: 45.0];
	passed=check(view2,view1,NSMakeRect(0,0,10,10),NSMakeRect(17.9289,25,14.1421,14.1421)) && passed;

	[view2 setFrameRotation: 0.0];
	passed=check(view2,view1,NSMakeRect(0,0,10,10),NSMakeRect(25,25,10,10)) && passed;
	passed=check(view1,view2,NSMakeRect(25,25,10,10),NSMakeRect(0,0,10,10)) && passed;

	[view1 setFrameRotation: 45.0];
	[view2 setFrameRotation: 45.0];
	passed=check(view2,view1,NSMakeRect(0,0,10,10),NSMakeRect(17.9289,25,14.1421,14.1421)) && passed;

	[view2 setFrameRotation: -45.0];
	passed=check(view2,view1,NSMakeRect(0,0,10,10),NSMakeRect(25,17.9289,14.1421,14.1421)) && passed;

	passed=check(view2,[window contentView],NSMakeRect(0,0,10,10),NSMakeRect(20,55.3553,10,10)) && passed;
	passed=check([window contentView],view2,NSMakeRect(20,55.3553,10,10),NSMakeRect(0,0,10,10)) && passed;

	pass(passed,"NSView -convertRect:fromView: and -convertRect:toView: work");

	END_SET("NView GNUstep converRect")

	DESTROY(arp);
	return 0;
}

