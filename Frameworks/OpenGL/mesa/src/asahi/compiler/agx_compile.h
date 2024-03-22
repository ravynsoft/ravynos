/*
 * Copyright 2018-2021 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef __AGX_PUBLIC_H_
#define __AGX_PUBLIC_H_

#include "compiler/nir/nir.h"
#include "util/u_dynarray.h"

/* 32 user varyings + some system values */
#define AGX_MAX_VARYING_SLOTS (48)

struct agx_varyings_vs {
   /* The number of user varyings of each type. The varyings must be allocated
    * in this order ({smooth, flat, linear} × {32, 16}), which may require
    * remapping.
    */
   unsigned num_32_smooth;
   unsigned num_32_flat;
   unsigned num_32_linear;
   unsigned num_16_smooth;
   unsigned num_16_flat;
   unsigned num_16_linear;

   /* The first index used for FP16 varyings. Indices less than this are treated
    * as FP32. This may require remapping slots to guarantee.
    */
   unsigned base_index_fp16;

   /* The total number of vertex shader indices output. Must be at least
    * base_index_fp16.
    */
   unsigned nr_index;

   /* If the slot is written, this is the base index that the first component
    * of the slot is written to.  The next components are found in the next
    * indices. If less than base_index_fp16, this is a 32-bit slot (with 4
    * indices for the 4 components), else this is a 16-bit slot (with 2
    * indices for the 4 components). This must be less than nr_index.
    *
    * If the slot is not written, this must be ~0.
    */
   unsigned slots[AGX_MAX_VARYING_SLOTS];

   /* Slot for the combined layer/viewport 32-bit sysval output, or ~0 if none
    * is written. What's at slots[VARYING_SLOT_LAYER] is the varying output.
    */
   unsigned layer_viewport_slot;
};

/* Conservative bound, * 4 due to offsets (TODO: maybe worth eliminating
 * coefficient register aliasing?)
 */
#define AGX_MAX_CF_BINDINGS (AGX_MAX_VARYING_SLOTS * 4)

struct agx_varyings_fs {
   /* Number of coefficient registers used */
   unsigned nr_cf;

   /* Number of coefficient register bindings */
   unsigned nr_bindings;

   /* Whether gl_FragCoord.z is read */
   bool reads_z;

   /* Coefficient register bindings */
   struct {
      /* Base coefficient register */
      unsigned cf_base;

      /* Slot being bound */
      gl_varying_slot slot;

      /* First component bound.
       *
       * Must be 2 (Z) or 3 (W) if slot == VARYING_SLOT_POS.
       */
      unsigned offset : 2;

      /* Number of components bound */
      unsigned count : 3;

      /* Is smooth shading enabled? If false, flat shading is used */
      bool smooth : 1;

      /* Perspective correct interpolation */
      bool perspective : 1;
   } bindings[AGX_MAX_CF_BINDINGS];
};

union agx_varyings {
   struct agx_varyings_vs vs;
   struct agx_varyings_fs fs;
};

struct agx_uncompiled_shader_info {
   uint64_t inputs_flat_shaded;
   uint64_t inputs_linear_shaded;
   uint8_t cull_distance_size;
   bool has_edgeflags;
};

struct agx_shader_info {
   union agx_varyings varyings;

   /* Number of uniforms */
   unsigned push_count;

   /* Local memory allocation in bytes */
   unsigned local_size;

   /* Does the shader have a preamble? If so, it is at offset preamble_offset.
    * The main shader is at offset main_offset. The preamble is executed first.
    */
   bool has_preamble;
   unsigned preamble_offset, main_offset;

   /* Does the shader read the tilebuffer? */
   bool reads_tib;

   /* Does the shader write point size? */
   bool writes_psiz;

   /* Does the shader potentially draw to a nonzero viewport? */
   bool nonzero_viewport;

   /* Does the shader write layer and/or viewport index? Written together */
   bool writes_layer_viewport;

   /* Does the shader control the sample mask? */
   bool writes_sample_mask;

   /* Depth layout, never equal to NONE */
   enum gl_frag_depth_layout depth_layout;

   /* Based only the compiled shader, should tag writes be disabled? This is set
    * based on what is outputted. Note if rasterizer discard is used, that needs
    * to disable tag writes regardless of this flag.
    */
   bool tag_write_disable;

   /* Shader is incompatible with triangle merging */
   bool disable_tri_merging;

   /* Reads draw ID system value */
   bool uses_draw_id;

   /* Reads base vertex/instance */
   bool uses_base_param;

   /* Shader uses txf, requiring a workaround sampler in the given location */
   bool uses_txf;
   unsigned txf_sampler;

   /* Number of bindful textures, images used */
   unsigned nr_bindful_textures, nr_bindful_images;

   /* Number of 16-bit registers used by the main shader and preamble
    * respectively.
    */
   unsigned nr_gprs, nr_preamble_gprs;
};

#define AGX_MAX_RTS (8)

