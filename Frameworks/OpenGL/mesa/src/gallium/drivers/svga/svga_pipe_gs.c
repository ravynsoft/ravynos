/**********************************************************
 * Copyright 2014-2022 VMware, Inc.  All rights reserved.
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

#include "draw/draw_context.h"
#include "nir/nir_to_tgsi.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_bitmask.h"

#include "svga_context.h"
#include "svga_cmd.h"
#include "svga_debug.h"
#include "svga_shader.h"
#include "svga_streamout.h"

static void *
svga_create_gs_state(struct pipe_context *pipe,
                     const struct pipe_shader_state *templ)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_geometry_shader *gs;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_CREATEGS);

   gs = (struct svga_geometry_shader *)
            svga_create_shader(pipe, templ, PIPE_SHADER_GEOMETRY,
                               sizeof(struct svga_geometry_shader));

   if (!gs)
      goto done;

   /* Original shader IR could have been deleted if it is converted from
    * NIR to TGSI. So need to explicitly set the shader state type to TGSI
    * before passing it to draw.
    */
   struct pipe_shader_state tmp = *templ;
   tmp.type = PIPE_SHADER_IR_TGSI;
   tmp.tokens = gs->base.tokens;

   gs->base.get_dummy_shader = svga_get_compiled_dummy_geometry_shader;
   gs->draw_shader = draw_create_geometry_shader(svga->swtnl.draw, &tmp);

done:
   SVGA_STATS_TIME_POP(svga_sws(svga));
   return gs;
}


static void
svga_bind_gs_state(struct pipe_context *pipe, void *shader)
{
   struct svga_geometry_shader *gs = (struct svga_geometry_shader *)shader;
   struct svga_context *svga = svga_context(pipe);

   svga->curr.user_gs = gs;
   svga->dirty |= SVGA_NEW_GS;

   /* Check if the shader uses samplers */
   svga_set_curr_shader_use_samplers_flag(svga, PIPE_SHADER_GEOMETRY,
                                          svga_shader_use_samplers(&gs->base));
}


static void
svga_delete_gs_state(struct pipe_context *pipe, void *shader)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_geometry_shader *gs = (struct svga_geometry_shader *)shader;
   struct svga_geometry_shader *next_gs;
   struct svga_shader_variant *variant, *tmp;

   svga_hwtnl_flush_retry(svga);

   /* Start deletion from the original geometry shader state */
   if (gs->base.parent != NULL)
      gs = (struct svga_geometry_shader *)gs->base.parent;

   /* Free the list of geometry shaders */
   while (gs) {
      next_gs = (struct svga_geometry_shader *)gs->base.next;

      if (gs->base.stream_output != NULL)
         svga_delete_stream_output(svga, gs->base.stream_output);

      draw_delete_geometry_shader(svga->swtnl.draw, gs->draw_shader);

      for (variant = gs->base.variants; variant; variant = tmp) {
         tmp = variant->next;

         /* Check if deleting currently bound shader */
         if (variant == svga->state.hw_draw.gs) {
            SVGA_RETRY(svga, svga_set_shader(svga, SVGA3D_SHADERTYPE_GS, NULL));
            svga->state.hw_draw.gs = NULL;
         }

         svga_destroy_shader_variant(svga, variant);
      }

      FREE((void *)gs->base.tokens);
      FREE(gs);
      gs = next_gs;
   }
}


void
svga_init_gs_functions(struct svga_context *svga)
{
   svga->pipe.create_gs_state = svga_create_gs_state;
   svga->pipe.bind_gs_state = svga_bind_gs_state;
   svga->pipe.delete_gs_state = svga_delete_gs_state;
}
