/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>
*/

#include "Testing.h"

#include <Foundation/NSAutoreleasePool.h>
#include <AppKit/NSBezierPath.h>

#include <math.h>


#define nextafterf(x,y) next(x,y)

static float next(float x,float y)
{
	if (y<x)
		return x-(x?0.00001:0.000005);
//		return x-(x?0.0001:0.00001);
	else if (y>x)
		return x+(x?0.00001:0.000005);
//		return x+(x?0.0001:0.00001);
	else
		return y;
}


//#define DRAW

#ifdef DRAW

#include <AppKit/NSApplication.h>
#include <AppKit/NSGraphics.h>
#include <AppKit/NSView.h>
#include <AppKit/NSWindow.h>
#include <AppKit/PSOperators.h>

@interface MyView : NSView
-(void) clear;
-(void) drawPath: (NSBezierPath *)p;
-(void) drawPoint: (NSPoint)p count: (int)c;
-(void) pause;
@end

#define MOVE \
	{ \
		double x0,y0,x1,y1; \
		double w=800,h=800; \
		\
		x0=76.980; \
		x1=76.981; \
		y0=-0.0005; \
		y1=0.0005; \
		\
		ww=w/(x1-x0); \
		hh=h/(y1-y0); \
		PSscale(ww,hh); \
		PStranslate(-x0,-y0); \
	}

@implementation MyView
-(void) clear
{
	[self lockFocus];
	PSsetrgbcolor(1,1,1);
	NSRectFill([self bounds]);
	[self unlockFocus];
	[[self window] flushWindow];
}
-(void) drawPath: (NSBezierPath *)p
{
	double ww,hh;
	[self lockFocus];
	MOVE
	PSsetrgbcolor(0.8,0.4,0.4);
	[p fill];
	PSsetrgbcolor(0.3,1.0,0.3);
	PSsetalpha(0.5);
	[[p bezierPathByFlatteningPath] fill];
	[self unlockFocus];
	[[self window] flushWindow];
}
-(void) drawPoint: (NSPoint)p count: (int)c
{
#define NC 3
static float colors[NC][3]={
{1,0,0},
{0,1,0},
{0,0,1},
};
	double ww,hh;
	c%=NC;
	if (c<0)
		c+=NC;
	[self lockFocus];
	MOVE
	PSsetrgbcolor(colors[c][0],colors[c][1],colors[c][2]);
	PSrectfill(p.x-4/ww,p.y-4/hh,8/ww,8/hh);
	[self unlockFocus];
	[[self window] flushWindow];
#undef NC
}
-(void) pause
{
	char buf[128];
	gets(buf);
}
@end

MyView *view;

#endif



