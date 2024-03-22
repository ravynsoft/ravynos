/**************************************************************************
 * 
 * Copyright 2003 VMware, Inc.
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

#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "pipe/p_shader_tokens.h"
#include "draw/draw_context.h"
#include "draw/draw_vertex.h"
#include "sp_context.h"
#include "sp_screen.h"
#include "sp_state.h"
#include "sp_texture.h"
#include "sp_tex_sample.h"
#include "sp_tex_tile_cache.h"


/**
 * Mark the current vertex layout as "invalid".
 * We'll validate the vertex layout later, when we start to actually
 * render a point or line or tri.
 */
static void
invalidate_vertex_layout(struct softpipe_context *softpipe)
{
   softpipe->setup_info.valid =  0;
}


/**
 * The vertex info describes how to convert the post-transformed vertices
 * (simple float[][4]) used by the 'draw' module into vertices for
 * rasterization.
 *
 * This function validates the vertex layout.
 */
static void
softpipe_compute_vertex_info(struct softpipe_context *softpipe)
{
   struct sp_setup_info *sinfo = &softpipe->setup_info;

   if (sinfo->valid == 0) {
      const struct tgsi_shader_info *fsInfo = &softpipe->fs_variant->info;
      struct vertex_info *vinfo = &softpipe->vertex_info;
      uint i;
      int vs_index;
      /*
       * This doesn't quite work right (wrt face injection, prim id,
       * wide points) - hit a couple assertions, misrenderings plus
       * memory corruption. Albeit could fix (the former two) by calling
       * this "more often" (rasterizer changes etc.). (The latter would
       * need to be included in draw_prepare_shader_outputs, but it looks
       * like that would potentially allocate quite some unused additional
       * vertex outputs.)
       * draw_prepare_shader_outputs(softpipe->draw);
       */

      /*
       * Those can't actually be 0 (because pos is always at 0).
       * But use ints anyway to avoid confusion (in vs outputs, they
       * can very well be at pos 0).
       */
      softpipe->viewport_index_slot = -1;
      softpipe->layer_slot = -1;
      softpipe->psize_slot = -1;

      vinfo->num_attribs = 0;

      /*
       * Put position always first (setup needs it there).
       */
      vs_index = draw_find_shader_output(softpipe->draw,
                                         TGSI_SEMANTIC_POSITION, 0);

      draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);

      /*
       * Match FS inputs against VS outputs, emitting the necessary
       * attributes.
       */
      for (i = 0; i < fsInfo->num_inputs; i++) {
         enum sp_interp_mode interp = SP_INTERP_LINEAR;

         switch (fsInfo->input_interpolate[i]) {
         case TGSI_INTERPOLATE_CONSTANT:
            interp = SP_INTERP_CONSTANT;
            break;
         case TGSI_INTERPOLATE_LINEAR:
            interp = SP_INTERP_LINEAR;
            break;
         case TGSI_INTERPOLATE_PERSPECTIVE:
            interp = SP_INTERP_PERSPECTIVE;
            break;
         case TGSI_INTERPOLATE_COLOR:
            assert(fsInfo->input_semantic_name[i] == TGSI_SEMANTIC_COLOR);
            break;
         default:
            assert(0);
         }

         switch (fsInfo->input_semantic_name[i]) {
         case TGSI_SEMANTIC_POSITION:
            interp = SP_INTERP_POS;
            break;

         case TGSI_SEMANTIC_COLOR:
            if (fsInfo->input_interpolate[i] == TGSI_INTERPOLATE_COLOR) {
               if (softpipe->rasterizer->flatshade)
                  interp = SP_INTERP_CONSTANT;
               else
                  interp = SP_INTERP_PERSPECTIVE;
            }
            break;
         }

         /*
          * Search for each input in current vs output:
          */
         vs_index = draw_find_shader_output(softpipe->draw,
                                            fsInfo->input_semantic_name[i],
                                            fsInfo->input_semantic_index[i]);

         if (fsInfo->input_semantic_name[i] == TGSI_SEMANTIC_COLOR &&
             vs_index == -1) {
            /*
             * try and find a bcolor.
             * Note that if there's both front and back color, draw will
             * have copied back to front color already.
             */
            vs_index = draw_find_shader_output(softpipe->draw,
                                               TGSI_SEMANTIC_BCOLOR,
                                               fsInfo->input_semantic_index[i]);
         }

         sinfo->attrib[i].interp = interp;
         /* extremely pointless index map */
         sinfo->attrib[i].src_index = i + 1;
         /*
          * For vp index and layer, if the fs requires them but the vs doesn't
          * provide them, draw (vbuf) will give us the required 0 (slot -1).
          * (This means in this case we'll also use those slots in setup, which
          * isn't necessary but they'll contain the correct (0) value.)
          */
         if (fsInfo->input_semantic_name[i] ==
                    TGSI_SEMANTIC_VIEWPORT_INDEX) {
            softpipe->viewport_index_slot = (int)vinfo->num_attribs;
            draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
         } else if (fsInfo->input_semantic_name[i] == TGSI_SEMANTIC_LAYER) {
            softpipe->layer_slot = (int)vinfo->num_attribs;
            draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
            /*
             * Note that we'd actually want to skip position (as we won't use
             * the attribute in the fs) but can't. The reason is that we don't
             * actually have an input/output map for setup (even though it looks
             * like we do...). Could adjust for this though even without a map.
             */
         } else {
            /*
             * Note that we'd actually want to skip position (as we won't use
             * the attribute in the fs) but can't. The reason is that we don't
             * actually have an input/output map for setup (even though it looks
             * like we do...). Could adjust for this though even without a map.
             */
            draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
         }
      }

      /* Figure out if we need pointsize as well.
       */
      vs_index = draw_find_shader_output(softpipe->draw,
                                         TGSI_SEMANTIC_PSIZE, 0);

      if (vs_index >= 0) {
         softpipe->psize_slot = (int)vinfo->num_attribs;
         draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
      }

      /* Figure out if we need viewport index (if it wasn't already in fs input) */
      if (softpipe->viewport_index_slot < 0) {
         vs_index = draw_find_shader_output(softpipe->draw,
                                            TGSI_SEMANTIC_VIEWPORT_INDEX,
                                            0);
         if (vs_index >= 0) {
            softpipe->viewport_index_slot =(int)vinfo->num_attribs;
            draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
         }
      }

      /* Figure out if we need layer (if it wasn't already in fs input) */
      if (softpipe->layer_slot < 0) {
         vs_index = draw_find_shader_output(softpipe->draw,
                                            TGSI_SEMANTIC_LAYER,
                                            0);
         if (vs_index >= 0) {
            softpipe->layer_slot = (int)vinfo->num_attribs;
            draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
         }
      }

      draw_compute_vertex_size(vinfo);
      softpipe->setup_info.valid = 1;
   }
   return;
}


