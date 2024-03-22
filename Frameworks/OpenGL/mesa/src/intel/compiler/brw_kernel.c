/*
 * Copyright © 2020 Corporation
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

#include "brw_kernel.h"
#include "brw_nir.h"

#include "nir_clc_helpers.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/spirv/nir_spirv.h"
#include "dev/intel_debug.h"
#include "util/u_atomic.h"

static const nir_shader *
load_clc_shader(struct brw_compiler *compiler, struct disk_cache *disk_cache,
                const nir_shader_compiler_options *nir_options,
                const struct spirv_to_nir_options *spirv_options)
{
   if (compiler->clc_shader)
      return compiler->clc_shader;

   nir_shader *nir =  nir_load_libclc_shader(64, disk_cache,
                                             spirv_options, nir_options,
                                             disk_cache != NULL);
   if (nir == NULL)
      return NULL;

   const nir_shader *old_nir =
      p_atomic_cmpxchg(&compiler->clc_shader, NULL, nir);
   if (old_nir == NULL) {
      /* We won the race */
      ralloc_steal(compiler, nir);
      return nir;
   } else {
      /* Someone else built the shader first */
      ralloc_free(nir);
      return old_nir;
   }
}

static nir_builder
builder_init_new_impl(nir_function *func)
{
   nir_function_impl *impl = nir_function_impl_create(func);
   return nir_builder_at(nir_before_impl(impl));
}

static void
implement_atomic_builtin(nir_function *func, nir_atomic_op atomic_op,
                         enum glsl_base_type data_base_type,
                         nir_variable_mode mode)
{
   nir_builder b = builder_init_new_impl(func);
   const struct glsl_type *data_type = glsl_scalar_type(data_base_type);

   unsigned p = 0;

   nir_deref_instr *ret = NULL;
   ret = nir_build_deref_cast(&b, nir_load_param(&b, p++),
                              nir_var_function_temp, data_type, 0);

   nir_intrinsic_op op = nir_intrinsic_deref_atomic;
   nir_intrinsic_instr *atomic = nir_intrinsic_instr_create(b.shader, op);
   nir_intrinsic_set_atomic_op(atomic, atomic_op);

   for (unsigned i = 0; i < nir_intrinsic_infos[op].num_srcs; i++) {
      nir_def *src = nir_load_param(&b, p++);
      if (i == 0) {
         /* The first source is our deref */
         assert(nir_intrinsic_infos[op].src_components[i] == -1);
         src = &nir_build_deref_cast(&b, src, mode, data_type, 0)->def;
      }
      atomic->src[i] = nir_src_for_ssa(src);
   }

   nir_def_init_for_type(&atomic->instr, &atomic->def, data_type);

   nir_builder_instr_insert(&b, &atomic->instr);
   nir_store_deref(&b, ret, &atomic->def, ~0);
}

static void
implement_sub_group_ballot_builtin(nir_function *func)
{
   nir_builder b = builder_init_new_impl(func);
   nir_deref_instr *ret =
      nir_build_deref_cast(&b, nir_load_param(&b, 0),
                           nir_var_function_temp, glsl_uint_type(), 0);
   nir_def *cond = nir_load_param(&b, 1);

   nir_intrinsic_instr *ballot =
      nir_intrinsic_instr_create(b.shader, nir_intrinsic_ballot);
   ballot->src[0] = nir_src_for_ssa(cond);
   ballot->num_components = 1;
   nir_def_init(&ballot->instr, &ballot->def, 1, 32);
   nir_builder_instr_insert(&b, &ballot->instr);

   nir_store_deref(&b, ret, &ballot->def, ~0);
}

