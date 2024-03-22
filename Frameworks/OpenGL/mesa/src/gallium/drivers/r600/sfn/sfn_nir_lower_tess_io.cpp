/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2022 Collabora LTD
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

#include "sfn_nir.h"

bool
r600_lower_tess_io_filter(const nir_instr *instr, gl_shader_stage stage)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *op = nir_instr_as_intrinsic(instr);
   switch (op->intrinsic) {
   case nir_intrinsic_load_input:
      return stage == MESA_SHADER_TESS_CTRL || stage == MESA_SHADER_TESS_EVAL;
   case nir_intrinsic_load_output:
   case nir_intrinsic_load_per_vertex_input:
   case nir_intrinsic_load_per_vertex_output:
   case nir_intrinsic_store_per_vertex_output:
   case nir_intrinsic_load_patch_vertices_in:
   case nir_intrinsic_load_tess_level_outer:
   case nir_intrinsic_load_tess_level_inner:
      return true;
   case nir_intrinsic_store_output:
      return stage == MESA_SHADER_TESS_CTRL || stage == MESA_SHADER_VERTEX;
   default:;
   }
   return false;
}

static int
get_tcs_varying_offset(nir_intrinsic_instr *op)
{
   unsigned location = nir_intrinsic_io_semantics(op).location;

   switch (location) {
   case VARYING_SLOT_POS:
      return 0;
   case VARYING_SLOT_PSIZ:
      return 0x10;
   case VARYING_SLOT_CLIP_DIST0:
      return 0x20;
   case VARYING_SLOT_CLIP_DIST1:
      return 0x30;
   case VARYING_SLOT_COL0:
      return 0x40;
   case VARYING_SLOT_COL1:
      return 0x50;
   case VARYING_SLOT_BFC0:
      return 0x60;
   case VARYING_SLOT_BFC1:
      return 0x70;
   case VARYING_SLOT_CLIP_VERTEX:
      return 0x80;
   case VARYING_SLOT_TESS_LEVEL_OUTER:
      return 0;
   case VARYING_SLOT_TESS_LEVEL_INNER:
      return 0x10;
   default:
      if (location >= VARYING_SLOT_VAR0 && location <= VARYING_SLOT_VAR31)
         return 0x10 * (location - VARYING_SLOT_VAR0) + 0x90;

      if (location >= VARYING_SLOT_PATCH0) {
         return 0x10 * (location - VARYING_SLOT_PATCH0) + 0x20;
      }
   }
   return 0;
}

static inline nir_def *
r600_tcs_base_address(nir_builder *b, nir_def *param_base, nir_def *rel_patch_id)
{
   return nir_umad24(b,
                     nir_channel(b, param_base, 0),
                     rel_patch_id,
                     nir_channel(b, param_base, 3));
}

static nir_def *
emil_lsd_in_addr(nir_builder *b,
                 nir_def *base,
                 nir_def *patch_id,
                 nir_intrinsic_instr *op)
{
   nir_def *addr =
      nir_build_alu(b, nir_op_umul24, nir_channel(b, base, 0), patch_id, NULL, NULL);

   auto idx1 = nir_src_as_const_value(op->src[0]);
   if (!idx1 || idx1->u32 != 0)
      addr = nir_umad24(b, nir_channel(b, base, 1), op->src[0].ssa, addr);

   auto offset = nir_imm_int(b, get_tcs_varying_offset(op));

   auto idx2 = nir_src_as_const_value(op->src[1]);
   if (!idx2 || idx2->u32 != 0)
      offset = nir_iadd(b, nir_ishl_imm(b, op->src[1].ssa, 4), offset);

   return nir_iadd(b, addr, offset);
}

