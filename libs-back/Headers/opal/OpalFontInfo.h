/*
   OpalFontInfo.h

   Copyright (C) 2013 Free Software Foundation, Inc.

   Author: Ivan Vucica <ivan@vucica.net>
   Date: June 2013

   This file is part of GNUstep.

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

#ifndef OpalFontInfo_h_defined
#define OpalFontInfo_h_defined

#include "fontconfig/FCFontInfo.h"
#include "opal/OpalFaceInfo.h"
#if 0
#include <cairo.h>
#endif
@interface OpalFontInfo : FCFontInfo
{
@public
#if 0
	cairo_scaled_font_t *_scaled;
#endif
}
/*
- (id) initWithFontName: (NSString *)name 
                 matrix: (const CGFloat *)fmatrix 
             screenFont: (BOOL)p_screenFont;
*/
#if 0
- (void) drawGlyphs: (const NSGlyph*)glyphs
	     length: (int)length 
	         on: (cairo_t*)ct;
#endif
@end

#endif


