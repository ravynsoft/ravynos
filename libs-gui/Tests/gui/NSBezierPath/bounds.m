/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>
*/

#include "Testing.h"

#include <Foundation/NSAutoreleasePool.h>
#include <AppKit/NSBezierPath.h>

#include <math.h>

int main(int argc, char **argv)
{
	CREATE_AUTORELEASE_POOL(arp);
	NSBezierPath *p=[[NSBezierPath alloc] init];
	NSRect r;

	pass(NSIsEmptyRect([p bounds]),"empty path gives empty bounds");

	[p moveToPoint: NSMakePoint(100,100)];
	[p lineToPoint: NSMakePoint(150,150)];

	pass(NSEqualRects([p bounds],NSMakeRect(100,100,50,50)),"bounds accuracy (1)");
	pass(NSEqualRects([p controlPointBounds],NSMakeRect(100,100,50,50)),"control-point bounds accuracy (1)");

	[p removeAllPoints];
	pass(NSIsEmptyRect([p bounds]),"empty path gives empty bounds (2)");

	[p moveToPoint: NSMakePoint(100,100)];
	[p curveToPoint: NSMakePoint(200,100)
		controlPoint1: NSMakePoint(125,50)
		controlPoint2: NSMakePoint(175,150)];

	/* Basic checking Y. */
	r=[p bounds];
	if (fabs(r.origin.x    - 100.0000) > 0.001 ||
	    fabs(r.origin.y    -  85.5662) > 0.001 ||
	    fabs(r.size.width  - 100.0000) > 0.001 ||
	    fabs(r.size.height -  28.8678) > 0.001)
	{
		pass(0,"bounds accuracy (2)");
		printf("expected %s, got %s\n",
			[NSStringFromRect(NSMakeRect(100.0000, 85.5662, 100.0000, 28.8678)) lossyCString],
			[NSStringFromRect(r) lossyCString]);
	}
	else
		pass(1,"bounds accuracy (2)");

	pass(NSEqualRects([p controlPointBounds],NSMakeRect(100,50,100,100)),"control-point bounds accuracy (2)");

	/* Basic checking X. */
	[p removeAllPoints];
	[p moveToPoint: NSMakePoint(100,100)];
	[p curveToPoint: NSMakePoint(100,200)
		controlPoint1: NSMakePoint(50,125)
		controlPoint2: NSMakePoint(150,175)];

	r=[p bounds];
	if (fabs(r.origin.y    - 100.0000) > 0.001 ||
	    fabs(r.origin.x    -  85.5662) > 0.001 ||
	    fabs(r.size.height - 100.0000) > 0.001 ||
	    fabs(r.size.width  -  28.8678) > 0.001)
	{
		pass(0,"bounds accuracy (3)");
		printf("expected %s, got %s\n",
			[NSStringFromRect(NSMakeRect(85.5662, 100.0000, 28.8678, 100.0000)) lossyCString],
			[NSStringFromRect(r) lossyCString]);
	}
	else
		pass(1,"bounds accuracy (3)");


	/* A bit of both, and extreme values beyond the initial points. */
	[p removeAllPoints];
	[p moveToPoint: NSMakePoint(-100,0)];
	[p curveToPoint: NSMakePoint(100,0)
		controlPoint1: NSMakePoint(-118.2, 10.393)
		controlPoint2: NSMakePoint( 118.2,-10.393)];

	r=[p bounds];
	if (fabs(r.origin.x    + 101.0) > 0.001 ||
	    fabs(r.origin.y    +   3.0) > 0.001 ||
	    fabs(r.size.width  - 202.0) > 0.001 ||
	    fabs(r.size.height -   6.0) > 0.001)
	{
		pass(0,"bounds accuracy (4)");
		printf("expected %s, got %s\n",
			[NSStringFromRect(NSMakeRect(-101.0, -3.0, 202.0, 6.0)) lossyCString],
			[NSStringFromRect(r) lossyCString]);
	}
	else
		pass(1,"bounds accuracy (4)");


	/* Check the control-point bounding box. */
	r=[p controlPointBounds];
	if (fabs(r.origin.x    + 118.2  ) > 0.001 ||
	    fabs(r.origin.y    +  10.393) > 0.001 ||
	    fabs(r.size.width  - 236.4  ) > 0.001 ||
	    fabs(r.size.height -  20.786) > 0.001)
	{
		pass(0,"control-point bounds accuracy (3)");
		printf("expected %s, got %s\n",
			[NSStringFromRect(NSMakeRect(-118.2, -10.393, 236.4, 20.786)) lossyCString],
			[NSStringFromRect(r) lossyCString]);
	}
	else
		pass(1,"control-point bounds accuracy (3)");


	/*

	p=(1-t)^3*a + 3*(1-t)^2*t*b + 3*(1-t)*t^2*c + t^3*d

	    c-2b+a +- sqrt(a(d-c)+b(-d-c)+c^2+b^2)
	t=  --------------------------------------
	                  -d+3c-3b+a

	*/


	if (0)
	{
		NSPoint a,b,c,d;
		double t1,t2;
		double t,ti;
		NSPoint p;

		a=NSMakePoint(-100,0);
		b=NSMakePoint(-118.2,10.39);
		c=NSMakePoint(118.2,-10.39);
		d=NSMakePoint(100,0);

#define D \
		ti=1.0-t; \
		p.x=ti*ti*ti*a.x + 3*ti*ti*t*b.x + 3*ti*t*t*c.x + t*t*t*d.x; \
		p.y=ti*ti*ti*a.y + 3*ti*ti*t*b.y + 3*ti*t*t*c.y + t*t*t*d.y; \
		printf("  t=%15.7f  (%15.7f %15.7f)\n",t,p.x,p.y);

		t=0; D
		t=1; D
		t=0.5; D

		t1=(c.x-2*b.x+a.x + sqrt(a.x*(d.x-c.x)+b.x*(-d.x-c.x)+c.x*c.x+b.x*b.x)) / (-d.x+3*c.x-3*b.x+a.x);
		t2=(c.x-2*b.x+a.x - sqrt(a.x*(d.x-c.x)+b.x*(-d.x-c.x)+c.x*c.x+b.x*b.x)) / (-d.x+3*c.x-3*b.x+a.x);
		printf("x:\n");


		t=t1;
		D
		t=t2;
		D

		t1=(c.y-2*b.y+a.y + sqrt(a.y*(d.y-c.y)+b.y*(-d.y-c.y)+c.y*c.y+b.y*b.y)) / (-d.y+3*c.y-3*b.y+a.y);
		t2=(c.y-2*b.y+a.y - sqrt(a.y*(d.y-c.y)+b.y*(-d.y-c.y)+c.y*c.y+b.y*b.y)) / (-d.y+3*c.y-3*b.y+a.y);
		printf("y:\n");
		t=t1;
		D
		t=t2;
		D
	}

//	printf("bounds=%@\n",NSStringFromRect([p bounds]));

	DESTROY(arp);

	return 0;
}

