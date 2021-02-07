/** <title>NSAffineTransform.m</title>

   <abstract>
   This class provides a way to perform affine transforms.  It provides 
   a matrix for transforming from one coordinate system to another.
   </abstract>
   Copyright (C) 1996,1999 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: August 1997
   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: March 1999
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#import "config.h"
#include <math.h>

#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>

#import "AppKit/NSAffineTransform.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/PSOperators.h"

/* Private definitions */
#define A matrix.m11
#define B matrix.m12
#define C matrix.m21
#define D matrix.m22
#define TX matrix.tX
#define TY matrix.tY

/* A Postscript matrix looks like this:

  /  a  b  0 \
  |  c  d  0 |
  \ tx ty  1 /

 */

static const CGFloat pi = 3.1415926535897932384626434;

@implementation NSAffineTransform (GUIAdditions)

/**
 * Concatenates the receiver's matrix with the one in the current graphics 
 * context.
 */
- (void) concat
{
  // FIXME Should use GSConcatCTM:
  NSAffineTransformStruct ats = [self transformStruct];
  CGFloat floatMatrix[6];

  floatMatrix[0] = ats.m11;
  floatMatrix[1] = ats.m12;
  floatMatrix[2] = ats.m21;
  floatMatrix[3] = ats.m22;
  floatMatrix[4] = ats.tX;
  floatMatrix[5] = ats.tY;
  PSconcat(floatMatrix);
}


/**
 * Get the currently active graphics context's transformation 
 * matrix and set it into the receiver.
 */
- (void) set
{
  GSSetCTM(GSCurrentContext(), self);
}

/**
 * <p>Applies the receiver's transformation matrix to each point in 
 * the bezier path, then returns the result.  The original bezier 
 * path is not modified.
 * </p>
 */
- (NSBezierPath*) transformBezierPath: (NSBezierPath*)aPath
{
  NSBezierPath *path = [aPath copy];

  [path transformUsingAffineTransform: self];
  return AUTORELEASE(path);
}

@end /* NSAffineTransform (GUIAdditions) */

@implementation NSAffineTransform (GNUstep)

- (void) scaleTo: (CGFloat)sx : (CGFloat)sy
{
  NSAffineTransformStruct matrix = [self transformStruct];

  /* If it's rotated.  */
  if (B != 0  ||  C != 0)
    {
      // FIXME: This case does not handle shear.
      CGFloat angle = [self rotationAngle];

      // Keep the translation and add scaling
      A = sx; B = 0;
      C = 0; D = sy;
      [self setTransformStruct: matrix];

      // Prepend the rotation to the scaling and translation
      [self rotateByDegrees: angle];
    }
  else
    {
      A = sx; B = 0;
      C = 0; D = sy;
      [self setTransformStruct: matrix];
    }
}

- (void) translateToPoint: (NSPoint)point
{
  [self translateXBy: point.x yBy: point.y];
}

- (void) makeIdentityMatrix
{
  [self init];
}

- (void) setFrameOrigin: (NSPoint)point
{
  NSAffineTransformStruct matrix = [self transformStruct];
  CGFloat dx = point.x - TX;
  CGFloat dy = point.y - TY;

  [self translateXBy: dx yBy: dy];
}

- (void) setFrameRotation: (CGFloat)angle
{
  [self rotateByDegrees: angle - [self rotationAngle]];
}

- (CGFloat) rotationAngle
{
  NSAffineTransformStruct matrix = [self transformStruct];
  CGFloat rotationAngle = atan2(-C, A);

  rotationAngle *= 180.0 / pi;
  if (rotationAngle < 0.0)
    rotationAngle += 360.0;

  return rotationAngle;
}

- (void) concatenateWith: (NSAffineTransform*)anotherMatrix
{
  GSOnceMLog(@"deprecated ... use -prependTransform:");
  [self prependTransform: anotherMatrix];
}

- (void) concatenateWithMatrix: (const float[6])anotherMatrix
{
  NSAffineTransformStruct amat;
  NSAffineTransform 	*other;

  GSOnceMLog(@"deprecated ... use -prependTransform:");
  amat.m11 = anotherMatrix[0];
  amat.m12 = anotherMatrix[1];
  amat.m21 = anotherMatrix[2];
  amat.m22 = anotherMatrix[3];
  amat.tX  = anotherMatrix[4];
  amat.tY  = anotherMatrix[5];
  other = [NSAffineTransform new];
  [other setTransformStruct: amat];
  [self prependTransform: other];
  RELEASE(other);
}

