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

#define XXH_INLINE_ALL
#include "util/xxhash.h"

/* Fuse pairs of TEXS_2D instructions into a single dual texture TEXC, if both
 * sample at the same coordinate with the default LOD mode for the shader stage
 * (computed LOD in fragment shaders, else zero LOD) and immediate
 * texture/sampler indices 0...3.
 *
 * Fusing across basic block boundaries is not expected to be useful, as it
 * increases register pressure and causes redundant memory traffic. As such, we
 * use a local optimization pass.
 *
 * To pair ops efficiently, we maintain a set (backed by a hash table) using
 * only the coordinate sources for hashing and equality. Hence the pass runs in
 * O(n) worst case expected time for n insturctions in a block. We reject
 * invalid texture instructions quickly to reduce the constant factor.
 *
 * Dual texture instructions have skip flags, like normal texture instructions.
 * Adding a skip flag to an instruction that doesn't have it is illegal, but
 * removing a skip flag from one that has it is legal. Accordingly, set the
 * fused TEXC's skip to the logical AND of the unfused TEXS flags. We run the
 * optimization pass to run after bi_analyze_helper_requirements.
 */

static inline bool
bi_can_fuse_dual_tex(bi_instr *I, bool fuse_zero_lod)
{
   return (I->op == BI_OPCODE_TEXS_2D_F32 || I->op == BI_OPCODE_TEXS_2D_F16) &&
          (I->texture_index < 4 && I->sampler_index < 4) &&
          (I->lod_mode == fuse_zero_lod);
}

static enum bifrost_texture_format
bi_format_for_texs_2d(enum bi_opcode op)
{
   switch (op) {
   case BI_OPCODE_TEXS_2D_F32:
      return BIFROST_TEXTURE_FORMAT_F32;
   case BI_OPCODE_TEXS_2D_F16:
      return BIFROST_TEXTURE_FORMAT_F16;
   default:
      unreachable("Invalid TEXS_2D instruction");
   }
}

static void
bi_fuse_dual(bi_context *ctx, bi_instr *I1, bi_instr *I2)
{
   /* Construct a texture operation descriptor for the dual texture */
   struct bifrost_dual_texture_operation desc = {
      .mode = BIFROST_TEXTURE_OPERATION_DUAL,

      .primary_texture_index = I1->texture_index,
      .primary_sampler_index = I1->sampler_index,
      .primary_format = bi_format_for_texs_2d(I1->op),
      .primary_mask = 0xF,

      .secondary_texture_index = I2->texture_index,
      .secondary_sampler_index = I2->sampler_index,
      .secondary_format = bi_format_for_texs_2d(I2->op),
      .secondary_mask = 0xF,
   };

   /* LOD mode is implied in a shader stage */
   assert(I1->lod_mode == I2->lod_mode);

   /* Insert before the earlier instruction in case its result is consumed
    * before the later instruction
    */
   bi_builder b = bi_init_builder(ctx, bi_before_instr(I1));

   bi_instr *I = bi_texc_dual_to(
      &b, I1->dest[0], I2->dest[0], bi_null(), /* staging */
      I1->src[0], I1->src[1],                  /* coordinates */
      bi_imm_u32(bi_dual_tex_as_u32(desc)), I1->lod_mode,
      bi_count_write_registers(I1, 0), bi_count_write_registers(I2, 0));

   I->skip = I1->skip && I2->skip;

   bi_remove_instruction(I1);
   bi_remove_instruction(I2);
}

#define HASH(hash, data) XXH32(&(data), sizeof(data), hash)

static uint32_t
coord_hash(const void *key)
{
   const bi_instr *I = key;

   return XXH32(&I->src[0], sizeof(I->src[0]) + sizeof(I->src[1]), 0);
}

static bool
coord_equal(const void *key1, const void *key2)
{
   const bi_instr *I = key1;
   const bi_instr *J = key2;

   return memcmp(&I->src[0], &J->src[0],
                 sizeof(I->src[0]) + sizeof(I->src[1])) == 0;
}

static void
bi_opt_fuse_dual_texture_block(bi_context *ctx, bi_block *block)
{
   struct set *set = _mesa_set_create(ctx, coord_hash, coord_equal);
   bool fuse_zero_lod = (ctx->stage != MESA_SHADER_FRAGMENT);
   bool found = false;

   bi_foreach_instr_in_block_safe(block, I) {
      if (!bi_can_fuse_dual_tex(I, fuse_zero_lod))
         continue;

      struct set_entry *ent = _mesa_set_search_or_add(set, I, &found);

      if (found) {
         bi_fuse_dual(ctx, (bi_instr *)ent->key, I);
         _mesa_set_remove(set, ent);
      }
   }
}

void
bi_opt_fuse_dual_texture(bi_context *ctx)
{
   bi_foreach_block(ctx, block) {
      bi_opt_fuse_dual_texture_block(ctx, block);
   }
}
