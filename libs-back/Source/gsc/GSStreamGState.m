/* GSStreamGState - PS Stream graphic state

   Copyright (C) 2002 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Sep 2002
   
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

#include "config.h"
#include <Foundation/NSObjCRuntime.h>
#include <AppKit/NSAffineTransform.h>
#include <AppKit/NSBezierPath.h>
#include <AppKit/NSColor.h>
#include <AppKit/NSFont.h>
#include <AppKit/NSGraphics.h>
#include "gsc/GSContext.h"
#include "gsc/GSStreamGState.h"
#include "math.h"

@implementation GSStreamGState

/* Designated initializer. */
- initWithDrawContext: (GSContext *)drawContext
{
  [super initWithDrawContext: drawContext];

  clinecap = 0;
  clinejoin = 0;
  clinewidth = 0;
  cmiterlimit = 0;
  cstrokeadjust = 0;
  return self;
}

- (void) DPScurrentlinecap: (int*)linecap
{
  if (linecap)
    *linecap = clinecap;
}

- (void) DPScurrentlinejoin: (int*)linejoin
{
  if (linejoin)
    *linejoin = clinejoin;
}

- (void) DPScurrentlinewidth: (CGFloat*)width
{
  if (width)
    *width = clinewidth;
}

- (void) DPScurrentmiterlimit: (CGFloat*)limit
{
  if (limit)
    *limit = cmiterlimit;
}

- (void) DPScurrentstrokeadjust: (int*)b
{
  if (b)
    *b = cstrokeadjust;
}

- (void) DPSsetlinecap: (int)linecap
{
  clinecap = linecap;
}

- (void) DPSsetlinejoin: (int)linejoin
{
  clinejoin = linejoin;
}

- (void) DPSsetlinewidth: (CGFloat)width
{
  clinewidth = width;
}

- (void) DPSsetmiterlimit: (CGFloat)limit
{
  cmiterlimit = limit;
}

- (void) DPSsetstrokeadjust: (int)b
{
  cstrokeadjust = b;
}

@end
