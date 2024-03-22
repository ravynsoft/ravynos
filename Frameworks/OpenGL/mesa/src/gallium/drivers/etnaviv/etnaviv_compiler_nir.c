/*
 * Copyright (c) 2012-2019 Etnaviv Project
 * Copyright (c) 2019 Zodiac Inflight Innovations
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Jonathan Marek <jonathan@marek.ca>
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 */

#include "etnaviv_compiler.h"
#include "etnaviv_compiler_nir.h"
#include "etnaviv_asm.h"
#include "etnaviv_context.h"
#include "etnaviv_debug.h"
#include "etnaviv_nir.h"
#include "etnaviv_uniforms.h"
#include "etnaviv_util.h"
#include "nir.h"

#include <math.h>
#include "util/u_memory.h"
#include "util/register_allocate.h"
#include "compiler/nir/nir_builder.h"

#include "util/compiler.h"
#include "util/half_float.h"

static bool
etna_alu_to_scalar_filter_cb(const nir_instr *instr, const void *data)
{
   const struct etna_specs *specs = data;

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
   case nir_op_fdiv:
   case nir_op_imul:
      return true;
   /* TODO: can do better than alu_to_scalar for vector compares */
   case nir_op_b32all_fequal2:
   case nir_op_b32all_fequal3:
   case nir_op_b32all_fequal4:
   case nir_op_b32any_fnequal2:
   case nir_op_b32any_fnequal3:
   case nir_op_b32any_fnequal4:
   case nir_op_b32all_iequal2:
   case nir_op_b32all_iequal3:
   case nir_op_b32all_iequal4:
   case nir_op_b32any_inequal2:
   case nir_op_b32any_inequal3:
   case nir_op_b32any_inequal4:
      return true;
   case nir_op_fdot2:
      if (!specs->has_halti2_instructions)
         return true;
      break;
   default:
      break;
   }

   return false;
}

static void
etna_emit_block_start(struct etna_compile *c, unsigned block)
{
   c->block_ptr[block] = c->inst_ptr;
}

static void
etna_emit_output(struct etna_compile *c, nir_variable *var, struct etna_inst_src src)
{
   struct etna_shader_io_file *sf = &c->variant->outfile;

   if (is_fs(c)) {
      switch (var->data.location) {
      case FRAG_RESULT_COLOR:
      case FRAG_RESULT_DATA0: /* DATA0 is used by gallium shaders for color */
         c->variant->ps_color_out_reg = src.reg;
         break;
      case FRAG_RESULT_DEPTH:
         c->variant->ps_depth_out_reg = src.reg;
         break;
      default:
         unreachable("Unsupported fs output");
      }
      return;
   }

   switch (var->data.location) {
   case VARYING_SLOT_POS:
      c->variant->vs_pos_out_reg = src.reg;
      break;
   case VARYING_SLOT_PSIZ:
      c->variant->vs_pointsize_out_reg = src.reg;
      break;
   default:
      assert(sf->num_reg < ETNA_NUM_INPUTS);
      sf->reg[sf->num_reg].reg = src.reg;
      sf->reg[sf->num_reg].slot = var->data.location;
      sf->reg[sf->num_reg].num_components = glsl_get_components(var->type);
      sf->num_reg++;
      break;
   }
}

#define OPT(nir, pass, ...) ({                             \
   bool this_progress = false;                             \
   NIR_PASS(this_progress, nir, pass, ##__VA_ARGS__);      \
   this_progress;                                          \
})

