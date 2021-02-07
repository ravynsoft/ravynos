/*
   Copyright (C) 2003 Free Software Foundation, Inc.

   Author:  Alexander Malmberg <alexander@malmberg.org>

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

/*
this code is rather experimental
*/

#include <math.h>

#include "ARTGState.h"

#ifndef RDS
#include "x11/XWindowBuffer.h"
#endif
#include "blit.h"

#include <Foundation/NSData.h>
#include <Foundation/NSDebug.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSValue.h>
#include <AppKit/NSAffineTransform.h>
#include <AppKit/NSGraphics.h>



/* TODO: share this with composite.m */

/*
Handle compositing in transformed coordinate spaces. _rect_setup sets up
the a rect_trace_t structure, and each call to _rect_advance returns YES
and the left and right points on the next row, or NO if all rows are done.
*/
typedef struct
{
  int x[4], y[4];
  int cy, ey;

  int left_delta;
  int lx, lx_frac, ldx, ldx_frac, l_de, le;
  int rx, rx_frac, rdx, rdx_frac, r_de, re;

  int cx0, cx1;
} rect_trace_t;

static void _rect_setup(rect_trace_t * t, NSRect r, int cx0, int cx1, 
  NSAffineTransform * ctm, int up, int *y0, NSPoint offset)
{
  float fx[4], fy[4];
  NSPoint p;

  t->cx0 = cx0;
  t->cx1 = cx1;

  p = r.origin;
  p = [ctm transformPoint: p];
  fx[0] = p.x; fy[0] = p.y;
  p = r.origin; p.x +=  r.size.width;
  p = [ctm transformPoint: p];
  fx[1] = p.x; fy[1] = p.y;
  p = r.origin; p.x +=  r.size.width; p.y +=  r.size.height;
  p = [ctm transformPoint: p];
  fx[2] = p.x; fy[2] = p.y;
  p = r.origin; p.y +=  r.size.height;
  p = [ctm transformPoint: p];
  fx[3] = p.x; fy[3] = p.y;

  if (fabs(fx[0] - floor(fx[0] + .5)) < 0.001) fx[0] = floor(fx[0] + .5);
  if (fabs(fx[1] - floor(fx[1] + .5)) < 0.001) fx[1] = floor(fx[1] + .5);
  if (fabs(fx[2] - floor(fx[2] + .5)) < 0.001) fx[2] = floor(fx[2] + .5);
  if (fabs(fx[3] - floor(fx[3] + .5)) < 0.001) fx[3] = floor(fx[3] + .5);
  if (fabs(fy[0] - floor(fy[0] + .5)) < 0.001) fy[0] = floor(fy[0] + .5);
  if (fabs(fy[1] - floor(fy[1] + .5)) < 0.001) fy[1] = floor(fy[1] + .5);
  if (fabs(fy[2] - floor(fy[2] + .5)) < 0.001) fy[2] = floor(fy[2] + .5);
  if (fabs(fy[3] - floor(fy[3] + .5)) < 0.001) fy[3] = floor(fy[3] + .5);

  t->x[0] = floor(fx[0]) - offset.x; t->y[0] = offset.y - floor(fy[0]);
  t->x[1] = floor(fx[1]) - offset.x; t->y[1] = offset.y - floor(fy[1]);
  t->x[2] = floor(fx[2]) - offset.x; t->y[2] = offset.y - floor(fy[2]);
  t->x[3] = floor(fx[3]) - offset.x; t->y[3] = offset.y - floor(fy[3]);

  /* If we're tracing the 'other way', we just flip the y -coordinates
  and unflip when returning them */
  if (up)
    {
      t->y[0] = -t->y[0];
      t->y[1] = -t->y[1];
      t->y[2] = -t->y[2];
      t->y[3] = -t->y[3];
    }

  t->cy = t->y[t->le = 0];
  if (t->y[1] < t->cy) t->cy = t->y[t->le = 1];
  if (t->y[2] < t->cy) t->cy = t->y[t->le = 2];
  if (t->y[3] < t->cy) t->cy = t->y[t->le = 3];
  t->re = t->le;

  t->ey = t->y[0];
  if (t->y[1] > t->ey) t->ey = t->y[1];
  if (t->y[2] > t->ey) t->ey = t->y[2];
  if (t->y[3] > t->ey) t->ey = t->y[3];

  if (t->x[(t->le + 1) & 3] < t->x[(t->le - 1) & 3])
    t->left_delta = 1;
  else
    t->left_delta = -1;

  /* silence the compiler */
  t->lx = t->lx_frac = t->ldx = t->ldx_frac = t->l_de = 0;
  t->rx = t->rx_frac = t->rdx = t->rdx_frac = t->r_de = 0;

  if (up)
    * y0 = -t->cy;
  else
    * y0 = t->cy;
}

