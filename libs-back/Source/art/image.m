/*
   Copyright (C) 2002 Free Software Foundation, Inc.

   Author:  Alexander Malmberg <alexander@malmberg.org>

   This file is part of GNUstep.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

/*
Image drawing. DPSimage and helpers.
*/

#include <math.h>

#include <AppKit/NSAffineTransform.h>
#include <AppKit/NSGraphics.h>

#include "ARTGState.h"

#ifndef RDS
#include "x11/XWindowBuffer.h"
#endif
#include "blit.h"


static unsigned int _get_8_bits(const unsigned char *ptr, int bit_ofs,
	int num_bits)
{
/*
TODO: if we get values with more than 8 bits, we should round properly and
not just discard the extra bits
*/
  int i;
  unsigned int v;
  ptr += bit_ofs / 8;
  bit_ofs %= 8;

  v = 0;

  /* if we are handling 16 bit values we optimize */


  if (num_bits == 16)
    {
#if (GS_WORDS_BIGENDIAN==0)
      ptr++;
#endif 
      v = *ptr;
      return v;
    }

  
  for (i = 0; i < 8 && i < num_bits; i++)
    {
      v <<= 1;
      if ((*ptr) & (128 >> bit_ofs))
        v |= 1;
      bit_ofs++;
      if (bit_ofs == 8)
        {
          ptr++;
          bit_ofs = 0;
        }
    }
  /* extend what we've got to 8 bits */
  switch (num_bits)
    {
    case 1:
      v *= 255;
      break;
    case 2:
      v *= 85;
      break;
    case 3:
      v = (v << 5) | (v << 2) | (v >> 1);
      break;
    case 4:
      v = (v << 4) | v;
      break;
    case 5:
      v = (v << 3) | (v >> 2);
      break;
    case 6:
      v = (v << 2) | (v >> 4);
      break;
    case 7:
      v = (v << 1) | (v >> 6);
      break;
    }
  return v;
}


typedef struct
{
  int width, height;
  int bits_per_sample, samples_per_pixel, bits_per_pixel, bytes_per_row;
  BOOL is_planar, has_alpha;
  const unsigned char **data;

  /*
    0  unknown, use colorspacename
    1  rgb
    2  cmyk
    3  gray, 1=white
    4  gray, 1=black
  */
  int colorspace;
  NSString *colorspacename;
} image_info_t;


static void _image_get_color_rgb_8(image_info_t *ii, render_run_t *ri,
        int x, int y)
{
  int ofs;

  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x >= ii->width) x = ii->width - 1;
  if (y >= ii->height) y = ii->height - 1;

  ofs = ii->bytes_per_row * y + x * ii->bits_per_pixel / 8;
  if (ii->is_planar)
    {
      ri->r = ii->data[0][ofs];
      ri->g = ii->data[1][ofs];
      ri->b = ii->data[2][ofs];
      if (ii->has_alpha)
        ri->a = ii->data[3][ofs];
      else
        ri->a = 255;
    }
  else
    {
      ri->r = ii->data[0][ofs];
      ri->g = ii->data[0][ofs + 1];
      ri->b = ii->data[0][ofs + 2];
      if (ii->has_alpha)
        ri->a = ii->data[0][ofs + 3];
      else
        ri->a = 255;
    }
  /* Undo premultiply  */
  if (ri->a && ri->a != 255)
    {
      ri->r = (255 * ri->r) / ri->a;
      ri->g = (255 * ri->g) / ri->a;
      ri->b = (255 * ri->b) / ri->a;
    }
}

