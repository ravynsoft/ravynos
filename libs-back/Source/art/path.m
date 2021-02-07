/*
   Copyright (C) 2002, 2003, 2005 Free Software Foundation, Inc.

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
Path handling.
*/

#include <math.h>

#include <AppKit/NSAffineTransform.h>
#include <AppKit/NSBezierPath.h>

#include "ARTGState.h"

#ifndef RDS
#include "x11/XWindowBuffer.h"
#endif
#include "blit.h"


#include <libart_lgpl/libart.h>
#include <libart_lgpl/art_svp_intersect.h>


#if 0
/* useful when debugging */
static void dump_vpath(ArtVpath *vp)
{
  int i;
  printf("** dumping %p **\n", vp);
  for (i = 0; ; i++)
    {
      if (vp[i].code == ART_MOVETO_OPEN)
	printf("  moveto_open");
      else if (vp[i].code == ART_MOVETO)
	printf("  moveto");
      else if (vp[i].code == ART_LINETO)
	printf("  lineto");
      else if (vp[i].code == ART_END)
	printf("  end");
      else
	printf("  unknown %i", vp[i].code);

      printf(" (%g %g)\n",
	     vp[i].x, vp[i].y);
      if (vp[i].code == ART_END)
	break;
    }
}
static void dump_bpath(ArtBpath *vp)
{
  int i;
  printf("** dumping %p **\n", vp);
  for (i = 0; ; i++)
    {
      if (vp[i].code == ART_MOVETO_OPEN)
	printf("  moveto_open");
      else if (vp[i].code == ART_MOVETO)
	printf("  moveto");
      else if (vp[i].code == ART_LINETO)
	printf("  lineto");
      else if (vp[i].code == ART_CURVETO)
	printf("  curveto");
      else
	printf("  unknown %i", vp[i].code);

      printf(" (%g %g) (%g %g) (%g %g)\n",
	     vp[i].x1, vp[i].y1,
	     vp[i].x2, vp[i].y2,
	     vp[i].x3, vp[i].y3);
      if (vp[i].code == ART_END)
	break;
    }
}

{
  int i;
  NSBezierPathElement type;
  NSPoint pts[3];
  for (i = 0; i < [newPath elementCount]; i++)
    {
      type = [newPath elementAtIndex: i associatedPoints: pts];
      switch (type)
	{
	case NSMoveToBezierPathElement:
	  printf("moveto (%g %g)\n", pts[0].x, pts[0].y);
	  break;
	case NSLineToBezierPathElement:
	  printf("lineto (%g %g)\n", pts[0].x, pts[0].y);
	  break;
	case NSCurveToBezierPathElement:
	  printf("curveto (%g %g) (%g %g) (%g %g)\n",
		 pts[0].x, pts[0].y,
		 pts[1].x, pts[1].y,
		 pts[2].x, pts[2].y);
	  break;
	}
    }
}

{
        int i,j;
        printf("size=%i num=%i\n",ci.span_size,ci.num_span);
        for (i=0;i<clip_sy;i++)
        {
                printf("y=%3i:",i);
                for (j=clip_index[i];j<clip_index[i+1];j++)
                {
                        printf(" %i",clip_span[j]);
                }
                printf("\n");
        }
}

#endif


/* rendering helpers */

typedef struct
{
  render_run_t ri;

  unsigned char real_a;
  int x0, x1, y0;
  int rowstride, arowstride, bpp;
  void (*run_alpha)(struct render_run_s *ri, int num);
  void (*run_opaque)(struct render_run_s *ri, int num);

  unsigned int *clip_span, *clip_index;
} svp_render_info_t;

static void render_svp_callback(void *data, int y, int start,
	ArtSVPRenderAAStep *steps, int n_steps)
{
  svp_render_info_t *ri = data;
  int x0 = ri->x0, x1;
  int num;
  int alpha;
  unsigned char *dst, *dsta;

  alpha = start;

  /* empty line; very common case */
  if (alpha < 0x10000 && !n_steps)
    {
      ri->ri.dst += ri->rowstride;
      ri->ri.dsta += ri->arowstride;
      return;
    }

  dst = ri->ri.dst + ri->rowstride;
  dsta = ri->ri.dsta + ri->arowstride;

  for (; n_steps; n_steps--, steps++)
    {
      x1 = steps->x;
      num = x1 - x0;

      ri->ri.a = (alpha * ri->real_a + 0x800000) >> 24;
      if (ri->ri.a && num)
	{
	  if (ri->ri.a == 255)
	    ri->run_opaque(&ri->ri, num);
	  else
	    ri->run_alpha(&ri->ri, num);
	}
      ri->ri.dst += ri->bpp * num;
      ri->ri.dsta += num;

      alpha += steps->delta;
      x0 = x1;
    }

  x1 = ri->x1;
  num = x1 - x0;
  ri->ri.a = (alpha * ri->real_a + 0x800000) >> 24;
  if (ri->ri.a && num)
    {
      if (ri->ri.a == 255)
	ri->run_opaque(&ri->ri, num);
      else
	ri->run_alpha(&ri->ri, num);
    }

  ri->ri.dst = dst;
  ri->ri.dsta = dsta;
}

