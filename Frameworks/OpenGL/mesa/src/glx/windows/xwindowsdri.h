/*
 * Copyright © 2014 Jon Turney
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef XWINDOWSDRI_H
#define XWINDOWSDRI_H

#include <X11/Xfuncproto.h>
#include <X11/Xlib.h>

typedef struct
{
   int type;                    /* of event */
   unsigned long serial;        /* # of last request processed by server */
   Bool send_event;             /* true if this came from a SendEvent request */
   Display *display;            /* Display the event was read from */
   Window window;               /* window of event */
   Time time;                   /* server timestamp when event happened */
   int kind;                    /* subtype of event */
   int arg;
} XWindowsDRINotifyEvent;

_XFUNCPROTOBEGIN
Bool XWindowsDRIQueryExtension(Display * dpy, int *event_base,
                              int *error_base);

Bool XWindowsDRIQueryVersion(Display * dpy, int *majorVersion,
                            int *minorVersion, int *patchVersion);

Bool XWindowsDRIQueryDirectRenderingCapable(Display * dpy, int screen,
                                          Bool *isCapable);

Bool XWindowsDRIQueryDrawable(Display * dpy, int screen, Drawable drawable,
                             unsigned int *type, void ** handle);

Bool XWindowsDRIFBConfigToPixelFormat(Display *dpy, int screen, int fbConfigID,
                                     int *pxfi);
_XFUNCPROTOEND

#endif /* XWINDOWSDRI_H */
