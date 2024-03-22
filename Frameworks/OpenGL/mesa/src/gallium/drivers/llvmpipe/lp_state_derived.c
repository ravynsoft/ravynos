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

#include "util/u_math.h"
#include "util/u_memory.h"
#include "pipe/p_shader_tokens.h"
#include "draw/draw_context.h"
#include "draw/draw_vertex.h"
#include "draw/draw_private.h"
#include "lp_context.h"
#include "lp_screen.h"
#include "lp_setup.h"
#include "lp_state.h"

#include "tgsi/tgsi_from_mesa.h"

/**
 * The vertex info describes how to convert the post-transformed vertices
 * (simple float[][4]) used by the 'draw' module into vertices for
 * rasterization.
 *
 * This function validates the vertex layout.
 */
static void
compute_vertex_info(struct llvmpipe_context *llvmpipe)
{
   struct vertex_info *vinfo = &llvmpipe->vertex_info;

   draw_prepare_shader_outputs(llvmpipe->draw);

   /*
    * Those can't actually be 0 (because pos is always at 0).
    * But use ints anyway to avoid confusion (in vs outputs, they
    * can very well be at pos 0).
    */
   llvmpipe->color_slot[0] = -1;
   llvmpipe->color_slot[1] = -1;
   llvmpipe->bcolor_slot[0] = -1;
   llvmpipe->bcolor_slot[1] = -1;
   llvmpipe->viewport_index_slot = -1;
   llvmpipe->layer_slot = -1;
   llvmpipe->face_slot = -1;
   llvmpipe->psize_slot = -1;

   /*
    * Match FS inputs against VS outputs, emitting the necessary
    * attributes.  Could cache these structs and look them up with a
    * combination of fragment shader, vertex shader ids.
    */

   vinfo->num_attribs = 0;

   int vs_index = draw_find_shader_output(llvmpipe->draw,
                                          TGSI_SEMANTIC_POSITION, 0);

   draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);

   struct nir_shader *nir = llvmpipe->fs->base.ir.nir;
   uint64_t slot_emitted = 0;
   nir_foreach_shader_in_variable(var, nir) {
      unsigned tgsi_semantic_name, tgsi_semantic_index;
      unsigned slots = nir_variable_count_slots(var, var->type);
      tgsi_get_gl_varying_semantic(var->data.location,
                                   true,
                                   &tgsi_semantic_name,
                                   &tgsi_semantic_index);

      for (unsigned i = 0; i < slots; i++) {
         vs_index = draw_find_shader_output(llvmpipe->draw,
                                            tgsi_semantic_name,
                                            tgsi_semantic_index);
         if (slot_emitted & BITFIELD64_BIT(vs_index)) {
            tgsi_semantic_index++;
            continue;
         }

         if (tgsi_semantic_name == TGSI_SEMANTIC_COLOR &&
             tgsi_semantic_index < 2) {
            int idx = tgsi_semantic_index;
            llvmpipe->color_slot[idx] = (int)vinfo->num_attribs;
         }
         if (tgsi_semantic_name == TGSI_SEMANTIC_FACE) {
            llvmpipe->face_slot = (int)vinfo->num_attribs;
            draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
            /*
             * For vp index and layer, if the fs requires them but the vs doesn't
             * provide them, draw (vbuf) will give us the required 0 (slot -1).
             * (This means in this case we'll also use those slots in setup, which
             * isn't necessary but they'll contain the correct (0) value.)
             */
         } else if (tgsi_semantic_name == TGSI_SEMANTIC_VIEWPORT_INDEX) {
            llvmpipe->viewport_index_slot = (int)vinfo->num_attribs;
            draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
         } else if (tgsi_semantic_name == TGSI_SEMANTIC_LAYER) {
            llvmpipe->layer_slot = (int)vinfo->num_attribs;
            draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
         } else {
            /*
             * Note that we'd actually want to skip position (as we won't use
             * the attribute in the fs) but can't. The reason is that we don't
             * actually have an input/output map for setup (even though it looks
             * like we do...). Could adjust for this though even without a map
             * (in llvmpipe_create_fs_state()).
             */
            draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
         }
         slot_emitted |= BITFIELD64_BIT(vs_index);
         tgsi_semantic_index++;
      }
   }

   /*
    * The new style front face is a system value, hence won't show up as
    * ordinary fs register above. But we still need to assign a vs output
    * location so draw can inject face info for unfilled tris.
    */
   if (llvmpipe->face_slot < 0 &&
       BITSET_TEST(nir->info.system_values_read, SYSTEM_VALUE_FRONT_FACE)) {
      vs_index = draw_find_shader_output(llvmpipe->draw,
                                         TGSI_SEMANTIC_FACE, 0);
      llvmpipe->face_slot = (int)vinfo->num_attribs;
      draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
   }

   /* Figure out if we need bcolor as well.
    */
   for (unsigned i = 0; i < 2; i++) {
      vs_index = draw_find_shader_output(llvmpipe->draw,
                                         TGSI_SEMANTIC_BCOLOR, i);

      if (vs_index >= 0) {
         llvmpipe->bcolor_slot[i] = (int)vinfo->num_attribs;
         draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
      }
   }

   /* Figure out if we need pointsize as well.
    */
   vs_index = draw_find_shader_output(llvmpipe->draw,
                                      TGSI_SEMANTIC_PSIZE, 0);

   if (vs_index >= 0) {
      llvmpipe->psize_slot = (int)vinfo->num_attribs;
      draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
   }

   /* Figure out if we need viewport index (if it wasn't already in fs input) */
   if (llvmpipe->viewport_index_slot < 0) {
      vs_index = draw_find_shader_output(llvmpipe->draw,
                                         TGSI_SEMANTIC_VIEWPORT_INDEX,
                                         0);
      if (vs_index >= 0) {
         llvmpipe->viewport_index_slot =(int)vinfo->num_attribs;
         draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
      }
   }

   /* Figure out if we need layer (if it wasn't already in fs input) */
   if (llvmpipe->layer_slot < 0) {
      vs_index = draw_find_shader_output(llvmpipe->draw,
                                         TGSI_SEMANTIC_LAYER,
                                         0);
      if (vs_index >= 0) {
         llvmpipe->layer_slot = (int)vinfo->num_attribs;
         draw_emit_vertex_attr(vinfo, EMIT_4F, vs_index);
      }
   }

   draw_compute_vertex_size(vinfo);
   lp_setup_set_vertex_info(llvmpipe->setup, vinfo);
}


