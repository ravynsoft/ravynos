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

#include "nir/nir_to_tgsi.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_bitmask.h"
#include "draw/draw_context.h"

#include "svga_context.h"
#include "svga_hw_reg.h"
#include "svga_cmd.h"
#include "svga_debug.h"
#include "svga_shader.h"


void *
svga_create_fs_state(struct pipe_context *pipe,
                     const struct pipe_shader_state *templ)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_fragment_shader *fs;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_CREATEFS);

   fs = (struct svga_fragment_shader *)
            svga_create_shader(pipe, templ, PIPE_SHADER_FRAGMENT,
                               sizeof(struct svga_fragment_shader));
   if (!fs)
      goto done;

   /* Original shader IR could have been deleted if it is converted from
    * NIR to TGSI. So need to explicitly set the shader state type to TGSI
    * before passing it to draw.
    */
   struct pipe_shader_state tmp = *templ;
   tmp.type = PIPE_SHADER_IR_TGSI;
   tmp.tokens = fs->base.tokens;

   fs->generic_inputs = svga_get_generic_inputs_mask(&fs->base.tgsi_info);

   fs->base.get_dummy_shader = svga_get_compiled_dummy_fragment_shader;

   svga_remap_generics(fs->base.info.generic_inputs_mask,
                       fs->generic_remap_table);

   fs->draw_shader = draw_create_fragment_shader(svga->swtnl.draw, &tmp);

done:
   SVGA_STATS_TIME_POP(svga_sws(svga));
   return fs;
}


void
svga_bind_fs_state(struct pipe_context *pipe, void *shader)
{
   struct svga_fragment_shader *fs = (struct svga_fragment_shader *) shader;
   struct svga_context *svga = svga_context(pipe);

   svga->curr.fs = fs;
   svga->dirty |= SVGA_NEW_FS;

   /* Check if shader uses samplers */
   svga_set_curr_shader_use_samplers_flag(svga, PIPE_SHADER_FRAGMENT,
                                          svga_shader_use_samplers(&fs->base));
}


static void
svga_delete_fs_state(struct pipe_context *pipe, void *shader)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_fragment_shader *fs = (struct svga_fragment_shader *) shader;
   struct svga_fragment_shader *next_fs;
   struct svga_shader_variant *variant, *tmp;

   svga_hwtnl_flush_retry(svga);

   assert(fs->base.parent == NULL);

   while (fs) {
      next_fs = (struct svga_fragment_shader *) fs->base.next;

      draw_delete_fragment_shader(svga->swtnl.draw, fs->draw_shader);

      for (variant = fs->base.variants; variant; variant = tmp) {
         tmp = variant->next;

         /* Check if deleting currently bound shader */
         if (variant == svga->state.hw_draw.fs) {
            SVGA_RETRY(svga, svga_set_shader(svga, SVGA3D_SHADERTYPE_PS, NULL));
            svga->state.hw_draw.fs = NULL;
         }

         svga_destroy_shader_variant(svga, variant);
      }

      FREE((void *)fs->base.tokens);
      FREE(fs);
      fs = next_fs;
   }
}


void
svga_init_fs_functions(struct svga_context *svga)
{
   svga->pipe.create_fs_state = svga_create_fs_state;
   svga->pipe.bind_fs_state = svga_bind_fs_state;
   svga->pipe.delete_fs_state = svga_delete_fs_state;
}
