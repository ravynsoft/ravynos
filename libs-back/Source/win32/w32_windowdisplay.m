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
#include <AppKit/NSView.h>
#include <AppKit/NSWindow.h>
#include "win32/WIN32Server.h"
#include "win32/WIN32Geometry.h"

static void 
invalidateWindow(WIN32Server *svr, HWND hwnd, RECT rect)
{
  WIN_INTERN *win = (WIN_INTERN *)GetWindowLong((HWND)hwnd, GWL_USERDATA);

  if (!win->useHDC || win->backingStoreEmpty)
    {
      NSWindow *window = GSWindowWithNumber((int)hwnd);
      NSRect r = MSWindowRectToGS(svr, hwnd, rect);
      
      /*
        NSLog(@"Invalidated window %d %@ (%d, %d, %d, %d)", hwnd, 
        NSStringFromRect(r), rect.left, rect.top, rect.right, rect.bottom);
      */

      /* Repaint the windows's client area */
      [[[window contentView] superview] setNeedsDisplayInRect: r];
      // FIXME: We should never do a direct draw call here!
      [[[window contentView] superview] displayIfNeeded];
      win->backingStoreEmpty = NO;
    }

#if (BUILD_GRAPHICS==GRAPHICS_winlib)
  if (win->useHDC)
    {
      HDC hdc = GetDC((HWND)hwnd);
      WINBOOL result;

      result = BitBlt(hdc, rect.left, rect.top, 
		      (rect.right - rect.left), (rect.bottom - rect.top), 
		      win->hdc, rect.left, rect.top, SRCCOPY);
      if (!result)
        {
          NSWarnMLog(@"validateWindow failed %d", GetLastError());
        }
      ReleaseDC((HWND)hwnd, hdc);
    }
#endif
}

@implementation WIN32Server (w32_windowdisplay)

- (void) decodeWM_SHOWWINDOWParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  //SW_OTHERUNZOOM //window is being uncovered
  //SW_OTHERZOOM  //window is being covered by window that has maximized. 
  //SW_PARENTCLOSING // window's owner window is being minimized.
  //SW_PARENTOPENING //The window's owner window is being restored.
  //zero - 0  //call to the ShowWindow function

  switch ((int)wParam)
    {
    case TRUE:
      {
	switch ((int)lParam) 
            
	  {            
	  case 0:
	    {
	      ShowWindow(hwnd, SW_SHOW);
	      flags._eventHandled=YES;
	    }
	    break;
	  case SW_PARENTCLOSING:
	    {
	      ShowWindow(hwnd, SW_SHOW);
	      flags._eventHandled=YES;
	    }
	    break;
                
	  default:
	    break;
	  }
        
      }
      break;
        
    case FALSE:
      {
      }
      break;
        
    default:
      break;
    }
}

- (void) decodeWM_NCPAINTParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
}

- (LRESULT) decodeWM_ERASEBKGNDParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  // GS handles this for now...
  return (LRESULT)1;
}

- (void) decodeWM_PAINTParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  RECT rect;
  if (GetUpdateRect(hwnd, &rect, TRUE))
    {
      NSRect r = MSWindowRectToGS(self, hwnd, rect);
      NSWindow *window = GSWindowWithNumber((int)hwnd);
      [[[window contentView] superview] setNeedsDisplayInRect: r];
    }

  //flags._eventHandled = YES; -->DefWindowProc validates the event
}

- (void) decodeWM_SYNCPAINTParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  // stub for future dev
}


- (void) decodeWM_CAPTURECHANGEDParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  // stub for future dev
}

- (HICON) decodeWM_GETICONParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  // stub for future dev
  return currentAppIcon;
}

- (HICON) decodeWM_SETICONParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  return currentAppIcon;
}

@end
