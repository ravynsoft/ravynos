/* gscolors - Color conversion routines

   Copyright (C) 1995 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Nov 1994
   
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

#ifndef _gscolors_h_INCLUDE
#define _gscolors_h_INCLUDE

#define AINDEX 5

typedef enum {
  gray_colorspace, rgb_colorspace, hsb_colorspace, cmyk_colorspace
} device_colorspace_t;

typedef struct _device_color {
  device_colorspace_t space;
  float field[6];
} device_color_t;

/* Internal conversion of colors to pixels values */
extern void gsMakeColor(device_color_t *dst, device_colorspace_t space,
				  float a, float b, float c, float d);
extern void gsColorToRGB(device_color_t *color);
extern void gsColorToGray(device_color_t *color);
extern void gsColorToCMYK(device_color_t *color);
extern void gsColorToHSB(device_color_t *color);

#endif


