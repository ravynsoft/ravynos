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

 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  *   Brian Paul
  */

#include "util/u_math.h"
#include "util/u_memory.h"

#include "pipe/p_shader_tokens.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"

#include "draw_private.h"
#include "draw_context.h"
#include "draw_vs.h"

#include "translate/translate.h"
#include "translate/translate_cache.h"

#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_exec.h"
#include "tgsi/tgsi_ureg.h"

#include "nir/nir_to_tgsi.h"

DEBUG_GET_ONCE_BOOL_OPTION(gallium_dump_vs, "GALLIUM_DUMP_VS", false)


struct draw_vertex_shader *
draw_create_vertex_shader(struct draw_context *draw,
                          const struct pipe_shader_state *shader)
{
   struct draw_vertex_shader *vs = NULL;
   struct pipe_shader_state state = *shader;

   if (draw->dump_vs) {
      tgsi_dump(shader->tokens, 0);
   }

#if DRAW_LLVM_AVAILABLE
   bool is_allocated = false;
   if (draw->pt.middle.llvm) {
      struct pipe_screen *screen = draw->pipe->screen;
      if (shader->type == PIPE_SHADER_IR_NIR &&
          !screen->get_shader_param(screen, PIPE_SHADER_VERTEX,
                                    PIPE_SHADER_CAP_INTEGERS)) {
        state.type = PIPE_SHADER_IR_TGSI;
        state.tokens = nir_to_tgsi(shader->ir.nir, screen);
        is_allocated = true;
      }
      vs = draw_create_vs_llvm(draw, &state);
   }
#endif

   if (!vs) {
      vs = draw_create_vs_exec(draw, &state);
   }

#if DRAW_LLVM_AVAILABLE
   if (is_allocated) {
      ureg_free_tokens(state.tokens);
   }
#endif

   if (vs) {
      bool found_clipvertex = false;
      vs->position_output = -1;
      for (unsigned i = 0; i < vs->info.num_outputs; i++) {
         if (vs->info.output_semantic_name[i] == TGSI_SEMANTIC_POSITION &&
             vs->info.output_semantic_index[i] == 0) {
            vs->position_output = i;
         } else if (vs->info.output_semantic_name[i] == TGSI_SEMANTIC_EDGEFLAG &&
             vs->info.output_semantic_index[i] == 0) {
            vs->edgeflag_output = i;
         } else if (vs->info.output_semantic_name[i] == TGSI_SEMANTIC_CLIPVERTEX &&
                  vs->info.output_semantic_index[i] == 0) {
            found_clipvertex = true;
            vs->clipvertex_output = i;
         } else if (vs->info.output_semantic_name[i] == TGSI_SEMANTIC_VIEWPORT_INDEX) {
            vs->viewport_index_output = i;
         } else if (vs->info.output_semantic_name[i] == TGSI_SEMANTIC_CLIPDIST) {
            assert(vs->info.output_semantic_index[i] <
                         PIPE_MAX_CLIP_OR_CULL_DISTANCE_ELEMENT_COUNT);
            vs->ccdistance_output[vs->info.output_semantic_index[i]] = i;
         }
      }
      if (!found_clipvertex)
         vs->clipvertex_output = vs->position_output;
   }

   assert(vs);
   return vs;
}


void
draw_bind_vertex_shader(struct draw_context *draw,
                        struct draw_vertex_shader *dvs)
{
   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);

   if (dvs) {
      draw->vs.vertex_shader = dvs;
      draw->vs.num_vs_outputs = dvs->info.num_outputs;
      draw->vs.position_output = dvs->position_output;
      draw->vs.edgeflag_output = dvs->edgeflag_output;
      draw->vs.clipvertex_output = dvs->clipvertex_output;
      draw->vs.ccdistance_output[0] = dvs->ccdistance_output[0];
      draw->vs.ccdistance_output[1] = dvs->ccdistance_output[1];
      dvs->prepare(dvs, draw);
      draw_update_clip_flags(draw);
      draw_update_viewport_flags(draw);
   } else {
      draw->vs.vertex_shader = NULL;
      draw->vs.num_vs_outputs = 0;
   }
}


void
draw_delete_vertex_shader(struct draw_context *draw,
                          struct draw_vertex_shader *dvs)
{
   for (unsigned i = 0; i < dvs->nr_variants; i++)
      dvs->variant[i]->destroy(dvs->variant[i]);

   dvs->nr_variants = 0;

   dvs->delete(dvs);
}


bool
draw_vs_init(struct draw_context *draw)
{
   draw->dump_vs = debug_get_option_gallium_dump_vs();

   if (!draw->llvm) {
      draw->vs.tgsi.machine = tgsi_exec_machine_create(PIPE_SHADER_VERTEX);
      if (!draw->vs.tgsi.machine)
         return false;
   }

   draw->vs.emit_cache = translate_cache_create();
   if (!draw->vs.emit_cache)
      return false;

   draw->vs.fetch_cache = translate_cache_create();
   if (!draw->vs.fetch_cache)
      return false;

   return true;
}


void
draw_vs_destroy(struct draw_context *draw)
{
   if (draw->vs.fetch_cache)
      translate_cache_destroy(draw->vs.fetch_cache);

   if (draw->vs.emit_cache)
      translate_cache_destroy(draw->vs.emit_cache);

   if (!draw->llvm)
      tgsi_exec_machine_destroy(draw->vs.tgsi.machine);
}


struct draw_vs_variant *
draw_vs_lookup_variant(struct draw_vertex_shader *vs,
                       const struct draw_vs_variant_key *key)
{
   /* Lookup existing variant:
    */
   for (unsigned i = 0; i < vs->nr_variants; i++)
      if (draw_vs_variant_key_compare(key, &vs->variant[i]->key) == 0)
         return vs->variant[i];

   /* Else have to create a new one:
    */
   struct draw_vs_variant *variant = vs->create_variant(vs, key);
   if (!variant)
      return NULL;

   /* Add it to our list, could be smarter:
    */
   if (vs->nr_variants < ARRAY_SIZE(vs->variant)) {
      vs->variant[vs->nr_variants++] = variant;
   } else {
      vs->last_variant++;
      vs->last_variant %= ARRAY_SIZE(vs->variant);
      vs->variant[vs->last_variant]->destroy(vs->variant[vs->last_variant]);
      vs->variant[vs->last_variant] = variant;
   }

   return variant;
}


struct translate *
draw_vs_get_fetch(struct draw_context *draw,
                  struct translate_key *key)
{
   if (!draw->vs.fetch ||
       translate_key_compare(&draw->vs.fetch->key, key) != 0) {
      translate_key_sanitize(key);
      draw->vs.fetch = translate_cache_find(draw->vs.fetch_cache, key);
   }

   return draw->vs.fetch;
}


struct translate *
draw_vs_get_emit(struct draw_context *draw,
                 struct translate_key *key)
{
   if (!draw->vs.emit ||
       translate_key_compare(&draw->vs.emit->key, key) != 0) {
      translate_key_sanitize(key);
      draw->vs.emit = translate_cache_find(draw->vs.emit_cache, key);
   }

   return draw->vs.emit;
}


void
draw_vs_attach_so(struct draw_vertex_shader *dvs,
                  const struct pipe_stream_output_info *info)
{
   dvs->state.stream_output = *info;
}


void
draw_vs_reset_so(struct draw_vertex_shader *dvs)
{
   memset(&dvs->state.stream_output, 0, sizeof(dvs->state.stream_output));
}