static void render_svp_clipped_callback(void *data, int y, int start,
	ArtSVPRenderAAStep *steps, int n_steps)
{
  svp_render_info_t *ri = data;
  int x0, x1;
  int num;
  int alpha;
  unsigned char *dst, *dsta;

  unsigned int *span, *end;
  BOOL state;

  alpha = start;

  /* empty line; very common case */
  if (alpha < 0x10000 && !n_steps)
    {
      ri->ri.dst += ri->rowstride;
      ri->ri.dsta += ri->arowstride;
      return;
    }

  /* completely clipped line? */
  if (ri->clip_index[y - ri->y0] == ri->clip_index[y - ri->y0 + 1])
    {
      ri->ri.dst += ri->rowstride;
      ri->ri.dsta += ri->arowstride;
      return;
    }

  span = &ri->clip_span[ri->clip_index[y - ri->y0]];
  end = &ri->clip_span[ri->clip_index[y - ri->y0 + 1]];
  state = NO;

  dst = ri->ri.dst + ri->rowstride;
  dsta = ri->ri.dsta + ri->arowstride;

  x0 = 0;
  for (; n_steps; n_steps--, steps++)
    {
      x1 = steps->x - ri->x0;

      ri->ri.a = (alpha * ri->real_a + 0x800000) >> 24;
      if (ri->ri.a)
	{
	  while (*span < x1)
	    {
	      num = *span - x0;
	      if (state)
		{
		  if (ri->ri.a == 255)
		    ri->run_opaque(&ri->ri, num);
		  else
		    ri->run_alpha(&ri->ri, num);
		}
	      x0 = *span;
	      ri->ri.dst += ri->bpp * num;
	      ri->ri.dsta += num;
	      state = !state;
	      span++;
	      if (span == end)
		{
		  ri->ri.dst = dst;
		  ri->ri.dsta = dsta;
		  return;
		}
	    }
	  num = x1 - x0;
	  if (num && state)
	    {
	      if (ri->ri.a == 255)
		ri->run_opaque(&ri->ri, num);
	      else
		ri->run_alpha(&ri->ri, num);
	    }
	  ri->ri.dst += ri->bpp * num;
	  ri->ri.dsta += num;
	}
      else
	{
	  num = x1 - x0;
	  while (*span <= x1)
	    {
	      state = !state;
	      span++;
	      if (span == end)
		{
		  ri->ri.dst = dst;
		  ri->ri.dsta = dsta;
		  return;
		}
	    }
	  ri->ri.dst += ri->bpp * num;
	  ri->ri.dsta += num;
	}

      alpha += steps->delta;
      x0 = x1;
    }
  x1 = ri->x1 - ri->x0;
  num = x1 - x0;
  ri->ri.a = (alpha * ri->real_a + 0x800000) >> 24;
  if (ri->ri.a && num)
    {
      while (*span < x1)
	{
	  num = *span - x0;
	  if (state)
	    {
	      if (ri->ri.a == 255)
		ri->run_opaque(&ri->ri, num);
	      else
		ri->run_alpha(&ri->ri, num);
	    }
	  x0 = *span;
	  ri->ri.dst += ri->bpp * num;
	  ri->ri.dsta += num;
	  state = !state;
	  span++;
	  if (span == end)
	    {
	      ri->ri.dst = dst;
	      ri->ri.dsta = dsta;
	      return;
	    }
	}
      num = x1 - x0;
      if (num && state)
	{
	  if (ri->ri.a == 255)
	    ri->run_opaque(&ri->ri, num);
	  else
	    ri->run_alpha(&ri->ri, num);
	}
    }

  ri->ri.dst = dst;
  ri->ri.dsta = dsta;
}

static void artcontext_render_svp(const ArtSVP *svp, int x0, int y0, int x1, int y1,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a,
	unsigned char *dst, int rowstride,
	unsigned char *dsta, int arowstride, int has_alpha,
	draw_info_t *di,
	unsigned int *clip_span, unsigned int *clip_index)
{
  svp_render_info_t ri;

  ri.x0 = x0;
  ri.x1 = x1;
  ri.y0 = y0;

  ri.ri.r = r;
  ri.ri.g = g;
  ri.ri.b = b;
  ri.real_a = ri.ri.a = a;

  ri.bpp = di->bytes_per_pixel;

  ri.ri.dst = dst;
  ri.rowstride = rowstride;

  ri.clip_span = clip_span;
  ri.clip_index = clip_index;

  if (has_alpha)
    {
      ri.ri.dsta = dsta;
      ri.arowstride = arowstride;
      ri.run_alpha = di->render_run_alpha_a;
      ri.run_opaque = di->render_run_opaque_a;
    }
  else
    {
      ri.run_alpha = di->render_run_alpha;
      ri.run_opaque = di->render_run_opaque;
    }

  art_svp_render_aa(svp, x0, y0, x1, y1,
    clip_span? render_svp_clipped_callback : render_svp_callback, &ri);
}


