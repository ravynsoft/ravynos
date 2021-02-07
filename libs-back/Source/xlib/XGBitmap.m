/*
   XGBitmapImageRep.m

   NSBitmapImageRep for GNUstep GUI X/GPS Backend

   Copyright (C) 1996-1999 Free Software Foundation, Inc.

   Author:  Adam Fedor <fedor@colorado.edu>
   Author:  Scott Christley <scottc@net-community.com>
   Date: Feb 1996
   Author:  Felipe A. Rodriguez <far@ix.netcom.com>
   Date: May 1998
   Author:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: Mar 1999

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

#include <config.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <Foundation/NSData.h>
#include <Foundation/NSDebug.h>
#include "gsc/gscolors.h"
#include "xlib/XGPrivate.h"

/* Macros that make it easier to convert
   between (r,g,b) <--> pixels.
*/

#define VARIABLES_DECLARATION \
        unsigned char _rshift, _gshift, _bshift, _ashift; \
        unsigned int  _rmask,  _gmask,  _bmask,  _amask; \
        unsigned char _rwidth, _gwidth, _bwidth, _awidth



#define InitRGBShiftsAndMasks(rs,rw,gs,gw,bs,bw,as,aw) \
     do { \
	_rshift = (rs);              \
	_rmask  = (1<<(rw)) -1;      \
	_rwidth = (rw);              \
	_gshift = (gs);              \
	_gmask  = (1<<(gw)) -1;      \
	_gwidth = (gw);              \
	_bshift = (bs);              \
	_bmask  = (1<<(bw)) -1;      \
	_bwidth = (bw);              \
	_amask  = (1<<(aw)) -1;      \
	_ashift = (as);              \
	_awidth = (aw);              \
       } while (0)


#define PixelToRGB(pixel,r,g,b)  \
     do { \
	(r) = (pixel >> _rshift) & _rmask; \
	(g) = (pixel >> _gshift) & _gmask; \
	(b) = (pixel >> _bshift) & _bmask; \
        } while (0)

/* Note that RGBToPixel assumes that the
   r,g,b values are in the correct domain.
   If not, the value is nonsense.
*/

#define RGBToPixel(r,g,b, pixel) \
     do { \
        pixel = ((r) << _rshift)  \
               |((g) << _gshift)  \
               |((b) << _bshift); \
        } while (0)

#define CLAMP(a) \
    do { \
	(a) = MAX(0, MIN(255, (a))); \
    } while (0)

#define CSIZE 16384
#define GS_QUERY_COLOR(color)					\
  do {								\
    int centry = color.pixel % CSIZE;				\
    if (empty[centry] == NO && pixels[centry] == color.pixel)	\
      {								\
	color = colors[centry];					\
      }								\
    else							\
      {								\
	empty[centry] = NO;					\
	XQueryColor(context->dpy, context->cmap, &color);	\
	pixels[centry] = color.pixel;				\
	colors[centry] = color;					\
      }								\
  } while (0)

