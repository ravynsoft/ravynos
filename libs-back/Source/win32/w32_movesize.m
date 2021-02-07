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

@implementation WIN32Server (w32_movesize)

- (LRESULT) decodeWM_MOVEParams:(HWND)hwnd : (WPARAM)wParam : (LPARAM)lParam
{
  if (flags.HOLD_MINI_FOR_SIZE == FALSE)
    {
      NSPoint eventLocation;
      NSRect rect;
      RECT r;
      NSEvent *ev = nil;
      
      GetWindowRect(hwnd, &r);
      rect = MSScreenRectToGS(r);
      eventLocation = rect.origin;
      
      ev = [NSEvent otherEventWithType: NSAppKitDefined
			      location: eventLocation
			 modifierFlags: 0
			     timestamp: 0
			  windowNumber: (int)hwnd
			       context: GSCurrentContext()
			       subtype: GSAppKitWindowMoved
				 data1: rect.origin.x
				 data2: rect.origin.y];                   
      
      
      //need native code here?
      [EVENT_WINDOW(hwnd) sendEvent: ev];
    }
  
  return 0;
}

- (LRESULT) decodeWM_SIZEParams:(HWND)hwnd : (WPARAM)wParam : (LPARAM)lParam
{
  switch ((int)wParam)
    {
      case SIZE_MAXHIDE:
        {
          // stubbed for future development
        }
        break;

      case SIZE_MAXSHOW:
        {
          // stubbed for future development
        }
        break;

      case SIZE_MINIMIZED:
        {
          if (flags.HOLD_MINI_FOR_SIZE == TRUE) // this is fix for [5, 25 bug]
            break;
	    
          [EVENT_WINDOW(hwnd) miniaturize: self]; 
          break;
        }
        break;

      case SIZE_MAXIMIZED:
      case SIZE_RESTORED:
        {
          NSPoint eventLocation;
          NSRect rect;
          RECT r;
          NSEvent *ev =nil;
          
          GetWindowRect(hwnd, &r);
          rect = MSScreenRectToGS(r);
          eventLocation = rect.origin;
          
          // make event
          ev = [NSEvent otherEventWithType: NSAppKitDefined
                                  location: eventLocation
                             modifierFlags: 0
                                 timestamp: 0
                              windowNumber: (int)hwnd
                                   context: GSCurrentContext()
                                   subtype: GSAppKitWindowResized
                                     data1: rect.size.width
                                     data2: rect.size.height];
                    
          [EVENT_WINDOW(hwnd) sendEvent: ev];
          [self resizeBackingStoreFor: hwnd];
          // fixes part one of bug [5, 25] see notes
          if ([self usesNativeTaskbar])
            [EVENT_WINDOW(hwnd) deminiaturize:self];
        }
        break;
	
      default:
        break;
    }
                      
  flags.HOLD_MINI_FOR_SIZE = FALSE;

  return 0;
}

- (void) decodeWM_NCCALCSIZEParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
#if 0
  DefWindowProc(hwnd, WM_NCCALCSIZE, wParam, lParam);
  flags._eventHandled = YES;
#endif
  return;
}