static void _image_get_color_rgb_cmyk_gray(image_info_t *ii, render_run_t *ri,
        int x, int y)
{
  int ofs, bit_ofs;
  int values[5];
  int i, j;

  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x >= ii->width) x = ii->width - 1;
  if (y >= ii->height) y = ii->height - 1;

  ofs = y * ii->bytes_per_row;
  bit_ofs = x * ii->bits_per_pixel;

  for (i = j = 0; i < ii->samples_per_pixel; i++)
    {
      values[i] = _get_8_bits(ii->data[j] + ofs, bit_ofs, ii->bits_per_sample);
      if (ii->is_planar)
        j++;
      else
        bit_ofs += ii->bits_per_sample;
    }
  if (ii->has_alpha)
    ri->a = values[i - 1];
  else
    ri->a = 255;

  if (ii->colorspace == 1)
    {
      ri->r = values[0];
      ri->g = values[1];
      ri->b = values[2];
    }
  else if (ii->colorspace == 2)
    {
      j = 255 - values[0] - values[3];
      ri->r = j < 0?0:j;
      j = 255 - values[1] - values[3];
      ri->g = j < 0?0:j;
      j = 255 - values[2] - values[3];
      ri->b = j < 0?0:j;
    }
  else if (ii->colorspace == 3)
    {
      ri->r = ri->g = ri->b = values[0];
    }
  else if (ii->colorspace == 4)
    {
      ri->r = ri->g = ri->b = 255 - values[0];
    }
  /* Undo premultiply  */
  if (ri->a && ri->a != 255)
    {
      ri->r = (255 * ri->r) / ri->a;
      ri->g = (255 * ri->g) / ri->a;
      ri->b = (255 * ri->b) / ri->a;
    }
}


@implementation ARTGState (image)

