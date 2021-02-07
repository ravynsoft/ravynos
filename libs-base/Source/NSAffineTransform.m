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
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import "common.h"
#include <math.h>

#define	EXPOSE_NSAffineTransform_IVARS	1

#import "Foundation/NSArray.h"
#import "Foundation/NSData.h"
#import "Foundation/NSException.h"
#import "Foundation/NSAffineTransform.h"
#import "Foundation/NSCoder.h"

/* Private definitions */
#define A _matrix.m11
#define B _matrix.m12
#define C _matrix.m21
#define D _matrix.m22
#define TX _matrix.tX
#define TY _matrix.tY

/* A Postscript matrix looks like this:

  /  a  b  0 \
  |  c  d  0 |
  \ tx ty  1 /

 */

static const CGFloat pi = 3.1415926535897932384626434;

#if 0
#define valid(o) NSAssert((o->_isIdentity && o->A==1.0 && o->B==0.0 && o->C==0.0 && o->D==1.0) || (o->_isFlipY && o->A==1.0 && o->B==0.0 && o->C==0.0 && o->D==-1.0) || !(o->_isIdentity||o->_isFlipY), NSInternalInconsistencyException) 
#define check() valid(self)
#else
#define valid(o)
#define check()
#endif

/* Quick function to multiply two coordinate matrices. C = AB */
static inline NSAffineTransformStruct 
matrix_multiply (NSAffineTransformStruct MA, NSAffineTransformStruct MB)
{
  NSAffineTransformStruct MC;
  MC.m11 = MA.m11 * MB.m11 + MA.m12 * MB.m21;
  MC.m12 = MA.m11 * MB.m12 + MA.m12 * MB.m22;
  MC.m21 = MA.m21 * MB.m11 + MA.m22 * MB.m21;
  MC.m22 = MA.m21 * MB.m12 + MA.m22 * MB.m22;
  MC.tX  = MA.tX * MB.m11 + MA.tY * MB.m21 + MB.tX;
  MC.tY  = MA.tX * MB.m12 + MA.tY * MB.m22 + MB.tY;
  return MC;
}

/*
  MC.m11 = MA->A * MB->A + MA->B * MB->C;
  MC.m12 = MA->A * MB->B + MA->B * MB->D;
  MC.m21 = MA->C * MB->A + MA->D * MB->C;
  MC.m22 = MA->C * MB->B + MA->D * MB->D;
  MC.tX  = MA->TX * MB->A + MA->TY * MB->C + MB->TX;
  MC.tY  = MA->TX * MB->B + MA->TY * MB->D + MB->TY;
 */

@implementation NSAffineTransform

static NSAffineTransformStruct identityTransform = {
   1.0, 0.0, 0.0, 1.0, 0.0, 0.0
};

/**
 * Return an autoreleased instance of this class.
 */
+ (NSAffineTransform*) transform
{
  NSAffineTransform	*t;

  t = (NSAffineTransform*)NSAllocateObject(self, 0, NSDefaultMallocZone());
  t->_matrix = identityTransform;
  t->_isIdentity = YES;
  return AUTORELEASE(t);
}

/**
 * Return an autoreleased instance of this class.
 */
+ (id) new
{
  NSAffineTransform	*t;

  t = (NSAffineTransform*)NSAllocateObject(self, 0, NSDefaultMallocZone());
  t->_matrix = identityTransform;
  t->_isIdentity = YES;
  return t;
}

/**
 * Appends the transform matrix to the receiver.  This is done by performing a
 * matrix multiplication of the receiver with aTransform so that aTransform
 * is the first transform applied to the user coordinate. The new
 * matrix then replaces the receiver's matrix.
 */
- (void) appendTransform: (NSAffineTransform*)aTransform
{
  valid(aTransform);

  if (aTransform->_isIdentity)
    {
      TX += aTransform->TX;
      TY += aTransform->TY;
      check();
      return;
    }

  if (aTransform->_isFlipY)
    {
      B = -B;
      D = -D;
      TX = aTransform->TX + TX;
      TY = aTransform->TY - TY;
      if (_isIdentity)
        {
	  _isFlipY = YES;
	  _isIdentity = NO;
	}
      else if (_isFlipY)
        {
	  _isFlipY = NO;
	  _isIdentity = YES;
	}
      check();
      return;
    }

  if (_isIdentity)
    {
      CGFloat newTX;

      A = aTransform->A;
      B = aTransform->B;
      C = aTransform->C;
      D = aTransform->D;
      newTX = TX * aTransform->A + TY * aTransform->C + aTransform->TX;
      TY = TX * aTransform->B + TY * aTransform->D + aTransform->TY;
      TX = newTX;
      _isIdentity = NO;  // because aTransform is not an identity transform.
      _isFlipY = aTransform->_isFlipY;
      check();
      return;
    }

  if (_isFlipY)
    {
      CGFloat newTX;

      A = aTransform->A;
      B = aTransform->B;
      C = -aTransform->C;
      D = -aTransform->D;
      newTX  = TX * aTransform->A + TY * aTransform->C + aTransform->TX;
      TY  = TX * aTransform->B + TY * aTransform->D + aTransform->TY;
      TX = newTX;
      _isIdentity = NO;
      _isFlipY = NO;
      check();
      return;
    }

  _matrix = matrix_multiply(_matrix, aTransform->_matrix);
  _isIdentity = NO;
  _isFlipY = NO;
  check();
}