static BOOL _rect_advance(rect_trace_t * t, int *x0, int *x1)
{
  int next;

  if (t->cy > t->ey)
    return NO;

  if (t->cy == t->y[t->le])
    {
      next = (t->le + t->left_delta) & 3;
      if (t->y[t->le] == t->y[next])
	{
	  t->le = next;
	  next = (t->le + t->left_delta) & 3;
	}
      t->l_de = t->y[next] - t->y[t->le];
      if (!t->l_de)
	return NO;
      t->lx = t->x[t->le];
      t->lx_frac = 0;
      t->ldx = (t->x[next] - t->x[t->le]) / t->l_de;
      t->ldx_frac = (t->x[next] - t->x[t->le]) % t->l_de;

      t->le = next;
    }
  else
    {
      t->lx +=  t->ldx;
      t->lx_frac +=  t->ldx_frac;
      if (t->lx_frac < 0)
	t->lx --, t->lx_frac +=  t->l_de;
      if (t->lx_frac > t->l_de)
	t->lx ++, t->lx_frac -= t->l_de;
    }

  if (t->cy == t->y[t->re])
    {
      next = (t->re - t->left_delta) & 3;
      if (t->y[t->re] == t->y[next])
	{
	  t->re = next;
	  next = (t->re - t->left_delta) & 3;
	}
      t->r_de = t->y[next] - t->y[t->re];
      if (!t->r_de)
	return NO;
      t->rx = t->x[t->re];
      t->rx_frac = t->r_de - 1; /* TODO? */
      t->rdx = (t->x[next] - t->x[t->re]) / t->r_de;
      t->rdx_frac = (t->x[next] - t->x[t->re]) % t->r_de;

      t->re = next;
    }
  else
    {
      t->rx +=  t->rdx;
      t->rx_frac +=  t->rdx_frac;
      if (t->rx_frac < 0)
	t->rx --, t->rx_frac +=  t->r_de;
      if (t->rx_frac > t->r_de)
	t->rx ++, t->rx_frac -= t->r_de;
    }

  if (t->rx > t->lx && t->rx >= t->cx0 && t->lx < t->cx1)
    {
      * x0 = t->lx - t->cx0;
      if (* x0 < 0)
	* x0 = 0;
      * x1 = t->rx - t->cx0;
      if (* x1 > t->cx1 - t->cx0)
	* x1 = t->cx1 - t->cx0;
    }
  else
    {
      * x0 = *x1 = 0;
    }

  t->cy ++;

  return YES;
}


@implementation ARTGState (shfill)


typedef struct function_s
{
  /* General information about the function. */
  int num_in, num_out;
  void (* eval)(struct function_s *f, double *in, double *out);

  double * domain; /* num_in * 2 */
  double * range; /* num_out * 2 */

  /* Type specific information */
  int * size; /* num_in */
  const unsigned char * data_source;
  int bits_per_sample;
  double * encode; /* num_in * 2 */
  double * decode; /* num_out * 2 */


  /* sample cache for in == 2, out == 3 */
  int sample_index[2];
  double sample_cache[4][3];
} function_t;