-(void) _image_do_rgb_transform: (image_info_t *)ii
        : (NSAffineTransform *)matrix
        : (void (*)(image_info_t *ii, render_run_t *ri, int x, int y))ifunc
{
/*
TODO:
seems to have problems with rotations. 0, 90, 180, 270 work fine, but others
seem to cause edges to be off by a pixel
*/
  int x[4], y[4];
  int tx[4], ty[4];
  float fx[4], fy[4];

  int cy, ey;
  int left_delta, next;
  int lx, lx_frac, ldx, ldx_frac, l_de, le;
  int rx, rx_frac, rdx, rdx_frac, r_de, re;
  int ltx, lty, ltx_frac, lty_frac, ldtx, ldty, ldtx_frac, ldty_frac;
  int rtx, rty, rtx_frac, rty_frac, rdtx, rdty, rdtx_frac, rdty_frac;

  NSPoint p;

  void (*render_run)(render_run_t *ri, int num);


  if (wi->has_alpha)
    render_run = RENDER_RUN_ALPHA_A;
  else
    render_run = RENDER_RUN_ALPHA;


  p = [matrix transformPoint: NSMakePoint(0, 0)];
  fx[0] = p.x; fy[0] = p.y;
  p = [matrix transformPoint: NSMakePoint(ii->width, 0)];
  fx[1] = p.x; fy[1] = p.y;
  p = [matrix transformPoint: NSMakePoint(ii->width, ii->height)];
  fx[2] = p.x; fy[2] = p.y;
  p = [matrix transformPoint: NSMakePoint(0, ii->height)];
  fx[3] = p.x; fy[3] = p.y;

  if (fabs(fx[0] - floor(fx[0] + .5)) < 0.001) fx[0] = floor(fx[0] + .5);
  if (fabs(fx[1] - floor(fx[1] + .5)) < 0.001) fx[1] = floor(fx[1] + .5);
  if (fabs(fx[2] - floor(fx[2] + .5)) < 0.001) fx[2] = floor(fx[2] + .5);
  if (fabs(fx[3] - floor(fx[3] + .5)) < 0.001) fx[3] = floor(fx[3] + .5);
  if (fabs(fy[0] - floor(fy[0] + .5)) < 0.001) fy[0] = floor(fy[0] + .5);
  if (fabs(fy[1] - floor(fy[1] + .5)) < 0.001) fy[1] = floor(fy[1] + .5);
  if (fabs(fy[2] - floor(fy[2] + .5)) < 0.001) fy[2] = floor(fy[2] + .5);
  if (fabs(fy[3] - floor(fy[3] + .5)) < 0.001) fy[3] = floor(fy[3] + .5);

  x[0] = floor(fx[0]) - offset.x; y[0] = offset.y - floor(fy[0]);
  x[1] = floor(fx[1]) - offset.x; y[1] = offset.y - floor(fy[1]);
  x[2] = floor(fx[2]) - offset.x; y[2] = offset.y - floor(fy[2]);
  x[3] = floor(fx[3]) - offset.x; y[3] = offset.y - floor(fy[3]);

  tx[0] = 0;         ty[0] = ii->height;
  tx[1] = ii->width; ty[1] = ii->height;
  tx[2] = ii->width; ty[2] = 0;
  tx[3] = 0;         ty[3] = 0;

  cy = y[le = 0];
  if (y[1] < cy) cy = y[le = 1];
  if (y[2] < cy) cy = y[le = 2];
  if (y[3] < cy) cy = y[le = 3];
  re = le;

  ey = y[0];
  if (y[1] > ey) ey = y[1];
  if (y[2] > ey) ey = y[2];
  if (y[3] > ey) ey = y[3];

  if (x[(le + 1) & 3] < x[(le - 1) & 3])
    left_delta = 1;
  else
    left_delta = -1;

/*  printf("x/y (%i %i) (%i %i) (%i %i) (%i %i)  t (%i %i) (%i %i) (%i %i) (%i %i)  le=%i\n",
        x[0],y[0],
        x[1],y[1],
        x[2],y[2],
        x[3],y[3],
        tx[0],ty[0],
        tx[1],ty[1],
        tx[2],ty[2],
        tx[3],ty[3],
        le);*/

  /* silence the compiler */
  lx = lx_frac = ldx = ldx_frac = l_de = 0;
  rx = rx_frac = rdx = rdx_frac = r_de = 0;
  ltx = lty = ltx_frac = lty_frac = ldtx = ldty = ldtx_frac = ldty_frac = 0;
  rtx = rty = rtx_frac = rty_frac = rdtx = rdty = rdtx_frac = rdty_frac = 0;

  while (cy <= ey && cy < clip_y1)
    {
      if (cy == y[le])
        {
          next = (le + left_delta) & 3;
          while (y[le] == y[next])
            {
              le = next;
              next = (le + left_delta) & 3;
            }
          l_de = y[next] - y[le];
          lx = x[le];
          lx_frac = 0;
          ldx = (x[next] - x[le]) / l_de;
          ldx_frac = (x[next] - x[le]) % l_de;

          ltx = tx[le];
          lty = ty[le];
          ltx_frac = 0;
          lty_frac = 0;
          ldtx = (tx[next] - tx[le]) / l_de;
          ldty = (ty[next] - ty[le]) / l_de;
          ldtx_frac = (tx[next] - tx[le]) % l_de;
          ldty_frac = (ty[next] - ty[le]) % l_de;

          le = next;
        }
      else
        {
          lx += ldx;
          lx_frac += ldx_frac;
          if (lx_frac < 0)
            lx--, lx_frac += l_de;
          if (lx_frac > l_de)
            lx++, lx_frac -= l_de;

          ltx += ldtx;
          ltx_frac += ldtx_frac;
          if (ltx_frac < 0)
            ltx--, ltx_frac += l_de;
          if (ltx_frac > l_de)
            ltx++, ltx_frac -= l_de;

          lty += ldty;
          lty_frac += ldty_frac;
          if (lty_frac < 0)
            lty--, lty_frac += l_de;
          if (lty_frac > l_de)
            lty++, lty_frac -= l_de;
        }

      if (cy == y[re])
        {
          next = (re - left_delta) & 3;
          while (y[re] == y[next])
            {
              re = next;
              next = (re - left_delta) & 3;
            }
          r_de = y[next] - y[re];
          rx = x[re];
          rx_frac = r_de - 1; /* TODO? */
          rdx = (x[next] - x[re]) / r_de;
          rdx_frac = (x[next] - x[re]) % r_de;

          rtx = tx[re];
          rty = ty[re];
          rtx_frac = 0;
          rty_frac = 0;
          rdtx = (tx[next] - tx[re]) / r_de;
          rdty = (ty[next] - ty[re]) / r_de;
          rdtx_frac = (tx[next] - tx[re]) % r_de;
          rdty_frac = (ty[next] - ty[re]) % r_de;

          re = next;
        }
      else
        {
          rx += rdx;
          rx_frac += rdx_frac;
          if (rx_frac < 0)
            rx--, rx_frac += r_de;
          if (rx_frac > r_de)
            rx++, rx_frac -= r_de;

          rtx += rdtx;
          rtx_frac += rdtx_frac;
          if (rtx_frac < 0)
            rtx--, rtx_frac += r_de;
          if (rtx_frac > r_de)
            rtx++, rtx_frac -= r_de;

          rty += rdty;
          rty_frac += rdty_frac;
          if (rty_frac < 0)
            rty--, rty_frac += r_de;
          if (rty_frac > r_de)
            rty++, rty_frac -= r_de;
        }

/*      printf("cy=%i  left(x=%i, tx=%i, ty=%i)  right(x=%i, tx=%i, ty=%i)\n",
             cy,
             lx,ltx,lty,
             rx,rtx,rty);*/

      if (cy >= clip_y0 && rx > lx)
        {
          render_run_t ri;
          int x0, x1, de;
          int tx, ty, tx_frac, ty_frac, dtx, dty, dtx_frac, dty_frac;
          int delta;
          int scale_factor;

          x0 = lx;
          x1 = rx;
          de = x1 - x0;

          tx = ltx;
          ty = lty;
          if (r_de * l_de > 0x10000)
            {
              /*
              If the numbers involved are really large, scale things down
              a bit. This loses some accuracy, but should keep things in
              range.
              */
              scale_factor = 10;
              tx_frac = ltx_frac * ((de * r_de) >> scale_factor);
              ty_frac = lty_frac * ((de * r_de) >> scale_factor);
              de *= (r_de * l_de) >> scale_factor;
            }
          else
            {
              tx_frac = ltx_frac * de * r_de;
              ty_frac = lty_frac * de * r_de;
              scale_factor = 0;
              de *= r_de * l_de;
            }

          delta = ((rtx * r_de + rtx_frac) >> scale_factor) * l_de
                  - ((ltx * l_de + ltx_frac) >> scale_factor) * r_de;
          dtx = delta / de;
          dtx_frac = delta % de;

          delta = ((rty * r_de + rty_frac) >> scale_factor) * l_de
                  - ((lty * l_de + lty_frac) >> scale_factor) * r_de;
          dty = delta / de;
          dty_frac = delta % de;

/*          printf("r_de=%i l_de=%i de=%i   dtx=%i dtx_frac=%i  dty=%i dty_frac=%i\n",
                 r_de,l_de,de,dtx,dtx_frac,dty,dty_frac);
          printf(" start at x(%i %i) y(%i %i)\n",
                 tx,tx_frac,ty,ty_frac);*/

          /*
            x0 x1 / x2 -> y0 y1 / y2  z
            ((y0 * y2 + y1) * x2 - (x0 * x2 + x1) * y2) / (x2 * y2 * z)
          */

          /*
          TODO: Would like to use this code instead for speed, but
          the multiplications dtx*(clip_x0-x0) may overflow. Need to figure
          out a way of doing this safely, or add checks for 'large numbers'
          so we can use this in most cases, at least.
          */
/*          if (x0 < clip_x0)
            {
              tx += dtx * (clip_x0 - x0);
              ty += dty * (clip_x0 - x0);
              tx_frac += dtx_frac * (clip_x0 - x0);
              while (tx_frac < 0)
                tx--, tx_frac += de;
              while (tx_frac >= de)
                tx++, tx_frac -= de;
              ty_frac += dty_frac * (clip_x0 - x0);
              while (ty_frac < 0)
                ty--, ty_frac += de;
              while (ty_frac >= de)
                ty++, ty_frac -= de;
              x0 = clip_x0;
            }*/
          if (x0 < clip_x0)
            {
              tx += dtx * (clip_x0 - x0);
              ty += dty * (clip_x0 - x0);
              while (x0 < clip_x0)
                {
                  tx_frac += dtx_frac;
                  if (tx_frac < 0)
                    tx--, tx_frac += de;
                  if (tx_frac >= de)
                    tx++, tx_frac -= de;
                  ty_frac += dty_frac;
                  if (ty_frac < 0)
                    ty--, ty_frac += de;
                  if (ty_frac >= de)
                    ty++, ty_frac -= de;
                  x0++;
                }
            }
          if (x1 > clip_x1) x1 = clip_x1;

          ri.dst = wi->data + x0 * DI.bytes_per_pixel
                   + cy * wi->bytes_per_line;
          ri.dsta = wi->alpha + x0 + cy * wi->sx;

          if (!clip_span)
            {
              for (; x0 < x1; x0++, ri.dst += DI.bytes_per_pixel, ri.dsta++)
                {
                  ifunc(ii, &ri, tx, ty);
                  render_run(&ri, 1);

                  tx += dtx;
                  ty += dty;
                  tx_frac += dtx_frac;
                  if (tx_frac < 0)
                    tx--, tx_frac += de;
                  if (tx_frac >= de)
                    tx++, tx_frac -= de;
                  ty_frac += dty_frac;
                  if (ty_frac < 0)
                    ty--, ty_frac += de;
                  if (ty_frac >= de)
                    ty++, ty_frac -= de;
                }
            }
          else
            {
              unsigned int *span, *end;
              BOOL state = NO;

              span = &clip_span[clip_index[cy - clip_y0]];
              end = &clip_span[clip_index[cy - clip_y0 + 1]];

              x0 -= clip_x0;
              x1 -= clip_x0;
              while (span != end && *span < x0)
                {
                  state = !state;
                  span++;
                  if (span == end)
                    break;
                }
              if (span != end)
                {
                  for (; x0 < x1; x0++, ri.dst += DI.bytes_per_pixel, ri.dsta++)
                    {
                      if (x0 == *span)
                        {
                          span++;
                          state = !state;
                          if (span == end)
                            break;
                        }
                    
                      if (state)
                        {
                          ifunc(ii, &ri, tx, ty);
                          render_run(&ri, 1);
                        }

                      tx += dtx;
                      ty += dty;
                      tx_frac += dtx_frac;
                      if (tx_frac < 0)
                        tx--, tx_frac += de;
                      if (tx_frac >= de)
                        tx++, tx_frac -= de;
                      ty_frac += dty_frac;
                      if (ty_frac < 0)
                        ty--, ty_frac += de;
                      if (ty_frac >= de)
                        ty++, ty_frac -= de;
                    }
                }
            }
        }

      cy++;
    }
}


