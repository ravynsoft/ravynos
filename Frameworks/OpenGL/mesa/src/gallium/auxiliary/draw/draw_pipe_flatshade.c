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

/* Authors:  Keith Whitwell <keithw@vmware.com>
 */

#include "util/u_math.h"
#include "util/u_memory.h"

#include "pipe/p_shader_tokens.h"
#include "draw_vs.h"
#include "draw_fs.h"
#include "draw_pipe.h"


/** subclass of draw_stage */
struct flat_stage
{
   struct draw_stage stage;

   unsigned num_flat_attribs;
   unsigned flat_attribs[PIPE_MAX_SHADER_OUTPUTS];  /* flatshaded attribs */
};


static inline struct flat_stage *
flat_stage(struct draw_stage *stage)
{
   return (struct flat_stage *) stage;
}


/** Copy all the constant attributes from 'src' vertex to 'dst' vertex */
static inline void
copy_flats(struct draw_stage *stage,
           struct vertex_header *dst,
           const struct vertex_header *src)
{
   const struct flat_stage *flat = flat_stage(stage);
   for (unsigned i = 0; i < flat->num_flat_attribs; i++) {
      const unsigned attr = flat->flat_attribs[i];
      COPY_4FV(dst->data[attr], src->data[attr]);
   }
}


/** Copy all the color attributes from src vertex to dst0 & dst1 vertices */
static inline void
copy_flats2(struct draw_stage *stage,
            struct vertex_header *dst0,
            struct vertex_header *dst1,
            const struct vertex_header *src)
{
   const struct flat_stage *flat = flat_stage(stage);
   for (unsigned i = 0; i < flat->num_flat_attribs; i++) {
      const unsigned attr = flat->flat_attribs[i];
      COPY_4FV(dst0->data[attr], src->data[attr]);
      COPY_4FV(dst1->data[attr], src->data[attr]);
   }
}


/**
 * Flatshade tri. Not required for clipping which handles this on its own,
 * but required for unfilled tris and other primitive-changing stages
 * (like widelines). If no such stages are active, handled by hardware.
 */
static void
flatshade_tri_0(struct draw_stage *stage,
                struct prim_header *header)
{
   struct prim_header tmp;

   tmp.det = header->det;
   tmp.flags = header->flags;
   tmp.pad = header->pad;
   tmp.v[0] = header->v[0];
   tmp.v[1] = dup_vert(stage, header->v[1], 0);
   tmp.v[2] = dup_vert(stage, header->v[2], 1);

   copy_flats2(stage, tmp.v[1], tmp.v[2], tmp.v[0]);

   stage->next->tri(stage->next, &tmp);
}


static void
flatshade_tri_2(struct draw_stage *stage,
                struct prim_header *header)
{
   struct prim_header tmp;

   tmp.det = header->det;
   tmp.flags = header->flags;
   tmp.pad = header->pad;
   tmp.v[0] = dup_vert(stage, header->v[0], 0);
   tmp.v[1] = dup_vert(stage, header->v[1], 1);
   tmp.v[2] = header->v[2];

   copy_flats2(stage, tmp.v[0], tmp.v[1], tmp.v[2]);

   stage->next->tri(stage->next, &tmp);
}


/**
 * Flatshade line.
 */
static void
flatshade_line_0(struct draw_stage *stage,
                 struct prim_header *header)
{
   struct prim_header tmp;

   tmp.det = header->det;
   tmp.flags = header->flags;
   tmp.pad = header->pad;
   tmp.v[0] = header->v[0];
   tmp.v[1] = dup_vert(stage, header->v[1], 0);

   copy_flats(stage, tmp.v[1], tmp.v[0]);

   stage->next->line(stage->next, &tmp);
}


static void
flatshade_line_1(struct draw_stage *stage,
                 struct prim_header *header)
{
   struct prim_header tmp;

   tmp.det = header->det;
   tmp.flags = header->flags;
   tmp.pad = header->pad;
   tmp.v[0] = dup_vert(stage, header->v[0], 0);
   tmp.v[1] = header->v[1];

   copy_flats(stage, tmp.v[0], tmp.v[1]);

   stage->next->line(stage->next, &tmp);
}


