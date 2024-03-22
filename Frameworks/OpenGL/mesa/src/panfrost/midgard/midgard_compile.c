/*
 * Copyright (C) 2018-2019 Alyssa Rosenzweig <alyssa@rosenzweig.io>
 * Copyright (C) 2019-2020 Collabora, Ltd.
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

#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "compiler/glsl_types.h"
#include "compiler/glsl/glsl_to_nir.h"
#include "compiler/nir/nir_builder.h"
#include "util/half_float.h"
#include "util/list.h"
#include "util/u_debug.h"
#include "util/u_dynarray.h"
#include "util/u_math.h"

#include "compiler.h"
#include "helpers.h"
#include "midgard.h"
#include "midgard_compile.h"
#include "midgard_nir.h"
#include "midgard_ops.h"
#include "midgard_quirks.h"

#include "disassemble.h"

static const struct debug_named_value midgard_debug_options[] = {
   {"shaders", MIDGARD_DBG_SHADERS, "Dump shaders in NIR and MIR"},
   {"shaderdb", MIDGARD_DBG_SHADERDB, "Prints shader-db statistics"},
   {"inorder", MIDGARD_DBG_INORDER, "Disables out-of-order scheduling"},
   {"verbose", MIDGARD_DBG_VERBOSE, "Dump shaders verbosely"},
   {"internal", MIDGARD_DBG_INTERNAL, "Dump internal shaders"},
   DEBUG_NAMED_VALUE_END};

DEBUG_GET_ONCE_FLAGS_OPTION(midgard_debug, "MIDGARD_MESA_DEBUG",
                            midgard_debug_options, 0)

int midgard_debug = 0;

static midgard_block *
create_empty_block(compiler_context *ctx)
{
   midgard_block *blk = rzalloc(ctx, midgard_block);

   blk->base.predecessors =
      _mesa_set_create(blk, _mesa_hash_pointer, _mesa_key_pointer_equal);

   blk->base.name = ctx->block_source_count++;

   return blk;
}

static void
schedule_barrier(compiler_context *ctx)
{
   midgard_block *temp = ctx->after_block;
   ctx->after_block = create_empty_block(ctx);
   ctx->block_count++;
   list_addtail(&ctx->after_block->base.link, &ctx->blocks);
   list_inithead(&ctx->after_block->base.instructions);
   pan_block_add_successor(&ctx->current_block->base, &ctx->after_block->base);
   ctx->current_block = ctx->after_block;
   ctx->after_block = temp;
}

/* Helpers to generate midgard_instruction's using macro magic, since every
 * driver seems to do it that way */

#define EMIT(op, ...) emit_mir_instruction(ctx, v_##op(__VA_ARGS__));

#define M_LOAD_STORE(name, store, T)                                           \
   static midgard_instruction m_##name(unsigned ssa, unsigned address)         \
   {                                                                           \
      midgard_instruction i = {                                                \
         .type = TAG_LOAD_STORE_4,                                             \
         .mask = 0xF,                                                          \
         .dest = ~0,                                                           \
         .src = {~0, ~0, ~0, ~0},                                              \
         .swizzle = SWIZZLE_IDENTITY_4,                                        \
         .op = midgard_op_##name,                                              \
         .load_store =                                                         \
            {                                                                  \
               .signed_offset = address,                                       \
            },                                                                 \
      };                                                                       \
                                                                               \
      if (store) {                                                             \
         i.src[0] = ssa;                                                       \
         i.src_types[0] = T;                                                   \
         i.dest_type = T;                                                      \
      } else {                                                                 \
         i.dest = ssa;                                                         \
         i.dest_type = T;                                                      \
      }                                                                        \
      return i;                                                                \
   }

#define M_LOAD(name, T)  M_LOAD_STORE(name, false, T)
#define M_STORE(name, T) M_LOAD_STORE(name, true, T)

M_LOAD(ld_attr_32, nir_type_uint32);
M_LOAD(ld_vary_32, nir_type_uint32);
M_LOAD(ld_ubo_u8, nir_type_uint32); /* mandatory extension to 32-bit */
M_LOAD(ld_ubo_u16, nir_type_uint32);
M_LOAD(ld_ubo_32, nir_type_uint32);
M_LOAD(ld_ubo_64, nir_type_uint32);
M_LOAD(ld_ubo_128, nir_type_uint32);
M_LOAD(ld_u8, nir_type_uint8);
M_LOAD(ld_u16, nir_type_uint16);
M_LOAD(ld_32, nir_type_uint32);
M_LOAD(ld_64, nir_type_uint32);
M_LOAD(ld_128, nir_type_uint32);
M_STORE(st_u8, nir_type_uint8);
M_STORE(st_u16, nir_type_uint16);
M_STORE(st_32, nir_type_uint32);
M_STORE(st_64, nir_type_uint32);
M_STORE(st_128, nir_type_uint32);
M_LOAD(ld_tilebuffer_raw, nir_type_uint32);
M_LOAD(ld_tilebuffer_16f, nir_type_float16);
M_LOAD(ld_tilebuffer_32f, nir_type_float32);
M_STORE(st_vary_32, nir_type_uint32);
M_LOAD(ld_cubemap_coords, nir_type_uint32);
M_LOAD(ldst_mov, nir_type_uint32);
M_LOAD(ld_image_32f, nir_type_float32);
M_LOAD(ld_image_16f, nir_type_float16);
M_LOAD(ld_image_32u, nir_type_uint32);
M_LOAD(ld_image_32i, nir_type_int32);
M_STORE(st_image_32f, nir_type_float32);
M_STORE(st_image_16f, nir_type_float16);
M_STORE(st_image_32u, nir_type_uint32);
M_STORE(st_image_32i, nir_type_int32);
M_LOAD(lea_image, nir_type_uint64);

#define M_IMAGE(op)                                                            \
   static midgard_instruction op##_image(nir_alu_type type, unsigned val,      \
                                         unsigned address)                     \
   {                                                                           \
      switch (type) {                                                          \
      case nir_type_float32:                                                   \
         return m_##op##_image_32f(val, address);                              \
      case nir_type_float16:                                                   \
         return m_##op##_image_16f(val, address);                              \
      case nir_type_uint32:                                                    \
         return m_##op##_image_32u(val, address);                              \
      case nir_type_int32:                                                     \
         return m_##op##_image_32i(val, address);                              \
      default:                                                                 \
         unreachable("Invalid image type");                                    \
      }                                                                        \
   }

M_IMAGE(ld);
M_IMAGE(st);

static midgard_instruction
v_branch(bool conditional, bool invert)
{
   midgard_instruction ins = {
      .type = TAG_ALU_4,
      .unit = ALU_ENAB_BRANCH,
      .compact_branch = true,
      .branch =
         {
            .conditional = conditional,
            .invert_conditional = invert,
         },
      .dest = ~0,
      .src = {~0, ~0, ~0, ~0},
   };

   return ins;
}

static void
attach_constants(compiler_context *ctx, midgard_instruction *ins,
                 void *constants, int name)
{
   ins->has_constants = true;
   memcpy(&ins->constants, constants, 16);
}

static int
glsl_type_size(const struct glsl_type *type, bool bindless)
{
   return glsl_count_attribute_slots(type, false);
}

static bool
midgard_nir_lower_global_load_instr(nir_builder *b, nir_intrinsic_instr *intr,
                                    void *data)
{
   if (intr->intrinsic != nir_intrinsic_load_global &&
       intr->intrinsic != nir_intrinsic_load_shared)
      return false;

   unsigned compsz = intr->def.bit_size;
   unsigned totalsz = compsz * intr->def.num_components;
   /* 8, 16, 32, 64 and 128 bit loads don't need to be lowered */
   if (util_bitcount(totalsz) < 2 && totalsz <= 128)
      return false;

   b->cursor = nir_before_instr(&intr->instr);

   nir_def *addr = intr->src[0].ssa;

   nir_def *comps[MIR_VEC_COMPONENTS];
   unsigned ncomps = 0;

   while (totalsz) {
      unsigned loadsz = MIN2(1 << (util_last_bit(totalsz) - 1), 128);
      unsigned loadncomps = loadsz / compsz;

      nir_def *load;
      if (intr->intrinsic == nir_intrinsic_load_global) {
         load = nir_load_global(b, addr, compsz / 8, loadncomps, compsz);
      } else {
         assert(intr->intrinsic == nir_intrinsic_load_shared);
         nir_intrinsic_instr *shared_load =
            nir_intrinsic_instr_create(b->shader, nir_intrinsic_load_shared);
         shared_load->num_components = loadncomps;
         shared_load->src[0] = nir_src_for_ssa(addr);
         nir_intrinsic_set_align(shared_load, compsz / 8, 0);
         nir_intrinsic_set_base(shared_load, nir_intrinsic_base(intr));
         nir_def_init(&shared_load->instr, &shared_load->def,
                      shared_load->num_components, compsz);
         nir_builder_instr_insert(b, &shared_load->instr);
         load = &shared_load->def;
      }

      for (unsigned i = 0; i < loadncomps; i++)
         comps[ncomps++] = nir_channel(b, load, i);

      totalsz -= loadsz;
      addr = nir_iadd_imm(b, addr, loadsz / 8);
   }

   assert(ncomps == intr->def.num_components);
   nir_def_rewrite_uses(&intr->def, nir_vec(b, comps, ncomps));

   return true;
}

static bool
midgard_nir_lower_global_load(nir_shader *shader)
{
   return nir_shader_intrinsics_pass(
      shader, midgard_nir_lower_global_load_instr,
      nir_metadata_block_index | nir_metadata_dominance, NULL);
}

static bool
mdg_should_scalarize(const nir_instr *instr, const void *_unused)
{
   const nir_alu_instr *alu = nir_instr_as_alu(instr);

   if (nir_src_bit_size(alu->src[0].src) == 64)
      return true;

   if (alu->def.bit_size == 64)
      return true;

   switch (alu->op) {
   case nir_op_fdot2:
   case nir_op_umul_high:
   case nir_op_imul_high:
   case nir_op_pack_half_2x16:
   case nir_op_unpack_half_2x16:

   /* The LUT unit is scalar */
   case nir_op_fsqrt:
   case nir_op_frcp:
   case nir_op_frsq:
   case nir_op_fsin_mdg:
   case nir_op_fcos_mdg:
   case nir_op_fexp2:
   case nir_op_flog2:
      return true;
   default:
      return false;
   }
}

/* Only vectorize int64 up to vec2 */
static uint8_t
midgard_vectorize_filter(const nir_instr *instr, const void *data)
{
   if (instr->type != nir_instr_type_alu)
      return 0;

   const nir_alu_instr *alu = nir_instr_as_alu(instr);
   int src_bit_size = nir_src_bit_size(alu->src[0].src);
   int dst_bit_size = alu->def.bit_size;

   if (src_bit_size == 64 || dst_bit_size == 64)
      return 2;

   return 4;
}

void
midgard_preprocess_nir(nir_shader *nir, unsigned gpu_id)
{
   unsigned quirks = midgard_get_quirks(gpu_id);

   /* Lower gl_Position pre-optimisation, but after lowering vars to ssa
    * (so we don't accidentally duplicate the epilogue since mesa/st has
    * messed with our I/O quite a bit already).
    */
   NIR_PASS_V(nir, nir_lower_vars_to_ssa);

   if (nir->info.stage == MESA_SHADER_VERTEX) {
      NIR_PASS_V(nir, nir_lower_viewport_transform);
      NIR_PASS_V(nir, nir_lower_point_size, 1.0, 0.0);
   }

   NIR_PASS_V(nir, nir_lower_var_copies);
   NIR_PASS_V(nir, nir_lower_vars_to_ssa);
   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_lower_var_copies);
   NIR_PASS_V(nir, nir_lower_global_vars_to_local);
   NIR_PASS_V(nir, nir_lower_var_copies);
   NIR_PASS_V(nir, nir_lower_vars_to_ssa);

   NIR_PASS_V(nir, nir_lower_io, nir_var_shader_in | nir_var_shader_out,
              glsl_type_size, 0);

   if (nir->info.stage == MESA_SHADER_VERTEX) {
      /* nir_lower[_explicit]_io is lazy and emits mul+add chains even
       * for offsets it could figure out are constant.  Do some
       * constant folding before pan_nir_lower_store_component below.
       */
      NIR_PASS_V(nir, nir_opt_constant_folding);
      NIR_PASS_V(nir, pan_nir_lower_store_component);
   }

   NIR_PASS_V(nir, nir_lower_ssbo);
   NIR_PASS_V(nir, pan_nir_lower_zs_store);

   NIR_PASS_V(nir, nir_lower_frexp);
   NIR_PASS_V(nir, midgard_nir_lower_global_load);

   nir_lower_idiv_options idiv_options = {
      .allow_fp16 = true,
   };

   NIR_PASS_V(nir, nir_lower_idiv, &idiv_options);

   nir_lower_tex_options lower_tex_options = {
      .lower_txs_lod = true,
      .lower_txp = ~0,
      .lower_tg4_broadcom_swizzle = true,
      .lower_txd = true,
      .lower_invalid_implicit_lod = true,
   };

   NIR_PASS_V(nir, nir_lower_tex, &lower_tex_options);
   NIR_PASS_V(nir, nir_lower_image_atomics_to_global);

   /* TEX_GRAD fails to apply sampler descriptor settings on some
    * implementations, requiring a lowering.
    */
   if (quirks & MIDGARD_BROKEN_LOD)
      NIR_PASS_V(nir, midgard_nir_lod_errata);

   /* Midgard image ops coordinates are 16-bit instead of 32-bit */
   NIR_PASS_V(nir, midgard_nir_lower_image_bitsize);

   if (nir->info.stage == MESA_SHADER_FRAGMENT)
      NIR_PASS_V(nir, nir_lower_helper_writes, true);

   NIR_PASS_V(nir, pan_lower_helper_invocation);
   NIR_PASS_V(nir, pan_lower_sample_pos);
   NIR_PASS_V(nir, midgard_nir_lower_algebraic_early);
   NIR_PASS_V(nir, nir_lower_alu_to_scalar, mdg_should_scalarize, NULL);
   NIR_PASS_V(nir, nir_lower_flrp, 16 | 32 | 64, false /* always_precise */);
   NIR_PASS_V(nir, nir_lower_var_copies);
}