- (void)DPSimage: (NSAffineTransform *) matrix
                : (NSInteger) pixelsWide : (NSInteger) pixelsHigh
                : (NSInteger) bitsPerSample : (NSInteger) samplesPerPixel 
                : (NSInteger) bitsPerPixel : (NSInteger) bytesPerRow : (BOOL) isPlanar
                : (BOOL) hasAlpha : (NSString *) colorSpaceName
                : (const unsigned char *const [5]) data
{
  BOOL identity_transform, is_rgb;
  image_info_t ii;
  NSAffineTransformStruct        ts;

  if (!wi || !wi->data) return;
  if (all_clipped) return;

  [matrix prependTransform: ctm];
  ts = [matrix transformStruct];
  if (fabs(ts.m11 - 1.0) < 0.001 && fabs(ts.m12) < 0.001
    && fabs(ts.m22 - 1.0) < 0.001 && fabs(ts.m21) < 0.001)
    identity_transform = YES;
  else
    identity_transform = NO;

  if (colorSpaceName == NSDeviceRGBColorSpace ||
      colorSpaceName == NSCalibratedRGBColorSpace)
    is_rgb = YES;
  else
    is_rgb = NO;

  /* optimize common case */
  if (identity_transform && is_rgb &&
      !clip_span &&
      bitsPerSample == 8 && !isPlanar &&
      bytesPerRow == samplesPerPixel * pixelsWide &&
      ((samplesPerPixel == 3 && bitsPerPixel == 24 && !hasAlpha) ||
       (samplesPerPixel == 4 && bitsPerPixel == 32 && hasAlpha)))
    {
      int x, y, ox, oy;
      const unsigned char *src = data[0];
      unsigned char *alpha_dest;
      render_run_t ri;

      if (wi->has_alpha)
        {
          if (DI.inline_alpha)
            alpha_dest = wi->data + DI.inline_alpha_ofs;
          else
            alpha_dest = wi->alpha;
        }
      else
        alpha_dest = NULL;

      ox = [matrix transformPoint: NSMakePoint(0, 0)].x - offset.x;
      oy = offset.y - [matrix transformPoint: NSMakePoint(0, 0)].y - pixelsHigh;

      for (y = 0; y < pixelsHigh; y++)
        {
          for (x = 0; x < pixelsWide; x++)
            {
              if (x + ox < clip_x0 || x + ox >= clip_x1 ||
                  y + oy < clip_y0 || y + oy >= clip_y1)
                {
                  if (hasAlpha)
                    src += 4;
                  else
                    src += 3;
                  continue;
                }
              ri.dst = wi->data + (x + ox) * DI.bytes_per_pixel
                       + (y + oy) * wi->bytes_per_line;
              ri.dsta = wi->alpha + (x + ox) + (y + oy) * wi->sx;
              ri.r = src[0];
              ri.g = src[1];
              ri.b = src[2];
              if (hasAlpha)
                {
                  ri.a = src[3];
                  /* Undo premultiply  */
                  if (ri.a && ri.a != 255)
                    {
                      ri.r = (255 * ri.r) / ri.a;
                      ri.g = (255 * ri.g) / ri.a;
                      ri.b = (255 * ri.b) / ri.a;
                    }
                  if (alpha_dest)
                    {
                      if (src[3] == 255)
                        RENDER_RUN_OPAQUE_A(&ri, 1);
                      else if (src[3])
                        RENDER_RUN_ALPHA_A(&ri, 1);
                    }
                  else
                    {
                      if (src[3] == 255)
                        RENDER_RUN_OPAQUE(&ri, 1);
                      else if (src[3])
                        RENDER_RUN_ALPHA(&ri, 1);
                    }
                  src += 4;
                }
              else
                {
                  ri.a = 255;
                  if (alpha_dest)
                    RENDER_RUN_OPAQUE_A(&ri, 1);
                  else
                    RENDER_RUN_OPAQUE(&ri, 1);
                  src += 3;
                }
            }
        }
      UPDATE_UNBUFFERED
      return;
    }

  ii.bits_per_sample = bitsPerSample;
  ii.bits_per_pixel = bitsPerPixel;
  ii.is_planar = isPlanar;
  ii.has_alpha = hasAlpha;
  ii.width = pixelsWide;
  ii.height = pixelsHigh;
  ii.samples_per_pixel = samplesPerPixel;
  ii.bytes_per_row = bytesPerRow;
  ii.data = (const unsigned char **)data;

  if (bitsPerSample == 8 && is_rgb &&
      ((samplesPerPixel == 3 && !hasAlpha) ||
       (samplesPerPixel == 4 && hasAlpha)))
    {
      [self _image_do_rgb_transform: &ii : matrix : _image_get_color_rgb_8];
      UPDATE_UNBUFFERED
      return;
    }

  if (is_rgb)
    ii.colorspace = 1;
  else if (colorSpaceName == NSDeviceCMYKColorSpace)
    ii.colorspace = 2;
  else if (colorSpaceName == NSDeviceWhiteColorSpace ||
           colorSpaceName == NSCalibratedWhiteColorSpace)
    ii.colorspace = 3;
  else if (colorSpaceName == NSDeviceBlackColorSpace ||
           colorSpaceName == NSCalibratedBlackColorSpace)
    ii.colorspace = 4;
  else
    ii.colorspace = 0;

  if (ii.colorspace != 0)
    {
      [self _image_do_rgb_transform: &ii : matrix :
        _image_get_color_rgb_cmyk_gray];
      UPDATE_UNBUFFERED
      return;
    }

  NSLog(@"unimplemented DPSimage  %lix%li  |%@|  bips=%li spp=%li bipp=%li bypr=%li  planar=%i alpha=%i\n",
        pixelsWide, pixelsHigh, matrix,
        bitsPerSample, samplesPerPixel, bitsPerPixel, bytesPerRow, isPlanar,
        hasAlpha);
}

@end