static int
find_interp(const struct draw_fragment_shader *fs, int *indexed_interp,
            enum tgsi_semantic semantic_name, unsigned semantic_index)
{
   int interp;
   /* If it's gl_{Front,Back}{,Secondary}Color, pick up the mode
    * from the array we've filled before. */
   if ((semantic_name == TGSI_SEMANTIC_COLOR ||
        semantic_name == TGSI_SEMANTIC_BCOLOR) &&
       semantic_index < 2) {
      interp = indexed_interp[semantic_index];
   } else {
      /* Otherwise, search in the FS inputs, with a decent default
       * if we don't find it.
       */
      interp = TGSI_INTERPOLATE_PERSPECTIVE;
      if (fs) {
         for (unsigned j = 0; j < fs->info.num_inputs; j++) {
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


static void
flatshade_init_state(struct draw_stage *stage)
{
   struct flat_stage *flat = flat_stage(stage);
   const struct draw_context *draw = stage->draw;
   const struct draw_fragment_shader *fs = draw->fs.fragment_shader;
   const struct tgsi_shader_info *info = draw_get_shader_info(draw);
   unsigned i, j;

   /* Find which vertex shader outputs need constant interpolation, make a list */

   /* XXX: this code is a near exact copy of the one in clip_init_state.
    * The latter also cares about perspective though.
    */

   /* First pick up the interpolation mode for
    * gl_Color/gl_SecondaryColor, with the correct default.
    */
   int indexed_interp[2];
   indexed_interp[0] = indexed_interp[1] = draw->rasterizer->flatshade ?
      TGSI_INTERPOLATE_CONSTANT : TGSI_INTERPOLATE_PERSPECTIVE;

   if (fs) {
      for (i = 0; i < fs->info.num_inputs; i++) {
         if (fs->info.input_semantic_name[i] == TGSI_SEMANTIC_COLOR &&
             fs->info.input_semantic_index[i] < 2) {
            if (fs->info.input_interpolate[i] != TGSI_INTERPOLATE_COLOR)
               indexed_interp[fs->info.input_semantic_index[i]] = fs->info.input_interpolate[i];
         }
      }
   }

   /* Then resolve the interpolation mode for every output attribute.
    *
    * Given how the rest of the code, the most efficient way is to
    * have a vector of flat-mode attributes.
    */
   flat->num_flat_attribs = 0;
   for (i = 0; i < info->num_outputs; i++) {
      /* Find the interpolation mode for a specific attribute */
      int interp = find_interp(fs, indexed_interp,
                               info->output_semantic_name[i],
                               info->output_semantic_index[i]);
      /* If it's flat, add it to the flat vector. */

      if (interp == TGSI_INTERPOLATE_CONSTANT ||
          (interp == TGSI_INTERPOLATE_COLOR && draw->rasterizer->flatshade)) {
         flat->flat_attribs[flat->num_flat_attribs] = i;
         flat->num_flat_attribs++;
      }
   }
   /* Search the extra vertex attributes */
   for (j = 0; j < draw->extra_shader_outputs.num; j++) {
      /* Find the interpolation mode for a specific attribute */
      int interp = find_interp(fs, indexed_interp,
                               draw->extra_shader_outputs.semantic_name[j],
                               draw->extra_shader_outputs.semantic_index[j]);
      /* If it's flat, add it to the flat vector. */
      if (interp == TGSI_INTERPOLATE_CONSTANT) {
         flat->flat_attribs[flat->num_flat_attribs] = i + j;
         flat->num_flat_attribs++;
      }
   }

   /* Choose flatshade routine according to provoking vertex:
    */
   if (draw->rasterizer->flatshade_first) {
      stage->line = flatshade_line_0;
      stage->tri = flatshade_tri_0;
   } else {
      stage->line = flatshade_line_1;
      stage->tri = flatshade_tri_2;
   }
}


static void
flatshade_first_tri(struct draw_stage *stage,
                    struct prim_header *header)
{
   flatshade_init_state(stage);
   stage->tri(stage, header);
}


static void
flatshade_first_line(struct draw_stage *stage,
                     struct prim_header *header)
{
   flatshade_init_state(stage);
   stage->line(stage, header);
}


static void
flatshade_flush(struct draw_stage *stage,
                unsigned flags)
{
   stage->tri = flatshade_first_tri;
   stage->line = flatshade_first_line;
   stage->next->flush(stage->next, flags);
}


static void
flatshade_reset_stipple_counter(struct draw_stage *stage)
{
   stage->next->reset_stipple_counter(stage->next);
}


static void
flatshade_destroy(struct draw_stage *stage)
{
   draw_free_temp_verts(stage);
   FREE(stage);
}


/**
 * Create flatshading drawing stage.
 */
struct draw_stage *
draw_flatshade_stage(struct draw_context *draw)
{
   struct flat_stage *flatshade = CALLOC_STRUCT(flat_stage);
   if (!flatshade)
      goto fail;

   flatshade->stage.draw = draw;
   flatshade->stage.name = "flatshade";
   flatshade->stage.next = NULL;
   flatshade->stage.point = draw_pipe_passthrough_point;
   flatshade->stage.line = flatshade_first_line;
   flatshade->stage.tri = flatshade_first_tri;
   flatshade->stage.flush = flatshade_flush;
   flatshade->stage.reset_stipple_counter = flatshade_reset_stipple_counter;
   flatshade->stage.destroy = flatshade_destroy;

   if (!draw_alloc_temp_verts(&flatshade->stage, 2))
      goto fail;

   return &flatshade->stage;

 fail:
   if (flatshade)
      flatshade->stage.destroy(&flatshade->stage);

   return NULL;
}