static void
check_linear_rasterizer(struct llvmpipe_context *lp)
{
   const bool valid_cb_format =
      (lp->framebuffer.nr_cbufs == 1 && lp->framebuffer.cbufs[0] &&
       util_res_sample_count(lp->framebuffer.cbufs[0]->texture) == 1 &&
       lp->framebuffer.cbufs[0]->texture->target == PIPE_TEXTURE_2D &&
       (lp->framebuffer.cbufs[0]->format == PIPE_FORMAT_B8G8R8A8_UNORM ||
        lp->framebuffer.cbufs[0]->format == PIPE_FORMAT_B8G8R8X8_UNORM ||
        lp->framebuffer.cbufs[0]->format == PIPE_FORMAT_R8G8B8A8_UNORM ||
        lp->framebuffer.cbufs[0]->format == PIPE_FORMAT_R8G8B8X8_UNORM));

   /* permit_linear means guardband, hence fake scissor, which we can only
    * handle if there's just one vp. */
   const bool single_vp = lp->viewport_index_slot < 0;
   const bool permit_linear = (!lp->framebuffer.zsbuf &&
                               valid_cb_format &&
                               single_vp);

   /* Tell draw that we're happy doing our own x/y clipping.
    */
   bool clipping_changed = false;
   if (lp->permit_linear_rasterizer != permit_linear) {
      lp->permit_linear_rasterizer = permit_linear;
      lp_setup_set_linear_mode(lp->setup, permit_linear);
      clipping_changed = true;
   }

   if (lp->single_vp != single_vp) {
      lp->single_vp = single_vp;
      clipping_changed = true;
   }

   /* Disable xy clipping in linear mode.
    *
    * Use a guard band if we don't have zsbuf.  Could enable
    * guardband always - this just to be conservative.
    *
    * Because we have a layering violation where the draw module emits
    * state changes to the driver while we're already inside a draw
    * call, need to be careful about when we make calls back to the
    * draw module.  Hence the clipping_changed flag which is as much
    * to prevent flush recursion as it is to short-circuit noop state
    * changes.
    */
   if (clipping_changed) {
      draw_set_driver_clipping(lp->draw,
                               false, // bypass_clip_xy
                               false, //bypass_clip_z
                               permit_linear, // guard_band_xy,
                               single_vp); // bypass_clip_points)
   }
}


/**
 * Handle state changes before clears.
 * Called just prior to clearing (pipe::clear()).
 */
void
llvmpipe_update_derived_clear(struct llvmpipe_context *llvmpipe)
{
   if (llvmpipe->dirty & (LP_NEW_FS |
                          LP_NEW_FRAMEBUFFER))
      check_linear_rasterizer(llvmpipe);
}


/**
 * Handle state changes.
 * Called just prior to drawing anything (pipe::draw_arrays(), etc).
 *
 * Hopefully this will remain quite simple, otherwise need to pull in
 * something like the gallium frontend mechanism.
 */
