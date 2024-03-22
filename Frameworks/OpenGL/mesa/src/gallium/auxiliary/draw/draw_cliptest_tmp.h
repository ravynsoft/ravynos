/**************************************************************************
 *
 * Copyright 2010, VMware, inc.
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

#include "util/u_bitcast.h"
#include <math.h>

static bool
TAG(do_cliptest)(struct pt_post_vs *pvs,
                 struct draw_vertex_info *info,
                 const struct draw_prim_info *prim_info)
{
   struct vertex_header *out = info->verts;
   /* const */ float (*plane)[4] = pvs->draw->plane;
   const unsigned pos = draw_current_shader_position_output(pvs->draw);
   const unsigned cv = draw_current_shader_clipvertex_output(pvs->draw);
   const unsigned ef = pvs->draw->vs.edgeflag_output;
   unsigned ucp_enable = pvs->draw->rasterizer->clip_plane_enable;
   unsigned flags = (FLAGS);
   unsigned need_pipeline = 0;
   bool have_cd = false;
   bool uses_vp_idx = draw_current_shader_uses_viewport_index(pvs->draw);
   unsigned viewport_index_output =
      draw_current_shader_viewport_index_output(pvs->draw);
   int viewport_index = 0;
   int num_written_clipdistance =
      draw_current_shader_num_written_clipdistances(pvs->draw);

   if (uses_vp_idx) {
      viewport_index = u_bitcast_f2u(out->data[viewport_index_output][0]);
      viewport_index = draw_clamp_viewport_idx(viewport_index);
   }

   unsigned cd[2];
   cd[0] = draw_current_shader_ccdistance_output(pvs->draw, 0);
   cd[1] = draw_current_shader_ccdistance_output(pvs->draw, 1);
   if (cd[0] != pos || cd[1] != pos)
      have_cd = true;

   /* If clipdistance semantic has been written by the shader
    * that means we're expected to do 'user plane clipping' */
   if (num_written_clipdistance && !(flags & DO_CLIP_USER)) {
      flags |= DO_CLIP_USER;
      ucp_enable = (1 << num_written_clipdistance) - 1;
   }

   assert(pos != -1);
   unsigned prim_idx = 0, prim_vert_idx = 0;
   for (unsigned j = 0; j < info->count; j++) {
      float *position = out->data[pos];
      unsigned mask = 0x0;

      if (uses_vp_idx) {
         /* only change the viewport_index for the leading vertex */
         if (prim_vert_idx == (prim_info->primitive_lengths[prim_idx])) {
            prim_idx++;
            prim_vert_idx = 0;
            viewport_index = u_bitcast_f2u(out->data[viewport_index_output][0]);
            viewport_index = draw_clamp_viewport_idx(viewport_index);
         }
         prim_vert_idx++;
      }
      float *scale = pvs->draw->viewports[viewport_index].scale;
      float *trans = pvs->draw->viewports[viewport_index].translate;
      initialize_vertex_header(out);

      if (flags & (DO_CLIP_XY | DO_CLIP_XY_GUARD_BAND |
                   DO_CLIP_FULL_Z | DO_CLIP_HALF_Z | DO_CLIP_USER)) {
         float *clipvertex = position;

         if ((flags & DO_CLIP_USER) && cv != pos) {
            assert(cv != -1);
            clipvertex = out->data[cv];
         }

         for (unsigned i = 0; i < 4; i++) {
            out->clip_pos[i] = position[i];
         }

         /* Be careful with NaNs. Comparisons must be true for them. */
         /* Do the hardwired planes first:
          */
         if (flags & DO_CLIP_XY_GUARD_BAND) {
            if (!(-0.50 * position[0] + position[3] >= 0)) mask |= (1<<0);
            if (!( 0.50 * position[0] + position[3] >= 0)) mask |= (1<<1);
            if (!(-0.50 * position[1] + position[3] >= 0)) mask |= (1<<2);
            if (!( 0.50 * position[1] + position[3] >= 0)) mask |= (1<<3);
         } else if (flags & DO_CLIP_XY) {
            if (!(-position[0] + position[3] >= 0)) mask |= (1<<0);
            if (!( position[0] + position[3] >= 0)) mask |= (1<<1);
            if (!(-position[1] + position[3] >= 0)) mask |= (1<<2);
            if (!( position[1] + position[3] >= 0)) mask |= (1<<3);
         }

         /* Clip Z planes according to full cube, half cube or none.
          */
         if (flags & DO_CLIP_FULL_Z) {
            if (!( position[2] + position[3] >= 0)) mask |= (1<<4);
            if (!(-position[2] + position[3] >= 0)) mask |= (1<<5);
         } else if (flags & DO_CLIP_HALF_Z) {
            if (!( position[2]               >= 0)) mask |= (1<<4);
            if (!(-position[2] + position[3] >= 0)) mask |= (1<<5);
         }

         if (flags & DO_CLIP_USER) {
            unsigned ucp_mask = ucp_enable;

            while (ucp_mask) {
               unsigned plane_idx = ffs(ucp_mask)-1;
               ucp_mask &= ~(1 << plane_idx);
               plane_idx += 6;

               /*
                * for user clipping check if we have a clip distance output
                * and the shader has written to it, otherwise use clipvertex
                * to decide when the plane is clipping.
                */
               if (have_cd && num_written_clipdistance) {
                  float clipdist;
                  unsigned i = plane_idx - 6;
                  /* first four clip distance in first vector etc. */
                  if (i < 4)
                     clipdist = out->data[cd[0]][i];
                  else
                     clipdist = out->data[cd[1]][i-4];
                  if (clipdist < 0 || util_is_inf_or_nan(clipdist))
                     mask |= 1 << plane_idx;
               } else {
                  if (!(dot4(clipvertex, plane[plane_idx]) >= 0))
                     mask |= 1 << plane_idx;
               }
            }
         }

         out->clipmask = mask;
         need_pipeline |= out->clipmask;
      }

      /*
       * Transform the vertex position from clip coords to window coords,
       * if the vertex is unclipped.
       */
      if ((flags & DO_VIEWPORT) && mask == 0) {
         /* divide by w */
         float w = 1.0f / position[3];

         /* Viewport mapping */
         position[0] = position[0] * w * scale[0] + trans[0];
         position[1] = position[1] * w * scale[1] + trans[1];
         position[2] = position[2] * w * scale[2] + trans[2];
         position[3] = w;
      }
#ifdef DEBUG
      /* For debug builds, set the clipped vertex's window coordinate
       * to NaN to help catch potential errors later.
       */
      else {
         position[0] =
         position[1] =
         position[2] =
         position[3] = NAN;
      }
#endif

      if ((flags & DO_EDGEFLAG) && ef) {
         const float *edgeflag = out->data[ef];
         out->edgeflag = !(edgeflag[0] != 1.0f);
         need_pipeline |= !out->edgeflag;
      }

      out = (struct vertex_header *)((char *)out + info->stride);
   }

   return need_pipeline != 0;
}


#undef FLAGS
#undef TAG
