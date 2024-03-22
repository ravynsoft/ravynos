/*
 * Copyright 2023 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */
#include "agx_builder.h"
#include "agx_compiler.h"

/*
 * Not all instructions can take uniforms. Memory instructions can take
 * uniforms, but only for their base (first) source and only in the
 * low-half of the uniform file.
 *
 * This pass lowers invalid uniforms.
 */
static bool
should_lower(enum agx_opcode op, agx_index uniform, unsigned src_index)
{
   if (uniform.type != AGX_INDEX_UNIFORM)
      return false;

   /* Some instructions only seem able to access uniforms in the low half */
   bool high = uniform.value >= 256;

   switch (op) {
   case AGX_OPCODE_IMAGE_LOAD:
   case AGX_OPCODE_TEXTURE_LOAD:
   case AGX_OPCODE_TEXTURE_SAMPLE:
      return src_index != 1 && src_index != 2;
   case AGX_OPCODE_DEVICE_LOAD:
      return src_index != 0 || high;
   case AGX_OPCODE_DEVICE_STORE:
   case AGX_OPCODE_ATOMIC:
      return src_index != 1 || high;
   case AGX_OPCODE_LOCAL_LOAD:
      return src_index != 0;
   case AGX_OPCODE_LOCAL_STORE:
      return src_index != 1;
   case AGX_OPCODE_IMAGE_WRITE:
      return src_index != 3;
   case AGX_OPCODE_ZS_EMIT:
   case AGX_OPCODE_ST_TILE:
   case AGX_OPCODE_LD_TILE:
   case AGX_OPCODE_BLOCK_IMAGE_STORE:
   case AGX_OPCODE_UNIFORM_STORE:
   case AGX_OPCODE_ST_VARY:
   case AGX_OPCODE_LOCAL_ATOMIC:
   case AGX_OPCODE_SAMPLE_MASK:
   case AGX_OPCODE_ITER:
   case AGX_OPCODE_ITERPROJ:
   case AGX_OPCODE_STACK_LOAD:
   case AGX_OPCODE_STACK_STORE:
      return true;
   default:
      return false;
   }
}

void
agx_lower_uniform_sources(agx_context *ctx)
{
   agx_foreach_instr_global_safe(ctx, I) {
      agx_builder b = agx_init_builder(ctx, agx_before_instr(I));

      agx_foreach_src(I, s) {
         if (should_lower(I->op, I->src[s], s))
            I->src[s] = agx_mov(&b, I->src[s]);
      }
   }
}