@implementation ARTGState (path)

/* Fills in vp. If the rectangle is axis- (and optionally pixel)-aligned,
also fills in the axis coordinates (x0/y0 is min) and returns 1. Otherwise
returns 0. (Actually, if pixel is NO, it's enough that the edges remain
within one pixel.) */
- (int) _axis_rectangle: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
		 vpath: (ArtVpath *)vp
		  axis: (int *)px0 : (int *)py0 : (int *)px1 : (int *)py1
		 pixel: (BOOL)pixel
{
  float matrix[6];
  float det;
  int i;
  int x0, y0, x1, y1;
  NSAffineTransformStruct	ts = [ctm transformStruct];

  if (w < 0) x += w, w = -w;
  if (h < 0) y += h, h = -h;

  matrix[0] = ts.m11;
  matrix[1] = -ts.m12;
  matrix[2] = ts.m21;
  matrix[3] = -ts.m22;
  matrix[4] = ts.tX - offset.x;
  matrix[5] = -ts.tY + offset.y;

  /* If the matrix is 'inverted', ie. if the determinant is negative,
     we need to flip the order of the vertices. Since it's a rectangle
     we can just swap vertex 1 and 3. */
  det = matrix[0] * matrix[3] - matrix[1] * matrix[2];

  vp[0].code = ART_MOVETO;
  vp[0].x = x * matrix[0] + y * matrix[2] + matrix[4];
  vp[0].y = x * matrix[1] + y * matrix[3] + matrix[5];

  i = det > 0? 3 : 1;
  vp[i].code = ART_LINETO;
  vp[i].x = vp[0].x + w * matrix[0];
  vp[i].y = vp[0].y + w * matrix[1];

  vp[2].code = ART_LINETO;
  vp[2].x = vp[0].x + w * matrix[0] + h * matrix[2];
  vp[2].y = vp[0].y + w * matrix[1] + h * matrix[3];

  i ^= 2;
  vp[i].code = ART_LINETO;
  vp[i].x = vp[0].x + h * matrix[2];
  vp[i].y = vp[0].y + h * matrix[3];

  vp[4].code = ART_LINETO;
  vp[4].x = vp[0].x;
  vp[4].y = vp[0].y;

  vp[5].code = ART_END;
  vp[5].x = vp[5].y = 0;

  /* Check if this rectangle is axis-aligned and on whole pixel
     boundaries. */
  x0 = vp[0].x + 0.5;
  x1 = vp[2].x + 0.5;
  y0 = vp[0].y + 0.5;
  y1 = vp[2].y + 0.5;

  if (pixel)
    {
      if (x0 > x1)
	*px0 = x1, *px1 = x0;
      else
	*px0 = x0, *px1 = x1;
      if (y0 > y1)
	*py0 = y1, *py1 = y0;
      else
	*py0 = y0, *py1 = y1;

      if (fabs(vp[0].x - vp[1].x) < 0.01 && fabs(vp[1].y - vp[2].y) < 0.01
	&& fabs(vp[0].x - x0) < 0.01 && fabs(vp[0].y - y0) < 0.01
	&& fabs(vp[2].x - x1) < 0.01 && fabs(vp[2].y - y1) < 0.01)
	{
	  return 1;
	}

      if (fabs(vp[0].y - vp[1].y) < 0.01 && fabs(vp[1].x - vp[2].x) < 0.01
	&& fabs(vp[0].x - x0) < 0.01 && fabs(vp[0].y - y0) < 0.01
	&& fabs(vp[2].x - x1) < 0.01 && fabs(vp[2].y - y1) < 0.01)
	{
	  return 1;
	}
    }
  else
    {
      /* This is used when clipping, so we need to make sure we
         contain all pixels. */
      if (vp[0].x < vp[2].x)
	*px0 = floor(vp[0].x), *px1 = ceil(vp[2].x);
      else
	*px0 = floor(vp[2].x), *px1 = ceil(vp[0].x);
      if (vp[0].y < vp[2].y)
	*py0 = floor(vp[0].y), *py1 = ceil(vp[2].y);
      else
	*py0 = floor(vp[2].y), *py1 = ceil(vp[0].y);

      if (floor(vp[0].x) == floor(vp[1].x) && floor(vp[0].y) == floor(vp[3].y)
	&& floor(vp[1].y) == floor(vp[2].y) && floor(vp[2].x) == floor(vp[3].x))
	{
	  return 1;
	}

      if (floor(vp[0].y) == floor(vp[1].y) && floor(vp[0].x) == floor(vp[3].x)
	&& floor(vp[1].x) == floor(vp[2].x) && floor(vp[2].y) == floor(vp[3].y))
	{
	  return 1;
	}
    }

  return 0;
}


