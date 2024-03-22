/*
 * Copyright © 2015-2016 Intel Corporation
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

#include "brw_compiler.h"
#include "brw_shader.h"
#include "brw_eu.h"
#include "dev/intel_debug.h"
#include "compiler/nir/nir.h"
#include "util/u_debug.h"

#define COMMON_OPTIONS                                                        \
   .has_uclz = true,                                                          \
   .lower_fdiv = true,                                                        \
   .lower_scmp = true,                                                        \
   .lower_flrp16 = true,                                                      \
   .lower_fmod = true,                                                        \
   .lower_ufind_msb = true,                                                   \
   .lower_uadd_carry = true,                                                  \
   .lower_usub_borrow = true,                                                 \
   .lower_flrp64 = true,                                                      \
   .lower_fisnormal = true,                                                   \
   .lower_isign = true,                                                       \
   .lower_ldexp = true,                                                       \
   .lower_bitfield_extract = true,                                            \
   .lower_bitfield_insert = true,                                             \
   .lower_device_index_to_zero = true,                                        \
   .vectorize_io = true,                                                      \
   .vectorize_tess_levels = true,                                             \
   .use_interpolated_input_intrinsics = true,                                 \
   .lower_insert_byte = true,                                                 \
   .lower_insert_word = true,                                                 \
   .vertex_id_zero_based = true,                                              \
   .lower_base_vertex = true,                                                 \
   .support_16bit_alu = true,                                                 \
   .lower_uniforms_to_ubo = true

#define COMMON_SCALAR_OPTIONS                                                 \
   .lower_to_scalar = true,                                                   \
   .lower_pack_half_2x16 = true,                                              \
   .lower_pack_snorm_2x16 = true,                                             \
   .lower_pack_snorm_4x8 = true,                                              \
   .lower_pack_unorm_2x16 = true,                                             \
   .lower_pack_unorm_4x8 = true,                                              \
   .lower_unpack_half_2x16 = true,                                            \
   .lower_unpack_snorm_2x16 = true,                                           \
   .lower_unpack_snorm_4x8 = true,                                            \
   .lower_unpack_unorm_2x16 = true,                                           \
   .lower_unpack_unorm_4x8 = true,                                            \
   .lower_hadd64 = true,                                                      \
   .avoid_ternary_with_two_constants = true,                                  \
   .has_pack_32_4x8 = true,                                                   \
   .max_unroll_iterations = 32,                                               \
   .force_indirect_unrolling = nir_var_function_temp,                         \
   .divergence_analysis_options =                                             \
      (nir_divergence_single_patch_per_tcs_subgroup |                         \
       nir_divergence_single_patch_per_tes_subgroup |                         \
       nir_divergence_shader_record_ptr_uniform)

static const struct nir_shader_compiler_options scalar_nir_options = {
   COMMON_OPTIONS,
   COMMON_SCALAR_OPTIONS,
};

static const struct nir_shader_compiler_options vector_nir_options = {
   COMMON_OPTIONS,

   /* In the vec4 backend, our dpN instruction replicates its result to all the
    * components of a vec4.  We would like NIR to give us replicated fdot
    * instructions because it can optimize better for us.
    */
   .fdot_replicates = true,

   .lower_usub_sat = true,
   .lower_pack_snorm_2x16 = true,
   .lower_pack_unorm_2x16 = true,
   .lower_unpack_snorm_2x16 = true,
   .lower_unpack_unorm_2x16 = true,
   .lower_extract_byte = true,
   .lower_extract_word = true,
   .intel_vec4 = true,
   .max_unroll_iterations = 32,
};

