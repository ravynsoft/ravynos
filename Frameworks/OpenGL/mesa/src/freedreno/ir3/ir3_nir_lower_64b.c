/*
 * Copyright © 2021 Google, Inc.
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

#include "ir3_nir.h"

/*
 * Lowering for 64b intrinsics generated with OpenCL or with
 * VK_KHR_buffer_device_address. All our intrinsics from a hw
 * standpoint are 32b, so we just need to combine in zero for
 * the upper 32bits and let the other nir passes clean up the mess.
 */

static bool
lower_64b_intrinsics_filter(const nir_instr *instr, const void *unused)
{
   (void)unused;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   if (intr->intrinsic == nir_intrinsic_load_deref ||
       intr->intrinsic == nir_intrinsic_store_deref)
      return false;

   if (is_intrinsic_store(intr->intrinsic))
      return nir_src_bit_size(intr->src[0]) == 64;

   if (nir_intrinsic_dest_components(intr) == 0)
      return false;

   return intr->def.bit_size == 64;
}

static nir_def *
lower_64b_intrinsics(nir_builder *b, nir_instr *instr, void *unused)
{
   (void)unused;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   /* We could be *slightly* more clever and, for ex, turn a 64b vec4
    * load into two 32b vec4 loads, rather than 4 32b vec2 loads.
    */

   if (is_intrinsic_store(intr->intrinsic)) {
      unsigned offset_src_idx;
      switch (intr->intrinsic) {
      case nir_intrinsic_store_ssbo:
      case nir_intrinsic_store_global_ir3:
         offset_src_idx = 2;
         break;
      default:
         offset_src_idx = 1;
      }

      unsigned num_comp = nir_intrinsic_src_components(intr, 0);
      unsigned wrmask = nir_intrinsic_has_write_mask(intr) ?
         nir_intrinsic_write_mask(intr) : BITSET_MASK(num_comp);
      nir_def *val = intr->src[0].ssa;
      nir_def *off = intr->src[offset_src_idx].ssa;

      for (unsigned i = 0; i < num_comp; i++) {
         if (!(wrmask & BITFIELD_BIT(i)))
            continue;

         nir_def *c64 = nir_channel(b, val, i);
         nir_def *c32 = nir_unpack_64_2x32(b, c64);

         nir_intrinsic_instr *store =
            nir_instr_as_intrinsic(nir_instr_clone(b->shader, &intr->instr));
         store->num_components = 2;
         store->src[0] = nir_src_for_ssa(c32);
         store->src[offset_src_idx] = nir_src_for_ssa(off);

         if (nir_intrinsic_has_write_mask(intr))
            nir_intrinsic_set_write_mask(store, 0x3);
         nir_builder_instr_insert(b, &store->instr);

         off = nir_iadd_imm(b, off, 8);
      }

      return NIR_LOWER_INSTR_PROGRESS_REPLACE;
   }

   unsigned num_comp = nir_intrinsic_dest_components(intr);

   nir_def *def = &intr->def;
   def->bit_size = 32;

   /* load_kernel_input is handled specially, lowering to two 32b inputs:
    */
   if (intr->intrinsic == nir_intrinsic_load_kernel_input) {
      assert(num_comp == 1);

      nir_def *offset = nir_iadd_imm(b,
            intr->src[0].ssa, 4);

      nir_def *upper = nir_load_kernel_input(b, 1, 32, offset);

      return nir_pack_64_2x32_split(b, def, upper);
   }

   nir_def *components[num_comp];

   if (is_intrinsic_load(intr->intrinsic)) {
      unsigned offset_src_idx;
      switch(intr->intrinsic) {
      case nir_intrinsic_load_ssbo:
      case nir_intrinsic_load_ubo:
      case nir_intrinsic_load_global_ir3:
         offset_src_idx = 1;
         break;
      default:
         offset_src_idx = 0;
      }

      nir_def *off = intr->src[offset_src_idx].ssa;

      for (unsigned i = 0; i < num_comp; i++) {
         nir_intrinsic_instr *load =
            nir_instr_as_intrinsic(nir_instr_clone(b->shader, &intr->instr));
         load->num_components = 2;
         load->src[offset_src_idx] = nir_src_for_ssa(off);

         nir_def_init(&load->instr, &load->def, 2, 32);
         nir_builder_instr_insert(b, &load->instr);

         components[i] = nir_pack_64_2x32(b, &load->def);

         off = nir_iadd_imm(b, off, 8);
      }
   } else {
      /* The remaining (non load/store) intrinsics just get zero-
       * extended from 32b to 64b:
       */
      for (unsigned i = 0; i < num_comp; i++) {
         nir_def *c = nir_channel(b, def, i);
         components[i] = nir_pack_64_2x32_split(b, c, nir_imm_zero(b, 1, 32));
      }
   }

   return nir_build_alu_src_arr(b, nir_op_vec(num_comp), components);
}

