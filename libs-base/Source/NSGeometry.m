/** NSGeometry.m - geometry functions
 * Copyright (C) 1993, 1994, 1995 Free Software Foundation, Inc.
 *
 * Written by:  Adam Fedor <fedor@boulder.colorado.edu>
 * Date: Mar 1995
 *
 * This file is part of the GNUstep Base Library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110 USA.

   <title>NSGeometry class reference</title>
   $Date$ $Revision$
 */

/*
 *	Define IN_NSGEOMETRY_M so that the Foundation/NSGeometry.h header can
 *	provide non-inline versions of the function implementations for us.
 */
#define	IN_NSGEOMETRY_M


/**** Included Headers *******************************************************/

#import "common.h"
#include <math.h>
#import "Foundation/NSGeometry.h"
#import "Foundation/NSScanner.h"
#import "Foundation/NSNotification.h"
#import "GSPrivate.h"

static Class	NSStringClass = 0;
static Class	NSScannerClass = 0;
static SEL	scanFloatSel;
static SEL	scanStringSel;
static SEL	scannerSel;
static BOOL	(*scanFloatImp)(NSScanner*, SEL, CGFloat*);
static BOOL	(*scanStringImp)(NSScanner*, SEL, NSString*, NSString**);
static id 	(*scannerImp)(Class, SEL, NSString*);

static inline void
setupCache(void)
{
  if (NSStringClass == 0)
    {
      NSStringClass = [NSString class];
      NSScannerClass = [NSScanner class];
      if (sizeof(CGFloat) == sizeof(double))
        {
          scanFloatSel = @selector(scanDouble:);
        }
      else
        {
          scanFloatSel = @selector(scanFloat:);
        }
      scanStringSel = @selector(scanString:intoString:);
      scannerSel = @selector(scannerWithString:);
      scanFloatImp = (BOOL (*)(NSScanner*, SEL, CGFloat*))
	[NSScannerClass instanceMethodForSelector: scanFloatSel];
      scanStringImp = (BOOL (*)(NSScanner*, SEL, NSString*, NSString**))
	[NSScannerClass instanceMethodForSelector: scanStringSel];
      scannerImp = (id (*)(Class, SEL, NSString*))
	[NSScannerClass methodForSelector: scannerSel];
    }
}

static BOOL GSMacOSXCompatibleGeometry(void)
{
  if (GSPrivateDefaultsFlag(GSOldStyleGeometry) == YES)
    return NO;
  return GSPrivateDefaultsFlag(GSMacOSXCompatible);
}

/**** Function Implementations ***********************************************/
/* Most of these are implemented in the header file as inline functkions */

NSRect
NSIntegralRect(NSRect aRect)
{
  NSRect	rect;

  if (NSIsEmptyRect(aRect))
    return NSMakeRect(0, 0, 0, 0);

  rect.origin.x = floor(NSMinX(aRect));
  rect.origin.y = floor(NSMinY(aRect));
  rect.size.width = ceil(NSMaxX(aRect)) - rect.origin.x;
  rect.size.height = ceil(NSMaxY(aRect)) - rect.origin.y;
  return rect;
}

