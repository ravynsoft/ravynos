/* XGGeometry - Utility functions that calculate in device space
   
   Copyright (C) 2002 Free Software Foundation, Inc.

   Written by: Willem Oudshoorn <woudshoo@xs4all.nl>
   
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
#include <Foundation/NSDebug.h>
#include "xlib/XGGeometry.h"
#include "x11/XGServer.h"

/**
 * Returns the area that is accessible in the underlying drawable
 * of win.
 *
 * Accessible means that we can use XPutPixel and XGetPixel
 * on all the points in the result.
 * If the window uses a backingstore this will be the size
 * of the underlying drawable, but if the window does not
 * use backing store and the window is partly outside the screen
 * it will be the part of the window that falls inside the screen.
 *
 * NOTE:
 * Unfortunately, the gswindow_device_t does not contain a reference
 * to the XDisplay it is displayed on and we need it.
 */
XRectangle
accessibleRectForWindow (gswindow_device_t* win)
{
    Display* xdpy = [XGServer xDisplay];
    Window root;
    Window ignoreWindow;
    int x, y;
    unsigned int w, h;
    int ignoreInt;
    unsigned int ignoreUInt;
    XRectangle winRect;
    
    if (!XGetGeometry (xdpy, GET_XDRAWABLE (win),
                       &root,
                       &x, &y,
                       &w, &h,
                       &ignoreUInt,
                       &ignoreUInt))
      {
        NSDebugLLog (@"XGGeometry", @"invalid Drawable in gswindow_device");
        return XGMakeRect (0, 0, 0, 0);
      }

    winRect = XGMakeRect (0, 0, w, h);

    if (win->buffer)
      {
        return winRect;
      }
    
    // we do not have backing store, so clip it to the screen.
    if (!XGetGeometry (xdpy, root,
                       &root,
                       &ignoreInt, &ignoreInt,
                       &w, &h,
                       &ignoreUInt, &ignoreUInt))
      {
        NSDebugLLog (@"XGGeometry",  @"could not determine size of root");
        return XGMakeRect (0, 0, 0, 0);
      }

    if (!XTranslateCoordinates (xdpy,
                                root,
                                GET_XDRAWABLE (win),
                                0, 0,
                                &x, &y,
                                &ignoreWindow))
      {
        NSDebugLLog (@"XGGeometry", @"could not determine position of device");
        return XGMakeRect (0, 0, 0, 0);
      }
    
    return XGIntersectionRect (winRect, XGMakeRect (x, y, w, h));
}


/**
 *
 * POST CONDITIONS
 * winA and winB are unmodified
 * rectA and rectB have the same size
 * rectA is an accessible rectangle in winA
 * rectB is an accessible rectangle in winB
 * rectA is a subrectangle of the argument rectA 
 * rectB is a subrectangle of the argument rectB
 * the size of RectA and rectB are maximal with respect to the conditions above.
 *
 * USAGE
 * typical usage will be in copy operations between one gswindow_device to
 * another gswindow_device.  This because the result will be the maximal
 * region that we are able to copy without generating X-protocol errors
 * or segfaults.
 */
void
clipXRectsForCopying (gswindow_device_t* winA, XRectangle* rectA,
                      gswindow_device_t* winB, XRectangle* rectB)
{
  XPoint shiftA, shiftB;
  // First make A smaller.
  shiftA.x = rectA->x;
  shiftA.y = rectA->y;
  *rectA = XGIntersectionRect (*rectA, accessibleRectForWindow (winA));
  // update size of B with the size of A
  rectB->x += rectA->x - shiftA.x;
  rectB->y += rectA->y - shiftA.y;
  rectB->width = MIN (rectA->width, rectB->width);
  rectB->height = MIN (rectA->height, rectB->height);
  // now make B smaller
  shiftB.x = rectB->x;
  shiftB.y = rectB->y;
  *rectB = XGIntersectionRect (*rectB, accessibleRectForWindow (winB));
  // and update size of A with size of B
  rectA->x += rectB->x - shiftB.x;
  rectA->y += rectB->y - shiftB.y;
  rectA->width = rectB->width;
  rectA->height = rectB->height;
}
