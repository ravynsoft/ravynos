/**********************************************************
 * Copyright 2008-2022 VMware, Inc.  All rights reserved.
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
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_bitmask.h"

#include "svga_context.h"
#include "svga_hw_reg.h"
#include "svga_cmd.h"
#include "svga_debug.h"
#include "svga_shader.h"
#include "svga_streamout.h"


static void *
svga_create_vs_state(struct pipe_context *pipe,
                     const struct pipe_shader_state *templ)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_vertex_shader *vs;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_CREATEVS);

   vs = (struct svga_vertex_shader *)
            svga_create_shader(pipe, templ, PIPE_SHADER_VERTEX,
                               sizeof(struct svga_vertex_shader));
   if (!vs)
      goto done;

   vs->base.get_dummy_shader = svga_get_compiled_dummy_vertex_shader;

   {
      /* Need to do construct a new template in case we substituted a
       * debug shader.
       */
      struct pipe_shader_state tmp2 = *templ;

      /* shader IR has been converted to tgsi */
      tmp2.type = PIPE_SHADER_IR_TGSI;
      tmp2.tokens = vs->base.tokens;
      vs->draw_shader = draw_create_vertex_shader(svga->swtnl.draw, &tmp2);
   }

done:
   SVGA_STATS_TIME_POP(svga_sws(svga));
   return vs;
}


static void
svga_bind_vs_state(struct pipe_context *pipe, void *shader)
{
   struct svga_vertex_shader *vs = (struct svga_vertex_shader *)shader;
   struct svga_context *svga = svga_context(pipe);

   if (vs == svga->curr.vs)
      return;

   /* If the currently bound vertex shader has a generated geometry shader,
    * then unbind the geometry shader before binding a new vertex shader.
    * We need to unbind the geometry shader here because there is no
    * pipe_shader associated with the generated geometry shader.
    */
   if (svga->curr.vs != NULL && svga->curr.vs->gs != NULL)
      svga->pipe.bind_gs_state(&svga->pipe, NULL);

   svga->curr.vs = vs;
   svga->dirty |= SVGA_NEW_VS;

   /* Check if the shader uses samplers */
   svga_set_curr_shader_use_samplers_flag(svga, PIPE_SHADER_VERTEX,
                                          svga_shader_use_samplers(&vs->base));
}


static void
svga_delete_vs_state(struct pipe_context *pipe, void *shader)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_vertex_shader *vs = (struct svga_vertex_shader *)shader;
   struct svga_vertex_shader *next_vs;
   struct svga_shader_variant *variant, *tmp;

   svga_hwtnl_flush_retry(svga);

   assert(vs->base.parent == NULL);

   while (vs) {
      next_vs = (struct svga_vertex_shader *)vs->base.next;

      /* Check if there is a generated geometry shader to go with this
       * vertex shader. If there is, then delete the geometry shader as well.
       */
      if (vs->gs != NULL) {
         svga->pipe.delete_gs_state(&svga->pipe, vs->gs);
      }

      if (vs->base.stream_output != NULL)
         svga_delete_stream_output(svga, vs->base.stream_output);

      draw_delete_vertex_shader(svga->swtnl.draw, vs->draw_shader);

      for (variant = vs->base.variants; variant; variant = tmp) {
         tmp = variant->next;

         /* Check if deleting currently bound shader */
         if (variant == svga->state.hw_draw.vs) {
            SVGA_RETRY(svga, svga_set_shader(svga, SVGA3D_SHADERTYPE_VS, NULL));
            svga->state.hw_draw.vs = NULL;
         }

         svga_destroy_shader_variant(svga, variant);
      }

      FREE((void *)vs->base.tokens);
      FREE(vs);
      vs = next_vs;
   }
}


void
svga_init_vs_functions(struct svga_context *svga)
{
   svga->pipe.create_vs_state = svga_create_vs_state;
   svga->pipe.bind_vs_state = svga_bind_vs_state;
   svga->pipe.delete_vs_state = svga_delete_vs_state;
}