bool
ir3_nir_lower_64b_intrinsics(nir_shader *shader)
{
   return nir_shader_lower_instructions(
         shader, lower_64b_intrinsics_filter,
         lower_64b_intrinsics, NULL);
}

/*
 * Lowering for 64b undef instructions, splitting into a two 32b undefs
 */

static nir_def *
lower_64b_undef(nir_builder *b, nir_instr *instr, void *unused)
{
   (void)unused;

   nir_undef_instr *undef = nir_instr_as_undef(instr);
   unsigned num_comp = undef->def.num_components;
   nir_def *components[num_comp];

   for (unsigned i = 0; i < num_comp; i++) {
      nir_def *lowered = nir_undef(b, 2, 32);

      components[i] = nir_pack_64_2x32_split(b,
                                             nir_channel(b, lowered, 0),
                                             nir_channel(b, lowered, 1));
   }

   return nir_build_alu_src_arr(b, nir_op_vec(num_comp), components);
}

static bool
lower_64b_undef_filter(const nir_instr *instr, const void *unused)
{
   (void)unused;

   return instr->type == nir_instr_type_undef &&
      nir_instr_as_undef(instr)->def.bit_size == 64;
}

bool
ir3_nir_lower_64b_undef(nir_shader *shader)
{
   return nir_shader_lower_instructions(
         shader, lower_64b_undef_filter,
         lower_64b_undef, NULL);
}

/*
 * Lowering for load_global/store_global with 64b addresses to ir3
 * variants, which instead take a uvec2_32
 */

static bool
lower_64b_global_filter(const nir_instr *instr, const void *unused)
{
   (void)unused;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   switch (intr->intrinsic) {
   case nir_intrinsic_load_global:
   case nir_intrinsic_load_global_constant:
   case nir_intrinsic_store_global:
   case nir_intrinsic_global_atomic:
   case nir_intrinsic_global_atomic_swap:
      return true;
   default:
      return false;
   }
}

static nir_def *
lower_64b_global(nir_builder *b, nir_instr *instr, void *unused)
{
   (void)unused;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   bool load = intr->intrinsic != nir_intrinsic_store_global;

   nir_def *addr64 = intr->src[load ? 0 : 1].ssa;
   nir_def *addr = nir_unpack_64_2x32(b, addr64);

   /*
    * Note that we can get vec8/vec16 with OpenCL.. we need to split
    * those up into max 4 components per load/store.
    */

   if (intr->intrinsic == nir_intrinsic_global_atomic) {
      return nir_global_atomic_ir3(
            b, intr->def.bit_size, addr,
            intr->src[1].ssa,
         .atomic_op = nir_intrinsic_atomic_op(intr));
   } else if (intr->intrinsic == nir_intrinsic_global_atomic_swap) {
      return nir_global_atomic_swap_ir3(
         b, intr->def.bit_size, addr,
         intr->src[1].ssa, intr->src[2].ssa,
         .atomic_op = nir_intrinsic_atomic_op(intr));
   }

   if (load) {
      unsigned num_comp = nir_intrinsic_dest_components(intr);
      nir_def *components[num_comp];
      for (unsigned off = 0; off < num_comp;) {
         unsigned c = MIN2(num_comp - off, 4);
         nir_def *val = nir_load_global_ir3(
               b, c, intr->def.bit_size,
               addr, nir_imm_int(b, off));
         for (unsigned i = 0; i < c; i++) {
            components[off++] = nir_channel(b, val, i);
         }
      }
      return nir_build_alu_src_arr(b, nir_op_vec(num_comp), components);
   } else {
      unsigned num_comp = nir_intrinsic_src_components(intr, 0);
      nir_def *value = intr->src[0].ssa;
      for (unsigned off = 0; off < num_comp; off += 4) {
         unsigned c = MIN2(num_comp - off, 4);
         nir_def *v = nir_channels(b, value, BITFIELD_MASK(c) << off);
         nir_store_global_ir3(b, v, addr, nir_imm_int(b, off));
      }
      return NIR_LOWER_INSTR_PROGRESS_REPLACE;
   }
}

