/*
 * Copyright (C) 2018 Jonathan Marek <jonathan@marek.ca>
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
 *
 * Authors:
 *    Jonathan Marek <jonathan@marek.ca>
 */

#include "ir2_private.h"

#include "fd2_program.h"
#include "freedreno_util.h"
#include "nir_legacy.h"

static const nir_shader_compiler_options options = {
   .lower_fpow = true,
   .lower_flrp32 = true,
   .lower_fmod = true,
   .lower_fdiv = true,
   .lower_fceil = true,
   .fuse_ffma16 = true,
   .fuse_ffma32 = true,
   .fuse_ffma64 = true,
   /* .fdot_replicates = true, it is replicated, but it makes things worse */
   .lower_all_io_to_temps = true,
   .vertex_id_zero_based = true, /* its not implemented anyway */
   .lower_bitops = true,
   .lower_vector_cmp = true,
   .lower_fdph = true,
   .has_fsub = true,
   .has_isub = true,
   .lower_insert_byte = true,
   .lower_insert_word = true,
   .force_indirect_unrolling = nir_var_all,
   .force_indirect_unrolling_sampler = true,
   .max_unroll_iterations = 32,
};

const nir_shader_compiler_options *
ir2_get_compiler_options(void)
{
   return &options;
}

#define OPT(nir, pass, ...)                                                    \
   ({                                                                          \
      bool this_progress = false;                                              \
      NIR_PASS(this_progress, nir, pass, ##__VA_ARGS__);                       \
      this_progress;                                                           \
   })
#define OPT_V(nir, pass, ...) NIR_PASS_V(nir, pass, ##__VA_ARGS__)

static void
ir2_optimize_loop(nir_shader *s)
{
   bool progress;
   do {
      progress = false;

      OPT_V(s, nir_lower_vars_to_ssa);
      progress |= OPT(s, nir_opt_copy_prop_vars);
      progress |= OPT(s, nir_copy_prop);
      progress |= OPT(s, nir_opt_dce);
      progress |= OPT(s, nir_opt_cse);
      /* progress |= OPT(s, nir_opt_gcm, true); */
      progress |= OPT(s, nir_opt_peephole_select, UINT_MAX, true, true);
      progress |= OPT(s, nir_opt_intrinsics);
      progress |= OPT(s, nir_opt_algebraic);
      progress |= OPT(s, nir_opt_constant_folding);
      progress |= OPT(s, nir_opt_dead_cf);
      if (OPT(s, nir_opt_loop)) {
         progress |= true;
         /* If nir_opt_loop makes progress, then we need to clean
          * things up if we want any hope of nir_opt_if or nir_opt_loop_unroll
          * to make progress.
          */
         OPT(s, nir_copy_prop);
         OPT(s, nir_opt_dce);
      }
      progress |= OPT(s, nir_opt_loop_unroll);
      progress |= OPT(s, nir_opt_if, nir_opt_if_optimize_phi_true_false);
      progress |= OPT(s, nir_opt_remove_phis);
      progress |= OPT(s, nir_opt_undef);

   } while (progress);
}

/* trig workarounds is the same as ir3.. but we don't want to include ir3 */
bool ir3_nir_apply_trig_workarounds(nir_shader *shader);

int
ir2_optimize_nir(nir_shader *s, bool lower)
{
   struct nir_lower_tex_options tex_options = {
      .lower_txp = ~0u,
      .lower_rect = 0,
      .lower_invalid_implicit_lod = true,
   };

   if (FD_DBG(DISASM)) {
      debug_printf("----------------------\n");
      nir_print_shader(s, stdout);
      debug_printf("----------------------\n");
   }

   OPT_V(s, nir_lower_vars_to_ssa);
   OPT_V(s, nir_lower_indirect_derefs, nir_var_shader_in | nir_var_shader_out,
         UINT32_MAX);

   if (lower) {
      OPT_V(s, ir3_nir_apply_trig_workarounds);
      OPT_V(s, nir_lower_tex, &tex_options);
   }

   ir2_optimize_loop(s);

   OPT_V(s, nir_remove_dead_variables, nir_var_function_temp, NULL);
   OPT_V(s, nir_opt_sink, nir_move_const_undef);

   /* TODO we dont want to get shaders writing to depth for depth textures */
   if (s->info.stage == MESA_SHADER_FRAGMENT) {
      nir_foreach_shader_out_variable (var, s) {
         if (var->data.location == FRAG_RESULT_DEPTH)
            return -1;
      }
   }

   return 0;
}

static struct ir2_src
load_const(struct ir2_context *ctx, float *value_f, unsigned ncomp)
{
   struct fd2_shader_stateobj *so = ctx->so;
   unsigned imm_ncomp, swiz, idx, i, j;
   uint32_t *value = (uint32_t *)value_f;

   /* try to merge with existing immediate (TODO: try with neg) */
   for (idx = 0; idx < so->num_immediates; idx++) {
      swiz = 0;
      imm_ncomp = so->immediates[idx].ncomp;
      for (i = 0; i < ncomp; i++) {
         for (j = 0; j < imm_ncomp; j++) {
            if (value[i] == so->immediates[idx].val[j])
               break;
         }
         if (j == imm_ncomp) {
            if (j == 4)
               break;
            so->immediates[idx].val[imm_ncomp++] = value[i];
         }
         swiz |= swiz_set(j, i);
      }
      /* matched all components */
      if (i == ncomp)
         break;
   }

   /* need to allocate new immediate */
   if (idx == so->num_immediates) {
      swiz = 0;
      imm_ncomp = 0;
      for (i = 0; i < ncomp; i++) {
         for (j = 0; j < imm_ncomp; j++) {
            if (value[i] == ctx->so->immediates[idx].val[j])
               break;
         }
         if (j == imm_ncomp) {
            so->immediates[idx].val[imm_ncomp++] = value[i];
         }
         swiz |= swiz_set(j, i);
      }
      so->num_immediates++;
   }
   so->immediates[idx].ncomp = imm_ncomp;

   if (ncomp == 1)
      swiz = swiz_merge(swiz, IR2_SWIZZLE_XXXX);

   return ir2_src(so->first_immediate + idx, swiz, IR2_SRC_CONST);
}

struct ir2_src
ir2_zero(struct ir2_context *ctx)
{
   return load_const(ctx, (float[]){0.0f}, 1);
}

static void
update_range(struct ir2_context *ctx, struct ir2_reg *reg)
{
   if (!reg->initialized) {
      reg->initialized = true;
      reg->loop_depth = ctx->loop_depth;
   }

   if (ctx->loop_depth > reg->loop_depth) {
      reg->block_idx_free = ctx->loop_last_block[reg->loop_depth + 1];
   } else {
      reg->loop_depth = ctx->loop_depth;
      reg->block_idx_free = -1;
   }

   /* for regs we want to free at the end of the loop in any case
    * XXX dont do this for ssa
    */
   if (reg->loop_depth)
      reg->block_idx_free = ctx->loop_last_block[reg->loop_depth];
}

static struct ir2_src
make_legacy_src(struct ir2_context *ctx, nir_legacy_src src)
{
   struct ir2_src res = {};
   struct ir2_reg *reg;

   /* Handle constants specially */
   if (src.is_ssa) {
      nir_const_value *const_value =
         nir_src_as_const_value(nir_src_for_ssa(src.ssa));

      if (const_value) {
         float c[src.ssa->num_components];
         nir_const_value_to_array(c, const_value, src.ssa->num_components, f32);
         return load_const(ctx, c, src.ssa->num_components);
      }
   }

   /* Otherwise translate the SSA def or register */
   if (!src.is_ssa) {
      res.num = src.reg.handle->index;
      res.type = IR2_SRC_REG;
      reg = &ctx->reg[res.num];
   } else {
      assert(ctx->ssa_map[src.ssa->index] >= 0);
      res.num = ctx->ssa_map[src.ssa->index];
      res.type = IR2_SRC_SSA;
      reg = &ctx->instr[res.num].ssa;
   }

   update_range(ctx, reg);
   return res;
}

static struct ir2_src
make_src(struct ir2_context *ctx, nir_src src)
{
   return make_legacy_src(ctx, nir_legacy_chase_src(&src));
}

static void
set_legacy_index(struct ir2_context *ctx, nir_legacy_dest dst,
                 struct ir2_instr *instr)
{
   struct ir2_reg *reg = &instr->ssa;

   if (dst.is_ssa) {
      ctx->ssa_map[dst.ssa->index] = instr->idx;
   } else {
      reg = &ctx->reg[dst.reg.handle->index];

      instr->is_ssa = false;
      instr->reg = reg;
   }
   update_range(ctx, reg);
}

static void
set_index(struct ir2_context *ctx, nir_def *def, struct ir2_instr *instr)
{
   set_legacy_index(ctx, nir_legacy_chase_dest(def), instr);
}

static struct ir2_instr *
ir2_instr_create(struct ir2_context *ctx, int type)
{
   struct ir2_instr *instr;

   instr = &ctx->instr[ctx->instr_count++];
   instr->idx = ctx->instr_count - 1;
   instr->type = type;
   instr->block_idx = ctx->block_idx;
   instr->pred = ctx->pred;
   instr->is_ssa = true;
   return instr;
}

static struct ir2_instr *
instr_create_alu(struct ir2_context *ctx, nir_op opcode, unsigned ncomp)
{
   /* emit_alu will fixup instrs that don't map directly */
   static const struct ir2_opc {
      int8_t scalar, vector;
   } nir_ir2_opc[nir_num_opcodes + 1] = {
      [0 ... nir_num_opcodes - 1] = {-1, -1},

      [nir_op_mov] = {MAXs, MAXv},
      [nir_op_fneg] = {MAXs, MAXv},
      [nir_op_fabs] = {MAXs, MAXv},
      [nir_op_fsat] = {MAXs, MAXv},
      [nir_op_fsign] = {-1, CNDGTEv},
      [nir_op_fadd] = {ADDs, ADDv},
      [nir_op_fsub] = {ADDs, ADDv},
      [nir_op_fmul] = {MULs, MULv},
      [nir_op_ffma] = {-1, MULADDv},
      [nir_op_fmax] = {MAXs, MAXv},
      [nir_op_fmin] = {MINs, MINv},
      [nir_op_ffloor] = {FLOORs, FLOORv},
      [nir_op_ffract] = {FRACs, FRACv},
      [nir_op_ftrunc] = {TRUNCs, TRUNCv},
      [nir_op_fdot2] = {-1, DOT2ADDv},
      [nir_op_fdot3] = {-1, DOT3v},
      [nir_op_fdot4] = {-1, DOT4v},
      [nir_op_sge] = {-1, SETGTEv},
      [nir_op_slt] = {-1, SETGTv},
      [nir_op_sne] = {-1, SETNEv},
      [nir_op_seq] = {-1, SETEv},
      [nir_op_fcsel] = {-1, CNDEv},
      [nir_op_frsq] = {RECIPSQ_IEEE, -1},
      [nir_op_frcp] = {RECIP_IEEE, -1},
      [nir_op_flog2] = {LOG_IEEE, -1},
      [nir_op_fexp2] = {EXP_IEEE, -1},
      [nir_op_fsqrt] = {SQRT_IEEE, -1},
      [nir_op_fcos] = {COS, -1},
      [nir_op_fsin] = {SIN, -1},
   /* no fsat, fneg, fabs since source mods deal with those */

   /* so we can use this function with non-nir op */
#define ir2_op_cube nir_num_opcodes
      [ir2_op_cube] = {-1, CUBEv},
   };

   struct ir2_opc op = nir_ir2_opc[opcode];
   assert(op.vector >= 0 || op.scalar >= 0);

   struct ir2_instr *instr = ir2_instr_create(ctx, IR2_ALU);
   instr->alu.vector_opc = op.vector;
   instr->alu.scalar_opc = op.scalar;
   instr->alu.export = -1;
   instr->alu.write_mask = (1 << ncomp) - 1;
   instr->src_count =
      opcode == ir2_op_cube ? 2 : nir_op_infos[opcode].num_inputs;
   instr->ssa.ncomp = ncomp;
   return instr;
}

static struct ir2_instr *
instr_create_alu_reg(struct ir2_context *ctx, nir_op opcode, uint8_t write_mask,
                     struct ir2_instr *share_reg)
{
   struct ir2_instr *instr;
   struct ir2_reg *reg;

   reg = share_reg ? share_reg->reg : &ctx->reg[ctx->reg_count++];
   reg->ncomp = MAX2(reg->ncomp, util_logbase2(write_mask) + 1);

   instr = instr_create_alu(ctx, opcode, util_bitcount(write_mask));
   instr->alu.write_mask = write_mask;
   instr->reg = reg;
   instr->is_ssa = false;
   return instr;
}

static struct ir2_instr *
instr_create_alu_dest(struct ir2_context *ctx, nir_op opcode, nir_def *def)
{
   struct ir2_instr *instr;
   instr = instr_create_alu(ctx, opcode, def->num_components);
   set_index(ctx, def, instr);
   return instr;
}

static struct ir2_instr *
ir2_instr_create_fetch(struct ir2_context *ctx, nir_def *def,
                       instr_fetch_opc_t opc)
{
   struct ir2_instr *instr = ir2_instr_create(ctx, IR2_FETCH);
   instr->fetch.opc = opc;
   instr->src_count = 1;
   instr->ssa.ncomp = def->num_components;
   set_index(ctx, def, instr);
   return instr;
}

static struct ir2_src
make_src_noconst(struct ir2_context *ctx, nir_src src)
{
   struct ir2_instr *instr;

   if (nir_src_as_const_value(src)) {
      instr = instr_create_alu(ctx, nir_op_mov, src.ssa->num_components);
      instr->src[0] = make_src(ctx, src);
      return ir2_src(instr->idx, 0, IR2_SRC_SSA);
   }

   return make_src(ctx, src);
}

static void
emit_alu(struct ir2_context *ctx, nir_alu_instr *alu)
{
   const nir_op_info *info = &nir_op_infos[alu->op];
   nir_def *def = &alu->def;
   struct ir2_instr *instr;
   struct ir2_src tmp;
   unsigned ncomp;

   /* Don't emit modifiers that are totally folded */
   if (((alu->op == nir_op_fneg) || (alu->op == nir_op_fabs)) &&
       nir_legacy_float_mod_folds(alu))
      return;

   if ((alu->op == nir_op_fsat) && nir_legacy_fsat_folds(alu))
      return;

   /* get the number of dst components */
   ncomp = def->num_components;

   instr = instr_create_alu(ctx, alu->op, ncomp);

   nir_legacy_alu_dest legacy_dest =
      nir_legacy_chase_alu_dest(&alu->def);
   set_legacy_index(ctx, legacy_dest.dest, instr);
   instr->alu.saturate = legacy_dest.fsat;
   instr->alu.write_mask = legacy_dest.write_mask;

   for (int i = 0; i < info->num_inputs; i++) {
      nir_alu_src *src = &alu->src[i];

      /* compress swizzle with writemask when applicable */
      unsigned swiz = 0, j = 0;
      for (int i = 0; i < 4; i++) {
         if (!(legacy_dest.write_mask & 1 << i) && !info->output_size)
            continue;
         swiz |= swiz_set(src->swizzle[i], j++);
      }

      nir_legacy_alu_src legacy_src =
         nir_legacy_chase_alu_src(src, true /* fuse_abs */);

      instr->src[i] = make_legacy_src(ctx, legacy_src.src);
      instr->src[i].swizzle = swiz_merge(instr->src[i].swizzle, swiz);
      instr->src[i].negate = legacy_src.fneg;
      instr->src[i].abs = legacy_src.fabs;
   }

   /* workarounds for NIR ops that don't map directly to a2xx ops */
   switch (alu->op) {
   case nir_op_fneg:
      instr->src[0].negate = 1;
      break;
   case nir_op_fabs:
      instr->src[0].abs = 1;
      break;
   case nir_op_fsat:
      instr->alu.saturate = 1;
      break;
   case nir_op_slt:
      tmp = instr->src[0];
      instr->src[0] = instr->src[1];
      instr->src[1] = tmp;
      break;
   case nir_op_fcsel:
      tmp = instr->src[1];
      instr->src[1] = instr->src[2];
      instr->src[2] = tmp;
      break;
   case nir_op_fsub:
      instr->src[1].negate = !instr->src[1].negate;
      break;
   case nir_op_fdot2:
      instr->src_count = 3;
      instr->src[2] = ir2_zero(ctx);
      break;
   case nir_op_fsign: {
      /* we need an extra instruction to deal with the zero case */
      struct ir2_instr *tmp;

      /* tmp = x == 0 ? 0 : 1 */
      tmp = instr_create_alu(ctx, nir_op_fcsel, ncomp);
      tmp->src[0] = instr->src[0];
      tmp->src[1] = ir2_zero(ctx);
      tmp->src[2] = load_const(ctx, (float[]){1.0f}, 1);

      /* result = x >= 0 ? tmp : -tmp */
      instr->src[1] = ir2_src(tmp->idx, 0, IR2_SRC_SSA);
      instr->src[2] = instr->src[1];
      instr->src[2].negate = true;
      instr->src_count = 3;
   } break;
   default:
      break;
   }
}

static void
load_input(struct ir2_context *ctx, nir_def *def, unsigned idx)
{
   struct ir2_instr *instr;
   int slot = -1;

   if (ctx->so->type == MESA_SHADER_VERTEX) {
      instr = ir2_instr_create_fetch(ctx, def, 0);
      instr->src[0] = ir2_src(0, 0, IR2_SRC_INPUT);
      instr->fetch.vtx.const_idx = 20 + (idx / 3);
      instr->fetch.vtx.const_idx_sel = idx % 3;
      return;
   }

   /* get slot from idx */
   nir_foreach_shader_in_variable (var, ctx->nir) {
      if (var->data.driver_location == idx) {
         slot = var->data.location;
         break;
      }
   }
   assert(slot >= 0);

   switch (slot) {
   case VARYING_SLOT_POS:
      /* need to extract xy with abs and add tile offset on a20x
       * zw from fragcoord input (w inverted in fragment shader)
       * TODO: only components that are required by fragment shader
       */
      instr = instr_create_alu_reg(
         ctx, ctx->so->is_a20x ? nir_op_fadd : nir_op_mov, 3, NULL);
      instr->src[0] = ir2_src(ctx->f->inputs_count, 0, IR2_SRC_INPUT);
      instr->src[0].abs = true;
      /* on a20x, C64 contains the tile offset */
      instr->src[1] = ir2_src(64, 0, IR2_SRC_CONST);

      instr = instr_create_alu_reg(ctx, nir_op_mov, 4, instr);
      instr->src[0] = ir2_src(ctx->f->fragcoord, 0, IR2_SRC_INPUT);

      instr = instr_create_alu_reg(ctx, nir_op_frcp, 8, instr);
      instr->src[0] = ir2_src(ctx->f->fragcoord, IR2_SWIZZLE_Y, IR2_SRC_INPUT);

      unsigned reg_idx = instr->reg - ctx->reg; /* XXX */
      instr = instr_create_alu_dest(ctx, nir_op_mov, def);
      instr->src[0] = ir2_src(reg_idx, 0, IR2_SRC_REG);
      break;
   default:
      instr = instr_create_alu_dest(ctx, nir_op_mov, def);
      instr->src[0] = ir2_src(idx, 0, IR2_SRC_INPUT);
      break;
   }
}

static unsigned
output_slot(struct ir2_context *ctx, nir_intrinsic_instr *intr)
{
   int slot = -1;
   unsigned idx = nir_intrinsic_base(intr);
   nir_foreach_shader_out_variable (var, ctx->nir) {
      if (var->data.driver_location == idx) {
         slot = var->data.location;
         break;
      }
   }
   assert(slot != -1);
   return slot;
}

static void
store_output(struct ir2_context *ctx, nir_src src, unsigned slot,
             unsigned ncomp)
{
   struct ir2_instr *instr;
   unsigned idx = 0;

   if (ctx->so->type == MESA_SHADER_VERTEX) {
      switch (slot) {
      case VARYING_SLOT_POS:
         ctx->position = make_src(ctx, src);
         idx = 62;
         break;
      case VARYING_SLOT_PSIZ:
         ctx->so->writes_psize = true;
         idx = 63;
         break;
      default:
         /* find matching slot from fragment shader input */
         for (idx = 0; idx < ctx->f->inputs_count; idx++)
            if (ctx->f->inputs[idx].slot == slot)
               break;
         if (idx == ctx->f->inputs_count)
            return;
      }
   } else if (slot != FRAG_RESULT_COLOR && slot != FRAG_RESULT_DATA0) {
      /* only color output is implemented */
      return;
   }

   instr = instr_create_alu(ctx, nir_op_mov, ncomp);
   instr->src[0] = make_src(ctx, src);
   instr->alu.export = idx;
}

static void
emit_intrinsic(struct ir2_context *ctx, nir_intrinsic_instr *intr)
{
   struct ir2_instr *instr;
   ASSERTED nir_const_value *const_offset;
   unsigned idx;

   switch (intr->intrinsic) {
   case nir_intrinsic_decl_reg:
   case nir_intrinsic_load_reg:
   case nir_intrinsic_store_reg:
      /* Nothing to do for these */
      break;

   case nir_intrinsic_load_input:
      load_input(ctx, &intr->def, nir_intrinsic_base(intr));
      break;
   case nir_intrinsic_store_output:
      store_output(ctx, intr->src[0], output_slot(ctx, intr),
                   intr->num_components);
      break;
   case nir_intrinsic_load_uniform:
      const_offset = nir_src_as_const_value(intr->src[0]);
      assert(const_offset); /* TODO can be false in ES2? */
      idx = nir_intrinsic_base(intr);
      idx += (uint32_t)const_offset[0].f32;
      instr = instr_create_alu_dest(ctx, nir_op_mov, &intr->def);
      instr->src[0] = ir2_src(idx, 0, IR2_SRC_CONST);
      break;
   case nir_intrinsic_discard:
   case nir_intrinsic_discard_if:
      instr = ir2_instr_create(ctx, IR2_ALU);
      instr->alu.vector_opc = VECTOR_NONE;
      if (intr->intrinsic == nir_intrinsic_discard_if) {
         instr->alu.scalar_opc = KILLNEs;
         instr->src[0] = make_src(ctx, intr->src[0]);
      } else {
         instr->alu.scalar_opc = KILLEs;
         instr->src[0] = ir2_zero(ctx);
      }
      instr->alu.export = -1;
      instr->src_count = 1;
      ctx->so->has_kill = true;
      break;
   case nir_intrinsic_load_front_face:
      /* gl_FrontFacing is in the sign of param.x
       * rcp required because otherwise we can't differentiate -0.0 and +0.0
       */
      ctx->so->need_param = true;

      struct ir2_instr *tmp = instr_create_alu(ctx, nir_op_frcp, 1);
      tmp->src[0] = ir2_src(ctx->f->inputs_count, 0, IR2_SRC_INPUT);

      instr = instr_create_alu_dest(ctx, nir_op_sge, &intr->def);
      instr->src[0] = ir2_src(tmp->idx, 0, IR2_SRC_SSA);
      instr->src[1] = ir2_zero(ctx);
      break;
   case nir_intrinsic_load_point_coord:
      /* param.zw (note: abs might be needed like fragcoord in param.xy?) */
      ctx->so->need_param = true;

      instr = instr_create_alu_dest(ctx, nir_op_mov, &intr->def);
      instr->src[0] =
         ir2_src(ctx->f->inputs_count, IR2_SWIZZLE_ZW, IR2_SRC_INPUT);
      break;
   default:
      compile_error(ctx, "unimplemented intr %d\n", intr->intrinsic);
      break;
   }
}

static void
emit_tex(struct ir2_context *ctx, nir_tex_instr *tex)
{
   bool is_rect = false, is_cube = false;
   struct ir2_instr *instr;
   nir_src *coord, *lod_bias;

   coord = lod_bias = NULL;

   for (unsigned i = 0; i < tex->num_srcs; i++) {
      switch (tex->src[i].src_type) {
      case nir_tex_src_coord:
         coord = &tex->src[i].src;
         break;
      case nir_tex_src_bias:
      case nir_tex_src_lod:
         assert(!lod_bias);
         lod_bias = &tex->src[i].src;
         break;
      default:
         compile_error(ctx, "Unhandled NIR tex src type: %d\n",
                       tex->src[i].src_type);
         return;
      }
   }

   switch (tex->op) {
   case nir_texop_tex:
   case nir_texop_txb:
   case nir_texop_txl:
      break;
   default:
      compile_error(ctx, "unimplemented texop %d\n", tex->op);
      return;
   }

   switch (tex->sampler_dim) {
   case GLSL_SAMPLER_DIM_2D:
   case GLSL_SAMPLER_DIM_EXTERNAL:
      break;
   case GLSL_SAMPLER_DIM_RECT:
      is_rect = true;
      break;
   case GLSL_SAMPLER_DIM_CUBE:
      is_cube = true;
      break;
   default:
      compile_error(ctx, "unimplemented sampler %d\n", tex->sampler_dim);
      return;
   }

   struct ir2_src src_coord = make_src_noconst(ctx, *coord);

   /* for cube maps
    * tmp = cube(coord)
    * tmp.xy = tmp.xy / |tmp.z| + 1.5
    * coord = tmp.xyw
    */
   if (is_cube) {
      struct ir2_instr *rcp, *coord_xy;
      unsigned reg_idx;

      instr = instr_create_alu_reg(ctx, ir2_op_cube, 15, NULL);
      instr->src[0] = src_coord;
      instr->src[0].swizzle = IR2_SWIZZLE_ZZXY;
      instr->src[1] = src_coord;
      instr->src[1].swizzle = IR2_SWIZZLE_YXZZ;

      reg_idx = instr->reg - ctx->reg; /* hacky */

      rcp = instr_create_alu(ctx, nir_op_frcp, 1);
      rcp->src[0] = ir2_src(reg_idx, IR2_SWIZZLE_Z, IR2_SRC_REG);
      rcp->src[0].abs = true;

      coord_xy = instr_create_alu_reg(ctx, nir_op_ffma, 3, instr);
      coord_xy->src[0] = ir2_src(reg_idx, 0, IR2_SRC_REG);
      coord_xy->src[1] = ir2_src(rcp->idx, IR2_SWIZZLE_XXXX, IR2_SRC_SSA);
      coord_xy->src[2] = load_const(ctx, (float[]){1.5f}, 1);

      src_coord = ir2_src(reg_idx, 0, IR2_SRC_REG);
      /* TODO: lod/bias transformed by src_coord.z ? */
   }

   instr = ir2_instr_create_fetch(ctx, &tex->def, TEX_FETCH);
   instr->src[0] = src_coord;
   instr->src[0].swizzle = is_cube ? IR2_SWIZZLE_YXW : 0;
   instr->fetch.tex.is_cube = is_cube;
   instr->fetch.tex.is_rect = is_rect;
   instr->fetch.tex.samp_id = tex->sampler_index;

   /* for lod/bias, we insert an extra src for the backend to deal with */
   if (lod_bias) {
      instr->src[1] = make_src_noconst(ctx, *lod_bias);
      /* backend will use 2-3 components so apply swizzle */
      swiz_merge_p(&instr->src[1].swizzle, IR2_SWIZZLE_XXXX);
      instr->src_count = 2;
   }
}

static void
setup_input(struct ir2_context *ctx, nir_variable *in)
{
   struct fd2_shader_stateobj *so = ctx->so;
   unsigned n = in->data.driver_location;
   unsigned slot = in->data.location;

   assert(glsl_type_is_vector_or_scalar(in->type) ||
          glsl_type_is_unsized_array(in->type));

   /* handle later */
   if (ctx->so->type == MESA_SHADER_VERTEX)
      return;

   if (ctx->so->type != MESA_SHADER_FRAGMENT)
      compile_error(ctx, "unknown shader type: %d\n", ctx->so->type);

   n = ctx->f->inputs_count++;

   /* half of fragcoord from param reg, half from a varying */
   if (slot == VARYING_SLOT_POS) {
      ctx->f->fragcoord = n;
      so->need_param = true;
   }

   ctx->f->inputs[n].slot = slot;
   ctx->f->inputs[n].ncomp = glsl_get_components(in->type);

   /* in->data.interpolation?
    * opengl ES 2.0 can't do flat mode, but we still get it from GALLIUM_HUD
    */
}

static void
emit_undef(struct ir2_context *ctx, nir_undef_instr *undef)
{
   /* TODO we don't want to emit anything for undefs */

   struct ir2_instr *instr;

   instr = instr_create_alu_dest(ctx, nir_op_mov, &undef->def);
   instr->src[0] = ir2_src(0, 0, IR2_SRC_CONST);
}

static void
emit_instr(struct ir2_context *ctx, nir_instr *instr)
{
   switch (instr->type) {
   case nir_instr_type_alu:
      emit_alu(ctx, nir_instr_as_alu(instr));
      break;
   case nir_instr_type_deref:
      /* ignored, handled as part of the intrinsic they are src to */
      break;
   case nir_instr_type_intrinsic:
      emit_intrinsic(ctx, nir_instr_as_intrinsic(instr));
      break;
   case nir_instr_type_load_const:
      /* dealt with when using nir_src */
      break;
   case nir_instr_type_tex:
      emit_tex(ctx, nir_instr_as_tex(instr));
      break;
   case nir_instr_type_jump:
      ctx->block_has_jump[ctx->block_idx] = true;
      break;
   case nir_instr_type_undef:
      emit_undef(ctx, nir_instr_as_undef(instr));
      break;
   default:
      break;
   }
}

/* fragcoord.zw and a20x hw binning outputs */
static void
extra_position_exports(struct ir2_context *ctx, bool binning)
{
   struct ir2_instr *instr, *rcp, *sc, *wincoord, *off;

   if (ctx->f->fragcoord < 0 && !binning)
      return;

   instr = instr_create_alu(ctx, nir_op_fmax, 1);
   instr->src[0] = ctx->position;
   instr->src[0].swizzle = IR2_SWIZZLE_W;
   instr->src[1] = ir2_zero(ctx);

   rcp = instr_create_alu(ctx, nir_op_frcp, 1);
   rcp->src[0] = ir2_src(instr->idx, 0, IR2_SRC_SSA);

   sc = instr_create_alu(ctx, nir_op_fmul, 4);
   sc->src[0] = ctx->position;
   sc->src[1] = ir2_src(rcp->idx, IR2_SWIZZLE_XXXX, IR2_SRC_SSA);

   wincoord = instr_create_alu(ctx, nir_op_ffma, 4);
   wincoord->src[0] = ir2_src(66, 0, IR2_SRC_CONST);
   wincoord->src[1] = ir2_src(sc->idx, 0, IR2_SRC_SSA);
   wincoord->src[2] = ir2_src(65, 0, IR2_SRC_CONST);

   /* fragcoord z/w */
   if (ctx->f->fragcoord >= 0 && !binning) {
      instr = instr_create_alu(ctx, nir_op_mov, 1);
      instr->src[0] = ir2_src(wincoord->idx, IR2_SWIZZLE_Z, IR2_SRC_SSA);
      instr->alu.export = ctx->f->fragcoord;

      instr = instr_create_alu(ctx, nir_op_mov, 1);
      instr->src[0] = ctx->position;
      instr->src[0].swizzle = IR2_SWIZZLE_W;
      instr->alu.export = ctx->f->fragcoord;
      instr->alu.write_mask = 2;
   }

   if (!binning)
      return;

   off = instr_create_alu(ctx, nir_op_fadd, 1);
   off->src[0] = ir2_src(64, 0, IR2_SRC_CONST);
   off->src[1] = ir2_src(2, 0, IR2_SRC_INPUT);

   /* 8 max set in freedreno_screen.. unneeded instrs patched out */
   for (int i = 0; i < 8; i++) {
      instr = instr_create_alu(ctx, nir_op_ffma, 4);
      instr->src[0] = ir2_src(1, IR2_SWIZZLE_WYWW, IR2_SRC_CONST);
      instr->src[1] = ir2_src(off->idx, IR2_SWIZZLE_XXXX, IR2_SRC_SSA);
      instr->src[2] = ir2_src(3 + i, 0, IR2_SRC_CONST);
      instr->alu.export = 32;

      instr = instr_create_alu(ctx, nir_op_ffma, 4);
      instr->src[0] = ir2_src(68 + i * 2, 0, IR2_SRC_CONST);
      instr->src[1] = ir2_src(wincoord->idx, 0, IR2_SRC_SSA);
      instr->src[2] = ir2_src(67 + i * 2, 0, IR2_SRC_CONST);
      instr->alu.export = 33;
   }
}

static bool emit_cf_list(struct ir2_context *ctx, struct exec_list *list);

static bool
emit_block(struct ir2_context *ctx, nir_block *block)
{
   struct ir2_instr *instr;
   nir_block *succs = block->successors[0];

   ctx->block_idx = block->index;

   nir_foreach_instr (instr, block)
      emit_instr(ctx, instr);

   if (!succs || !succs->index)
      return false;

   /* we want to be smart and always jump and have the backend cleanup
    * but we are not, so there are two cases where jump is needed:
    *  loops (succs index lower)
    *  jumps (jump instruction seen in block)
    */
   if (succs->index > block->index && !ctx->block_has_jump[block->index])
      return false;

   assert(block->successors[1] == NULL);

   instr = ir2_instr_create(ctx, IR2_CF);
   instr->cf.block_idx = succs->index;
   /* XXX can't jump to a block with different predicate */
   return true;
}

static void
emit_if(struct ir2_context *ctx, nir_if *nif)
{
   unsigned pred = ctx->pred, pred_idx = ctx->pred_idx;
   struct ir2_instr *instr;

   /* XXX: blob seems to always use same register for condition */

   instr = ir2_instr_create(ctx, IR2_ALU);
   instr->src[0] = make_src(ctx, nif->condition);
   instr->src_count = 1;
   instr->ssa.ncomp = 1;
   instr->alu.vector_opc = VECTOR_NONE;
   instr->alu.scalar_opc = SCALAR_NONE;
   instr->alu.export = -1;
   instr->alu.write_mask = 1;
   instr->pred = 0;

   /* if nested, use PRED_SETNE_PUSHv */
   if (pred) {
      instr->alu.vector_opc = PRED_SETNE_PUSHv;
      instr->src[1] = instr->src[0];
      instr->src[0] = ir2_src(pred_idx, 0, IR2_SRC_SSA);
      instr->src[0].swizzle = IR2_SWIZZLE_XXXX;
      instr->src[1].swizzle = IR2_SWIZZLE_XXXX;
      instr->src_count = 2;
   } else {
      instr->alu.scalar_opc = PRED_SETNEs;
   }

   ctx->pred_idx = instr->idx;
   ctx->pred = 3;

   emit_cf_list(ctx, &nif->then_list);

   /* TODO: if these is no else branch we don't need this
    * and if the else branch is simple, can just flip ctx->pred instead
    */
   instr = ir2_instr_create(ctx, IR2_ALU);
   instr->src[0] = ir2_src(ctx->pred_idx, 0, IR2_SRC_SSA);
   instr->src_count = 1;
   instr->ssa.ncomp = 1;
   instr->alu.vector_opc = VECTOR_NONE;
   instr->alu.scalar_opc = PRED_SET_INVs;
   instr->alu.export = -1;
   instr->alu.write_mask = 1;
   instr->pred = 0;
   ctx->pred_idx = instr->idx;

   emit_cf_list(ctx, &nif->else_list);

   /* restore predicate for nested predicates */
   if (pred) {
      instr = ir2_instr_create(ctx, IR2_ALU);
      instr->src[0] = ir2_src(ctx->pred_idx, 0, IR2_SRC_SSA);
      instr->src_count = 1;
      instr->ssa.ncomp = 1;
      instr->alu.vector_opc = VECTOR_NONE;
      instr->alu.scalar_opc = PRED_SET_POPs;
      instr->alu.export = -1;
      instr->alu.write_mask = 1;
      instr->pred = 0;
      ctx->pred_idx = instr->idx;
   }

   /* restore ctx->pred */
   ctx->pred = pred;
}

/* get the highest block idx in the loop, so we know when
 * we can free registers that are allocated outside the loop
 */
static unsigned
loop_last_block(struct exec_list *list)
{
   nir_cf_node *node =
      exec_node_data(nir_cf_node, exec_list_get_tail(list), node);
   switch (node->type) {
   case nir_cf_node_block:
      return nir_cf_node_as_block(node)->index;
   case nir_cf_node_if:
      assert(0); /* XXX could this ever happen? */
      return 0;
   case nir_cf_node_loop:
      return loop_last_block(&nir_cf_node_as_loop(node)->body);
   default:
      compile_error(ctx, "Not supported\n");
      return 0;
   }
}

static void
emit_loop(struct ir2_context *ctx, nir_loop *nloop)
{
   assert(!nir_loop_has_continue_construct(nloop));
   ctx->loop_last_block[++ctx->loop_depth] = loop_last_block(&nloop->body);
   emit_cf_list(ctx, &nloop->body);
   ctx->loop_depth--;
}

static bool
emit_cf_list(struct ir2_context *ctx, struct exec_list *list)
{
   bool ret = false;
   foreach_list_typed (nir_cf_node, node, node, list) {
      ret = false;
      switch (node->type) {
      case nir_cf_node_block:
         ret = emit_block(ctx, nir_cf_node_as_block(node));
         break;
      case nir_cf_node_if:
         emit_if(ctx, nir_cf_node_as_if(node));
         break;
      case nir_cf_node_loop:
         emit_loop(ctx, nir_cf_node_as_loop(node));
         break;
      case nir_cf_node_function:
         compile_error(ctx, "Not supported\n");
         break;
      }
   }
   return ret;
}

static void
cleanup_binning(struct ir2_context *ctx)
{
   assert(ctx->so->type == MESA_SHADER_VERTEX);

   /* kill non-position outputs for binning variant */
   nir_foreach_block (block, nir_shader_get_entrypoint(ctx->nir)) {
      nir_foreach_instr_safe (instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
         if (intr->intrinsic != nir_intrinsic_store_output)
            continue;

         if (output_slot(ctx, intr) != VARYING_SLOT_POS)
            nir_instr_remove(instr);
      }
   }

   ir2_optimize_nir(ctx->nir, false);
}

static bool
ir2_alu_to_scalar_filter_cb(const nir_instr *instr, const void *data)
{
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(instr);
   switch (alu->op) {
   case nir_op_frsq:
   case nir_op_frcp:
   case nir_op_flog2:
   case nir_op_fexp2:
   case nir_op_fsqrt:
   case nir_op_fcos:
   case nir_op_fsin:
      return true;
   default:
      break;
   }

   return false;
}

void
ir2_nir_compile(struct ir2_context *ctx, bool binning)
{
   struct fd2_shader_stateobj *so = ctx->so;

   memset(ctx->ssa_map, 0xff, sizeof(ctx->ssa_map));

   ctx->nir = nir_shader_clone(NULL, so->nir);

   if (binning)
      cleanup_binning(ctx);

   OPT_V(ctx->nir, nir_copy_prop);
   OPT_V(ctx->nir, nir_opt_dce);
   OPT_V(ctx->nir, nir_opt_move, nir_move_comparisons);

   OPT_V(ctx->nir, nir_lower_int_to_float);
   OPT_V(ctx->nir, nir_lower_bool_to_float, true);
   while (OPT(ctx->nir, nir_opt_algebraic))
      ;
   OPT_V(ctx->nir, nir_opt_algebraic_late);
   OPT_V(ctx->nir, nir_lower_alu_to_scalar, ir2_alu_to_scalar_filter_cb, NULL);

   OPT_V(ctx->nir, nir_convert_from_ssa, true);

   OPT_V(ctx->nir, nir_move_vec_src_uses_to_dest, false);
   OPT_V(ctx->nir, nir_lower_vec_to_regs, NULL, NULL);

   OPT_V(ctx->nir, nir_legacy_trivialize, true);

   OPT_V(ctx->nir, nir_opt_dce);

   nir_sweep(ctx->nir);

   if (FD_DBG(DISASM)) {
      debug_printf("----------------------\n");
      nir_print_shader(ctx->nir, stdout);
      debug_printf("----------------------\n");
   }

   /* fd2_shader_stateobj init */
   if (so->type == MESA_SHADER_FRAGMENT) {
      ctx->f->fragcoord = -1;
      ctx->f->inputs_count = 0;
      memset(ctx->f->inputs, 0, sizeof(ctx->f->inputs));
   }

   /* Setup inputs: */
   nir_foreach_shader_in_variable (in, ctx->nir)
      setup_input(ctx, in);

   if (so->type == MESA_SHADER_FRAGMENT) {
      unsigned idx;
      for (idx = 0; idx < ctx->f->inputs_count; idx++) {
         ctx->input[idx].ncomp = ctx->f->inputs[idx].ncomp;
         update_range(ctx, &ctx->input[idx]);
      }
      /* assume we have param input and kill it later if not */
      ctx->input[idx].ncomp = 4;
      update_range(ctx, &ctx->input[idx]);
   } else {
      ctx->input[0].ncomp = 1;
      ctx->input[2].ncomp = 1;
      update_range(ctx, &ctx->input[0]);
      update_range(ctx, &ctx->input[2]);
   }

   /* And emit the body: */
   nir_function_impl *fxn = nir_shader_get_entrypoint(ctx->nir);

   nir_foreach_reg_decl (decl, fxn) {
      assert(decl->def.index < ARRAY_SIZE(ctx->reg));
      ctx->reg[decl->def.index].ncomp = nir_intrinsic_num_components(decl);
      ctx->reg_count = MAX2(ctx->reg_count, decl->def.index + 1);
   }

   nir_metadata_require(fxn, nir_metadata_block_index);
   emit_cf_list(ctx, &fxn->body);
   /* TODO emit_block(ctx, fxn->end_block); */

   if (so->type == MESA_SHADER_VERTEX)
      extra_position_exports(ctx, binning);

   ralloc_free(ctx->nir);

   /* kill unused param input */
   if (so->type == MESA_SHADER_FRAGMENT && !so->need_param)
      ctx->input[ctx->f->inputs_count].initialized = false;
}