static bool
implement_intel_builtins(nir_shader *nir)
{
   bool progress = false;

   nir_foreach_function(func, nir) {
      if (strcmp(func->name, "_Z10atomic_minPU3AS1Vff") == 0) {
         /* float atom_min(__global float volatile *p, float val) */
         implement_atomic_builtin(func, nir_atomic_op_fmin,
                                  GLSL_TYPE_FLOAT, nir_var_mem_global);
         progress = true;
      } else if (strcmp(func->name, "_Z10atomic_maxPU3AS1Vff") == 0) {
         /* float atom_max(__global float volatile *p, float val) */
         implement_atomic_builtin(func, nir_atomic_op_fmax,
                                  GLSL_TYPE_FLOAT, nir_var_mem_global);
         progress = true;
      } else if (strcmp(func->name, "_Z10atomic_minPU3AS3Vff") == 0) {
         /* float atomic_min(__shared float volatile *, float) */
         implement_atomic_builtin(func, nir_atomic_op_fmin,
                                  GLSL_TYPE_FLOAT, nir_var_mem_shared);
         progress = true;
      } else if (strcmp(func->name, "_Z10atomic_maxPU3AS3Vff") == 0) {
         /* float atomic_max(__shared float volatile *, float) */
         implement_atomic_builtin(func, nir_atomic_op_fmax,
                                  GLSL_TYPE_FLOAT, nir_var_mem_shared);
         progress = true;
      } else if (strcmp(func->name, "intel_sub_group_ballot") == 0) {
         implement_sub_group_ballot_builtin(func);
         progress = true;
      }
   }

   nir_shader_preserve_all_metadata(nir);

   return progress;
}

static bool
lower_kernel_intrinsics(nir_shader *nir)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);

   bool progress = false;

   unsigned kernel_sysvals_start = 0;
   unsigned kernel_arg_start = sizeof(struct brw_kernel_sysvals);
   nir->num_uniforms += kernel_arg_start;

   nir_builder b = nir_builder_create(impl);

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         switch (intrin->intrinsic) {
         case nir_intrinsic_load_kernel_input: {
            b.cursor = nir_instr_remove(&intrin->instr);

            nir_intrinsic_instr *load =
               nir_intrinsic_instr_create(nir, nir_intrinsic_load_uniform);
            load->num_components = intrin->num_components;
            load->src[0] = nir_src_for_ssa(nir_u2u32(&b, intrin->src[0].ssa));
            nir_intrinsic_set_base(load, kernel_arg_start);
            nir_intrinsic_set_range(load, nir->num_uniforms);
            nir_def_init(&load->instr, &load->def,
                         intrin->def.num_components,
                         intrin->def.bit_size);
            nir_builder_instr_insert(&b, &load->instr);

            nir_def_rewrite_uses(&intrin->def, &load->def);
            progress = true;
            break;
         }

         case nir_intrinsic_load_constant_base_ptr: {
            b.cursor = nir_instr_remove(&intrin->instr);
            nir_def *const_data_base_addr = nir_pack_64_2x32_split(&b,
               nir_load_reloc_const_intel(&b, BRW_SHADER_RELOC_CONST_DATA_ADDR_LOW),
               nir_load_reloc_const_intel(&b, BRW_SHADER_RELOC_CONST_DATA_ADDR_HIGH));
            nir_def_rewrite_uses(&intrin->def, const_data_base_addr);
            progress = true;
            break;
         }

         case nir_intrinsic_load_num_workgroups: {
            b.cursor = nir_instr_remove(&intrin->instr);

            nir_intrinsic_instr *load =
               nir_intrinsic_instr_create(nir, nir_intrinsic_load_uniform);
            load->num_components = 3;
            load->src[0] = nir_src_for_ssa(nir_imm_int(&b, 0));
            nir_intrinsic_set_base(load, kernel_sysvals_start +
               offsetof(struct brw_kernel_sysvals, num_work_groups));
            nir_intrinsic_set_range(load, 3 * 4);
            nir_def_init(&load->instr, &load->def, 3, 32);
            nir_builder_instr_insert(&b, &load->instr);
            nir_def_rewrite_uses(&intrin->def, &load->def);
            progress = true;
            break;
         }

         default:
            break;
         }
      }
   }

   if (progress) {
      nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);
   } else {
      nir_metadata_preserve(impl, nir_metadata_all);
   }

   return progress;
}

