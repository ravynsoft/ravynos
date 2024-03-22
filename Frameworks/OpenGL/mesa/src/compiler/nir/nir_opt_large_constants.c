/*
 * Copyright © 2018 Intel Corporation
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
 */

#include "nir.h"
#include "nir_builder.h"
#include "nir_deref.h"

#include "util/u_math.h"

static void
read_const_values(nir_const_value *dst, const void *src,
                  unsigned num_components, unsigned bit_size)
{
   memset(dst, 0, num_components * sizeof(*dst));

   switch (bit_size) {
   case 1:
      /* Booleans are special-cased to be 32-bit */
      assert(((uintptr_t)src & 0x3) == 0);
      for (unsigned i = 0; i < num_components; i++)
         dst[i].b = ((int32_t *)src)[i] != 0;
      break;

   case 8:
      for (unsigned i = 0; i < num_components; i++)
         dst[i].u8 = ((int8_t *)src)[i];
      break;

   case 16:
      assert(((uintptr_t)src & 0x1) == 0);
      for (unsigned i = 0; i < num_components; i++)
         dst[i].u16 = ((int16_t *)src)[i];
      break;

   case 32:
      assert(((uintptr_t)src & 0x3) == 0);
      for (unsigned i = 0; i < num_components; i++)
         dst[i].u32 = ((int32_t *)src)[i];
      break;

   case 64:
      assert(((uintptr_t)src & 0x7) == 0);
      for (unsigned i = 0; i < num_components; i++)
         dst[i].u64 = ((int64_t *)src)[i];
      break;

   default:
      unreachable("Invalid bit size");
   }
}

static void
write_const_values(void *dst, const nir_const_value *src,
                   nir_component_mask_t write_mask,
                   unsigned bit_size)
{
   switch (bit_size) {
   case 1:
      /* Booleans are special-cased to be 32-bit */
      assert(((uintptr_t)dst & 0x3) == 0);
      u_foreach_bit(i, write_mask)
         ((int32_t *)dst)[i] = -(int)src[i].b;
      break;

   case 8:
      u_foreach_bit(i, write_mask)
         ((int8_t *)dst)[i] = src[i].u8;
      break;

   case 16:
      assert(((uintptr_t)dst & 0x1) == 0);
      u_foreach_bit(i, write_mask)
         ((int16_t *)dst)[i] = src[i].u16;
      break;

   case 32:
      assert(((uintptr_t)dst & 0x3) == 0);
      u_foreach_bit(i, write_mask)
         ((int32_t *)dst)[i] = src[i].u32;
      break;

   case 64:
      assert(((uintptr_t)dst & 0x7) == 0);
      u_foreach_bit(i, write_mask)
         ((int64_t *)dst)[i] = src[i].u64;
      break;

   default:
      unreachable("Invalid bit size");
   }
}

struct small_constant {
   uint64_t data;
   uint32_t bit_size;
   bool is_float;
   uint32_t bit_stride;
};

struct var_info {
   nir_variable *var;

   bool is_constant;
   bool is_small;
   bool found_read;
   bool duplicate;

   /* Block that has all the variable stores.  All the blocks with reads
    * should be dominated by this block.
    */
   nir_block *block;

   /* If is_constant, hold the collected constant data for this var. */
   uint32_t constant_data_size;
   void *constant_data;

   struct small_constant small_constant;
};

static int
var_info_cmp(const void *_a, const void *_b)
{
   const struct var_info *a = _a;
   const struct var_info *b = _b;
   uint32_t a_size = a->constant_data_size;
   uint32_t b_size = b->constant_data_size;

   if (a->is_constant != b->is_constant) {
      return (int)a->is_constant - (int)b->is_constant;
   } else if (a_size < b_size) {
      return -1;
   } else if (a_size > b_size) {
      return 1;
   } else if (a_size == 0) {
      /* Don't call memcmp with invalid pointers. */
      return 0;
   } else {
      return memcmp(a->constant_data, b->constant_data, a_size);
   }
}