- (NSString*) description
{
  return [NSString stringWithFormat:
    @"NSAffineTransform ((%f, %f) (%f, %f) (%f, %f))", A, B, C, D, TX, TY];
}

/**
 * Initialize the transformation matrix instance to the identity matrix.
 * The identity matrix transforms a point to itself.
 */ 
- (id) init
{
  _matrix = identityTransform;
  _isIdentity = YES;
  return self;
}

/**
 * Initialize the receiever's instance with the instance represented 
 * by aTransform. 
 */
- (id) initWithTransform: (NSAffineTransform*)aTransform
{
  _matrix = aTransform->_matrix;
  _isIdentity = aTransform->_isIdentity;
  _isFlipY = aTransform->_isFlipY;
  return self;
}

/**
 * Calculates the inverse of the receiver's matrix and replaces the 
 * receiever's matrix with it.
 */
- (void) invert
{
  CGFloat newA, newB, newC, newD, newTX, newTY;
  CGFloat det;

  if (_isIdentity)
    {
      TX = -TX;
      TY = -TY;
      return;
    }

  if (_isFlipY)
    {
      TX = -TX;
      return;
    }

  det = A * D - B * C;
  if (det == 0)
    {
      NSLog (@"error: determinant of matrix is 0!");
      return;
    }

  newA = D / det;
  newB = -B / det;
  newC = -C / det;
  newD = A / det;
  newTX = (-D * TX + C * TY) / det;
  newTY = (B * TX - A * TY) / det;

  NSDebugLLog(@"NSAffineTransform",
	@"inverse of matrix ((%f, %f) (%f, %f) (%f, %f))\n"
	@"is ((%f, %f) (%f, %f) (%f, %f))",
	A, B, C, D, TX, TY,
	newA, newB, newC, newD, newTX, newTY);

  A = newA; B = newB;
  C = newC; D = newD;
  TX = newTX; TY = newTY;
}

/**
 * Prepends the transform matrix to the receiver.  This is done by performing a
 * matrix multiplication of the receiver with aTransform so that aTransform
 * is the last transform applied to the user coordinate. The new
 * matrix then replaces the receiver's matrix.
 */
- (void) prependTransform: (NSAffineTransform*)aTransform
{
  valid(aTransform);

  if (aTransform->_isIdentity)
    {
      CGFloat newTX;

      newTX = aTransform->TX * A + aTransform->TY * C + TX;
      TY = aTransform->TX * B + aTransform->TY * D + TY;
      TX = newTX;
      check();
      return;
    }

  if (aTransform->_isFlipY)
    {
      CGFloat newTX;

      newTX = aTransform->TX * A + aTransform->TY * C + TX;
      TY = aTransform->TX * B + aTransform->TY * D + TY;
      TX = newTX;
      C = -C;
      D = -D;
      if (_isIdentity)
        {
	  _isFlipY = YES;
	  _isIdentity = NO;
	}
      else if (_isFlipY)
        {
	  _isFlipY = NO;
	  _isIdentity = YES;
	}
      check();
      return;
    }

  if (_isIdentity)
    {
      A = aTransform->A;
      B = aTransform->B;
      C = aTransform->C;
      D = aTransform->D;
      TX += aTransform->TX;
      TY += aTransform->TY;
      _isIdentity = NO;
      _isFlipY = aTransform->_isFlipY;
      check();
      return;
    }

  if (_isFlipY)
    {
      A = aTransform->A;
      B = -aTransform->B;
      C = aTransform->C;
      D = -aTransform->D;
      TX += aTransform->TX;
      TY -= aTransform->TY;
      _isIdentity = NO;
      _isFlipY = NO;
      check();
      return;
    }

  _matrix = matrix_multiply(aTransform->_matrix, _matrix);
  _isIdentity = NO;
  _isFlipY = NO;
  check();
}

