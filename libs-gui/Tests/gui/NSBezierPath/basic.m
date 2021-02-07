#import "ObjectTesting.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <AppKit/NSBezierPath.h>
#include <math.h>

static BOOL eq(double d1, double d2)
{
  if (abs(d1 - d2) < 0.000001)
    return YES;
  return NO;
}

int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  id testObject;
  id testObject1;
  id testObject2;
  NSArray *testObjects;

  testObject = [NSBezierPath new];
  testObject1 = [NSBezierPath bezierPathWithRoundedRect: NSMakeRect(0, 0, 20, 30)
                                                xRadius: 2.0
                                                yRadius: 3.0];
  testObject2 = [NSBezierPath new];
  [testObject2 setLineWidth: 12.4];
  [testObject2 setFlatness: 13.4];
  [testObject2 setLineCapStyle: NSSquareLineCapStyle];
  [testObject2 setLineJoinStyle: NSRoundLineJoinStyle];
  [testObject2 setMiterLimit: 14.4];
  [testObject2 setWindingRule: NSEvenOddWindingRule];
  testObjects = [NSArray arrayWithObjects: testObject, testObject1, testObject2, nil];
  RELEASE(testObject);
  RELEASE(testObject2);

  test_alloc(@"NSBezierPath");
  test_NSObject(@"NSBezierPath", testObjects);
  test_NSCoding(testObjects);
  test_keyed_NSCoding(testObjects);
  test_NSCopying(@"NSBezierPath",
                 @"NSBezierPath",
		 testObjects, NO, NO);

  [arp release];
  return 0;
}

@implementation NSBezierPath (Testing)

- (BOOL) isEqual: (id)anObject
{
  NSInteger i;
  NSBezierPathElement type1;
  NSPoint pts1[3];
  NSBezierPathElement type2;
  NSPoint pts2[3];

  if (self == anObject)
    return YES;
  if (![anObject isKindOfClass: [NSBezierPath class]])
    return NO;
  if (!eq([self lineWidth], [anObject lineWidth]))
    {
      NSLog(@"different lineWidth %g %g", (float)[self lineWidth], (float)[anObject lineWidth]);
      return NO;
    }
  if (!eq([self flatness], [anObject flatness]))
    {
      NSLog(@"different flatness %g %g", (float)[self flatness], (float)[anObject flatness]);
      return NO;
    }
  if (!eq([self miterLimit], [anObject miterLimit]))
    {
      NSLog(@"different miterLimit %g %g", (float)[self miterLimit], (float)[anObject miterLimit]);
      return NO;
    }
  if ([self lineCapStyle] != [anObject lineCapStyle])
    {
      NSLog(@"different lineCapStyle %d %d", [self lineCapStyle], [anObject lineCapStyle]);
      return NO;
    }
  if ([self lineJoinStyle] != [anObject lineJoinStyle])
    {
      NSLog(@"different lineJoinStyle %d %d", [self lineJoinStyle], [anObject lineJoinStyle]);
      return NO;
    }
  if ([self windingRule] != [anObject windingRule])
    {
      NSLog(@"different winding rule %d %d", [self windingRule], [anObject windingRule]);
      return NO;
    }

  if ([self elementCount] != [anObject elementCount])
    {
      NSLog(@"different element count %d %d", [self elementCount], [anObject elementCount]);
      return NO;
    }

  for (i = 0; i < [self elementCount]; i++) 
    {
      type1 = [self elementAtIndex: i associatedPoints: pts1];
      type2 = [anObject elementAtIndex: i associatedPoints: pts2];
      if (type1 != type2)
        {
          NSLog(@"different type count %d %d", type1, type2);
          return NO;
        }

      if (!eq(pts1[0].x, pts2[0].x) || !eq(pts1[0].y, pts2[0].y))
        {
          NSLog(@"different point %@ %@", NSStringFromPoint(pts1[0]), NSStringFromPoint(pts2[0]));
          return NO;
        }

      if (type1 == NSCurveToBezierPathElement)
        {
          if (!eq(pts1[1].x, pts2[1].x) || !eq(pts1[1].y, pts2[1].y))
            {
              NSLog(@"different point %@ %@", NSStringFromPoint(pts1[1]), NSStringFromPoint(pts2[1]));
              return NO;
            }
          if (!eq(pts1[2].x, pts2[2].x) || !eq(pts1[2].y, pts2[2].y))
            {
              NSLog(@"different point %@ %@", NSStringFromPoint(pts1[2]), NSStringFromPoint(pts2[2]));
              return NO;
            }
        }
    }

  return YES;
}

@end