/* Composite source image (pixmap) onto a destination image with alpha.
   Only works for op=Sover now

   Assumptions:
    - all images are valid.
    - srect is completely contained in the source images
    - srect put at the origin is contained in the destination image
*/
int
_pixmap_combine_alpha(RContext *context,
                      RXImage *source_im, RXImage *source_alpha,
                      RXImage *dest_im, RXImage *dest_alpha,
                      XRectangle srect,
                      NSCompositingOperation op,
                      XGDrawMechanism drawMechanism,
		      float fraction)
{
  unsigned long  pixel;
  unsigned long  oldPixel = 0;

  fraction = MAX(0.0, MIN(1.0, fraction));

  if (drawMechanism == XGDM_FAST15
      || drawMechanism == XGDM_FAST16
      || drawMechanism == XGDM_FAST32
      || drawMechanism == XGDM_FAST32_BGR
      || drawMechanism == XGDM_FAST8)
    {
      VARIABLES_DECLARATION;
      unsigned	row;

      switch (drawMechanism)
	{
	case XGDM_FAST15:
	  InitRGBShiftsAndMasks(10,5,5,5,0,5,0,8);
	  break;
	case XGDM_FAST16:
	  InitRGBShiftsAndMasks(11,5,5,6,0,5,0,8);
	  break;
	case XGDM_FAST32:
	  InitRGBShiftsAndMasks(16,8,8,8,0,8,0,8);
	  break;
	case XGDM_FAST32_BGR:
	  InitRGBShiftsAndMasks(0,8,8,8,16,8,0,8);
	  break;
	case XGDM_FAST8:
	  InitRGBShiftsAndMasks(5,3,2,3,0,2,0,8);
	  break;
	default:
	  NSLog(@"Huh? Backend confused about XGDrawMechanism");
	  //Try something.  With a bit of luck we see
	  //which picture goes wrong.
	  InitRGBShiftsAndMasks(11,5,5,6,0,5,0,8);
	}

      for (row = 0; row < srect.height; row++)
	{
	  unsigned	col;

	  for (col = 0; col < srect.width; col++)
	    {
	      unsigned	sr, sg, sb, sa; // source
	      unsigned	dr, dg, db, da; // dest
	      unsigned	ialpha;

	      // Get the source pixel information
	      pixel = XGetPixel(source_im->image, srect.x+col, srect.y+row);
	      PixelToRGB(pixel, sr, sg, sb);

	      if (source_alpha)
		{
		  pixel = XGetPixel(source_alpha->image,
				    srect.x + col, srect.y + row);
		  sa = (pixel >> _ashift) & _amask;
		}
	      else
		sa = _amask;

	      if (sa == 0)	// dest wouldn't be changed
		continue;

	      if (fraction < 1.0)
		sa *= fraction;

	      sr *= sa;
	      sg *= sa;
	      sb *= sa;

      	      ialpha = (_amask - sa);

	      // Now get dest pixel
	      pixel = XGetPixel(dest_im->image, col, row);
	      PixelToRGB(pixel, dr, dg, db);

      	      dr *= ialpha;
      	      dg *= ialpha;
      	      db *= ialpha;
	      
	      if (dest_alpha)
		{
		  pixel = XGetPixel(dest_alpha->image, col, row);
		  da = (pixel >> _ashift) & _amask;
		}
	      else  // no alpha channel, background is opaque
		da = _amask;

	      dr = (sr + dr) / _amask;
	      dg = (sg + dg) / _amask;
	      db = (sb + db) / _amask;

	      // calc final alpha
	      if (sa == _amask || da == _amask)
		da = _amask;
	      else
		da = sa + ((da * ialpha) / _amask);

	      CLAMP(dr);
	      CLAMP(dg);
	      CLAMP(db);
	      CLAMP(da);

	      RGBToPixel(dr, dg, db, pixel);
	      XPutPixel(dest_im->image, col, row, pixel);
	      if (dest_alpha)
		XPutPixel(dest_alpha->image, col, row, da << _ashift);
	    }
	}
    }
  else
    {
      unsigned long pixels[CSIZE];
      XColor colors[CSIZE];
      BOOL empty[CSIZE];
      XColor		c2;
      unsigned		row;
      int cind;

      for (cind = 0; cind < CSIZE; cind++)
	{
	  empty[cind] = YES;
	}
      
      /*
       * This block of code should be totally portable as it uses the
       * 'official' X mechanism for converting from pixel values to
       * RGB color values - on the downside, it's very slow.
       */
      pixel = (unsigned long)-1;	// Never valid?
      c2.pixel = pixel;

       for (row = 0; row < srect.height; row++)
	{
	  unsigned	col;

	  for (col = 0; col < srect.width; col++)
	    {
	      int r, g, b, alpha;
	      XColor pcolor, acolor, c0, c1;
	      pcolor.pixel = XGetPixel(source_im->image, 
				       col + srect.x, row + srect.y);
	      GS_QUERY_COLOR(pcolor);
	      r = pcolor.red >> 8;
	      g = pcolor.green >> 8;
	      b = pcolor.blue >> 8;
	      alpha = 255;
	      if (source_alpha)
		{
		  acolor.pixel = XGetPixel(source_alpha->image,
				    col + srect.x, row + srect.y);
		  GS_QUERY_COLOR(acolor);
		  alpha = acolor.red >> 8;
		}
	      if (alpha == 0)
		continue;		// background unchanged.

	      if (fraction < 1)
		alpha *= fraction;
	      if (alpha == 255)
		{
		  c1 = pcolor;
		}
	      else
		{
		  unsigned short	ialpha = 255 - alpha;
		      
		  /*
		   * RGB color components in X are 16-bit values -
		   * but our bitmap contains 8-bit values so we must
		   * adjust our values up to 16-bit, which we can do
		   * by increasing their alpha component by 256.
		   */
		  alpha <<= 8;
		      
		  /*
		   * Get the background pixel and convert to RGB if
		   * we haven't already done the conversion earlier.
		   */
		  c0.pixel = XGetPixel(dest_im->image, col, row);
		  if (c0.pixel != oldPixel)
		    {
		      oldPixel = c0.pixel;
		      GS_QUERY_COLOR(c0);
		    }
		      
		  // mix in alpha to produce RGB out
		  c1.red = ((c0.red*ialpha) + (r*alpha))/255;
		  c1.green = ((c0.green*ialpha) + (g*alpha))/255;
		  c1.blue = ((c0.blue*ialpha) + (b*alpha))/255;
		}

	      /*
	       * Convert the X RGB value to a colormap entry if we
	       * don't already have one.  Then set the pixel.
	       * NB.  We will not necessarily get a
	       * color with exactly the rgb components we gave it, so
	       * we must record those components beforehand.
	       */
	      if (c2.pixel == (unsigned long)-1
		  || c2.red != c1.red
		  || c2.green != c1.green
		  || c2.blue != c1.blue)
		{
		  RColor rc;
		  c2 = c1;
		  rc.red = c1.red >> 8;
		  rc.green = c1.green >> 8;
		  rc.blue = c1.blue >> 8;
		  RGetClosestXColor(context, &rc, &c1);
		  c2.pixel = c1.pixel;
		}
	      XPutPixel(dest_im->image, col, row, c1.pixel);
	      if (dest_alpha)
		{
		  RColor rc;
		  XColor da;
		  /* Alpha gets mixed the same as all the
		     other color components */
		  da.pixel = XGetPixel(dest_alpha->image, col, row);
		  GS_QUERY_COLOR(da);
		  rc.red = acolor.red >> 8;
		  rc.red = rc.red + (da.red >> 8) * (256 - rc.red)/256;
		  rc.green = rc.blue = rc.red;
		  RGetClosestXColor(context, &rc, &da);
		  XPutPixel(dest_alpha->image, col, row, da.pixel);
		}
	    }
	}
    }
  
  return 0;
}

