/*
 * Copyright (C) 2021 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "bi_builder.h"
#include "compiler.h"

/* Bifrost v7 can preload up to two messages of the form:
 *
 * 1. +LD_VAR_IMM, register_format f32/f16, sample mode
 * 2. +VAR_TEX, register format f32/f16, sample mode (TODO)
 *
 * Analyze the shader for these instructions and push accordingly.
 */

static bool
bi_is_regfmt_float(enum bi_register_format regfmt)
{
   return (regfmt == BI_REGISTER_FORMAT_F32) ||
          (regfmt == BI_REGISTER_FORMAT_F16);
}

/*
 * Preloaded varyings are interpolated at the sample location. Check if an
 * instruction can use this interpolation mode.
 */
static bool
bi_can_interp_at_sample(bi_instr *I)
{
   /* .sample mode with r61 corresponds to per-sample interpolation */
   if (I->sample == BI_SAMPLE_SAMPLE)
      return bi_is_value_equiv(I->src[0], bi_register(61));

   /* If the shader runs with pixel-frequency shading, .sample is
    * equivalent to .center, so allow .center
    *
    * If the shader runs with sample-frequency shading, .sample and .center
    * are not equivalent. However, the ESSL 3.20 specification
    * stipulates in section 4.5 ("Interpolation Qualifiers"):
    *
    *    for fragment shader input variables qualified with neither
    *    centroid nor sample, the value of the assigned variable may be
    *    interpolated anywhere within the pixel and a single value may be
    *    assigned to each sample within the pixel, to the extent permitted
    *    by the OpenGL ES Specification.
    *
    * We only produce .center for variables qualified with neither centroid
    * nor sample, so if .center is specified this section applies. This
    * suggests that, although per-pixel interpolation is allowed, it is not
    * mandated ("may" rather than "must" or "should"). Therefore it appears
    * safe to substitute sample.
    */
   return (I->sample == BI_SAMPLE_CENTER);
}

static bool
bi_can_preload_ld_var(bi_instr *I)
{
   return (I->op == BI_OPCODE_LD_VAR_IMM) && bi_can_interp_at_sample(I) &&
          bi_is_regfmt_float(I->register_format);
}

static bool
bi_is_var_tex(enum bi_opcode op)
{
   return (op == BI_OPCODE_VAR_TEX_F32) || (op == BI_OPCODE_VAR_TEX_F16);
}

void
bi_opt_message_preload(bi_context *ctx)
{
   unsigned nr_preload = 0;

   /* We only preload from the first block */
   bi_block *block = bi_start_block(&ctx->blocks);
   bi_builder b = bi_init_builder(ctx, bi_before_nonempty_block(block));

   bi_foreach_instr_in_block_safe(block, I) {
      if (I->nr_dests != 1)
         continue;

      struct bifrost_message_preload msg;

      if (bi_can_preload_ld_var(I)) {
         msg = (struct bifrost_message_preload){
            .enabled = true,
            .varying_index = I->varying_index,
            .fp16 = (I->register_format == BI_REGISTER_FORMAT_F16),
            .num_components = I->vecsize + 1,
         };
      } else if (bi_is_var_tex(I->op)) {
         msg = (struct bifrost_message_preload){
            .enabled = true,
            .texture = true,
            .varying_index = I->varying_index,
            .texture_index = I->texture_index,
            .fp16 = (I->op == BI_OPCODE_VAR_TEX_F16),
            .skip = I->skip,
            .zero_lod = I->lod_mode,
         };
      } else {
         continue;
      }

      /* Report the preloading */
      ctx->info.bifrost->messages[nr_preload] = msg;

      /* Replace with a collect of preloaded registers. The collect
       * kills the moves, so the collect is free (it is coalesced).
       */
      b.cursor = bi_before_instr(I);

      unsigned nr = bi_count_write_registers(I, 0);
      bi_instr *collect = bi_collect_i32_to(&b, I->dest[0], nr);

      /* The registers themselves must be preloaded at the start of
       * the program. Preloaded registers are coalesced, so these
       * moves are free.
       */
      b.cursor = bi_before_block(block);
      bi_foreach_src(collect, i) {
         unsigned reg = (nr_preload * 4) + i;

         collect->src[i] = bi_mov_i32(&b, bi_register(reg));
      }

      bi_remove_instruction(I);

      /* Maximum number of preloaded messages */
      if ((++nr_preload) == 2)
         break;
   }
}
