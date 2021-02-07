/*
copyright 2011 HNS <hns@goldelico.com>
Modified by Fred Kiefer on 18.07.11.
*/
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

int CHECK_BOUNDS_ROTATION(NSView *view, CGFloat rot)
{
  if (fabs([view boundsRotation] - rot) > 0.001)
  {
    printf("expected bounds rotation %g got %g\n", rot, [view boundsRotation]);
    return 0;
  }

  return 1;
}

int CHECK_MATRIX(NSView *view, CGFloat *ts)
{
  NSView *superView = [view superview];
  CGFloat tsm[6];
  NSPoint res;
  
  res = [view convertPoint: NSMakePoint(0, 0) toView: superView];
  tsm[4] = res.x;
  tsm[5] = res.y;
  res = [view convertPoint: NSMakePoint(1, 0) toView: superView];
  tsm[0] = res.x - tsm[4];
  tsm[1] = res.y - tsm[5];
  res = [view convertPoint: NSMakePoint(0, 1) toView: superView];
  tsm[2] = res.x - tsm[4];
  tsm[3] = res.y - tsm[5];
  if (fabs(ts[0] - tsm[0]) > 0.001
    || fabs(ts[1] - tsm[1]) > 0.001
    || fabs(ts[2] - tsm[2]) > 0.001
    || fabs(ts[3] - tsm[3]) > 0.001
    || fabs(ts[4] - tsm[4]) > 0.001
      || fabs(ts[5] - tsm[5]) > 0.001)
    {
      printf("expected bounds matrix (%g %g %g %g %g %g) got (%g %g %g %g %g %g)\n", ts[0], ts[1], ts[2], ts[3], ts[4], ts[5],
             tsm[0], tsm[1], tsm[2], tsm[3], tsm[4], tsm[5]);
      return 0;
    }
  
  return 1;
}