static void
optimise_nir(nir_shader *nir, unsigned quirks, bool is_blend)
{
   bool progress;

   do {
      progress = false;

      NIR_PASS(progress, nir, nir_lower_vars_to_ssa);

      NIR_PASS(progress, nir, nir_copy_prop);
      NIR_PASS(progress, nir, nir_opt_remove_phis);
      NIR_PASS(progress, nir, nir_opt_dce);
      NIR_PASS(progress, nir, nir_opt_dead_cf);
      NIR_PASS(progress, nir, nir_opt_cse);
      NIR_PASS(progress, nir, nir_opt_peephole_select, 64, false, true);
      NIR_PASS(progress, nir, nir_opt_algebraic);
      NIR_PASS(progress, nir, nir_opt_constant_folding);
      NIR_PASS(progress, nir, nir_opt_undef);
      NIR_PASS(progress, nir, nir_lower_undef_to_zero);

      NIR_PASS(progress, nir, nir_opt_loop_unroll);

      NIR_PASS(progress, nir, nir_opt_vectorize, midgard_vectorize_filter,
               NULL);
   } while (progress);

   NIR_PASS_V(nir, nir_lower_alu_to_scalar, mdg_should_scalarize, NULL);

   /* Run after opts so it can hit more */
   if (!is_blend)
      NIR_PASS(progress, nir, nir_fuse_io_16);

   do {
      progress = false;

      NIR_PASS(progress, nir, nir_opt_dce);
      NIR_PASS(progress, nir, nir_opt_algebraic);
      NIR_PASS(progress, nir, nir_opt_constant_folding);
      NIR_PASS(progress, nir, nir_copy_prop);
   } while (progress);

   NIR_PASS(progress, nir, nir_opt_algebraic_late);
   NIR_PASS(progress, nir, nir_opt_algebraic_distribute_src_mods);

   /* We implement booleans as 32-bit 0/~0 */
   NIR_PASS(progress, nir, nir_lower_bool_to_int32);

   /* Now that booleans are lowered, we can run out late opts */
   NIR_PASS(progress, nir, midgard_nir_lower_algebraic_late);
   NIR_PASS(progress, nir, midgard_nir_cancel_inot);
   NIR_PASS_V(nir, midgard_nir_type_csel);

   /* Clean up after late opts */
   do {
      progress = false;

      NIR_PASS(progress, nir, nir_opt_dce);
      NIR_PASS(progress, nir, nir_opt_constant_folding);
      NIR_PASS(progress, nir, nir_copy_prop);
   } while (progress);

   /* Backend scheduler is purely local, so do some global optimizations
    * to reduce register pressure. */
   nir_move_options move_all = nir_move_const_undef | nir_move_load_ubo |
                               nir_move_load_input | nir_move_comparisons |
                               nir_move_copies | nir_move_load_ssbo;

   NIR_PASS_V(nir, nir_opt_sink, move_all);
   NIR_PASS_V(nir, nir_opt_move, move_all);

   /* Take us out of SSA */
   NIR_PASS(progress, nir, nir_convert_from_ssa, true);

   /* We are a vector architecture; write combine where possible */
   NIR_PASS(progress, nir, nir_move_vec_src_uses_to_dest, false);
   NIR_PASS(progress, nir, nir_lower_vec_to_regs, NULL, NULL);

   NIR_PASS(progress, nir, nir_opt_dce);
   NIR_PASS_V(nir, nir_trivialize_registers);
}

/* Do not actually emit a load; instead, cache the constant for inlining */

