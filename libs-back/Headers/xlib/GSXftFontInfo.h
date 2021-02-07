/*
   GSXftFontInfo

   NSFont helper for GNUstep GUI X/GPS Backend

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Fred Kiefer <fredkiefer@gmx.de>
   Date: July 2001

   This file is part of the GNUstep GUI X/GPS Backend.

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

// Include this before we include any objC defines, otherwise id is defined
#include <X11/Xlib.h>
#define id xwindowsid
#include <X11/Xft/Xft.h>
#undef id

#import "fontconfig/FCFontInfo.h"
#import "fontconfig/FCFontEnumerator.h"

@interface GSXftFaceInfo : FCFaceInfo
@end

@interface GSXftFontEnumerator : FCFontEnumerator
+ (Class) faceInfoClass;
+ (GSXftFaceInfo *) fontWithName: (NSString *)name;
@end

@interface GSXftFontInfo : FCFontInfo
{
  XftFont *font_info;
}

- (void) drawString:  (NSString*)string
	  onDisplay: (Display*) xdpy drawable: (Drawable) draw
	       with: (GC) xgcntxt at: (XPoint) xp;
- (void) draw: (const char*) s length: (int) len 
    onDisplay: (Display*) xdpy drawable: (Drawable) draw
	 with: (GC) xgcntxt at: (XPoint) xp;
- (CGFloat) widthOf: (const char*) s length: (int) len;
- (void) setActiveFor: (Display*) xdpy gc: (GC) xgcntxt;

@end