static void
etna_optimize_loop(nir_shader *s)
{
   bool progress;
   do {
      progress = false;

      NIR_PASS_V(s, nir_lower_vars_to_ssa);
      progress |= OPT(s, nir_opt_copy_prop_vars);
      progress |= OPT(s, nir_opt_shrink_stores, true);
      progress |= OPT(s, nir_opt_shrink_vectors);
      progress |= OPT(s, nir_copy_prop);
      progress |= OPT(s, nir_opt_dce);
      progress |= OPT(s, nir_opt_cse);
      progress |= OPT(s, nir_opt_peephole_select, 16, true, true);
      progress |= OPT(s, nir_opt_intrinsics);
      progress |= OPT(s, nir_opt_algebraic);
      progress |= OPT(s, nir_opt_constant_folding);
      progress |= OPT(s, nir_opt_dead_cf);
      if (OPT(s, nir_opt_loop)) {
         progress = true;
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
   }
   while (progress);
}

static int
etna_glsl_type_size(const struct glsl_type *type, bool bindless)
{
   return glsl_count_attribute_slots(type, false);
}

static void
copy_uniform_state_to_shader(struct etna_shader_variant *sobj, uint64_t *consts, unsigned count)
{
   struct etna_shader_uniform_info *uinfo = &sobj->uniforms;

   uinfo->count = count * 4;
   uinfo->data = MALLOC(uinfo->count * sizeof(*uinfo->data));
   uinfo->contents = MALLOC(uinfo->count * sizeof(*uinfo->contents));

   for (unsigned i = 0; i < uinfo->count; i++) {
      uinfo->data[i] = consts[i];
      uinfo->contents[i] = consts[i] >> 32;
   }

   etna_set_shader_uniforms_dirty_flags(sobj);
}

#define ALU_SWIZ(s) INST_SWIZ((s)->swizzle[0], (s)->swizzle[1], (s)->swizzle[2], (s)->swizzle[3])
#define SRC_DISABLE ((hw_src){})
#define SRC_CONST(idx, s) ((hw_src){.use=1, .rgroup = INST_RGROUP_UNIFORM_0, .reg=idx, .swiz=s})
#define SRC_REG(idx, s) ((hw_src){.use=1, .rgroup = INST_RGROUP_TEMP, .reg=idx, .swiz=s})

typedef struct etna_inst_dst hw_dst;
typedef struct etna_inst_src hw_src;

static inline hw_src
src_swizzle(hw_src src, unsigned swizzle)
{
   if (src.rgroup != INST_RGROUP_IMMEDIATE)
      src.swiz = inst_swiz_compose(src.swiz, swizzle);

   return src;
}

/* constants are represented as 64-bit ints
 * 32-bit for the value and 32-bit for the type (imm, uniform, etc)
 */

#define CONST_VAL(a, b) (nir_const_value) {.u64 = (uint64_t)(a) << 32 | (uint64_t)(b)}
#define CONST(x) CONST_VAL(ETNA_UNIFORM_CONSTANT, x)
#define UNIFORM(x) CONST_VAL(ETNA_UNIFORM_UNIFORM, x)
#define TEXSCALE(x, i) CONST_VAL(ETNA_UNIFORM_TEXRECT_SCALE_X + (i), x)
#define TEXSIZE(x, i) CONST_VAL(ETNA_UNIFORM_TEXTURE_WIDTH + (i), x)

static int
const_add(uint64_t *c, uint64_t value)
{
   for (unsigned i = 0; i < 4; i++) {
      if (c[i] == value || !c[i]) {
         c[i] = value;
         return i;
      }
   }
   return -1;
}

static hw_src
const_src(struct etna_compile *c, nir_const_value *value, unsigned num_components)
{
   /* use inline immediates if possible */
   if (c->specs->halti >= 2 && num_components == 1 &&
       value[0].u64 >> 32 == ETNA_UNIFORM_CONSTANT) {
      uint32_t bits = value[0].u32;

      /* "float" - shifted by 12 */
      if ((bits & 0xfff) == 0)
         return etna_immediate_src(0, bits >> 12);

      /* "unsigned" - raw 20 bit value */
      if (bits < (1 << 20))
         return etna_immediate_src(2, bits);

      /* "signed" - sign extended 20-bit (sign included) value */
      if (bits >= 0xfff80000)
         return etna_immediate_src(1, bits);
   }

   unsigned i;
   int swiz = -1;
   for (i = 0; swiz < 0; i++) {
      uint64_t *a = &c->consts[i*4];
      uint64_t save[4];
      memcpy(save, a, sizeof(save));
      swiz = 0;
      for (unsigned j = 0; j < num_components; j++) {
         int c = const_add(a, value[j].u64);
         if (c < 0) {
            memcpy(a, save, sizeof(save));
            swiz = -1;
            break;
         }
         swiz |= c << j * 2;
      }
   }

   assert(i <= ETNA_MAX_IMM / 4);
   c->const_count = MAX2(c->const_count, i);

   return SRC_CONST(i - 1, swiz);
}

/* how to swizzle when used as a src */
static const uint8_t
reg_swiz[NUM_REG_TYPES] = {
   [REG_TYPE_VEC4] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_SCALAR_X] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_SCALAR_Y] = SWIZZLE(Y, Y, Y, Y),
   [REG_TYPE_VIRT_VEC2_XY] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_VEC2T_XY] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_VEC2C_XY] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_SCALAR_Z] = SWIZZLE(Z, Z, Z, Z),
   [REG_TYPE_VIRT_VEC2_XZ] = SWIZZLE(X, Z, X, Z),
   [REG_TYPE_VIRT_VEC2_YZ] = SWIZZLE(Y, Z, Y, Z),
   [REG_TYPE_VIRT_VEC2C_YZ] = SWIZZLE(Y, Z, Y, Z),
   [REG_TYPE_VIRT_VEC3_XYZ] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_VEC3C_XYZ] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_SCALAR_W] = SWIZZLE(W, W, W, W),
   [REG_TYPE_VIRT_VEC2_XW] = SWIZZLE(X, W, X, W),
   [REG_TYPE_VIRT_VEC2_YW] = SWIZZLE(Y, W, Y, W),
   [REG_TYPE_VIRT_VEC3_XYW] = SWIZZLE(X, Y, W, X),
   [REG_TYPE_VIRT_VEC2_ZW] = SWIZZLE(Z, W, Z, W),
   [REG_TYPE_VIRT_VEC2T_ZW] = SWIZZLE(Z, W, Z, W),
   [REG_TYPE_VIRT_VEC2C_ZW] = SWIZZLE(Z, W, Z, W),
   [REG_TYPE_VIRT_VEC3_XZW] = SWIZZLE(X, Z, W, X),
   [REG_TYPE_VIRT_VEC3_YZW] = SWIZZLE(Y, Z, W, X),
   [REG_TYPE_VIRT_VEC3C_YZW] = SWIZZLE(Y, Z, W, X),
};

/* how to swizzle when used as a dest */
static const uint8_t
reg_dst_swiz[NUM_REG_TYPES] = {
   [REG_TYPE_VEC4] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_SCALAR_X] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_SCALAR_Y] = SWIZZLE(X, X, X, X),
   [REG_TYPE_VIRT_VEC2_XY] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_VEC2T_XY] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_VEC2C_XY] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_SCALAR_Z] = SWIZZLE(X, X, X, X),
   [REG_TYPE_VIRT_VEC2_XZ] = SWIZZLE(X, X, Y, Y),
   [REG_TYPE_VIRT_VEC2_YZ] = SWIZZLE(X, X, Y, Y),
   [REG_TYPE_VIRT_VEC2C_YZ] = SWIZZLE(X, X, Y, Y),
   [REG_TYPE_VIRT_VEC3_XYZ] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_VEC3C_XYZ] = INST_SWIZ_IDENTITY,
   [REG_TYPE_VIRT_SCALAR_W] = SWIZZLE(X, X, X, X),
   [REG_TYPE_VIRT_VEC2_XW] = SWIZZLE(X, X, Y, Y),
   [REG_TYPE_VIRT_VEC2_YW] = SWIZZLE(X, X, Y, Y),
   [REG_TYPE_VIRT_VEC3_XYW] = SWIZZLE(X, Y, Z, Z),
   [REG_TYPE_VIRT_VEC2_ZW] = SWIZZLE(X, X, X, Y),
   [REG_TYPE_VIRT_VEC2T_ZW] = SWIZZLE(X, X, X, Y),
   [REG_TYPE_VIRT_VEC2C_ZW] = SWIZZLE(X, X, X, Y),
   [REG_TYPE_VIRT_VEC3_XZW] = SWIZZLE(X, Y, Y, Z),
   [REG_TYPE_VIRT_VEC3_YZW] = SWIZZLE(X, X, Y, Z),
   [REG_TYPE_VIRT_VEC3C_YZW] = SWIZZLE(X, X, Y, Z),
};