bool
brw_kernel_from_spirv(struct brw_compiler *compiler,
                      struct disk_cache *disk_cache,
                      struct brw_kernel *kernel,
                      void *log_data, void *mem_ctx,
                      const uint32_t *spirv, size_t spirv_size,
                      const char *entrypoint_name,
                      char **error_str)
{
   const struct intel_device_info *devinfo = compiler->devinfo;
   const nir_shader_compiler_options *nir_options =
      compiler->nir_options[MESA_SHADER_KERNEL];

   struct spirv_to_nir_options spirv_options = {
      .environment = NIR_SPIRV_OPENCL,
      .caps = {
         .address = true,
         .float16 = devinfo->ver >= 8,
         .float64 = devinfo->ver >= 8,
         .groups = true,
         .image_write_without_format = true,
         .int8 = devinfo->ver >= 8,
         .int16 = devinfo->ver >= 8,
         .int64 = devinfo->ver >= 8,
         .int64_atomics = devinfo->ver >= 9,
         .kernel = true,
         .linkage = true, /* We receive linked kernel from clc */
         .float_controls = devinfo->ver >= 8,
         .generic_pointers = true,
         .storage_8bit = devinfo->ver >= 8,
         .storage_16bit = devinfo->ver >= 8,
         .subgroup_arithmetic = true,
         .subgroup_basic = true,
         .subgroup_ballot = true,
         .subgroup_dispatch = true,
         .subgroup_quad = true,
         .subgroup_shuffle = true,
         .subgroup_vote = true,

         .intel_subgroup_shuffle = true,
         .intel_subgroup_buffer_block_io = true,
      },
      .shared_addr_format = nir_address_format_62bit_generic,
      .global_addr_format = nir_address_format_62bit_generic,
      .temp_addr_format = nir_address_format_62bit_generic,
      .constant_addr_format = nir_address_format_64bit_global,
   };

   spirv_options.clc_shader = load_clc_shader(compiler, disk_cache,
                                              nir_options, &spirv_options);
   if (spirv_options.clc_shader == NULL) {
      fprintf(stderr, "ERROR: libclc shader missing."
              " Consider installing the libclc package\n");
      abort();
   }

   assert(spirv_size % 4 == 0);
   nir_shader *nir =
      spirv_to_nir(spirv, spirv_size / 4, NULL, 0, MESA_SHADER_KERNEL,
                   entrypoint_name, &spirv_options, nir_options);
   nir_validate_shader(nir, "after spirv_to_nir");
   nir_validate_ssa_dominance(nir, "after spirv_to_nir");
   ralloc_steal(mem_ctx, nir);
   nir->info.name = ralloc_strdup(nir, entrypoint_name);

   if (INTEL_DEBUG(DEBUG_CS)) {
      /* Re-index SSA defs so we print more sensible numbers. */
      nir_foreach_function_impl(impl, nir) {
         nir_index_ssa_defs(impl);
      }

      fprintf(stderr, "NIR (from SPIR-V) for kernel\n");
      nir_print_shader(nir, stderr);
   }

   NIR_PASS_V(nir, implement_intel_builtins);
   NIR_PASS_V(nir, nir_link_shader_functions, spirv_options.clc_shader);

   /* We have to lower away local constant initializers right before we
    * inline functions.  That way they get properly initialized at the top
    * of the function and not at the top of its caller.
    */
   NIR_PASS_V(nir, nir_lower_variable_initializers, nir_var_function_temp);
   NIR_PASS_V(nir, nir_lower_returns);
   NIR_PASS_V(nir, nir_inline_functions);
   NIR_PASS_V(nir, nir_copy_prop);
   NIR_PASS_V(nir, nir_opt_deref);

   /* Pick off the single entrypoint that we want */
   nir_remove_non_entrypoints(nir);

   /* Now that we've deleted all but the main function, we can go ahead and
    * lower the rest of the constant initializers.  We do this here so that
    * nir_remove_dead_variables and split_per_member_structs below see the
    * corresponding stores.
    */
   NIR_PASS_V(nir, nir_lower_variable_initializers, ~0);

   /* LLVM loves take advantage of the fact that vec3s in OpenCL are 16B
    * aligned and so it can just read/write them as vec4s.  This results in a
    * LOT of vec4->vec3 casts on loads and stores.  One solution to this
    * problem is to get rid of all vec3 variables.
    */
   NIR_PASS_V(nir, nir_lower_vec3_to_vec4,
              nir_var_shader_temp | nir_var_function_temp |
              nir_var_mem_shared | nir_var_mem_global|
              nir_var_mem_constant);

   /* We assign explicit types early so that the optimizer can take advantage
    * of that information and hopefully get rid of some of our memcpys.
    */
   NIR_PASS_V(nir, nir_lower_vars_to_explicit_types,
              nir_var_uniform |
              nir_var_shader_temp | nir_var_function_temp |
              nir_var_mem_shared | nir_var_mem_global,
              glsl_get_cl_type_size_align);

   struct brw_nir_compiler_opts opts = {};
   brw_preprocess_nir(compiler, nir, &opts);

   int max_arg_idx = -1;
   nir_foreach_uniform_variable(var, nir) {
      assert(var->data.location < 256);
      max_arg_idx = MAX2(max_arg_idx, var->data.location);
   }

   kernel->args_size = nir->num_uniforms;
   kernel->arg_count = max_arg_idx + 1;

   /* No bindings */
   struct brw_kernel_arg_desc *args =
      rzalloc_array(mem_ctx, struct brw_kernel_arg_desc, kernel->arg_count);
   kernel->args = args;

   nir_foreach_uniform_variable(var, nir) {
      struct brw_kernel_arg_desc arg_desc = {
         .offset = var->data.driver_location,
         .size = glsl_get_explicit_size(var->type, false),
      };
      assert(arg_desc.offset + arg_desc.size <= nir->num_uniforms);

      assert(var->data.location >= 0);
      args[var->data.location] = arg_desc;
   }

   NIR_PASS_V(nir, nir_remove_dead_variables, nir_var_all, NULL);

   /* Lower again, this time after dead-variables to get more compact variable
    * layouts.
    */
   nir->global_mem_size = 0;
   nir->scratch_size = 0;
   nir->info.shared_size = 0;
   NIR_PASS_V(nir, nir_lower_vars_to_explicit_types,
              nir_var_shader_temp | nir_var_function_temp |
              nir_var_mem_shared | nir_var_mem_global | nir_var_mem_constant,
              glsl_get_cl_type_size_align);
   if (nir->constant_data_size > 0) {
      assert(nir->constant_data == NULL);
      nir->constant_data = rzalloc_size(nir, nir->constant_data_size);
      nir_gather_explicit_io_initializers(nir, nir->constant_data,
                                          nir->constant_data_size,
                                          nir_var_mem_constant);
   }

   if (INTEL_DEBUG(DEBUG_CS)) {
      /* Re-index SSA defs so we print more sensible numbers. */
      nir_foreach_function_impl(impl, nir) {
         nir_index_ssa_defs(impl);
      }

      fprintf(stderr, "NIR (before I/O lowering) for kernel\n");
      nir_print_shader(nir, stderr);
   }

   NIR_PASS_V(nir, nir_lower_memcpy);

   NIR_PASS_V(nir, nir_lower_explicit_io, nir_var_mem_constant,
              nir_address_format_64bit_global);

   NIR_PASS_V(nir, nir_lower_explicit_io, nir_var_uniform,
              nir_address_format_32bit_offset_as_64bit);

   NIR_PASS_V(nir, nir_lower_explicit_io,
              nir_var_shader_temp | nir_var_function_temp |
              nir_var_mem_shared | nir_var_mem_global,
              nir_address_format_62bit_generic);

   NIR_PASS_V(nir, nir_lower_convert_alu_types, NULL);

   NIR_PASS_V(nir, brw_nir_lower_cs_intrinsics);
   NIR_PASS_V(nir, lower_kernel_intrinsics);

   struct brw_cs_prog_key key = { };

   memset(&kernel->prog_data, 0, sizeof(kernel->prog_data));
   kernel->prog_data.base.nr_params = DIV_ROUND_UP(nir->num_uniforms, 4);

   struct brw_compile_cs_params params = {
      .base = {
         .nir = nir,
         .stats = kernel->stats,
         .log_data = log_data,
         .mem_ctx = mem_ctx,
      },
      .key = &key,
      .prog_data = &kernel->prog_data,
   };

   kernel->code = brw_compile_cs(compiler, &params);

   if (error_str)
      *error_str = params.base.error_str;

   return kernel->code != NULL;
}