/**
 * Applies the rotation specified by angle in degrees.   Points transformed
 * with the transformation matrix of the receiver are rotated counter-clockwise 
 * by the number of degrees specified by angle.
 */
- (void) rotateByDegrees: (CGFloat)angle
{
  [self rotateByRadians: pi * angle / 180];
}

/**
 * Applies the rotation specified by angle in radians.   Points transformed
 * with the transformation matrix of the receiver are rotated counter-clockwise 
 * by the number of radians specified by angle.
 */
- (void) rotateByRadians: (CGFloat)angleRad
{
  if (angleRad != 0.0)
    {
      CGFloat sine;
      CGFloat cosine;
      NSAffineTransformStruct rotm;

      sine = sin (angleRad);
      cosine = cos (angleRad);
      rotm.m11 = cosine;
      rotm.m12 = sine;
      rotm.m21 = -sine;
      rotm.m22 = cosine;
      rotm.tX = rotm.tY = 0;
      _matrix = matrix_multiply(rotm, _matrix);
      _isIdentity = NO;
      _isFlipY = NO;
      check();
    }
}

/**
 * Scales the transformation matrix of the reciever by the factor specified
 * by scale.  
 */
- (void) scaleBy: (CGFloat)scale
{
  NSAffineTransformStruct scam = identityTransform;

  scam.m11 = scale;
  scam.m22 = scale;
  _matrix = matrix_multiply(scam, _matrix);
  _isIdentity = NO;
  _isFlipY = NO;
  check();
}

/**
 * Scales the X axis of the receiver's transformation matrix 
 * by scaleX and the Y axis of the transformation matrix by scaleY.
 */
- (void) scaleXBy: (CGFloat)scaleX yBy: (CGFloat)scaleY
{
  if (_isIdentity && scaleX == 1.0)
    {
      if (scaleY == 1.0)
        {
	  return;	// no scaling
	}
      if (scaleY == -1.0)
	{
	  D = -1.0;
	  _isFlipY = YES;
	  _isIdentity = NO;
	  return;
	}
    }

  if (_isFlipY && scaleX == 1.0)
    {
      if (scaleY == 1.0)
        {
	  return;	// no scaling
	}
      if (scaleY == -1.0)
	{
	  D = 1.0;
	  _isFlipY = NO;
	  _isIdentity = YES;
	  return;
	}
    }

  A *= scaleX;
  B *= scaleX;
  C *= scaleY;
  D *= scaleY;
  _isIdentity = NO;
  _isFlipY = NO;
}

/**
 * <p>
 * Sets the structure which represents the matrix of the reciever. 
 * The struct is of the form:</p>
 * <p>{m11, m12, m21, m22, tX, tY}</p>
 */
- (void) setTransformStruct: (NSAffineTransformStruct)val
{
  _matrix = val;
  _isIdentity = NO;
  _isFlipY = NO;
  if (A == 1.0 && B == 0.0 && C == 0.0)
    {
      if (D == 1.0)
	{
	  _isIdentity = YES;
	}
      else if (D == -1.0)
	{
	  _isFlipY = YES;
	}
    }
  check();
}

/**
 * Transforms a single point based on the transformation matrix.
 * Returns the resulting point.
 */
- (NSPoint) transformPoint: (NSPoint)aPoint
{
  NSPoint new;

  if (_isIdentity)
    {
      new.x = TX + aPoint.x;
      new.y = TY + aPoint.y;
    }
  else if (_isFlipY)
    {
      new.x = TX + aPoint.x;
      new.y = TY - aPoint.y;
    }
  else
    {
      new.x = A * aPoint.x + C * aPoint.y + TX;
      new.y = B * aPoint.x + D * aPoint.y + TY;
    }

  return new;
}

/**
 * Transforms the NSSize represented by aSize using the reciever's 
 * transformation matrix.  Returns the resulting NSSize.<br />
 * NB. A transform can result in negative size components ... so calling
 * code should check for and deal with that situation.
 */
- (NSSize) transformSize: (NSSize)aSize
{
  if (_isIdentity)
    {
      return aSize;
    }
  else
    {
      NSSize new;

      if (_isFlipY)
        {
	  new.width = aSize.width;
	  new.height = -aSize.height;
	}
      else
        {
	  new.width = A * aSize.width + C * aSize.height;
	  new.height = B * aSize.width + D * aSize.height;
	}
      return new;
    }
}