/* nir_src to allocated register */
static hw_src
ra_src(struct etna_compile *c, nir_src *src)
{
   unsigned reg = ra_get_node_reg(c->g, c->live_map[src_index(c->impl, src)]);
   return SRC_REG(reg_get_base(c, reg), reg_swiz[reg_get_type(reg)]);
}

static hw_src
get_src(struct etna_compile *c, nir_src *src)
{
   nir_instr *instr = src->ssa->parent_instr;

   if (instr->pass_flags & BYPASS_SRC) {
      assert(instr->type == nir_instr_type_alu);
      nir_alu_instr *alu = nir_instr_as_alu(instr);
      assert(alu->op == nir_op_mov);
      return src_swizzle(get_src(c, &alu->src[0].src), ALU_SWIZ(&alu->src[0]));
   }

   switch (instr->type) {
   case nir_instr_type_load_const:
      return const_src(c, nir_instr_as_load_const(instr)->value, src->ssa->num_components);
   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
      switch (intr->intrinsic) {
      case nir_intrinsic_load_input:
      case nir_intrinsic_load_instance_id:
      case nir_intrinsic_load_uniform:
      case nir_intrinsic_load_ubo:
      case nir_intrinsic_load_reg:
         return ra_src(c, src);
      case nir_intrinsic_load_front_face:
         return (hw_src) { .use = 1, .rgroup = INST_RGROUP_INTERNAL };
      case nir_intrinsic_load_frag_coord:
         return SRC_REG(0, INST_SWIZ_IDENTITY);
      case nir_intrinsic_load_texture_scale: {
         int sampler = nir_src_as_int(intr->src[0]);
         nir_const_value values[] = {
            TEXSCALE(sampler, 0),
            TEXSCALE(sampler, 1),
         };

         return src_swizzle(const_src(c, values, 2), SWIZZLE(X,Y,X,X));
      }
      case nir_intrinsic_load_texture_size_etna: {
         int sampler = nir_src_as_int(intr->src[0]);
         nir_const_value values[] = {
            TEXSIZE(sampler, 0),
            TEXSIZE(sampler, 1),
            TEXSIZE(sampler, 2),
         };

         return src_swizzle(const_src(c, values, 3), SWIZZLE(X,Y,Z,X));
      }
      default:
         compile_error(c, "Unhandled NIR intrinsic type: %s\n",
                       nir_intrinsic_infos[intr->intrinsic].name);
         break;
      }
   } break;
   case nir_instr_type_alu:
   case nir_instr_type_tex:
      return ra_src(c, src);
   case nir_instr_type_undef: {
      /* return zero to deal with broken Blur demo */
      nir_const_value value = CONST(0);
      return src_swizzle(const_src(c, &value, 1), SWIZZLE(X,X,X,X));
   }
   default:
      compile_error(c, "Unhandled NIR instruction type: %d\n", instr->type);
      break;
   }

   return SRC_DISABLE;
}

static bool
vec_dest_has_swizzle(nir_alu_instr *vec, nir_def *ssa)
{
   for (unsigned i = 0; i < vec->def.num_components; i++) {
      if (vec->src[i].src.ssa != ssa)
         continue;

      if (vec->src[i].swizzle[0] != i)
         return true;
   }

   /* don't deal with possible bypassed vec/mov chain */
   nir_foreach_use(use_src, ssa) {
      nir_instr *instr = nir_src_parent_instr(use_src);
      if (instr->type != nir_instr_type_alu)
         continue;

      nir_alu_instr *alu = nir_instr_as_alu(instr);

      switch (alu->op) {
      case nir_op_mov:
      case nir_op_vec2:
      case nir_op_vec3:
      case nir_op_vec4:
         return true;
      default:
         break;
      }
   }
   return false;
}

/* get allocated dest register for nir_def
 * *p_swiz tells how the components need to be placed into register
 */
static hw_dst
ra_def(struct etna_compile *c, nir_def *def, unsigned *p_swiz)
{
   unsigned swiz = INST_SWIZ_IDENTITY, mask = 0xf;
   def = real_def(def, &swiz, &mask);

   unsigned r = ra_get_node_reg(c->g, c->live_map[def_index(c->impl, def)]);
   unsigned t = reg_get_type(r);

   *p_swiz = inst_swiz_compose(swiz, reg_dst_swiz[t]);

   return (hw_dst) {
      .use = 1,
      .reg = reg_get_base(c, r),
      .write_mask = inst_write_mask_compose(mask, reg_writemask[t]),
   };
}

static void
emit_alu(struct etna_compile *c, nir_alu_instr * alu)
{
   const nir_op_info *info = &nir_op_infos[alu->op];

   /* marked as dead instruction (vecN and other bypassed instr) */
   if (is_dead_instruction(&alu->instr))
      return;

   assert(!(alu->op >= nir_op_vec2 && alu->op <= nir_op_vec4));

   unsigned dst_swiz;
   hw_dst dst = ra_def(c, &alu->def, &dst_swiz);

   switch (alu->op) {
   case nir_op_fdot2:
   case nir_op_fdot3:
   case nir_op_fdot4:
      /* not per-component - don't compose dst_swiz */
      dst_swiz = INST_SWIZ_IDENTITY;
      break;
   default:
      break;
   }

   hw_src srcs[3];

   for (int i = 0; i < info->num_inputs; i++) {
      nir_alu_src *asrc = &alu->src[i];
      hw_src src;

      src = src_swizzle(get_src(c, &asrc->src), ALU_SWIZ(asrc));
      src = src_swizzle(src, dst_swiz);

      if (src.rgroup != INST_RGROUP_IMMEDIATE) {
         src.neg = is_src_mod_neg(&alu->instr, i) || (alu->op == nir_op_fneg);
         src.abs = is_src_mod_abs(&alu->instr, i) || (alu->op == nir_op_fabs);
      } else {
         assert(alu->op != nir_op_fabs);
         assert(!is_src_mod_abs(&alu->instr, i) && alu->op != nir_op_fabs);

         if (src.imm_type > 0)
            assert(!is_src_mod_neg(&alu->instr, i));

         if (is_src_mod_neg(&alu->instr, i) && src.imm_type == 0)
            src.imm_val ^= 0x80000;
      }

      srcs[i] = src;
   }

   etna_emit_alu(c, alu->op, dst, srcs, alu->op == nir_op_fsat);
}