struct brw_compiler *
brw_compiler_create(void *mem_ctx, const struct intel_device_info *devinfo)
{
   struct brw_compiler *compiler = rzalloc(mem_ctx, struct brw_compiler);

   compiler->devinfo = devinfo;

   brw_init_isa_info(&compiler->isa, devinfo);

   brw_fs_alloc_reg_sets(compiler);
   if (devinfo->ver < 8)
      brw_vec4_alloc_reg_set(compiler);

   compiler->precise_trig = debug_get_bool_option("INTEL_PRECISE_TRIG", false);

   compiler->use_tcs_multi_patch = devinfo->ver >= 12;

   /* Default to the sampler since that's what we've done since forever */
   compiler->indirect_ubos_use_sampler = true;

   compiler->lower_dpas = devinfo->verx10 < 125 ||
      intel_device_info_is_mtl(devinfo) ||
      debug_get_bool_option("INTEL_LOWER_DPAS", false);

   /* There is no vec4 mode on Gfx10+, and we don't use it at all on Gfx8+. */
   for (int i = MESA_SHADER_VERTEX; i < MESA_ALL_SHADER_STAGES; i++) {
      compiler->scalar_stage[i] = devinfo->ver >= 8 ||
         i == MESA_SHADER_FRAGMENT || i == MESA_SHADER_COMPUTE;
   }

   for (int i = MESA_SHADER_TASK; i < MESA_VULKAN_SHADER_STAGES; i++)
      compiler->scalar_stage[i] = true;

   nir_lower_int64_options int64_options =
      nir_lower_imul64 |
      nir_lower_isign64 |
      nir_lower_divmod64 |
      nir_lower_imul_high64 |
      nir_lower_find_lsb64 |
      nir_lower_ufind_msb64 |
      nir_lower_bit_count64;
   nir_lower_doubles_options fp64_options =
      nir_lower_drcp |
      nir_lower_dsqrt |
      nir_lower_drsq |
      nir_lower_dtrunc |
      nir_lower_dfloor |
      nir_lower_dceil |
      nir_lower_dfract |
      nir_lower_dround_even |
      nir_lower_dmod |
      nir_lower_dsub |
      nir_lower_ddiv;

   if (!devinfo->has_64bit_float || INTEL_DEBUG(DEBUG_SOFT64))
      fp64_options |= nir_lower_fp64_full_software;
   if (!devinfo->has_64bit_int)
      int64_options |= (nir_lower_int64_options)~0;

   /* The Bspec's section titled "Instruction_multiply[DevBDW+]" claims that
    * destination type can be Quadword and source type Doubleword for Gfx8 and
    * Gfx9. So, lower 64 bit multiply instruction on rest of the platforms.
    */
   if (devinfo->ver < 8 || devinfo->ver > 9)
      int64_options |= nir_lower_imul_2x32_64;

   /* We want the GLSL compiler to emit code that uses condition codes */
   for (int i = 0; i < MESA_ALL_SHADER_STAGES; i++) {
      struct nir_shader_compiler_options *nir_options =
         rzalloc(compiler, struct nir_shader_compiler_options);
      bool is_scalar = compiler->scalar_stage[i];
      if (is_scalar) {
         *nir_options = scalar_nir_options;
         int64_options |= nir_lower_usub_sat64;
      } else {
         *nir_options = vector_nir_options;
      }

      /* Prior to Gfx6, there are no three source operations, and Gfx11 loses
       * LRP.
       */
      nir_options->lower_ffma16 = devinfo->ver < 6;
      nir_options->lower_ffma32 = devinfo->ver < 6;
      nir_options->lower_ffma64 = devinfo->ver < 6;
      nir_options->lower_flrp32 = devinfo->ver < 6 || devinfo->ver >= 11;
      nir_options->lower_fpow = devinfo->ver >= 12;

      nir_options->has_bfe = devinfo->ver >= 7;
      nir_options->has_bfm = devinfo->ver >= 7;
      nir_options->has_bfi = devinfo->ver >= 7;

      nir_options->has_rotate16 = devinfo->ver >= 11;
      nir_options->has_rotate32 = devinfo->ver >= 11;
      nir_options->lower_bitfield_reverse = devinfo->ver < 7;
      nir_options->lower_find_lsb = devinfo->ver < 7;
      nir_options->lower_ifind_msb = devinfo->ver < 7;
      nir_options->has_iadd3 = devinfo->verx10 >= 125;

      nir_options->has_sdot_4x8 = devinfo->ver >= 12;
      nir_options->has_udot_4x8 = devinfo->ver >= 12;
      nir_options->has_sudot_4x8 = devinfo->ver >= 12;
      nir_options->has_sdot_4x8_sat = devinfo->ver >= 12;
      nir_options->has_udot_4x8_sat = devinfo->ver >= 12;
      nir_options->has_sudot_4x8_sat = devinfo->ver >= 12;

      nir_options->lower_int64_options = int64_options;
      nir_options->lower_doubles_options = fp64_options;

      nir_options->unify_interfaces = i < MESA_SHADER_FRAGMENT;

      nir_options->force_indirect_unrolling |=
         brw_nir_no_indirect_mask(compiler, i);
      nir_options->force_indirect_unrolling_sampler = devinfo->ver < 7;

      if (compiler->use_tcs_multi_patch) {
         /* TCS MULTI_PATCH mode has multiple patches per subgroup */
         nir_options->divergence_analysis_options &=
            ~nir_divergence_single_patch_per_tcs_subgroup;
      }

      if (devinfo->ver < 12)
         nir_options->divergence_analysis_options |=
            nir_divergence_single_prim_per_subgroup;

      compiler->nir_options[i] = nir_options;
   }

   compiler->mesh.mue_header_packing =
         (unsigned)debug_get_num_option("INTEL_MESH_HEADER_PACKING", 3);
   compiler->mesh.mue_compaction =
         debug_get_bool_option("INTEL_MESH_COMPACTION", true);

   return compiler;
}