void 	
NSDivideRect(NSRect aRect,
             NSRect *slice,
             NSRect *remainder,
             CGFloat amount,
             NSRectEdge edge)
{
  static NSRect sRect;
  static NSRect	rRect;

  if (!slice)
    slice = &sRect;
  if (!remainder)
    remainder = &rRect;

  if (NSIsEmptyRect(aRect))
    {
      *slice = NSMakeRect(0,0,0,0);
      *remainder = NSMakeRect(0,0,0,0);
      return;
    }

  switch (edge)
    {
      case NSMinXEdge:
	if (amount > aRect.size.width)
	  {
	    *slice = aRect;
	    *remainder = NSMakeRect(NSMaxX(aRect),
				    aRect.origin.y,
				    0,
				    aRect.size.height);
	  }
	else
	  {
	    *slice = NSMakeRect(aRect.origin.x,
				aRect.origin.y,
				amount,
				aRect.size.height);
	    *remainder = NSMakeRect(NSMaxX(*slice),
				    aRect.origin.y,
				    NSMaxX(aRect) - NSMaxX(*slice),
				    aRect.size.height);
	  }
	break;
      case NSMinYEdge:
	if (amount > aRect.size.height)
	  {
	    *slice = aRect;
	    *remainder = NSMakeRect(aRect.origin.x,
				    NSMaxY(aRect),
				    aRect.size.width, 0);
	  }
	else
	  {
	    *slice = NSMakeRect(aRect.origin.x,
				aRect.origin.y,
				aRect.size.width,
				amount);
	    *remainder = NSMakeRect(aRect.origin.x,
				    NSMaxY(*slice),
				    aRect.size.width,
				    NSMaxY(aRect) - NSMaxY(*slice));
	  }
	break;
      case (NSMaxXEdge):
	if (amount > aRect.size.width)
	  {
	    *slice = aRect;
	    *remainder = NSMakeRect(aRect.origin.x,
				    aRect.origin.y,
				    0,
				    aRect.size.height);
	  }
	else
	  {
	    *slice = NSMakeRect(NSMaxX(aRect) - amount,
				aRect.origin.y,
				amount,
				aRect.size.height);
	    *remainder = NSMakeRect(aRect.origin.x,
				    aRect.origin.y,
				    NSMinX(*slice) - aRect.origin.x,
				    aRect.size.height);
	  }
	break;
      case NSMaxYEdge:
	if (amount > aRect.size.height)
	  {
	    *slice = aRect;
	    *remainder = NSMakeRect(aRect.origin.x,
				    aRect.origin.y,
				    aRect.size.width,
				    0);
	  }
	else
	  {
	    *slice = NSMakeRect(aRect.origin.x,
				NSMaxY(aRect) - amount,
				aRect.size.width,
				amount);
	    *remainder = NSMakeRect(aRect.origin.x,
				    aRect.origin.y,
				    aRect.size.width,
				    NSMinY(*slice) - aRect.origin.y);
	  }
	break;
      default:
	break;
    }

  return;
}

/** Get a String Representation... **/
/* NOTE: Spaces around '=' so that old OpenStep implementations can
   read our strings (Both GNUstep and Mac OS X can read these as well).  */

NSString*
NSStringFromPoint(NSPoint aPoint)
{
  setupCache();
  if (GSMacOSXCompatibleGeometry() == YES)
    return [NSStringClass stringWithFormat:
      @"{%g, %g}", aPoint.x, aPoint.y];
  else
    return [NSStringClass stringWithFormat:
      @"{x = %g; y = %g}", aPoint.x, aPoint.y];
}

NSString*
NSStringFromRect(NSRect aRect)
{
  setupCache();
  if (GSMacOSXCompatibleGeometry() == YES)
    return [NSStringClass stringWithFormat:
      @"{{%g, %g}, {%g, %g}}",
      aRect.origin.x, aRect.origin.y, aRect.size.width, aRect.size.height];
  else
    return [NSStringClass stringWithFormat:
      @"{x = %g; y = %g; width = %g; height = %g}",
      aRect.origin.x, aRect.origin.y, aRect.size.width, aRect.size.height];
}

NSString*
NSStringFromSize(NSSize aSize)
{
  setupCache();
  if (GSMacOSXCompatibleGeometry() == YES)
    return [NSStringClass stringWithFormat:
      @"{%g, %g}", aSize.width, aSize.height];
  else
    return [NSStringClass stringWithFormat:
      @"{width = %g; height = %g}", aSize.width, aSize.height];
}

NSPoint
NSPointFromString(NSString* string)
{
  NSScanner	*scanner;
  NSPoint	point;

  setupCache();
  scanner = (*scannerImp)(NSScannerClass, scannerSel, string);
  if ((*scanStringImp)(scanner, scanStringSel, @"{", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"x", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"=", NULL)
    && (*scanFloatImp)(scanner, scanFloatSel, &point.x)
    && (*scanStringImp)(scanner, scanStringSel, @";", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"y", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"=", NULL)
    && (*scanFloatImp)(scanner, scanFloatSel, &point.y)
    && (*scanStringImp)(scanner, scanStringSel, @"}", NULL))
    {
      return point;
    }
  else
    {
      [scanner setScanLocation: 0];
      if ((*scanStringImp)(scanner, scanStringSel, @"{", NULL)
	&& (*scanFloatImp)(scanner, scanFloatSel, &point.x)
	&& (*scanStringImp)(scanner, scanStringSel, @",", NULL)
	&& (*scanFloatImp)(scanner, scanFloatSel, &point.y)
	&& (*scanStringImp)(scanner, scanStringSel, @"}", NULL))
	{
	  return point;
	}
      else
	{
	  return NSMakePoint(0, 0);
	}
    }
}