static void
emit_tex(struct etna_compile *c, nir_tex_instr * tex)
{
   unsigned dst_swiz;
   hw_dst dst = ra_def(c, &tex->def, &dst_swiz);
   nir_src *coord = NULL, *src1 = NULL, *src2 = NULL;

   for (unsigned i = 0; i < tex->num_srcs; i++) {
      switch (tex->src[i].src_type) {
      case nir_tex_src_coord:
         coord = &tex->src[i].src;
         break;
      case nir_tex_src_bias:
      case nir_tex_src_lod:
      case nir_tex_src_ddx:
         assert(!src1);
         src1 = &tex->src[i].src;
         break;
      case nir_tex_src_comparator:
      case nir_tex_src_ddy:
         src2 = &tex->src[i].src;
         break;
      default:
         compile_error(c, "Unhandled NIR tex src type: %d\n",
                       tex->src[i].src_type);
         break;
      }
   }

   etna_emit_tex(c, tex->op, tex->sampler_index, dst_swiz, dst, get_src(c, coord),
                 src1 ? get_src(c, src1) : SRC_DISABLE,
                 src2 ? get_src(c, src2) : SRC_DISABLE);
}

static void
emit_intrinsic(struct etna_compile *c, nir_intrinsic_instr * intr)
{
   switch (intr->intrinsic) {
   case nir_intrinsic_store_deref:
      etna_emit_output(c, nir_src_as_deref(intr->src[0])->var, get_src(c, &intr->src[1]));
      break;
   case nir_intrinsic_discard_if:
      etna_emit_discard(c, get_src(c, &intr->src[0]));
      break;
   case nir_intrinsic_discard:
      etna_emit_discard(c, SRC_DISABLE);
      break;
   case nir_intrinsic_load_uniform: {
      unsigned dst_swiz;
      struct etna_inst_dst dst = ra_def(c, &intr->def, &dst_swiz);

      /* TODO: rework so extra MOV isn't required, load up to 4 addresses at once */
      emit_inst(c, &(struct etna_inst) {
         .opcode = INST_OPCODE_MOVAR,
         .dst.write_mask = 0x1,
         .src[2] = get_src(c, &intr->src[0]),
      });
      emit_inst(c, &(struct etna_inst) {
         .opcode = INST_OPCODE_MOV,
         .dst = dst,
         .src[2] = {
            .use = 1,
            .rgroup = INST_RGROUP_UNIFORM_0,
            .reg = nir_intrinsic_base(intr),
            .swiz = dst_swiz,
            .amode = INST_AMODE_ADD_A_X,
         },
      });
   } break;
   case nir_intrinsic_load_ubo: {
      /* TODO: if offset is of the form (x + C) then add C to the base instead */
      unsigned idx = nir_src_as_const_value(intr->src[0])[0].u32;
      unsigned dst_swiz;
      emit_inst(c, &(struct etna_inst) {
         .opcode = INST_OPCODE_LOAD,
         .type = INST_TYPE_U32,
         .dst = ra_def(c, &intr->def, &dst_swiz),
         .src[0] = get_src(c, &intr->src[1]),
         .src[1] = const_src(c, &CONST_VAL(ETNA_UNIFORM_UBO0_ADDR + idx, 0), 1),
      });
   } break;
   case nir_intrinsic_load_front_face:
   case nir_intrinsic_load_frag_coord:
      break;
   case nir_intrinsic_load_input:
   case nir_intrinsic_load_instance_id:
   case nir_intrinsic_load_texture_scale:
   case nir_intrinsic_load_texture_size_etna:
   case nir_intrinsic_decl_reg:
   case nir_intrinsic_load_reg:
   case nir_intrinsic_store_reg:
      break;
   default:
      compile_error(c, "Unhandled NIR intrinsic type: %s\n",
                    nir_intrinsic_infos[intr->intrinsic].name);
   }
}

static void
emit_instr(struct etna_compile *c, nir_instr * instr)
{
   switch (instr->type) {
   case nir_instr_type_alu:
      emit_alu(c, nir_instr_as_alu(instr));
      break;
   case nir_instr_type_tex:
      emit_tex(c, nir_instr_as_tex(instr));
      break;
   case nir_instr_type_intrinsic:
      emit_intrinsic(c, nir_instr_as_intrinsic(instr));
      break;
   case nir_instr_type_jump:
      assert(nir_instr_is_last(instr));
      break;
   case nir_instr_type_load_const:
   case nir_instr_type_undef:
   case nir_instr_type_deref:
      break;
   default:
      compile_error(c, "Unhandled NIR instruction type: %d\n", instr->type);
      break;
   }
}

static void
emit_block(struct etna_compile *c, nir_block * block)
{
   etna_emit_block_start(c, block->index);

   nir_foreach_instr(instr, block)
      emit_instr(c, instr);

   /* succs->index < block->index is for the loop case  */
   nir_block *succs = block->successors[0];
   if (nir_block_ends_in_jump(block) || succs->index < block->index)
      etna_emit_jump(c, succs->index, SRC_DISABLE);
}

static void
emit_cf_list(struct etna_compile *c, struct exec_list *list);

static void
emit_if(struct etna_compile *c, nir_if * nif)
{
   etna_emit_jump(c, nir_if_first_else_block(nif)->index, get_src(c, &nif->condition));
   emit_cf_list(c, &nif->then_list);

   /* jump at end of then_list to skip else_list
    * not needed if then_list already ends with a jump or else_list is empty
    */
   if (!nir_block_ends_in_jump(nir_if_last_then_block(nif)) &&
       !nir_cf_list_is_empty_block(&nif->else_list))
      etna_emit_jump(c, nir_if_last_then_block(nif)->successors[0]->index, SRC_DISABLE);

   emit_cf_list(c, &nif->else_list);
}

static void
emit_cf_list(struct etna_compile *c, struct exec_list *list)
{
   foreach_list_typed(nir_cf_node, node, node, list) {
      switch (node->type) {
      case nir_cf_node_block:
         emit_block(c, nir_cf_node_as_block(node));
         break;
      case nir_cf_node_if:
         emit_if(c, nir_cf_node_as_if(node));
         break;
      case nir_cf_node_loop:
         assert(!nir_loop_has_continue_construct(nir_cf_node_as_loop(node)));
         emit_cf_list(c, &nir_cf_node_as_loop(node)->body);
         break;
      default:
         compile_error(c, "Unknown NIR node type\n");
         break;
      }
   }
}