/**
 * Called from vbuf module.
 *
 * This will trigger validation of the vertex layout (and also compute
 * the required information for setup).
 */
struct vertex_info *
softpipe_get_vbuf_vertex_info(struct softpipe_context *softpipe)
{
   softpipe_compute_vertex_info(softpipe);
   return &softpipe->vertex_info;
}


/**
 * Recompute cliprect from scissor bounds, scissor enable and surface size.
 */
static void
compute_cliprect(struct softpipe_context *sp)
{
   unsigned i;
   /* SP_NEW_FRAMEBUFFER
    */
   uint surfWidth = sp->framebuffer.width;
   uint surfHeight = sp->framebuffer.height;

   for (i = 0; i < PIPE_MAX_VIEWPORTS; i++) {
      /* SP_NEW_RASTERIZER
       */
      if (sp->rasterizer->scissor) {

         /* SP_NEW_SCISSOR
          *
          * clip to scissor rect:
          */
         sp->cliprect[i].minx = MAX2(sp->scissors[i].minx, 0);
         sp->cliprect[i].miny = MAX2(sp->scissors[i].miny, 0);
         sp->cliprect[i].maxx = MIN2(sp->scissors[i].maxx, surfWidth);
         sp->cliprect[i].maxy = MIN2(sp->scissors[i].maxy, surfHeight);
      }
      else {
         /* clip to surface bounds */
         sp->cliprect[i].minx = 0;
         sp->cliprect[i].miny = 0;
         sp->cliprect[i].maxx = surfWidth;
         sp->cliprect[i].maxy = surfHeight;
      }
   }
}


static void
set_shader_sampler(struct softpipe_context *softpipe,
                   enum pipe_shader_type shader,
                   int max_sampler)
{
   int i;
   for (i = 0; i <= max_sampler; i++) {
      softpipe->tgsi.sampler[shader]->sp_sampler[i] =
         (struct sp_sampler *)(softpipe->samplers[shader][i]);
   }
}

void
softpipe_update_compute_samplers(struct softpipe_context *softpipe)
{
   set_shader_sampler(softpipe, PIPE_SHADER_COMPUTE, softpipe->cs->max_sampler);
}