NSSize
NSSizeFromString(NSString* string)
{
  NSScanner	*scanner;
  NSSize	size;

  setupCache();
  scanner = (*scannerImp)(NSScannerClass, scannerSel, string);
  if ((*scanStringImp)(scanner, scanStringSel, @"{", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"width", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"=", NULL)
    && (*scanFloatImp)(scanner, scanFloatSel, &size.width)
    && (*scanStringImp)(scanner, scanStringSel, @";", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"height", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"=", NULL)
    && (*scanFloatImp)(scanner, scanFloatSel, &size.height)
    && (*scanStringImp)(scanner, scanStringSel, @"}", NULL))
    {
      return size;
    }
  else
    {
      [scanner setScanLocation: 0];
      if ((*scanStringImp)(scanner, scanStringSel, @"{", NULL)
	&& (*scanFloatImp)(scanner, scanFloatSel, &size.width)
	&& (*scanStringImp)(scanner, scanStringSel, @",", NULL)
	&& (*scanFloatImp)(scanner, scanFloatSel, &size.height)
	&& (*scanStringImp)(scanner, scanStringSel, @"}", NULL))
	{
	  return size;
	}
      else
	{
	  return NSMakeSize(0, 0);
	}
    }
}

NSRect
NSRectFromString(NSString* string)
{
  NSScanner	*scanner;
  NSRect	rect;

  setupCache();
  scanner = (*scannerImp)(NSScannerClass, scannerSel, string);
  if ((*scanStringImp)(scanner, scanStringSel, @"{", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"x", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"=", NULL)
    && (*scanFloatImp)(scanner, scanFloatSel, &rect.origin.x)
    && (*scanStringImp)(scanner, scanStringSel, @";", NULL)

    && (*scanStringImp)(scanner, scanStringSel, @"y", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"=", NULL)
    && (*scanFloatImp)(scanner, scanFloatSel, &rect.origin.y)
    && (*scanStringImp)(scanner, scanStringSel, @";", NULL)

    && (*scanStringImp)(scanner, scanStringSel, @"width", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"=", NULL)
    && (*scanFloatImp)(scanner, scanFloatSel, &rect.size.width)
    && (*scanStringImp)(scanner, scanStringSel, @";", NULL)

    && (*scanStringImp)(scanner, scanStringSel, @"height", NULL)
    && (*scanStringImp)(scanner, scanStringSel, @"=", NULL)
    && (*scanFloatImp)(scanner, scanFloatSel, &rect.size.height)
    && (*scanStringImp)(scanner, scanStringSel, @"}", NULL))
    {
      return rect;
    }
  else
    {
      [scanner setScanLocation: 0];
      if ((*scanStringImp)(scanner, scanStringSel, @"{", NULL)
	&& (*scanStringImp)(scanner, scanStringSel, @"{", NULL)
	&& (*scanFloatImp)(scanner, scanFloatSel, &rect.origin.x)
	&& (*scanStringImp)(scanner, scanStringSel, @",", NULL)

	&& (*scanFloatImp)(scanner, scanFloatSel, &rect.origin.y)
	&& (*scanStringImp)(scanner, scanStringSel, @"}", NULL)
	&& (*scanStringImp)(scanner, scanStringSel, @",", NULL)
	
	&& (*scanStringImp)(scanner, scanStringSel, @"{", NULL)
	&& (*scanFloatImp)(scanner, scanFloatSel, &rect.size.width)
	&& (*scanStringImp)(scanner, scanStringSel, @",", NULL)
	
	&& (*scanFloatImp)(scanner, scanFloatSel, &rect.size.height)
	&& (*scanStringImp)(scanner, scanStringSel, @"}", NULL)
	&& (*scanStringImp)(scanner, scanStringSel, @"}", NULL))
	{
	  return rect;
	}
      else
	{
	  return NSMakeRect(0, 0, 0, 0);
	}
    }
}

