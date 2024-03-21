/* Output stream for attributed text, producing ANSI escape sequences.
   Copyright (C) 2006-2008, 2017, 2019-2020, 2022-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "term-ostream.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#if HAVE_TCDRAIN
# include <termios.h>
#endif
#if defined _WIN32 || defined __CYGWIN__ /* Windows */
# define HAVE_WINDOWS_CONSOLES 1
# include <windows.h>
#endif

#include "error.h"
#include "full-write.h"
#include "get_ppid_of.h"
#include "get_progname_of.h"
#include "terminfo.h"
#include "xalloc.h"
#include "xgethostname.h"
#include "xsize.h"
#if HAVE_WINDOWS_CONSOLES
/* Get _get_osfhandle().  */
# if defined _WIN32 && ! defined __CYGWIN__
#  include "msvc-nothrow.h"
# else
#  include <io.h>
# endif
#endif
#include "gettext.h"

#define _(str) gettext (str)

#if HAVE_TPARAM
/* GNU termcap's tparam() function requires a buffer argument.  Make it so
   large that there is no risk that tparam() needs to call malloc().  */
static char tparambuf[100];
/* Define tparm in terms of tparam.  In the scope of this file, it is called
   with at most one argument after the string.  */
# define tparm(str, arg1) \
  tparam (str, tparambuf, sizeof (tparambuf), arg1)
#endif


/* =========================== Color primitives =========================== */

/* A color in RGB format.  */
typedef struct
{
  unsigned int red   : 8; /* range 0..255 */
  unsigned int green : 8; /* range 0..255 */
  unsigned int blue  : 8; /* range 0..255 */
} rgb_t;

/* A color in HSV (a.k.a. HSB) format.  */
typedef struct
{
  float hue;        /* normalized to interval [0,6) */
  float saturation; /* normalized to interval [0,1] */
  float brightness; /* a.k.a. value, normalized to interval [0,1] */
} hsv_t;

/* Conversion of a color in RGB to HSV format.  */
static void
rgb_to_hsv (rgb_t c, hsv_t *result)
{
  unsigned int r = c.red;
  unsigned int g = c.green;
  unsigned int b = c.blue;

  if (r > g)
    {
      if (b > r)
        {
          /* b > r > g, so max = b, min = g */
          result->hue = 4.0f + (float) (r - g) / (float) (b - g);
          result->saturation = 1.0f - (float) g / (float) b;
          result->brightness = (float) b / 255.0f;
        }
      else if (b <= g)
        {
          /* r > g >= b, so max = r, min = b */
          result->hue = 0.0f + (float) (g - b) / (float) (r - b);
          result->saturation = 1.0f - (float) b / (float) r;
          result->brightness = (float) r / 255.0f;
        }
      else
        {
          /* r >= b > g, so max = r, min = g */
          result->hue = 6.0f - (float) (b - g) / (float) (r - g);
          result->saturation = 1.0f - (float) g / (float) r;
          result->brightness = (float) r / 255.0f;
        }
    }
  else
    {
      if (b > g)
        {
          /* b > g >= r, so max = b, min = r */
          result->hue = 4.0f - (float) (g - r) / (float) (b - r);
          result->saturation = 1.0f - (float) r / (float) b;
          result->brightness = (float) b / 255.0f;
        }
      else if (b < r)
        {
          /* g >= r > b, so max = g, min = b */
          result->hue = 2.0f - (float) (r - b) / (float) (g - b);
          result->saturation = 1.0f - (float) b / (float) g;
          result->brightness = (float) g / 255.0f;
        }
      else if (g > r)
        {
          /* g >= b >= r, g > r, so max = g, min = r */
          result->hue = 2.0f + (float) (b - r) / (float) (g - r);
          result->saturation = 1.0f - (float) r / (float) g;
          result->brightness = (float) g / 255.0f;
        }
      else
        {
          /* r = g = b.  A grey color.  */
          result->hue = 0; /* arbitrary */
          result->saturation = 0;
          result->brightness = (float) r / 255.0f;
        }
    }
}

/* Square of distance of two colors.  */
static float
color_distance (const hsv_t *color1, const hsv_t *color2)
{
#if 0
  /* Formula taken from "John Smith: Color Similarity",
       http://www.ctr.columbia.edu/~jrsmith/html/pubs/acmmm96/node8.html.  */
  float angle1 = color1->hue * 1.04719755f; /* normalize to [0,2π] */
  float angle2 = color2->hue * 1.04719755f; /* normalize to [0,2π] */
  float delta_x = color1->saturation * cosf (angle1)
                  - color2->saturation * cosf (angle2);
  float delta_y = color1->saturation * sinf (angle1)
                  - color2->saturation * sinf (angle2);
  float delta_v = color1->brightness
                  - color2->brightness;

  return delta_x * delta_x + delta_y * delta_y + delta_v * delta_v;
#else
  /* Formula that considers hue differences with more weight than saturation
     or brightness differences, like the human eye does.  */
  float delta_hue =
    (color1->hue >= color2->hue
     ? (color1->hue - color2->hue >= 3.0f
        ? 6.0f + color2->hue - color1->hue
        : color1->hue - color2->hue)
     : (color2->hue - color1->hue >= 3.0f
        ? 6.0f + color1->hue - color2->hue
        : color2->hue - color1->hue));
  float min_saturation =
    (color1->saturation < color2->saturation
     ? color1->saturation
     : color2->saturation);
  float delta_saturation = color1->saturation - color2->saturation;
  float delta_brightness = color1->brightness - color2->brightness;

  return delta_hue * delta_hue * min_saturation
         + delta_saturation * delta_saturation * 0.2f
         + delta_brightness * delta_brightness * 0.8f;
#endif
}

/* Return the index of the color in a color table that is nearest to a given
   color.  */
static unsigned int
nearest_color (rgb_t given, const rgb_t *table, unsigned int table_size)
{
  hsv_t given_hsv;
  unsigned int best_index;
  float best_distance;
  unsigned int i;

  assert (table_size > 0);

  rgb_to_hsv (given, &given_hsv);

  best_index = 0;
  best_distance = 1000000.0f;
  for (i = 0; i < table_size; i++)
    {
      hsv_t i_hsv;

      rgb_to_hsv (table[i], &i_hsv);

      /* Avoid converting a color to grey, or fading out a color too much.  */
      if (i_hsv.saturation > given_hsv.saturation * 0.5f)
        {
          float distance = color_distance (&given_hsv, &i_hsv);
          if (distance < best_distance)
            {
              best_index = i;
              best_distance = distance;
            }
        }
    }

#if 0 /* Debugging code */
  hsv_t best_hsv;
  rgb_to_hsv (table[best_index], &best_hsv);
  fprintf (stderr, "nearest: (%d,%d,%d) = (%f,%f,%f)\n    -> (%f,%f,%f) = (%d,%d,%d)\n",
                   given.red, given.green, given.blue,
                   (double)given_hsv.hue, (double)given_hsv.saturation, (double)given_hsv.brightness,
                   (double)best_hsv.hue, (double)best_hsv.saturation, (double)best_hsv.brightness,
                   table[best_index].red, table[best_index].green, table[best_index].blue);
#endif

  return best_index;
}

/* The luminance of a color.  This is the brightness of the color, as it
   appears to the human eye.  This must be used in color to grey conversion.  */
static float
color_luminance (int r, int g, int b)
{
  /* Use the luminance model used by NTSC and JPEG.
     Taken from http://www.fho-emden.de/~hoffmann/gray10012001.pdf .
     No need to care about rounding errors leading to luminance > 1;
     this cannot happen.  */
  return (0.299f * r + 0.587f * g + 0.114f * b) / 255.0f;
}


/* ============================= Color models ============================= */

/* The color model used by the terminal.  */
typedef enum
{
  cm_monochrome,        /* No colors.  */
  cm_common8,           /* Usual terminal with at least 8 colors.  */
  cm_xterm8,            /* TERM=xterm, with 8 colors.  */
  cm_xterm16,           /* TERM=xterm-16color, with 16 colors.  */
  cm_xterm88,           /* TERM=xterm-88color, with 88 colors.  */
  cm_xterm256,          /* TERM=xterm-256color, with 256 colors.  */
  cm_xtermrgb           /* TERM=xterm-direct, with 256*256*256 colors.  */
} colormodel_t;

/* ----------------------- cm_monochrome color model ----------------------- */

/* A non-default color index doesn't exist in this color model.  */
static inline term_color_t
rgb_to_color_monochrome (void)
{
  return COLOR_DEFAULT;
}

/* ------------------------ cm_common8 color model ------------------------ */

/* A non-default color index is in the range 0..7.
                       RGB components
   COLOR_BLACK         000
   COLOR_BLUE          001
   COLOR_GREEN         010
   COLOR_CYAN          011
   COLOR_RED           100
   COLOR_MAGENTA       101
   COLOR_YELLOW        110
   COLOR_WHITE         111 */
static const rgb_t colors_of_common8[8] =
{
  /* R    G    B        grey  index */
  {   0,   0,   0 }, /* 0.000   0 */
  {   0,   0, 255 },
  {   0, 255,   0 },
  {   0, 255, 255 },
  { 255,   0,   0 },
  { 255,   0, 255 },
  { 255, 255,   0 },
  { 255, 255, 255 }  /* 1.000   7 */
};

static inline term_color_t
rgb_to_color_common8 (int r, int g, int b)
{
  rgb_t color;
  hsv_t hsv;

  color.red = r; color.green = g; color.blue = b;
  rgb_to_hsv (color, &hsv);

  if (hsv.saturation < 0.065f)
    {
      /* Greyscale approximation.  */
      float luminance = color_luminance (r, g, b);
      if (luminance < 0.500f)
        return 0;
      else
        return 7;
    }
  else
    /* Color approximation.  */
    return nearest_color (color, colors_of_common8, 8);
}

/* Convert a cm_common8 color in RGB encoding to BGR encoding.
   See the ncurses terminfo(5) manual page, section "Color Handling", for an
   explanation why this is needed.  */
static _GL_ASYNC_SAFE inline int
color_bgr (term_color_t color)
{
  return ((color & 4) >> 2) | (color & 2) | ((color & 1) << 2);
}

/* ------------------------- cm_xterm8 color model ------------------------- */

/* A non-default color index is in the range 0..7.
                       BGR components
   COLOR_BLACK         000
   COLOR_RED           001
   COLOR_GREEN         010
   COLOR_YELLOW        011
   COLOR_BLUE          100
   COLOR_MAGENTA       101
   COLOR_CYAN          110
   COLOR_WHITE         111 */
static const rgb_t colors_of_xterm8[8] =
{
  /* The real xterm's colors are dimmed; assume full-brightness instead.  */
  /* R    G    B        grey  index */
  {   0,   0,   0 }, /* 0.000   0 */
  { 255,   0,   0 },
  {   0, 255,   0 },
  { 255, 255,   0 },
  {   0,   0, 255 },
  { 255,   0, 255 },
  {   0, 255, 255 },
  { 255, 255, 255 }  /* 1.000   7 */
};

static inline term_color_t
rgb_to_color_xterm8 (int r, int g, int b)
{
  rgb_t color;
  hsv_t hsv;

  color.red = r; color.green = g; color.blue = b;
  rgb_to_hsv (color, &hsv);

  if (hsv.saturation < 0.065f)
    {
      /* Greyscale approximation.  */
      float luminance = color_luminance (r, g, b);
      if (luminance < 0.500f)
        return 0;
      else
        return 7;
    }
  else
    /* Color approximation.  */
    return nearest_color (color, colors_of_xterm8, 8);
}

/* ------------------------ cm_xterm16 color model ------------------------ */

/* A non-default color index is in the range 0..15.
   The RGB values come from xterm's XTerm-col.ad.  */