static nir_def *
emil_lsd_out_addr(nir_builder *b,
                  nir_def *base,
                  nir_def *patch_id,
                  nir_intrinsic_instr *op,
                  UNUSED nir_variable_mode mode,
                  int src_offset)
{

   nir_def *addr1 =
      nir_umad24(b, nir_channel(b, base, 0), patch_id, nir_channel(b, base, 2));
   nir_def *addr2 =
      nir_umad24(b, nir_channel(b, base, 1), op->src[src_offset].ssa, addr1);
   int offset = get_tcs_varying_offset(op);
   return nir_iadd_imm(b,
                       nir_iadd(b,
                                addr2,
                                nir_ishl_imm(b, op->src[src_offset + 1].ssa, 4)),
                       offset);
}

static nir_def *
load_offset_group(nir_builder *b, int ncomponents)
{
   switch (ncomponents) {
   /* tess outer offsets */
   case 1:
      return nir_imm_int(b, 0);
   case 2:
      return nir_imm_ivec2(b, 0, 4);
   case 3:
      return r600_imm_ivec3(b, 0, 4, 8);
   case 4:
      return nir_imm_ivec4(b, 0, 4, 8, 12);
      /* tess inner offsets */
   case 5:
      return nir_imm_int(b, 16);
   case 6:
      return nir_imm_ivec2(b, 16, 20);
   default:
      debug_printf("Got %d components\n", ncomponents);
      unreachable("Unsupported component count");
   }
}

static nir_def *
load_offset_group_from_mask(nir_builder *b, uint32_t mask)
{
   auto full_mask = nir_imm_ivec4(b, 0, 4, 8, 12);
   return nir_channels(b, full_mask, mask);
}

struct MaskQuery {
   uint32_t mask;
   uint32_t ssa_index;
   nir_alu_instr *alu;
   int index;
   uint32_t full_mask;
};

static bool
update_alu_mask(nir_src *src, void *data)
{
   auto mq = reinterpret_cast<MaskQuery *>(data);

   if (mq->ssa_index == src->ssa->index) {
      mq->mask |= nir_alu_instr_src_read_mask(mq->alu, mq->index);
   }
   ++mq->index;

   return mq->mask != mq->full_mask;
}

static uint32_t
get_dest_usee_mask(nir_intrinsic_instr *op)
{
   MaskQuery mq = {0};
   mq.full_mask = (1 << op->def.num_components) - 1;

   nir_foreach_use(use_src, &op->def)
   {
      auto use_instr = nir_src_parent_instr(use_src);
      mq.ssa_index = use_src->ssa->index;

      switch (use_instr->type) {
      case nir_instr_type_alu: {
         mq.alu = nir_instr_as_alu(use_instr);
         mq.index = 0;
         if (!nir_foreach_src(use_instr, update_alu_mask, &mq))
            return 0xf;
         break;
      }
      case nir_instr_type_intrinsic: {
         auto intr = nir_instr_as_intrinsic(use_instr);
         switch (intr->intrinsic) {
         case nir_intrinsic_store_output:
         case nir_intrinsic_store_per_vertex_output:
            mq.mask |= nir_intrinsic_write_mask(intr) << nir_intrinsic_component(intr);
            break;
         case nir_intrinsic_store_scratch:
         case nir_intrinsic_store_local_shared_r600:
            mq.mask |= nir_intrinsic_write_mask(intr);
            break;
         default:
            return 0xf;
         }
         break;
      }
      default:
         return 0xf;
      }
   }
   return mq.mask;
}

static void
replace_load_instr(nir_builder *b, nir_intrinsic_instr *op, nir_def *addr)
{
   uint32_t mask = get_dest_usee_mask(op);
   if (mask) {
      nir_def *addr_outer = nir_iadd(b, addr, load_offset_group_from_mask(b, mask));
      if (nir_intrinsic_component(op))
         addr_outer =
            nir_iadd_imm(b, addr_outer, 4 * nir_intrinsic_component(op));

      auto new_load = nir_load_local_shared_r600(b, 32, addr_outer);

      auto undef = nir_undef(b, 1, 32);
      int comps = op->def.num_components;
      nir_def *remix[4] = {undef, undef, undef, undef};

      int chan = 0;
      for (int i = 0; i < comps; ++i) {
         if (mask & (1 << i)) {
            remix[i] = nir_channel(b, new_load, chan++);
         }
      }
      auto new_load_remixed = nir_vec(b, remix, comps);
      nir_def_rewrite_uses(&op->def, new_load_remixed);
   }
   nir_instr_remove(&op->instr);
}

