/* WIN32Server - Implements window handling for MSWindows

   Copyright (C) 2005 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2002
   Part of this code have been written by:
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
#include <GNUstepGUI/GSTheme.h>

@implementation WIN32Server (w32_General)

- (void) decodeWM_CLOSEParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  NSEvent *ev;
  NSPoint eventLocation = NSMakePoint(0, 0);

  ev = [NSEvent otherEventWithType: NSAppKitDefined
		      location: eventLocation
		      modifierFlags: 0
		      timestamp: 0
		      windowNumber: (int)hwnd
		      context: GSCurrentContext()
		      subtype: GSAppKitWindowClose
		      data1: 0
		      data2: 0];
		    
  // Sending the event directly to the window bypasses the event queue,
  // which can cause a modal loop to lock up.
  [GSCurrentServer() postEvent: ev atStart: NO];
  
  flags._eventHandled = YES;
}
      
- (void) decodeWM_NCDESTROYParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
}

- (void) decodeWM_DESTROYParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  WIN_INTERN *win = (WIN_INTERN *)GetWindowLong(hwnd, GWL_USERDATA);

  // Clean up window-specific data objects. 
	
  if (win->useHDC)
    {
      HGDIOBJ old;
	    
      old = SelectObject(win->hdc, win->old);
      DeleteObject(old);
      DeleteDC(win->hdc);
    }
  free(win);
  free((IME_INFO_T*)GetWindowLongPtr(hwnd, IME_INFO));
  flags._eventHandled=YES;
}

- (void) decodeWM_QUERYOPENParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
}

- (void) decodeWM_SYSCOMMANDParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  // stubbed for future development

   switch (wParam)
   {
      case SC_CLOSE:
      break;
      case SC_CONTEXTHELP:
      break;
      case SC_HOTKEY:
      break;
      case SC_HSCROLL:
      break;
      case SC_KEYMENU:
      break;
      case SC_MAXIMIZE:
      break;
      case SC_MINIMIZE:
       flags.HOLD_MINI_FOR_SIZE=TRUE;
      break;
      case SC_MONITORPOWER:
      break;
      case SC_MOUSEMENU:
      break;
      case SC_MOVE:
      break;
      case SC_NEXTWINDOW:
      break;  
      case SC_PREVWINDOW:
      break;
      case SC_RESTORE:
      break;
      case SC_SCREENSAVE:
      break;
      case SC_SIZE:
      break;
      case SC_TASKLIST:
      break;
      case SC_VSCROLL:
      break;
        
      default:
      break;
   }
}

- (void) decodeWM_COMMANDParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  [[GSTheme theme] processCommand: (void *)wParam];
} 

- (void) decodeWM_THEMECHANGEDParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  // Reactivate the theme when the host system changes it's theme...
  [[GSTheme theme] activate];
}
@end 