static nir_def *
build_constant_load(nir_builder *b, nir_deref_instr *deref,
                    glsl_type_size_align_func size_align)
{
   nir_variable *var = nir_deref_instr_get_variable(deref);

   const unsigned bit_size = glsl_get_bit_size(deref->type);
   const unsigned num_components = glsl_get_vector_elements(deref->type);

   UNUSED unsigned var_size, var_align;
   size_align(var->type, &var_size, &var_align);
   assert(var->data.location % var_align == 0);

   UNUSED unsigned deref_size, deref_align;
   size_align(deref->type, &deref_size, &deref_align);

   nir_def *src = nir_build_deref_offset(b, deref, size_align);
   nir_def *load =
      nir_load_constant(b, num_components, bit_size, src,
                        .base = var->data.location,
                        .range = var_size,
                        .align_mul = deref_align,
                        .align_offset = 0);

   if (load->bit_size < 8) {
      /* Booleans are special-cased to be 32-bit */
      assert(glsl_type_is_boolean(deref->type));
      assert(deref_size == num_components * 4);
      load->bit_size = 32;
      return nir_b2b1(b, load);
   } else {
      assert(deref_size == num_components * bit_size / 8);
      return load;
   }
}

static void
handle_constant_store(void *mem_ctx, struct var_info *info,
                      nir_deref_instr *deref, nir_const_value *val,
                      nir_component_mask_t write_mask,
                      glsl_type_size_align_func size_align)
{
   assert(!nir_deref_instr_has_indirect(deref));
   const unsigned bit_size = glsl_get_bit_size(deref->type);
   const unsigned num_components = glsl_get_vector_elements(deref->type);

   if (info->constant_data_size == 0) {
      unsigned var_size, var_align;
      size_align(info->var->type, &var_size, &var_align);
      info->constant_data_size = var_size;
      info->constant_data = rzalloc_size(mem_ctx, var_size);
   }

   const unsigned offset = nir_deref_instr_get_const_offset(deref, size_align);
   if (offset >= info->constant_data_size)
      return;

   write_const_values((char *)info->constant_data + offset, val,
                      write_mask & nir_component_mask(num_components),
                      bit_size);
}

static void
get_small_constant(struct var_info *info, glsl_type_size_align_func size_align)
{
   if (!glsl_type_is_array(info->var->type))
      return;

   const struct glsl_type *elem_type = glsl_get_array_element(info->var->type);
   if (!glsl_type_is_scalar(elem_type))
      return;

   uint32_t array_len = glsl_get_length(info->var->type);
   uint32_t bit_size = glsl_get_bit_size(elem_type);

   /* If our array is large, don't even bother */
   if (array_len > 64)
      return;

   /* Skip cases that can be lowered to a bcsel ladder more efficiently. */
   if (array_len <= 3)
      return;

   uint32_t elem_size, elem_align;
   size_align(elem_type, &elem_size, &elem_align);
   uint32_t stride = ALIGN_POT(elem_size, elem_align);

   if (stride != (bit_size == 1 ? 4 : bit_size / 8))
      return;

   nir_const_value values[64];
   read_const_values(values, info->constant_data, array_len, bit_size);

   bool is_float = true;
   if (bit_size < 16) {
      is_float = false;
   } else {
      for (unsigned i = 0; i < array_len; i++) {
         /* See if it's an easily convertible float.
          * TODO: Compute greatest common divisor to support non-integer floats.
          * TODO: Compute min value and add it to the result of
          *       build_small_constant_load for handling negative floats.
          */
         uint64_t u = nir_const_value_as_float(values[i], bit_size);
         nir_const_value fc = nir_const_value_for_float(u, bit_size);
         is_float &= !memcmp(&fc, &values[i], bit_size / 8);
      }
   }

   uint32_t used_bits = 0;
   for (unsigned i = 0; i < array_len; i++) {
      uint64_t u64_elem = is_float ? nir_const_value_as_float(values[i], bit_size)
                                   : nir_const_value_as_uint(values[i], bit_size);
      if (!u64_elem)
         continue;

      uint32_t elem_bits = util_logbase2_64(u64_elem) + 1;
      used_bits = MAX2(used_bits, elem_bits);
   }

   /* Only use power-of-two numbers of bits so we end up with a shift
    * instead of a multiply on our index.
    */
   used_bits = util_next_power_of_two(used_bits);

   if (used_bits * array_len > 64)
      return;

   info->is_small = true;

   for (unsigned i = 0; i < array_len; i++) {
      uint64_t u64_elem = is_float ? nir_const_value_as_float(values[i], bit_size)
                                   : nir_const_value_as_uint(values[i], bit_size);

      info->small_constant.data |= u64_elem << (i * used_bits);
   }

   /* Limit bit_size >= 32 to avoid unnecessary conversions.  */
   info->small_constant.bit_size =
      MAX2(util_next_power_of_two(used_bits * array_len), 32);
   info->small_constant.is_float = is_float;
   info->small_constant.bit_stride = used_bits;
}