static void
emit_store_lds(nir_builder *b, nir_intrinsic_instr *op, nir_def *addr)
{
   uint32_t orig_writemask = nir_intrinsic_write_mask(op) << nir_intrinsic_component(op);

   for (int i = 0; i < 2; ++i) {
      unsigned test_mask = (0x3 << 2 * i);
      unsigned wmask = orig_writemask & test_mask;
      if (!(wmask))
         continue;

      uint32_t writemask = wmask >> nir_intrinsic_component(op);

      bool start_even = (orig_writemask & (1u << (2 * i)));
      nir_def *addr2 = nir_iadd_imm(b, addr, 8 * i + (start_even ? 0 : 4));
      nir_store_local_shared_r600(b, op->src[0].ssa, addr2,
                                  .write_mask = writemask);
   }
}

static nir_def *
emil_tcs_io_offset(nir_builder *b,
                   nir_def *addr,
                   nir_intrinsic_instr *op,
                   int src_offset)
{
   int offset = get_tcs_varying_offset(op);
   return nir_iadd_imm(b,
                       nir_iadd(b,
                                addr,
                                nir_ishl_imm(b, op->src[src_offset].ssa, 4)),
                       offset);
}

inline unsigned
outer_tf_components(mesa_prim prim_type)
{
   switch (prim_type) {
   case MESA_PRIM_LINES:
      return 2;
   case MESA_PRIM_TRIANGLES:
      return 3;
   case MESA_PRIM_QUADS:
      return 4;
   default:
      return 0;
   }
}