static const rgb_t colors_of_xterm16[16] =
{
  /* R    G    B        grey  index */
  {   0,   0,   0 }, /* 0.000   0 */
  { 205,   0,   0 },
  {   0, 205,   0 },
  { 205, 205,   0 },
  {   0,   0, 205 },
  { 205,   0, 205 },
  {   0, 205, 205 },
  { 229, 229, 229 }, /* 0.898   7 */
  {  77,  77,  77 }, /* 0.302   8 */
  { 255,   0,   0 },
  {   0, 255,   0 },
  { 255, 255,   0 },
  {   0,   0, 255 },
  { 255,   0, 255 },
  {   0, 255, 255 },
  { 255, 255, 255 }  /* 1.000  15 */
};

static inline term_color_t
rgb_to_color_xterm16 (int r, int g, int b)
{
  rgb_t color;
  hsv_t hsv;

  color.red = r; color.green = g; color.blue = b;
  rgb_to_hsv (color, &hsv);

  if (hsv.saturation < 0.065f)
    {
      /* Greyscale approximation.  */
      float luminance = color_luminance (r, g, b);
      if (luminance < 0.151f)
        return 0;
      else if (luminance < 0.600f)
        return 8;
      else if (luminance < 0.949f)
        return 7;
      else
        return 15;
    }
  else
    /* Color approximation.  */
    return nearest_color (color, colors_of_xterm16, 16);
}

/* ------------------------ cm_xterm88 color model ------------------------ */

/* A non-default color index is in the range 0..87.
   Colors 0..15 are the same as in the cm_xterm16 color model.
   Colors 16..87 are defined in xterm's 88colres.h.  */

static const rgb_t colors_of_xterm88[88] =
{
  /* R    G    B        grey  index */
  {   0,   0,   0 }, /* 0.000   0 */
  { 205,   0,   0 },
  {   0, 205,   0 },
  { 205, 205,   0 },
  {   0,   0, 205 },
  { 205,   0, 205 },
  {   0, 205, 205 },
  { 229, 229, 229 }, /* 0.898   7 */
  {  77,  77,  77 }, /* 0.302   8 */
  { 255,   0,   0 },
  {   0, 255,   0 },
  { 255, 255,   0 },
  {   0,   0, 255 },
  { 255,   0, 255 },
  {   0, 255, 255 },
  { 255, 255, 255 }, /* 1.000  15 */
  {   0,   0,   0 }, /* 0.000  16 */
  {   0,   0, 139 },
  {   0,   0, 205 },
  {   0,   0, 255 },
  {   0, 139,   0 },
  {   0, 139, 139 },
  {   0, 139, 205 },
  {   0, 139, 255 },
  {   0, 205,   0 },
  {   0, 205, 139 },
  {   0, 205, 205 },
  {   0, 205, 255 },
  {   0, 255,   0 },
  {   0, 255, 139 },
  {   0, 255, 205 },
  {   0, 255, 255 },
  { 139,   0,   0 },
  { 139,   0, 139 },
  { 139,   0, 205 },
  { 139,   0, 255 },
  { 139, 139,   0 },
  { 139, 139, 139 }, /* 0.545  37 */
  { 139, 139, 205 },
  { 139, 139, 255 },
  { 139, 205,   0 },
  { 139, 205, 139 },
  { 139, 205, 205 },
  { 139, 205, 255 },
  { 139, 255,   0 },
  { 139, 255, 139 },
  { 139, 255, 205 },
  { 139, 255, 255 },
  { 205,   0,   0 },
  { 205,   0, 139 },
  { 205,   0, 205 },
  { 205,   0, 255 },
  { 205, 139,   0 },
  { 205, 139, 139 },
  { 205, 139, 205 },
  { 205, 139, 255 },
  { 205, 205,   0 },
  { 205, 205, 139 },
  { 205, 205, 205 }, /* 0.804  58 */
  { 205, 205, 255 },
  { 205, 255,   0 },
  { 205, 255, 139 },
  { 205, 255, 205 },
  { 205, 255, 255 },
  { 255,   0,   0 },
  { 255,   0, 139 },
  { 255,   0, 205 },
  { 255,   0, 255 },
  { 255, 139,   0 },
  { 255, 139, 139 },
  { 255, 139, 205 },
  { 255, 139, 255 },
  { 255, 205,   0 },
  { 255, 205, 139 },
  { 255, 205, 205 },
  { 255, 205, 255 },
  { 255, 255,   0 },
  { 255, 255, 139 },
  { 255, 255, 205 },
  { 255, 255, 255 }, /* 1.000  79 */
  {  46,  46,  46 }, /* 0.180  80 */
  {  92,  92,  92 }, /* 0.361  81 */
  { 115, 115, 115 }, /* 0.451  82 */
  { 139, 139, 139 }, /* 0.545  83 */
  { 162, 162, 162 }, /* 0.635  84 */
  { 185, 185, 185 }, /* 0.725  85 */
  { 208, 208, 208 }, /* 0.816  86 */
  { 231, 231, 231 }  /* 0.906  87 */
};

static inline term_color_t
rgb_to_color_xterm88 (int r, int g, int b)
{
  rgb_t color;
  hsv_t hsv;

  color.red = r; color.green = g; color.blue = b;
  rgb_to_hsv (color, &hsv);

  if (hsv.saturation < 0.065f)
    {
      /* Greyscale approximation.  */
      float luminance = color_luminance (r, g, b);
      if (luminance < 0.090f)
        return 0;
      else if (luminance < 0.241f)
        return 80;
      else if (luminance < 0.331f)
        return 8;
      else if (luminance < 0.406f)
        return 81;
      else if (luminance < 0.498f)
        return 82;
      else if (luminance < 0.585f)
        return 37;
      else if (luminance < 0.680f)
        return 84;
      else if (luminance < 0.764f)
        return 85;
      else if (luminance < 0.810f)
        return 58;
      else if (luminance < 0.857f)
        return 86;
      else if (luminance < 0.902f)
        return 7;
      else if (luminance < 0.953f)
        return 87;
      else
        return 15;
    }
  else
    /* Color approximation.  */
    return nearest_color (color, colors_of_xterm88, 88);
}

/* ------------------------ cm_xterm256 color model ------------------------ */

/* A non-default color index is in the range 0..255.
   Colors 0..15 are the same as in the cm_xterm16 color model.
   Colors 16..255 are defined in xterm's 256colres.h.  */

static const rgb_t colors_of_xterm256[256] =
{
  /* R    G    B        grey  index */
  {   0,   0,   0 }, /* 0.000   0 */
  { 205,   0,   0 },
  {   0, 205,   0 },
  { 205, 205,   0 },
  {   0,   0, 205 },
  { 205,   0, 205 },
  {   0, 205, 205 },
  { 229, 229, 229 }, /* 0.898   7 */
  {  77,  77,  77 }, /* 0.302   8 */
  { 255,   0,   0 },
  {   0, 255,   0 },
  { 255, 255,   0 },
  {   0,   0, 255 },
  { 255,   0, 255 },
  {   0, 255, 255 },
  { 255, 255, 255 }, /* 1.000  15 */
  {   0,   0,   0 }, /* 0.000  16 */
  {   0,   0,  42 },
  {   0,   0,  85 },
  {   0,   0, 127 },
  {   0,   0, 170 },
  {   0,   0, 212 },
  {   0,  42,   0 },
  {   0,  42,  42 },
  {   0,  42,  85 },
  {   0,  42, 127 },
  {   0,  42, 170 },
  {   0,  42, 212 },
  {   0,  85,   0 },
  {   0,  85,  42 },
  {   0,  85,  85 },
  {   0,  85, 127 },
  {   0,  85, 170 },
  {   0,  85, 212 },
  {   0, 127,   0 },
  {   0, 127,  42 },
  {   0, 127,  85 },
  {   0, 127, 127 },
  {   0, 127, 170 },
  {   0, 127, 212 },
  {   0, 170,   0 },
  {   0, 170,  42 },
  {   0, 170,  85 },
  {   0, 170, 127 },
  {   0, 170, 170 },
  {   0, 170, 212 },
  {   0, 212,   0 },
  {   0, 212,  42 },
  {   0, 212,  85 },
  {   0, 212, 127 },
  {   0, 212, 170 },
  {   0, 212, 212 },
  {  42,   0,   0 },
  {  42,   0,  42 },
  {  42,   0,  85 },
  {  42,   0, 127 },
  {  42,   0, 170 },
  {  42,   0, 212 },
  {  42,  42,   0 },
  {  42,  42,  42 }, /* 0.165  59 */
  {  42,  42,  85 },
  {  42,  42, 127 },
  {  42,  42, 170 },
  {  42,  42, 212 },
  {  42,  85,   0 },
  {  42,  85,  42 },
  {  42,  85,  85 },
  {  42,  85, 127 },
  {  42,  85, 170 },
  {  42,  85, 212 },
  {  42, 127,   0 },
  {  42, 127,  42 },
  {  42, 127,  85 },
  {  42, 127, 127 },
  {  42, 127, 170 },
  {  42, 127, 212 },
  {  42, 170,   0 },
  {  42, 170,  42 },
  {  42, 170,  85 },
  {  42, 170, 127 },
  {  42, 170, 170 },
  {  42, 170, 212 },
  {  42, 212,   0 },
  {  42, 212,  42 },
  {  42, 212,  85 },
  {  42, 212, 127 },
  {  42, 212, 170 },
  {  42, 212, 212 },
  {  85,   0,   0 },
  {  85,   0,  42 },
  {  85,   0,  85 },
  {  85,   0, 127 },
  {  85,   0, 170 },
  {  85,   0, 212 },
  {  85,  42,   0 },
  {  85,  42,  42 },
  {  85,  42,  85 },
  {  85,  42, 127 },
  {  85,  42, 170 },
  {  85,  42, 212 },
  {  85,  85,   0 },
  {  85,  85,  42 },
  {  85,  85,  85 }, /* 0.333 102 */
  {  85,  85, 127 },
  {  85,  85, 170 },
  {  85,  85, 212 },
  {  85, 127,   0 },
  {  85, 127,  42 },
  {  85, 127,  85 },
  {  85, 127, 127 },
  {  85, 127, 170 },
  {  85, 127, 212 },
  {  85, 170,   0 },
  {  85, 170,  42 },
  {  85, 170,  85 },
  {  85, 170, 127 },
  {  85, 170, 170 },
  {  85, 170, 212 },
  {  85, 212,   0 },
  {  85, 212,  42 },
  {  85, 212,  85 },
  {  85, 212, 127 },
  {  85, 212, 170 },
  {  85, 212, 212 },
  { 127,   0,   0 },
  { 127,   0,  42 },
  { 127,   0,  85 },
  { 127,   0, 127 },
  { 127,   0, 170 },
  { 127,   0, 212 },
  { 127,  42,   0 },
  { 127,  42,  42 },
  { 127,  42,  85 },
  { 127,  42, 127 },
  { 127,  42, 170 },
  { 127,  42, 212 },
  { 127,  85,   0 },
  { 127,  85,  42 },
  { 127,  85,  85 },
  { 127,  85, 127 },
  { 127,  85, 170 },
  { 127,  85, 212 },
  { 127, 127,   0 },
  { 127, 127,  42 },
  { 127, 127,  85 },
  { 127, 127, 127 }, /* 0.498 145 */
  { 127, 127, 170 },
  { 127, 127, 212 },
  { 127, 170,   0 },
  { 127, 170,  42 },
  { 127, 170,  85 },
  { 127, 170, 127 },
  { 127, 170, 170 },
  { 127, 170, 212 },
  { 127, 212,   0 },
  { 127, 212,  42 },
  { 127, 212,  85 },
  { 127, 212, 127 },
  { 127, 212, 170 },
  { 127, 212, 212 },
  { 170,   0,   0 },
  { 170,   0,  42 },
  { 170,   0,  85 },
  { 170,   0, 127 },
  { 170,   0, 170 },
  { 170,   0, 212 },
  { 170,  42,   0 },
  { 170,  42,  42 },
  { 170,  42,  85 },
  { 170,  42, 127 },
  { 170,  42, 170 },
  { 170,  42, 212 },
  { 170,  85,   0 },
  { 170,  85,  42 },
  { 170,  85,  85 },
  { 170,  85, 127 },
  { 170,  85, 170 },
  { 170,  85, 212 },
  { 170, 127,   0 },
  { 170, 127,  42 },
  { 170, 127,  85 },
  { 170, 127, 127 },
  { 170, 127, 170 },
  { 170, 127, 212 },
  { 170, 170,   0 },
  { 170, 170,  42 },
  { 170, 170,  85 },
  { 170, 170, 127 },
  { 170, 170, 170 }, /* 0.667 188 */
  { 170, 170, 212 },
  { 170, 212,   0 },
  { 170, 212,  42 },
  { 170, 212,  85 },
  { 170, 212, 127 },
  { 170, 212, 170 },
  { 170, 212, 212 },
  { 212,   0,   0 },
  { 212,   0,  42 },
  { 212,   0,  85 },
  { 212,   0, 127 },
  { 212,   0, 170 },
  { 212,   0, 212 },
  { 212,  42,   0 },
  { 212,  42,  42 },
  { 212,  42,  85 },
  { 212,  42, 127 },
  { 212,  42, 170 },
  { 212,  42, 212 },
  { 212,  85,   0 },
  { 212,  85,  42 },
  { 212,  85,  85 },
  { 212,  85, 127 },
  { 212,  85, 170 },
  { 212,  85, 212 },
  { 212, 127,   0 },
  { 212, 127,  42 },
  { 212, 127,  85 },
  { 212, 127, 127 },
  { 212, 127, 170 },
  { 212, 127, 212 },
  { 212, 170,   0 },
  { 212, 170,  42 },
  { 212, 170,  85 },
  { 212, 170, 127 },
  { 212, 170, 170 },
  { 212, 170, 212 },
  { 212, 212,   0 },
  { 212, 212,  42 },
  { 212, 212,  85 },
  { 212, 212, 127 },
  { 212, 212, 170 },
  { 212, 212, 212 }, /* 0.831 231 */
  {   8,   8,   8 }, /* 0.031 232 */
  {  18,  18,  18 }, /* 0.071 233 */
  {  28,  28,  28 }, /* 0.110 234 */
  {  38,  38,  38 }, /* 0.149 235 */
  {  48,  48,  48 }, /* 0.188 236 */
  {  58,  58,  58 }, /* 0.227 237 */
  {  68,  68,  68 }, /* 0.267 238 */
  {  78,  78,  78 }, /* 0.306 239 */
  {  88,  88,  88 }, /* 0.345 240 */
  {  98,  98,  98 }, /* 0.384 241 */
  { 108, 108, 108 }, /* 0.424 242 */
  { 118, 118, 118 }, /* 0.463 243 */
  { 128, 128, 128 }, /* 0.502 244 */
  { 138, 138, 138 }, /* 0.541 245 */
  { 148, 148, 148 }, /* 0.580 246 */
  { 158, 158, 158 }, /* 0.620 247 */
  { 168, 168, 168 }, /* 0.659 248 */
  { 178, 178, 178 }, /* 0.698 249 */
  { 188, 188, 188 }, /* 0.737 250 */
  { 198, 198, 198 }, /* 0.776 251 */
  { 208, 208, 208 }, /* 0.816 252 */
  { 218, 218, 218 }, /* 0.855 253 */
  { 228, 228, 228 }, /* 0.894 254 */
  { 238, 238, 238 }  /* 0.933 255 */
};