bool
ir3_nir_lower_64b_global(nir_shader *shader)
{
   return nir_shader_lower_instructions(
         shader, lower_64b_global_filter,
         lower_64b_global, NULL);
}

/*
 * Lowering for 64b registers:
 * - @decl_reg -> split in two 32b ones
 * - @store_reg -> unpack_64_2x32_split_x/y and two separate stores
 * - @load_reg -> two separate loads and pack_64_2x32_split
 */

static void
lower_64b_reg(nir_builder *b, nir_intrinsic_instr *reg)
{
   unsigned num_components = nir_intrinsic_num_components(reg);
   unsigned num_array_elems = nir_intrinsic_num_array_elems(reg);

   nir_def *reg_hi = nir_decl_reg(b, num_components, 32, num_array_elems);
   nir_def *reg_lo = nir_decl_reg(b, num_components, 32, num_array_elems);

   nir_foreach_reg_store_safe (store_reg_src, reg) {
      nir_intrinsic_instr *store =
         nir_instr_as_intrinsic(nir_src_parent_instr(store_reg_src));
      b->cursor = nir_before_instr(&store->instr);

      nir_def *packed = store->src[0].ssa;
      nir_def *unpacked_lo = nir_unpack_64_2x32_split_x(b, packed);
      nir_def *unpacked_hi = nir_unpack_64_2x32_split_y(b, packed);
      int base = nir_intrinsic_base(store);

      if (store->intrinsic == nir_intrinsic_store_reg) {
         nir_build_store_reg(b, unpacked_lo, reg_lo, .base = base);
         nir_build_store_reg(b, unpacked_hi, reg_hi, .base = base);
      } else {
         assert(store->intrinsic == nir_intrinsic_store_reg_indirect);

         nir_def *offset = store->src[2].ssa;
         nir_store_reg_indirect(b, unpacked_lo, reg_lo, offset, .base = base);
         nir_store_reg_indirect(b, unpacked_hi, reg_hi, offset, .base = base);
      }

      nir_instr_remove(&store->instr);
   }

   nir_foreach_reg_load_safe (load_reg_src, reg) {
      nir_intrinsic_instr *load =
         nir_instr_as_intrinsic(nir_src_parent_instr(load_reg_src));
      b->cursor = nir_before_instr(&load->instr);

      int base = nir_intrinsic_base(load);
      nir_def *load_lo, *load_hi;

      if (load->intrinsic == nir_intrinsic_load_reg) {
         load_lo =
            nir_build_load_reg(b, num_components, 32, reg_lo, .base = base);
         load_hi =
            nir_build_load_reg(b, num_components, 32, reg_hi, .base = base);
      } else {
         assert(load->intrinsic == nir_intrinsic_load_reg_indirect);

         nir_def *offset = load->src[1].ssa;
         load_lo = nir_load_reg_indirect(b, num_components, 32, reg_lo, offset,
                                         .base = base);
         load_hi = nir_load_reg_indirect(b, num_components, 32, reg_hi, offset,
                                         .base = base);
      }

      nir_def *packed = nir_pack_64_2x32_split(b, load_lo, load_hi);
      nir_def_rewrite_uses(&load->def, packed);
      nir_instr_remove(&load->instr);
   }

   nir_instr_remove(&reg->instr);
}

bool
ir3_nir_lower_64b_regs(nir_shader *shader)
{
   bool progress = false;

   nir_foreach_function_impl (impl, shader) {
      bool impl_progress = false;
      nir_builder b = nir_builder_create(impl);

      nir_foreach_reg_decl_safe (reg, impl) {
         if (nir_intrinsic_bit_size(reg) == 64) {
            lower_64b_reg(&b, reg);
            impl_progress = true;
         }
      }

      if (impl_progress) {
         nir_metadata_preserve(
            impl, nir_metadata_block_index | nir_metadata_dominance);
         progress = true;
      }
   }

   return progress;
}