/*
 * The following structure holds the information necessary for unpacking
 * a bitmap data object. It holds an index into the raw data, precalculated
 * spans for magnification and minifaction, plus the buffer which holds
 * a complete line of colour to be output to the screen in RGBA form.
 */

struct _bitmap_decompose {
	unsigned char *plane[5];
  unsigned int bit_off[5];
	long image_w, image_h, screen_w, screen_h;
	int bps, spp, bpp, bpr;
	BOOL has_alpha, is_direct_packed, one_is_black;
	int cspace, pro_mul;

	unsigned char *r, *g, *b, *a;
	int cur_image_row, cur_screen_row;
	int first_vis_col, last_vis_col;

	unsigned int *row_starts, *row_ends, *col_starts, *col_ends;
	unsigned int *r_sum, *g_sum, *b_sum, *a_sum, *pix_count;
};

/*
 * Here we extract a value a given number of bits wide from a bit
 * offset into a block of memory starting at "base". The bit numbering
 * is assumed to be such that a bit offset of zero and a width of 4 gives
 * the upper 4 bits of the first byte, *not* the lower 4 bits. We do allow
 * the value to cross a byte boundary, though it is unclear as to whether
 * this is strictly necessary for OpenStep tiffs.
 */

static int
_get_bit_value(unsigned char *base, long msb_off, int bit_width)
{
  long lsb_off, byte1, byte2;
  int shift, value;

  /*
   * Firstly we calculate the position of the msb and lsb in terms
   * of bit offsets and thus byte offsets. The shift is the number of
   * spare bits left in the byte containing the lsb
   */
  lsb_off = msb_off + bit_width - 1;
  byte1 = msb_off / 8;
  byte2 = lsb_off / 8;
  shift = 7-(lsb_off % 8);

  /*
   * We now get the value from the byte array, possibly using two bytes if
   * the required set of bits crosses the byte boundary. This is then shifted
   * down to it's correct position and extraneous bits masked off before
   * being returned.
   */
  value = base[byte2];
  if (byte1 != byte2)
    value |= base[byte1] << 8;
  value >>= shift;

  return value & ((1 << bit_width) - 1);
}

/*
 * Extract a single pixel from a row. We are passed addresses for the red,
 * green, blue and alpha components, along with the column number and all the
 * necessary information to access the raw data. This function is responsible
 * for extracting the raw data and converting it to 8 bit RGB for return so
 * as to present a unified interface to the higher functions.
 */

static void
_get_image_pixel(int col, unsigned char *r, unsigned char *g,
	unsigned char *b, unsigned char *a,
	unsigned char **planes, unsigned int *bit_off,
	int spp, int bpp, int bps,
	int pro_mul, int cspace, BOOL has_alpha, BOOL one_is_black)
{
  int p, values[5]; 
  long off = col * bpp;

  for (p = 0; p < spp; p++)
    {
      int raw_value = _get_bit_value(planes[p], off + bit_off[p], bps);
      values[p] = raw_value * pro_mul;
    }
	
  /* deal with the alpha */
  *a = has_alpha ? values[spp-1] : 255;

  /* handle the colourspace */
  switch (cspace)
    {
      case rgb_colorspace:
        *r = values[0];
        *g = values[1];
        *b = values[2];
        break;
      case cmyk_colorspace:
        *r = MAX(0, 255 - (values[0] + values[3]));
        *g = MAX(0, 255 - (values[1] + values[3]));
        *b = MAX(0, 255 - (values[2] + values[3]));
        break;
      case gray_colorspace:
        *r = *g = *b = (one_is_black ? (255 - values[0]) : values[0]);
        break;
    }
}

/*
 * Main image decomposing function. This function creates the next row
 * in the image that needs to be output to the screen. For direct packed
 * images it simply copies the data directly, for all others it runs through
 * a set of loops pulling out image values and forming the avregae colour in 8
 * it RGB for that particular screen pixel from the underlying image pixels,
 * no matter what format they are in.
 */

