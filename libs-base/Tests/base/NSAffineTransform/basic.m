#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSAffineTransform.h>

#include <math.h>
static BOOL eq(double d1, double d2)
{
  if (abs(d1 - d2) < 0.000001)
    return YES;
  return NO;
}

static BOOL
is_equal_struct(NSAffineTransformStruct as, NSAffineTransformStruct bs)
{
  if (eq(as.m11, bs.m11) && eq(as.m12, bs.m12) && eq(as.m21, bs.m21)
    && eq(as.m22, bs.m22) && eq(as.tX, bs.tX) && eq(as.tY, bs.tY))
    return YES;
  return NO;
}

#if 0
static void
print_matrix (const char *str, NSAffineTransformStruct MM)
{
  printf("%s = %f %f %f %f  %f %f\n", str, MM.m11, MM.m12,
  	MM.m21, MM.m22, MM.tX, MM.tY);
}
#endif

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSAffineTransform *testObj;
  NSAffineTransformStruct flip = {1.0,0.0,0.0,-1.0,0.0,0.0};
  NSMutableArray *testObjs = [NSMutableArray new];
  NSAffineTransform *aa, *bb, *cc;
  NSAffineTransformStruct as = {2, 3, 4, 5, 10, 20};
  NSAffineTransformStruct bs = {6, 7, 8, 9, 14, 15};
  NSAffineTransformStruct cs;
  NSAffineTransformStruct answer1 = 
    {36.000000, 41.000000, 64.000000, 73.000000,  234.000000, 265.000000};
  NSAffineTransformStruct answer2 = 
    {40.000000, 53.000000, 52.000000, 69.000000,  98.000000, 137.000000};
  NSAffineTransformStruct answer3 = 
    {6.000000, 9.000000, 8.000000, 10.000000,  10.000000, 20.000000};
  NSAffineTransformStruct answer4 = 
    {6.000000, 9.000000, 8.000000, 10.000000,  194.000000, 268.000000};
  NSAffineTransformStruct answer5 = 
    {2.172574, 3.215242, 3.908954, 4.864383,  10.000000, 20.000000};
  NSAffineTransformStruct answer6 = 
    {2.172574, 3.215242, 3.908954, 4.864383,  90.796249, 126.684265};
  NSAffineTransformStruct answer7 = 
    {1.651156, 2.443584, 1.329044, 1.653890,  90.796249, 126.684265};
  NSPoint	p;
  NSSize	s;

  testObj = [NSAffineTransform new];
  [testObjs addObject:testObj];
  PASS(testObj != nil, "can create a new transfor");
   
  test_NSObject(@"NSAffineTransform", testObjs);
  test_NSCoding(testObjs);
  test_keyed_NSCoding(testObjs);
  test_NSCopying(@"NSAffineTransform", @"NSAffineTransform", testObjs, NO, YES);
  
  testObj = [NSAffineTransform transform];
  PASS(testObj != nil, "can create an autoreleased transform");

  [testObj setTransformStruct: flip];
  p = [testObj transformPoint: NSMakePoint(10,10)];
  PASS(eq(p.x, 10) && eq(p.y, -10), "flip transform inverts point y");

  s = [testObj transformSize: NSMakeSize(10,10)];
  PASS(s.width == 10 && s.height == -10, "flip transform inverts size height");

  p = [testObj transformPoint: p];
  s = [testObj transformSize: s];
  PASS(eq(p.x, 10) && eq(p.y, 10) && s.width == 10 && s.height == 10,
    "flip is reversible");
  
  testObj = [NSAffineTransform transform];
  [testObj translateXBy: 5.0 yBy: 6.0];
  p = [testObj transformPoint: NSMakePoint(10,10)];
  PASS(eq(p.x, 15.0) && eq(p.y, 16.0), "simple translate works");

  [testObj translateXBy: 5.0 yBy: 4.0];
  p = [testObj transformPoint: NSMakePoint(10,10)];
  PASS(eq(p.x, 20.0) && eq(p.y, 20.0), "two simple translates work");
  
  [testObj rotateByDegrees: 90.0];
  p = [testObj transformPoint: NSMakePoint(10,10)];
  PASS(eq(p.x, 0.0) && eq(p.y, 20.0), "translate and rotate works");
  
  testObj = [NSAffineTransform transform];

  [testObj rotateByDegrees: 90.0];
  p = [testObj transformPoint: NSMakePoint(10,10)];
  PASS(eq(p.x, -10.0) && eq(p.y, 10.0), "simple rotate works");
  
  [testObj translateXBy: 5.0 yBy: 6.0];
  p = [testObj transformPoint: NSMakePoint(10,10)];
  PASS(eq(p.x, -16.0) && eq(p.y, 15.0), "rotate and translate works");


  aa = [NSAffineTransform transform];
  bb = [NSAffineTransform transform];
  [aa setTransformStruct: as];
  [bb setTransformStruct: bs];

  /* Append matrix */
  cc = [aa copy];
  [cc appendTransform: bb];
  cs = [cc transformStruct];
  PASS((is_equal_struct(cs, answer1)), "appendTransform:")

  /* Prepend matrix */
  cc = [aa copy];
  [cc prependTransform: bb];
  cs = [cc transformStruct];
  PASS((is_equal_struct(cs, answer2)), "prependTransform:")

  /* scaling */
  cc = [aa copy];
  [cc scaleXBy: 3 yBy: 2];
  cs = [cc transformStruct];
  PASS((is_equal_struct(cs, answer3)), "scaleXBy:yBy:")
  //print_matrix ("Scale X A", cs);
  [cc translateXBy: 12 yBy: 14];
  cs = [cc transformStruct];
  PASS((is_equal_struct(cs, answer4)), "translateXBy:yBy:")
  //print_matrix ("Trans X Scale X A", cs);

  /* rotation */
  cc = [aa copy];
  [cc rotateByDegrees: 2.5];
  cs = [cc transformStruct];
  PASS((is_equal_struct(cs, answer5)), "rotateByDegrees")
  //print_matrix ("Rotate X A", cs);
  [cc translateXBy: 12 yBy: 14];
  cs = [cc transformStruct];
  PASS((is_equal_struct(cs, answer6)), "Translate X Rotate X A")
  //print_matrix ("Trans X Rotate X A", cs);

  /* multiple */
  [cc scaleXBy: .76 yBy: .34];
  cs = [cc transformStruct];
  PASS((is_equal_struct(cs, answer7)), "Scale X Translate X Rotate X A")
  //print_matrix ("Scale X Trans X Rotate X A", cs);

  [arp release]; arp = nil;
  return 0;
}
