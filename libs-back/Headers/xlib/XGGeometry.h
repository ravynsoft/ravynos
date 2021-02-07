/* -*- mode: ObjC -*-

   <title>XGGeometry</title>

   <abstract>Point and rectangle manipulations for X-structures</abstract>
   
   Copyright (C) 2001-2005 Free Software Foundation, Inc.

   Written by:  <author name="Wim Oudshoorn">
   <email>woudshoo@xs4all.nl</email></author>
   Date: Nov, 2001
   
   This file is part of the GNU Objective C User Interface Library.

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

/*
  This file implements the NSGeometry manipulation functions for
  the XPoint and XRectangle structures.

  So most code is copied from NSGeometry, with only the structs changed
*/

#ifndef _XGGeometry_h_INCLUDE
#define _XGGeometry_h_INCLUDE

#include <AppKit/NSAffineTransform.h>
#include <X11/Xlib.h>
#include "xlib/XGGState.h"
#include "x11/XGServerWindow.h"

XRectangle
accessibleRectForWindow (gswindow_device_t *win);

void
clipXRectsForCopying (gswindow_device_t* winA, XRectangle* rectA,
                      gswindow_device_t* winB, XRectangle* rectB);

static inline XPoint
XGMakePoint (short x, short y)
{
  XPoint p;

  p.x = x;
  p.y = y;

  return p;
}

static inline XRectangle
XGMakeRect (short x, short y, unsigned short w, unsigned short h)
{
  XRectangle rect;

  rect.x = x;
  rect.y = y;
  rect.width = w;
  rect.height = h;

  return rect;
} 


static inline short
XGMinX (XRectangle aRect)
{
  return aRect.x;
}

static inline short
XGMinY (XRectangle aRect)
{
  return aRect.y;
}


static inline short
XGMaxX (XRectangle aRect)
{
  return aRect.x + aRect.width;
}

static inline short
XGMaxY (XRectangle aRect)
{
  return aRect.y + aRect.height;
}

static inline short
XGWidth (XRectangle aRect)
{
  return aRect.width;
}

static inline short
XGHeight (XRectangle aRect)
{
  return aRect.height;
}


static inline XRectangle
XGIntersectionRect (XRectangle aRect, XRectangle bRect)
{
  if (XGMaxX (aRect) <= XGMinX (bRect) || XGMaxX (bRect) <= XGMinX (aRect)
    || XGMaxY (aRect) <= XGMinY (bRect) || XGMaxY (bRect) <= XGMinY (aRect))
    {
      return XGMakeRect (0, 0, 0, 0);
    }
  else
    {
      XRectangle rect;

      if (XGMinX (aRect) <= XGMinX (bRect))
        rect.x = bRect.x;
      else
        rect.x = aRect.x;

      if (XGMaxX (aRect) >= XGMaxX (bRect))
        rect.width = XGMaxX (bRect) - rect.x;
      else
        rect.width = XGMaxX (aRect) - rect.x;

      if (XGMinY (aRect) <= XGMinY (bRect))
        rect.y = bRect.y;
      else
        rect.y = aRect.y;

      if (XGMaxY (aRect) >= XGMaxY (bRect))
        rect.height = XGMaxY (bRect) - rect.y;
      else
        rect.height = XGMaxY (aRect) - rect.y;

      return rect;
    }
}


static inline BOOL
XGIsEmptyRect (XRectangle aRect)
{
  if (aRect.width == 0 || aRect.height == 0)
    return YES;
  else
    return NO;
}


// Just in case this are not defined on a system
#ifndef SHRT_MAX
#define SHRT_MAX 32767
#endif 
#ifndef SHRT_MIN
#define SHRT_MIN (-32768)
#endif 

/* Quick floor using C casts <called gs_floor only to avoid clashes if
   math.h is included>. This casts to short as this is the type X uses 
   for all geometry. */
static inline short gs_floor (float f)
{  
  if (f >= 0)
    {
      if (f > SHRT_MAX)
	return SHRT_MAX;
      else 
	return (short)f;
    }
  else
    {
      if (f < SHRT_MIN)
	return SHRT_MIN;
      else
	{
	  int g = (int)f;
      
	  if (f - ((float)g) > 0)
	    {
	      return g - 1;
	    }
	  else
	    {
	      return g;
	    }
	}
    }
}

/*
 *	Inline functions to convert from OpenStep view coordinates or
 *	OpenStep window coordinates to X window coordinates.
 */
static inline XPoint
XGWindowPointToX (XGGState *s, NSPoint p)
{
  XPoint newPoint;

  newPoint.x = gs_floor(p.x - s->offset.x);
  newPoint.y = gs_floor(s->offset.y - p.y);

  return newPoint;
}

static inline XRectangle
XGWindowRectToX (XGGState *s, NSRect r)
{
  XRectangle newRect;

  newRect.x = gs_floor(r.origin.x - s->offset.x);
  /* We gs_floor the extreme points, and get the width as the difference */
  newRect.width = gs_floor(r.origin.x - s->offset.x + r.size.width) 
                  - newRect.x;

  newRect.y = gs_floor(s->offset.y - r.origin.y - r.size.height);
  newRect.height = gs_floor(s->offset.y - r.origin.y) - newRect.y;

  return newRect;
}

/*
 *	Inline functions to convert from OpenStep view coordinates or
 *	OpenStep window coordinates to X window coordinates.
 */

static inline XPoint
XGViewPointToX(XGGState *s, NSPoint p)
{
  p = [s->ctm transformPoint: p];
  return XGWindowPointToX(s, p);
}

static inline XRectangle
XGViewRectToX(XGGState *s, NSRect r)
{
  r = [s->ctm rectInMatrixSpace: r];
  return XGWindowRectToX(s, r);
}

#endif  /* _XGGeometry_h_INCLUDE */
