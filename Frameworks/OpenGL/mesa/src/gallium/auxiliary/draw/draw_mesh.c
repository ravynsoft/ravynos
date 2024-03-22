/**************************************************************************
 *
 * Copyright 2023 Red Hat.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "draw_mesh.h"
#include "draw_context.h"

#include "tgsi/tgsi_scan.h"
#include "nir/nir_to_tgsi_info.h"
#include "pipe/p_shader_tokens.h"

struct draw_mesh_shader *
draw_create_mesh_shader(struct draw_context *draw,
                        const struct pipe_shader_state *state)
{
   struct draw_mesh_shader *ms;

   ms = CALLOC_STRUCT(draw_mesh_shader);

   if (!ms)
      return NULL;

   ms->draw = draw;

   nir_tgsi_scan_shader(state->ir.nir, &ms->info, true);

   nir_shader *nir = state->ir.nir;

   ms->output_primitive = nir->info.mesh.primitive_type;

   ms->position_output = -1;
   bool found_clipvertex = false;
   for (unsigned i = 0; i < ms->info.num_outputs; i++) {
      if (ms->info.output_semantic_name[i] == TGSI_SEMANTIC_POSITION &&
          ms->info.output_semantic_index[i] == 0)
         ms->position_output = i;
      if (ms->info.output_semantic_name[i] == TGSI_SEMANTIC_VIEWPORT_INDEX)
         ms->viewport_index_output = i;
      if (ms->info.output_semantic_name[i] == TGSI_SEMANTIC_CLIPVERTEX &&
          ms->info.output_semantic_index[i] == 0) {
         found_clipvertex = true;
         ms->clipvertex_output = i;
      }
      if (ms->info.output_semantic_name[i] == TGSI_SEMANTIC_CLIPDIST) {
         assert(ms->info.output_semantic_index[i] <
                      PIPE_MAX_CLIP_OR_CULL_DISTANCE_ELEMENT_COUNT);
         ms->ccdistance_output[ms->info.output_semantic_index[i]] = i;
      }
   }
   if (!found_clipvertex)
      ms->clipvertex_output = ms->position_output;
   return ms;
}

void
draw_bind_mesh_shader(struct draw_context *draw,
                      struct draw_mesh_shader *dms)
{
   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);

   if (dms) {
      draw->ms.mesh_shader = dms;
      draw->ms.num_ms_outputs = dms->info.num_outputs;
      draw->ms.position_output = dms->position_output;
      draw->ms.clipvertex_output = dms->clipvertex_output;
   } else {
      draw->ms.mesh_shader = NULL;
      draw->ms.num_ms_outputs = 0;
   }
}

void
draw_delete_mesh_shader(struct draw_context *draw,
                        struct draw_mesh_shader *dms)
{
   if (!dms)
      return;

   FREE(dms);
}