static bool
r600_lower_tess_io_impl(nir_builder *b, nir_instr *instr, enum mesa_prim prim_type)
{
   static nir_def *load_in_param_base = nullptr;
   static nir_def *load_out_param_base = nullptr;

   b->cursor = nir_before_instr(instr);
   nir_intrinsic_instr *op = nir_instr_as_intrinsic(instr);

   if (b->shader->info.stage == MESA_SHADER_TESS_CTRL) {
      load_in_param_base = nir_load_tcs_in_param_base_r600(b);
      load_out_param_base = nir_load_tcs_out_param_base_r600(b);
   } else if (b->shader->info.stage == MESA_SHADER_TESS_EVAL) {
      load_in_param_base = nir_load_tcs_out_param_base_r600(b);
   } else if (b->shader->info.stage == MESA_SHADER_VERTEX) {
      load_out_param_base = nir_load_tcs_in_param_base_r600(b);
   }

   auto rel_patch_id = nir_load_tcs_rel_patch_id_r600(b);

   unsigned tf_inner_address_offset = 0;
   unsigned ncomps_correct = 0;

   switch (op->intrinsic) {
   case nir_intrinsic_load_patch_vertices_in: {
      nir_def *vertices_in;
      if (b->shader->info.stage == MESA_SHADER_TESS_CTRL)
         vertices_in = nir_channel(b, load_in_param_base, 2);
      else {
         auto base = nir_load_tcs_in_param_base_r600(b);
         vertices_in = nir_channel(b, base, 2);
      }
      nir_def_rewrite_uses(&op->def, vertices_in);
      nir_instr_remove(&op->instr);
      return true;
   }
   case nir_intrinsic_load_per_vertex_input: {
      nir_def *addr =
         b->shader->info.stage == MESA_SHADER_TESS_CTRL
            ? emil_lsd_in_addr(b, load_in_param_base, rel_patch_id, op)
            : emil_lsd_out_addr(
                 b, load_in_param_base, rel_patch_id, op, nir_var_shader_in, 0);
      replace_load_instr(b, op, addr);
      return true;
   }
   case nir_intrinsic_store_per_vertex_output: {
      nir_def *addr = emil_lsd_out_addr(
         b, load_out_param_base, rel_patch_id, op, nir_var_shader_out, 1);
      emit_store_lds(b, op, addr);
      nir_instr_remove(instr);
      return true;
   }
   case nir_intrinsic_load_per_vertex_output: {
      nir_def *addr = emil_lsd_out_addr(
         b, load_out_param_base, rel_patch_id, op, nir_var_shader_out, 0);
      replace_load_instr(b, op, addr);
      return true;
   }
   case nir_intrinsic_store_output: {
      nir_def *addr = (b->shader->info.stage == MESA_SHADER_TESS_CTRL)
                             ? r600_tcs_base_address(b, load_out_param_base, rel_patch_id)
                             : nir_build_alu(b,
                                             nir_op_umul24,
                                             nir_channel(b, load_out_param_base, 1),
                                             rel_patch_id,
                                             NULL,
                                             NULL);
      addr = emil_tcs_io_offset(b, addr, op, 1);
      emit_store_lds(b, op, addr);
      nir_instr_remove(instr);
      return true;
   }
   case nir_intrinsic_load_output: {
      nir_def *addr = r600_tcs_base_address(b, load_out_param_base, rel_patch_id);
      addr = emil_tcs_io_offset(b, addr, op, 0);
      replace_load_instr(b, op, addr);
      return true;
   }
   case nir_intrinsic_load_input: {
      nir_def *addr = r600_tcs_base_address(b, load_in_param_base, rel_patch_id);
      addr = emil_tcs_io_offset(b, addr, op, 0);
      replace_load_instr(b, op, addr);
      return true;
   }
   case nir_intrinsic_load_tess_level_inner:
      tf_inner_address_offset = 4;
      ncomps_correct = 2;
      FALLTHROUGH;
   case nir_intrinsic_load_tess_level_outer: {
      auto ncomps = outer_tf_components(prim_type);
      if (!ncomps)
         return false;
      ncomps -= ncomps_correct;
      auto base = nir_load_tcs_out_param_base_r600(b);
      auto rel_patch_id = nir_load_tcs_rel_patch_id_r600(b);
      nir_def *addr0 = r600_tcs_base_address(b, base, rel_patch_id);
      nir_def *addr_outer =
         nir_iadd(b, addr0, load_offset_group(b, tf_inner_address_offset + ncomps));

      nir_def *tf = nir_load_local_shared_r600(b, 32, addr_outer);
      if (ncomps < 4 && b->shader->info.stage != MESA_SHADER_TESS_EVAL) {
         auto undef = nir_undef(b, 1, 32);
         nir_def *srcs[4] = {undef, undef, undef, undef};
         for (unsigned i = 0; i < ncomps; ++i)
            srcs[i] = nir_channel(b, tf, i);
         auto help = nir_vec(b, srcs, 4);
         nir_def_rewrite_uses(&op->def, help);
      } else {
         nir_def_rewrite_uses(&op->def, tf);
      }
      nir_instr_remove(instr);
      return true;
   }
   default:;
   }

   return false;
}

bool
r600_lower_tess_io(nir_shader *shader, enum mesa_prim prim_type)
{
   bool progress = false;
   nir_foreach_function_impl(impl, shader)
   {
      nir_builder b = nir_builder_create(impl);

      nir_foreach_block(block, impl)
      {
         nir_foreach_instr_safe(instr, block)
         {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            if (r600_lower_tess_io_filter(instr, shader->info.stage))
               progress |= r600_lower_tess_io_impl(&b, instr, prim_type);
         }
      }
   }
   return progress;
}