enum agx_format {
   AGX_FORMAT_I8 = 0,
   AGX_FORMAT_I16 = 1,
   AGX_FORMAT_I32 = 2,
   AGX_FORMAT_F16 = 3,
   AGX_FORMAT_U8NORM = 4,
   AGX_FORMAT_S8NORM = 5,
   AGX_FORMAT_U16NORM = 6,
   AGX_FORMAT_S16NORM = 7,
   AGX_FORMAT_RGB10A2 = 8,
   AGX_FORMAT_SRGBA8 = 10,
   AGX_FORMAT_RG11B10F = 12,
   AGX_FORMAT_RGB9E5 = 13,

   /* Keep last */
   AGX_NUM_FORMATS,
};

struct agx_vs_shader_key {
   /* The GPU ABI requires all smooth shaded varyings to come first, then all
    * flat shaded varyings, then all linear shaded varyings, as written by the
    * VS. In order to correctly remap the varyings into the right order in the
    * VS, we need to propagate the mask of flat/linear shaded varyings into the
    * compiler.
    */
   uint64_t outputs_flat_shaded;
   uint64_t outputs_linear_shaded;
};

struct agx_fs_shader_key {
   /* Normally, access to the tilebuffer must be guarded by appropriate fencing
    * instructions to ensure correct results in the presence of out-of-order
    * hardware optimizations. However, specially dispatched clear shaders are
    * not subject to these conditions and can omit the wait instructions.
    *
    * Must (only) be set for special clear shaders.
    *
    * Must not be used with sample mask writes (including discards) or
    * tilebuffer loads (including blending).
    */
   bool ignore_tib_dependencies;

   /* In a monolithic fragment shader or in a fragment epilogue, the number of
    * samples in the tilebuffer. In a non-monolithic fragment shader, leave
    * zero. This is used for the correct lowering of sample_mask instructions,
    * to ensure that all samples are written out. Can be set conservatively.
    */
   unsigned nr_samples;
};

struct agx_shader_key {
   /* Number of reserved preamble slots at the start */
   unsigned reserved_preamble;

   /* Does the target GPU need explicit cluster coherency for atomics?
    * Only used on G13X.
    */
   bool needs_g13x_coherency;

   /* Library routines to link against */
   const nir_shader *libagx;

   union {
      struct agx_vs_shader_key vs;
      struct agx_fs_shader_key fs;
   };
};

/* Texture backend flags */
#define AGX_TEXTURE_FLAG_NO_CLAMP (1 << 0)

bool agx_nir_lower_texture_early(nir_shader *s, bool support_lod_bias);

void agx_preprocess_nir(nir_shader *nir, const nir_shader *libagx,
                        bool allow_mediump,
                        struct agx_uncompiled_shader_info *out);

bool agx_nir_lower_discard_zs_emit(nir_shader *s);

void agx_nir_lower_cull_distance_fs(struct nir_shader *s,
                                    unsigned nr_distances);

bool agx_nir_needs_texture_crawl(nir_instr *instr);

void agx_compile_shader_nir(nir_shader *nir, struct agx_shader_key *key,
                            struct util_debug_callback *debug,
                            struct util_dynarray *binary,
                            struct agx_shader_info *out);

struct agx_occupancy {
   unsigned max_registers;
   unsigned max_threads;
};

struct agx_occupancy agx_occupancy_for_register_count(unsigned halfregs);

static const nir_shader_compiler_options agx_nir_options = {
   .lower_fdiv = true,
   .fuse_ffma16 = true,
   .fuse_ffma32 = true,
   .lower_flrp16 = true,
   .lower_flrp32 = true,
   .lower_fpow = true,
   .lower_fmod = true,
   .lower_bitfield_insert = true,
   .lower_ifind_msb = true,
   .lower_find_lsb = true,
   .lower_uadd_carry = true,
   .lower_usub_borrow = true,
   .lower_fisnormal = true,
   .lower_scmp = true,
   .lower_isign = true,
   .lower_fsign = true,
   .lower_iabs = true,
   .lower_fdph = true,
   .lower_ffract = true,
   .lower_ldexp = true,
   .lower_pack_half_2x16 = true,
   .lower_pack_64_2x32 = true,
   .lower_unpack_half_2x16 = true,
   .lower_extract_byte = true,
   .lower_insert_byte = true,
   .lower_insert_word = true,
   .has_cs_global_id = true,
   .lower_hadd = true,
   .vectorize_io = true,
   .use_interpolated_input_intrinsics = true,
   .has_isub = true,
   .support_16bit_alu = true,
   .max_unroll_iterations = 32,
   .lower_uniforms_to_ubo = true,
   .force_indirect_unrolling_sampler = true,
   .force_indirect_unrolling =
      (nir_var_shader_in | nir_var_shader_out | nir_var_function_temp),
   .lower_int64_options =
      (nir_lower_int64_options) ~(nir_lower_iadd64 | nir_lower_imul_2x32_64),
   .lower_doubles_options = (nir_lower_doubles_options)(~0),
   .lower_fquantize2f16 = true,
};

#endif
