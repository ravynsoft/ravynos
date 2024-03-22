/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * \brief  Clipping stage
 *
 * \author  Keith Whitwell <keithw@vmware.com>
 */


#include "util/u_bitcast.h"
#include "util/u_memory.h"
#include "util/u_math.h"

#include "pipe/p_shader_tokens.h"

#include "draw_vs.h"
#include "draw_pipe.h"
#include "draw_fs.h"
#include "draw_gs.h"


/** Set to 1 to enable printing of coords before/after clipping */
#define DEBUG_CLIP 0

#define MAX_CLIPPED_VERTICES ((2 * (6 + PIPE_MAX_CLIP_PLANES))+1)


struct clip_stage {
   struct draw_stage stage;      /**< base class */

   unsigned pos_attr;
   bool have_clipdist;
   int cv_attr;

   /* List of the attributes to be constant interpolated. */
   unsigned num_const_attribs;
   uint8_t const_attribs[PIPE_MAX_SHADER_OUTPUTS];
   /* List of the attributes to be linear interpolated. */
   unsigned num_linear_attribs;
   uint8_t linear_attribs[PIPE_MAX_SHADER_OUTPUTS];
   /* List of the attributes to be perspective interpolated. */
   unsigned num_perspect_attribs;
   uint8_t perspect_attribs[PIPE_MAX_SHADER_OUTPUTS];

   float (*plane)[4];
};


/** Cast wrapper */
static inline struct clip_stage *
clip_stage(struct draw_stage *stage)
{
   return (struct clip_stage *) stage;
}


static inline unsigned
draw_viewport_index(struct draw_context *draw,
                    const struct vertex_header *leading_vertex)
{
   if (draw_current_shader_uses_viewport_index(draw)) {
      unsigned viewport_index_output =
         draw_current_shader_viewport_index_output(draw);
      unsigned viewport_index =
         u_bitcast_f2u(leading_vertex->data[viewport_index_output][0]);
      return draw_clamp_viewport_idx(viewport_index);
   } else {
      return 0;
   }
}


#define LINTERP(T, OUT, IN) ((OUT) + (T) * ((IN) - (OUT)))


/* All attributes are float[4], so this is easy:
 */
static void
interp_attr(float dst[4],
            float t,
            const float in[4],
            const float out[4])
{
   dst[0] = LINTERP(t, out[0], in[0]);
   dst[1] = LINTERP(t, out[1], in[1]);
   dst[2] = LINTERP(t, out[2], in[2]);
   dst[3] = LINTERP(t, out[3], in[3]);
}


/**
 * Copy flat shaded attributes src vertex to dst vertex.
 */
static void
copy_flat(struct draw_stage *stage,
          struct vertex_header *dst,
          const struct vertex_header *src)
{
   const struct clip_stage *clipper = clip_stage(stage);
   for (unsigned i = 0; i < clipper->num_const_attribs; i++) {
      const unsigned attr = clipper->const_attribs[i];
      COPY_4FV(dst->data[attr], src->data[attr]);
   }
}


/* Interpolate between two vertices to produce a third.
 */