static void
update_tgsi_samplers( struct softpipe_context *softpipe )
{
   unsigned i, sh;

   set_shader_sampler(softpipe, PIPE_SHADER_VERTEX,
                      softpipe->vs->max_sampler);
   set_shader_sampler(softpipe, PIPE_SHADER_FRAGMENT,
                      softpipe->fs_variant->info.file_max[TGSI_FILE_SAMPLER]);
   if (softpipe->gs) {
      set_shader_sampler(softpipe, PIPE_SHADER_GEOMETRY,
                         softpipe->gs->max_sampler);
   }

   /* XXX is this really necessary here??? */
   for (sh = 0; sh < ARRAY_SIZE(softpipe->tex_cache); sh++) {
      for (i = 0; i < PIPE_MAX_SAMPLERS; i++) {
         struct softpipe_tex_tile_cache *tc = softpipe->tex_cache[sh][i];
         if (tc && tc->texture) {
            struct softpipe_resource *spt = softpipe_resource(tc->texture);
            if (spt->timestamp != tc->timestamp) {
               sp_tex_tile_cache_validate_texture( tc );
               /*
                 _debug_printf("INV %d %d\n", tc->timestamp, spt->timestamp);
               */
               tc->timestamp = spt->timestamp;
            }
         }
      }
   }
}


static void
update_fragment_shader(struct softpipe_context *softpipe, unsigned prim)
{
   struct sp_fragment_shader_variant_key key;

   memset(&key, 0, sizeof(key));

   if (softpipe->fs) {
      softpipe->fs_variant = softpipe_find_fs_variant(softpipe,
                                                      softpipe->fs, &key);

      /* prepare the TGSI interpreter for FS execution */
      softpipe->fs_variant->prepare(softpipe->fs_variant, 
                                    softpipe->fs_machine,
                                    (struct tgsi_sampler *) softpipe->
                                    tgsi.sampler[PIPE_SHADER_FRAGMENT],
                                    (struct tgsi_image *)softpipe->tgsi.image[PIPE_SHADER_FRAGMENT],
                                    (struct tgsi_buffer *)softpipe->tgsi.buffer[PIPE_SHADER_FRAGMENT]);
   }
   else {
      softpipe->fs_variant = NULL;
   }

   /* This would be the logical place to pass the fragment shader
    * to the draw module.  However, doing this here, during state
    * validation, causes problems with the 'draw' module helpers for
    * wide/AA/stippled lines.
    * In principle, the draw's fragment shader should be per-variant
    * but that doesn't work.  So we use a single draw fragment shader
    * per fragment shader, not per variant.
    */
#if 0
   if (softpipe->fs_variant) {
      draw_bind_fragment_shader(softpipe->draw,
                                softpipe->fs_variant->draw_shader);
   }
   else {
      draw_bind_fragment_shader(softpipe->draw, NULL);
   }
#endif
}


/* Hopefully this will remain quite simple, otherwise need to pull in
 * something like the gallium frontend mechanism.
 */
void
softpipe_update_derived(struct softpipe_context *softpipe, unsigned prim)
{
   struct softpipe_screen *sp_screen = softpipe_screen(softpipe->pipe.screen);

   /* Check for updated textures.
    */
   if (softpipe->tex_timestamp != sp_screen->timestamp) {
      softpipe->tex_timestamp = sp_screen->timestamp;
      softpipe->dirty |= SP_NEW_TEXTURE;
   }

   if (softpipe->dirty & (SP_NEW_RASTERIZER |
                          SP_NEW_FS))
      update_fragment_shader(softpipe, prim);

   /* TODO: this looks suboptimal */
   if (softpipe->dirty & (SP_NEW_SAMPLER |
                          SP_NEW_TEXTURE |
                          SP_NEW_FS | 
                          SP_NEW_VS))
      update_tgsi_samplers( softpipe );

   if (softpipe->dirty & (SP_NEW_RASTERIZER |
                          SP_NEW_FS |
                          SP_NEW_VS))
      invalidate_vertex_layout( softpipe );

   if (softpipe->dirty & (SP_NEW_SCISSOR |
                          SP_NEW_RASTERIZER |
                          SP_NEW_FRAMEBUFFER))
      compute_cliprect(softpipe);

   if (softpipe->dirty & (SP_NEW_BLEND |
                          SP_NEW_DEPTH_STENCIL_ALPHA |
                          SP_NEW_FRAMEBUFFER |
                          SP_NEW_FS))
      sp_build_quad_pipeline(softpipe);

   softpipe->dirty = 0;
}