/**
 * <p>
 * Returns the <code>NSAffineTransformStruct</code> structure 
 * which represents the matrix of the reciever. 
 * The struct is of the form:</p>
 * <p>{m11, m12, m21, m22, tX, tY}</p>
 */
- (NSAffineTransformStruct) transformStruct
{
  return _matrix;
}

/**
 * Applies the translation specified by tranX and tranY to the receiver's
 * matrix.
 * Points transformed by the reciever's matrix after this operation will 
 * be shifted in position based on the specified translation.
 */
- (void) translateXBy: (CGFloat)tranX  yBy: (CGFloat)tranY
{
  if (_isIdentity)
    {
      TX += tranX;
      TY += tranY;
    }
  else if (_isFlipY)
    {
      TX += tranX;
      TY -= tranY;
    }
  else
    {
      TX += A * tranX + C * tranY;
      TY += B * tranX + D * tranY;
    }
  check();
}

- (id) copyWithZone: (NSZone*)zone
{
  return NSCopyObject(self, 0, zone);
}

- (BOOL) isEqual: (id)anObject
{
  if (anObject == self)
    {
      return YES;
    }
  if (YES == [anObject isKindOfClass: [NSAffineTransform class]])
    {
      NSAffineTransformStruct	o = [anObject transformStruct];

      if (0 == memcmp((void*)&o, (void*)&_matrix, sizeof(o)))
	{
	  return YES;
	}
    }
  return NO;
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  NSAffineTransformStruct replace = identityTransform;
    
  if ([aCoder allowsKeyedCoding])
    {
      if ([aCoder containsValueForKey: @"NSTransformStruct"])
        {
	  NSUInteger length;
	  const uint8_t *data;
          NSData *d;
          unsigned int cursor = 0;

          data = [aCoder decodeBytesForKey: @"NSTransformStruct"
                              returnedLength: &length]; 
          d = [NSData dataWithBytes: data length: length];

          if (length == 9)
            {
              float f, g;
              replace = identityTransform;
              
              cursor = 1;
              [d deserializeDataAt: &f
                        ofObjCType: "f"
                          atCursor: &cursor
                           context: nil];
              [d deserializeDataAt: &g
                        ofObjCType: "f"
                          atCursor: &cursor
                           context: nil];
              if (data[0] == 1)
                {
                  replace.tX = f;
                  replace.tY = g;
                }
              else if (data[0] == 2)
                {
                  replace.m11 = f;
                  replace.m22 = g;
                }
              else
                {
                  // FIXME
                  NSLog(@"Got type %d for affine transform", data[0]);
                  return [self notImplemented: _cmd];
                }
            }
          else if (16 == length)
            {
              float s[4];
              
              [d deserializeDataAt: s
                        ofObjCType: "[4f]"
                          atCursor: &cursor
                           context: nil];
              replace.m11 = s[0];
              replace.m22 = s[1];
              replace.tX = s[2];
              replace.tY = s[3];
            }
          else if (24 == length)
            {
              float s[6];
              
              [d deserializeDataAt: s
                        ofObjCType: "[6f]"
                          atCursor: &cursor
                           context: nil];
              replace.m11 = s[0];
              replace.m12 = s[1];
              replace.m21 = s[2];
              replace.m22 = s[3];
              replace.tX = s[4];
              replace.tY = s[5];
            }
          else
            {
              // FIXME
              NSLog(@"Got data %@ len %d for affine transform", d, (int)length);
              return [self notImplemented: _cmd];
            }
        }
      else
        {
          replace = identityTransform;
        }
    }
  else
    {
      [aCoder decodeArrayOfObjCType: @encode(CGFloat)
                              count: 6
                                 at: (CGFloat*)&replace];
    }
  [self setTransformStruct: replace];
  return self;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  NSAffineTransformStruct replace;
    
  replace = [self transformStruct];
  if ([aCoder allowsKeyedCoding])
    {
      if (!_isIdentity)
        {
          float s[6];
          NSMutableData *d = [NSMutableData data];

          s[0] = replace.m11;
          s[1] = replace.m12;
          s[2] = replace.m21;
          s[3] = replace.m22;
          s[4] = replace.tX;
          s[5] = replace.tY;
          [d serializeDataAt: s
                  ofObjCType: "[6f]"
                     context: nil];
          [aCoder encodeBytes: [d bytes]
                       length: [d length]
                       forKey: @"NSTransformStruct"];
        }
    }
  else
    {
      [aCoder encodeArrayOfObjCType: @encode(CGFloat)
                              count: 6
                                 at: (CGFloat*)&replace];
    }
}

@end /* NSAffineTransform */