static double function_getsample(function_t * f, int sample, int i)
{
  double v;

  if (f->bits_per_sample == 8)
    {
  //    printf("get at %i \n", sample * f->num_out +i);
      v = f->data_source[sample * f->num_out +i] /255.0;
  //    printf("got %g \n", v);
    }
  else if (f->bits_per_sample == 16)
    {
      int c0, c1;
      c0 = f->data_source[(sample * f->num_out +i) * 2 +0];
      c1 = f->data_source[(sample * f->num_out +i) * 2 + 1];
      v =(c0 * 256 +c1)/65535.0;
    }
  else
    {
      NSLog(@"unhandled bits per sample %i", f->bits_per_sample);
      v = 0.0;
    }

  v = f->decode[i * 2] + v * (f->decode[i * 2 + 1] - f->decode[i * 2]);
  if (v < f->range[i * 2]) v = f->range[i * 2];
  if (v > f->range[i * 2 + 1]) v = f->range[i * 2 + 1];
  return v;
}

static void function_eval(function_t * f, double *a_in, double *out)
{
  double in[f->num_in];
  int sample[f->num_in];
  int i, j, sample_index, sample_factor;
  unsigned int u, v;
  double c;

  for (i = 0; i < f->num_in; i ++)
    {
      in[i] =(a_in[i]-f->domain[i * 2])/(f->domain[i * 2 + 1]-f->domain[i * 2]);
      if (in[i] <0.0) in[i] = 0.0;
      if (in[i] >1.0) in[i] = 1.0;

      in[i] = f->encode[i * 2]+in[i]* (f->encode[i * 2 + 1]-f->encode[i * 2]);
      sample[i] = floor(in[i]);
      /* we only want sample[i] == f->size[i] -1 when f->size[i] == 1 */
      if (sample[i] >= f->size[i]-1) sample[i] = f->size[i]-2;
      if (sample[i] <0) sample[i] = 0;

      in[i] = in[i]-sample[i];
      if (in[i] <0.0) in[i] = 0.0;
      if (in[i] >1.0) in[i] = 1.0;

  //    printf("  coord %i, sample %i, frac %g \n", i, sample[i], in[i]);
    }

  for (i = 0; i<f->num_out; i ++)
    {
      double out_value;
      /*
      iterate over all corners in the f->num_in -dimensional
      hypercube we're in
      */
      out_value = 0.0;
      for (u = 0; u<1<<f->num_in; u ++)
	{
	  sample_index = 0;
	  sample_factor = 1;
	  c = 1;
	  for (v = 1, j = 0; j<f->num_in; j ++, v<<= 1)
	    {
	      sample_index += sample[j] * sample_factor;
	      if (u&v)
		{
		  c *= in[j];
		  sample_index += sample_factor;
		}
	      else
		c *=(1.0 -in[j]);
	      sample_factor *= f->size[j];
	      if (c == 0.0)
		break;
	    }
    //      printf("    %08x  index %i, factor %i, c =%g \n", u, sample_index, sample_factor, c);
	  if (c>0.0)
	    out_value += c * function_getsample(f, sample_index, i);
	}
  //    printf("  final =%g \n", out_value);
      out[i] = out_value;
    }
}