/* based on nir_lower_vec_to_movs */
static unsigned
insert_vec_mov(nir_alu_instr *vec, unsigned start_idx, nir_shader *shader)
{
   assert(start_idx < nir_op_infos[vec->op].num_inputs);
   unsigned write_mask = (1u << start_idx);

   nir_alu_instr *mov = nir_alu_instr_create(shader, nir_op_mov);
   nir_alu_src_copy(&mov->src[0], &vec->src[start_idx]);

   mov->src[0].swizzle[0] = vec->src[start_idx].swizzle[0];

   if (is_src_mod_neg(&vec->instr, start_idx))
      set_src_mod_neg(&mov->instr, 0);

   if (is_src_mod_abs(&vec->instr, start_idx))
      set_src_mod_abs(&mov->instr, 0);

   unsigned num_components = 1;

   for (unsigned i = start_idx + 1; i < vec->def.num_components; i++) {
      if (nir_srcs_equal(vec->src[i].src, vec->src[start_idx].src) &&
         is_src_mod_neg(&vec->instr, i) == is_src_mod_neg(&vec->instr, start_idx) &&
         is_src_mod_abs(&vec->instr, i) == is_src_mod_neg(&vec->instr, start_idx)) {
         write_mask |= (1 << i);
         mov->src[0].swizzle[num_components] = vec->src[i].swizzle[0];
         num_components++;
      }
   }

   nir_def_init(&mov->instr, &mov->def, num_components, 32);

   /* replace vec srcs with inserted mov */
   for (unsigned i = 0, j = 0; i < 4; i++) {
      if (!(write_mask & (1 << i)))
         continue;

      nir_src_rewrite(&vec->src[i].src, &mov->def);
      vec->src[i].swizzle[0] = j++;
   }

   nir_instr_insert_before(&vec->instr, &mov->instr);

   return write_mask;
}

/*
 * Get the nir_const_value from an alu src.  Also look at
 * the parent instruction as it could be a fabs/fneg.
 */
static nir_const_value *get_alu_cv(nir_alu_src *src)
 {
   nir_const_value *cv = nir_src_as_const_value(src->src);

   if (!cv &&
       (src->src.ssa->parent_instr->type == nir_instr_type_alu)) {
      nir_alu_instr *parent = nir_instr_as_alu(src->src.ssa->parent_instr);

      if ((parent->op == nir_op_fabs) ||
          (parent->op == nir_op_fneg)) {
         cv = nir_src_as_const_value(parent->src[0].src);

         if (cv) {
            /* Validate that we are only using ETNA_UNIFORM_CONSTANT const_values. */
            for (unsigned i = 0; i < parent->def.num_components; i++) {
               if (cv[i].u64 >> 32 != ETNA_UNIFORM_CONSTANT) {
                  cv = NULL;
                  break;
               }
            }
         }
      }
   }

   return cv;
 }

/*
 * for vecN instructions:
 * -merge constant sources into a single src
 * -insert movs (nir_lower_vec_to_movs equivalent)
 * for non-vecN instructions:
 * -try to merge constants as single constant
 * -insert movs for multiple constants if required
 */
static void
lower_alu(struct etna_compile *c, nir_alu_instr *alu)
{
   const nir_op_info *info = &nir_op_infos[alu->op];

   nir_builder b = nir_builder_at(nir_before_instr(&alu->instr));

   switch (alu->op) {
   case nir_op_vec2:
   case nir_op_vec3:
   case nir_op_vec4:
      break;
   default:
      if (c->specs->has_no_oneconst_limit)
         return;

      nir_const_value value[4] = {};
      uint8_t swizzle[4][4] = {};
      unsigned swiz_max = 0, num_different_const_srcs = 0;
      int first_const = -1;

      for (unsigned i = 0; i < info->num_inputs; i++) {
         nir_const_value *cv = get_alu_cv(&alu->src[i]);
         if (!cv)
            continue;

         unsigned num_components = info->input_sizes[i] ?: alu->def.num_components;
         for (unsigned j = 0; j < num_components; j++) {
            int idx = const_add(&value[0].u64, cv[alu->src[i].swizzle[j]].u64);
            swizzle[i][j] = idx;
            swiz_max = MAX2(swiz_max, (unsigned) idx);
         }

         if (first_const == -1)
            first_const = i;

         if (!nir_srcs_equal(alu->src[first_const].src, alu->src[i].src))
            num_different_const_srcs++;
      }

      /* nothing to do */
      if (num_different_const_srcs == 0)
         return;

      /* resolve with single combined const src */
      if (swiz_max < 4) {
         nir_def *def = nir_build_imm(&b, swiz_max + 1, 32, value);

         for (unsigned i = 0; i < info->num_inputs; i++) {
            nir_const_value *cv = get_alu_cv(&alu->src[i]);
            if (!cv)
               continue;

            nir_src_rewrite(&alu->src[i].src, def);

            for (unsigned j = 0; j < 4; j++)
               alu->src[i].swizzle[j] = swizzle[i][j];
         }
         return;
      }

      /* resolve with movs */
      unsigned num_const = 0;
      for (unsigned i = 0; i < info->num_inputs; i++) {
         nir_const_value *cv = get_alu_cv(&alu->src[i]);
         if (!cv)
            continue;

         num_const++;
         if (num_const == 1)
            continue;

         nir_def *mov = nir_mov(&b, alu->src[i].src.ssa);
         nir_src_rewrite(&alu->src[i].src, mov);
      }
      return;
   }

   nir_const_value value[4];
   unsigned num_components = 0;

   for (unsigned i = 0; i < info->num_inputs; i++) {
      nir_const_value *cv = get_alu_cv(&alu->src[i]);
      if (cv)
         value[num_components++] = cv[alu->src[i].swizzle[0]];
   }

   /* if there is more than one constant source to the vecN, combine them
    * into a single load_const (removing the vecN completely if all components
    * are constant)
    */
   if (num_components > 1) {
      nir_def *def = nir_build_imm(&b, num_components, 32, value);

      if (num_components == info->num_inputs) {
         nir_def_rewrite_uses(&alu->def, def);
         nir_instr_remove(&alu->instr);
         return;
      }

      for (unsigned i = 0, j = 0; i < info->num_inputs; i++) {
         nir_const_value *cv = get_alu_cv(&alu->src[i]);
         if (!cv)
            continue;

         nir_src_rewrite(&alu->src[i].src, def);
         alu->src[i].swizzle[0] = j++;
      }
   }

   unsigned finished_write_mask = 0;
   for (unsigned i = 0; i < alu->def.num_components; i++) {
      nir_def *ssa = alu->src[i].src.ssa;

      /* check that vecN instruction is only user of this */
      bool need_mov = false;
      nir_foreach_use_including_if(use_src, ssa) {
         if (nir_src_is_if(use_src) || nir_src_parent_instr(use_src) != &alu->instr)
            need_mov = true;
      }

      nir_instr *instr = ssa->parent_instr;
      switch (instr->type) {
      case nir_instr_type_alu:
      case nir_instr_type_tex:
         break;
      case nir_instr_type_intrinsic:
         if (nir_instr_as_intrinsic(instr)->intrinsic == nir_intrinsic_load_input) {
            need_mov = vec_dest_has_swizzle(alu, &nir_instr_as_intrinsic(instr)->def);
            break;
         }
         FALLTHROUGH;
      default:
         need_mov = true;
      }

      if (need_mov && !(finished_write_mask & (1 << i)))
         finished_write_mask |= insert_vec_mov(alu, i, c->nir);
   }
}