static void
_create_image_row(struct _bitmap_decompose *img)
{
  if (img->cur_screen_row >= img->screen_h)
   {
      NSLog(@"Tried to create too many screen rows");
      return;
   }

  if (img->is_direct_packed)
    /* do direct copy, only limited formats supported */
    { 
      unsigned char *ptr = img->plane[0];
      unsigned char *rptr = img->r;
      unsigned char *gptr = img->g;
      unsigned char *bptr = img->b;
      unsigned char *aptr = img->a;
      BOOL oib = img->one_is_black;
      BOOL ha = img->has_alpha;
      BOOL is_grey = (img->cspace == gray_colorspace);
      int i, fc = img->first_vis_col, lc = img->last_vis_col;

      /* do the offset from the right */
      ptr += fc * ((is_grey ? 1 : 3) + (ha ? 1 : 0));
      rptr += fc;
      gptr += fc;
      bptr += fc;
      aptr += fc;

      for (i = fc;i <= lc; i++)
        {
          *rptr = *ptr++;
	  if (is_grey)
            {
	      if (oib)
		*rptr = 255 - *rptr;
	      *gptr = *bptr = *rptr;
            }
	  else
	    {
	      *gptr = *ptr++;
	      *bptr = *ptr++;
	    }
          *aptr = ha ? *ptr++ : 255;	/* opaque default */

	  rptr++;
	  gptr++;
	  bptr++;
	  aptr++;
        }

      img->plane[0] += img->bpr;
      img->cur_image_row++;
    }
  else
    /* do the full averaging row creation */
    {
      int tr, sc;
      int s_row = img->row_starts[img->cur_screen_row];
      int e_row = img->row_ends[img->cur_screen_row];
      BOOL zero_count = YES;

      /* local copies of stuff we use a lot to avoid indirect */
      unsigned char **plane = img->plane;
      unsigned int *bit_off = img->bit_off;
      unsigned int *col_starts = img->col_starts;
      unsigned int *col_ends = img->col_ends;
      unsigned int *r_sum = img->r_sum;
      unsigned int *g_sum = img->g_sum;
      unsigned int *b_sum = img->b_sum;
      unsigned int *a_sum = img->a_sum;
      unsigned int *pix_count = img->pix_count;
      unsigned char *rptr = img->r;
      unsigned char *gptr = img->g;
      unsigned char *bptr = img->b;
      unsigned char *aptr = img->a;
      int spp = img->spp;
      int bpp = img->bpp;
      int bps = img->bps;
      int bpr = img->bpr;
      int pro_mul = img->pro_mul;
      int cspace = img->cspace;
      BOOL ha = img->has_alpha;
      BOOL oib = img->one_is_black;
      int fc = img->first_vis_col, lc = img->last_vis_col;


      /* loop for each required row */
      zero_count = YES;
      for (tr = s_row; tr <= e_row; tr++)
        {
          /* move to the required image row */
          while (img->cur_image_row < tr)
            {
              int p;
              for (p = 0; p < spp; p++)
                plane[p] += bpr;
              img->cur_image_row++;
            }

	  /* for each screen pixel */
	  for (sc = fc; sc <= lc; sc++)
	    {
	      int s_col = col_starts[sc];
	      int e_col = col_ends[sc];
	      int tc;

	      if (zero_count)
		{
		  r_sum[sc] = 0;
		  g_sum[sc] = 0;
		  b_sum[sc] = 0;
		  a_sum[sc] = 0;
		  pix_count[sc] = 0;
		}

	      /* for each image pixel */
	      for (tc = s_col; tc <= e_col; tc++)
		{
		  unsigned char r, g, b, a;
		  _get_image_pixel(tc, &r, &g, &b, &a,
				plane, bit_off, spp, bpp, bps,
				pro_mul, cspace, ha, oib);
		  r_sum[sc] += r;
		  g_sum[sc] += g;
		  b_sum[sc] += b;
		  a_sum[sc] += a;
		  pix_count[sc]++;
		}
	    }

	  zero_count =NO;
        }

	/*
	 * Build the line by averaging. As integer divide always
	 * rounds down we can get the effect of adding 0.5 before
	 * trucating by adding the value of the count right shifted
	 * one bit, thus giving us the nearest value and therefore a
	 * better approximation t the colour we hope.
	 */
	for (sc = fc; sc <= lc; sc++)
	  {
	    int count = pix_count[sc];
	    int half = count >> 1;
	    rptr[sc] = (r_sum[sc] + half) / count;
	    gptr[sc] = (g_sum[sc] + half) / count;
	    bptr[sc] = (b_sum[sc] + half) / count;
	    aptr[sc] = (a_sum[sc] + half) / count;
	  }
    }

  img->cur_screen_row++;
}

/*
 * Set the ranges covered by a pixel within the image. Given the source
 * number of pixels and the destination number of pixels we calculate which
 * pixels in the source are more than 50% overlapped by each pixel in the
 * destination and record the start and end of the range. For mappings where
 * the source is being magnified this will only be a single pixel, for others
 * it may be one or more pixels, spaced evenly along the line. These are
 * the pixels which will then be averaged to make the best guess colour for
 * the destination pixel. As this is a slow process then a flag is passed in
 * which will cause the nearest pixel alorithm that is used for magnification
 * to be applied to minificationas well. The result looks rougher, but is much
 * faster and proprtional to the size of the output rather than the input
 * image.
 */

