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
 * \brief  polygon offset state
 *
 * \author  Keith Whitwell <keithw@vmware.com>
 * \author  Brian Paul
 */

#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "draw_pipe.h"


struct offset_stage {
   struct draw_stage stage;

   float scale;
   float units;
   float clamp;
};


static inline struct offset_stage *
offset_stage(struct draw_stage *stage)
{
   return (struct offset_stage *) stage;
}


/**
 * Offset tri Z.  Some hardware can handle this, but not usually when
 * doing unfilled rendering.
 */
static void
do_offset_tri(struct draw_stage *stage,
              struct prim_header *header)
{
   const unsigned pos = draw_current_shader_position_output(stage->draw);
   struct offset_stage *offset = offset_stage(stage);
   float inv_det = 1.0f / header->det;

   /* Window coords:
    */
   float *v0 = header->v[0]->data[pos];
   float *v1 = header->v[1]->data[pos];
   float *v2 = header->v[2]->data[pos];

   /* edge vectors e = v0 - v2, f = v1 - v2 */
   float ex = v0[0] - v2[0];
   float ey = v0[1] - v2[1];
   float ez = v0[2] - v2[2];
   float fx = v1[0] - v2[0];
   float fy = v1[1] - v2[1];
   float fz = v1[2] - v2[2];

   /* (a,b) = cross(e,f).xy */
   float a = ey*fz - ez*fy;
   float b = ez*fx - ex*fz;

   float dzdx = fabsf(a * inv_det);
   float dzdy = fabsf(b * inv_det);

   float mult = MAX2(dzdx, dzdy) * offset->scale;

   float zoffset;
   if (stage->draw->floating_point_depth) {
      float bias;
      union fi maxz;
      maxz.f = MAX3(fabs(v0[2]), fabs(v1[2]), fabs(v2[2]));
      /* just do the math directly on shifted number */
      maxz.ui &= 0xff << 23;
      maxz.i -= 23 << 23;
      /* Clamping to zero means mrd will be zero for very small numbers,
       * but specs do not indicate this should be prevented by clamping
       * mrd to smallest normal number instead. */
      maxz.i = MAX2(maxz.i, 0);

      bias = offset->units * maxz.f;
      zoffset = bias + mult;
   } else {
      zoffset = offset->units + mult;
   }

   if (offset->clamp)
      zoffset = (offset->clamp < 0.0f) ? MAX2(zoffset, offset->clamp) :
                                         MIN2(zoffset, offset->clamp);

   /*
    * Note: we're applying the offset and clamping per-vertex.
    * Ideally, the offset is applied per-fragment prior to fragment shading.
    */
   v0[2] = SATURATE(v0[2] + zoffset);
   v1[2] = SATURATE(v1[2] + zoffset);
   v2[2] = SATURATE(v2[2] + zoffset);

   stage->next->tri(stage->next, header);
}


static void
offset_tri(struct draw_stage *stage,
           struct prim_header *header)
{
   struct prim_header tmp;

   tmp.det = header->det;
   tmp.flags = header->flags;
   tmp.pad = header->pad;
   tmp.v[0] = dup_vert(stage, header->v[0], 0);
   tmp.v[1] = dup_vert(stage, header->v[1], 1);
   tmp.v[2] = dup_vert(stage, header->v[2], 2);

   do_offset_tri(stage, &tmp);
}


static void
offset_first_tri(struct draw_stage *stage,
                 struct prim_header *header)
{
   struct offset_stage *offset = offset_stage(stage);
   const struct pipe_rasterizer_state *rast = stage->draw->rasterizer;
   unsigned fill_mode = rast->fill_front;
   bool do_offset;

   if (rast->fill_back != rast->fill_front) {
      /* Need to check for back-facing triangle */
      bool ccw = header->det < 0.0f;
      if (ccw != rast->front_ccw)
         fill_mode = rast->fill_back;
   }

   /* Now determine if we need to do offsetting for the point/line/fill mode */
   switch (fill_mode) {
   case PIPE_POLYGON_MODE_FILL:
      do_offset = rast->offset_tri;
      break;
   case PIPE_POLYGON_MODE_LINE:
      do_offset = rast->offset_line;
      break;
   case PIPE_POLYGON_MODE_POINT:
      do_offset = rast->offset_point;
      break;
   default:
      assert(!"invalid fill_mode in offset_first_tri()");
      do_offset = rast->offset_tri;
   }

   if (do_offset) {
      offset->scale = rast->offset_scale;
      offset->clamp = rast->offset_clamp;

      /*
       * If depth is floating point, depth bias is calculated with respect
       * to the primitive's maximum Z value. Retain the original depth bias
       * value until that stage.
       */
      if (stage->draw->floating_point_depth) {
         offset->units = (float) rast->offset_units;
      } else {
         offset->units = (float) (rast->offset_units * stage->draw->mrd * 2);
      }
   } else {
      offset->scale = 0.0f;
      offset->clamp = 0.0f;
      offset->units = 0.0f;
   }

   stage->tri = offset_tri;
   stage->tri(stage, header);
}


static void
offset_flush(struct draw_stage *stage,
             unsigned flags)
{
   stage->tri = offset_first_tri;
   stage->next->flush(stage->next, flags);
}


static void
offset_reset_stipple_counter(struct draw_stage *stage)
{
   stage->next->reset_stipple_counter(stage->next);
}


static void
offset_destroy(struct draw_stage *stage)
{
   draw_free_temp_verts(stage);
   FREE(stage);
}


/**
 * Create polygon offset drawing stage.
 */
struct draw_stage *
draw_offset_stage(struct draw_context *draw)
{
   struct offset_stage *offset = CALLOC_STRUCT(offset_stage);
   if (!offset)
      goto fail;

   offset->stage.draw = draw;
   offset->stage.name = "offset";
   offset->stage.next = NULL;
   offset->stage.point = draw_pipe_passthrough_point;
   offset->stage.line = draw_pipe_passthrough_line;
   offset->stage.tri = offset_first_tri;
   offset->stage.flush = offset_flush;
   offset->stage.reset_stipple_counter = offset_reset_stipple_counter;
   offset->stage.destroy = offset_destroy;

   if (!draw_alloc_temp_verts(&offset->stage, 3))
      goto fail;

   return &offset->stage;

fail:
   if (offset)
      offset->stage.destroy(&offset->stage);

   return NULL;
}