/*
special case: f->num_in == 2, f->num_out == 3
*/
static void function_eval_in2_out3(function_t * f, double *a_in, double *out)
{
  double in[2];
  int sample[2];
  int i;

  for (i = 0; i<2; i ++)
    {
      in[i] =(a_in[i]-f->domain[i * 2])/(f->domain[i * 2 + 1]-f->domain[i * 2]);
      if (in[i] <0.0) in[i] = 0.0;
      if (in[i] >1.0) in[i] = 1.0;

      in[i] = f->encode[i * 2]+in[i]* (f->encode[i * 2 + 1]-f->encode[i * 2]);
      sample[i] = floor(in[i]);
      /* we only want sample[i] == f->size[i]-1 when f->size[i] == 1 */
      if (sample[i] >= f->size[i]-1) sample[i] = f->size[i]-2;
      if (sample[i] <0) sample[i] = 0;

      in[i] = in[i]-sample[i];
      if (in[i] <0.0) in[i] = 0.0;
      if (in[i] >1.0) in[i] = 1.0;

  //    printf("  coord %i, sample %i, frac %g \n", i, sample[i], in[i]);
    }

  if (sample[0] != f->sample_index[0] || sample[1] != f->sample_index[1])
    {
      f->sample_index[0] = sample[0];
      f->sample_index[1] = sample[1];

      for (i = 0; i < 3; i ++)
	{
	  f->sample_cache[0][i] = function_getsample(f,
	    sample[0] + (sample[1]) * f->size[0], i);
	  if (sample[0] + 1 < f->size[0])
	    f->sample_cache[1][i] = function_getsample(f,
	      sample[0] + 1 + (sample[1]) * f->size[0], i);
	  if (sample[1] + 1 < f->size[1])
	    f->sample_cache[2][i] = function_getsample(f,
	      sample[0] + (sample[1] + 1) * f->size[0], i);
	  if (sample[0] + 1 < f->size[0] && sample[1] + 1 < f->size[1])
	    f->sample_cache[3][i] = function_getsample(f,
	      sample[0] + 1 +(sample[1] + 1) * f->size[0], i);
	}
    }

  for (i = 0; i<3; i ++)
    {
      double out_value;
      double A, B, C, D;
      double p, q, pq;

      A = f->sample_cache[0][i];
      B = f->sample_cache[1][i];
      C = f->sample_cache[2][i];
      D = f->sample_cache[3][i];

      out_value = 0.0;
      p = in[0];
      q = in[1];
      pq = p * q;
      if (p!= 1.0 && q!= 1.0) out_value += A * (1 -p -q +pq);
      if (p!= 0.0 && q!= 1.0) out_value += B * (p -pq);
      if (p!= 1.0 && q!= 0.0) out_value += C * (q -pq);
      if (p!= 0.0 && q!= 0.0) out_value += D * pq;

      out[i] = out_value;
    }
}


static BOOL function_setup(NSDictionary * d, function_t *f)
{
  NSNumber * v =[d objectForKey: @"FunctionType"];
  NSArray * a;
  NSData * data;
  int i, j;

  if ([v intValue]!= 0)
  {
    NSDebugLLog(@"GSArt -shfill", @"FunctionType!= 0 not supported.");
    return NO;
  }

  memset(f, 0, sizeof(function_t));

  a =[d objectForKey: @"Size"];
  f->num_in =[a count];
  if (!f->num_in)
  {
    NSDebugLLog(@"GSArt -shfill", @"Size has no entries.");
    return NO;
  }

  f->num_out =[[d objectForKey: @"Range"] count] /2;
  if (!f->num_out)
  {
    NSDebugLLog(@"GSArt -shfill", @"Range has no entries.");
    return NO;
  }

  f->bits_per_sample =[[d objectForKey: @"BitsPerSample"] intValue];
  if (!(f->bits_per_sample == 8 || f->bits_per_sample == 16))
  {
    NSDebugLLog(@"GSArt -shfill", @"BitsPerSample other than 8 or 16 aren't supported.");
    return NO;
  }

  data =[d objectForKey: @"DataSource"];
  if (!data || ![data isKindOfClass: [NSData class]])
  {
    NSDebugLLog(@"GSArt -shfill", @"No valid DataSource given.");
    return NO;
  }
  f->data_source =[data bytes];

  f->size = malloc(sizeof(int) * f->num_in);
  f->domain = malloc(sizeof(double) * f->num_in * 2);
  f->range = malloc(sizeof(double) * f->num_out * 2);
  f->encode = malloc(sizeof(double) * f->num_in * 2);
  f->decode = malloc(sizeof(double) * f->num_out * 2);

  if (!f->size || !f->domain || !f->range || !f->encode || !f->decode)
  {
    free(f->size);
    f->size = NULL;
    free(f->domain);
    f->domain = NULL;
    free(f->range);
    f->range = NULL;
    free(f->encode);
    f->encode = NULL;
    free(f->decode);
    f->decode = NULL;
    NSDebugLLog(@"GSArt -shfill", @"Memory allocation failed.");
    return NO;
  }


  j = 1;
  for (i = 0; i<f->num_in; i ++)
  {
    f->size[i] =[[a objectAtIndex: i] intValue];
    j *= f->size[i];

    f->encode[i * 2 +0] = 0;
    f->encode[i * 2 + 1] = f->size[i]-1;
  }
  j *= f->bits_per_sample * f->num_out;
  j =(j +7)/8;
  if ([data length] <j)
  {
    free(f->size);
    f->size = NULL;
    free(f->domain);
    f->domain = NULL;
    free(f->range);
    f->range = NULL;
    free(f->encode);
    f->encode = NULL;
    free(f->decode);
    f->decode = NULL;
    NSDebugLLog(@"GSArt -shfill", @"Need %i bytes of data, DataSource only has %lu bytes.", 
      j, [data length]);
    return NO;
  }

  a =[d objectForKey: @"Domain"];
  for (i = 0; i<f->num_in * 2; i ++)
    f->domain[i] =[[a objectAtIndex: i] doubleValue];

  a =[d objectForKey: @"Range"];
  for (i = 0; i<f->num_out * 2; i ++)
    f->decode[i] = f->range[i] =[[a objectAtIndex: i] doubleValue];

  a =[d objectForKey: @"Decode"];
  if (a)
  {
    for (i = 0; i<f->num_out * 2; i ++)
      f->decode[i] =[[a objectAtIndex: i] doubleValue];
  }

  a =[d objectForKey: @"Encode"];
  if (a)
  {
    for (i = 0; i<f->num_in * 2; i ++)
      f->encode[i] =[[a objectAtIndex: i] doubleValue];
  }

  f->eval = function_eval;

  if (f->num_in == 2 && f->num_out == 3)
  {
    f->eval = function_eval_in2_out3;
    f->sample_index[0] = f->sample_index[1] =-1;
  }

  return YES;
}