void
llvmpipe_update_derived(struct llvmpipe_context *llvmpipe)
{
   struct llvmpipe_screen *lp_screen = llvmpipe_screen(llvmpipe->pipe.screen);

   /* Check for updated textures.
    */
   if (llvmpipe->tex_timestamp != lp_screen->timestamp) {
      llvmpipe->tex_timestamp = lp_screen->timestamp;
      llvmpipe->dirty |= LP_NEW_SAMPLER_VIEW;
   }

   if (llvmpipe->dirty & (LP_NEW_TASK))
      llvmpipe_update_task_shader(llvmpipe);

   if (llvmpipe->dirty & (LP_NEW_MESH))
      llvmpipe_update_mesh_shader(llvmpipe);

   /* This needs LP_NEW_RASTERIZER because of draw_prepare_shader_outputs(). */
   if (llvmpipe->dirty & (LP_NEW_RASTERIZER |
                          LP_NEW_FS |
                          LP_NEW_GS |
                          LP_NEW_TCS |
                          LP_NEW_TES |
                          LP_NEW_MESH |
                          LP_NEW_VS))
      compute_vertex_info(llvmpipe);

   if (llvmpipe->dirty & (LP_NEW_FS |
                          LP_NEW_FRAMEBUFFER |
                          LP_NEW_BLEND |
                          LP_NEW_SCISSOR |
                          LP_NEW_DEPTH_STENCIL_ALPHA |
                          LP_NEW_RASTERIZER |
                          LP_NEW_SAMPLER |
                          LP_NEW_SAMPLER_VIEW |
                          LP_NEW_OCCLUSION_QUERY))
      llvmpipe_update_fs(llvmpipe);

   if (llvmpipe->dirty & (LP_NEW_FS |
                          LP_NEW_FRAMEBUFFER |
                          LP_NEW_RASTERIZER |
                          LP_NEW_SAMPLE_MASK |
                          LP_NEW_DEPTH_STENCIL_ALPHA)) {
      bool discard =
         llvmpipe->rasterizer ? llvmpipe->rasterizer->rasterizer_discard : false;
      lp_setup_set_rasterizer_discard(llvmpipe->setup, discard);
   }

   if (llvmpipe->dirty & (LP_NEW_FS |
                          LP_NEW_FRAMEBUFFER |
                          LP_NEW_RASTERIZER))
      llvmpipe_update_setup(llvmpipe);

   if (llvmpipe->dirty & LP_NEW_SAMPLE_MASK)
      lp_setup_set_sample_mask(llvmpipe->setup, llvmpipe->sample_mask);

   if (llvmpipe->dirty & LP_NEW_BLEND_COLOR)
      lp_setup_set_blend_color(llvmpipe->setup,
                               &llvmpipe->blend_color);

   if (llvmpipe->dirty & LP_NEW_SCISSOR)
      lp_setup_set_scissors(llvmpipe->setup, llvmpipe->scissors);

   if (llvmpipe->dirty & LP_NEW_DEPTH_STENCIL_ALPHA) {
      lp_setup_set_alpha_ref_value(llvmpipe->setup,
                                   llvmpipe->depth_stencil->alpha_ref_value);
      lp_setup_set_stencil_ref_values(llvmpipe->setup,
                                      llvmpipe->stencil_ref.ref_value);
   }

   if (llvmpipe->dirty & LP_NEW_FS_CONSTANTS)
      lp_setup_set_fs_constants(llvmpipe->setup,
                                ARRAY_SIZE(llvmpipe->constants[PIPE_SHADER_FRAGMENT]),
                                llvmpipe->constants[PIPE_SHADER_FRAGMENT]);

   if (llvmpipe->dirty & LP_NEW_FS_SSBOS)
      lp_setup_set_fs_ssbos(llvmpipe->setup,
                            ARRAY_SIZE(llvmpipe->ssbos[PIPE_SHADER_FRAGMENT]),
                            llvmpipe->ssbos[PIPE_SHADER_FRAGMENT], llvmpipe->fs_ssbo_write_mask);

   if (llvmpipe->dirty & LP_NEW_FS_IMAGES)
      lp_setup_set_fs_images(llvmpipe->setup,
                             ARRAY_SIZE(llvmpipe->images[PIPE_SHADER_FRAGMENT]),
                             llvmpipe->images[PIPE_SHADER_FRAGMENT]);

   if (llvmpipe->dirty & (LP_NEW_SAMPLER_VIEW))
      lp_setup_set_fragment_sampler_views(llvmpipe->setup,
                                          llvmpipe->num_sampler_views[PIPE_SHADER_FRAGMENT],
                                          llvmpipe->sampler_views[PIPE_SHADER_FRAGMENT]);

   if (llvmpipe->dirty & (LP_NEW_SAMPLER))
      lp_setup_set_fragment_sampler_state(llvmpipe->setup,
                                          llvmpipe->num_samplers[PIPE_SHADER_FRAGMENT],
                                          llvmpipe->samplers[PIPE_SHADER_FRAGMENT]);

   if (llvmpipe->dirty & LP_NEW_VIEWPORT) {
      /*
       * Update setup and fragment's view of the active viewport state.
       *
       * XXX TODO: It is possible to only loop over the active viewports
       *           instead of all viewports (PIPE_MAX_VIEWPORTS).
       */
      lp_setup_set_viewports(llvmpipe->setup,
                             PIPE_MAX_VIEWPORTS,
                             llvmpipe->viewports);
   }

   llvmpipe_task_update_derived(llvmpipe);
   llvmpipe_mesh_update_derived(llvmpipe);

   llvmpipe_update_derived_clear(llvmpipe);

   llvmpipe->dirty = 0;
}
