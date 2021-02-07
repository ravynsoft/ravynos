/* <title>XGFontSetFontInfo</title>

   <abstract>NSFont helper for GNUstep X/GPS Backend</abstract>

   Copyright (C) 2003-2005 Free Software Foundation, Inc.

   Author: Kazunobu Kuriyama <kazunobu.kuriyama@nifty.com>
   Date: July 2003
   
   This file is part of the GNU Objective C User Interface library.

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

#ifndef __XGFontSetFontInfo_h
#define __XGFontSetFontInfo_h

#include <X11/Xlib.h>
#import <GNUstepGUI/GSFontInfo.h>

#ifdef X_HAVE_UTF8_STRING

#if 0 // Commented out till the implementation completes.
// ----------------------------------------------------------------------------
//  XGFontSetEnumerator
// ----------------------------------------------------------------------------
@interface XGFontSetEnumerator : GSFontEnumerator
{
}

- (void) enumerateFontsAndFamilies;

@end // XGFontSetEnumerator : GSFontEnumerator
#endif // #if 0


// ----------------------------------------------------------------------------
//  XGFontSetFontInfo
// ----------------------------------------------------------------------------
@interface XGFontSetFontInfo : GSFontInfo
{
    XFontSet	_font_set;
    XFontStruct	**_fonts;
    int		_num_fonts;
}

- (id) initWithFontName: (NSString*)name
		 matrix: (const CGFloat*)matrix
	     screenFont: (BOOL)screenFont;
- (void) drawGlyphs: (const NSGlyph*)glyphs
             length: (int)len
          onDisplay: (Display*)dpy
	   drawable: (Drawable)win
	       with: (GC)gc
	         at: (XPoint)xp;
- (CGFloat) widthOfGlyphs: (const NSGlyph*)glyphs
                 length: (int)len;
- (void) setActiveFor: (Display*)dpy
                   gc: (GC)gc;

@end // XGFontSetFontInfo : GSFontInfo

#endif // X_HAVE_UTF8_STRING defined
#endif // __XGFontSetFontInfo_h