int main(int argc, char **argv)
{
  CREATE_AUTORELEASE_POOL(arp);
  
  NSWindow *window;
  NSView *view1;
  CGFloat ts[6];
  int passed = 1;

  START_SET("NView GNUstep bounds_scale")

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
  
  [view1 setBounds: NSMakeRect(30.4657, 88.5895, 21.2439, 60.8716)];
  passed = CHECK(view1, NSMakeRect(20,20,100,100), NSMakeRect(30.4657, 88.5895, 21.2439, 60.8716)) && passed;
  passed = CHECK_BOUNDS_ROTATION(view1, 0) && passed;
  ts[0] = 4.70723;
  ts[1] = 0;
  ts[2] = 0;
  ts[3] = 1.64281;
  ts[4] = -123.409;
  ts[5] = -125.535;
  passed = CHECK_MATRIX(view1, ts) && passed;
  [view1 setBoundsRotation: 30];
  passed = CHECK(view1, NSMakeRect(20,20,100,100), NSMakeRect(70.6788, 50.866, 48.8336, 63.3383)) && passed;
  passed = CHECK_BOUNDS_ROTATION(view1, 30) && passed;
  ts[0] = 4.07658;
  ts[1] = 0.821396;
  ts[2] = -2.35362;
  ts[3] = 1.42271;
  ts[4] = -123.409;
  ts[5] = -125.535;
  passed = CHECK_MATRIX(view1, ts) && passed;
  [view1 setBounds:(NSRect){{30.4657, 88.5895}, {21.2439, 60.8716}}];
  passed = CHECK(view1, NSMakeRect(20,20,100,100),(NSRect) {{30.4657, 77.9676}, {48.8336, 63.3383}}) && passed;
  passed = CHECK_BOUNDS_ROTATION(view1, 30) && passed;
  ts[0] = 4.07658;
  ts[1] = 0.821396;
  ts[2] = -2.35361;
  ts[3] = 1.42271;
  ts[4] = 104.31;
  ts[5] = -131.062;
  passed = CHECK_MATRIX(view1, ts) && passed;
  [view1 scaleUnitSquareToSize:(NSSize){0.720733, 0.747573}];
  passed = CHECK(view1, NSMakeRect(20,20,100,100),(NSRect) {{42.2704, 104.294}, {67.7554, 84.7253}}) && passed;
  passed = CHECK_BOUNDS_ROTATION(view1, 30) && passed;
  ts[0] = 2.93813;
  ts[1] = 0.59201;
  ts[2] = -1.7595;
  ts[3] = 1.06358;
  ts[4] = 104.31;
  ts[5] = -131.062;
  passed = CHECK_MATRIX(view1, ts) && passed;
  [view1 setBoundsRotation:30-1e-6];
  passed = (fabs([view1 boundsRotation] - 30.0 + 1e-6) <= 0.001) && passed;
  passed = CHECK(view1, NSMakeRect(20,20,100,100),(NSRect) {{39.9801, 104.211}, {66.2393, 85.2544}}) && passed;
  passed = CHECK_BOUNDS_ROTATION(view1, 30 - 1e-6) && passed;
  ts[0] = 2.93813;
  ts[1] = 0.614059;
  ts[2] = -1.69633;
  ts[3] = 1.06358;
  ts[4] = 104.31;
  ts[5] = -131.062;
  passed = CHECK_MATRIX(view1, ts) && passed;
  [view1 rotateByAngle:1e-6];
  passed = CHECK(view1, NSMakeRect(20,20,100,100),(NSRect) {{39.9801, 104.211}, {66.2393, 85.2544}}) && passed;
  passed = CHECK_BOUNDS_ROTATION(view1, 30) && passed;



  view1 = [[NSView alloc] initWithFrame: NSMakeRect(20,20,100,100)];
  [[window contentView] addSubview: view1];
  
  [view1 setBounds: NSMakeRect(30.4657, 88.5895, 21.2439, 60.8716)];
  passed = CHECK(view1, NSMakeRect(20,20,100,100), NSMakeRect(30.4657, 88.5895, 21.2439, 60.8716)) && passed;
  [view1 scaleUnitSquareToSize:(NSSize){0.720733, 0.747573}];
  passed = CHECK(view1, NSMakeRect(20,20,100,100),(NSRect) {{42.2704, 118.503}, {29.4754, 81.4256}}) && passed;
  passed = CHECK_BOUNDS_ROTATION(view1, 0) && passed;
  [view1 setBoundsRotation:30];
  passed = CHECK(view1, NSMakeRect(20,20,100,100),(NSRect) {{95.8587, 66.7535}, {66.2393, 85.2544}}) && passed;
  passed = CHECK_BOUNDS_ROTATION(view1, 30) && passed;



  view1 = [[NSView alloc] initWithFrame: NSMakeRect(20,20,100,100)];
  [[window contentView] addSubview: view1];
  
  [view1 setBounds: NSMakeRect(30.4657, 88.5895, 21.2439, 60.8716)];
  passed = CHECK(view1, NSMakeRect(20,20,100,100), NSMakeRect(30.4657, 88.5895, 21.2439, 60.8716)) && passed;
  [view1 setBoundsRotation:30];
  passed = CHECK(view1, NSMakeRect(20,20,100,100),(NSRect) {{70.6788, 50.866}, {48.8336, 63.3383}}) && passed;
  passed = CHECK_BOUNDS_ROTATION(view1, 30) && passed;
  [view1 scaleUnitSquareToSize:(NSSize){0.720733, 0.747573}];
  passed = CHECK(view1, NSMakeRect(20,20,100,100),(NSRect) {{98.0652, 68.0415}, {67.7554, 84.7252}}) && passed;
  passed = CHECK_BOUNDS_ROTATION(view1, 30) && passed;
  
  testHopeful = YES;
  pass(passed,"NSView -scaleUnitSquareToSize works");
  testHopeful = NO;

  END_SET("NView GNUstep bounds_scale")

  DESTROY(arp);
  return 0;
}