- (ArtVpath *) _vpath_from_current_path: (BOOL)fill
{
  ArtBpath *bpath, *bp2;
  ArtVpath *vp;
  int i, j, c, cur_start, cur_line;
  NSPoint points[3];
  NSBezierPathElement t;
  double matrix[6];


  c = [path elementCount];
  if (!c)
    return NULL;

  if (fill)
    bpath = art_new(ArtBpath, 2 * c + 1);
  else
    bpath = art_new(ArtBpath, c + 1);

  cur_start = -1;
  cur_line = 0;
  for (i = j = 0; i < c; i++)
    {
      t = [path elementAtIndex: i associatedPoints: points];
      switch (t)
	{
	case NSMoveToBezierPathElement:
	  /* When filling, the path must be closed, so if
             it isn't already closed, we fix that here. */
	  if (fill)
	    {
	      if (cur_start != -1 && cur_line)
		{
		  if (bpath[j - 1].x3 != bpath[cur_start].x3 ||
		      bpath[j - 1].y3 != bpath[cur_start].y3)
		    {
		      bpath[j].x3 = bpath[cur_start].x3;
		      bpath[j].y3 = bpath[cur_start].y3;
		      bpath[j].code = ART_LINETO;
		      j++;
		    }
		}
	      bpath[j].code = ART_MOVETO;
	    }
	  else
	    {
	      bpath[j].code = ART_MOVETO_OPEN;
	    }
	  bpath[j].x3 = points[0].x;
	  bpath[j].y3 = points[0].y;
	  cur_start = j;
	  j++;
	  cur_line = 0;
	  break;

	case NSLineToBezierPathElement:
	  cur_line++;
	  bpath[j].code = ART_LINETO;
	  bpath[j].x3 = points[0].x;
	  bpath[j].y3 = points[0].y;
	  j++;
	  break;

	case NSCurveToBezierPathElement:
	  cur_line++;
	  bpath[j].code = ART_CURVETO;
	  bpath[j].x1 = points[0].x;
	  bpath[j].y1 = points[0].y;
	  bpath[j].x2 = points[1].x;
	  bpath[j].y2 = points[1].y;
	  bpath[j].x3 = points[2].x;
	  bpath[j].y3 = points[2].y;
	  j++;
	  break;

	case NSClosePathBezierPathElement:
	  if (cur_start != -1 && cur_line)
	    {
	      bpath[cur_start].code = ART_MOVETO;
	      bpath[j].code = ART_LINETO;
	      bpath[j].x3 = bpath[cur_start].x3;
	      bpath[j].y3 = bpath[cur_start].y3;
	      j++;
	    }
	  break;

	default:
	  NSLog(@"invalid type %i\n", t);
	  art_free(bpath);
	  return NULL;
	}
    }

  if (fill && cur_start != -1 && cur_line)
    {
      if (bpath[j - 1].x3 != bpath[cur_start].x3 ||
	  bpath[j - 1].y3 != bpath[cur_start].y3)
	{
	  bpath[j].x3 = bpath[cur_start].x3;
	  bpath[j].y3 = bpath[cur_start].y3;
	  bpath[j].code = ART_LINETO;
	  j++;
	}
    }
  bpath[j].code = ART_END;

  matrix[0]= 1;
  matrix[1]= 0;
  matrix[2]= 0;
  matrix[3]=-1;
  matrix[4]= 0 - offset.x;
  matrix[5]= offset.y;

  bp2 = art_bpath_affine_transform(bpath, matrix);
  art_free(bpath);

  vp = art_bez_path_to_vec(bp2, 0.5);
  art_free(bp2);

  return vp;
}


/** Clipping **/

typedef struct
{
  int x0, x1, y0, sy;

  int minx,maxx;
  int first_y,last_y;

  unsigned int *span;
  unsigned int *index;
  int span_size, num_span;

  unsigned int *cur_span;
  unsigned int *cur_index;
} clip_info_t;


static void clip_svp_callback(void *data, int y, int start,
	ArtSVPRenderAAStep *steps, int n_steps)
{
  clip_info_t *ci = data;
  int x;
  int alpha;
  BOOL state, nstate;

  alpha = start;

  ci->index[y - ci->y0 - ci->first_y] = ci->num_span;
  if (y-ci->y0<0 || y-ci->y0>=ci->sy)
  {
	printf("weird y=%i (%i)\n",y,y-ci->y0);
  }

  /* empty line; very common case */
  if (alpha < 0x10000 && !n_steps)
    {
      if (ci->first_y == y - ci->y0)
        ci->first_y++;
      return;
    }

  ci->last_y = y - ci->y0 + 1;

  x = 0;
  state = alpha >= 0x10000;
  if (state)
    {
      if (ci->num_span == ci->span_size)
	{
	  ci->span_size += 16;
	  ci->span = realloc(ci->span, sizeof(unsigned int) * ci->span_size);
	}
      ci->span[ci->num_span++] = x;
      if (x < ci->minx) ci->minx = x;
    }


  for (; n_steps; n_steps--, steps++)
    {
      alpha += steps->delta;
      x = steps->x - ci->x0;
      nstate = alpha >= 0x10000;
      if (state != nstate)
	{
	  if (ci->num_span == ci->span_size)
	    {
	      ci->span_size += 16;
	      ci->span = realloc(ci->span, sizeof(unsigned int) * ci->span_size);
	    }
	  ci->span[ci->num_span++] = x;
	  if (x < ci->minx) ci->minx = x;
	  if (x > ci->maxx) ci->maxx = x;
	  state = nstate;
	}
    }
  if (state)
    {
      if (ci->num_span == ci->span_size)
	{
	  ci->span_size += 16;
	  ci->span = realloc(ci->span, sizeof(unsigned int) * ci->span_size);
	}
      x = ci->x1 - ci->x0;
      ci->span[ci->num_span++] = x;
      if (x > ci->maxx) ci->maxx = x;
    }
}