static void
emit_load_const(compiler_context *ctx, nir_load_const_instr *instr)
{
   nir_def def = instr->def;

   midgard_constants *consts = rzalloc(ctx, midgard_constants);

   assert(instr->def.num_components * instr->def.bit_size <=
          sizeof(*consts) * 8);

#define RAW_CONST_COPY(bits)                                                   \
   nir_const_value_to_array(consts->u##bits, instr->value,                     \
                            instr->def.num_components, u##bits)

   switch (instr->def.bit_size) {
   case 64:
      RAW_CONST_COPY(64);
      break;
   case 32:
      RAW_CONST_COPY(32);
      break;
   case 16:
      RAW_CONST_COPY(16);
      break;
   case 8:
      RAW_CONST_COPY(8);
      break;
   default:
      unreachable("Invalid bit_size for load_const instruction\n");
   }

   /* Shifted for SSA, +1 for off-by-one */
   _mesa_hash_table_u64_insert(ctx->ssa_constants, (def.index << 1) + 1,
                               consts);
}

/* Normally constants are embedded implicitly, but for I/O and such we have to
 * explicitly emit a move with the constant source */

static void
emit_explicit_constant(compiler_context *ctx, unsigned node)
{
   void *constant_value =
      _mesa_hash_table_u64_search(ctx->ssa_constants, node + 1);

   if (constant_value) {
      midgard_instruction ins =
         v_mov(SSA_FIXED_REGISTER(REGISTER_CONSTANT), node);
      attach_constants(ctx, &ins, constant_value, node + 1);
      emit_mir_instruction(ctx, ins);
   }
}

static bool
nir_is_non_scalar_swizzle(nir_alu_src *src, unsigned nr_components)
{
   unsigned comp = src->swizzle[0];

   for (unsigned c = 1; c < nr_components; ++c) {
      if (src->swizzle[c] != comp)
         return true;
   }

   return false;
}

#define ALU_CASE(nir, _op)                                                     \
   case nir_op_##nir:                                                          \
      op = midgard_alu_op_##_op;                                               \
      assert(src_bitsize == dst_bitsize);                                      \
      break;

#define ALU_CASE_RTZ(nir, _op)                                                 \
   case nir_op_##nir:                                                          \
      op = midgard_alu_op_##_op;                                               \
      roundmode = MIDGARD_RTZ;                                                 \
      break;

#define ALU_CHECK_CMP()                                                        \
   assert(src_bitsize == 16 || src_bitsize == 32 || src_bitsize == 64);        \
   assert(dst_bitsize == 16 || dst_bitsize == 32);

#define ALU_CASE_BCAST(nir, _op, count)                                        \
   case nir_op_##nir:                                                          \
      op = midgard_alu_op_##_op;                                               \
      broadcast_swizzle = count;                                               \
      ALU_CHECK_CMP();                                                         \
      break;

#define ALU_CASE_CMP(nir, _op)                                                 \
   case nir_op_##nir:                                                          \
      op = midgard_alu_op_##_op;                                               \
      ALU_CHECK_CMP();                                                         \
      break;

static void
mir_copy_src(midgard_instruction *ins, nir_alu_instr *instr, unsigned i,
             unsigned to, bool is_int, unsigned bcast_count)
{
   nir_alu_src src = instr->src[i];
   unsigned bits = nir_src_bit_size(src.src);

   ins->src[to] = nir_src_index(NULL, &src.src);
   ins->src_types[to] = nir_op_infos[instr->op].input_types[i] | bits;

   /* Figure out which component we should fill unused channels with. This
    * doesn't matter too much in the non-broadcast case, but it makes
    * should that scalar sources are packed with replicated swizzles,
    * which works around issues seen with the combination of source
    * expansion and destination shrinking.
    */
   unsigned replicate_c = 0;
   if (bcast_count) {
      replicate_c = bcast_count - 1;
   } else {
      for (unsigned c = 0; c < NIR_MAX_VEC_COMPONENTS; ++c) {
         if (nir_alu_instr_channel_used(instr, i, c))
            replicate_c = c;
      }
   }

   for (unsigned c = 0; c < NIR_MAX_VEC_COMPONENTS; ++c) {
      ins->swizzle[to][c] =
         src.swizzle[((!bcast_count || c < bcast_count) &&
                      nir_alu_instr_channel_used(instr, i, c))
                        ? c
                        : replicate_c];
   }
}

static void
emit_alu(compiler_context *ctx, nir_alu_instr *instr)
{
   /* Derivatives end up emitted on the texture pipe, not the ALUs. This
    * is handled elsewhere */

   if (instr->op == nir_op_fddx || instr->op == nir_op_fddy) {
      midgard_emit_derivatives(ctx, instr);
      return;
   }

   unsigned nr_components = instr->def.num_components;
   unsigned nr_inputs = nir_op_infos[instr->op].num_inputs;
   unsigned op = 0;

   /* Number of components valid to check for the instruction (the rest
    * will be forced to the last), or 0 to use as-is. Relevant as
    * ball-type instructions have a channel count in NIR but are all vec4
    * in Midgard */

   unsigned broadcast_swizzle = 0;

   /* Should we swap arguments? */
   bool flip_src12 = false;

   ASSERTED unsigned src_bitsize = nir_src_bit_size(instr->src[0].src);
   unsigned dst_bitsize = instr->def.bit_size;

   enum midgard_roundmode roundmode = MIDGARD_RTE;

   switch (instr->op) {
      ALU_CASE(fadd, fadd);
      ALU_CASE(fmul, fmul);
      ALU_CASE(fmin, fmin);
      ALU_CASE(fmax, fmax);
      ALU_CASE(imin, imin);
      ALU_CASE(imax, imax);
      ALU_CASE(umin, umin);
      ALU_CASE(umax, umax);
      ALU_CASE(ffloor, ffloor);
      ALU_CASE(fround_even, froundeven);
      ALU_CASE(ftrunc, ftrunc);
      ALU_CASE(fceil, fceil);
      ALU_CASE(fdot3, fdot3);
      ALU_CASE(fdot4, fdot4);
      ALU_CASE(iadd, iadd);
      ALU_CASE(isub, isub);
      ALU_CASE(iadd_sat, iaddsat);
      ALU_CASE(isub_sat, isubsat);
      ALU_CASE(uadd_sat, uaddsat);
      ALU_CASE(usub_sat, usubsat);
      ALU_CASE(imul, imul);
      ALU_CASE(imul_high, imul);
      ALU_CASE(umul_high, imul);
      ALU_CASE(uclz, iclz);

      /* Zero shoved as second-arg */
      ALU_CASE(iabs, iabsdiff);

      ALU_CASE(uabs_isub, iabsdiff);
      ALU_CASE(uabs_usub, uabsdiff);

      ALU_CASE(mov, imov);

      ALU_CASE_CMP(feq32, feq);
      ALU_CASE_CMP(fneu32, fne);
      ALU_CASE_CMP(flt32, flt);
      ALU_CASE_CMP(ieq32, ieq);
      ALU_CASE_CMP(ine32, ine);
      ALU_CASE_CMP(ilt32, ilt);
      ALU_CASE_CMP(ult32, ult);

      /* We don't have a native b2f32 instruction. Instead, like many
       * GPUs, we exploit booleans as 0/~0 for false/true, and
       * correspondingly AND
       * by 1.0 to do the type conversion. For the moment, prime us
       * to emit:
       *
       * iand [whatever], #0
       *
       * At the end of emit_alu (as MIR), we'll fix-up the constant
       */

      ALU_CASE_CMP(b2f32, iand);
      ALU_CASE_CMP(b2f16, iand);
      ALU_CASE_CMP(b2i32, iand);

      ALU_CASE(frcp, frcp);
      ALU_CASE(frsq, frsqrt);
      ALU_CASE(fsqrt, fsqrt);
      ALU_CASE(fexp2, fexp2);
      ALU_CASE(flog2, flog2);

      ALU_CASE_RTZ(f2i64, f2i_rte);
      ALU_CASE_RTZ(f2u64, f2u_rte);
      ALU_CASE_RTZ(i2f64, i2f_rte);
      ALU_CASE_RTZ(u2f64, u2f_rte);

      ALU_CASE_RTZ(f2i32, f2i_rte);
      ALU_CASE_RTZ(f2u32, f2u_rte);
      ALU_CASE_RTZ(i2f32, i2f_rte);
      ALU_CASE_RTZ(u2f32, u2f_rte);

      ALU_CASE_RTZ(f2i8, f2i_rte);
      ALU_CASE_RTZ(f2u8, f2u_rte);

      ALU_CASE_RTZ(f2i16, f2i_rte);
      ALU_CASE_RTZ(f2u16, f2u_rte);
      ALU_CASE_RTZ(i2f16, i2f_rte);
      ALU_CASE_RTZ(u2f16, u2f_rte);

      ALU_CASE(fsin_mdg, fsinpi);
      ALU_CASE(fcos_mdg, fcospi);

      /* We'll get 0 in the second arg, so:
       * ~a = ~(a | 0) = nor(a, 0) */
      ALU_CASE(inot, inor);
      ALU_CASE(iand, iand);
      ALU_CASE(ior, ior);
      ALU_CASE(ixor, ixor);
      ALU_CASE(ishl, ishl);
      ALU_CASE(ishr, iasr);
      ALU_CASE(ushr, ilsr);

      ALU_CASE_BCAST(b32all_fequal2, fball_eq, 2);
      ALU_CASE_BCAST(b32all_fequal3, fball_eq, 3);
      ALU_CASE_CMP(b32all_fequal4, fball_eq);

      ALU_CASE_BCAST(b32any_fnequal2, fbany_neq, 2);
      ALU_CASE_BCAST(b32any_fnequal3, fbany_neq, 3);
      ALU_CASE_CMP(b32any_fnequal4, fbany_neq);

      ALU_CASE_BCAST(b32all_iequal2, iball_eq, 2);
      ALU_CASE_BCAST(b32all_iequal3, iball_eq, 3);
      ALU_CASE_CMP(b32all_iequal4, iball_eq);

      ALU_CASE_BCAST(b32any_inequal2, ibany_neq, 2);
      ALU_CASE_BCAST(b32any_inequal3, ibany_neq, 3);
      ALU_CASE_CMP(b32any_inequal4, ibany_neq);

      /* Source mods will be shoved in later */
      ALU_CASE(fabs, fmov);
      ALU_CASE(fneg, fmov);
      ALU_CASE(fsat, fmov);
      ALU_CASE(fsat_signed_mali, fmov);
      ALU_CASE(fclamp_pos_mali, fmov);

      /* For size conversion, we use a move. Ideally though we would squash
       * these ops together; maybe that has to happen after in NIR as part of
       * propagation...? An earlier algebraic pass ensured we step down by
       * only / exactly one size. If stepping down, we use a dest override to
       * reduce the size; if stepping up, we use a larger-sized move with a
       * half source and a sign/zero-extension modifier */

   case nir_op_i2i8:
   case nir_op_i2i16:
   case nir_op_i2i32:
   case nir_op_i2i64:
   case nir_op_u2u8:
   case nir_op_u2u16:
   case nir_op_u2u32:
   case nir_op_u2u64:
   case nir_op_f2f16:
   case nir_op_f2f32:
   case nir_op_f2f64: {
      if (instr->op == nir_op_f2f16 || instr->op == nir_op_f2f32 ||
          instr->op == nir_op_f2f64)
         op = midgard_alu_op_fmov;
      else
         op = midgard_alu_op_imov;

      break;
   }

      /* For greater-or-equal, we lower to less-or-equal and flip the
       * arguments */

   case nir_op_fge:
   case nir_op_fge32:
   case nir_op_ige32:
   case nir_op_uge32: {
      op = instr->op == nir_op_fge     ? midgard_alu_op_fle
           : instr->op == nir_op_fge32 ? midgard_alu_op_fle
           : instr->op == nir_op_ige32 ? midgard_alu_op_ile
           : instr->op == nir_op_uge32 ? midgard_alu_op_ule
                                       : 0;

      flip_src12 = true;
      ALU_CHECK_CMP();
      break;
   }

   case nir_op_b32csel:
   case nir_op_b32fcsel_mdg: {
      bool mixed = nir_is_non_scalar_swizzle(&instr->src[0], nr_components);
      bool is_float = instr->op == nir_op_b32fcsel_mdg;
      op = is_float ? (mixed ? midgard_alu_op_fcsel_v : midgard_alu_op_fcsel)
                    : (mixed ? midgard_alu_op_icsel_v : midgard_alu_op_icsel);

      int index = nir_src_index(ctx, &instr->src[0].src);
      emit_explicit_constant(ctx, index);

      break;
   }

   case nir_op_unpack_32_2x16:
   case nir_op_unpack_32_4x8:
   case nir_op_pack_32_2x16:
   case nir_op_pack_32_4x8: {
      op = midgard_alu_op_imov;
      break;
   }

   default:
      mesa_loge("Unhandled ALU op %s\n", nir_op_infos[instr->op].name);
      assert(0);
      return;
   }

   /* Promote imov to fmov if it might help inline a constant */
   if (op == midgard_alu_op_imov && nir_src_is_const(instr->src[0].src) &&
       nir_src_bit_size(instr->src[0].src) == 32 &&
       nir_is_same_comp_swizzle(instr->src[0].swizzle,
                                nir_src_num_components(instr->src[0].src))) {
      op = midgard_alu_op_fmov;
   }

   /* Midgard can perform certain modifiers on output of an ALU op */

   unsigned outmod = 0;
   bool is_int = midgard_is_integer_op(op);

   if (instr->op == nir_op_umul_high || instr->op == nir_op_imul_high) {
      outmod = midgard_outmod_keephi;
   } else if (midgard_is_integer_out_op(op)) {
      outmod = midgard_outmod_keeplo;
   } else if (instr->op == nir_op_fsat) {
      outmod = midgard_outmod_clamp_0_1;
   } else if (instr->op == nir_op_fsat_signed_mali) {
      outmod = midgard_outmod_clamp_m1_1;
   } else if (instr->op == nir_op_fclamp_pos_mali) {
      outmod = midgard_outmod_clamp_0_inf;
   }

   /* Fetch unit, quirks, etc information */
   unsigned opcode_props = alu_opcode_props[op].props;
   bool quirk_flipped_r24 = opcode_props & QUIRK_FLIPPED_R24;

   midgard_instruction ins = {
      .type = TAG_ALU_4,
      .dest_type = nir_op_infos[instr->op].output_type | dst_bitsize,
      .roundmode = roundmode,
   };

   ins.dest = nir_def_index_with_mask(&instr->def, &ins.mask);

   for (unsigned i = nr_inputs; i < ARRAY_SIZE(ins.src); ++i)
      ins.src[i] = ~0;

   if (quirk_flipped_r24) {
      ins.src[0] = ~0;
      mir_copy_src(&ins, instr, 0, 1, is_int, broadcast_swizzle);
   } else {
      for (unsigned i = 0; i < nr_inputs; ++i) {
         unsigned to = i;

         if (instr->op == nir_op_b32csel || instr->op == nir_op_b32fcsel_mdg) {
            /* The condition is the first argument; move
             * the other arguments up one to be a binary
             * instruction for Midgard with the condition
             * last */

            if (i == 0)
               to = 2;
            else if (flip_src12)
               to = 2 - i;
            else
               to = i - 1;
         } else if (flip_src12) {
            to = 1 - to;
         }

         mir_copy_src(&ins, instr, i, to, is_int, broadcast_swizzle);
      }
   }

   if (instr->op == nir_op_fneg || instr->op == nir_op_fabs) {
      /* Lowered to move */
      if (instr->op == nir_op_fneg)
         ins.src_neg[1] ^= true;

      if (instr->op == nir_op_fabs)
         ins.src_abs[1] = true;
   }

   ins.op = op;
   ins.outmod = outmod;

   /* Late fixup for emulated instructions */

   if (instr->op == nir_op_b2f32 || instr->op == nir_op_b2i32) {
      /* Presently, our second argument is an inline #0 constant.
       * Switch over to an embedded 1.0 constant (that can't fit
       * inline, since we're 32-bit, not 16-bit like the inline
       * constants) */

      ins.has_inline_constant = false;
      ins.src[1] = SSA_FIXED_REGISTER(REGISTER_CONSTANT);
      ins.src_types[1] = nir_type_float32;
      ins.has_constants = true;

      if (instr->op == nir_op_b2f32)
         ins.constants.f32[0] = 1.0f;
      else
         ins.constants.i32[0] = 1;

      for (unsigned c = 0; c < 16; ++c)
         ins.swizzle[1][c] = 0;
   } else if (instr->op == nir_op_b2f16) {
      ins.src[1] = SSA_FIXED_REGISTER(REGISTER_CONSTANT);
      ins.src_types[1] = nir_type_float16;
      ins.has_constants = true;
      ins.constants.i16[0] = _mesa_float_to_half(1.0);

      for (unsigned c = 0; c < 16; ++c)
         ins.swizzle[1][c] = 0;
   } else if (nr_inputs == 1 && !quirk_flipped_r24) {
      /* Lots of instructions need a 0 plonked in */
      ins.has_inline_constant = false;
      ins.src[1] = SSA_FIXED_REGISTER(REGISTER_CONSTANT);
      ins.src_types[1] = ins.src_types[0];
      ins.has_constants = true;
      ins.constants.u32[0] = 0;

      for (unsigned c = 0; c < 16; ++c)
         ins.swizzle[1][c] = 0;
   } else if (instr->op == nir_op_pack_32_2x16) {
      ins.dest_type = nir_type_uint16;
      ins.mask = mask_of(nr_components * 2);
      ins.is_pack = true;
   } else if (instr->op == nir_op_pack_32_4x8) {
      ins.dest_type = nir_type_uint8;
      ins.mask = mask_of(nr_components * 4);
      ins.is_pack = true;
   } else if (instr->op == nir_op_unpack_32_2x16) {
      ins.dest_type = nir_type_uint32;
      ins.mask = mask_of(nr_components >> 1);
      ins.is_pack = true;
   } else if (instr->op == nir_op_unpack_32_4x8) {
      ins.dest_type = nir_type_uint32;
      ins.mask = mask_of(nr_components >> 2);
      ins.is_pack = true;
   }

   emit_mir_instruction(ctx, ins);
}

#undef ALU_CASE

static void
mir_set_intr_mask(nir_instr *instr, midgard_instruction *ins, bool is_read)
{
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   unsigned nir_mask = 0;
   unsigned dsize = 0;

   if (is_read) {
      nir_mask = mask_of(nir_intrinsic_dest_components(intr));

      /* Extension is mandatory for 8/16-bit loads */
      dsize = intr->def.bit_size == 64 ? 64 : 32;
   } else {
      nir_mask = nir_intrinsic_write_mask(intr);
      dsize = OP_IS_COMMON_STORE(ins->op) ? nir_src_bit_size(intr->src[0]) : 32;
   }

   /* Once we have the NIR mask, we need to normalize to work in 32-bit space */
   unsigned bytemask = pan_to_bytemask(dsize, nir_mask);
   ins->dest_type = nir_type_uint | dsize;
   mir_set_bytemask(ins, bytemask);
}

/* Uniforms and UBOs use a shared code path, as uniforms are just (slightly
 * optimized) versions of UBO #0 */

static midgard_instruction *
emit_ubo_read(compiler_context *ctx, nir_instr *instr, unsigned dest,
              unsigned offset, nir_src *indirect_offset,
              unsigned indirect_shift, unsigned index, unsigned nr_comps)
{
   midgard_instruction ins;

   unsigned dest_size = (instr->type == nir_instr_type_intrinsic)
                           ? nir_instr_as_intrinsic(instr)->def.bit_size
                           : 32;

   unsigned bitsize = dest_size * nr_comps;

   /* Pick the smallest intrinsic to avoid out-of-bounds reads */
   if (bitsize <= 8)
      ins = m_ld_ubo_u8(dest, 0);
   else if (bitsize <= 16)
      ins = m_ld_ubo_u16(dest, 0);
   else if (bitsize <= 32)
      ins = m_ld_ubo_32(dest, 0);
   else if (bitsize <= 64)
      ins = m_ld_ubo_64(dest, 0);
   else if (bitsize <= 128)
      ins = m_ld_ubo_128(dest, 0);
   else
      unreachable("Invalid UBO read size");

   ins.constants.u32[0] = offset;

   if (instr->type == nir_instr_type_intrinsic)
      mir_set_intr_mask(instr, &ins, true);

   if (indirect_offset) {
      ins.src[2] = nir_src_index(ctx, indirect_offset);
      ins.src_types[2] = nir_type_uint32;
      ins.load_store.index_shift = indirect_shift;

      /* X component for the whole swizzle to prevent register
       * pressure from ballooning from the extra components */
      for (unsigned i = 0; i < ARRAY_SIZE(ins.swizzle[2]); ++i)
         ins.swizzle[2][i] = 0;
   } else {
      ins.load_store.index_reg = REGISTER_LDST_ZERO;
   }

   if (indirect_offset && !indirect_shift)
      mir_set_ubo_offset(&ins, indirect_offset, offset);

   midgard_pack_ubo_index_imm(&ins.load_store, index);

   return emit_mir_instruction(ctx, ins);
}

/* Globals are like UBOs if you squint. And shared memory is like globals if
 * you squint even harder */

static void
emit_global(compiler_context *ctx, nir_instr *instr, bool is_read,
            unsigned srcdest, nir_src *offset, unsigned seg)
{
   midgard_instruction ins;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (is_read) {
      unsigned bitsize = intr->def.bit_size * intr->def.num_components;

      switch (bitsize) {
      case 8:
         ins = m_ld_u8(srcdest, 0);
         break;
      case 16:
         ins = m_ld_u16(srcdest, 0);
         break;
      case 32:
         ins = m_ld_32(srcdest, 0);
         break;
      case 64:
         ins = m_ld_64(srcdest, 0);
         break;
      case 128:
         ins = m_ld_128(srcdest, 0);
         break;
      default:
         unreachable("Invalid global read size");
      }

      mir_set_intr_mask(instr, &ins, is_read);

      /* For anything not aligned on 32bit, make sure we write full
       * 32 bits registers. */
      if (bitsize & 31) {
         unsigned comps_per_32b = 32 / intr->def.bit_size;

         for (unsigned c = 0; c < 4 * comps_per_32b; c += comps_per_32b) {
            if (!(ins.mask & BITFIELD_RANGE(c, comps_per_32b)))
               continue;

            unsigned base = ~0;
            for (unsigned i = 0; i < comps_per_32b; i++) {
               if (ins.mask & BITFIELD_BIT(c + i)) {
                  base = ins.swizzle[0][c + i];
                  break;
               }
            }

            assert(base != ~0);

            for (unsigned i = 0; i < comps_per_32b; i++) {
               if (!(ins.mask & BITFIELD_BIT(c + i))) {
                  ins.swizzle[0][c + i] = base + i;
                  ins.mask |= BITFIELD_BIT(c + i);
               }
               assert(ins.swizzle[0][c + i] == base + i);
            }
         }
      }
   } else {
      unsigned bitsize =
         nir_src_bit_size(intr->src[0]) * nir_src_num_components(intr->src[0]);

      if (bitsize == 8)
         ins = m_st_u8(srcdest, 0);
      else if (bitsize == 16)
         ins = m_st_u16(srcdest, 0);
      else if (bitsize <= 32)
         ins = m_st_32(srcdest, 0);
      else if (bitsize <= 64)
         ins = m_st_64(srcdest, 0);
      else if (bitsize <= 128)
         ins = m_st_128(srcdest, 0);
      else
         unreachable("Invalid global store size");

      mir_set_intr_mask(instr, &ins, is_read);
   }

   mir_set_offset(ctx, &ins, offset, seg);

   /* Set a valid swizzle for masked out components */
   assert(ins.mask);
   unsigned first_component = __builtin_ffs(ins.mask) - 1;

   for (unsigned i = 0; i < ARRAY_SIZE(ins.swizzle[0]); ++i) {
      if (!(ins.mask & (1 << i)))
         ins.swizzle[0][i] = first_component;
   }

   emit_mir_instruction(ctx, ins);
}

static midgard_load_store_op
translate_atomic_op(nir_atomic_op op)
{
   /* clang-format off */
   switch (op) {
   case nir_atomic_op_xchg:    return midgard_op_atomic_xchg;
   case nir_atomic_op_cmpxchg: return midgard_op_atomic_cmpxchg;
   case nir_atomic_op_iadd:    return midgard_op_atomic_add;
   case nir_atomic_op_iand:    return midgard_op_atomic_and;
   case nir_atomic_op_imax:    return midgard_op_atomic_imax;
   case nir_atomic_op_imin:    return midgard_op_atomic_imin;
   case nir_atomic_op_ior:     return midgard_op_atomic_or;
   case nir_atomic_op_umax:    return midgard_op_atomic_umax;
   case nir_atomic_op_umin:    return midgard_op_atomic_umin;
   case nir_atomic_op_ixor:    return midgard_op_atomic_xor;
   default: unreachable("Unexpected atomic");
   }
   /* clang-format on */
}

/* Emit an atomic to shared memory or global memory. */
static void
emit_atomic(compiler_context *ctx, nir_intrinsic_instr *instr)
{
   midgard_load_store_op op =
      translate_atomic_op(nir_intrinsic_atomic_op(instr));

   nir_alu_type type =
      (op == midgard_op_atomic_imin || op == midgard_op_atomic_imax)
         ? nir_type_int
         : nir_type_uint;

   bool is_shared = (instr->intrinsic == nir_intrinsic_shared_atomic) ||
                    (instr->intrinsic == nir_intrinsic_shared_atomic_swap);

   unsigned dest = nir_def_index(&instr->def);
   unsigned val = nir_src_index(ctx, &instr->src[1]);
   unsigned bitsize = nir_src_bit_size(instr->src[1]);
   emit_explicit_constant(ctx, val);

   midgard_instruction ins = {
      .type = TAG_LOAD_STORE_4,
      .mask = 0xF,
      .dest = dest,
      .src = {~0, ~0, ~0, val},
      .src_types = {0, 0, 0, type | bitsize},
      .op = op,
   };

   nir_src *src_offset = nir_get_io_offset_src(instr);

   if (op == midgard_op_atomic_cmpxchg) {
      unsigned xchg_val = nir_src_index(ctx, &instr->src[2]);
      emit_explicit_constant(ctx, xchg_val);

      ins.src[2] = val;
      ins.src_types[2] = type | bitsize;
      ins.src[3] = xchg_val;

      if (is_shared) {
         ins.load_store.arg_reg = REGISTER_LDST_LOCAL_STORAGE_PTR;
         ins.load_store.arg_comp = COMPONENT_Z;
         ins.load_store.bitsize_toggle = true;
      } else {
         for (unsigned i = 0; i < 2; ++i)
            ins.swizzle[1][i] = i;

         ins.src[1] = nir_src_index(ctx, src_offset);
         ins.src_types[1] = nir_type_uint64;
      }
   } else
      mir_set_offset(ctx, &ins, src_offset,
                     is_shared ? LDST_SHARED : LDST_GLOBAL);

   mir_set_intr_mask(&instr->instr, &ins, true);

   emit_mir_instruction(ctx, ins);
}

static void
emit_varying_read(compiler_context *ctx, unsigned dest, unsigned offset,
                  unsigned nr_comp, unsigned component,
                  nir_src *indirect_offset, nir_alu_type type, bool flat)
{
   midgard_instruction ins = m_ld_vary_32(dest, PACK_LDST_ATTRIB_OFS(offset));
   ins.mask = mask_of(nr_comp);
   ins.dest_type = type;

   if (type == nir_type_float16) {
      /* Ensure we are aligned so we can pack it later */
      ins.mask = mask_of(ALIGN_POT(nr_comp, 2));
   }

   for (unsigned i = 0; i < ARRAY_SIZE(ins.swizzle[0]); ++i)
      ins.swizzle[0][i] = MIN2(i + component, COMPONENT_W);

   midgard_varying_params p = {
      .flat_shading = flat,
      .perspective_correction = 1,
      .interpolate_sample = true,
   };
   midgard_pack_varying_params(&ins.load_store, p);

   if (indirect_offset) {
      ins.src[2] = nir_src_index(ctx, indirect_offset);
      ins.src_types[2] = nir_type_uint32;
   } else
      ins.load_store.index_reg = REGISTER_LDST_ZERO;

   ins.load_store.arg_reg = REGISTER_LDST_ZERO;
   ins.load_store.index_format = midgard_index_address_u32;

   /* For flat shading, we always use .u32 and require 32-bit mode. For
    * smooth shading, we use the appropriate floating-point type.
    *
    * This could be optimized, but it makes it easy to check correctness.
    */
   if (flat) {
      assert(nir_alu_type_get_type_size(type) == 32);
      ins.op = midgard_op_ld_vary_32u;
   } else {
      assert(nir_alu_type_get_base_type(type) == nir_type_float);

      ins.op = (nir_alu_type_get_type_size(type) == 32) ? midgard_op_ld_vary_32
                                                        : midgard_op_ld_vary_16;
   }

   emit_mir_instruction(ctx, ins);
}

static midgard_instruction
emit_image_op(compiler_context *ctx, nir_intrinsic_instr *instr)
{
   enum glsl_sampler_dim dim = nir_intrinsic_image_dim(instr);
   unsigned nr_attr = ctx->stage == MESA_SHADER_VERTEX
                         ? util_bitcount64(ctx->nir->info.inputs_read)
                         : 0;
   unsigned nr_dim = glsl_get_sampler_dim_coordinate_components(dim);
   bool is_array = nir_intrinsic_image_array(instr);
   bool is_store = instr->intrinsic == nir_intrinsic_image_store;

   /* TODO: MSAA */
   assert(dim != GLSL_SAMPLER_DIM_MS && "MSAA'd images not supported");

   unsigned coord_reg = nir_src_index(ctx, &instr->src[1]);
   emit_explicit_constant(ctx, coord_reg);

   nir_src *index = &instr->src[0];
   bool is_direct = nir_src_is_const(*index);

   /* For image opcodes, address is used as an index into the attribute
    * descriptor */
   unsigned address = nr_attr;
   if (is_direct)
      address += nir_src_as_uint(*index);

   midgard_instruction ins;
   if (is_store) { /* emit st_image_* */
      unsigned val = nir_src_index(ctx, &instr->src[3]);
      emit_explicit_constant(ctx, val);

      nir_alu_type type = nir_intrinsic_src_type(instr);
      ins = st_image(type, val, PACK_LDST_ATTRIB_OFS(address));
      nir_alu_type base_type = nir_alu_type_get_base_type(type);
      ins.src_types[0] = base_type | nir_src_bit_size(instr->src[3]);
   } else if (instr->intrinsic == nir_intrinsic_image_texel_address) {
      ins =
         m_lea_image(nir_def_index(&instr->def), PACK_LDST_ATTRIB_OFS(address));
      ins.mask = mask_of(2); /* 64-bit memory address */
   } else {                  /* emit ld_image_* */
      nir_alu_type type = nir_intrinsic_dest_type(instr);
      ins = ld_image(type, nir_def_index(&instr->def),
                     PACK_LDST_ATTRIB_OFS(address));
      ins.mask = mask_of(nir_intrinsic_dest_components(instr));
      ins.dest_type = type;
   }

   /* Coord reg */
   ins.src[1] = coord_reg;
   ins.src_types[1] = nir_type_uint16;
   if (nr_dim == 3 || is_array) {
      ins.load_store.bitsize_toggle = true;
   }

   /* Image index reg */
   if (!is_direct) {
      ins.src[2] = nir_src_index(ctx, index);
      ins.src_types[2] = nir_type_uint32;
   } else
      ins.load_store.index_reg = REGISTER_LDST_ZERO;

   emit_mir_instruction(ctx, ins);

   return ins;
}

static void
emit_attr_read(compiler_context *ctx, unsigned dest, unsigned offset,
               unsigned nr_comp, nir_alu_type t)
{
   midgard_instruction ins = m_ld_attr_32(dest, PACK_LDST_ATTRIB_OFS(offset));
   ins.load_store.arg_reg = REGISTER_LDST_ZERO;
   ins.load_store.index_reg = REGISTER_LDST_ZERO;
   ins.mask = mask_of(nr_comp);

   /* Use the type appropriate load */
   switch (t) {
   case nir_type_uint:
   case nir_type_bool:
      ins.op = midgard_op_ld_attr_32u;
      break;
   case nir_type_int:
      ins.op = midgard_op_ld_attr_32i;
      break;
   case nir_type_float:
      ins.op = midgard_op_ld_attr_32;
      break;
   default:
      unreachable("Attempted to load unknown type");
      break;
   }

   emit_mir_instruction(ctx, ins);
}

static unsigned
compute_builtin_arg(nir_intrinsic_op op)
{
   switch (op) {
   case nir_intrinsic_load_workgroup_id:
      return REGISTER_LDST_GROUP_ID;
   case nir_intrinsic_load_local_invocation_id:
      return REGISTER_LDST_LOCAL_THREAD_ID;
   case nir_intrinsic_load_global_invocation_id:
   case nir_intrinsic_load_global_invocation_id_zero_base:
      return REGISTER_LDST_GLOBAL_THREAD_ID;
   default:
      unreachable("Invalid compute paramater loaded");
   }
}

static void
emit_fragment_store(compiler_context *ctx, unsigned src, unsigned src_z,
                    unsigned src_s, enum midgard_rt_id rt, unsigned sample_iter)
{
   assert(rt < ARRAY_SIZE(ctx->writeout_branch));
   assert(sample_iter < ARRAY_SIZE(ctx->writeout_branch[0]));

   midgard_instruction *br = ctx->writeout_branch[rt][sample_iter];

   assert(!br);

   emit_explicit_constant(ctx, src);

   struct midgard_instruction ins = v_branch(false, false);

   bool depth_only = (rt == MIDGARD_ZS_RT);

   ins.writeout = depth_only ? 0 : PAN_WRITEOUT_C;

   /* Add dependencies */
   ins.src[0] = src;
   ins.src_types[0] = nir_type_uint32;

   if (depth_only)
      ins.constants.u32[0] = 0xFF;
   else
      ins.constants.u32[0] = ((rt - MIDGARD_COLOR_RT0) << 8) | sample_iter;

   for (int i = 0; i < 4; ++i)
      ins.swizzle[0][i] = i;

   if (~src_z) {
      emit_explicit_constant(ctx, src_z);
      ins.src[2] = src_z;
      ins.src_types[2] = nir_type_uint32;
      ins.writeout |= PAN_WRITEOUT_Z;
   }
   if (~src_s) {
      emit_explicit_constant(ctx, src_s);
      ins.src[3] = src_s;
      ins.src_types[3] = nir_type_uint32;
      ins.writeout |= PAN_WRITEOUT_S;
   }

   /* Emit the branch */
   br = emit_mir_instruction(ctx, ins);
   schedule_barrier(ctx);
   ctx->writeout_branch[rt][sample_iter] = br;

   /* Push our current location = current block count - 1 = where we'll
    * jump to. Maybe a bit too clever for my own good */

   br->branch.target_block = ctx->block_count - 1;
}

static void
emit_compute_builtin(compiler_context *ctx, nir_intrinsic_instr *instr)
{
   unsigned reg = nir_def_index(&instr->def);
   midgard_instruction ins = m_ldst_mov(reg, 0);
   ins.mask = mask_of(3);
   ins.swizzle[0][3] = COMPONENT_X; /* xyzx */
   ins.load_store.arg_reg = compute_builtin_arg(instr->intrinsic);
   emit_mir_instruction(ctx, ins);
}

static unsigned
vertex_builtin_arg(nir_intrinsic_op op)
{
   switch (op) {
   case nir_intrinsic_load_vertex_id_zero_base:
      return PAN_VERTEX_ID;
   case nir_intrinsic_load_instance_id:
      return PAN_INSTANCE_ID;
   default:
      unreachable("Invalid vertex builtin");
   }
}

static void
emit_vertex_builtin(compiler_context *ctx, nir_intrinsic_instr *instr)
{
   unsigned reg = nir_def_index(&instr->def);
   emit_attr_read(ctx, reg, vertex_builtin_arg(instr->intrinsic), 1,
                  nir_type_int);
}

static void
emit_special(compiler_context *ctx, nir_intrinsic_instr *instr, unsigned idx)
{
   unsigned reg = nir_def_index(&instr->def);

   midgard_instruction ld = m_ld_tilebuffer_raw(reg, 0);
   ld.op = midgard_op_ld_special_32u;
   ld.load_store.signed_offset = PACK_LDST_SELECTOR_OFS(idx);
   ld.load_store.index_reg = REGISTER_LDST_ZERO;

   for (int i = 0; i < 4; ++i)
      ld.swizzle[0][i] = COMPONENT_X;

   emit_mir_instruction(ctx, ld);
}

static void
emit_control_barrier(compiler_context *ctx)
{
   midgard_instruction ins = {
      .type = TAG_TEXTURE_4,
      .dest = ~0,
      .src = {~0, ~0, ~0, ~0},
      .op = midgard_tex_op_barrier,
   };

   emit_mir_instruction(ctx, ins);
}

static uint8_t
output_load_rt_addr(compiler_context *ctx, nir_intrinsic_instr *instr)
{
   unsigned loc = nir_intrinsic_io_semantics(instr).location;

   if (loc >= FRAG_RESULT_DATA0)
      return loc - FRAG_RESULT_DATA0;

   if (loc == FRAG_RESULT_DEPTH)
      return 0x1F;
   if (loc == FRAG_RESULT_STENCIL)
      return 0x1E;

   unreachable("Invalid RT to load from");
}

static void
emit_intrinsic(compiler_context *ctx, nir_intrinsic_instr *instr)
{
   unsigned offset = 0, reg;

   switch (instr->intrinsic) {
   case nir_intrinsic_decl_reg:
   case nir_intrinsic_store_reg:
      /* Always fully consumed */
      break;

   case nir_intrinsic_load_reg: {
      /* NIR guarantees that, for typical isel, this will always be fully
       * consumed. However, we also do our own nir_scalar chasing for
       * address arithmetic, bypassing the source chasing helpers. So we can end
       * up with unconsumed load_register instructions. Translate them here. 99%
       * of the time, these moves will be DCE'd away.
       */
      nir_def *handle = instr->src[0].ssa;

      midgard_instruction ins =
         v_mov(nir_reg_index(handle), nir_def_index(&instr->def));

      ins.dest_type = ins.src_types[1] = nir_type_uint | instr->def.bit_size;

      ins.mask = BITFIELD_MASK(instr->def.num_components);
      emit_mir_instruction(ctx, ins);
      break;
   }

   case nir_intrinsic_discard_if:
   case nir_intrinsic_discard: {
      bool conditional = instr->intrinsic == nir_intrinsic_discard_if;
      struct midgard_instruction discard = v_branch(conditional, false);
      discard.branch.target_type = TARGET_DISCARD;

      if (conditional) {
         discard.src[0] = nir_src_index(ctx, &instr->src[0]);
         discard.src_types[0] = nir_type_uint32;
      }

      emit_mir_instruction(ctx, discard);
      schedule_barrier(ctx);

      break;
   }

   case nir_intrinsic_image_load:
   case nir_intrinsic_image_store:
   case nir_intrinsic_image_texel_address:
      emit_image_op(ctx, instr);
      break;

   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_global:
   case nir_intrinsic_load_global_constant:
   case nir_intrinsic_load_shared:
   case nir_intrinsic_load_scratch:
   case nir_intrinsic_load_input:
   case nir_intrinsic_load_interpolated_input: {
      bool is_ubo = instr->intrinsic == nir_intrinsic_load_ubo;
      bool is_global = instr->intrinsic == nir_intrinsic_load_global ||
                       instr->intrinsic == nir_intrinsic_load_global_constant;
      bool is_shared = instr->intrinsic == nir_intrinsic_load_shared;
      bool is_scratch = instr->intrinsic == nir_intrinsic_load_scratch;
      bool is_flat = instr->intrinsic == nir_intrinsic_load_input;
      bool is_interp =
         instr->intrinsic == nir_intrinsic_load_interpolated_input;

      /* Get the base type of the intrinsic */
      /* TODO: Infer type? Does it matter? */
      nir_alu_type t = (is_interp) ? nir_type_float
                       : (is_flat) ? nir_intrinsic_dest_type(instr)
                                   : nir_type_uint;

      t = nir_alu_type_get_base_type(t);

      if (!(is_ubo || is_global || is_scratch)) {
         offset = nir_intrinsic_base(instr);
      }

      unsigned nr_comp = nir_intrinsic_dest_components(instr);

      nir_src *src_offset = nir_get_io_offset_src(instr);

      bool direct = nir_src_is_const(*src_offset);
      nir_src *indirect_offset = direct ? NULL : src_offset;

      if (direct)
         offset += nir_src_as_uint(*src_offset);

      /* We may need to apply a fractional offset */
      int component =
         (is_flat || is_interp) ? nir_intrinsic_component(instr) : 0;
      reg = nir_def_index(&instr->def);

      if (is_ubo) {
         nir_src index = instr->src[0];

         /* TODO: Is indirect block number possible? */
         assert(nir_src_is_const(index));

         uint32_t uindex = nir_src_as_uint(index);
         emit_ubo_read(ctx, &instr->instr, reg, offset, indirect_offset, 0,
                       uindex, nr_comp);
      } else if (is_global || is_shared || is_scratch) {
         unsigned seg =
            is_global ? LDST_GLOBAL : (is_shared ? LDST_SHARED : LDST_SCRATCH);
         emit_global(ctx, &instr->instr, true, reg, src_offset, seg);
      } else if (ctx->stage == MESA_SHADER_FRAGMENT && !ctx->inputs->is_blend) {
         emit_varying_read(ctx, reg, offset, nr_comp, component,
                           indirect_offset, t | instr->def.bit_size, is_flat);
      } else if (ctx->inputs->is_blend) {
         /* ctx->blend_input will be precoloured to r0/r2, where
          * the input is preloaded */

         unsigned *input = offset ? &ctx->blend_src1 : &ctx->blend_input;

         if (*input == ~0)
            *input = reg;
         else
            emit_mir_instruction(ctx, v_mov(*input, reg));
      } else if (ctx->stage == MESA_SHADER_VERTEX) {
         emit_attr_read(ctx, reg, offset, nr_comp, t);
      } else {
         unreachable("Unknown load");
      }

      break;
   }

   /* Handled together with load_interpolated_input */
   case nir_intrinsic_load_barycentric_pixel:
   case nir_intrinsic_load_barycentric_centroid:
   case nir_intrinsic_load_barycentric_sample:
      break;

      /* Reads 128-bit value raw off the tilebuffer during blending, tasty */

   case nir_intrinsic_load_raw_output_pan: {
      reg = nir_def_index(&instr->def);

      /* T720 and below use different blend opcodes with slightly
       * different semantics than T760 and up */

      midgard_instruction ld = m_ld_tilebuffer_raw(reg, 0);

      unsigned target = output_load_rt_addr(ctx, instr);
      ld.load_store.index_comp = target & 0x3;
      ld.load_store.index_reg = target >> 2;

      if (nir_src_is_const(instr->src[0])) {
         unsigned sample = nir_src_as_uint(instr->src[0]);
         ld.load_store.arg_comp = sample & 0x3;
         ld.load_store.arg_reg = sample >> 2;
      } else {
         /* Enable sample index via register. */
         ld.load_store.signed_offset |= 1;
         ld.src[1] = nir_src_index(ctx, &instr->src[0]);
         ld.src_types[1] = nir_type_int32;
      }

      if (ctx->quirks & MIDGARD_OLD_BLEND) {
         ld.op = midgard_op_ld_special_32u;
         ld.load_store.signed_offset = PACK_LDST_SELECTOR_OFS(16);
         ld.load_store.index_reg = REGISTER_LDST_ZERO;
      }

      emit_mir_instruction(ctx, ld);
      break;
   }

   case nir_intrinsic_load_output: {
      reg = nir_def_index(&instr->def);

      unsigned bits = instr->def.bit_size;

      midgard_instruction ld;
      if (bits == 16)
         ld = m_ld_tilebuffer_16f(reg, 0);
      else
         ld = m_ld_tilebuffer_32f(reg, 0);

      unsigned index = output_load_rt_addr(ctx, instr);
      ld.load_store.index_comp = index & 0x3;
      ld.load_store.index_reg = index >> 2;

      for (unsigned c = 4; c < 16; ++c)
         ld.swizzle[0][c] = 0;

      if (ctx->quirks & MIDGARD_OLD_BLEND) {
         if (bits == 16)
            ld.op = midgard_op_ld_special_16f;
         else
            ld.op = midgard_op_ld_special_32f;
         ld.load_store.signed_offset = PACK_LDST_SELECTOR_OFS(1);
         ld.load_store.index_reg = REGISTER_LDST_ZERO;
      }

      emit_mir_instruction(ctx, ld);
      break;
   }

   case nir_intrinsic_store_output:
   case nir_intrinsic_store_combined_output_pan:
      assert(nir_src_is_const(instr->src[1]) && "no indirect outputs");

      reg = nir_src_index(ctx, &instr->src[0]);

      if (ctx->stage == MESA_SHADER_FRAGMENT) {
         bool combined =
            instr->intrinsic == nir_intrinsic_store_combined_output_pan;

         enum midgard_rt_id rt;

         unsigned reg_z = ~0, reg_s = ~0, reg_2 = ~0;
         unsigned writeout = PAN_WRITEOUT_C;
         if (combined) {
            writeout = nir_intrinsic_component(instr);
            if (writeout & PAN_WRITEOUT_Z)
               reg_z = nir_src_index(ctx, &instr->src[2]);
            if (writeout & PAN_WRITEOUT_S)
               reg_s = nir_src_index(ctx, &instr->src[3]);
            if (writeout & PAN_WRITEOUT_2)
               reg_2 = nir_src_index(ctx, &instr->src[4]);
         }

         if (writeout & PAN_WRITEOUT_C) {
            nir_io_semantics sem = nir_intrinsic_io_semantics(instr);

            rt = MIDGARD_COLOR_RT0 + (sem.location - FRAG_RESULT_DATA0);
         } else {
            rt = MIDGARD_ZS_RT;
         }

         /* Dual-source blend writeout is done by leaving the
          * value in r2 for the blend shader to use. */
         if (~reg_2) {
            emit_explicit_constant(ctx, reg_2);

            unsigned out = make_compiler_temp(ctx);

            midgard_instruction ins = v_mov(reg_2, out);
            emit_mir_instruction(ctx, ins);

            ctx->blend_src1 = out;
         }

         emit_fragment_store(ctx, reg, reg_z, reg_s, rt, 0);
      } else if (ctx->stage == MESA_SHADER_VERTEX) {
         assert(instr->intrinsic == nir_intrinsic_store_output);

         /* We should have been vectorized, though we don't
          * currently check that st_vary is emitted only once
          * per slot (this is relevant, since there's not a mask
          * parameter available on the store [set to 0 by the
          * blob]). We do respect the component by adjusting the
          * swizzle. If this is a constant source, we'll need to
          * emit that explicitly. */

         emit_explicit_constant(ctx, reg);

         offset = nir_intrinsic_base(instr) + nir_src_as_uint(instr->src[1]);

         unsigned dst_component = nir_intrinsic_component(instr);
         unsigned nr_comp = nir_src_num_components(instr->src[0]);

         /* ABI: Format controlled by the attribute descriptor.
          * This simplifies flat shading, although it prevents
          * certain (unimplemented) 16-bit optimizations.
          *
          * In particular, it lets the driver handle internal
          * TGSI shaders that set flat in the VS but smooth in
          * the FS. This matches our handling on Bifrost.
          */
         bool auto32 = true;
         assert(nir_alu_type_get_type_size(nir_intrinsic_src_type(instr)) ==
                32);

         /* ABI: varyings in the secondary attribute table */
         bool secondary_table = true;

         midgard_instruction st =
            m_st_vary_32(reg, PACK_LDST_ATTRIB_OFS(offset));
         st.load_store.arg_reg = REGISTER_LDST_ZERO;
         st.load_store.index_reg = REGISTER_LDST_ZERO;

         /* Attribute instruction uses these 2-bits for the
          * a32 and table bits, pack this specially.
          */
         st.load_store.index_format =
            (auto32 ? (1 << 0) : 0) | (secondary_table ? (1 << 1) : 0);

         /* nir_intrinsic_component(store_intr) encodes the
          * destination component start. Source component offset
          * adjustment is taken care of in
          * install_registers_instr(), when offset_swizzle() is
          * called.
          */
         unsigned src_component = COMPONENT_X;

         assert(nr_comp > 0);
         for (unsigned i = 0; i < ARRAY_SIZE(st.swizzle); ++i) {
            st.swizzle[0][i] = src_component;
            if (i >= dst_component && i < dst_component + nr_comp - 1)
               src_component++;
         }

         emit_mir_instruction(ctx, st);
      } else {
         unreachable("Unknown store");
      }

      break;

   /* Special case of store_output for lowered blend shaders */
   case nir_intrinsic_store_raw_output_pan: {
      assert(ctx->stage == MESA_SHADER_FRAGMENT);
      reg = nir_src_index(ctx, &instr->src[0]);

      nir_io_semantics sem = nir_intrinsic_io_semantics(instr);
      assert(sem.location >= FRAG_RESULT_DATA0);
      unsigned rt = sem.location - FRAG_RESULT_DATA0;

      emit_fragment_store(ctx, reg, ~0, ~0, rt + MIDGARD_COLOR_RT0,
                          nir_intrinsic_base(instr));
      break;
   }

   case nir_intrinsic_store_global:
   case nir_intrinsic_store_shared:
   case nir_intrinsic_store_scratch:
      reg = nir_src_index(ctx, &instr->src[0]);
      emit_explicit_constant(ctx, reg);

      unsigned seg;
      if (instr->intrinsic == nir_intrinsic_store_global)
         seg = LDST_GLOBAL;
      else if (instr->intrinsic == nir_intrinsic_store_shared)
         seg = LDST_SHARED;
      else
         seg = LDST_SCRATCH;

      emit_global(ctx, &instr->instr, false, reg, &instr->src[1], seg);
      break;

   case nir_intrinsic_load_workgroup_id:
   case nir_intrinsic_load_local_invocation_id:
   case nir_intrinsic_load_global_invocation_id:
   case nir_intrinsic_load_global_invocation_id_zero_base:
      emit_compute_builtin(ctx, instr);
      break;

   case nir_intrinsic_load_vertex_id_zero_base:
   case nir_intrinsic_load_instance_id:
      emit_vertex_builtin(ctx, instr);
      break;

   case nir_intrinsic_load_sample_mask_in:
      emit_special(ctx, instr, 96);
      break;

   case nir_intrinsic_load_sample_id:
      emit_special(ctx, instr, 97);
      break;

   case nir_intrinsic_barrier:
      if (nir_intrinsic_execution_scope(instr) != SCOPE_NONE) {
         schedule_barrier(ctx);
         emit_control_barrier(ctx);
         schedule_barrier(ctx);
      } else if (nir_intrinsic_memory_scope(instr) != SCOPE_NONE) {
         /* Midgard doesn't seem to want special handling, though we do need to
          * take care when scheduling to avoid incorrect reordering.
          *
          * Note this is an "else if" since the handling for the execution scope
          * case already covers the case when both scopes are present.
          */
         schedule_barrier(ctx);
      }
      break;

   case nir_intrinsic_shared_atomic:
   case nir_intrinsic_shared_atomic_swap:
   case nir_intrinsic_global_atomic:
   case nir_intrinsic_global_atomic_swap:
      emit_atomic(ctx, instr);
      break;

   default:
      fprintf(stderr, "Unhandled intrinsic %s\n",
              nir_intrinsic_infos[instr->intrinsic].name);
      assert(0);
      break;
   }
}

/* Returns dimension with 0 special casing cubemaps */
static unsigned
midgard_tex_format(enum glsl_sampler_dim dim)
{
   switch (dim) {
   case GLSL_SAMPLER_DIM_1D:
   case GLSL_SAMPLER_DIM_BUF:
      return 1;

   case GLSL_SAMPLER_DIM_2D:
   case GLSL_SAMPLER_DIM_MS:
   case GLSL_SAMPLER_DIM_EXTERNAL:
   case GLSL_SAMPLER_DIM_RECT:
      return 2;

   case GLSL_SAMPLER_DIM_3D:
      return 3;

   case GLSL_SAMPLER_DIM_CUBE:
      return 0;

   default:
      unreachable("Unknown sampler dim type");
   }
}

/* Tries to attach an explicit LOD or bias as a constant. Returns whether this
 * was successful */

static bool
pan_attach_constant_bias(compiler_context *ctx, nir_src lod,
                         midgard_texture_word *word)
{
   /* To attach as constant, it has to *be* constant */

   if (!nir_src_is_const(lod))
      return false;

   float f = nir_src_as_float(lod);

   /* Break into fixed-point */
   signed lod_int = f;
   float lod_frac = f - lod_int;

   /* Carry over negative fractions */
   if (lod_frac < 0.0) {
      lod_int--;
      lod_frac += 1.0;
   }

   /* Encode */
   word->bias = float_to_ubyte(lod_frac);
   word->bias_int = lod_int;

   return true;
}

static enum mali_texture_mode
mdg_texture_mode(nir_tex_instr *instr)
{
   if (instr->op == nir_texop_tg4 && instr->is_shadow)
      return TEXTURE_GATHER_SHADOW;
   else if (instr->op == nir_texop_tg4)
      return TEXTURE_GATHER_X + instr->component;
   else if (instr->is_shadow)
      return TEXTURE_SHADOW;
   else
      return TEXTURE_NORMAL;
}

static void
set_tex_coord(compiler_context *ctx, nir_tex_instr *instr,
              midgard_instruction *ins)
{
   int coord_idx = nir_tex_instr_src_index(instr, nir_tex_src_coord);

   assert(coord_idx >= 0);

   int comparator_idx = nir_tex_instr_src_index(instr, nir_tex_src_comparator);
   int ms_idx = nir_tex_instr_src_index(instr, nir_tex_src_ms_index);
   assert(comparator_idx < 0 || ms_idx < 0);
   int ms_or_comparator_idx = ms_idx >= 0 ? ms_idx : comparator_idx;

   unsigned coords = nir_src_index(ctx, &instr->src[coord_idx].src);

   emit_explicit_constant(ctx, coords);

   ins->src_types[1] = nir_tex_instr_src_type(instr, coord_idx) |
                       nir_src_bit_size(instr->src[coord_idx].src);

   unsigned nr_comps = instr->coord_components;
   unsigned written_mask = 0, write_mask = 0;

   /* Initialize all components to coord.x which is expected to always be
    * present. Swizzle is updated below based on the texture dimension
    * and extra attributes that are packed in the coordinate argument.
    */
   for (unsigned c = 0; c < MIR_VEC_COMPONENTS; c++)
      ins->swizzle[1][c] = COMPONENT_X;

   /* Shadow ref value is part of the coordinates if there's no comparator
    * source, in that case it's always placed in the last component.
    * Midgard wants the ref value in coord.z.
    */
   if (instr->is_shadow && comparator_idx < 0) {
      ins->swizzle[1][COMPONENT_Z] = --nr_comps;
      write_mask |= 1 << COMPONENT_Z;
   }

   /* The array index is the last component if there's no shadow ref value
    * or second last if there's one. We already decremented the number of
    * components to account for the shadow ref value above.
    * Midgard wants the array index in coord.w.
    */
   if (instr->is_array) {
      ins->swizzle[1][COMPONENT_W] = --nr_comps;
      write_mask |= 1 << COMPONENT_W;
   }

   if (instr->sampler_dim == GLSL_SAMPLER_DIM_CUBE) {
      /* texelFetch is undefined on samplerCube */
      assert(ins->op != midgard_tex_op_fetch);

      ins->src[1] = make_compiler_temp_reg(ctx);

      /* For cubemaps, we use a special ld/st op to select the face
       * and copy the xy into the texture register
       */
      midgard_instruction ld = m_ld_cubemap_coords(ins->src[1], 0);
      ld.src[1] = coords;
      ld.src_types[1] = ins->src_types[1];
      ld.mask = 0x3; /* xy */
      ld.load_store.bitsize_toggle = true;
      ld.swizzle[1][3] = COMPONENT_X;
      emit_mir_instruction(ctx, ld);

      /* We packed cube coordiates (X,Y,Z) into (X,Y), update the
       * written mask accordingly and decrement the number of
       * components
       */
      nr_comps--;
      written_mask |= 3;
   }

   /* Now flag tex coord components that have not been written yet */
   write_mask |= mask_of(nr_comps) & ~written_mask;
   for (unsigned c = 0; c < nr_comps; c++)
      ins->swizzle[1][c] = c;

   /* Sample index and shadow ref are expected in coord.z */
   if (ms_or_comparator_idx >= 0) {
      assert(!((write_mask | written_mask) & (1 << COMPONENT_Z)));

      unsigned sample_or_ref =
         nir_src_index(ctx, &instr->src[ms_or_comparator_idx].src);

      emit_explicit_constant(ctx, sample_or_ref);

      if (ins->src[1] == ~0)
         ins->src[1] = make_compiler_temp_reg(ctx);

      midgard_instruction mov = v_mov(sample_or_ref, ins->src[1]);

      for (unsigned c = 0; c < MIR_VEC_COMPONENTS; c++)
         mov.swizzle[1][c] = COMPONENT_X;

      mov.mask = 1 << COMPONENT_Z;
      written_mask |= 1 << COMPONENT_Z;
      ins->swizzle[1][COMPONENT_Z] = COMPONENT_Z;
      emit_mir_instruction(ctx, mov);
   }

   /* Texelfetch coordinates uses all four elements (xyz/index) regardless
    * of texture dimensionality, which means it's necessary to zero the
    * unused components to keep everything happy.
    */
   if (ins->op == midgard_tex_op_fetch && (written_mask | write_mask) != 0xF) {
      if (ins->src[1] == ~0)
         ins->src[1] = make_compiler_temp_reg(ctx);

      /* mov index.zw, #0, or generalized */
      midgard_instruction mov =
         v_mov(SSA_FIXED_REGISTER(REGISTER_CONSTANT), ins->src[1]);
      mov.has_constants = true;
      mov.mask = (written_mask | write_mask) ^ 0xF;
      emit_mir_instruction(ctx, mov);
      for (unsigned c = 0; c < MIR_VEC_COMPONENTS; c++) {
         if (mov.mask & (1 << c))
            ins->swizzle[1][c] = c;
      }
   }

   if (ins->src[1] == ~0) {
      /* No temporary reg created, use the src coords directly */
      ins->src[1] = coords;
   } else if (write_mask) {
      /* Move the remaining coordinates to the temporary reg */
      midgard_instruction mov = v_mov(coords, ins->src[1]);

      for (unsigned c = 0; c < MIR_VEC_COMPONENTS; c++) {
         if ((1 << c) & write_mask) {
            mov.swizzle[1][c] = ins->swizzle[1][c];
            ins->swizzle[1][c] = c;
         } else {
            mov.swizzle[1][c] = COMPONENT_X;
         }
      }

      mov.mask = write_mask;
      emit_mir_instruction(ctx, mov);
   }
}

static void
emit_texop_native(compiler_context *ctx, nir_tex_instr *instr,
                  unsigned midgard_texop)
{
   int texture_index = instr->texture_index;
   int sampler_index = instr->sampler_index;

   /* If txf is used, we assume there is a valid sampler bound at index 0. Use
    * it for txf operations, since there may be no other valid samplers. This is
    * a workaround: txf does not require a sampler in NIR (so sampler_index is
    * undefined) but we need one in the hardware. This is ABI with the driver.
    */
   if (!nir_tex_instr_need_sampler(instr))
      sampler_index = 0;

   midgard_instruction ins = {
      .type = TAG_TEXTURE_4,
      .mask = 0xF,
      .dest = nir_def_index(&instr->def),
      .src = {~0, ~0, ~0, ~0},
      .dest_type = instr->dest_type,
      .swizzle = SWIZZLE_IDENTITY_4,
      .outmod = midgard_outmod_none,
      .op = midgard_texop,
      .texture = {
         .format = midgard_tex_format(instr->sampler_dim),
         .texture_handle = texture_index,
         .sampler_handle = sampler_index,
         .mode = mdg_texture_mode(instr),
      }};

   if (instr->is_shadow && !instr->is_new_style_shadow &&
       instr->op != nir_texop_tg4)
      for (int i = 0; i < 4; ++i)
         ins.swizzle[0][i] = COMPONENT_X;

   for (unsigned i = 0; i < instr->num_srcs; ++i) {
      int index = nir_src_index(ctx, &instr->src[i].src);
      unsigned sz = nir_src_bit_size(instr->src[i].src);
      nir_alu_type T = nir_tex_instr_src_type(instr, i) | sz;

      switch (instr->src[i].src_type) {
      case nir_tex_src_coord:
         set_tex_coord(ctx, instr, &ins);
         break;

      case nir_tex_src_bias:
      case nir_tex_src_lod: {
         /* Try as a constant if we can */

         bool is_txf = midgard_texop == midgard_tex_op_fetch;
         if (!is_txf &&
             pan_attach_constant_bias(ctx, instr->src[i].src, &ins.texture))
            break;

         ins.texture.lod_register = true;
         ins.src[2] = index;
         ins.src_types[2] = T;

         for (unsigned c = 0; c < MIR_VEC_COMPONENTS; ++c)
            ins.swizzle[2][c] = COMPONENT_X;

         emit_explicit_constant(ctx, index);

         break;
      };

      case nir_tex_src_offset: {
         ins.texture.offset_register = true;
         ins.src[3] = index;
         ins.src_types[3] = T;

         for (unsigned c = 0; c < MIR_VEC_COMPONENTS; ++c)
            ins.swizzle[3][c] = (c > COMPONENT_Z) ? 0 : c;

         emit_explicit_constant(ctx, index);
         break;
      };

      case nir_tex_src_comparator:
      case nir_tex_src_ms_index:
         /* Nothing to do, handled in set_tex_coord() */
         break;

      default: {
         fprintf(stderr, "Unknown texture source type: %d\n",
                 instr->src[i].src_type);
         assert(0);
      }
      }
   }

   emit_mir_instruction(ctx, ins);
}

static void
emit_tex(compiler_context *ctx, nir_tex_instr *instr)
{
   switch (instr->op) {
   case nir_texop_tex:
   case nir_texop_txb:
      emit_texop_native(ctx, instr, midgard_tex_op_normal);
      break;
   case nir_texop_txl:
   case nir_texop_tg4:
      emit_texop_native(ctx, instr, midgard_tex_op_gradient);
      break;
   case nir_texop_txf:
   case nir_texop_txf_ms:
      emit_texop_native(ctx, instr, midgard_tex_op_fetch);
      break;
   default: {
      fprintf(stderr, "Unhandled texture op: %d\n", instr->op);
      assert(0);
   }
   }
}

static void
emit_jump(compiler_context *ctx, nir_jump_instr *instr)
{
   switch (instr->type) {
   case nir_jump_break: {
      /* Emit a branch out of the loop */
      struct midgard_instruction br = v_branch(false, false);
      br.branch.target_type = TARGET_BREAK;
      br.branch.target_break = ctx->current_loop_depth;
      emit_mir_instruction(ctx, br);
      break;
   }

   default:
      unreachable("Unhandled jump");
   }
}

static void
emit_instr(compiler_context *ctx, struct nir_instr *instr)
{
   switch (instr->type) {
   case nir_instr_type_load_const:
      emit_load_const(ctx, nir_instr_as_load_const(instr));
      break;

   case nir_instr_type_intrinsic:
      emit_intrinsic(ctx, nir_instr_as_intrinsic(instr));
      break;

   case nir_instr_type_alu:
      emit_alu(ctx, nir_instr_as_alu(instr));
      break;

   case nir_instr_type_tex:
      emit_tex(ctx, nir_instr_as_tex(instr));
      break;

   case nir_instr_type_jump:
      emit_jump(ctx, nir_instr_as_jump(instr));
      break;

   case nir_instr_type_undef:
      /* Spurious */
      break;

   default:
      unreachable("Unhandled instruction type");
   }
}

/* ALU instructions can inline or embed constants, which decreases register
 * pressure and saves space. */

#define CONDITIONAL_ATTACH(idx)                                                \
   {                                                                           \
      void *entry =                                                            \
         _mesa_hash_table_u64_search(ctx->ssa_constants, alu->src[idx] + 1);   \
                                                                               \
      if (entry) {                                                             \
         attach_constants(ctx, alu, entry, alu->src[idx] + 1);                 \
         alu->src[idx] = SSA_FIXED_REGISTER(REGISTER_CONSTANT);                \
      }                                                                        \
   }

static void
inline_alu_constants(compiler_context *ctx, midgard_block *block)
{
   mir_foreach_instr_in_block(block, alu) {
      /* Other instructions cannot inline constants */
      if (alu->type != TAG_ALU_4)
         continue;
      if (alu->compact_branch)
         continue;

      /* If there is already a constant here, we can do nothing */
      if (alu->has_constants)
         continue;

      CONDITIONAL_ATTACH(0);

      if (!alu->has_constants) {
         CONDITIONAL_ATTACH(1)
      } else if (!alu->inline_constant) {
         /* Corner case: _two_ vec4 constants, for instance with a
          * csel. For this case, we can only use a constant
          * register for one, we'll have to emit a move for the
          * other. */

         void *entry =
            _mesa_hash_table_u64_search(ctx->ssa_constants, alu->src[1] + 1);
         unsigned scratch = make_compiler_temp(ctx);

         if (entry) {
            midgard_instruction ins =
               v_mov(SSA_FIXED_REGISTER(REGISTER_CONSTANT), scratch);
            attach_constants(ctx, &ins, entry, alu->src[1] + 1);

            /* Set the source */
            alu->src[1] = scratch;

            /* Inject us -before- the last instruction which set r31, if
             * possible.
             */
            midgard_instruction *first = list_first_entry(
               &block->base.instructions, midgard_instruction, link);

            if (alu == first) {
               mir_insert_instruction_before(ctx, alu, ins);
            } else {
               mir_insert_instruction_before(ctx, mir_prev_op(alu), ins);
            }
         }
      }
   }
}

unsigned
max_bitsize_for_alu(midgard_instruction *ins)
{
   unsigned max_bitsize = 0;
   for (int i = 0; i < MIR_SRC_COUNT; i++) {
      if (ins->src[i] == ~0)
         continue;
      unsigned src_bitsize = nir_alu_type_get_type_size(ins->src_types[i]);
      max_bitsize = MAX2(src_bitsize, max_bitsize);
   }
   unsigned dst_bitsize = nir_alu_type_get_type_size(ins->dest_type);
   max_bitsize = MAX2(dst_bitsize, max_bitsize);

   /* We emulate 8-bit as 16-bit for simplicity of packing */
   max_bitsize = MAX2(max_bitsize, 16);

   /* We don't have fp16 LUTs, so we'll want to emit code like:
    *
    *      vlut.fsinr hr0, hr0
    *
    * where both input and output are 16-bit but the operation is carried
    * out in 32-bit
    */

   switch (ins->op) {
   case midgard_alu_op_fsqrt:
   case midgard_alu_op_frcp:
   case midgard_alu_op_frsqrt:
   case midgard_alu_op_fsinpi:
   case midgard_alu_op_fcospi:
   case midgard_alu_op_fexp2:
   case midgard_alu_op_flog2:
      max_bitsize = MAX2(max_bitsize, 32);
      break;

   default:
      break;
   }

   /* High implies computing at a higher bitsize, e.g umul_high of 32-bit
    * requires computing at 64-bit */
   if (midgard_is_integer_out_op(ins->op) &&
       ins->outmod == midgard_outmod_keephi) {
      max_bitsize *= 2;
      assert(max_bitsize <= 64);
   }

   return max_bitsize;
}

midgard_reg_mode
reg_mode_for_bitsize(unsigned bitsize)
{
   switch (bitsize) {
      /* use 16 pipe for 8 since we don't support vec16 yet */
   case 8:
   case 16:
      return midgard_reg_mode_16;
   case 32:
      return midgard_reg_mode_32;
   case 64:
      return midgard_reg_mode_64;
   default:
      unreachable("invalid bit size");
   }
}

/* Midgard supports two types of constants, embedded constants (128-bit) and
 * inline constants (16-bit). Sometimes, especially with scalar ops, embedded
 * constants can be demoted to inline constants, for space savings and
 * sometimes a performance boost */

static void
embedded_to_inline_constant(compiler_context *ctx, midgard_block *block)
{
   mir_foreach_instr_in_block(block, ins) {
      if (!ins->has_constants)
         continue;
      if (ins->has_inline_constant)
         continue;

      unsigned max_bitsize = max_bitsize_for_alu(ins);

      /* We can inline 32-bit (sometimes) or 16-bit (usually) */
      bool is_16 = max_bitsize == 16;
      bool is_32 = max_bitsize == 32;

      if (!(is_16 || is_32))
         continue;

      /* src1 cannot be an inline constant due to encoding
       * restrictions. So, if possible we try to flip the arguments
       * in that case */

      int op = ins->op;

      if (ins->src[0] == SSA_FIXED_REGISTER(REGISTER_CONSTANT) &&
          alu_opcode_props[op].props & OP_COMMUTES) {
         mir_flip(ins);
      }

      if (ins->src[1] == SSA_FIXED_REGISTER(REGISTER_CONSTANT)) {
         /* Component is from the swizzle. Take a nonzero component */
         assert(ins->mask);
         unsigned first_comp = ffs(ins->mask) - 1;
         unsigned component = ins->swizzle[1][first_comp];

         /* Scale constant appropriately, if we can legally */
         int16_t scaled_constant = 0;

         if (is_16) {
            scaled_constant = ins->constants.u16[component];
         } else if (midgard_is_integer_op(op)) {
            scaled_constant = ins->constants.u32[component];

            /* Constant overflow after resize */
            if (scaled_constant != ins->constants.u32[component])
               continue;
         } else {
            float original = ins->constants.f32[component];
            scaled_constant = _mesa_float_to_half(original);

            /* Check for loss of precision. If this is
             * mediump, we don't care, but for a highp
             * shader, we need to pay attention. NIR
             * doesn't yet tell us which mode we're in!
             * Practically this prevents most constants
             * from being inlined, sadly. */

            float fp32 = _mesa_half_to_float(scaled_constant);

            if (fp32 != original)
               continue;
         }

         /* Should've been const folded */
         if (ins->src_abs[1] || ins->src_neg[1])
            continue;

         /* Make sure that the constant is not itself a vector
          * by checking if all accessed values are the same. */

         const midgard_constants *cons = &ins->constants;
         uint32_t value = is_16 ? cons->u16[component] : cons->u32[component];

         bool is_vector = false;
         unsigned mask = effective_writemask(ins->op, ins->mask);

         for (unsigned c = 0; c < MIR_VEC_COMPONENTS; ++c) {
            /* We only care if this component is actually used */
            if (!(mask & (1 << c)))
               continue;

            uint32_t test = is_16 ? cons->u16[ins->swizzle[1][c]]
                                  : cons->u32[ins->swizzle[1][c]];

            if (test != value) {
               is_vector = true;
               break;
            }
         }

         if (is_vector)
            continue;

         /* Get rid of the embedded constant */
         ins->has_constants = false;
         ins->src[1] = ~0;
         ins->has_inline_constant = true;
         ins->inline_constant = scaled_constant;
      }
   }
}

/* Dead code elimination for branches at the end of a block - only one branch
 * per block is legal semantically */

static void
midgard_cull_dead_branch(compiler_context *ctx, midgard_block *block)
{
   bool branched = false;

   mir_foreach_instr_in_block_safe(block, ins) {
      if (!midgard_is_branch_unit(ins->unit))
         continue;

      if (branched)
         mir_remove_instruction(ins);

      branched = true;
   }
}

/* We want to force the invert on AND/OR to the second slot to legalize into
 * iandnot/iornot. The relevant patterns are for AND (and OR respectively)
 *
 *   ~a & #b = ~a & ~(#~b)
 *   ~a & b = b & ~a
 */

static void
midgard_legalize_invert(compiler_context *ctx, midgard_block *block)
{
   mir_foreach_instr_in_block(block, ins) {
      if (ins->type != TAG_ALU_4)
         continue;

      if (ins->op != midgard_alu_op_iand && ins->op != midgard_alu_op_ior)
         continue;

      if (ins->src_invert[1] || !ins->src_invert[0])
         continue;

      if (ins->has_inline_constant) {
         /* ~(#~a) = ~(~#a) = a, so valid, and forces both
          * inverts on */
         ins->inline_constant = ~ins->inline_constant;
         ins->src_invert[1] = true;
      } else {
         /* Flip to the right invert order. Note
          * has_inline_constant false by assumption on the
          * branch, so flipping makes sense. */
         mir_flip(ins);
      }
   }
}

static unsigned
emit_fragment_epilogue(compiler_context *ctx, unsigned rt, unsigned sample_iter)
{
   /* Loop to ourselves */
   midgard_instruction *br = ctx->writeout_branch[rt][sample_iter];
   struct midgard_instruction ins = v_branch(false, false);
   ins.writeout = br->writeout;
   ins.branch.target_block = ctx->block_count - 1;
   ins.constants.u32[0] = br->constants.u32[0];
   memcpy(&ins.src_types, &br->src_types, sizeof(ins.src_types));
   emit_mir_instruction(ctx, ins);

   ctx->current_block->epilogue = true;
   schedule_barrier(ctx);
   return ins.branch.target_block;
}

static midgard_block *
emit_block_init(compiler_context *ctx)
{
   midgard_block *this_block = ctx->after_block;
   ctx->after_block = NULL;

   if (!this_block)
      this_block = create_empty_block(ctx);

   list_addtail(&this_block->base.link, &ctx->blocks);

   this_block->scheduled = false;
   ++ctx->block_count;

   /* Set up current block */
   list_inithead(&this_block->base.instructions);
   ctx->current_block = this_block;

   return this_block;
}

static midgard_block *
emit_block(compiler_context *ctx, nir_block *block)
{
   midgard_block *this_block = emit_block_init(ctx);

   nir_foreach_instr(instr, block) {
      emit_instr(ctx, instr);
      ++ctx->instruction_count;
   }

   return this_block;
}

static midgard_block *emit_cf_list(struct compiler_context *ctx,
                                   struct exec_list *list);

static void
emit_if(struct compiler_context *ctx, nir_if *nif)
{
   midgard_block *before_block = ctx->current_block;

   /* Speculatively emit the branch, but we can't fill it in until later */
   EMIT(branch, true, true);
   midgard_instruction *then_branch = mir_last_in_block(ctx->current_block);
   then_branch->src[0] = nir_src_index(ctx, &nif->condition);
   then_branch->src_types[0] = nir_type_uint32;

   /* Emit the two subblocks. */
   midgard_block *then_block = emit_cf_list(ctx, &nif->then_list);
   midgard_block *end_then_block = ctx->current_block;

   /* Emit a jump from the end of the then block to the end of the else */
   EMIT(branch, false, false);
   midgard_instruction *then_exit = mir_last_in_block(ctx->current_block);

   /* Emit second block, and check if it's empty */

   int else_idx = ctx->block_count;
   int count_in = ctx->instruction_count;
   midgard_block *else_block = emit_cf_list(ctx, &nif->else_list);
   midgard_block *end_else_block = ctx->current_block;
   int after_else_idx = ctx->block_count;

   /* Now that we have the subblocks emitted, fix up the branches */

   assert(then_block);
   assert(else_block);

   if (ctx->instruction_count == count_in) {
      /* The else block is empty, so don't emit an exit jump */
      mir_remove_instruction(then_exit);
      then_branch->branch.target_block = after_else_idx;
   } else {
      then_branch->branch.target_block = else_idx;
      then_exit->branch.target_block = after_else_idx;
   }

   /* Wire up the successors */

   ctx->after_block = create_empty_block(ctx);

   pan_block_add_successor(&before_block->base, &then_block->base);
   pan_block_add_successor(&before_block->base, &else_block->base);

   pan_block_add_successor(&end_then_block->base, &ctx->after_block->base);
   pan_block_add_successor(&end_else_block->base, &ctx->after_block->base);
}

static void
emit_loop(struct compiler_context *ctx, nir_loop *nloop)
{
   assert(!nir_loop_has_continue_construct(nloop));

   /* Remember where we are */
   midgard_block *start_block = ctx->current_block;

   /* Allocate a loop number, growing the current inner loop depth */
   int loop_idx = ++ctx->current_loop_depth;

   /* Get index from before the body so we can loop back later */
   int start_idx = ctx->block_count;

   /* Emit the body itself */
   midgard_block *loop_block = emit_cf_list(ctx, &nloop->body);

   /* Branch back to loop back */
   struct midgard_instruction br_back = v_branch(false, false);
   br_back.branch.target_block = start_idx;
   emit_mir_instruction(ctx, br_back);

   /* Mark down that branch in the graph. */
   pan_block_add_successor(&start_block->base, &loop_block->base);
   pan_block_add_successor(&ctx->current_block->base, &loop_block->base);

   /* Find the index of the block about to follow us (note: we don't add
    * one; blocks are 0-indexed so we get a fencepost problem) */
   int break_block_idx = ctx->block_count;

   /* Fix up the break statements we emitted to point to the right place,
    * now that we can allocate a block number for them */
   ctx->after_block = create_empty_block(ctx);

   mir_foreach_block_from(ctx, start_block, _block) {
      mir_foreach_instr_in_block(((midgard_block *)_block), ins) {
         if (ins->type != TAG_ALU_4)
            continue;
         if (!ins->compact_branch)
            continue;

         /* We found a branch -- check the type to see if we need to do anything
          */
         if (ins->branch.target_type != TARGET_BREAK)
            continue;

         /* It's a break! Check if it's our break */
         if (ins->branch.target_break != loop_idx)
            continue;

         /* Okay, cool, we're breaking out of this loop.
          * Rewrite from a break to a goto */

         ins->branch.target_type = TARGET_GOTO;
         ins->branch.target_block = break_block_idx;

         pan_block_add_successor(_block, &ctx->after_block->base);
      }
   }

   /* Now that we've finished emitting the loop, free up the depth again
    * so we play nice with recursion amid nested loops */
   --ctx->current_loop_depth;

   /* Dump loop stats */
   ++ctx->loop_count;
}

static midgard_block *
emit_cf_list(struct compiler_context *ctx, struct exec_list *list)
{
   midgard_block *start_block = NULL;

   foreach_list_typed(nir_cf_node, node, node, list) {
      switch (node->type) {
      case nir_cf_node_block: {
         midgard_block *block = emit_block(ctx, nir_cf_node_as_block(node));

         if (!start_block)
            start_block = block;

         break;
      }

      case nir_cf_node_if:
         emit_if(ctx, nir_cf_node_as_if(node));
         break;

      case nir_cf_node_loop:
         emit_loop(ctx, nir_cf_node_as_loop(node));
         break;

      case nir_cf_node_function:
         assert(0);
         break;
      }
   }

   return start_block;
}

/* Due to lookahead, we need to report the first tag executed in the command
 * stream and in branch targets. An initial block might be empty, so iterate
 * until we find one that 'works' */

unsigned
midgard_get_first_tag_from_block(compiler_context *ctx, unsigned block_idx)
{
   midgard_block *initial_block = mir_get_block(ctx, block_idx);

   mir_foreach_block_from(ctx, initial_block, _v) {
      midgard_block *v = (midgard_block *)_v;
      if (v->quadword_count) {
         midgard_bundle *initial_bundle =
            util_dynarray_element(&v->bundles, midgard_bundle, 0);

         return initial_bundle->tag;
      }
   }

   /* Default to a tag 1 which will break from the shader, in case we jump
    * to the exit block (i.e. `return` in a compute shader) */

   return 1;
}

/* For each fragment writeout instruction, generate a writeout loop to
 * associate with it */

static void
mir_add_writeout_loops(compiler_context *ctx)
{
   for (unsigned rt = 0; rt < ARRAY_SIZE(ctx->writeout_branch); ++rt) {
      for (unsigned s = 0; s < MIDGARD_MAX_SAMPLE_ITER; ++s) {
         midgard_instruction *br = ctx->writeout_branch[rt][s];
         if (!br)
            continue;

         unsigned popped = br->branch.target_block;
         pan_block_add_successor(&(mir_get_block(ctx, popped - 1)->base),
                                 &ctx->current_block->base);
         br->branch.target_block = emit_fragment_epilogue(ctx, rt, s);
         br->branch.target_type = TARGET_GOTO;

         /* If we have more RTs, we'll need to restore back after our
          * loop terminates */
         midgard_instruction *next_br = NULL;

         if ((s + 1) < MIDGARD_MAX_SAMPLE_ITER)
            next_br = ctx->writeout_branch[rt][s + 1];

         if (!next_br && (rt + 1) < ARRAY_SIZE(ctx->writeout_branch))
            next_br = ctx->writeout_branch[rt + 1][0];

         if (next_br) {
            midgard_instruction uncond = v_branch(false, false);
            uncond.branch.target_block = popped;
            uncond.branch.target_type = TARGET_GOTO;
            emit_mir_instruction(ctx, uncond);
            pan_block_add_successor(&ctx->current_block->base,
                                    &(mir_get_block(ctx, popped)->base));
            schedule_barrier(ctx);
         } else {
            /* We're last, so we can terminate here */
            br->last_writeout = true;
         }
      }
   }
}

void
midgard_compile_shader_nir(nir_shader *nir,
                           const struct panfrost_compile_inputs *inputs,
                           struct util_dynarray *binary,
                           struct pan_shader_info *info)
{
   midgard_debug = debug_get_option_midgard_debug();

   /* TODO: Bound against what? */
   compiler_context *ctx = rzalloc(NULL, compiler_context);

   ctx->inputs = inputs;
   ctx->nir = nir;
   ctx->info = info;
   ctx->stage = nir->info.stage;
   ctx->blend_input = ~0;
   ctx->blend_src1 = ~0;
   ctx->quirks = midgard_get_quirks(inputs->gpu_id);

   /* Initialize at a global (not block) level hash tables */

   ctx->ssa_constants = _mesa_hash_table_u64_create(ctx);

   /* Collect varyings after lowering I/O */
   pan_nir_collect_varyings(nir, info);

   /* Optimisation passes */
   optimise_nir(nir, ctx->quirks, inputs->is_blend);

   bool skip_internal = nir->info.internal;
   skip_internal &= !(midgard_debug & MIDGARD_DBG_INTERNAL);

   if (midgard_debug & MIDGARD_DBG_SHADERS && !skip_internal)
      nir_print_shader(nir, stdout);

   info->tls_size = nir->scratch_size;

   nir_foreach_function_with_impl(func, impl, nir) {
      list_inithead(&ctx->blocks);
      ctx->block_count = 0;
      ctx->func = func;

      if (nir->info.outputs_read && !inputs->is_blend) {
         emit_block_init(ctx);

         struct midgard_instruction wait = v_branch(false, false);
         wait.branch.target_type = TARGET_TILEBUF_WAIT;

         emit_mir_instruction(ctx, wait);

         ++ctx->instruction_count;
      }

      emit_cf_list(ctx, &impl->body);
      break; /* TODO: Multi-function shaders */
   }

   /* Per-block lowering before opts */

   mir_foreach_block(ctx, _block) {
      midgard_block *block = (midgard_block *)_block;
      inline_alu_constants(ctx, block);
      embedded_to_inline_constant(ctx, block);
   }
   /* MIR-level optimizations */

   bool progress = false;

   do {
      progress = false;
      progress |= midgard_opt_dead_code_eliminate(ctx);
      progress |= midgard_opt_prop(ctx);

      mir_foreach_block(ctx, _block) {
         midgard_block *block = (midgard_block *)_block;
         progress |= midgard_opt_copy_prop(ctx, block);
         progress |= midgard_opt_combine_projection(ctx, block);
         progress |= midgard_opt_varying_projection(ctx, block);
      }
   } while (progress);

   mir_foreach_block(ctx, _block) {
      midgard_block *block = (midgard_block *)_block;
      midgard_lower_derivatives(ctx, block);
      midgard_legalize_invert(ctx, block);
      midgard_cull_dead_branch(ctx, block);
   }

   if (ctx->stage == MESA_SHADER_FRAGMENT)
      mir_add_writeout_loops(ctx);

   /* Analyze now that the code is known but before scheduling creates
    * pipeline registers which are harder to track */
   mir_analyze_helper_requirements(ctx);

   if (midgard_debug & MIDGARD_DBG_SHADERS && !skip_internal)
      mir_print_shader(ctx);

   /* Schedule! */
   midgard_schedule_program(ctx);
   mir_ra(ctx);

   if (midgard_debug & MIDGARD_DBG_SHADERS && !skip_internal)
      mir_print_shader(ctx);

   /* Analyze after scheduling since this is order-dependent */
   mir_analyze_helper_terminate(ctx);

   /* Emit flat binary from the instruction arrays. Iterate each block in
    * sequence. Save instruction boundaries such that lookahead tags can
    * be assigned easily */

   /* Cache _all_ bundles in source order for lookahead across failed branches */

   int bundle_count = 0;
   mir_foreach_block(ctx, _block) {
      midgard_block *block = (midgard_block *)_block;
      bundle_count += block->bundles.size / sizeof(midgard_bundle);
   }
   midgard_bundle **source_order_bundles =
      malloc(sizeof(midgard_bundle *) * bundle_count);
   int bundle_idx = 0;
   mir_foreach_block(ctx, _block) {
      midgard_block *block = (midgard_block *)_block;
      util_dynarray_foreach(&block->bundles, midgard_bundle, bundle) {
         source_order_bundles[bundle_idx++] = bundle;
      }
   }

   int current_bundle = 0;

   /* Midgard prefetches instruction types, so during emission we
    * need to lookahead. Unless this is the last instruction, in
    * which we return 1. */

   mir_foreach_block(ctx, _block) {
      midgard_block *block = (midgard_block *)_block;
      mir_foreach_bundle_in_block(block, bundle) {
         int lookahead = 1;

         if (!bundle->last_writeout && (current_bundle + 1 < bundle_count))
            lookahead = source_order_bundles[current_bundle + 1]->tag;

         emit_binary_bundle(ctx, block, bundle, binary, lookahead);
         ++current_bundle;
      }

      /* TODO: Free deeper */
      // util_dynarray_fini(&block->instructions);
   }

   free(source_order_bundles);

   /* Report the very first tag executed */
   info->midgard.first_tag = midgard_get_first_tag_from_block(ctx, 0);

   info->ubo_mask = ctx->ubo_mask & ((1 << ctx->nir->info.num_ubos) - 1);

   if (midgard_debug & MIDGARD_DBG_SHADERS && !skip_internal) {
      disassemble_midgard(stdout, binary->data, binary->size, inputs->gpu_id,
                          midgard_debug & MIDGARD_DBG_VERBOSE);
      fflush(stdout);
   }

   /* A shader ending on a 16MB boundary causes INSTR_INVALID_PC faults,
    * workaround by adding some padding to the end of the shader. (The
    * kernel makes sure shader BOs can't cross 16MB boundaries.) */
   if (binary->size)
      memset(util_dynarray_grow(binary, uint8_t, 16), 0, 16);

   if ((midgard_debug & MIDGARD_DBG_SHADERDB || inputs->debug) &&
       !nir->info.internal) {
      unsigned nr_bundles = 0, nr_ins = 0;

      /* Count instructions and bundles */

      mir_foreach_block(ctx, _block) {
         midgard_block *block = (midgard_block *)_block;
         nr_bundles +=
            util_dynarray_num_elements(&block->bundles, midgard_bundle);

         mir_foreach_bundle_in_block(block, bun)
            nr_ins += bun->instruction_count;
      }

      /* Calculate thread count. There are certain cutoffs by
       * register count for thread count */

      unsigned nr_registers = info->work_reg_count;

      unsigned nr_threads = (nr_registers <= 4)   ? 4
                            : (nr_registers <= 8) ? 2
                                                  : 1;

      char *shaderdb = NULL;

      /* Dump stats */

      asprintf(&shaderdb,
               "%s shader: "
               "%u inst, %u bundles, %u quadwords, "
               "%u registers, %u threads, %u loops, "
               "%u:%u spills:fills",
               ctx->inputs->is_blend ? "PAN_SHADER_BLEND"
                                     : gl_shader_stage_name(ctx->stage),
               nr_ins, nr_bundles, ctx->quadword_count, nr_registers,
               nr_threads, ctx->loop_count, ctx->spills, ctx->fills);

      if (midgard_debug & MIDGARD_DBG_SHADERDB)
         fprintf(stderr, "SHADER-DB: %s\n", shaderdb);

      if (inputs->debug)
         util_debug_message(inputs->debug, SHADER_INFO, "%s", shaderdb);

      free(shaderdb);
   }

   _mesa_hash_table_u64_destroy(ctx->ssa_constants);
   ralloc_free(ctx);
}