static void
_set_ranges(long src_len, long dst_len,
		unsigned int *start_ptr, unsigned int *end_ptr, BOOL fast_min)
{
  float dst_f = (float)dst_len;
  int d;

  if (fast_min || (src_len <= dst_len))
    /* magnifying */
    {
      float src_f = (float)src_len;
      for (d = 0; d < dst_len; d++)
        {
          int middle = (int)((((float)d + 0.5) * src_f) / dst_f);
          *start_ptr++ = middle;
          *end_ptr++ = middle;
          if (middle >= src_len)
            NSLog(@"Problem with magnification!");
        }
    }
  else
    {
      int start = 0;
      for (d = 0; d < dst_len; d++)
        {
          int end_i = (int)(0.5 + (((d+1) * src_len) / dst_f));

          if ((end_i > src_len) || (end_i < 1))
            NSLog(@"Problem with minification!");
          *start_ptr++ = start;
          *end_ptr++ = end_i - 1;
          start = end_i;
        }
    }
}

/*
 * Combine the image data with the currentonscreen data and push the
 * result back to the screen. The screen handling code is almost identical
 * to the original code. The function assumes that the displayable rectangle
 * is always a subset of the complete screen rectangle which is the area
 * in pixels that would be covered by the entire image. This is used to 
 * calculate the scaling required.
 */