static bool
emit_shader(struct etna_compile *c, unsigned *num_temps, unsigned *num_consts)
{
   nir_shader *shader = c->nir;
   c->impl = nir_shader_get_entrypoint(shader);

   bool have_indirect_uniform = false;
   unsigned indirect_max = 0;

   nir_builder b = nir_builder_create(c->impl);

   /* convert non-dynamic uniform loads to constants, etc */
   nir_foreach_block(block, c->impl) {
      nir_foreach_instr_safe(instr, block) {
         switch(instr->type) {
         case nir_instr_type_alu:
            /* deals with vecN and const srcs */
            lower_alu(c, nir_instr_as_alu(instr));
            break;
         case nir_instr_type_load_const: {
            nir_load_const_instr *load_const = nir_instr_as_load_const(instr);
            for (unsigned  i = 0; i < load_const->def.num_components; i++)
               load_const->value[i] = CONST(load_const->value[i].u32);
         } break;
         case nir_instr_type_intrinsic: {
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            /* TODO: load_ubo can also become a constant in some cases
             * (at the moment it can end up emitting a LOAD with two
             *  uniform sources, which could be a problem on HALTI2)
             */
            if (intr->intrinsic != nir_intrinsic_load_uniform)
               break;
            nir_const_value *off = nir_src_as_const_value(intr->src[0]);
            if (!off || off[0].u64 >> 32 != ETNA_UNIFORM_CONSTANT) {
               have_indirect_uniform = true;
               indirect_max = nir_intrinsic_base(intr) + nir_intrinsic_range(intr);
               break;
            }

            unsigned base = nir_intrinsic_base(intr);
            /* pre halti2 uniform offset will be float */
            if (c->specs->halti < 2)
               base += (unsigned) off[0].f32;
            else
               base += off[0].u32;
            nir_const_value value[4];

            for (unsigned i = 0; i < intr->def.num_components; i++)
               value[i] = UNIFORM(base * 4 + i);

            b.cursor = nir_after_instr(instr);
            nir_def *def = nir_build_imm(&b, intr->def.num_components, 32, value);

            nir_def_rewrite_uses(&intr->def, def);
            nir_instr_remove(instr);
         } break;
         default:
            break;
         }
      }
   }

   /* TODO: only emit required indirect uniform ranges */
   if (have_indirect_uniform) {
      for (unsigned i = 0; i < indirect_max * 4; i++)
         c->consts[i] = UNIFORM(i).u64;
      c->const_count = indirect_max;
   }

   /* add mov for any store output using sysval/const and for depth stores from intrinsics */
   nir_foreach_block(block, c->impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

         switch (intr->intrinsic) {
         case nir_intrinsic_store_deref: {
            nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
            nir_src *src = &intr->src[1];
            if (nir_src_is_const(*src) || is_sysval(src->ssa->parent_instr) ||
                (shader->info.stage == MESA_SHADER_FRAGMENT &&
                 deref->var->data.location == FRAG_RESULT_DEPTH &&
                 src->ssa->parent_instr->type != nir_instr_type_alu)) {
               b.cursor = nir_before_instr(instr);
               nir_src_rewrite(src, nir_mov(&b, src->ssa));
            }
         } break;
         default:
            break;
         }
      }
   }

   /* call directly to avoid validation (load_const don't pass validation at this point) */
   nir_convert_from_ssa(shader, true);
   nir_trivialize_registers(shader);

   etna_ra_assign(c, shader);

   emit_cf_list(c, &nir_shader_get_entrypoint(shader)->body);

   *num_temps = etna_ra_finish(c);
   *num_consts = c->const_count;
   return true;
}

static bool
etna_compile_check_limits(struct etna_shader_variant *v)
{
   const struct etna_specs *specs = v->shader->specs;
   int max_uniforms = (v->stage == MESA_SHADER_VERTEX)
                         ? specs->max_vs_uniforms
                         : specs->max_ps_uniforms;

   if (!specs->has_icache && v->needs_icache) {
      DBG("Number of instructions (%d) exceeds maximum %d", v->code_size / 4,
          specs->max_instructions);
      return false;
   }

   if (v->num_temps > specs->max_registers) {
      DBG("Number of registers (%d) exceeds maximum %d", v->num_temps,
          specs->max_registers);
      return false;
   }

   if (v->uniforms.count / 4 > max_uniforms) {
      DBG("Number of uniforms (%d) exceeds maximum %d",
          v->uniforms.count / 4, max_uniforms);
      return false;
   }

   return true;
}

