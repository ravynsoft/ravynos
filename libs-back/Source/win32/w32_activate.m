/* WIN32Server - Implements window handling for MSWindows

   Copyright (C) 2005 Free Software Foundation, Inc.

   Written by: Tom MacSween <macsweent@sympatico.ca>
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

@implementation WIN32Server (w32_activate)

- (LRESULT) decodeWM_ACTIVEParams:(WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  // decode our params
  int last_WM_ACTIVATE = LOWORD(wParam);
  //int minimized = HIWORD(wParam);

  switch (last_WM_ACTIVATE)
    {
      case WA_ACTIVE:
        {
	  /* we become the inactivation event on WM_INACTIVE */
	  /* Sending another leave event might confuse the front event */
	  currentActive = hwnd;
        }
        break;
      case WA_CLICKACTIVE:
        {
	  /* we become the inactivation event on WM_INACTIVE */
	  /* Sending another leave event might confuse the front event */
	  currentActive = hwnd;
        }
        break;
      case WA_INACTIVE:
        {
	  NSEvent *ev;
      
	  ev = [NSEvent otherEventWithType: NSAppKitDefined
		      	          location: NSMakePoint(0, 0)
			     modifierFlags: 0
			         timestamp: 0
			      windowNumber: (int)hwnd
			           context: GSCurrentContext()
			           subtype: GSAppKitWindowLeave
			             data1: 0
			             data2: 0];
      
	  [EVENT_WINDOW(hwnd) sendEvent: ev];
        }
        break;
	
      default:
        break;
    }
  
  return 0;
}

- (LRESULT) decodeWM_ACTIVEAPPParams: (HWND)hwnd : (WPARAM)wParam 
                                    : (LPARAM)lParam
{
  switch ((int)wParam)
    {
      case TRUE:
        {
          [NSApp activateIgnoringOtherApps: YES];
          flags._eventHandled = YES;
        }
        break;
      case FALSE:
        {
// FIXME: We should deactivate the application, however the following
// causes problems. See "Re: Problem with recent change to w32_activate.m
// -- PLEASE REVERT" for details.
//          [NSApp deactivate];
//          flags._eventHandled = YES;
        }            
        break;
              
      default:
        break;            
    }
            
  return 0;
}

- (void) decodeWM_NCACTIVATEParams: (WPARAM)wParam : (LPARAM)lParam 
                                  : (HWND)hwnd
{

}

@end
