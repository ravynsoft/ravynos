/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2019 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "r600_sfn.h"

#include "compiler/nir/nir.h"
#include "compiler/shader_enums.h"
#include "sfn/sfn_assembler.h"
#include "sfn/sfn_debug.h"
#include "sfn/sfn_memorypool.h"
#include "sfn/sfn_nir.h"
#include "sfn/sfn_shader.h"
#include "r600_asm.h"
#include "r600_pipe.h"
#include "util/macros.h"
#include "util/ralloc.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>

char *
r600_finalize_nir(pipe_screen *screen, void *shader)
{
   auto rs = container_of(screen, r600_screen, b.b);
   auto nir = static_cast<nir_shader *>(shader);
   r600_finalize_nir_common(nir, rs->b.gfx_level);
   return nullptr;
}

class MallocPoolRelease {
public:
   MallocPoolRelease() { r600::init_pool(); }
   ~MallocPoolRelease() { r600::release_pool(); }
};

int
r600_shader_from_nir(struct r600_context *rctx,
                     struct r600_pipe_shader *pipeshader,
                     r600_shader_key *key)
{

   MallocPoolRelease pool_release;

   struct r600_pipe_shader_selector *sel = pipeshader->selector;

   if (rctx->screen->b.debug_flags & DBG_PREOPT_IR) {
      fprintf(stderr, "PRE-OPT-NIR-----------.------------------------------\n");
      nir_print_shader(sel->nir, stderr);
      fprintf(stderr, "END PRE-OPT-NIR--------------------------------------\n\n");
   }

   auto sh = nir_shader_clone(sel->nir, sel->nir);

   r600_lower_and_optimize_nir(sh, key, rctx->b.gfx_level, &sel->so);

   if (rctx->screen->b.debug_flags & DBG_ALL_SHADERS) {
      fprintf(stderr,
              "-- NIR --------------------------------------------------------\n");
      struct nir_function *func =
         (struct nir_function *)exec_list_get_head(&sh->functions);
      nir_index_ssa_defs(func->impl);
      nir_print_shader(sh, stderr);
      fprintf(stderr,
              "-- END --------------------------------------------------------\n");
   }

   memset(&pipeshader->shader, 0, sizeof(r600_shader));
   pipeshader->scratch_space_needed = sh->scratch_size;

   if (sh->info.stage == MESA_SHADER_TESS_EVAL || sh->info.stage == MESA_SHADER_VERTEX ||
       sh->info.stage == MESA_SHADER_GEOMETRY) {
      pipeshader->shader.clip_dist_write |=
         ((1 << sh->info.clip_distance_array_size) - 1);
      pipeshader->shader.cull_dist_write = ((1 << sh->info.cull_distance_array_size) - 1)
                                           << sh->info.clip_distance_array_size;
      pipeshader->shader.cc_dist_mask =
         (1 << (sh->info.cull_distance_array_size + sh->info.clip_distance_array_size)) -
         1;
   }
   struct r600_shader *gs_shader = nullptr;
   if (rctx->gs_shader)
      gs_shader = &rctx->gs_shader->current->shader;
   r600_screen *rscreen = rctx->screen;

   r600::Shader *shader =
      r600::Shader::translate_from_nir(sh, &sel->so, gs_shader, *key,
                                       rctx->isa->hw_class, rscreen->b.family);

   assert(shader);
   if (!shader)
      return -2;

   pipeshader->enabled_stream_buffers_mask = shader->enabled_stream_buffers_mask();
   pipeshader->selector->info.file_count[TGSI_FILE_HW_ATOMIC] +=
      shader->atomic_file_count();
   pipeshader->selector->info.writes_memory =
      shader->has_flag(r600::Shader::sh_writes_memory);

   r600_finalize_and_optimize_shader(shader);

   auto scheduled_shader = r600_schedule_shader(shader);
   if (!scheduled_shader) {
      return -1;
   }

   scheduled_shader->get_shader_info(&pipeshader->shader);
   pipeshader->shader.uses_doubles = sh->info.bit_sizes_float & 64 ? 1 : 0;

   r600_bytecode_init(&pipeshader->shader.bc,
                      rscreen->b.gfx_level,
                      rscreen->b.family,
                      rscreen->has_compressed_msaa_texturing);

   /* We already schedule the code with this in mind, no need to handle this
    * in the backend assembler */
   pipeshader->shader.bc.ar_handling = AR_HANDLE_NORMAL;
   pipeshader->shader.bc.r6xx_nop_after_rel_dst = 0;

   r600::sfn_log << r600::SfnLog::shader_info << "pipeshader->shader.processor_type = "
                 << pipeshader->shader.processor_type << "\n";

   pipeshader->shader.bc.type = pipeshader->shader.processor_type;
   pipeshader->shader.bc.isa = rctx->isa;
   pipeshader->shader.bc.ngpr = scheduled_shader->required_registers();

   r600::Assembler afs(&pipeshader->shader, *key);
   if (!afs.lower(scheduled_shader)) {
      R600_ERR("%s: Lowering to assembly failed\n", __func__);

      scheduled_shader->print(std::cerr);
      /* For now crash if the shader could not be generated */
      assert(0);
      return -1;
   }

   if (sh->info.stage == MESA_SHADER_VERTEX) {
      pipeshader->shader.vs_position_window_space =
            sh->info.vs.window_space_position;
   }

   if (sh->info.stage == MESA_SHADER_FRAGMENT)
      pipeshader->shader.ps_conservative_z =
            sh->info.fs.depth_layout;

   if (sh->info.stage == MESA_SHADER_GEOMETRY) {
      r600::sfn_log << r600::SfnLog::shader_info
                    << "Geometry shader, create copy shader\n";
      generate_gs_copy_shader(rctx, pipeshader, &sel->so);
      assert(pipeshader->gs_copy_shader);
   } else {
      r600::sfn_log << r600::SfnLog::shader_info << "This is not a Geometry shader\n";
   }
   ralloc_free(sh);

   return 0;
}