static void
interp(const struct clip_stage *clip,
       struct vertex_header *dst,
       float t,
       const struct vertex_header *out,
       const struct vertex_header *in,
       unsigned viewport_index)
{
   const unsigned pos_attr = clip->pos_attr;

   /* Vertex header.
    */
   dst->clipmask = 0;
   dst->edgeflag = 0;        /* will get overwritten later */
   dst->pad = 0;
   dst->vertex_id = UNDEFINED_VERTEX_ID;

   /* Interpolate the clip-space coords.
    */
   if (clip->cv_attr >= 0) {
      interp_attr(dst->data[clip->cv_attr], t,
                  in->data[clip->cv_attr], out->data[clip->cv_attr]);
   }
   /* interpolate the clip-space position */
   interp_attr(dst->clip_pos, t, in->clip_pos, out->clip_pos);

   /* Do the projective divide and viewport transformation to get
    * new window coordinates:
    */
   {
      const float *pos = dst->clip_pos;
      const float *scale =
         clip->stage.draw->viewports[viewport_index].scale;
      const float *trans =
         clip->stage.draw->viewports[viewport_index].translate;
      const float oow = 1.0f / pos[3];

      dst->data[pos_attr][0] = pos[0] * oow * scale[0] + trans[0];
      dst->data[pos_attr][1] = pos[1] * oow * scale[1] + trans[1];
      dst->data[pos_attr][2] = pos[2] * oow * scale[2] + trans[2];
      dst->data[pos_attr][3] = oow;
   }

   /* interp perspective attribs */
   for (unsigned j = 0; j < clip->num_perspect_attribs; j++) {
      const unsigned attr = clip->perspect_attribs[j];
      interp_attr(dst->data[attr], t, in->data[attr], out->data[attr]);
   }

   /**
    * Compute the t in screen-space instead of 3d space to use
    * for noperspective interpolation.
    *
    * The points can be aligned with the X axis, so in that case try
    * the Y.  When both points are at the same screen position, we can
    * pick whatever value (the interpolated point won't be in front
    * anyway), so just use the 3d t.
    */
   if (clip->num_linear_attribs) {
      float t_nopersp = t;
      /* find either in.x != out.x or in.y != out.y */
      for (int k = 0; k < 2; k++) {
         if (in->clip_pos[k] != out->clip_pos[k]) {
            /* do divide by W, then compute linear interpolation factor */
            float in_coord = in->clip_pos[k] / in->clip_pos[3];
            float out_coord = out->clip_pos[k] / out->clip_pos[3];
            float dst_coord = dst->clip_pos[k] / dst->clip_pos[3];
            t_nopersp = (dst_coord - out_coord) / (in_coord - out_coord);
            break;
         }
      }
      for (unsigned j = 0; j < clip->num_linear_attribs; j++) {
         const unsigned attr = clip->linear_attribs[j];
         interp_attr(dst->data[attr], t_nopersp, in->data[attr], out->data[attr]);
      }
   }
}


/**
 * Emit a post-clip polygon to the next pipeline stage.  The polygon
 * will be convex and the provoking vertex will always be vertex[0].
 */
static void
emit_poly(struct draw_stage *stage,
          struct vertex_header **inlist,
          const bool *edgeflags,
          unsigned n,
          const struct prim_header *origPrim)
{
   const struct clip_stage *clipper = clip_stage(stage);
   uint16_t edge_first, edge_middle, edge_last;

   if (stage->draw->rasterizer->flatshade_first) {
      edge_first  = DRAW_PIPE_EDGE_FLAG_0;
      edge_middle = DRAW_PIPE_EDGE_FLAG_1;
      edge_last   = DRAW_PIPE_EDGE_FLAG_2;
   } else {
      edge_first  = DRAW_PIPE_EDGE_FLAG_2;
      edge_middle = DRAW_PIPE_EDGE_FLAG_0;
      edge_last   = DRAW_PIPE_EDGE_FLAG_1;
   }

   if (!edgeflags[0])
      edge_first = 0;

   /* later stages may need the determinant, but only the sign matters */
   struct prim_header header;
   header.det = origPrim->det;
   header.flags = DRAW_PIPE_RESET_STIPPLE | edge_first | edge_middle;
   header.pad = 0;

   for (unsigned i = 2; i < n; i++, header.flags = edge_middle) {
      /* order the triangle verts to respect the provoking vertex mode */
      if (stage->draw->rasterizer->flatshade_first) {
         header.v[0] = inlist[0];  /* the provoking vertex */
         header.v[1] = inlist[i-1];
         header.v[2] = inlist[i];
      } else {
         header.v[0] = inlist[i-1];
         header.v[1] = inlist[i];
         header.v[2] = inlist[0];  /* the provoking vertex */
      }

      if (!edgeflags[i-1]) {
         header.flags &= ~edge_middle;
      }

      if (i == n - 1 && edgeflags[i])
         header.flags |= edge_last;

      if (DEBUG_CLIP) {
         debug_printf("Clipped tri: (flat-shade-first = %d)\n",
                      stage->draw->rasterizer->flatshade_first);
         for (unsigned j = 0; j < 3; j++) {
            debug_printf("  Vert %d: clip pos: %f %f %f %f\n", j,
                         header.v[j]->clip_pos[0],
                         header.v[j]->clip_pos[1],
                         header.v[j]->clip_pos[2],
                         header.v[j]->clip_pos[3]);
            if (clipper->cv_attr >= 0) {
               debug_printf("  Vert %d: cv: %f %f %f %f\n", j,
                            header.v[j]->data[clipper->cv_attr][0],
                            header.v[j]->data[clipper->cv_attr][1],
                            header.v[j]->data[clipper->cv_attr][2],
                            header.v[j]->data[clipper->cv_attr][3]);
            }
            for (unsigned k = 0; k < draw_num_shader_outputs(stage->draw); k++) {
               debug_printf("  Vert %d: Attr %d:  %f %f %f %f\n", j, k,
                            header.v[j]->data[k][0],
                            header.v[j]->data[k][1],
                            header.v[j]->data[k][2],
                            header.v[j]->data[k][3]);
            }
         }
      }
      stage->next->tri(stage->next, &header);
   }
}


