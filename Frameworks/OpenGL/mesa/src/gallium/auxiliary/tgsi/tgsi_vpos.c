/*
 * Copyright 2018 VMware, Inc.
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
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * TGSI utility transforms the shader to write imm(0, 0, 0, 1) to vertex
 * position
 */

#include "util/u_debug.h"
#include "util/u_math.h"
#include "tgsi_vpos.h"
#include "tgsi_transform.h"
#include "tgsi_dump.h"
#include "pipe/p_state.h"


struct write_vpos_context
{
   struct tgsi_transform_context base;
   unsigned imm_index;
};


static inline struct write_vpos_context *
write_vpos_context(struct tgsi_transform_context *ctx)
{
   return (struct write_vpos_context *) ctx;
}


/**
 * TGSI transform prolog callback.
 */
static void
write_vpos_prolog(struct tgsi_transform_context *ctx)
{
   struct write_vpos_context *vc = write_vpos_context(ctx);
   struct tgsi_full_instruction inst;

   tgsi_transform_immediate_decl(ctx, 0.0, 0.0, 0.0, 1.0);
   tgsi_transform_output_decl(ctx, 0,
                             TGSI_SEMANTIC_POSITION, 0,
                             0);
   inst = tgsi_default_full_instruction();
   inst.Instruction.Opcode = TGSI_OPCODE_MOV;
   inst.Instruction.NumDstRegs = 1;
   tgsi_transform_dst_reg(&inst.Dst[0], TGSI_FILE_OUTPUT,
                          0, TGSI_WRITEMASK_XYZW);
   inst.Instruction.NumSrcRegs = 1;
   tgsi_transform_src_reg(&inst.Src[0], TGSI_FILE_IMMEDIATE,
                          vc->imm_index, TGSI_SWIZZLE_X,
                          TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Z, TGSI_SWIZZLE_W);
   ctx->emit_instruction(ctx, &inst);
}


/**
 * TGSI utility writes imm(0, 0, 0, 1) to vertex position
 */
struct tgsi_token *
tgsi_write_vpos(const struct tgsi_token *tokens_in,
                unsigned num_immediates)
{
   struct write_vpos_context transform;
   const unsigned num_new_tokens = 1000; /* should be enough */
   const unsigned new_len = tgsi_num_tokens(tokens_in) + num_new_tokens;

   /* setup transformation context */
   memset(&transform, 0, sizeof(transform));
   transform.base.prolog = write_vpos_prolog;

   transform.imm_index = num_immediates;

   return tgsi_transform_shader(tokens_in, new_len, &transform.base);
}