int
_bitmap_combine_alpha(RContext *context,
		unsigned char * data_planes[5],
		int width, int height,
		int bits_per_sample, int samples_per_pixel,
		int bits_per_pixel, int bytes_per_row,
		int colour_space, BOOL one_is_black,
		BOOL is_planar, BOOL has_alpha, BOOL fast_min,
		RXImage *dest_im, RXImage *dest_alpha,
		XRectangle srect, XRectangle drect,
		NSCompositingOperation op,
		XGDrawMechanism drawMechanism)
{
  struct _bitmap_decompose img;
  XColor	c0;
  XColor	c1;
  int		col_shift = drect.x - srect.x;
  int		row_shift = drect.y - srect.y;

  /* Sanity check on colourspace and number of colours */
  {
    int num_of_colours = samples_per_pixel - (has_alpha ? 1 : 0);
    switch(colour_space)
      {
        case hsb_colorspace:
          NSLog(@"HSB colourspace not supported for images");
          return -1;
        case rgb_colorspace:
          if (num_of_colours != 3)
            {
              NSLog(@"Bad number of colour planes - %d", num_of_colours);
              NSLog(@"RGB colourspace requires three planes excluding alpha");
              return -1;
            }
          break;
        case cmyk_colorspace:
          if (num_of_colours != 4)
            {
              NSLog(@"Bad number of colour planes - %d", num_of_colours);
              NSLog(@"CMYK colourspace requires four planes excluding alpha");
              return -1;
            }
          break;
        case gray_colorspace:
          if (num_of_colours != 1)
            {
              NSLog(@"Bad number of colour planes - %d", num_of_colours);
              NSLog(@"Gray colourspace requires one plane excluding alpha");
              return -1;
            }
          break;
        default:
          NSLog(@"Unknown colourspace found");
          return -1;
      }
  }

  /* bitmap decomposition structure */
  img.bps = bits_per_sample;
  img.bpp = bits_per_pixel;
  img.spp = samples_per_pixel;
  img.bpr = bytes_per_row;
  img.image_w = width;
  img.image_h = height;
  img.screen_w = srect.width;
  img.screen_h = srect.height;
  img.has_alpha = has_alpha;
  img.one_is_black = one_is_black;
  img.cspace = colour_space;
  img.cur_image_row = 0;
  img.cur_screen_row = 0;

  /*
   * Promotion value, this is what the samples need to
   * be mutiplied by to get an 8 bit value. This is done
   * rather than shifting to fill in the lower bits properly.
   * The values for 6, 5 and 3 bits should be floats by rights,
   * but the difference is negligble and for speed we want to use
   * integers.
   */
   switch (bits_per_sample)
     {
        case 16:
        case 8:
          img.pro_mul = 1;
          break;
        case 7:
          img.pro_mul = 2;
          break;
        case 6:
          img.pro_mul = 4;	/* should be 4.05 */
          break;
        case 5:
          img.pro_mul = 8;	/* should be 8.226 */
          break;
        case 4:
          img.pro_mul = 17;
          break;
        case 3:
          img.pro_mul = 36;	/* should be 36.43 */
          break;
        case 2:
          img.pro_mul = 85;
          break;
        case 1:
          img.pro_mul = 255;
          break;
        default:
          NSLog(@"Bizarre number of bits per sample %d", bits_per_sample);
          return -1;
     }

  /*
   * Handle planar or meshed data. We hold an array of
   * base addresses and bit offsets for all formats.
   * Thus for meshed data the bases are the same with
   * increasing bit offsets, but for planar data the bases
   * are those of the planes, with the bit offset always being
   * zero.
   */
  {
    int i;

    /* zero them */
    for (i=0;i<5;i++)
      {
        img.plane[i] = NULL;
        img.bit_off[i] = 0;
      }

    /* set as appropriate */
    if (is_planar)
      for (i=0;i<img.spp;i++)
        img.plane[i] = data_planes[i];
    else
      for (i=0;i<img.spp;i++)
       {
         img.plane[i] = data_planes[0];
         img.bit_off[i] = i * img.bps;
       }
  }

  /*
   * Set the range mappings. If the data can be copied at 1:1
   * then a flag is set to allow us to do this quickly. Else a set
   * of ranges are produced. Each of these correspons to the pixels
   * within the image that are behind each point on the screen. Rows
   * are then composed by averaging all the image points behind a screen
   * pixel to give the illusion of smooth minification. For a magnification
   * the range boundaries are equal and thus a "nearest" function is
   * produced such that the image pixels become visible as it is enlarged.
   * As the calculation of the ranges requires floats we do it once here
   * and then re-use the values calculated on each row to avoid excessive
   * floating point calculation.
   */

  if (!is_planar && (img.screen_w == img.image_w)
  	&& (img.screen_h == img.image_h) && (img.bps == 8)
	&& ((img.cspace == gray_colorspace) || (img.cspace == rgb_colorspace))
	&& ((img.bpr * 8) == (img.bps * img.spp * img.image_w)))
    {
      img.is_direct_packed = YES;
    }
  else
    {
      img.is_direct_packed = NO;

      img.row_starts = malloc(img.screen_h * sizeof(int));
      img.row_ends = malloc(img.screen_h * sizeof(int));
      _set_ranges(img.image_h, img.screen_h,
	  img.row_starts, img.row_ends, fast_min);

      img.col_starts = malloc(img.screen_w * sizeof(int));
      img.col_ends = malloc(img.screen_w * sizeof(int));
      _set_ranges(img.image_w, img.screen_w,
	  img.col_starts, img.col_ends, fast_min);

      img.r_sum = malloc(img.screen_w * sizeof(int));
      img.g_sum = malloc(img.screen_w * sizeof(int));
      img.b_sum = malloc(img.screen_w * sizeof(int));
      img.a_sum = malloc(img.screen_w * sizeof(int));
      img.pix_count = malloc(img.screen_w * sizeof(int));
    }

  /* skip the top rows if a shift is needed */
  if (row_shift)
    {
      if (img.is_direct_packed)	/* need to move data */
	{
	  int i;
	  for (i=0;i<img.spp;i++)
	    img.plane[i] += row_shift * img.bpr;
	}
      img.cur_screen_row = row_shift;
    }

  /* set the visible segment of the row */
  img.first_vis_col = col_shift;
  img.last_vis_col = img.first_vis_col + drect.width -1;

  /* make space for the row buffers */
  img.r = malloc(img.screen_w);
  img.g = malloc(img.screen_w);
  img.b = malloc(img.screen_w);
  img.a = malloc(img.screen_w);

  {
    unsigned long pixel;

    /* Two cases, the *_FAST* method, which
       is covered in the first a part of the if
       clause.
       When no FAST method is available (or recognized)
       use the `official' X mechanism.  This is
       covered in the else clause */

    if (drawMechanism == XGDM_FAST15
	|| drawMechanism == XGDM_FAST16
	|| drawMechanism == XGDM_FAST32
	|| drawMechanism == XGDM_FAST32_BGR
	|| drawMechanism == XGDM_FAST8)
      {
	VARIABLES_DECLARATION;
	unsigned	row;

	switch (drawMechanism)
	  {
	  case XGDM_FAST15:
	    InitRGBShiftsAndMasks(10,5,5,5,0,5,0,8);
	    break;
	  case XGDM_FAST16:
	    InitRGBShiftsAndMasks(11,5,5,6,0,5,0,8);
	    break;
	  case XGDM_FAST32:
	    InitRGBShiftsAndMasks(16,8,8,8,0,8,0,8);
	    break;
	  case XGDM_FAST32_BGR:
	    InitRGBShiftsAndMasks(0,8,8,8,16,8,0,8);
	    break;
	  case XGDM_FAST8:
	    InitRGBShiftsAndMasks(5,3,2,3,0,2,0,8);
	    break;
	  default:
	    NSLog(@"Huh? Backend confused about XGDrawMechanism");
	    //Try something.  With a bit of luck we see
	    //which picture goes wrong.
	    InitRGBShiftsAndMasks(11,5,5,6,0,5,0,8);
	  }

	for (row = 0; row < drect.height; row++)
	  {
	    unsigned	col;
	    unsigned	char *rptr = img.r + col_shift;
	    unsigned	char *gptr = img.g + col_shift;
	    unsigned	char *bptr = img.b + col_shift;
	    unsigned	char *aptr = img.a + col_shift;

	    _create_image_row(&img);
	
	    for (col = 0; col < drect.width; col++)
	      {
		unsigned short	sr = (*rptr++ >> (8 - _rwidth));
		unsigned short	sg = (*gptr++ >> (8 - _gwidth));
		unsigned short	sb = (*bptr++ >> (8 - _bwidth));
		unsigned short	sa = (*aptr++ >> (8 - _awidth));
		unsigned	dr, dg, db, da; // dest

		if (sa == 0)	// dest wouldn't be changed
		  continue;

      	      	if (sa == _amask)  // source only, don't bother with the rest
		  {
      	      	    // Yes, this is duplicated code -- but it's worth it.
		    RGBToPixel(sr, sg, sb, pixel);
		    XPutPixel(dest_im->image, col, row, pixel);
		    if (dest_alpha)
		      XPutPixel(dest_alpha->image, col, row, sa << _ashift);
		    continue;
		  }

		// get the destination pixel
		pixel = XGetPixel(dest_im->image, col, row);
		PixelToRGB(pixel, dr, dg, db);

		if (dest_alpha)
		  {
		    pixel = XGetPixel(dest_alpha->image, col, row);
		    da = (pixel >> _ashift) & _amask;
		  }
		else  // no alpha channel, background is opaque
		  da = _amask;

      	      	if (da == 0)
		  {
      	      	    /*
		     * Unscale the colors
		     */
		    dr = (sr * _amask) / sa;
		    dg = (sg * _amask) / sa;
		    db = (sb * _amask) / sa;
		    da = sa;
		  }
		else
		  {
		    unsigned  ialpha = _amask - sa;

		    dr = (sr * sa + (dr * ialpha)) / _amask;
		    dg = (sg * sa + (dg * ialpha)) / _amask;
		    db = (sb * sa + (db * ialpha)) / _amask;
		    if (da == _amask || da == _amask)
		      da = _amask;
		    else
		      da = sa + ((da * ialpha) / _amask);
		  }

		CLAMP(dr);
		CLAMP(dg);
		CLAMP(db);
		CLAMP(da);

		RGBToPixel(dr, dg, db, pixel);
		XPutPixel(dest_im->image, col, row, pixel);
		if (dest_alpha)
		  XPutPixel(dest_alpha->image, col, row, da << _ashift);
	      }
	  }
      }
    else
      {
	XColor	 c2, a2;
	RColor   rc, rc2;
	unsigned row, oldAlpha = 65537;
	unsigned long pixels[CSIZE];
	XColor colors[CSIZE];
	BOOL empty[CSIZE];
	int cind;

	for (cind = 0; cind < CSIZE; cind++)
	  {
	    empty[cind] = YES;
	  }

	/*
	 * This block of code should be totally portable as it uses the
	 * 'official' X mechanism for converting from pixel values to
	 * RGB color values - on the downside, it's very slow.
	 */
	pixel = (unsigned long)-1;	// Never valid?
	c1.pixel = c2.pixel = a2.pixel = pixel;
	
	for (row = 0; row < drect.height; row++)
	  {
	    unsigned	col;
	    unsigned	char *rptr = img.r + col_shift;
	    unsigned	char *gptr = img.g + col_shift;
	    unsigned	char *bptr = img.b + col_shift;
	    unsigned	char *aptr = img.a + col_shift;

	    _create_image_row(&img);
	
	    for (col = 0; col < drect.width; col++)
	      {
		unsigned short r = *rptr++;
		unsigned short g = *gptr++;
		unsigned short b = *bptr++;
		unsigned short alpha = *aptr++;;

		if (has_alpha)
		  {
		    if (alpha != oldAlpha)
		      {
			oldAlpha = alpha;
			rc.red = rc.green = rc.blue = alpha;
			RGetClosestXColor(context, &rc, &a2);
		      }
		    if (dest_alpha)
		      {
			XColor da;
			/* Alpha gets mixed the same as all the
			   other color components */
			da.pixel = XGetPixel(dest_alpha->image, col, row);
			GS_QUERY_COLOR(da);
			rc.red = alpha + (da.red >> 8) * (256 - alpha)/256;
			rc.green = rc.blue = rc.red;
			RGetClosestXColor(context, &rc, &da);
			XPutPixel(dest_alpha->image, col, row, da.pixel);
		      }
		    if (alpha == 0)
		      continue;		// background unchanged.
		
		    if (alpha == 255)
		      {
			rc.red = r;
			rc.green = g;
			rc.blue = b;
		      }
		    else
		      {
			unsigned short	ialpha = 255 - alpha;
			
			/*
			 * RGB color components in X are 16-bit values -
			 * but our bitmap contains 8-bit values so we must
			 * adjust our values up to 16-bit, which we can do
			 * by increasing their alpha component by 256.
			 */
			alpha <<= 8;
			
			/*
			 * Get the background pixel and convert to RGB if
			 * we haven't already done the conversion earlier.
			 */
			c0.pixel = XGetPixel(dest_im->image, col, row);
			if (c0.pixel != c2.pixel)
			  {
			    c2.pixel = c0.pixel;
			    GS_QUERY_COLOR(c0);
			  }
			
			// mix in alpha to produce RGB out
			rc.red = ((c0.red*ialpha) + (r*alpha))/(255*256);
			rc.green = ((c0.green*ialpha) + (g*alpha))/(255*256);
			rc.blue = ((c0.blue*ialpha) + (b*alpha))/(255*256);
		      }
		  }
		else
		  {
		    /* Not using alpha, but we still have to set it
		       in the pixmap */
		    if (a2.pixel == pixel)
		      {
			rc.red = rc.green = rc.blue = 255;
			RGetClosestXColor(context, &rc, &a2);
		      }
		    if (dest_alpha)
		      XPutPixel(dest_alpha->image, col, row, a2.pixel);
		    rc.red = r;
		    rc.green = g;
		    rc.blue = b;
		  }
		
		/*
		 * Convert the X RGB value to a colormap entry if we
		 * don't already have one.  Then set the pixel.
		 * NB.  We will not necessarily get a
		 * color with exactly the rgb components we gave it, so
		 * we must record those components beforehand.
		 */
		if (c1.pixel == pixel
		    || rc.red != rc2.red
		    || rc.green != rc2.green
		    || rc.blue != rc2.blue)
		  {
		    rc2 = rc;
		    RGetClosestXColor(context, &rc, &c1);
		  }
		XPutPixel(dest_im->image, col, row, c1.pixel);
	      }
	  }
      }
  }

  /* free used memory */
  free(img.r);
  free(img.g);
  free(img.b);
  free(img.a);
  if (!img.is_direct_packed)
    {
      free(img.row_starts);
      free(img.row_ends);
      free(img.col_starts);
      free(img.col_ends);
      free(img.r_sum);
      free(img.g_sum);
      free(img.b_sum);
      free(img.a_sum);
      free(img.pix_count);
    }
  return 0;
}