static void
fill_vs_mystery(struct etna_shader_variant *v)
{
   const struct etna_specs *specs = v->shader->specs;

   v->input_count_unk8 = DIV_ROUND_UP(v->infile.num_reg + 4, 16); /* XXX what is this */

   /* fill in "mystery meat" load balancing value. This value determines how
    * work is scheduled between VS and PS
    * in the unified shader architecture. More precisely, it is determined from
    * the number of VS outputs, as well as chip-specific
    * vertex output buffer size, vertex cache size, and the number of shader
    * cores.
    *
    * XXX this is a conservative estimate, the "optimal" value is only known for
    * sure at link time because some
    * outputs may be unused and thus unmapped. Then again, in the general use
    * case with GLSL the vertex and fragment
    * shaders are linked already before submitting to Gallium, thus all outputs
    * are used.
    *
    * note: TGSI compiler counts all outputs (including position and pointsize), here
    * v->outfile.num_reg only counts varyings, +1 to compensate for the position output
    * TODO: might have a problem that we don't count pointsize when it is used
    */

   int half_out = v->outfile.num_reg / 2 + 1;
   assert(half_out);

   uint32_t b = ((20480 / (specs->vertex_output_buffer_size -
                           2 * half_out * specs->vertex_cache_size)) +
                 9) /
                10;
   uint32_t a = (b + 256 / (specs->shader_core_count * half_out)) / 2;
   v->vs_load_balancing = VIVS_VS_LOAD_BALANCING_A(MIN2(a, 255)) |
                             VIVS_VS_LOAD_BALANCING_B(MIN2(b, 255)) |
                             VIVS_VS_LOAD_BALANCING_C(0x3f) |
                             VIVS_VS_LOAD_BALANCING_D(0x0f);
}

bool
etna_compile_shader(struct etna_shader_variant *v)
{
   if (unlikely(!v))
      return false;

   struct etna_compile *c = CALLOC_STRUCT(etna_compile);
   if (!c)
      return false;

   c->variant = v;
   c->specs = v->shader->specs;
   c->nir = nir_shader_clone(NULL, v->shader->nir);

   nir_shader *s = c->nir;
   const struct etna_specs *specs = c->specs;

   v->stage = s->info.stage;
   v->uses_discard = s->info.fs.uses_discard;
   v->num_loops = 0; /* TODO */
   v->vs_id_in_reg = -1;
   v->vs_pos_out_reg = -1;
   v->vs_pointsize_out_reg = -1;
   v->ps_color_out_reg = 0; /* 0 for shader that doesn't write fragcolor.. */
   v->ps_depth_out_reg = -1;

   /*
    * Lower glTexCoord, fixes e.g. neverball point sprite (exit cylinder stars)
    * and gl4es pointsprite.trace apitrace
    */
   if (s->info.stage == MESA_SHADER_FRAGMENT && v->key.sprite_coord_enable) {
      NIR_PASS_V(s, nir_lower_texcoord_replace, v->key.sprite_coord_enable,
                 false, v->key.sprite_coord_yinvert);
   }

   /*
    * Remove any dead in variables before we iterate over them
    */
   NIR_PASS_V(s, nir_remove_dead_variables, nir_var_shader_in, NULL);

   /* setup input linking */
   struct etna_shader_io_file *sf = &v->infile;
   if (s->info.stage == MESA_SHADER_VERTEX) {
      nir_foreach_shader_in_variable(var, s) {
         unsigned idx = var->data.driver_location;
         sf->reg[idx].reg = idx;
         sf->reg[idx].slot = var->data.location;
         sf->reg[idx].num_components = glsl_get_components(var->type);
         sf->num_reg = MAX2(sf->num_reg, idx+1);
      }
   } else {
      unsigned count = 0;
      nir_foreach_shader_in_variable(var, s) {
         unsigned idx = var->data.driver_location;
         sf->reg[idx].reg = idx + 1;
         sf->reg[idx].slot = var->data.location;
         sf->reg[idx].num_components = glsl_get_components(var->type);
         sf->num_reg = MAX2(sf->num_reg, idx+1);
         count++;
      }
      assert(sf->num_reg == count);
   }

   NIR_PASS_V(s, nir_lower_io, nir_var_shader_in | nir_var_uniform, etna_glsl_type_size,
            (nir_lower_io_options)0);

   NIR_PASS_V(s, nir_lower_vars_to_ssa);
   NIR_PASS_V(s, nir_lower_indirect_derefs, nir_var_all, UINT32_MAX);
   NIR_PASS_V(s, etna_nir_lower_texture, &v->key);

   NIR_PASS_V(s, nir_lower_alu_to_scalar, etna_alu_to_scalar_filter_cb, specs);
   if (c->specs->halti >= 2) {
      nir_lower_idiv_options idiv_options = {
         .allow_fp16 = true,
      };
      NIR_PASS_V(s, nir_lower_idiv, &idiv_options);
   }
   NIR_PASS_V(s, nir_lower_alu);

   etna_optimize_loop(s);

   /* TODO: remove this extra run if nir_opt_peephole_select is able to handle ubo's. */
   if (OPT(s, etna_nir_lower_ubo_to_uniform))
      etna_optimize_loop(s);

   NIR_PASS_V(s, etna_lower_io, v);
   NIR_PASS_V(s, nir_lower_pack);
   etna_optimize_loop(s);

   if (v->shader->specs->vs_need_z_div)
      NIR_PASS_V(s, nir_lower_clip_halfz);

   /* lower pre-halti2 to float (halti0 has integers, but only scalar..) */
   if (c->specs->halti < 2) {
      /* use opt_algebraic between int_to_float and boot_to_float because
       * int_to_float emits ftrunc, and ftrunc lowering generates bool ops
       */
      NIR_PASS_V(s, nir_lower_int_to_float);
      NIR_PASS_V(s, nir_opt_algebraic);
      NIR_PASS_V(s, nir_lower_bool_to_float, true);
   } else {
      NIR_PASS_V(s, nir_lower_bool_to_int32);
   }

   while( OPT(s, nir_opt_vectorize, NULL, NULL) );
   NIR_PASS_V(s, nir_lower_alu_to_scalar, etna_alu_to_scalar_filter_cb, specs);

   NIR_PASS_V(s, nir_remove_dead_variables, nir_var_function_temp, NULL);
   NIR_PASS_V(s, nir_opt_algebraic_late);

   NIR_PASS_V(s, nir_move_vec_src_uses_to_dest, false);
   NIR_PASS_V(s, nir_copy_prop);
   /* need copy prop after uses_to_dest, and before src mods: see
    * dEQP-GLES2.functional.shaders.random.all_features.fragment.95
    */

   NIR_PASS_V(s, nir_opt_dce);
   NIR_PASS_V(s, nir_opt_cse);

   NIR_PASS_V(s, nir_lower_bool_to_bitsize);
   NIR_PASS_V(s, etna_lower_alu, c->specs->has_new_transcendentals);

   /* needs to be the last pass that touches pass_flags! */
   NIR_PASS_V(s, etna_nir_lower_to_source_mods);

   if (DBG_ENABLED(ETNA_DBG_DUMP_SHADERS))
      nir_print_shader(s, stdout);

   unsigned block_ptr[nir_shader_get_entrypoint(s)->num_blocks];
   c->block_ptr = block_ptr;

   unsigned num_consts;
   ASSERTED bool ok = emit_shader(c, &v->num_temps, &num_consts);
   assert(ok);

   /* empty shader, emit NOP */
   if (!c->inst_ptr)
      emit_inst(c, &(struct etna_inst) { .opcode = INST_OPCODE_NOP });

   /* assemble instructions, fixing up labels */
   uint32_t *code = MALLOC(c->inst_ptr * 16);
   for (unsigned i = 0; i < c->inst_ptr; i++) {
      struct etna_inst *inst = &c->code[i];
      if (inst->opcode == INST_OPCODE_BRANCH)
         inst->imm = block_ptr[inst->imm];

      inst->no_oneconst_limit = specs->has_no_oneconst_limit;
      etna_assemble(&code[i * 4], inst);
   }

   v->code_size = c->inst_ptr * 4;
   v->code = code;
   v->needs_icache = c->inst_ptr > specs->max_instructions;

   copy_uniform_state_to_shader(v, c->consts, num_consts);

   if (s->info.stage == MESA_SHADER_FRAGMENT) {
      v->input_count_unk8 = 31; /* XXX what is this */
      assert(v->ps_depth_out_reg <= 0);
   } else {
      fill_vs_mystery(v);
   }

   bool result = etna_compile_check_limits(v);
   ralloc_free(c->nir);
   FREE(c);
   return result;
}

