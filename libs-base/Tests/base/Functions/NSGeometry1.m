/* NSGeometry tests */
#import <Foundation/Foundation.h>
#import "Testing.h"

static BOOL	MacOSXCompatibleGeometry()
{
  NSUserDefaults *dflt = [NSUserDefaults standardUserDefaults];
  if ([dflt boolForKey: @"GSOldStyleGeometry"] == YES)
    return NO;
  return [dflt boolForKey: @"GSMacOSXCompatible"];
}

/* Test the string functions */
int
geom_string()
{
#if     defined(GNUSTEP_BASE_LIBRARY)
  NSUserDefaults *dflt = [NSUserDefaults standardUserDefaults];
  BOOL compat_mode = MacOSXCompatibleGeometry();
#endif
  NSPoint p, p2;
  NSRect r, r2;
  NSSize s, s2;
  NSEdgeInsets ei;
  NSString *sp, *sr, *ss;
  
  p = NSMakePoint(23.45, -3.45);
  r = NSMakeRect(23.45, -3.45, 2044.3, 2033);
  s = NSMakeSize(0.5, 0.22);
  ei = NSEdgeInsetsMake(23.45, -3.45, 2044.3, 2033);

  PASS(NSEqualPoints(p, NSMakePoint(23.45, -3.45)),
    "identical points are equal");
  if (sizeof(CGFloat) == sizeof(float))
    {
      PASS(NSEqualPoints(p, NSMakePoint(23.450001, -3.45)),
        "near identical points are equal");
    }
  else
    {
      PASS(NSEqualPoints(p, NSMakePoint(23.450000000000001, -3.45)),
        "near identical points are equal");
    }
  PASS(!NSEqualPoints(p, NSMakePoint(23.4500019, -3.45)),
    "moderately similar points are not equal");
  PASS(NSEqualSizes(s, NSMakeSize(0.5, 0.22)),
    "identical sizes are equal");
  if (sizeof(CGFloat) == sizeof(float))
    {
      PASS(NSEqualSizes(s, NSMakeSize(0.50000001, 0.22)),
       "near identical sizes are equal");
    }
  else
    {
      PASS(NSEqualSizes(s, NSMakeSize(0.50000000000000001, 0.22)),
       "near identical sizes are equal");
    }
  PASS(!NSEqualSizes(s, NSMakeSize(0.50000003, 0.22)),
    "moderately similar sizes are not equal");
  PASS(NSEqualRects(r, NSMakeRect(23.45, -3.45, 2044.3, 2033)),
    "identical rects are equal");
  if (sizeof(CGFloat) == sizeof(float))
    {
      PASS(NSEqualRects(r, NSMakeRect(23.45, -3.45, 2044.3, 2033.00001)),
        "near identical rects are equal");
    }
  else
    {
      PASS(NSEqualRects(r, NSMakeRect(23.45, -3.45, 2044.3, 2033.0000000000001)),
        "near identical rects are equal");
    }
  PASS(!NSEqualRects(r, NSMakeRect(23.45, -3.45, 2044.3, 2033.0001)),
    "moderately similar rects are not equal");

  PASS(NSIntersectsRect(r, NSMakeRect(23.45, -3.45, 2044.3, 2033)),
    "identical rects intersect");

  PASS(!NSIntersectsRect(NSMakeRect(1,1,2,2), NSMakeRect(2,2,0,0)),
    "an empty rect does not intersect with one containing it");
  PASS(!NSIntersectsRect(NSMakeRect(1,1,2,2), NSMakeRect(3,3,0,0)),
    "an empty rect does not intersect with one touching it");
  PASS(!NSIntersectsRect(NSMakeRect(1,1,0,0), NSMakeRect(1,1,0,0)),
    "identical empty rects do not intersec");
  
  PASS(NSEdgeInsetsEqual(ei, NSEdgeInsetsMake(23.45, -3.45, 2044.3, 2033)),
    "identical rects are equal");
  if (sizeof(CGFloat) == sizeof(float))
    {
      PASS(NSEdgeInsetsEqual(ei, NSEdgeInsetsMake(23.45, -3.45, 2044.3, 2033.00001)),
        "near identical rects are equal");
    }
  else
    {
      PASS(NSEdgeInsetsEqual(ei, NSEdgeInsetsMake(23.45, -3.45, 2044.3, 2033.0000000000001)),
        "near identical rects are equal");
    }
  PASS(!NSEdgeInsetsEqual(ei, NSEdgeInsetsMake(23.45, -3.45, 2044.3, 2033.0001)),
    "moderately similar rects are not equal");

#if     defined(GNUSTEP_BASE_LIBRARY)
  if (compat_mode == YES)
    {
      [dflt setBool: NO forKey: @"GSMacOSXCompatible"];
      [NSUserDefaults resetStandardUserDefaults];
    }
  PASS((MacOSXCompatibleGeometry() == NO), 
       "Not in MacOSX geometry compat mode");

  sp = NSStringFromPoint(p);
  p2 = NSPointFromString(sp);
  PASS((EQ(p2.x, p.x) && EQ(p2.y, p.y)), 
       "Can read output of NSStringFromPoint");

  sr = NSStringFromRect(r);
  r2 = NSRectFromString(sr);
  PASS((EQ(r2.origin.x, r.origin.x) && EQ(r2.origin.y, r.origin.y)
    && EQ(r2.size.width, r.size.width) && EQ(r2.size.height, r.size.height)), 
       "Can read output of NSStringFromRect");

  ss = NSStringFromSize(s);
  s2 = NSSizeFromString(ss);
  PASS((EQ(s2.width, s.width) && EQ(s2.height, s.height)), 
       "Can read output of NSStringFromSize");

  if ([[NSFileManager defaultManager] isWritableFileAtPath: NSHomeDirectory()])
    {
      dflt = [NSUserDefaults standardUserDefaults];
      [dflt setBool: YES forKey: @"GSMacOSXCompatible"];
      [NSUserDefaults resetStandardUserDefaults];
      PASS((MacOSXCompatibleGeometry() == YES),
	   "In MacOSX geometry compat mode");
    }
#endif

  sp = NSStringFromPoint(p);
  p2 = NSPointFromString(sp);
  PASS((EQ(p2.x, p.x) && EQ(p2.y, p.y)), 
       "Can read output of NSStringFromPoint (MacOSX compat)");

  sr = NSStringFromRect(r);
  r2 = NSRectFromString(sr);
  PASS((EQ(r2.origin.x, r.origin.x) && EQ(r2.origin.y, r.origin.y)
    && EQ(r2.size.width, r.size.width) && EQ(r2.size.height, r.size.height)), 
       "Can read output of NSStringFromRect (MacOSX compat)");

  ss = NSStringFromSize(s);
  s2 = NSSizeFromString(ss);
  PASS((EQ(s2.width, s.width) && EQ(s2.height, s.height)), 
       "Can read output of NSStringFromSize (MacOSX compat)");

#if     defined(GNUSTEP_BASE_LIBRARY)
  if (compat_mode != MacOSXCompatibleGeometry())
    {
      [dflt setBool: NO forKey: @"GSMacOSXCompatible"];
    }
#endif
  return 0;
}

int main()
{ 
  NSAutoreleasePool   *pool = [NSAutoreleasePool new];

  geom_string();
  
  [pool release]; pool = nil;
 
  return 0;
}
