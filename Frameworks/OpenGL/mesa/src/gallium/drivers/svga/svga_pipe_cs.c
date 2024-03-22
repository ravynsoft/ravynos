/**********************************************************
 * Copyright 2022-2023 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#include "compiler/nir/nir.h"
#include "compiler/glsl/gl_nir.h"
#include "nir/nir_to_tgsi.h"

#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_bitmask.h"
#include "tgsi/tgsi_parse.h"

#include "svga_context.h"
#include "svga_cmd.h"
#include "svga_debug.h"
#include "svga_shader.h"
#include "svga_streamout.h"
#include "svga_resource_buffer.h"
#include "svga_tgsi.h"

/**
 * Create the compute program.
 */
static void *
svga_create_compute_state(struct pipe_context *pipe,
                          const struct pipe_compute_state *templ)
{
   struct svga_context *svga = svga_context(pipe);

   struct svga_compute_shader *cs = CALLOC_STRUCT(svga_compute_shader);
   nir_shader *nir = (nir_shader *)templ->prog;

   if (!cs)
      return NULL;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_CREATECS);

   assert(templ->ir_type == PIPE_SHADER_IR_NIR);
   /* nir_to_tgsi requires lowered images */
   NIR_PASS_V(nir, gl_nir_lower_images, false);

   cs->base.tokens = nir_to_tgsi((void *)nir, pipe->screen);

   struct svga_shader *shader = &cs->base;
   shader->id = svga->debug.shader_id++;
   shader->type = PIPE_SHADER_IR_TGSI;
   shader->stage = PIPE_SHADER_COMPUTE;

   /* Collect shader basic info */
   svga_tgsi_scan_shader(&cs->base);

   cs->shared_mem_size = templ->static_shared_mem;

   SVGA_STATS_TIME_POP(svga_sws(svga));
   return cs;
}


/**
 * Bind the compute program.
 */
static void
svga_bind_compute_state(struct pipe_context *pipe, void *shader)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_compute_shader *cs = (struct svga_compute_shader *)shader;

   svga->curr.cs = cs;
   svga->dirty |= SVGA_NEW_CS;

   /* Check if the shader uses samplers */
   svga_set_curr_shader_use_samplers_flag(svga, PIPE_SHADER_COMPUTE,
                                          svga_shader_use_samplers(&cs->base));
}


/**
 * Delete the compute program.
 */
static void
svga_delete_compute_state(struct pipe_context *pipe, void *shader)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_compute_shader *cs = (struct svga_compute_shader *)shader;
   struct svga_compute_shader *next_cs;
   struct svga_shader_variant *variant, *tmp;

   svga_hwtnl_flush_retry(svga);

   /* Free the list of compute shaders */
   while (cs) {
      next_cs = (struct svga_compute_shader *)cs->base.next;

      for (variant = cs->base.variants; variant; variant = tmp) {
         tmp = variant->next;

         /* Check if deleting currently bound shader */
         if (variant == svga->state.hw_draw.cs) {
            SVGA_RETRY(svga, svga_set_shader(svga, SVGA3D_SHADERTYPE_CS, NULL));
            svga->state.hw_draw.cs = NULL;
         }

         svga_destroy_shader_variant(svga, variant);
      }

      FREE((void *)cs->base.tokens);
      FREE(cs);
      cs = next_cs;
   }
}


/**
 * Bind an array of shader resources that will be used by the
 * compute program.  Any resources that were previously bound to
 * the specified range will be unbound after this call.
 */
static void
svga_set_compute_resources(struct pipe_context *pipe,
                           unsigned start, unsigned count,
                           struct pipe_surface **resources)
{
   //TODO
   return;
}


/**
 * Bind an array of buffers to be mapped into the address space of
 * the GLOBAL resource.  Any buffers that were previously bound
 * between [first, first + count - 1] are unbound after this call.
 */
static void
svga_set_global_binding(struct pipe_context *pipe,
                        unsigned first, unsigned count,
                        struct pipe_resource **resources,
                        uint32_t **handles)
{
   //TODO
   return;
}


/**
 */
static void
svga_validate_compute_resources(struct svga_context *svga)
{
   /* validate sampler view resources */
   SVGA_RETRY(svga,
              svga_validate_sampler_resources(svga, SVGA_PIPE_COMPUTE));

   /* validate constant buffer resources */
   SVGA_RETRY(svga,
              svga_validate_constant_buffers(svga, SVGA_PIPE_COMPUTE));

   /* validate image view resources */
   SVGA_RETRY(svga,
              svga_validate_image_views(svga, SVGA_PIPE_COMPUTE));

   /* validate shader buffer resources */
   SVGA_RETRY(svga,
              svga_validate_shader_buffers(svga, SVGA_PIPE_COMPUTE));
}


/**
 * Launch the compute kernel starting from instruction pc of the
 * currently bound compute program.
 */
static void
svga_launch_grid(struct pipe_context *pipe,
                 const struct pipe_grid_info *info)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_winsys_context *swc = svga->swc;

   assert(svga_have_gl43(svga));

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_LAUNCHGRID);

   if (info->indirect) {
      svga->curr.grid_info.indirect= info->indirect;
   }

   svga_update_compute_state(svga);

   /* validate compute resources */
   svga_validate_compute_resources(svga);

   if (info->indirect) {
      struct svga_winsys_surface *indirect_surf;
      indirect_surf = svga_buffer_handle(svga, info->indirect,
                                         PIPE_BIND_COMMAND_ARGS_BUFFER);
      SVGA_RETRY(svga, SVGA3D_sm5_DispatchIndirect(swc, indirect_surf,
                                                   info->indirect_offset));
   }
   else {
      svga->curr.grid_info.size[0] = info->grid[0];
      svga->curr.grid_info.size[1] = info->grid[1];
      svga->curr.grid_info.size[2] = info->grid[2];

      SVGA_RETRY(svga, SVGA3D_sm5_Dispatch(swc, info->grid));
   }

   SVGA_STATS_TIME_POP(svga_sws(svga));
   return;
}


/**
 * Initialize the compute interface function pointers.
 */
void
svga_init_cs_functions(struct svga_context *svga)
{
   svga->pipe.create_compute_state = svga_create_compute_state;
   svga->pipe.bind_compute_state = svga_bind_compute_state;
   svga->pipe.delete_compute_state = svga_delete_compute_state;
   svga->pipe.set_compute_resources = svga_set_compute_resources;
   svga->pipe.set_global_binding = svga_set_global_binding;
   svga->pipe.launch_grid = svga_launch_grid;
}