/* will free the passed in svp */
- (void) _clip_add_svp: (ArtSVP *)svp
{
  clip_info_t ci;
  ci.span = NULL;
  ci.index = malloc(sizeof(unsigned int) * (clip_sy + 1));
  if (!ci.index)
    {
      NSLog(@"Warning: out of memory calculating clipping spans (%lu bytes)",
	    sizeof(unsigned int) * (clip_sy + 1));
      return;
    }
  ci.span_size = ci.num_span = 0;
  ci.x0 = clip_x0;
  ci.x1 = clip_x1;
  ci.y0 = clip_y0;
  ci.sy = clip_sy;

  ci.minx = ci.x1 - ci.x0;
  ci.maxx = 0;
  ci.first_y = 0;
  ci.last_y = -1;

  if (clip_span)
    {
      NSLog(@"TODO: _clip_add_svp: with existing clip_span not implemented");
      free(ci.index);
      return;
    }
  else
    {
      art_svp_render_aa(svp, clip_x0, clip_y0, clip_x1, clip_y1, clip_svp_callback, &ci);
      if (!ci.span)
	{
	  /* This can happen if the path is empty, or doesn't intersect the
	     current clipping path.  The result then is that everything
	     is clipped.  */
	  free(ci.index);
	  all_clipped = YES;
	  clip_x0 = clip_x1 = clip_sx = 0;
	  clip_y0 = clip_y1 = clip_sy = 0;
	  return;
	}
      clip_span = ci.span;
      clip_index = ci.index;
      clip_index[clip_sy - ci.first_y] = clip_num_span = ci.num_span;

      clip_y1 = clip_y0 + ci.last_y;
      clip_y0 += ci.first_y;
      clip_sy = clip_y1 - clip_y0;
      if (clip_y1 <= clip_y0)
	all_clipped = YES;

      if (ci.minx > 0)
	{
	  int i;
	  for (i = 0; i < clip_num_span; i++)
	    {
	      if (clip_span[i] < ci.minx)
	        NSLog(@"_clip_add_svp: clip_span[i]<0 when adjusting for minx");
	      clip_span[i] -= ci.minx;
	      if (clip_span[i] > ci.maxx - ci.minx)
	        NSLog(@"_clip_add_svp: clip_span[i] too large when adjusting for minx");
	    }
	}

      clip_x1 = clip_x0 + ci.maxx;
      clip_x0 += ci.minx;
      clip_sx = clip_x1 - clip_x0;
      if (clip_x1 <= clip_x0)
	all_clipped = YES;
    }
  art_svp_free(svp);
}

- (void) _clip: (int)rule
{
  ArtVpath *vp;
  ArtSVP *svp;

  vp = [self _vpath_from_current_path: NO];
  if (!vp)
    return;
  svp = art_svp_from_vpath(vp);
  art_free(vp);

  {
    ArtSVP *svp2;
    ArtSvpWriter *svpw;

    svpw = art_svp_writer_rewind_new(rule);
    art_svp_intersector(svp, svpw);
    svp2 = art_svp_writer_rewind_reap(svpw);
    art_svp_free(svp);
    svp = svp2;
  }

  [self _clip_add_svp: svp];
}


- (void) DPSclip
{
  [self _clip: ART_WIND_RULE_NONZERO];
}

- (void) DPSeoclip
{
  [self _clip: ART_WIND_RULE_ODDEVEN];
}

- (void) DPSrectclip: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
{
  ArtVpath vp[6];
  ArtSVP *svp;
  int x0, y0, x1, y1;
  int axis_aligned;
  
  [self DPSnewpath];

  if (all_clipped)
    return;

  if (!wi)
    {
      all_clipped = YES;
      return;
    }

  axis_aligned = [self _axis_rectangle: x : y : w : h vpath: vp
		     axis: &x0 : &y0 : &x1 : &y1
		     pixel: NO];

  if (!axis_aligned || clip_span)
    {
      svp = art_svp_from_vpath(vp);
      [self _clip_add_svp: svp];
      return;
    }

  if (x0 > clip_x0)
    clip_x0 = x0;
  if (y0 > clip_y0)
    clip_y0 = y0;

  if (x1 < clip_x1)
    clip_x1 = x1;
  if (y1 < clip_y1)
    clip_y1 = y1;

  if (clip_x0 >= clip_x1 || clip_y0 >= clip_y1)
    {
      all_clipped = YES;
    }

  clip_sx = clip_x1 - clip_x0;
  clip_sy = clip_y1 - clip_y0;
}