static void function_free(function_t * f)
{
  free(f->size);
  f->size = NULL;
  free(f->domain);
  f->domain = NULL;
  free(f->range);
  f->range = NULL;
  free(f->encode);
  f->encode = NULL;
  free(f->decode);
  f->decode = NULL;
}


- (void) DPSshfill: (NSDictionary *)shader
{
  NSNumber * v;
  NSDictionary * function_dict;
  function_t function;
  NSAffineTransform * matrix, *inverse;

  if (!wi || !wi->data || all_clipped) return;

//  printf("DPSshfill: %@\n", shader);

  v = [shader objectForKey: @"ShadingType"];

  /* only type 1 shaders */
  if ([v intValue] != 1)
    {
      NSDebugLLog(@"GSArt -shfill", @"ShadingType!= 1 not supported.");
      return;
    }

  /* in device rgb space */
  if ([shader objectForKey: @"ColorSpace"])
    if (![[shader objectForKey: @"ColorSpace"] isEqual: NSDeviceRGBColorSpace])
      {
	NSDebugLLog(@"GSArt -shfill", @"Only device RGB ColorSpace supported.");
	return;
      }

  function_dict =[shader objectForKey: @"Function"];
  if (!function_dict)
    {
      NSDebugLLog(@"GSArt -shfill", @"Function not set.");
      return;
    }

  if (!function_setup(function_dict, &function))
    return;

  if (function.num_in!= 2 || function.num_out!= 3)
    {
      function_free(&function);
      NSDebugLLog(@"GSArt -shfill",
	@"Function doesn't have 2 inputs and 3 outputs.");
      return;
    }

  matrix =[ctm copy];
  if ([shader objectForKey: @"Matrix"])
    {
      [matrix prependTransform: [shader objectForKey: @"Matrix"]];
    }

  inverse = [matrix copy];
  [inverse invert];

  {
    rect_trace_t rt;
    NSRect rect;
    int y, x0, x1, x;
    render_run_t r;
    unsigned char * dst, *dsta;
    NSAffineTransformStruct	ts;
    double in[2], out[3];
    NSPoint p;

    ts = [inverse transformStruct];

    rect.origin.x = function.domain[0];
    rect.size.width = function.domain[1] -function.domain[0];
    rect.origin.y = function.domain[2];
    rect.size.height = function.domain[3] -function.domain[2];

/*    printf("rect =(%g %g)+(%g %g)\n", 
      rect.origin.x, rect.origin.y, 
      rect.size.width, rect.size.height);*/

    dst = wi->data +wi->bytes_per_line * clip_y0 +clip_x0 *DI.bytes_per_pixel;
    dsta = wi->alpha +wi->sx * clip_y0 +clip_x0;

    _rect_setup(&rt, rect, clip_x0, clip_x1, matrix, 0, &y, offset);

    while (y < clip_y0)
      {
  //      printf("skip initial clip y =%i, %i \n", y, clip_y0);
	if (!_rect_advance(&rt, &x0, &x1))
	  goto done;
  //      printf("   %i %i \n", x0, x1);
	y ++;
      }

    if (y > clip_y0)
      {
	dst += wi->bytes_per_line * (y -clip_y0);
	dsta += wi->sx * (y -clip_y0);
      }

    while (y < clip_y1 && _rect_advance(&rt, &x0, &x1))
      {
	if (!clip_span)
	  {
	    r.dst = dst + x0 * DI.bytes_per_pixel;
	    r.dsta = dsta + x0;

	    p = [inverse transformPoint:
	      NSMakePoint(clip_x0 + x0 - offset.x, offset.y - y)];
	    in[0] = p.x;
	    in[1] = p.y;

	    out[0] = out[1] = out[2] = 0.0;
	    for (x = x0; x < x1; x ++)
	      {
		function.eval(&function, in, out);
		r.r = out[0] * 255;
		r.g = out[1] * 255;
		r.b = out[2] * 255;
		if (wi->has_alpha)
		  DI.render_run_opaque_a(&r, 1);
		else
		  DI.render_run_opaque(&r, 1);
		r.dsta ++;
		r.dst += DI.bytes_per_pixel;

		in[0] += ts.m11;
		in[1] += ts.m12;
	      }
	  }
	else
	  {
	    unsigned int * span, *end;
	    BOOL state = NO;

	    span = &clip_span[clip_index[y - clip_y0]];
	    end = &clip_span[clip_index[y - clip_y0 + 1]];

	    while (span != end && *span < x0)
	      {
		state = !state;
		span ++;
	      }
	    if (span != end)
	      {
		while (span != end && *span < x1)
		  {
		    if (state)
		      {
			p = [inverse transformPoint:
			  NSMakePoint(clip_x0 + x0 - offset.x, offset.y - y)];

			in[0] = p.x;
			in[1] = p.y;

			out[0] = out[1] = out[2] = 0.0;
			r.dst = dst + x0 * DI.bytes_per_pixel;
			r.dsta = dsta + x0;
			for (x = x0; x < *span; x ++)
			  {
			    function.eval(&function, in, out);
			    r.r = out[0] * 255;
			    r.g = out[1] * 255;
			    r.b = out[2] * 255;
			    if (wi->has_alpha)
			      DI.render_run_opaque_a(&r, 1);
			    else
			      DI.render_run_opaque(&r, 1);
			    r.dsta ++;
			    r.dst += DI.bytes_per_pixel;

			    in[0] += ts.m11;
			    in[1] += ts.m12;
			  }
		      }
		    x0 = *span;

		    state = !state;
		    span ++;
		    if (span == end)
		      break;
		  }
		if (state)
		  {
		    p = [inverse transformPoint:
		      NSMakePoint(clip_x0 + x0 - offset.x, offset.y - y)];
		      
		    in[0] = p.x;
		    in[1] = p.y;

		    out[0] = out[1] = out[2] = 0.0;
		    r.dst = dst + x0 * DI.bytes_per_pixel;
		    r.dsta = dsta + x0;
		    for (x = x0; x < x1; x ++)
		      {
			function.eval(&function, in, out);
			r.r = out[0] * 255;
			r.g = out[1] * 255;
			r.b = out[2] * 255;
			if (wi->has_alpha)
			  DI.render_run_opaque_a(&r, 1);
			else
			  DI.render_run_opaque(&r, 1);
			r.dsta ++;
			r.dst += DI.bytes_per_pixel;

			in[0] += ts.m11;
			in[1] += ts.m12;
		      }
		  }
	      }
	  }

	y ++;
	dst += wi->bytes_per_line;
	dsta += wi->sx;
      }
  }

  UPDATE_UNBUFFERED

done:
  DESTROY(matrix);
  DESTROY(inverse);
  function_free(&function);
}

@end

