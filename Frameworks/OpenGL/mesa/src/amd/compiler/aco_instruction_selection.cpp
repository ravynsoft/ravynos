/*
 * Copyright © 2018 Valve Corporation
 * Copyright © 2018 Google
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include "aco_instruction_selection.h"

#include "aco_builder.h"
#include "aco_interface.h"
#include "aco_ir.h"

#include "common/ac_nir.h"
#include "common/sid.h"

#include "util/fast_idiv_by_const.h"
#include "util/memstream.h"

#include <array>
#include <functional>
#include <map>
#include <numeric>
#include <stack>
#include <utility>
#include <vector>

namespace aco {
namespace {

#define isel_err(...) _isel_err(ctx, __FILE__, __LINE__, __VA_ARGS__)

static void
_isel_err(isel_context* ctx, const char* file, unsigned line, const nir_instr* instr,
          const char* msg)
{
   char* out;
   size_t outsize;
   struct u_memstream mem;
   u_memstream_open(&mem, &out, &outsize);
   FILE* const memf = u_memstream_get(&mem);

   fprintf(memf, "%s: ", msg);
   nir_print_instr(instr, memf);
   u_memstream_close(&mem);

   _aco_err(ctx->program, file, line, out);
   free(out);
}

struct if_context {
   Temp cond;

   bool divergent_old;
   bool exec_potentially_empty_discard_old;
   bool exec_potentially_empty_break_old;
   bool had_divergent_discard_old;
   bool had_divergent_discard_then;
   uint16_t exec_potentially_empty_break_depth_old;

   unsigned BB_if_idx;
   unsigned invert_idx;
   bool uniform_has_then_branch;
   bool then_branch_divergent;
   Block BB_invert;
   Block BB_endif;
};

struct loop_context {
   Block loop_exit;

   unsigned header_idx_old;
   Block* exit_old;
   bool divergent_cont_old;
   bool divergent_branch_old;
   bool divergent_if_old;
};

static bool visit_cf_list(struct isel_context* ctx, struct exec_list* list);

static void
add_logical_edge(unsigned pred_idx, Block* succ)
{
   succ->logical_preds.emplace_back(pred_idx);
}

static void
add_linear_edge(unsigned pred_idx, Block* succ)
{
   succ->linear_preds.emplace_back(pred_idx);
}

static void
add_edge(unsigned pred_idx, Block* succ)
{
   add_logical_edge(pred_idx, succ);
   add_linear_edge(pred_idx, succ);
}

static void
append_logical_start(Block* b)
{
   Builder(NULL, b).pseudo(aco_opcode::p_logical_start);
}

static void
append_logical_end(Block* b)
{
   Builder(NULL, b).pseudo(aco_opcode::p_logical_end);
}

Temp
get_ssa_temp(struct isel_context* ctx, nir_def* def)
{
   uint32_t id = ctx->first_temp_id + def->index;
   return Temp(id, ctx->program->temp_rc[id]);
}

Temp
emit_mbcnt(isel_context* ctx, Temp dst, Operand mask = Operand(), Operand base = Operand::zero())
{
   Builder bld(ctx->program, ctx->block);
   assert(mask.isUndefined() || mask.isTemp() || (mask.isFixed() && mask.physReg() == exec));
   assert(mask.isUndefined() || mask.bytes() == bld.lm.bytes());

   if (ctx->program->wave_size == 32) {
      Operand mask_lo = mask.isUndefined() ? Operand::c32(-1u) : mask;
      return bld.vop3(aco_opcode::v_mbcnt_lo_u32_b32, Definition(dst), mask_lo, base);
   }

   Operand mask_lo = Operand::c32(-1u);
   Operand mask_hi = Operand::c32(-1u);

   if (mask.isTemp()) {
      RegClass rc = RegClass(mask.regClass().type(), 1);
      Builder::Result mask_split =
         bld.pseudo(aco_opcode::p_split_vector, bld.def(rc), bld.def(rc), mask);
      mask_lo = Operand(mask_split.def(0).getTemp());
      mask_hi = Operand(mask_split.def(1).getTemp());
   } else if (mask.physReg() == exec) {
      mask_lo = Operand(exec_lo, s1);
      mask_hi = Operand(exec_hi, s1);
   }

   Temp mbcnt_lo = bld.vop3(aco_opcode::v_mbcnt_lo_u32_b32, bld.def(v1), mask_lo, base);

   if (ctx->program->gfx_level <= GFX7)
      return bld.vop2(aco_opcode::v_mbcnt_hi_u32_b32, Definition(dst), mask_hi, mbcnt_lo);
   else
      return bld.vop3(aco_opcode::v_mbcnt_hi_u32_b32_e64, Definition(dst), mask_hi, mbcnt_lo);
}

inline void
set_wqm(isel_context* ctx, bool enable_helpers = false)
{
   if (ctx->program->stage == fragment_fs) {
      ctx->wqm_block_idx = ctx->block->index;
      ctx->wqm_instruction_idx = ctx->block->instructions.size();
      if (ctx->shader)
         enable_helpers |= ctx->shader->info.fs.require_full_quads;
      ctx->program->needs_wqm |= enable_helpers;
   }
}

static Temp
emit_bpermute(isel_context* ctx, Builder& bld, Temp index, Temp data)
{
   if (index.regClass() == s1)
      return bld.readlane(bld.def(s1), data, index);

   /* Avoid using shared VGPRs for shuffle on GFX10 when the shader consists
    * of multiple binaries, because the VGPR use is not known when choosing
    * which registers to use for the shared VGPRs.
    */
   const bool avoid_shared_vgprs =
      ctx->options->gfx_level >= GFX10 && ctx->options->gfx_level < GFX11 &&
      ctx->program->wave_size == 64 &&
      (ctx->program->info.has_epilog || ctx->program->info.merged_shader_compiled_separately ||
       ctx->stage == raytracing_cs);

   if (ctx->options->gfx_level <= GFX7 || avoid_shared_vgprs) {
      /* GFX6-7: there is no bpermute instruction */
      Operand index_op(index);
      Operand input_data(data);
      index_op.setLateKill(true);
      input_data.setLateKill(true);

      return bld.pseudo(aco_opcode::p_bpermute_readlane, bld.def(v1), bld.def(bld.lm),
                        bld.def(bld.lm, vcc), index_op, input_data);
   } else if (ctx->options->gfx_level >= GFX10 && ctx->program->wave_size == 64) {

      /* GFX10 wave64 mode: emulate full-wave bpermute */
      Temp index_is_lo =
         bld.vopc(aco_opcode::v_cmp_ge_u32, bld.def(bld.lm), Operand::c32(31u), index);
      Builder::Result index_is_lo_split =
         bld.pseudo(aco_opcode::p_split_vector, bld.def(s1), bld.def(s1), index_is_lo);
      Temp index_is_lo_n1 = bld.sop1(aco_opcode::s_not_b32, bld.def(s1), bld.def(s1, scc),
                                     index_is_lo_split.def(1).getTemp());
      Operand same_half = bld.pseudo(aco_opcode::p_create_vector, bld.def(s2),
                                     index_is_lo_split.def(0).getTemp(), index_is_lo_n1);
      Operand index_x4 = bld.vop2(aco_opcode::v_lshlrev_b32, bld.def(v1), Operand::c32(2u), index);
      Operand input_data(data);

      index_x4.setLateKill(true);
      input_data.setLateKill(true);
      same_half.setLateKill(true);

      if (ctx->options->gfx_level <= GFX10_3) {
         /* We need one pair of shared VGPRs:
          * Note, that these have twice the allocation granularity of normal VGPRs
          */
         ctx->program->config->num_shared_vgprs = 2 * ctx->program->dev.vgpr_alloc_granule;

         return bld.pseudo(aco_opcode::p_bpermute_shared_vgpr, bld.def(v1), bld.def(s2),
                           bld.def(s1, scc), index_x4, input_data, same_half);
      } else {
         return bld.pseudo(aco_opcode::p_bpermute_permlane, bld.def(v1), bld.def(s2),
                           bld.def(s1, scc), Operand(v1.as_linear()), index_x4, input_data,
                           same_half);
      }
   } else {
      /* GFX8-9 or GFX10 wave32: bpermute works normally */
      Temp index_x4 = bld.vop2(aco_opcode::v_lshlrev_b32, bld.def(v1), Operand::c32(2u), index);
      return bld.ds(aco_opcode::ds_bpermute_b32, bld.def(v1), index_x4, data);
   }
}

static Temp
emit_masked_swizzle(isel_context* ctx, Builder& bld, Temp src, unsigned mask, bool allow_fi)
{
   if (ctx->options->gfx_level >= GFX8) {
      unsigned and_mask = mask & 0x1f;
      unsigned or_mask = (mask >> 5) & 0x1f;
      unsigned xor_mask = (mask >> 10) & 0x1f;

      /* Eliminate or_mask. */
      and_mask &= ~or_mask;
      xor_mask ^= or_mask;

      uint16_t dpp_ctrl = 0xffff;

      /* DPP16 before DPP8 before v_permlane(x)16_b32
       * because DPP16 supports modifiers and v_permlane
       * can't be folded into valu instructions.
       */
      if ((and_mask & 0x1c) == 0x1c && xor_mask < 4) {
         unsigned res[4];
         for (unsigned i = 0; i < 4; i++)
            res[i] = ((i & and_mask) ^ xor_mask);
         dpp_ctrl = dpp_quad_perm(res[0], res[1], res[2], res[3]);
      } else if (and_mask == 0x1f && xor_mask == 8) {
         dpp_ctrl = dpp_row_rr(8);
      } else if (and_mask == 0x1f && xor_mask == 0xf) {
         dpp_ctrl = dpp_row_mirror;
      } else if (and_mask == 0x1f && xor_mask == 0x7) {
         dpp_ctrl = dpp_row_half_mirror;
      } else if (ctx->options->gfx_level >= GFX11 && and_mask == 0x10 && xor_mask < 0x10) {
         dpp_ctrl = dpp_row_share(xor_mask);
      } else if (ctx->options->gfx_level >= GFX11 && and_mask == 0x1f && xor_mask < 0x10) {
         dpp_ctrl = dpp_row_xmask(xor_mask);
      } else if (ctx->options->gfx_level >= GFX10 && (and_mask & 0x18) == 0x18 && xor_mask < 8) {
         uint32_t lane_sel = 0;
         for (unsigned i = 0; i < 8; i++)
            lane_sel |= ((i & and_mask) ^ xor_mask) << (i * 3);
         return bld.vop1_dpp8(aco_opcode::v_mov_b32, bld.def(v1), src, lane_sel, allow_fi);
      } else if (ctx->options->gfx_level >= GFX10 && (and_mask & 0x10) == 0x10) {
         uint64_t lane_mask = 0;
         for (unsigned i = 0; i < 16; i++)
            lane_mask |= uint64_t((i & and_mask) ^ (xor_mask & 0xf)) << i * 4;
         aco_opcode opcode =
            xor_mask & 0x10 ? aco_opcode::v_permlanex16_b32 : aco_opcode::v_permlane16_b32;
         Temp op1 = bld.copy(bld.def(s1), Operand::c32(lane_mask & 0xffffffff));
         Temp op2 = bld.copy(bld.def(s1), Operand::c32(lane_mask >> 32));
         Builder::Result ret = bld.vop3(opcode, bld.def(v1), src, op1, op2);
         ret->valu().opsel[0] = allow_fi; /* set FETCH_INACTIVE */
         ret->valu().opsel[1] = true;     /* set BOUND_CTRL */
         return ret;
      }

      if (dpp_ctrl != 0xffff)
         return bld.vop1_dpp(aco_opcode::v_mov_b32, bld.def(v1), src, dpp_ctrl, 0xf, 0xf, true,
                             allow_fi);
   }

   return bld.ds(aco_opcode::ds_swizzle_b32, bld.def(v1), src, mask, 0, false);
}

Temp
as_vgpr(Builder& bld, Temp val)
{
   if (val.type() == RegType::sgpr)
      return bld.copy(bld.def(RegType::vgpr, val.size()), val);
   assert(val.type() == RegType::vgpr);
   return val;
}

Temp
as_vgpr(isel_context* ctx, Temp val)
{
   Builder bld(ctx->program, ctx->block);
   return as_vgpr(bld, val);
}

void
emit_extract_vector(isel_context* ctx, Temp src, uint32_t idx, Temp dst)
{
   Builder bld(ctx->program, ctx->block);
   bld.pseudo(aco_opcode::p_extract_vector, Definition(dst), src, Operand::c32(idx));
}

Temp
emit_extract_vector(isel_context* ctx, Temp src, uint32_t idx, RegClass dst_rc)
{
   /* no need to extract the whole vector */
   if (src.regClass() == dst_rc) {
      assert(idx == 0);
      return src;
   }

   assert(src.bytes() > (idx * dst_rc.bytes()));
   Builder bld(ctx->program, ctx->block);
   auto it = ctx->allocated_vec.find(src.id());
   if (it != ctx->allocated_vec.end() && dst_rc.bytes() == it->second[idx].regClass().bytes()) {
      if (it->second[idx].regClass() == dst_rc) {
         return it->second[idx];
      } else {
         assert(!dst_rc.is_subdword());
         assert(dst_rc.type() == RegType::vgpr && it->second[idx].type() == RegType::sgpr);
         return bld.copy(bld.def(dst_rc), it->second[idx]);
      }
   }

   if (dst_rc.is_subdword())
      src = as_vgpr(ctx, src);

   if (src.bytes() == dst_rc.bytes()) {
      assert(idx == 0);
      return bld.copy(bld.def(dst_rc), src);
   } else {
      Temp dst = bld.tmp(dst_rc);
      emit_extract_vector(ctx, src, idx, dst);
      return dst;
   }
}

void
emit_split_vector(isel_context* ctx, Temp vec_src, unsigned num_components)
{
   if (num_components == 1)
      return;
   if (ctx->allocated_vec.find(vec_src.id()) != ctx->allocated_vec.end())
      return;
   RegClass rc;
   if (num_components > vec_src.size()) {
      if (vec_src.type() == RegType::sgpr) {
         /* should still help get_alu_src() */
         emit_split_vector(ctx, vec_src, vec_src.size());
         return;
      }
      /* sub-dword split */
      rc = RegClass(RegType::vgpr, vec_src.bytes() / num_components).as_subdword();
   } else {
      rc = RegClass(vec_src.type(), vec_src.size() / num_components);
   }
   aco_ptr<Pseudo_instruction> split{create_instruction<Pseudo_instruction>(
      aco_opcode::p_split_vector, Format::PSEUDO, 1, num_components)};
   split->operands[0] = Operand(vec_src);
   std::array<Temp, NIR_MAX_VEC_COMPONENTS> elems;
   for (unsigned i = 0; i < num_components; i++) {
      elems[i] = ctx->program->allocateTmp(rc);
      split->definitions[i] = Definition(elems[i]);
   }
   ctx->block->instructions.emplace_back(std::move(split));
   ctx->allocated_vec.emplace(vec_src.id(), elems);
}

/* This vector expansion uses a mask to determine which elements in the new vector
 * come from the original vector. The other elements are undefined. */
void
expand_vector(isel_context* ctx, Temp vec_src, Temp dst, unsigned num_components, unsigned mask,
              bool zero_padding = false)
{
   assert(vec_src.type() == RegType::vgpr);
   Builder bld(ctx->program, ctx->block);

   if (dst.type() == RegType::sgpr && num_components > dst.size()) {
      Temp tmp_dst = bld.tmp(RegClass::get(RegType::vgpr, 2 * num_components));
      expand_vector(ctx, vec_src, tmp_dst, num_components, mask, zero_padding);
      bld.pseudo(aco_opcode::p_as_uniform, Definition(dst), tmp_dst);
      ctx->allocated_vec[dst.id()] = ctx->allocated_vec[tmp_dst.id()];
      return;
   }

   emit_split_vector(ctx, vec_src, util_bitcount(mask));

   if (vec_src == dst)
      return;

   if (num_components == 1) {
      if (dst.type() == RegType::sgpr)
         bld.pseudo(aco_opcode::p_as_uniform, Definition(dst), vec_src);
      else
         bld.copy(Definition(dst), vec_src);
      return;
   }

   unsigned component_bytes = dst.bytes() / num_components;
   RegClass src_rc = RegClass::get(RegType::vgpr, component_bytes);
   RegClass dst_rc = RegClass::get(dst.type(), component_bytes);
   assert(dst.type() == RegType::vgpr || !src_rc.is_subdword());
   std::array<Temp, NIR_MAX_VEC_COMPONENTS> elems;

   Temp padding = Temp(0, dst_rc);
   if (zero_padding)
      padding = bld.copy(bld.def(dst_rc), Operand::zero(component_bytes));

   aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
      aco_opcode::p_create_vector, Format::PSEUDO, num_components, 1)};
   vec->definitions[0] = Definition(dst);
   unsigned k = 0;
   for (unsigned i = 0; i < num_components; i++) {
      if (mask & (1 << i)) {
         Temp src = emit_extract_vector(ctx, vec_src, k++, src_rc);
         if (dst.type() == RegType::sgpr)
            src = bld.as_uniform(src);
         vec->operands[i] = Operand(src);
         elems[i] = src;
      } else {
         vec->operands[i] = Operand::zero(component_bytes);
         elems[i] = padding;
      }
   }
   ctx->block->instructions.emplace_back(std::move(vec));
   ctx->allocated_vec.emplace(dst.id(), elems);
}

/* adjust misaligned small bit size loads */
void
byte_align_scalar(isel_context* ctx, Temp vec, Operand offset, Temp dst)
{
   Builder bld(ctx->program, ctx->block);
   Operand shift;
   Temp select = Temp();
   if (offset.isConstant()) {
      assert(offset.constantValue() && offset.constantValue() < 4);
      shift = Operand::c32(offset.constantValue() * 8);
   } else {
      /* bit_offset = 8 * (offset & 0x3) */
      Temp tmp =
         bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc), offset, Operand::c32(3u));
      select = bld.tmp(s1);
      shift = bld.sop2(aco_opcode::s_lshl_b32, bld.def(s1), bld.scc(Definition(select)), tmp,
                       Operand::c32(3u));
   }

   if (vec.size() == 1) {
      bld.sop2(aco_opcode::s_lshr_b32, Definition(dst), bld.def(s1, scc), vec, shift);
   } else if (vec.size() == 2) {
      Temp tmp = dst.size() == 2 ? dst : bld.tmp(s2);
      bld.sop2(aco_opcode::s_lshr_b64, Definition(tmp), bld.def(s1, scc), vec, shift);
      if (tmp == dst)
         emit_split_vector(ctx, dst, 2);
      else
         emit_extract_vector(ctx, tmp, 0, dst);
   } else if (vec.size() == 3 || vec.size() == 4) {
      Temp lo = bld.tmp(s2), hi;
      if (vec.size() == 3) {
         /* this can happen if we use VMEM for a uniform load */
         hi = bld.tmp(s1);
         bld.pseudo(aco_opcode::p_split_vector, Definition(lo), Definition(hi), vec);
      } else {
         hi = bld.tmp(s2);
         bld.pseudo(aco_opcode::p_split_vector, Definition(lo), Definition(hi), vec);
         hi = bld.pseudo(aco_opcode::p_extract_vector, bld.def(s1), hi, Operand::zero());
      }
      if (select != Temp())
         hi =
            bld.sop2(aco_opcode::s_cselect_b32, bld.def(s1), hi, Operand::zero(), bld.scc(select));
      lo = bld.sop2(aco_opcode::s_lshr_b64, bld.def(s2), bld.def(s1, scc), lo, shift);
      Temp mid = bld.tmp(s1);
      lo = bld.pseudo(aco_opcode::p_split_vector, bld.def(s1), Definition(mid), lo);
      hi = bld.sop2(aco_opcode::s_lshl_b32, bld.def(s1), bld.def(s1, scc), hi, shift);
      mid = bld.sop2(aco_opcode::s_or_b32, bld.def(s1), bld.def(s1, scc), hi, mid);
      bld.pseudo(aco_opcode::p_create_vector, Definition(dst), lo, mid);
      emit_split_vector(ctx, dst, 2);
   }
}

void
byte_align_vector(isel_context* ctx, Temp vec, Operand offset, Temp dst, unsigned component_size)
{
   Builder bld(ctx->program, ctx->block);
   if (offset.isTemp()) {
      Temp tmp[4] = {vec, vec, vec, vec};

      if (vec.size() == 4) {
         tmp[0] = bld.tmp(v1), tmp[1] = bld.tmp(v1), tmp[2] = bld.tmp(v1), tmp[3] = bld.tmp(v1);
         bld.pseudo(aco_opcode::p_split_vector, Definition(tmp[0]), Definition(tmp[1]),
                    Definition(tmp[2]), Definition(tmp[3]), vec);
      } else if (vec.size() == 3) {
         tmp[0] = bld.tmp(v1), tmp[1] = bld.tmp(v1), tmp[2] = bld.tmp(v1);
         bld.pseudo(aco_opcode::p_split_vector, Definition(tmp[0]), Definition(tmp[1]),
                    Definition(tmp[2]), vec);
      } else if (vec.size() == 2) {
         tmp[0] = bld.tmp(v1), tmp[1] = bld.tmp(v1), tmp[2] = tmp[1];
         bld.pseudo(aco_opcode::p_split_vector, Definition(tmp[0]), Definition(tmp[1]), vec);
      }
      for (unsigned i = 0; i < dst.size(); i++)
         tmp[i] = bld.vop3(aco_opcode::v_alignbyte_b32, bld.def(v1), tmp[i + 1], tmp[i], offset);

      vec = tmp[0];
      if (dst.size() == 2)
         vec = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), tmp[0], tmp[1]);

      offset = Operand::zero();
   }

   unsigned num_components = vec.bytes() / component_size;
   if (vec.regClass() == dst.regClass()) {
      assert(offset.constantValue() == 0);
      bld.copy(Definition(dst), vec);
      emit_split_vector(ctx, dst, num_components);
      return;
   }

   emit_split_vector(ctx, vec, num_components);
   std::array<Temp, NIR_MAX_VEC_COMPONENTS> elems;
   RegClass rc = RegClass(RegType::vgpr, component_size).as_subdword();

   assert(offset.constantValue() % component_size == 0);
   unsigned skip = offset.constantValue() / component_size;
   for (unsigned i = skip; i < num_components; i++)
      elems[i - skip] = emit_extract_vector(ctx, vec, i, rc);

   if (dst.type() == RegType::vgpr) {
      /* if dst is vgpr - split the src and create a shrunk version according to the mask. */
      num_components = dst.bytes() / component_size;
      aco_ptr<Pseudo_instruction> create_vec{create_instruction<Pseudo_instruction>(
         aco_opcode::p_create_vector, Format::PSEUDO, num_components, 1)};
      for (unsigned i = 0; i < num_components; i++)
         create_vec->operands[i] = Operand(elems[i]);
      create_vec->definitions[0] = Definition(dst);
      bld.insert(std::move(create_vec));

   } else if (skip) {
      /* if dst is sgpr - split the src, but move the original to sgpr. */
      vec = bld.pseudo(aco_opcode::p_as_uniform, bld.def(RegClass(RegType::sgpr, vec.size())), vec);
      byte_align_scalar(ctx, vec, offset, dst);
   } else {
      assert(dst.size() == vec.size());
      bld.pseudo(aco_opcode::p_as_uniform, Definition(dst), vec);
   }

   ctx->allocated_vec.emplace(dst.id(), elems);
}

Temp
get_ssa_temp_tex(struct isel_context* ctx, nir_def* def, bool is_16bit)
{
   RegClass rc = RegClass::get(RegType::vgpr, (is_16bit ? 2 : 4) * def->num_components);
   Temp tmp = get_ssa_temp(ctx, def);
   if (tmp.bytes() != rc.bytes())
      return emit_extract_vector(ctx, tmp, 0, rc);
   else
      return tmp;
}

Temp
bool_to_vector_condition(isel_context* ctx, Temp val, Temp dst = Temp(0, s2))
{
   Builder bld(ctx->program, ctx->block);
   if (!dst.id())
      dst = bld.tmp(bld.lm);

   assert(val.regClass() == s1);
   assert(dst.regClass() == bld.lm);

   return bld.sop2(Builder::s_cselect, Definition(dst), Operand::c32(-1), Operand::zero(),
                   bld.scc(val));
}

Temp
bool_to_scalar_condition(isel_context* ctx, Temp val, Temp dst = Temp(0, s1))
{
   Builder bld(ctx->program, ctx->block);
   if (!dst.id())
      dst = bld.tmp(s1);

   assert(val.regClass() == bld.lm);
   assert(dst.regClass() == s1);

   /* if we're currently in WQM mode, ensure that the source is also computed in WQM */
   bld.sop2(Builder::s_and, bld.def(bld.lm), bld.scc(Definition(dst)), val, Operand(exec, bld.lm));
   return dst;
}

/**
 * Copies the first src_bits of the input to the output Temp. Input bits at positions larger than
 * src_bits and dst_bits are truncated.
 *
 * Sign extension may be applied using the sign_extend parameter. The position of the input sign
 * bit is indicated by src_bits in this case.
 *
 * If dst.bytes() is larger than dst_bits/8, the value of the upper bits is undefined.
 */
Temp
convert_int(isel_context* ctx, Builder& bld, Temp src, unsigned src_bits, unsigned dst_bits,
            bool sign_extend, Temp dst = Temp())
{
   assert(!(sign_extend && dst_bits < src_bits) &&
          "Shrinking integers is not supported for signed inputs");

   if (!dst.id()) {
      if (dst_bits % 32 == 0 || src.type() == RegType::sgpr)
         dst = bld.tmp(src.type(), DIV_ROUND_UP(dst_bits, 32u));
      else
         dst = bld.tmp(RegClass(RegType::vgpr, dst_bits / 8u).as_subdword());
   }

   assert(src.type() == RegType::sgpr || src_bits == src.bytes() * 8);
   assert(dst.type() == RegType::sgpr || dst_bits == dst.bytes() * 8);

   if (dst.bytes() == src.bytes() && dst_bits < src_bits) {
      /* Copy the raw value, leaving an undefined value in the upper bits for
       * the caller to handle appropriately */
      return bld.copy(Definition(dst), src);
   } else if (dst.bytes() < src.bytes()) {
      return bld.pseudo(aco_opcode::p_extract_vector, Definition(dst), src, Operand::zero());
   }

   Temp tmp = dst;
   if (dst_bits == 64)
      tmp = src_bits == 32 ? src : bld.tmp(src.type(), 1);

   if (tmp == src) {
   } else if (src.regClass() == s1) {
      assert(src_bits < 32);
      bld.pseudo(aco_opcode::p_extract, Definition(tmp), bld.def(s1, scc), src, Operand::zero(),
                 Operand::c32(src_bits), Operand::c32((unsigned)sign_extend));
   } else {
      assert(src_bits < 32);
      bld.pseudo(aco_opcode::p_extract, Definition(tmp), src, Operand::zero(),
                 Operand::c32(src_bits), Operand::c32((unsigned)sign_extend));
   }

   if (dst_bits == 64) {
      if (sign_extend && dst.regClass() == s2) {
         Temp high =
            bld.sop2(aco_opcode::s_ashr_i32, bld.def(s1), bld.def(s1, scc), tmp, Operand::c32(31u));
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), tmp, high);
      } else if (sign_extend && dst.regClass() == v2) {
         Temp high = bld.vop2(aco_opcode::v_ashrrev_i32, bld.def(v1), Operand::c32(31u), tmp);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), tmp, high);
      } else {
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), tmp, Operand::zero());
      }
   }

   return dst;
}

enum sgpr_extract_mode {
   sgpr_extract_sext,
   sgpr_extract_zext,
   sgpr_extract_undef,
};

Temp
extract_8_16_bit_sgpr_element(isel_context* ctx, Temp dst, nir_alu_src* src, sgpr_extract_mode mode)
{
   Temp vec = get_ssa_temp(ctx, src->src.ssa);
   unsigned src_size = src->src.ssa->bit_size;
   unsigned swizzle = src->swizzle[0];

   if (vec.size() > 1) {
      assert(src_size == 16);
      vec = emit_extract_vector(ctx, vec, swizzle / 2, s1);
      swizzle = swizzle & 1;
   }

   Builder bld(ctx->program, ctx->block);
   Temp tmp = dst.regClass() == s2 ? bld.tmp(s1) : dst;

   if (mode == sgpr_extract_undef && swizzle == 0)
      bld.copy(Definition(tmp), vec);
   else
      bld.pseudo(aco_opcode::p_extract, Definition(tmp), bld.def(s1, scc), Operand(vec),
                 Operand::c32(swizzle), Operand::c32(src_size),
                 Operand::c32((mode == sgpr_extract_sext)));

   if (dst.regClass() == s2)
      convert_int(ctx, bld, tmp, 32, 64, mode == sgpr_extract_sext, dst);

   return dst;
}

Temp
get_alu_src(struct isel_context* ctx, nir_alu_src src, unsigned size = 1)
{
   if (src.src.ssa->num_components == 1 && size == 1)
      return get_ssa_temp(ctx, src.src.ssa);

   Temp vec = get_ssa_temp(ctx, src.src.ssa);
   unsigned elem_size = src.src.ssa->bit_size / 8u;
   bool identity_swizzle = true;

   for (unsigned i = 0; identity_swizzle && i < size; i++) {
      if (src.swizzle[i] != i)
         identity_swizzle = false;
   }
   if (identity_swizzle)
      return emit_extract_vector(ctx, vec, 0, RegClass::get(vec.type(), elem_size * size));

   assert(elem_size > 0);
   assert(vec.bytes() % elem_size == 0);

   if (elem_size < 4 && vec.type() == RegType::sgpr && size == 1) {
      assert(src.src.ssa->bit_size == 8 || src.src.ssa->bit_size == 16);
      return extract_8_16_bit_sgpr_element(ctx, ctx->program->allocateTmp(s1), &src,
                                           sgpr_extract_undef);
   }

   bool as_uniform = elem_size < 4 && vec.type() == RegType::sgpr;
   if (as_uniform)
      vec = as_vgpr(ctx, vec);

   RegClass elem_rc = elem_size < 4 ? RegClass(vec.type(), elem_size).as_subdword()
                                    : RegClass(vec.type(), elem_size / 4);
   if (size == 1) {
      return emit_extract_vector(ctx, vec, src.swizzle[0], elem_rc);
   } else {
      assert(size <= 4);
      std::array<Temp, NIR_MAX_VEC_COMPONENTS> elems;
      aco_ptr<Pseudo_instruction> vec_instr{create_instruction<Pseudo_instruction>(
         aco_opcode::p_create_vector, Format::PSEUDO, size, 1)};
      for (unsigned i = 0; i < size; ++i) {
         elems[i] = emit_extract_vector(ctx, vec, src.swizzle[i], elem_rc);
         vec_instr->operands[i] = Operand{elems[i]};
      }
      Temp dst = ctx->program->allocateTmp(RegClass(vec.type(), elem_size * size / 4));
      vec_instr->definitions[0] = Definition(dst);
      ctx->block->instructions.emplace_back(std::move(vec_instr));
      ctx->allocated_vec.emplace(dst.id(), elems);
      return as_uniform ? Builder(ctx->program, ctx->block).as_uniform(dst) : dst;
   }
}

Temp
get_alu_src_vop3p(struct isel_context* ctx, nir_alu_src src)
{
   /* returns v2b or v1 for vop3p usage.
    * The source expects exactly 2 16bit components
    * which are within the same dword
    */
   assert(src.src.ssa->bit_size == 16);
   assert(src.swizzle[0] >> 1 == src.swizzle[1] >> 1);

   Temp tmp = get_ssa_temp(ctx, src.src.ssa);
   if (tmp.size() == 1)
      return tmp;

   /* the size is larger than 1 dword: check the swizzle */
   unsigned dword = src.swizzle[0] >> 1;

   /* extract a full dword if possible */
   if (tmp.bytes() >= (dword + 1) * 4) {
      /* if the source is split into components, use p_create_vector */
      auto it = ctx->allocated_vec.find(tmp.id());
      if (it != ctx->allocated_vec.end()) {
         unsigned index = dword << 1;
         Builder bld(ctx->program, ctx->block);
         if (it->second[index].regClass() == v2b)
            return bld.pseudo(aco_opcode::p_create_vector, bld.def(v1), it->second[index],
                              it->second[index + 1]);
      }
      return emit_extract_vector(ctx, tmp, dword, v1);
   } else {
      /* This must be a swizzled access to %a.zz where %a is v6b */
      assert(((src.swizzle[0] | src.swizzle[1]) & 1) == 0);
      assert(tmp.regClass() == v6b && dword == 1);
      return emit_extract_vector(ctx, tmp, dword * 2, v2b);
   }
}

uint32_t
get_alu_src_ub(isel_context* ctx, nir_alu_instr* instr, int src_idx)
{
   nir_scalar scalar = nir_scalar{instr->src[src_idx].src.ssa, instr->src[src_idx].swizzle[0]};
   return nir_unsigned_upper_bound(ctx->shader, ctx->range_ht, scalar, &ctx->ub_config);
}

Temp
convert_pointer_to_64_bit(isel_context* ctx, Temp ptr, bool non_uniform = false)
{
   if (ptr.size() == 2)
      return ptr;
   Builder bld(ctx->program, ctx->block);
   if (ptr.type() == RegType::vgpr && !non_uniform)
      ptr = bld.as_uniform(ptr);
   return bld.pseudo(aco_opcode::p_create_vector, bld.def(RegClass(ptr.type(), 2)), ptr,
                     Operand::c32((unsigned)ctx->options->address32_hi));
}

void
emit_sop2_instruction(isel_context* ctx, nir_alu_instr* instr, aco_opcode op, Temp dst,
                      bool writes_scc, uint8_t uses_ub = 0)
{
   aco_ptr<SOP2_instruction> sop2{
      create_instruction<SOP2_instruction>(op, Format::SOP2, 2, writes_scc ? 2 : 1)};
   sop2->operands[0] = Operand(get_alu_src(ctx, instr->src[0]));
   sop2->operands[1] = Operand(get_alu_src(ctx, instr->src[1]));
   sop2->definitions[0] = Definition(dst);
   if (instr->no_unsigned_wrap)
      sop2->definitions[0].setNUW(true);
   if (writes_scc)
      sop2->definitions[1] = Definition(ctx->program->allocateId(s1), scc, s1);

   for (int i = 0; i < 2; i++) {
      if (uses_ub & (1 << i)) {
         uint32_t src_ub = get_alu_src_ub(ctx, instr, i);
         if (src_ub <= 0xffff)
            sop2->operands[i].set16bit(true);
         else if (src_ub <= 0xffffff)
            sop2->operands[i].set24bit(true);
      }
   }

   ctx->block->instructions.emplace_back(std::move(sop2));
}

void
emit_vop2_instruction(isel_context* ctx, nir_alu_instr* instr, aco_opcode opc, Temp dst,
                      bool commutative, bool swap_srcs = false, bool flush_denorms = false,
                      bool nuw = false, uint8_t uses_ub = 0)
{
   Builder bld(ctx->program, ctx->block);
   bld.is_precise = instr->exact;

   Temp src0 = get_alu_src(ctx, instr->src[swap_srcs ? 1 : 0]);
   Temp src1 = get_alu_src(ctx, instr->src[swap_srcs ? 0 : 1]);
   if (src1.type() == RegType::sgpr) {
      if (commutative && src0.type() == RegType::vgpr) {
         Temp t = src0;
         src0 = src1;
         src1 = t;
      } else {
         src1 = as_vgpr(ctx, src1);
      }
   }

   Operand op[2] = {Operand(src0), Operand(src1)};

   for (int i = 0; i < 2; i++) {
      if (uses_ub & (1 << i)) {
         uint32_t src_ub = get_alu_src_ub(ctx, instr, swap_srcs ? !i : i);
         if (src_ub <= 0xffff)
            op[i].set16bit(true);
         else if (src_ub <= 0xffffff)
            op[i].set24bit(true);
      }
   }

   if (flush_denorms && ctx->program->gfx_level < GFX9) {
      assert(dst.size() == 1);
      Temp tmp = bld.vop2(opc, bld.def(dst.regClass()), op[0], op[1]);
      if (dst.bytes() == 2)
         bld.vop2(aco_opcode::v_mul_f16, Definition(dst), Operand::c16(0x3c00), tmp);
      else
         bld.vop2(aco_opcode::v_mul_f32, Definition(dst), Operand::c32(0x3f800000u), tmp);
   } else {
      if (nuw) {
         bld.nuw().vop2(opc, Definition(dst), op[0], op[1]);
      } else {
         bld.vop2(opc, Definition(dst), op[0], op[1]);
      }
   }
}

void
emit_vop2_instruction_logic64(isel_context* ctx, nir_alu_instr* instr, aco_opcode op, Temp dst)
{
   Builder bld(ctx->program, ctx->block);
   bld.is_precise = instr->exact;

   Temp src0 = get_alu_src(ctx, instr->src[0]);
   Temp src1 = get_alu_src(ctx, instr->src[1]);

   if (src1.type() == RegType::sgpr) {
      assert(src0.type() == RegType::vgpr);
      std::swap(src0, src1);
   }

   Temp src00 = bld.tmp(src0.type(), 1);
   Temp src01 = bld.tmp(src0.type(), 1);
   bld.pseudo(aco_opcode::p_split_vector, Definition(src00), Definition(src01), src0);
   Temp src10 = bld.tmp(v1);
   Temp src11 = bld.tmp(v1);
   bld.pseudo(aco_opcode::p_split_vector, Definition(src10), Definition(src11), src1);
   Temp lo = bld.vop2(op, bld.def(v1), src00, src10);
   Temp hi = bld.vop2(op, bld.def(v1), src01, src11);
   bld.pseudo(aco_opcode::p_create_vector, Definition(dst), lo, hi);
}

void
emit_vop3a_instruction(isel_context* ctx, nir_alu_instr* instr, aco_opcode op, Temp dst,
                       bool flush_denorms = false, unsigned num_sources = 2, bool swap_srcs = false)
{
   assert(num_sources == 2 || num_sources == 3);
   Temp src[3] = {Temp(0, v1), Temp(0, v1), Temp(0, v1)};
   bool has_sgpr = false;
   for (unsigned i = 0; i < num_sources; i++) {
      src[i] = get_alu_src(ctx, instr->src[(swap_srcs && i < 2) ? 1 - i : i]);
      if (has_sgpr)
         src[i] = as_vgpr(ctx, src[i]);
      else
         has_sgpr = src[i].type() == RegType::sgpr;
   }

   Builder bld(ctx->program, ctx->block);
   bld.is_precise = instr->exact;
   if (flush_denorms && ctx->program->gfx_level < GFX9) {
      Temp tmp;
      if (num_sources == 3)
         tmp = bld.vop3(op, bld.def(dst.regClass()), src[0], src[1], src[2]);
      else
         tmp = bld.vop3(op, bld.def(dst.regClass()), src[0], src[1]);
      if (dst.size() == 1)
         bld.vop2(aco_opcode::v_mul_f32, Definition(dst), Operand::c32(0x3f800000u), tmp);
      else
         bld.vop3(aco_opcode::v_mul_f64, Definition(dst), Operand::c64(0x3FF0000000000000), tmp);
   } else if (num_sources == 3) {
      bld.vop3(op, Definition(dst), src[0], src[1], src[2]);
   } else {
      bld.vop3(op, Definition(dst), src[0], src[1]);
   }
}

Builder::Result
emit_vop3p_instruction(isel_context* ctx, nir_alu_instr* instr, aco_opcode op, Temp dst,
                       bool swap_srcs = false)
{
   Temp src0 = get_alu_src_vop3p(ctx, instr->src[swap_srcs]);
   Temp src1 = get_alu_src_vop3p(ctx, instr->src[!swap_srcs]);
   if (src0.type() == RegType::sgpr && src1.type() == RegType::sgpr)
      src1 = as_vgpr(ctx, src1);
   assert(instr->def.num_components == 2);

   /* swizzle to opsel: all swizzles are either 0 (x) or 1 (y) */
   unsigned opsel_lo =
      (instr->src[!swap_srcs].swizzle[0] & 1) << 1 | (instr->src[swap_srcs].swizzle[0] & 1);
   unsigned opsel_hi =
      (instr->src[!swap_srcs].swizzle[1] & 1) << 1 | (instr->src[swap_srcs].swizzle[1] & 1);

   Builder bld(ctx->program, ctx->block);
   bld.is_precise = instr->exact;
   Builder::Result res = bld.vop3p(op, Definition(dst), src0, src1, opsel_lo, opsel_hi);
   return res;
}

void
emit_idot_instruction(isel_context* ctx, nir_alu_instr* instr, aco_opcode op, Temp dst, bool clamp,
                      unsigned neg_lo = 0)
{
   Temp src[3] = {Temp(0, v1), Temp(0, v1), Temp(0, v1)};
   bool has_sgpr = false;
   for (unsigned i = 0; i < 3; i++) {
      src[i] = get_alu_src(ctx, instr->src[i]);
      if (has_sgpr)
         src[i] = as_vgpr(ctx, src[i]);
      else
         has_sgpr = src[i].type() == RegType::sgpr;
   }

   Builder bld(ctx->program, ctx->block);
   bld.is_precise = instr->exact;
   VALU_instruction& vop3p =
      bld.vop3p(op, Definition(dst), src[0], src[1], src[2], 0x0, 0x7)->valu();
   vop3p.clamp = clamp;
   vop3p.neg_lo = neg_lo;
}

void
emit_vop1_instruction(isel_context* ctx, nir_alu_instr* instr, aco_opcode op, Temp dst)
{
   Builder bld(ctx->program, ctx->block);
   bld.is_precise = instr->exact;
   if (dst.type() == RegType::sgpr)
      bld.pseudo(aco_opcode::p_as_uniform, Definition(dst),
                 bld.vop1(op, bld.def(RegType::vgpr, dst.size()), get_alu_src(ctx, instr->src[0])));
   else
      bld.vop1(op, Definition(dst), get_alu_src(ctx, instr->src[0]));
}

void
emit_vopc_instruction(isel_context* ctx, nir_alu_instr* instr, aco_opcode op, Temp dst)
{
   Temp src0 = get_alu_src(ctx, instr->src[0]);
   Temp src1 = get_alu_src(ctx, instr->src[1]);
   assert(src0.size() == src1.size());

   aco_ptr<Instruction> vopc;
   if (src1.type() == RegType::sgpr) {
      if (src0.type() == RegType::vgpr) {
         /* to swap the operands, we might also have to change the opcode */
         switch (op) {
         case aco_opcode::v_cmp_lt_f16: op = aco_opcode::v_cmp_gt_f16; break;
         case aco_opcode::v_cmp_ge_f16: op = aco_opcode::v_cmp_le_f16; break;
         case aco_opcode::v_cmp_lt_i16: op = aco_opcode::v_cmp_gt_i16; break;
         case aco_opcode::v_cmp_ge_i16: op = aco_opcode::v_cmp_le_i16; break;
         case aco_opcode::v_cmp_lt_u16: op = aco_opcode::v_cmp_gt_u16; break;
         case aco_opcode::v_cmp_ge_u16: op = aco_opcode::v_cmp_le_u16; break;
         case aco_opcode::v_cmp_lt_f32: op = aco_opcode::v_cmp_gt_f32; break;
         case aco_opcode::v_cmp_ge_f32: op = aco_opcode::v_cmp_le_f32; break;
         case aco_opcode::v_cmp_lt_i32: op = aco_opcode::v_cmp_gt_i32; break;
         case aco_opcode::v_cmp_ge_i32: op = aco_opcode::v_cmp_le_i32; break;
         case aco_opcode::v_cmp_lt_u32: op = aco_opcode::v_cmp_gt_u32; break;
         case aco_opcode::v_cmp_ge_u32: op = aco_opcode::v_cmp_le_u32; break;
         case aco_opcode::v_cmp_lt_f64: op = aco_opcode::v_cmp_gt_f64; break;
         case aco_opcode::v_cmp_ge_f64: op = aco_opcode::v_cmp_le_f64; break;
         case aco_opcode::v_cmp_lt_i64: op = aco_opcode::v_cmp_gt_i64; break;
         case aco_opcode::v_cmp_ge_i64: op = aco_opcode::v_cmp_le_i64; break;
         case aco_opcode::v_cmp_lt_u64: op = aco_opcode::v_cmp_gt_u64; break;
         case aco_opcode::v_cmp_ge_u64: op = aco_opcode::v_cmp_le_u64; break;
         default: /* eq and ne are commutative */ break;
         }
         Temp t = src0;
         src0 = src1;
         src1 = t;
      } else {
         src1 = as_vgpr(ctx, src1);
      }
   }

   Builder bld(ctx->program, ctx->block);
   bld.vopc(op, Definition(dst), src0, src1);
}

void
emit_sopc_instruction(isel_context* ctx, nir_alu_instr* instr, aco_opcode op, Temp dst)
{
   Temp src0 = get_alu_src(ctx, instr->src[0]);
   Temp src1 = get_alu_src(ctx, instr->src[1]);
   Builder bld(ctx->program, ctx->block);

   assert(dst.regClass() == bld.lm);
   assert(src0.type() == RegType::sgpr);
   assert(src1.type() == RegType::sgpr);

   /* Emit the SALU comparison instruction */
   Temp cmp = bld.sopc(op, bld.scc(bld.def(s1)), src0, src1);
   /* Turn the result into a per-lane bool */
   bool_to_vector_condition(ctx, cmp, dst);
}

void
emit_comparison(isel_context* ctx, nir_alu_instr* instr, Temp dst, aco_opcode v16_op,
                aco_opcode v32_op, aco_opcode v64_op, aco_opcode s32_op = aco_opcode::num_opcodes,
                aco_opcode s64_op = aco_opcode::num_opcodes)
{
   aco_opcode s_op = instr->src[0].src.ssa->bit_size == 64   ? s64_op
                     : instr->src[0].src.ssa->bit_size == 32 ? s32_op
                                                             : aco_opcode::num_opcodes;
   aco_opcode v_op = instr->src[0].src.ssa->bit_size == 64   ? v64_op
                     : instr->src[0].src.ssa->bit_size == 32 ? v32_op
                                                             : v16_op;
   bool use_valu = s_op == aco_opcode::num_opcodes || instr->def.divergent ||
                   get_ssa_temp(ctx, instr->src[0].src.ssa).type() == RegType::vgpr ||
                   get_ssa_temp(ctx, instr->src[1].src.ssa).type() == RegType::vgpr;
   aco_opcode op = use_valu ? v_op : s_op;
   assert(op != aco_opcode::num_opcodes);
   assert(dst.regClass() == ctx->program->lane_mask);

   if (use_valu)
      emit_vopc_instruction(ctx, instr, op, dst);
   else
      emit_sopc_instruction(ctx, instr, op, dst);
}

void
emit_boolean_logic(isel_context* ctx, nir_alu_instr* instr, Builder::WaveSpecificOpcode op,
                   Temp dst)
{
   Builder bld(ctx->program, ctx->block);
   Temp src0 = get_alu_src(ctx, instr->src[0]);
   Temp src1 = get_alu_src(ctx, instr->src[1]);

   assert(dst.regClass() == bld.lm);
   assert(src0.regClass() == bld.lm);
   assert(src1.regClass() == bld.lm);

   bld.sop2(op, Definition(dst), bld.def(s1, scc), src0, src1);
}

void
select_vec2(isel_context* ctx, Temp dst, Temp cond, Temp then, Temp els)
{
   Builder bld(ctx->program, ctx->block);

   Temp then_lo = bld.tmp(v1), then_hi = bld.tmp(v1);
   bld.pseudo(aco_opcode::p_split_vector, Definition(then_lo), Definition(then_hi), then);
   Temp else_lo = bld.tmp(v1), else_hi = bld.tmp(v1);
   bld.pseudo(aco_opcode::p_split_vector, Definition(else_lo), Definition(else_hi), els);

   Temp dst0 = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), else_lo, then_lo, cond);
   Temp dst1 = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), else_hi, then_hi, cond);

   bld.pseudo(aco_opcode::p_create_vector, Definition(dst), dst0, dst1);
}

void
emit_bcsel(isel_context* ctx, nir_alu_instr* instr, Temp dst)
{
   Builder bld(ctx->program, ctx->block);
   Temp cond = get_alu_src(ctx, instr->src[0]);
   Temp then = get_alu_src(ctx, instr->src[1]);
   Temp els = get_alu_src(ctx, instr->src[2]);

   assert(cond.regClass() == bld.lm);

   if (dst.type() == RegType::vgpr) {
      aco_ptr<Instruction> bcsel;
      if (dst.size() == 1) {
         then = as_vgpr(ctx, then);
         els = as_vgpr(ctx, els);

         bld.vop2(aco_opcode::v_cndmask_b32, Definition(dst), els, then, cond);
      } else if (dst.size() == 2) {
         select_vec2(ctx, dst, cond, then, els);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      return;
   }

   if (instr->def.bit_size == 1) {
      assert(dst.regClass() == bld.lm);
      assert(then.regClass() == bld.lm);
      assert(els.regClass() == bld.lm);
   }

   if (!nir_src_is_divergent(instr->src[0].src)) { /* uniform condition and values in sgpr */
      if (dst.regClass() == s1 || dst.regClass() == s2) {
         assert((then.regClass() == s1 || then.regClass() == s2) &&
                els.regClass() == then.regClass());
         assert(dst.size() == then.size());
         aco_opcode op =
            dst.regClass() == s1 ? aco_opcode::s_cselect_b32 : aco_opcode::s_cselect_b64;
         bld.sop2(op, Definition(dst), then, els, bld.scc(bool_to_scalar_condition(ctx, cond)));
      } else {
         isel_err(&instr->instr, "Unimplemented uniform bcsel bit size");
      }
      return;
   }

   /* divergent boolean bcsel
    * this implements bcsel on bools: dst = s0 ? s1 : s2
    * are going to be: dst = (s0 & s1) | (~s0 & s2) */
   assert(instr->def.bit_size == 1);

   if (cond.id() != then.id())
      then = bld.sop2(Builder::s_and, bld.def(bld.lm), bld.def(s1, scc), cond, then);

   if (cond.id() == els.id())
      bld.copy(Definition(dst), then);
   else
      bld.sop2(Builder::s_or, Definition(dst), bld.def(s1, scc), then,
               bld.sop2(Builder::s_andn2, bld.def(bld.lm), bld.def(s1, scc), els, cond));
}

void
emit_scaled_op(isel_context* ctx, Builder& bld, Definition dst, Temp val, aco_opcode op,
               uint32_t undo)
{
   /* multiply by 16777216 to handle denormals */
   Temp is_denormal = bld.tmp(bld.lm);
   VALU_instruction& valu =
      bld.vopc_e64(aco_opcode::v_cmp_class_f32, Definition(is_denormal), val, Operand::c32(1u << 4))
         ->valu();
   valu.neg[0] = true;
   valu.abs[0] = true;
   Temp scaled = bld.vop2(aco_opcode::v_mul_f32, bld.def(v1), Operand::c32(0x4b800000u), val);
   scaled = bld.vop1(op, bld.def(v1), scaled);
   scaled = bld.vop2(aco_opcode::v_mul_f32, bld.def(v1), Operand::c32(undo), scaled);

   Temp not_scaled = bld.vop1(op, bld.def(v1), val);

   bld.vop2(aco_opcode::v_cndmask_b32, dst, not_scaled, scaled, is_denormal);
}

void
emit_rcp(isel_context* ctx, Builder& bld, Definition dst, Temp val)
{
   if (ctx->block->fp_mode.denorm32 == 0) {
      bld.vop1(aco_opcode::v_rcp_f32, dst, val);
      return;
   }

   emit_scaled_op(ctx, bld, dst, val, aco_opcode::v_rcp_f32, 0x4b800000u);
}

void
emit_rsq(isel_context* ctx, Builder& bld, Definition dst, Temp val)
{
   if (ctx->block->fp_mode.denorm32 == 0) {
      bld.vop1(aco_opcode::v_rsq_f32, dst, val);
      return;
   }

   emit_scaled_op(ctx, bld, dst, val, aco_opcode::v_rsq_f32, 0x45800000u);
}

void
emit_sqrt(isel_context* ctx, Builder& bld, Definition dst, Temp val)
{
   if (ctx->block->fp_mode.denorm32 == 0) {
      bld.vop1(aco_opcode::v_sqrt_f32, dst, val);
      return;
   }

   emit_scaled_op(ctx, bld, dst, val, aco_opcode::v_sqrt_f32, 0x39800000u);
}

void
emit_log2(isel_context* ctx, Builder& bld, Definition dst, Temp val)
{
   if (ctx->block->fp_mode.denorm32 == 0) {
      bld.vop1(aco_opcode::v_log_f32, dst, val);
      return;
   }

   emit_scaled_op(ctx, bld, dst, val, aco_opcode::v_log_f32, 0xc1c00000u);
}

Temp
emit_trunc_f64(isel_context* ctx, Builder& bld, Definition dst, Temp val)
{
   if (ctx->options->gfx_level >= GFX7)
      return bld.vop1(aco_opcode::v_trunc_f64, Definition(dst), val);

   /* GFX6 doesn't support V_TRUNC_F64, lower it. */
   /* TODO: create more efficient code! */
   if (val.type() == RegType::sgpr)
      val = as_vgpr(ctx, val);

   /* Split the input value. */
   Temp val_lo = bld.tmp(v1), val_hi = bld.tmp(v1);
   bld.pseudo(aco_opcode::p_split_vector, Definition(val_lo), Definition(val_hi), val);

   /* Extract the exponent and compute the unbiased value. */
   Temp exponent =
      bld.vop3(aco_opcode::v_bfe_u32, bld.def(v1), val_hi, Operand::c32(20u), Operand::c32(11u));
   exponent = bld.vsub32(bld.def(v1), exponent, Operand::c32(1023u));

   /* Extract the fractional part. */
   Temp fract_mask = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), Operand::c32(-1u),
                                Operand::c32(0x000fffffu));
   fract_mask = bld.vop3(aco_opcode::v_lshr_b64, bld.def(v2), fract_mask, exponent);

   Temp fract_mask_lo = bld.tmp(v1), fract_mask_hi = bld.tmp(v1);
   bld.pseudo(aco_opcode::p_split_vector, Definition(fract_mask_lo), Definition(fract_mask_hi),
              fract_mask);

   Temp fract_lo = bld.tmp(v1), fract_hi = bld.tmp(v1);
   Temp tmp = bld.vop1(aco_opcode::v_not_b32, bld.def(v1), fract_mask_lo);
   fract_lo = bld.vop2(aco_opcode::v_and_b32, bld.def(v1), val_lo, tmp);
   tmp = bld.vop1(aco_opcode::v_not_b32, bld.def(v1), fract_mask_hi);
   fract_hi = bld.vop2(aco_opcode::v_and_b32, bld.def(v1), val_hi, tmp);

   /* Get the sign bit. */
   Temp sign = bld.vop2(aco_opcode::v_and_b32, bld.def(v1), Operand::c32(0x80000000u), val_hi);

   /* Decide the operation to apply depending on the unbiased exponent. */
   Temp exp_lt0 =
      bld.vopc_e64(aco_opcode::v_cmp_lt_i32, bld.def(bld.lm), exponent, Operand::zero());
   Temp dst_lo = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), fract_lo,
                          bld.copy(bld.def(v1), Operand::zero()), exp_lt0);
   Temp dst_hi = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), fract_hi, sign, exp_lt0);
   Temp exp_gt51 = bld.vopc_e64(aco_opcode::v_cmp_gt_i32, bld.def(s2), exponent, Operand::c32(51u));
   dst_lo = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), dst_lo, val_lo, exp_gt51);
   dst_hi = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), dst_hi, val_hi, exp_gt51);

   return bld.pseudo(aco_opcode::p_create_vector, Definition(dst), dst_lo, dst_hi);
}

Temp
emit_floor_f64(isel_context* ctx, Builder& bld, Definition dst, Temp val)
{
   if (ctx->options->gfx_level >= GFX7)
      return bld.vop1(aco_opcode::v_floor_f64, Definition(dst), val);

   /* GFX6 doesn't support V_FLOOR_F64, lower it (note that it's actually
    * lowered at NIR level for precision reasons). */
   Temp src0 = as_vgpr(ctx, val);

   Temp min_val = bld.pseudo(aco_opcode::p_create_vector, bld.def(s2), Operand::c32(-1u),
                             Operand::c32(0x3fefffffu));

   Temp isnan = bld.vopc(aco_opcode::v_cmp_neq_f64, bld.def(bld.lm), src0, src0);
   Temp fract = bld.vop1(aco_opcode::v_fract_f64, bld.def(v2), src0);
   Temp min = bld.vop3(aco_opcode::v_min_f64, bld.def(v2), fract, min_val);

   Temp then_lo = bld.tmp(v1), then_hi = bld.tmp(v1);
   bld.pseudo(aco_opcode::p_split_vector, Definition(then_lo), Definition(then_hi), src0);
   Temp else_lo = bld.tmp(v1), else_hi = bld.tmp(v1);
   bld.pseudo(aco_opcode::p_split_vector, Definition(else_lo), Definition(else_hi), min);

   Temp dst0 = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), else_lo, then_lo, isnan);
   Temp dst1 = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), else_hi, then_hi, isnan);

   Temp v = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), dst0, dst1);

   Instruction* add = bld.vop3(aco_opcode::v_add_f64, Definition(dst), src0, v);
   add->valu().neg[1] = true;

   return add->definitions[0].getTemp();
}

Temp
uadd32_sat(Builder& bld, Definition dst, Temp src0, Temp src1)
{
   if (bld.program->gfx_level < GFX8) {
      Builder::Result add = bld.vadd32(bld.def(v1), src0, src1, true);
      return bld.vop2_e64(aco_opcode::v_cndmask_b32, dst, add.def(0).getTemp(), Operand::c32(-1),
                          add.def(1).getTemp());
   }

   Builder::Result add(NULL);
   if (bld.program->gfx_level >= GFX9) {
      add = bld.vop2_e64(aco_opcode::v_add_u32, dst, src0, src1);
   } else {
      add = bld.vop2_e64(aco_opcode::v_add_co_u32, dst, bld.def(bld.lm), src0, src1);
   }
   add->valu().clamp = 1;
   return dst.getTemp();
}

Temp
usub32_sat(Builder& bld, Definition dst, Temp src0, Temp src1)
{
   if (bld.program->gfx_level < GFX8) {
      Builder::Result sub = bld.vsub32(bld.def(v1), src0, src1, true);
      return bld.vop2_e64(aco_opcode::v_cndmask_b32, dst, sub.def(0).getTemp(), Operand::c32(0u),
                          sub.def(1).getTemp());
   }

   Builder::Result sub(NULL);
   if (bld.program->gfx_level >= GFX9) {
      sub = bld.vop2_e64(aco_opcode::v_sub_u32, dst, src0, src1);
   } else {
      sub = bld.vop2_e64(aco_opcode::v_sub_co_u32, dst, bld.def(bld.lm), src0, src1);
   }
   sub->valu().clamp = 1;
   return dst.getTemp();
}

void
emit_vec2_f2f16(isel_context* ctx, nir_alu_instr* instr, Temp dst)
{
   Builder bld(ctx->program, ctx->block);
   Temp src = get_ssa_temp(ctx, instr->src[0].src.ssa);
   RegClass rc = RegClass(src.regClass().type(), instr->src[0].src.ssa->bit_size / 32);
   Temp src0 = emit_extract_vector(ctx, src, instr->src[0].swizzle[0], rc);
   Temp src1 = emit_extract_vector(ctx, src, instr->src[0].swizzle[1], rc);

   src1 = as_vgpr(ctx, src1);
   if (ctx->program->gfx_level == GFX8 || ctx->program->gfx_level == GFX9)
      bld.vop3(aco_opcode::v_cvt_pkrtz_f16_f32_e64, Definition(dst), src0, src1);
   else
      bld.vop2(aco_opcode::v_cvt_pkrtz_f16_f32, Definition(dst), src0, src1);
   emit_split_vector(ctx, dst, 2);
}

void
visit_alu_instr(isel_context* ctx, nir_alu_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   bld.is_precise = instr->exact;
   Temp dst = get_ssa_temp(ctx, &instr->def);
   switch (instr->op) {
   case nir_op_vec2:
   case nir_op_vec3:
   case nir_op_vec4:
   case nir_op_vec5:
   case nir_op_vec8:
   case nir_op_vec16: {
      std::array<Temp, NIR_MAX_VEC_COMPONENTS> elems;
      unsigned num = instr->def.num_components;
      for (unsigned i = 0; i < num; ++i)
         elems[i] = get_alu_src(ctx, instr->src[i]);

      if (instr->def.bit_size >= 32 || dst.type() == RegType::vgpr) {
         aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
            aco_opcode::p_create_vector, Format::PSEUDO, instr->def.num_components, 1)};
         RegClass elem_rc = RegClass::get(RegType::vgpr, instr->def.bit_size / 8u);
         for (unsigned i = 0; i < num; ++i) {
            if (elems[i].type() == RegType::sgpr && elem_rc.is_subdword())
               elems[i] = emit_extract_vector(ctx, elems[i], 0, elem_rc);
            vec->operands[i] = Operand{elems[i]};
         }
         vec->definitions[0] = Definition(dst);
         ctx->block->instructions.emplace_back(std::move(vec));
         ctx->allocated_vec.emplace(dst.id(), elems);
      } else {
         bool use_s_pack = ctx->program->gfx_level >= GFX9;
         Temp mask = bld.copy(bld.def(s1), Operand::c32((1u << instr->def.bit_size) - 1));

         std::array<Temp, NIR_MAX_VEC_COMPONENTS> packed;
         uint32_t const_vals[NIR_MAX_VEC_COMPONENTS] = {};
         for (unsigned i = 0; i < num; i++) {
            unsigned packed_size = use_s_pack ? 16 : 32;
            unsigned idx = i * instr->def.bit_size / packed_size;
            unsigned offset = i * instr->def.bit_size % packed_size;
            if (nir_src_is_const(instr->src[i].src)) {
               const_vals[idx] |= nir_src_as_uint(instr->src[i].src) << offset;
               continue;
            }
            if (nir_src_is_undef(instr->src[i].src))
               continue;

            if (offset != packed_size - instr->def.bit_size)
               elems[i] =
                  bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc), elems[i], mask);

            if (offset)
               elems[i] = bld.sop2(aco_opcode::s_lshl_b32, bld.def(s1), bld.def(s1, scc), elems[i],
                                   Operand::c32(offset));

            if (packed[idx].id())
               packed[idx] = bld.sop2(aco_opcode::s_or_b32, bld.def(s1), bld.def(s1, scc), elems[i],
                                      packed[idx]);
            else
               packed[idx] = elems[i];
         }

         if (use_s_pack) {
            for (unsigned i = 0; i < dst.size(); i++) {
               bool same = !!packed[i * 2].id() == !!packed[i * 2 + 1].id();

               if (packed[i * 2].id() && packed[i * 2 + 1].id())
                  packed[i] = bld.sop2(aco_opcode::s_pack_ll_b32_b16, bld.def(s1), packed[i * 2],
                                       packed[i * 2 + 1]);
               else if (packed[i * 2 + 1].id())
                  packed[i] = bld.sop2(aco_opcode::s_pack_ll_b32_b16, bld.def(s1),
                                       Operand::c32(const_vals[i * 2]), packed[i * 2 + 1]);
               else if (packed[i * 2].id())
                  packed[i] = bld.sop2(aco_opcode::s_pack_ll_b32_b16, bld.def(s1), packed[i * 2],
                                       Operand::c32(const_vals[i * 2 + 1]));
               else
                  packed[i] = Temp(); /* Both constants, so reset the entry */

               if (same)
                  const_vals[i] = const_vals[i * 2] | (const_vals[i * 2 + 1] << 16);
               else
                  const_vals[i] = 0;
            }
         }

         for (unsigned i = 0; i < dst.size(); i++) {
            if (const_vals[i] && packed[i].id())
               packed[i] = bld.sop2(aco_opcode::s_or_b32, bld.def(s1), bld.def(s1, scc),
                                    Operand::c32(const_vals[i]), packed[i]);
            else if (!packed[i].id())
               packed[i] = bld.copy(bld.def(s1), Operand::c32(const_vals[i]));
         }

         if (dst.size() == 1)
            bld.copy(Definition(dst), packed[0]);
         else {
            aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
               aco_opcode::p_create_vector, Format::PSEUDO, dst.size(), 1)};
            vec->definitions[0] = Definition(dst);
            for (unsigned i = 0; i < dst.size(); ++i)
               vec->operands[i] = Operand(packed[i]);
            bld.insert(std::move(vec));
         }
      }
      break;
   }
   case nir_op_mov: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (src.type() == RegType::vgpr && dst.type() == RegType::sgpr) {
         /* use size() instead of bytes() for 8/16-bit */
         assert(src.size() == dst.size() && "wrong src or dst register class for nir_op_mov");
         bld.pseudo(aco_opcode::p_as_uniform, Definition(dst), src);
      } else {
         assert(src.bytes() == dst.bytes() && "wrong src or dst register class for nir_op_mov");
         bld.copy(Definition(dst), src);
      }
      break;
   }
   case nir_op_inot: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (dst.regClass() == v1 || dst.regClass() == v2b || dst.regClass() == v1b) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_not_b32, dst);
      } else if (dst.regClass() == v2) {
         Temp lo = bld.tmp(v1), hi = bld.tmp(v1);
         bld.pseudo(aco_opcode::p_split_vector, Definition(lo), Definition(hi), src);
         lo = bld.vop1(aco_opcode::v_not_b32, bld.def(v1), lo);
         hi = bld.vop1(aco_opcode::v_not_b32, bld.def(v1), hi);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), lo, hi);
      } else if (dst.type() == RegType::sgpr) {
         aco_opcode opcode = dst.size() == 1 ? aco_opcode::s_not_b32 : aco_opcode::s_not_b64;
         bld.sop1(opcode, Definition(dst), bld.def(s1, scc), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_iabs: {
      if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         Temp src = get_alu_src_vop3p(ctx, instr->src[0]);

         unsigned opsel_lo = (instr->src[0].swizzle[0] & 1) << 1;
         unsigned opsel_hi = ((instr->src[0].swizzle[1] & 1) << 1) | 1;

         Temp sub = bld.vop3p(aco_opcode::v_pk_sub_u16, Definition(bld.tmp(v1)), Operand::zero(),
                              src, opsel_lo, opsel_hi);
         bld.vop3p(aco_opcode::v_pk_max_i16, Definition(dst), sub, src, opsel_lo, opsel_hi);
         break;
      }
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (dst.regClass() == s1) {
         bld.sop1(aco_opcode::s_abs_i32, Definition(dst), bld.def(s1, scc), src);
      } else if (dst.regClass() == v1) {
         bld.vop2(aco_opcode::v_max_i32, Definition(dst), src,
                  bld.vsub32(bld.def(v1), Operand::zero(), src));
      } else if (dst.regClass() == v2b && ctx->program->gfx_level >= GFX10) {
         bld.vop3(
            aco_opcode::v_max_i16_e64, Definition(dst), src,
            bld.vop3(aco_opcode::v_sub_u16_e64, Definition(bld.tmp(v2b)), Operand::zero(2), src));
      } else if (dst.regClass() == v2b) {
         src = as_vgpr(ctx, src);
         bld.vop2(aco_opcode::v_max_i16, Definition(dst), src,
                  bld.vop2(aco_opcode::v_sub_u16, Definition(bld.tmp(v2b)), Operand::zero(2), src));
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_isign: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (dst.regClass() == s1) {
         Temp tmp =
            bld.sop2(aco_opcode::s_max_i32, bld.def(s1), bld.def(s1, scc), src, Operand::c32(-1));
         bld.sop2(aco_opcode::s_min_i32, Definition(dst), bld.def(s1, scc), tmp, Operand::c32(1u));
      } else if (dst.regClass() == s2) {
         Temp neg =
            bld.sop2(aco_opcode::s_ashr_i64, bld.def(s2), bld.def(s1, scc), src, Operand::c32(63u));
         Temp neqz;
         if (ctx->program->gfx_level >= GFX8)
            neqz = bld.sopc(aco_opcode::s_cmp_lg_u64, bld.def(s1, scc), src, Operand::zero());
         else
            neqz =
               bld.sop2(aco_opcode::s_or_b64, bld.def(s2), bld.def(s1, scc), src, Operand::zero())
                  .def(1)
                  .getTemp();
         /* SCC gets zero-extended to 64 bit */
         bld.sop2(aco_opcode::s_or_b64, Definition(dst), bld.def(s1, scc), neg, bld.scc(neqz));
      } else if (dst.regClass() == v1) {
         bld.vop3(aco_opcode::v_med3_i32, Definition(dst), Operand::c32(-1), src, Operand::c32(1u));
      } else if (dst.regClass() == v2b && ctx->program->gfx_level >= GFX9) {
         bld.vop3(aco_opcode::v_med3_i16, Definition(dst), Operand::c16(-1), src, Operand::c16(1u));
      } else if (dst.regClass() == v2b) {
         src = as_vgpr(ctx, src);
         bld.vop2(aco_opcode::v_max_i16, Definition(dst), Operand::c16(-1),
                  bld.vop2(aco_opcode::v_min_i16, Definition(bld.tmp(v1)), Operand::c16(1u), src));
      } else if (dst.regClass() == v2) {
         Temp upper = emit_extract_vector(ctx, src, 1, v1);
         Temp neg = bld.vop2(aco_opcode::v_ashrrev_i32, bld.def(v1), Operand::c32(31u), upper);
         Temp gtz = bld.vopc(aco_opcode::v_cmp_ge_i64, bld.def(bld.lm), Operand::zero(), src);
         Temp lower = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), Operand::c32(1u), neg, gtz);
         upper = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), Operand::zero(), neg, gtz);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), lower, upper);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_imax: {
      if (dst.regClass() == v2b && ctx->program->gfx_level >= GFX10) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_max_i16_e64, dst);
      } else if (dst.regClass() == v2b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_max_i16, dst, true);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_max_i16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_max_i32, dst, true);
      } else if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_max_i32, dst, true);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_umax: {
      if (dst.regClass() == v2b && ctx->program->gfx_level >= GFX10) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_max_u16_e64, dst);
      } else if (dst.regClass() == v2b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_max_u16, dst, true);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_max_u16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_max_u32, dst, true);
      } else if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_max_u32, dst, true);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_imin: {
      if (dst.regClass() == v2b && ctx->program->gfx_level >= GFX10) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_min_i16_e64, dst);
      } else if (dst.regClass() == v2b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_min_i16, dst, true);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_min_i16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_min_i32, dst, true);
      } else if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_min_i32, dst, true);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_umin: {
      if (dst.regClass() == v2b && ctx->program->gfx_level >= GFX10) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_min_u16_e64, dst);
      } else if (dst.regClass() == v2b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_min_u16, dst, true);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_min_u16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_min_u32, dst, true);
      } else if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_min_u32, dst, true);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ior: {
      if (instr->def.bit_size == 1) {
         emit_boolean_logic(ctx, instr, Builder::s_or, dst);
      } else if (dst.regClass() == v1 || dst.regClass() == v2b || dst.regClass() == v1b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_or_b32, dst, true);
      } else if (dst.regClass() == v2) {
         emit_vop2_instruction_logic64(ctx, instr, aco_opcode::v_or_b32, dst);
      } else if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_or_b32, dst, true);
      } else if (dst.regClass() == s2) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_or_b64, dst, true);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_iand: {
      if (instr->def.bit_size == 1) {
         emit_boolean_logic(ctx, instr, Builder::s_and, dst);
      } else if (dst.regClass() == v1 || dst.regClass() == v2b || dst.regClass() == v1b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_and_b32, dst, true);
      } else if (dst.regClass() == v2) {
         emit_vop2_instruction_logic64(ctx, instr, aco_opcode::v_and_b32, dst);
      } else if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_and_b32, dst, true);
      } else if (dst.regClass() == s2) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_and_b64, dst, true);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ixor: {
      if (instr->def.bit_size == 1) {
         emit_boolean_logic(ctx, instr, Builder::s_xor, dst);
      } else if (dst.regClass() == v1 || dst.regClass() == v2b || dst.regClass() == v1b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_xor_b32, dst, true);
      } else if (dst.regClass() == v2) {
         emit_vop2_instruction_logic64(ctx, instr, aco_opcode::v_xor_b32, dst);
      } else if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_xor_b32, dst, true);
      } else if (dst.regClass() == s2) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_xor_b64, dst, true);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ushr: {
      if (dst.regClass() == v2b && ctx->program->gfx_level >= GFX10) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_lshrrev_b16_e64, dst, false, 2, true);
      } else if (dst.regClass() == v2b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_lshrrev_b16, dst, false, true);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_lshrrev_b16, dst, true);
      } else if (dst.regClass() == v1) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_lshrrev_b32, dst, false, true);
      } else if (dst.regClass() == v2 && ctx->program->gfx_level >= GFX8) {
         bld.vop3(aco_opcode::v_lshrrev_b64, Definition(dst), get_alu_src(ctx, instr->src[1]),
                  get_alu_src(ctx, instr->src[0]));
      } else if (dst.regClass() == v2) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_lshr_b64, dst);
      } else if (dst.regClass() == s2) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_lshr_b64, dst, true);
      } else if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_lshr_b32, dst, true);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ishl: {
      if (dst.regClass() == v2b && ctx->program->gfx_level >= GFX10) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_lshlrev_b16_e64, dst, false, 2, true);
      } else if (dst.regClass() == v2b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_lshlrev_b16, dst, false, true);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_lshlrev_b16, dst, true);
      } else if (dst.regClass() == v1) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_lshlrev_b32, dst, false, true, false,
                               false, 2);
      } else if (dst.regClass() == v2 && ctx->program->gfx_level >= GFX8) {
         bld.vop3(aco_opcode::v_lshlrev_b64, Definition(dst), get_alu_src(ctx, instr->src[1]),
                  get_alu_src(ctx, instr->src[0]));
      } else if (dst.regClass() == v2) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_lshl_b64, dst);
      } else if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_lshl_b32, dst, true, 1);
      } else if (dst.regClass() == s2) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_lshl_b64, dst, true);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ishr: {
      if (dst.regClass() == v2b && ctx->program->gfx_level >= GFX10) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_ashrrev_i16_e64, dst, false, 2, true);
      } else if (dst.regClass() == v2b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_ashrrev_i16, dst, false, true);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_ashrrev_i16, dst, true);
      } else if (dst.regClass() == v1) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_ashrrev_i32, dst, false, true);
      } else if (dst.regClass() == v2 && ctx->program->gfx_level >= GFX8) {
         bld.vop3(aco_opcode::v_ashrrev_i64, Definition(dst), get_alu_src(ctx, instr->src[1]),
                  get_alu_src(ctx, instr->src[0]));
      } else if (dst.regClass() == v2) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_ashr_i64, dst);
      } else if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_ashr_i32, dst, true);
      } else if (dst.regClass() == s2) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_ashr_i64, dst, true);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_find_lsb: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (src.regClass() == s1) {
         bld.sop1(aco_opcode::s_ff1_i32_b32, Definition(dst), src);
      } else if (src.regClass() == v1) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_ffbl_b32, dst);
      } else if (src.regClass() == s2) {
         bld.sop1(aco_opcode::s_ff1_i32_b64, Definition(dst), src);
      } else if (src.regClass() == v2) {
         Temp lo = bld.tmp(v1), hi = bld.tmp(v1);
         bld.pseudo(aco_opcode::p_split_vector, Definition(lo), Definition(hi), src);
         lo = bld.vop1(aco_opcode::v_ffbl_b32, bld.def(v1), lo);
         hi = bld.vop1(aco_opcode::v_ffbl_b32, bld.def(v1), hi);
         hi = uadd32_sat(bld, bld.def(v1), bld.copy(bld.def(s1), Operand::c32(32u)), hi);
         bld.vop2(aco_opcode::v_min_u32, Definition(dst), lo, hi);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ufind_msb:
   case nir_op_ifind_msb: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (src.regClass() == s1 || src.regClass() == s2) {
         aco_opcode op = src.regClass() == s2
                            ? (instr->op == nir_op_ufind_msb ? aco_opcode::s_flbit_i32_b64
                                                             : aco_opcode::s_flbit_i32_i64)
                            : (instr->op == nir_op_ufind_msb ? aco_opcode::s_flbit_i32_b32
                                                             : aco_opcode::s_flbit_i32);
         Temp msb_rev = bld.sop1(op, bld.def(s1), src);

         Builder::Result sub = bld.sop2(aco_opcode::s_sub_u32, bld.def(s1), bld.def(s1, scc),
                                        Operand::c32(src.size() * 32u - 1u), msb_rev);
         Temp msb = sub.def(0).getTemp();
         Temp carry = sub.def(1).getTemp();

         bld.sop2(aco_opcode::s_cselect_b32, Definition(dst), Operand::c32(-1), msb,
                  bld.scc(carry));
      } else if (src.regClass() == v1) {
         aco_opcode op =
            instr->op == nir_op_ufind_msb ? aco_opcode::v_ffbh_u32 : aco_opcode::v_ffbh_i32;
         Temp msb_rev = bld.tmp(v1);
         emit_vop1_instruction(ctx, instr, op, msb_rev);
         Temp msb = bld.tmp(v1);
         Temp carry =
            bld.vsub32(Definition(msb), Operand::c32(31u), Operand(msb_rev), true).def(1).getTemp();
         bld.vop2(aco_opcode::v_cndmask_b32, Definition(dst), msb, msb_rev, carry);
      } else if (src.regClass() == v2) {
         aco_opcode op =
            instr->op == nir_op_ufind_msb ? aco_opcode::v_ffbh_u32 : aco_opcode::v_ffbh_i32;

         Temp lo = bld.tmp(v1), hi = bld.tmp(v1);
         bld.pseudo(aco_opcode::p_split_vector, Definition(lo), Definition(hi), src);

         lo = uadd32_sat(bld, bld.def(v1), bld.copy(bld.def(s1), Operand::c32(32u)),
                         bld.vop1(op, bld.def(v1), lo));
         hi = bld.vop1(op, bld.def(v1), hi);
         Temp found_hi = bld.vopc(aco_opcode::v_cmp_lg_u32, bld.def(bld.lm), Operand::c32(-1), hi);

         Temp msb_rev = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), lo, hi, found_hi);

         Temp msb = bld.tmp(v1);
         Temp carry =
            bld.vsub32(Definition(msb), Operand::c32(63u), Operand(msb_rev), true).def(1).getTemp();
         bld.vop2(aco_opcode::v_cndmask_b32, Definition(dst), msb, msb_rev, carry);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ufind_msb_rev:
   case nir_op_ifind_msb_rev: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (src.regClass() == s1) {
         aco_opcode op = instr->op == nir_op_ufind_msb_rev ? aco_opcode::s_flbit_i32_b32
                                                           : aco_opcode::s_flbit_i32;
         bld.sop1(op, Definition(dst), src);
      } else if (src.regClass() == v1) {
         aco_opcode op =
            instr->op == nir_op_ufind_msb_rev ? aco_opcode::v_ffbh_u32 : aco_opcode::v_ffbh_i32;
         emit_vop1_instruction(ctx, instr, op, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_bitfield_reverse: {
      if (dst.regClass() == s1) {
         bld.sop1(aco_opcode::s_brev_b32, Definition(dst), get_alu_src(ctx, instr->src[0]));
      } else if (dst.regClass() == v1) {
         bld.vop1(aco_opcode::v_bfrev_b32, Definition(dst), get_alu_src(ctx, instr->src[0]));
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_iadd: {
      if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_add_u32, dst, true);
         break;
      } else if (dst.bytes() <= 2 && ctx->program->gfx_level >= GFX10) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_add_u16_e64, dst);
         break;
      } else if (dst.bytes() <= 2 && ctx->program->gfx_level >= GFX8) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_add_u16, dst, true);
         break;
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_add_u16, dst);
         break;
      }

      Temp src0 = get_alu_src(ctx, instr->src[0]);
      Temp src1 = get_alu_src(ctx, instr->src[1]);
      if (dst.type() == RegType::vgpr && dst.bytes() <= 4) {
         if (instr->no_unsigned_wrap)
            bld.nuw().vadd32(Definition(dst), Operand(src0), Operand(src1));
         else
            bld.vadd32(Definition(dst), Operand(src0), Operand(src1));
         break;
      }

      assert(src0.size() == 2 && src1.size() == 2);
      Temp src00 = bld.tmp(src0.type(), 1);
      Temp src01 = bld.tmp(dst.type(), 1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src00), Definition(src01), src0);
      Temp src10 = bld.tmp(src1.type(), 1);
      Temp src11 = bld.tmp(dst.type(), 1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src10), Definition(src11), src1);

      if (dst.regClass() == s2) {
         Temp carry = bld.tmp(s1);
         Temp dst0 =
            bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.scc(Definition(carry)), src00, src10);
         Temp dst1 = bld.sop2(aco_opcode::s_addc_u32, bld.def(s1), bld.def(s1, scc), src01, src11,
                              bld.scc(carry));
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), dst0, dst1);
      } else if (dst.regClass() == v2) {
         Temp dst0 = bld.tmp(v1);
         Temp carry = bld.vadd32(Definition(dst0), src00, src10, true).def(1).getTemp();
         Temp dst1 = bld.vadd32(bld.def(v1), src01, src11, false, carry);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), dst0, dst1);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_uadd_sat: {
      if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         Instruction* add_instr = emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_add_u16, dst);
         add_instr->valu().clamp = 1;
         break;
      }
      Temp src0 = get_alu_src(ctx, instr->src[0]);
      Temp src1 = get_alu_src(ctx, instr->src[1]);
      if (dst.regClass() == s1) {
         Temp tmp = bld.tmp(s1), carry = bld.tmp(s1);
         bld.sop2(aco_opcode::s_add_u32, Definition(tmp), bld.scc(Definition(carry)), src0, src1);
         bld.sop2(aco_opcode::s_cselect_b32, Definition(dst), Operand::c32(-1), tmp,
                  bld.scc(carry));
         break;
      } else if (dst.regClass() == v2b) {
         Instruction* add_instr;
         if (ctx->program->gfx_level >= GFX10) {
            add_instr = bld.vop3(aco_opcode::v_add_u16_e64, Definition(dst), src0, src1).instr;
         } else {
            if (src1.type() == RegType::sgpr)
               std::swap(src0, src1);
            add_instr =
               bld.vop2_e64(aco_opcode::v_add_u16, Definition(dst), src0, as_vgpr(ctx, src1)).instr;
         }
         add_instr->valu().clamp = 1;
         break;
      } else if (dst.regClass() == v1) {
         uadd32_sat(bld, Definition(dst), src0, src1);
         break;
      }

      assert(src0.size() == 2 && src1.size() == 2);

      Temp src00 = bld.tmp(src0.type(), 1);
      Temp src01 = bld.tmp(src0.type(), 1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src00), Definition(src01), src0);
      Temp src10 = bld.tmp(src1.type(), 1);
      Temp src11 = bld.tmp(src1.type(), 1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src10), Definition(src11), src1);

      if (dst.regClass() == s2) {
         Temp carry0 = bld.tmp(s1);
         Temp carry1 = bld.tmp(s1);

         Temp no_sat0 =
            bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.scc(Definition(carry0)), src00, src10);
         Temp no_sat1 = bld.sop2(aco_opcode::s_addc_u32, bld.def(s1), bld.scc(Definition(carry1)),
                                 src01, src11, bld.scc(carry0));

         Temp no_sat = bld.pseudo(aco_opcode::p_create_vector, bld.def(s2), no_sat0, no_sat1);

         bld.sop2(aco_opcode::s_cselect_b64, Definition(dst), Operand::c64(-1), no_sat,
                  bld.scc(carry1));
      } else if (dst.regClass() == v2) {
         Temp no_sat0 = bld.tmp(v1);
         Temp dst0 = bld.tmp(v1);
         Temp dst1 = bld.tmp(v1);

         Temp carry0 = bld.vadd32(Definition(no_sat0), src00, src10, true).def(1).getTemp();
         Temp carry1;

         if (ctx->program->gfx_level >= GFX8) {
            carry1 = bld.tmp(bld.lm);
            bld.vop2_e64(aco_opcode::v_addc_co_u32, Definition(dst1), Definition(carry1),
                         as_vgpr(ctx, src01), as_vgpr(ctx, src11), carry0)
               ->valu()
               .clamp = 1;
         } else {
            Temp no_sat1 = bld.tmp(v1);
            carry1 = bld.vadd32(Definition(no_sat1), src01, src11, true, carry0).def(1).getTemp();
            bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(dst1), no_sat1, Operand::c32(-1),
                         carry1);
         }

         bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(dst0), no_sat0, Operand::c32(-1),
                      carry1);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), dst0, dst1);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_iadd_sat: {
      if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         Instruction* add_instr = emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_add_i16, dst);
         add_instr->valu().clamp = 1;
         break;
      }
      Temp src0 = get_alu_src(ctx, instr->src[0]);
      Temp src1 = get_alu_src(ctx, instr->src[1]);
      if (dst.regClass() == s1) {
         Temp cond = bld.sopc(aco_opcode::s_cmp_lt_i32, bld.def(s1, scc), src1, Operand::zero());
         Temp bound = bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.scc(bld.def(s1, scc)),
                               Operand::c32(INT32_MAX), cond);
         Temp overflow = bld.tmp(s1);
         Temp add =
            bld.sop2(aco_opcode::s_add_i32, bld.def(s1), bld.scc(Definition(overflow)), src0, src1);
         bld.sop2(aco_opcode::s_cselect_b32, Definition(dst), bound, add, bld.scc(overflow));
         break;
      }

      src1 = as_vgpr(ctx, src1);

      if (dst.regClass() == v2b) {
         Instruction* add_instr =
            bld.vop3(aco_opcode::v_add_i16, Definition(dst), src0, src1).instr;
         add_instr->valu().clamp = 1;
      } else if (dst.regClass() == v1) {
         Instruction* add_instr =
            bld.vop3(aco_opcode::v_add_i32, Definition(dst), src0, src1).instr;
         add_instr->valu().clamp = 1;
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_uadd_carry: {
      Temp src0 = get_alu_src(ctx, instr->src[0]);
      Temp src1 = get_alu_src(ctx, instr->src[1]);
      if (dst.regClass() == s1) {
         bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.scc(Definition(dst)), src0, src1);
         break;
      }
      if (dst.regClass() == v1) {
         Temp carry = bld.vadd32(bld.def(v1), src0, src1, true).def(1).getTemp();
         bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(dst), Operand::zero(), Operand::c32(1u),
                      carry);
         break;
      }

      Temp src00 = bld.tmp(src0.type(), 1);
      Temp src01 = bld.tmp(dst.type(), 1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src00), Definition(src01), src0);
      Temp src10 = bld.tmp(src1.type(), 1);
      Temp src11 = bld.tmp(dst.type(), 1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src10), Definition(src11), src1);
      if (dst.regClass() == s2) {
         Temp carry = bld.tmp(s1);
         bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.scc(Definition(carry)), src00, src10);
         carry = bld.sop2(aco_opcode::s_addc_u32, bld.def(s1), bld.scc(bld.def(s1)), src01, src11,
                          bld.scc(carry))
                    .def(1)
                    .getTemp();
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), carry, Operand::zero());
      } else if (dst.regClass() == v2) {
         Temp carry = bld.vadd32(bld.def(v1), src00, src10, true).def(1).getTemp();
         carry = bld.vadd32(bld.def(v1), src01, src11, true, carry).def(1).getTemp();
         carry = bld.vop2_e64(aco_opcode::v_cndmask_b32, bld.def(v1), Operand::zero(),
                              Operand::c32(1u), carry);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), carry, Operand::zero());
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_isub: {
      if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_sub_i32, dst, true);
         break;
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_sub_u16, dst);
         break;
      }

      Temp src0 = get_alu_src(ctx, instr->src[0]);
      Temp src1 = get_alu_src(ctx, instr->src[1]);
      if (dst.regClass() == v1) {
         bld.vsub32(Definition(dst), src0, src1);
         break;
      } else if (dst.bytes() <= 2) {
         if (ctx->program->gfx_level >= GFX10)
            bld.vop3(aco_opcode::v_sub_u16_e64, Definition(dst), src0, src1);
         else if (src1.type() == RegType::sgpr)
            bld.vop2(aco_opcode::v_subrev_u16, Definition(dst), src1, as_vgpr(ctx, src0));
         else if (ctx->program->gfx_level >= GFX8)
            bld.vop2(aco_opcode::v_sub_u16, Definition(dst), src0, as_vgpr(ctx, src1));
         else
            bld.vsub32(Definition(dst), src0, src1);
         break;
      }

      Temp src00 = bld.tmp(src0.type(), 1);
      Temp src01 = bld.tmp(dst.type(), 1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src00), Definition(src01), src0);
      Temp src10 = bld.tmp(src1.type(), 1);
      Temp src11 = bld.tmp(dst.type(), 1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src10), Definition(src11), src1);
      if (dst.regClass() == s2) {
         Temp borrow = bld.tmp(s1);
         Temp dst0 =
            bld.sop2(aco_opcode::s_sub_u32, bld.def(s1), bld.scc(Definition(borrow)), src00, src10);
         Temp dst1 = bld.sop2(aco_opcode::s_subb_u32, bld.def(s1), bld.def(s1, scc), src01, src11,
                              bld.scc(borrow));
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), dst0, dst1);
      } else if (dst.regClass() == v2) {
         Temp lower = bld.tmp(v1);
         Temp borrow = bld.vsub32(Definition(lower), src00, src10, true).def(1).getTemp();
         Temp upper = bld.vsub32(bld.def(v1), src01, src11, false, borrow);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), lower, upper);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_usub_borrow: {
      Temp src0 = get_alu_src(ctx, instr->src[0]);
      Temp src1 = get_alu_src(ctx, instr->src[1]);
      if (dst.regClass() == s1) {
         bld.sop2(aco_opcode::s_sub_u32, bld.def(s1), bld.scc(Definition(dst)), src0, src1);
         break;
      } else if (dst.regClass() == v1) {
         Temp borrow = bld.vsub32(bld.def(v1), src0, src1, true).def(1).getTemp();
         bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(dst), Operand::zero(), Operand::c32(1u),
                      borrow);
         break;
      }

      Temp src00 = bld.tmp(src0.type(), 1);
      Temp src01 = bld.tmp(dst.type(), 1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src00), Definition(src01), src0);
      Temp src10 = bld.tmp(src1.type(), 1);
      Temp src11 = bld.tmp(dst.type(), 1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src10), Definition(src11), src1);
      if (dst.regClass() == s2) {
         Temp borrow = bld.tmp(s1);
         bld.sop2(aco_opcode::s_sub_u32, bld.def(s1), bld.scc(Definition(borrow)), src00, src10);
         borrow = bld.sop2(aco_opcode::s_subb_u32, bld.def(s1), bld.scc(bld.def(s1)), src01, src11,
                           bld.scc(borrow))
                     .def(1)
                     .getTemp();
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), borrow, Operand::zero());
      } else if (dst.regClass() == v2) {
         Temp borrow = bld.vsub32(bld.def(v1), src00, src10, true).def(1).getTemp();
         borrow = bld.vsub32(bld.def(v1), src01, src11, true, Operand(borrow)).def(1).getTemp();
         borrow = bld.vop2_e64(aco_opcode::v_cndmask_b32, bld.def(v1), Operand::zero(),
                               Operand::c32(1u), borrow);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), borrow, Operand::zero());
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_usub_sat: {
      if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         Instruction* sub_instr = emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_sub_u16, dst);
         sub_instr->valu().clamp = 1;
         break;
      }
      Temp src0 = get_alu_src(ctx, instr->src[0]);
      Temp src1 = get_alu_src(ctx, instr->src[1]);
      if (dst.regClass() == s1) {
         Temp tmp = bld.tmp(s1), carry = bld.tmp(s1);
         bld.sop2(aco_opcode::s_sub_u32, Definition(tmp), bld.scc(Definition(carry)), src0, src1);
         bld.sop2(aco_opcode::s_cselect_b32, Definition(dst), Operand::c32(0), tmp, bld.scc(carry));
         break;
      } else if (dst.regClass() == v2b) {
         Instruction* sub_instr;
         if (ctx->program->gfx_level >= GFX10) {
            sub_instr = bld.vop3(aco_opcode::v_sub_u16_e64, Definition(dst), src0, src1).instr;
         } else {
            aco_opcode op = aco_opcode::v_sub_u16;
            if (src1.type() == RegType::sgpr) {
               std::swap(src0, src1);
               op = aco_opcode::v_subrev_u16;
            }
            sub_instr = bld.vop2_e64(op, Definition(dst), src0, as_vgpr(ctx, src1)).instr;
         }
         sub_instr->valu().clamp = 1;
         break;
      } else if (dst.regClass() == v1) {
         usub32_sat(bld, Definition(dst), src0, as_vgpr(ctx, src1));
         break;
      }

      assert(src0.size() == 2 && src1.size() == 2);
      Temp src00 = bld.tmp(src0.type(), 1);
      Temp src01 = bld.tmp(src0.type(), 1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src00), Definition(src01), src0);
      Temp src10 = bld.tmp(src1.type(), 1);
      Temp src11 = bld.tmp(src1.type(), 1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src10), Definition(src11), src1);

      if (dst.regClass() == s2) {
         Temp carry0 = bld.tmp(s1);
         Temp carry1 = bld.tmp(s1);

         Temp no_sat0 =
            bld.sop2(aco_opcode::s_sub_u32, bld.def(s1), bld.scc(Definition(carry0)), src00, src10);
         Temp no_sat1 = bld.sop2(aco_opcode::s_subb_u32, bld.def(s1), bld.scc(Definition(carry1)),
                                 src01, src11, bld.scc(carry0));

         Temp no_sat = bld.pseudo(aco_opcode::p_create_vector, bld.def(s2), no_sat0, no_sat1);

         bld.sop2(aco_opcode::s_cselect_b64, Definition(dst), Operand::c64(0ull), no_sat,
                  bld.scc(carry1));
      } else if (dst.regClass() == v2) {
         Temp no_sat0 = bld.tmp(v1);
         Temp dst0 = bld.tmp(v1);
         Temp dst1 = bld.tmp(v1);

         Temp carry0 = bld.vsub32(Definition(no_sat0), src00, src10, true).def(1).getTemp();
         Temp carry1;

         if (ctx->program->gfx_level >= GFX8) {
            carry1 = bld.tmp(bld.lm);
            bld.vop2_e64(aco_opcode::v_subb_co_u32, Definition(dst1), Definition(carry1),
                         as_vgpr(ctx, src01), as_vgpr(ctx, src11), carry0)
               ->valu()
               .clamp = 1;
         } else {
            Temp no_sat1 = bld.tmp(v1);
            carry1 = bld.vsub32(Definition(no_sat1), src01, src11, true, carry0).def(1).getTemp();
            bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(dst1), no_sat1, Operand::c32(0u),
                         carry1);
         }

         bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(dst0), no_sat0, Operand::c32(0u),
                      carry1);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), dst0, dst1);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_isub_sat: {
      if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         Instruction* sub_instr = emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_sub_i16, dst);
         sub_instr->valu().clamp = 1;
         break;
      }
      Temp src0 = get_alu_src(ctx, instr->src[0]);
      Temp src1 = get_alu_src(ctx, instr->src[1]);
      if (dst.regClass() == s1) {
         Temp cond = bld.sopc(aco_opcode::s_cmp_gt_i32, bld.def(s1, scc), src1, Operand::zero());
         Temp bound = bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.scc(bld.def(s1, scc)),
                               Operand::c32(INT32_MAX), cond);
         Temp overflow = bld.tmp(s1);
         Temp sub =
            bld.sop2(aco_opcode::s_sub_i32, bld.def(s1), bld.scc(Definition(overflow)), src0, src1);
         bld.sop2(aco_opcode::s_cselect_b32, Definition(dst), bound, sub, bld.scc(overflow));
         break;
      }

      src1 = as_vgpr(ctx, src1);

      if (dst.regClass() == v2b) {
         Instruction* sub_instr =
            bld.vop3(aco_opcode::v_sub_i16, Definition(dst), src0, src1).instr;
         sub_instr->valu().clamp = 1;
      } else if (dst.regClass() == v1) {
         Instruction* sub_instr =
            bld.vop3(aco_opcode::v_sub_i32, Definition(dst), src0, src1).instr;
         sub_instr->valu().clamp = 1;
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_imul: {
      if (dst.bytes() <= 2 && ctx->program->gfx_level >= GFX10) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_mul_lo_u16_e64, dst);
      } else if (dst.bytes() <= 2 && ctx->program->gfx_level >= GFX8) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_mul_lo_u16, dst, true);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_mul_lo_u16, dst);
      } else if (dst.type() == RegType::vgpr) {
         uint32_t src0_ub = get_alu_src_ub(ctx, instr, 0);
         uint32_t src1_ub = get_alu_src_ub(ctx, instr, 1);

         if (src0_ub <= 0xffffff && src1_ub <= 0xffffff) {
            bool nuw_16bit = src0_ub <= 0xffff && src1_ub <= 0xffff && src0_ub * src1_ub <= 0xffff;
            emit_vop2_instruction(ctx, instr, aco_opcode::v_mul_u32_u24, dst,
                                  true /* commutative */, false, false, nuw_16bit);
         } else if (nir_src_is_const(instr->src[0].src)) {
            bld.v_mul_imm(Definition(dst), get_alu_src(ctx, instr->src[1]),
                          nir_src_as_uint(instr->src[0].src), false);
         } else if (nir_src_is_const(instr->src[1].src)) {
            bld.v_mul_imm(Definition(dst), get_alu_src(ctx, instr->src[0]),
                          nir_src_as_uint(instr->src[1].src), false);
         } else {
            emit_vop3a_instruction(ctx, instr, aco_opcode::v_mul_lo_u32, dst);
         }
      } else if (dst.regClass() == s1) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_mul_i32, dst, false);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_umul_high: {
      if (dst.regClass() == s1 && ctx->options->gfx_level >= GFX9) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_mul_hi_u32, dst, false);
      } else if (dst.bytes() == 4) {
         uint32_t src0_ub = get_alu_src_ub(ctx, instr, 0);
         uint32_t src1_ub = get_alu_src_ub(ctx, instr, 1);

         Temp tmp = dst.regClass() == s1 ? bld.tmp(v1) : dst;
         if (src0_ub <= 0xffffff && src1_ub <= 0xffffff) {
            emit_vop2_instruction(ctx, instr, aco_opcode::v_mul_hi_u32_u24, tmp, true);
         } else {
            emit_vop3a_instruction(ctx, instr, aco_opcode::v_mul_hi_u32, tmp);
         }

         if (dst.regClass() == s1)
            bld.pseudo(aco_opcode::p_as_uniform, Definition(dst), tmp);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_imul_high: {
      if (dst.regClass() == v1) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_mul_hi_i32, dst);
      } else if (dst.regClass() == s1 && ctx->options->gfx_level >= GFX9) {
         emit_sop2_instruction(ctx, instr, aco_opcode::s_mul_hi_i32, dst, false);
      } else if (dst.regClass() == s1) {
         Temp tmp = bld.vop3(aco_opcode::v_mul_hi_i32, bld.def(v1), get_alu_src(ctx, instr->src[0]),
                             as_vgpr(ctx, get_alu_src(ctx, instr->src[1])));
         bld.pseudo(aco_opcode::p_as_uniform, Definition(dst), tmp);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fmul: {
      if (dst.regClass() == v2b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_mul_f16, dst, true);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_mul_f16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_mul_f32, dst, true);
      } else if (dst.regClass() == v2) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_mul_f64, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fmulz: {
      if (dst.regClass() == v1) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_mul_legacy_f32, dst, true);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fadd: {
      if (dst.regClass() == v2b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_add_f16, dst, true);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_add_f16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_add_f32, dst, true);
      } else if (dst.regClass() == v2) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_add_f64, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fsub: {
      if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         Instruction* add = emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_add_f16, dst);
         VALU_instruction& sub = add->valu();
         sub.neg_lo[1] = true;
         sub.neg_hi[1] = true;
         break;
      }

      Temp src0 = get_alu_src(ctx, instr->src[0]);
      Temp src1 = get_alu_src(ctx, instr->src[1]);
      if (dst.regClass() == v2b) {
         if (src1.type() == RegType::vgpr || src0.type() != RegType::vgpr)
            emit_vop2_instruction(ctx, instr, aco_opcode::v_sub_f16, dst, false);
         else
            emit_vop2_instruction(ctx, instr, aco_opcode::v_subrev_f16, dst, true);
      } else if (dst.regClass() == v1) {
         if (src1.type() == RegType::vgpr || src0.type() != RegType::vgpr)
            emit_vop2_instruction(ctx, instr, aco_opcode::v_sub_f32, dst, false);
         else
            emit_vop2_instruction(ctx, instr, aco_opcode::v_subrev_f32, dst, true);
      } else if (dst.regClass() == v2) {
         Instruction* add = bld.vop3(aco_opcode::v_add_f64, Definition(dst), as_vgpr(ctx, src0),
                                     as_vgpr(ctx, src1));
         add->valu().neg[1] = true;
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ffma: {
      if (dst.regClass() == v2b) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_fma_f16, dst, false, 3);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         assert(instr->def.num_components == 2);

         Temp src0 = as_vgpr(ctx, get_alu_src_vop3p(ctx, instr->src[0]));
         Temp src1 = as_vgpr(ctx, get_alu_src_vop3p(ctx, instr->src[1]));
         Temp src2 = as_vgpr(ctx, get_alu_src_vop3p(ctx, instr->src[2]));

         /* swizzle to opsel: all swizzles are either 0 (x) or 1 (y) */
         unsigned opsel_lo = 0, opsel_hi = 0;
         for (unsigned i = 0; i < 3; i++) {
            opsel_lo |= (instr->src[i].swizzle[0] & 1) << i;
            opsel_hi |= (instr->src[i].swizzle[1] & 1) << i;
         }

         bld.vop3p(aco_opcode::v_pk_fma_f16, Definition(dst), src0, src1, src2, opsel_lo, opsel_hi);
      } else if (dst.regClass() == v1) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_fma_f32, dst,
                                ctx->block->fp_mode.must_flush_denorms32, 3);
      } else if (dst.regClass() == v2) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_fma_f64, dst, false, 3);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ffmaz: {
      if (dst.regClass() == v1) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_fma_legacy_f32, dst,
                                ctx->block->fp_mode.must_flush_denorms32, 3);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fmax: {
      if (dst.regClass() == v2b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_max_f16, dst, true, false,
                               ctx->block->fp_mode.must_flush_denorms16_64);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_max_f16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_max_f32, dst, true, false,
                               ctx->block->fp_mode.must_flush_denorms32);
      } else if (dst.regClass() == v2) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_max_f64, dst,
                                ctx->block->fp_mode.must_flush_denorms16_64);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fmin: {
      if (dst.regClass() == v2b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_min_f16, dst, true, false,
                               ctx->block->fp_mode.must_flush_denorms16_64);
      } else if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         emit_vop3p_instruction(ctx, instr, aco_opcode::v_pk_min_f16, dst, true);
      } else if (dst.regClass() == v1) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_min_f32, dst, true, false,
                               ctx->block->fp_mode.must_flush_denorms32);
      } else if (dst.regClass() == v2) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_min_f64, dst,
                                ctx->block->fp_mode.must_flush_denorms16_64);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_sdot_4x8_iadd: {
      if (ctx->options->gfx_level >= GFX11)
         emit_idot_instruction(ctx, instr, aco_opcode::v_dot4_i32_iu8, dst, false, 0x3);
      else
         emit_idot_instruction(ctx, instr, aco_opcode::v_dot4_i32_i8, dst, false);
      break;
   }
   case nir_op_sdot_4x8_iadd_sat: {
      if (ctx->options->gfx_level >= GFX11)
         emit_idot_instruction(ctx, instr, aco_opcode::v_dot4_i32_iu8, dst, true, 0x3);
      else
         emit_idot_instruction(ctx, instr, aco_opcode::v_dot4_i32_i8, dst, true);
      break;
   }
   case nir_op_sudot_4x8_iadd: {
      emit_idot_instruction(ctx, instr, aco_opcode::v_dot4_i32_iu8, dst, false, 0x1);
      break;
   }
   case nir_op_sudot_4x8_iadd_sat: {
      emit_idot_instruction(ctx, instr, aco_opcode::v_dot4_i32_iu8, dst, true, 0x1);
      break;
   }
   case nir_op_udot_4x8_uadd: {
      emit_idot_instruction(ctx, instr, aco_opcode::v_dot4_u32_u8, dst, false);
      break;
   }
   case nir_op_udot_4x8_uadd_sat: {
      emit_idot_instruction(ctx, instr, aco_opcode::v_dot4_u32_u8, dst, true);
      break;
   }
   case nir_op_sdot_2x16_iadd: {
      emit_idot_instruction(ctx, instr, aco_opcode::v_dot2_i32_i16, dst, false);
      break;
   }
   case nir_op_sdot_2x16_iadd_sat: {
      emit_idot_instruction(ctx, instr, aco_opcode::v_dot2_i32_i16, dst, true);
      break;
   }
   case nir_op_udot_2x16_uadd: {
      emit_idot_instruction(ctx, instr, aco_opcode::v_dot2_u32_u16, dst, false);
      break;
   }
   case nir_op_udot_2x16_uadd_sat: {
      emit_idot_instruction(ctx, instr, aco_opcode::v_dot2_u32_u16, dst, true);
      break;
   }
   case nir_op_cube_amd: {
      Temp in = get_alu_src(ctx, instr->src[0], 3);
      Temp src[3] = {emit_extract_vector(ctx, in, 0, v1), emit_extract_vector(ctx, in, 1, v1),
                     emit_extract_vector(ctx, in, 2, v1)};
      Temp ma = bld.vop3(aco_opcode::v_cubema_f32, bld.def(v1), src[0], src[1], src[2]);
      Temp sc = bld.vop3(aco_opcode::v_cubesc_f32, bld.def(v1), src[0], src[1], src[2]);
      Temp tc = bld.vop3(aco_opcode::v_cubetc_f32, bld.def(v1), src[0], src[1], src[2]);
      Temp id = bld.vop3(aco_opcode::v_cubeid_f32, bld.def(v1), src[0], src[1], src[2]);
      bld.pseudo(aco_opcode::p_create_vector, Definition(dst), tc, sc, ma, id);
      break;
   }
   case nir_op_bcsel: {
      emit_bcsel(ctx, instr, dst);
      break;
   }
   case nir_op_frsq: {
      if (dst.regClass() == v2b) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_rsq_f16, dst);
      } else if (dst.regClass() == v1) {
         Temp src = get_alu_src(ctx, instr->src[0]);
         emit_rsq(ctx, bld, Definition(dst), src);
      } else if (dst.regClass() == v2) {
         /* Lowered at NIR level for precision reasons. */
         emit_vop1_instruction(ctx, instr, aco_opcode::v_rsq_f64, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fneg: {
      if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         Temp src = get_alu_src_vop3p(ctx, instr->src[0]);
         Instruction* vop3p =
            bld.vop3p(aco_opcode::v_pk_mul_f16, Definition(dst), src, Operand::c16(0x3C00),
                      instr->src[0].swizzle[0] & 1, instr->src[0].swizzle[1] & 1);
         vop3p->valu().neg_lo[0] = true;
         vop3p->valu().neg_hi[0] = true;
         break;
      }
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (dst.regClass() == v2b) {
         bld.vop2(aco_opcode::v_mul_f16, Definition(dst), Operand::c16(0xbc00u), as_vgpr(ctx, src));
      } else if (dst.regClass() == v1) {
         bld.vop2(aco_opcode::v_mul_f32, Definition(dst), Operand::c32(0xbf800000u),
                  as_vgpr(ctx, src));
      } else if (dst.regClass() == v2) {
         if (ctx->block->fp_mode.must_flush_denorms16_64)
            src = bld.vop3(aco_opcode::v_mul_f64, bld.def(v2), Operand::c64(0x3FF0000000000000),
                           as_vgpr(ctx, src));
         Temp upper = bld.tmp(v1), lower = bld.tmp(v1);
         bld.pseudo(aco_opcode::p_split_vector, Definition(lower), Definition(upper), src);
         upper = bld.vop2(aco_opcode::v_xor_b32, bld.def(v1), Operand::c32(0x80000000u), upper);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), lower, upper);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fabs: {
      if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         Temp src = get_alu_src_vop3p(ctx, instr->src[0]);
         Instruction* vop3p =
            bld.vop3p(aco_opcode::v_pk_max_f16, Definition(dst), src, src,
                      instr->src[0].swizzle[0] & 1 ? 3 : 0, instr->src[0].swizzle[1] & 1 ? 3 : 0)
               .instr;
         vop3p->valu().neg_lo[1] = true;
         vop3p->valu().neg_hi[1] = true;
         break;
      }
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (dst.regClass() == v2b) {
         Instruction* mul = bld.vop2_e64(aco_opcode::v_mul_f16, Definition(dst),
                                         Operand::c16(0x3c00), as_vgpr(ctx, src))
                               .instr;
         mul->valu().abs[1] = true;
      } else if (dst.regClass() == v1) {
         Instruction* mul = bld.vop2_e64(aco_opcode::v_mul_f32, Definition(dst),
                                         Operand::c32(0x3f800000u), as_vgpr(ctx, src))
                               .instr;
         mul->valu().abs[1] = true;
      } else if (dst.regClass() == v2) {
         if (ctx->block->fp_mode.must_flush_denorms16_64)
            src = bld.vop3(aco_opcode::v_mul_f64, bld.def(v2), Operand::c64(0x3FF0000000000000),
                           as_vgpr(ctx, src));
         Temp upper = bld.tmp(v1), lower = bld.tmp(v1);
         bld.pseudo(aco_opcode::p_split_vector, Definition(lower), Definition(upper), src);
         upper = bld.vop2(aco_opcode::v_and_b32, bld.def(v1), Operand::c32(0x7FFFFFFFu), upper);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), lower, upper);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fsat: {
      if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         Temp src = get_alu_src_vop3p(ctx, instr->src[0]);
         Instruction* vop3p =
            bld.vop3p(aco_opcode::v_pk_mul_f16, Definition(dst), src, Operand::c16(0x3C00),
                      instr->src[0].swizzle[0] & 1, instr->src[0].swizzle[1] & 1);
         vop3p->valu().clamp = true;
         break;
      }
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (dst.regClass() == v2b && ctx->program->gfx_level >= GFX9) {
         bld.vop3(aco_opcode::v_med3_f16, Definition(dst), Operand::c16(0u), Operand::c16(0x3c00),
                  src);
      } else if (dst.regClass() == v2b) {
         bld.vop2_e64(aco_opcode::v_mul_f16, Definition(dst), Operand::c16(0x3c00), src)
            ->valu()
            .clamp = true;
      } else if (dst.regClass() == v1) {
         bld.vop3(aco_opcode::v_med3_f32, Definition(dst), Operand::zero(),
                  Operand::c32(0x3f800000u), src);
         /* apparently, it is not necessary to flush denorms if this instruction is used with these
          * operands */
         // TODO: confirm that this holds under any circumstances
      } else if (dst.regClass() == v2) {
         Instruction* add = bld.vop3(aco_opcode::v_add_f64, Definition(dst), src, Operand::zero());
         add->valu().clamp = true;
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_flog2: {
      if (dst.regClass() == v2b) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_log_f16, dst);
      } else if (dst.regClass() == v1) {
         Temp src = get_alu_src(ctx, instr->src[0]);
         emit_log2(ctx, bld, Definition(dst), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_frcp: {
      if (dst.regClass() == v2b) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_rcp_f16, dst);
      } else if (dst.regClass() == v1) {
         Temp src = get_alu_src(ctx, instr->src[0]);
         emit_rcp(ctx, bld, Definition(dst), src);
      } else if (dst.regClass() == v2) {
         /* Lowered at NIR level for precision reasons. */
         emit_vop1_instruction(ctx, instr, aco_opcode::v_rcp_f64, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fexp2: {
      if (dst.regClass() == v2b) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_exp_f16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_exp_f32, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fsqrt: {
      if (dst.regClass() == v2b) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_sqrt_f16, dst);
      } else if (dst.regClass() == v1) {
         Temp src = get_alu_src(ctx, instr->src[0]);
         emit_sqrt(ctx, bld, Definition(dst), src);
      } else if (dst.regClass() == v2) {
         /* Lowered at NIR level for precision reasons. */
         emit_vop1_instruction(ctx, instr, aco_opcode::v_sqrt_f64, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ffract: {
      if (dst.regClass() == v2b) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_fract_f16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_fract_f32, dst);
      } else if (dst.regClass() == v2) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_fract_f64, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ffloor: {
      if (dst.regClass() == v2b) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_floor_f16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_floor_f32, dst);
      } else if (dst.regClass() == v2) {
         Temp src = get_alu_src(ctx, instr->src[0]);
         emit_floor_f64(ctx, bld, Definition(dst), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fceil: {
      if (dst.regClass() == v2b) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_ceil_f16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_ceil_f32, dst);
      } else if (dst.regClass() == v2) {
         if (ctx->options->gfx_level >= GFX7) {
            emit_vop1_instruction(ctx, instr, aco_opcode::v_ceil_f64, dst);
         } else {
            /* GFX6 doesn't support V_CEIL_F64, lower it. */
            /* trunc = trunc(src0)
             * if (src0 > 0.0 && src0 != trunc)
             *    trunc += 1.0
             */
            Temp src0 = get_alu_src(ctx, instr->src[0]);
            Temp trunc = emit_trunc_f64(ctx, bld, bld.def(v2), src0);
            Temp tmp0 =
               bld.vopc_e64(aco_opcode::v_cmp_gt_f64, bld.def(bld.lm), src0, Operand::zero());
            Temp tmp1 = bld.vopc(aco_opcode::v_cmp_lg_f64, bld.def(bld.lm), src0, trunc);
            Temp cond = bld.sop2(aco_opcode::s_and_b64, bld.def(s2), bld.def(s1, scc), tmp0, tmp1);
            Temp add = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1),
                                bld.copy(bld.def(v1), Operand::zero()),
                                bld.copy(bld.def(v1), Operand::c32(0x3ff00000u)), cond);
            add = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2),
                             bld.copy(bld.def(v1), Operand::zero()), add);
            bld.vop3(aco_opcode::v_add_f64, Definition(dst), trunc, add);
         }
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ftrunc: {
      if (dst.regClass() == v2b) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_trunc_f16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_trunc_f32, dst);
      } else if (dst.regClass() == v2) {
         Temp src = get_alu_src(ctx, instr->src[0]);
         emit_trunc_f64(ctx, bld, Definition(dst), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fround_even: {
      if (dst.regClass() == v2b) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_rndne_f16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_rndne_f32, dst);
      } else if (dst.regClass() == v2) {
         if (ctx->options->gfx_level >= GFX7) {
            emit_vop1_instruction(ctx, instr, aco_opcode::v_rndne_f64, dst);
         } else {
            /* GFX6 doesn't support V_RNDNE_F64, lower it. */
            Temp src0_lo = bld.tmp(v1), src0_hi = bld.tmp(v1);
            Temp src0 = get_alu_src(ctx, instr->src[0]);
            bld.pseudo(aco_opcode::p_split_vector, Definition(src0_lo), Definition(src0_hi), src0);

            Temp bitmask = bld.sop1(aco_opcode::s_brev_b32, bld.def(s1),
                                    bld.copy(bld.def(s1), Operand::c32(-2u)));
            Temp bfi =
               bld.vop3(aco_opcode::v_bfi_b32, bld.def(v1), bitmask,
                        bld.copy(bld.def(v1), Operand::c32(0x43300000u)), as_vgpr(ctx, src0_hi));
            Temp tmp =
               bld.vop3(aco_opcode::v_add_f64, bld.def(v2), src0,
                        bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), Operand::zero(), bfi));
            Instruction* sub =
               bld.vop3(aco_opcode::v_add_f64, bld.def(v2), tmp,
                        bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), Operand::zero(), bfi));
            sub->valu().neg[1] = true;
            tmp = sub->definitions[0].getTemp();

            Temp v = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), Operand::c32(-1u),
                                Operand::c32(0x432fffffu));
            Instruction* vop3 = bld.vopc_e64(aco_opcode::v_cmp_gt_f64, bld.def(bld.lm), src0, v);
            vop3->valu().abs[0] = true;
            Temp cond = vop3->definitions[0].getTemp();

            Temp tmp_lo = bld.tmp(v1), tmp_hi = bld.tmp(v1);
            bld.pseudo(aco_opcode::p_split_vector, Definition(tmp_lo), Definition(tmp_hi), tmp);
            Temp dst0 = bld.vop2_e64(aco_opcode::v_cndmask_b32, bld.def(v1), tmp_lo,
                                     as_vgpr(ctx, src0_lo), cond);
            Temp dst1 = bld.vop2_e64(aco_opcode::v_cndmask_b32, bld.def(v1), tmp_hi,
                                     as_vgpr(ctx, src0_hi), cond);

            bld.pseudo(aco_opcode::p_create_vector, Definition(dst), dst0, dst1);
         }
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fsin_amd:
   case nir_op_fcos_amd: {
      Temp src = as_vgpr(ctx, get_alu_src(ctx, instr->src[0]));
      aco_ptr<Instruction> norm;
      if (dst.regClass() == v2b) {
         aco_opcode opcode =
            instr->op == nir_op_fsin_amd ? aco_opcode::v_sin_f16 : aco_opcode::v_cos_f16;
         bld.vop1(opcode, Definition(dst), src);
      } else if (dst.regClass() == v1) {
         /* before GFX9, v_sin_f32 and v_cos_f32 had a valid input domain of [-256, +256] */
         if (ctx->options->gfx_level < GFX9)
            src = bld.vop1(aco_opcode::v_fract_f32, bld.def(v1), src);

         aco_opcode opcode =
            instr->op == nir_op_fsin_amd ? aco_opcode::v_sin_f32 : aco_opcode::v_cos_f32;
         bld.vop1(opcode, Definition(dst), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ldexp: {
      if (dst.regClass() == v2b) {
         emit_vop2_instruction(ctx, instr, aco_opcode::v_ldexp_f16, dst, false);
      } else if (dst.regClass() == v1) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_ldexp_f32, dst);
      } else if (dst.regClass() == v2) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_ldexp_f64, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_frexp_sig: {
      if (dst.regClass() == v2b) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_frexp_mant_f16, dst);
      } else if (dst.regClass() == v1) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_frexp_mant_f32, dst);
      } else if (dst.regClass() == v2) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_frexp_mant_f64, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_frexp_exp: {
      if (instr->src[0].src.ssa->bit_size == 16) {
         Temp src = get_alu_src(ctx, instr->src[0]);
         Temp tmp = bld.vop1(aco_opcode::v_frexp_exp_i16_f16, bld.def(v1), src);
         tmp = bld.pseudo(aco_opcode::p_extract_vector, bld.def(v1b), tmp, Operand::zero());
         convert_int(ctx, bld, tmp, 8, 32, true, dst);
      } else if (instr->src[0].src.ssa->bit_size == 32) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_frexp_exp_i32_f32, dst);
      } else if (instr->src[0].src.ssa->bit_size == 64) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_frexp_exp_i32_f64, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_fsign: {
      Temp src = as_vgpr(ctx, get_alu_src(ctx, instr->src[0]));
      if (dst.regClass() == v2b) {
         /* replace negative zero with positive zero */
         src = bld.vop2(aco_opcode::v_add_f16, bld.def(v2b), Operand::zero(), src);
         if (ctx->program->gfx_level >= GFX9) {
            src = bld.vop3(aco_opcode::v_med3_i16, bld.def(v2b), Operand::c16(-1), src,
                           Operand::c16(1u));
            bld.vop1(aco_opcode::v_cvt_f16_i16, Definition(dst), src);
         } else {
            src = convert_int(ctx, bld, src, 16, 32, true);
            src = bld.vop3(aco_opcode::v_med3_i32, bld.def(v1), Operand::c32(-1), src,
                           Operand::c32(1u));
            bld.vop1(aco_opcode::v_cvt_f16_i16, Definition(dst), src);
         }
      } else if (dst.regClass() == v1) {
         /* Legacy multiply with +Inf means +-0.0 becomes +0.0 and all other numbers
          * the correctly signed Inf. After that, we only need to clamp between -1.0 and +1.0.
          */
         Temp inf = bld.copy(bld.def(s1), Operand::c32(0x7f800000));
         src = bld.vop2(aco_opcode::v_mul_legacy_f32, bld.def(v1), inf, src);
         bld.vop3(aco_opcode::v_med3_f32, Definition(dst), Operand::c32(0x3f800000), src,
                  Operand::c32(0xbf800000));
      } else if (dst.regClass() == v2) {
         Temp cond = bld.vopc(aco_opcode::v_cmp_nlt_f64, bld.def(bld.lm), Operand::zero(), src);
         Temp tmp = bld.copy(bld.def(v1), Operand::c32(0x3FF00000u));
         Temp upper = bld.vop2_e64(aco_opcode::v_cndmask_b32, bld.def(v1), tmp,
                                   emit_extract_vector(ctx, src, 1, v1), cond);

         cond = bld.vopc(aco_opcode::v_cmp_le_f64, bld.def(bld.lm), Operand::zero(), src);
         tmp = bld.copy(bld.def(v1), Operand::c32(0xBFF00000u));
         upper = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), tmp, upper, cond);

         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), Operand::zero(), upper);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_f2f16:
   case nir_op_f2f16_rtne: {
      assert(instr->src[0].src.ssa->bit_size == 32);
      if (instr->def.num_components == 2) {
         /* Vectorizing f2f16 is only possible with rtz. */
         assert(instr->op != nir_op_f2f16_rtne);
         assert(ctx->block->fp_mode.round16_64 == fp_round_tz ||
                !ctx->block->fp_mode.care_about_round16_64);
         emit_vec2_f2f16(ctx, instr, dst);
         break;
      }
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (instr->op == nir_op_f2f16_rtne && ctx->block->fp_mode.round16_64 != fp_round_ne)
         /* We emit s_round_mode/s_setreg_imm32 in lower_to_hw_instr to
          * keep value numbering and the scheduler simpler.
          */
         bld.vop1(aco_opcode::p_cvt_f16_f32_rtne, Definition(dst), src);
      else
         bld.vop1(aco_opcode::v_cvt_f16_f32, Definition(dst), src);
      break;
   }
   case nir_op_f2f16_rtz: {
      assert(instr->src[0].src.ssa->bit_size == 32);
      if (instr->def.num_components == 2) {
         emit_vec2_f2f16(ctx, instr, dst);
         break;
      }
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (ctx->block->fp_mode.round16_64 == fp_round_tz)
         bld.vop1(aco_opcode::v_cvt_f16_f32, Definition(dst), src);
      else if (ctx->program->gfx_level == GFX8 || ctx->program->gfx_level == GFX9)
         bld.vop3(aco_opcode::v_cvt_pkrtz_f16_f32_e64, Definition(dst), src, Operand::zero());
      else
         bld.vop2(aco_opcode::v_cvt_pkrtz_f16_f32, Definition(dst), src, as_vgpr(ctx, src));
      break;
   }
   case nir_op_f2f32: {
      if (instr->src[0].src.ssa->bit_size == 16) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_f32_f16, dst);
      } else if (instr->src[0].src.ssa->bit_size == 64) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_f32_f64, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_f2f64: {
      assert(instr->src[0].src.ssa->bit_size == 32);
      Temp src = get_alu_src(ctx, instr->src[0]);
      bld.vop1(aco_opcode::v_cvt_f64_f32, Definition(dst), src);
      break;
   }
   case nir_op_i2f16: {
      assert(dst.regClass() == v2b);
      Temp src = get_alu_src(ctx, instr->src[0]);
      const unsigned input_size = instr->src[0].src.ssa->bit_size;
      if (input_size <= 16) {
         /* Expand integer to the size expected by the uint→float converter used below */
         unsigned target_size = (ctx->program->gfx_level >= GFX8 ? 16 : 32);
         if (input_size != target_size) {
            src = convert_int(ctx, bld, src, input_size, target_size, true);
         }
      }

      if (ctx->program->gfx_level >= GFX8 && input_size <= 16) {
         bld.vop1(aco_opcode::v_cvt_f16_i16, Definition(dst), src);
      } else {
         /* Large 32bit inputs need to return +-inf/FLOAT_MAX.
          *
          * This is also the fallback-path taken on GFX7 and earlier, which
          * do not support direct f16⟷i16 conversions.
          */
         src = bld.vop1(aco_opcode::v_cvt_f32_i32, bld.def(v1), src);
         bld.vop1(aco_opcode::v_cvt_f16_f32, Definition(dst), src);
      }
      break;
   }
   case nir_op_i2f32: {
      assert(dst.size() == 1);
      Temp src = get_alu_src(ctx, instr->src[0]);
      const unsigned input_size = instr->src[0].src.ssa->bit_size;
      if (input_size <= 32) {
         if (input_size <= 16) {
            /* Sign-extend to 32-bits */
            src = convert_int(ctx, bld, src, input_size, 32, true);
         }
         bld.vop1(aco_opcode::v_cvt_f32_i32, Definition(dst), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_i2f64: {
      if (instr->src[0].src.ssa->bit_size <= 32) {
         Temp src = get_alu_src(ctx, instr->src[0]);
         if (instr->src[0].src.ssa->bit_size <= 16)
            src = convert_int(ctx, bld, src, instr->src[0].src.ssa->bit_size, 32, true);
         bld.vop1(aco_opcode::v_cvt_f64_i32, Definition(dst), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_u2f16: {
      assert(dst.regClass() == v2b);
      Temp src = get_alu_src(ctx, instr->src[0]);
      const unsigned input_size = instr->src[0].src.ssa->bit_size;
      if (input_size <= 16) {
         /* Expand integer to the size expected by the uint→float converter used below */
         unsigned target_size = (ctx->program->gfx_level >= GFX8 ? 16 : 32);
         if (input_size != target_size) {
            src = convert_int(ctx, bld, src, input_size, target_size, false);
         }
      }

      if (ctx->program->gfx_level >= GFX8 && input_size <= 16) {
         bld.vop1(aco_opcode::v_cvt_f16_u16, Definition(dst), src);
      } else {
         /* Large 32bit inputs need to return inf/FLOAT_MAX.
          *
          * This is also the fallback-path taken on GFX7 and earlier, which
          * do not support direct f16⟷u16 conversions.
          */
         src = bld.vop1(aco_opcode::v_cvt_f32_u32, bld.def(v1), src);
         bld.vop1(aco_opcode::v_cvt_f16_f32, Definition(dst), src);
      }
      break;
   }
   case nir_op_u2f32: {
      assert(dst.size() == 1);
      Temp src = get_alu_src(ctx, instr->src[0]);
      const unsigned input_size = instr->src[0].src.ssa->bit_size;
      if (input_size == 8) {
         bld.vop1(aco_opcode::v_cvt_f32_ubyte0, Definition(dst), src);
      } else if (input_size <= 32) {
         if (input_size == 16)
            src = convert_int(ctx, bld, src, instr->src[0].src.ssa->bit_size, 32, false);
         bld.vop1(aco_opcode::v_cvt_f32_u32, Definition(dst), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_u2f64: {
      if (instr->src[0].src.ssa->bit_size <= 32) {
         Temp src = get_alu_src(ctx, instr->src[0]);
         if (instr->src[0].src.ssa->bit_size <= 16)
            src = convert_int(ctx, bld, src, instr->src[0].src.ssa->bit_size, 32, false);
         bld.vop1(aco_opcode::v_cvt_f64_u32, Definition(dst), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_f2i8:
   case nir_op_f2i16: {
      if (instr->src[0].src.ssa->bit_size == 16) {
         if (ctx->program->gfx_level >= GFX8) {
            emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_i16_f16, dst);
         } else {
            /* GFX7 and earlier do not support direct f16⟷i16 conversions */
            Temp tmp = bld.tmp(v1);
            emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_f32_f16, tmp);
            tmp = bld.vop1(aco_opcode::v_cvt_i32_f32, bld.def(v1), tmp);
            tmp = convert_int(ctx, bld, tmp, 32, instr->def.bit_size, false,
                              (dst.type() == RegType::sgpr) ? Temp() : dst);
            if (dst.type() == RegType::sgpr) {
               bld.pseudo(aco_opcode::p_as_uniform, Definition(dst), tmp);
            }
         }
      } else if (instr->src[0].src.ssa->bit_size == 32) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_i32_f32, dst);
      } else {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_i32_f64, dst);
      }
      break;
   }
   case nir_op_f2u8:
   case nir_op_f2u16: {
      if (instr->src[0].src.ssa->bit_size == 16) {
         if (ctx->program->gfx_level >= GFX8) {
            emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_u16_f16, dst);
         } else {
            /* GFX7 and earlier do not support direct f16⟷u16 conversions */
            Temp tmp = bld.tmp(v1);
            emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_f32_f16, tmp);
            tmp = bld.vop1(aco_opcode::v_cvt_u32_f32, bld.def(v1), tmp);
            tmp = convert_int(ctx, bld, tmp, 32, instr->def.bit_size, false,
                              (dst.type() == RegType::sgpr) ? Temp() : dst);
            if (dst.type() == RegType::sgpr) {
               bld.pseudo(aco_opcode::p_as_uniform, Definition(dst), tmp);
            }
         }
      } else if (instr->src[0].src.ssa->bit_size == 32) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_u32_f32, dst);
      } else {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_u32_f64, dst);
      }
      break;
   }
   case nir_op_f2i32: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (instr->src[0].src.ssa->bit_size == 16) {
         Temp tmp = bld.vop1(aco_opcode::v_cvt_f32_f16, bld.def(v1), src);
         if (dst.type() == RegType::vgpr) {
            bld.vop1(aco_opcode::v_cvt_i32_f32, Definition(dst), tmp);
         } else {
            bld.pseudo(aco_opcode::p_as_uniform, Definition(dst),
                       bld.vop1(aco_opcode::v_cvt_i32_f32, bld.def(v1), tmp));
         }
      } else if (instr->src[0].src.ssa->bit_size == 32) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_i32_f32, dst);
      } else if (instr->src[0].src.ssa->bit_size == 64) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_i32_f64, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_f2u32: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (instr->src[0].src.ssa->bit_size == 16) {
         Temp tmp = bld.vop1(aco_opcode::v_cvt_f32_f16, bld.def(v1), src);
         if (dst.type() == RegType::vgpr) {
            bld.vop1(aco_opcode::v_cvt_u32_f32, Definition(dst), tmp);
         } else {
            bld.pseudo(aco_opcode::p_as_uniform, Definition(dst),
                       bld.vop1(aco_opcode::v_cvt_u32_f32, bld.def(v1), tmp));
         }
      } else if (instr->src[0].src.ssa->bit_size == 32) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_u32_f32, dst);
      } else if (instr->src[0].src.ssa->bit_size == 64) {
         emit_vop1_instruction(ctx, instr, aco_opcode::v_cvt_u32_f64, dst);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_b2f16: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      assert(src.regClass() == bld.lm);

      if (dst.regClass() == s1) {
         src = bool_to_scalar_condition(ctx, src);
         bld.sop2(aco_opcode::s_mul_i32, Definition(dst), Operand::c32(0x3c00u), src);
      } else if (dst.regClass() == v2b) {
         Temp one = bld.copy(bld.def(v1), Operand::c32(0x3c00u));
         bld.vop2(aco_opcode::v_cndmask_b32, Definition(dst), Operand::zero(), one, src);
      } else {
         unreachable("Wrong destination register class for nir_op_b2f16.");
      }
      break;
   }
   case nir_op_b2f32: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      assert(src.regClass() == bld.lm);

      if (dst.regClass() == s1) {
         src = bool_to_scalar_condition(ctx, src);
         bld.sop2(aco_opcode::s_mul_i32, Definition(dst), Operand::c32(0x3f800000u), src);
      } else if (dst.regClass() == v1) {
         bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(dst), Operand::zero(),
                      Operand::c32(0x3f800000u), src);
      } else {
         unreachable("Wrong destination register class for nir_op_b2f32.");
      }
      break;
   }
   case nir_op_b2f64: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      assert(src.regClass() == bld.lm);

      if (dst.regClass() == s2) {
         src = bool_to_scalar_condition(ctx, src);
         bld.sop2(aco_opcode::s_cselect_b64, Definition(dst), Operand::c32(0x3f800000u),
                  Operand::zero(), bld.scc(src));
      } else if (dst.regClass() == v2) {
         Temp one = bld.copy(bld.def(v1), Operand::c32(0x3FF00000u));
         Temp upper =
            bld.vop2_e64(aco_opcode::v_cndmask_b32, bld.def(v1), Operand::zero(), one, src);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), Operand::zero(), upper);
      } else {
         unreachable("Wrong destination register class for nir_op_b2f64.");
      }
      break;
   }
   case nir_op_i2i8:
   case nir_op_i2i16:
   case nir_op_i2i32: {
      if (dst.type() == RegType::sgpr && instr->src[0].src.ssa->bit_size < 32) {
         /* no need to do the extract in get_alu_src() */
         sgpr_extract_mode mode = instr->def.bit_size > instr->src[0].src.ssa->bit_size
                                     ? sgpr_extract_sext
                                     : sgpr_extract_undef;
         extract_8_16_bit_sgpr_element(ctx, dst, &instr->src[0], mode);
      } else {
         const unsigned input_bitsize = instr->src[0].src.ssa->bit_size;
         const unsigned output_bitsize = instr->def.bit_size;
         convert_int(ctx, bld, get_alu_src(ctx, instr->src[0]), input_bitsize, output_bitsize,
                     output_bitsize > input_bitsize, dst);
      }
      break;
   }
   case nir_op_u2u8:
   case nir_op_u2u16:
   case nir_op_u2u32: {
      if (dst.type() == RegType::sgpr && instr->src[0].src.ssa->bit_size < 32) {
         /* no need to do the extract in get_alu_src() */
         sgpr_extract_mode mode = instr->def.bit_size > instr->src[0].src.ssa->bit_size
                                     ? sgpr_extract_zext
                                     : sgpr_extract_undef;
         extract_8_16_bit_sgpr_element(ctx, dst, &instr->src[0], mode);
      } else {
         convert_int(ctx, bld, get_alu_src(ctx, instr->src[0]), instr->src[0].src.ssa->bit_size,
                     instr->def.bit_size, false, dst);
      }
      break;
   }
   case nir_op_b2b32:
   case nir_op_b2i8:
   case nir_op_b2i16:
   case nir_op_b2i32: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      assert(src.regClass() == bld.lm);

      if (dst.regClass() == s1) {
         bool_to_scalar_condition(ctx, src, dst);
      } else if (dst.type() == RegType::vgpr) {
         bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(dst), Operand::zero(), Operand::c32(1u),
                      src);
      } else {
         unreachable("Invalid register class for b2i32");
      }
      break;
   }
   case nir_op_b2b1: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      assert(dst.regClass() == bld.lm);

      if (src.type() == RegType::vgpr) {
         assert(src.regClass() == v1 || src.regClass() == v2);
         assert(dst.regClass() == bld.lm);
         bld.vopc(src.size() == 2 ? aco_opcode::v_cmp_lg_u64 : aco_opcode::v_cmp_lg_u32,
                  Definition(dst), Operand::zero(), src);
      } else {
         assert(src.regClass() == s1 || src.regClass() == s2);
         Temp tmp;
         if (src.regClass() == s2 && ctx->program->gfx_level <= GFX7) {
            tmp =
               bld.sop2(aco_opcode::s_or_b64, bld.def(s2), bld.def(s1, scc), Operand::zero(), src)
                  .def(1)
                  .getTemp();
         } else {
            tmp = bld.sopc(src.size() == 2 ? aco_opcode::s_cmp_lg_u64 : aco_opcode::s_cmp_lg_u32,
                           bld.scc(bld.def(s1)), Operand::zero(), src);
         }
         bool_to_vector_condition(ctx, tmp, dst);
      }
      break;
   }
   case nir_op_unpack_64_2x32:
   case nir_op_unpack_32_2x16:
   case nir_op_unpack_64_4x16:
   case nir_op_unpack_32_4x8:
      bld.copy(Definition(dst), get_alu_src(ctx, instr->src[0]));
      emit_split_vector(
         ctx, dst, instr->op == nir_op_unpack_32_4x8 || instr->op == nir_op_unpack_64_4x16 ? 4 : 2);
      break;
   case nir_op_pack_64_2x32_split: {
      Temp src0 = get_alu_src(ctx, instr->src[0]);
      Temp src1 = get_alu_src(ctx, instr->src[1]);

      bld.pseudo(aco_opcode::p_create_vector, Definition(dst), src0, src1);
      break;
   }
   case nir_op_unpack_64_2x32_split_x:
      bld.pseudo(aco_opcode::p_split_vector, Definition(dst), bld.def(dst.regClass()),
                 get_alu_src(ctx, instr->src[0]));
      break;
   case nir_op_unpack_64_2x32_split_y:
      bld.pseudo(aco_opcode::p_split_vector, bld.def(dst.regClass()), Definition(dst),
                 get_alu_src(ctx, instr->src[0]));
      break;
   case nir_op_unpack_32_2x16_split_x:
      if (dst.type() == RegType::vgpr) {
         bld.pseudo(aco_opcode::p_split_vector, Definition(dst), bld.def(dst.regClass()),
                    get_alu_src(ctx, instr->src[0]));
      } else {
         bld.copy(Definition(dst), get_alu_src(ctx, instr->src[0]));
      }
      break;
   case nir_op_unpack_32_2x16_split_y:
      if (dst.type() == RegType::vgpr) {
         bld.pseudo(aco_opcode::p_split_vector, bld.def(dst.regClass()), Definition(dst),
                    get_alu_src(ctx, instr->src[0]));
      } else {
         bld.pseudo(aco_opcode::p_extract, Definition(dst), bld.def(s1, scc),
                    get_alu_src(ctx, instr->src[0]), Operand::c32(1u), Operand::c32(16u),
                    Operand::zero());
      }
      break;
   case nir_op_pack_32_2x16_split: {
      Temp src0 = get_alu_src(ctx, instr->src[0]);
      Temp src1 = get_alu_src(ctx, instr->src[1]);
      if (dst.regClass() == v1) {
         src0 = emit_extract_vector(ctx, src0, 0, v2b);
         src1 = emit_extract_vector(ctx, src1, 0, v2b);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), src0, src1);
      } else {
         src0 = bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc), src0,
                         Operand::c32(0xFFFFu));
         src1 = bld.sop2(aco_opcode::s_lshl_b32, bld.def(s1), bld.def(s1, scc), src1,
                         Operand::c32(16u));
         bld.sop2(aco_opcode::s_or_b32, Definition(dst), bld.def(s1, scc), src0, src1);
      }
      break;
   }
   case nir_op_pack_32_4x8: bld.copy(Definition(dst), get_alu_src(ctx, instr->src[0], 4)); break;
   case nir_op_pack_half_2x16_rtz_split:
   case nir_op_pack_half_2x16_split: {
      if (dst.regClass() == v1) {
         if (ctx->program->gfx_level == GFX8 || ctx->program->gfx_level == GFX9)
            emit_vop3a_instruction(ctx, instr, aco_opcode::v_cvt_pkrtz_f16_f32_e64, dst);
         else
            emit_vop2_instruction(ctx, instr, aco_opcode::v_cvt_pkrtz_f16_f32, dst, false);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_pack_unorm_2x16:
   case nir_op_pack_snorm_2x16: {
      unsigned bit_size = instr->src[0].src.ssa->bit_size;
      /* Only support 16 and 32bit. */
      assert(bit_size == 32 || bit_size == 16);

      RegClass src_rc = bit_size == 32 ? v1 : v2b;
      Temp src = get_alu_src(ctx, instr->src[0], 2);
      Temp src0 = emit_extract_vector(ctx, src, 0, src_rc);
      Temp src1 = emit_extract_vector(ctx, src, 1, src_rc);

      /* Work around for pre-GFX9 GPU which don't have fp16 pknorm instruction. */
      if (bit_size == 16 && ctx->program->gfx_level < GFX9) {
         src0 = bld.vop1(aco_opcode::v_cvt_f32_f16, bld.def(v1), src0);
         src1 = bld.vop1(aco_opcode::v_cvt_f32_f16, bld.def(v1), src1);
         bit_size = 32;
      }

      aco_opcode opcode;
      if (bit_size == 32) {
         opcode = instr->op == nir_op_pack_unorm_2x16 ? aco_opcode::v_cvt_pknorm_u16_f32
                                                      : aco_opcode::v_cvt_pknorm_i16_f32;
      } else {
         opcode = instr->op == nir_op_pack_unorm_2x16 ? aco_opcode::v_cvt_pknorm_u16_f16
                                                      : aco_opcode::v_cvt_pknorm_i16_f16;
      }
      bld.vop3(opcode, Definition(dst), src0, src1);
      break;
   }
   case nir_op_pack_uint_2x16:
   case nir_op_pack_sint_2x16: {
      Temp src = get_alu_src(ctx, instr->src[0], 2);
      Temp src0 = emit_extract_vector(ctx, src, 0, v1);
      Temp src1 = emit_extract_vector(ctx, src, 1, v1);
      aco_opcode opcode = instr->op == nir_op_pack_uint_2x16 ? aco_opcode::v_cvt_pk_u16_u32
                                                             : aco_opcode::v_cvt_pk_i16_i32;
      bld.vop3(opcode, Definition(dst), src0, src1);
      break;
   }
   case nir_op_unpack_half_2x16_split_x_flush_to_zero:
   case nir_op_unpack_half_2x16_split_x: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (src.regClass() == v1)
         src = bld.pseudo(aco_opcode::p_split_vector, bld.def(v2b), bld.def(v2b), src);
      if (dst.regClass() == v1) {
         assert(ctx->block->fp_mode.must_flush_denorms16_64 ==
                (instr->op == nir_op_unpack_half_2x16_split_x_flush_to_zero));
         bld.vop1(aco_opcode::v_cvt_f32_f16, Definition(dst), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_unpack_half_2x16_split_y_flush_to_zero:
   case nir_op_unpack_half_2x16_split_y: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (src.regClass() == s1)
         src = bld.pseudo(aco_opcode::p_extract, bld.def(s1), bld.def(s1, scc), src,
                          Operand::c32(1u), Operand::c32(16u), Operand::zero());
      else
         src =
            bld.pseudo(aco_opcode::p_split_vector, bld.def(v2b), bld.def(v2b), src).def(1).getTemp();
      if (dst.regClass() == v1) {
         assert(ctx->block->fp_mode.must_flush_denorms16_64 ==
                (instr->op == nir_op_unpack_half_2x16_split_y_flush_to_zero));
         bld.vop1(aco_opcode::v_cvt_f32_f16, Definition(dst), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_msad_4x8: {
      assert(dst.regClass() == v1);
      emit_vop3a_instruction(ctx, instr, aco_opcode::v_msad_u8, dst, false, 3u, true);
      break;
   }
   case nir_op_fquantize2f16: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      Temp f16;
      if (ctx->block->fp_mode.round16_64 != fp_round_ne)
         f16 = bld.vop1(aco_opcode::p_cvt_f16_f32_rtne, bld.def(v2b), src);
      else
         f16 = bld.vop1(aco_opcode::v_cvt_f16_f32, bld.def(v2b), src);
      Temp f32, cmp_res;

      if (ctx->program->gfx_level >= GFX8) {
         Temp mask = bld.copy(
            bld.def(s1), Operand::c32(0x36Fu)); /* value is NOT negative/positive denormal value */
         cmp_res = bld.vopc_e64(aco_opcode::v_cmp_class_f16, bld.def(bld.lm), f16, mask);
         f32 = bld.vop1(aco_opcode::v_cvt_f32_f16, bld.def(v1), f16);
      } else {
         /* 0x38800000 is smallest half float value (2^-14) in 32-bit float,
          * so compare the result and flush to 0 if it's smaller.
          */
         f32 = bld.vop1(aco_opcode::v_cvt_f32_f16, bld.def(v1), f16);
         Temp smallest = bld.copy(bld.def(s1), Operand::c32(0x38800000u));
         Instruction* tmp0 = bld.vopc_e64(aco_opcode::v_cmp_lt_f32, bld.def(bld.lm), f32, smallest);
         tmp0->valu().abs[0] = true;
         Temp tmp1 = bld.vopc(aco_opcode::v_cmp_lg_f32, bld.def(bld.lm), Operand::zero(), f32);
         cmp_res = bld.sop2(aco_opcode::s_nand_b64, bld.def(s2), bld.def(s1, scc),
                            tmp0->definitions[0].getTemp(), tmp1);
      }

      if (ctx->block->fp_mode.preserve_signed_zero_inf_nan32) {
         Temp copysign_0 =
            bld.vop2(aco_opcode::v_mul_f32, bld.def(v1), Operand::zero(), as_vgpr(ctx, src));
         bld.vop2(aco_opcode::v_cndmask_b32, Definition(dst), copysign_0, f32, cmp_res);
      } else {
         bld.vop2(aco_opcode::v_cndmask_b32, Definition(dst), Operand::zero(), f32, cmp_res);
      }
      break;
   }
   case nir_op_bfm: {
      Temp bits = get_alu_src(ctx, instr->src[0]);
      Temp offset = get_alu_src(ctx, instr->src[1]);

      if (dst.regClass() == s1) {
         bld.sop2(aco_opcode::s_bfm_b32, Definition(dst), bits, offset);
      } else if (dst.regClass() == v1) {
         bld.vop3(aco_opcode::v_bfm_b32, Definition(dst), bits, offset);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_bitfield_select: {

      /* dst = (insert & bitmask) | (base & ~bitmask) */
      if (dst.regClass() == s1) {
         Temp bitmask = get_alu_src(ctx, instr->src[0]);
         Temp insert = get_alu_src(ctx, instr->src[1]);
         Temp base = get_alu_src(ctx, instr->src[2]);
         aco_ptr<Instruction> sop2;
         nir_const_value* const_bitmask = nir_src_as_const_value(instr->src[0].src);
         nir_const_value* const_insert = nir_src_as_const_value(instr->src[1].src);
         Operand lhs;
         if (const_insert && const_bitmask) {
            lhs = Operand::c32(const_insert->u32 & const_bitmask->u32);
         } else {
            insert =
               bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc), insert, bitmask);
            lhs = Operand(insert);
         }

         Operand rhs;
         nir_const_value* const_base = nir_src_as_const_value(instr->src[2].src);
         if (const_base && const_bitmask) {
            rhs = Operand::c32(const_base->u32 & ~const_bitmask->u32);
         } else {
            base = bld.sop2(aco_opcode::s_andn2_b32, bld.def(s1), bld.def(s1, scc), base, bitmask);
            rhs = Operand(base);
         }

         bld.sop2(aco_opcode::s_or_b32, Definition(dst), bld.def(s1, scc), rhs, lhs);

      } else if (dst.regClass() == v1) {
         emit_vop3a_instruction(ctx, instr, aco_opcode::v_bfi_b32, dst, false, 3);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_ubfe:
   case nir_op_ibfe: {
      if (dst.bytes() != 4)
         unreachable("Unsupported BFE bit size");

      if (dst.type() == RegType::sgpr) {
         Temp base = get_alu_src(ctx, instr->src[0]);

         nir_const_value* const_offset = nir_src_as_const_value(instr->src[1].src);
         nir_const_value* const_bits = nir_src_as_const_value(instr->src[2].src);
         aco_opcode opcode =
            instr->op == nir_op_ubfe ? aco_opcode::s_bfe_u32 : aco_opcode::s_bfe_i32;
         if (const_offset && const_bits) {
            uint32_t extract = ((const_bits->u32 & 0x1f) << 16) | (const_offset->u32 & 0x1f);
            bld.sop2(opcode, Definition(dst), bld.def(s1, scc), base, Operand::c32(extract));
            break;
         }

         Temp offset = get_alu_src(ctx, instr->src[1]);
         Temp bits = get_alu_src(ctx, instr->src[2]);

         if (ctx->program->gfx_level >= GFX9) {
            Operand bits_op = const_bits ? Operand::c32(const_bits->u32 & 0x1f)
                                         : bld.sop2(aco_opcode::s_and_b32, bld.def(s1),
                                                    bld.def(s1, scc), bits, Operand::c32(0x1fu));
            Temp extract = bld.sop2(aco_opcode::s_pack_ll_b32_b16, bld.def(s1), offset, bits_op);
            bld.sop2(opcode, Definition(dst), bld.def(s1, scc), base, extract);
         } else if (instr->op == nir_op_ubfe) {
            Temp mask = bld.sop2(aco_opcode::s_bfm_b32, bld.def(s1), bits, offset);
            Temp masked =
               bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc), base, mask);
            bld.sop2(aco_opcode::s_lshr_b32, Definition(dst), bld.def(s1, scc), masked, offset);
         } else {
            Operand bits_op = const_bits
                                 ? Operand::c32((const_bits->u32 & 0x1f) << 16)
                                 : bld.sop2(aco_opcode::s_lshl_b32, bld.def(s1), bld.def(s1, scc),
                                            bld.sop2(aco_opcode::s_and_b32, bld.def(s1),
                                                     bld.def(s1, scc), bits, Operand::c32(0x1fu)),
                                            Operand::c32(16u));
            Operand offset_op = const_offset
                                   ? Operand::c32(const_offset->u32 & 0x1fu)
                                   : bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc),
                                              offset, Operand::c32(0x1fu));

            Temp extract =
               bld.sop2(aco_opcode::s_or_b32, bld.def(s1), bld.def(s1, scc), bits_op, offset_op);
            bld.sop2(aco_opcode::s_bfe_i32, Definition(dst), bld.def(s1, scc), base, extract);
         }

      } else {
         aco_opcode opcode =
            instr->op == nir_op_ubfe ? aco_opcode::v_bfe_u32 : aco_opcode::v_bfe_i32;
         emit_vop3a_instruction(ctx, instr, opcode, dst, false, 3);
      }
      break;
   }
   case nir_op_extract_u8:
   case nir_op_extract_i8:
   case nir_op_extract_u16:
   case nir_op_extract_i16: {
      bool is_signed = instr->op == nir_op_extract_i16 || instr->op == nir_op_extract_i8;
      unsigned comp = instr->op == nir_op_extract_u8 || instr->op == nir_op_extract_i8 ? 4 : 2;
      uint32_t bits = comp == 4 ? 8 : 16;
      unsigned index = nir_src_as_uint(instr->src[1].src);
      if (bits >= instr->def.bit_size || index * bits >= instr->def.bit_size) {
         assert(index == 0);
         bld.copy(Definition(dst), get_alu_src(ctx, instr->src[0]));
      } else if (dst.regClass() == s1 && instr->def.bit_size == 16) {
         Temp vec = get_ssa_temp(ctx, instr->src[0].src.ssa);
         unsigned swizzle = instr->src[0].swizzle[0];
         if (vec.size() > 1) {
            vec = emit_extract_vector(ctx, vec, swizzle / 2, s1);
            swizzle = swizzle & 1;
         }
         index += swizzle * instr->def.bit_size / bits;
         bld.pseudo(aco_opcode::p_extract, Definition(dst), bld.def(s1, scc), Operand(vec),
                    Operand::c32(index), Operand::c32(bits), Operand::c32(is_signed));
      } else {
         Temp src = get_alu_src(ctx, instr->src[0]);
         Definition def(dst);
         if (dst.bytes() == 8) {
            src = emit_extract_vector(ctx, src, index / comp, RegClass(src.type(), 1));
            index %= comp;
            def = bld.def(src.type(), 1);
         }
         assert(def.bytes() <= 4);
         if (def.regClass() == s1) {
            bld.pseudo(aco_opcode::p_extract, def, bld.def(s1, scc), Operand(src),
                       Operand::c32(index), Operand::c32(bits), Operand::c32(is_signed));
         } else {
            src = emit_extract_vector(ctx, src, 0, def.regClass());
            bld.pseudo(aco_opcode::p_extract, def, Operand(src), Operand::c32(index),
                       Operand::c32(bits), Operand::c32(is_signed));
         }
         if (dst.size() == 2)
            bld.pseudo(aco_opcode::p_create_vector, Definition(dst), def.getTemp(),
                       Operand::zero());
      }
      break;
   }
   case nir_op_insert_u8:
   case nir_op_insert_u16: {
      unsigned comp = instr->op == nir_op_insert_u8 ? 4 : 2;
      uint32_t bits = comp == 4 ? 8 : 16;
      unsigned index = nir_src_as_uint(instr->src[1].src);
      if (bits >= instr->def.bit_size || index * bits >= instr->def.bit_size) {
         assert(index == 0);
         bld.copy(Definition(dst), get_alu_src(ctx, instr->src[0]));
      } else {
         Temp src = get_alu_src(ctx, instr->src[0]);
         Definition def(dst);
         bool swap = false;
         if (dst.bytes() == 8) {
            src = emit_extract_vector(ctx, src, 0u, RegClass(src.type(), 1));
            swap = index >= comp;
            index %= comp;
            def = bld.def(src.type(), 1);
         }
         if (def.regClass() == s1) {
            bld.pseudo(aco_opcode::p_insert, def, bld.def(s1, scc), Operand(src),
                       Operand::c32(index), Operand::c32(bits));
         } else {
            src = emit_extract_vector(ctx, src, 0, def.regClass());
            bld.pseudo(aco_opcode::p_insert, def, Operand(src), Operand::c32(index),
                       Operand::c32(bits));
         }
         if (dst.size() == 2 && swap)
            bld.pseudo(aco_opcode::p_create_vector, Definition(dst), Operand::zero(),
                       def.getTemp());
         else if (dst.size() == 2)
            bld.pseudo(aco_opcode::p_create_vector, Definition(dst), def.getTemp(),
                       Operand::zero());
      }
      break;
   }
   case nir_op_bit_count: {
      Temp src = get_alu_src(ctx, instr->src[0]);
      if (src.regClass() == s1) {
         bld.sop1(aco_opcode::s_bcnt1_i32_b32, Definition(dst), bld.def(s1, scc), src);
      } else if (src.regClass() == v1) {
         bld.vop3(aco_opcode::v_bcnt_u32_b32, Definition(dst), src, Operand::zero());
      } else if (src.regClass() == v2) {
         bld.vop3(aco_opcode::v_bcnt_u32_b32, Definition(dst), emit_extract_vector(ctx, src, 1, v1),
                  bld.vop3(aco_opcode::v_bcnt_u32_b32, bld.def(v1),
                           emit_extract_vector(ctx, src, 0, v1), Operand::zero()));
      } else if (src.regClass() == s2) {
         bld.sop1(aco_opcode::s_bcnt1_i32_b64, Definition(dst), bld.def(s1, scc), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_op_flt: {
      emit_comparison(ctx, instr, dst, aco_opcode::v_cmp_lt_f16, aco_opcode::v_cmp_lt_f32,
                      aco_opcode::v_cmp_lt_f64);
      break;
   }
   case nir_op_fge: {
      emit_comparison(ctx, instr, dst, aco_opcode::v_cmp_ge_f16, aco_opcode::v_cmp_ge_f32,
                      aco_opcode::v_cmp_ge_f64);
      break;
   }
   case nir_op_feq: {
      emit_comparison(ctx, instr, dst, aco_opcode::v_cmp_eq_f16, aco_opcode::v_cmp_eq_f32,
                      aco_opcode::v_cmp_eq_f64);
      break;
   }
   case nir_op_fneu: {
      emit_comparison(ctx, instr, dst, aco_opcode::v_cmp_neq_f16, aco_opcode::v_cmp_neq_f32,
                      aco_opcode::v_cmp_neq_f64);
      break;
   }
   case nir_op_ilt: {
      emit_comparison(ctx, instr, dst, aco_opcode::v_cmp_lt_i16, aco_opcode::v_cmp_lt_i32,
                      aco_opcode::v_cmp_lt_i64, aco_opcode::s_cmp_lt_i32);
      break;
   }
   case nir_op_ige: {
      emit_comparison(ctx, instr, dst, aco_opcode::v_cmp_ge_i16, aco_opcode::v_cmp_ge_i32,
                      aco_opcode::v_cmp_ge_i64, aco_opcode::s_cmp_ge_i32);
      break;
   }
   case nir_op_ieq: {
      if (instr->src[0].src.ssa->bit_size == 1)
         emit_boolean_logic(ctx, instr, Builder::s_xnor, dst);
      else
         emit_comparison(
            ctx, instr, dst, aco_opcode::v_cmp_eq_i16, aco_opcode::v_cmp_eq_i32,
            aco_opcode::v_cmp_eq_i64, aco_opcode::s_cmp_eq_i32,
            ctx->program->gfx_level >= GFX8 ? aco_opcode::s_cmp_eq_u64 : aco_opcode::num_opcodes);
      break;
   }
   case nir_op_ine: {
      if (instr->src[0].src.ssa->bit_size == 1)
         emit_boolean_logic(ctx, instr, Builder::s_xor, dst);
      else
         emit_comparison(
            ctx, instr, dst, aco_opcode::v_cmp_lg_i16, aco_opcode::v_cmp_lg_i32,
            aco_opcode::v_cmp_lg_i64, aco_opcode::s_cmp_lg_i32,
            ctx->program->gfx_level >= GFX8 ? aco_opcode::s_cmp_lg_u64 : aco_opcode::num_opcodes);
      break;
   }
   case nir_op_ult: {
      emit_comparison(ctx, instr, dst, aco_opcode::v_cmp_lt_u16, aco_opcode::v_cmp_lt_u32,
                      aco_opcode::v_cmp_lt_u64, aco_opcode::s_cmp_lt_u32);
      break;
   }
   case nir_op_uge: {
      emit_comparison(ctx, instr, dst, aco_opcode::v_cmp_ge_u16, aco_opcode::v_cmp_ge_u32,
                      aco_opcode::v_cmp_ge_u64, aco_opcode::s_cmp_ge_u32);
      break;
   }
   case nir_op_bitz:
   case nir_op_bitnz: {
      assert(instr->src[0].src.ssa->bit_size != 1);
      bool test0 = instr->op == nir_op_bitz;
      Temp src0 = get_alu_src(ctx, instr->src[0]);
      Temp src1 = get_alu_src(ctx, instr->src[1]);
      bool use_valu = src0.type() == RegType::vgpr || src1.type() == RegType::vgpr;
      if (!use_valu) {
         aco_opcode op = instr->src[0].src.ssa->bit_size == 64 ? aco_opcode::s_bitcmp1_b64
                                                               : aco_opcode::s_bitcmp1_b32;
         if (test0)
            op = instr->src[0].src.ssa->bit_size == 64 ? aco_opcode::s_bitcmp0_b64
                                                       : aco_opcode::s_bitcmp0_b32;
         emit_sopc_instruction(ctx, instr, op, dst);
         break;
      }

      /* We do not have a VALU version of s_bitcmp.
       * But if the second source is constant, we can use
       * v_cmp_class_f32's LUT to check the bit.
       * The LUT only has 10 entries, so extract a higher byte if we have to.
       * For sign bits comparision with 0 is better because v_cmp_class
       * can't be inverted.
       */
      if (nir_src_is_const(instr->src[1].src)) {
         uint32_t bit = nir_alu_src_as_uint(instr->src[1]);
         bit &= instr->src[0].src.ssa->bit_size - 1;
         src0 = as_vgpr(ctx, src0);

         if (src0.regClass() == v2) {
            src0 = emit_extract_vector(ctx, src0, (bit & 32) != 0, v1);
            bit &= 31;
         }

         if (bit == 31) {
            bld.vopc(test0 ? aco_opcode::v_cmp_le_i32 : aco_opcode::v_cmp_gt_i32, Definition(dst),
                     Operand::c32(0), src0);
            break;
         }

         if (bit == 15 && ctx->program->gfx_level >= GFX8) {
            bld.vopc(test0 ? aco_opcode::v_cmp_le_i16 : aco_opcode::v_cmp_gt_i16, Definition(dst),
                     Operand::c32(0), src0);
            break;
         }

         /* Set max_bit lower to avoid +inf if we can use sdwa+qnan instead. */
         const bool can_sdwa = ctx->program->gfx_level >= GFX8 && ctx->program->gfx_level < GFX11;
         const unsigned max_bit = can_sdwa ? 0x8 : 0x9;
         const bool use_opsel = bit > 0xf && (bit & 0xf) <= max_bit;
         if (use_opsel) {
            src0 = bld.pseudo(aco_opcode::p_extract, bld.def(v1), src0, Operand::c32(1),
                              Operand::c32(16), Operand::c32(0));
            bit &= 0xf;
         }

         /* If we can use sdwa the extract is free, while test0's s_not is not. */
         if (bit == 7 && test0 && can_sdwa) {
            src0 = bld.pseudo(aco_opcode::p_extract, bld.def(v1), src0, Operand::c32(bit / 8),
                              Operand::c32(8), Operand::c32(1));
            bld.vopc(test0 ? aco_opcode::v_cmp_le_i32 : aco_opcode::v_cmp_gt_i32, Definition(dst),
                     Operand::c32(0), src0);
            break;
         }

         if (bit > max_bit) {
            src0 = bld.pseudo(aco_opcode::p_extract, bld.def(v1), src0, Operand::c32(bit / 8),
                              Operand::c32(8), Operand::c32(0));
            bit &= 0x7;
         }

         /* denorm and snan/qnan inputs are preserved using all float control modes. */
         static const struct {
            uint32_t fp32;
            uint32_t fp16;
            bool negate;
         } float_lut[10] = {
            {0x7f800001, 0x7c01, false}, /* snan */
            {~0u, ~0u, false},           /* qnan */
            {0xff800000, 0xfc00, false}, /* -inf */
            {0xbf800000, 0xbc00, false}, /* -normal (-1.0) */
            {1, 1, true},                /* -denormal */
            {0, 0, true},                /* -0.0 */
            {0, 0, false},               /* +0.0 */
            {1, 1, false},               /* +denormal */
            {0x3f800000, 0x3c00, false}, /* +normal (+1.0) */
            {0x7f800000, 0x7c00, false}, /* +inf */
         };

         Temp tmp = test0 ? bld.tmp(bld.lm) : dst;
         /* fp16 can use s_movk for bit 0. It also supports opsel on gfx11. */
         const bool use_fp16 = (ctx->program->gfx_level >= GFX8 && bit == 0) ||
                               (ctx->program->gfx_level >= GFX11 && use_opsel);
         const aco_opcode op = use_fp16 ? aco_opcode::v_cmp_class_f16 : aco_opcode::v_cmp_class_f32;
         const uint32_t c = use_fp16 ? float_lut[bit].fp16 : float_lut[bit].fp32;

         VALU_instruction& res =
            bld.vopc(op, Definition(tmp), bld.copy(bld.def(s1), Operand::c32(c)), src0)->valu();
         if (float_lut[bit].negate) {
            res.format = asVOP3(res.format);
            res.neg[0] = true;
         }

         if (test0)
            bld.sop1(Builder::s_not, Definition(dst), bld.def(s1, scc), tmp);

         break;
      }

      Temp res;
      aco_opcode op = test0 ? aco_opcode::v_cmp_eq_i32 : aco_opcode::v_cmp_lg_i32;
      if (instr->src[0].src.ssa->bit_size == 16) {
         op = test0 ? aco_opcode::v_cmp_eq_i16 : aco_opcode::v_cmp_lg_i16;
         if (ctx->program->gfx_level < GFX10)
            res = bld.vop2_e64(aco_opcode::v_lshlrev_b16, bld.def(v2b), src1, Operand::c32(1));
         else
            res = bld.vop3(aco_opcode::v_lshlrev_b16_e64, bld.def(v2b), src1, Operand::c32(1));

         res = bld.vop2(aco_opcode::v_and_b32, bld.def(v2b), src0, res);
      } else if (instr->src[0].src.ssa->bit_size == 32) {
         res = bld.vop3(aco_opcode::v_bfe_u32, bld.def(v1), src0, src1, Operand::c32(1));
      } else if (instr->src[0].src.ssa->bit_size == 64) {
         if (ctx->program->gfx_level < GFX8)
            res = bld.vop3(aco_opcode::v_lshr_b64, bld.def(v2), src0, src1);
         else
            res = bld.vop3(aco_opcode::v_lshrrev_b64, bld.def(v2), src1, src0);

         res = emit_extract_vector(ctx, res, 0, v1);
         res = bld.vop2(aco_opcode::v_and_b32, bld.def(v1), Operand::c32(0x1), res);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      bld.vopc(op, Definition(dst), Operand::c32(0), res);
      break;
   }
   case nir_op_fddx:
   case nir_op_fddy:
   case nir_op_fddx_fine:
   case nir_op_fddy_fine:
   case nir_op_fddx_coarse:
   case nir_op_fddy_coarse: {
      if (!nir_src_is_divergent(instr->src[0].src)) {
         /* Source is the same in all lanes, so the derivative is zero.
          * This also avoids emitting invalid IR.
          */
         bld.copy(Definition(dst), Operand::zero(dst.bytes()));
         break;
      }

      uint16_t dpp_ctrl1, dpp_ctrl2;
      if (instr->op == nir_op_fddx_fine) {
         dpp_ctrl1 = dpp_quad_perm(0, 0, 2, 2);
         dpp_ctrl2 = dpp_quad_perm(1, 1, 3, 3);
      } else if (instr->op == nir_op_fddy_fine) {
         dpp_ctrl1 = dpp_quad_perm(0, 1, 0, 1);
         dpp_ctrl2 = dpp_quad_perm(2, 3, 2, 3);
      } else {
         dpp_ctrl1 = dpp_quad_perm(0, 0, 0, 0);
         if (instr->op == nir_op_fddx || instr->op == nir_op_fddx_coarse)
            dpp_ctrl2 = dpp_quad_perm(1, 1, 1, 1);
         else
            dpp_ctrl2 = dpp_quad_perm(2, 2, 2, 2);
      }

      if (dst.regClass() == v1 && instr->def.bit_size == 16) {
         assert(instr->def.num_components == 2);

         Temp src = as_vgpr(ctx, get_alu_src_vop3p(ctx, instr->src[0]));

         /* swizzle to opsel: all swizzles are either 0 (x) or 1 (y) */
         unsigned opsel_lo = instr->src[0].swizzle[0] & 1;
         unsigned opsel_hi = instr->src[0].swizzle[1] & 1;
         opsel_lo |= opsel_lo << 1;
         opsel_hi |= opsel_hi << 1;

         Temp tl = bld.vop1_dpp(aco_opcode::v_mov_b32, bld.def(v1), src, dpp_ctrl1);
         Temp tr = bld.vop1_dpp(aco_opcode::v_mov_b32, bld.def(v1), src, dpp_ctrl2);

         VALU_instruction& sub =
            bld.vop3p(aco_opcode::v_pk_add_f16, Definition(dst), tr, tl, opsel_lo, opsel_hi)
               .instr->valu();
         sub.neg_lo[1] = true;
         sub.neg_hi[1] = true;
      } else {
         Temp src = as_vgpr(ctx, get_alu_src(ctx, instr->src[0]));

         if (ctx->program->gfx_level >= GFX8) {
            aco_opcode sub =
               instr->def.bit_size == 16 ? aco_opcode::v_sub_f16 : aco_opcode::v_sub_f32;
            Temp tl = bld.vop1_dpp(aco_opcode::v_mov_b32, bld.def(v1), src, dpp_ctrl1);
            bld.vop2_dpp(sub, Definition(dst), src, tl, dpp_ctrl2);
         } else {
            Temp tl = bld.ds(aco_opcode::ds_swizzle_b32, bld.def(v1), src, (1 << 15) | dpp_ctrl1);
            Temp tr = bld.ds(aco_opcode::ds_swizzle_b32, bld.def(v1), src, (1 << 15) | dpp_ctrl2);
            bld.vop2(aco_opcode::v_sub_f32, Definition(dst), tr, tl);
         }
      }
      set_wqm(ctx, true);
      break;
   }
   default: isel_err(&instr->instr, "Unknown NIR ALU instr");
   }
}

void
visit_load_const(isel_context* ctx, nir_load_const_instr* instr)
{
   Temp dst = get_ssa_temp(ctx, &instr->def);

   // TODO: we really want to have the resulting type as this would allow for 64bit literals
   // which get truncated the lsb if double and msb if int
   // for now, we only use s_mov_b64 with 64bit inline constants
   assert(instr->def.num_components == 1 && "Vector load_const should be lowered to scalar.");
   assert(dst.type() == RegType::sgpr);

   Builder bld(ctx->program, ctx->block);

   if (instr->def.bit_size == 1) {
      assert(dst.regClass() == bld.lm);
      int val = instr->value[0].b ? -1 : 0;
      Operand op = bld.lm.size() == 1 ? Operand::c32(val) : Operand::c64(val);
      bld.copy(Definition(dst), op);
   } else if (instr->def.bit_size == 8) {
      bld.copy(Definition(dst), Operand::c32(instr->value[0].u8));
   } else if (instr->def.bit_size == 16) {
      /* sign-extend to use s_movk_i32 instead of a literal */
      bld.copy(Definition(dst), Operand::c32(instr->value[0].i16));
   } else if (dst.size() == 1) {
      bld.copy(Definition(dst), Operand::c32(instr->value[0].u32));
   } else {
      assert(dst.size() != 1);
      aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
         aco_opcode::p_create_vector, Format::PSEUDO, dst.size(), 1)};
      if (instr->def.bit_size == 64)
         for (unsigned i = 0; i < dst.size(); i++)
            vec->operands[i] = Operand::c32(instr->value[0].u64 >> i * 32);
      else {
         for (unsigned i = 0; i < dst.size(); i++)
            vec->operands[i] = Operand::c32(instr->value[i].u32);
      }
      vec->definitions[0] = Definition(dst);
      ctx->block->instructions.emplace_back(std::move(vec));
   }
}

Temp
emit_readfirstlane(isel_context* ctx, Temp src, Temp dst)
{
   Builder bld(ctx->program, ctx->block);

   if (src.regClass().type() == RegType::sgpr) {
      bld.copy(Definition(dst), src);
   } else if (src.size() == 1) {
      bld.vop1(aco_opcode::v_readfirstlane_b32, Definition(dst), src);
   } else {
      aco_ptr<Pseudo_instruction> split{create_instruction<Pseudo_instruction>(
         aco_opcode::p_split_vector, Format::PSEUDO, 1, src.size())};
      split->operands[0] = Operand(src);

      for (unsigned i = 0; i < src.size(); i++) {
         split->definitions[i] =
            bld.def(RegClass::get(RegType::vgpr, MIN2(src.bytes() - i * 4, 4)));
      }

      Instruction* split_raw = split.get();
      ctx->block->instructions.emplace_back(std::move(split));

      aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
         aco_opcode::p_create_vector, Format::PSEUDO, src.size(), 1)};
      vec->definitions[0] = Definition(dst);
      for (unsigned i = 0; i < src.size(); i++) {
         vec->operands[i] = bld.vop1(aco_opcode::v_readfirstlane_b32, bld.def(s1),
                                     split_raw->definitions[i].getTemp());
      }

      ctx->block->instructions.emplace_back(std::move(vec));
      if (src.bytes() % 4 == 0)
         emit_split_vector(ctx, dst, src.size());
   }

   return dst;
}

bool
can_use_byte_align_for_global_load(unsigned num_components, unsigned component_size,
                                   unsigned align_, bool support_12_byte)
{
   /* Only use byte-align for 8/16-bit loads if we won't have to increase it's size and won't have
    * to use unsupported load sizes.
    */
   assert(util_is_power_of_two_nonzero(align_));
   if (align_ < 4) {
      assert(component_size < 4);
      unsigned load_size = num_components * component_size;
      uint32_t new_size = align(load_size + (4 - align_), 4);
      return new_size == align(load_size, 4) && (new_size != 12 || support_12_byte);
   }
   return true;
}

struct LoadEmitInfo {
   Operand offset;
   Temp dst;
   unsigned num_components;
   unsigned component_size;
   Temp resource = Temp(0, s1); /* buffer resource or base 64-bit address */
   Temp idx = Temp(0, v1);      /* buffer index */
   unsigned component_stride = 0;
   unsigned const_offset = 0;
   unsigned align_mul = 0;
   unsigned align_offset = 0;
   pipe_format format;

   bool glc = false;
   bool slc = false;
   bool split_by_component_stride = true;
   bool readfirstlane_for_uniform = false;
   unsigned swizzle_component_size = 0;
   memory_sync_info sync;
   Temp soffset = Temp(0, s1);
};

struct EmitLoadParameters {
   using Callback = Temp (*)(Builder& bld, const LoadEmitInfo& info, Temp offset,
                             unsigned bytes_needed, unsigned align, unsigned const_offset,
                             Temp dst_hint);

   Callback callback;
   bool byte_align_loads;
   bool supports_8bit_16bit_loads;
   unsigned max_const_offset_plus_one;
};

void
emit_load(isel_context* ctx, Builder& bld, const LoadEmitInfo& info,
          const EmitLoadParameters& params)
{
   unsigned load_size = info.num_components * info.component_size;
   unsigned component_size = info.component_size;

   unsigned num_vals = 0;
   Temp* const vals = (Temp*)alloca(info.dst.bytes() * sizeof(Temp));

   unsigned const_offset = info.const_offset;

   const unsigned align_mul = info.align_mul ? info.align_mul : component_size;
   unsigned align_offset = info.align_offset % align_mul;

   unsigned bytes_read = 0;
   while (bytes_read < load_size) {
      unsigned bytes_needed = load_size - bytes_read;

      /* add buffer for unaligned loads */
      int byte_align = 0;
      if (params.byte_align_loads) {
         byte_align = align_mul % 4 == 0 ? align_offset % 4 : -1;
      }

      if (byte_align) {
         if (bytes_needed > 2 || (bytes_needed == 2 && (align_mul % 2 || align_offset % 2)) ||
             !params.supports_8bit_16bit_loads) {
            if (info.component_stride) {
               assert(params.supports_8bit_16bit_loads && "unimplemented");
               bytes_needed = 2;
               byte_align = 0;
            } else {
               bytes_needed += byte_align == -1 ? 4 - info.align_mul : byte_align;
               bytes_needed = align(bytes_needed, 4);
            }
         } else {
            byte_align = 0;
         }
      }

      if (info.split_by_component_stride) {
         if (info.swizzle_component_size)
            bytes_needed = MIN2(bytes_needed, info.swizzle_component_size);
         if (info.component_stride)
            bytes_needed = MIN2(bytes_needed, info.component_size);
      }

      bool need_to_align_offset = byte_align && (align_mul % 4 || align_offset % 4);

      /* reduce constant offset */
      Operand offset = info.offset;
      unsigned reduced_const_offset = const_offset;
      bool remove_const_offset_completely = need_to_align_offset;
      if (const_offset &&
          (remove_const_offset_completely || const_offset >= params.max_const_offset_plus_one)) {
         unsigned to_add = const_offset;
         if (remove_const_offset_completely) {
            reduced_const_offset = 0;
         } else {
            to_add =
               const_offset / params.max_const_offset_plus_one * params.max_const_offset_plus_one;
            reduced_const_offset %= params.max_const_offset_plus_one;
         }
         Temp offset_tmp = offset.isTemp() ? offset.getTemp() : Temp();
         if (offset.isConstant()) {
            offset = Operand::c32(offset.constantValue() + to_add);
         } else if (offset.isUndefined()) {
            offset = Operand::c32(to_add);
         } else if (offset_tmp.regClass() == s1) {
            offset = bld.sop2(aco_opcode::s_add_i32, bld.def(s1), bld.def(s1, scc), offset_tmp,
                              Operand::c32(to_add));
         } else if (offset_tmp.regClass() == v1) {
            offset = bld.vadd32(bld.def(v1), offset_tmp, Operand::c32(to_add));
         } else {
            Temp lo = bld.tmp(offset_tmp.type(), 1);
            Temp hi = bld.tmp(offset_tmp.type(), 1);
            bld.pseudo(aco_opcode::p_split_vector, Definition(lo), Definition(hi), offset_tmp);

            if (offset_tmp.regClass() == s2) {
               Temp carry = bld.tmp(s1);
               lo = bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.scc(Definition(carry)), lo,
                             Operand::c32(to_add));
               hi = bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.def(s1, scc), hi, carry);
               offset = bld.pseudo(aco_opcode::p_create_vector, bld.def(s2), lo, hi);
            } else {
               Temp new_lo = bld.tmp(v1);
               Temp carry =
                  bld.vadd32(Definition(new_lo), lo, Operand::c32(to_add), true).def(1).getTemp();
               hi = bld.vadd32(bld.def(v1), hi, Operand::zero(), false, carry);
               offset = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), new_lo, hi);
            }
         }
      }

      /* align offset down if needed */
      Operand aligned_offset = offset;
      unsigned align = align_offset ? 1 << (ffs(align_offset) - 1) : align_mul;
      if (need_to_align_offset) {
         align = 4;
         Temp offset_tmp = offset.isTemp() ? offset.getTemp() : Temp();
         if (offset.isConstant()) {
            aligned_offset = Operand::c32(offset.constantValue() & 0xfffffffcu);
         } else if (offset.isUndefined()) {
            aligned_offset = Operand::zero();
         } else if (offset_tmp.regClass() == s1) {
            aligned_offset = bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc),
                                      Operand::c32(0xfffffffcu), offset_tmp);
         } else if (offset_tmp.regClass() == s2) {
            aligned_offset = bld.sop2(aco_opcode::s_and_b64, bld.def(s2), bld.def(s1, scc),
                                      Operand::c64(0xfffffffffffffffcllu), offset_tmp);
         } else if (offset_tmp.regClass() == v1) {
            aligned_offset =
               bld.vop2(aco_opcode::v_and_b32, bld.def(v1), Operand::c32(0xfffffffcu), offset_tmp);
         } else if (offset_tmp.regClass() == v2) {
            Temp hi = bld.tmp(v1), lo = bld.tmp(v1);
            bld.pseudo(aco_opcode::p_split_vector, Definition(lo), Definition(hi), offset_tmp);
            lo = bld.vop2(aco_opcode::v_and_b32, bld.def(v1), Operand::c32(0xfffffffcu), lo);
            aligned_offset = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), lo, hi);
         }
      }
      Temp aligned_offset_tmp = aligned_offset.isTemp() ? aligned_offset.getTemp()
                                : aligned_offset.isConstant()
                                   ? bld.copy(bld.def(s1), aligned_offset)
                                   : Temp(0, s1);

      Temp val = params.callback(bld, info, aligned_offset_tmp, bytes_needed, align,
                                 reduced_const_offset, byte_align ? Temp() : info.dst);

      /* the callback wrote directly to dst */
      if (val == info.dst) {
         assert(num_vals == 0);
         emit_split_vector(ctx, info.dst, info.num_components);
         return;
      }

      /* shift result right if needed */
      if (params.byte_align_loads && info.component_size < 4) {
         Operand byte_align_off = Operand::c32(byte_align);
         if (byte_align == -1) {
            if (offset.isConstant())
               byte_align_off = Operand::c32(offset.constantValue() % 4u);
            else if (offset.isUndefined())
               byte_align_off = Operand::zero();
            else if (offset.size() == 2)
               byte_align_off = Operand(emit_extract_vector(ctx, offset.getTemp(), 0,
                                                            RegClass(offset.getTemp().type(), 1)));
            else
               byte_align_off = offset;
         }

         assert(val.bytes() >= load_size && "unimplemented");
         if (val.type() == RegType::sgpr)
            byte_align_scalar(ctx, val, byte_align_off, info.dst);
         else
            byte_align_vector(ctx, val, byte_align_off, info.dst, component_size);
         return;
      }

      /* add result to list and advance */
      if (info.component_stride) {
         assert(val.bytes() % info.component_size == 0);
         unsigned num_loaded_components = val.bytes() / info.component_size;
         unsigned advance_bytes = info.component_stride * num_loaded_components;
         const_offset += advance_bytes;
         align_offset = (align_offset + advance_bytes) % align_mul;
      } else {
         const_offset += val.bytes();
         align_offset = (align_offset + val.bytes()) % align_mul;
      }
      bytes_read += val.bytes();
      vals[num_vals++] = val;
   }

   /* create array of components */
   unsigned components_split = 0;
   std::array<Temp, NIR_MAX_VEC_COMPONENTS> allocated_vec;
   bool has_vgprs = false;
   for (unsigned i = 0; i < num_vals;) {
      Temp* const tmp = (Temp*)alloca(num_vals * sizeof(Temp));
      unsigned num_tmps = 0;
      unsigned tmp_size = 0;
      RegType reg_type = RegType::sgpr;
      while ((!tmp_size || (tmp_size % component_size)) && i < num_vals) {
         if (vals[i].type() == RegType::vgpr)
            reg_type = RegType::vgpr;
         tmp_size += vals[i].bytes();
         tmp[num_tmps++] = vals[i++];
      }
      if (num_tmps > 1) {
         aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
            aco_opcode::p_create_vector, Format::PSEUDO, num_tmps, 1)};
         for (unsigned j = 0; j < num_tmps; j++)
            vec->operands[j] = Operand(tmp[j]);
         tmp[0] = bld.tmp(RegClass::get(reg_type, tmp_size));
         vec->definitions[0] = Definition(tmp[0]);
         bld.insert(std::move(vec));
      }

      if (tmp[0].bytes() % component_size) {
         /* trim tmp[0] */
         assert(i == num_vals);
         RegClass new_rc =
            RegClass::get(reg_type, tmp[0].bytes() / component_size * component_size);
         tmp[0] =
            bld.pseudo(aco_opcode::p_extract_vector, bld.def(new_rc), tmp[0], Operand::zero());
      }

      RegClass elem_rc = RegClass::get(reg_type, component_size);

      unsigned start = components_split;

      if (tmp_size == elem_rc.bytes()) {
         allocated_vec[components_split++] = tmp[0];
      } else {
         assert(tmp_size % elem_rc.bytes() == 0);
         aco_ptr<Pseudo_instruction> split{create_instruction<Pseudo_instruction>(
            aco_opcode::p_split_vector, Format::PSEUDO, 1, tmp_size / elem_rc.bytes())};
         for (auto& def : split->definitions) {
            Temp component = bld.tmp(elem_rc);
            allocated_vec[components_split++] = component;
            def = Definition(component);
         }
         split->operands[0] = Operand(tmp[0]);
         bld.insert(std::move(split));
      }

      /* try to p_as_uniform early so we can create more optimizable code and
       * also update allocated_vec */
      for (unsigned j = start; j < components_split; j++) {
         if (allocated_vec[j].bytes() % 4 == 0 && info.dst.type() == RegType::sgpr) {
            if (info.readfirstlane_for_uniform) {
               allocated_vec[j] = emit_readfirstlane(
                  ctx, allocated_vec[j], bld.tmp(RegClass(RegType::sgpr, allocated_vec[j].size())));
            } else {
               allocated_vec[j] = bld.as_uniform(allocated_vec[j]);
            }
         }
         has_vgprs |= allocated_vec[j].type() == RegType::vgpr;
      }
   }

   /* concatenate components and p_as_uniform() result if needed */
   if (info.dst.type() == RegType::vgpr || !has_vgprs)
      ctx->allocated_vec.emplace(info.dst.id(), allocated_vec);

   int padding_bytes =
      MAX2((int)info.dst.bytes() - int(allocated_vec[0].bytes() * info.num_components), 0);

   aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
      aco_opcode::p_create_vector, Format::PSEUDO, info.num_components + !!padding_bytes, 1)};
   for (unsigned i = 0; i < info.num_components; i++)
      vec->operands[i] = Operand(allocated_vec[i]);
   if (padding_bytes)
      vec->operands[info.num_components] = Operand(RegClass::get(RegType::vgpr, padding_bytes));
   if (info.dst.type() == RegType::sgpr && has_vgprs) {
      Temp tmp = bld.tmp(RegType::vgpr, info.dst.size());
      vec->definitions[0] = Definition(tmp);
      bld.insert(std::move(vec));
      if (info.readfirstlane_for_uniform)
         emit_readfirstlane(ctx, tmp, info.dst);
      else
         bld.pseudo(aco_opcode::p_as_uniform, Definition(info.dst), tmp);
   } else {
      vec->definitions[0] = Definition(info.dst);
      bld.insert(std::move(vec));
   }
}

Operand
load_lds_size_m0(Builder& bld)
{
   /* m0 does not need to be initialized on GFX9+ */
   if (bld.program->gfx_level >= GFX9)
      return Operand(s1);

   return bld.m0((Temp)bld.copy(bld.def(s1, m0), Operand::c32(0xffffffffu)));
}

Temp
lds_load_callback(Builder& bld, const LoadEmitInfo& info, Temp offset, unsigned bytes_needed,
                  unsigned align, unsigned const_offset, Temp dst_hint)
{
   offset = offset.regClass() == s1 ? bld.copy(bld.def(v1), offset) : offset;

   Operand m = load_lds_size_m0(bld);

   bool large_ds_read = bld.program->gfx_level >= GFX7;
   bool usable_read2 = bld.program->gfx_level >= GFX7;

   bool read2 = false;
   unsigned size = 0;
   aco_opcode op;
   if (bytes_needed >= 16 && align % 16 == 0 && large_ds_read) {
      size = 16;
      op = aco_opcode::ds_read_b128;
   } else if (bytes_needed >= 16 && align % 8 == 0 && const_offset % 8 == 0 && usable_read2) {
      size = 16;
      read2 = true;
      op = aco_opcode::ds_read2_b64;
   } else if (bytes_needed >= 12 && align % 16 == 0 && large_ds_read) {
      size = 12;
      op = aco_opcode::ds_read_b96;
   } else if (bytes_needed >= 8 && align % 8 == 0) {
      size = 8;
      op = aco_opcode::ds_read_b64;
   } else if (bytes_needed >= 8 && align % 4 == 0 && const_offset % 4 == 0 && usable_read2) {
      size = 8;
      read2 = true;
      op = aco_opcode::ds_read2_b32;
   } else if (bytes_needed >= 4 && align % 4 == 0) {
      size = 4;
      op = aco_opcode::ds_read_b32;
   } else if (bytes_needed >= 2 && align % 2 == 0) {
      size = 2;
      op = bld.program->gfx_level >= GFX9 ? aco_opcode::ds_read_u16_d16 : aco_opcode::ds_read_u16;
   } else {
      size = 1;
      op = bld.program->gfx_level >= GFX9 ? aco_opcode::ds_read_u8_d16 : aco_opcode::ds_read_u8;
   }

   unsigned const_offset_unit = read2 ? size / 2u : 1u;
   unsigned const_offset_range = read2 ? 255 * const_offset_unit : 65536;

   if (const_offset > (const_offset_range - const_offset_unit)) {
      unsigned excess = const_offset - (const_offset % const_offset_range);
      offset = bld.vadd32(bld.def(v1), offset, Operand::c32(excess));
      const_offset -= excess;
   }

   const_offset /= const_offset_unit;

   RegClass rc = RegClass::get(RegType::vgpr, size);
   Temp val = rc == info.dst.regClass() && dst_hint.id() ? dst_hint : bld.tmp(rc);
   Instruction* instr;
   if (read2)
      instr = bld.ds(op, Definition(val), offset, m, const_offset, const_offset + 1);
   else
      instr = bld.ds(op, Definition(val), offset, m, const_offset);
   instr->ds().sync = info.sync;

   if (m.isUndefined())
      instr->operands.pop_back();

   return val;
}

const EmitLoadParameters lds_load_params{lds_load_callback, false, true, UINT32_MAX};

Temp
smem_load_callback(Builder& bld, const LoadEmitInfo& info, Temp offset, unsigned bytes_needed,
                   unsigned align, unsigned const_offset, Temp dst_hint)
{
   assert(align >= 4u);

   bld.program->has_smem_buffer_or_global_loads = true;

   bool buffer = info.resource.id() && info.resource.bytes() == 16;
   Temp addr = info.resource;
   if (!buffer && !addr.id()) {
      addr = offset;
      offset = Temp();
   }

   bytes_needed = MIN2(bytes_needed, 64);
   unsigned needed_round_up = util_next_power_of_two(bytes_needed);
   unsigned needed_round_down = needed_round_up >> (needed_round_up != bytes_needed ? 1 : 0);
   /* Only round-up global loads if it's aligned so that it won't cross pages */
   bytes_needed = buffer || align % needed_round_up == 0 ? needed_round_up : needed_round_down;

   aco_opcode op;
   if (bytes_needed <= 4) {
      op = buffer ? aco_opcode::s_buffer_load_dword : aco_opcode::s_load_dword;
   } else if (bytes_needed <= 8) {
      op = buffer ? aco_opcode::s_buffer_load_dwordx2 : aco_opcode::s_load_dwordx2;
   } else if (bytes_needed <= 16) {
      op = buffer ? aco_opcode::s_buffer_load_dwordx4 : aco_opcode::s_load_dwordx4;
   } else if (bytes_needed <= 32) {
      op = buffer ? aco_opcode::s_buffer_load_dwordx8 : aco_opcode::s_load_dwordx8;
   } else {
      assert(bytes_needed == 64);
      op = buffer ? aco_opcode::s_buffer_load_dwordx16 : aco_opcode::s_load_dwordx16;
   }

   aco_ptr<SMEM_instruction> load{create_instruction<SMEM_instruction>(op, Format::SMEM, 2, 1)};
   if (buffer) {
      if (const_offset)
         offset = bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.def(s1, scc), offset,
                           Operand::c32(const_offset));
      load->operands[0] = Operand(info.resource);
      load->operands[1] = Operand(offset);
   } else {
      load->operands[0] = Operand(addr);
      if (offset.id() && const_offset)
         load->operands[1] = bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.def(s1, scc), offset,
                                      Operand::c32(const_offset));
      else if (offset.id())
         load->operands[1] = Operand(offset);
      else
         load->operands[1] = Operand::c32(const_offset);
   }
   RegClass rc(RegType::sgpr, DIV_ROUND_UP(bytes_needed, 4u));
   Temp val = dst_hint.id() && dst_hint.regClass() == rc ? dst_hint : bld.tmp(rc);
   load->definitions[0] = Definition(val);
   load->glc = info.glc;
   load->dlc = info.glc && (bld.program->gfx_level == GFX10 || bld.program->gfx_level == GFX10_3);
   load->sync = info.sync;
   bld.insert(std::move(load));
   return val;
}

const EmitLoadParameters smem_load_params{smem_load_callback, true, false, 1024};

Temp
mubuf_load_callback(Builder& bld, const LoadEmitInfo& info, Temp offset, unsigned bytes_needed,
                    unsigned align_, unsigned const_offset, Temp dst_hint)
{
   Operand vaddr = offset.type() == RegType::vgpr ? Operand(offset) : Operand(v1);
   Operand soffset = offset.type() == RegType::sgpr ? Operand(offset) : Operand::c32(0);

   if (info.soffset.id()) {
      if (soffset.isTemp())
         vaddr = bld.copy(bld.def(v1), soffset);
      soffset = Operand(info.soffset);
   }

   if (soffset.isUndefined())
      soffset = Operand::zero();

   bool offen = !vaddr.isUndefined();
   bool idxen = info.idx.id();

   if (offen && idxen)
      vaddr = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), info.idx, vaddr);
   else if (idxen)
      vaddr = Operand(info.idx);

   unsigned bytes_size = 0;
   aco_opcode op;
   if (bytes_needed == 1 || align_ % 2) {
      bytes_size = 1;
      op = aco_opcode::buffer_load_ubyte;
   } else if (bytes_needed == 2 || align_ % 4) {
      bytes_size = 2;
      op = aco_opcode::buffer_load_ushort;
   } else if (bytes_needed <= 4) {
      bytes_size = 4;
      op = aco_opcode::buffer_load_dword;
   } else if (bytes_needed <= 8) {
      bytes_size = 8;
      op = aco_opcode::buffer_load_dwordx2;
   } else if (bytes_needed <= 12 && bld.program->gfx_level > GFX6) {
      bytes_size = 12;
      op = aco_opcode::buffer_load_dwordx3;
   } else {
      bytes_size = 16;
      op = aco_opcode::buffer_load_dwordx4;
   }
   aco_ptr<MUBUF_instruction> mubuf{create_instruction<MUBUF_instruction>(op, Format::MUBUF, 3, 1)};
   mubuf->operands[0] = Operand(info.resource);
   mubuf->operands[1] = vaddr;
   mubuf->operands[2] = soffset;
   mubuf->offen = offen;
   mubuf->idxen = idxen;
   mubuf->glc = info.glc;
   mubuf->dlc = info.glc && (bld.program->gfx_level == GFX10 || bld.program->gfx_level == GFX10_3);
   mubuf->slc = info.slc;
   mubuf->sync = info.sync;
   mubuf->offset = const_offset;
   mubuf->swizzled = info.swizzle_component_size != 0;
   RegClass rc = RegClass::get(RegType::vgpr, bytes_size);
   Temp val = dst_hint.id() && rc == dst_hint.regClass() ? dst_hint : bld.tmp(rc);
   mubuf->definitions[0] = Definition(val);
   bld.insert(std::move(mubuf));

   return val;
}

const EmitLoadParameters mubuf_load_params{mubuf_load_callback, true, true, 4096};

Temp
mubuf_load_format_callback(Builder& bld, const LoadEmitInfo& info, Temp offset,
                           unsigned bytes_needed, unsigned align_, unsigned const_offset,
                           Temp dst_hint)
{
   Operand vaddr = offset.type() == RegType::vgpr ? Operand(offset) : Operand(v1);
   Operand soffset = offset.type() == RegType::sgpr ? Operand(offset) : Operand::c32(0);

   if (info.soffset.id()) {
      if (soffset.isTemp())
         vaddr = bld.copy(bld.def(v1), soffset);
      soffset = Operand(info.soffset);
   }

   if (soffset.isUndefined())
      soffset = Operand::zero();

   bool offen = !vaddr.isUndefined();
   bool idxen = info.idx.id();

   if (offen && idxen)
      vaddr = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), info.idx, vaddr);
   else if (idxen)
      vaddr = Operand(info.idx);

   aco_opcode op = aco_opcode::num_opcodes;
   if (info.component_size == 2) {
      switch (bytes_needed) {
      case 2: op = aco_opcode::buffer_load_format_d16_x; break;
      case 4: op = aco_opcode::buffer_load_format_d16_xy; break;
      case 6: op = aco_opcode::buffer_load_format_d16_xyz; break;
      case 8: op = aco_opcode::buffer_load_format_d16_xyzw; break;
      default: unreachable("invalid buffer load format size"); break;
      }
   } else {
      assert(info.component_size == 4);
      switch (bytes_needed) {
      case 4: op = aco_opcode::buffer_load_format_x; break;
      case 8: op = aco_opcode::buffer_load_format_xy; break;
      case 12: op = aco_opcode::buffer_load_format_xyz; break;
      case 16: op = aco_opcode::buffer_load_format_xyzw; break;
      default: unreachable("invalid buffer load format size"); break;
      }
   }

   aco_ptr<MUBUF_instruction> mubuf{create_instruction<MUBUF_instruction>(op, Format::MUBUF, 3, 1)};
   mubuf->operands[0] = Operand(info.resource);
   mubuf->operands[1] = vaddr;
   mubuf->operands[2] = soffset;
   mubuf->offen = offen;
   mubuf->idxen = idxen;
   mubuf->glc = info.glc;
   mubuf->dlc = info.glc && (bld.program->gfx_level == GFX10 || bld.program->gfx_level == GFX10_3);
   mubuf->slc = info.slc;
   mubuf->sync = info.sync;
   mubuf->offset = const_offset;
   RegClass rc = RegClass::get(RegType::vgpr, bytes_needed);
   Temp val = dst_hint.id() && rc == dst_hint.regClass() ? dst_hint : bld.tmp(rc);
   mubuf->definitions[0] = Definition(val);
   bld.insert(std::move(mubuf));

   return val;
}

const EmitLoadParameters mubuf_load_format_params{mubuf_load_format_callback, false, true, 4096};

Temp
scratch_load_callback(Builder& bld, const LoadEmitInfo& info, Temp offset, unsigned bytes_needed,
                      unsigned align_, unsigned const_offset, Temp dst_hint)
{
   unsigned bytes_size = 0;
   aco_opcode op;
   if (bytes_needed == 1 || align_ % 2u) {
      bytes_size = 1;
      op = aco_opcode::scratch_load_ubyte;
   } else if (bytes_needed == 2 || align_ % 4u) {
      bytes_size = 2;
      op = aco_opcode::scratch_load_ushort;
   } else if (bytes_needed <= 4) {
      bytes_size = 4;
      op = aco_opcode::scratch_load_dword;
   } else if (bytes_needed <= 8) {
      bytes_size = 8;
      op = aco_opcode::scratch_load_dwordx2;
   } else if (bytes_needed <= 12) {
      bytes_size = 12;
      op = aco_opcode::scratch_load_dwordx3;
   } else {
      bytes_size = 16;
      op = aco_opcode::scratch_load_dwordx4;
   }
   RegClass rc = RegClass::get(RegType::vgpr, bytes_size);
   Temp val = dst_hint.id() && rc == dst_hint.regClass() ? dst_hint : bld.tmp(rc);
   aco_ptr<FLAT_instruction> flat{create_instruction<FLAT_instruction>(op, Format::SCRATCH, 2, 1)};
   flat->operands[0] = offset.regClass() == s1 ? Operand(v1) : Operand(offset);
   flat->operands[1] = offset.regClass() == s1 ? Operand(offset) : Operand(s1);
   flat->sync = info.sync;
   flat->offset = const_offset;
   flat->definitions[0] = Definition(val);
   bld.insert(std::move(flat));

   return val;
}

const EmitLoadParameters scratch_mubuf_load_params{mubuf_load_callback, false, true, 4096};
const EmitLoadParameters scratch_flat_load_params{scratch_load_callback, false, true, 2048};

Temp
get_gfx6_global_rsrc(Builder& bld, Temp addr)
{
   uint32_t rsrc_conf = S_008F0C_NUM_FORMAT(V_008F0C_BUF_NUM_FORMAT_FLOAT) |
                        S_008F0C_DATA_FORMAT(V_008F0C_BUF_DATA_FORMAT_32);

   if (addr.type() == RegType::vgpr)
      return bld.pseudo(aco_opcode::p_create_vector, bld.def(s4), Operand::zero(), Operand::zero(),
                        Operand::c32(-1u), Operand::c32(rsrc_conf));
   return bld.pseudo(aco_opcode::p_create_vector, bld.def(s4), addr, Operand::c32(-1u),
                     Operand::c32(rsrc_conf));
}

Temp
add64_32(Builder& bld, Temp src0, Temp src1)
{
   Temp src00 = bld.tmp(src0.type(), 1);
   Temp src01 = bld.tmp(src0.type(), 1);
   bld.pseudo(aco_opcode::p_split_vector, Definition(src00), Definition(src01), src0);

   if (src0.type() == RegType::vgpr || src1.type() == RegType::vgpr) {
      Temp dst0 = bld.tmp(v1);
      Temp carry = bld.vadd32(Definition(dst0), src00, src1, true).def(1).getTemp();
      Temp dst1 = bld.vadd32(bld.def(v1), src01, Operand::zero(), false, carry);
      return bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), dst0, dst1);
   } else {
      Temp carry = bld.tmp(s1);
      Temp dst0 =
         bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.scc(Definition(carry)), src00, src1);
      Temp dst1 = bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.def(s1, scc), src01, carry);
      return bld.pseudo(aco_opcode::p_create_vector, bld.def(s2), dst0, dst1);
   }
}

void
lower_global_address(Builder& bld, uint32_t offset_in, Temp* address_inout,
                     uint32_t* const_offset_inout, Temp* offset_inout)
{
   Temp address = *address_inout;
   uint64_t const_offset = *const_offset_inout + offset_in;
   Temp offset = *offset_inout;

   uint64_t max_const_offset_plus_one =
      1; /* GFX7/8/9: FLAT loads do not support constant offsets */
   if (bld.program->gfx_level >= GFX9)
      max_const_offset_plus_one = bld.program->dev.scratch_global_offset_max;
   else if (bld.program->gfx_level == GFX6)
      max_const_offset_plus_one = 4096; /* MUBUF has a 12-bit unsigned offset field */
   uint64_t excess_offset = const_offset - (const_offset % max_const_offset_plus_one);
   const_offset %= max_const_offset_plus_one;

   if (!offset.id()) {
      while (unlikely(excess_offset > UINT32_MAX)) {
         address = add64_32(bld, address, bld.copy(bld.def(s1), Operand::c32(UINT32_MAX)));
         excess_offset -= UINT32_MAX;
      }
      if (excess_offset)
         offset = bld.copy(bld.def(s1), Operand::c32(excess_offset));
   } else {
      /* If we add to "offset", we would transform the indended
       * "address + u2u64(offset) + u2u64(const_offset)" into
       * "address + u2u64(offset + const_offset)", so add to the address.
       * This could be more efficient if excess_offset>UINT32_MAX by doing a full 64-bit addition,
       * but that should be really rare.
       */
      while (excess_offset) {
         uint32_t src2 = MIN2(excess_offset, UINT32_MAX);
         address = add64_32(bld, address, bld.copy(bld.def(s1), Operand::c32(src2)));
         excess_offset -= src2;
      }
   }

   if (bld.program->gfx_level == GFX6) {
      /* GFX6 (MUBUF): (SGPR address, SGPR offset) or (VGPR address, SGPR offset) */
      if (offset.type() != RegType::sgpr) {
         address = add64_32(bld, address, offset);
         offset = Temp();
      }
      offset = offset.id() ? offset : bld.copy(bld.def(s1), Operand::zero());
   } else if (bld.program->gfx_level <= GFX8) {
      /* GFX7,8 (FLAT): VGPR address */
      if (offset.id()) {
         address = add64_32(bld, address, offset);
         offset = Temp();
      }
      address = as_vgpr(bld, address);
   } else {
      /* GFX9+ (GLOBAL): (VGPR address), or (SGPR address and VGPR offset) */
      if (address.type() == RegType::vgpr && offset.id()) {
         address = add64_32(bld, address, offset);
         offset = Temp();
      } else if (address.type() == RegType::sgpr && offset.id()) {
         offset = as_vgpr(bld, offset);
      }
      if (address.type() == RegType::sgpr && !offset.id())
         offset = bld.copy(bld.def(v1), bld.copy(bld.def(s1), Operand::zero()));
   }

   *address_inout = address;
   *const_offset_inout = const_offset;
   *offset_inout = offset;
}

Temp
global_load_callback(Builder& bld, const LoadEmitInfo& info, Temp offset, unsigned bytes_needed,
                     unsigned align_, unsigned const_offset, Temp dst_hint)
{
   Temp addr = info.resource;
   if (!addr.id()) {
      addr = offset;
      offset = Temp();
   }
   lower_global_address(bld, 0, &addr, &const_offset, &offset);

   unsigned bytes_size = 0;
   bool use_mubuf = bld.program->gfx_level == GFX6;
   bool global = bld.program->gfx_level >= GFX9;
   aco_opcode op;
   if (bytes_needed == 1 || align_ % 2u) {
      bytes_size = 1;
      op = use_mubuf ? aco_opcode::buffer_load_ubyte
           : global  ? aco_opcode::global_load_ubyte
                     : aco_opcode::flat_load_ubyte;
   } else if (bytes_needed == 2 || align_ % 4u) {
      bytes_size = 2;
      op = use_mubuf ? aco_opcode::buffer_load_ushort
           : global  ? aco_opcode::global_load_ushort
                     : aco_opcode::flat_load_ushort;
   } else if (bytes_needed <= 4) {
      bytes_size = 4;
      op = use_mubuf ? aco_opcode::buffer_load_dword
           : global  ? aco_opcode::global_load_dword
                     : aco_opcode::flat_load_dword;
   } else if (bytes_needed <= 8 || (bytes_needed <= 12 && use_mubuf)) {
      bytes_size = 8;
      op = use_mubuf ? aco_opcode::buffer_load_dwordx2
           : global  ? aco_opcode::global_load_dwordx2
                     : aco_opcode::flat_load_dwordx2;
   } else if (bytes_needed <= 12 && !use_mubuf) {
      bytes_size = 12;
      op = global ? aco_opcode::global_load_dwordx3 : aco_opcode::flat_load_dwordx3;
   } else {
      bytes_size = 16;
      op = use_mubuf ? aco_opcode::buffer_load_dwordx4
           : global  ? aco_opcode::global_load_dwordx4
                     : aco_opcode::flat_load_dwordx4;
   }
   RegClass rc = RegClass::get(RegType::vgpr, bytes_size);
   Temp val = dst_hint.id() && rc == dst_hint.regClass() ? dst_hint : bld.tmp(rc);
   if (use_mubuf) {
      aco_ptr<MUBUF_instruction> mubuf{
         create_instruction<MUBUF_instruction>(op, Format::MUBUF, 3, 1)};
      mubuf->operands[0] = Operand(get_gfx6_global_rsrc(bld, addr));
      mubuf->operands[1] = addr.type() == RegType::vgpr ? Operand(addr) : Operand(v1);
      mubuf->operands[2] = Operand(offset);
      mubuf->glc = info.glc;
      mubuf->dlc = false;
      mubuf->offset = const_offset;
      mubuf->addr64 = addr.type() == RegType::vgpr;
      mubuf->disable_wqm = false;
      mubuf->sync = info.sync;
      mubuf->definitions[0] = Definition(val);
      bld.insert(std::move(mubuf));
   } else {
      aco_ptr<FLAT_instruction> flat{
         create_instruction<FLAT_instruction>(op, global ? Format::GLOBAL : Format::FLAT, 2, 1)};
      if (addr.regClass() == s2) {
         assert(global && offset.id() && offset.type() == RegType::vgpr);
         flat->operands[0] = Operand(offset);
         flat->operands[1] = Operand(addr);
      } else {
         assert(addr.type() == RegType::vgpr && !offset.id());
         flat->operands[0] = Operand(addr);
         flat->operands[1] = Operand(s1);
      }
      flat->glc = info.glc;
      flat->dlc =
         info.glc && (bld.program->gfx_level == GFX10 || bld.program->gfx_level == GFX10_3);
      flat->sync = info.sync;
      assert(global || !const_offset);
      flat->offset = const_offset;
      flat->definitions[0] = Definition(val);
      bld.insert(std::move(flat));
   }

   return val;
}

const EmitLoadParameters global_load_params{global_load_callback, true, true, UINT32_MAX};

Temp
load_lds(isel_context* ctx, unsigned elem_size_bytes, unsigned num_components, Temp dst,
         Temp address, unsigned base_offset, unsigned align)
{
   assert(util_is_power_of_two_nonzero(align));

   Builder bld(ctx->program, ctx->block);

   LoadEmitInfo info = {Operand(as_vgpr(ctx, address)), dst, num_components, elem_size_bytes};
   info.align_mul = align;
   info.align_offset = 0;
   info.sync = memory_sync_info(storage_shared);
   info.const_offset = base_offset;
   /* The 2 separate loads for gfx10+ wave64 can see different values, even for uniform addresses,
    * if another wave writes LDS in between. Use v_readfirstlane instead of p_as_uniform in order
    * to avoid copy-propagation.
    */
   info.readfirstlane_for_uniform = ctx->options->gfx_level >= GFX10 &&
                                    ctx->program->wave_size == 64 &&
                                    ctx->program->workgroup_size > 64;
   emit_load(ctx, bld, info, lds_load_params);

   return dst;
}

void
split_store_data(isel_context* ctx, RegType dst_type, unsigned count, Temp* dst, unsigned* bytes,
                 Temp src)
{
   if (!count)
      return;

   Builder bld(ctx->program, ctx->block);

   /* count == 1 fast path */
   if (count == 1) {
      if (dst_type == RegType::sgpr)
         dst[0] = bld.as_uniform(src);
      else
         dst[0] = as_vgpr(ctx, src);
      return;
   }

   /* elem_size_bytes is the greatest common divisor which is a power of 2 */
   unsigned elem_size_bytes =
      1u << (ffs(std::accumulate(bytes, bytes + count, 8, std::bit_or<>{})) - 1);

   ASSERTED bool is_subdword = elem_size_bytes < 4;
   assert(!is_subdword || dst_type == RegType::vgpr);

   for (unsigned i = 0; i < count; i++)
      dst[i] = bld.tmp(RegClass::get(dst_type, bytes[i]));

   std::vector<Temp> temps;
   /* use allocated_vec if possible */
   auto it = ctx->allocated_vec.find(src.id());
   if (it != ctx->allocated_vec.end()) {
      if (!it->second[0].id())
         goto split;
      unsigned elem_size = it->second[0].bytes();
      assert(src.bytes() % elem_size == 0);

      for (unsigned i = 0; i < src.bytes() / elem_size; i++) {
         if (!it->second[i].id())
            goto split;
      }
      if (elem_size_bytes % elem_size)
         goto split;

      temps.insert(temps.end(), it->second.begin(), it->second.begin() + src.bytes() / elem_size);
      elem_size_bytes = elem_size;
   }

split:
   /* split src if necessary */
   if (temps.empty()) {
      if (is_subdword && src.type() == RegType::sgpr)
         src = as_vgpr(ctx, src);
      if (dst_type == RegType::sgpr)
         src = bld.as_uniform(src);

      unsigned num_elems = src.bytes() / elem_size_bytes;
      aco_ptr<Instruction> split{create_instruction<Pseudo_instruction>(
         aco_opcode::p_split_vector, Format::PSEUDO, 1, num_elems)};
      split->operands[0] = Operand(src);
      for (unsigned i = 0; i < num_elems; i++) {
         temps.emplace_back(bld.tmp(RegClass::get(dst_type, elem_size_bytes)));
         split->definitions[i] = Definition(temps.back());
      }
      bld.insert(std::move(split));
   }

   unsigned idx = 0;
   for (unsigned i = 0; i < count; i++) {
      unsigned op_count = dst[i].bytes() / elem_size_bytes;
      if (op_count == 1) {
         if (dst_type == RegType::sgpr)
            dst[i] = bld.as_uniform(temps[idx++]);
         else
            dst[i] = as_vgpr(ctx, temps[idx++]);
         continue;
      }

      aco_ptr<Instruction> vec{create_instruction<Pseudo_instruction>(aco_opcode::p_create_vector,
                                                                      Format::PSEUDO, op_count, 1)};
      for (unsigned j = 0; j < op_count; j++) {
         Temp tmp = temps[idx++];
         if (dst_type == RegType::sgpr)
            tmp = bld.as_uniform(tmp);
         vec->operands[j] = Operand(tmp);
      }
      vec->definitions[0] = Definition(dst[i]);
      bld.insert(std::move(vec));
   }
   return;
}

bool
scan_write_mask(uint32_t mask, uint32_t todo_mask, int* start, int* count)
{
   unsigned start_elem = ffs(todo_mask) - 1;
   bool skip = !(mask & (1 << start_elem));
   if (skip)
      mask = ~mask & todo_mask;

   mask &= todo_mask;

   u_bit_scan_consecutive_range(&mask, start, count);

   return !skip;
}

void
advance_write_mask(uint32_t* todo_mask, int start, int count)
{
   *todo_mask &= ~u_bit_consecutive(0, count) << start;
}

void
store_lds(isel_context* ctx, unsigned elem_size_bytes, Temp data, uint32_t wrmask, Temp address,
          unsigned base_offset, unsigned align)
{
   assert(util_is_power_of_two_nonzero(align));
   assert(util_is_power_of_two_nonzero(elem_size_bytes) && elem_size_bytes <= 8);

   Builder bld(ctx->program, ctx->block);
   bool large_ds_write = ctx->options->gfx_level >= GFX7;
   bool usable_write2 = ctx->options->gfx_level >= GFX7;

   unsigned write_count = 0;
   Temp write_datas[32];
   unsigned offsets[32];
   unsigned bytes[32];
   aco_opcode opcodes[32];

   wrmask = util_widen_mask(wrmask, elem_size_bytes);

   const unsigned wrmask_bitcnt = util_bitcount(wrmask);
   uint32_t todo = u_bit_consecutive(0, data.bytes());

   if (u_bit_consecutive(0, wrmask_bitcnt) == wrmask)
      todo = MIN2(todo, wrmask);

   while (todo) {
      int offset, byte;
      if (!scan_write_mask(wrmask, todo, &offset, &byte)) {
         offsets[write_count] = offset;
         bytes[write_count] = byte;
         opcodes[write_count] = aco_opcode::num_opcodes;
         write_count++;
         advance_write_mask(&todo, offset, byte);
         continue;
      }

      bool aligned2 = offset % 2 == 0 && align % 2 == 0;
      bool aligned4 = offset % 4 == 0 && align % 4 == 0;
      bool aligned8 = offset % 8 == 0 && align % 8 == 0;
      bool aligned16 = offset % 16 == 0 && align % 16 == 0;

      // TODO: use ds_write_b8_d16_hi/ds_write_b16_d16_hi if beneficial
      aco_opcode op = aco_opcode::num_opcodes;
      if (byte >= 16 && aligned16 && large_ds_write) {
         op = aco_opcode::ds_write_b128;
         byte = 16;
      } else if (byte >= 12 && aligned16 && large_ds_write) {
         op = aco_opcode::ds_write_b96;
         byte = 12;
      } else if (byte >= 8 && aligned8) {
         op = aco_opcode::ds_write_b64;
         byte = 8;
      } else if (byte >= 4 && aligned4) {
         op = aco_opcode::ds_write_b32;
         byte = 4;
      } else if (byte >= 2 && aligned2) {
         op = aco_opcode::ds_write_b16;
         byte = 2;
      } else if (byte >= 1) {
         op = aco_opcode::ds_write_b8;
         byte = 1;
      } else {
         assert(false);
      }

      offsets[write_count] = offset;
      bytes[write_count] = byte;
      opcodes[write_count] = op;
      write_count++;
      advance_write_mask(&todo, offset, byte);
   }

   Operand m = load_lds_size_m0(bld);

   split_store_data(ctx, RegType::vgpr, write_count, write_datas, bytes, data);

   for (unsigned i = 0; i < write_count; i++) {
      aco_opcode op = opcodes[i];
      if (op == aco_opcode::num_opcodes)
         continue;

      Temp split_data = write_datas[i];

      unsigned second = write_count;
      if (usable_write2 && (op == aco_opcode::ds_write_b32 || op == aco_opcode::ds_write_b64)) {
         for (second = i + 1; second < write_count; second++) {
            if (opcodes[second] == op && (offsets[second] - offsets[i]) % split_data.bytes() == 0) {
               op = split_data.bytes() == 4 ? aco_opcode::ds_write2_b32 : aco_opcode::ds_write2_b64;
               opcodes[second] = aco_opcode::num_opcodes;
               break;
            }
         }
      }

      bool write2 = op == aco_opcode::ds_write2_b32 || op == aco_opcode::ds_write2_b64;
      unsigned write2_off = (offsets[second] - offsets[i]) / split_data.bytes();

      unsigned inline_offset = base_offset + offsets[i];
      unsigned max_offset = write2 ? (255 - write2_off) * split_data.bytes() : 65535;
      Temp address_offset = address;
      if (inline_offset > max_offset) {
         address_offset = bld.vadd32(bld.def(v1), Operand::c32(base_offset), address_offset);
         inline_offset = offsets[i];
      }

      /* offsets[i] shouldn't be large enough for this to happen */
      assert(inline_offset <= max_offset);

      Instruction* instr;
      if (write2) {
         Temp second_data = write_datas[second];
         inline_offset /= split_data.bytes();
         instr = bld.ds(op, address_offset, split_data, second_data, m, inline_offset,
                        inline_offset + write2_off);
      } else {
         instr = bld.ds(op, address_offset, split_data, m, inline_offset);
      }
      instr->ds().sync = memory_sync_info(storage_shared);

      if (m.isUndefined())
         instr->operands.pop_back();
   }
}

aco_opcode
get_buffer_store_op(unsigned bytes)
{
   switch (bytes) {
   case 1: return aco_opcode::buffer_store_byte;
   case 2: return aco_opcode::buffer_store_short;
   case 4: return aco_opcode::buffer_store_dword;
   case 8: return aco_opcode::buffer_store_dwordx2;
   case 12: return aco_opcode::buffer_store_dwordx3;
   case 16: return aco_opcode::buffer_store_dwordx4;
   }
   unreachable("Unexpected store size");
   return aco_opcode::num_opcodes;
}

void
split_buffer_store(isel_context* ctx, nir_intrinsic_instr* instr, bool smem, RegType dst_type,
                   Temp data, unsigned writemask, int swizzle_element_size, unsigned* write_count,
                   Temp* write_datas, unsigned* offsets)
{
   unsigned write_count_with_skips = 0;
   bool skips[16];
   unsigned bytes[16];

   /* determine how to split the data */
   unsigned todo = u_bit_consecutive(0, data.bytes());
   while (todo) {
      int offset, byte;
      skips[write_count_with_skips] = !scan_write_mask(writemask, todo, &offset, &byte);
      offsets[write_count_with_skips] = offset;
      if (skips[write_count_with_skips]) {
         bytes[write_count_with_skips] = byte;
         advance_write_mask(&todo, offset, byte);
         write_count_with_skips++;
         continue;
      }

      /* only supported sizes are 1, 2, 4, 8, 12 and 16 bytes and can't be
       * larger than swizzle_element_size */
      byte = MIN2(byte, swizzle_element_size);
      if (byte % 4)
         byte = byte > 4 ? byte & ~0x3 : MIN2(byte, 2);

      /* SMEM and GFX6 VMEM can't emit 12-byte stores */
      if ((ctx->program->gfx_level == GFX6 || smem) && byte == 12)
         byte = 8;

      /* dword or larger stores have to be dword-aligned */
      unsigned align_mul = instr ? nir_intrinsic_align_mul(instr) : 4;
      unsigned align_offset = (instr ? nir_intrinsic_align_offset(instr) : 0) + offset;
      bool dword_aligned = align_offset % 4 == 0 && align_mul % 4 == 0;
      if (!dword_aligned)
         byte = MIN2(byte, (align_offset % 2 == 0 && align_mul % 2 == 0) ? 2 : 1);

      bytes[write_count_with_skips] = byte;
      advance_write_mask(&todo, offset, byte);
      write_count_with_skips++;
   }

   /* actually split data */
   split_store_data(ctx, dst_type, write_count_with_skips, write_datas, bytes, data);

   /* remove skips */
   for (unsigned i = 0; i < write_count_with_skips; i++) {
      if (skips[i])
         continue;
      write_datas[*write_count] = write_datas[i];
      offsets[*write_count] = offsets[i];
      (*write_count)++;
   }
}

Temp
create_vec_from_array(isel_context* ctx, Temp arr[], unsigned cnt, RegType reg_type,
                      unsigned elem_size_bytes, unsigned split_cnt = 0u, Temp dst = Temp())
{
   Builder bld(ctx->program, ctx->block);
   unsigned dword_size = elem_size_bytes / 4;

   if (!dst.id())
      dst = bld.tmp(RegClass(reg_type, cnt * dword_size));

   std::array<Temp, NIR_MAX_VEC_COMPONENTS> allocated_vec;
   aco_ptr<Pseudo_instruction> instr{
      create_instruction<Pseudo_instruction>(aco_opcode::p_create_vector, Format::PSEUDO, cnt, 1)};
   instr->definitions[0] = Definition(dst);

   for (unsigned i = 0; i < cnt; ++i) {
      if (arr[i].id()) {
         assert(arr[i].size() == dword_size);
         allocated_vec[i] = arr[i];
         instr->operands[i] = Operand(arr[i]);
      } else {
         Temp zero = bld.copy(bld.def(RegClass(reg_type, dword_size)),
                              Operand::zero(dword_size == 2 ? 8 : 4));
         allocated_vec[i] = zero;
         instr->operands[i] = Operand(zero);
      }
   }

   bld.insert(std::move(instr));

   if (split_cnt)
      emit_split_vector(ctx, dst, split_cnt);
   else
      ctx->allocated_vec.emplace(dst.id(), allocated_vec); /* emit_split_vector already does this */

   return dst;
}

inline unsigned
resolve_excess_vmem_const_offset(Builder& bld, Temp& voffset, unsigned const_offset)
{
   if (const_offset >= 4096) {
      unsigned excess_const_offset = const_offset / 4096u * 4096u;
      const_offset %= 4096u;

      if (!voffset.id())
         voffset = bld.copy(bld.def(v1), Operand::c32(excess_const_offset));
      else if (unlikely(voffset.regClass() == s1))
         voffset = bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.def(s1, scc),
                            Operand::c32(excess_const_offset), Operand(voffset));
      else if (likely(voffset.regClass() == v1))
         voffset = bld.vadd32(bld.def(v1), Operand(voffset), Operand::c32(excess_const_offset));
      else
         unreachable("Unsupported register class of voffset");
   }

   return const_offset;
}

void
emit_single_mubuf_store(isel_context* ctx, Temp descriptor, Temp voffset, Temp soffset, Temp idx,
                        Temp vdata, unsigned const_offset, memory_sync_info sync, bool glc,
                        bool slc, bool swizzled)
{
   assert(vdata.id());
   assert(vdata.size() != 3 || ctx->program->gfx_level != GFX6);
   assert(vdata.size() >= 1 && vdata.size() <= 4);

   Builder bld(ctx->program, ctx->block);
   aco_opcode op = get_buffer_store_op(vdata.bytes());
   const_offset = resolve_excess_vmem_const_offset(bld, voffset, const_offset);

   bool offen = voffset.id();
   bool idxen = idx.id();

   Operand soffset_op = soffset.id() ? Operand(soffset) : Operand::zero();
   glc &= ctx->program->gfx_level < GFX11;

   Operand vaddr_op(v1);
   if (offen && idxen)
      vaddr_op = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), idx, voffset);
   else if (offen)
      vaddr_op = Operand(voffset);
   else if (idxen)
      vaddr_op = Operand(idx);

   Builder::Result r =
      bld.mubuf(op, Operand(descriptor), vaddr_op, soffset_op, Operand(vdata), const_offset, offen,
                swizzled, idxen, /* addr64 */ false, /* disable_wqm */ false, glc,
                /* dlc*/ false, slc);

   r->mubuf().sync = sync;
}

void
store_vmem_mubuf(isel_context* ctx, Temp src, Temp descriptor, Temp voffset, Temp soffset, Temp idx,
                 unsigned base_const_offset, unsigned elem_size_bytes, unsigned write_mask,
                 bool swizzled, memory_sync_info sync, bool glc, bool slc)
{
   Builder bld(ctx->program, ctx->block);
   assert(elem_size_bytes == 1 || elem_size_bytes == 2 || elem_size_bytes == 4 ||
          elem_size_bytes == 8);
   assert(write_mask);
   write_mask = util_widen_mask(write_mask, elem_size_bytes);

   unsigned write_count = 0;
   Temp write_datas[32];
   unsigned offsets[32];
   split_buffer_store(ctx, NULL, false, RegType::vgpr, src, write_mask,
                      swizzled && ctx->program->gfx_level <= GFX8 ? 4 : 16, &write_count,
                      write_datas, offsets);

   for (unsigned i = 0; i < write_count; i++) {
      unsigned const_offset = offsets[i] + base_const_offset;
      emit_single_mubuf_store(ctx, descriptor, voffset, soffset, idx, write_datas[i], const_offset,
                              sync, glc, slc, swizzled);
   }
}

Temp
wave_id_in_threadgroup(isel_context* ctx)
{
   Builder bld(ctx->program, ctx->block);
   return bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc),
                   get_arg(ctx, ctx->args->merged_wave_info), Operand::c32(24u | (4u << 16)));
}

Temp
thread_id_in_threadgroup(isel_context* ctx)
{
   /* tid_in_tg = wave_id * wave_size + tid_in_wave */

   Builder bld(ctx->program, ctx->block);
   Temp tid_in_wave = emit_mbcnt(ctx, bld.tmp(v1));

   if (ctx->program->workgroup_size <= ctx->program->wave_size)
      return tid_in_wave;

   Temp wave_id_in_tg = wave_id_in_threadgroup(ctx);
   Temp num_pre_threads =
      bld.sop2(aco_opcode::s_lshl_b32, bld.def(s1), bld.def(s1, scc), wave_id_in_tg,
               Operand::c32(ctx->program->wave_size == 64 ? 6u : 5u));
   return bld.vadd32(bld.def(v1), Operand(num_pre_threads), Operand(tid_in_wave));
}

bool
store_output_to_temps(isel_context* ctx, nir_intrinsic_instr* instr)
{
   unsigned write_mask = nir_intrinsic_write_mask(instr);
   unsigned component = nir_intrinsic_component(instr);
   nir_src offset = *nir_get_io_offset_src(instr);

   if (!nir_src_is_const(offset) || nir_src_as_uint(offset))
      return false;

   Temp src = get_ssa_temp(ctx, instr->src[0].ssa);

   if (instr->src[0].ssa->bit_size == 64)
      write_mask = util_widen_mask(write_mask, 2);

   RegClass rc = instr->src[0].ssa->bit_size == 16 ? v2b : v1;

   /* Use semantic location as index. radv already uses it as intrinsic base
    * but radeonsi does not. We need to make LS output and TCS input index
    * match each other, so need to use semantic location explicitly. Also for
    * TCS epilog to index tess factor temps using semantic location directly.
    */
   nir_io_semantics sem = nir_intrinsic_io_semantics(instr);
   unsigned base = sem.location;
   if (ctx->stage == fragment_fs) {
      /* color result is a legacy slot which won't appear with data result
       * at the same time. Here we just use the data slot for it to simplify
       * code handling for both of them.
       */
      if (base == FRAG_RESULT_COLOR)
         base = FRAG_RESULT_DATA0;

      /* Sencond output of dual source blend just use data1 slot for simplicity,
       * because dual source blend does not support multi render target.
       */
      base += sem.dual_source_blend_index;
   }
   unsigned idx = base * 4u + component;

   for (unsigned i = 0; i < 8; ++i) {
      if (write_mask & (1 << i)) {
         ctx->outputs.mask[idx / 4u] |= 1 << (idx % 4u);
         ctx->outputs.temps[idx] = emit_extract_vector(ctx, src, i, rc);
      }
      idx++;
   }

   if (ctx->stage == fragment_fs && ctx->program->info.has_epilog && base >= FRAG_RESULT_DATA0) {
      unsigned index = base - FRAG_RESULT_DATA0;

      if (nir_intrinsic_src_type(instr) == nir_type_float16) {
         ctx->output_color_types |= ACO_TYPE_FLOAT16 << (index * 2);
      } else if (nir_intrinsic_src_type(instr) == nir_type_int16) {
         ctx->output_color_types |= ACO_TYPE_INT16 << (index * 2);
      } else if (nir_intrinsic_src_type(instr) == nir_type_uint16) {
         ctx->output_color_types |= ACO_TYPE_UINT16 << (index * 2);
      }
   }

   return true;
}

bool
load_input_from_temps(isel_context* ctx, nir_intrinsic_instr* instr, Temp dst)
{
   /* Only TCS per-vertex inputs are supported by this function.
    * Per-vertex inputs only match between the VS/TCS invocation id when the number of invocations
    * is the same.
    */
   if (ctx->shader->info.stage != MESA_SHADER_TESS_CTRL || !ctx->tcs_in_out_eq)
      return false;

   nir_src* off_src = nir_get_io_offset_src(instr);
   nir_src* vertex_index_src = nir_get_io_arrayed_index_src(instr);
   nir_instr* vertex_index_instr = vertex_index_src->ssa->parent_instr;
   bool can_use_temps =
      nir_src_is_const(*off_src) && vertex_index_instr->type == nir_instr_type_intrinsic &&
      nir_instr_as_intrinsic(vertex_index_instr)->intrinsic == nir_intrinsic_load_invocation_id;

   if (!can_use_temps)
      return false;

   nir_io_semantics sem = nir_intrinsic_io_semantics(instr);

   unsigned idx =
      sem.location * 4u + nir_intrinsic_component(instr) + 4 * nir_src_as_uint(*off_src);
   Temp* src = &ctx->inputs.temps[idx];
   create_vec_from_array(ctx, src, dst.size(), dst.regClass().type(), 4u, 0, dst);

   return true;
}

void
visit_store_output(isel_context* ctx, nir_intrinsic_instr* instr)
{
   /* LS pass output to TCS by temp if they have same in/out patch size. */
   bool ls_need_output = ctx->stage == vertex_tess_control_hs &&
                         ctx->shader->info.stage == MESA_SHADER_VERTEX && ctx->tcs_in_out_eq;

   bool tcs_need_output = ctx->shader->info.stage == MESA_SHADER_TESS_CTRL &&
                          ctx->program->info.has_epilog &&
                          ctx->program->info.tcs.pass_tessfactors_by_reg;

   bool ps_need_output = ctx->stage == fragment_fs;

   if (ls_need_output || tcs_need_output || ps_need_output) {
      bool stored_to_temps = store_output_to_temps(ctx, instr);
      if (!stored_to_temps) {
         isel_err(instr->src[1].ssa->parent_instr, "Unimplemented output offset instruction");
         abort();
      }
   } else {
      unreachable("Shader stage not implemented");
   }
}

bool
in_exec_divergent_or_in_loop(isel_context* ctx)
{
   return ctx->block->loop_nest_depth || ctx->cf_info.parent_if.is_divergent ||
          ctx->cf_info.had_divergent_discard;
}

void
emit_interp_instr_gfx11(isel_context* ctx, unsigned idx, unsigned component, Temp src, Temp dst,
                        Temp prim_mask)
{
   Temp coord1 = emit_extract_vector(ctx, src, 0, v1);
   Temp coord2 = emit_extract_vector(ctx, src, 1, v1);

   Builder bld(ctx->program, ctx->block);

   if (in_exec_divergent_or_in_loop(ctx)) {
      Operand prim_mask_op = bld.m0(prim_mask);
      prim_mask_op.setLateKill(true); /* we don't want the bld.lm definition to use m0 */
      Operand coord2_op(coord2);
      coord2_op.setLateKill(true); /* we re-use the destination reg in the middle */
      bld.pseudo(aco_opcode::p_interp_gfx11, Definition(dst), Operand(v1.as_linear()),
                 Operand::c32(idx), Operand::c32(component), coord1, coord2_op, prim_mask_op);
      return;
   }

   Temp p = bld.ldsdir(aco_opcode::lds_param_load, bld.def(v1), bld.m0(prim_mask), idx, component);

   Temp res;
   if (dst.regClass() == v2b) {
      Temp p10 =
         bld.vinterp_inreg(aco_opcode::v_interp_p10_f16_f32_inreg, bld.def(v1), p, coord1, p);
      res = bld.vinterp_inreg(aco_opcode::v_interp_p2_f16_f32_inreg, bld.def(v1), p, coord2, p10);
      emit_extract_vector(ctx, res, 0, dst);
   } else {
      Temp p10 = bld.vinterp_inreg(aco_opcode::v_interp_p10_f32_inreg, bld.def(v1), p, coord1, p);
      bld.vinterp_inreg(aco_opcode::v_interp_p2_f32_inreg, Definition(dst), p, coord2, p10);
   }
   /* lds_param_load must be done in WQM, and the result kept valid for helper lanes. */
   set_wqm(ctx, true);
}

void
emit_interp_instr(isel_context* ctx, unsigned idx, unsigned component, Temp src, Temp dst,
                  Temp prim_mask)
{
   if (ctx->options->gfx_level >= GFX11) {
      emit_interp_instr_gfx11(ctx, idx, component, src, dst, prim_mask);
      return;
   }

   Temp coord1 = emit_extract_vector(ctx, src, 0, v1);
   Temp coord2 = emit_extract_vector(ctx, src, 1, v1);

   Builder bld(ctx->program, ctx->block);

   if (dst.regClass() == v2b) {
      if (ctx->program->dev.has_16bank_lds) {
         assert(ctx->options->gfx_level <= GFX8);
         Builder::Result interp_p1 =
            bld.vintrp(aco_opcode::v_interp_mov_f32, bld.def(v1), Operand::c32(2u) /* P0 */,
                       bld.m0(prim_mask), idx, component);
         interp_p1 = bld.vintrp(aco_opcode::v_interp_p1lv_f16, bld.def(v2b), coord1,
                                bld.m0(prim_mask), interp_p1, idx, component);
         bld.vintrp(aco_opcode::v_interp_p2_legacy_f16, Definition(dst), coord2, bld.m0(prim_mask),
                    interp_p1, idx, component);
      } else {
         aco_opcode interp_p2_op = aco_opcode::v_interp_p2_f16;

         if (ctx->options->gfx_level == GFX8)
            interp_p2_op = aco_opcode::v_interp_p2_legacy_f16;

         Builder::Result interp_p1 = bld.vintrp(aco_opcode::v_interp_p1ll_f16, bld.def(v1), coord1,
                                                bld.m0(prim_mask), idx, component);
         bld.vintrp(interp_p2_op, Definition(dst), coord2, bld.m0(prim_mask), interp_p1, idx,
                    component);
      }
   } else {
      Builder::Result interp_p1 = bld.vintrp(aco_opcode::v_interp_p1_f32, bld.def(v1), coord1,
                                             bld.m0(prim_mask), idx, component);

      if (ctx->program->dev.has_16bank_lds)
         interp_p1->operands[0].setLateKill(true);

      bld.vintrp(aco_opcode::v_interp_p2_f32, Definition(dst), coord2, bld.m0(prim_mask), interp_p1,
                 idx, component);
   }
}

void
emit_interp_mov_instr(isel_context* ctx, unsigned idx, unsigned component, unsigned vertex_id,
                      Temp dst, Temp prim_mask)
{
   Builder bld(ctx->program, ctx->block);
   if (ctx->options->gfx_level >= GFX11) {
      uint16_t dpp_ctrl = dpp_quad_perm(vertex_id, vertex_id, vertex_id, vertex_id);
      if (in_exec_divergent_or_in_loop(ctx)) {
         Operand prim_mask_op = bld.m0(prim_mask);
         prim_mask_op.setLateKill(true); /* we don't want the bld.lm definition to use m0 */
         bld.pseudo(aco_opcode::p_interp_gfx11, Definition(dst), Operand(v1.as_linear()),
                    Operand::c32(idx), Operand::c32(component), Operand::c32(dpp_ctrl),
                    prim_mask_op);
      } else {
         Temp p =
            bld.ldsdir(aco_opcode::lds_param_load, bld.def(v1), bld.m0(prim_mask), idx, component);
         if (dst.regClass() == v2b) {
            Temp res = bld.vop1_dpp(aco_opcode::v_mov_b32, bld.def(v1), p, dpp_ctrl);
            emit_extract_vector(ctx, res, 0, dst);
         } else {
            bld.vop1_dpp(aco_opcode::v_mov_b32, Definition(dst), p, dpp_ctrl);
         }
         /* lds_param_load must be done in WQM, and the result kept valid for helper lanes. */
         set_wqm(ctx, true);
      }
   } else {
      bld.vintrp(aco_opcode::v_interp_mov_f32, Definition(dst), Operand::c32((vertex_id + 2) % 3),
                 bld.m0(prim_mask), idx, component);
   }
}

void
emit_load_frag_coord(isel_context* ctx, Temp dst, unsigned num_components)
{
   Builder bld(ctx->program, ctx->block);

   aco_ptr<Pseudo_instruction> vec(create_instruction<Pseudo_instruction>(
      aco_opcode::p_create_vector, Format::PSEUDO, num_components, 1));
   for (unsigned i = 0; i < num_components; i++) {
      if (ctx->args->frag_pos[i].used)
         vec->operands[i] = Operand(get_arg(ctx, ctx->args->frag_pos[i]));
      else
         vec->operands[i] = Operand(v1);
   }
   if (G_0286CC_POS_W_FLOAT_ENA(ctx->program->config->spi_ps_input_ena)) {
      assert(num_components == 4);
      vec->operands[3] =
         bld.vop1(aco_opcode::v_rcp_f32, bld.def(v1), get_arg(ctx, ctx->args->frag_pos[3]));
   }

   for (Operand& op : vec->operands)
      op = op.isUndefined() ? Operand::zero() : op;

   vec->definitions[0] = Definition(dst);
   ctx->block->instructions.emplace_back(std::move(vec));
   emit_split_vector(ctx, dst, num_components);
   return;
}

void
emit_load_frag_shading_rate(isel_context* ctx, Temp dst)
{
   Builder bld(ctx->program, ctx->block);
   Temp cond;

   /* VRS Rate X = Ancillary[2:3]
    * VRS Rate Y = Ancillary[4:5]
    */
   Temp x_rate = bld.vop3(aco_opcode::v_bfe_u32, bld.def(v1), get_arg(ctx, ctx->args->ancillary),
                          Operand::c32(2u), Operand::c32(2u));
   Temp y_rate = bld.vop3(aco_opcode::v_bfe_u32, bld.def(v1), get_arg(ctx, ctx->args->ancillary),
                          Operand::c32(4u), Operand::c32(2u));

   /* xRate = xRate == 0x1 ? Horizontal2Pixels : None. */
   cond = bld.vopc(aco_opcode::v_cmp_eq_i32, bld.def(bld.lm), Operand::c32(1u), Operand(x_rate));
   x_rate = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), bld.copy(bld.def(v1), Operand::zero()),
                     bld.copy(bld.def(v1), Operand::c32(4u)), cond);

   /* yRate = yRate == 0x1 ? Vertical2Pixels : None. */
   cond = bld.vopc(aco_opcode::v_cmp_eq_i32, bld.def(bld.lm), Operand::c32(1u), Operand(y_rate));
   y_rate = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), bld.copy(bld.def(v1), Operand::zero()),
                     bld.copy(bld.def(v1), Operand::c32(1u)), cond);

   bld.vop2(aco_opcode::v_or_b32, Definition(dst), Operand(x_rate), Operand(y_rate));
}

void
visit_load_interpolated_input(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Temp dst = get_ssa_temp(ctx, &instr->def);
   Temp coords = get_ssa_temp(ctx, instr->src[0].ssa);
   unsigned idx = nir_intrinsic_base(instr);
   unsigned component = nir_intrinsic_component(instr);
   Temp prim_mask = get_arg(ctx, ctx->args->prim_mask);

   assert(nir_src_is_const(instr->src[1]) && !nir_src_as_uint(instr->src[1]));

   if (instr->def.num_components == 1) {
      emit_interp_instr(ctx, idx, component, coords, dst, prim_mask);
   } else {
      aco_ptr<Pseudo_instruction> vec(create_instruction<Pseudo_instruction>(
         aco_opcode::p_create_vector, Format::PSEUDO, instr->def.num_components, 1));
      for (unsigned i = 0; i < instr->def.num_components; i++) {
         Temp tmp = ctx->program->allocateTmp(instr->def.bit_size == 16 ? v2b : v1);
         emit_interp_instr(ctx, idx, component + i, coords, tmp, prim_mask);
         vec->operands[i] = Operand(tmp);
      }
      vec->definitions[0] = Definition(dst);
      ctx->block->instructions.emplace_back(std::move(vec));
   }
}

Temp
mtbuf_load_callback(Builder& bld, const LoadEmitInfo& info, Temp offset, unsigned bytes_needed,
                    unsigned alignment, unsigned const_offset, Temp dst_hint)
{
   Operand vaddr = offset.type() == RegType::vgpr ? Operand(offset) : Operand(v1);
   Operand soffset = offset.type() == RegType::sgpr ? Operand(offset) : Operand::c32(0);

   if (info.soffset.id()) {
      if (soffset.isTemp())
         vaddr = bld.copy(bld.def(v1), soffset);
      soffset = Operand(info.soffset);
   }

   if (soffset.isUndefined())
      soffset = Operand::zero();

   const bool offen = !vaddr.isUndefined();
   const bool idxen = info.idx.id();

   if (offen && idxen)
      vaddr = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), info.idx, vaddr);
   else if (idxen)
      vaddr = Operand(info.idx);

   /* Determine number of fetched components.
    * Note, ACO IR works with GFX6-8 nfmt + dfmt fields, these are later converted for GFX10+.
    */
   const struct ac_vtx_format_info* vtx_info =
      ac_get_vtx_format_info(GFX8, CHIP_POLARIS10, info.format);
   /* The number of channels in the format determines the memory range. */
   const unsigned max_components = vtx_info->num_channels;
   /* Calculate maximum number of components loaded according to alignment. */
   unsigned max_fetched_components = bytes_needed / info.component_size;
   max_fetched_components =
      ac_get_safe_fetch_size(bld.program->gfx_level, vtx_info, const_offset, max_components,
                             alignment, max_fetched_components);
   const unsigned fetch_fmt = vtx_info->hw_format[max_fetched_components - 1];
   /* Adjust bytes needed in case we need to do a smaller load due to alignment.
    * If a larger format is selected, it's still OK to load a smaller amount from it.
    */
   bytes_needed = MIN2(bytes_needed, max_fetched_components * info.component_size);
   unsigned bytes_size = 0;
   const unsigned bit_size = info.component_size * 8;
   aco_opcode op = aco_opcode::num_opcodes;

   if (bytes_needed == 2) {
      bytes_size = 2;
      op = aco_opcode::tbuffer_load_format_d16_x;
   } else if (bytes_needed <= 4) {
      bytes_size = 4;
      if (bit_size == 16)
         op = aco_opcode::tbuffer_load_format_d16_xy;
      else
         op = aco_opcode::tbuffer_load_format_x;
   } else if (bytes_needed <= 6) {
      bytes_size = 6;
      if (bit_size == 16)
         op = aco_opcode::tbuffer_load_format_d16_xyz;
      else
         op = aco_opcode::tbuffer_load_format_xy;
   } else if (bytes_needed <= 8) {
      bytes_size = 8;
      if (bit_size == 16)
         op = aco_opcode::tbuffer_load_format_d16_xyzw;
      else
         op = aco_opcode::tbuffer_load_format_xy;
   } else if (bytes_needed <= 12) {
      bytes_size = 12;
      op = aco_opcode::tbuffer_load_format_xyz;
   } else {
      bytes_size = 16;
      op = aco_opcode::tbuffer_load_format_xyzw;
   }

   /* Abort when suitable opcode wasn't found so we don't compile buggy shaders. */
   if (op == aco_opcode::num_opcodes) {
      aco_err(bld.program, "unsupported bit size for typed buffer load");
      abort();
   }

   aco_ptr<MTBUF_instruction> mtbuf{create_instruction<MTBUF_instruction>(op, Format::MTBUF, 3, 1)};
   mtbuf->operands[0] = Operand(info.resource);
   mtbuf->operands[1] = vaddr;
   mtbuf->operands[2] = soffset;
   mtbuf->offen = offen;
   mtbuf->idxen = idxen;
   mtbuf->glc = info.glc;
   mtbuf->dlc = info.glc && (bld.program->gfx_level == GFX10 || bld.program->gfx_level == GFX10_3);
   mtbuf->slc = info.slc;
   mtbuf->sync = info.sync;
   mtbuf->offset = const_offset;
   mtbuf->dfmt = fetch_fmt & 0xf;
   mtbuf->nfmt = fetch_fmt >> 4;
   RegClass rc = RegClass::get(RegType::vgpr, bytes_size);
   Temp val = dst_hint.id() && rc == dst_hint.regClass() ? dst_hint : bld.tmp(rc);
   mtbuf->definitions[0] = Definition(val);
   bld.insert(std::move(mtbuf));

   return val;
}

const EmitLoadParameters mtbuf_load_params{mtbuf_load_callback, false, true, 4096};

void
visit_load_fs_input(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   Temp dst = get_ssa_temp(ctx, &instr->def);
   nir_src offset = *nir_get_io_offset_src(instr);

   if (!nir_src_is_const(offset) || nir_src_as_uint(offset))
      isel_err(offset.ssa->parent_instr, "Unimplemented non-zero nir_intrinsic_load_input offset");

   Temp prim_mask = get_arg(ctx, ctx->args->prim_mask);

   unsigned idx = nir_intrinsic_base(instr);
   unsigned component = nir_intrinsic_component(instr);
   unsigned vertex_id = 0; /* P0 */

   if (instr->intrinsic == nir_intrinsic_load_input_vertex)
      vertex_id = nir_src_as_uint(instr->src[0]);

   if (instr->def.num_components == 1 && instr->def.bit_size != 64) {
      emit_interp_mov_instr(ctx, idx, component, vertex_id, dst, prim_mask);
   } else {
      unsigned num_components = instr->def.num_components;
      if (instr->def.bit_size == 64)
         num_components *= 2;
      aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
         aco_opcode::p_create_vector, Format::PSEUDO, num_components, 1)};
      for (unsigned i = 0; i < num_components; i++) {
         unsigned chan_component = (component + i) % 4;
         unsigned chan_idx = idx + (component + i) / 4;
         vec->operands[i] = Operand(bld.tmp(instr->def.bit_size == 16 ? v2b : v1));
         emit_interp_mov_instr(ctx, chan_idx, chan_component, vertex_id, vec->operands[i].getTemp(),
                               prim_mask);
      }
      vec->definitions[0] = Definition(dst);
      bld.insert(std::move(vec));
   }
}

void
visit_load_tcs_per_vertex_input(isel_context* ctx, nir_intrinsic_instr* instr)
{
   assert(ctx->shader->info.stage == MESA_SHADER_TESS_CTRL);

   Builder bld(ctx->program, ctx->block);
   Temp dst = get_ssa_temp(ctx, &instr->def);

   if (load_input_from_temps(ctx, instr, dst))
      return;

   unreachable("LDS-based TCS input should have been lowered in NIR.");
}

void
visit_load_per_vertex_input(isel_context* ctx, nir_intrinsic_instr* instr)
{
   switch (ctx->shader->info.stage) {
   case MESA_SHADER_TESS_CTRL: visit_load_tcs_per_vertex_input(ctx, instr); break;
   default: unreachable("Unimplemented shader stage");
   }
}

void
visit_load_tess_coord(isel_context* ctx, nir_intrinsic_instr* instr)
{
   assert(ctx->shader->info.stage == MESA_SHADER_TESS_EVAL);

   Builder bld(ctx->program, ctx->block);
   Temp dst = get_ssa_temp(ctx, &instr->def);

   Operand tes_u(get_arg(ctx, ctx->args->tes_u));
   Operand tes_v(get_arg(ctx, ctx->args->tes_v));
   Operand tes_w = Operand::zero();

   if (ctx->shader->info.tess._primitive_mode == TESS_PRIMITIVE_TRIANGLES) {
      Temp tmp = bld.vop2(aco_opcode::v_add_f32, bld.def(v1), tes_u, tes_v);
      tmp = bld.vop2(aco_opcode::v_sub_f32, bld.def(v1), Operand::c32(0x3f800000u /* 1.0f */), tmp);
      tes_w = Operand(tmp);
   }

   Temp tess_coord = bld.pseudo(aco_opcode::p_create_vector, Definition(dst), tes_u, tes_v, tes_w);
   emit_split_vector(ctx, tess_coord, 3);
}

void
load_buffer(isel_context* ctx, unsigned num_components, unsigned component_size, Temp dst,
            Temp rsrc, Temp offset, unsigned align_mul, unsigned align_offset, bool glc = false,
            bool allow_smem = true, memory_sync_info sync = memory_sync_info())
{
   Builder bld(ctx->program, ctx->block);

   bool use_smem =
      dst.type() != RegType::vgpr && (!glc || ctx->options->gfx_level >= GFX8) && allow_smem;
   if (use_smem)
      offset = bld.as_uniform(offset);
   else {
      /* GFX6-7 are affected by a hw bug that prevents address clamping to
       * work correctly when the SGPR offset is used.
       */
      if (offset.type() == RegType::sgpr && ctx->options->gfx_level < GFX8)
         offset = as_vgpr(ctx, offset);
   }

   LoadEmitInfo info = {Operand(offset), dst, num_components, component_size, rsrc};
   info.glc = glc;
   info.sync = sync;
   info.align_mul = align_mul;
   info.align_offset = align_offset;
   if (use_smem)
      emit_load(ctx, bld, info, smem_load_params);
   else
      emit_load(ctx, bld, info, mubuf_load_params);
}

void
visit_load_ubo(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Temp dst = get_ssa_temp(ctx, &instr->def);
   Builder bld(ctx->program, ctx->block);
   Temp rsrc = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));

   unsigned size = instr->def.bit_size / 8;
   load_buffer(ctx, instr->num_components, size, dst, rsrc, get_ssa_temp(ctx, instr->src[1].ssa),
               nir_intrinsic_align_mul(instr), nir_intrinsic_align_offset(instr));
}

void
visit_load_push_constant(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   Temp dst = get_ssa_temp(ctx, &instr->def);
   unsigned offset = nir_intrinsic_base(instr);
   unsigned count = instr->def.num_components;
   nir_const_value* index_cv = nir_src_as_const_value(instr->src[0]);

   if (instr->def.bit_size == 64)
      count *= 2;

   if (index_cv && instr->def.bit_size >= 32) {
      unsigned start = (offset + index_cv->u32) / 4u;
      uint64_t mask = BITFIELD64_MASK(count) << start;
      if ((ctx->args->inline_push_const_mask | mask) == ctx->args->inline_push_const_mask &&
          start + count <= (sizeof(ctx->args->inline_push_const_mask) * 8u)) {
         std::array<Temp, NIR_MAX_VEC_COMPONENTS> elems;
         aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
            aco_opcode::p_create_vector, Format::PSEUDO, count, 1)};
         unsigned arg_index =
            util_bitcount64(ctx->args->inline_push_const_mask & BITFIELD64_MASK(start));
         for (unsigned i = 0; i < count; ++i) {
            elems[i] = get_arg(ctx, ctx->args->inline_push_consts[arg_index++]);
            vec->operands[i] = Operand{elems[i]};
         }
         vec->definitions[0] = Definition(dst);
         ctx->block->instructions.emplace_back(std::move(vec));
         ctx->allocated_vec.emplace(dst.id(), elems);
         return;
      }
   }

   Temp index = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));
   if (offset != 0) // TODO check if index != 0 as well
      index = bld.nuw().sop2(aco_opcode::s_add_i32, bld.def(s1), bld.def(s1, scc),
                             Operand::c32(offset), index);
   Temp ptr = convert_pointer_to_64_bit(ctx, get_arg(ctx, ctx->args->push_constants));
   Temp vec = dst;
   bool trim = false;
   bool aligned = true;

   if (instr->def.bit_size == 8) {
      aligned = index_cv && (offset + index_cv->u32) % 4 == 0;
      bool fits_in_dword = count == 1 || (index_cv && ((offset + index_cv->u32) % 4 + count) <= 4);
      if (!aligned)
         vec = fits_in_dword ? bld.tmp(s1) : bld.tmp(s2);
   } else if (instr->def.bit_size == 16) {
      aligned = index_cv && (offset + index_cv->u32) % 4 == 0;
      if (!aligned)
         vec = count == 4 ? bld.tmp(s4) : count > 1 ? bld.tmp(s2) : bld.tmp(s1);
   }

   aco_opcode op;

   switch (vec.size()) {
   case 1: op = aco_opcode::s_load_dword; break;
   case 2: op = aco_opcode::s_load_dwordx2; break;
   case 3:
      vec = bld.tmp(s4);
      trim = true;
      FALLTHROUGH;
   case 4: op = aco_opcode::s_load_dwordx4; break;
   case 6:
      vec = bld.tmp(s8);
      trim = true;
      FALLTHROUGH;
   case 8: op = aco_opcode::s_load_dwordx8; break;
   default: unreachable("unimplemented or forbidden load_push_constant.");
   }

   bld.smem(op, Definition(vec), ptr, index);

   if (!aligned) {
      Operand byte_offset = index_cv ? Operand::c32((offset + index_cv->u32) % 4) : Operand(index);
      byte_align_scalar(ctx, vec, byte_offset, dst);
      return;
   }

   if (trim) {
      emit_split_vector(ctx, vec, 4);
      RegClass rc = dst.size() == 3 ? s1 : s2;
      bld.pseudo(aco_opcode::p_create_vector, Definition(dst), emit_extract_vector(ctx, vec, 0, rc),
                 emit_extract_vector(ctx, vec, 1, rc), emit_extract_vector(ctx, vec, 2, rc));
   }
   emit_split_vector(ctx, dst, instr->def.num_components);
}

void
visit_load_constant(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Temp dst = get_ssa_temp(ctx, &instr->def);

   Builder bld(ctx->program, ctx->block);

   uint32_t desc_type =
      S_008F0C_DST_SEL_X(V_008F0C_SQ_SEL_X) | S_008F0C_DST_SEL_Y(V_008F0C_SQ_SEL_Y) |
      S_008F0C_DST_SEL_Z(V_008F0C_SQ_SEL_Z) | S_008F0C_DST_SEL_W(V_008F0C_SQ_SEL_W);
   if (ctx->options->gfx_level >= GFX10) {
      desc_type |= S_008F0C_FORMAT(V_008F0C_GFX10_FORMAT_32_FLOAT) |
                   S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_RAW) |
                   S_008F0C_RESOURCE_LEVEL(ctx->options->gfx_level < GFX11);
   } else {
      desc_type |= S_008F0C_NUM_FORMAT(V_008F0C_BUF_NUM_FORMAT_FLOAT) |
                   S_008F0C_DATA_FORMAT(V_008F0C_BUF_DATA_FORMAT_32);
   }

   unsigned base = nir_intrinsic_base(instr);
   unsigned range = nir_intrinsic_range(instr);

   Temp offset = get_ssa_temp(ctx, instr->src[0].ssa);
   if (base && offset.type() == RegType::sgpr)
      offset = bld.nuw().sop2(aco_opcode::s_add_u32, bld.def(s1), bld.def(s1, scc), offset,
                              Operand::c32(base));
   else if (base && offset.type() == RegType::vgpr)
      offset = bld.vadd32(bld.def(v1), Operand::c32(base), offset);

   Temp rsrc = bld.pseudo(aco_opcode::p_create_vector, bld.def(s4),
                          bld.pseudo(aco_opcode::p_constaddr, bld.def(s2), bld.def(s1, scc),
                                     Operand::c32(ctx->constant_data_offset)),
                          Operand::c32(MIN2(base + range, ctx->shader->constant_data_size)),
                          Operand::c32(desc_type));
   unsigned size = instr->def.bit_size / 8;
   // TODO: get alignment information for subdword constants
   load_buffer(ctx, instr->num_components, size, dst, rsrc, offset, size, 0);
}

/* Packs multiple Temps of different sizes in to a vector of v1 Temps.
 * The byte count of each input Temp must be a multiple of 2.
 */
static std::vector<Temp>
emit_pack_v1(isel_context* ctx, const std::vector<Temp>& unpacked)
{
   Builder bld(ctx->program, ctx->block);
   std::vector<Temp> packed;
   Temp low = Temp();
   for (Temp tmp : unpacked) {
      assert(tmp.bytes() % 2 == 0);
      unsigned byte_idx = 0;
      while (byte_idx < tmp.bytes()) {
         if (low != Temp()) {
            Temp high = emit_extract_vector(ctx, tmp, byte_idx / 2, v2b);
            Temp dword = bld.pseudo(aco_opcode::p_create_vector, bld.def(v1), low, high);
            low = Temp();
            packed.push_back(dword);
            byte_idx += 2;
         } else if (byte_idx % 4 == 0 && (byte_idx + 4) <= tmp.bytes()) {
            packed.emplace_back(emit_extract_vector(ctx, tmp, byte_idx / 4, v1));
            byte_idx += 4;
         } else {
            low = emit_extract_vector(ctx, tmp, byte_idx / 2, v2b);
            byte_idx += 2;
         }
      }
   }
   if (low != Temp()) {
      Temp dword = bld.pseudo(aco_opcode::p_create_vector, bld.def(v1), low, Operand(v2b));
      packed.push_back(dword);
   }
   return packed;
}

static bool
should_declare_array(ac_image_dim dim)
{
   return dim == ac_image_cube || dim == ac_image_1darray || dim == ac_image_2darray ||
          dim == ac_image_2darraymsaa;
}

static int
image_type_to_components_count(enum glsl_sampler_dim dim, bool array)
{
   switch (dim) {
   case GLSL_SAMPLER_DIM_BUF: return 1;
   case GLSL_SAMPLER_DIM_1D: return array ? 2 : 1;
   case GLSL_SAMPLER_DIM_2D: return array ? 3 : 2;
   case GLSL_SAMPLER_DIM_MS: return array ? 3 : 2;
   case GLSL_SAMPLER_DIM_3D:
   case GLSL_SAMPLER_DIM_CUBE: return 3;
   case GLSL_SAMPLER_DIM_RECT:
   case GLSL_SAMPLER_DIM_SUBPASS: return 2;
   case GLSL_SAMPLER_DIM_SUBPASS_MS: return 2;
   default: break;
   }
   return 0;
}

static MIMG_instruction*
emit_mimg(Builder& bld, aco_opcode op, Temp dst, Temp rsrc, Operand samp, std::vector<Temp> coords,
          Operand vdata = Operand(v1))
{
   size_t nsa_size = bld.program->dev.max_nsa_vgprs;
   nsa_size = bld.program->gfx_level >= GFX11 || coords.size() <= nsa_size ? nsa_size : 0;

   const bool strict_wqm = coords[0].regClass().is_linear_vgpr();
   if (strict_wqm)
      nsa_size = coords.size();

   for (unsigned i = 0; i < std::min(coords.size(), nsa_size); i++) {
      if (!coords[i].id())
         continue;

      coords[i] = as_vgpr(bld, coords[i]);
   }

   if (nsa_size < coords.size()) {
      Temp coord = coords[nsa_size];
      if (coords.size() - nsa_size > 1) {
         aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
            aco_opcode::p_create_vector, Format::PSEUDO, coords.size() - nsa_size, 1)};

         unsigned coord_size = 0;
         for (unsigned i = nsa_size; i < coords.size(); i++) {
            vec->operands[i - nsa_size] = Operand(coords[i]);
            coord_size += coords[i].size();
         }

         coord = bld.tmp(RegType::vgpr, coord_size);
         vec->definitions[0] = Definition(coord);
         bld.insert(std::move(vec));
      } else {
         coord = as_vgpr(bld, coord);
      }

      coords[nsa_size] = coord;
      coords.resize(nsa_size + 1);
   }

   bool has_dst = dst.id() != 0;

   aco_ptr<MIMG_instruction> mimg{
      create_instruction<MIMG_instruction>(op, Format::MIMG, 3 + coords.size(), has_dst)};
   if (has_dst)
      mimg->definitions[0] = Definition(dst);
   mimg->operands[0] = Operand(rsrc);
   mimg->operands[1] = samp;
   mimg->operands[2] = vdata;
   for (unsigned i = 0; i < coords.size(); i++)
      mimg->operands[3 + i] = Operand(coords[i]);
   mimg->strict_wqm = strict_wqm;

   MIMG_instruction* res = mimg.get();
   bld.insert(std::move(mimg));
   return res;
}

void
visit_bvh64_intersect_ray_amd(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   Temp dst = get_ssa_temp(ctx, &instr->def);
   Temp resource = get_ssa_temp(ctx, instr->src[0].ssa);
   Temp node = get_ssa_temp(ctx, instr->src[1].ssa);
   Temp tmax = get_ssa_temp(ctx, instr->src[2].ssa);
   Temp origin = get_ssa_temp(ctx, instr->src[3].ssa);
   Temp dir = get_ssa_temp(ctx, instr->src[4].ssa);
   Temp inv_dir = get_ssa_temp(ctx, instr->src[5].ssa);

   /* On GFX11 image_bvh64_intersect_ray has a special vaddr layout with NSA:
    * There are five smaller vector groups:
    * node_pointer, ray_extent, ray_origin, ray_dir, ray_inv_dir.
    * These directly match the NIR intrinsic sources.
    */
   std::vector<Temp> args = {
      node, tmax, origin, dir, inv_dir,
   };

   if (bld.program->gfx_level == GFX10_3) {
      std::vector<Temp> scalar_args;
      for (Temp tmp : args) {
         for (unsigned i = 0; i < tmp.size(); i++)
            scalar_args.push_back(emit_extract_vector(ctx, tmp, i, v1));
      }
      args = std::move(scalar_args);
   }

   MIMG_instruction* mimg =
      emit_mimg(bld, aco_opcode::image_bvh64_intersect_ray, dst, resource, Operand(s4), args);
   mimg->dim = ac_image_1d;
   mimg->dmask = 0xf;
   mimg->unrm = true;
   mimg->r128 = true;

   emit_split_vector(ctx, dst, instr->def.num_components);
}

static std::vector<Temp>
get_image_coords(isel_context* ctx, const nir_intrinsic_instr* instr)
{

   Temp src0 = get_ssa_temp(ctx, instr->src[1].ssa);
   bool a16 = instr->src[1].ssa->bit_size == 16;
   RegClass rc = a16 ? v2b : v1;
   enum glsl_sampler_dim dim = nir_intrinsic_image_dim(instr);
   bool is_array = nir_intrinsic_image_array(instr);
   ASSERTED bool add_frag_pos =
      (dim == GLSL_SAMPLER_DIM_SUBPASS || dim == GLSL_SAMPLER_DIM_SUBPASS_MS);
   assert(!add_frag_pos && "Input attachments should be lowered.");
   bool is_ms = (dim == GLSL_SAMPLER_DIM_MS || dim == GLSL_SAMPLER_DIM_SUBPASS_MS);
   bool gfx9_1d = ctx->options->gfx_level == GFX9 && dim == GLSL_SAMPLER_DIM_1D;
   int count = image_type_to_components_count(dim, is_array);
   std::vector<Temp> coords;
   Builder bld(ctx->program, ctx->block);

   if (gfx9_1d) {
      coords.emplace_back(emit_extract_vector(ctx, src0, 0, rc));
      coords.emplace_back(bld.copy(bld.def(rc), Operand::zero(a16 ? 2 : 4)));
      if (is_array)
         coords.emplace_back(emit_extract_vector(ctx, src0, 1, rc));
   } else {
      for (int i = 0; i < count; i++)
         coords.emplace_back(emit_extract_vector(ctx, src0, i, rc));
   }

   bool has_lod = false;
   Temp lod;

   if (instr->intrinsic == nir_intrinsic_bindless_image_load ||
       instr->intrinsic == nir_intrinsic_bindless_image_sparse_load ||
       instr->intrinsic == nir_intrinsic_bindless_image_store) {
      int lod_index = instr->intrinsic == nir_intrinsic_bindless_image_store ? 4 : 3;
      assert(instr->src[lod_index].ssa->bit_size == (a16 ? 16 : 32));
      has_lod =
         !nir_src_is_const(instr->src[lod_index]) || nir_src_as_uint(instr->src[lod_index]) != 0;

      if (has_lod)
         lod = get_ssa_temp_tex(ctx, instr->src[lod_index].ssa, a16);
   }

   if (ctx->program->info.image_2d_view_of_3d && dim == GLSL_SAMPLER_DIM_2D && !is_array) {
      /* The hw can't bind a slice of a 3D image as a 2D image, because it
       * ignores BASE_ARRAY if the target is 3D. The workaround is to read
       * BASE_ARRAY and set it as the 3rd address operand for all 2D images.
       */
      assert(ctx->options->gfx_level == GFX9);
      Temp rsrc = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));
      Temp rsrc_word5 = emit_extract_vector(ctx, rsrc, 5, v1);
      /* Extract the BASE_ARRAY field [0:12] from the descriptor. */
      Temp first_layer = bld.vop3(aco_opcode::v_bfe_u32, bld.def(v1), rsrc_word5, Operand::c32(0u),
                                  Operand::c32(13u));

      if (has_lod) {
         /* If there's a lod parameter it matter if the image is 3d or 2d because
          * the hw reads either the fourth or third component as lod. So detect
          * 3d images and place the lod at the third component otherwise.
          * For non 3D descriptors we effectively add lod twice to coords,
          * but the hw will only read the first one, the second is ignored.
          */
         Temp rsrc_word3 = emit_extract_vector(ctx, rsrc, 3, s1);
         Temp type = bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc), rsrc_word3,
                              Operand::c32(28 | (4 << 16))); /* extract last 4 bits */
         Temp is_3d = bld.vopc_e64(aco_opcode::v_cmp_eq_u32, bld.def(bld.lm), type,
                                   Operand::c32(V_008F1C_SQ_RSRC_IMG_3D));
         first_layer =
            bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), as_vgpr(ctx, lod), first_layer, is_3d);
      }

      if (a16)
         coords.emplace_back(emit_extract_vector(ctx, first_layer, 0, v2b));
      else
         coords.emplace_back(first_layer);
   }

   if (is_ms && instr->intrinsic != nir_intrinsic_bindless_image_fragment_mask_load_amd) {
      assert(instr->src[2].ssa->bit_size == (a16 ? 16 : 32));
      coords.emplace_back(get_ssa_temp_tex(ctx, instr->src[2].ssa, a16));
   }

   if (has_lod)
      coords.emplace_back(lod);

   return emit_pack_v1(ctx, coords);
}

memory_sync_info
get_memory_sync_info(nir_intrinsic_instr* instr, storage_class storage, unsigned semantics)
{
   /* atomicrmw might not have NIR_INTRINSIC_ACCESS and there's nothing interesting there anyway */
   if (semantics & semantic_atomicrmw)
      return memory_sync_info(storage, semantics);

   unsigned access = nir_intrinsic_access(instr);

   if (access & ACCESS_VOLATILE)
      semantics |= semantic_volatile;
   if (access & ACCESS_CAN_REORDER)
      semantics |= semantic_can_reorder | semantic_private;

   return memory_sync_info(storage, semantics);
}

Operand
emit_tfe_init(Builder& bld, Temp dst)
{
   Temp tmp = bld.tmp(dst.regClass());

   aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
      aco_opcode::p_create_vector, Format::PSEUDO, dst.size(), 1)};
   for (unsigned i = 0; i < dst.size(); i++)
      vec->operands[i] = Operand::zero();
   vec->definitions[0] = Definition(tmp);
   /* Since this is fixed to an instruction's definition register, any CSE will
    * just create copies. Copying costs about the same as zero-initialization,
    * but these copies can break up clauses.
    */
   vec->definitions[0].setNoCSE(true);
   bld.insert(std::move(vec));

   return Operand(tmp);
}

void
visit_image_load(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   const enum glsl_sampler_dim dim = nir_intrinsic_image_dim(instr);
   bool is_array = nir_intrinsic_image_array(instr);
   bool is_sparse = instr->intrinsic == nir_intrinsic_bindless_image_sparse_load;
   Temp dst = get_ssa_temp(ctx, &instr->def);

   memory_sync_info sync = get_memory_sync_info(instr, storage_image, 0);
   unsigned access = nir_intrinsic_access(instr);

   unsigned result_size = instr->def.num_components - is_sparse;
   unsigned expand_mask = nir_def_components_read(&instr->def) & u_bit_consecutive(0, result_size);
   expand_mask = MAX2(expand_mask, 1); /* this can be zero in the case of sparse image loads */
   if (dim == GLSL_SAMPLER_DIM_BUF)
      expand_mask = (1u << util_last_bit(expand_mask)) - 1u;
   unsigned dmask = expand_mask;
   if (instr->def.bit_size == 64) {
      expand_mask &= 0x9;
      /* only R64_UINT and R64_SINT supported. x is in xy of the result, w in zw */
      dmask = ((expand_mask & 0x1) ? 0x3 : 0) | ((expand_mask & 0x8) ? 0xc : 0);
   }
   if (is_sparse)
      expand_mask |= 1 << result_size;

   bool d16 = instr->def.bit_size == 16;
   assert(!d16 || !is_sparse);

   unsigned num_bytes = util_bitcount(dmask) * (d16 ? 2 : 4) + is_sparse * 4;

   Temp tmp;
   if (num_bytes == dst.bytes() && dst.type() == RegType::vgpr)
      tmp = dst;
   else
      tmp = bld.tmp(RegClass::get(RegType::vgpr, num_bytes));

   Temp resource = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));

   if (dim == GLSL_SAMPLER_DIM_BUF) {
      Temp vindex = emit_extract_vector(ctx, get_ssa_temp(ctx, instr->src[1].ssa), 0, v1);

      aco_opcode opcode;
      if (!d16) {
         switch (util_bitcount(dmask)) {
         case 1: opcode = aco_opcode::buffer_load_format_x; break;
         case 2: opcode = aco_opcode::buffer_load_format_xy; break;
         case 3: opcode = aco_opcode::buffer_load_format_xyz; break;
         case 4: opcode = aco_opcode::buffer_load_format_xyzw; break;
         default: unreachable(">4 channel buffer image load");
         }
      } else {
         switch (util_bitcount(dmask)) {
         case 1: opcode = aco_opcode::buffer_load_format_d16_x; break;
         case 2: opcode = aco_opcode::buffer_load_format_d16_xy; break;
         case 3: opcode = aco_opcode::buffer_load_format_d16_xyz; break;
         case 4: opcode = aco_opcode::buffer_load_format_d16_xyzw; break;
         default: unreachable(">4 channel buffer image load");
         }
      }
      aco_ptr<MUBUF_instruction> load{
         create_instruction<MUBUF_instruction>(opcode, Format::MUBUF, 3 + is_sparse, 1)};
      load->operands[0] = Operand(resource);
      load->operands[1] = Operand(vindex);
      load->operands[2] = Operand::c32(0);
      load->definitions[0] = Definition(tmp);
      load->idxen = true;
      load->glc = access & (ACCESS_VOLATILE | ACCESS_COHERENT);
      load->dlc =
         load->glc && (ctx->options->gfx_level == GFX10 || ctx->options->gfx_level == GFX10_3);
      load->sync = sync;
      load->tfe = is_sparse;
      if (load->tfe)
         load->operands[3] = emit_tfe_init(bld, tmp);
      ctx->block->instructions.emplace_back(std::move(load));
   } else {
      std::vector<Temp> coords = get_image_coords(ctx, instr);

      aco_opcode opcode;
      if (instr->intrinsic == nir_intrinsic_bindless_image_fragment_mask_load_amd) {
         opcode = aco_opcode::image_load;
      } else {
         bool level_zero = nir_src_is_const(instr->src[3]) && nir_src_as_uint(instr->src[3]) == 0;
         opcode = level_zero ? aco_opcode::image_load : aco_opcode::image_load_mip;
      }

      Operand vdata = is_sparse ? emit_tfe_init(bld, tmp) : Operand(v1);
      MIMG_instruction* load = emit_mimg(bld, opcode, tmp, resource, Operand(s4), coords, vdata);
      load->glc = access & (ACCESS_VOLATILE | ACCESS_COHERENT) ? 1 : 0;
      load->dlc =
         load->glc && (ctx->options->gfx_level == GFX10 || ctx->options->gfx_level == GFX10_3);
      load->a16 = instr->src[1].ssa->bit_size == 16;
      load->d16 = d16;
      load->dmask = dmask;
      load->unrm = true;
      load->tfe = is_sparse;

      if (instr->intrinsic == nir_intrinsic_bindless_image_fragment_mask_load_amd) {
         load->dim = is_array ? ac_image_2darray : ac_image_2d;
         load->da = is_array;
         load->sync = memory_sync_info();
      } else {
         ac_image_dim sdim = ac_get_image_dim(ctx->options->gfx_level, dim, is_array);
         load->dim = sdim;
         load->da = should_declare_array(sdim);
         load->sync = sync;
      }
   }

   if (is_sparse && instr->def.bit_size == 64) {
      /* The result components are 64-bit but the sparse residency code is
       * 32-bit. So add a zero to the end so expand_vector() works correctly.
       */
      tmp = bld.pseudo(aco_opcode::p_create_vector, bld.def(RegType::vgpr, tmp.size() + 1), tmp,
                       Operand::zero());
   }

   expand_vector(ctx, tmp, dst, instr->def.num_components, expand_mask, instr->def.bit_size == 64);
}

void
visit_image_store(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   const enum glsl_sampler_dim dim = nir_intrinsic_image_dim(instr);
   bool is_array = nir_intrinsic_image_array(instr);
   Temp data = get_ssa_temp(ctx, instr->src[3].ssa);
   bool d16 = instr->src[3].ssa->bit_size == 16;

   /* only R64_UINT and R64_SINT supported */
   if (instr->src[3].ssa->bit_size == 64 && data.bytes() > 8)
      data = emit_extract_vector(ctx, data, 0, RegClass(data.type(), 2));
   data = as_vgpr(ctx, data);

   uint32_t num_components = d16 ? instr->src[3].ssa->num_components : data.size();

   memory_sync_info sync = get_memory_sync_info(instr, storage_image, 0);
   unsigned access = nir_intrinsic_access(instr);
   bool glc = ctx->options->gfx_level == GFX6 ||
              ((access & (ACCESS_VOLATILE | ACCESS_COHERENT)) && ctx->program->gfx_level < GFX11);

   uint32_t dmask = BITFIELD_MASK(num_components);
   if (instr->src[3].ssa->bit_size == 32 || instr->src[3].ssa->bit_size == 16) {
      for (uint32_t i = 0; i < instr->num_components; i++) {
         /* components not in dmask receive:
          * GFX6-11.5:  zero
          * GFX12+: first component in dmask
          */
         nir_scalar comp = nir_scalar_resolved(instr->src[3].ssa, i);
         if (nir_scalar_is_undef(comp)) {
            dmask &= ~BITFIELD_BIT(i);
         } else if (ctx->options->gfx_level <= GFX11_5) {
            if (nir_scalar_is_const(comp) && nir_scalar_as_uint(comp) == 0)
               dmask &= ~BITFIELD_BIT(i);
         } else {
            unsigned first = dim == GLSL_SAMPLER_DIM_BUF ? 0 : ffs(dmask) - 1;
            if (i != first && nir_scalar_equal(nir_scalar_resolved(instr->src[3].ssa, first), comp))
               dmask &= ~BITFIELD_BIT(i);
         }
      }

      /* dmask cannot be 0, at least one vgpr is always read */
      if (dmask == 0)
         dmask = 1;
      /* buffer store only supports consecutive components. */
      if (dim == GLSL_SAMPLER_DIM_BUF)
         dmask = BITFIELD_MASK(util_last_bit(dmask));

      if (dmask != BITFIELD_MASK(num_components)) {
         uint32_t dmask_count = util_bitcount(dmask);
         RegClass rc = d16 ? v2b : v1;
         if (dmask_count == 1) {
            data = emit_extract_vector(ctx, data, ffs(dmask) - 1, rc);
         } else {
            aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
               aco_opcode::p_create_vector, Format::PSEUDO, dmask_count, 1)};
            uint32_t index = 0;
            u_foreach_bit (bit, dmask) {
               vec->operands[index++] = Operand(emit_extract_vector(ctx, data, bit, rc));
            }
            data = bld.tmp(RegClass::get(RegType::vgpr, dmask_count * rc.bytes()));
            vec->definitions[0] = Definition(data);
            bld.insert(std::move(vec));
         }
      }
   }

   if (dim == GLSL_SAMPLER_DIM_BUF) {
      Temp rsrc = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));
      Temp vindex = emit_extract_vector(ctx, get_ssa_temp(ctx, instr->src[1].ssa), 0, v1);
      aco_opcode opcode;
      if (!d16) {
         switch (dmask) {
         case 0x1: opcode = aco_opcode::buffer_store_format_x; break;
         case 0x3: opcode = aco_opcode::buffer_store_format_xy; break;
         case 0x7: opcode = aco_opcode::buffer_store_format_xyz; break;
         case 0xf: opcode = aco_opcode::buffer_store_format_xyzw; break;
         default: unreachable(">4 channel buffer image store");
         }
      } else {
         switch (dmask) {
         case 0x1: opcode = aco_opcode::buffer_store_format_d16_x; break;
         case 0x3: opcode = aco_opcode::buffer_store_format_d16_xy; break;
         case 0x7: opcode = aco_opcode::buffer_store_format_d16_xyz; break;
         case 0xf: opcode = aco_opcode::buffer_store_format_d16_xyzw; break;
         default: unreachable(">4 channel buffer image store");
         }
      }
      aco_ptr<MUBUF_instruction> store{
         create_instruction<MUBUF_instruction>(opcode, Format::MUBUF, 4, 0)};
      store->operands[0] = Operand(rsrc);
      store->operands[1] = Operand(vindex);
      store->operands[2] = Operand::c32(0);
      store->operands[3] = Operand(data);
      store->idxen = true;
      store->glc = glc;
      store->dlc = false;
      store->disable_wqm = true;
      store->sync = sync;
      ctx->program->needs_exact = true;
      ctx->block->instructions.emplace_back(std::move(store));
      return;
   }

   assert(data.type() == RegType::vgpr);
   std::vector<Temp> coords = get_image_coords(ctx, instr);
   Temp resource = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));

   bool level_zero = nir_src_is_const(instr->src[4]) && nir_src_as_uint(instr->src[4]) == 0;
   aco_opcode opcode = level_zero ? aco_opcode::image_store : aco_opcode::image_store_mip;

   MIMG_instruction* store =
      emit_mimg(bld, opcode, Temp(0, v1), resource, Operand(s4), coords, Operand(data));
   store->glc = glc;
   store->dlc = false;
   store->a16 = instr->src[1].ssa->bit_size == 16;
   store->d16 = d16;
   store->dmask = dmask;
   store->unrm = true;
   ac_image_dim sdim = ac_get_image_dim(ctx->options->gfx_level, dim, is_array);
   store->dim = sdim;
   store->da = should_declare_array(sdim);
   store->disable_wqm = true;
   store->sync = sync;
   ctx->program->needs_exact = true;
   return;
}

void
translate_buffer_image_atomic_op(const nir_atomic_op op, aco_opcode* buf_op, aco_opcode* buf_op64,
                                 aco_opcode* image_op)
{
   switch (op) {
   case nir_atomic_op_iadd:
      *buf_op = aco_opcode::buffer_atomic_add;
      *buf_op64 = aco_opcode::buffer_atomic_add_x2;
      *image_op = aco_opcode::image_atomic_add;
      break;
   case nir_atomic_op_umin:
      *buf_op = aco_opcode::buffer_atomic_umin;
      *buf_op64 = aco_opcode::buffer_atomic_umin_x2;
      *image_op = aco_opcode::image_atomic_umin;
      break;
   case nir_atomic_op_imin:
      *buf_op = aco_opcode::buffer_atomic_smin;
      *buf_op64 = aco_opcode::buffer_atomic_smin_x2;
      *image_op = aco_opcode::image_atomic_smin;
      break;
   case nir_atomic_op_umax:
      *buf_op = aco_opcode::buffer_atomic_umax;
      *buf_op64 = aco_opcode::buffer_atomic_umax_x2;
      *image_op = aco_opcode::image_atomic_umax;
      break;
   case nir_atomic_op_imax:
      *buf_op = aco_opcode::buffer_atomic_smax;
      *buf_op64 = aco_opcode::buffer_atomic_smax_x2;
      *image_op = aco_opcode::image_atomic_smax;
      break;
   case nir_atomic_op_iand:
      *buf_op = aco_opcode::buffer_atomic_and;
      *buf_op64 = aco_opcode::buffer_atomic_and_x2;
      *image_op = aco_opcode::image_atomic_and;
      break;
   case nir_atomic_op_ior:
      *buf_op = aco_opcode::buffer_atomic_or;
      *buf_op64 = aco_opcode::buffer_atomic_or_x2;
      *image_op = aco_opcode::image_atomic_or;
      break;
   case nir_atomic_op_ixor:
      *buf_op = aco_opcode::buffer_atomic_xor;
      *buf_op64 = aco_opcode::buffer_atomic_xor_x2;
      *image_op = aco_opcode::image_atomic_xor;
      break;
   case nir_atomic_op_xchg:
      *buf_op = aco_opcode::buffer_atomic_swap;
      *buf_op64 = aco_opcode::buffer_atomic_swap_x2;
      *image_op = aco_opcode::image_atomic_swap;
      break;
   case nir_atomic_op_cmpxchg:
      *buf_op = aco_opcode::buffer_atomic_cmpswap;
      *buf_op64 = aco_opcode::buffer_atomic_cmpswap_x2;
      *image_op = aco_opcode::image_atomic_cmpswap;
      break;
   case nir_atomic_op_inc_wrap:
      *buf_op = aco_opcode::buffer_atomic_inc;
      *buf_op64 = aco_opcode::buffer_atomic_inc_x2;
      *image_op = aco_opcode::image_atomic_inc;
      break;
   case nir_atomic_op_dec_wrap:
      *buf_op = aco_opcode::buffer_atomic_dec;
      *buf_op64 = aco_opcode::buffer_atomic_dec_x2;
      *image_op = aco_opcode::image_atomic_dec;
      break;
   case nir_atomic_op_fadd:
      *buf_op = aco_opcode::buffer_atomic_add_f32;
      *buf_op64 = aco_opcode::num_opcodes;
      *image_op = aco_opcode::num_opcodes;
      break;
   case nir_atomic_op_fmin:
      *buf_op = aco_opcode::buffer_atomic_fmin;
      *buf_op64 = aco_opcode::buffer_atomic_fmin_x2;
      *image_op = aco_opcode::image_atomic_fmin;
      break;
   case nir_atomic_op_fmax:
      *buf_op = aco_opcode::buffer_atomic_fmax;
      *buf_op64 = aco_opcode::buffer_atomic_fmax_x2;
      *image_op = aco_opcode::image_atomic_fmax;
      break;
   default: unreachable("unsupported atomic operation");
   }
}

void
visit_image_atomic(isel_context* ctx, nir_intrinsic_instr* instr)
{
   bool return_previous = !nir_def_is_unused(&instr->def);
   const enum glsl_sampler_dim dim = nir_intrinsic_image_dim(instr);
   bool is_array = nir_intrinsic_image_array(instr);
   Builder bld(ctx->program, ctx->block);

   const nir_atomic_op op = nir_intrinsic_atomic_op(instr);
   const bool cmpswap = op == nir_atomic_op_cmpxchg;

   aco_opcode buf_op, buf_op64, image_op;
   translate_buffer_image_atomic_op(op, &buf_op, &buf_op64, &image_op);

   Temp data = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[3].ssa));
   bool is_64bit = data.bytes() == 8;
   assert((data.bytes() == 4 || data.bytes() == 8) && "only 32/64-bit image atomics implemented.");

   if (cmpswap)
      data = bld.pseudo(aco_opcode::p_create_vector, bld.def(is_64bit ? v4 : v2),
                        get_ssa_temp(ctx, instr->src[4].ssa), data);

   Temp dst = get_ssa_temp(ctx, &instr->def);
   memory_sync_info sync = get_memory_sync_info(instr, storage_image, semantic_atomicrmw);

   if (dim == GLSL_SAMPLER_DIM_BUF) {
      Temp vindex = emit_extract_vector(ctx, get_ssa_temp(ctx, instr->src[1].ssa), 0, v1);
      Temp resource = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));
      // assert(ctx->options->gfx_level < GFX9 && "GFX9 stride size workaround not yet
      // implemented.");
      aco_ptr<MUBUF_instruction> mubuf{create_instruction<MUBUF_instruction>(
         is_64bit ? buf_op64 : buf_op, Format::MUBUF, 4, return_previous ? 1 : 0)};
      mubuf->operands[0] = Operand(resource);
      mubuf->operands[1] = Operand(vindex);
      mubuf->operands[2] = Operand::c32(0);
      mubuf->operands[3] = Operand(data);
      Definition def =
         return_previous ? (cmpswap ? bld.def(data.regClass()) : Definition(dst)) : Definition();
      if (return_previous)
         mubuf->definitions[0] = def;
      mubuf->offset = 0;
      mubuf->idxen = true;
      mubuf->glc = return_previous;
      mubuf->dlc = false; /* Not needed for atomics */
      mubuf->disable_wqm = true;
      mubuf->sync = sync;
      ctx->program->needs_exact = true;
      ctx->block->instructions.emplace_back(std::move(mubuf));
      if (return_previous && cmpswap)
         bld.pseudo(aco_opcode::p_extract_vector, Definition(dst), def.getTemp(), Operand::zero());
      return;
   }

   std::vector<Temp> coords = get_image_coords(ctx, instr);
   Temp resource = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));
   Temp tmp = return_previous ? (cmpswap ? bld.tmp(data.regClass()) : dst) : Temp(0, v1);
   MIMG_instruction* mimg =
      emit_mimg(bld, image_op, tmp, resource, Operand(s4), coords, Operand(data));
   mimg->glc = return_previous;
   mimg->dlc = false; /* Not needed for atomics */
   mimg->dmask = (1 << data.size()) - 1;
   mimg->a16 = instr->src[1].ssa->bit_size == 16;
   mimg->unrm = true;
   ac_image_dim sdim = ac_get_image_dim(ctx->options->gfx_level, dim, is_array);
   mimg->dim = sdim;
   mimg->da = should_declare_array(sdim);
   mimg->disable_wqm = true;
   mimg->sync = sync;
   ctx->program->needs_exact = true;
   if (return_previous && cmpswap)
      bld.pseudo(aco_opcode::p_extract_vector, Definition(dst), tmp, Operand::zero());
   return;
}

void
visit_load_ssbo(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   unsigned num_components = instr->num_components;

   Temp dst = get_ssa_temp(ctx, &instr->def);
   Temp rsrc = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));

   unsigned access = nir_intrinsic_access(instr);
   bool glc = access & (ACCESS_VOLATILE | ACCESS_COHERENT);
   unsigned size = instr->def.bit_size / 8;

   bool allow_smem = access & ACCESS_CAN_REORDER;

   load_buffer(ctx, num_components, size, dst, rsrc, get_ssa_temp(ctx, instr->src[1].ssa),
               nir_intrinsic_align_mul(instr), nir_intrinsic_align_offset(instr), glc, allow_smem,
               get_memory_sync_info(instr, storage_buffer, 0));
}

void
visit_store_ssbo(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   Temp data = get_ssa_temp(ctx, instr->src[0].ssa);
   unsigned elem_size_bytes = instr->src[0].ssa->bit_size / 8;
   unsigned writemask = util_widen_mask(nir_intrinsic_write_mask(instr), elem_size_bytes);
   Temp offset = get_ssa_temp(ctx, instr->src[2].ssa);

   Temp rsrc = bld.as_uniform(get_ssa_temp(ctx, instr->src[1].ssa));

   memory_sync_info sync = get_memory_sync_info(instr, storage_buffer, 0);
   bool glc = (nir_intrinsic_access(instr) & (ACCESS_VOLATILE | ACCESS_COHERENT)) &&
              ctx->program->gfx_level < GFX11;

   unsigned write_count = 0;
   Temp write_datas[32];
   unsigned offsets[32];
   split_buffer_store(ctx, instr, false, RegType::vgpr, data, writemask, 16, &write_count,
                      write_datas, offsets);

   /* GFX6-7 are affected by a hw bug that prevents address clamping to work
    * correctly when the SGPR offset is used.
    */
   if (offset.type() == RegType::sgpr && ctx->options->gfx_level < GFX8)
      offset = as_vgpr(ctx, offset);

   for (unsigned i = 0; i < write_count; i++) {
      aco_opcode op = get_buffer_store_op(write_datas[i].bytes());

      aco_ptr<MUBUF_instruction> store{
         create_instruction<MUBUF_instruction>(op, Format::MUBUF, 4, 0)};
      store->operands[0] = Operand(rsrc);
      store->operands[1] = offset.type() == RegType::vgpr ? Operand(offset) : Operand(v1);
      store->operands[2] = offset.type() == RegType::sgpr ? Operand(offset) : Operand::c32(0);
      store->operands[3] = Operand(write_datas[i]);
      store->offset = offsets[i];
      store->offen = (offset.type() == RegType::vgpr);
      store->glc = glc;
      store->dlc = false;
      store->disable_wqm = true;
      store->sync = sync;
      ctx->program->needs_exact = true;
      ctx->block->instructions.emplace_back(std::move(store));
   }
}

void
visit_atomic_ssbo(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   bool return_previous = !nir_def_is_unused(&instr->def);
   Temp data = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[2].ssa));

   const nir_atomic_op nir_op = nir_intrinsic_atomic_op(instr);
   const bool cmpswap = nir_op == nir_atomic_op_cmpxchg;

   aco_opcode op32, op64, image_op;
   translate_buffer_image_atomic_op(nir_op, &op32, &op64, &image_op);

   if (cmpswap)
      data = bld.pseudo(aco_opcode::p_create_vector, bld.def(RegType::vgpr, data.size() * 2),
                        get_ssa_temp(ctx, instr->src[3].ssa), data);

   Temp offset = get_ssa_temp(ctx, instr->src[1].ssa);
   Temp rsrc = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));
   Temp dst = get_ssa_temp(ctx, &instr->def);

   aco_opcode op = instr->def.bit_size == 32 ? op32 : op64;
   aco_ptr<MUBUF_instruction> mubuf{
      create_instruction<MUBUF_instruction>(op, Format::MUBUF, 4, return_previous ? 1 : 0)};
   mubuf->operands[0] = Operand(rsrc);
   mubuf->operands[1] = offset.type() == RegType::vgpr ? Operand(offset) : Operand(v1);
   mubuf->operands[2] = offset.type() == RegType::sgpr ? Operand(offset) : Operand::c32(0);
   mubuf->operands[3] = Operand(data);
   Definition def =
      return_previous ? (cmpswap ? bld.def(data.regClass()) : Definition(dst)) : Definition();
   if (return_previous)
      mubuf->definitions[0] = def;
   mubuf->offset = 0;
   mubuf->offen = (offset.type() == RegType::vgpr);
   mubuf->glc = return_previous;
   mubuf->dlc = false; /* Not needed for atomics */
   mubuf->disable_wqm = true;
   mubuf->sync = get_memory_sync_info(instr, storage_buffer, semantic_atomicrmw);
   ctx->program->needs_exact = true;
   ctx->block->instructions.emplace_back(std::move(mubuf));
   if (return_previous && cmpswap)
      bld.pseudo(aco_opcode::p_extract_vector, Definition(dst), def.getTemp(), Operand::zero());
}

void
parse_global(isel_context* ctx, nir_intrinsic_instr* intrin, Temp* address, uint32_t* const_offset,
             Temp* offset)
{
   bool is_store = intrin->intrinsic == nir_intrinsic_store_global_amd;
   *address = get_ssa_temp(ctx, intrin->src[is_store ? 1 : 0].ssa);

   *const_offset = nir_intrinsic_base(intrin);

   unsigned num_src = nir_intrinsic_infos[intrin->intrinsic].num_srcs;
   nir_src offset_src = intrin->src[num_src - 1];
   if (!nir_src_is_const(offset_src) || nir_src_as_uint(offset_src))
      *offset = get_ssa_temp(ctx, offset_src.ssa);
   else
      *offset = Temp();
}

void
visit_load_global(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   unsigned num_components = instr->num_components;
   unsigned component_size = instr->def.bit_size / 8;

   Temp addr, offset;
   uint32_t const_offset;
   parse_global(ctx, instr, &addr, &const_offset, &offset);

   LoadEmitInfo info = {Operand(addr), get_ssa_temp(ctx, &instr->def), num_components,
                        component_size};
   if (offset.id()) {
      info.resource = addr;
      info.offset = Operand(offset);
   }
   info.const_offset = const_offset;
   info.glc = nir_intrinsic_access(instr) & (ACCESS_VOLATILE | ACCESS_COHERENT);
   info.align_mul = nir_intrinsic_align_mul(instr);
   info.align_offset = nir_intrinsic_align_offset(instr);
   info.sync = get_memory_sync_info(instr, storage_buffer, 0);

   /* Don't expand global loads when they use MUBUF or SMEM.
    * Global loads don't have the bounds checking that buffer loads have that
    * makes this safe.
    */
   unsigned align = nir_intrinsic_align(instr);
   bool byte_align_for_smem_mubuf =
      can_use_byte_align_for_global_load(num_components, component_size, align, false);

   /* VMEM stores don't update the SMEM cache and it's difficult to prove that
    * it's safe to use SMEM */
   bool can_use_smem =
      (nir_intrinsic_access(instr) & ACCESS_NON_WRITEABLE) && byte_align_for_smem_mubuf;
   if (info.dst.type() == RegType::vgpr || (info.glc && ctx->options->gfx_level < GFX8) ||
       !can_use_smem) {
      EmitLoadParameters params = global_load_params;
      params.byte_align_loads = ctx->options->gfx_level > GFX6 || byte_align_for_smem_mubuf;
      emit_load(ctx, bld, info, params);
   } else {
      if (info.resource.id())
         info.resource = bld.as_uniform(info.resource);
      info.offset = Operand(bld.as_uniform(info.offset));
      emit_load(ctx, bld, info, smem_load_params);
   }
}

void
visit_store_global(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   unsigned elem_size_bytes = instr->src[0].ssa->bit_size / 8;
   unsigned writemask = util_widen_mask(nir_intrinsic_write_mask(instr), elem_size_bytes);

   Temp data = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[0].ssa));
   memory_sync_info sync = get_memory_sync_info(instr, storage_buffer, 0);
   bool glc = (nir_intrinsic_access(instr) & (ACCESS_VOLATILE | ACCESS_COHERENT)) &&
              ctx->program->gfx_level < GFX11;

   unsigned write_count = 0;
   Temp write_datas[32];
   unsigned offsets[32];
   split_buffer_store(ctx, instr, false, RegType::vgpr, data, writemask, 16, &write_count,
                      write_datas, offsets);

   Temp addr, offset;
   uint32_t const_offset;
   parse_global(ctx, instr, &addr, &const_offset, &offset);

   for (unsigned i = 0; i < write_count; i++) {
      Temp write_address = addr;
      uint32_t write_const_offset = const_offset;
      Temp write_offset = offset;
      lower_global_address(bld, offsets[i], &write_address, &write_const_offset, &write_offset);

      if (ctx->options->gfx_level >= GFX7) {
         bool global = ctx->options->gfx_level >= GFX9;
         aco_opcode op;
         switch (write_datas[i].bytes()) {
         case 1: op = global ? aco_opcode::global_store_byte : aco_opcode::flat_store_byte; break;
         case 2: op = global ? aco_opcode::global_store_short : aco_opcode::flat_store_short; break;
         case 4: op = global ? aco_opcode::global_store_dword : aco_opcode::flat_store_dword; break;
         case 8:
            op = global ? aco_opcode::global_store_dwordx2 : aco_opcode::flat_store_dwordx2;
            break;
         case 12:
            op = global ? aco_opcode::global_store_dwordx3 : aco_opcode::flat_store_dwordx3;
            break;
         case 16:
            op = global ? aco_opcode::global_store_dwordx4 : aco_opcode::flat_store_dwordx4;
            break;
         default: unreachable("store_global not implemented for this size.");
         }

         aco_ptr<FLAT_instruction> flat{
            create_instruction<FLAT_instruction>(op, global ? Format::GLOBAL : Format::FLAT, 3, 0)};
         if (write_address.regClass() == s2) {
            assert(global && write_offset.id() && write_offset.type() == RegType::vgpr);
            flat->operands[0] = Operand(write_offset);
            flat->operands[1] = Operand(write_address);
         } else {
            assert(write_address.type() == RegType::vgpr && !write_offset.id());
            flat->operands[0] = Operand(write_address);
            flat->operands[1] = Operand(s1);
         }
         flat->operands[2] = Operand(write_datas[i]);
         flat->glc = glc;
         flat->dlc = false;
         assert(global || !write_const_offset);
         flat->offset = write_const_offset;
         flat->disable_wqm = true;
         flat->sync = sync;
         ctx->program->needs_exact = true;
         ctx->block->instructions.emplace_back(std::move(flat));
      } else {
         assert(ctx->options->gfx_level == GFX6);

         aco_opcode op = get_buffer_store_op(write_datas[i].bytes());

         Temp rsrc = get_gfx6_global_rsrc(bld, write_address);

         aco_ptr<MUBUF_instruction> mubuf{
            create_instruction<MUBUF_instruction>(op, Format::MUBUF, 4, 0)};
         mubuf->operands[0] = Operand(rsrc);
         mubuf->operands[1] =
            write_address.type() == RegType::vgpr ? Operand(write_address) : Operand(v1);
         mubuf->operands[2] = Operand(write_offset);
         mubuf->operands[3] = Operand(write_datas[i]);
         mubuf->glc = glc;
         mubuf->dlc = false;
         mubuf->offset = write_const_offset;
         mubuf->addr64 = write_address.type() == RegType::vgpr;
         mubuf->disable_wqm = true;
         mubuf->sync = sync;
         ctx->program->needs_exact = true;
         ctx->block->instructions.emplace_back(std::move(mubuf));
      }
   }
}

void
visit_global_atomic(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   bool return_previous = !nir_def_is_unused(&instr->def);
   Temp data = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[1].ssa));

   const nir_atomic_op nir_op = nir_intrinsic_atomic_op(instr);
   const bool cmpswap = nir_op == nir_atomic_op_cmpxchg;

   if (cmpswap)
      data = bld.pseudo(aco_opcode::p_create_vector, bld.def(RegType::vgpr, data.size() * 2),
                        get_ssa_temp(ctx, instr->src[2].ssa), data);

   Temp dst = get_ssa_temp(ctx, &instr->def);

   aco_opcode op32, op64;

   Temp addr, offset;
   uint32_t const_offset;
   parse_global(ctx, instr, &addr, &const_offset, &offset);
   lower_global_address(bld, 0, &addr, &const_offset, &offset);

   if (ctx->options->gfx_level >= GFX7) {
      bool global = ctx->options->gfx_level >= GFX9;
      switch (nir_op) {
      case nir_atomic_op_iadd:
         op32 = global ? aco_opcode::global_atomic_add : aco_opcode::flat_atomic_add;
         op64 = global ? aco_opcode::global_atomic_add_x2 : aco_opcode::flat_atomic_add_x2;
         break;
      case nir_atomic_op_imin:
         op32 = global ? aco_opcode::global_atomic_smin : aco_opcode::flat_atomic_smin;
         op64 = global ? aco_opcode::global_atomic_smin_x2 : aco_opcode::flat_atomic_smin_x2;
         break;
      case nir_atomic_op_umin:
         op32 = global ? aco_opcode::global_atomic_umin : aco_opcode::flat_atomic_umin;
         op64 = global ? aco_opcode::global_atomic_umin_x2 : aco_opcode::flat_atomic_umin_x2;
         break;
      case nir_atomic_op_imax:
         op32 = global ? aco_opcode::global_atomic_smax : aco_opcode::flat_atomic_smax;
         op64 = global ? aco_opcode::global_atomic_smax_x2 : aco_opcode::flat_atomic_smax_x2;
         break;
      case nir_atomic_op_umax:
         op32 = global ? aco_opcode::global_atomic_umax : aco_opcode::flat_atomic_umax;
         op64 = global ? aco_opcode::global_atomic_umax_x2 : aco_opcode::flat_atomic_umax_x2;
         break;
      case nir_atomic_op_iand:
         op32 = global ? aco_opcode::global_atomic_and : aco_opcode::flat_atomic_and;
         op64 = global ? aco_opcode::global_atomic_and_x2 : aco_opcode::flat_atomic_and_x2;
         break;
      case nir_atomic_op_ior:
         op32 = global ? aco_opcode::global_atomic_or : aco_opcode::flat_atomic_or;
         op64 = global ? aco_opcode::global_atomic_or_x2 : aco_opcode::flat_atomic_or_x2;
         break;
      case nir_atomic_op_ixor:
         op32 = global ? aco_opcode::global_atomic_xor : aco_opcode::flat_atomic_xor;
         op64 = global ? aco_opcode::global_atomic_xor_x2 : aco_opcode::flat_atomic_xor_x2;
         break;
      case nir_atomic_op_xchg:
         op32 = global ? aco_opcode::global_atomic_swap : aco_opcode::flat_atomic_swap;
         op64 = global ? aco_opcode::global_atomic_swap_x2 : aco_opcode::flat_atomic_swap_x2;
         break;
      case nir_atomic_op_cmpxchg:
         op32 = global ? aco_opcode::global_atomic_cmpswap : aco_opcode::flat_atomic_cmpswap;
         op64 = global ? aco_opcode::global_atomic_cmpswap_x2 : aco_opcode::flat_atomic_cmpswap_x2;
         break;
      case nir_atomic_op_fadd:
         op32 = global ? aco_opcode::global_atomic_add_f32 : aco_opcode::flat_atomic_add_f32;
         op64 = aco_opcode::num_opcodes;
         break;
      case nir_atomic_op_fmin:
         op32 = global ? aco_opcode::global_atomic_fmin : aco_opcode::flat_atomic_fmin;
         op64 = global ? aco_opcode::global_atomic_fmin_x2 : aco_opcode::flat_atomic_fmin_x2;
         break;
      case nir_atomic_op_fmax:
         op32 = global ? aco_opcode::global_atomic_fmax : aco_opcode::flat_atomic_fmax;
         op64 = global ? aco_opcode::global_atomic_fmax_x2 : aco_opcode::flat_atomic_fmax_x2;
         break;
      default: unreachable("unsupported atomic operation");
      }

      aco_opcode op = instr->def.bit_size == 32 ? op32 : op64;
      aco_ptr<FLAT_instruction> flat{create_instruction<FLAT_instruction>(
         op, global ? Format::GLOBAL : Format::FLAT, 3, return_previous ? 1 : 0)};
      if (addr.regClass() == s2) {
         assert(global && offset.id() && offset.type() == RegType::vgpr);
         flat->operands[0] = Operand(offset);
         flat->operands[1] = Operand(addr);
      } else {
         assert(addr.type() == RegType::vgpr && !offset.id());
         flat->operands[0] = Operand(addr);
         flat->operands[1] = Operand(s1);
      }
      flat->operands[2] = Operand(data);
      if (return_previous)
         flat->definitions[0] = Definition(dst);
      flat->glc = return_previous;
      flat->dlc = false; /* Not needed for atomics */
      assert(global || !const_offset);
      flat->offset = const_offset;
      flat->disable_wqm = true;
      flat->sync = get_memory_sync_info(instr, storage_buffer, semantic_atomicrmw);
      ctx->program->needs_exact = true;
      ctx->block->instructions.emplace_back(std::move(flat));
   } else {
      assert(ctx->options->gfx_level == GFX6);

      UNUSED aco_opcode image_op;
      translate_buffer_image_atomic_op(nir_op, &op32, &op64, &image_op);

      Temp rsrc = get_gfx6_global_rsrc(bld, addr);

      aco_opcode op = instr->def.bit_size == 32 ? op32 : op64;

      aco_ptr<MUBUF_instruction> mubuf{
         create_instruction<MUBUF_instruction>(op, Format::MUBUF, 4, return_previous ? 1 : 0)};
      mubuf->operands[0] = Operand(rsrc);
      mubuf->operands[1] = addr.type() == RegType::vgpr ? Operand(addr) : Operand(v1);
      mubuf->operands[2] = Operand(offset);
      mubuf->operands[3] = Operand(data);
      Definition def =
         return_previous ? (cmpswap ? bld.def(data.regClass()) : Definition(dst)) : Definition();
      if (return_previous)
         mubuf->definitions[0] = def;
      mubuf->glc = return_previous;
      mubuf->dlc = false;
      mubuf->offset = const_offset;
      mubuf->addr64 = addr.type() == RegType::vgpr;
      mubuf->disable_wqm = true;
      mubuf->sync = get_memory_sync_info(instr, storage_buffer, semantic_atomicrmw);
      ctx->program->needs_exact = true;
      ctx->block->instructions.emplace_back(std::move(mubuf));
      if (return_previous && cmpswap)
         bld.pseudo(aco_opcode::p_extract_vector, Definition(dst), def.getTemp(), Operand::zero());
   }
}

unsigned
aco_storage_mode_from_nir_mem_mode(unsigned mem_mode)
{
   unsigned storage = storage_none;

   if (mem_mode & nir_var_shader_out)
      storage |= storage_vmem_output;
   if ((mem_mode & nir_var_mem_ssbo) || (mem_mode & nir_var_mem_global))
      storage |= storage_buffer;
   if (mem_mode & nir_var_mem_task_payload)
      storage |= storage_task_payload;
   if (mem_mode & nir_var_mem_shared)
      storage |= storage_shared;
   if (mem_mode & nir_var_image)
      storage |= storage_image;

   return storage;
}

void
visit_load_buffer(isel_context* ctx, nir_intrinsic_instr* intrin)
{
   Builder bld(ctx->program, ctx->block);

   /* Swizzled buffer addressing seems to be broken on GFX11 without the idxen bit. */
   bool swizzled = nir_intrinsic_access(intrin) & ACCESS_IS_SWIZZLED_AMD;
   bool idxen = (swizzled && ctx->program->gfx_level >= GFX11) ||
                !nir_src_is_const(intrin->src[3]) || nir_src_as_uint(intrin->src[3]);
   bool v_offset_zero = nir_src_is_const(intrin->src[1]) && !nir_src_as_uint(intrin->src[1]);
   bool s_offset_zero = nir_src_is_const(intrin->src[2]) && !nir_src_as_uint(intrin->src[2]);

   Temp dst = get_ssa_temp(ctx, &intrin->def);
   Temp descriptor = bld.as_uniform(get_ssa_temp(ctx, intrin->src[0].ssa));
   Temp v_offset =
      v_offset_zero ? Temp(0, v1) : as_vgpr(ctx, get_ssa_temp(ctx, intrin->src[1].ssa));
   Temp s_offset =
      s_offset_zero ? Temp(0, s1) : bld.as_uniform(get_ssa_temp(ctx, intrin->src[2].ssa));
   Temp idx = idxen ? as_vgpr(ctx, get_ssa_temp(ctx, intrin->src[3].ssa)) : Temp();

   bool glc = nir_intrinsic_access(intrin) & ACCESS_COHERENT;
   bool slc = nir_intrinsic_access(intrin) & ACCESS_NON_TEMPORAL;

   unsigned const_offset = nir_intrinsic_base(intrin);
   unsigned elem_size_bytes = intrin->def.bit_size / 8u;
   unsigned num_components = intrin->def.num_components;

   nir_variable_mode mem_mode = nir_intrinsic_memory_modes(intrin);
   memory_sync_info sync(aco_storage_mode_from_nir_mem_mode(mem_mode));

   LoadEmitInfo info = {Operand(v_offset), dst, num_components, elem_size_bytes, descriptor};
   info.idx = idx;
   info.glc = glc;
   info.slc = slc;
   info.soffset = s_offset;
   info.const_offset = const_offset;
   info.sync = sync;

   if (intrin->intrinsic == nir_intrinsic_load_typed_buffer_amd) {
      const pipe_format format = nir_intrinsic_format(intrin);
      const struct ac_vtx_format_info* vtx_info =
         ac_get_vtx_format_info(ctx->program->gfx_level, ctx->program->family, format);
      const struct util_format_description* f = util_format_description(format);
      const unsigned align_mul = nir_intrinsic_align_mul(intrin);
      const unsigned align_offset = nir_intrinsic_align_offset(intrin);

      /* Avoid splitting:
       * - non-array formats because that would result in incorrect code
       * - when element size is same as component size (to reduce instruction count)
       */
      const bool can_split = f->is_array && elem_size_bytes != vtx_info->chan_byte_size;

      info.align_mul = align_mul;
      info.align_offset = align_offset;
      info.format = format;
      info.component_stride = can_split ? vtx_info->chan_byte_size : 0;
      info.split_by_component_stride = false;

      emit_load(ctx, bld, info, mtbuf_load_params);
   } else {
      assert(intrin->intrinsic == nir_intrinsic_load_buffer_amd);

      if (nir_intrinsic_access(intrin) & ACCESS_USES_FORMAT_AMD) {
         assert(!swizzled);

         emit_load(ctx, bld, info, mubuf_load_format_params);
      } else {
         const unsigned swizzle_element_size =
            swizzled ? (ctx->program->gfx_level <= GFX8 ? 4 : 16) : 0;

         info.component_stride = swizzle_element_size;
         info.swizzle_component_size = swizzle_element_size ? 4 : 0;
         info.align_mul = MIN2(elem_size_bytes, 4);
         info.align_offset = 0;

         emit_load(ctx, bld, info, mubuf_load_params);
      }
   }
}

void
visit_store_buffer(isel_context* ctx, nir_intrinsic_instr* intrin)
{
   Builder bld(ctx->program, ctx->block);

   /* Swizzled buffer addressing seems to be broken on GFX11 without the idxen bit. */
   bool swizzled = nir_intrinsic_access(intrin) & ACCESS_IS_SWIZZLED_AMD;
   bool idxen = (swizzled && ctx->program->gfx_level >= GFX11) ||
                !nir_src_is_const(intrin->src[4]) || nir_src_as_uint(intrin->src[4]);
   bool v_offset_zero = nir_src_is_const(intrin->src[2]) && !nir_src_as_uint(intrin->src[2]);
   bool s_offset_zero = nir_src_is_const(intrin->src[3]) && !nir_src_as_uint(intrin->src[3]);

   Temp store_src = get_ssa_temp(ctx, intrin->src[0].ssa);
   Temp descriptor = bld.as_uniform(get_ssa_temp(ctx, intrin->src[1].ssa));
   Temp v_offset =
      v_offset_zero ? Temp(0, v1) : as_vgpr(ctx, get_ssa_temp(ctx, intrin->src[2].ssa));
   Temp s_offset =
      s_offset_zero ? Temp(0, s1) : bld.as_uniform(get_ssa_temp(ctx, intrin->src[3].ssa));
   Temp idx = idxen ? as_vgpr(ctx, get_ssa_temp(ctx, intrin->src[4].ssa)) : Temp();

   bool glc = nir_intrinsic_access(intrin) & ACCESS_COHERENT;
   bool slc = nir_intrinsic_access(intrin) & ACCESS_NON_TEMPORAL;

   unsigned const_offset = nir_intrinsic_base(intrin);
   unsigned write_mask = nir_intrinsic_write_mask(intrin);
   unsigned elem_size_bytes = intrin->src[0].ssa->bit_size / 8u;

   nir_variable_mode mem_mode = nir_intrinsic_memory_modes(intrin);
   /* GS outputs are only written once. */
   const bool written_once =
      mem_mode == nir_var_shader_out && ctx->shader->info.stage == MESA_SHADER_GEOMETRY;
   memory_sync_info sync(aco_storage_mode_from_nir_mem_mode(mem_mode),
                         written_once ? semantic_can_reorder : semantic_none);

   store_vmem_mubuf(ctx, store_src, descriptor, v_offset, s_offset, idx, const_offset,
                    elem_size_bytes, write_mask, swizzled, sync, glc, slc);
}

void
visit_load_smem(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   Temp dst = get_ssa_temp(ctx, &instr->def);
   Temp base = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));
   Temp offset = bld.as_uniform(get_ssa_temp(ctx, instr->src[1].ssa));

   /* If base address is 32bit, convert to 64bit with the high 32bit part. */
   if (base.bytes() == 4) {
      base = bld.pseudo(aco_opcode::p_create_vector, bld.def(s2), base,
                        Operand::c32(ctx->options->address32_hi));
   }

   aco_opcode opcode = aco_opcode::s_load_dword;
   unsigned size = 1;

   assert(dst.bytes() <= 64);

   if (dst.bytes() > 32) {
      opcode = aco_opcode::s_load_dwordx16;
      size = 16;
   } else if (dst.bytes() > 16) {
      opcode = aco_opcode::s_load_dwordx8;
      size = 8;
   } else if (dst.bytes() > 8) {
      opcode = aco_opcode::s_load_dwordx4;
      size = 4;
   } else if (dst.bytes() > 4) {
      opcode = aco_opcode::s_load_dwordx2;
      size = 2;
   }

   if (dst.size() != size) {
      bld.pseudo(aco_opcode::p_extract_vector, Definition(dst),
                 bld.smem(opcode, bld.def(RegType::sgpr, size), base, offset), Operand::c32(0u));
   } else {
      bld.smem(opcode, Definition(dst), base, offset);
   }
   emit_split_vector(ctx, dst, instr->def.num_components);
}

sync_scope
translate_nir_scope(mesa_scope scope)
{
   switch (scope) {
   case SCOPE_NONE:
   case SCOPE_INVOCATION: return scope_invocation;
   case SCOPE_SUBGROUP: return scope_subgroup;
   case SCOPE_WORKGROUP: return scope_workgroup;
   case SCOPE_QUEUE_FAMILY: return scope_queuefamily;
   case SCOPE_DEVICE: return scope_device;
   case SCOPE_SHADER_CALL: return scope_invocation;
   }
   unreachable("invalid scope");
}

void
emit_barrier(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);

   unsigned storage_allowed = storage_buffer | storage_image;
   unsigned semantics = 0;
   sync_scope mem_scope = translate_nir_scope(nir_intrinsic_memory_scope(instr));
   sync_scope exec_scope = translate_nir_scope(nir_intrinsic_execution_scope(instr));

   /* We use shared storage for the following:
    * - compute shaders expose it in their API
    * - when tessellation is used, TCS and VS I/O is lowered to shared memory
    * - when GS is used on GFX9+, VS->GS and TES->GS I/O is lowered to shared memory
    * - additionally, when NGG is used on GFX10+, shared memory is used for certain features
    */
   bool shared_storage_used =
      ctx->stage.hw == AC_HW_COMPUTE_SHADER || ctx->stage.hw == AC_HW_LOCAL_SHADER ||
      ctx->stage.hw == AC_HW_HULL_SHADER ||
      (ctx->stage.hw == AC_HW_LEGACY_GEOMETRY_SHADER && ctx->program->gfx_level >= GFX9) ||
      ctx->stage.hw == AC_HW_NEXT_GEN_GEOMETRY_SHADER;

   if (shared_storage_used)
      storage_allowed |= storage_shared;

   /* Task payload: Task Shader output, Mesh Shader input */
   if (ctx->stage.has(SWStage::MS) || ctx->stage.has(SWStage::TS))
      storage_allowed |= storage_task_payload;

   /* Allow VMEM output for all stages that can have outputs. */
   if ((ctx->stage.hw != AC_HW_COMPUTE_SHADER && ctx->stage.hw != AC_HW_PIXEL_SHADER) ||
       ctx->stage.has(SWStage::TS))
      storage_allowed |= storage_vmem_output;

   /* Workgroup barriers can hang merged shaders that can potentially have 0 threads in either half.
    * They are allowed in CS, TCS, and in any NGG shader.
    */
   ASSERTED bool workgroup_scope_allowed = ctx->stage.hw == AC_HW_COMPUTE_SHADER ||
                                           ctx->stage.hw == AC_HW_HULL_SHADER ||
                                           ctx->stage.hw == AC_HW_NEXT_GEN_GEOMETRY_SHADER;

   unsigned nir_storage = nir_intrinsic_memory_modes(instr);
   unsigned storage = aco_storage_mode_from_nir_mem_mode(nir_storage);
   storage &= storage_allowed;

   unsigned nir_semantics = nir_intrinsic_memory_semantics(instr);
   if (nir_semantics & NIR_MEMORY_ACQUIRE)
      semantics |= semantic_acquire | semantic_release;
   if (nir_semantics & NIR_MEMORY_RELEASE)
      semantics |= semantic_acquire | semantic_release;

   assert(!(nir_semantics & (NIR_MEMORY_MAKE_AVAILABLE | NIR_MEMORY_MAKE_VISIBLE)));
   assert(exec_scope != scope_workgroup || workgroup_scope_allowed);

   bld.barrier(aco_opcode::p_barrier,
               memory_sync_info((storage_class)storage, (memory_semantics)semantics, mem_scope),
               exec_scope);
}

void
visit_load_shared(isel_context* ctx, nir_intrinsic_instr* instr)
{
   // TODO: implement sparse reads using ds_read2_b32 and nir_def_components_read()
   Temp dst = get_ssa_temp(ctx, &instr->def);
   Temp address = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[0].ssa));
   Builder bld(ctx->program, ctx->block);

   unsigned elem_size_bytes = instr->def.bit_size / 8;
   unsigned num_components = instr->def.num_components;
   unsigned align = nir_intrinsic_align_mul(instr) ? nir_intrinsic_align(instr) : elem_size_bytes;
   load_lds(ctx, elem_size_bytes, num_components, dst, address, nir_intrinsic_base(instr), align);
}

void
visit_store_shared(isel_context* ctx, nir_intrinsic_instr* instr)
{
   unsigned writemask = nir_intrinsic_write_mask(instr);
   Temp data = get_ssa_temp(ctx, instr->src[0].ssa);
   Temp address = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[1].ssa));
   unsigned elem_size_bytes = instr->src[0].ssa->bit_size / 8;

   unsigned align = nir_intrinsic_align_mul(instr) ? nir_intrinsic_align(instr) : elem_size_bytes;
   store_lds(ctx, elem_size_bytes, data, writemask, address, nir_intrinsic_base(instr), align);
}

void
visit_shared_atomic(isel_context* ctx, nir_intrinsic_instr* instr)
{
   unsigned offset = nir_intrinsic_base(instr);
   Builder bld(ctx->program, ctx->block);
   Operand m = load_lds_size_m0(bld);
   Temp data = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[1].ssa));
   Temp address = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[0].ssa));

   unsigned num_operands = 3;
   aco_opcode op32, op64, op32_rtn, op64_rtn;
   switch (nir_intrinsic_atomic_op(instr)) {
   case nir_atomic_op_iadd:
      op32 = aco_opcode::ds_add_u32;
      op64 = aco_opcode::ds_add_u64;
      op32_rtn = aco_opcode::ds_add_rtn_u32;
      op64_rtn = aco_opcode::ds_add_rtn_u64;
      break;
   case nir_atomic_op_imin:
      op32 = aco_opcode::ds_min_i32;
      op64 = aco_opcode::ds_min_i64;
      op32_rtn = aco_opcode::ds_min_rtn_i32;
      op64_rtn = aco_opcode::ds_min_rtn_i64;
      break;
   case nir_atomic_op_umin:
      op32 = aco_opcode::ds_min_u32;
      op64 = aco_opcode::ds_min_u64;
      op32_rtn = aco_opcode::ds_min_rtn_u32;
      op64_rtn = aco_opcode::ds_min_rtn_u64;
      break;
   case nir_atomic_op_imax:
      op32 = aco_opcode::ds_max_i32;
      op64 = aco_opcode::ds_max_i64;
      op32_rtn = aco_opcode::ds_max_rtn_i32;
      op64_rtn = aco_opcode::ds_max_rtn_i64;
      break;
   case nir_atomic_op_umax:
      op32 = aco_opcode::ds_max_u32;
      op64 = aco_opcode::ds_max_u64;
      op32_rtn = aco_opcode::ds_max_rtn_u32;
      op64_rtn = aco_opcode::ds_max_rtn_u64;
      break;
   case nir_atomic_op_iand:
      op32 = aco_opcode::ds_and_b32;
      op64 = aco_opcode::ds_and_b64;
      op32_rtn = aco_opcode::ds_and_rtn_b32;
      op64_rtn = aco_opcode::ds_and_rtn_b64;
      break;
   case nir_atomic_op_ior:
      op32 = aco_opcode::ds_or_b32;
      op64 = aco_opcode::ds_or_b64;
      op32_rtn = aco_opcode::ds_or_rtn_b32;
      op64_rtn = aco_opcode::ds_or_rtn_b64;
      break;
   case nir_atomic_op_ixor:
      op32 = aco_opcode::ds_xor_b32;
      op64 = aco_opcode::ds_xor_b64;
      op32_rtn = aco_opcode::ds_xor_rtn_b32;
      op64_rtn = aco_opcode::ds_xor_rtn_b64;
      break;
   case nir_atomic_op_xchg:
      op32 = aco_opcode::ds_write_b32;
      op64 = aco_opcode::ds_write_b64;
      op32_rtn = aco_opcode::ds_wrxchg_rtn_b32;
      op64_rtn = aco_opcode::ds_wrxchg_rtn_b64;
      break;
   case nir_atomic_op_cmpxchg:
      op32 = aco_opcode::ds_cmpst_b32;
      op64 = aco_opcode::ds_cmpst_b64;
      op32_rtn = aco_opcode::ds_cmpst_rtn_b32;
      op64_rtn = aco_opcode::ds_cmpst_rtn_b64;
      num_operands = 4;
      break;
   case nir_atomic_op_fadd:
      op32 = aco_opcode::ds_add_f32;
      op32_rtn = aco_opcode::ds_add_rtn_f32;
      op64 = aco_opcode::num_opcodes;
      op64_rtn = aco_opcode::num_opcodes;
      break;
   case nir_atomic_op_fmin:
      op32 = aco_opcode::ds_min_f32;
      op32_rtn = aco_opcode::ds_min_rtn_f32;
      op64 = aco_opcode::ds_min_f64;
      op64_rtn = aco_opcode::ds_min_rtn_f64;
      break;
   case nir_atomic_op_fmax:
      op32 = aco_opcode::ds_max_f32;
      op32_rtn = aco_opcode::ds_max_rtn_f32;
      op64 = aco_opcode::ds_max_f64;
      op64_rtn = aco_opcode::ds_max_rtn_f64;
      break;
   default: unreachable("Unhandled shared atomic intrinsic");
   }

   bool return_previous = !nir_def_is_unused(&instr->def);

   aco_opcode op;
   if (data.size() == 1) {
      assert(instr->def.bit_size == 32);
      op = return_previous ? op32_rtn : op32;
   } else {
      assert(instr->def.bit_size == 64);
      op = return_previous ? op64_rtn : op64;
   }

   if (offset > 65535) {
      address = bld.vadd32(bld.def(v1), Operand::c32(offset), address);
      offset = 0;
   }

   aco_ptr<DS_instruction> ds;
   ds.reset(
      create_instruction<DS_instruction>(op, Format::DS, num_operands, return_previous ? 1 : 0));
   ds->operands[0] = Operand(address);
   ds->operands[1] = Operand(data);
   if (num_operands == 4) {
      Temp data2 = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[2].ssa));
      ds->operands[2] = Operand(data2);
      if (bld.program->gfx_level >= GFX11)
         std::swap(ds->operands[1], ds->operands[2]);
   }
   ds->operands[num_operands - 1] = m;
   ds->offset0 = offset;
   if (return_previous)
      ds->definitions[0] = Definition(get_ssa_temp(ctx, &instr->def));
   ds->sync = memory_sync_info(storage_shared, semantic_atomicrmw);

   if (m.isUndefined())
      ds->operands.pop_back();

   ctx->block->instructions.emplace_back(std::move(ds));
}

void
visit_access_shared2_amd(isel_context* ctx, nir_intrinsic_instr* instr)
{
   bool is_store = instr->intrinsic == nir_intrinsic_store_shared2_amd;
   Temp address = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[is_store].ssa));
   Builder bld(ctx->program, ctx->block);

   assert(bld.program->gfx_level >= GFX7);

   bool is64bit = (is_store ? instr->src[0].ssa->bit_size : instr->def.bit_size) == 64;
   uint8_t offset0 = nir_intrinsic_offset0(instr);
   uint8_t offset1 = nir_intrinsic_offset1(instr);
   bool st64 = nir_intrinsic_st64(instr);

   Operand m = load_lds_size_m0(bld);
   Instruction* ds;
   if (is_store) {
      aco_opcode op = st64
                         ? (is64bit ? aco_opcode::ds_write2st64_b64 : aco_opcode::ds_write2st64_b32)
                         : (is64bit ? aco_opcode::ds_write2_b64 : aco_opcode::ds_write2_b32);
      Temp data = get_ssa_temp(ctx, instr->src[0].ssa);
      RegClass comp_rc = is64bit ? v2 : v1;
      Temp data0 = emit_extract_vector(ctx, data, 0, comp_rc);
      Temp data1 = emit_extract_vector(ctx, data, 1, comp_rc);
      ds = bld.ds(op, address, data0, data1, m, offset0, offset1);
   } else {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      Definition tmp_dst(dst.type() == RegType::vgpr ? dst : bld.tmp(is64bit ? v4 : v2));
      aco_opcode op = st64 ? (is64bit ? aco_opcode::ds_read2st64_b64 : aco_opcode::ds_read2st64_b32)
                           : (is64bit ? aco_opcode::ds_read2_b64 : aco_opcode::ds_read2_b32);
      ds = bld.ds(op, tmp_dst, address, m, offset0, offset1);
   }
   ds->ds().sync = memory_sync_info(storage_shared);
   if (m.isUndefined())
      ds->operands.pop_back();

   if (!is_store) {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      if (dst.type() == RegType::sgpr) {
         emit_split_vector(ctx, ds->definitions[0].getTemp(), dst.size());
         Temp comp[4];
         /* Use scalar v_readfirstlane_b32 for better 32-bit copy propagation */
         for (unsigned i = 0; i < dst.size(); i++)
            comp[i] = bld.as_uniform(emit_extract_vector(ctx, ds->definitions[0].getTemp(), i, v1));
         if (is64bit) {
            Temp comp0 = bld.pseudo(aco_opcode::p_create_vector, bld.def(s2), comp[0], comp[1]);
            Temp comp1 = bld.pseudo(aco_opcode::p_create_vector, bld.def(s2), comp[2], comp[3]);
            ctx->allocated_vec[comp0.id()] = {comp[0], comp[1]};
            ctx->allocated_vec[comp1.id()] = {comp[2], comp[3]};
            bld.pseudo(aco_opcode::p_create_vector, Definition(dst), comp0, comp1);
            ctx->allocated_vec[dst.id()] = {comp0, comp1};
         } else {
            bld.pseudo(aco_opcode::p_create_vector, Definition(dst), comp[0], comp[1]);
         }
      }

      emit_split_vector(ctx, dst, 2);
   }
}

Temp
get_scratch_resource(isel_context* ctx)
{
   Builder bld(ctx->program, ctx->block);
   Temp scratch_addr = ctx->program->private_segment_buffer;
   if (!scratch_addr.bytes()) {
      Temp addr_lo =
         bld.sop1(aco_opcode::p_load_symbol, bld.def(s1), Operand::c32(aco_symbol_scratch_addr_lo));
      Temp addr_hi =
         bld.sop1(aco_opcode::p_load_symbol, bld.def(s1), Operand::c32(aco_symbol_scratch_addr_hi));
      scratch_addr = bld.pseudo(aco_opcode::p_create_vector, bld.def(s2), addr_lo, addr_hi);
   } else if (ctx->stage.hw != AC_HW_COMPUTE_SHADER) {
      scratch_addr =
         bld.smem(aco_opcode::s_load_dwordx2, bld.def(s2), scratch_addr, Operand::zero());
   }

   uint32_t rsrc_conf =
      S_008F0C_ADD_TID_ENABLE(1) | S_008F0C_INDEX_STRIDE(ctx->program->wave_size == 64 ? 3 : 2);

   if (ctx->program->gfx_level >= GFX10) {
      rsrc_conf |= S_008F0C_FORMAT(V_008F0C_GFX10_FORMAT_32_FLOAT) |
                   S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_RAW) |
                   S_008F0C_RESOURCE_LEVEL(ctx->program->gfx_level < GFX11);
   } else if (ctx->program->gfx_level <=
              GFX7) { /* dfmt modifies stride on GFX8/GFX9 when ADD_TID_EN=1 */
      rsrc_conf |= S_008F0C_NUM_FORMAT(V_008F0C_BUF_NUM_FORMAT_FLOAT) |
                   S_008F0C_DATA_FORMAT(V_008F0C_BUF_DATA_FORMAT_32);
   }

   /* older generations need element size = 4 bytes. element size removed in GFX9 */
   if (ctx->program->gfx_level <= GFX8)
      rsrc_conf |= S_008F0C_ELEMENT_SIZE(1);

   return bld.pseudo(aco_opcode::p_create_vector, bld.def(s4), scratch_addr, Operand::c32(-1u),
                     Operand::c32(rsrc_conf));
}

void
visit_load_scratch(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   Temp dst = get_ssa_temp(ctx, &instr->def);

   LoadEmitInfo info = {Operand(v1), dst, instr->def.num_components, instr->def.bit_size / 8u};
   info.align_mul = nir_intrinsic_align_mul(instr);
   info.align_offset = nir_intrinsic_align_offset(instr);
   info.swizzle_component_size = ctx->program->gfx_level <= GFX8 ? 4 : 0;
   info.sync = memory_sync_info(storage_scratch, semantic_private);
   if (ctx->program->gfx_level >= GFX9) {
      if (nir_src_is_const(instr->src[0])) {
         uint32_t max = ctx->program->dev.scratch_global_offset_max + 1;
         info.offset =
            bld.copy(bld.def(s1), Operand::c32(ROUND_DOWN_TO(nir_src_as_uint(instr->src[0]), max)));
         info.const_offset = nir_src_as_uint(instr->src[0]) % max;
      } else {
         info.offset = Operand(get_ssa_temp(ctx, instr->src[0].ssa));
      }
      EmitLoadParameters params = scratch_flat_load_params;
      params.max_const_offset_plus_one = ctx->program->dev.scratch_global_offset_max + 1;
      emit_load(ctx, bld, info, params);
   } else {
      info.resource = get_scratch_resource(ctx);
      info.offset = Operand(as_vgpr(ctx, get_ssa_temp(ctx, instr->src[0].ssa)));
      info.soffset = ctx->program->scratch_offset;
      emit_load(ctx, bld, info, scratch_mubuf_load_params);
   }
}

void
visit_store_scratch(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   Temp data = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[0].ssa));
   Temp offset = get_ssa_temp(ctx, instr->src[1].ssa);

   unsigned elem_size_bytes = instr->src[0].ssa->bit_size / 8;
   unsigned writemask = util_widen_mask(nir_intrinsic_write_mask(instr), elem_size_bytes);

   unsigned write_count = 0;
   Temp write_datas[32];
   unsigned offsets[32];
   unsigned swizzle_component_size = ctx->program->gfx_level <= GFX8 ? 4 : 16;
   split_buffer_store(ctx, instr, false, RegType::vgpr, data, writemask, swizzle_component_size,
                      &write_count, write_datas, offsets);

   if (ctx->program->gfx_level >= GFX9) {
      uint32_t max = ctx->program->dev.scratch_global_offset_max + 1;
      offset = nir_src_is_const(instr->src[1]) ? Temp(0, s1) : offset;
      uint32_t base_const_offset =
         nir_src_is_const(instr->src[1]) ? nir_src_as_uint(instr->src[1]) : 0;

      for (unsigned i = 0; i < write_count; i++) {
         aco_opcode op;
         switch (write_datas[i].bytes()) {
         case 1: op = aco_opcode::scratch_store_byte; break;
         case 2: op = aco_opcode::scratch_store_short; break;
         case 4: op = aco_opcode::scratch_store_dword; break;
         case 8: op = aco_opcode::scratch_store_dwordx2; break;
         case 12: op = aco_opcode::scratch_store_dwordx3; break;
         case 16: op = aco_opcode::scratch_store_dwordx4; break;
         default: unreachable("Unexpected store size");
         }

         uint32_t const_offset = base_const_offset + offsets[i];
         assert(const_offset < max || offset.id() == 0);

         Operand addr = offset.regClass() == s1 ? Operand(v1) : Operand(offset);
         Operand saddr = offset.regClass() == s1 ? Operand(offset) : Operand(s1);
         if (offset.id() == 0)
            saddr = bld.copy(bld.def(s1), Operand::c32(ROUND_DOWN_TO(const_offset, max)));

         bld.scratch(op, addr, saddr, write_datas[i], const_offset % max,
                     memory_sync_info(storage_scratch, semantic_private));
      }
   } else {
      Temp rsrc = get_scratch_resource(ctx);
      offset = as_vgpr(ctx, offset);
      for (unsigned i = 0; i < write_count; i++) {
         aco_opcode op = get_buffer_store_op(write_datas[i].bytes());
         Instruction* mubuf = bld.mubuf(op, rsrc, offset, ctx->program->scratch_offset,
                                        write_datas[i], offsets[i], true, true);
         mubuf->mubuf().sync = memory_sync_info(storage_scratch, semantic_private);
      }
   }
}

ReduceOp
get_reduce_op(nir_op op, unsigned bit_size)
{
   switch (op) {
#define CASEI(name)                                                                                \
   case nir_op_##name:                                                                             \
      return (bit_size == 32)   ? name##32                                                         \
             : (bit_size == 16) ? name##16                                                         \
             : (bit_size == 8)  ? name##8                                                          \
                                : name##64;
#define CASEF(name)                                                                                \
   case nir_op_##name: return (bit_size == 32) ? name##32 : (bit_size == 16) ? name##16 : name##64;
      CASEI(iadd)
      CASEI(imul)
      CASEI(imin)
      CASEI(umin)
      CASEI(imax)
      CASEI(umax)
      CASEI(iand)
      CASEI(ior)
      CASEI(ixor)
      CASEF(fadd)
      CASEF(fmul)
      CASEF(fmin)
      CASEF(fmax)
   default: unreachable("unknown reduction op");
#undef CASEI
#undef CASEF
   }
}

void
emit_uniform_subgroup(isel_context* ctx, nir_intrinsic_instr* instr, Temp src)
{
   Builder bld(ctx->program, ctx->block);
   Definition dst(get_ssa_temp(ctx, &instr->def));
   assert(dst.regClass().type() != RegType::vgpr);
   if (src.regClass().type() == RegType::vgpr)
      bld.pseudo(aco_opcode::p_as_uniform, dst, src);
   else
      bld.copy(dst, src);
}

void
emit_addition_uniform_reduce(isel_context* ctx, nir_op op, Definition dst, nir_src src, Temp count)
{
   Builder bld(ctx->program, ctx->block);
   Temp src_tmp = get_ssa_temp(ctx, src.ssa);

   if (op == nir_op_fadd) {
      src_tmp = as_vgpr(ctx, src_tmp);
      Temp tmp = dst.regClass() == s1 ? bld.tmp(RegClass::get(RegType::vgpr, src.ssa->bit_size / 8))
                                      : dst.getTemp();

      if (src.ssa->bit_size == 16) {
         count = bld.vop1(aco_opcode::v_cvt_f16_u16, bld.def(v2b), count);
         bld.vop2(aco_opcode::v_mul_f16, Definition(tmp), count, src_tmp);
      } else {
         assert(src.ssa->bit_size == 32);
         count = bld.vop1(aco_opcode::v_cvt_f32_u32, bld.def(v1), count);
         bld.vop2(aco_opcode::v_mul_f32, Definition(tmp), count, src_tmp);
      }

      if (tmp != dst.getTemp())
         bld.pseudo(aco_opcode::p_as_uniform, dst, tmp);

      return;
   }

   if (dst.regClass() == s1)
      src_tmp = bld.as_uniform(src_tmp);

   if (op == nir_op_ixor && count.type() == RegType::sgpr)
      count =
         bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc), count, Operand::c32(1u));
   else if (op == nir_op_ixor)
      count = bld.vop2(aco_opcode::v_and_b32, bld.def(v1), Operand::c32(1u), count);

   assert(dst.getTemp().type() == count.type());

   if (nir_src_is_const(src)) {
      if (nir_src_as_uint(src) == 1 && dst.bytes() <= 2)
         bld.pseudo(aco_opcode::p_extract_vector, dst, count, Operand::zero());
      else if (nir_src_as_uint(src) == 1)
         bld.copy(dst, count);
      else if (nir_src_as_uint(src) == 0)
         bld.copy(dst, Operand::zero(dst.bytes()));
      else if (count.type() == RegType::vgpr)
         bld.v_mul_imm(dst, count, nir_src_as_uint(src));
      else
         bld.sop2(aco_opcode::s_mul_i32, dst, src_tmp, count);
   } else if (dst.bytes() <= 2 && ctx->program->gfx_level >= GFX10) {
      bld.vop3(aco_opcode::v_mul_lo_u16_e64, dst, src_tmp, count);
   } else if (dst.bytes() <= 2 && ctx->program->gfx_level >= GFX8) {
      bld.vop2(aco_opcode::v_mul_lo_u16, dst, src_tmp, count);
   } else if (dst.getTemp().type() == RegType::vgpr) {
      bld.vop3(aco_opcode::v_mul_lo_u32, dst, src_tmp, count);
   } else {
      bld.sop2(aco_opcode::s_mul_i32, dst, src_tmp, count);
   }
}

bool
emit_uniform_reduce(isel_context* ctx, nir_intrinsic_instr* instr)
{
   nir_op op = (nir_op)nir_intrinsic_reduction_op(instr);
   if (op == nir_op_imul || op == nir_op_fmul)
      return false;

   if (op == nir_op_iadd || op == nir_op_ixor || op == nir_op_fadd) {
      Builder bld(ctx->program, ctx->block);
      Definition dst(get_ssa_temp(ctx, &instr->def));
      unsigned bit_size = instr->src[0].ssa->bit_size;
      if (bit_size > 32)
         return false;

      Temp thread_count =
         bld.sop1(Builder::s_bcnt1_i32, bld.def(s1), bld.def(s1, scc), Operand(exec, bld.lm));
      set_wqm(ctx);

      emit_addition_uniform_reduce(ctx, op, dst, instr->src[0], thread_count);
   } else {
      emit_uniform_subgroup(ctx, instr, get_ssa_temp(ctx, instr->src[0].ssa));
   }

   return true;
}

bool
emit_uniform_scan(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   Definition dst(get_ssa_temp(ctx, &instr->def));
   nir_op op = (nir_op)nir_intrinsic_reduction_op(instr);
   bool inc = instr->intrinsic == nir_intrinsic_inclusive_scan;

   if (op == nir_op_imul || op == nir_op_fmul)
      return false;

   if (op == nir_op_iadd || op == nir_op_ixor || op == nir_op_fadd) {
      if (instr->src[0].ssa->bit_size > 32)
         return false;

      Temp packed_tid;
      if (inc)
         packed_tid = emit_mbcnt(ctx, bld.tmp(v1), Operand(exec, bld.lm), Operand::c32(1u));
      else
         packed_tid = emit_mbcnt(ctx, bld.tmp(v1), Operand(exec, bld.lm));
      set_wqm(ctx);

      emit_addition_uniform_reduce(ctx, op, dst, instr->src[0], packed_tid);
      return true;
   }

   assert(op == nir_op_imin || op == nir_op_umin || op == nir_op_imax || op == nir_op_umax ||
          op == nir_op_iand || op == nir_op_ior || op == nir_op_fmin || op == nir_op_fmax);

   if (inc) {
      emit_uniform_subgroup(ctx, instr, get_ssa_temp(ctx, instr->src[0].ssa));
      return true;
   }

   /* Copy the source and write the reduction operation identity to the first lane. */
   Temp lane = bld.sop1(Builder::s_ff1_i32, bld.def(s1), Operand(exec, bld.lm));
   Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
   ReduceOp reduce_op = get_reduce_op(op, instr->src[0].ssa->bit_size);
   if (dst.bytes() == 8) {
      Temp lo = bld.tmp(v1), hi = bld.tmp(v1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(lo), Definition(hi), src);
      uint32_t identity_lo = get_reduction_identity(reduce_op, 0);
      uint32_t identity_hi = get_reduction_identity(reduce_op, 1);

      lo =
         bld.writelane(bld.def(v1), bld.copy(bld.def(s1, m0), Operand::c32(identity_lo)), lane, lo);
      hi =
         bld.writelane(bld.def(v1), bld.copy(bld.def(s1, m0), Operand::c32(identity_hi)), lane, hi);
      bld.pseudo(aco_opcode::p_create_vector, dst, lo, hi);
   } else {
      uint32_t identity = get_reduction_identity(reduce_op, 0);
      bld.writelane(dst, bld.copy(bld.def(s1, m0), Operand::c32(identity)), lane,
                    as_vgpr(ctx, src));
   }

   set_wqm(ctx);
   return true;
}

Temp
emit_reduction_instr(isel_context* ctx, aco_opcode aco_op, ReduceOp op, unsigned cluster_size,
                     Definition dst, Temp src)
{
   assert(src.bytes() <= 8);
   assert(src.type() == RegType::vgpr);

   Builder bld(ctx->program, ctx->block);

   unsigned num_defs = 0;
   Definition defs[5];
   defs[num_defs++] = dst;
   defs[num_defs++] = bld.def(bld.lm); /* used internally to save/restore exec */

   /* scalar identity temporary */
   bool need_sitmp = (ctx->program->gfx_level <= GFX7 || ctx->program->gfx_level >= GFX10) &&
                     aco_op != aco_opcode::p_reduce;
   if (aco_op == aco_opcode::p_exclusive_scan) {
      need_sitmp |= (op == imin8 || op == imin16 || op == imin32 || op == imin64 || op == imax8 ||
                     op == imax16 || op == imax32 || op == imax64 || op == fmin16 || op == fmin32 ||
                     op == fmin64 || op == fmax16 || op == fmax32 || op == fmax64 || op == fmul16 ||
                     op == fmul64);
   }
   if (need_sitmp)
      defs[num_defs++] = bld.def(RegType::sgpr, dst.size());

   /* scc clobber */
   defs[num_defs++] = bld.def(s1, scc);

   /* vcc clobber */
   bool clobber_vcc = false;
   if ((op == iadd32 || op == imul64) && ctx->program->gfx_level < GFX9)
      clobber_vcc = true;
   if ((op == iadd8 || op == iadd16) && ctx->program->gfx_level < GFX8)
      clobber_vcc = true;
   if (op == iadd64 || op == umin64 || op == umax64 || op == imin64 || op == imax64)
      clobber_vcc = true;

   if (clobber_vcc)
      defs[num_defs++] = bld.def(bld.lm, vcc);

   Pseudo_reduction_instruction* reduce = create_instruction<Pseudo_reduction_instruction>(
      aco_op, Format::PSEUDO_REDUCTION, 3, num_defs);
   reduce->operands[0] = Operand(src);
   /* setup_reduce_temp will update these undef operands if needed */
   reduce->operands[1] = Operand(RegClass(RegType::vgpr, dst.size()).as_linear());
   reduce->operands[2] = Operand(v1.as_linear());
   std::copy(defs, defs + num_defs, reduce->definitions.begin());

   reduce->reduce_op = op;
   reduce->cluster_size = cluster_size;
   bld.insert(std::move(reduce));

   return dst.getTemp();
}

Temp
inclusive_scan_to_exclusive(isel_context* ctx, ReduceOp op, Definition dst, Temp src)
{
   Builder bld(ctx->program, ctx->block);

   Temp scan = emit_reduction_instr(ctx, aco_opcode::p_inclusive_scan, op, ctx->program->wave_size,
                                    bld.def(dst.regClass()), src);

   switch (op) {
   case iadd8:
   case iadd16:
   case iadd32: return bld.vsub32(dst, scan, src);
   case ixor64:
   case iadd64: {
      Temp src00 = bld.tmp(v1);
      Temp src01 = bld.tmp(v1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src00), Definition(src01), scan);
      Temp src10 = bld.tmp(v1);
      Temp src11 = bld.tmp(v1);
      bld.pseudo(aco_opcode::p_split_vector, Definition(src10), Definition(src11), src);

      Temp lower = bld.tmp(v1);
      Temp upper = bld.tmp(v1);
      if (op == iadd64) {
         Temp borrow = bld.vsub32(Definition(lower), src00, src10, true).def(1).getTemp();
         bld.vsub32(Definition(upper), src01, src11, false, borrow);
      } else {
         bld.vop2(aco_opcode::v_xor_b32, Definition(lower), src00, src10);
         bld.vop2(aco_opcode::v_xor_b32, Definition(upper), src01, src11);
      }
      return bld.pseudo(aco_opcode::p_create_vector, dst, lower, upper);
   }
   case ixor8:
   case ixor16:
   case ixor32: return bld.vop2(aco_opcode::v_xor_b32, dst, scan, src);
   default: unreachable("Unsupported op");
   }
}

void
emit_interp_center(isel_context* ctx, Temp dst, Temp bary, Temp pos1, Temp pos2)
{
   Builder bld(ctx->program, ctx->block);
   Temp p1 = emit_extract_vector(ctx, bary, 0, v1);
   Temp p2 = emit_extract_vector(ctx, bary, 1, v1);

   Temp ddx_1, ddx_2, ddy_1, ddy_2;
   uint32_t dpp_ctrl0 = dpp_quad_perm(0, 0, 0, 0);
   uint32_t dpp_ctrl1 = dpp_quad_perm(1, 1, 1, 1);
   uint32_t dpp_ctrl2 = dpp_quad_perm(2, 2, 2, 2);

   /* Build DD X/Y */
   if (ctx->program->gfx_level >= GFX8) {
      Temp tl_1 = bld.vop1_dpp(aco_opcode::v_mov_b32, bld.def(v1), p1, dpp_ctrl0);
      ddx_1 = bld.vop2_dpp(aco_opcode::v_sub_f32, bld.def(v1), p1, tl_1, dpp_ctrl1);
      ddy_1 = bld.vop2_dpp(aco_opcode::v_sub_f32, bld.def(v1), p1, tl_1, dpp_ctrl2);
      Temp tl_2 = bld.vop1_dpp(aco_opcode::v_mov_b32, bld.def(v1), p2, dpp_ctrl0);
      ddx_2 = bld.vop2_dpp(aco_opcode::v_sub_f32, bld.def(v1), p2, tl_2, dpp_ctrl1);
      ddy_2 = bld.vop2_dpp(aco_opcode::v_sub_f32, bld.def(v1), p2, tl_2, dpp_ctrl2);
   } else {
      Temp tl_1 = bld.ds(aco_opcode::ds_swizzle_b32, bld.def(v1), p1, (1 << 15) | dpp_ctrl0);
      ddx_1 = bld.ds(aco_opcode::ds_swizzle_b32, bld.def(v1), p1, (1 << 15) | dpp_ctrl1);
      ddx_1 = bld.vop2(aco_opcode::v_sub_f32, bld.def(v1), ddx_1, tl_1);
      ddy_1 = bld.ds(aco_opcode::ds_swizzle_b32, bld.def(v1), p1, (1 << 15) | dpp_ctrl2);
      ddy_1 = bld.vop2(aco_opcode::v_sub_f32, bld.def(v1), ddy_1, tl_1);

      Temp tl_2 = bld.ds(aco_opcode::ds_swizzle_b32, bld.def(v1), p2, (1 << 15) | dpp_ctrl0);
      ddx_2 = bld.ds(aco_opcode::ds_swizzle_b32, bld.def(v1), p2, (1 << 15) | dpp_ctrl1);
      ddx_2 = bld.vop2(aco_opcode::v_sub_f32, bld.def(v1), ddx_2, tl_2);
      ddy_2 = bld.ds(aco_opcode::ds_swizzle_b32, bld.def(v1), p2, (1 << 15) | dpp_ctrl2);
      ddy_2 = bld.vop2(aco_opcode::v_sub_f32, bld.def(v1), ddy_2, tl_2);
   }

   /* res_k = p_k + ddx_k * pos1 + ddy_k * pos2 */
   aco_opcode mad =
      ctx->program->gfx_level >= GFX10_3 ? aco_opcode::v_fma_f32 : aco_opcode::v_mad_f32;
   Temp tmp1 = bld.vop3(mad, bld.def(v1), ddx_1, pos1, p1);
   Temp tmp2 = bld.vop3(mad, bld.def(v1), ddx_2, pos1, p2);
   tmp1 = bld.vop3(mad, bld.def(v1), ddy_1, pos2, tmp1);
   tmp2 = bld.vop3(mad, bld.def(v1), ddy_2, pos2, tmp2);
   bld.pseudo(aco_opcode::p_create_vector, Definition(dst), tmp1, tmp2);
   set_wqm(ctx, true);
   return;
}

Temp merged_wave_info_to_mask(isel_context* ctx, unsigned i);
Temp lanecount_to_mask(isel_context* ctx, Temp count);
void pops_await_overlapped_waves(isel_context* ctx);

Temp
get_interp_param(isel_context* ctx, nir_intrinsic_op intrin, enum glsl_interp_mode interp)
{
   bool linear = interp == INTERP_MODE_NOPERSPECTIVE;
   if (intrin == nir_intrinsic_load_barycentric_pixel ||
       intrin == nir_intrinsic_load_barycentric_at_offset) {
      return get_arg(ctx, linear ? ctx->args->linear_center : ctx->args->persp_center);
   } else if (intrin == nir_intrinsic_load_barycentric_centroid) {
      return get_arg(ctx, linear ? ctx->args->linear_centroid : ctx->args->persp_centroid);
   } else {
      assert(intrin == nir_intrinsic_load_barycentric_sample);
      return get_arg(ctx, linear ? ctx->args->linear_sample : ctx->args->persp_sample);
   }
}

void
ds_ordered_count_offsets(isel_context* ctx, unsigned index_operand, unsigned wave_release,
                         unsigned wave_done, unsigned* offset0, unsigned* offset1)
{
   unsigned ordered_count_index = index_operand & 0x3f;
   unsigned count_dword = (index_operand >> 24) & 0xf;

   assert(ctx->options->gfx_level >= GFX10);
   assert(count_dword >= 1 && count_dword <= 4);

   *offset0 = ordered_count_index << 2;
   *offset1 = wave_release | (wave_done << 1) | ((count_dword - 1) << 6);

   if (ctx->options->gfx_level < GFX11)
      *offset1 |= 3 /* GS shader type */ << 2;
}

struct aco_export_mrt {
   Operand out[4];
   unsigned enabled_channels;
   unsigned target;
   bool compr;
};

static void
create_fs_dual_src_export_gfx11(isel_context* ctx, const struct aco_export_mrt* mrt0,
                                const struct aco_export_mrt* mrt1)
{
   Builder bld(ctx->program, ctx->block);

   aco_ptr<Pseudo_instruction> exp{create_instruction<Pseudo_instruction>(
      aco_opcode::p_dual_src_export_gfx11, Format::PSEUDO, 8, 6)};
   for (unsigned i = 0; i < 4; i++) {
      exp->operands[i] = mrt0 ? mrt0->out[i] : Operand(v1);
      exp->operands[i].setLateKill(true);
      exp->operands[i + 4] = mrt1 ? mrt1->out[i] : Operand(v1);
      exp->operands[i + 4].setLateKill(true);
   }

   RegClass type = RegClass(RegType::vgpr, util_bitcount(mrt0->enabled_channels));
   exp->definitions[0] = bld.def(type); /* mrt0 */
   exp->definitions[1] = bld.def(type); /* mrt1 */
   exp->definitions[2] = bld.def(bld.lm);
   exp->definitions[3] = bld.def(bld.lm);
   exp->definitions[4] = bld.def(bld.lm, vcc);
   exp->definitions[5] = bld.def(s1, scc);
   ctx->block->instructions.emplace_back(std::move(exp));

   ctx->program->has_color_exports = true;
}

static void
visit_cmat_muladd(isel_context* ctx, nir_intrinsic_instr* instr)
{
   aco_opcode opcode = aco_opcode::num_opcodes;
   unsigned signed_mask = 0;
   bool clamp = false;

   switch (instr->src[0].ssa->bit_size) {
   case 16:
      switch (instr->def.bit_size) {
      case 32: opcode = aco_opcode::v_wmma_f32_16x16x16_f16; break;
      case 16: opcode = aco_opcode::v_wmma_f16_16x16x16_f16; break;
      }
      break;
   case 8:
      opcode = aco_opcode::v_wmma_i32_16x16x16_iu8;
      signed_mask = nir_intrinsic_cmat_signed_mask(instr);
      clamp = nir_intrinsic_saturate(instr);
      break;
   }

   if (opcode == aco_opcode::num_opcodes)
      unreachable("visit_cmat_muladd: invalid bit size combination");

   Builder bld(ctx->program, ctx->block);

   Temp dst = get_ssa_temp(ctx, &instr->def);
   Operand A(as_vgpr(ctx, get_ssa_temp(ctx, instr->src[0].ssa)));
   Operand B(as_vgpr(ctx, get_ssa_temp(ctx, instr->src[1].ssa)));
   Operand C(as_vgpr(ctx, get_ssa_temp(ctx, instr->src[2].ssa)));

   A.setLateKill(true);
   B.setLateKill(true);

   VALU_instruction& vop3p = bld.vop3p(opcode, Definition(dst), A, B, C, 0, 0)->valu();
   vop3p.neg_lo[0] = (signed_mask & 0x1) != 0;
   vop3p.neg_lo[1] = (signed_mask & 0x2) != 0;
   vop3p.clamp = clamp;

   emit_split_vector(ctx, dst, instr->def.num_components);
}

void
visit_intrinsic(isel_context* ctx, nir_intrinsic_instr* instr)
{
   Builder bld(ctx->program, ctx->block);
   switch (instr->intrinsic) {
   case nir_intrinsic_load_barycentric_sample:
   case nir_intrinsic_load_barycentric_pixel:
   case nir_intrinsic_load_barycentric_centroid: {
      glsl_interp_mode mode = (glsl_interp_mode)nir_intrinsic_interp_mode(instr);
      Temp bary = get_interp_param(ctx, instr->intrinsic, mode);
      assert(bary.size() == 2);
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.copy(Definition(dst), bary);
      emit_split_vector(ctx, dst, 2);
      break;
   }
   case nir_intrinsic_load_barycentric_model: {
      Temp model = get_arg(ctx, ctx->args->pull_model);
      assert(model.size() == 3);
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.copy(Definition(dst), model);
      emit_split_vector(ctx, dst, 3);
      break;
   }
   case nir_intrinsic_load_barycentric_at_offset: {
      Temp offset = get_ssa_temp(ctx, instr->src[0].ssa);
      RegClass rc = RegClass(offset.type(), 1);
      Temp pos1 = bld.tmp(rc), pos2 = bld.tmp(rc);
      bld.pseudo(aco_opcode::p_split_vector, Definition(pos1), Definition(pos2), offset);
      Temp bary = get_interp_param(ctx, instr->intrinsic,
                                   (glsl_interp_mode)nir_intrinsic_interp_mode(instr));
      emit_interp_center(ctx, get_ssa_temp(ctx, &instr->def), bary, pos1, pos2);
      break;
   }
   case nir_intrinsic_load_front_face: {
      bld.vopc(aco_opcode::v_cmp_lg_u32, Definition(get_ssa_temp(ctx, &instr->def)),
               Operand::zero(), get_arg(ctx, ctx->args->front_face));
      break;
   }
   case nir_intrinsic_load_view_index: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.copy(Definition(dst), Operand(get_arg(ctx, ctx->args->view_index)));
      break;
   }
   case nir_intrinsic_load_frag_coord: {
      emit_load_frag_coord(ctx, get_ssa_temp(ctx, &instr->def), 4);
      break;
   }
   case nir_intrinsic_load_frag_shading_rate:
      emit_load_frag_shading_rate(ctx, get_ssa_temp(ctx, &instr->def));
      break;
   case nir_intrinsic_load_sample_pos: {
      Temp posx = get_arg(ctx, ctx->args->frag_pos[0]);
      Temp posy = get_arg(ctx, ctx->args->frag_pos[1]);
      bld.pseudo(
         aco_opcode::p_create_vector, Definition(get_ssa_temp(ctx, &instr->def)),
         posx.id() ? bld.vop1(aco_opcode::v_fract_f32, bld.def(v1), posx) : Operand::zero(),
         posy.id() ? bld.vop1(aco_opcode::v_fract_f32, bld.def(v1), posy) : Operand::zero());
      break;
   }
   case nir_intrinsic_load_tess_coord: visit_load_tess_coord(ctx, instr); break;
   case nir_intrinsic_load_interpolated_input: visit_load_interpolated_input(ctx, instr); break;
   case nir_intrinsic_store_output: visit_store_output(ctx, instr); break;
   case nir_intrinsic_load_input:
   case nir_intrinsic_load_input_vertex:
      if (ctx->program->stage == fragment_fs)
         visit_load_fs_input(ctx, instr);
      else
         isel_err(&instr->instr, "Shader inputs should have been lowered in NIR.");
      break;
   case nir_intrinsic_load_per_vertex_input: visit_load_per_vertex_input(ctx, instr); break;
   case nir_intrinsic_load_ubo: visit_load_ubo(ctx, instr); break;
   case nir_intrinsic_load_push_constant: visit_load_push_constant(ctx, instr); break;
   case nir_intrinsic_load_constant: visit_load_constant(ctx, instr); break;
   case nir_intrinsic_load_shared: visit_load_shared(ctx, instr); break;
   case nir_intrinsic_store_shared: visit_store_shared(ctx, instr); break;
   case nir_intrinsic_shared_atomic:
   case nir_intrinsic_shared_atomic_swap: visit_shared_atomic(ctx, instr); break;
   case nir_intrinsic_load_shared2_amd:
   case nir_intrinsic_store_shared2_amd: visit_access_shared2_amd(ctx, instr); break;
   case nir_intrinsic_bindless_image_load:
   case nir_intrinsic_bindless_image_fragment_mask_load_amd:
   case nir_intrinsic_bindless_image_sparse_load: visit_image_load(ctx, instr); break;
   case nir_intrinsic_bindless_image_store: visit_image_store(ctx, instr); break;
   case nir_intrinsic_bindless_image_atomic:
   case nir_intrinsic_bindless_image_atomic_swap: visit_image_atomic(ctx, instr); break;
   case nir_intrinsic_load_ssbo: visit_load_ssbo(ctx, instr); break;
   case nir_intrinsic_store_ssbo: visit_store_ssbo(ctx, instr); break;
   case nir_intrinsic_load_typed_buffer_amd:
   case nir_intrinsic_load_buffer_amd: visit_load_buffer(ctx, instr); break;
   case nir_intrinsic_store_buffer_amd: visit_store_buffer(ctx, instr); break;
   case nir_intrinsic_load_smem_amd: visit_load_smem(ctx, instr); break;
   case nir_intrinsic_load_global_amd: visit_load_global(ctx, instr); break;
   case nir_intrinsic_store_global_amd: visit_store_global(ctx, instr); break;
   case nir_intrinsic_global_atomic_amd:
   case nir_intrinsic_global_atomic_swap_amd: visit_global_atomic(ctx, instr); break;
   case nir_intrinsic_ssbo_atomic:
   case nir_intrinsic_ssbo_atomic_swap: visit_atomic_ssbo(ctx, instr); break;
   case nir_intrinsic_load_scratch: visit_load_scratch(ctx, instr); break;
   case nir_intrinsic_store_scratch: visit_store_scratch(ctx, instr); break;
   case nir_intrinsic_barrier: emit_barrier(ctx, instr); break;
   case nir_intrinsic_load_num_workgroups: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      if (ctx->options->load_grid_size_from_user_sgpr) {
         bld.copy(Definition(dst), get_arg(ctx, ctx->args->num_work_groups));
      } else {
         Temp addr = get_arg(ctx, ctx->args->num_work_groups);
         assert(addr.regClass() == s2);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst),
                    bld.smem(aco_opcode::s_load_dwordx2, bld.def(s2), addr, Operand::zero()),
                    bld.smem(aco_opcode::s_load_dword, bld.def(s1), addr, Operand::c32(8)));
      }
      emit_split_vector(ctx, dst, 3);
      break;
   }
   case nir_intrinsic_load_ray_launch_size: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.copy(Definition(dst), Operand(get_arg(ctx, ctx->args->rt.launch_size)));
      emit_split_vector(ctx, dst, 3);
      break;
   }
   case nir_intrinsic_load_ray_launch_id: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.copy(Definition(dst), Operand(get_arg(ctx, ctx->args->rt.launch_id)));
      emit_split_vector(ctx, dst, 3);
      break;
   }
   case nir_intrinsic_load_ray_launch_size_addr_amd: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      Temp addr = get_arg(ctx, ctx->args->rt.launch_size_addr);
      assert(addr.regClass() == s2);
      bld.copy(Definition(dst), Operand(addr));
      break;
   }
   case nir_intrinsic_load_local_invocation_id: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      if (ctx->options->gfx_level >= GFX11) {
         Temp local_ids[3];

         /* Thread IDs are packed in VGPR0, 10 bits per component. */
         for (uint32_t i = 0; i < 3; i++) {
            if (i == 0 && ctx->shader->info.workgroup_size[1] == 1 &&
                ctx->shader->info.workgroup_size[2] == 1 &&
                !ctx->shader->info.workgroup_size_variable) {
               local_ids[i] = get_arg(ctx, ctx->args->local_invocation_ids);
            } else if (i == 2 || (i == 1 && ctx->shader->info.workgroup_size[2] == 1 &&
                                  !ctx->shader->info.workgroup_size_variable)) {
               local_ids[i] =
                  bld.vop2(aco_opcode::v_lshrrev_b32, bld.def(v1), Operand::c32(i * 10u),
                           get_arg(ctx, ctx->args->local_invocation_ids));
            } else {
               local_ids[i] = bld.vop3(aco_opcode::v_bfe_u32, bld.def(v1),
                                       get_arg(ctx, ctx->args->local_invocation_ids),
                                       Operand::c32(i * 10u), Operand::c32(10u));
            }
         }

         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), local_ids[0], local_ids[1],
                    local_ids[2]);
      } else {
         bld.copy(Definition(dst), Operand(get_arg(ctx, ctx->args->local_invocation_ids)));
      }
      emit_split_vector(ctx, dst, 3);
      break;
   }
   case nir_intrinsic_load_workgroup_id: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      if (ctx->stage.hw == AC_HW_COMPUTE_SHADER) {
         const struct ac_arg* ids = ctx->args->workgroup_ids;
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst),
                    ids[0].used ? Operand(get_arg(ctx, ids[0])) : Operand::zero(),
                    ids[1].used ? Operand(get_arg(ctx, ids[1])) : Operand::zero(),
                    ids[2].used ? Operand(get_arg(ctx, ids[2])) : Operand::zero());
         emit_split_vector(ctx, dst, 3);
      } else {
         isel_err(&instr->instr, "Unsupported stage for load_workgroup_id");
      }
      break;
   }
   case nir_intrinsic_load_local_invocation_index: {
      if (ctx->stage.hw == AC_HW_LOCAL_SHADER || ctx->stage.hw == AC_HW_HULL_SHADER) {
         if (ctx->options->gfx_level >= GFX11) {
            /* On GFX11, RelAutoIndex is WaveID * WaveSize + ThreadID. */
            Temp wave_id =
               bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc),
                        get_arg(ctx, ctx->args->tcs_wave_id), Operand::c32(0u | (3u << 16)));

            Temp temp = bld.sop2(aco_opcode::s_mul_i32, bld.def(s1), wave_id,
                                 Operand::c32(ctx->program->wave_size));
            emit_mbcnt(ctx, get_ssa_temp(ctx, &instr->def), Operand(), Operand(temp));
         } else {
            bld.copy(Definition(get_ssa_temp(ctx, &instr->def)),
                     get_arg(ctx, ctx->args->vs_rel_patch_id));
         }
         break;
      } else if (ctx->stage.hw == AC_HW_LEGACY_GEOMETRY_SHADER ||
                 ctx->stage.hw == AC_HW_NEXT_GEN_GEOMETRY_SHADER) {
         bld.copy(Definition(get_ssa_temp(ctx, &instr->def)), thread_id_in_threadgroup(ctx));
         break;
      } else if (ctx->program->workgroup_size <= ctx->program->wave_size) {
         emit_mbcnt(ctx, get_ssa_temp(ctx, &instr->def));
         break;
      }

      Temp id = emit_mbcnt(ctx, bld.tmp(v1));

      /* The tg_size bits [6:11] contain the subgroup id,
       * we need this multiplied by the wave size, and then OR the thread id to it.
       */
      if (ctx->program->wave_size == 64) {
         /* After the s_and the bits are already multiplied by 64 (left shifted by 6) so we can just
          * feed that to v_or */
         Temp tg_num = bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc),
                                Operand::c32(0xfc0u), get_arg(ctx, ctx->args->tg_size));
         bld.vop2(aco_opcode::v_or_b32, Definition(get_ssa_temp(ctx, &instr->def)), tg_num, id);
      } else {
         /* Extract the bit field and multiply the result by 32 (left shift by 5), then do the OR */
         Temp tg_num =
            bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc),
                     get_arg(ctx, ctx->args->tg_size), Operand::c32(0x6u | (0x6u << 16)));
         bld.vop3(aco_opcode::v_lshl_or_b32, Definition(get_ssa_temp(ctx, &instr->def)), tg_num,
                  Operand::c32(0x5u), id);
      }
      break;
   }
   case nir_intrinsic_load_subgroup_invocation: {
      emit_mbcnt(ctx, get_ssa_temp(ctx, &instr->def));
      break;
   }
   case nir_intrinsic_ballot: {
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
      Temp dst = get_ssa_temp(ctx, &instr->def);

      if (instr->src[0].ssa->bit_size == 1) {
         assert(src.regClass() == bld.lm);
      } else if (instr->src[0].ssa->bit_size == 32 && src.regClass() == v1) {
         src = bld.vopc(aco_opcode::v_cmp_lg_u32, bld.def(bld.lm), Operand::zero(), src);
      } else if (instr->src[0].ssa->bit_size == 64 && src.regClass() == v2) {
         src = bld.vopc(aco_opcode::v_cmp_lg_u64, bld.def(bld.lm), Operand::zero(), src);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }

      /* Make sure that all inactive lanes return zero.
       * Value-numbering might remove the comparison above */
      Definition def = dst.size() == bld.lm.size() ? Definition(dst) : bld.def(bld.lm);
      src = bld.sop2(Builder::s_and, def, bld.def(s1, scc), src, Operand(exec, bld.lm));
      if (dst.size() != bld.lm.size()) {
         /* Wave32 with ballot size set to 64 */
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), src, Operand::zero());
      }

      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_inverse_ballot: {
      Temp src = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));
      Temp dst = get_ssa_temp(ctx, &instr->def);

      assert(dst.size() == bld.lm.size());
      if (src.size() > dst.size()) {
         emit_extract_vector(ctx, src, 0, dst);
      } else if (src.size() < dst.size()) {
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), src, Operand::zero());
      } else {
         bld.copy(Definition(dst), src);
      }
      break;
   }
   case nir_intrinsic_shuffle:
   case nir_intrinsic_read_invocation: {
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
      if (!nir_src_is_divergent(instr->src[0])) {
         emit_uniform_subgroup(ctx, instr, src);
      } else {
         Temp tid = get_ssa_temp(ctx, instr->src[1].ssa);
         if (instr->intrinsic == nir_intrinsic_read_invocation ||
             !nir_src_is_divergent(instr->src[1]))
            tid = bld.as_uniform(tid);
         Temp dst = get_ssa_temp(ctx, &instr->def);

         if (instr->def.bit_size != 1)
            src = as_vgpr(ctx, src);

         if (src.regClass() == v1b || src.regClass() == v2b) {
            Temp tmp = bld.tmp(v1);
            tmp = emit_bpermute(ctx, bld, tid, src);
            if (dst.type() == RegType::vgpr)
               bld.pseudo(aco_opcode::p_split_vector, Definition(dst),
                          bld.def(src.regClass() == v1b ? v3b : v2b), tmp);
            else
               bld.pseudo(aco_opcode::p_as_uniform, Definition(dst), tmp);
         } else if (src.regClass() == v1) {
            Temp tmp = emit_bpermute(ctx, bld, tid, src);
            bld.copy(Definition(dst), tmp);
         } else if (src.regClass() == v2) {
            Temp lo = bld.tmp(v1), hi = bld.tmp(v1);
            bld.pseudo(aco_opcode::p_split_vector, Definition(lo), Definition(hi), src);
            lo = emit_bpermute(ctx, bld, tid, lo);
            hi = emit_bpermute(ctx, bld, tid, hi);
            bld.pseudo(aco_opcode::p_create_vector, Definition(dst), lo, hi);
            emit_split_vector(ctx, dst, 2);
         } else if (instr->def.bit_size == 1 && tid.regClass() == s1) {
            assert(src.regClass() == bld.lm);
            Temp tmp = bld.sopc(Builder::s_bitcmp1, bld.def(s1, scc), src, tid);
            bool_to_vector_condition(ctx, tmp, dst);
         } else if (instr->def.bit_size == 1 && tid.regClass() == v1) {
            assert(src.regClass() == bld.lm);
            Temp tmp;
            if (ctx->program->gfx_level <= GFX7)
               tmp = bld.vop3(aco_opcode::v_lshr_b64, bld.def(v2), src, tid);
            else if (ctx->program->wave_size == 64)
               tmp = bld.vop3(aco_opcode::v_lshrrev_b64, bld.def(v2), tid, src);
            else
               tmp = bld.vop2_e64(aco_opcode::v_lshrrev_b32, bld.def(v1), tid, src);
            tmp = emit_extract_vector(ctx, tmp, 0, v1);
            tmp = bld.vop2(aco_opcode::v_and_b32, bld.def(v1), Operand::c32(1u), tmp);
            bld.vopc(aco_opcode::v_cmp_lg_u32, Definition(dst), Operand::zero(), tmp);
         } else {
            isel_err(&instr->instr, "Unimplemented NIR instr bit size");
         }
         set_wqm(ctx);
      }
      break;
   }
   case nir_intrinsic_load_sample_id: {
      bld.vop3(aco_opcode::v_bfe_u32, Definition(get_ssa_temp(ctx, &instr->def)),
               get_arg(ctx, ctx->args->ancillary), Operand::c32(8u), Operand::c32(4u));
      break;
   }
   case nir_intrinsic_read_first_invocation: {
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
      Temp dst = get_ssa_temp(ctx, &instr->def);
      if (instr->def.bit_size == 1) {
         assert(src.regClass() == bld.lm);
         Temp tmp = bld.sopc(Builder::s_bitcmp1, bld.def(s1, scc), src,
                             bld.sop1(Builder::s_ff1_i32, bld.def(s1), Operand(exec, bld.lm)));
         bool_to_vector_condition(ctx, tmp, dst);
      } else {
         emit_readfirstlane(ctx, src, dst);
      }
      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_vote_all: {
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
      Temp dst = get_ssa_temp(ctx, &instr->def);
      assert(src.regClass() == bld.lm);
      assert(dst.regClass() == bld.lm);

      Temp tmp = bld.sop1(Builder::s_not, bld.def(bld.lm), bld.def(s1, scc), src);
      tmp = bld.sop2(Builder::s_and, bld.def(bld.lm), bld.def(s1, scc), tmp, Operand(exec, bld.lm))
               .def(1)
               .getTemp();
      Temp cond = bool_to_vector_condition(ctx, tmp);
      bld.sop1(Builder::s_not, Definition(dst), bld.def(s1, scc), cond);
      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_vote_any: {
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
      Temp dst = get_ssa_temp(ctx, &instr->def);
      assert(src.regClass() == bld.lm);
      assert(dst.regClass() == bld.lm);

      Temp tmp = bool_to_scalar_condition(ctx, src);
      bool_to_vector_condition(ctx, tmp, dst);
      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_quad_vote_any: {
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
      src = bld.sop2(Builder::s_and, bld.def(bld.lm), bld.def(s1, scc), src, Operand(exec, bld.lm));
      bld.sop1(Builder::s_wqm, Definition(get_ssa_temp(ctx, &instr->def)), bld.def(s1, scc), src);
      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_quad_vote_all: {
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
      src = bld.sop1(Builder::s_not, bld.def(bld.lm), bld.def(s1, scc), src);
      src = bld.sop2(Builder::s_and, bld.def(bld.lm), bld.def(s1, scc), src, Operand(exec, bld.lm));
      src = bld.sop1(Builder::s_wqm, bld.def(bld.lm), bld.def(s1, scc), src);
      bld.sop1(Builder::s_not, Definition(get_ssa_temp(ctx, &instr->def)), bld.def(s1, scc), src);
      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_reduce:
   case nir_intrinsic_inclusive_scan:
   case nir_intrinsic_exclusive_scan: {
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
      Temp dst = get_ssa_temp(ctx, &instr->def);
      nir_op op = (nir_op)nir_intrinsic_reduction_op(instr);
      unsigned cluster_size =
         instr->intrinsic == nir_intrinsic_reduce ? nir_intrinsic_cluster_size(instr) : 0;
      cluster_size = util_next_power_of_two(
         MIN2(cluster_size ? cluster_size : ctx->program->wave_size, ctx->program->wave_size));
      const unsigned bit_size = instr->src[0].ssa->bit_size;
      assert(bit_size != 1);

      if (!nir_src_is_divergent(instr->src[0]) && cluster_size == ctx->program->wave_size) {
         /* We use divergence analysis to assign the regclass, so check if it's
          * working as expected */
         ASSERTED bool expected_divergent = instr->intrinsic == nir_intrinsic_exclusive_scan;
         if (instr->intrinsic == nir_intrinsic_inclusive_scan)
            expected_divergent = op == nir_op_iadd || op == nir_op_fadd || op == nir_op_ixor;
         assert(instr->def.divergent == expected_divergent);

         if (instr->intrinsic == nir_intrinsic_reduce) {
            if (emit_uniform_reduce(ctx, instr))
               break;
         } else if (emit_uniform_scan(ctx, instr)) {
            break;
         }
      }

      src = emit_extract_vector(ctx, src, 0, RegClass::get(RegType::vgpr, bit_size / 8));
      ReduceOp reduce_op = get_reduce_op(op, bit_size);

      aco_opcode aco_op;
      switch (instr->intrinsic) {
      case nir_intrinsic_reduce: aco_op = aco_opcode::p_reduce; break;
      case nir_intrinsic_inclusive_scan: aco_op = aco_opcode::p_inclusive_scan; break;
      case nir_intrinsic_exclusive_scan: aco_op = aco_opcode::p_exclusive_scan; break;
      default: unreachable("unknown reduce intrinsic");
      }

      /* Avoid whole wave shift. */
      const bool use_inclusive_for_exclusive = aco_op == aco_opcode::p_exclusive_scan &&
                                               (op == nir_op_iadd || op == nir_op_ixor) &&
                                               dst.type() == RegType::vgpr;
      if (use_inclusive_for_exclusive)
         inclusive_scan_to_exclusive(ctx, reduce_op, Definition(dst), src);
      else
         emit_reduction_instr(ctx, aco_op, reduce_op, cluster_size, Definition(dst), src);

      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_quad_broadcast:
   case nir_intrinsic_quad_swap_horizontal:
   case nir_intrinsic_quad_swap_vertical:
   case nir_intrinsic_quad_swap_diagonal:
   case nir_intrinsic_quad_swizzle_amd: {
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);

      if (!instr->def.divergent) {
         emit_uniform_subgroup(ctx, instr, src);
         break;
      }

      /* Quad broadcast lane. */
      unsigned lane = 0;
      /* Use VALU for the bool instructions that don't have a SALU-only special case. */
      bool bool_use_valu = instr->def.bit_size == 1;

      uint16_t dpp_ctrl = 0;

      bool allow_fi = true;
      switch (instr->intrinsic) {
      case nir_intrinsic_quad_swap_horizontal: dpp_ctrl = dpp_quad_perm(1, 0, 3, 2); break;
      case nir_intrinsic_quad_swap_vertical: dpp_ctrl = dpp_quad_perm(2, 3, 0, 1); break;
      case nir_intrinsic_quad_swap_diagonal: dpp_ctrl = dpp_quad_perm(3, 2, 1, 0); break;
      case nir_intrinsic_quad_swizzle_amd:
         dpp_ctrl = nir_intrinsic_swizzle_mask(instr);
         allow_fi &= nir_intrinsic_fetch_inactive(instr);
         break;
      case nir_intrinsic_quad_broadcast:
         lane = nir_src_as_const_value(instr->src[1])->u32;
         dpp_ctrl = dpp_quad_perm(lane, lane, lane, lane);
         bool_use_valu = false;
         break;
      default: break;
      }

      Temp dst = get_ssa_temp(ctx, &instr->def);

      /* Setup source. */
      if (bool_use_valu)
         src = bld.vop2_e64(aco_opcode::v_cndmask_b32, bld.def(v1), Operand::zero(),
                            Operand::c32(-1), src);
      else if (instr->def.bit_size != 1)
         src = as_vgpr(ctx, src);

      if (instr->def.bit_size == 1 && instr->intrinsic == nir_intrinsic_quad_broadcast) {
         /* Special case for quad broadcast using SALU only. */
         assert(src.regClass() == bld.lm && dst.regClass() == bld.lm);

         uint32_t half_mask = 0x11111111u << lane;
         Operand mask_tmp = bld.lm.bytes() == 4
                               ? Operand::c32(half_mask)
                               : bld.pseudo(aco_opcode::p_create_vector, bld.def(bld.lm),
                                            Operand::c32(half_mask), Operand::c32(half_mask));

         src =
            bld.sop2(Builder::s_and, bld.def(bld.lm), bld.def(s1, scc), src, Operand(exec, bld.lm));
         src = bld.sop2(Builder::s_and, bld.def(bld.lm), bld.def(s1, scc), mask_tmp, src);
         bld.sop1(Builder::s_wqm, Definition(dst), bld.def(s1, scc), src);
      } else if (instr->def.bit_size <= 32 || bool_use_valu) {
         unsigned excess_bytes = bool_use_valu ? 0 : 4 - instr->def.bit_size / 8;
         Definition def = (excess_bytes || bool_use_valu) ? bld.def(v1) : Definition(dst);

         if (ctx->program->gfx_level >= GFX8)
            bld.vop1_dpp(aco_opcode::v_mov_b32, def, src, dpp_ctrl, 0xf, 0xf, true, allow_fi);
         else
            bld.ds(aco_opcode::ds_swizzle_b32, def, src, (1 << 15) | dpp_ctrl);

         if (excess_bytes)
            bld.pseudo(aco_opcode::p_split_vector, Definition(dst),
                       bld.def(RegClass::get(dst.type(), excess_bytes)), def.getTemp());
         if (bool_use_valu)
            bld.vopc(aco_opcode::v_cmp_lg_u32, Definition(dst), Operand::zero(), def.getTemp());
      } else if (instr->def.bit_size == 64) {
         Temp lo = bld.tmp(v1), hi = bld.tmp(v1);
         bld.pseudo(aco_opcode::p_split_vector, Definition(lo), Definition(hi), src);

         if (ctx->program->gfx_level >= GFX8) {
            lo = bld.vop1_dpp(aco_opcode::v_mov_b32, bld.def(v1), lo, dpp_ctrl, 0xf, 0xf, true,
                              allow_fi);
            hi = bld.vop1_dpp(aco_opcode::v_mov_b32, bld.def(v1), hi, dpp_ctrl, 0xf, 0xf, true,
                              allow_fi);
         } else {
            lo = bld.ds(aco_opcode::ds_swizzle_b32, bld.def(v1), lo, (1 << 15) | dpp_ctrl);
            hi = bld.ds(aco_opcode::ds_swizzle_b32, bld.def(v1), hi, (1 << 15) | dpp_ctrl);
         }

         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), lo, hi);
         emit_split_vector(ctx, dst, 2);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR quad group instruction bit size.");
      }

      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_masked_swizzle_amd: {
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
      if (!instr->def.divergent) {
         emit_uniform_subgroup(ctx, instr, src);
         break;
      }
      Temp dst = get_ssa_temp(ctx, &instr->def);
      uint32_t mask = nir_intrinsic_swizzle_mask(instr);
      bool allow_fi = nir_intrinsic_fetch_inactive(instr);

      if (instr->def.bit_size != 1)
         src = as_vgpr(ctx, src);

      if (instr->def.bit_size == 1) {
         assert(src.regClass() == bld.lm);
         src = bld.vop2_e64(aco_opcode::v_cndmask_b32, bld.def(v1), Operand::zero(),
                            Operand::c32(-1), src);
         src = emit_masked_swizzle(ctx, bld, src, mask, allow_fi);
         bld.vopc(aco_opcode::v_cmp_lg_u32, Definition(dst), Operand::zero(), src);
      } else if (dst.regClass() == v1b) {
         Temp tmp = emit_masked_swizzle(ctx, bld, src, mask, allow_fi);
         emit_extract_vector(ctx, tmp, 0, dst);
      } else if (dst.regClass() == v2b) {
         Temp tmp = emit_masked_swizzle(ctx, bld, src, mask, allow_fi);
         emit_extract_vector(ctx, tmp, 0, dst);
      } else if (dst.regClass() == v1) {
         bld.copy(Definition(dst), emit_masked_swizzle(ctx, bld, src, mask, allow_fi));
      } else if (dst.regClass() == v2) {
         Temp lo = bld.tmp(v1), hi = bld.tmp(v1);
         bld.pseudo(aco_opcode::p_split_vector, Definition(lo), Definition(hi), src);
         lo = emit_masked_swizzle(ctx, bld, lo, mask, allow_fi);
         hi = emit_masked_swizzle(ctx, bld, hi, mask, allow_fi);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), lo, hi);
         emit_split_vector(ctx, dst, 2);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_write_invocation_amd: {
      Temp src = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[0].ssa));
      Temp val = bld.as_uniform(get_ssa_temp(ctx, instr->src[1].ssa));
      Temp lane = bld.as_uniform(get_ssa_temp(ctx, instr->src[2].ssa));
      Temp dst = get_ssa_temp(ctx, &instr->def);
      if (dst.regClass() == v1) {
         /* src2 is ignored for writelane. RA assigns the same reg for dst */
         bld.writelane(Definition(dst), val, lane, src);
      } else if (dst.regClass() == v2) {
         Temp src_lo = bld.tmp(v1), src_hi = bld.tmp(v1);
         Temp val_lo = bld.tmp(s1), val_hi = bld.tmp(s1);
         bld.pseudo(aco_opcode::p_split_vector, Definition(src_lo), Definition(src_hi), src);
         bld.pseudo(aco_opcode::p_split_vector, Definition(val_lo), Definition(val_hi), val);
         Temp lo = bld.writelane(bld.def(v1), val_lo, lane, src_hi);
         Temp hi = bld.writelane(bld.def(v1), val_hi, lane, src_hi);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), lo, hi);
         emit_split_vector(ctx, dst, 2);
      } else {
         isel_err(&instr->instr, "Unimplemented NIR instr bit size");
      }
      break;
   }
   case nir_intrinsic_mbcnt_amd: {
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
      Temp add_src = as_vgpr(ctx, get_ssa_temp(ctx, instr->src[1].ssa));
      Temp dst = get_ssa_temp(ctx, &instr->def);
      /* Fit 64-bit mask for wave32 */
      src = emit_extract_vector(ctx, src, 0, RegClass(src.type(), bld.lm.size()));
      emit_mbcnt(ctx, dst, Operand(src), Operand(add_src));
      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_lane_permute_16_amd: {
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
      Temp dst = get_ssa_temp(ctx, &instr->def);
      assert(ctx->program->gfx_level >= GFX10);

      if (src.regClass() == s1) {
         bld.copy(Definition(dst), src);
      } else if (dst.regClass() == v1 && src.regClass() == v1) {
         bld.vop3(aco_opcode::v_permlane16_b32, Definition(dst), src,
                  bld.as_uniform(get_ssa_temp(ctx, instr->src[1].ssa)),
                  bld.as_uniform(get_ssa_temp(ctx, instr->src[2].ssa)));
      } else {
         isel_err(&instr->instr, "Unimplemented lane_permute_16_amd");
      }
      break;
   }
   case nir_intrinsic_load_helper_invocation:
   case nir_intrinsic_is_helper_invocation: {
      /* load_helper() after demote() get lowered to is_helper().
       * Otherwise, these two behave the same. */
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.pseudo(aco_opcode::p_is_helper, Definition(dst), Operand(exec, bld.lm));
      ctx->program->needs_exact = true;
      break;
   }
   case nir_intrinsic_demote:
   case nir_intrinsic_demote_if: {
      Operand cond = Operand::c32(-1u);
      if (instr->intrinsic == nir_intrinsic_demote_if) {
         Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
         assert(src.regClass() == bld.lm);
         cond =
            bld.sop2(Builder::s_and, bld.def(bld.lm), bld.def(s1, scc), src, Operand(exec, bld.lm));
      }

      bld.pseudo(aco_opcode::p_demote_to_helper, cond);

      if (ctx->block->loop_nest_depth || ctx->cf_info.parent_if.is_divergent)
         ctx->cf_info.exec_potentially_empty_discard = true;

      ctx->block->kind |= block_kind_uses_discard;
      ctx->program->needs_exact = true;
      break;
   }
   case nir_intrinsic_terminate:
   case nir_intrinsic_terminate_if:
   case nir_intrinsic_discard:
   case nir_intrinsic_discard_if: {
      Operand cond = Operand::c32(-1u);
      if (instr->intrinsic == nir_intrinsic_discard_if ||
          instr->intrinsic == nir_intrinsic_terminate_if) {
         Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
         assert(src.regClass() == bld.lm);
         cond =
            bld.sop2(Builder::s_and, bld.def(bld.lm), bld.def(s1, scc), src, Operand(exec, bld.lm));

         ctx->cf_info.had_divergent_discard |= nir_src_is_divergent(instr->src[0]);
      }

      bld.pseudo(aco_opcode::p_discard_if, cond);

      if (ctx->block->loop_nest_depth || ctx->cf_info.parent_if.is_divergent)
         ctx->cf_info.exec_potentially_empty_discard = true;
      ctx->cf_info.had_divergent_discard |= in_exec_divergent_or_in_loop(ctx);
      ctx->block->kind |= block_kind_uses_discard;
      ctx->program->needs_exact = true;
      break;
   }
   case nir_intrinsic_first_invocation: {
      bld.sop1(Builder::s_ff1_i32, Definition(get_ssa_temp(ctx, &instr->def)),
               Operand(exec, bld.lm));
      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_last_invocation: {
      Temp flbit = bld.sop1(Builder::s_flbit_i32, bld.def(s1), Operand(exec, bld.lm));
      bld.sop2(aco_opcode::s_sub_i32, Definition(get_ssa_temp(ctx, &instr->def)), bld.def(s1, scc),
               Operand::c32(ctx->program->wave_size - 1u), flbit);
      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_elect: {
      /* p_elect is lowered in aco_insert_exec_mask.
       * Use exec as an operand so value numbering and the pre-RA optimizer won't recognize
       * two p_elect with different exec masks as the same.
       */
      bld.pseudo(aco_opcode::p_elect, Definition(get_ssa_temp(ctx, &instr->def)),
                 Operand(exec, bld.lm));
      set_wqm(ctx);
      break;
   }
   case nir_intrinsic_shader_clock: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      if (nir_intrinsic_memory_scope(instr) == SCOPE_SUBGROUP &&
          ctx->options->gfx_level >= GFX10_3) {
         /* "((size - 1) << 11) | register" (SHADER_CYCLES is encoded as register 29) */
         Temp clock = bld.sopk(aco_opcode::s_getreg_b32, bld.def(s1), ((20 - 1) << 11) | 29);
         bld.pseudo(aco_opcode::p_create_vector, Definition(dst), clock, Operand::zero());
      } else if (nir_intrinsic_memory_scope(instr) == SCOPE_DEVICE &&
                 ctx->options->gfx_level >= GFX11) {
         bld.sop1(aco_opcode::s_sendmsg_rtn_b64, Definition(dst),
                  Operand::c32(sendmsg_rtn_get_realtime));
      } else {
         aco_opcode opcode = nir_intrinsic_memory_scope(instr) == SCOPE_DEVICE
                                ? aco_opcode::s_memrealtime
                                : aco_opcode::s_memtime;
         bld.smem(opcode, Definition(dst), memory_sync_info(0, semantic_volatile));
      }
      emit_split_vector(ctx, dst, 2);
      break;
   }
   case nir_intrinsic_load_vertex_id_zero_base: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.copy(Definition(dst), get_arg(ctx, ctx->args->vertex_id));
      break;
   }
   case nir_intrinsic_load_first_vertex: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.copy(Definition(dst), get_arg(ctx, ctx->args->base_vertex));
      break;
   }
   case nir_intrinsic_load_base_instance: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.copy(Definition(dst), get_arg(ctx, ctx->args->start_instance));
      break;
   }
   case nir_intrinsic_load_instance_id: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.copy(Definition(dst), get_arg(ctx, ctx->args->instance_id));
      break;
   }
   case nir_intrinsic_load_draw_id: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.copy(Definition(dst), get_arg(ctx, ctx->args->draw_id));
      break;
   }
   case nir_intrinsic_load_invocation_id: {
      Temp dst = get_ssa_temp(ctx, &instr->def);

      if (ctx->shader->info.stage == MESA_SHADER_GEOMETRY) {
         if (ctx->options->gfx_level >= GFX10)
            bld.vop2_e64(aco_opcode::v_and_b32, Definition(dst), Operand::c32(127u),
                         get_arg(ctx, ctx->args->gs_invocation_id));
         else
            bld.copy(Definition(dst), get_arg(ctx, ctx->args->gs_invocation_id));
      } else if (ctx->shader->info.stage == MESA_SHADER_TESS_CTRL) {
         bld.vop3(aco_opcode::v_bfe_u32, Definition(dst), get_arg(ctx, ctx->args->tcs_rel_ids),
                  Operand::c32(8u), Operand::c32(5u));
      } else {
         unreachable("Unsupported stage for load_invocation_id");
      }

      break;
   }
   case nir_intrinsic_load_primitive_id: {
      Temp dst = get_ssa_temp(ctx, &instr->def);

      switch (ctx->shader->info.stage) {
      case MESA_SHADER_GEOMETRY:
         bld.copy(Definition(dst), get_arg(ctx, ctx->args->gs_prim_id));
         break;
      case MESA_SHADER_TESS_CTRL:
         bld.copy(Definition(dst), get_arg(ctx, ctx->args->tcs_patch_id));
         break;
      case MESA_SHADER_TESS_EVAL:
         bld.copy(Definition(dst), get_arg(ctx, ctx->args->tes_patch_id));
         break;
      default:
         if (ctx->stage.hw == AC_HW_NEXT_GEN_GEOMETRY_SHADER && !ctx->stage.has(SWStage::GS)) {
            /* In case of NGG, the GS threads always have the primitive ID
             * even if there is no SW GS. */
            bld.copy(Definition(dst), get_arg(ctx, ctx->args->gs_prim_id));
            break;
         } else if (ctx->shader->info.stage == MESA_SHADER_VERTEX) {
            bld.copy(Definition(dst), get_arg(ctx, ctx->args->vs_prim_id));
            break;
         }
         unreachable("Unimplemented shader stage for nir_intrinsic_load_primitive_id");
      }

      break;
   }
   case nir_intrinsic_sendmsg_amd: {
      unsigned imm = nir_intrinsic_base(instr);
      Temp m0_content = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));
      bld.sopp(aco_opcode::s_sendmsg, bld.m0(m0_content), -1, imm);
      break;
   }
   case nir_intrinsic_load_gs_wave_id_amd: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      if (ctx->args->merged_wave_info.used)
         bld.pseudo(aco_opcode::p_extract, Definition(dst), bld.def(s1, scc),
                    get_arg(ctx, ctx->args->merged_wave_info), Operand::c32(2u), Operand::c32(8u),
                    Operand::zero());
      else if (ctx->args->gs_wave_id.used)
         bld.copy(Definition(dst), get_arg(ctx, ctx->args->gs_wave_id));
      else
         unreachable("Shader doesn't have GS wave ID.");
      break;
   }
   case nir_intrinsic_is_subgroup_invocation_lt_amd: {
      Temp src = bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));
      bld.copy(Definition(get_ssa_temp(ctx, &instr->def)), lanecount_to_mask(ctx, src));
      break;
   }
   case nir_intrinsic_gds_atomic_add_amd: {
      Temp store_val = get_ssa_temp(ctx, instr->src[0].ssa);
      Temp gds_addr = get_ssa_temp(ctx, instr->src[1].ssa);
      Temp m0_val = get_ssa_temp(ctx, instr->src[2].ssa);
      Operand m = bld.m0((Temp)bld.copy(bld.def(s1, m0), bld.as_uniform(m0_val)));
      bld.ds(aco_opcode::ds_add_u32, as_vgpr(ctx, gds_addr), as_vgpr(ctx, store_val), m, 0u, 0u,
             true);
      break;
   }
   case nir_intrinsic_load_sbt_base_amd: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      Temp addr = get_arg(ctx, ctx->args->rt.sbt_descriptors);
      assert(addr.regClass() == s2);
      bld.copy(Definition(dst), Operand(addr));
      break;
   }
   case nir_intrinsic_bvh64_intersect_ray_amd: visit_bvh64_intersect_ray_amd(ctx, instr); break;
   case nir_intrinsic_load_rt_dynamic_callable_stack_base_amd:
      bld.copy(Definition(get_ssa_temp(ctx, &instr->def)),
               get_arg(ctx, ctx->args->rt.dynamic_callable_stack_base));
      break;
   case nir_intrinsic_load_resume_shader_address_amd: {
      bld.pseudo(aco_opcode::p_resume_shader_address, Definition(get_ssa_temp(ctx, &instr->def)),
                 bld.def(s1, scc), Operand::c32(nir_intrinsic_call_idx(instr)));
      break;
   }
   case nir_intrinsic_overwrite_vs_arguments_amd: {
      ctx->arg_temps[ctx->args->vertex_id.arg_index] = get_ssa_temp(ctx, instr->src[0].ssa);
      ctx->arg_temps[ctx->args->instance_id.arg_index] = get_ssa_temp(ctx, instr->src[1].ssa);
      break;
   }
   case nir_intrinsic_overwrite_tes_arguments_amd: {
      ctx->arg_temps[ctx->args->tes_u.arg_index] = get_ssa_temp(ctx, instr->src[0].ssa);
      ctx->arg_temps[ctx->args->tes_v.arg_index] = get_ssa_temp(ctx, instr->src[1].ssa);
      ctx->arg_temps[ctx->args->tes_rel_patch_id.arg_index] = get_ssa_temp(ctx, instr->src[3].ssa);
      ctx->arg_temps[ctx->args->tes_patch_id.arg_index] = get_ssa_temp(ctx, instr->src[2].ssa);
      break;
   }
   case nir_intrinsic_load_scalar_arg_amd:
   case nir_intrinsic_load_vector_arg_amd: {
      assert(nir_intrinsic_base(instr) < ctx->args->arg_count);
      Temp dst = get_ssa_temp(ctx, &instr->def);
      Temp src = ctx->arg_temps[nir_intrinsic_base(instr)];
      assert(src.id());
      assert(src.type() == (instr->intrinsic == nir_intrinsic_load_scalar_arg_amd ? RegType::sgpr
                                                                                  : RegType::vgpr));
      bld.copy(Definition(dst), src);
      emit_split_vector(ctx, dst, dst.size());
      break;
   }
   case nir_intrinsic_ordered_xfb_counter_add_amd: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      Temp ordered_id = get_ssa_temp(ctx, instr->src[0].ssa);
      Temp counter = get_ssa_temp(ctx, instr->src[1].ssa);

      Temp gds_base = bld.copy(bld.def(v1), Operand::c32(0u));
      unsigned offset0, offset1;
      Instruction* ds_instr;
      Operand m;

      /* Lock a GDS mutex. */
      ds_ordered_count_offsets(ctx, 1 << 24u, false, false, &offset0, &offset1);
      m = bld.m0(bld.as_uniform(ordered_id));
      ds_instr =
         bld.ds(aco_opcode::ds_ordered_count, bld.def(v1), gds_base, m, offset0, offset1, true);
      ds_instr->ds().sync = memory_sync_info(storage_gds, semantic_volatile);

      aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
         aco_opcode::p_create_vector, Format::PSEUDO, instr->num_components, 1)};
      unsigned write_mask = nir_intrinsic_write_mask(instr);

      for (unsigned i = 0; i < instr->num_components; i++) {
         if (write_mask & (1 << i)) {
            Temp chan_counter = emit_extract_vector(ctx, counter, i, v1);

            ds_instr = bld.ds(aco_opcode::ds_add_gs_reg_rtn, bld.def(v1), Operand(), chan_counter,
                              i * 4, 0u, true);
            ds_instr->ds().sync = memory_sync_info(storage_gds, semantic_atomicrmw);

            vec->operands[i] = Operand(ds_instr->definitions[0].getTemp());
         } else {
            vec->operands[i] = Operand::zero();
         }
      }

      vec->definitions[0] = Definition(dst);
      ctx->block->instructions.emplace_back(std::move(vec));

      /* Unlock a GDS mutex. */
      ds_ordered_count_offsets(ctx, 1 << 24u, true, true, &offset0, &offset1);
      m = bld.m0(bld.as_uniform(ordered_id));
      ds_instr =
         bld.ds(aco_opcode::ds_ordered_count, bld.def(v1), gds_base, m, offset0, offset1, true);
      ds_instr->ds().sync = memory_sync_info(storage_gds, semantic_volatile);

      emit_split_vector(ctx, dst, instr->num_components);
      break;
   }
   case nir_intrinsic_xfb_counter_sub_amd: {
      unsigned write_mask = nir_intrinsic_write_mask(instr);
      Temp counter = get_ssa_temp(ctx, instr->src[0].ssa);

      u_foreach_bit (i, write_mask) {
         Temp chan_counter = emit_extract_vector(ctx, counter, i, v1);
         Instruction* ds_instr;

         ds_instr = bld.ds(aco_opcode::ds_sub_gs_reg_rtn, bld.def(v1), Operand(), chan_counter,
                           i * 4, 0u, true);
         ds_instr->ds().sync = memory_sync_info(storage_gds, semantic_atomicrmw);
      }
      break;
   }
   case nir_intrinsic_export_amd:
   case nir_intrinsic_export_row_amd: {
      unsigned flags = nir_intrinsic_flags(instr);
      unsigned target = nir_intrinsic_base(instr);
      unsigned write_mask = nir_intrinsic_write_mask(instr);

      /* Mark vertex export block. */
      if (target == V_008DFC_SQ_EXP_POS || target <= V_008DFC_SQ_EXP_NULL)
         ctx->block->kind |= block_kind_export_end;

      if (target < V_008DFC_SQ_EXP_MRTZ)
         ctx->program->has_color_exports = true;

      const bool row_en = instr->intrinsic == nir_intrinsic_export_row_amd;

      aco_ptr<Export_instruction> exp{
         create_instruction<Export_instruction>(aco_opcode::exp, Format::EXP, 4 + row_en, 0)};

      exp->dest = target;
      exp->enabled_mask = write_mask;
      exp->compressed = flags & AC_EXP_FLAG_COMPRESSED;

      /* ACO may reorder position/mrt export instructions, then mark done for last
       * export instruction. So don't respect the nir AC_EXP_FLAG_DONE for position/mrt
       * exports here and leave it to ACO.
       */
      if (target == V_008DFC_SQ_EXP_PRIM)
         exp->done = flags & AC_EXP_FLAG_DONE;
      else
         exp->done = false;

      /* ACO may reorder mrt export instructions, then mark valid mask for last
       * export instruction. So don't respect the nir AC_EXP_FLAG_VALID_MASK for mrt
       * exports here and leave it to ACO.
       */
      if (target > V_008DFC_SQ_EXP_NULL)
         exp->valid_mask = flags & AC_EXP_FLAG_VALID_MASK;
      else
         exp->valid_mask = false;

      exp->row_en = row_en;

      /* Compressed export uses two bits for a channel. */
      uint32_t channel_mask =
         exp->compressed ? (write_mask & 0x3 ? 1 : 0) | (write_mask & 0xc ? 2 : 0) : write_mask;

      Temp value = get_ssa_temp(ctx, instr->src[0].ssa);
      for (unsigned i = 0; i < 4; i++) {
         exp->operands[i] = channel_mask & BITFIELD_BIT(i)
                               ? Operand(emit_extract_vector(ctx, value, i, v1))
                               : Operand(v1);
      }

      if (row_en) {
         Temp row = bld.as_uniform(get_ssa_temp(ctx, instr->src[1].ssa));
         /* Hack to prevent the RA from moving the source into m0 and then back to a normal SGPR. */
         row = bld.copy(bld.def(s1, m0), row);
         exp->operands[4] = bld.m0(row);
      }

      ctx->block->instructions.emplace_back(std::move(exp));
      break;
   }
   case nir_intrinsic_export_dual_src_blend_amd: {
      Temp val0 = get_ssa_temp(ctx, instr->src[0].ssa);
      Temp val1 = get_ssa_temp(ctx, instr->src[1].ssa);
      unsigned write_mask = nir_intrinsic_write_mask(instr);

      struct aco_export_mrt mrt0, mrt1;
      for (unsigned i = 0; i < 4; i++) {
         mrt0.out[i] = write_mask & BITFIELD_BIT(i) ? Operand(emit_extract_vector(ctx, val0, i, v1))
                                                    : Operand(v1);

         mrt1.out[i] = write_mask & BITFIELD_BIT(i) ? Operand(emit_extract_vector(ctx, val1, i, v1))
                                                    : Operand(v1);
      }
      mrt0.enabled_channels = mrt1.enabled_channels = write_mask;

      create_fs_dual_src_export_gfx11(ctx, &mrt0, &mrt1);

      ctx->block->kind |= block_kind_export_end;
      break;
   }
   case nir_intrinsic_strict_wqm_coord_amd: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      Temp src = get_ssa_temp(ctx, instr->src[0].ssa);
      Temp tmp = bld.tmp(RegClass::get(RegType::vgpr, dst.bytes()));
      unsigned begin_size = nir_intrinsic_base(instr);

      unsigned num_src = 1;
      auto it = ctx->allocated_vec.find(src.id());
      if (it != ctx->allocated_vec.end())
         num_src = src.bytes() / it->second[0].bytes();

      aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
         aco_opcode::p_create_vector, Format::PSEUDO, num_src + !!begin_size, 1)};

      if (begin_size)
         vec->operands[0] = Operand(RegClass::get(RegType::vgpr, begin_size));
      for (unsigned i = 0; i < num_src; i++) {
         Temp comp = it != ctx->allocated_vec.end() ? it->second[i] : src;
         vec->operands[i + !!begin_size] = Operand(comp);
      }

      vec->definitions[0] = Definition(tmp);
      ctx->block->instructions.emplace_back(std::move(vec));

      bld.pseudo(aco_opcode::p_start_linear_vgpr, Definition(dst), tmp);
      break;
   }
   case nir_intrinsic_load_lds_ngg_scratch_base_amd: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.sop1(aco_opcode::p_load_symbol, Definition(dst),
               Operand::c32(aco_symbol_lds_ngg_scratch_base));
      break;
   }
   case nir_intrinsic_load_lds_ngg_gs_out_vertex_base_amd: {
      Temp dst = get_ssa_temp(ctx, &instr->def);
      bld.sop1(aco_opcode::p_load_symbol, Definition(dst),
               Operand::c32(aco_symbol_lds_ngg_gs_out_vertex_base));
      break;
   }
   case nir_intrinsic_store_scalar_arg_amd: {
      ctx->arg_temps[nir_intrinsic_base(instr)] =
         bld.as_uniform(get_ssa_temp(ctx, instr->src[0].ssa));
      break;
   }
   case nir_intrinsic_store_vector_arg_amd: {
      ctx->arg_temps[nir_intrinsic_base(instr)] =
         as_vgpr(ctx, get_ssa_temp(ctx, instr->src[0].ssa));
      break;
   }
   case nir_intrinsic_begin_invocation_interlock: {
      pops_await_overlapped_waves(ctx);
      break;
   }
   case nir_intrinsic_end_invocation_interlock: {
      if (ctx->options->gfx_level < GFX11)
         bld.pseudo(aco_opcode::p_pops_gfx9_ordered_section_done);
      break;
   }
   case nir_intrinsic_cmat_muladd_amd: visit_cmat_muladd(ctx, instr); break;
   default:
      isel_err(&instr->instr, "Unimplemented intrinsic instr");
      abort();

      break;
   }
}

void
get_const_vec(nir_def* vec, nir_const_value* cv[4])
{
   if (vec->parent_instr->type != nir_instr_type_alu)
      return;
   nir_alu_instr* vec_instr = nir_instr_as_alu(vec->parent_instr);
   if (vec_instr->op != nir_op_vec(vec->num_components))
      return;

   for (unsigned i = 0; i < vec->num_components; i++) {
      cv[i] =
         vec_instr->src[i].swizzle[0] == 0 ? nir_src_as_const_value(vec_instr->src[i].src) : NULL;
   }
}

void
visit_tex(isel_context* ctx, nir_tex_instr* instr)
{
   assert(instr->op != nir_texop_samples_identical);

   Builder bld(ctx->program, ctx->block);
   bool has_bias = false, has_lod = false, level_zero = false, has_compare = false,
        has_offset = false, has_ddx = false, has_ddy = false, has_derivs = false,
        has_sample_index = false, has_clamped_lod = false, has_wqm_coord = false;
   Temp resource, sampler, bias = Temp(), compare = Temp(), sample_index = Temp(), lod = Temp(),
                           offset = Temp(), ddx = Temp(), ddy = Temp(), clamped_lod = Temp(),
                           coord = Temp(), wqm_coord = Temp();
   std::vector<Temp> coords;
   std::vector<Temp> derivs;
   nir_const_value* const_offset[4] = {NULL, NULL, NULL, NULL};

   for (unsigned i = 0; i < instr->num_srcs; i++) {
      switch (instr->src[i].src_type) {
      case nir_tex_src_texture_handle:
         resource = bld.as_uniform(get_ssa_temp(ctx, instr->src[i].src.ssa));
         break;
      case nir_tex_src_sampler_handle:
         sampler = bld.as_uniform(get_ssa_temp(ctx, instr->src[i].src.ssa));
         break;
      default: break;
      }
   }

   bool tg4_integer_workarounds = ctx->options->gfx_level <= GFX8 && instr->op == nir_texop_tg4 &&
                                  (instr->dest_type & (nir_type_int | nir_type_uint));
   bool tg4_integer_cube_workaround =
      tg4_integer_workarounds && instr->sampler_dim == GLSL_SAMPLER_DIM_CUBE;

   bool a16 = false, g16 = false;

   int coord_idx = nir_tex_instr_src_index(instr, nir_tex_src_coord);
   if (coord_idx > 0)
      a16 = instr->src[coord_idx].src.ssa->bit_size == 16;

   int ddx_idx = nir_tex_instr_src_index(instr, nir_tex_src_ddx);
   if (ddx_idx > 0)
      g16 = instr->src[ddx_idx].src.ssa->bit_size == 16;

   for (unsigned i = 0; i < instr->num_srcs; i++) {
      switch (instr->src[i].src_type) {
      case nir_tex_src_coord: {
         assert(instr->src[i].src.ssa->bit_size == (a16 ? 16 : 32));
         coord = get_ssa_temp_tex(ctx, instr->src[i].src.ssa, a16);
         break;
      }
      case nir_tex_src_backend1: {
         assert(instr->src[i].src.ssa->bit_size == 32);
         wqm_coord = get_ssa_temp(ctx, instr->src[i].src.ssa);
         has_wqm_coord = true;
         break;
      }
      case nir_tex_src_bias:
         assert(instr->src[i].src.ssa->bit_size == (a16 ? 16 : 32));
         /* Doesn't need get_ssa_temp_tex because we pack it into its own dword anyway. */
         bias = get_ssa_temp(ctx, instr->src[i].src.ssa);
         has_bias = true;
         break;
      case nir_tex_src_lod: {
         if (nir_src_is_const(instr->src[i].src) && nir_src_as_uint(instr->src[i].src) == 0) {
            level_zero = true;
         } else {
            assert(instr->src[i].src.ssa->bit_size == (a16 ? 16 : 32));
            lod = get_ssa_temp_tex(ctx, instr->src[i].src.ssa, a16);
            has_lod = true;
         }
         break;
      }
      case nir_tex_src_min_lod:
         assert(instr->src[i].src.ssa->bit_size == (a16 ? 16 : 32));
         clamped_lod = get_ssa_temp_tex(ctx, instr->src[i].src.ssa, a16);
         has_clamped_lod = true;
         break;
      case nir_tex_src_comparator:
         if (instr->is_shadow) {
            assert(instr->src[i].src.ssa->bit_size == 32);
            compare = get_ssa_temp(ctx, instr->src[i].src.ssa);
            has_compare = true;
         }
         break;
      case nir_tex_src_offset:
      case nir_tex_src_backend2:
         assert(instr->src[i].src.ssa->bit_size == 32);
         offset = get_ssa_temp(ctx, instr->src[i].src.ssa);
         get_const_vec(instr->src[i].src.ssa, const_offset);
         has_offset = true;
         break;
      case nir_tex_src_ddx:
         assert(instr->src[i].src.ssa->bit_size == (g16 ? 16 : 32));
         ddx = get_ssa_temp_tex(ctx, instr->src[i].src.ssa, g16);
         has_ddx = true;
         break;
      case nir_tex_src_ddy:
         assert(instr->src[i].src.ssa->bit_size == (g16 ? 16 : 32));
         ddy = get_ssa_temp_tex(ctx, instr->src[i].src.ssa, g16);
         has_ddy = true;
         break;
      case nir_tex_src_ms_index:
         assert(instr->src[i].src.ssa->bit_size == (a16 ? 16 : 32));
         sample_index = get_ssa_temp_tex(ctx, instr->src[i].src.ssa, a16);
         has_sample_index = true;
         break;
      case nir_tex_src_texture_offset:
      case nir_tex_src_sampler_offset:
      default: break;
      }
   }

   if (has_wqm_coord) {
      assert(instr->op == nir_texop_tex || instr->op == nir_texop_txb ||
             instr->op == nir_texop_lod);
      assert(wqm_coord.regClass().is_linear_vgpr());
      assert(!a16 && !g16);
   }

   if (instr->op == nir_texop_tg4 && !has_lod && !instr->is_gather_implicit_lod)
      level_zero = true;

   if (has_offset) {
      assert(instr->op != nir_texop_txf);

      aco_ptr<Instruction> tmp_instr;
      Temp acc, pack = Temp();

      uint32_t pack_const = 0;
      for (unsigned i = 0; i < offset.size(); i++) {
         if (!const_offset[i])
            continue;
         pack_const |= (const_offset[i]->u32 & 0x3Fu) << (8u * i);
      }

      if (offset.type() == RegType::sgpr) {
         for (unsigned i = 0; i < offset.size(); i++) {
            if (const_offset[i])
               continue;

            acc = emit_extract_vector(ctx, offset, i, s1);
            acc = bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc), acc,
                           Operand::c32(0x3Fu));

            if (i) {
               acc = bld.sop2(aco_opcode::s_lshl_b32, bld.def(s1), bld.def(s1, scc), acc,
                              Operand::c32(8u * i));
            }

            if (pack == Temp()) {
               pack = acc;
            } else {
               pack = bld.sop2(aco_opcode::s_or_b32, bld.def(s1), bld.def(s1, scc), pack, acc);
            }
         }

         if (pack_const && pack != Temp())
            pack = bld.sop2(aco_opcode::s_or_b32, bld.def(s1), bld.def(s1, scc),
                            Operand::c32(pack_const), pack);
      } else {
         for (unsigned i = 0; i < offset.size(); i++) {
            if (const_offset[i])
               continue;

            acc = emit_extract_vector(ctx, offset, i, v1);
            acc = bld.vop2(aco_opcode::v_and_b32, bld.def(v1), Operand::c32(0x3Fu), acc);

            if (i) {
               acc = bld.vop2(aco_opcode::v_lshlrev_b32, bld.def(v1), Operand::c32(8u * i), acc);
            }

            if (pack == Temp()) {
               pack = acc;
            } else {
               pack = bld.vop2(aco_opcode::v_or_b32, bld.def(v1), pack, acc);
            }
         }

         if (pack_const && pack != Temp())
            pack = bld.vop2(aco_opcode::v_or_b32, bld.def(v1), Operand::c32(pack_const), pack);
      }
      if (pack == Temp())
         offset = bld.copy(bld.def(v1), Operand::c32(pack_const));
      else
         offset = pack;
   }

   std::vector<Temp> unpacked_coord;
   if (coord != Temp())
      unpacked_coord.push_back(coord);
   if (has_sample_index)
      unpacked_coord.push_back(sample_index);
   if (has_lod)
      unpacked_coord.push_back(lod);
   if (has_clamped_lod)
      unpacked_coord.push_back(clamped_lod);

   coords = emit_pack_v1(ctx, unpacked_coord);

   /* pack derivatives */
   if (has_ddx || has_ddy) {
      assert(a16 == g16 || ctx->options->gfx_level >= GFX10);
      std::array<Temp, 2> ddxddy = {ddx, ddy};
      for (Temp tmp : ddxddy) {
         if (tmp == Temp())
            continue;
         std::vector<Temp> unpacked = {tmp};
         for (Temp derv : emit_pack_v1(ctx, unpacked))
            derivs.push_back(derv);
      }
      has_derivs = true;
   }

   unsigned dim = 0;
   bool da = false;
   if (instr->sampler_dim != GLSL_SAMPLER_DIM_BUF) {
      dim = ac_get_sampler_dim(ctx->options->gfx_level, instr->sampler_dim, instr->is_array);
      da = should_declare_array((ac_image_dim)dim);
   }

   /* Build tex instruction */
   unsigned dmask = nir_def_components_read(&instr->def) & 0xf;
   if (instr->sampler_dim == GLSL_SAMPLER_DIM_BUF)
      dmask = u_bit_consecutive(0, util_last_bit(dmask));
   if (instr->is_sparse)
      dmask = MAX2(dmask, 1) | 0x10;
   bool d16 = instr->def.bit_size == 16;
   Temp dst = get_ssa_temp(ctx, &instr->def);
   Temp tmp_dst = dst;

   /* gather4 selects the component by dmask and always returns vec4 (vec5 if sparse) */
   if (instr->op == nir_texop_tg4) {
      assert(instr->def.num_components == (4 + instr->is_sparse));
      if (instr->is_shadow)
         dmask = 1;
      else
         dmask = 1 << instr->component;
      if (tg4_integer_cube_workaround || dst.type() == RegType::sgpr)
         tmp_dst = bld.tmp(instr->is_sparse ? v5 : (d16 ? v2 : v4));
   } else if (instr->op == nir_texop_fragment_mask_fetch_amd) {
      tmp_dst = bld.tmp(v1);
   } else if (util_bitcount(dmask) != instr->def.num_components || dst.type() == RegType::sgpr) {
      unsigned bytes = util_bitcount(dmask) * instr->def.bit_size / 8;
      tmp_dst = bld.tmp(RegClass::get(RegType::vgpr, bytes));
   }

   Temp tg4_compare_cube_wa64 = Temp();

   if (tg4_integer_workarounds) {
      Temp half_texel[2];
      if (instr->sampler_dim == GLSL_SAMPLER_DIM_RECT) {
         half_texel[0] = half_texel[1] = bld.copy(bld.def(v1), Operand::c32(0xbf000000 /*-0.5*/));
      } else {
         Temp tg4_lod = bld.copy(bld.def(v1), Operand::zero());
         Temp size = bld.tmp(v2);
         MIMG_instruction* tex = emit_mimg(bld, aco_opcode::image_get_resinfo, size, resource,
                                           Operand(s4), std::vector<Temp>{tg4_lod});
         tex->dim = dim;
         tex->dmask = 0x3;
         tex->da = da;
         emit_split_vector(ctx, size, size.size());

         for (unsigned i = 0; i < 2; i++) {
            half_texel[i] = emit_extract_vector(ctx, size, i, v1);
            half_texel[i] = bld.vop1(aco_opcode::v_cvt_f32_i32, bld.def(v1), half_texel[i]);
            half_texel[i] = bld.vop1(aco_opcode::v_rcp_iflag_f32, bld.def(v1), half_texel[i]);
            half_texel[i] = bld.vop2(aco_opcode::v_mul_f32, bld.def(v1),
                                     Operand::c32(0xbf000000 /*-0.5*/), half_texel[i]);
         }

         if (instr->sampler_dim == GLSL_SAMPLER_DIM_2D && !instr->is_array) {
            /* In vulkan, whether the sampler uses unnormalized
             * coordinates or not is a dynamic property of the
             * sampler. Hence, to figure out whether or not we
             * need to divide by the texture size, we need to test
             * the sampler at runtime. This tests the bit set by
             * radv_init_sampler().
             */
            unsigned bit_idx = ffs(S_008F30_FORCE_UNNORMALIZED(1)) - 1;
            Temp dword0 = emit_extract_vector(ctx, sampler, 0, s1);
            Temp not_needed =
               bld.sopc(aco_opcode::s_bitcmp0_b32, bld.def(s1, scc), dword0, Operand::c32(bit_idx));

            not_needed = bool_to_vector_condition(ctx, not_needed);
            half_texel[0] = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1),
                                     Operand::c32(0xbf000000 /*-0.5*/), half_texel[0], not_needed);
            half_texel[1] = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1),
                                     Operand::c32(0xbf000000 /*-0.5*/), half_texel[1], not_needed);
         }
      }

      Temp new_coords[2] = {bld.vop2(aco_opcode::v_add_f32, bld.def(v1), coords[0], half_texel[0]),
                            bld.vop2(aco_opcode::v_add_f32, bld.def(v1), coords[1], half_texel[1])};

      if (tg4_integer_cube_workaround) {
         /* see comment in ac_nir_to_llvm.c's lower_gather4_integer() */
         Temp* const desc = (Temp*)alloca(resource.size() * sizeof(Temp));
         aco_ptr<Instruction> split{create_instruction<Pseudo_instruction>(
            aco_opcode::p_split_vector, Format::PSEUDO, 1, resource.size())};
         split->operands[0] = Operand(resource);
         for (unsigned i = 0; i < resource.size(); i++) {
            desc[i] = bld.tmp(s1);
            split->definitions[i] = Definition(desc[i]);
         }
         ctx->block->instructions.emplace_back(std::move(split));

         Temp dfmt = bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc), desc[1],
                              Operand::c32(20u | (6u << 16)));
         Temp compare_cube_wa = bld.sopc(aco_opcode::s_cmp_eq_u32, bld.def(s1, scc), dfmt,
                                         Operand::c32(V_008F14_IMG_DATA_FORMAT_8_8_8_8));

         Temp nfmt;
         if (instr->dest_type & nir_type_uint) {
            nfmt = bld.sop2(aco_opcode::s_cselect_b32, bld.def(s1),
                            Operand::c32(V_008F14_IMG_NUM_FORMAT_USCALED),
                            Operand::c32(V_008F14_IMG_NUM_FORMAT_UINT), bld.scc(compare_cube_wa));
         } else {
            nfmt = bld.sop2(aco_opcode::s_cselect_b32, bld.def(s1),
                            Operand::c32(V_008F14_IMG_NUM_FORMAT_SSCALED),
                            Operand::c32(V_008F14_IMG_NUM_FORMAT_SINT), bld.scc(compare_cube_wa));
         }
         tg4_compare_cube_wa64 = bld.tmp(bld.lm);
         bool_to_vector_condition(ctx, compare_cube_wa, tg4_compare_cube_wa64);

         nfmt = bld.sop2(aco_opcode::s_lshl_b32, bld.def(s1), bld.def(s1, scc), nfmt,
                         Operand::c32(26u));

         desc[1] = bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc), desc[1],
                            Operand::c32(C_008F14_NUM_FORMAT));
         desc[1] = bld.sop2(aco_opcode::s_or_b32, bld.def(s1), bld.def(s1, scc), desc[1], nfmt);

         aco_ptr<Instruction> vec{create_instruction<Pseudo_instruction>(
            aco_opcode::p_create_vector, Format::PSEUDO, resource.size(), 1)};
         for (unsigned i = 0; i < resource.size(); i++)
            vec->operands[i] = Operand(desc[i]);
         resource = bld.tmp(resource.regClass());
         vec->definitions[0] = Definition(resource);
         ctx->block->instructions.emplace_back(std::move(vec));

         new_coords[0] = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), new_coords[0], coords[0],
                                  tg4_compare_cube_wa64);
         new_coords[1] = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), new_coords[1], coords[1],
                                  tg4_compare_cube_wa64);
      }
      coords[0] = new_coords[0];
      coords[1] = new_coords[1];
   }

   if (instr->sampler_dim == GLSL_SAMPLER_DIM_BUF) {
      // FIXME: if (ctx->abi->gfx9_stride_size_workaround) return
      // ac_build_buffer_load_format_gfx9_safe()

      assert(coords.size() == 1);
      aco_opcode op;
      if (d16) {
         switch (util_last_bit(dmask & 0xf)) {
         case 1: op = aco_opcode::buffer_load_format_d16_x; break;
         case 2: op = aco_opcode::buffer_load_format_d16_xy; break;
         case 3: op = aco_opcode::buffer_load_format_d16_xyz; break;
         case 4: op = aco_opcode::buffer_load_format_d16_xyzw; break;
         default: unreachable("Tex instruction loads more than 4 components.");
         }
      } else {
         switch (util_last_bit(dmask & 0xf)) {
         case 1: op = aco_opcode::buffer_load_format_x; break;
         case 2: op = aco_opcode::buffer_load_format_xy; break;
         case 3: op = aco_opcode::buffer_load_format_xyz; break;
         case 4: op = aco_opcode::buffer_load_format_xyzw; break;
         default: unreachable("Tex instruction loads more than 4 components.");
         }
      }

      aco_ptr<MUBUF_instruction> mubuf{
         create_instruction<MUBUF_instruction>(op, Format::MUBUF, 3 + instr->is_sparse, 1)};
      mubuf->operands[0] = Operand(resource);
      mubuf->operands[1] = Operand(coords[0]);
      mubuf->operands[2] = Operand::c32(0);
      mubuf->definitions[0] = Definition(tmp_dst);
      mubuf->idxen = true;
      mubuf->tfe = instr->is_sparse;
      if (mubuf->tfe)
         mubuf->operands[3] = emit_tfe_init(bld, tmp_dst);
      ctx->block->instructions.emplace_back(std::move(mubuf));

      expand_vector(ctx, tmp_dst, dst, instr->def.num_components, dmask);
      return;
   }

   /* gather MIMG address components */
   std::vector<Temp> args;
   if (has_wqm_coord) {
      args.emplace_back(wqm_coord);
      if (!(ctx->block->kind & block_kind_top_level))
         ctx->unended_linear_vgprs.push_back(wqm_coord);
   }
   if (has_offset)
      args.emplace_back(offset);
   if (has_bias)
      args.emplace_back(emit_pack_v1(ctx, {bias})[0]);
   if (has_compare)
      args.emplace_back(compare);
   if (has_derivs)
      args.insert(args.end(), derivs.begin(), derivs.end());

   args.insert(args.end(), coords.begin(), coords.end());

   if (instr->op == nir_texop_txf || instr->op == nir_texop_fragment_fetch_amd ||
       instr->op == nir_texop_fragment_mask_fetch_amd || instr->op == nir_texop_txf_ms) {
      aco_opcode op = level_zero || instr->sampler_dim == GLSL_SAMPLER_DIM_MS ||
                            instr->sampler_dim == GLSL_SAMPLER_DIM_SUBPASS_MS
                         ? aco_opcode::image_load
                         : aco_opcode::image_load_mip;
      Operand vdata = instr->is_sparse ? emit_tfe_init(bld, tmp_dst) : Operand(v1);
      MIMG_instruction* tex = emit_mimg(bld, op, tmp_dst, resource, Operand(s4), args, vdata);
      if (instr->op == nir_texop_fragment_mask_fetch_amd)
         tex->dim = da ? ac_image_2darray : ac_image_2d;
      else
         tex->dim = dim;
      tex->dmask = dmask & 0xf;
      tex->unrm = true;
      tex->da = da;
      tex->tfe = instr->is_sparse;
      tex->d16 = d16;
      tex->a16 = a16;

      if (instr->op == nir_texop_fragment_mask_fetch_amd) {
         /* Use 0x76543210 if the image doesn't have FMASK. */
         assert(dmask == 1 && dst.bytes() == 4);
         assert(dst.id() != tmp_dst.id());

         if (dst.regClass() == s1) {
            Temp is_not_null = bld.sopc(aco_opcode::s_cmp_lg_u32, bld.def(s1, scc), Operand::zero(),
                                        emit_extract_vector(ctx, resource, 1, s1));
            bld.sop2(aco_opcode::s_cselect_b32, Definition(dst), bld.as_uniform(tmp_dst),
                     Operand::c32(0x76543210), bld.scc(is_not_null));
         } else {
            Temp is_not_null = bld.tmp(bld.lm);
            bld.vopc_e64(aco_opcode::v_cmp_lg_u32, Definition(is_not_null), Operand::zero(),
                         emit_extract_vector(ctx, resource, 1, s1));
            bld.vop2(aco_opcode::v_cndmask_b32, Definition(dst),
                     bld.copy(bld.def(v1), Operand::c32(0x76543210)), tmp_dst, is_not_null);
         }
      } else {
         expand_vector(ctx, tmp_dst, dst, instr->def.num_components, dmask);
      }
      return;
   }

   bool separate_g16 = ctx->options->gfx_level >= GFX10 && g16;

   // TODO: would be better to do this by adding offsets, but needs the opcodes ordered.
   aco_opcode opcode = aco_opcode::image_sample;
   if (has_offset) { /* image_sample_*_o */
      if (has_clamped_lod) {
         if (has_compare) {
            opcode = aco_opcode::image_sample_c_cl_o;
            if (separate_g16)
               opcode = aco_opcode::image_sample_c_d_cl_o_g16;
            else if (has_derivs)
               opcode = aco_opcode::image_sample_c_d_cl_o;
            if (has_bias)
               opcode = aco_opcode::image_sample_c_b_cl_o;
         } else {
            opcode = aco_opcode::image_sample_cl_o;
            if (separate_g16)
               opcode = aco_opcode::image_sample_d_cl_o_g16;
            else if (has_derivs)
               opcode = aco_opcode::image_sample_d_cl_o;
            if (has_bias)
               opcode = aco_opcode::image_sample_b_cl_o;
         }
      } else if (has_compare) {
         opcode = aco_opcode::image_sample_c_o;
         if (separate_g16)
            opcode = aco_opcode::image_sample_c_d_o_g16;
         else if (has_derivs)
            opcode = aco_opcode::image_sample_c_d_o;
         if (has_bias)
            opcode = aco_opcode::image_sample_c_b_o;
         if (level_zero)
            opcode = aco_opcode::image_sample_c_lz_o;
         if (has_lod)
            opcode = aco_opcode::image_sample_c_l_o;
      } else {
         opcode = aco_opcode::image_sample_o;
         if (separate_g16)
            opcode = aco_opcode::image_sample_d_o_g16;
         else if (has_derivs)
            opcode = aco_opcode::image_sample_d_o;
         if (has_bias)
            opcode = aco_opcode::image_sample_b_o;
         if (level_zero)
            opcode = aco_opcode::image_sample_lz_o;
         if (has_lod)
            opcode = aco_opcode::image_sample_l_o;
      }
   } else if (has_clamped_lod) { /* image_sample_*_cl */
      if (has_compare) {
         opcode = aco_opcode::image_sample_c_cl;
         if (separate_g16)
            opcode = aco_opcode::image_sample_c_d_cl_g16;
         else if (has_derivs)
            opcode = aco_opcode::image_sample_c_d_cl;
         if (has_bias)
            opcode = aco_opcode::image_sample_c_b_cl;
      } else {
         opcode = aco_opcode::image_sample_cl;
         if (separate_g16)
            opcode = aco_opcode::image_sample_d_cl_g16;
         else if (has_derivs)
            opcode = aco_opcode::image_sample_d_cl;
         if (has_bias)
            opcode = aco_opcode::image_sample_b_cl;
      }
   } else { /* no offset */
      if (has_compare) {
         opcode = aco_opcode::image_sample_c;
         if (separate_g16)
            opcode = aco_opcode::image_sample_c_d_g16;
         else if (has_derivs)
            opcode = aco_opcode::image_sample_c_d;
         if (has_bias)
            opcode = aco_opcode::image_sample_c_b;
         if (level_zero)
            opcode = aco_opcode::image_sample_c_lz;
         if (has_lod)
            opcode = aco_opcode::image_sample_c_l;
      } else {
         opcode = aco_opcode::image_sample;
         if (separate_g16)
            opcode = aco_opcode::image_sample_d_g16;
         else if (has_derivs)
            opcode = aco_opcode::image_sample_d;
         if (has_bias)
            opcode = aco_opcode::image_sample_b;
         if (level_zero)
            opcode = aco_opcode::image_sample_lz;
         if (has_lod)
            opcode = aco_opcode::image_sample_l;
      }
   }

   if (instr->op == nir_texop_tg4) {
      /* GFX11 supports implicit LOD, but the extension is unsupported. */
      assert(level_zero || ctx->options->gfx_level < GFX11);

      if (has_offset) { /* image_gather4_*_o */
         if (has_compare) {
            opcode = aco_opcode::image_gather4_c_o;
            if (level_zero)
               opcode = aco_opcode::image_gather4_c_lz_o;
            if (has_lod)
               opcode = aco_opcode::image_gather4_c_l_o;
            if (has_bias)
               opcode = aco_opcode::image_gather4_c_b_o;
         } else {
            opcode = aco_opcode::image_gather4_o;
            if (level_zero)
               opcode = aco_opcode::image_gather4_lz_o;
            if (has_lod)
               opcode = aco_opcode::image_gather4_l_o;
            if (has_bias)
               opcode = aco_opcode::image_gather4_b_o;
         }
      } else {
         if (has_compare) {
            opcode = aco_opcode::image_gather4_c;
            if (level_zero)
               opcode = aco_opcode::image_gather4_c_lz;
            if (has_lod)
               opcode = aco_opcode::image_gather4_c_l;
            if (has_bias)
               opcode = aco_opcode::image_gather4_c_b;
         } else {
            opcode = aco_opcode::image_gather4;
            if (level_zero)
               opcode = aco_opcode::image_gather4_lz;
            if (has_lod)
               opcode = aco_opcode::image_gather4_l;
            if (has_bias)
               opcode = aco_opcode::image_gather4_b;
         }
      }
   } else if (instr->op == nir_texop_lod) {
      opcode = aco_opcode::image_get_lod;
   }

   bool implicit_derivs = bld.program->stage == fragment_fs && !has_derivs && !has_lod &&
                          !level_zero && instr->sampler_dim != GLSL_SAMPLER_DIM_MS &&
                          instr->sampler_dim != GLSL_SAMPLER_DIM_SUBPASS_MS;

   Operand vdata = instr->is_sparse ? emit_tfe_init(bld, tmp_dst) : Operand(v1);
   MIMG_instruction* tex = emit_mimg(bld, opcode, tmp_dst, resource, Operand(sampler), args, vdata);
   tex->dim = dim;
   tex->dmask = dmask & 0xf;
   tex->da = da;
   tex->unrm = instr->sampler_dim == GLSL_SAMPLER_DIM_RECT;
   tex->tfe = instr->is_sparse;
   tex->d16 = d16;
   tex->a16 = a16;
   if (implicit_derivs)
      set_wqm(ctx, true);

   if (tg4_integer_cube_workaround) {
      assert(tmp_dst.id() != dst.id());
      assert(tmp_dst.size() == dst.size());

      emit_split_vector(ctx, tmp_dst, tmp_dst.size());
      Temp val[4];
      for (unsigned i = 0; i < 4; i++) {
         val[i] = emit_extract_vector(ctx, tmp_dst, i, v1);
         Temp cvt_val;
         if (instr->dest_type & nir_type_uint)
            cvt_val = bld.vop1(aco_opcode::v_cvt_u32_f32, bld.def(v1), val[i]);
         else
            cvt_val = bld.vop1(aco_opcode::v_cvt_i32_f32, bld.def(v1), val[i]);
         val[i] = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), val[i], cvt_val,
                           tg4_compare_cube_wa64);
      }

      Temp tmp = dst.regClass() == tmp_dst.regClass() ? dst : bld.tmp(tmp_dst.regClass());
      if (instr->is_sparse)
         tmp_dst = bld.pseudo(aco_opcode::p_create_vector, Definition(tmp), val[0], val[1], val[2],
                              val[3], emit_extract_vector(ctx, tmp_dst, 4, v1));
      else
         tmp_dst = bld.pseudo(aco_opcode::p_create_vector, Definition(tmp), val[0], val[1], val[2],
                              val[3]);
   }
   unsigned mask = instr->op == nir_texop_tg4 ? (instr->is_sparse ? 0x1F : 0xF) : dmask;
   expand_vector(ctx, tmp_dst, dst, instr->def.num_components, mask);
}

Operand
get_phi_operand(isel_context* ctx, nir_def* ssa, RegClass rc, bool logical)
{
   Temp tmp = get_ssa_temp(ctx, ssa);
   if (ssa->parent_instr->type == nir_instr_type_undef) {
      return Operand(rc);
   } else if (logical && ssa->bit_size == 1 &&
              ssa->parent_instr->type == nir_instr_type_load_const) {
      bool val = nir_instr_as_load_const(ssa->parent_instr)->value[0].b;
      return Operand::c32_or_c64(val ? -1 : 0, ctx->program->lane_mask == s2);
   } else {
      return Operand(tmp);
   }
}

void
visit_phi(isel_context* ctx, nir_phi_instr* instr)
{
   aco_ptr<Pseudo_instruction> phi;
   Temp dst = get_ssa_temp(ctx, &instr->def);
   assert(instr->def.bit_size != 1 || dst.regClass() == ctx->program->lane_mask);

   bool logical = !dst.is_linear() || instr->def.divergent;
   logical |= (ctx->block->kind & block_kind_merge) != 0;
   aco_opcode opcode = logical ? aco_opcode::p_phi : aco_opcode::p_linear_phi;

   /* we want a sorted list of sources, since the predecessor list is also sorted */
   std::map<unsigned, nir_def*> phi_src;
   nir_foreach_phi_src (src, instr)
      phi_src[src->pred->index] = src->src.ssa;

   std::vector<unsigned>& preds = logical ? ctx->block->logical_preds : ctx->block->linear_preds;
   unsigned num_operands = 0;
   Operand* const operands = (Operand*)alloca(
      (std::max(exec_list_length(&instr->srcs), (unsigned)preds.size()) + 1) * sizeof(Operand));
   unsigned num_defined = 0;
   unsigned cur_pred_idx = 0;
   for (std::pair<unsigned, nir_def*> src : phi_src) {
      if (cur_pred_idx < preds.size()) {
         /* handle missing preds (IF merges with discard/break) and extra preds
          * (loop exit with discard) */
         unsigned block = ctx->cf_info.nir_to_aco[src.first];
         unsigned skipped = 0;
         while (cur_pred_idx + skipped < preds.size() && preds[cur_pred_idx + skipped] != block)
            skipped++;
         if (cur_pred_idx + skipped < preds.size()) {
            for (unsigned i = 0; i < skipped; i++)
               operands[num_operands++] = Operand(dst.regClass());
            cur_pred_idx += skipped;
         } else {
            continue;
         }
      }
      /* Handle missing predecessors at the end. This shouldn't happen with loop
       * headers and we can't ignore these sources for loop header phis. */
      if (!(ctx->block->kind & block_kind_loop_header) && cur_pred_idx >= preds.size())
         continue;
      cur_pred_idx++;
      Operand op = get_phi_operand(ctx, src.second, dst.regClass(), logical);
      operands[num_operands++] = op;
      num_defined += !op.isUndefined();
   }
   /* handle block_kind_continue_or_break at loop exit blocks */
   while (cur_pred_idx++ < preds.size())
      operands[num_operands++] = Operand(dst.regClass());

   /* If the loop ends with a break, still add a linear continue edge in case
    * that break is divergent or continue_or_break is used. We'll either remove
    * this operand later in visit_loop() if it's not necessary or replace the
    * undef with something correct. */
   if (!logical && ctx->block->kind & block_kind_loop_header) {
      nir_loop* loop = nir_cf_node_as_loop(instr->instr.block->cf_node.parent);
      nir_block* last = nir_loop_last_block(loop);
      if (last->successors[0] != instr->instr.block)
         operands[num_operands++] = Operand(RegClass());
   }

   /* we can use a linear phi in some cases if one src is undef */
   if (dst.is_linear() && ctx->block->kind & block_kind_merge && num_defined == 1) {
      phi.reset(create_instruction<Pseudo_instruction>(aco_opcode::p_linear_phi, Format::PSEUDO,
                                                       num_operands, 1));

      Block* linear_else = &ctx->program->blocks[ctx->block->linear_preds[1]];
      Block* invert = &ctx->program->blocks[linear_else->linear_preds[0]];
      assert(invert->kind & block_kind_invert);

      unsigned then_block = invert->linear_preds[0];

      Block* insert_block = NULL;
      for (unsigned i = 0; i < num_operands; i++) {
         Operand op = operands[i];
         if (op.isUndefined())
            continue;
         insert_block = ctx->block->logical_preds[i] == then_block ? invert : ctx->block;
         phi->operands[0] = op;
         break;
      }
      assert(insert_block); /* should be handled by the "num_defined == 0" case above */
      phi->operands[1] = Operand(dst.regClass());
      phi->definitions[0] = Definition(dst);
      insert_block->instructions.emplace(insert_block->instructions.begin(), std::move(phi));
      return;
   }

   phi.reset(create_instruction<Pseudo_instruction>(opcode, Format::PSEUDO, num_operands, 1));
   for (unsigned i = 0; i < num_operands; i++)
      phi->operands[i] = operands[i];
   phi->definitions[0] = Definition(dst);
   ctx->block->instructions.emplace(ctx->block->instructions.begin(), std::move(phi));
}

void
visit_undef(isel_context* ctx, nir_undef_instr* instr)
{
   Temp dst = get_ssa_temp(ctx, &instr->def);

   assert(dst.type() == RegType::sgpr);

   if (dst.size() == 1) {
      Builder(ctx->program, ctx->block).copy(Definition(dst), Operand::zero());
   } else {
      aco_ptr<Pseudo_instruction> vec{create_instruction<Pseudo_instruction>(
         aco_opcode::p_create_vector, Format::PSEUDO, dst.size(), 1)};
      for (unsigned i = 0; i < dst.size(); i++)
         vec->operands[i] = Operand::zero();
      vec->definitions[0] = Definition(dst);
      ctx->block->instructions.emplace_back(std::move(vec));
   }
}

void
begin_loop(isel_context* ctx, loop_context* lc)
{
   // TODO: we might want to wrap the loop around a branch if exec_potentially_empty=true
   append_logical_end(ctx->block);
   ctx->block->kind |= block_kind_loop_preheader | block_kind_uniform;
   Builder bld(ctx->program, ctx->block);
   bld.branch(aco_opcode::p_branch, bld.def(s2));
   unsigned loop_preheader_idx = ctx->block->index;

   lc->loop_exit.kind |= (block_kind_loop_exit | (ctx->block->kind & block_kind_top_level));

   ctx->program->next_loop_depth++;

   Block* loop_header = ctx->program->create_and_insert_block();
   loop_header->kind |= block_kind_loop_header;
   add_edge(loop_preheader_idx, loop_header);
   ctx->block = loop_header;

   append_logical_start(ctx->block);

   lc->header_idx_old = std::exchange(ctx->cf_info.parent_loop.header_idx, loop_header->index);
   lc->exit_old = std::exchange(ctx->cf_info.parent_loop.exit, &lc->loop_exit);
   lc->divergent_cont_old = std::exchange(ctx->cf_info.parent_loop.has_divergent_continue, false);
   lc->divergent_branch_old = std::exchange(ctx->cf_info.parent_loop.has_divergent_branch, false);
   lc->divergent_if_old = std::exchange(ctx->cf_info.parent_if.is_divergent, false);
}

void
end_loop(isel_context* ctx, loop_context* lc)
{
   // TODO: what if a loop ends with a unconditional or uniformly branched continue
   //       and this branch is never taken?
   if (!ctx->cf_info.has_branch) {
      unsigned loop_header_idx = ctx->cf_info.parent_loop.header_idx;
      Builder bld(ctx->program, ctx->block);
      append_logical_end(ctx->block);

      if (ctx->cf_info.exec_potentially_empty_discard ||
          ctx->cf_info.exec_potentially_empty_break) {
         /* Discards can result in code running with an empty exec mask.
          * This would result in divergent breaks not ever being taken. As a
          * workaround, break the loop when the loop mask is empty instead of
          * always continuing. */
         ctx->block->kind |= (block_kind_continue_or_break | block_kind_uniform);
         unsigned block_idx = ctx->block->index;

         /* create helper blocks to avoid critical edges */
         Block* break_block = ctx->program->create_and_insert_block();
         break_block->kind = block_kind_uniform;
         bld.reset(break_block);
         bld.branch(aco_opcode::p_branch, bld.def(s2));
         add_linear_edge(block_idx, break_block);
         add_linear_edge(break_block->index, &lc->loop_exit);

         Block* continue_block = ctx->program->create_and_insert_block();
         continue_block->kind = block_kind_uniform;
         bld.reset(continue_block);
         bld.branch(aco_opcode::p_branch, bld.def(s2));
         add_linear_edge(block_idx, continue_block);
         add_linear_edge(continue_block->index, &ctx->program->blocks[loop_header_idx]);

         if (!ctx->cf_info.parent_loop.has_divergent_branch)
            add_logical_edge(block_idx, &ctx->program->blocks[loop_header_idx]);
         ctx->block = &ctx->program->blocks[block_idx];
      } else {
         ctx->block->kind |= (block_kind_continue | block_kind_uniform);
         if (!ctx->cf_info.parent_loop.has_divergent_branch)
            add_edge(ctx->block->index, &ctx->program->blocks[loop_header_idx]);
         else
            add_linear_edge(ctx->block->index, &ctx->program->blocks[loop_header_idx]);
      }

      bld.reset(ctx->block);
      bld.branch(aco_opcode::p_branch, bld.def(s2));
   }

   ctx->cf_info.has_branch = false;
   ctx->program->next_loop_depth--;

   // TODO: if the loop has not a single exit, we must add one °°
   /* emit loop successor block */
   ctx->block = ctx->program->insert_block(std::move(lc->loop_exit));
   append_logical_start(ctx->block);

#if 0
   // TODO: check if it is beneficial to not branch on continues
   /* trim linear phis in loop header */
   for (auto&& instr : loop_entry->instructions) {
      if (instr->opcode == aco_opcode::p_linear_phi) {
         aco_ptr<Pseudo_instruction> new_phi{create_instruction<Pseudo_instruction>(aco_opcode::p_linear_phi, Format::PSEUDO, loop_entry->linear_predecessors.size(), 1)};
         new_phi->definitions[0] = instr->definitions[0];
         for (unsigned i = 0; i < new_phi->operands.size(); i++)
            new_phi->operands[i] = instr->operands[i];
         /* check that the remaining operands are all the same */
         for (unsigned i = new_phi->operands.size(); i < instr->operands.size(); i++)
            assert(instr->operands[i].tempId() == instr->operands.back().tempId());
         instr.swap(new_phi);
      } else if (instr->opcode == aco_opcode::p_phi) {
         continue;
      } else {
         break;
      }
   }
#endif

   ctx->cf_info.parent_loop.header_idx = lc->header_idx_old;
   ctx->cf_info.parent_loop.exit = lc->exit_old;
   ctx->cf_info.parent_loop.has_divergent_continue = lc->divergent_cont_old;
   ctx->cf_info.parent_loop.has_divergent_branch = lc->divergent_branch_old;
   ctx->cf_info.parent_if.is_divergent = lc->divergent_if_old;
   if (!ctx->block->loop_nest_depth && !ctx->cf_info.parent_if.is_divergent)
      ctx->cf_info.exec_potentially_empty_discard = false;
}

void
emit_loop_jump(isel_context* ctx, bool is_break)
{
   Builder bld(ctx->program, ctx->block);
   Block* logical_target;
   append_logical_end(ctx->block);
   unsigned idx = ctx->block->index;

   if (is_break) {
      logical_target = ctx->cf_info.parent_loop.exit;
      add_logical_edge(idx, logical_target);
      ctx->block->kind |= block_kind_break;

      if (!ctx->cf_info.parent_if.is_divergent &&
          !ctx->cf_info.parent_loop.has_divergent_continue) {
         /* uniform break - directly jump out of the loop */
         ctx->block->kind |= block_kind_uniform;
         ctx->cf_info.has_branch = true;
         bld.branch(aco_opcode::p_branch, bld.def(s2));
         add_linear_edge(idx, logical_target);
         return;
      }
      ctx->cf_info.parent_loop.has_divergent_branch = true;
   } else {
      logical_target = &ctx->program->blocks[ctx->cf_info.parent_loop.header_idx];
      add_logical_edge(idx, logical_target);
      ctx->block->kind |= block_kind_continue;

      if (!ctx->cf_info.parent_if.is_divergent) {
         /* uniform continue - directly jump to the loop header */
         ctx->block->kind |= block_kind_uniform;
         ctx->cf_info.has_branch = true;
         bld.branch(aco_opcode::p_branch, bld.def(s2));
         add_linear_edge(idx, logical_target);
         return;
      }

      /* for potential uniform breaks after this continue,
         we must ensure that they are handled correctly */
      ctx->cf_info.parent_loop.has_divergent_continue = true;
      ctx->cf_info.parent_loop.has_divergent_branch = true;
   }

   if (ctx->cf_info.parent_if.is_divergent && !ctx->cf_info.exec_potentially_empty_break) {
      ctx->cf_info.exec_potentially_empty_break = true;
      ctx->cf_info.exec_potentially_empty_break_depth = ctx->block->loop_nest_depth;
   }

   /* remove critical edges from linear CFG */
   bld.branch(aco_opcode::p_branch, bld.def(s2));
   Block* break_block = ctx->program->create_and_insert_block();
   break_block->kind |= block_kind_uniform;
   add_linear_edge(idx, break_block);
   /* the loop_header pointer might be invalidated by this point */
   if (!is_break)
      logical_target = &ctx->program->blocks[ctx->cf_info.parent_loop.header_idx];
   add_linear_edge(break_block->index, logical_target);
   bld.reset(break_block);
   bld.branch(aco_opcode::p_branch, bld.def(s2));

   Block* continue_block = ctx->program->create_and_insert_block();
   add_linear_edge(idx, continue_block);
   append_logical_start(continue_block);
   ctx->block = continue_block;
}

void
emit_loop_break(isel_context* ctx)
{
   emit_loop_jump(ctx, true);
}

void
emit_loop_continue(isel_context* ctx)
{
   emit_loop_jump(ctx, false);
}

void
visit_jump(isel_context* ctx, nir_jump_instr* instr)
{
   /* visit_block() would usually do this but divergent jumps updates ctx->block */
   ctx->cf_info.nir_to_aco[instr->instr.block->index] = ctx->block->index;

   switch (instr->type) {
   case nir_jump_break: emit_loop_break(ctx); break;
   case nir_jump_continue: emit_loop_continue(ctx); break;
   default: isel_err(&instr->instr, "Unknown NIR jump instr"); abort();
   }
}

void
visit_block(isel_context* ctx, nir_block* block)
{
   if (ctx->block->kind & block_kind_top_level) {
      Builder bld(ctx->program, ctx->block);
      for (Temp tmp : ctx->unended_linear_vgprs)
         bld.pseudo(aco_opcode::p_end_linear_vgpr, tmp);
      ctx->unended_linear_vgprs.clear();
   }

   ctx->block->instructions.reserve(ctx->block->instructions.size() +
                                    exec_list_length(&block->instr_list) * 2);
   nir_foreach_instr (instr, block) {
      switch (instr->type) {
      case nir_instr_type_alu: visit_alu_instr(ctx, nir_instr_as_alu(instr)); break;
      case nir_instr_type_load_const: visit_load_const(ctx, nir_instr_as_load_const(instr)); break;
      case nir_instr_type_intrinsic: visit_intrinsic(ctx, nir_instr_as_intrinsic(instr)); break;
      case nir_instr_type_tex: visit_tex(ctx, nir_instr_as_tex(instr)); break;
      case nir_instr_type_phi: visit_phi(ctx, nir_instr_as_phi(instr)); break;
      case nir_instr_type_undef: visit_undef(ctx, nir_instr_as_undef(instr)); break;
      case nir_instr_type_deref: break;
      case nir_instr_type_jump: visit_jump(ctx, nir_instr_as_jump(instr)); break;
      default: isel_err(instr, "Unknown NIR instr type");
      }
   }

   if (!ctx->cf_info.parent_loop.has_divergent_branch)
      ctx->cf_info.nir_to_aco[block->index] = ctx->block->index;
}

static Operand
create_continue_phis(isel_context* ctx, unsigned first, unsigned last,
                     aco_ptr<Instruction>& header_phi, Operand* vals)
{
   vals[0] = Operand(header_phi->definitions[0].getTemp());
   RegClass rc = vals[0].regClass();

   unsigned loop_nest_depth = ctx->program->blocks[first].loop_nest_depth;

   unsigned next_pred = 1;

   for (unsigned idx = first + 1; idx <= last; idx++) {
      Block& block = ctx->program->blocks[idx];
      if (block.loop_nest_depth != loop_nest_depth) {
         vals[idx - first] = vals[idx - 1 - first];
         continue;
      }

      if ((block.kind & block_kind_continue) && block.index != last) {
         vals[idx - first] = header_phi->operands[next_pred];
         next_pred++;
         continue;
      }

      bool all_same = true;
      for (unsigned i = 1; all_same && (i < block.linear_preds.size()); i++)
         all_same = vals[block.linear_preds[i] - first] == vals[block.linear_preds[0] - first];

      Operand val;
      if (all_same) {
         val = vals[block.linear_preds[0] - first];
      } else {
         aco_ptr<Instruction> phi(create_instruction<Pseudo_instruction>(
            aco_opcode::p_linear_phi, Format::PSEUDO, block.linear_preds.size(), 1));
         for (unsigned i = 0; i < block.linear_preds.size(); i++)
            phi->operands[i] = vals[block.linear_preds[i] - first];
         val = Operand(ctx->program->allocateTmp(rc));
         phi->definitions[0] = Definition(val.getTemp());
         block.instructions.emplace(block.instructions.begin(), std::move(phi));
      }
      vals[idx - first] = val;
   }

   return vals[last - first];
}

static void begin_uniform_if_then(isel_context* ctx, if_context* ic, Temp cond);
static void begin_uniform_if_else(isel_context* ctx, if_context* ic);
static void end_uniform_if(isel_context* ctx, if_context* ic);

static void
visit_loop(isel_context* ctx, nir_loop* loop)
{
   assert(!nir_loop_has_continue_construct(loop));
   loop_context lc;
   begin_loop(ctx, &lc);

   bool unreachable = visit_cf_list(ctx, &loop->body);

   unsigned loop_header_idx = ctx->cf_info.parent_loop.header_idx;

   /* Fixup phis in loop header from unreachable blocks.
    * has_branch/has_divergent_branch also indicates if the loop ends with a
    * break/continue instruction, but we don't emit those if unreachable=true */
   if (unreachable) {
      assert(ctx->cf_info.has_branch || ctx->cf_info.parent_loop.has_divergent_branch);
      bool linear = ctx->cf_info.has_branch;
      bool logical = ctx->cf_info.has_branch || ctx->cf_info.parent_loop.has_divergent_branch;
      for (aco_ptr<Instruction>& instr : ctx->program->blocks[loop_header_idx].instructions) {
         if ((logical && instr->opcode == aco_opcode::p_phi) ||
             (linear && instr->opcode == aco_opcode::p_linear_phi)) {
            /* the last operand should be the one that needs to be removed */
            instr->operands.pop_back();
         } else if (!is_phi(instr)) {
            break;
         }
      }
   }

   /* Fixup linear phis in loop header from expecting a continue. Both this fixup
    * and the previous one shouldn't both happen at once because a break in the
    * merge block would get CSE'd */
   if (nir_loop_last_block(loop)->successors[0] != nir_loop_first_block(loop)) {
      unsigned num_vals = ctx->cf_info.has_branch ? 1 : (ctx->block->index - loop_header_idx + 1);
      Operand* const vals = (Operand*)alloca(num_vals * sizeof(Operand));
      for (aco_ptr<Instruction>& instr : ctx->program->blocks[loop_header_idx].instructions) {
         if (instr->opcode == aco_opcode::p_linear_phi) {
            if (ctx->cf_info.has_branch)
               instr->operands.pop_back();
            else
               instr->operands.back() =
                  create_continue_phis(ctx, loop_header_idx, ctx->block->index, instr, vals);
         } else if (!is_phi(instr)) {
            break;
         }
      }
   }

   /* NIR seems to allow this, and even though the loop exit has no predecessors, SSA defs from the
    * loop header are live. Handle this without complicating the ACO IR by creating a dummy break.
    */
   if (nir_cf_node_cf_tree_next(&loop->cf_node)->predecessors->entries == 0) {
      Builder bld(ctx->program, ctx->block);
      Temp cond = bld.copy(bld.def(s1, scc), Operand::zero());
      if_context ic;
      begin_uniform_if_then(ctx, &ic, cond);
      emit_loop_break(ctx);
      begin_uniform_if_else(ctx, &ic);
      end_uniform_if(ctx, &ic);
   }

   end_loop(ctx, &lc);
}

static void
begin_divergent_if_then(isel_context* ctx, if_context* ic, Temp cond,
                        nir_selection_control sel_ctrl = nir_selection_control_none)
{
   ic->cond = cond;

   append_logical_end(ctx->block);
   ctx->block->kind |= block_kind_branch;

   /* branch to linear then block */
   assert(cond.regClass() == ctx->program->lane_mask);
   aco_ptr<Pseudo_branch_instruction> branch;
   branch.reset(create_instruction<Pseudo_branch_instruction>(aco_opcode::p_cbranch_z,
                                                              Format::PSEUDO_BRANCH, 1, 1));
   branch->definitions[0] = Definition(ctx->program->allocateTmp(s2));
   branch->operands[0] = Operand(cond);
   branch->selection_control_remove = sel_ctrl == nir_selection_control_flatten ||
                                      sel_ctrl == nir_selection_control_divergent_always_taken;
   ctx->block->instructions.push_back(std::move(branch));

   ic->BB_if_idx = ctx->block->index;
   ic->BB_invert = Block();
   /* Invert blocks are intentionally not marked as top level because they
    * are not part of the logical cfg. */
   ic->BB_invert.kind |= block_kind_invert;
   ic->BB_endif = Block();
   ic->BB_endif.kind |= (block_kind_merge | (ctx->block->kind & block_kind_top_level));

   ic->exec_potentially_empty_discard_old = ctx->cf_info.exec_potentially_empty_discard;
   ic->exec_potentially_empty_break_old = ctx->cf_info.exec_potentially_empty_break;
   ic->exec_potentially_empty_break_depth_old = ctx->cf_info.exec_potentially_empty_break_depth;
   ic->divergent_old = ctx->cf_info.parent_if.is_divergent;
   ic->had_divergent_discard_old = ctx->cf_info.had_divergent_discard;
   ctx->cf_info.parent_if.is_divergent = true;

   /* divergent branches use cbranch_execz */
   ctx->cf_info.exec_potentially_empty_discard = false;
   ctx->cf_info.exec_potentially_empty_break = false;
   ctx->cf_info.exec_potentially_empty_break_depth = UINT16_MAX;

   /** emit logical then block */
   ctx->program->next_divergent_if_logical_depth++;
   Block* BB_then_logical = ctx->program->create_and_insert_block();
   add_edge(ic->BB_if_idx, BB_then_logical);
   ctx->block = BB_then_logical;
   append_logical_start(BB_then_logical);
}

static void
begin_divergent_if_else(isel_context* ctx, if_context* ic,
                        nir_selection_control sel_ctrl = nir_selection_control_none)
{
   Block* BB_then_logical = ctx->block;
   append_logical_end(BB_then_logical);
   /* branch from logical then block to invert block */
   aco_ptr<Pseudo_branch_instruction> branch;
   branch.reset(create_instruction<Pseudo_branch_instruction>(aco_opcode::p_branch,
                                                              Format::PSEUDO_BRANCH, 0, 1));
   branch->definitions[0] = Definition(ctx->program->allocateTmp(s2));
   BB_then_logical->instructions.emplace_back(std::move(branch));
   add_linear_edge(BB_then_logical->index, &ic->BB_invert);
   if (!ctx->cf_info.parent_loop.has_divergent_branch)
      add_logical_edge(BB_then_logical->index, &ic->BB_endif);
   BB_then_logical->kind |= block_kind_uniform;
   assert(!ctx->cf_info.has_branch);
   ic->then_branch_divergent = ctx->cf_info.parent_loop.has_divergent_branch;
   ctx->cf_info.parent_loop.has_divergent_branch = false;
   ctx->program->next_divergent_if_logical_depth--;

   /** emit linear then block */
   Block* BB_then_linear = ctx->program->create_and_insert_block();
   BB_then_linear->kind |= block_kind_uniform;
   add_linear_edge(ic->BB_if_idx, BB_then_linear);
   /* branch from linear then block to invert block */
   branch.reset(create_instruction<Pseudo_branch_instruction>(aco_opcode::p_branch,
                                                              Format::PSEUDO_BRANCH, 0, 1));
   branch->definitions[0] = Definition(ctx->program->allocateTmp(s2));
   BB_then_linear->instructions.emplace_back(std::move(branch));
   add_linear_edge(BB_then_linear->index, &ic->BB_invert);

   /** emit invert merge block */
   ctx->block = ctx->program->insert_block(std::move(ic->BB_invert));
   ic->invert_idx = ctx->block->index;

   /* branch to linear else block (skip else) */
   branch.reset(create_instruction<Pseudo_branch_instruction>(aco_opcode::p_branch,
                                                              Format::PSEUDO_BRANCH, 0, 1));
   branch->definitions[0] = Definition(ctx->program->allocateTmp(s2));
   branch->selection_control_remove = sel_ctrl == nir_selection_control_flatten ||
                                      sel_ctrl == nir_selection_control_divergent_always_taken;
   ctx->block->instructions.push_back(std::move(branch));

   ic->exec_potentially_empty_discard_old |= ctx->cf_info.exec_potentially_empty_discard;
   ic->exec_potentially_empty_break_old |= ctx->cf_info.exec_potentially_empty_break;
   ic->exec_potentially_empty_break_depth_old = std::min(
      ic->exec_potentially_empty_break_depth_old, ctx->cf_info.exec_potentially_empty_break_depth);
   /* divergent branches use cbranch_execz */
   ctx->cf_info.exec_potentially_empty_discard = false;
   ctx->cf_info.exec_potentially_empty_break = false;
   ctx->cf_info.exec_potentially_empty_break_depth = UINT16_MAX;

   ic->had_divergent_discard_then = ctx->cf_info.had_divergent_discard;
   ctx->cf_info.had_divergent_discard = ic->had_divergent_discard_old;

   /** emit logical else block */
   ctx->program->next_divergent_if_logical_depth++;
   Block* BB_else_logical = ctx->program->create_and_insert_block();
   add_logical_edge(ic->BB_if_idx, BB_else_logical);
   add_linear_edge(ic->invert_idx, BB_else_logical);
   ctx->block = BB_else_logical;
   append_logical_start(BB_else_logical);
}

static void
end_divergent_if(isel_context* ctx, if_context* ic)
{
   Block* BB_else_logical = ctx->block;
   append_logical_end(BB_else_logical);

   /* branch from logical else block to endif block */
   aco_ptr<Pseudo_branch_instruction> branch;
   branch.reset(create_instruction<Pseudo_branch_instruction>(aco_opcode::p_branch,
                                                              Format::PSEUDO_BRANCH, 0, 1));
   branch->definitions[0] = Definition(ctx->program->allocateTmp(s2));
   BB_else_logical->instructions.emplace_back(std::move(branch));
   add_linear_edge(BB_else_logical->index, &ic->BB_endif);
   if (!ctx->cf_info.parent_loop.has_divergent_branch)
      add_logical_edge(BB_else_logical->index, &ic->BB_endif);
   BB_else_logical->kind |= block_kind_uniform;
   ctx->program->next_divergent_if_logical_depth--;

   assert(!ctx->cf_info.has_branch);
   ctx->cf_info.parent_loop.has_divergent_branch &= ic->then_branch_divergent;

   /** emit linear else block */
   Block* BB_else_linear = ctx->program->create_and_insert_block();
   BB_else_linear->kind |= block_kind_uniform;
   add_linear_edge(ic->invert_idx, BB_else_linear);

   /* branch from linear else block to endif block */
   branch.reset(create_instruction<Pseudo_branch_instruction>(aco_opcode::p_branch,
                                                              Format::PSEUDO_BRANCH, 0, 1));
   branch->definitions[0] = Definition(ctx->program->allocateTmp(s2));
   BB_else_linear->instructions.emplace_back(std::move(branch));
   add_linear_edge(BB_else_linear->index, &ic->BB_endif);

   /** emit endif merge block */
   ctx->block = ctx->program->insert_block(std::move(ic->BB_endif));
   append_logical_start(ctx->block);

   ctx->cf_info.parent_if.is_divergent = ic->divergent_old;
   ctx->cf_info.exec_potentially_empty_discard |= ic->exec_potentially_empty_discard_old;
   ctx->cf_info.exec_potentially_empty_break |= ic->exec_potentially_empty_break_old;
   ctx->cf_info.exec_potentially_empty_break_depth = std::min(
      ic->exec_potentially_empty_break_depth_old, ctx->cf_info.exec_potentially_empty_break_depth);
   if (ctx->block->loop_nest_depth == ctx->cf_info.exec_potentially_empty_break_depth &&
       !ctx->cf_info.parent_if.is_divergent) {
      ctx->cf_info.exec_potentially_empty_break = false;
      ctx->cf_info.exec_potentially_empty_break_depth = UINT16_MAX;
   }
   /* uniform control flow never has an empty exec-mask */
   if (!ctx->block->loop_nest_depth && !ctx->cf_info.parent_if.is_divergent) {
      ctx->cf_info.exec_potentially_empty_discard = false;
      ctx->cf_info.exec_potentially_empty_break = false;
      ctx->cf_info.exec_potentially_empty_break_depth = UINT16_MAX;
   }
   ctx->cf_info.had_divergent_discard |= ic->had_divergent_discard_then;
}

static void
begin_uniform_if_then(isel_context* ctx, if_context* ic, Temp cond)
{
   assert(cond.regClass() == s1);

   append_logical_end(ctx->block);
   ctx->block->kind |= block_kind_uniform;

   aco_ptr<Pseudo_branch_instruction> branch;
   aco_opcode branch_opcode = aco_opcode::p_cbranch_z;
   branch.reset(
      create_instruction<Pseudo_branch_instruction>(branch_opcode, Format::PSEUDO_BRANCH, 1, 1));
   branch->definitions[0] = Definition(ctx->program->allocateTmp(s2));
   branch->operands[0] = Operand(cond);
   branch->operands[0].setFixed(scc);
   ctx->block->instructions.emplace_back(std::move(branch));

   ic->BB_if_idx = ctx->block->index;
   ic->BB_endif = Block();
   ic->BB_endif.kind |= ctx->block->kind & block_kind_top_level;

   ctx->cf_info.has_branch = false;
   ctx->cf_info.parent_loop.has_divergent_branch = false;

   ic->had_divergent_discard_old = ctx->cf_info.had_divergent_discard;

   /** emit then block */
   ctx->program->next_uniform_if_depth++;
   Block* BB_then = ctx->program->create_and_insert_block();
   add_edge(ic->BB_if_idx, BB_then);
   append_logical_start(BB_then);
   ctx->block = BB_then;
}

static void
begin_uniform_if_else(isel_context* ctx, if_context* ic)
{
   Block* BB_then = ctx->block;

   ic->uniform_has_then_branch = ctx->cf_info.has_branch;
   ic->then_branch_divergent = ctx->cf_info.parent_loop.has_divergent_branch;

   if (!ic->uniform_has_then_branch) {
      append_logical_end(BB_then);
      /* branch from then block to endif block */
      aco_ptr<Pseudo_branch_instruction> branch;
      branch.reset(create_instruction<Pseudo_branch_instruction>(aco_opcode::p_branch,
                                                                 Format::PSEUDO_BRANCH, 0, 1));
      branch->definitions[0] = Definition(ctx->program->allocateTmp(s2));
      BB_then->instructions.emplace_back(std::move(branch));
      add_linear_edge(BB_then->index, &ic->BB_endif);
      if (!ic->then_branch_divergent)
         add_logical_edge(BB_then->index, &ic->BB_endif);
      BB_then->kind |= block_kind_uniform;
   }

   ctx->cf_info.has_branch = false;
   ctx->cf_info.parent_loop.has_divergent_branch = false;

   ic->had_divergent_discard_then = ctx->cf_info.had_divergent_discard;
   ctx->cf_info.had_divergent_discard = ic->had_divergent_discard_old;

   /** emit else block */
   Block* BB_else = ctx->program->create_and_insert_block();
   add_edge(ic->BB_if_idx, BB_else);
   append_logical_start(BB_else);
   ctx->block = BB_else;
}

static void
end_uniform_if(isel_context* ctx, if_context* ic)
{
   Block* BB_else = ctx->block;

   if (!ctx->cf_info.has_branch) {
      append_logical_end(BB_else);
      /* branch from then block to endif block */
      aco_ptr<Pseudo_branch_instruction> branch;
      branch.reset(create_instruction<Pseudo_branch_instruction>(aco_opcode::p_branch,
                                                                 Format::PSEUDO_BRANCH, 0, 1));
      branch->definitions[0] = Definition(ctx->program->allocateTmp(s2));
      BB_else->instructions.emplace_back(std::move(branch));
      add_linear_edge(BB_else->index, &ic->BB_endif);
      if (!ctx->cf_info.parent_loop.has_divergent_branch)
         add_logical_edge(BB_else->index, &ic->BB_endif);
      BB_else->kind |= block_kind_uniform;
   }

   ctx->cf_info.has_branch &= ic->uniform_has_then_branch;
   ctx->cf_info.parent_loop.has_divergent_branch &= ic->then_branch_divergent;
   ctx->cf_info.had_divergent_discard |= ic->had_divergent_discard_then;

   /** emit endif merge block */
   ctx->program->next_uniform_if_depth--;
   if (!ctx->cf_info.has_branch) {
      ctx->block = ctx->program->insert_block(std::move(ic->BB_endif));
      append_logical_start(ctx->block);
   }
}

static bool
visit_if(isel_context* ctx, nir_if* if_stmt)
{
   Temp cond = get_ssa_temp(ctx, if_stmt->condition.ssa);
   Builder bld(ctx->program, ctx->block);
   aco_ptr<Pseudo_branch_instruction> branch;
   if_context ic;

   if (!nir_src_is_divergent(if_stmt->condition)) { /* uniform condition */
      /**
       * Uniform conditionals are represented in the following way*) :
       *
       * The linear and logical CFG:
       *                        BB_IF
       *                        /    \
       *       BB_THEN (logical)      BB_ELSE (logical)
       *                        \    /
       *                        BB_ENDIF
       *
       * *) Exceptions may be due to break and continue statements within loops
       *    If a break/continue happens within uniform control flow, it branches
       *    to the loop exit/entry block. Otherwise, it branches to the next
       *    merge block.
       **/

      assert(cond.regClass() == ctx->program->lane_mask);
      cond = bool_to_scalar_condition(ctx, cond);

      begin_uniform_if_then(ctx, &ic, cond);
      visit_cf_list(ctx, &if_stmt->then_list);

      begin_uniform_if_else(ctx, &ic);
      visit_cf_list(ctx, &if_stmt->else_list);

      end_uniform_if(ctx, &ic);
   } else { /* non-uniform condition */
      /**
       * To maintain a logical and linear CFG without critical edges,
       * non-uniform conditionals are represented in the following way*) :
       *
       * The linear CFG:
       *                        BB_IF
       *                        /    \
       *       BB_THEN (logical)      BB_THEN (linear)
       *                        \    /
       *                        BB_INVERT (linear)
       *                        /    \
       *       BB_ELSE (logical)      BB_ELSE (linear)
       *                        \    /
       *                        BB_ENDIF
       *
       * The logical CFG:
       *                        BB_IF
       *                        /    \
       *       BB_THEN (logical)      BB_ELSE (logical)
       *                        \    /
       *                        BB_ENDIF
       *
       * *) Exceptions may be due to break and continue statements within loops
       **/

      begin_divergent_if_then(ctx, &ic, cond, if_stmt->control);
      visit_cf_list(ctx, &if_stmt->then_list);

      begin_divergent_if_else(ctx, &ic, if_stmt->control);
      visit_cf_list(ctx, &if_stmt->else_list);

      end_divergent_if(ctx, &ic);
   }

   return !ctx->cf_info.has_branch && !ctx->block->logical_preds.empty();
}

static bool
visit_cf_list(isel_context* ctx, struct exec_list* list)
{
   foreach_list_typed (nir_cf_node, node, node, list) {
      switch (node->type) {
      case nir_cf_node_block: visit_block(ctx, nir_cf_node_as_block(node)); break;
      case nir_cf_node_if:
         if (!visit_if(ctx, nir_cf_node_as_if(node)))
            return true;
         break;
      case nir_cf_node_loop: visit_loop(ctx, nir_cf_node_as_loop(node)); break;
      default: unreachable("unimplemented cf list type");
      }
   }
   return false;
}

static void
export_mrt(isel_context* ctx, const struct aco_export_mrt* mrt)
{
   Builder bld(ctx->program, ctx->block);

   bld.exp(aco_opcode::exp, mrt->out[0], mrt->out[1], mrt->out[2], mrt->out[3],
           mrt->enabled_channels, mrt->target, mrt->compr);

   ctx->program->has_color_exports = true;
}

static bool
export_fs_mrt_color(isel_context* ctx, const struct aco_ps_epilog_info* info, Temp colors[4],
                    unsigned slot, struct aco_export_mrt* mrt)
{
   unsigned col_format = (info->spi_shader_col_format >> (slot * 4)) & 0xf;

   if (col_format == V_028714_SPI_SHADER_ZERO)
      return false;

   Builder bld(ctx->program, ctx->block);
   Operand values[4];

   for (unsigned i = 0; i < 4; ++i) {
      values[i] = Operand(colors[i]);
   }

   unsigned enabled_channels = 0;
   aco_opcode compr_op = aco_opcode::num_opcodes;
   bool compr = false;
   bool is_16bit = colors[0].regClass() == v2b;
   bool is_int8 = (info->color_is_int8 >> slot) & 1;
   bool is_int10 = (info->color_is_int10 >> slot) & 1;
   bool enable_mrt_output_nan_fixup = (ctx->options->enable_mrt_output_nan_fixup >> slot) & 1;

   /* Replace NaN by zero (only 32-bit) to fix game bugs if requested. */
   if (enable_mrt_output_nan_fixup && !is_16bit &&
       (col_format == V_028714_SPI_SHADER_32_R || col_format == V_028714_SPI_SHADER_32_GR ||
        col_format == V_028714_SPI_SHADER_32_AR || col_format == V_028714_SPI_SHADER_32_ABGR ||
        col_format == V_028714_SPI_SHADER_FP16_ABGR)) {
      for (unsigned i = 0; i < 4; i++) {
         Temp is_not_nan =
            bld.vopc(aco_opcode::v_cmp_eq_f32, bld.def(bld.lm), values[i], values[i]);
         values[i] = bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), Operand::zero(), values[i],
                              is_not_nan);
      }
   }

   switch (col_format) {
   case V_028714_SPI_SHADER_32_R: enabled_channels = 1; break;

   case V_028714_SPI_SHADER_32_GR: enabled_channels = 0x3; break;

   case V_028714_SPI_SHADER_32_AR:
      if (ctx->options->gfx_level >= GFX10) {
         /* Special case: on GFX10, the outputs are different for 32_AR */
         enabled_channels = 0x3;
         values[1] = values[3];
         values[3] = Operand(v1);
      } else {
         enabled_channels = 0x9;
      }
      break;

   case V_028714_SPI_SHADER_FP16_ABGR:
      for (int i = 0; i < 2; i++) {
         if (is_16bit) {
            values[i] = bld.pseudo(aco_opcode::p_create_vector, bld.def(v1), values[i * 2],
                                   values[i * 2 + 1]);
         } else if (ctx->options->gfx_level == GFX8 || ctx->options->gfx_level == GFX9) {
            values[i] = bld.vop3(aco_opcode::v_cvt_pkrtz_f16_f32_e64, bld.def(v1), values[i * 2],
                                 values[i * 2 + 1]);
         } else {
            values[i] = bld.vop2(aco_opcode::v_cvt_pkrtz_f16_f32, bld.def(v1), values[i * 2],
                                 values[i * 2 + 1]);
         }
      }
      values[2] = Operand(v1);
      values[3] = Operand(v1);
      enabled_channels = 0xf;
      compr = true;
      break;

   case V_028714_SPI_SHADER_UNORM16_ABGR:
      if (is_16bit && ctx->options->gfx_level >= GFX9) {
         compr_op = aco_opcode::v_cvt_pknorm_u16_f16;
      } else {
         compr_op = aco_opcode::v_cvt_pknorm_u16_f32;
      }
      break;

   case V_028714_SPI_SHADER_SNORM16_ABGR:
      if (is_16bit && ctx->options->gfx_level >= GFX9) {
         compr_op = aco_opcode::v_cvt_pknorm_i16_f16;
      } else {
         compr_op = aco_opcode::v_cvt_pknorm_i16_f32;
      }
      break;

   case V_028714_SPI_SHADER_UINT16_ABGR:
      compr_op = aco_opcode::v_cvt_pk_u16_u32;
      if (is_int8 || is_int10) {
         /* clamp */
         uint32_t max_rgb = is_int8 ? 255 : is_int10 ? 1023 : 0;

         for (unsigned i = 0; i < 4; i++) {
            uint32_t max = i == 3 && is_int10 ? 3 : max_rgb;

            values[i] = bld.vop2(aco_opcode::v_min_u32, bld.def(v1), Operand::c32(max), values[i]);
         }
      } else if (is_16bit) {
         for (unsigned i = 0; i < 4; i++) {
            Temp tmp = convert_int(ctx, bld, values[i].getTemp(), 16, 32, false);
            values[i] = Operand(tmp);
         }
      }
      break;

   case V_028714_SPI_SHADER_SINT16_ABGR:
      compr_op = aco_opcode::v_cvt_pk_i16_i32;
      if (is_int8 || is_int10) {
         /* clamp */
         uint32_t max_rgb = is_int8 ? 127 : is_int10 ? 511 : 0;
         uint32_t min_rgb = is_int8 ? -128 : is_int10 ? -512 : 0;

         for (unsigned i = 0; i < 4; i++) {
            uint32_t max = i == 3 && is_int10 ? 1 : max_rgb;
            uint32_t min = i == 3 && is_int10 ? -2u : min_rgb;

            values[i] = bld.vop2(aco_opcode::v_min_i32, bld.def(v1), Operand::c32(max), values[i]);
            values[i] = bld.vop2(aco_opcode::v_max_i32, bld.def(v1), Operand::c32(min), values[i]);
         }
      } else if (is_16bit) {
         for (unsigned i = 0; i < 4; i++) {
            Temp tmp = convert_int(ctx, bld, values[i].getTemp(), 16, 32, true);
            values[i] = Operand(tmp);
         }
      }
      break;

   case V_028714_SPI_SHADER_32_ABGR: enabled_channels = 0xF; break;

   case V_028714_SPI_SHADER_ZERO:
   default: return false;
   }

   if (compr_op != aco_opcode::num_opcodes) {
      values[0] = bld.vop3(compr_op, bld.def(v1), values[0], values[1]);
      values[1] = bld.vop3(compr_op, bld.def(v1), values[2], values[3]);
      values[2] = Operand(v1);
      values[3] = Operand(v1);
      enabled_channels = 0xf;
      compr = true;
   } else if (!compr) {
      for (int i = 0; i < 4; i++)
         values[i] = enabled_channels & (1 << i) ? values[i] : Operand(v1);
   }

   if (ctx->program->gfx_level >= GFX11) {
      /* GFX11 doesn't use COMPR for exports, but the channel mask should be
       * 0x3 instead.
       */
      enabled_channels = compr ? 0x3 : enabled_channels;
      compr = false;
   }

   for (unsigned i = 0; i < 4; i++)
      mrt->out[i] = values[i];
   mrt->target = V_008DFC_SQ_EXP_MRT;
   mrt->enabled_channels = enabled_channels;
   mrt->compr = compr;

   return true;
}

static void
export_fs_mrtz(isel_context* ctx, Temp depth, Temp stencil, Temp samplemask, Temp alpha)
{
   Builder bld(ctx->program, ctx->block);
   unsigned enabled_channels = 0;
   bool compr = false;
   Operand values[4];

   for (unsigned i = 0; i < 4; ++i) {
      values[i] = Operand(v1);
   }

   /* Both stencil and sample mask only need 16-bits. */
   if (!depth.id() && !alpha.id() && (stencil.id() || samplemask.id())) {
      compr = ctx->program->gfx_level < GFX11; /* COMPR flag */

      if (stencil.id()) {
         /* Stencil should be in X[23:16]. */
         values[0] = bld.vop2(aco_opcode::v_lshlrev_b32, bld.def(v1), Operand::c32(16u), stencil);
         enabled_channels |= ctx->program->gfx_level >= GFX11 ? 0x1 : 0x3;
      }

      if (samplemask.id()) {
         /* SampleMask should be in Y[15:0]. */
         values[1] = Operand(samplemask);
         enabled_channels |= ctx->program->gfx_level >= GFX11 ? 0x2 : 0xc;
      }
   } else {
      if (depth.id()) {
         values[0] = Operand(depth);
         enabled_channels |= 0x1;
      }

      if (stencil.id()) {
         values[1] = Operand(stencil);
         enabled_channels |= 0x2;
      }

      if (samplemask.id()) {
         values[2] = Operand(samplemask);
         enabled_channels |= 0x4;
      }

      if (alpha.id()) {
         assert(ctx->program->gfx_level >= GFX11);
         values[3] = Operand(alpha);
         enabled_channels |= 0x8;
      }
   }

   /* GFX6 (except OLAND and HAINAN) has a bug that it only looks at the X
    * writemask component.
    */
   if (ctx->options->gfx_level == GFX6 && ctx->options->family != CHIP_OLAND &&
       ctx->options->family != CHIP_HAINAN) {
      enabled_channels |= 0x1;
   }

   bld.exp(aco_opcode::exp, values[0], values[1], values[2], values[3], enabled_channels,
           V_008DFC_SQ_EXP_MRTZ, compr);
}

static void
create_fs_null_export(isel_context* ctx)
{
   /* FS must always have exports.
    * So when there are none, we need to add a null export.
    */

   Builder bld(ctx->program, ctx->block);
   /* GFX11 doesn't support NULL exports, and MRT0 should be exported instead. */
   unsigned dest = ctx->options->gfx_level >= GFX11 ? V_008DFC_SQ_EXP_MRT : V_008DFC_SQ_EXP_NULL;
   bld.exp(aco_opcode::exp, Operand(v1), Operand(v1), Operand(v1), Operand(v1),
           /* enabled_mask */ 0, dest, /* compr */ false, /* done */ true, /* vm */ true);

   ctx->program->has_color_exports = true;
}

static void
create_fs_jump_to_epilog(isel_context* ctx)
{
   Builder bld(ctx->program, ctx->block);
   std::vector<Operand> exports;
   unsigned vgpr = 256; /* VGPR 0 */

   if (ctx->outputs.mask[FRAG_RESULT_DEPTH])
      exports.emplace_back(Operand(ctx->outputs.temps[FRAG_RESULT_DEPTH * 4u], PhysReg{vgpr++}));

   if (ctx->outputs.mask[FRAG_RESULT_STENCIL])
      exports.emplace_back(Operand(ctx->outputs.temps[FRAG_RESULT_STENCIL * 4u], PhysReg{vgpr++}));

   if (ctx->outputs.mask[FRAG_RESULT_SAMPLE_MASK])
      exports.emplace_back(
         Operand(ctx->outputs.temps[FRAG_RESULT_SAMPLE_MASK * 4u], PhysReg{vgpr++}));

   PhysReg exports_start(vgpr);

   for (unsigned slot = FRAG_RESULT_DATA0; slot < FRAG_RESULT_DATA7 + 1; ++slot) {
      unsigned color_index = slot - FRAG_RESULT_DATA0;
      unsigned color_type = (ctx->output_color_types >> (color_index * 2)) & 0x3;
      unsigned write_mask = ctx->outputs.mask[slot];

      if (!write_mask)
         continue;

      PhysReg color_start(exports_start.reg() + color_index * 4);

      for (unsigned i = 0; i < 4; i++) {
         if (!(write_mask & BITFIELD_BIT(i))) {
            exports.emplace_back(Operand(v1));
            continue;
         }

         PhysReg chan_reg = color_start.advance(i * 4u);
         Operand chan(ctx->outputs.temps[slot * 4u + i]);

         if (color_type == ACO_TYPE_FLOAT16) {
            chan = bld.vop1(aco_opcode::v_cvt_f32_f16, bld.def(v1), chan);
         } else if (color_type == ACO_TYPE_INT16 || color_type == ACO_TYPE_UINT16) {
            bool sign_ext = color_type == ACO_TYPE_INT16;
            Temp tmp = convert_int(ctx, bld, chan.getTemp(), 16, 32, sign_ext);
            chan = Operand(tmp);
         }

         chan.setFixed(chan_reg);
         exports.emplace_back(chan);
      }
   }

   Temp continue_pc = convert_pointer_to_64_bit(ctx, get_arg(ctx, ctx->program->info.ps.epilog_pc));

   aco_ptr<Pseudo_instruction> jump{create_instruction<Pseudo_instruction>(
      aco_opcode::p_jump_to_epilog, Format::PSEUDO, 1 + exports.size(), 0)};
   jump->operands[0] = Operand(continue_pc);
   for (unsigned i = 0; i < exports.size(); i++) {
      jump->operands[i + 1] = exports[i];
   }
   ctx->block->instructions.emplace_back(std::move(jump));
}

PhysReg
get_arg_reg(const struct ac_shader_args* args, struct ac_arg arg)
{
   assert(arg.used);
   enum ac_arg_regfile file = args->args[arg.arg_index].file;
   unsigned reg = args->args[arg.arg_index].offset;
   return PhysReg(file == AC_ARG_SGPR ? reg : reg + 256);
}

static Operand
get_arg_for_end(isel_context* ctx, struct ac_arg arg)
{
   return Operand(get_arg(ctx, arg), get_arg_reg(ctx->args, arg));
}

static Temp
get_tcs_out_current_patch_data_offset(isel_context* ctx)
{
   Builder bld(ctx->program, ctx->block);

   const unsigned output_vertex_size = ctx->program->info.tcs.num_linked_outputs * 4u;
   const unsigned pervertex_output_patch_size =
      ctx->program->info.tcs.tcs_vertices_out * output_vertex_size;
   const unsigned output_patch_stride =
      pervertex_output_patch_size + ctx->program->info.tcs.num_linked_patch_outputs * 4u;

   Temp tcs_rel_ids = get_arg(ctx, ctx->args->tcs_rel_ids);
   Temp rel_patch_id =
      bld.vop3(aco_opcode::v_bfe_u32, bld.def(v1), tcs_rel_ids, Operand::c32(0u), Operand::c32(8u));
   Temp patch_offset = bld.v_mul_imm(bld.def(v1), rel_patch_id, output_patch_stride, false);

   Temp tcs_offchip_layout = get_arg(ctx, ctx->program->info.tcs.tcs_offchip_layout);

   Temp patch_control_points = bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc),
                                        tcs_offchip_layout, Operand::c32(0x3f));

   Temp num_patches = bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc),
                               tcs_offchip_layout, Operand::c32(0x60006));

   Temp lshs_vertex_stride = bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc),
                                      tcs_offchip_layout, Operand::c32(0x8000c));

   Temp input_patch_size =
      bld.sop2(aco_opcode::s_mul_i32, bld.def(s1), patch_control_points, lshs_vertex_stride);

   Temp output_patch0_offset =
      bld.sop2(aco_opcode::s_mul_i32, bld.def(s1), num_patches, input_patch_size);

   Temp output_patch_offset =
      bld.nuw().sop2(aco_opcode::s_add_i32, bld.def(s1), bld.def(s1, scc),
                     Operand::c32(pervertex_output_patch_size), output_patch0_offset);

   return bld.nuw().vadd32(bld.def(v1), patch_offset, output_patch_offset);
}

static Temp
get_patch_base(isel_context* ctx)
{
   Builder bld(ctx->program, ctx->block);

   const unsigned output_vertex_size = ctx->program->info.tcs.num_linked_outputs * 16u;
   const unsigned pervertex_output_patch_size =
      ctx->program->info.tcs.tcs_vertices_out * output_vertex_size;

   Temp num_patches =
      bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc),
               get_arg(ctx, ctx->program->info.tcs.tcs_offchip_layout), Operand::c32(0x60006));

   return bld.sop2(aco_opcode::s_mul_i32, bld.def(s1), num_patches,
                   Operand::c32(pervertex_output_patch_size));
}

static void
passthrough_all_args(isel_context* ctx, std::vector<Operand>& regs)
{
   struct ac_arg arg;
   arg.used = true;

   for (arg.arg_index = 0; arg.arg_index < ctx->args->arg_count; arg.arg_index++)
      regs.emplace_back(get_arg_for_end(ctx, arg));
}

static void
build_end_with_regs(isel_context* ctx, std::vector<Operand>& regs)
{
   aco_ptr<Pseudo_instruction> end{create_instruction<Pseudo_instruction>(
      aco_opcode::p_end_with_regs, Format::PSEUDO, regs.size(), 0)};

   for (unsigned i = 0; i < regs.size(); i++)
      end->operands[i] = regs[i];

   ctx->block->instructions.emplace_back(std::move(end));

   ctx->block->kind |= block_kind_end_with_regs;
}

static void
create_tcs_jump_to_epilog(isel_context* ctx)
{
   Builder bld(ctx->program, ctx->block);

   PhysReg vgpr_start(256); /* VGPR 0 */
   PhysReg sgpr_start(0);   /* SGPR 0 */

   /* SGPRs */
   Operand ring_offsets = Operand(get_arg(ctx, ctx->args->ring_offsets));
   ring_offsets.setFixed(sgpr_start);

   Operand tess_offchip_offset = Operand(get_arg(ctx, ctx->args->tess_offchip_offset));
   tess_offchip_offset.setFixed(sgpr_start.advance(8u));

   Operand tcs_factor_offset = Operand(get_arg(ctx, ctx->args->tcs_factor_offset));
   tcs_factor_offset.setFixed(sgpr_start.advance(12u));

   Operand tcs_offchip_layout = Operand(get_arg(ctx, ctx->program->info.tcs.tcs_offchip_layout));
   tcs_offchip_layout.setFixed(sgpr_start.advance(16u));

   Operand patch_base = Operand(get_patch_base(ctx));
   patch_base.setFixed(sgpr_start.advance(20u));

   /* VGPRs */
   Operand tcs_out_current_patch_data_offset = Operand(get_tcs_out_current_patch_data_offset(ctx));
   tcs_out_current_patch_data_offset.setFixed(vgpr_start);

   Operand invocation_id =
      bld.vop3(aco_opcode::v_bfe_u32, bld.def(v1), get_arg(ctx, ctx->args->tcs_rel_ids),
               Operand::c32(8u), Operand::c32(5u));
   invocation_id.setFixed(vgpr_start.advance(4u));

   Operand rel_patch_id =
      bld.pseudo(aco_opcode::p_extract, bld.def(v1), get_arg(ctx, ctx->args->tcs_rel_ids),
                 Operand::c32(0u), Operand::c32(8u), Operand::c32(0u));
   rel_patch_id.setFixed(vgpr_start.advance(8u));

   Temp continue_pc =
      convert_pointer_to_64_bit(ctx, get_arg(ctx, ctx->program->info.tcs.epilog_pc));

   aco_ptr<Pseudo_instruction> jump{
      create_instruction<Pseudo_instruction>(aco_opcode::p_jump_to_epilog, Format::PSEUDO, 9, 0)};
   jump->operands[0] = Operand(continue_pc);
   jump->operands[1] = ring_offsets;
   jump->operands[2] = tess_offchip_offset;
   jump->operands[3] = tcs_factor_offset;
   jump->operands[4] = tcs_offchip_layout;
   jump->operands[5] = patch_base;
   jump->operands[6] = tcs_out_current_patch_data_offset;
   jump->operands[7] = invocation_id;
   jump->operands[8] = rel_patch_id;
   ctx->block->instructions.emplace_back(std::move(jump));
}

static void
create_tcs_end_for_epilog(isel_context* ctx)
{
   std::vector<Operand> regs;

   regs.emplace_back(get_arg_for_end(ctx, ctx->program->info.tcs.tcs_offchip_layout));
   regs.emplace_back(get_arg_for_end(ctx, ctx->program->info.tcs.tes_offchip_addr));
   regs.emplace_back(get_arg_for_end(ctx, ctx->args->tess_offchip_offset));
   regs.emplace_back(get_arg_for_end(ctx, ctx->args->tcs_factor_offset));

   Builder bld(ctx->program, ctx->block);

   /* Leave a hole corresponding to the two input VGPRs. This ensures that
    * the invocation_id output does not alias the tcs_rel_ids input,
    * which saves a V_MOV on gfx9.
    */
   unsigned vgpr = 256 + ctx->args->num_vgprs_used;

   Temp rel_patch_id =
      bld.pseudo(aco_opcode::p_extract, bld.def(v1), get_arg(ctx, ctx->args->tcs_rel_ids),
                 Operand::c32(0u), Operand::c32(8u), Operand::c32(0u));
   regs.emplace_back(Operand(rel_patch_id, PhysReg{vgpr++}));

   Temp invocation_id =
      bld.vop3(aco_opcode::v_bfe_u32, bld.def(v1), get_arg(ctx, ctx->args->tcs_rel_ids),
               Operand::c32(8u), Operand::c32(5u));
   regs.emplace_back(Operand(invocation_id, PhysReg{vgpr++}));

   if (ctx->program->info.tcs.pass_tessfactors_by_reg) {
      vgpr++; /* skip the tess factor LDS offset */

      unsigned slot = VARYING_SLOT_TESS_LEVEL_OUTER;
      u_foreach_bit (i, ctx->outputs.mask[slot]) {
         regs.emplace_back(Operand(ctx->outputs.temps[slot * 4 + i], PhysReg{vgpr + i}));
      }
      vgpr += 4;

      slot = VARYING_SLOT_TESS_LEVEL_INNER;
      u_foreach_bit (i, ctx->outputs.mask[slot]) {
         regs.emplace_back(Operand(ctx->outputs.temps[slot * 4 + i], PhysReg{vgpr + i}));
      }
   } else {
      Temp patch0_patch_data_offset =
         bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc),
                  get_arg(ctx, ctx->program->info.tcs.vs_state_bits), Operand::c32(0xe000a));

      Temp tf_lds_offset =
         bld.v_mul24_imm(bld.def(v1), rel_patch_id, ctx->program->info.tcs.patch_stride);
      tf_lds_offset = bld.nuw().vadd32(bld.def(v1), tf_lds_offset, patch0_patch_data_offset);

      regs.emplace_back(Operand(tf_lds_offset, PhysReg{vgpr}));
   }

   build_end_with_regs(ctx, regs);
}

static void
create_fs_end_for_epilog(isel_context* ctx)
{
   Builder bld(ctx->program, ctx->block);

   std::vector<Operand> regs;

   regs.emplace_back(get_arg_for_end(ctx, ctx->program->info.ps.alpha_reference));

   unsigned vgpr = 256;

   for (unsigned slot = FRAG_RESULT_DATA0; slot <= FRAG_RESULT_DATA7; slot++) {
      unsigned index = slot - FRAG_RESULT_DATA0;
      unsigned type = (ctx->output_color_types >> (index * 2)) & 0x3;
      unsigned write_mask = ctx->outputs.mask[slot];

      if (!write_mask)
         continue;

      if (type == ACO_TYPE_ANY32) {
         u_foreach_bit (i, write_mask) {
            regs.emplace_back(Operand(ctx->outputs.temps[slot * 4 + i], PhysReg{vgpr + i}));
         }
      } else {
         for (unsigned i = 0; i < 2; i++) {
            unsigned mask = (write_mask >> (i * 2)) & 0x3;
            if (!mask)
               continue;

            unsigned chan = slot * 4 + i * 2;
            Operand lo = mask & 0x1 ? Operand(ctx->outputs.temps[chan]) : Operand(v2b);
            Operand hi = mask & 0x2 ? Operand(ctx->outputs.temps[chan + 1]) : Operand(v2b);

            Temp dst = bld.pseudo(aco_opcode::p_create_vector, bld.def(v1), lo, hi);
            regs.emplace_back(Operand(dst, PhysReg{vgpr + i}));
         }
      }
      vgpr += 4;
   }

   if (ctx->outputs.mask[FRAG_RESULT_DEPTH])
      regs.emplace_back(Operand(ctx->outputs.temps[FRAG_RESULT_DEPTH * 4], PhysReg{vgpr++}));

   if (ctx->outputs.mask[FRAG_RESULT_STENCIL])
      regs.emplace_back(Operand(ctx->outputs.temps[FRAG_RESULT_STENCIL * 4], PhysReg{vgpr++}));

   if (ctx->outputs.mask[FRAG_RESULT_SAMPLE_MASK])
      regs.emplace_back(Operand(ctx->outputs.temps[FRAG_RESULT_SAMPLE_MASK * 4], PhysReg{vgpr++}));

   build_end_with_regs(ctx, regs);

   /* Exit WQM mode finally. */
   ctx->program->needs_exact = true;
}

Pseudo_instruction*
add_startpgm(struct isel_context* ctx)
{
   unsigned def_count = 0;
   for (unsigned i = 0; i < ctx->args->arg_count; i++) {
      if (ctx->args->args[i].skip)
         continue;
      unsigned align = MIN2(4, util_next_power_of_two(ctx->args->args[i].size));
      if (ctx->args->args[i].file == AC_ARG_SGPR && ctx->args->args[i].offset % align)
         def_count += ctx->args->args[i].size;
      else
         def_count++;
   }

   Pseudo_instruction* startpgm =
      create_instruction<Pseudo_instruction>(aco_opcode::p_startpgm, Format::PSEUDO, 0, def_count);
   ctx->block->instructions.emplace_back(startpgm);
   for (unsigned i = 0, arg = 0; i < ctx->args->arg_count; i++) {
      if (ctx->args->args[i].skip)
         continue;

      enum ac_arg_regfile file = ctx->args->args[i].file;
      unsigned size = ctx->args->args[i].size;
      unsigned reg = ctx->args->args[i].offset;
      RegClass type = RegClass(file == AC_ARG_SGPR ? RegType::sgpr : RegType::vgpr, size);

      if (file == AC_ARG_SGPR && reg % MIN2(4, util_next_power_of_two(size))) {
         Temp elems[16];
         for (unsigned j = 0; j < size; j++) {
            elems[j] = ctx->program->allocateTmp(s1);
            startpgm->definitions[arg++] = Definition(elems[j].id(), PhysReg{reg + j}, s1);
         }
         ctx->arg_temps[i] = create_vec_from_array(ctx, elems, size, RegType::sgpr, 4);
      } else {
         Temp dst = ctx->program->allocateTmp(type);
         Definition def(dst);
         def.setFixed(PhysReg{file == AC_ARG_SGPR ? reg : reg + 256});
         ctx->arg_temps[i] = dst;
         startpgm->definitions[arg++] = def;

         if (ctx->args->args[i].pending_vmem) {
            assert(file == AC_ARG_VGPR);
            ctx->program->args_pending_vmem.push_back(def);
         }
      }
   }

   /* epilog has no scratch */
   if (ctx->args->scratch_offset.used) {
      if (ctx->program->gfx_level < GFX9) {
         /* Stash these in the program so that they can be accessed later when
          * handling spilling.
          */
         if (ctx->args->ring_offsets.used)
            ctx->program->private_segment_buffer = get_arg(ctx, ctx->args->ring_offsets);

         ctx->program->scratch_offset = get_arg(ctx, ctx->args->scratch_offset);
      } else if (ctx->program->gfx_level <= GFX10_3 && ctx->program->stage != raytracing_cs) {
         /* Manually initialize scratch. For RT stages scratch initialization is done in the prolog.
          */
         Operand scratch_offset = Operand(get_arg(ctx, ctx->args->scratch_offset));
         scratch_offset.setLateKill(true);

         Operand scratch_addr = ctx->args->ring_offsets.used
                                   ? Operand(get_arg(ctx, ctx->args->ring_offsets))
                                   : Operand(s2);

         Builder bld(ctx->program, ctx->block);
         bld.pseudo(aco_opcode::p_init_scratch, bld.def(s2), bld.def(s1, scc), scratch_addr,
                    scratch_offset);
      }
   }

   return startpgm;
}

void
fix_ls_vgpr_init_bug(isel_context* ctx)
{
   Builder bld(ctx->program, ctx->block);
   constexpr unsigned hs_idx = 1u;
   Builder::Result hs_thread_count =
      bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc),
               get_arg(ctx, ctx->args->merged_wave_info), Operand::c32((8u << 16) | (hs_idx * 8u)));
   Temp ls_has_nonzero_hs_threads = bool_to_vector_condition(ctx, hs_thread_count.def(1).getTemp());

   /* If there are no HS threads, SPI mistakenly loads the LS VGPRs starting at VGPR 0. */

   Temp instance_id =
      bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), get_arg(ctx, ctx->args->vertex_id),
               get_arg(ctx, ctx->args->instance_id), ls_has_nonzero_hs_threads);
   Temp vs_rel_patch_id =
      bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), get_arg(ctx, ctx->args->tcs_rel_ids),
               get_arg(ctx, ctx->args->vs_rel_patch_id), ls_has_nonzero_hs_threads);
   Temp vertex_id =
      bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), get_arg(ctx, ctx->args->tcs_patch_id),
               get_arg(ctx, ctx->args->vertex_id), ls_has_nonzero_hs_threads);

   ctx->arg_temps[ctx->args->instance_id.arg_index] = instance_id;
   ctx->arg_temps[ctx->args->vs_rel_patch_id.arg_index] = vs_rel_patch_id;
   ctx->arg_temps[ctx->args->vertex_id.arg_index] = vertex_id;
}

void
split_arguments(isel_context* ctx, Pseudo_instruction* startpgm)
{
   /* Split all arguments except for the first (ring_offsets) and the last
    * (exec) so that the dead channels don't stay live throughout the program.
    */
   for (int i = 1; i < startpgm->definitions.size(); i++) {
      if (startpgm->definitions[i].regClass().size() > 1) {
         emit_split_vector(ctx, startpgm->definitions[i].getTemp(),
                           startpgm->definitions[i].regClass().size());
      }
   }
}

void
setup_fp_mode(isel_context* ctx, nir_shader* shader)
{
   Program* program = ctx->program;

   unsigned float_controls = shader->info.float_controls_execution_mode;

   program->next_fp_mode.preserve_signed_zero_inf_nan32 =
      float_controls & FLOAT_CONTROLS_SIGNED_ZERO_INF_NAN_PRESERVE_FP32;
   program->next_fp_mode.preserve_signed_zero_inf_nan16_64 =
      float_controls & (FLOAT_CONTROLS_SIGNED_ZERO_INF_NAN_PRESERVE_FP16 |
                        FLOAT_CONTROLS_SIGNED_ZERO_INF_NAN_PRESERVE_FP64);

   program->next_fp_mode.must_flush_denorms32 =
      float_controls & FLOAT_CONTROLS_DENORM_FLUSH_TO_ZERO_FP32;
   program->next_fp_mode.must_flush_denorms16_64 =
      float_controls &
      (FLOAT_CONTROLS_DENORM_FLUSH_TO_ZERO_FP16 | FLOAT_CONTROLS_DENORM_FLUSH_TO_ZERO_FP64);

   program->next_fp_mode.care_about_round32 =
      float_controls &
      (FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP32 | FLOAT_CONTROLS_ROUNDING_MODE_RTE_FP32);

   program->next_fp_mode.care_about_round16_64 =
      float_controls &
      (FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP16 | FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP64 |
       FLOAT_CONTROLS_ROUNDING_MODE_RTE_FP16 | FLOAT_CONTROLS_ROUNDING_MODE_RTE_FP64);

   /* default to preserving fp16 and fp64 denorms, since it's free for fp64 and
    * the precision seems needed for Wolfenstein: Youngblood to render correctly */
   if (program->next_fp_mode.must_flush_denorms16_64)
      program->next_fp_mode.denorm16_64 = 0;
   else
      program->next_fp_mode.denorm16_64 = fp_denorm_keep;

   /* preserving fp32 denorms is expensive, so only do it if asked */
   if (float_controls & FLOAT_CONTROLS_DENORM_PRESERVE_FP32)
      program->next_fp_mode.denorm32 = fp_denorm_keep;
   else
      program->next_fp_mode.denorm32 = 0;

   if (float_controls & FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP32)
      program->next_fp_mode.round32 = fp_round_tz;
   else
      program->next_fp_mode.round32 = fp_round_ne;

   if (float_controls &
       (FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP16 | FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP64))
      program->next_fp_mode.round16_64 = fp_round_tz;
   else
      program->next_fp_mode.round16_64 = fp_round_ne;

   ctx->block->fp_mode = program->next_fp_mode;
}

void
cleanup_cfg(Program* program)
{
   /* create linear_succs/logical_succs */
   for (Block& BB : program->blocks) {
      for (unsigned idx : BB.linear_preds)
         program->blocks[idx].linear_succs.emplace_back(BB.index);
      for (unsigned idx : BB.logical_preds)
         program->blocks[idx].logical_succs.emplace_back(BB.index);
   }
}

void
finish_program(isel_context* ctx)
{
   cleanup_cfg(ctx->program);

   /* Insert a single p_end_wqm instruction after the last derivative calculation */
   if (ctx->program->stage == fragment_fs && ctx->program->needs_wqm && ctx->program->needs_exact) {
      /* Find the next BB at top-level CFG */
      while (!(ctx->program->blocks[ctx->wqm_block_idx].kind & block_kind_top_level)) {
         ctx->wqm_block_idx++;
         ctx->wqm_instruction_idx = 0;
      }

      std::vector<aco_ptr<Instruction>>* instrs =
         &ctx->program->blocks[ctx->wqm_block_idx].instructions;
      auto it = instrs->begin() + ctx->wqm_instruction_idx;

      /* Delay transistion to Exact to help optimizations and scheduling */
      while (it != instrs->end()) {
         aco_ptr<Instruction>& instr = *it;
         /* End WQM before: */
         if (instr->isVMEM() || instr->isFlatLike() || instr->isDS() || instr->isEXP() ||
             instr->opcode == aco_opcode::p_dual_src_export_gfx11 ||
             instr->opcode == aco_opcode::p_jump_to_epilog ||
             instr->opcode == aco_opcode::p_logical_start)
            break;

         ++it;

         /* End WQM after: */
         if (instr->opcode == aco_opcode::p_logical_end ||
             instr->opcode == aco_opcode::p_discard_if ||
             instr->opcode == aco_opcode::p_demote_to_helper ||
             instr->opcode == aco_opcode::p_end_with_regs)
            break;
      }

      Builder bld(ctx->program);
      bld.reset(instrs, it);
      bld.pseudo(aco_opcode::p_end_wqm);
   }
}

Temp
lanecount_to_mask(isel_context* ctx, Temp count)
{
   assert(count.regClass() == s1);

   Builder bld(ctx->program, ctx->block);
   Temp mask = bld.sop2(aco_opcode::s_bfm_b64, bld.def(s2), count, Operand::zero());
   Temp cond;

   if (ctx->program->wave_size == 64) {
      /* Special case for 64 active invocations, because 64 doesn't work with s_bfm */
      Temp active_64 = bld.sopc(aco_opcode::s_bitcmp1_b32, bld.def(s1, scc), count,
                                Operand::c32(6u /* log2(64) */));
      cond =
         bld.sop2(Builder::s_cselect, bld.def(bld.lm), Operand::c32(-1u), mask, bld.scc(active_64));
   } else {
      /* We use s_bfm_b64 (not _b32) which works with 32, but we need to extract the lower half of
       * the register */
      cond = emit_extract_vector(ctx, mask, 0, bld.lm);
   }

   return cond;
}

Temp
merged_wave_info_to_mask(isel_context* ctx, unsigned i)
{
   Builder bld(ctx->program, ctx->block);

   /* lanecount_to_mask() only cares about s0.u[6:0] so we don't need either s_bfe nor s_and here */
   Temp count = i == 0 ? get_arg(ctx, ctx->args->merged_wave_info)
                       : bld.sop2(aco_opcode::s_lshr_b32, bld.def(s1), bld.def(s1, scc),
                                  get_arg(ctx, ctx->args->merged_wave_info), Operand::c32(i * 8u));

   return lanecount_to_mask(ctx, count);
}

static void
insert_rt_jump_next(isel_context& ctx, const struct ac_shader_args* args)
{
   unsigned src_count = ctx.args->arg_count;
   Pseudo_instruction* ret =
      create_instruction<Pseudo_instruction>(aco_opcode::p_return, Format::PSEUDO, src_count, 0);
   ctx.block->instructions.emplace_back(ret);

   for (unsigned i = 0; i < src_count; i++) {
      enum ac_arg_regfile file = ctx.args->args[i].file;
      unsigned size = ctx.args->args[i].size;
      unsigned reg = ctx.args->args[i].offset + (file == AC_ARG_SGPR ? 0 : 256);
      RegClass type = RegClass(file == AC_ARG_SGPR ? RegType::sgpr : RegType::vgpr, size);
      Operand op = ctx.arg_temps[i].id() ? Operand(ctx.arg_temps[i], PhysReg{reg})
                                         : Operand(PhysReg{reg}, type);
      ret->operands[i] = op;
   }

   Builder bld(ctx.program, ctx.block);
   bld.sop1(aco_opcode::s_setpc_b64, get_arg(&ctx, ctx.args->rt.uniform_shader_addr));
}

void
select_program_rt(isel_context& ctx, unsigned shader_count, struct nir_shader* const* shaders,
                  const struct ac_shader_args* args)
{
   for (unsigned i = 0; i < shader_count; i++) {
      if (i) {
         ctx.block = ctx.program->create_and_insert_block();
         ctx.block->kind = block_kind_top_level | block_kind_resume;
      }

      nir_shader* nir = shaders[i];
      init_context(&ctx, nir);
      setup_fp_mode(&ctx, nir);

      Pseudo_instruction* startpgm = add_startpgm(&ctx);
      append_logical_start(ctx.block);
      split_arguments(&ctx, startpgm);
      visit_cf_list(&ctx, &nir_shader_get_entrypoint(nir)->body);
      append_logical_end(ctx.block);
      ctx.block->kind |= block_kind_uniform;

      /* Fix output registers and jump to next shader. We can skip this when dealing with a raygen
       * shader without shader calls.
       */
      if (shader_count > 1 || shaders[i]->info.stage != MESA_SHADER_RAYGEN)
         insert_rt_jump_next(ctx, args);

      cleanup_context(&ctx);
   }

   ctx.program->config->float_mode = ctx.program->blocks[0].fp_mode.val;
   finish_program(&ctx);
}

void
pops_await_overlapped_waves(isel_context* ctx)
{
   ctx->program->has_pops_overlapped_waves_wait = true;

   Builder bld(ctx->program, ctx->block);

   if (ctx->program->gfx_level >= GFX11) {
      /* GFX11+ - waiting for the export from the overlapped waves.
       * Await the export_ready event (bit wait_event_imm_dont_wait_export_ready clear).
       */
      bld.sopp(aco_opcode::s_wait_event, -1, 0);
      return;
   }

   /* Pre-GFX11 - sleep loop polling the exiting wave ID. */

   const Temp collision = get_arg(ctx, ctx->args->pops_collision_wave_id);

   /* Check if there's an overlap in the current wave - otherwise, the wait may result in a hang. */
   const Temp did_overlap =
      bld.sopc(aco_opcode::s_bitcmp1_b32, bld.def(s1, scc), collision, Operand::c32(31));
   if_context did_overlap_if_context;
   begin_uniform_if_then(ctx, &did_overlap_if_context, did_overlap);
   bld.reset(ctx->block);

   /* Set the packer register - after this, pops_exiting_wave_id can be polled. */
   if (ctx->program->gfx_level >= GFX10) {
      /* 2 packer ID bits on GFX10-10.3. */
      const Temp packer_id = bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc),
                                      collision, Operand::c32(0x2001c));
      /* POPS_PACKER register: bit 0 - POPS enabled for this wave, bits 2:1 - packer ID. */
      const Temp packer_id_hwreg_bits = bld.sop2(aco_opcode::s_lshl1_add_u32, bld.def(s1),
                                                 bld.def(s1, scc), packer_id, Operand::c32(1));
      bld.sopk(aco_opcode::s_setreg_b32, packer_id_hwreg_bits, ((3 - 1) << 11) | 25);
   } else {
      /* 1 packer ID bit on GFX9. */
      const Temp packer_id = bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc),
                                      collision, Operand::c32(0x1001c));
      /* MODE register: bit 24 - wave is associated with packer 0, bit 25 - with packer 1.
       * Packer index to packer bits: 0 to 0b01, 1 to 0b10.
       */
      const Temp packer_id_hwreg_bits =
         bld.sop2(aco_opcode::s_add_i32, bld.def(s1), bld.def(s1, scc), packer_id, Operand::c32(1));
      bld.sopk(aco_opcode::s_setreg_b32, packer_id_hwreg_bits, ((2 - 1) << 11) | (24 << 6) | 1);
   }

   Temp newest_overlapped_wave_id = bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc),
                                             collision, Operand::c32(0xa0010));
   if (ctx->program->gfx_level < GFX10) {
      /* On GFX9, the newest overlapped wave ID value passed to the shader is smaller than the
       * actual wave ID by 1 in case of wraparound.
       */
      const Temp current_wave_id = bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc),
                                            collision, Operand::c32(0x3ff));
      const Temp newest_overlapped_wave_id_wrapped = bld.sopc(
         aco_opcode::s_cmp_gt_u32, bld.def(s1, scc), newest_overlapped_wave_id, current_wave_id);
      newest_overlapped_wave_id =
         bld.sop2(aco_opcode::s_add_i32, bld.def(s1), bld.def(s1, scc), newest_overlapped_wave_id,
                  newest_overlapped_wave_id_wrapped);
   }

   /* The wave IDs are the low 10 bits of a monotonically increasing wave counter.
    * The overlapped and the exiting wave IDs can't be larger than the current wave ID, and they are
    * no more than 1023 values behind the current wave ID.
    * Remap the overlapped and the exiting wave IDs from wrapping to monotonic so an unsigned
    * comparison can be used: the wave `current - 1023` becomes 0, it's followed by a piece growing
    * away from 0, then a piece increasing until UINT32_MAX, and the current wave is UINT32_MAX.
    * To do that, subtract `current - 1023`, which with wrapping arithmetic is (current + 1), and
    * `a - (b + 1)` is `a + ~b`.
    * Note that if the 10-bit current wave ID is 1023 (thus 1024 will be subtracted), the wave
    * `current - 1023` will become `UINT32_MAX - 1023` rather than 0, but all the possible wave IDs
    * will still grow monotonically in the 32-bit value, and the unsigned comparison will behave as
    * expected.
    */
   const Temp wave_id_offset = bld.sop2(aco_opcode::s_nand_b32, bld.def(s1), bld.def(s1, scc),
                                        collision, Operand::c32(0x3ff));
   newest_overlapped_wave_id = bld.sop2(aco_opcode::s_add_i32, bld.def(s1), bld.def(s1, scc),
                                        newest_overlapped_wave_id, wave_id_offset);

   /* Await the overlapped waves. */

   loop_context wait_loop_context;
   begin_loop(ctx, &wait_loop_context);
   bld.reset(ctx->block);

   const Temp exiting_wave_id = bld.pseudo(aco_opcode::p_pops_gfx9_add_exiting_wave_id, bld.def(s1),
                                           bld.def(s1, scc), wave_id_offset);
   /* If the exiting (not exited) wave ID is larger than the newest overlapped wave ID (after
    * remapping both to monotonically increasing unsigned integers), the newest overlapped wave has
    * exited the ordered section.
    */
   const Temp newest_overlapped_wave_exited = bld.sopc(aco_opcode::s_cmp_lt_u32, bld.def(s1, scc),
                                                       newest_overlapped_wave_id, exiting_wave_id);
   if_context newest_overlapped_wave_exited_if_context;
   begin_uniform_if_then(ctx, &newest_overlapped_wave_exited_if_context,
                         newest_overlapped_wave_exited);
   emit_loop_break(ctx);
   begin_uniform_if_else(ctx, &newest_overlapped_wave_exited_if_context);
   end_uniform_if(ctx, &newest_overlapped_wave_exited_if_context);
   bld.reset(ctx->block);

   /* Sleep before rechecking to let overlapped waves run for some time. */
   bld.sopp(aco_opcode::s_sleep, -1, ctx->program->gfx_level >= GFX10 ? UINT16_MAX : 3);

   end_loop(ctx, &wait_loop_context);
   bld.reset(ctx->block);

   /* Indicate the wait has been done to subsequent compilation stages. */
   bld.pseudo(aco_opcode::p_pops_gfx9_overlapped_wave_wait_done);

   begin_uniform_if_else(ctx, &did_overlap_if_context);
   end_uniform_if(ctx, &did_overlap_if_context);
   bld.reset(ctx->block);
}

static void
create_merged_jump_to_epilog(isel_context* ctx)
{
   Builder bld(ctx->program, ctx->block);
   std::vector<Operand> regs;

   for (unsigned i = 0; i < ctx->args->arg_count; i++) {
      if (!ctx->args->args[i].preserved)
         continue;

      const enum ac_arg_regfile file = ctx->args->args[i].file;
      const unsigned reg = ctx->args->args[i].offset;

      Operand op(ctx->arg_temps[i]);
      op.setFixed(PhysReg{file == AC_ARG_SGPR ? reg : reg + 256});
      regs.emplace_back(op);
   }

   Temp continue_pc =
      convert_pointer_to_64_bit(ctx, get_arg(ctx, ctx->program->info.next_stage_pc));

   aco_ptr<Pseudo_instruction> jump{create_instruction<Pseudo_instruction>(
      aco_opcode::p_jump_to_epilog, Format::PSEUDO, 1 + regs.size(), 0)};
   jump->operands[0] = Operand(continue_pc);
   for (unsigned i = 0; i < regs.size(); i++) {
      jump->operands[i + 1] = regs[i];
   }
   ctx->block->instructions.emplace_back(std::move(jump));
}

static void
create_end_for_merged_shader(isel_context* ctx)
{
   std::vector<Operand> regs;

   unsigned max_args;
   if (ctx->stage.sw == SWStage::VS) {
      assert(ctx->args->vertex_id.used);
      max_args = ctx->args->vertex_id.arg_index;
   } else {
      assert(ctx->stage.sw == SWStage::TES);
      assert(ctx->args->tes_u.used);
      max_args = ctx->args->tes_u.arg_index;
   }

   struct ac_arg arg;
   arg.used = true;

   for (arg.arg_index = 0; arg.arg_index < max_args; arg.arg_index++)
      regs.emplace_back(get_arg_for_end(ctx, arg));

   build_end_with_regs(ctx, regs);
}

void
select_shader(isel_context& ctx, nir_shader* nir, const bool need_startpgm, const bool need_endpgm,
              const bool need_barrier, if_context* ic_merged_wave_info,
              const bool check_merged_wave_info, const bool endif_merged_wave_info)
{
   init_context(&ctx, nir);
   setup_fp_mode(&ctx, nir);

   Program* program = ctx.program;

   if (need_startpgm) {
      /* Needs to be after init_context() for FS. */
      Pseudo_instruction* startpgm = add_startpgm(&ctx);
      append_logical_start(ctx.block);

      if (ctx.options->has_ls_vgpr_init_bug && ctx.stage == vertex_tess_control_hs &&
          !program->info.vs.has_prolog)
         fix_ls_vgpr_init_bug(&ctx);

      split_arguments(&ctx, startpgm);

      if (!program->info.vs.has_prolog &&
          (program->stage.has(SWStage::VS) || program->stage.has(SWStage::TES))) {
         Builder(ctx.program, ctx.block).sopp(aco_opcode::s_setprio, -1u, 0x3u);
      }
   }

   if (program->gfx_level == GFX10 && program->stage.hw == AC_HW_NEXT_GEN_GEOMETRY_SHADER &&
       !program->stage.has(SWStage::GS)) {
      /* Workaround for Navi1x HW bug to ensure that all NGG waves launch before
       * s_sendmsg(GS_ALLOC_REQ).
       */
      Builder(ctx.program, ctx.block).sopp(aco_opcode::s_barrier, -1u, 0u);
   }

   if (check_merged_wave_info) {
      const unsigned i =
         nir->info.stage == MESA_SHADER_VERTEX || nir->info.stage == MESA_SHADER_TESS_EVAL ? 0 : 1;
      const Temp cond = merged_wave_info_to_mask(&ctx, i);
      begin_divergent_if_then(&ctx, ic_merged_wave_info, cond);
   }

   if (need_barrier) {
      const sync_scope scope = ctx.stage == vertex_tess_control_hs && ctx.tcs_in_out_eq &&
                                     program->wave_size % nir->info.tess.tcs_vertices_out == 0
                                  ? scope_subgroup
                                  : scope_workgroup;

      Builder(ctx.program, ctx.block)
         .barrier(aco_opcode::p_barrier, memory_sync_info(storage_shared, semantic_acqrel, scope),
                  scope);
   }

   nir_function_impl* func = nir_shader_get_entrypoint(nir);
   visit_cf_list(&ctx, &func->body);

   if (ctx.program->info.has_epilog) {
      if (ctx.stage == fragment_fs) {
         if (ctx.options->is_opengl)
            create_fs_end_for_epilog(&ctx);
         else
            create_fs_jump_to_epilog(&ctx);

         /* FS epilogs always have at least one color/null export. */
         ctx.program->has_color_exports = true;
      } else if (nir->info.stage == MESA_SHADER_TESS_CTRL) {
         assert(ctx.stage == tess_control_hs || ctx.stage == vertex_tess_control_hs);
         if (ctx.options->is_opengl)
            create_tcs_end_for_epilog(&ctx);
         else
            create_tcs_jump_to_epilog(&ctx);
      }
   }

   if (endif_merged_wave_info) {
      begin_divergent_if_else(&ctx, ic_merged_wave_info);
      end_divergent_if(&ctx, ic_merged_wave_info);
   }

   bool is_first_stage_of_merged_shader = false;

   if (ctx.program->info.merged_shader_compiled_separately &&
       (ctx.stage.sw == SWStage::VS || ctx.stage.sw == SWStage::TES)) {
      assert(program->gfx_level >= GFX9);
      if (ctx.options->is_opengl)
         create_end_for_merged_shader(&ctx);
      else
         create_merged_jump_to_epilog(&ctx);

      is_first_stage_of_merged_shader = true;
   }

   cleanup_context(&ctx);

   if (need_endpgm) {
      program->config->float_mode = program->blocks[0].fp_mode.val;

      append_logical_end(ctx.block);
      ctx.block->kind |= block_kind_uniform;

      if ((!program->info.has_epilog && !is_first_stage_of_merged_shader) ||
          (nir->info.stage == MESA_SHADER_TESS_CTRL && program->gfx_level >= GFX9)) {
         Builder(program, ctx.block).sopp(aco_opcode::s_endpgm);
      }

      finish_program(&ctx);
   }
}

void
select_program_merged(isel_context& ctx, const unsigned shader_count, nir_shader* const* shaders)
{
   if_context ic_merged_wave_info;
   const bool ngg_gs = ctx.stage.hw == AC_HW_NEXT_GEN_GEOMETRY_SHADER && ctx.stage.has(SWStage::GS);

   for (unsigned i = 0; i < shader_count; i++) {
      nir_shader* nir = shaders[i];

      /* We always need to insert p_startpgm at the beginning of the first shader.  */
      const bool need_startpgm = i == 0;

      /* Need to handle program end for last shader stage. */
      const bool need_endpgm = i == shader_count - 1;

      /* In a merged VS+TCS HS, the VS implementation can be completely empty. */
      nir_function_impl* func = nir_shader_get_entrypoint(nir);
      const bool empty_shader =
         nir_cf_list_is_empty_block(&func->body) &&
         ((nir->info.stage == MESA_SHADER_VERTEX &&
           (ctx.stage == vertex_tess_control_hs || ctx.stage == vertex_geometry_gs)) ||
          (nir->info.stage == MESA_SHADER_TESS_EVAL && ctx.stage == tess_eval_geometry_gs));

      /* See if we need to emit a check of the merged wave info SGPR. */
      const bool check_merged_wave_info =
         ctx.tcs_in_out_eq ? i == 0 : (shader_count >= 2 && !empty_shader && !(ngg_gs && i == 1));
      const bool endif_merged_wave_info =
         ctx.tcs_in_out_eq ? i == 1 : (check_merged_wave_info && !(ngg_gs && i == 1));

      /* Skip s_barrier from TCS when VS outputs are not stored in the LDS. */
      const bool tcs_skip_barrier =
         ctx.stage == vertex_tess_control_hs && ctx.tcs_temp_only_inputs == nir->info.inputs_read;

      /* A barrier is usually needed at the beginning of the second shader, with exceptions. */
      const bool need_barrier = i != 0 && !ngg_gs && !tcs_skip_barrier;

      select_shader(ctx, nir, need_startpgm, need_endpgm, need_barrier, &ic_merged_wave_info,
                    check_merged_wave_info, endif_merged_wave_info);

      if (i == 0 && ctx.stage == vertex_tess_control_hs && ctx.tcs_in_out_eq) {
         /* Special handling when TCS input and output patch size is the same.
          * Outputs of the previous stage are inputs to the next stage.
          */
         ctx.inputs = ctx.outputs;
         ctx.outputs = shader_io_state();
      }
   }
}

Temp
get_tess_ring_descriptor(isel_context* ctx, const struct aco_tcs_epilog_info* einfo,
                         bool is_tcs_factor_ring)
{
   Builder bld(ctx->program, ctx->block);

   if (!ctx->options->is_opengl) {
      Temp ring_offsets = get_arg(ctx, ctx->args->ring_offsets);
      uint32_t tess_ring_offset =
         is_tcs_factor_ring ? 5 /* RING_HS_TESS_FACTOR */ : 6 /* RING_HS_TESS_OFFCHIP */;
      return bld.smem(aco_opcode::s_load_dwordx4, bld.def(s4), ring_offsets,
                      Operand::c32(tess_ring_offset * 16u));
   }

   Temp addr = get_arg(ctx, einfo->tcs_out_lds_layout);
   /* TCS only receives high 13 bits of the address. */
   addr = bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc), addr,
                   Operand::c32(0xfff80000));

   if (is_tcs_factor_ring) {
      addr = bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.def(s1, scc), addr,
                      Operand::c32(einfo->tess_offchip_ring_size));
   }

   uint32_t rsrc3 = S_008F0C_DST_SEL_X(V_008F0C_SQ_SEL_X) | S_008F0C_DST_SEL_Y(V_008F0C_SQ_SEL_Y) |
                    S_008F0C_DST_SEL_Z(V_008F0C_SQ_SEL_Z) | S_008F0C_DST_SEL_W(V_008F0C_SQ_SEL_W);

   if (ctx->options->gfx_level >= GFX11) {
      rsrc3 |= S_008F0C_FORMAT(V_008F0C_GFX11_FORMAT_32_FLOAT) |
               S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_RAW);
   } else if (ctx->options->gfx_level >= GFX10) {
      rsrc3 |= S_008F0C_FORMAT(V_008F0C_GFX10_FORMAT_32_FLOAT) |
               S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_RAW) | S_008F0C_RESOURCE_LEVEL(1);
   } else {
      rsrc3 |= S_008F0C_NUM_FORMAT(V_008F0C_BUF_NUM_FORMAT_FLOAT) |
               S_008F0C_DATA_FORMAT(V_008F0C_BUF_DATA_FORMAT_32);
   }

   return bld.pseudo(aco_opcode::p_create_vector, bld.def(s4), addr,
                     Operand::c32(ctx->options->address32_hi), Operand::c32(0xffffffff),
                     Operand::c32(rsrc3));
}

void
store_tess_factor_to_tess_ring(isel_context* ctx, Temp tess_ring_desc, Temp factors[],
                               unsigned factor_comps, Temp sbase, Temp voffset, Temp num_patches,
                               unsigned patch_offset)
{
   Builder bld(ctx->program, ctx->block);

   Temp soffset = sbase;
   if (patch_offset) {
      Temp offset =
         bld.sop2(aco_opcode::s_mul_i32, bld.def(s1), num_patches, Operand::c32(patch_offset));
      soffset = bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.def(s1, scc), soffset, offset);
   }

   Temp data = factor_comps == 1
                  ? factors[0]
                  : create_vec_from_array(ctx, factors, factor_comps, RegType::vgpr, 4);

   emit_single_mubuf_store(ctx, tess_ring_desc, voffset, soffset, Temp(), data, 0,
                           memory_sync_info(storage_vmem_output), true, false, false);
}

Temp
build_fast_udiv_nuw(isel_context* ctx, Temp num, Temp multiplier, Temp pre_shift, Temp post_shift,
                    Temp increment)
{
   Builder bld(ctx->program, ctx->block);

   num = bld.vop2(aco_opcode::v_lshrrev_b32, bld.def(v1), pre_shift, num);
   num = bld.nuw().vadd32(bld.def(v1), num, increment);
   num = bld.vop3(aco_opcode::v_mul_hi_u32, bld.def(v1), num, multiplier);
   return bld.vop2(aco_opcode::v_lshrrev_b32, bld.def(v1), post_shift, num);
}

Temp
get_gl_vs_prolog_vertex_index(isel_context* ctx, const struct aco_gl_vs_prolog_info* vinfo,
                              unsigned input_index, Temp instance_divisor_constbuf)
{
   bool divisor_is_one = vinfo->instance_divisor_is_one & (1u << input_index);
   bool divisor_is_fetched = vinfo->instance_divisor_is_fetched & (1u << input_index);

   Builder bld(ctx->program, ctx->block);

   Temp index;
   if (divisor_is_one) {
      index = get_arg(ctx, ctx->args->instance_id);
   } else if (divisor_is_fetched) {
      Temp instance_id = get_arg(ctx, ctx->args->instance_id);

      Temp udiv_factors = bld.smem(aco_opcode::s_buffer_load_dwordx4, bld.def(s4),
                                   instance_divisor_constbuf, Operand::c32(input_index * 16));
      emit_split_vector(ctx, udiv_factors, 4);

      index = build_fast_udiv_nuw(ctx, instance_id, emit_extract_vector(ctx, udiv_factors, 0, s1),
                                  emit_extract_vector(ctx, udiv_factors, 1, s1),
                                  emit_extract_vector(ctx, udiv_factors, 2, s1),
                                  emit_extract_vector(ctx, udiv_factors, 3, s1));
   }

   if (divisor_is_one || divisor_is_fetched) {
      Temp start_instance = get_arg(ctx, ctx->args->start_instance);
      index = bld.vadd32(bld.def(v1), index, start_instance);
   } else {
      Temp base_vertex = get_arg(ctx, ctx->args->base_vertex);
      Temp vertex_id = get_arg(ctx, ctx->args->vertex_id);
      index = bld.vadd32(bld.def(v1), base_vertex, vertex_id);
   }

   return index;
}

void
emit_polygon_stipple(isel_context* ctx, const struct aco_ps_prolog_info* finfo)
{
   Builder bld(ctx->program, ctx->block);

   /* Use the fixed-point gl_FragCoord input.
    * Since the stipple pattern is 32x32 and it repeats, just get 5 bits
    * per coordinate to get the repeating effect.
    */
   Temp pos_fixed_pt = get_arg(ctx, ctx->args->pos_fixed_pt);
   Temp addr0 = bld.vop2(aco_opcode::v_and_b32, bld.def(v1), Operand::c32(0x1f), pos_fixed_pt);
   Temp addr1 = bld.vop3(aco_opcode::v_bfe_u32, bld.def(v1), pos_fixed_pt, Operand::c32(16u),
                         Operand::c32(5u));

   /* Load the buffer descriptor. */
   Temp list = get_arg(ctx, finfo->internal_bindings);
   list = convert_pointer_to_64_bit(ctx, list);
   Temp desc = bld.smem(aco_opcode::s_load_dwordx4, bld.def(s4), list,
                        Operand::c32(finfo->poly_stipple_buf_offset));

   /* The stipple pattern is 32x32, each row has 32 bits. */
   Temp offset = bld.vop2(aco_opcode::v_lshlrev_b32, bld.def(v1), Operand::c32(2), addr1);
   Temp row = bld.mubuf(aco_opcode::buffer_load_dword, bld.def(v1), desc, offset, Operand::c32(0u),
                        0, true);
   Temp bit = bld.vop3(aco_opcode::v_bfe_u32, bld.def(v1), row, addr0, Operand::c32(1u));
   Temp cond = bld.vopc(aco_opcode::v_cmp_eq_u32, bld.def(bld.lm), Operand::zero(), bit);
   bld.pseudo(aco_opcode::p_demote_to_helper, cond);

   ctx->block->kind |= block_kind_uses_discard;
   ctx->program->needs_exact = true;
}

void
overwrite_interp_args(isel_context* ctx, const struct aco_ps_prolog_info* finfo)
{
   Builder bld(ctx->program, ctx->block);

   if (finfo->bc_optimize_for_persp || finfo->bc_optimize_for_linear) {
      /* The shader should do: if (PRIM_MASK[31]) CENTROID = CENTER;
       * The hw doesn't compute CENTROID if the whole wave only
       * contains fully-covered quads.
       */
      Temp bc_optimize = get_arg(ctx, ctx->args->prim_mask);

      /* enabled when bit 31 is set */
      Temp cond =
         bld.sopc(aco_opcode::s_bitcmp1_b32, bld.def(s1, scc), bc_optimize, Operand::c32(31u));

      /* scale 1bit scc to wave size bits used by v_cndmask */
      cond = bool_to_vector_condition(ctx, cond);

      if (finfo->bc_optimize_for_persp) {
         Temp center = get_arg(ctx, ctx->args->persp_center);
         Temp centroid = get_arg(ctx, ctx->args->persp_centroid);

         Temp dst = bld.tmp(v2);
         select_vec2(ctx, dst, cond, center, centroid);
         ctx->arg_temps[ctx->args->persp_centroid.arg_index] = dst;
      }

      if (finfo->bc_optimize_for_linear) {
         Temp center = get_arg(ctx, ctx->args->linear_center);
         Temp centroid = get_arg(ctx, ctx->args->linear_centroid);

         Temp dst = bld.tmp(v2);
         select_vec2(ctx, dst, cond, center, centroid);
         ctx->arg_temps[ctx->args->linear_centroid.arg_index] = dst;
      }
   }

   if (finfo->force_persp_sample_interp) {
      Temp persp_sample = get_arg(ctx, ctx->args->persp_sample);
      ctx->arg_temps[ctx->args->persp_center.arg_index] = persp_sample;
      ctx->arg_temps[ctx->args->persp_centroid.arg_index] = persp_sample;
   }

   if (finfo->force_linear_sample_interp) {
      Temp linear_sample = get_arg(ctx, ctx->args->linear_sample);
      ctx->arg_temps[ctx->args->linear_center.arg_index] = linear_sample;
      ctx->arg_temps[ctx->args->linear_centroid.arg_index] = linear_sample;
   }

   if (finfo->force_persp_center_interp) {
      Temp persp_center = get_arg(ctx, ctx->args->persp_center);
      ctx->arg_temps[ctx->args->persp_sample.arg_index] = persp_center;
      ctx->arg_temps[ctx->args->persp_centroid.arg_index] = persp_center;
   }

   if (finfo->force_linear_center_interp) {
      Temp linear_center = get_arg(ctx, ctx->args->linear_center);
      ctx->arg_temps[ctx->args->linear_sample.arg_index] = linear_center;
      ctx->arg_temps[ctx->args->linear_centroid.arg_index] = linear_center;
   }
}

void
overwrite_samplemask_arg(isel_context* ctx, const struct aco_ps_prolog_info* finfo)
{
   Builder bld(ctx->program, ctx->block);

   /* Section 15.2.2 (Shader Inputs) of the OpenGL 4.5 (Core Profile) spec
    * says:
    *
    *    "When per-sample shading is active due to the use of a fragment
    *     input qualified by sample or due to the use of the gl_SampleID
    *     or gl_SamplePosition variables, only the bit for the current
    *     sample is set in gl_SampleMaskIn. When state specifies multiple
    *     fragment shader invocations for a given fragment, the sample
    *     mask for any single fragment shader invocation may specify a
    *     subset of the covered samples for the fragment. In this case,
    *     the bit corresponding to each covered sample will be set in
    *     exactly one fragment shader invocation."
    *
    * The samplemask loaded by hardware is always the coverage of the
    * entire pixel/fragment, so mask bits out based on the sample ID.
    */
   if (finfo->samplemask_log_ps_iter) {
      Temp ancillary = get_arg(ctx, ctx->args->ancillary);
      Temp sampleid = bld.vop3(aco_opcode::v_bfe_u32, bld.def(v1), ancillary, Operand::c32(8u),
                               Operand::c32(4u));
      Temp samplemask = get_arg(ctx, ctx->args->sample_coverage);

      uint32_t ps_iter_mask = ac_get_ps_iter_mask(1 << finfo->samplemask_log_ps_iter);
      Temp iter_mask = bld.copy(bld.def(v1), Operand::c32(ps_iter_mask));

      Temp mask = bld.vop2(aco_opcode::v_lshlrev_b32, bld.def(v1), sampleid, iter_mask);
      samplemask = bld.vop2(aco_opcode::v_and_b32, bld.def(v1), samplemask, mask);

      ctx->arg_temps[ctx->args->sample_coverage.arg_index] = samplemask;
   }
}

Temp
get_interp_color(isel_context* ctx, int interp_vgpr, unsigned attr_index, unsigned comp)
{
   Builder bld(ctx->program, ctx->block);

   Temp dst = bld.tmp(v1);

   Temp prim_mask = get_arg(ctx, ctx->args->prim_mask);

   if (interp_vgpr != -1) {
      /* interp args are all 2 vgprs */
      int arg_index = ctx->args->persp_sample.arg_index + interp_vgpr / 2;
      Temp interp_ij = ctx->arg_temps[arg_index];

      emit_interp_instr(ctx, attr_index, comp, interp_ij, dst, prim_mask);
   } else {
      emit_interp_mov_instr(ctx, attr_index, comp, 0, dst, prim_mask);
   }

   return dst;
}

void
interpolate_color_args(isel_context* ctx, const struct aco_ps_prolog_info* finfo,
                       std::vector<Operand>& regs)
{
   if (!finfo->colors_read)
      return;

   Builder bld(ctx->program, ctx->block);

   unsigned vgpr = 256 + ctx->args->num_vgprs_used;

   if (finfo->color_two_side) {
      Temp face = get_arg(ctx, ctx->args->front_face);
      Temp is_face_positive =
         bld.vopc(aco_opcode::v_cmp_lg_u32, bld.def(bld.lm), Operand::zero(), face);

      u_foreach_bit (i, finfo->colors_read) {
         unsigned color_index = i / 4;
         unsigned front_index = finfo->color_attr_index[color_index];
         int interp_vgpr = finfo->color_interp_vgpr_index[color_index];

         /* If BCOLOR0 is used, BCOLOR1 is at offset "num_inputs + 1",
          * otherwise it's at offset "num_inputs".
          */
         unsigned back_index = finfo->num_interp_inputs;
         if (color_index == 1 && finfo->colors_read & 0xf)
            back_index++;

         Temp front = get_interp_color(ctx, interp_vgpr, front_index, i % 4);
         Temp back = get_interp_color(ctx, interp_vgpr, back_index, i % 4);

         Temp color =
            bld.vop2(aco_opcode::v_cndmask_b32, bld.def(v1), back, front, is_face_positive);

         regs.emplace_back(Operand(color, PhysReg{vgpr++}));
      }
   } else {
      u_foreach_bit (i, finfo->colors_read) {
         unsigned color_index = i / 4;
         unsigned attr_index = finfo->color_attr_index[color_index];
         int interp_vgpr = finfo->color_interp_vgpr_index[color_index];
         Temp color = get_interp_color(ctx, interp_vgpr, attr_index, i % 4);

         regs.emplace_back(Operand(color, PhysReg{vgpr++}));
      }
   }
}

void
emit_clamp_alpha_test(isel_context* ctx, const struct aco_ps_epilog_info* info, Temp colors[4],
                      unsigned color_index)
{
   Builder bld(ctx->program, ctx->block);

   if (info->clamp_color) {
      for (unsigned i = 0; i < 4; i++) {
         if (colors[i].regClass() == v2b) {
            colors[i] = bld.vop3(aco_opcode::v_med3_f16, bld.def(v2b), Operand::c16(0u),
                                 Operand::c16(0x3c00), colors[i]);
         } else {
            assert(colors[i].regClass() == v1);
            colors[i] = bld.vop3(aco_opcode::v_med3_f32, bld.def(v1), Operand::zero(),
                                 Operand::c32(0x3f800000u), colors[i]);
         }
      }
   }

   if (info->alpha_to_one) {
      if (colors[3].regClass() == v2b)
         colors[3] = bld.copy(bld.def(v2b), Operand::c16(0x3c00));
      else
         colors[3] = bld.copy(bld.def(v1), Operand::c32(0x3f800000u));
   }

   if (color_index == 0 && info->alpha_func != COMPARE_FUNC_ALWAYS) {
      Operand cond = Operand::c32(-1u);
      if (info->alpha_func != COMPARE_FUNC_NEVER) {
         aco_opcode opcode = aco_opcode::num_opcodes;

         switch (info->alpha_func) {
         case COMPARE_FUNC_LESS: opcode = aco_opcode::v_cmp_ngt_f32; break;
         case COMPARE_FUNC_EQUAL: opcode = aco_opcode::v_cmp_neq_f32; break;
         case COMPARE_FUNC_LEQUAL: opcode = aco_opcode::v_cmp_nge_f32; break;
         case COMPARE_FUNC_GREATER: opcode = aco_opcode::v_cmp_nlt_f32; break;
         case COMPARE_FUNC_NOTEQUAL: opcode = aco_opcode::v_cmp_nlg_f32; break;
         case COMPARE_FUNC_GEQUAL: opcode = aco_opcode::v_cmp_nle_f32; break;
         default: unreachable("invalid alpha func");
         }

         Temp ref = get_arg(ctx, info->alpha_reference);

         Temp alpha = colors[3].regClass() == v2b
                         ? bld.vop1(aco_opcode::v_cvt_f32_f16, bld.def(v1), colors[3])
                         : colors[3];

         /* true if not pass */
         cond = bld.vopc(opcode, bld.def(bld.lm), ref, alpha);
      }

      bld.pseudo(aco_opcode::p_discard_if, cond);
      ctx->block->kind |= block_kind_uses_discard;
      ctx->program->needs_exact = true;
   }
}

} /* end namespace */

void
select_program(Program* program, unsigned shader_count, struct nir_shader* const* shaders,
               ac_shader_config* config, const struct aco_compiler_options* options,
               const struct aco_shader_info* info, const struct ac_shader_args* args)
{
   isel_context ctx =
      setup_isel_context(program, shader_count, shaders, config, options, info, args);

   if (ctx.stage == raytracing_cs)
      return select_program_rt(ctx, shader_count, shaders, args);

   if (shader_count >= 2) {
      select_program_merged(ctx, shader_count, shaders);
   } else {
      bool need_barrier = false, check_merged_wave_info = false, endif_merged_wave_info = false;
      if_context ic_merged_wave_info;

      /* Handle separate compilation of VS+TCS and {VS,TES}+GS on GFX9+. */
      if (ctx.program->info.merged_shader_compiled_separately) {
         assert(ctx.program->gfx_level >= GFX9);
         if (ctx.stage.sw == SWStage::VS || ctx.stage.sw == SWStage::TES) {
            check_merged_wave_info = endif_merged_wave_info = true;
         } else {
            const bool ngg_gs =
               ctx.stage.hw == AC_HW_NEXT_GEN_GEOMETRY_SHADER && ctx.stage.sw == SWStage::GS;
            assert(ctx.stage == tess_control_hs || ctx.stage == geometry_gs || ngg_gs);
            check_merged_wave_info = endif_merged_wave_info = !ngg_gs;
            need_barrier = !ngg_gs;
         }
      }

      select_shader(ctx, shaders[0], true, true, need_barrier, &ic_merged_wave_info,
                    check_merged_wave_info, endif_merged_wave_info);
   }
}

void
select_trap_handler_shader(Program* program, struct nir_shader* shader, ac_shader_config* config,
                           const struct aco_compiler_options* options,
                           const struct aco_shader_info* info, const struct ac_shader_args* args)
{
   assert(options->gfx_level == GFX8);

   init_program(program, compute_cs, info, options->gfx_level, options->family, options->wgp_mode,
                config);

   isel_context ctx = {};
   ctx.program = program;
   ctx.args = args;
   ctx.options = options;
   ctx.stage = program->stage;

   ctx.block = ctx.program->create_and_insert_block();
   ctx.block->kind = block_kind_top_level;

   program->workgroup_size = 1; /* XXX */

   add_startpgm(&ctx);
   append_logical_start(ctx.block);

   Builder bld(ctx.program, ctx.block);

   /* Load the buffer descriptor from TMA. */
   bld.smem(aco_opcode::s_load_dwordx4, Definition(PhysReg{ttmp4}, s4), Operand(PhysReg{tma}, s2),
            Operand::zero());

   /* Store TTMP0-TTMP1. */
   bld.smem(aco_opcode::s_buffer_store_dwordx2, Operand(PhysReg{ttmp4}, s4), Operand::zero(),
            Operand(PhysReg{ttmp0}, s2), memory_sync_info(), true);

   uint32_t hw_regs_idx[] = {
      2, /* HW_REG_STATUS */
      3, /* HW_REG_TRAP_STS */
      4, /* HW_REG_HW_ID */
      7, /* HW_REG_IB_STS */
   };

   /* Store some hardware registers. */
   for (unsigned i = 0; i < ARRAY_SIZE(hw_regs_idx); i++) {
      /* "((size - 1) << 11) | register" */
      bld.sopk(aco_opcode::s_getreg_b32, Definition(PhysReg{ttmp8}, s1),
               ((20 - 1) << 11) | hw_regs_idx[i]);

      bld.smem(aco_opcode::s_buffer_store_dword, Operand(PhysReg{ttmp4}, s4),
               Operand::c32(8u + i * 4), Operand(PhysReg{ttmp8}, s1), memory_sync_info(), true);
   }

   program->config->float_mode = program->blocks[0].fp_mode.val;

   append_logical_end(ctx.block);
   ctx.block->kind |= block_kind_uniform;
   bld.sopp(aco_opcode::s_endpgm);

   finish_program(&ctx);
}

Operand
get_arg_fixed(const struct ac_shader_args* args, struct ac_arg arg)
{
   enum ac_arg_regfile file = args->args[arg.arg_index].file;
   unsigned size = args->args[arg.arg_index].size;
   RegClass rc = RegClass(file == AC_ARG_SGPR ? RegType::sgpr : RegType::vgpr, size);
   return Operand(get_arg_reg(args, arg), rc);
}

unsigned
load_vb_descs(Builder& bld, PhysReg dest, Operand base, unsigned start, unsigned max)
{
   unsigned count = MIN2((bld.program->dev.sgpr_limit - dest.reg()) / 4u, max);

   unsigned num_loads = (count / 4u) + util_bitcount(count & 0x3);
   if (bld.program->gfx_level >= GFX10 && num_loads > 1)
      bld.sopp(aco_opcode::s_clause, -1, num_loads - 1);

   for (unsigned i = 0; i < count;) {
      unsigned size = 1u << util_logbase2(MIN2(count - i, 4));

      if (size == 4)
         bld.smem(aco_opcode::s_load_dwordx16, Definition(dest, s16), base,
                  Operand::c32((start + i) * 16u));
      else if (size == 2)
         bld.smem(aco_opcode::s_load_dwordx8, Definition(dest, s8), base,
                  Operand::c32((start + i) * 16u));
      else
         bld.smem(aco_opcode::s_load_dwordx4, Definition(dest, s4), base,
                  Operand::c32((start + i) * 16u));

      dest = dest.advance(size * 16u);
      i += size;
   }

   return count;
}

Operand
calc_nontrivial_instance_id(Builder& bld, const struct ac_shader_args* args,
                            const struct aco_vs_prolog_info* pinfo, unsigned index,
                            Operand instance_id, Operand start_instance, PhysReg tmp_sgpr,
                            PhysReg tmp_vgpr0, PhysReg tmp_vgpr1)
{
   bld.smem(aco_opcode::s_load_dwordx2, Definition(tmp_sgpr, s2),
            get_arg_fixed(args, pinfo->inputs), Operand::c32(8u + index * 8u));

   wait_imm lgkm_imm;
   lgkm_imm.lgkm = 0;
   bld.sopp(aco_opcode::s_waitcnt, -1, lgkm_imm.pack(bld.program->gfx_level));

   Definition fetch_index_def(tmp_vgpr0, v1);
   Operand fetch_index(tmp_vgpr0, v1);

   Operand div_info(tmp_sgpr, s1);
   if (bld.program->gfx_level >= GFX8 && bld.program->gfx_level < GFX11) {
      /* use SDWA */
      if (bld.program->gfx_level < GFX9) {
         bld.vop1(aco_opcode::v_mov_b32, Definition(tmp_vgpr1, v1), div_info);
         div_info = Operand(tmp_vgpr1, v1);
      }

      bld.vop2(aco_opcode::v_lshrrev_b32, fetch_index_def, div_info, instance_id);

      Instruction* instr;
      if (bld.program->gfx_level >= GFX9)
         instr = bld.vop2_sdwa(aco_opcode::v_add_u32, fetch_index_def, div_info, fetch_index).instr;
      else
         instr = bld.vop2_sdwa(aco_opcode::v_add_co_u32, fetch_index_def, Definition(vcc, bld.lm),
                               div_info, fetch_index)
                    .instr;
      instr->sdwa().sel[0] = SubdwordSel::ubyte1;

      bld.vop3(aco_opcode::v_mul_hi_u32, fetch_index_def, Operand(tmp_sgpr.advance(4), s1),
               fetch_index);

      instr =
         bld.vop2_sdwa(aco_opcode::v_lshrrev_b32, fetch_index_def, div_info, fetch_index).instr;
      instr->sdwa().sel[0] = SubdwordSel::ubyte2;
   } else {
      Operand tmp_op(tmp_vgpr1, v1);
      Definition tmp_def(tmp_vgpr1, v1);

      bld.vop2(aco_opcode::v_lshrrev_b32, fetch_index_def, div_info, instance_id);

      bld.vop3(aco_opcode::v_bfe_u32, tmp_def, div_info, Operand::c32(8u), Operand::c32(8u));
      bld.vadd32(fetch_index_def, tmp_op, fetch_index, false, Operand(s2), true);

      bld.vop3(aco_opcode::v_mul_hi_u32, fetch_index_def, fetch_index,
               Operand(tmp_sgpr.advance(4), s1));

      bld.vop3(aco_opcode::v_bfe_u32, tmp_def, div_info, Operand::c32(16u), Operand::c32(8u));
      bld.vop2(aco_opcode::v_lshrrev_b32, fetch_index_def, tmp_op, fetch_index);
   }

   bld.vadd32(fetch_index_def, start_instance, fetch_index, false, Operand(s2), true);

   return fetch_index;
}

void
select_rt_prolog(Program* program, ac_shader_config* config,
                 const struct aco_compiler_options* options, const struct aco_shader_info* info,
                 const struct ac_shader_args* in_args, const struct ac_shader_args* out_args)
{
   init_program(program, compute_cs, info, options->gfx_level, options->family, options->wgp_mode,
                config);
   Block* block = program->create_and_insert_block();
   block->kind = block_kind_top_level;
   program->workgroup_size = info->workgroup_size;
   program->wave_size = info->workgroup_size;
   calc_min_waves(program);
   Builder bld(program, block);
   block->instructions.reserve(32);
   unsigned num_sgprs = MAX2(in_args->num_sgprs_used, out_args->num_sgprs_used);
   unsigned num_vgprs = MAX2(in_args->num_vgprs_used, out_args->num_vgprs_used);

   /* Inputs:
    * Ring offsets:                s[0-1]
    * Indirect descriptor sets:    s[2]
    * Push constants pointer:      s[3]
    * SBT descriptors:             s[4-5]
    * Traversal shader address:    s[6-7]
    * Ray launch size address:     s[8-9]
    * Dynamic callable stack base: s[10]
    * Workgroup IDs (xyz):         s[11], s[12], s[13]
    * Scratch offset:              s[14]
    * Local invocation IDs:        v[0-2]
    */
   PhysReg in_ring_offsets = get_arg_reg(in_args, in_args->ring_offsets);
   PhysReg in_sbt_desc = get_arg_reg(in_args, in_args->rt.sbt_descriptors);
   PhysReg in_launch_size_addr = get_arg_reg(in_args, in_args->rt.launch_size_addr);
   PhysReg in_stack_base = get_arg_reg(in_args, in_args->rt.dynamic_callable_stack_base);
   PhysReg in_wg_id_x = get_arg_reg(in_args, in_args->workgroup_ids[0]);
   PhysReg in_wg_id_y = get_arg_reg(in_args, in_args->workgroup_ids[1]);
   PhysReg in_wg_id_z = get_arg_reg(in_args, in_args->workgroup_ids[2]);
   PhysReg in_scratch_offset;
   if (options->gfx_level < GFX11)
      in_scratch_offset = get_arg_reg(in_args, in_args->scratch_offset);
   PhysReg in_local_ids[2] = {
      get_arg_reg(in_args, in_args->local_invocation_ids),
      get_arg_reg(in_args, in_args->local_invocation_ids).advance(4),
   };

   /* Outputs:
    * Callee shader PC:            s[0-1]
    * Indirect descriptor sets:    s[2]
    * Push constants pointer:      s[3]
    * SBT descriptors:             s[4-5]
    * Traversal shader address:    s[6-7]
    * Ray launch sizes (xyz):      s[8], s[9], s[10]
    * Scratch offset (<GFX9 only): s[11]
    * Ring offsets (<GFX9 only):   s[12-13]
    * Ray launch IDs:              v[0-2]
    * Stack pointer:               v[3]
    * Shader VA:                   v[4-5]
    * Shader Record Ptr:           v[6-7]
    */
   PhysReg out_uniform_shader_addr = get_arg_reg(out_args, out_args->rt.uniform_shader_addr);
   PhysReg out_launch_size_x = get_arg_reg(out_args, out_args->rt.launch_size);
   PhysReg out_launch_size_y = out_launch_size_x.advance(4);
   PhysReg out_launch_size_z = out_launch_size_y.advance(4);
   PhysReg out_launch_ids[3];
   for (unsigned i = 0; i < 3; i++)
      out_launch_ids[i] = get_arg_reg(out_args, out_args->rt.launch_id).advance(i * 4);
   PhysReg out_stack_ptr = get_arg_reg(out_args, out_args->rt.dynamic_callable_stack_base);
   PhysReg out_record_ptr = get_arg_reg(out_args, out_args->rt.shader_record);

   /* Temporaries: */
   num_sgprs = align(num_sgprs, 2);
   PhysReg tmp_raygen_sbt = PhysReg{num_sgprs};
   num_sgprs += 2;
   PhysReg tmp_ring_offsets = PhysReg{num_sgprs};
   num_sgprs += 2;

   PhysReg tmp_invocation_idx = PhysReg{256 + num_vgprs++};

   /* Confirm some assumptions about register aliasing */
   assert(in_ring_offsets == out_uniform_shader_addr);
   assert(get_arg_reg(in_args, in_args->push_constants) ==
          get_arg_reg(out_args, out_args->push_constants));
   assert(get_arg_reg(in_args, in_args->rt.sbt_descriptors) ==
          get_arg_reg(out_args, out_args->rt.sbt_descriptors));
   assert(in_launch_size_addr == out_launch_size_x);
   assert(in_stack_base == out_launch_size_z);
   assert(in_local_ids[0] == out_launch_ids[0]);

   /* load raygen sbt */
   bld.smem(aco_opcode::s_load_dwordx2, Definition(tmp_raygen_sbt, s2), Operand(in_sbt_desc, s2),
            Operand::c32(0u));

   /* init scratch */
   if (options->gfx_level < GFX9) {
      /* copy ring offsets to temporary location*/
      bld.sop1(aco_opcode::s_mov_b64, Definition(tmp_ring_offsets, s2),
               Operand(in_ring_offsets, s2));
   } else if (options->gfx_level < GFX11) {
      hw_init_scratch(bld, Definition(in_ring_offsets, s1), Operand(in_ring_offsets, s2),
                      Operand(in_scratch_offset, s1));
   }

   /* set stack ptr */
   bld.vop1(aco_opcode::v_mov_b32, Definition(out_stack_ptr, v1), Operand(in_stack_base, s1));

   /* load raygen address */
   bld.smem(aco_opcode::s_load_dwordx2, Definition(out_uniform_shader_addr, s2),
            Operand(tmp_raygen_sbt, s2), Operand::c32(0u));

   /* load ray launch sizes */
   bld.smem(aco_opcode::s_load_dword, Definition(out_launch_size_z, s1),
            Operand(in_launch_size_addr, s2), Operand::c32(8u));
   bld.smem(aco_opcode::s_load_dwordx2, Definition(out_launch_size_x, s2),
            Operand(in_launch_size_addr, s2), Operand::c32(0u));

   /* calculate ray launch ids */
   if (options->gfx_level >= GFX11) {
      /* Thread IDs are packed in VGPR0, 10 bits per component. */
      bld.vop3(aco_opcode::v_bfe_u32, Definition(in_local_ids[1], v1), Operand(in_local_ids[0], v1),
               Operand::c32(10u), Operand::c32(3u));
      bld.vop2(aco_opcode::v_and_b32, Definition(in_local_ids[0], v1), Operand::c32(0x7),
               Operand(in_local_ids[0], v1));
   }
   /* Do this backwards to reduce some RAW hazards on GFX11+ */
   bld.vop1(aco_opcode::v_mov_b32, Definition(out_launch_ids[2], v1), Operand(in_wg_id_z, s1));
   bld.vop3(aco_opcode::v_mad_u32_u24, Definition(out_launch_ids[1], v1), Operand(in_wg_id_y, s1),
            Operand::c32(program->workgroup_size == 32 ? 4 : 8), Operand(in_local_ids[1], v1));
   bld.vop3(aco_opcode::v_mad_u32_u24, Definition(out_launch_ids[0], v1), Operand(in_wg_id_x, s1),
            Operand::c32(8), Operand(in_local_ids[0], v1));

   if (options->gfx_level < GFX9) {
      /* write scratch/ring offsets to outputs, if needed */
      bld.sop1(aco_opcode::s_mov_b32,
               Definition(get_arg_reg(out_args, out_args->scratch_offset), s1),
               Operand(in_scratch_offset, s1));
      bld.sop1(aco_opcode::s_mov_b64, Definition(get_arg_reg(out_args, out_args->ring_offsets), s2),
               Operand(tmp_ring_offsets, s2));
   }

   /* calculate shader record ptr: SBT + RADV_RT_HANDLE_SIZE */
   if (options->gfx_level < GFX9) {
      bld.vop2_e64(aco_opcode::v_add_co_u32, Definition(out_record_ptr, v1), Definition(vcc, s2),
                   Operand(tmp_raygen_sbt, s1), Operand::c32(32u));
   } else {
      bld.vop2_e64(aco_opcode::v_add_u32, Definition(out_record_ptr, v1),
                   Operand(tmp_raygen_sbt, s1), Operand::c32(32u));
   }
   bld.vop1(aco_opcode::v_mov_b32, Definition(out_record_ptr.advance(4), v1),
            Operand(tmp_raygen_sbt.advance(4), s1));

   /* For 1D dispatches converted into 2D ones, we need to fix up the launch IDs.
    * Calculating the 1D launch ID is: id = local_invocation_index + (wg_id.x * wg_size).
    * in_wg_id_x now holds wg_id.x * wg_size.
    */
   bld.sop2(aco_opcode::s_lshl_b32, Definition(in_wg_id_x, s1), Definition(scc, s1),
            Operand(in_wg_id_x, s1), Operand::c32(program->workgroup_size == 32 ? 5 : 6));

   /* Calculate and add local_invocation_index */
   bld.vop3(aco_opcode::v_mbcnt_lo_u32_b32, Definition(tmp_invocation_idx, v1), Operand::c32(-1u),
            Operand(in_wg_id_x, s1));
   if (program->wave_size == 64) {
      if (program->gfx_level <= GFX7)
         bld.vop2(aco_opcode::v_mbcnt_hi_u32_b32, Definition(tmp_invocation_idx, v1),
                  Operand::c32(-1u), Operand(tmp_invocation_idx, v1));
      else
         bld.vop3(aco_opcode::v_mbcnt_hi_u32_b32_e64, Definition(tmp_invocation_idx, v1),
                  Operand::c32(-1u), Operand(tmp_invocation_idx, v1));
   }

   /* Make fixup operations a no-op if this is not a converted 2D dispatch. */
   bld.sopc(aco_opcode::s_cmp_lg_u32, Definition(scc, s1),
            Operand::c32(ACO_RT_CONVERTED_2D_LAUNCH_SIZE), Operand(out_launch_size_y, s1));
   bld.sop2(Builder::s_cselect, Definition(vcc, bld.lm),
            Operand::c32_or_c64(-1u, program->wave_size == 64),
            Operand::c32_or_c64(0, program->wave_size == 64), Operand(scc, s1));
   bld.vop2(aco_opcode::v_cndmask_b32, Definition(out_launch_ids[0], v1),
            Operand(tmp_invocation_idx, v1), Operand(out_launch_ids[0], v1), Operand(vcc, bld.lm));
   bld.vop2(aco_opcode::v_cndmask_b32, Definition(out_launch_ids[1], v1), Operand::zero(),
            Operand(out_launch_ids[1], v1), Operand(vcc, bld.lm));

   /* jump to raygen */
   bld.sop1(aco_opcode::s_setpc_b64, Operand(out_uniform_shader_addr, s2));

   program->config->float_mode = program->blocks[0].fp_mode.val;
   program->config->num_vgprs = get_vgpr_alloc(program, num_vgprs);
   program->config->num_sgprs = get_sgpr_alloc(program, num_sgprs);
}

void
select_vs_prolog(Program* program, const struct aco_vs_prolog_info* pinfo, ac_shader_config* config,
                 const struct aco_compiler_options* options, const struct aco_shader_info* info,
                 const struct ac_shader_args* args)
{
   assert(pinfo->num_attributes > 0);

   /* This should be enough for any shader/stage. */
   unsigned max_user_sgprs = options->gfx_level >= GFX9 ? 32 : 16;

   init_program(program, compute_cs, info, options->gfx_level, options->family, options->wgp_mode,
                config);
   program->dev.vgpr_limit = 256;

   Block* block = program->create_and_insert_block();
   block->kind = block_kind_top_level;

   program->workgroup_size = 64;
   calc_min_waves(program);

   Builder bld(program, block);

   block->instructions.reserve(16 + pinfo->num_attributes * 4);

   bld.sopp(aco_opcode::s_setprio, -1u, 0x3u);

   uint32_t attrib_mask = BITFIELD_MASK(pinfo->num_attributes);
   bool has_nontrivial_divisors = pinfo->nontrivial_divisors;

   wait_imm lgkm_imm;
   lgkm_imm.lgkm = 0;

   /* choose sgprs */
   PhysReg vertex_buffers(align(max_user_sgprs + 14, 2));
   PhysReg prolog_input = vertex_buffers.advance(8);
   PhysReg desc(
      align((has_nontrivial_divisors ? prolog_input : vertex_buffers).advance(8).reg(), 4));

   Operand start_instance = get_arg_fixed(args, args->start_instance);
   Operand instance_id = get_arg_fixed(args, args->instance_id);

   PhysReg attributes_start(256 + args->num_vgprs_used);
   /* choose vgprs that won't be used for anything else until the last attribute load */
   PhysReg vertex_index(attributes_start.reg() + pinfo->num_attributes * 4 - 1);
   PhysReg instance_index(attributes_start.reg() + pinfo->num_attributes * 4 - 2);
   PhysReg start_instance_vgpr(attributes_start.reg() + pinfo->num_attributes * 4 - 3);
   PhysReg nontrivial_tmp_vgpr0(attributes_start.reg() + pinfo->num_attributes * 4 - 4);
   PhysReg nontrivial_tmp_vgpr1(attributes_start.reg() + pinfo->num_attributes * 4);

   bld.sop1(aco_opcode::s_mov_b32, Definition(vertex_buffers, s1),
            get_arg_fixed(args, args->vertex_buffers));
   if (options->address32_hi >= 0xffff8000 || options->address32_hi <= 0x7fff) {
      bld.sopk(aco_opcode::s_movk_i32, Definition(vertex_buffers.advance(4), s1),
               options->address32_hi & 0xFFFF);
   } else {
      bld.sop1(aco_opcode::s_mov_b32, Definition(vertex_buffers.advance(4), s1),
               Operand::c32((unsigned)options->address32_hi));
   }

   /* calculate vgpr requirements */
   unsigned num_vgprs = attributes_start.reg() - 256;
   num_vgprs += pinfo->num_attributes * 4;
   if (has_nontrivial_divisors && program->gfx_level <= GFX8)
      num_vgprs++; /* make space for nontrivial_tmp_vgpr1 */
   unsigned num_sgprs = 0;

   const struct ac_vtx_format_info* vtx_info_table =
      ac_get_vtx_format_info_table(GFX8, CHIP_POLARIS10);

   for (unsigned loc = 0; loc < pinfo->num_attributes;) {
      unsigned num_descs =
         load_vb_descs(bld, desc, Operand(vertex_buffers, s2), loc, pinfo->num_attributes - loc);
      num_sgprs = MAX2(num_sgprs, desc.advance(num_descs * 16u).reg());

      if (loc == 0) {
         /* perform setup while we load the descriptors */
         if (pinfo->is_ngg || pinfo->next_stage != MESA_SHADER_VERTEX) {
            Operand count = get_arg_fixed(args, args->merged_wave_info);
            bld.sop2(aco_opcode::s_bfm_b64, Definition(exec, s2), count, Operand::c32(0u));
            if (program->wave_size == 64) {
               bld.sopc(aco_opcode::s_bitcmp1_b32, Definition(scc, s1), count,
                        Operand::c32(6u /* log2(64) */));
               bld.sop2(aco_opcode::s_cselect_b64, Definition(exec, s2), Operand::c64(UINT64_MAX),
                        Operand(exec, s2), Operand(scc, s1));
            }
         }

         /* If there are no HS threads, SPI mistakenly loads the LS VGPRs starting at VGPR 0. */
         if (info->hw_stage == AC_HW_HULL_SHADER && options->has_ls_vgpr_init_bug) {
            /* We don't want load_vb_descs() to write vcc. */
            assert(program->dev.sgpr_limit <= vcc.reg());

            bld.sop2(aco_opcode::s_bfe_u32, Definition(vcc, s1), Definition(scc, s1),
                     get_arg_fixed(args, args->merged_wave_info), Operand::c32((8u << 16) | 8u));
            bld.sop2(Builder::s_cselect, Definition(vcc, bld.lm), Operand::c32(-1), Operand::zero(),
                     Operand(scc, s1));

            /* These copies are ordered so that vertex_id=tcs_patch_id doesn't overwrite vertex_id
             * before instance_id=vertex_id. */
            ac_arg src_args[] = {args->vertex_id, args->tcs_rel_ids, args->tcs_patch_id};
            ac_arg dst_args[] = {args->instance_id, args->vs_rel_patch_id, args->vertex_id};
            for (unsigned i = 0; i < 3; i++) {
               bld.vop2(aco_opcode::v_cndmask_b32, Definition(get_arg_reg(args, dst_args[i]), v1),
                        get_arg_fixed(args, src_args[i]), get_arg_fixed(args, dst_args[i]),
                        Operand(vcc, bld.lm));
            }
         }

         bool needs_instance_index =
            pinfo->instance_rate_inputs &
            ~(pinfo->zero_divisors | pinfo->nontrivial_divisors); /* divisor is 1 */
         bool needs_start_instance = pinfo->instance_rate_inputs & pinfo->zero_divisors;
         bool needs_vertex_index = ~pinfo->instance_rate_inputs & attrib_mask;
         if (needs_vertex_index)
            bld.vadd32(Definition(vertex_index, v1), get_arg_fixed(args, args->base_vertex),
                       get_arg_fixed(args, args->vertex_id), false, Operand(s2), true);
         if (needs_instance_index)
            bld.vadd32(Definition(instance_index, v1), start_instance, instance_id, false,
                       Operand(s2), true);
         if (needs_start_instance)
            bld.vop1(aco_opcode::v_mov_b32, Definition(start_instance_vgpr, v1), start_instance);
      }

      bld.sopp(aco_opcode::s_waitcnt, -1, lgkm_imm.pack(program->gfx_level));

      for (unsigned i = 0; i < num_descs;) {
         PhysReg dest(attributes_start.reg() + loc * 4u);

         /* calculate index */
         Operand fetch_index = Operand(vertex_index, v1);
         if (pinfo->instance_rate_inputs & (1u << loc)) {
            if (!(pinfo->zero_divisors & (1u << loc))) {
               fetch_index = instance_id;
               if (pinfo->nontrivial_divisors & (1u << loc)) {
                  unsigned index = util_bitcount(pinfo->nontrivial_divisors & BITFIELD_MASK(loc));
                  fetch_index = calc_nontrivial_instance_id(
                     bld, args, pinfo, index, instance_id, start_instance, prolog_input,
                     nontrivial_tmp_vgpr0, nontrivial_tmp_vgpr1);
               } else {
                  fetch_index = Operand(instance_index, v1);
               }
            } else {
               fetch_index = Operand(start_instance_vgpr, v1);
            }
         }

         /* perform load */
         PhysReg cur_desc = desc.advance(i * 16);
         if ((pinfo->misaligned_mask & (1u << loc))) {
            const struct ac_vtx_format_info* vtx_info = &vtx_info_table[pinfo->formats[loc]];

            assert(vtx_info->has_hw_format & 0x1);
            unsigned dfmt = vtx_info->hw_format[0] & 0xf;
            unsigned nfmt = vtx_info->hw_format[0] >> 4;

            for (unsigned j = 0; j < vtx_info->num_channels; j++) {
               bool post_shuffle = pinfo->post_shuffle & (1u << loc);
               unsigned offset = vtx_info->chan_byte_size * (post_shuffle && j < 3 ? 2 - j : j);

               /* Use MUBUF to workaround hangs for byte-aligned dword loads. The Vulkan spec
                * doesn't require this to work, but some GL CTS tests over Zink do this anyway.
                * MTBUF can hang, but MUBUF doesn't (probably gives garbage, but GL CTS doesn't
                * care).
                */
               if (dfmt == V_008F0C_BUF_DATA_FORMAT_32)
                  bld.mubuf(aco_opcode::buffer_load_dword, Definition(dest.advance(j * 4u), v1),
                            Operand(cur_desc, s4), fetch_index, Operand::c32(0u), offset, false,
                            false, true);
               else if (vtx_info->chan_byte_size == 8)
                  bld.mtbuf(aco_opcode::tbuffer_load_format_xy,
                            Definition(dest.advance(j * 8u), v2), Operand(cur_desc, s4),
                            fetch_index, Operand::c32(0u), dfmt, nfmt, offset, false, true);
               else
                  bld.mtbuf(aco_opcode::tbuffer_load_format_x, Definition(dest.advance(j * 4u), v1),
                            Operand(cur_desc, s4), fetch_index, Operand::c32(0u), dfmt, nfmt,
                            offset, false, true);
            }
            uint32_t one =
               nfmt == V_008F0C_BUF_NUM_FORMAT_UINT || nfmt == V_008F0C_BUF_NUM_FORMAT_SINT
                  ? 1u
                  : 0x3f800000u;
            /* 22.1.1. Attribute Location and Component Assignment of Vulkan 1.3 specification:
             * For 64-bit data types, no default attribute values are provided. Input variables must
             * not use more components than provided by the attribute.
             */
            for (unsigned j = vtx_info->num_channels; vtx_info->chan_byte_size != 8 && j < 4; j++) {
               bld.vop1(aco_opcode::v_mov_b32, Definition(dest.advance(j * 4u), v1),
                        Operand::c32(j == 3 ? one : 0u));
            }

            unsigned slots = vtx_info->chan_byte_size == 8 && vtx_info->num_channels > 2 ? 2 : 1;
            loc += slots;
            i += slots;
         } else {
            bld.mubuf(aco_opcode::buffer_load_format_xyzw, Definition(dest, v4),
                      Operand(cur_desc, s4), fetch_index, Operand::c32(0u), 0u, false, false, true);
            loc++;
            i++;
         }
      }
   }

   if (pinfo->alpha_adjust_lo | pinfo->alpha_adjust_hi) {
      wait_imm vm_imm;
      vm_imm.vm = 0;
      bld.sopp(aco_opcode::s_waitcnt, -1, vm_imm.pack(program->gfx_level));
   }

   /* For 2_10_10_10 formats the alpha is handled as unsigned by pre-vega HW.
    * so we may need to fix it up. */
   u_foreach_bit (loc, (pinfo->alpha_adjust_lo | pinfo->alpha_adjust_hi)) {
      PhysReg alpha(attributes_start.reg() + loc * 4u + 3);

      unsigned alpha_adjust = (pinfo->alpha_adjust_lo >> loc) & 0x1;
      alpha_adjust |= ((pinfo->alpha_adjust_hi >> loc) & 0x1) << 1;

      if (alpha_adjust == AC_ALPHA_ADJUST_SSCALED)
         bld.vop1(aco_opcode::v_cvt_u32_f32, Definition(alpha, v1), Operand(alpha, v1));

      /* For the integer-like cases, do a natural sign extension.
       *
       * For the SNORM case, the values are 0.0, 0.333, 0.666, 1.0
       * and happen to contain 0, 1, 2, 3 as the two LSBs of the
       * exponent.
       */
      unsigned offset = alpha_adjust == AC_ALPHA_ADJUST_SNORM ? 23u : 0u;
      bld.vop3(aco_opcode::v_bfe_i32, Definition(alpha, v1), Operand(alpha, v1),
               Operand::c32(offset), Operand::c32(2u));

      /* Convert back to the right type. */
      if (alpha_adjust == AC_ALPHA_ADJUST_SNORM) {
         bld.vop1(aco_opcode::v_cvt_f32_i32, Definition(alpha, v1), Operand(alpha, v1));
         bld.vop2(aco_opcode::v_max_f32, Definition(alpha, v1), Operand::c32(0xbf800000u),
                  Operand(alpha, v1));
      } else if (alpha_adjust == AC_ALPHA_ADJUST_SSCALED) {
         bld.vop1(aco_opcode::v_cvt_f32_i32, Definition(alpha, v1), Operand(alpha, v1));
      }
   }

   block->kind |= block_kind_uniform;

   /* continue on to the main shader */
   Operand continue_pc = get_arg_fixed(args, pinfo->inputs);
   if (has_nontrivial_divisors) {
      bld.smem(aco_opcode::s_load_dwordx2, Definition(prolog_input, s2),
               get_arg_fixed(args, pinfo->inputs), Operand::c32(0u));
      bld.sopp(aco_opcode::s_waitcnt, -1, lgkm_imm.pack(program->gfx_level));
      continue_pc = Operand(prolog_input, s2);
   }

   bld.sop1(aco_opcode::s_setpc_b64, continue_pc);

   program->config->float_mode = program->blocks[0].fp_mode.val;
   /* addition on GFX6-8 requires a carry-out (we use VCC) */
   program->needs_vcc = program->gfx_level <= GFX8;
   program->config->num_vgprs = std::min<uint16_t>(get_vgpr_alloc(program, num_vgprs), 256);
   program->config->num_sgprs = get_sgpr_alloc(program, num_sgprs);
}

void
select_ps_epilog(Program* program, void* pinfo, ac_shader_config* config,
                 const struct aco_compiler_options* options, const struct aco_shader_info* info,
                 const struct ac_shader_args* args)
{
   const struct aco_ps_epilog_info* einfo = (const struct aco_ps_epilog_info*)pinfo;
   isel_context ctx =
      setup_isel_context(program, 0, NULL, config, options, info, args, SWStage::FS);

   ctx.block->fp_mode = program->next_fp_mode;

   add_startpgm(&ctx);
   append_logical_start(ctx.block);

   Builder bld(ctx.program, ctx.block);

   Temp colors[MAX_DRAW_BUFFERS][4];
   for (unsigned i = 0; i < MAX_DRAW_BUFFERS; i++) {
      if (!einfo->colors[i].used)
         continue;

      Temp color = get_arg(&ctx, einfo->colors[i]);
      unsigned col_types = (einfo->color_types >> (i * 2)) & 0x3;

      emit_split_vector(&ctx, color, col_types == ACO_TYPE_ANY32 ? 4 : 8);
      for (unsigned c = 0; c < 4; ++c) {
         colors[i][c] = emit_extract_vector(&ctx, color, c, col_types == ACO_TYPE_ANY32 ? v1 : v2b);
      }

      emit_clamp_alpha_test(&ctx, einfo, colors[i], i);
   }

   bool has_mrtz_depth = einfo->depth.used;
   bool has_mrtz_stencil = einfo->stencil.used;
   bool has_mrtz_samplemask = einfo->samplemask.used;
   bool has_mrtz_alpha = einfo->alpha_to_coverage_via_mrtz && einfo->colors[0].used;
   bool has_mrtz_export =
      has_mrtz_depth || has_mrtz_stencil || has_mrtz_samplemask || has_mrtz_alpha;
   if (has_mrtz_export) {
      Temp depth = has_mrtz_depth ? get_arg(&ctx, einfo->depth) : Temp();
      Temp stencil = has_mrtz_stencil ? get_arg(&ctx, einfo->stencil) : Temp();
      Temp samplemask = has_mrtz_samplemask ? get_arg(&ctx, einfo->samplemask) : Temp();
      Temp alpha = has_mrtz_alpha ? colors[0][3] : Temp();

      export_fs_mrtz(&ctx, depth, stencil, samplemask, alpha);
   }

   /* Export all color render targets */
   struct aco_export_mrt mrts[MAX_DRAW_BUFFERS];
   unsigned mrt_num = 0;

   if (einfo->broadcast_last_cbuf) {
      for (unsigned i = 0; i <= einfo->broadcast_last_cbuf; i++) {
         struct aco_export_mrt* mrt = &mrts[mrt_num];
         if (export_fs_mrt_color(&ctx, einfo, colors[0], i, mrt))
            mrt->target += mrt_num++;
      }
   } else {
      for (unsigned i = 0; i < MAX_DRAW_BUFFERS; i++) {
         struct aco_export_mrt* mrt = &mrts[mrt_num];
         if (export_fs_mrt_color(&ctx, einfo, colors[i], i, mrt))
            mrt->target += mrt_num++;
      }
   }

   if (mrt_num) {
      if (ctx.options->gfx_level >= GFX11 && einfo->mrt0_is_dual_src) {
         assert(mrt_num == 2);
         create_fs_dual_src_export_gfx11(&ctx, &mrts[0], &mrts[1]);
      } else {
         for (unsigned i = 0; i < mrt_num; i++)
            export_mrt(&ctx, &mrts[i]);
      }
   } else if (!has_mrtz_export && !einfo->skip_null_export) {
      create_fs_null_export(&ctx);
   }

   program->config->float_mode = program->blocks[0].fp_mode.val;

   append_logical_end(ctx.block);
   ctx.block->kind |= block_kind_export_end;
   bld.reset(ctx.block);
   bld.sopp(aco_opcode::s_endpgm);

   finish_program(&ctx);
}

void
select_tcs_epilog(Program* program, void* pinfo, ac_shader_config* config,
                  const struct aco_compiler_options* options, const struct aco_shader_info* info,
                  const struct ac_shader_args* args)
{
   const struct aco_tcs_epilog_info* einfo = (const struct aco_tcs_epilog_info*)pinfo;
   isel_context ctx =
      setup_isel_context(program, 0, NULL, config, options, info, args, SWStage::TCS);

   ctx.block->fp_mode = program->next_fp_mode;

   add_startpgm(&ctx);
   append_logical_start(ctx.block);

   Builder bld(ctx.program, ctx.block);

   /* Add a barrier before loading tess factors from LDS. */
   if (!einfo->pass_tessfactors_by_reg) {
      /* To generate s_waitcnt lgkmcnt(0) when waitcnt insertion. */
      program->pending_lds_access = true;

      sync_scope scope = einfo->tcs_out_patch_fits_subgroup ? scope_subgroup : scope_workgroup;
      bld.barrier(aco_opcode::p_barrier, memory_sync_info(storage_shared, semantic_acqrel, scope),
                  scope);
   }

   Temp invocation_id = get_arg(&ctx, einfo->invocation_id);

   Temp cond = bld.vopc(aco_opcode::v_cmp_eq_u32, bld.def(bld.lm), Operand::zero(), invocation_id);

   if_context ic_invoc_0;
   begin_divergent_if_then(&ctx, &ic_invoc_0, cond);

   int outer_comps, inner_comps;
   switch (einfo->primitive_mode) {
   case TESS_PRIMITIVE_ISOLINES:
      outer_comps = 2;
      inner_comps = 0;
      break;
   case TESS_PRIMITIVE_TRIANGLES:
      outer_comps = 3;
      inner_comps = 1;
      break;
   case TESS_PRIMITIVE_QUADS:
      outer_comps = 4;
      inner_comps = 2;
      break;
   default: unreachable("invalid primitive mode"); return;
   }

   bld.reset(ctx.block);

   unsigned tess_lvl_out_loc =
      ac_shader_io_get_unique_index_patch(VARYING_SLOT_TESS_LEVEL_OUTER) * 16;
   unsigned tess_lvl_in_loc =
      ac_shader_io_get_unique_index_patch(VARYING_SLOT_TESS_LEVEL_INNER) * 16;

   Temp outer[4];
   Temp inner[2];
   if (einfo->pass_tessfactors_by_reg) {
      for (int i = 0; i < outer_comps; i++)
         outer[i] = get_arg(&ctx, einfo->tess_lvl_out[i]);

      for (int i = 0; i < inner_comps; i++)
         inner[i] = get_arg(&ctx, einfo->tess_lvl_in[i]);
   } else {
      Temp addr = get_arg(&ctx, einfo->tcs_out_current_patch_data_offset);
      addr = bld.vop2(aco_opcode::v_lshlrev_b32, bld.def(v1), Operand::c32(2), addr);

      Temp data = program->allocateTmp(RegClass(RegType::vgpr, outer_comps));
      load_lds(&ctx, 4, outer_comps, data, addr, tess_lvl_out_loc, 4);
      for (int i = 0; i < outer_comps; i++)
         outer[i] = emit_extract_vector(&ctx, data, i, v1);

      if (inner_comps) {
         data = program->allocateTmp(RegClass(RegType::vgpr, inner_comps));
         load_lds(&ctx, 4, inner_comps, data, addr, tess_lvl_in_loc, 4);
         for (int i = 0; i < inner_comps; i++)
            inner[i] = emit_extract_vector(&ctx, data, i, v1);
      }
   }

   Temp tess_factor_ring_desc = get_tess_ring_descriptor(&ctx, einfo, true);
   Temp tess_factor_ring_base = get_arg(&ctx, args->tcs_factor_offset);
   Temp rel_patch_id = get_arg(&ctx, einfo->rel_patch_id);
   unsigned tess_factor_ring_const_offset = 0;

   if (program->gfx_level <= GFX8) {
      /* Store the dynamic HS control word. */
      cond = bld.vopc(aco_opcode::v_cmp_eq_u32, bld.def(bld.lm), Operand::zero(), rel_patch_id);

      if_context ic_patch_0;
      begin_divergent_if_then(&ctx, &ic_patch_0, cond);

      bld.reset(ctx.block);

      Temp data = bld.copy(bld.def(v1), Operand::c32(0x80000000u));

      emit_single_mubuf_store(&ctx, tess_factor_ring_desc, Temp(0, v1), tess_factor_ring_base,
                              Temp(), data, 0, memory_sync_info(), true, false, false);

      tess_factor_ring_const_offset += 4;

      begin_divergent_if_else(&ctx, &ic_patch_0);
      end_divergent_if(&ctx, &ic_patch_0);
   }

   bld.reset(ctx.block);

   Temp tess_factor_ring_offset =
      bld.v_mul_imm(bld.def(v1), rel_patch_id, (inner_comps + outer_comps) * 4, false);

   switch (einfo->primitive_mode) {
   case TESS_PRIMITIVE_ISOLINES: {
      /* For isolines, the hardware expects tess factors in the reverse order. */
      Temp data = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), outer[1], outer[0]);
      emit_single_mubuf_store(&ctx, tess_factor_ring_desc, tess_factor_ring_offset,
                              tess_factor_ring_base, Temp(), data, tess_factor_ring_const_offset,
                              memory_sync_info(), true, false, false);
      break;
   }
   case TESS_PRIMITIVE_TRIANGLES: {
      Temp data = bld.pseudo(aco_opcode::p_create_vector, bld.def(v4), outer[0], outer[1], outer[2],
                             inner[0]);
      emit_single_mubuf_store(&ctx, tess_factor_ring_desc, tess_factor_ring_offset,
                              tess_factor_ring_base, Temp(), data, tess_factor_ring_const_offset,
                              memory_sync_info(), true, false, false);
      break;
   }
   case TESS_PRIMITIVE_QUADS: {
      Temp data = bld.pseudo(aco_opcode::p_create_vector, bld.def(v4), outer[0], outer[1], outer[2],
                             outer[3]);
      emit_single_mubuf_store(&ctx, tess_factor_ring_desc, tess_factor_ring_offset,
                              tess_factor_ring_base, Temp(), data, tess_factor_ring_const_offset,
                              memory_sync_info(), true, false, false);

      data = bld.pseudo(aco_opcode::p_create_vector, bld.def(v2), inner[0], inner[1]);
      emit_single_mubuf_store(
         &ctx, tess_factor_ring_desc, tess_factor_ring_offset, tess_factor_ring_base, Temp(), data,
         tess_factor_ring_const_offset + 16, memory_sync_info(), true, false, false);
      break;
   }
   default: unreachable("invalid primitive mode"); break;
   }

   if (einfo->tes_reads_tessfactors) {
      Temp layout = get_arg(&ctx, einfo->tcs_offchip_layout);
      Temp num_patches, patch_base;

      if (ctx.options->is_opengl) {
         num_patches = bld.sop2(aco_opcode::s_and_b32, bld.def(s1), bld.def(s1, scc), layout,
                                Operand::c32(0x3f));
         num_patches = bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.def(s1, scc), num_patches,
                                Operand::c32(1));

         patch_base = bld.sop2(aco_opcode::s_lshr_b32, bld.def(s1), bld.def(s1, scc), layout,
                               Operand::c32(16));
      } else {
         num_patches = bld.sop2(aco_opcode::s_bfe_u32, bld.def(s1), bld.def(s1, scc), layout,
                                Operand::c32(0x60006));

         patch_base = get_arg(&ctx, einfo->patch_base);
      }

      Temp tess_ring_desc = get_tess_ring_descriptor(&ctx, einfo, false);
      Temp tess_ring_base = get_arg(&ctx, args->tess_offchip_offset);

      Temp sbase =
         bld.sop2(aco_opcode::s_add_u32, bld.def(s1), bld.def(s1, scc), tess_ring_base, patch_base);

      Temp voffset =
         bld.vop2(aco_opcode::v_lshlrev_b32, bld.def(v1), Operand::c32(4), rel_patch_id);

      store_tess_factor_to_tess_ring(&ctx, tess_ring_desc, outer, outer_comps, sbase, voffset,
                                     num_patches, tess_lvl_out_loc);

      if (inner_comps) {
         store_tess_factor_to_tess_ring(&ctx, tess_ring_desc, inner, inner_comps, sbase, voffset,
                                        num_patches, tess_lvl_in_loc);
      }
   }

   begin_divergent_if_else(&ctx, &ic_invoc_0);
   end_divergent_if(&ctx, &ic_invoc_0);

   program->config->float_mode = program->blocks[0].fp_mode.val;

   append_logical_end(ctx.block);

   bld.reset(ctx.block);
   bld.sopp(aco_opcode::s_endpgm);

   finish_program(&ctx);
}

void
select_gl_vs_prolog(Program* program, void* pinfo, ac_shader_config* config,
                    const struct aco_compiler_options* options, const struct aco_shader_info* info,
                    const struct ac_shader_args* args)
{
   const struct aco_gl_vs_prolog_info* vinfo = (const struct aco_gl_vs_prolog_info*)pinfo;
   isel_context ctx =
      setup_isel_context(program, 0, NULL, config, options, info, args, SWStage::VS);

   ctx.block->fp_mode = program->next_fp_mode;

   add_startpgm(&ctx);
   append_logical_start(ctx.block);

   Builder bld(ctx.program, ctx.block);

   bld.sopp(aco_opcode::s_setprio, -1u, 0x3u);

   if (vinfo->as_ls && options->has_ls_vgpr_init_bug)
      fix_ls_vgpr_init_bug(&ctx);

   std::vector<Operand> regs;
   passthrough_all_args(&ctx, regs);

   Temp instance_divisor_constbuf;

   if (vinfo->instance_divisor_is_fetched) {
      Temp list = get_arg(&ctx, vinfo->internal_bindings);
      list = convert_pointer_to_64_bit(&ctx, list);

      instance_divisor_constbuf = bld.smem(aco_opcode::s_load_dwordx4, bld.def(s4), list,
                                           Operand::c32(vinfo->instance_diviser_buf_offset));
   }

   unsigned vgpr = 256 + ctx.args->num_vgprs_used;

   for (unsigned i = 0; i < vinfo->num_inputs; i++) {
      Temp index = get_gl_vs_prolog_vertex_index(&ctx, vinfo, i, instance_divisor_constbuf);
      regs.emplace_back(Operand(index, PhysReg{vgpr + i}));
   }

   program->config->float_mode = program->blocks[0].fp_mode.val;

   append_logical_end(ctx.block);

   build_end_with_regs(&ctx, regs);

   finish_program(&ctx);
}

void
select_ps_prolog(Program* program, void* pinfo, ac_shader_config* config,
                 const struct aco_compiler_options* options, const struct aco_shader_info* info,
                 const struct ac_shader_args* args)
{
   const struct aco_ps_prolog_info* finfo = (const struct aco_ps_prolog_info*)pinfo;
   isel_context ctx =
      setup_isel_context(program, 0, NULL, config, options, info, args, SWStage::FS);

   ctx.block->fp_mode = program->next_fp_mode;

   add_startpgm(&ctx);
   append_logical_start(ctx.block);

   if (finfo->poly_stipple)
      emit_polygon_stipple(&ctx, finfo);

   overwrite_interp_args(&ctx, finfo);

   overwrite_samplemask_arg(&ctx, finfo);

   std::vector<Operand> regs;
   passthrough_all_args(&ctx, regs);

   interpolate_color_args(&ctx, finfo, regs);

   program->config->float_mode = program->blocks[0].fp_mode.val;

   append_logical_end(ctx.block);

   build_end_with_regs(&ctx, regs);

   /* To compute all end args in WQM mode if required by main part. */
   if (finfo->needs_wqm)
      set_wqm(&ctx, true);

   /* Exit WQM mode finally. */
   program->needs_exact = true;

   finish_program(&ctx);
}

} // namespace aco