- (void) DPSinitclip;
{
  if (!wi)
    {
      all_clipped = YES;
      return;
    }

  clip_x0 = clip_y0 = 0;
  clip_x1 = wi->sx;
  clip_y1 = wi->sy;
  all_clipped = NO;
  clip_sx = clip_x1 - clip_x0;
  clip_sy = clip_y1 - clip_y0;

  if (clip_span)
    {
      free(clip_span);
      free(clip_index);
      clip_span = clip_index = NULL;
      clip_num_span = 0;
    }
}


/** Filling **/

- (void) _fill: (int)rule
{
  ArtVpath *vp;
  ArtSVP *svp;

  if (!wi || !wi->data) return;
  if (all_clipped) return;
  if (!fill_color[3]) return;

  vp = [self _vpath_from_current_path: YES];
  if (!vp)
    return;
  svp = art_svp_from_vpath(vp);
  art_free(vp);

  {
    ArtSVP *svp2;
    ArtSvpWriter *svpw;

    svpw = art_svp_writer_rewind_new(rule);
    art_svp_intersector(svp, svpw);
    svp2 = art_svp_writer_rewind_reap(svpw);
    art_svp_free(svp);
    svp = svp2;
  }


  artcontext_render_svp(svp, clip_x0, clip_y0, clip_x1, clip_y1,
    fill_color[0], fill_color[1], fill_color[2], fill_color[3],
    CLIP_DATA, wi->bytes_per_line,
    wi->has_alpha? wi->alpha + clip_x0 + clip_y0 * wi->sx : NULL, wi->sx,
    wi->has_alpha,
    &DI, clip_span, clip_index);

  art_svp_free(svp);

  [path removeAllPoints];

  UPDATE_UNBUFFERED
}

- (void) DPSeofill
{
  if (pattern != nil)
    {
      [self eofillPath: path withPattern: pattern];
      return;
    }

  [self _fill: ART_WIND_RULE_ODDEVEN];
}

- (void) DPSfill
{
  if (pattern != nil)
    {
      [self fillPath: path withPattern: pattern];
      return;
    }

  [self _fill: ART_WIND_RULE_NONZERO];
}

- (void) DPSrectfill: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
{
  ArtVpath vp[6];
  ArtSVP *svp;
  int x0, y0, x1, y1;
  int axis_aligned;

  if (!wi || !wi->data) return;
  if (all_clipped) return;

  if (pattern != nil)
    {
      NSBezierPath *rpath;

      rpath = [[NSBezierPath alloc] init];
      [rpath appendBezierPathWithRect: NSMakeRect(x, y, w, h)];
      [rpath transformUsingAffineTransform: ctm];
      [self fillPath: rpath withPattern: pattern];
      RELEASE(rpath);
      return;
    }

  if (!fill_color[3]) return;

  axis_aligned = [self _axis_rectangle: x : y : w : h vpath: vp
		     axis: &x0 : &y0 : &x1 : &y1
		     pixel: YES];

  if (!axis_aligned || clip_span)
    {
      /* Not properly aligned. Handle the general case. */
      svp = art_svp_from_vpath(vp);

      artcontext_render_svp(svp, clip_x0, clip_y0, clip_x1, clip_y1,
	fill_color[0], fill_color[1], fill_color[2], fill_color[3],
	CLIP_DATA, wi->bytes_per_line,
	wi->has_alpha? wi->alpha + clip_x0 + clip_y0 * wi->sx : NULL, wi->sx,
	wi->has_alpha,
	&DI, clip_span, clip_index);

      art_svp_free(svp);
      UPDATE_UNBUFFERED
      return;
    }

  /* optimize axis- and pixel-aligned rectangles */
  {
    unsigned char *dst = CLIP_DATA;
    unsigned char *dsta = wi->alpha + clip_x0 + clip_y0 * wi->sx;
    render_run_t ri;

    x0 -= clip_x0;
    x1 -= clip_x0;
    if (x0 <= 0)
      x0 = 0;
    else
      {
	dst += x0 * DI.bytes_per_pixel;
	dsta += x0;
      }
    if (x1 > clip_sx) x1 = clip_sx;

    x1 -= x0;
    if (x1 <= 0)
      return;

    y0 -= clip_y0;
    y1 -= clip_y0;
    if (y0 <= 0)
      y0 = 0;
    else
      {
	dst += y0 * wi->bytes_per_line;
	dsta += y0 * wi->sx;
      }
    if (y1 > clip_sy) y1 = clip_sy;

    if (y1 <= y0)
      return;

    ri.dst = dst;
    ri.r = fill_color[0];
    ri.g = fill_color[1];
    ri.b = fill_color[2];
    ri.a = fill_color[3];
    if (wi->has_alpha)
      {
	ri.dsta = dsta;

	if (fill_color[3] == 255)
	  {
	    while (y0 < y1)
	      {
		RENDER_RUN_OPAQUE_A(&ri, x1);
		y0++;
		ri.dst += wi->bytes_per_line;
		ri.dsta += wi->sx;
	      }
	  }
	else
	  {
	    while (y0 < y1)
	      {
		RENDER_RUN_ALPHA_A(&ri, x1);
		y0++;
		ri.dst += wi->bytes_per_line;
		ri.dsta += wi->sx; 
	      }
	  }
      }
    else
      {
	if (fill_color[3] == 255)
	  {
	    while (y0 < y1)
	      {
		RENDER_RUN_OPAQUE(&ri, x1);
		y0++;
		ri.dst += wi->bytes_per_line;
	      }
	  }
	else
	  {
	    while (y0 < y1)
	      {
		RENDER_RUN_ALPHA(&ri, x1);
		y0++;
		ri.dst += wi->bytes_per_line;
	      }
	  }
      }
    UPDATE_UNBUFFERED
  }
}


