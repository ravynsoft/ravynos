/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */
#ifndef PIPE_NIR_H
#define PIPE_NIR_H

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "nir.h"
#include "shader_enums.h"

static inline void *
pipe_shader_from_nir(struct pipe_context *pipe, nir_shader *nir)
{
   struct pipe_shader_state state = {
      .type = PIPE_SHADER_IR_NIR,
      .ir.nir = nir,
   };

   switch (nir->info.stage) {
   case MESA_SHADER_VERTEX:
      return pipe->create_vs_state(pipe, &state);
   case MESA_SHADER_TESS_CTRL:
      return pipe->create_tcs_state(pipe, &state);
   case MESA_SHADER_TESS_EVAL:
      return pipe->create_tes_state(pipe, &state);
   case MESA_SHADER_GEOMETRY:
      return pipe->create_gs_state(pipe, &state);
   case MESA_SHADER_FRAGMENT:
      return pipe->create_fs_state(pipe, &state);

   case MESA_SHADER_COMPUTE:
   case MESA_SHADER_KERNEL: {
      struct pipe_compute_state compute = {
         .ir_type = PIPE_SHADER_IR_NIR,
         .prog = nir,
         .static_shared_mem = nir->info.shared_size,
      };

      return pipe->create_compute_state(pipe, &compute);
   }

   default:
      unreachable("unexpected shader stage");
   }
}

#endif