static void
insert_u64_bit(uint64_t *val, bool add)
{
   *val = (*val << 1) | !!add;
}

uint64_t
brw_get_compiler_config_value(const struct brw_compiler *compiler)
{
   uint64_t config = 0;
   unsigned bits = 0;

   insert_u64_bit(&config, compiler->precise_trig);
   bits++;
   insert_u64_bit(&config, compiler->lower_dpas);
   bits++;
   insert_u64_bit(&config, compiler->mesh.mue_compaction);
   bits++;

   uint64_t mask = DEBUG_DISK_CACHE_MASK;
   bits += util_bitcount64(mask);
   while (mask != 0) {
      const uint64_t bit = 1ULL << (ffsll(mask) - 1);
      insert_u64_bit(&config, INTEL_DEBUG(bit));
      mask &= ~bit;
   }

   mask = SIMD_DISK_CACHE_MASK;
   bits += util_bitcount64(mask);
   while (mask != 0) {
      const uint64_t bit = 1ULL << (ffsll(mask) - 1);
      insert_u64_bit(&config, (intel_simd & bit) != 0);
      mask &= ~bit;
   }

   mask = 3;
   bits += util_bitcount64(mask);

   u_foreach_bit64(bit, mask)
      insert_u64_bit(&config, (compiler->mesh.mue_header_packing & (1ULL << bit)) != 0);

   assert(bits <= util_bitcount64(UINT64_MAX));

   return config;
}