/** Stroking **/

/* will free the passed in vpath */
- (void) _stroke: (ArtVpath *)vp
{
  double temp_scale;
  ArtSVP *svp;
  NSAffineTransformStruct	ts = [ctm transformStruct];
  float dash_adjust;


  /* TODO: this is a hack, but it's better than nothing */
  /* since we flip vertically, the signs here should really be
     inverted, but the fabs() means that it doesn't matter */
  temp_scale = sqrt(fabs(ts.m11 * ts.m22 - ts.m12 * ts.m21));
  if (temp_scale <= 0) temp_scale = 1;


  /*
  If stroke-adjusting (or something equivalent) is active, we want to adjust
  the path so it will turn out nice and sharp.

  To do this, we round the line width to the closest integer width. To get
  sharp lines, we then want each pixel to be at an integer coordinate, or
  an integer plus 0.5, depending on the width and a bunch of other things.
  */
  if (strokeadjust

      /* No point trying to adjust width 0 lines.  (if they were implemented
         properly)  */
      && line_width > 0

      /* Nor if the path is empty.  */
      && vp[0].code != ART_END)
    {
      int i;

      int effective_width = rint(temp_scale * line_width);
      float ofs;

      int last_move;

      /* Paths with more elements than this won't be adjusted.  */
#define MAX_LEN 1024
      unsigned char flags[MAX_LEN];
      /*
	 1 start of vertical line
	 2 start of horizontal line
	 4 end-point on vertical line
	 8 end-point on horizontal line
      */

      temp_scale = effective_width / line_width;

      if (effective_width & 1)
	ofs = 0.5;
      else
	ofs = 0.0;

      last_move = 0;
      /* TODO: use epsilons instead of exact comparisons?  makes rounding
	 a huge mess.  */
      flags[0] = 0;
      for (i = 1; i < MAX_LEN; i++)
	{
	  flags[i] = 0;
	  /* If this is a closed sub-path, consider the line from the last
	     element to the moveto.  */
	  if (vp[i].code != ART_LINETO)
	    {
	      if (vp[last_move].code == ART_MOVETO)
		{
		  if (vp[i - 1].x == vp[last_move].x)
		    {
		      flags[i - 1] |= 1;
		      flags[last_move] |= 1;
		    }
		  if (vp[i - 1].y == vp[last_move].y)
		    {
		      flags[i - 1] |= 2;
		      flags[last_move] |= 2;
		    }
		}
	      else
		{
		  if (flags[last_move] & 1)
		    flags[last_move] |= 4;
		  if (flags[last_move] & 2)
		    flags[last_move] |= 8;

		  if (flags[i - 1] & 1)
		    flags[i - 1] |= 4;
		  if (flags[i - 1] & 2)
		    flags[i - 1] |= 8;
		}
	    }

	  if (vp[i].code == ART_END)
	    break;

	  if (vp[i].code == ART_MOVETO
	      || vp[i].code == ART_MOVETO_OPEN)
	    {
	      last_move = i;
	    }
	  else
	    {
	      if (vp[i - 1].x == vp[i].x)
		{
		  flags[i - 1] |= 1;
		  flags[i] |= 1;
		}
	      if (vp[i - 1].y == vp[i].y)
		{
		  flags[i - 1] |= 2;
		  flags[i] |= 2;
		}
	    }
	}

      if (i < MAX_LEN)
	{
	  for (i = 0; ; i++)
	    {
	      if (vp[i].code == ART_END)
		break;
	      if (flags[i] & 1)
		vp[i].x = floor(vp[i].x) + ofs;
	      else if (flags[i] & 8)
		vp[i].x = floor(vp[i].x + 0.5);
	      if (flags[i] & 2)
		vp[i].y = floor(vp[i].y) + ofs;
	      else if (flags[i] & 4)
		vp[i].y = floor(vp[i].y + 0.5);
	    }
	}

      /* Try to line an integer dash offset up on a pixel boundary near
	 the first point.  (Safe because we know the path isn't empty
	 at this point.)  */
      dash_adjust = 0.0;
      if (vp[1].code == ART_LINETO)
	{
	  if (fabs(vp[0].x - vp[1].x) < 0.1)
	    dash_adjust = rint(vp[0].y) - vp[0].y;
	  else if (fabs(vp[0].y - vp[1].y) < 0.1)
	    dash_adjust = rint(vp[0].x) - vp[0].x;
	}
#undef MAX_LEN
    }
  else
    {
      dash_adjust = 0.0;
    }


  if (do_dash)
    {
      /* try to adjust the offset so dashes appear on pixel boundaries
         (otherwise it turns into an antialiased blur) */
      int i;
      float old_offset = dash.offset;
      ArtVpath *vp2;

      if (!dash.offset)
	dash.offset += dash_adjust;

      for (i = 0; i < dash.n_dash; i++)
	dash.dash[i] *= temp_scale;
      dash.offset *= temp_scale;
      vp2 = art_vpath_dash(vp, &dash);
      dash.offset /= temp_scale;
      for (i = 0; i < dash.n_dash; i++)
	dash.dash[i] /= temp_scale;
      art_free(vp);
      vp = vp2;

      dash.offset = old_offset;
    }

  svp = art_svp_vpath_stroke(vp, linejoinstyle, linecapstyle,
			     temp_scale * line_width, miter_limit, 0.5);
  art_free(vp);

  artcontext_render_svp(svp, clip_x0, clip_y0, clip_x1, clip_y1,
    stroke_color[0], stroke_color[1], stroke_color[2], stroke_color[3],
    CLIP_DATA, wi->bytes_per_line,
    wi->has_alpha? wi->alpha + clip_x0 + clip_y0 * wi->sx : NULL, wi->sx,
    wi->has_alpha,
    &DI, clip_span, clip_index);

  art_svp_free(svp);
  UPDATE_UNBUFFERED
}