static inline term_color_t
rgb_to_color_xterm256 (int r, int g, int b)
{
  rgb_t color;
  hsv_t hsv;

  color.red = r; color.green = g; color.blue = b;
  rgb_to_hsv (color, &hsv);

  if (hsv.saturation < 0.065f)
    {
      /* Greyscale approximation.  */
      float luminance = color_luminance (r, g, b);
      if (luminance < 0.015f)
        return 0;
      else if (luminance < 0.051f)
        return 232;
      else if (luminance < 0.090f)
        return 233;
      else if (luminance < 0.129f)
        return 234;
      else if (luminance < 0.157f)
        return 235;
      else if (luminance < 0.177f)
        return 59;
      else if (luminance < 0.207f)
        return 236;
      else if (luminance < 0.247f)
        return 237;
      else if (luminance < 0.284f)
        return 238;
      else if (luminance < 0.304f)
        return 8;
      else if (luminance < 0.319f)
        return 239;
      else if (luminance < 0.339f)
        return 102;
      else if (luminance < 0.364f)
        return 240;
      else if (luminance < 0.404f)
        return 241;
      else if (luminance < 0.443f)
        return 242;
      else if (luminance < 0.480f)
        return 243;
      else if (luminance < 0.500f)
        return 145;
      else if (luminance < 0.521f)
        return 244;
      else if (luminance < 0.560f)
        return 245;
      else if (luminance < 0.600f)
        return 246;
      else if (luminance < 0.639f)
        return 247;
      else if (luminance < 0.663f)
        return 248;
      else if (luminance < 0.682f)
        return 188;
      else if (luminance < 0.717f)
        return 249;
      else if (luminance < 0.756f)
        return 250;
      else if (luminance < 0.796f)
        return 251;
      else if (luminance < 0.823f)
        return 252;
      else if (luminance < 0.843f)
        return 231;
      else if (luminance < 0.874f)
        return 253;
      else if (luminance < 0.896f)
        return 254;
      else if (luminance < 0.915f)
        return 7;
      else if (luminance < 0.966f)
        return 255;
      else
        return 15;
    }
  else
    /* Color approximation.  */
    return nearest_color (color, colors_of_xterm256, 256);
}

/* ------------------------ cm_xtermrgb color model ------------------------ */

/* We represent a color as an RGB triplet: (r << 16) | (g << 8) | (b << 0),
   where r, g, b are in the range [0..255].  */

static inline term_color_t
rgb_to_color_xtermrgb (int r, int g, int b)
{
  return (r << 16) | (g << 8) | (b << 0);
}


/* ============================== hyperlink_t ============================== */

/* A hyperlink is a heap-allocated structure that can be assigned to a run
   of characters.  */
typedef struct
{
  /* URL.
     Should better be <= 2083 bytes long (because of Microsoft Internet
     Explorer).  */
  char *ref;
  /* Id.
     Used when the same hyperlink persists across newlines.
     Should better be <= 256 bytes long (because of VTE and iTerm2).  */
  char *id;
  /* Same as id, if non-NULL.  Or some generated id.  */
  char *real_id;
} hyperlink_t;

static inline void
free_hyperlink (hyperlink_t *hyperlink)
{
  free (hyperlink->ref);
  free (hyperlink->real_id);
  free (hyperlink);
}


/* ============================= attributes_t ============================= */

/* ANSI C and ISO C99 6.7.2.1.(4) forbid use of bit fields for types other
   than 'int' or 'unsigned int'.
   On the other hand, C++ forbids conversion between enum types and integer
   types without an explicit cast.  */
#ifdef __cplusplus
# define BITFIELD_TYPE(orig_type,integer_type) orig_type
#else
# define BITFIELD_TYPE(orig_type,integer_type) integer_type
#endif

/* Attributes that can be set on a character.  */
typedef struct
{
  BITFIELD_TYPE(term_color_t,     signed int)   color     : 25;
  BITFIELD_TYPE(term_color_t,     signed int)   bgcolor   : 25;
  BITFIELD_TYPE(term_weight_t,    unsigned int) weight    : 1;
  BITFIELD_TYPE(term_posture_t,   unsigned int) posture   : 1;
  BITFIELD_TYPE(term_underline_t, unsigned int) underline : 1;
  /* Hyperlink, or NULL for none.  */
  hyperlink_t *hyperlink;
} attributes_t;

/* Compare two sets of attributes for equality.  */
static inline bool
equal_attributes (attributes_t attr1, attributes_t attr2)
{
  return (attr1.color == attr2.color
          && attr1.bgcolor == attr2.bgcolor
          && attr1.weight == attr2.weight
          && attr1.posture == attr2.posture
          && attr1.underline == attr2.underline
          && attr1.hyperlink == attr2.hyperlink);
}


/* ============================ EINTR handling ============================ */

/* EINTR handling for tcdrain().
   This function can return -1/EINTR even when we don't have any
   signal handlers set up, namely when we get interrupted via SIGSTOP.  */

#if HAVE_TCDRAIN

static inline int
nonintr_tcdrain (int fd)
{
  int retval;

  do
    retval = tcdrain (fd);
  while (retval < 0 && errno == EINTR);

  return retval;
}

#endif


/* ============================ term_ostream_t ============================ */

struct term_ostream : struct ostream
{
fields:
  /* The file descriptor used for output.  Note that ncurses termcap emulation
     uses the baud rate information from file descriptor 1 (stdout) if it is
     a tty, or from file descriptor 2 (stderr) otherwise.  */
  int volatile fd;
  #if HAVE_WINDOWS_CONSOLES
  HANDLE volatile handle;
  bool volatile is_windows_console;
  #endif
  char *filename;
  ttyctl_t tty_control;
  /* Values from the terminal type's terminfo/termcap description.
     See terminfo(5) for details.  */
                                         /* terminfo  termcap */
  int max_colors;                        /* colors    Co */
  int no_color_video;                    /* ncv       NC */
  char * volatile set_a_foreground;      /* setaf     AF */
  char * volatile set_foreground;        /* setf      Sf */
  char * volatile set_a_background;      /* setab     AB */
  char * volatile set_background;        /* setb      Sb */
  char *orig_pair;                       /* op        op */
  char * volatile enter_bold_mode;       /* bold      md */
  char * volatile enter_italics_mode;    /* sitm      ZH */
  char *exit_italics_mode;               /* ritm      ZR */
  char * volatile enter_underline_mode;  /* smul      us */
  char *exit_underline_mode;             /* rmul      ue */
  char *exit_attribute_mode;             /* sgr0      me */
  /* Inferred values.  */
  bool volatile supports_foreground;
  bool volatile supports_background;
  colormodel_t volatile colormodel;
  bool volatile supports_weight;
  bool volatile supports_posture;
  bool volatile supports_underline;
  bool volatile supports_hyperlink;
  /* Inferred values for the exit handler and the signal handlers.  */
  const char * volatile restore_colors;
  const char * volatile restore_weight;
  const char * volatile restore_posture;
  const char * volatile restore_underline;
  const char * volatile restore_hyperlink;
  /* Signal handling and tty control.  */
  struct term_style_control_data control_data;
  /* State for producing hyperlink ids.  */
  uint32_t hostname_hash;
  uint64_t start_time;
  uint32_t id_serial;
  /* Set of hyperlink_t that are currently in use.  */
  hyperlink_t **hyperlinks_array;
  size_t hyperlinks_count;
  size_t hyperlinks_allocated;
  /* Variable state, representing past output.  */
  #if HAVE_WINDOWS_CONSOLES
  WORD volatile default_console_attributes;
  WORD volatile current_console_attributes;
  #endif
  attributes_t default_attr;         /* Default simplified attributes of the
                                        terminal.  */
  attributes_t volatile active_attr; /* Simplified attributes that we have set
                                        on the terminal.  */
  term_color_t volatile active_attr_color;   /* Same as active_attr.color,
                                                atomically accessible.  */
  term_color_t volatile active_attr_bgcolor; /* Same as active_attr.bgcolor,
                                                atomically accessible.  */
  hyperlink_t *volatile active_attr_hyperlink; /* Same as active_attr.hyperlink,
                                                  atomically accessible.  */
  /* Variable state, representing future output.  */
  char *buffer;                      /* Buffer for the current line.  */
  attributes_t *attrbuffer;          /* Buffer for the simplified attributes;
                                        same length as buffer.  */
  size_t buflen;                     /* Number of bytes stored so far.  */
  size_t allocated;                  /* Allocated size of the buffer.  */
  attributes_t curr_attr;            /* Current attributes.  */
  attributes_t simp_attr;            /* Simplified current attributes.  */
};

static struct term_style_control_data *
get_control_data (term_ostream_t stream)
{
  return &stream->control_data;
}