static nir_def *
build_small_constant_load(nir_builder *b, nir_deref_instr *deref,
                          struct var_info *info, glsl_type_size_align_func size_align)
{
   struct small_constant *constant = &info->small_constant;

   nir_def *imm = nir_imm_intN_t(b, constant->data, constant->bit_size);

   assert(deref->deref_type == nir_deref_type_array);
   nir_def *index = deref->arr.index.ssa;

   nir_def *shift = nir_imul_imm(b, index, constant->bit_stride);

   nir_def *ret = nir_ushr(b, imm, nir_u2u32(b, shift));
   ret = nir_iand_imm(b, ret, BITFIELD64_MASK(constant->bit_stride));

   const unsigned bit_size = glsl_get_bit_size(deref->type);
   if (bit_size < 8) {
      /* Booleans are special-cased to be 32-bit */
      assert(glsl_type_is_boolean(deref->type));
      ret = nir_ine_imm(b, ret, 0);
   } else {
      if (constant->is_float)
         ret = nir_u2fN(b, ret, bit_size);
      else if (bit_size != constant->bit_size)
         ret = nir_u2uN(b, ret, bit_size);
   }

   return ret;
}

/** Lower large constant variables to shader constant data
 *
 * This pass looks for large (type_size(var->type) > threshold) variables
 * which are statically constant and moves them into shader constant data.
 * This is especially useful when large tables are baked into the shader
 * source code because they can be moved into a UBO by the driver to reduce
 * register pressure and make indirect access cheaper.
 */