bool
r600_emit_tf(nir_builder *b, nir_def *val)
{
   nir_intrinsic_instr *store_tf =
      nir_intrinsic_instr_create(b->shader, nir_intrinsic_store_tf_r600);
   store_tf->num_components = val->num_components;
   store_tf->src[0] = nir_src_for_ssa(val);
   nir_builder_instr_insert(b, &store_tf->instr);
   return true;
}

bool
r600_append_tcs_TF_emission(nir_shader *shader, enum mesa_prim prim_type)
{
   if (shader->info.stage != MESA_SHADER_TESS_CTRL)
      return false;

   nir_foreach_function_impl(impl, shader)
   {
      nir_foreach_block(block, impl)
      {
         nir_foreach_instr_safe(instr, block)
         {
            if (instr->type != nir_instr_type_intrinsic)
               continue;
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic == nir_intrinsic_store_tf_r600) {
               return false;
            }
         }
      }
   }

   assert(exec_list_length(&shader->functions) == 1);
   nir_function *f = (nir_function *)shader->functions.get_head();
   nir_builder builder = nir_builder_create(f->impl);
   nir_builder *b = &builder;

   auto outer_comps = outer_tf_components(prim_type);
   if (!outer_comps)
      return false;

   unsigned inner_comps = outer_comps - 2;
   unsigned stride = (inner_comps + outer_comps) * 4;

   b->cursor = nir_after_cf_list(&f->impl->body);

   nir_def *invocation_id = nir_load_invocation_id(b);

   nir_push_if(b, nir_ieq_imm(b, invocation_id, 0));
   auto base = nir_load_tcs_out_param_base_r600(b);
   auto rel_patch_id = nir_load_tcs_rel_patch_id_r600(b);

   nir_def *addr0 = r600_tcs_base_address(b, base, rel_patch_id);

   nir_def *addr_outer = nir_iadd(b, addr0, load_offset_group(b, outer_comps));
   nir_def *tf_outer = nir_load_local_shared_r600(b, 32, addr_outer);

   std::vector<nir_def *> tf_out;

   nir_def *tf_out_base = nir_load_tcs_tess_factor_base_r600(b);
   nir_def *out_addr0 = nir_umad24(b,
                                   rel_patch_id,
                                   nir_imm_int(b, stride),
                                   tf_out_base);
   int chanx = 0;
   int chany = 1;

   if (prim_type == MESA_PRIM_LINES)
      std::swap(chanx, chany);

   int inner_base = 12;

   tf_out.push_back(nir_vec2(b,
                             out_addr0,
                             nir_channel(b, tf_outer, chanx)));

   tf_out.push_back(nir_vec2(b, nir_iadd_imm(b, out_addr0, 4),
                             nir_channel(b, tf_outer, chany)));


   if (outer_comps > 2) {
      tf_out.push_back(nir_vec2(b,
                                nir_iadd_imm(b, out_addr0, 8),
                                nir_channel(b, tf_outer, 2)));
   }

   if (outer_comps > 3) {
      tf_out.push_back(nir_vec2(b,
                                nir_iadd_imm(b, out_addr0, 12),
                                nir_channel(b, tf_outer, 3)));
      inner_base = 16;

   }

   if (inner_comps) {
      nir_def *addr1 = nir_iadd(b, addr0, load_offset_group(b, 4 + inner_comps));
      nir_def *tf_inner = nir_load_local_shared_r600(b, 32, addr1);

      tf_out.push_back(nir_vec2(b,
                                nir_iadd_imm(b, out_addr0, inner_base),
                                nir_channel(b, tf_inner, 0)));


      if (inner_comps > 1) {
         tf_out.push_back(nir_vec2(b,
                                   nir_iadd_imm(b, out_addr0, inner_base + 4),
                                   nir_channel(b, tf_inner, 1)));

      }
   }

   for (auto tf : tf_out)
      r600_emit_tf(b, tf);

   nir_pop_if(b, nullptr);

   nir_metadata_preserve(f->impl, nir_metadata_none);

   return true;
}