/* Simplify attributes, according to the terminal's capabilities.  */
static attributes_t
simplify_attributes (term_ostream_t stream, attributes_t attr)
{
  if ((attr.color != COLOR_DEFAULT || attr.bgcolor != COLOR_DEFAULT)
      && stream->no_color_video > 0)
    {
      /* When colors and attributes can not be represented simultaneously,
         we give preference to the color.  */
      if (stream->no_color_video & 2)
        /* Colors conflict with underlining.  */
        attr.underline = UNDERLINE_OFF;
      if (stream->no_color_video & 32)
        /* Colors conflict with bold weight.  */
        attr.weight = WEIGHT_NORMAL;
    }
  if (!stream->supports_foreground)
    attr.color = COLOR_DEFAULT;
  if (!stream->supports_background)
    attr.bgcolor = COLOR_DEFAULT;
  if (!stream->supports_weight)
    attr.weight = WEIGHT_DEFAULT;
  if (!stream->supports_posture)
    attr.posture = POSTURE_DEFAULT;
  if (!stream->supports_underline)
    attr.underline = UNDERLINE_DEFAULT;
  if (!stream->supports_hyperlink)
    attr.hyperlink = NULL;
  return attr;
}

/* Generate an id for a hyperlink.  */
static char *
generate_hyperlink_id (term_ostream_t stream)
{
  /* A UUID would be optimal, but is overkill here.  An id of 128 bits
     (32 hexadecimal digits) should be sufficient.  */
  static const char hexdigits[16] =
    {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
  char *id = (char *) xmalloc (128 / 4 + 1);
  uint32_t words[4] =
    {
      stream->hostname_hash,
      (uint32_t) (stream->start_time >> 32),
      (uint32_t) stream->start_time,
      stream->id_serial
    };
  char *p = id;
  unsigned int i;
  for (i = 0; i < 4; i++)
    {
      uint32_t word = words[i];
      unsigned int j;
      for (j = 0; j < 32 / 4; j++)
        *p++ = hexdigits[(word >> (32 - 4 * (j + 1))) & 0x0f];
    }
  *p = '\0';
  stream->id_serial++;
  return id;
}

/* Stream that contains information about how the various out_* functions shall
   do output.  */
static term_ostream_t volatile out_stream;

/* File descriptor to which out_char and out_char_unchecked shall output escape
   sequences.
   Same as (out_stream != NULL ? out_stream->fd : -1).  */
static int volatile out_fd = -1;

/* Signal error after full_write failed.  */
static void
out_error (void)
{
  error (EXIT_FAILURE, errno, _("error writing to %s"), out_stream->filename);
}

/* Output a single char to out_fd.  */
static int
out_char (int c)
{
  char bytes[1];

  bytes[0] = (char)c;
  /* We have to write directly to the file descriptor, not to a buffer with
     the same destination, because of the padding and sleeping that tputs()
     does.  */
  if (full_write (out_fd, bytes, 1) < 1)
    out_error ();
  return 0;
}

/* Output a single char to out_fd.  Ignore errors.  */
static _GL_ASYNC_SAFE int
out_char_unchecked (int c)
{
  char bytes[1];

  bytes[0] = (char)c;
  full_write (out_fd, bytes, 1);
  return 0;
}

/* Output escape sequences to switch the foreground color to NEW_COLOR.  */
static _GL_ASYNC_SAFE void
out_color_change (term_ostream_t stream, term_color_t new_color,
                  bool async_safe)
{
  assert (stream->supports_foreground);
  assert (new_color != COLOR_DEFAULT);
  switch (stream->colormodel)
    {
    case cm_common8:
      assert (new_color >= 0 && new_color < 8);
      #if HAVE_WINDOWS_CONSOLES
      if (stream->is_windows_console)
        {
          /* SetConsoleTextAttribute
             <https://docs.microsoft.com/en-us/windows/console/setconsoletextattribute>
             <https://docs.microsoft.com/en-us/windows/console/console-screen-buffers>  */
          /* Assign to stream->current_console_attributes *before* calling
             SetConsoleTextAttribute, otherwise async_set_attributes_from_default
             will not do its job correctly.  */
          stream->current_console_attributes =
            (stream->current_console_attributes & ~(7 << 0))
            | (new_color << 0);
          SetConsoleTextAttribute (stream->handle, stream->current_console_attributes);
        }
      else
      #endif
        {
          if (stream->set_a_foreground != NULL)
            tputs (tparm (stream->set_a_foreground, color_bgr (new_color)),
                   1, async_safe ? out_char_unchecked : out_char);
          else
            tputs (tparm (stream->set_foreground, new_color),
                   1, async_safe ? out_char_unchecked : out_char);
        }
      break;
    /* When we are dealing with an xterm, there is no need to go through
       tputs() because we know there is no padding and sleeping.  */
    case cm_xterm8:
      assert (new_color >= 0 && new_color < 8);
      {
        char bytes[5];
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '3'; bytes[3] = '0' + new_color;
        bytes[4] = 'm';
        if (full_write (out_fd, bytes, 5) < 5)
          if (!async_safe)
            out_error ();
      }
      break;
    case cm_xterm16:
      assert (new_color >= 0 && new_color < 16);
      {
        char bytes[5];
        bytes[0] = 0x1B; bytes[1] = '[';
        if (new_color < 8)
          {
            bytes[2] = '3'; bytes[3] = '0' + new_color;
          }
        else
          {
            bytes[2] = '9'; bytes[3] = '0' + (new_color - 8);
          }
        bytes[4] = 'm';
        if (full_write (out_fd, bytes, 5) < 5)
          if (!async_safe)
            out_error ();
      }
      break;
    case cm_xterm88:
      assert (new_color >= 0 && new_color < 88);
      {
        char bytes[10];
        char *p;
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '3'; bytes[3] = '8'; bytes[4] = ';';
        bytes[5] = '5'; bytes[6] = ';';
        p = bytes + 7;
        if (new_color >= 10)
          *p++ = '0' + (new_color / 10);
        *p++ = '0' + (new_color % 10);
        *p++ = 'm';
        if (full_write (out_fd, bytes, p - bytes) < p - bytes)
          if (!async_safe)
            out_error ();
      }
      break;
    case cm_xterm256:
      assert (new_color >= 0 && new_color < 256);
      {
        char bytes[11];
        char *p;
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '3'; bytes[3] = '8'; bytes[4] = ';';
        bytes[5] = '5'; bytes[6] = ';';
        p = bytes + 7;
        if (new_color >= 100)
          *p++ = '0' + (new_color / 100);
        if (new_color >= 10)
          *p++ = '0' + ((new_color % 100) / 10);
        *p++ = '0' + (new_color % 10);
        *p++ = 'm';
        if (full_write (out_fd, bytes, p - bytes) < p - bytes)
          if (!async_safe)
            out_error ();
      }
      break;
    case cm_xtermrgb:
      assert (new_color >= 0 && new_color < 0x1000000);
      {
        char bytes[19];
        char *p;
        unsigned int r = (new_color >> 16) & 0xff;
        unsigned int g = (new_color >> 8) & 0xff;
        unsigned int b = new_color & 0xff;
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '3'; bytes[3] = '8'; bytes[4] = ';';
        bytes[5] = '2'; bytes[6] = ';';
        p = bytes + 7;
        if (r >= 100)
          *p++ = '0' + (r / 100);
        if (r >= 10)
          *p++ = '0' + ((r % 100) / 10);
        *p++ = '0' + (r % 10);
        *p++ = ';';
        if (g >= 100)
          *p++ = '0' + (g / 100);
        if (g >= 10)
          *p++ = '0' + ((g % 100) / 10);
        *p++ = '0' + (g % 10);
        *p++ = ';';
        if (b >= 100)
          *p++ = '0' + (b / 100);
        if (b >= 10)
          *p++ = '0' + ((b % 100) / 10);
        *p++ = '0' + (b % 10);
        *p++ = 'm';
        if (full_write (out_fd, bytes, p - bytes) < p - bytes)
          if (!async_safe)
            out_error ();
      }
      break;
    default:
      abort ();
    }
}

/* Output escape sequences to switch the background color to NEW_BGCOLOR.  */
static _GL_ASYNC_SAFE void
out_bgcolor_change (term_ostream_t stream, term_color_t new_bgcolor,
                    bool async_safe)
{
  assert (stream->supports_background);
  assert (new_bgcolor != COLOR_DEFAULT);
  switch (stream->colormodel)
    {
    case cm_common8:
      assert (new_bgcolor >= 0 && new_bgcolor < 8);
      #if HAVE_WINDOWS_CONSOLES
      if (stream->is_windows_console)
        {
          /* SetConsoleTextAttribute
             <https://docs.microsoft.com/en-us/windows/console/setconsoletextattribute>
             <https://docs.microsoft.com/en-us/windows/console/console-screen-buffers>  */
          /* Assign to stream->current_console_attributes *before* calling
             SetConsoleTextAttribute, otherwise async_set_attributes_from_default
             will not do its job correctly.  */
          stream->current_console_attributes =
            (stream->current_console_attributes & ~(7 << 4))
            | (new_bgcolor << 4);
          SetConsoleTextAttribute (stream->handle, stream->current_console_attributes);
        }
      else
      #endif
        {
          if (stream->set_a_background != NULL)
            tputs (tparm (stream->set_a_background, color_bgr (new_bgcolor)),
                   1, async_safe ? out_char_unchecked : out_char);
          else
            tputs (tparm (stream->set_background, new_bgcolor),
                   1, async_safe ? out_char_unchecked : out_char);
        }
      break;
    /* When we are dealing with an xterm, there is no need to go through
       tputs() because we know there is no padding and sleeping.  */
    case cm_xterm8:
      assert (new_bgcolor >= 0 && new_bgcolor < 8);
      {
        char bytes[5];
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '4'; bytes[3] = '0' + new_bgcolor;
        bytes[4] = 'm';
        if (full_write (out_fd, bytes, 5) < 5)
          if (!async_safe)
            out_error ();
      }
      break;
    case cm_xterm16:
      assert (new_bgcolor >= 0 && new_bgcolor < 16);
      {
        char bytes[6];
        bytes[0] = 0x1B; bytes[1] = '[';
        if (new_bgcolor < 8)
          {
            bytes[2] = '4'; bytes[3] = '0' + new_bgcolor;
            bytes[4] = 'm';
            if (full_write (out_fd, bytes, 5) < 5)
              if (!async_safe)
                out_error ();
          }
        else
          {
            bytes[2] = '1'; bytes[3] = '0';
            bytes[4] = '0' + (new_bgcolor - 8); bytes[5] = 'm';
            if (full_write (out_fd, bytes, 6) < 6)
              if (!async_safe)
                out_error ();
          }
      }
      break;
    case cm_xterm88:
      assert (new_bgcolor >= 0 && new_bgcolor < 88);
      {
        char bytes[10];
        char *p;
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '4'; bytes[3] = '8'; bytes[4] = ';';
        bytes[5] = '5'; bytes[6] = ';';
        p = bytes + 7;
        if (new_bgcolor >= 10)
          *p++ = '0' + (new_bgcolor / 10);
        *p++ = '0' + (new_bgcolor % 10);
        *p++ = 'm';
        if (full_write (out_fd, bytes, p - bytes) < p - bytes)
          if (!async_safe)
            out_error ();
      }
      break;
    case cm_xterm256:
      assert (new_bgcolor >= 0 && new_bgcolor < 256);
      {
        char bytes[11];
        char *p;
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '4'; bytes[3] = '8'; bytes[4] = ';';
        bytes[5] = '5'; bytes[6] = ';';
        p = bytes + 7;
        if (new_bgcolor >= 100)
          *p++ = '0' + (new_bgcolor / 100);
        if (new_bgcolor >= 10)
          *p++ = '0' + ((new_bgcolor % 100) / 10);
        *p++ = '0' + (new_bgcolor % 10);
        *p++ = 'm';
        if (full_write (out_fd, bytes, p - bytes) < p - bytes)
          if (!async_safe)
            out_error ();
      }
      break;
    case cm_xtermrgb:
      assert (new_bgcolor >= 0 && new_bgcolor < 0x1000000);
      {
        char bytes[19];
        char *p;
        unsigned int r = (new_bgcolor >> 16) & 0xff;
        unsigned int g = (new_bgcolor >> 8) & 0xff;
        unsigned int b = new_bgcolor & 0xff;
        bytes[0] = 0x1B; bytes[1] = '[';
        bytes[2] = '4'; bytes[3] = '8'; bytes[4] = ';';
        bytes[5] = '2'; bytes[6] = ';';
        p = bytes + 7;
        if (r >= 100)
          *p++ = '0' + (r / 100);
        if (r >= 10)
          *p++ = '0' + ((r % 100) / 10);
        *p++ = '0' + (r % 10);
        *p++ = ';';
        if (g >= 100)
          *p++ = '0' + (g / 100);
        if (g >= 10)
          *p++ = '0' + ((g % 100) / 10);
        *p++ = '0' + (g % 10);
        *p++ = ';';
        if (b >= 100)
          *p++ = '0' + (b / 100);
        if (b >= 10)
          *p++ = '0' + ((b % 100) / 10);
        *p++ = '0' + (b % 10);
        *p++ = 'm';
        if (full_write (out_fd, bytes, p - bytes) < p - bytes)
          if (!async_safe)
            out_error ();
      }
      break;
    default:
      abort ();
    }
}

/* Output escape sequences to switch the weight to NEW_WEIGHT.  */
static _GL_ASYNC_SAFE void
out_weight_change (term_ostream_t stream, term_weight_t new_weight,
                   bool async_safe)
{
  assert (stream->supports_weight);
  assert (new_weight != WEIGHT_DEFAULT);
  /* This implies:  */
  assert (new_weight == WEIGHT_BOLD);
  tputs (stream->enter_bold_mode,
         1, async_safe ? out_char_unchecked : out_char);
}

/* Output escape sequences to switch the posture to NEW_POSTURE.  */
static _GL_ASYNC_SAFE void
out_posture_change (term_ostream_t stream, term_posture_t new_posture,
                    bool async_safe)
{
  assert (stream->supports_posture);
  assert (new_posture != POSTURE_DEFAULT);
  /* This implies:  */
  assert (new_posture == POSTURE_ITALIC);
  tputs (stream->enter_italics_mode,
         1, async_safe ? out_char_unchecked : out_char);
}

/* Output escape sequences to switch the underline to NEW_UNDERLINE.  */
static _GL_ASYNC_SAFE void
out_underline_change (term_ostream_t stream, term_underline_t new_underline,
                      bool async_safe)
{
  assert (stream->supports_underline);
  assert (new_underline != UNDERLINE_DEFAULT);
  /* This implies:  */
  assert (new_underline == UNDERLINE_ON);
  #if HAVE_WINDOWS_CONSOLES
  if (stream->is_windows_console)
    {
      /* SetConsoleTextAttribute
         <https://docs.microsoft.com/en-us/windows/console/setconsoletextattribute>
         <https://docs.microsoft.com/en-us/windows/console/console-screen-buffers>  */
      /* Assign to stream->current_console_attributes *before* calling
         SetConsoleTextAttribute, otherwise async_set_attributes_from_default
         will not do its job correctly.  */
      stream->current_console_attributes =
        stream->current_console_attributes | COMMON_LVB_UNDERSCORE;
      SetConsoleTextAttribute (stream->handle, stream->current_console_attributes);
    }
  else
  #endif
    {
      tputs (stream->enter_underline_mode,
             1, async_safe ? out_char_unchecked : out_char);
    }
}

/* Output escape seqeuences to switch the hyperlink to NEW_HYPERLINK.  */
static _GL_ASYNC_SAFE void
out_hyperlink_change (term_ostream_t stream, hyperlink_t *new_hyperlink,
                      bool async_safe)
{
  int (*out_ch) (int) = (async_safe ? out_char_unchecked : out_char);
  assert (stream->supports_hyperlink);
  if (new_hyperlink != NULL)
    {
      assert (new_hyperlink->real_id != NULL);
      tputs ("\033]8;id=",           1, out_ch);
      tputs (new_hyperlink->real_id, 1, out_ch);
      tputs (";",                    1, out_ch);
      tputs (new_hyperlink->ref,     1, out_ch);
      tputs ("\033\\",               1, out_ch);
    }
  else
    tputs ("\033]8;;\033\\", 1, out_ch);
}

/* Output escape sequences to switch from STREAM->ACTIVE_ATTR to NEW_ATTR,
   and update STREAM->ACTIVE_ATTR.  */
static void
out_attr_change (term_ostream_t stream, attributes_t new_attr)
{
  attributes_t old_attr = stream->active_attr;

  /* Keep track of the active attributes.  Do this *before* emitting the
     escape sequences, otherwise async_set_attributes_from_default will not
     do its job correctly.  */
  stream->active_attr = new_attr;
  stream->active_attr_color = new_attr.color;
  stream->active_attr_bgcolor = new_attr.bgcolor;
  stream->active_attr_hyperlink = new_attr.hyperlink;

  #if HAVE_WINDOWS_CONSOLES
  if (stream->is_windows_console)
    {
      /* SetConsoleTextAttribute
         <https://docs.microsoft.com/en-us/windows/console/setconsoletextattribute>
         <https://docs.microsoft.com/en-us/windows/console/console-screen-buffers>  */
      /* Assign to stream->current_console_attributes *before* calling
         SetConsoleTextAttribute, otherwise async_set_attributes_from_default
         will not do its job correctly.  */
      stream->current_console_attributes =
        (stream->current_console_attributes
         & ~((7 << 0) | (7 << 4) | COMMON_LVB_UNDERSCORE))
        | (new_attr.color == COLOR_DEFAULT
           ? stream->default_console_attributes & (7 << 0)
           : (new_attr.color << 0))
        | (new_attr.bgcolor == COLOR_DEFAULT
           ? stream->default_console_attributes & (7 << 4)
           : (new_attr.bgcolor << 4))
        | (new_attr.underline ? COMMON_LVB_UNDERSCORE : 0);
      SetConsoleTextAttribute (stream->handle, stream->current_console_attributes);
    }
  else
  #endif
    {
      bool cleared_attributes;

      /* For out_char to work.  */
      out_stream = stream;
      out_fd = stream->fd;

      /* We don't know the default colors of the terminal.  The only way to
         switch back to a default color is to use stream->orig_pair.  */
      if ((new_attr.color == COLOR_DEFAULT && old_attr.color != COLOR_DEFAULT)
          || (new_attr.bgcolor == COLOR_DEFAULT && old_attr.bgcolor != COLOR_DEFAULT))
        {
          assert (stream->supports_foreground || stream->supports_background);
          tputs (stream->orig_pair, 1, out_char);
          old_attr.color = COLOR_DEFAULT;
          old_attr.bgcolor = COLOR_DEFAULT;
        }

      /* To turn off WEIGHT_BOLD, the only way is to output the
         exit_attribute_mode sequence.  (With xterm, you can also do it with
         "Esc [ 0 m", but this escape sequence is not contained in the terminfo
         description.)  It may also clear the colors; this is the case e.g. when
         TERM="xterm" or TERM="ansi".
         To turn off UNDERLINE_ON, we can use the exit_underline_mode or the
         exit_attribute_mode sequence.  In the latter case, it will not only
         turn off UNDERLINE_ON, but also the other attributes, and possibly also
         the colors.
         To turn off POSTURE_ITALIC, we can use the exit_italics_mode or the
         exit_attribute_mode sequence.  Again, in the latter case, it will not
         only turn off POSTURE_ITALIC, but also the other attributes, and
         possibly also the colors.
         There is no point in setting an attribute just before emitting an
         escape sequence that may again turn off the attribute.  Therefore we
         proceed in two steps: First, clear the attributes that need to be
         cleared; then - taking into account that this may have cleared all
         attributes and all colors - set the colors and the attributes.
         The variable 'cleared_attributes' tells whether an escape sequence
         has been output that may have cleared all attributes and all color
         settings.  */
      cleared_attributes = false;
      if (old_attr.posture != POSTURE_NORMAL
          && new_attr.posture == POSTURE_NORMAL
          && stream->exit_italics_mode != NULL)
        {
          tputs (stream->exit_italics_mode, 1, out_char);
          old_attr.posture = POSTURE_NORMAL;
          cleared_attributes = true;
        }
      if (old_attr.underline != UNDERLINE_OFF
          && new_attr.underline == UNDERLINE_OFF
          && stream->exit_underline_mode != NULL)
        {
          tputs (stream->exit_underline_mode, 1, out_char);
          old_attr.underline = UNDERLINE_OFF;
          cleared_attributes = true;
        }
      if ((old_attr.weight != WEIGHT_NORMAL
           && new_attr.weight == WEIGHT_NORMAL)
          || (old_attr.posture != POSTURE_NORMAL
              && new_attr.posture == POSTURE_NORMAL
              /* implies stream->exit_italics_mode == NULL */)
          || (old_attr.underline != UNDERLINE_OFF
              && new_attr.underline == UNDERLINE_OFF
              /* implies stream->exit_underline_mode == NULL */))
        {
          tputs (stream->exit_attribute_mode, 1, out_char);
          /* We don't know exactly what effects exit_attribute_mode has, but
             this is the minimum effect:  */
          old_attr.weight = WEIGHT_NORMAL;
          if (stream->exit_italics_mode == NULL)
            old_attr.posture = POSTURE_NORMAL;
          if (stream->exit_underline_mode == NULL)
            old_attr.underline = UNDERLINE_OFF;
          cleared_attributes = true;
        }

      /* Turn on the colors.  */
      if (new_attr.color != old_attr.color
          || (cleared_attributes && new_attr.color != COLOR_DEFAULT))
        {
          out_color_change (stream, new_attr.color, false);
        }
      if (new_attr.bgcolor != old_attr.bgcolor
          || (cleared_attributes && new_attr.bgcolor != COLOR_DEFAULT))
        {
          out_bgcolor_change (stream, new_attr.bgcolor, false);
        }
      if (new_attr.weight != old_attr.weight
          || (cleared_attributes && new_attr.weight != WEIGHT_DEFAULT))
        {
          out_weight_change (stream, new_attr.weight, false);
        }
      if (new_attr.posture != old_attr.posture
          || (cleared_attributes && new_attr.posture != POSTURE_DEFAULT))
        {
          out_posture_change (stream, new_attr.posture, false);
        }
      if (new_attr.underline != old_attr.underline
          || (cleared_attributes && new_attr.underline != UNDERLINE_DEFAULT))
        {
          out_underline_change (stream, new_attr.underline, false);
        }
      if (new_attr.hyperlink != old_attr.hyperlink)
        {
          out_hyperlink_change (stream, new_attr.hyperlink, false);
        }
    }
}

static void
restore (term_ostream_t stream)
{
  #if HAVE_WINDOWS_CONSOLES
  if (stream->is_windows_console)
    {
      /* SetConsoleTextAttribute
         <https://docs.microsoft.com/en-us/windows/console/setconsoletextattribute>
         <https://docs.microsoft.com/en-us/windows/console/console-screen-buffers>  */
      SetConsoleTextAttribute (stream->handle, stream->default_console_attributes);
    }
  else
  #endif
    {
      /* For out_char_unchecked to work.  */
      out_stream = stream;
      out_fd = stream->fd;

      if (stream->restore_colors != NULL)
        tputs (stream->restore_colors, 1, out_char_unchecked);
      if (stream->restore_weight != NULL)
        tputs (stream->restore_weight, 1, out_char_unchecked);
      if (stream->restore_posture != NULL)
        tputs (stream->restore_posture, 1, out_char_unchecked);
      if (stream->restore_underline != NULL)
        tputs (stream->restore_underline, 1, out_char_unchecked);
      if (stream->restore_hyperlink != NULL)
        tputs (stream->restore_hyperlink, 1, out_char_unchecked);
    }
}

static _GL_ASYNC_SAFE void
async_restore (term_ostream_t stream)
{
  #if HAVE_WINDOWS_CONSOLES
  if (stream->is_windows_console)
    {
      /* SetConsoleTextAttribute
         <https://docs.microsoft.com/en-us/windows/console/setconsoletextattribute>
         <https://docs.microsoft.com/en-us/windows/console/console-screen-buffers>  */
      SetConsoleTextAttribute (stream->handle, stream->default_console_attributes);
    }
  else
  #endif
    {
      /* For out_char_unchecked to work.  */
      out_stream = stream;
      out_fd = stream->fd;

      if (stream->restore_colors != NULL)
        tputs (stream->restore_colors, 1, out_char_unchecked);
      if (stream->restore_weight != NULL)
        tputs (stream->restore_weight, 1, out_char_unchecked);
      if (stream->restore_posture != NULL)
        tputs (stream->restore_posture, 1, out_char_unchecked);
      if (stream->restore_underline != NULL)
        tputs (stream->restore_underline, 1, out_char_unchecked);
      if (stream->restore_hyperlink != NULL)
        tputs (stream->restore_hyperlink, 1, out_char_unchecked);
    }
}

static _GL_ASYNC_SAFE void
async_set_attributes_from_default (term_ostream_t stream)
{
  #if HAVE_WINDOWS_CONSOLES
  if (stream->is_windows_console)
    {
      /* SetConsoleTextAttribute
         <https://docs.microsoft.com/en-us/windows/console/setconsoletextattribute>
         <https://docs.microsoft.com/en-us/windows/console/console-screen-buffers>  */
      SetConsoleTextAttribute (stream->handle, stream->current_console_attributes);
    }
  else
  #endif
    {
      attributes_t new_attr = stream->active_attr;
      /* Since stream->active_attr is not guaranteed to be loaded atomically,
         new_attr.color and new_attr.bgcolor may have invalid values.
         Use the atomically loadable values instead.  */
      new_attr.color = stream->active_attr_color;
      new_attr.bgcolor = stream->active_attr_bgcolor;
      new_attr.hyperlink = stream->active_attr_hyperlink;

      /* For out_char_unchecked to work.  */
      out_stream = stream;
      out_fd = stream->fd;

      if (new_attr.color != COLOR_DEFAULT)
        out_color_change (stream, new_attr.color, true);
      if (new_attr.bgcolor != COLOR_DEFAULT)
        out_bgcolor_change (stream, new_attr.bgcolor, true);
      if (new_attr.weight != WEIGHT_DEFAULT)
        out_weight_change (stream, new_attr.weight, true);
      if (new_attr.posture != POSTURE_DEFAULT)
        out_posture_change (stream, new_attr.posture, true);
      if (new_attr.underline != UNDERLINE_DEFAULT)
        out_underline_change (stream, new_attr.underline, true);
      if (new_attr.hyperlink != NULL)
        out_hyperlink_change (stream, new_attr.hyperlink, true);
    }
}

static const struct term_style_controller controller =
{
  get_control_data,
  restore,
  async_restore,
  async_set_attributes_from_default
};

/* Activate the default attributes.  */
static void
activate_default_attr (term_ostream_t stream)
{
  /* Switch back to the default attributes.  */
  out_attr_change (stream, stream->default_attr);

  deactivate_term_non_default_mode (&controller, stream);
}

/* Output the buffered line atomically.
   The terminal is left in the the state (regarding colors and attributes)
   represented by the simplified attributes goal_attr.  */
static void
output_buffer (term_ostream_t stream, attributes_t goal_attr)
{
  const char *cp;
  const attributes_t *ap;
  size_t len;
  size_t n;

  cp = stream->buffer;
  ap = stream->attrbuffer;
  len = stream->buflen;

  /* See how much we can output without blocking signals.  */
  for (n = 0; n < len && equal_attributes (ap[n], stream->active_attr); n++)
    ;
  if (n > 0)
    {
      if (full_write (stream->fd, cp, n) < n)
        {
          int error_code = errno;
          /* Do output to stderr only after we have switched back to the
             default attributes.  Otherwise this output may come out with
             the wrong text attributes.  */
          if (!equal_attributes (stream->active_attr, stream->default_attr))
            activate_default_attr (stream);
          error (EXIT_FAILURE, error_code, _("error writing to %s"),
                 stream->filename);
        }
      cp += n;
      ap += n;
      len -= n;
    }
  if (len > 0)
    {
      if (!equal_attributes (*ap, stream->default_attr))
        activate_term_non_default_mode (&controller, stream);

      do
        {
          /* Activate the attributes in *ap.  */
          out_attr_change (stream, *ap);
          /* See how many characters we can output without further attribute
             changes.  */
          for (n = 1; n < len && equal_attributes (ap[n], stream->active_attr); n++)
            ;
          if (full_write (stream->fd, cp, n) < n)
            {
              int error_code = errno;
              /* Do output to stderr only after we have switched back to the
                 default attributes.  Otherwise this output may come out with
                 the wrong text attributes.  */
              if (!equal_attributes (stream->active_attr, stream->default_attr))
                activate_default_attr (stream);
              error (EXIT_FAILURE, error_code, _("error writing to %s"),
                     stream->filename);
            }
          cp += n;
          ap += n;
          len -= n;
        }
      while (len > 0);
    }
  stream->buflen = 0;

  /* Before changing to goal_attr, we may need to enable the non-default
     attributes mode.  */
  if (!equal_attributes (goal_attr, stream->default_attr))
    activate_term_non_default_mode (&controller, stream);
  /* Change to goal_attr.  */
  if (!equal_attributes (goal_attr, stream->active_attr))
    out_attr_change (stream, goal_attr);
  /* When we can deactivate the non-default attributes mode, do so.  */
  if (equal_attributes (goal_attr, stream->default_attr))
    deactivate_term_non_default_mode (&controller, stream);

  /* Free the hyperlink_t objects that are no longer referenced by the
     stream->attrbuffer.  */
  {
    size_t count = stream->hyperlinks_count;
    size_t j = 0;
    size_t i;
    for (i = 0; i < count; i++)
      {
        /* Here 0 <= j <= i.  */
        hyperlink_t *hyperlink = stream->hyperlinks_array[i];
        /* stream->default_attr.hyperlink is always == NULL.
           stream->simp_attr.hyperlink is either == NULL
                                              or == stream->curr_attr.hyperlink.
           We can therefore ignore both.  */
        if (hyperlink == stream->curr_attr.hyperlink
            || hyperlink == stream->active_attr.hyperlink)
          {
            /* The hyperlink is still in use.  */
            stream->hyperlinks_array[j] = hyperlink;
            j++;
          }
        else
          {
            /* The hyperlink is not in use any more.  */
            free_hyperlink (hyperlink);
          }
      }
    stream->hyperlinks_count = j;
  }
}

/* Implementation of ostream_t methods.  */

static term_color_t
term_ostream::rgb_to_color (term_ostream_t stream, int red, int green, int blue)
{
  switch (stream->colormodel)
    {
    case cm_monochrome:
      return rgb_to_color_monochrome ();
    case cm_common8:
      return rgb_to_color_common8 (red, green, blue);
    case cm_xterm8:
      return rgb_to_color_xterm8 (red, green, blue);
    case cm_xterm16:
      return rgb_to_color_xterm16 (red, green, blue);
    case cm_xterm88:
      return rgb_to_color_xterm88 (red, green, blue);
    case cm_xterm256:
      return rgb_to_color_xterm256 (red, green, blue);
    case cm_xtermrgb:
      return rgb_to_color_xtermrgb (red, green, blue);
    default:
      abort ();
    }
}

static void
term_ostream::write_mem (term_ostream_t stream, const void *data, size_t len)
{
  const char *cp = (const char *) data;
  while (len > 0)
    {
      /* Look for the next newline.  */
      const char *newline = (const char *) memchr (cp, '\n', len);
      size_t n = (newline != NULL ? newline - cp : len);

      /* Copy n bytes into the buffer.  */
      if (n > stream->allocated - stream->buflen)
        {
          size_t new_allocated =
            xmax (xsum (stream->buflen, n),
                  xsum (stream->allocated, stream->allocated));
          if (size_overflow_p (new_allocated))
            error (EXIT_FAILURE, 0,
                   _("%s: too much output, buffer size overflow"),
                   "term_ostream");
          stream->buffer = (char *) xrealloc (stream->buffer, new_allocated);
          stream->attrbuffer =
            (attributes_t *)
            xrealloc (stream->attrbuffer,
                      new_allocated * sizeof (attributes_t));
          stream->allocated = new_allocated;
        }
      memcpy (stream->buffer + stream->buflen, cp, n);
      {
        attributes_t attr = stream->simp_attr;
        attributes_t *ap = stream->attrbuffer + stream->buflen;
        attributes_t *ap_end = ap + n;
        for (; ap < ap_end; ap++)
          *ap = attr;
      }
      stream->buflen += n;

      if (newline != NULL)
        {
          output_buffer (stream, stream->default_attr);
          if (full_write (stream->fd, "\n", 1) < 1)
            error (EXIT_FAILURE, errno, _("error writing to %s"),
                   stream->filename);
          cp += n + 1; /* cp = newline + 1; */
          len -= n + 1;
        }
      else
        break;
    }
}

static void
term_ostream::flush (term_ostream_t stream, ostream_flush_scope_t scope)
{
  output_buffer (stream, stream->default_attr);
  if (scope == FLUSH_ALL)
    {
      #if HAVE_WINDOWS_CONSOLES
      if (!stream->is_windows_console)
      #endif
        {
          /* For streams connected to a disk file:  */
          fsync (stream->fd);
          #if HAVE_TCDRAIN
          /* For streams connected to a terminal:  */
          nonintr_tcdrain (stream->fd);
          #endif
        }
    }
}

static void
term_ostream::free (term_ostream_t stream)
{
  term_ostream_flush (stream, FLUSH_THIS_STREAM);

  deactivate_term_style_controller (&controller, stream);

  free (stream->filename);
  if (stream->set_a_foreground != NULL)
    free (stream->set_a_foreground);
  if (stream->set_foreground != NULL)
    free (stream->set_foreground);
  if (stream->set_a_background != NULL)
    free (stream->set_a_background);
  if (stream->set_background != NULL)
    free (stream->set_background);
  if (stream->orig_pair != NULL)
    free (stream->orig_pair);
  if (stream->enter_bold_mode != NULL)
    free (stream->enter_bold_mode);
  if (stream->enter_italics_mode != NULL)
    free (stream->enter_italics_mode);
  if (stream->exit_italics_mode != NULL)
    free (stream->exit_italics_mode);
  if (stream->enter_underline_mode != NULL)
    free (stream->enter_underline_mode);
  if (stream->exit_underline_mode != NULL)
    free (stream->exit_underline_mode);
  if (stream->exit_attribute_mode != NULL)
    free (stream->exit_attribute_mode);
  if (stream->hyperlinks_array != NULL)
    {
      size_t count = stream->hyperlinks_count;
      size_t i;
      for (i = 0; i < count; i++)
        free_hyperlink (stream->hyperlinks_array[i]);
      free (stream->hyperlinks_array);
    }
  free (stream->buffer);
  free (stream->attrbuffer);
  free (stream);
}

/* Implementation of term_ostream_t methods.  */

static term_color_t
term_ostream::get_color (term_ostream_t stream)
{
  return stream->curr_attr.color;
}

static void
term_ostream::set_color (term_ostream_t stream, term_color_t color)
{
  stream->curr_attr.color = color;
  stream->simp_attr = simplify_attributes (stream, stream->curr_attr);
}

static term_color_t
term_ostream::get_bgcolor (term_ostream_t stream)
{
  return stream->curr_attr.bgcolor;
}

static void
term_ostream::set_bgcolor (term_ostream_t stream, term_color_t color)
{
  stream->curr_attr.bgcolor = color;
  stream->simp_attr = simplify_attributes (stream, stream->curr_attr);
}

static term_weight_t
term_ostream::get_weight (term_ostream_t stream)
{
  return stream->curr_attr.weight;
}

static void
term_ostream::set_weight (term_ostream_t stream, term_weight_t weight)
{
  stream->curr_attr.weight = weight;
  stream->simp_attr = simplify_attributes (stream, stream->curr_attr);
}

static term_posture_t
term_ostream::get_posture (term_ostream_t stream)
{
  return stream->curr_attr.posture;
}

static void
term_ostream::set_posture (term_ostream_t stream, term_posture_t posture)
{
  stream->curr_attr.posture = posture;
  stream->simp_attr = simplify_attributes (stream, stream->curr_attr);
}

static term_underline_t
term_ostream::get_underline (term_ostream_t stream)
{
  return stream->curr_attr.underline;
}

static void
term_ostream::set_underline (term_ostream_t stream, term_underline_t underline)
{
  stream->curr_attr.underline = underline;
  stream->simp_attr = simplify_attributes (stream, stream->curr_attr);
}

static const char *
term_ostream::get_hyperlink_ref (term_ostream_t stream)
{
  hyperlink_t *hyperlink = stream->curr_attr.hyperlink;
  return (hyperlink != NULL ? hyperlink->ref : NULL);
}

static const char *
term_ostream::get_hyperlink_id (term_ostream_t stream)
{
  hyperlink_t *hyperlink = stream->curr_attr.hyperlink;
  return (hyperlink != NULL ? hyperlink->id : NULL);
}

static void
term_ostream::set_hyperlink (term_ostream_t stream,
                             const char *ref, const char *id)
{
  if (ref == NULL)
    stream->curr_attr.hyperlink = NULL;
  else
    {
      /* Create a new hyperlink_t object.  */
      hyperlink_t *hyperlink = XMALLOC (hyperlink_t);

      hyperlink->ref = xstrdup (ref);
      if (id != NULL)
        {
          hyperlink->id = xstrdup (id);
          hyperlink->real_id = hyperlink->id;
        }
      else
        {
          hyperlink->id = NULL;
          if (stream->supports_hyperlink)
            {
              /* Generate an id always, since we don't know at this point
                 whether the hyperlink will span multiple lines.  */
              hyperlink->real_id = generate_hyperlink_id (stream);
            }
          else
            hyperlink->real_id = NULL;
        }

      /* Store it.  */
      if (stream->hyperlinks_count == stream->hyperlinks_allocated)
        {
          stream->hyperlinks_allocated = 2 * stream->hyperlinks_allocated + 10;
          stream->hyperlinks_array =
            (hyperlink_t **)
            xrealloc (stream->hyperlinks_array,
                      stream->hyperlinks_allocated * sizeof (hyperlink_t *));
        }
      stream->hyperlinks_array[stream->hyperlinks_count++] = hyperlink;

      /* Install it.  */
      stream->curr_attr.hyperlink = hyperlink;
    }
  stream->simp_attr = simplify_attributes (stream, stream->curr_attr);
}

static void
term_ostream::flush_to_current_style (term_ostream_t stream)
{
  output_buffer (stream, stream->simp_attr);
}

/* Constructor.  */

static inline char *
xstrdup0 (const char *str)
{
  if (str == NULL)
    return NULL;
#if HAVE_TERMINFO
  if (str == (const char *)(-1))
    return NULL;
#endif
  return xstrdup (str);
}

/* Returns the base name of the terminal emulator program, possibly truncated,
   as a freshly allocated string, or NULL if it cannot be determined.
   Note: This function is a hack.  It does not work across ssh, and it may fail
   in some local situations as well.  */
static inline char *
get_terminal_emulator_progname (void)
{
  #if HAVE_GETSID
  /* Get the process id of the session leader.
     When running in a terminal emulator, it's the shell process that was
     spawned by the terminal emulator.  When running in a console, it's the
     'login' process.
     On some operating systems (Linux, *BSD, AIX), the same result could also
     be obtained through
       pid_t p;
       if (ioctl (1, TIOCGSID, &p) >= 0) ...
   */
  pid_t session_leader_pid = getsid (0);
  if (session_leader_pid != (pid_t)(-1))
    {
      /* Get the process id of the terminal emulator.
         When running in a console, it's the process id of the 'init'
         process.  */
      pid_t terminal_emulator_pid = get_ppid_of (session_leader_pid);
      if (terminal_emulator_pid != 0)
        {
          /* Retrieve the base name of the program name of this process.  */
          return get_progname_of (terminal_emulator_pid);
        }
    }
  #endif
  return NULL;
}

/* Returns true if we should enable hyperlinks.
   term is the value of the TERM environment variable.  */
static inline bool
should_enable_hyperlinks (const char *term)
{
  if (getenv ("NO_TERM_HYPERLINKS") != NULL)
    /* The user has disabled hyperlinks.  */
    return false;

  /* Dispatch based on $TERM.  */
  if (term != NULL)
    {
      /* rxvt-based terminal emulators:
           Program           | TERM         | Supports hyperlinks?
           ------------------+--------------+-------------------------------------
           rxvt 2.7.10       | rxvt         | hangs after "cat hyperlink-demo.txt"
           mrxvt 0.5.3       | rxvt         | no
           rxvt-unicode 9.22 | rxvt-unicode | no
       */
      if (strcmp (term, "rxvt") == 0)
        return false;

      /* Emacs-based terminal emulators:
           Program             | TERM        | Supports hyperlinks?
           --------------------+-------------+---------------------
           emacs-terminal 26.1 | eterm-color | produces garbage
       */
      if (strncmp (term, "eterm", 5) == 0)
        return false;

      /* xterm-compatible terminal emulators:
           Program          | TERM           | Supports hyperlinks?
           -----------------+----------------+---------------------------
           guake 0.8.8      | xterm          | produces garbage
           lilyterm 0.9.9.2 | xterm          | produces garbage
           lterm 1.5.1      | xterm          | produces garbage
           lxterminal 0.3.2 | xterm          | produces garbage
           termit 2.9.6     | xterm          | produces garbage
           konsole 18.12.3  | xterm-256color | produces extra backslashes
           yakuake 3.0.5    | xterm-256color | produces extra backslashes
           other            |                | yes or no, no severe bugs

         TODO: Revisit this table periodically.
       */
      if (strncmp (term, "xterm", 5) == 0)
        {
          char *progname = get_terminal_emulator_progname ();
          if (progname != NULL)
            {
              bool known_buggy =
                strncmp (progname, "python", 6) == 0 /* guake */
                || strcmp (progname, "lilyterm") == 0
                || strcmp (progname, "lterm") == 0
                || strcmp (progname, "lxterminal") == 0
                || strcmp (progname, "termit") == 0
                || strcmp (progname, "konsole") == 0
                || strcmp (progname, "yakuake") == 0;
              free (progname);
              /* Enable hyperlinks except for programs that are known buggy.  */
              return !known_buggy;
            }
        }

      /* Solaris console.
           Program                            | TERM      | Supports hyperlinks?
           -----------------------------------+-----------+---------------------------
           Solaris kernel's terminal emulator | sun-color | produces garbage
           SPARC PROM's terminal emulator     | sun       | ?
       */
      if (strcmp (term, "sun") == 0 || strcmp (term, "sun-color") == 0)
        return false;
    }

  /* In case of doubt, enable hyperlinks.  So this code does not need to change
     as more and more terminal emulators support hyperlinks.
     If there are adverse effects, the user can disable hyperlinks by setting
     NO_TERM_HYPERLINKS.  */
  return true;
}

term_ostream_t
term_ostream_create (int fd, const char *filename, ttyctl_t tty_control)
{
  term_ostream_t stream = XMALLOC (struct term_ostream_representation);

  stream->base.vtable = &term_ostream_vtable;
  stream->fd = fd;
  #if HAVE_WINDOWS_CONSOLES
  stream->handle = (HANDLE) _get_osfhandle (fd);
  {
    DWORD mode;

    if (stream->handle != INVALID_HANDLE_VALUE
        /* GetConsoleMode
           <https://docs.microsoft.com/en-us/windows/console/getconsolemode>  */
        && GetConsoleMode (stream->handle, &mode) != 0)
      {
        CONSOLE_SCREEN_BUFFER_INFO info;
        BOOL ok;

        /* GetConsoleScreenBufferInfo
           <https://docs.microsoft.com/en-us/windows/console/getconsolescreenbufferinfo>
           <https://docs.microsoft.com/en-us/windows/console/console-screen-buffer-info-str>  */
        ok = GetConsoleScreenBufferInfo (stream->handle, &info);
        if (!ok)
          {
            /* GetConsoleScreenBufferInfo
                 - fails when the handle is == GetStdHandle (STD_INPUT_HANDLE)
                 - but succeeds when it is == GetStdHandle (STD_OUTPUT_HANDLE)
                   or == GetStdHandle (STD_ERROR_HANDLE).
               Native Windows programs use GetStdHandle (STD_OUTPUT_HANDLE) for
               fd 1, as expected.
               But Cygwin uses GetStdHandle (STD_INPUT_HANDLE) for all of fd 0,
               1, 2.  So, we have to use either GetStdHandle (STD_OUTPUT_HANDLE)
               or GetStdHandle (STD_ERROR_HANDLE) in order to be able to use
               GetConsoleScreenBufferInfo.  */
            if (fd == 1 || fd == 2)
              {
                HANDLE handle =
                  GetStdHandle (fd == 1 ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
                ok = GetConsoleScreenBufferInfo (handle, &info);
                if (ok)
                  stream->handle = handle;
              }
          }
        if (ok)
          {
            stream->is_windows_console = true;
            stream->default_console_attributes = info.wAttributes;
            stream->current_console_attributes = stream->default_console_attributes;
          }
        else
          /* It's a console, but we cannot use GetConsoleScreenBufferInfo.  */
          stream->is_windows_console = false;
      }
    else
      stream->is_windows_console = false;
  }
  #endif
  stream->filename = xstrdup (filename);
  stream->tty_control = tty_control;

  /* Defaults.  */
  stream->max_colors = -1;
  stream->no_color_video = -1;
  stream->set_a_foreground = NULL;
  stream->set_foreground = NULL;
  stream->set_a_background = NULL;
  stream->set_background = NULL;
  stream->orig_pair = NULL;
  stream->enter_bold_mode = NULL;
  stream->enter_italics_mode = NULL;
  stream->exit_italics_mode = NULL;
  stream->enter_underline_mode = NULL;
  stream->exit_underline_mode = NULL;
  stream->exit_attribute_mode = NULL;

  #if HAVE_WINDOWS_CONSOLES
  if (stream->is_windows_console)
    {
      /* For Windows consoles, two approaches are possible:
         (A) Use SetConsoleMode
             <https://docs.microsoft.com/en-us/windows/console/setconsolemode>
             to enable the ENABLE_VIRTUAL_TERMINAL_PROCESSING flag, and then
             emit escape sequences, as documented in
             <https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences>.
         (B) Use SetConsoleTextAttribute
             <https://docs.microsoft.com/en-us/windows/console/setconsoletextattribute>
             to change the text attributes.
         Approach (A) has two drawbacks:
           * It produces colors that ignore the console's configuration: it
             assumes the default configuration (light grey foreground).  Thus
             when you ask for cyan, you will always get some blue color, never
             real cyan.  Whereas approach (B) produces colors that respect the
             "Screen Text" and "Screen Background" settings in the console's
             configuration.
           * When the program terminates abnormally, we would leave the console
             with ENABLE_VIRTUAL_TERMINAL_PROCESSING enabled, which can be
             dangerous.
         Therefore we use approach (B).  */
      stream->max_colors = 8;
      stream->no_color_video = 1 | 4;
      stream->supports_foreground = true;
      stream->supports_background = true;
      stream->colormodel = cm_common8;
      /* The Windows consoles have high and low intensity, but the default is
         high intensity.  If we wanted to support WEIGHT_BOLD, we would have to
         use low-intensity rendering for normal output, which would look ugly
         compared to the output by other programs.  We could support WEIGHT_DIM,
         but this is not part of our enum term_weight_t.  */
      stream->supports_weight = false;
      stream->supports_posture = false;
      stream->supports_underline = true;
      stream->supports_hyperlink = false;
      stream->restore_colors = NULL;
      stream->restore_weight = NULL;
      stream->restore_posture = NULL;
      stream->restore_underline = NULL;
      stream->restore_hyperlink = NULL;
    }
  else
  #endif
    {
      const char *term;

      /* Retrieve the terminal type.  */
      term = getenv ("TERM");
      if (term != NULL && term[0] != '\0')
        {
          /* When the terminfo function are available, we prefer them over the
             termcap functions because
               1. they don't risk a buffer overflow,
               2. on OSF/1, for TERM=xterm, the tiget* functions provide access
                  to the number of colors and the color escape sequences,
                  whereas the tget* functions don't provide them.  */
          #if HAVE_TERMINFO
          int err = 1;

          if (setupterm (term, fd, &err) == 0 || err == 1)
            {
              /* Retrieve particular values depending on the terminal type.  */
              stream->max_colors = tigetnum ("colors");
              stream->no_color_video = tigetnum ("ncv");
              stream->set_a_foreground = xstrdup0 (tigetstr ("setaf"));
              stream->set_foreground = xstrdup0 (tigetstr ("setf"));
              stream->set_a_background = xstrdup0 (tigetstr ("setab"));
              stream->set_background = xstrdup0 (tigetstr ("setb"));
              stream->orig_pair = xstrdup0 (tigetstr ("op"));
              stream->enter_bold_mode = xstrdup0 (tigetstr ("bold"));
              stream->enter_italics_mode = xstrdup0 (tigetstr ("sitm"));
              stream->exit_italics_mode = xstrdup0 (tigetstr ("ritm"));
              stream->enter_underline_mode = xstrdup0 (tigetstr ("smul"));
              stream->exit_underline_mode = xstrdup0 (tigetstr ("rmul"));
              stream->exit_attribute_mode = xstrdup0 (tigetstr ("sgr0"));
            }
          #elif HAVE_TERMCAP
          /* The buffer size needed for termcap was 1024 bytes in the past, but
             nowadays the largest termcap description (bq300-8-pc-w-rv) is 1507
             bytes long.  <https://tldp.org/LDP/lpg/node91.html> suggests a
             buffer size of 2048 bytes.  */
          struct { char buf[2048]; char canary[4]; } termcapbuf;
          int retval;

          /* Call tgetent, being defensive against buffer overflow.  */
          memcpy (termcapbuf.canary, "CnRy", 4);
          retval = tgetent (termcapbuf.buf, term);
          if (memcmp (termcapbuf.canary, "CnRy", 4) != 0)
            /* Buffer overflow!  */
            abort ();

          if (retval > 0)
            {
              /* The buffer size needed for a termcap entry was 1024 bytes in
                 the past, but nowadays the largest one (in bq300-8-pc-w-rv)
                 is 1034 bytes long.  */
              struct { char buf[2048]; char canary[4]; } termentrybuf;
              char *termentryptr;

              /* Prepare for calling tgetstr, being defensive against buffer
                 overflow.  ncurses' tgetstr() supports a second argument NULL,
                 but NetBSD's tgetstr() doesn't.  */
              memcpy (termentrybuf.canary, "CnRz", 4);
              #define TEBP ((termentryptr = termentrybuf.buf), &termentryptr)

              /* Retrieve particular values depending on the terminal type.  */
              stream->max_colors = tgetnum ("Co");
              stream->no_color_video = tgetnum ("NC");
              stream->set_a_foreground = xstrdup0 (tgetstr ("AF", TEBP));
              stream->set_foreground = xstrdup0 (tgetstr ("Sf", TEBP));
              stream->set_a_background = xstrdup0 (tgetstr ("AB", TEBP));
              stream->set_background = xstrdup0 (tgetstr ("Sb", TEBP));
              stream->orig_pair = xstrdup0 (tgetstr ("op", TEBP));
              stream->enter_bold_mode = xstrdup0 (tgetstr ("md", TEBP));
              stream->enter_italics_mode = xstrdup0 (tgetstr ("ZH", TEBP));
              stream->exit_italics_mode = xstrdup0 (tgetstr ("ZR", TEBP));
              stream->enter_underline_mode = xstrdup0 (tgetstr ("us", TEBP));
              stream->exit_underline_mode = xstrdup0 (tgetstr ("ue", TEBP));
              stream->exit_attribute_mode = xstrdup0 (tgetstr ("me", TEBP));

              #ifdef __BEOS__
              /* The BeOS termcap entry for "beterm" is broken: For "AF" and
                 "AB" it contains values in terminfo syntax but the system's
                 tparam() function understands only the termcap syntax.  */
              if (stream->set_a_foreground != NULL
                  && strcmp (stream->set_a_foreground, "\033[3%p1%dm") == 0)
                {
                  free (stream->set_a_foreground);
                  stream->set_a_foreground = xstrdup ("\033[3%dm");
                }
              if (stream->set_a_background != NULL
                  && strcmp (stream->set_a_background, "\033[4%p1%dm") == 0)
                {
                  free (stream->set_a_background);
                  stream->set_a_background = xstrdup ("\033[4%dm");
                }
              #endif

              /* The termcap entry for cygwin is broken: It has no "ncv" value,
                 but bold and underline are actually rendered through colors.  */
              if (strcmp (term, "cygwin") == 0)
                stream->no_color_video |= 2 | 32;

              /* Done with tgetstr.  Detect possible buffer overflow.  */
              #undef TEBP
              if (memcmp (termentrybuf.canary, "CnRz", 4) != 0)
                /* Buffer overflow!  */
                abort ();
            }
          #else
          /* Fallback code for platforms with neither the terminfo nor the
             termcap functions, such as mingw.
             Assume the ANSI escape sequences.  Extracted through
             "TERM=ansi infocmp", replacing \E with \033.  */
          stream->max_colors = 8;
          stream->no_color_video = 3;
          stream->set_a_foreground = xstrdup ("\033[3%p1%dm");
          stream->set_a_background = xstrdup ("\033[4%p1%dm");
          stream->orig_pair = xstrdup ("\033[39;49m");
          stream->enter_bold_mode = xstrdup ("\033[1m");
          stream->enter_underline_mode = xstrdup ("\033[4m");
          stream->exit_underline_mode = xstrdup ("\033[m");
          stream->exit_attribute_mode = xstrdup ("\033[0;10m");
          #endif

          /* AIX 4.3.2, IRIX 6.5, HP-UX 11, Solaris 7..10 all lack the
             description of color capabilities of "xterm" and "xterms"
             in their terminfo database.  But it is important to have
             color in xterm.  So we provide the color capabilities here.  */
          if (stream->max_colors <= 1
              && (strcmp (term, "xterm") == 0 || strcmp (term, "xterms") == 0))
            {
              stream->max_colors = 8;
              stream->set_a_foreground = xstrdup ("\033[3%p1%dm");
              stream->set_a_background = xstrdup ("\033[4%p1%dm");
              stream->orig_pair = xstrdup ("\033[39;49m");
            }
        }

      /* Infer the capabilities.  */
      stream->supports_foreground =
        (stream->max_colors >= 8
         && (stream->set_a_foreground != NULL || stream->set_foreground != NULL)
         && stream->orig_pair != NULL);
      stream->supports_background =
        (stream->max_colors >= 8
         && (stream->set_a_background != NULL || stream->set_background != NULL)
         && stream->orig_pair != NULL);
      stream->colormodel =
        (stream->supports_foreground || stream->supports_background
         ? (term != NULL
            && (/* Recognize xterm-16color, xterm-88color, xterm-256color.  */
                (strlen (term) >= 5 && memcmp (term, "xterm", 5) == 0)
                || /* Recognize *-16color.  */
                   (strlen (term) > 8
                    && strcmp (term + strlen (term) - 8, "-16color") == 0)
                || /* Recognize *-256color.  */
                   (strlen (term) > 9
                    && strcmp (term + strlen (term) - 9, "-256color") == 0)
                || /* Recognize *-direct.  */
                   (strlen (term) > 8
                    && strcmp (term + strlen (term) - 8, "-direct") == 0))
            ? (stream->max_colors >= 0x7fff ? cm_xtermrgb :
               stream->max_colors == 256 ? cm_xterm256 :
               stream->max_colors == 88 ? cm_xterm88 :
               stream->max_colors == 16 ? cm_xterm16 :
               cm_xterm8)
            : cm_common8)
         : cm_monochrome);
      stream->supports_weight =
        (stream->enter_bold_mode != NULL
         && stream->exit_attribute_mode != NULL);
      stream->supports_posture =
        (stream->enter_italics_mode != NULL
         && (stream->exit_italics_mode != NULL
             || stream->exit_attribute_mode != NULL));
      stream->supports_underline =
        (stream->enter_underline_mode != NULL
         && (stream->exit_underline_mode != NULL
             || stream->exit_attribute_mode != NULL));
      /* TODO: Use a terminfo capability, once ncurses implements it.  */
      stream->supports_hyperlink = should_enable_hyperlinks (term);

      /* Infer the restore strings.  */
      stream->restore_colors =
        (stream->supports_foreground || stream->supports_background
         ? stream->orig_pair
         : NULL);
      stream->restore_weight =
        (stream->supports_weight ? stream->exit_attribute_mode : NULL);
      stream->restore_posture =
        (stream->supports_posture
         ? (stream->exit_italics_mode != NULL
            ? stream->exit_italics_mode
            : stream->exit_attribute_mode)
         : NULL);
      stream->restore_underline =
        (stream->supports_underline
         ? (stream->exit_underline_mode != NULL
            ? stream->exit_underline_mode
            : stream->exit_attribute_mode)
         : NULL);
      stream->restore_hyperlink =
        (stream->supports_hyperlink
         ? "\033]8;;\033\\"
         : NULL);
    }

  /* Initialize the hyperlink id generator.  */
  if (stream->supports_hyperlink)
    {
      char *hostname = xgethostname ();
      { /* Compute a hash code, like in gnulib/lib/hash-pjw.c.  */
        uint32_t h = 0;
        if (hostname != NULL)
          {
            const char *p;
            for (p = hostname; *p; p++)
              h = (unsigned char) *p + ((h << 9) | (h >> (32 - 9)));
          }
        stream->hostname_hash = h;
      }
      free (hostname);

      {
        struct timeval tv;
        gettimeofday (&tv, NULL);
        stream->start_time =
          (uint64_t) tv.tv_sec * (uint64_t) 1000000 + (uint64_t) tv.tv_usec;
      }

      stream->id_serial = 0;
    }

  /* Initialize the set of hyperlink_t.  */
  stream->hyperlinks_array = NULL;
  stream->hyperlinks_count = 0;
  stream->hyperlinks_allocated = 0;

  /* Initialize the buffer.  */
  stream->allocated = 120;
  stream->buffer = XNMALLOC (stream->allocated, char);
  stream->attrbuffer = XNMALLOC (stream->allocated, attributes_t);
  stream->buflen = 0;

  /* Initialize the current attributes.  */
  {
    attributes_t assumed_default;
    attributes_t simplified_default;

    assumed_default.color = COLOR_DEFAULT;
    assumed_default.bgcolor = COLOR_DEFAULT;
    assumed_default.weight = WEIGHT_DEFAULT;
    assumed_default.posture = POSTURE_DEFAULT;
    assumed_default.underline = UNDERLINE_DEFAULT;
    assumed_default.hyperlink = NULL;

    simplified_default = simplify_attributes (stream, assumed_default);

    stream->default_attr = simplified_default;
    stream->active_attr = simplified_default;
    stream->curr_attr = assumed_default;
    stream->simp_attr = simplified_default;
  }

  /* Prepare tty control.  */
  activate_term_style_controller (&controller, stream, fd, tty_control);

  return stream;
}

/* Accessors.  */

static int
term_ostream::get_descriptor (term_ostream_t stream)
{
  return stream->fd;
}

static const char *
term_ostream::get_filename (term_ostream_t stream)
{
  return stream->filename;
}

static ttyctl_t
term_ostream::get_tty_control (term_ostream_t stream)
{
  return stream->tty_control;
}

static ttyctl_t
term_ostream::get_effective_tty_control (term_ostream_t stream)
{
  return stream->control_data.tty_control;
}

/* Instanceof test.  */

bool
is_instance_of_term_ostream (ostream_t stream)
{
  return IS_INSTANCE (stream, ostream, term_ostream);
}