bool
nir_opt_large_constants(nir_shader *shader,
                        glsl_type_size_align_func size_align,
                        unsigned threshold)
{
   /* Default to a natural alignment if none is provided */
   if (size_align == NULL)
      size_align = glsl_get_natural_size_align_bytes;

   /* This only works with a single entrypoint */
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);

   unsigned num_locals = nir_function_impl_index_vars(impl);

   if (num_locals == 0) {
      nir_shader_preserve_all_metadata(shader);
      return false;
   }

   struct var_info *var_infos = ralloc_array(NULL, struct var_info, num_locals);
   nir_foreach_function_temp_variable(var, impl) {
      var_infos[var->index] = (struct var_info){
         .var = var,
         .is_constant = true,
         .found_read = false,
      };
   }

   nir_metadata_require(impl, nir_metadata_dominance);

   /* First, walk through the shader and figure out what variables we can
    * lower to the constant blob.
    */
   nir_foreach_block(block, impl) {
      nir_foreach_instr(instr, block) {
         if (instr->type == nir_instr_type_deref) {
            /* If we ever see a complex use of a deref_var, we have to assume
             * that variable is non-constant because we can't guarantee we
             * will find all of the writers of that variable.
             */
            nir_deref_instr *deref = nir_instr_as_deref(instr);
            if (deref->deref_type == nir_deref_type_var &&
                deref->var->data.mode == nir_var_function_temp &&
                nir_deref_instr_has_complex_use(deref, 0))
               var_infos[deref->var->index].is_constant = false;
            continue;
         }

         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

         bool src_is_const = false;
         nir_deref_instr *src_deref = NULL, *dst_deref = NULL;
         nir_component_mask_t write_mask = 0;
         switch (intrin->intrinsic) {
         case nir_intrinsic_store_deref:
            dst_deref = nir_src_as_deref(intrin->src[0]);
            src_is_const = nir_src_is_const(intrin->src[1]);
            write_mask = nir_intrinsic_write_mask(intrin);
            break;

         case nir_intrinsic_load_deref:
            src_deref = nir_src_as_deref(intrin->src[0]);
            break;

         case nir_intrinsic_copy_deref:
            assert(!"Lowering of copy_deref with large constants is prohibited");
            break;

         default:
            continue;
         }

         if (dst_deref && nir_deref_mode_must_be(dst_deref, nir_var_function_temp)) {
            nir_variable *var = nir_deref_instr_get_variable(dst_deref);
            if (var == NULL)
               continue;

            assert(var->data.mode == nir_var_function_temp);

            struct var_info *info = &var_infos[var->index];
            if (!info->is_constant)
               continue;

            if (!info->block)
               info->block = block;

            /* We only consider variables constant if they only have constant
             * stores, all the stores come before any reads, and all stores
             * come from the same block.  We also can't handle indirect stores.
             */
            if (!src_is_const || info->found_read || block != info->block ||
                nir_deref_instr_has_indirect(dst_deref)) {
               info->is_constant = false;
            } else {
               nir_const_value *val = nir_src_as_const_value(intrin->src[1]);
               handle_constant_store(var_infos, info, dst_deref, val, write_mask,
                                     size_align);
            }
         }

         if (src_deref && nir_deref_mode_must_be(src_deref, nir_var_function_temp)) {
            nir_variable *var = nir_deref_instr_get_variable(src_deref);
            if (var == NULL)
               continue;

            assert(var->data.mode == nir_var_function_temp);

            /* We only consider variables constant if all the reads are
             * dominated by the block that writes to it.
             */
            struct var_info *info = &var_infos[var->index];
            if (!info->is_constant)
               continue;

            if (!info->block || !nir_block_dominates(info->block, block))
               info->is_constant = false;

            info->found_read = true;
         }
      }
   }

   bool has_constant = false;

   /* Allocate constant data space for each variable that just has constant
    * data.  We sort them by size and content so we can easily find
    * duplicates.
    */
   const unsigned old_constant_data_size = shader->constant_data_size;
   qsort(var_infos, num_locals, sizeof(struct var_info), var_info_cmp);
   for (int i = 0; i < num_locals; i++) {
      struct var_info *info = &var_infos[i];

      /* Fix up indices after we sorted. */
      info->var->index = i;

      if (!info->is_constant)
         continue;

      get_small_constant(info, size_align);

      unsigned var_size, var_align;
      size_align(info->var->type, &var_size, &var_align);
      if ((var_size <= threshold && !info->is_small) || !info->found_read) {
         /* Don't bother lowering small stuff or data that's never read */
         info->is_constant = false;
         continue;
      }

      if (i > 0 && var_info_cmp(info, &var_infos[i - 1]) == 0) {
         info->var->data.location = var_infos[i - 1].var->data.location;
         info->duplicate = true;
      } else {
         info->var->data.location = ALIGN_POT(shader->constant_data_size, var_align);
         shader->constant_data_size = info->var->data.location + var_size;
      }

      has_constant |= info->is_constant;
   }

   if (!has_constant) {
      nir_shader_preserve_all_metadata(shader);
      ralloc_free(var_infos);
      return false;
   }

   if (shader->constant_data_size != old_constant_data_size) {
      assert(shader->constant_data_size > old_constant_data_size);
      shader->constant_data = rerzalloc_size(shader, shader->constant_data,
                                             old_constant_data_size,
                                             shader->constant_data_size);
      for (int i = 0; i < num_locals; i++) {
         struct var_info *info = &var_infos[i];
         if (!info->duplicate && info->is_constant) {
            memcpy((char *)shader->constant_data + info->var->data.location,
                   info->constant_data, info->constant_data_size);
         }
      }
   }

   nir_builder b = nir_builder_create(impl);

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

         switch (intrin->intrinsic) {
         case nir_intrinsic_load_deref: {
            nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
            if (!nir_deref_mode_is(deref, nir_var_function_temp))
               continue;

            nir_variable *var = nir_deref_instr_get_variable(deref);
            if (var == NULL)
               continue;

            struct var_info *info = &var_infos[var->index];
            if (info->is_small) {
               b.cursor = nir_after_instr(&intrin->instr);
               nir_def *val = build_small_constant_load(&b, deref, info, size_align);
               nir_def_rewrite_uses(&intrin->def, val);
               nir_instr_remove(&intrin->instr);
               nir_deref_instr_remove_if_unused(deref);
            } else if (info->is_constant) {
               b.cursor = nir_after_instr(&intrin->instr);
               nir_def *val = build_constant_load(&b, deref, size_align);
               nir_def_rewrite_uses(&intrin->def,
                                    val);
               nir_instr_remove(&intrin->instr);
               nir_deref_instr_remove_if_unused(deref);
            }
            break;
         }

         case nir_intrinsic_store_deref: {
            nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
            if (!nir_deref_mode_is(deref, nir_var_function_temp))
               continue;

            nir_variable *var = nir_deref_instr_get_variable(deref);
            if (var == NULL)
               continue;

            struct var_info *info = &var_infos[var->index];
            if (info->is_constant) {
               nir_instr_remove(&intrin->instr);
               nir_deref_instr_remove_if_unused(deref);
            }
            break;
         }
         case nir_intrinsic_copy_deref:
         default:
            continue;
         }
      }
   }

   /* Clean up the now unused variables */
   for (int i = 0; i < num_locals; i++) {
      struct var_info *info = &var_infos[i];
      if (info->is_constant)
         exec_node_remove(&info->var->node);
   }

   ralloc_free(var_infos);

   nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);
   return true;
}