int main(int argc, char **argv)
{
	CREATE_AUTORELEASE_POOL(arp);
	NSBezierPath *p=[[NSBezierPath alloc] init];
	int i;
	const char *str;
	int X=-1000;


#ifdef DRAW
	{
		NSWindow *w;

		[NSApplication sharedApplication];
		w=[[NSWindow alloc] initWithContentRect: NSMakeRect(50,50,800,800)
			styleMask: NSTitledWindowMask
			backing: NSBackingStoreRetained
			defer: YES];

		view=[[MyView alloc] init];
		[w setContentView: view];
		[w orderFront: nil];
	}
#endif

#ifdef DRAW
#define DP(e,x,y) [view drawPoint: NSMakePoint(x,y)  count: e];
#define CLEAR [view clear];
#define PAUSE [view pause];
#define DRAWPATH [view drawPath: p];
#else
#define DP(e,x,y)
#define CLEAR
#define PAUSE
#define DRAWPATH
#endif

#define T(e,x,y) \
	{ \
		int i,r; \
		DP(e,x,y) \
		r=[p windingCountAtPoint: NSMakePoint(x,y)]; \
		for (i=5;i;i--) \
		{ \
			if ([p windingCountAtPoint: NSMakePoint(x,y)]!=r) \
				break; \
		} \
		if (i) \
		{ \
			pass(NO, \
				"path '%s', %15.8e %15.8e, expected %i, got inconsistant results", \
				str,(double)x,(double)y, \
				e); \
		} \
		else \
		{ \
			pass(r == e, \
				"path '%s', %15.8e %15.8e, expected %i, got %i", \
				str,(double)x,(double)y, \
				e,r); \
		} \
	}

#define CHECK_AROUND(x,y, p00,p10,p20, p01,p11,p21, p02,p12,p22) \
	if (p00!=X) T(p00,nextafterf(x,-1000),nextafterf(y, 1000)) \
	if (p10!=X) T(p10,           x       ,nextafterf(y, 1000)) \
	if (p20!=X) T(p20,nextafterf(x, 1000),nextafterf(y, 1000)) \
	\
	if (p01!=X) T(p01,nextafterf(x,-1000),           y       ) \
	if (p11!=X) T(p11,           x       ,           y       ) \
	if (p21!=X) T(p21,nextafterf(x, 1000),           y       ) \
	\
	if (p02!=X) T(p02,nextafterf(x,-1000),nextafterf(y,-1000)) \
	if (p12!=X) T(p12,           x       ,nextafterf(y,-1000)) \
	if (p22!=X) T(p22,nextafterf(x, 1000),nextafterf(y,-1000))

#if 1
	str="empty";
	T(0,0,0)


	for (i=0;i<3;i++)
	{
		[p removeAllPoints];
		[p moveToPoint: NSMakePoint(100,100)];
		[p lineToPoint: NSMakePoint(100,200)];
		[p lineToPoint: NSMakePoint(200,200)];
		[p lineToPoint: NSMakePoint(200,100)];

		if (i==0)
		{
			str="(u) rect";
		}
		else if (i==1)
		{
			[p closePath];
			str="(c) rect";
		}
		else
		{
			[p lineToPoint: NSMakePoint(100,100)];
			[p closePath];
			str="(d) rect";
		}

		/* Obvious stuff. */
		T(0,0,0)
		T(1,150,150)

		/* Check around each corner. */
		CHECK_AROUND(100,100,
			0,X,1,
			0,X,X,
			0,0,0)

		CHECK_AROUND(200,100,
			1,X,0,
			X,X,0,
			0,0,0)

		CHECK_AROUND(200,200,
			0,0,0,
			X,X,0,
			1,X,0)

		CHECK_AROUND(100,200,
			0,0,0,
			0,X,X,
			0,X,1)

		if (!i)
			[p closePath];
	}


	for (i=0;i<3;i++)
	{
		[p removeAllPoints];
		[p moveToPoint: NSMakePoint(0,-100)];
		[p lineToPoint: NSMakePoint(-100,0)];
		[p lineToPoint: NSMakePoint(0,100)];
		[p lineToPoint: NSMakePoint(100,0)];

		if (i==0)
		{
			str="(u) tilted rect";
		}
		else if (i==1)
		{
			[p closePath];
			str="(c) tilted rect";
		}
		else
		{
			[p lineToPoint: NSMakePoint(0,-100)];
			[p closePath];
			str="(d) tilted rect";
		}

		/* Obvious stuff. */
		T(1,0,0)
		T(0,200,200)

		/* Check around each corner. */
		CHECK_AROUND(0,-100,
			1,1,1,
			0,X,0,
			0,0,0)

		CHECK_AROUND(0,100,
			0,0,0,
			0,X,0,
			1,1,1)

		CHECK_AROUND(-100,0,
			0,0,1,
			0,X,1,
			0,0,1)

		CHECK_AROUND(100,0,
			1,0,0,
			1,X,0,
			1,0,0)

		/* Check some points on the edges. */
		CHECK_AROUND(50,50,
			X,0,0,
			1,X,0,
			1,1,X)

		CHECK_AROUND(-50,50,
			0,0,X,
			0,X,1,
			X,1,1)

		CHECK_AROUND(-50,-50,
			X,1,1,
			0,X,1,
			0,0,X)

		CHECK_AROUND(50,-50,
			1,1,X,
			1,X,0,
			X,0,0)
	}

	for (i=0;i<3;i++)
	{
		[p removeAllPoints];
		[p moveToPoint: NSMakePoint(200,200)];
		[p lineToPoint: NSMakePoint(200,100)];
		[p lineToPoint: NSMakePoint(100,100)];
		[p lineToPoint: NSMakePoint(100,200)];
		if (i==2)
			[p lineToPoint: NSMakePoint(200,200)];
		if (i>=1)
			[p closePath];

		[p moveToPoint: NSMakePoint(200,200)];
		[p lineToPoint: NSMakePoint(300,200)];
		[p lineToPoint: NSMakePoint(300,100)];
		[p lineToPoint: NSMakePoint(200,100)];
		if (i==2)
			[p lineToPoint: NSMakePoint(200,200)];
		if (i>=1)
			[p closePath];

		if (i==0)
			str="(u) touching rects";
		else if (i==1)
			str="(c) touching rects";
		else
			str="(d) touching rects";

		/* Obvious stuff. */
		T(0,0,0)
		T(1,150,150)
		T(1,250,150)

		/* Check around the touching corners and edge. */
		CHECK_AROUND(200,200,
			0,0,0,
			X,X,X,
			1,1,1)

		CHECK_AROUND(200,100,
			1,1,1,
			X,X,X,
			0,0,0)

		CHECK_AROUND(200,150,
			1,1,1,
			1,1,1,
			1,1,1)
	}

	for (i=0;i<3;i++)
	{
		[p removeAllPoints];
		[p moveToPoint: NSMakePoint(-100,200)];
		[p lineToPoint: NSMakePoint(100,200)];
		[p lineToPoint: NSMakePoint(100,-200)];
		[p lineToPoint: NSMakePoint(-100,-200)];
		if (i==2)
			[p lineToPoint: NSMakePoint(-100,200)];
		if (i>=1)
			[p closePath];

		[p moveToPoint: NSMakePoint(200,100)];
		[p lineToPoint: NSMakePoint(200,-100)];
		[p lineToPoint: NSMakePoint(-200,-100)];
		[p lineToPoint: NSMakePoint(-200,100)];
		if (i==2)
			[p lineToPoint: NSMakePoint(200,100)];
		if (i>=1)
			[p closePath];

		if (i==0)
			str="(u) intersecting rects";
		else if (i==1)
			str="(c) intersecting rects";
		else
			str="(d) intersecting rects";

		/* Obvious stuff. */
		T(2,0,0)
		T(0,200,200)
		T(1,150,0)
		T(1,0,150)
	}

	for (i=0;i<3;i++)
	{
		[p removeAllPoints];
		[p moveToPoint: NSMakePoint(200,0)];
		[p lineToPoint: NSMakePoint(100,-100)];
		[p lineToPoint: NSMakePoint(100,100)];
		if (i==2)
			[p lineToPoint: NSMakePoint(200,0)];
		if (i>=1)
			[p closePath];

		[p moveToPoint: NSMakePoint(200,0)];
		[p lineToPoint: NSMakePoint(300,100)];
		[p lineToPoint: NSMakePoint(300,-100)];
		if (i==2)
			[p lineToPoint: NSMakePoint(200,0)];
		if (i>=1)
			[p closePath];

		if (i==0)
			str="(u) touching triangles";
		else if (i==1)
			str="(c) touching triangles";
		else
			str="(d) touching triangles";

		/* Obvious stuff. */
		T(0,0,0)
		T(1,150,0)
		T(1,250,0)

		CHECK_AROUND(200,0,
			1,0,1,
			1,X,1,
			1,0,1)
	}

	for (i=0;i<3;i++)
	{
		CLEAR
		[p removeAllPoints];
		[p moveToPoint: NSMakePoint(100,100)];
		[p lineToPoint: NSMakePoint(300,100)];
		[p lineToPoint: NSMakePoint(100,-100)];
		[p lineToPoint: NSMakePoint(300,-100)];
		if (i==2)
			[p lineToPoint: NSMakePoint(100,100)];
		if (i>=1)
			[p closePath];
//		[view drawPath: p];

		if (i==0)
			str="(u) self-intersection";
		else if (i==1)
			str="(c) self-intersection";
		else
			str="(d) self-intersection";

		/* Obvious stuff. */
		T(0,0,0)
		T(1,200,50)
		T(-1,200,-50)

		CHECK_AROUND(200,0,
			0,1,0,
			0,X,0,
			0,-1,0)
//		[view pause];
	}

	for (i=0;i<3;i++)
	{
		[p removeAllPoints];
		[p moveToPoint: NSMakePoint(-100,200)];
		[p lineToPoint: NSMakePoint(100,200)];
		[p lineToPoint: NSMakePoint(100,-200)];
		[p lineToPoint: NSMakePoint(-100,-200)];
		if (i==2)
			[p lineToPoint: NSMakePoint(-100,200)];
		if (i>=1)
			[p closePath];

		[p moveToPoint: NSMakePoint(200,100)];
		[p lineToPoint: NSMakePoint(200,-100)];
		[p lineToPoint: NSMakePoint(-200,-100)];
		[p lineToPoint: NSMakePoint(-200,100)];
		if (i==2)
			[p lineToPoint: NSMakePoint(200,100)];
		if (i>=1)
			[p closePath];

		if (i==0)
			str="(u) intersecting rects";
		else if (i==1)
			str="(c) intersecting rects";
		else
			str="(d) intersecting rects";

		/* Obvious stuff. */
		T(2,0,0)
		T(0,200,200)
		T(1,150,0)
		T(1,0,150)

		/* Intersection corners. */
		CHECK_AROUND(100,100,
			1,X,0,
			X,X,X,
			2,X,1)

		CHECK_AROUND(100,-100,
			2,X,1,
			X,X,X,
			1,X,0)

		CHECK_AROUND(-100,-100,
			1,X,2,
			X,X,X,
			0,X,1)

		CHECK_AROUND(-100,100,
			0,X,1,
			X,X,X,
			1,X,2)
	}

	for (i=0;i<3;i++)
	{
//		CLEAR
		[p removeAllPoints];
		[p moveToPoint: NSMakePoint(100,100)];
		[p curveToPoint: NSMakePoint(100,-100)
			controlPoint1: NSMakePoint(200,100)
			controlPoint2: NSMakePoint(200,-100)];
		if (i==2)
			[p lineToPoint: NSMakePoint(100,100)];
		if (i>=1)
			[p closePath];


//		DRAWPATH

		if (i==0)
			str="(u) curve 1";
		else if (i==1)
			str="(c) curve 1";
		else
			str="(d) curve 1";

		/* Obvious stuff. */
		T(0,0,0)
		T(0,210,0)
		T(0,190,0)
		T(1,110,0)

		/* "Extreme" point is at 175, 0.  This is at the half-way point, so
		   any tesselation by the standard method should get it right.  */
		CHECK_AROUND(175, 0,
			1,0,0,
			1,X,0,
			1,0,0)

//		PAUSE
	}

	for (i=0;i<3;i++)
	{
		CLEAR
		[p removeAllPoints];
		[p moveToPoint: NSMakePoint(-100,100)];
		[p curveToPoint: NSMakePoint(100,100)
			controlPoint1: NSMakePoint(-100,-100)
			controlPoint2: NSMakePoint(100,-100)];
		[p lineToPoint: NSMakePoint(100,-100)];
		[p curveToPoint: NSMakePoint(-100,-100)
			controlPoint1: NSMakePoint(100,100)
			controlPoint2: NSMakePoint(-100,100)];
		if (i==2)
			[p lineToPoint: NSMakePoint(-100,100)];
		if (i>=1)
			[p closePath];


		DRAWPATH

		if (i==0)
			str="(u) curve 2";
		else if (i==1)
			str="(c) curve 2";
		else
			str="(d) curve 2";

		/* Obvious stuff. */
		T(-1,0,0)
		T(1,-90,0)
		T(1, 90,0)

		/* The two curves intersect at y=0, x= +- 400 * sqrt(3) / 9.  */

		CHECK_AROUND(-400*sqrt(3)/9, 0,
			1,0,-1,
			1,X,-1,
			1,0,-1)

		CHECK_AROUND( 400*sqrt(3)/9, 0,
			-1,0,1,
			-1,X,1,
			-1,0,1)

		PAUSE
	}

	for (i=0;i<3;i++)
	{
		CLEAR
		[p removeAllPoints];
		[p moveToPoint: NSMakePoint(100,100)];
		[p curveToPoint: NSMakePoint(100,100)
			controlPoint1: NSMakePoint(100,200)
			controlPoint2: NSMakePoint(200,100)];
		if (i==2)
			[p lineToPoint: NSMakePoint(100,100)];
		if (i>=1)
			[p closePath];


		DRAWPATH

		if (i==0)
			str="(u) curve 3";
		else if (i==1)
			str="(c) curve 3";
		else
			str="(d) curve 3";

		/* Obvious stuff. */
		T(0,0,0)
		T(1,105,105)

		CHECK_AROUND(100,100,
			0,0,1,
			0,X,0,
			0,0,0)

		PAUSE
	}
#endif

	for (i=0;i<3;i++)
	{
		CLEAR

		/*

		  +-+
		  | |
		+-* |
		|   |
		+---+

		*/

		[p removeAllPoints];
		[p moveToPoint: NSMakePoint(0,0)];
		[p lineToPoint: NSMakePoint(0,100)];
		[p lineToPoint: NSMakePoint(100,100)];
		[p lineToPoint: NSMakePoint(100,-100)];
		[p lineToPoint: NSMakePoint(-100,-100)];
		[p lineToPoint: NSMakePoint(-100,0)];
		if (i==2)
			[p lineToPoint: NSMakePoint(0,0)];
		if (i>=1)
			[p closePath];


		DRAWPATH

		if (i==0)
			str="(u) curve 3";
		else if (i==1)
			str="(c) curve 3";
		else
			str="(d) curve 3";

		/* Obvious stuff. */
		T(0,-5,5)
		T(1,5,-5)

		CHECK_AROUND(0,0,
			0,X,1,
			X,X,1,
			1,1,1)

		PAUSE
	}

	DESTROY(arp);

	return 0;
}