unsigned
brw_prog_data_size(gl_shader_stage stage)
{
   static const size_t stage_sizes[] = {
      [MESA_SHADER_VERTEX]       = sizeof(struct brw_vs_prog_data),
      [MESA_SHADER_TESS_CTRL]    = sizeof(struct brw_tcs_prog_data),
      [MESA_SHADER_TESS_EVAL]    = sizeof(struct brw_tes_prog_data),
      [MESA_SHADER_GEOMETRY]     = sizeof(struct brw_gs_prog_data),
      [MESA_SHADER_FRAGMENT]     = sizeof(struct brw_wm_prog_data),
      [MESA_SHADER_COMPUTE]      = sizeof(struct brw_cs_prog_data),
      [MESA_SHADER_TASK]         = sizeof(struct brw_task_prog_data),
      [MESA_SHADER_MESH]         = sizeof(struct brw_mesh_prog_data),
      [MESA_SHADER_RAYGEN]       = sizeof(struct brw_bs_prog_data),
      [MESA_SHADER_ANY_HIT]      = sizeof(struct brw_bs_prog_data),
      [MESA_SHADER_CLOSEST_HIT]  = sizeof(struct brw_bs_prog_data),
      [MESA_SHADER_MISS]         = sizeof(struct brw_bs_prog_data),
      [MESA_SHADER_INTERSECTION] = sizeof(struct brw_bs_prog_data),
      [MESA_SHADER_CALLABLE]     = sizeof(struct brw_bs_prog_data),
      [MESA_SHADER_KERNEL]       = sizeof(struct brw_cs_prog_data),
   };
   assert((int)stage >= 0 && stage < ARRAY_SIZE(stage_sizes));
   return stage_sizes[stage];
}

unsigned
brw_prog_key_size(gl_shader_stage stage)
{
   static const size_t stage_sizes[] = {
      [MESA_SHADER_VERTEX]       = sizeof(struct brw_vs_prog_key),
      [MESA_SHADER_TESS_CTRL]    = sizeof(struct brw_tcs_prog_key),
      [MESA_SHADER_TESS_EVAL]    = sizeof(struct brw_tes_prog_key),
      [MESA_SHADER_GEOMETRY]     = sizeof(struct brw_gs_prog_key),
      [MESA_SHADER_FRAGMENT]     = sizeof(struct brw_wm_prog_key),
      [MESA_SHADER_COMPUTE]      = sizeof(struct brw_cs_prog_key),
      [MESA_SHADER_TASK]         = sizeof(struct brw_task_prog_key),
      [MESA_SHADER_MESH]         = sizeof(struct brw_mesh_prog_key),
      [MESA_SHADER_RAYGEN]       = sizeof(struct brw_bs_prog_key),
      [MESA_SHADER_ANY_HIT]      = sizeof(struct brw_bs_prog_key),
      [MESA_SHADER_CLOSEST_HIT]  = sizeof(struct brw_bs_prog_key),
      [MESA_SHADER_MISS]         = sizeof(struct brw_bs_prog_key),
      [MESA_SHADER_INTERSECTION] = sizeof(struct brw_bs_prog_key),
      [MESA_SHADER_CALLABLE]     = sizeof(struct brw_bs_prog_key),
      [MESA_SHADER_KERNEL]       = sizeof(struct brw_cs_prog_key),
   };
   assert((int)stage >= 0 && stage < ARRAY_SIZE(stage_sizes));
   return stage_sizes[stage];
}

void
brw_write_shader_relocs(const struct brw_isa_info *isa,
                        void *program,
                        const struct brw_stage_prog_data *prog_data,
                        struct brw_shader_reloc_value *values,
                        unsigned num_values)
{
   for (unsigned i = 0; i < prog_data->num_relocs; i++) {
      assert(prog_data->relocs[i].offset % 8 == 0);
      void *dst = program + prog_data->relocs[i].offset;
      for (unsigned j = 0; j < num_values; j++) {
         if (prog_data->relocs[i].id == values[j].id) {
            uint32_t value = values[j].value + prog_data->relocs[i].delta;
            switch (prog_data->relocs[i].type) {
            case BRW_SHADER_RELOC_TYPE_U32:
               *(uint32_t *)dst = value;
               break;
            case BRW_SHADER_RELOC_TYPE_MOV_IMM:
               brw_update_reloc_imm(isa, dst, value);
               break;
            default:
               unreachable("Invalid relocation type");
            }
            break;
         }
      }
   }
}
