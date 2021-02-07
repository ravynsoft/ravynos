/*
   CairoFontEnumerator.h

   Copyright (C) 2003 Free Software Foundation, Inc.
   August 31, 2003
   Written by Banlu Kemiyatorn <object at gmail dot com>

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


#ifndef CairoFontEnumerator_h
#define CairoFontEnumerator_h

#include "fontconfig/FCFontEnumerator.h"
#include <cairo.h>

@class CairoFaceInfo;

@interface CairoFontEnumerator : FCFontEnumerator
{
}
+ (Class) faceInfoClass;
+ (CairoFaceInfo *) fontWithName: (NSString *)name;
@end

#endif
