/* XGInputServer - Keyboard input handling

   Copyright (C) 2002 Free Software Foundation, Inc.

   Author: Adam Fedor <fedor@gnu.org>
   Date: January 2002

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

#ifndef _GNUstep_H_XGInputServer
#define _GNUstep_H_XGInputServer

#include <AppKit/NSInputServer.h>
#include <x11/XGServerWindow.h>

@protocol XInputFiltering
- (BOOL) filterEvent: (XEvent *)event;
- (NSString *) lookupStringForEvent: (XKeyEvent *)event 
			     window: (gswindow_device_t *)window
                             keysym: (KeySym *)keysymptr;
@end


@interface XIMInputServer: NSInputServer <XInputFiltering>
{
  id        delegate;
  NSString *server_name;
  XIM       xim;
  XIMStyle  xim_style;

  /* Track the XIC:s and destroy them explicitly to work around an XFree86
  bug:
  http://www.xfree86.org/pipermail/xpert/2002-June/018370.html
  */
  XIC      *xics;
  int       num_xics;
}

- (id) initWithDelegate: (id)aDelegate
		display: (Display *)dpy
		   name: (NSString *)name;
- (void) ximFocusICWindow: (gswindow_device_t *)windev;
- (void) ximCloseIC: (XIC)xic;

@end

// Public interface for the input methods
@interface XIMInputServer (InputMethod)
- (NSString *) inputMethodStyle;
- (NSString *) fontSize: (int *)size;
- (BOOL) clientWindowRect: (NSRect *)rect;

- (BOOL) statusArea: (NSRect *)rect;
- (BOOL) preeditArea: (NSRect *)rect;
- (BOOL) preeditSpot: (NSPoint *)p;

- (BOOL) setStatusArea: (NSRect *)rect;
- (BOOL) setPreeditArea: (NSRect *)rect;
- (BOOL) setPreeditSpot: (NSPoint *)p;
@end // XIMInputServer (InputMethod)

#endif
