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
 * \brief  Drawing stage for user culling
 */

#include "util/u_math.h"
#include "util/u_memory.h"
#include "pipe/p_defines.h"
#include "draw_pipe.h"


struct user_cull_stage {
   struct draw_stage stage;
};


static inline bool
cull_distance_is_out(float dist)
{
   return (dist < 0.0f) || util_is_inf_or_nan(dist);
}


/*
 * If the shader writes the culldistance then we can
 * perform distance based culling. Distance based
 * culling doesn't require a face and can be performed
 * on primitives without faces (e.g. points and lines)
 */
static void
user_cull_point(struct draw_stage *stage,
                struct prim_header *header)
{
   const unsigned num_written_culldistances =
      draw_current_shader_num_written_culldistances(stage->draw);
   const unsigned num_written_clipdistances =
      draw_current_shader_num_written_clipdistances(stage->draw);

   assert(num_written_culldistances);

   for (unsigned i = 0; i < num_written_culldistances; ++i) {
      unsigned cull_idx = (num_written_clipdistances + i) / 4;
      unsigned out_idx =
         draw_current_shader_ccdistance_output(stage->draw, cull_idx);
      unsigned idx = (num_written_clipdistances + i) % 4;
      float cull1 = header->v[0]->data[out_idx][idx];
      bool vert1_out = cull_distance_is_out(cull1);
      if (vert1_out)
         return;
   }
   stage->next->point(stage->next, header);
}


/*
 * If the shader writes the culldistance then we can
 * perform distance based culling. Distance based
 * culling doesn't require a face and can be performed
 * on primitives without faces (e.g. points and lines)
 */
static void
user_cull_line(struct draw_stage *stage,
               struct prim_header *header)
{
   const unsigned num_written_culldistances =
      draw_current_shader_num_written_culldistances(stage->draw);
   const unsigned num_written_clipdistances =
      draw_current_shader_num_written_clipdistances(stage->draw);

   assert(num_written_culldistances);

   for (unsigned i = 0; i < num_written_culldistances; ++i) {
      unsigned cull_idx = (num_written_clipdistances + i) / 4;
      unsigned out_idx =
         draw_current_shader_ccdistance_output(stage->draw, cull_idx);
      unsigned idx = (num_written_clipdistances + i) % 4;
      float cull1 = header->v[0]->data[out_idx][idx];
      float cull2 = header->v[1]->data[out_idx][idx];
      bool vert1_out = cull_distance_is_out(cull1);
      bool vert2_out = cull_distance_is_out(cull2);
      if (vert1_out && vert2_out)
         return;
   }
   stage->next->line(stage->next, header);
}


/*
 * Triangles can be culled either using the cull distance
 * shader outputs or the regular face culling. If required
 * this function performs both, starting with distance culling.
 */
static void
user_cull_tri(struct draw_stage *stage,
              struct prim_header *header)
{
   const unsigned num_written_culldistances =
      draw_current_shader_num_written_culldistances(stage->draw);
   const unsigned num_written_clipdistances =
      draw_current_shader_num_written_clipdistances(stage->draw);
   unsigned i;

   assert(num_written_culldistances);

   /* Do the distance culling */
   for (i = 0; i < num_written_culldistances; ++i) {
      unsigned cull_idx = (num_written_clipdistances + i) / 4;
      unsigned out_idx =
         draw_current_shader_ccdistance_output(stage->draw, cull_idx);
      unsigned idx = (num_written_clipdistances + i) % 4;
      float cull1 = header->v[0]->data[out_idx][idx];
      float cull2 = header->v[1]->data[out_idx][idx];
      float cull3 = header->v[2]->data[out_idx][idx];
      bool vert1_out = cull_distance_is_out(cull1);
      bool vert2_out = cull_distance_is_out(cull2);
      bool vert3_out = cull_distance_is_out(cull3);
      if (vert1_out && vert2_out && vert3_out) {
         return;
      }
   }
   stage->next->tri(stage->next, header);
}


static void
user_cull_flush(struct draw_stage *stage, unsigned flags)
{
   stage->point = user_cull_point;
   stage->line = user_cull_line;
   stage->tri = user_cull_tri;
   stage->next->flush(stage->next, flags);
}


static void
user_cull_reset_stipple_counter(struct draw_stage *stage)
{
   stage->next->reset_stipple_counter(stage->next);
}


static void
user_cull_destroy(struct draw_stage *stage)
{
   draw_free_temp_verts(stage);
   FREE(stage);
}


/**
 * Create a new polygon culling stage.
 */
struct draw_stage *
draw_user_cull_stage(struct draw_context *draw)
{
   struct user_cull_stage *user_cull = CALLOC_STRUCT(user_cull_stage);
   if (!user_cull)
      goto fail;

   user_cull->stage.draw = draw;
   user_cull->stage.name = "user_cull";
   user_cull->stage.next = NULL;
   user_cull->stage.point = user_cull_point;
   user_cull->stage.line = user_cull_line;
   user_cull->stage.tri = user_cull_tri;
   user_cull->stage.flush = user_cull_flush;
   user_cull->stage.reset_stipple_counter = user_cull_reset_stipple_counter;
   user_cull->stage.destroy = user_cull_destroy;

   if (!draw_alloc_temp_verts(&user_cull->stage, 0))
      goto fail;

   return &user_cull->stage;

fail:
   if (user_cull)
      user_cull->stage.destroy(&user_cull->stage);

   return NULL;
}
