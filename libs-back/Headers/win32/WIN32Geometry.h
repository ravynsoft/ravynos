/* WIN32Geometry - Implements coordinate transformations for MSWindows

   Copyright (C) 2002 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Date: April 2002
   
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

#ifndef _WIN32Geometry_h_INCLUDE
#define _WIN32Geometry_h_INCLUDE

#include <Foundation/NSGeometry.h>

#include <windows.h>

@class WIN32Server;

static inline NSPoint
MSWindowPointToGS(WIN32Server *svr, HWND hwnd, int x, int y)
{
  NSGraphicsContext *ctxt;
  RECT rect;
  float h, l, r, t, b;
  NSPoint p1;
  NSWindow *window;

  ctxt = GSCurrentContext();
  window = GSWindowWithNumber((int)hwnd);
  GetClientRect(hwnd, &rect);
  h = rect.bottom - rect.top;
  [svr styleoffsets: &l : &r : &t : &b : [window styleMask]];

  p1.x = x + l;
  p1.y = h - y + b;
  return p1;
}

static inline NSRect
MSWindowRectToGS(WIN32Server *svr, HWND hwnd, RECT r0)
{
  NSGraphicsContext *ctxt;
  RECT rect;
  float h, l, r, t, b;
  NSRect r1;
  NSWindow *window;

  ctxt = GSCurrentContext();
  window = GSWindowWithNumber((int)hwnd);
  GetClientRect(hwnd, &rect);
  h = rect.bottom - rect.top;
  [svr styleoffsets: &l : &r : &t : &b : [window styleMask]];

  r1.origin.x = r0.left + l;
  r1.origin.y = h - r0.bottom + b;
  r1.size.width = r0.right - r0.left;
  r1.size.height = r0.bottom - r0.top;
  return r1;
}

static inline RECT
GSWindowRectToMS(WIN32Server *svr, HWND hwnd, NSRect r0)
{
  NSGraphicsContext *ctxt;
  RECT rect;
  float h, l, r, t, b;
  RECT r1;
  NSWindow *window;

  ctxt = GSCurrentContext();
  window = GSWindowWithNumber((int)hwnd);
  GetClientRect(hwnd, &rect);
  h = rect.bottom - rect.top;
  [svr styleoffsets: &l : &r : &t : &b : [window styleMask]];

  r1.left = r0.origin.x - l;
  r1.bottom = h - r0.origin.y + b;
  r1.right = r1.left + r0.size.width;
  r1.top = r1.bottom - r0.size.height;
  return r1;
}


static inline
NSPoint MSWindowOriginToGS(HWND hwnd, int x, int y)
{
  NSPoint p1;
  RECT rect;
  int h;
  int screen_height = GetSystemMetrics(SM_CYSCREEN);

  GetWindowRect(hwnd, &rect);
  h = rect.bottom - rect.top;

  p1.x = x;
  p1.y = screen_height - y - h;
  return p1;
}

static inline
POINT GSWindowOriginToMS(HWND hwnd, NSPoint p)
{
  POINT p1;
  RECT rect;
  int h;
  int screen_height = GetSystemMetrics(SM_CYSCREEN);

  GetWindowRect(hwnd, &rect);
  h = rect.bottom - rect.top;

  p1.x = p.x;
  p1.y = screen_height - p.y - h;
  return p1;
}

static inline
NSPoint MSScreenPointToGS(int x, int y)
{
  NSPoint p1;
  int screen_height = GetSystemMetrics(SM_CYSCREEN);

  p1.x = x;
  p1.y = screen_height - y;
  return p1;
}

static inline
NSRect MSScreenRectToGS(RECT r)
{
  NSRect r1;
  int screen_height = GetSystemMetrics(SM_CYSCREEN);

  r1.origin.x = r.left;
  r1.origin.y = screen_height - r.bottom;
  r1.size.width = r.right - r.left;
  r1.size.height = r.bottom - r.top;

  return r1;
}

static inline
POINT GSScreenPointToMS(NSPoint p)
{
  POINT p1;
  int screen_height = GetSystemMetrics(SM_CYSCREEN);

  p1.x = p.x;
  p1.y = screen_height - p.y;
  return p1;
}

static inline
RECT GSScreenRectToMS(NSRect r)
{
  RECT r1;
  int screen_height = GetSystemMetrics(SM_CYSCREEN);

  r1.left = r.origin.x;
  r1.bottom = screen_height - r.origin.y;
  r1.right = r.origin.x + r.size.width;
  r1.top = screen_height - r.origin.y - r.size.height;

  return r1;
}


#endif /* _WIN32Geometry_h_INCLUDE */