/* Tests for equality of floats/doubles.
 * WARNING assumes the values are in the standard IEEE format ...
 * this may not be true on all systems, though afaik it is the case
 * on all systems we target.
 *
 * We use integer arithmetic for speed, assigning the float/double
 * to an integer of the same size and then converting any negative
 * values to twos-complement integer values so that a simple integer
 * comparison can be done.
 *
 * MAX_ULP specified the number of Units in the Last Place by which
 * the two values may differ and still be considered equal.  A value
 * of zero means that the two numbers must be identical.
 *
 * The way that infinity is represented means that it will be considered
 * equal to MAX_FLT (or MAX_DBL) unless we are doing an exact comparison
 * with MAX_ULP set to zero.
 *
 * The implementation will also treat two NaN values as being equal, which
 * is technically wrong ... but is it worth adding a check for that?
 */
#define	MAX_ULP	0
static inline BOOL
almostEqual(CGFloat A, CGFloat B)
{
#if	MAX_ULP == 0
  return (A == B) ? YES : NO;
#else	/* MAX_UPL == 0 */
#if	defined(CGFLOAT_IS_DBL)
  union {int64_t i; double d;} valA, valB;

  valA.d = A;
  valB.d = B;
#if	GS_SIZEOF_LONG == 8
  if (valA.i < 0)
    {
      valA.i = 0x8000000000000000L - valA.i;
    }
  if (valB.i < 0)
    {
      valB.i = 0x8000000000000000L - valB.i;
    }
  if (labs(valA.i - valB.i) <= MAX_ULP)
    {
      return YES;
    }
#else	/* GS_SIZEOF_LONG == 8 */
  if (valA.i < 0)
    {
      valA.i = 0x8000000000000000LL - valA.i;
    }
  if (valB.i < 0)
    {
      valB.i = 0x8000000000000000LL - valB.i;
    }
  if (llabs(valA.i - valB.i) <= MAX_ULP)
    {
      return YES;
    }
#endif	/* GS_SIZEOF_LONG == 8 */
  return NO;
#else	/* DEFINED(CGFLOAT_IS_DBL) */
  union {int32_t i; float f;} valA, valB;

  valA.f = A;
  if (valA.i < 0)
    {
      valA.i = 0x80000000 - valA.i;
    }
  valB.f = B;
  if (valB.i < 0)
    {
      valB.i = 0x80000000 - valB.i;
    }
  if (abs(valA.i - valB.i) <= MAX_ULP)
    {
      return YES;
    }
  return NO;
#endif	/* DEFINED(CGFLOAT_IS_DBL) */
#endif	/* MAX_UPL == 0 */
}

BOOL
NSEqualRects(NSRect aRect, NSRect bRect)
{
  return (almostEqual(NSMinX(aRect), NSMinX(bRect))
    && almostEqual(NSMinY(aRect), NSMinY(bRect))
    && almostEqual(NSWidth(aRect), NSWidth(bRect))
    && almostEqual(NSHeight(aRect), NSHeight(bRect))) ? YES : NO;
}

BOOL
NSEqualSizes(NSSize aSize, NSSize bSize)
{
  return (almostEqual(aSize.width, bSize.width)
    && almostEqual(aSize.height, bSize.height)) ? YES : NO;
}

BOOL
NSEqualPoints(NSPoint aPoint, NSPoint bPoint)
{
  return (almostEqual(aPoint.x, bPoint.x)
    && almostEqual(aPoint.y, bPoint.y)) ? YES : NO;
}

BOOL
NSEdgeInsetsEqual(NSEdgeInsets e1, NSEdgeInsets e2)
{
  return (
    almostEqual(e1.top, e2.top)
    && almostEqual(e1.left, e2.left)
    && almostEqual(e1.bottom, e2.bottom)
    && almostEqual(e1.right, e2.right)
  );
}