static const struct etna_shader_inout *
etna_shader_vs_lookup(const struct etna_shader_variant *sobj,
                      const struct etna_shader_inout *in)
{
   for (int i = 0; i < sobj->outfile.num_reg; i++)
      if (sobj->outfile.reg[i].slot == in->slot)
         return &sobj->outfile.reg[i];

   /*
    * There are valid NIR shaders pairs where the vertex shader has
    * a VARYING_SLOT_BFC0 shader_out and the corresponding framgent
    * shader has a VARYING_SLOT_COL0 shader_in.
    * So at link time if there is no matching VARYING_SLOT_BFC[n],
    * we must map VARYING_SLOT_BFC0[n] to VARYING_SLOT_COL[n].
    */
   gl_varying_slot slot;

   if (in->slot == VARYING_SLOT_COL0)
      slot = VARYING_SLOT_BFC0;
   else if (in->slot == VARYING_SLOT_COL1)
      slot = VARYING_SLOT_BFC1;
   else
      return NULL;

   for (int i = 0; i < sobj->outfile.num_reg; i++)
      if (sobj->outfile.reg[i].slot == slot)
         return &sobj->outfile.reg[i];

   return NULL;
}

void
etna_link_shader(struct etna_shader_link_info *info,
                 const struct etna_shader_variant *vs,
                 const struct etna_shader_variant *fs)
{
   int comp_ofs = 0;
   /* For each fragment input we need to find the associated vertex shader
    * output, which can be found by matching on semantic name and index. A
    * binary search could be used because the vs outputs are sorted by their
    * semantic index and grouped by semantic type by fill_in_vs_outputs.
    */
   assert(fs->infile.num_reg < ETNA_NUM_INPUTS);
   info->pcoord_varying_comp_ofs = -1;

   for (int idx = 0; idx < fs->infile.num_reg; ++idx) {
      const struct etna_shader_inout *fsio = &fs->infile.reg[idx];
      const struct etna_shader_inout *vsio = etna_shader_vs_lookup(vs, fsio);
      struct etna_varying *varying;
      bool interpolate_always = true;

      assert(fsio->reg > 0 && fsio->reg <= ARRAY_SIZE(info->varyings));

      if (fsio->reg > info->num_varyings)
         info->num_varyings = fsio->reg;

      varying = &info->varyings[fsio->reg - 1];
      varying->num_components = fsio->num_components;

      if (!interpolate_always) /* colors affected by flat shading */
         varying->pa_attributes = 0x200;
      else /* texture coord or other bypasses flat shading */
         varying->pa_attributes = 0x2f1;

      varying->use[0] = VARYING_COMPONENT_USE_UNUSED;
      varying->use[1] = VARYING_COMPONENT_USE_UNUSED;
      varying->use[2] = VARYING_COMPONENT_USE_UNUSED;
      varying->use[3] = VARYING_COMPONENT_USE_UNUSED;

      /* point/tex coord is an input to the PS without matching VS output,
       * so it gets a varying slot without being assigned a VS register.
       */
      if (fsio->slot == VARYING_SLOT_PNTC) {
         varying->use[0] = VARYING_COMPONENT_USE_POINTCOORD_X;
         varying->use[1] = VARYING_COMPONENT_USE_POINTCOORD_Y;

         info->pcoord_varying_comp_ofs = comp_ofs;
      } else if (util_varying_is_point_coord(fsio->slot, fs->key.sprite_coord_enable)) {
         /*
	  * Do nothing, TexCoord is lowered to PointCoord above
	  * and the TexCoord here is just a remnant. This needs
	  * to be removed with some nir_remove_dead_variables(),
	  * but that one removes all FS inputs ... why?
	  */
      } else {
         /* pick a random register to use if there is no VS output */
         if (vsio == NULL)
            varying->reg = 0;
         else
            varying->reg = vsio->reg;
      }

      comp_ofs += varying->num_components;
   }

   assert(info->num_varyings == fs->infile.num_reg);
}