static inline float
dot4(const float *a, const float *b)
{
   return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

/*
 * this function extracts the clip distance for the current plane,
 * it first checks if the shader provided a clip distance, otherwise
 * it works out the value using the clipvertex
 */
static inline float
getclipdist(const struct clip_stage *clipper,
            struct vertex_header *vert,
            int plane_idx)
{
   const float *plane;
   float dp;

   if (plane_idx < 6) {
      /* ordinary xyz view volume clipping uses pos output */
      plane = clipper->plane[plane_idx];
      dp = dot4(vert->clip_pos, plane);
   }
   else if (clipper->have_clipdist) {
      /* pick the correct clipdistance element from the output vectors */
      int _idx = plane_idx - 6;
      int cdi = _idx >= 4;
      int vidx = cdi ? _idx - 4 : _idx;
      dp = vert->data[draw_current_shader_ccdistance_output(clipper->stage.draw, cdi)][vidx];
   } else {
      /*
       * legacy user clip planes or gl_ClipVertex
       */
      plane = clipper->plane[plane_idx];
      if (clipper->cv_attr >= 0) {
         dp = dot4(vert->data[clipper->cv_attr], plane);
      }
      else {
         dp = dot4(vert->clip_pos, plane);
      }
   }
   return dp;
}


/* Clip a triangle against the viewport and user clip planes.
 */
static void
do_clip_tri(struct draw_stage *stage,
            struct prim_header *header,
            unsigned clipmask)
{
   struct clip_stage *clipper = clip_stage(stage);
   struct vertex_header *a[MAX_CLIPPED_VERTICES];
   struct vertex_header *b[MAX_CLIPPED_VERTICES];
   struct vertex_header **inlist = a;
   struct vertex_header **outlist = b;
   struct vertex_header *prov_vertex;
   unsigned tmpnr = 0;
   unsigned n = 3;
   bool aEdges[MAX_CLIPPED_VERTICES];
   bool bEdges[MAX_CLIPPED_VERTICES];
   bool *inEdges = aEdges;
   bool *outEdges = bEdges;
   int viewport_index = 0;

   inlist[0] = header->v[0];
   inlist[1] = header->v[1];
   inlist[2] = header->v[2];

   /*
    * For d3d10, we need to take this from the leading (first) vertex.
    * For GL, we could do anything (as long as we advertize
    * GL_UNDEFINED_VERTEX for the VIEWPORT_INDEX_PROVOKING_VERTEX query),
    * but it needs to be consistent with what other parts (i.e. driver)
    * will do, and that seems easier with GL_PROVOKING_VERTEX logic.
    */
   if (stage->draw->rasterizer->flatshade_first) {
      prov_vertex = inlist[0];
   } else {
      prov_vertex = inlist[2];
   }
   viewport_index = draw_viewport_index(clipper->stage.draw, prov_vertex);

   if (DEBUG_CLIP) {
      const float *v0 = header->v[0]->clip_pos;
      const float *v1 = header->v[1]->clip_pos;
      const float *v2 = header->v[2]->clip_pos;
      debug_printf("Clip triangle pos:\n");
      debug_printf(" %f, %f, %f, %f\n", v0[0], v0[1], v0[2], v0[3]);
      debug_printf(" %f, %f, %f, %f\n", v1[0], v1[1], v1[2], v1[3]);
      debug_printf(" %f, %f, %f, %f\n", v2[0], v2[1], v2[2], v2[3]);
      if (clipper->cv_attr >= 0) {
         const float *v0 = header->v[0]->data[clipper->cv_attr];
         const float *v1 = header->v[1]->data[clipper->cv_attr];
         const float *v2 = header->v[2]->data[clipper->cv_attr];
         debug_printf("Clip triangle cv:\n");
         debug_printf(" %f, %f, %f, %f\n", v0[0], v0[1], v0[2], v0[3]);
         debug_printf(" %f, %f, %f, %f\n", v1[0], v1[1], v1[2], v1[3]);
         debug_printf(" %f, %f, %f, %f\n", v2[0], v2[1], v2[2], v2[3]);
      }
   }

   /*
    * Note: at this point we can't just use the per-vertex edge flags.
    * We have to observe the edge flag bits set in header->flags which
    * were set during primitive decomposition.  Put those flags into
    * an edge flags array which parallels the vertex array.
    * Later, in the 'unfilled' pipeline stage we'll draw the edge if both
    * the header.flags bit is set AND the per-vertex edgeflag field is set.
    */
   inEdges[0] = !!(header->flags & DRAW_PIPE_EDGE_FLAG_0);
   inEdges[1] = !!(header->flags & DRAW_PIPE_EDGE_FLAG_1);
   inEdges[2] = !!(header->flags & DRAW_PIPE_EDGE_FLAG_2);

   while (clipmask && n >= 3) {
      const unsigned plane_idx = ffs(clipmask)-1;
      const bool is_user_clip_plane = plane_idx >= 6;
      struct vertex_header *vert_prev = inlist[0];
      bool *edge_prev = &inEdges[0];
      float dp_prev;
      unsigned outcount = 0;

      dp_prev = getclipdist(clipper, vert_prev, plane_idx);
      clipmask &= ~(1<<plane_idx);

      if (util_is_inf_or_nan(dp_prev))
         return; //discard nan

      assert(n < MAX_CLIPPED_VERTICES);
      if (n >= MAX_CLIPPED_VERTICES)
         return;
      inlist[n] = inlist[0]; /* prevent rotation of vertices */
      inEdges[n] = inEdges[0];

      for (unsigned i = 1; i <= n; i++) {
         struct vertex_header *vert = inlist[i];
         bool *edge = &inEdges[i];
         bool different_sign;

         float dp = getclipdist(clipper, vert, plane_idx);

         if (util_is_inf_or_nan(dp))
            return; //discard nan

         if (dp_prev >= 0.0f) {
            assert(outcount < MAX_CLIPPED_VERTICES);
            if (outcount >= MAX_CLIPPED_VERTICES)
               return;
            outEdges[outcount] = *edge_prev;
            outlist[outcount++] = vert_prev;
            different_sign = dp < 0.0f;
         } else {
            different_sign = !(dp < 0.0f);
         }

         if (different_sign) {
            struct vertex_header *new_vert;
            bool *new_edge;

            assert(tmpnr < MAX_CLIPPED_VERTICES + 1);
            if (tmpnr >= MAX_CLIPPED_VERTICES + 1)
               return;
            new_vert = clipper->stage.tmp[tmpnr++];

            assert(outcount < MAX_CLIPPED_VERTICES);
            if (outcount >= MAX_CLIPPED_VERTICES)
               return;

            new_edge = &outEdges[outcount];
            outlist[outcount++] = new_vert;

            float denom = dp - dp_prev;
            if (dp < 0.0f) {
               /* Going out of bounds.  Avoid division by zero as we
                * know dp != dp_prev from different_sign, above.
                */
               if (-dp < dp_prev) {
                  float t = dp / denom;
                  interp(clipper, new_vert, t, vert, vert_prev, viewport_index);
               } else {
                  float t = -dp_prev / denom;
                  interp(clipper, new_vert, t, vert_prev, vert, viewport_index);
               }

               /* Whether or not to set edge flag for the new vert depends
                * on whether it's a user-defined clipping plane.  We're
                * copying NVIDIA's behaviour here.
                */
               if (is_user_clip_plane) {
                  /* we want to see an edge along the clip plane */
                  *new_edge = true;
                  new_vert->edgeflag = true;
               }
               else {
                  /* we don't want to see an edge along the frustum clip plane */
                  *new_edge = *edge_prev;
                  new_vert->edgeflag = false;
               }
            }
            else {
               /* Coming back in.
                */
               if (-dp_prev < dp) {
                  float t = -dp_prev / denom;
                  interp(clipper, new_vert, t, vert_prev, vert, viewport_index);
               } else {
                  float t = dp / denom;
                  interp(clipper, new_vert, t, vert, vert_prev, viewport_index);
               }

               /* Copy starting vert's edgeflag:
                */
               new_vert->edgeflag = vert_prev->edgeflag;
               *new_edge = *edge_prev;
            }
         }

         vert_prev = vert;
         edge_prev = edge;
         dp_prev = dp;
      }

      /* swap in/out lists */
      {
         struct vertex_header **tmp = inlist;
         inlist = outlist;
         outlist = tmp;
         n = outcount;
      }
      {
         bool *tmp = inEdges;
         inEdges = outEdges;
         outEdges = tmp;
      }

   }

   /* If constant interpolated, copy provoking vertex attrib to polygon vertex[0]
    */
   if (n >= 3) {
      if (clipper->num_const_attribs) {
         if (stage->draw->rasterizer->flatshade_first) {
            if (inlist[0] != header->v[0]) {
               assert(tmpnr < MAX_CLIPPED_VERTICES + 1);
               if (tmpnr >= MAX_CLIPPED_VERTICES + 1)
                  return;
               inlist[0] = dup_vert(stage, inlist[0], tmpnr++);
               copy_flat(stage, inlist[0], header->v[0]);
            }
         }
         else {
            if (inlist[0] != header->v[2]) {
               assert(tmpnr < MAX_CLIPPED_VERTICES + 1);
               if (tmpnr >= MAX_CLIPPED_VERTICES + 1)
                  return;
               inlist[0] = dup_vert(stage, inlist[0], tmpnr++);
               copy_flat(stage, inlist[0], header->v[2]);
            }
         }
      }

      /* Emit the polygon as triangles to the setup stage:
       */
      emit_poly(stage, inlist, inEdges, n, header);
   }
}


/* Clip a line against the viewport and user clip planes.
 */
static void
do_clip_line(struct draw_stage *stage,
             struct prim_header *header,
             unsigned clipmask)
{
   const struct clip_stage *clipper = clip_stage(stage);
   struct vertex_header *v0 = header->v[0];
   struct vertex_header *v1 = header->v[1];
   struct vertex_header *prov_vertex;
   float t0 = 0.0F;
   float t1 = 0.0F;
   struct prim_header newprim;
   int viewport_index;

   newprim.flags = header->flags;

   if (stage->draw->rasterizer->flatshade_first) {
      prov_vertex = v0;
   }
   else {
      prov_vertex = v1;
   }
   viewport_index = draw_viewport_index(clipper->stage.draw, prov_vertex);

   while (clipmask) {
      const unsigned plane_idx = ffs(clipmask)-1;
      const float dp0 = getclipdist(clipper, v0, plane_idx);
      const float dp1 = getclipdist(clipper, v1, plane_idx);

      if (util_is_inf_or_nan(dp0) || util_is_inf_or_nan(dp1))
         return; //discard nan

      if (dp1 < 0.0F) {
         float t = dp1 / (dp1 - dp0);
         t1 = MAX2(t1, t);
      }

      if (dp0 < 0.0F) {
         float t = dp0 / (dp0 - dp1);
         t0 = MAX2(t0, t);
      }

      if (t0 + t1 >= 1.0F)
         return; /* discard */

      clipmask &= ~(1 << plane_idx);  /* turn off this plane's bit */
   }

   if (v0->clipmask) {
      interp(clipper, stage->tmp[0], t0, v0, v1, viewport_index);
      if (stage->draw->rasterizer->flatshade_first) {
         copy_flat(stage, stage->tmp[0], v0);  /* copy v0 color to tmp[0] */
      }
      else {
         copy_flat(stage, stage->tmp[0], v1);  /* copy v1 color to tmp[0] */
      }
      newprim.v[0] = stage->tmp[0];
   }
   else {
      newprim.v[0] = v0;
   }

   if (v1->clipmask) {
      interp(clipper, stage->tmp[1], t1, v1, v0, viewport_index);
      if (stage->draw->rasterizer->flatshade_first) {
         copy_flat(stage, stage->tmp[1], v0);  /* copy v0 color to tmp[1] */
      }
      else {
         copy_flat(stage, stage->tmp[1], v1);  /* copy v1 color to tmp[1] */
      }
      newprim.v[1] = stage->tmp[1];
   }
   else {
      newprim.v[1] = v1;
   }

   stage->next->line(stage->next, &newprim);
}


static void
clip_point(struct draw_stage *stage, struct prim_header *header)
{
   if (header->v[0]->clipmask == 0)
      stage->next->point(stage->next, header);
}


/*
 * Clip points but ignore the first 4 (xy) clip planes.
 * (Because the generated clip mask is completely unaffacted by guard band,
 * we still need to manually evaluate the x/y planes if they are outside
 * the guard band and not just outside the vp.)
 */
static void
clip_point_guard_xy(struct draw_stage *stage, struct prim_header *header)
{
   unsigned clipmask = header->v[0]->clipmask;
   if ((clipmask & 0xffffffff) == 0)
      stage->next->point(stage->next, header);
   else if ((clipmask & 0xfffffff0) == 0) {
      while (clipmask) {
         const unsigned plane_idx = ffs(clipmask)-1;
         clipmask &= ~(1 << plane_idx);  /* turn off this plane's bit */
         /* TODO: this should really do proper guardband clipping,
          * currently just throw out infs/nans.
          * Also note that vertices with negative w values MUST be tossed
          * out (not sure if proper guardband clipping would do this
          * automatically). These would usually be captured by depth clip
          * too but this can be disabled.
          */
         if (header->v[0]->clip_pos[3] <= 0.0f ||
             util_is_inf_or_nan(header->v[0]->clip_pos[0]) ||
             util_is_inf_or_nan(header->v[0]->clip_pos[1]))
            return;
      }
      stage->next->point(stage->next, header);
   }
}


static void
clip_first_point(struct draw_stage *stage, struct prim_header *header)
{
   stage->point = stage->draw->guard_band_points_lines_xy ? clip_point_guard_xy : clip_point;
   stage->point(stage, header);
}


static void
clip_line(struct draw_stage *stage, struct prim_header *header)
{
   unsigned clipmask = (header->v[0]->clipmask |
                        header->v[1]->clipmask);

   if (clipmask == 0) {
      /* no clipping needed */
      stage->next->line(stage->next, header);
   }
   else if ((header->v[0]->clipmask &
             header->v[1]->clipmask) == 0) {
      do_clip_line(stage, header, clipmask);
   }
   /* else, totally clipped */
}

static void
clip_line_guard_xy(struct draw_stage *stage, struct prim_header *header)
{
   unsigned clipmask = (header->v[0]->clipmask |
                        header->v[1]->clipmask);

   if ((clipmask & 0xffffffff) == 0) {
      stage->next->line(stage->next, header);
   }
   else if ((clipmask & 0xfffffff0) == 0) {
      while (clipmask) {
         const unsigned plane_idx = ffs(clipmask)-1;
         clipmask &= ~(1 << plane_idx);  /* turn off this plane's bit */
         /* TODO: this should really do proper guardband clipping,
          * currently just throw out infs/nans.
          * Also note that vertices with negative w values MUST be tossed
          * out (not sure if proper guardband clipping would do this
          * automatically). These would usually be captured by depth clip
          * too but this can be disabled.
          */
         if ((header->v[0]->clip_pos[3] <= 0.0f &&
              header->v[1]->clip_pos[3] <= 0.0f) ||
             util_is_nan(header->v[0]->clip_pos[0]) ||
             util_is_nan(header->v[0]->clip_pos[1]) ||
             util_is_nan(header->v[1]->clip_pos[0]) ||
             util_is_nan(header->v[1]->clip_pos[1]))
            return;
      }
      stage->next->line(stage->next, header);
   } else if ((header->v[0]->clipmask &
               header->v[1]->clipmask) == 0) {
      do_clip_line(stage, header, clipmask & 0xfffffff0);
   }
}

static void
clip_tri(struct draw_stage *stage, struct prim_header *header)
{
   unsigned clipmask = (header->v[0]->clipmask |
                        header->v[1]->clipmask |
                        header->v[2]->clipmask);

   if (clipmask == 0) {
      /* no clipping needed */
      stage->next->tri(stage->next, header);
   }
   else if ((header->v[0]->clipmask &
             header->v[1]->clipmask &
             header->v[2]->clipmask) == 0) {
      do_clip_tri(stage, header, clipmask);
   }
}


static enum tgsi_interpolate_mode
find_interp(const struct draw_fragment_shader *fs,
            enum tgsi_interpolate_mode *indexed_interp,
            enum tgsi_semantic semantic_name, unsigned semantic_index)
{
   enum tgsi_interpolate_mode interp;

   /* If it's gl_{Front,Back}{,Secondary}Color, pick up the mode
    * from the array we've filled before. */
   if ((semantic_name == TGSI_SEMANTIC_COLOR ||
        semantic_name == TGSI_SEMANTIC_BCOLOR) &&
       semantic_index < 2) {
      interp = indexed_interp[semantic_index];
   } else if (semantic_name == TGSI_SEMANTIC_POSITION ||
              semantic_name == TGSI_SEMANTIC_CLIPVERTEX) {
      /* these inputs are handled specially always */
      return -1;
   } else {
      /* Otherwise, search in the FS inputs, with a decent default
       * if we don't find it.
       * This probably only matters for layer, vpindex, culldist, maybe
       * front_face.
       */
      unsigned j;
      if (semantic_name == TGSI_SEMANTIC_LAYER ||
          semantic_name == TGSI_SEMANTIC_VIEWPORT_INDEX) {
         interp = TGSI_INTERPOLATE_CONSTANT;
      }
      else {
         interp = TGSI_INTERPOLATE_PERSPECTIVE;
      }
      if (fs) {
         for (j = 0; j < fs->info.num_inputs; j++) {
            if (semantic_name == fs->info.input_semantic_name[j] &&
                semantic_index == fs->info.input_semantic_index[j]) {
               interp = fs->info.input_interpolate[j];
               break;
            }
         }
      }
   }
   return interp;
}


/* Update state.  Could further delay this until we hit the first
 * primitive that really requires clipping.
 */
static void
clip_init_state(struct draw_stage *stage)
{
   struct clip_stage *clipper = clip_stage(stage);
   const struct draw_context *draw = stage->draw;
   const struct draw_fragment_shader *fs = draw->fs.fragment_shader;
   const struct tgsi_shader_info *info = draw_get_shader_info(draw);

   clipper->pos_attr = draw_current_shader_position_output(draw);
   clipper->have_clipdist = draw_current_shader_num_written_clipdistances(draw) > 0;
   if (draw_current_shader_clipvertex_output(draw) != clipper->pos_attr) {
      clipper->cv_attr = (int)draw_current_shader_clipvertex_output(draw);
   }
   else {
      clipper->cv_attr = -1;
   }

   /* We need to know for each attribute what kind of interpolation is
    * done on it (flat, smooth or noperspective).  But the information
    * is not directly accessible for outputs, only for inputs.  So we
    * have to match semantic name and index between the VS (or GS/ES)
    * outputs and the FS inputs to get to the interpolation mode.
    *
    * The only hitch is with gl_FrontColor/gl_BackColor which map to
    * gl_Color, and their Secondary versions.  First there are (up to)
    * two outputs for one input, so we tuck the information in a
    * specific array.  Second if they don't have qualifiers, the
    * default value has to be picked from the global shade mode.
    *
    * Of course, if we don't have a fragment shader in the first
    * place, defaults should be used.
    */

   /* First pick up the interpolation mode for
    * gl_Color/gl_SecondaryColor, with the correct default.
    */
   enum tgsi_interpolate_mode indexed_interp[2];
   indexed_interp[0] = indexed_interp[1] = draw->rasterizer->flatshade ?
      TGSI_INTERPOLATE_CONSTANT : TGSI_INTERPOLATE_PERSPECTIVE;

   if (fs) {
      for (unsigned i = 0; i < fs->info.num_inputs; i++) {
         if (fs->info.input_semantic_name[i] == TGSI_SEMANTIC_COLOR &&
             fs->info.input_semantic_index[i] < 2) {
            if (fs->info.input_interpolate[i] != TGSI_INTERPOLATE_COLOR)
               indexed_interp[fs->info.input_semantic_index[i]] = fs->info.input_interpolate[i];
         }
      }
   }

   /* Then resolve the interpolation mode for every output attribute. */

   clipper->num_const_attribs = 0;
   clipper->num_linear_attribs = 0;
   clipper->num_perspect_attribs = 0;
   unsigned i;
   for (i = 0; i < info->num_outputs; i++) {
      /* Find the interpolation mode for a specific attribute */
      int interp = find_interp(fs, indexed_interp,
                               info->output_semantic_name[i],
                               info->output_semantic_index[i]);
      switch (interp) {
      case TGSI_INTERPOLATE_CONSTANT:
         clipper->const_attribs[clipper->num_const_attribs] = i;
         clipper->num_const_attribs++;
         break;
      case TGSI_INTERPOLATE_LINEAR:
         clipper->linear_attribs[clipper->num_linear_attribs] = i;
         clipper->num_linear_attribs++;
         break;
      case TGSI_INTERPOLATE_PERSPECTIVE:
         clipper->perspect_attribs[clipper->num_perspect_attribs] = i;
         clipper->num_perspect_attribs++;
         break;
      case TGSI_INTERPOLATE_COLOR:
         if (draw->rasterizer->flatshade) {
            clipper->const_attribs[clipper->num_const_attribs] = i;
            clipper->num_const_attribs++;
         } else {
            clipper->perspect_attribs[clipper->num_perspect_attribs] = i;
            clipper->num_perspect_attribs++;
         }
         break;
      default:
         assert(interp == -1);
         break;
      }
   }

   /* Search the extra vertex attributes */
   for (unsigned j = 0; j < draw->extra_shader_outputs.num; j++) {
      /* Find the interpolation mode for a specific attribute */
      enum tgsi_interpolate_mode interp =
         find_interp(fs, indexed_interp,
                     draw->extra_shader_outputs.semantic_name[j],
                     draw->extra_shader_outputs.semantic_index[j]);
      switch (interp) {
      case TGSI_INTERPOLATE_CONSTANT:
         clipper->const_attribs[clipper->num_const_attribs] = i + j;
         clipper->num_const_attribs++;
         break;
      case TGSI_INTERPOLATE_LINEAR:
         clipper->linear_attribs[clipper->num_linear_attribs] = i + j;
         clipper->num_linear_attribs++;
         break;
      case TGSI_INTERPOLATE_PERSPECTIVE:
         clipper->perspect_attribs[clipper->num_perspect_attribs] = i + j;
         clipper->num_perspect_attribs++;
         break;
      default:
         assert(interp == -1);
         break;
      }
   }

   stage->tri = clip_tri;
}


static void
clip_first_tri(struct draw_stage *stage,
               struct prim_header *header)
{
   clip_init_state(stage);
   stage->tri(stage, header);
}


static void
clip_first_line(struct draw_stage *stage,
                struct prim_header *header)
{
   clip_init_state(stage);
   stage->line = stage->draw->guard_band_points_lines_xy ? clip_line_guard_xy : clip_line;
   stage->line(stage, header);
}


static void
clip_flush(struct draw_stage *stage, unsigned flags)
{
   stage->tri = clip_first_tri;
   stage->line = clip_first_line;
   stage->next->flush(stage->next, flags);
}


static void
clip_reset_stipple_counter(struct draw_stage *stage)
{
   stage->next->reset_stipple_counter(stage->next);
}


static void
clip_destroy(struct draw_stage *stage)
{
   draw_free_temp_verts(stage);
   FREE(stage);
}


/**
 * Allocate a new clipper stage.
 * \return pointer to new stage object
 */
struct draw_stage *
draw_clip_stage(struct draw_context *draw)
{
   struct clip_stage *clipper = CALLOC_STRUCT(clip_stage);
   if (!clipper)
      goto fail;

   clipper->stage.draw = draw;
   clipper->stage.name = "clipper";
   clipper->stage.point = clip_first_point;
   clipper->stage.line = clip_first_line;
   clipper->stage.tri = clip_first_tri;
   clipper->stage.flush = clip_flush;
   clipper->stage.reset_stipple_counter = clip_reset_stipple_counter;
   clipper->stage.destroy = clip_destroy;

   clipper->plane = draw->plane;

   if (!draw_alloc_temp_verts(&clipper->stage, MAX_CLIPPED_VERTICES+1))
      goto fail;

   return &clipper->stage;

 fail:
   if (clipper)
      clipper->stage.destroy(&clipper->stage);

   return NULL;
}