NSData *
_pixmap_read_alpha(RContext *context,
		   RXImage *source_im, RXImage *source_alpha,
		   XRectangle srect,
		   XGDrawMechanism drawMechanism)
{
  unsigned long  pixel;
  NSMutableData *data;
  unsigned char *bytes;
  int spp;

  spp = (source_alpha) ? 4 : 3;
  data = [NSMutableData dataWithLength: srect.width*srect.height*spp];
  if (data == nil)
    return nil;
  bytes = [data mutableBytes];

  if (drawMechanism == XGDM_FAST15
      || drawMechanism == XGDM_FAST16
      || drawMechanism == XGDM_FAST32
      || drawMechanism == XGDM_FAST32_BGR
      || drawMechanism == XGDM_FAST8)
    {
      VARIABLES_DECLARATION;
      unsigned	row;

      switch (drawMechanism)
	{
	case XGDM_FAST15:
	  InitRGBShiftsAndMasks(10,5,5,5,0,5,0,8);
	  break;
	case XGDM_FAST16:
	  InitRGBShiftsAndMasks(11,5,5,6,0,5,0,8);
	  break;
	case XGDM_FAST32:
	  InitRGBShiftsAndMasks(16,8,8,8,0,8,0,8);
	  break;
	case XGDM_FAST32_BGR:
	  InitRGBShiftsAndMasks(0,8,8,8,16,8,0,8);
	  break;
	case XGDM_FAST8:
	  InitRGBShiftsAndMasks(5,3,2,3,0,2,0,8);
	  break;
	default:
	  NSLog(@"Huh? Backend confused about XGDrawMechanism");
	  //Try something.  With a bit of luck we see
	  //which picture goes wrong.
	  InitRGBShiftsAndMasks(11,5,5,6,0,5,0,8);
	}

      for (row = 0; row < srect.height; row++)
	{
	  unsigned	col;

	  for (col = 0; col < srect.width; col++)
	    {
	      unsigned	sr, sg, sb, sa;

	      // Get the source pixel information
	      pixel = XGetPixel(source_im->image, col, row);
	      PixelToRGB(pixel, sr, sg, sb);
	      // Expand to 8 bit value
	      sr = (sr << (8-_rwidth));
	      sg = (sg << (8-_gwidth));
	      sb = (sb << (8-_bwidth));

	      if (source_alpha)
		{
		  pixel = XGetPixel(source_alpha->image, col, row);
		  sa = (pixel >> _ashift) & _amask;
		}
	      else
		sa = _amask;

	      bytes[(row * srect.width + col)*spp]   = sr;
	      bytes[(row * srect.width + col)*spp+1] = sg;
	      bytes[(row * srect.width + col)*spp+2] = sb;
	      if (source_alpha)
		bytes[(row * srect.width + col)*spp+3] = sa;
	    }
	}
    }
  else
    {
      unsigned row;
      unsigned long pixels[CSIZE];
      XColor colors[CSIZE];
      BOOL empty[CSIZE];
      int cind;
      
      for (cind = 0; cind < CSIZE; cind++)
	{
	  empty[cind] = YES;
	}

      /*
       * This block of code should be totally portable as it uses the
       * 'official' X mechanism for converting from pixel values to
       * RGB color values - on the downside, it's very slow.
       */
      pixel = (unsigned long)-1;	// Never valid?

      for (row = 0; row < srect.height; row++)
	{
	  unsigned	col;

	  for (col = 0; col < srect.width; col++)
	    {
	      int r, g, b, alpha;
	      XColor pcolor, acolor;
	      pcolor.pixel = XGetPixel(source_im->image, col, row);
	      GS_QUERY_COLOR(pcolor);
	      r = pcolor.red >> 8;
	      g = pcolor.green >> 8;
	      b = pcolor.blue >> 8;
	      alpha = 255;
	      if (source_alpha)
		{
		  acolor.pixel = XGetPixel(source_alpha->image, col, row);
		  GS_QUERY_COLOR(acolor);
		  alpha = acolor.red >> 8;
		}

	      bytes[(row * srect.width + col)*spp]   = r;
	      bytes[(row * srect.width + col)*spp+1] = g;
	      bytes[(row * srect.width + col)*spp+2] = b;
	      if (source_alpha)
		bytes[(row * srect.width + col)*spp+3] = alpha;
	    }
	}
    }

  return (NSData *)data;
}

