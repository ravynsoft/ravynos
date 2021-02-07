/* Win32FontInfo - Implements font enumerator for MSWindows

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

#ifndef _WIN32FontInfo_h_INCLUDE
#define _WIN32FontInfo_h_INCLUDE

#include "windows.h"
#include "GNUstepGUI/GSFontInfo.h"

@interface WIN32FontInfo : GSFontInfo
{
  HFONT hFont;
}

- (void) draw: (const char*)s length: (int)len 
	 onDC: (HDC)hdc at: (POINT)p;
- (void) drawGlyphs: (const NSGlyph*)s
	     length: (int)len 
	       onDC: (HDC)hdc
		 at: (POINT)p;
@end

#endif/* _WIN32FontInfo_h_INCLUDE */


