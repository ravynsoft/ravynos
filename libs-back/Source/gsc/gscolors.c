/* gscolors - Color conversion routines

   Copyright (C) 1998 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Oct 1998
   
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gsc/gscolors.h"

void
gsMakeColor(device_color_t *dst, device_colorspace_t space, float a, float b, float c, float d)
{
  dst->space = space;
  dst->field[0] = a;
  dst->field[1] = b;
  dst->field[2] = c;
  dst->field[3] = d;
}

void
gsGrayToRGB(device_color_t  *color)
{
  gsMakeColor(color, rgb_colorspace, color->field[0], color->field[0],
	      color->field[0], 0);
}

void
gsHSBToRGB(device_color_t  *color)
{
  int i;
  float h, s, v;
  float f, p, q, t;
  float red, green, blue;

  h = color->field[0];
  s = color->field[1];
  v = color->field[2];

  if (s == 0)
    {
      gsMakeColor(color, rgb_colorspace, v, v, v, 0);
      return;
    }

  h = h * 6;
  i = (int)h;
  f = h - i;
  p = v * (1.0 - s);
  q = v * (1.0 - s * f);
  t = v * (1.0 - s * (1 - f));
  
  switch (i) 
    {
    default: /* catch h==1.0 */
    case 0:
      red = v;
      green = t;
      blue = p;
      break;
    case 1:
      red = q;
      green = v;
      blue = p;
      break;
    case 2:
      red = p;
      green = v;
      blue = t;
      break;
    case 3:
      red = p;
      green = q;
      blue = v;
      break;
    case 4:
      red = t;
      green = p;
      blue = v;
      break;
    case 5:
      red = v;
      green = p;
      blue = q;
      break;
    }
  gsMakeColor(color, rgb_colorspace, red, green, blue, 0);
}

/* FIXME */   
void
gsCMYKToRGB(device_color_t  *color)
{
  float c, m, y, k;
  float red, green, blue;
  double white;

  c = color->field[0];
  m = color->field[1];
  y = color->field[2];
  k = color->field[3];
  white = 1 - k;

  if (k == 0)
    {
      red = 1 - c;
      green = 1 - m;
      blue = 1 - y;
    }
  else if (k == 1)
    {
      red = 0;
      green = 0;
      blue = 0;
    }
  else
    {  
      red = (c > white ? 0 : white - c);
      green = (m > white ? 0 : white - m);
      blue = (y > white ? 0 : white - y);
    }
  gsMakeColor(color, rgb_colorspace, red, green, blue, 0);
}

void
gsColorToRGB(device_color_t *color)
{
  switch(color->space)
    {
    case gray_colorspace:
      gsGrayToRGB(color);
      break;
    case rgb_colorspace:
      break;
    case hsb_colorspace: 
      gsHSBToRGB(color);
      break;
    case cmyk_colorspace: 
      gsCMYKToRGB(color);
      break;
    default:
      break;
    }
}

void
gsColorToGray(device_color_t *color)
{
  switch(color->space)
    {
    case gray_colorspace:
      break;
    case hsb_colorspace:
    case cmyk_colorspace: 
      gsColorToRGB(color);
      /* NO BREAK */
    case rgb_colorspace:
      color->field[0] =
	((0.3*color->field[0]) + (0.59*color->field[1]) + (0.11*color->field[2]));
      break;
    default:
      break;
    }
  color->space = gray_colorspace;
}

void
gsColorToCMYK(device_color_t *color)
{
  switch(color->space)
    {
    case gray_colorspace:
      color->field[3] = color->field[0];
      color->field[0] = 0.0;
      color->field[1] = 0.0;
      color->field[2] = 0.0;
      break;
    case hsb_colorspace:
      gsColorToRGB(color);
      /* NO BREAK */
    case rgb_colorspace:
      color->field[0] = 1.0 - color->field[0];
      color->field[1] = 1.0 - color->field[1];
      color->field[2] = 1.0 - color->field[2];
      color->field[3] = 0;

      /* Add a bit of black if possible (for no reason, really). */
      color->field[3] = color->field[0];
      color->field[0] = 0.0;
      color->field[1] -= color->field[3];
      color->field[2] -= color->field[3];
      if (color->field[1] > color->field[2])
	{
	  if (color->field[2] < 0.0)
	    {
	      color->field[0] -= color->field[2];
	      color->field[1] -= color->field[2];
	      color->field[3] += color->field[2];
	      color->field[2] = 0;
	    }
	}
      else
	{
	  if (color->field[1] < 0.0)
	    {
	      color->field[0] -= color->field[1];
	      color->field[2] -= color->field[1];
	      color->field[3] += color->field[1];
	      color->field[1] = 0;
	    }
	}
      
      break;
    case cmyk_colorspace: 
      break;
    default:
      break;
    }
  color->space = cmyk_colorspace;
}

void
gsColorToHSB(device_color_t *color)
{
  switch(color->space)
    {
    case gray_colorspace:
      color->field[2] = color->field[0];
      color->field[0] = 0.0;
      color->field[1] = 0.0;
      break;
    case cmyk_colorspace:
      gsColorToRGB(color);
      /* NO BREAK */
    case rgb_colorspace:
      {
	float r = color->field[0];
	float g = color->field[1];
	float b = color->field[2];
	float _hue_component, _saturation_component, _brightness_component;

	if (r == g && r == b)
	  {
	    _hue_component = 0;
	    _saturation_component = 0;
	    _brightness_component = r;
	  }
	else
	  {
	    double H;
	    double V;
	    double Temp;
	    double diff;
	
	    V = (r > g ? r : g);
	    V = (b > V ? b : V);
	    Temp = (r < g ? r : g);
	    Temp = (b < Temp ? b : Temp);
	    diff = V - Temp;
	    if (V == r)
	      {
	        H = (g - b)/diff;
	      }
	    else if (V == g)
	      {
	        H = (b - r)/diff + 2;
	      }
	    else
	      {
	        H = (r - g)/diff + 4;
	      }
	    if (H < 0)
	      {
	        H += 6;
	      }
	    _hue_component = H/6;
	    _saturation_component = diff/V;
	    _brightness_component = V;
	  }
	color->field[0] = _hue_component;
	color->field[1] = _saturation_component;
	color->field[2] = _brightness_component;
      }
      break;
    case hsb_colorspace:
      break;
    default:
      break;
    }
  color->space = hsb_colorspace;
}