- (void) DPSrectstroke: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h
{
  ArtVpath *vp, *vp2;
  double matrix[6];
  NSAffineTransformStruct	ts;

  if (!wi || !wi->data) return;
  if (all_clipped) return;
  if (!stroke_color[3]) return;

  vp = art_new(ArtVpath, 6);

  vp[0].code = ART_MOVETO;
  vp[0].x = x; vp[0].y = y;

  vp[1].code = ART_LINETO;
  vp[1].x = x + w; vp[1].y = y;

  vp[2].code = ART_LINETO;
  vp[2].x = x + w; vp[2].y = y + h;

  vp[3].code = ART_LINETO;
  vp[3].x = x; vp[3].y = y + h;

  vp[4].code = ART_LINETO;
  vp[4].x = x; vp[4].y = y;

  vp[5].code = ART_END;
  vp[5].x = vp[5].y = 0;

  ts = [ctm transformStruct];
  matrix[0] = ts.m11;
  matrix[1] = -ts.m12;
  matrix[2] = ts.m21;
  matrix[3] = -ts.m22;
  matrix[4] = ts.tX - offset.x;
  matrix[5] = -ts.tY + offset.y;

  vp2 = art_vpath_affine_transform(vp, matrix);
  art_free(vp);
  vp = vp2;

  [self _stroke: vp];
}

- (void) DPSstroke
{
/* TODO: Resolve line-width and dash scaling issues. The way this is
currently done is the most obvious libart approach:

1. convert the NSBezierPath to an ArtBpath
2. transform the Bpath
3. convert the Bpath to a Vpath, approximating the curves with lines
  (1-3 are done in -_vpath_from_current_path:)

4. apply dashing to the Vpath
  (art_vpath_dash, called below)
5. stroke and convert the Vpath to an svp
  (art_svp_vpath_stroke, called below)

To do this correctly, we need to do dashing and stroking (4 and part of 5)
in user space. It is possible to do the transform _after_ step 5 (although
it's less efficient), but we want to do any curve approximation (3, and 5 if
there are round line ends or joins) in device space.

The best way to solve this is probably to keep doing the transform first,
and to add transform-aware dashing and stroking functions to libart.

Currently, a single scale value is applied to dashing and stroking. This
will give correct results as long as both axises are scaled the same.

*/
  ArtVpath *vp;

  if (!wi || !wi->data) return;
  if (all_clipped) return;
  if (!stroke_color[3]) return;

  /* TODO: this is wrong. we should transform _after_ we dash and
     stroke */
  vp = [self _vpath_from_current_path: NO];
  if (!vp)
    return;

  [self _stroke: vp];

  [path removeAllPoints];
}

@end


@interface ARTGState (path_testing)
- (void) GScurrentpath: (NSBezierPath **)p;
@end

@implementation ARTGState (path_testing)
- (void) GScurrentpath: (NSBezierPath **)p
{
  *p = [path copy];
}
@end

@implementation ARTContext (path_testing)
/* TODO: this is just for testing */
- (void) GScurrentpath: (NSBezierPath **)p
{
  [(ARTGState *)gstate GScurrentpath: p];
}
@end

