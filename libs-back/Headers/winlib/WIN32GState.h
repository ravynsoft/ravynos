/* WIN32GState - Implements graphic state drawing for MSWindows

   Copyright (C) 2002 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2002
   
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

#ifndef _WIN32GState_h_INCLUDE
#define _WIN32GState_h_INCLUDE

#include "gsc/GSGState.h"

#include "windows.h"

@interface WIN32GState : GSGState
{
@public
  HWND window;
  COLORREF wfcolor;
  COLORREF wscolor;
  int joinStyle;
  int lineCap;
  float lineWidth;
  float miterlimit;
  HRGN clipRegion;

  HGDIOBJ oldBrush;
  HGDIOBJ oldPen;
  HRGN oldClipRegion;
  XFORM oldWorldTransform;
}

- (void) setWindow: (HWND)number;
- (HWND) window;
// declared to fix conflicting type with super class warning during compile 
- (void) compositeGState: (WIN32GState *) source
		fromRect: (NSRect) sourceRect
		 toPoint: (NSPoint) destPoint
		      op: (NSCompositingOperation) op
		fraction: (float)delta;

@end

#endif /* _WIN32GState_h_INCLUDE */
