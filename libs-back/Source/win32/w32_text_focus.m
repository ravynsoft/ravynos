/* WIN32Server - Implements window handling for MSWindows

   Copyright (C) 2005 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2002
   Part of this code have been re-written by:
   Tom MacSween <macsweent@sympatico.ca>
   Date August 2005

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

#include <AppKit/NSEvent.h>
#include <AppKit/NSWindow.h>
#include "win32/WIN32Server.h"
#include "win32/WIN32Geometry.h"

@implementation WIN32Server (w32_text_focus)

- (LRESULT) decodeWM_SETFOCUSParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  /* This message comes when the window already got focus, so we send a focus
     in event to the front end, but also mark the window as having current focus
     so that the front end doesn't try to focus the window again. */
   
  currentFocus = hwnd;
  if (currentFocus == desiredFocus)
    {
      /* This was from a request from the front end. Mark as done. */
      desiredFocus = 0;
    }
  else
    {
      /* We need to do this directly and not send an event to the frontend - 
         that's too slow and allows the window state to get out of sync, 
         causing bad recursion problems */

      NSEvent *ev;
      
      ev = [NSEvent otherEventWithType: NSAppKitDefined
                              location: NSMakePoint(0, 0)
                         modifierFlags: 0
                             timestamp: 0
                          windowNumber: (int)hwnd
                               context: GSCurrentContext()
                               subtype: GSAppKitWindowFocusIn
                                 data1: 0
                                 data2: 0];
      
      NSDebugLLog(@"Focus", @"Making %d key", (int)hwnd);
      [EVENT_WINDOW(hwnd) sendEvent: ev];
    }
    
  return 0;
}

- (void) decodeWM_KILLFOCUSParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  NSEvent *ev;

  // Remember the now focused window
  currentFocus = (HWND) wParam;

  ev = [NSEvent otherEventWithType: NSAppKitDefined
			  location: NSMakePoint(0, 0)
		     modifierFlags: 0
			 timestamp: 0
		      windowNumber: (int)hwnd
			   context: GSCurrentContext()
			   subtype: GSAppKitWindowFocusOut
			     data1: 0
			     data2: 0];
		      
  [EVENT_WINDOW(hwnd) sendEvent: ev];
  flags._eventHandled = YES;
}

- (void) decodeWM_GETTEXTParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  // stub for future dev
}

/*
- (LRESULT) decodeWM_SETTEXTParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  printf("WM_SETTEXT\n");
  printf("Window text is: %s\n", (LPSTR)lParam);
  
  if (SetWindowText(hwnd, (LPSTR)lParam) == 0)
    printf("error on setWindow text %ld\n", GetLastError());
  
  return 0;
}
*/

@end