- (void) decodeWM_WINDOWPOSCHANGEDParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  WINDOWPOS	*inf = (WINDOWPOS*)lParam;

  if ((inf->flags & SWP_NOZORDER) == 0)
    {
      HWND		hi;
      HWND		lo;
      int		hl;
      int		ll;

      /* For debugging, log current window stack.
       */
      if (GSDebugSet(@"WTrace") == YES)
        {
          NSString	*s = @"Window list:\n";

          hi = GetDesktopWindow();
          hi = GetWindow(hi, GW_CHILD);
          if (hi > 0)
            {
              hi = GetWindow(hi, GW_HWNDLAST);
            }
          while (hi > 0)
            {
              TCHAR	buf[32];

              hi = GetNextWindow(hi, GW_HWNDPREV);

              if (hi > 0
                  && GetClassName(hi, buf, 32) == 18
                  && strncmp(buf, "GNUstepWindowClass", 18) == 0)
                {
                  if (GetWindowLong(hi, OFF_ORDERED) == 1)
                    {
                      hl = GetWindowLong(hi, OFF_LEVEL);
                      s = [s stringByAppendingFormat: @"%d (%d)\n", hi, hl];
                    }
                }
            }
          NSLog(@"window pos changed: %@", s);
        }

      /* This window has changed its z-order, so we take the opportunity
       * to check that all the GNUstep windows are in the correct order
       * of their levels.
       * We do this as a simple bubble sort ...swapping over a pait of
       * windows if we find any pair in the list which are in the wrong
       * order.  This sort has the virtue of being simple and of
       * maintaining window order within a level. ... but may be too slow.
       */
      hi = GetDesktopWindow();
      hi = GetWindow(hi, GW_CHILD);
      while (hi > 0)
        {
          TCHAR	buf[32];

          /* Find a GNUstep window which is ordered in and above desktop
           */
          while (hi > 0)
            {
              if (GetClassName(hi, buf, 32) == 18
                  && strncmp(buf, "GNUstepWindowClass", 18) == 0
                  && GetWindowLong(hi, OFF_ORDERED) == 1
                  && (hl = GetWindowLong(hi, OFF_LEVEL))
                  > NSDesktopWindowLevel)
                {
                  break;
                }
              hi = GetNextWindow(hi, GW_HWNDNEXT);
            }
        
          if (hi > 0)
            {
              NSDebugLLog(@"WTrace", @"sort hi %d (%d)", hi, hl);
              /* Find the next (lower in z-order) GNUstep window which
               * is ordered in and above desktop
               */
              lo = GetNextWindow(hi, GW_HWNDNEXT);
              while (lo > 0)
                {
                  if (GetClassName(lo, buf, 32) == 18
                      && strncmp(buf, "GNUstepWindowClass", 18) == 0
                      && GetWindowLong(lo, OFF_ORDERED) == 1
                      && (ll = GetWindowLong(lo, OFF_LEVEL))
                      > NSDesktopWindowLevel)
                    {
                      break;
                    }
                  lo = GetNextWindow(lo, GW_HWNDNEXT);
                }

              if (lo > 0)
                {
                  NSDebugLLog(@"WTrace", @"sort lo %d (%d)", lo, ll);
                  /* Check to see if the higher of the two windows should
                   * actually be lower.
                   */
                  if (hl < ll)
                    {
                      HWND	higher;

                      /* Insert the low window before the high one.
                       * ie after the window preceding the high one.
                       */
                      higher = GetNextWindow(hi, GW_HWNDPREV);
                      if (higher == 0)
                        {
                          higher = HWND_TOP;
                        }
                      NSDebugLLog(@"WTrace", @"swap %d (%d) with %d (%d)", hi, hl, lo, ll);
                      SetWindowPos(lo, higher, 0, 0, 0, 0,
                                   SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

                      /* Done  this iteration of the sort ... the next
                       * iteration takes place when we get notified
                       * that the swap we have just donew is complete.
                       */
                      break;
                    }
                }
              hi = lo;
            }
        }
    }
}

- (void) decodeWM_WINDOWPOSCHANGINGParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  WINDOWPOS *inf = (WINDOWPOS*)lParam;

  if ((inf->flags & SWP_NOZORDER) == 0)
    {
      /* desktop level windows should stay at the bottom of the
       * window list, so we can simply override any re-ordering
       * to ensure that they are at the bottom unless another
       * desktop level window is inserted below them.
       */
      if (GetWindowLong(hwnd, OFF_LEVEL) <= NSDesktopWindowLevel)
        {
          inf->hwndInsertAfter = HWND_BOTTOM;
        }
    }
}

- (LRESULT) decodeWM_GETMINMAXINFOParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  WIN_INTERN *win = (WIN_INTERN *)GetWindowLong(hwnd, GWL_USERDATA);
  MINMAXINFO *mm;

  if (win != NULL)
    {
      mm = (MINMAXINFO*)lParam;
      mm->ptMinTrackSize = win->minmax.ptMinTrackSize;
      mm->ptMaxTrackSize = win->minmax.ptMaxTrackSize;
      return 0;
    }
	  
  return 0; 
}

- (LRESULT) decodeWM_ENTERSIZEMOVEParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  return DefWindowProc(hwnd, WM_ENTERSIZEMOVE, wParam, lParam);
}

- (LRESULT) decodeWM_EXITSIZEMOVEParams: (WPARAM)wParam : (LPARAM)lParam : (HWND)hwnd
{
  return DefWindowProc(hwnd, WM_EXITSIZEMOVE, wParam, lParam);
}

- (LRESULT) decodeWM_SIZINGParams:(HWND)hwnd : (WPARAM)wParam : (LPARAM)lParam
{
  [EVENT_WINDOW(hwnd) displayIfNeeded];
  return TRUE;
}

- (LRESULT) decodeWM_MOVINGParams:(HWND)hwnd : (WPARAM)wParam : (LPARAM)lParam
{
  [EVENT_WINDOW(hwnd) display];
  return TRUE;
}

@end