- (void)inverse
{
  GSOnceMLog(@"deprecated ... use -invert:");
  [self invert];
}

- (BOOL) isRotated
{
  NSAffineTransformStruct matrix = [self transformStruct];

  if (B == 0 && C == 0)
    {
      return NO;
    }
  else
    {
      return YES;
    }
}

- (void) boundingRectFor: (NSRect)rect result: (NSRect*)newRect
{
  NSAffineTransformStruct matrix = [self transformStruct];
  /* Shortcuts of the usual rect values */
  CGFloat x = rect.origin.x;
  CGFloat y = rect.origin.y;
  CGFloat width = rect.size.width;
  CGFloat height = rect.size.height;
  CGFloat xc[3];
  CGFloat yc[3];
  CGFloat min_x;
  CGFloat max_x;
  CGFloat min_y;
  CGFloat max_y;
  int i;

  max_x = A * x + C * y + TX;
  max_y = B * x + D * y + TY;
  xc[0] = max_x + A * width;
  yc[0] = max_y + B * width;
  xc[1] = max_x + C * height;
  yc[1] = max_y + D * height;
  xc[2] = max_x + A * width + C * height;
  yc[2] = max_y + B * width + D * height;
  
  min_x = max_x;
  min_y = max_y;
  
  for (i = 0; i < 3; i++) 
    {
      if (xc[i] < min_x)
	min_x = xc[i];
      if (xc[i] > max_x)
	max_x = xc[i];

      if (yc[i] < min_y)
	 min_y = yc[i];
      if (yc[i] > max_y)
	max_y = yc[i];
    }

  newRect->origin.x = min_x;
  newRect->origin.y = min_y;
  newRect->size.width = max_x -min_x;
  newRect->size.height = max_y -min_y;
}

- (NSPoint) pointInMatrixSpace: (NSPoint)point
{
  GSOnceMLog(@"deprecated ... use -transformPoint:");
  return [self transformPoint: point];
}

- (NSPoint) deltaPointInMatrixSpace: (NSPoint)point
{
  NSAffineTransformStruct matrix = [self transformStruct];
  NSPoint new;

  new.x = A * point.x + C * point.y;
  new.y = B * point.x + D * point.y;

  return new;
}

- (NSSize) sizeInMatrixSpace: (NSSize)size
{
  GSOnceMLog(@"deprecated ... use -transformSize:");
  return [self transformSize: size];
}

- (NSRect) rectInMatrixSpace: (NSRect)rect
{
  NSRect new;

  new.origin = [self transformPoint: rect.origin];
  new.size = [self transformSize: rect.size];

  if (new.size.width < 0)
    {
      new.origin.x += new.size.width;
      new.size.width *= -1;
    }

  if (new.size.height < 0)
    {
      new.origin.y += new.size.height;
      new.size.height *= -1;
    }

  return new;
}

- (void) setMatrix: (const float[6])replace
{
  NSAffineTransformStruct matrix;

  GSOnceMLog(@"deprecated ... use -setTransformStruct:");
  matrix.m11 = replace[0];
  matrix.m12 = replace[1];
  matrix.m21 = replace[2];
  matrix.m22 = replace[3];
  matrix.tX = replace[4];
  matrix.tY = replace[5];
  [self setTransformStruct: matrix];
}

- (void) getMatrix: (float[6])replace
{
  NSAffineTransformStruct matrix = [self transformStruct];

  GSOnceMLog(@"deprecated ... use -transformStruct");
  replace[0] = matrix.m11;
  replace[1] = matrix.m12;
  replace[2] = matrix.m21;
  replace[3] = matrix.m22;
  replace[4] = matrix.tX;
  replace[5] = matrix.tY;
}

- (void) takeMatrixFromTransform: (NSAffineTransform *)aTransform
{
  NSAffineTransformStruct matrix;

  GSOnceMLog(@"deprecated ... use -transformStruct and setTransformStruct:");
  matrix = [aTransform transformStruct];
  [self setTransformStruct: matrix];
}


@end /* NSAffineTransform (GNUstep) */

