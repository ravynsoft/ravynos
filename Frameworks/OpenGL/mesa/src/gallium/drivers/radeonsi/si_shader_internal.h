/*
 * Copyright 2016 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SI_SHADER_PRIVATE_H
#define SI_SHADER_PRIVATE_H

#include "ac_hw_stage.h"
#include "ac_shader_args.h"
#include "ac_shader_util.h"
#include "si_shader.h"

#define SI_SPI_PS_INPUT_ADDR_FOR_PROLOG (       \
   S_0286D0_PERSP_SAMPLE_ENA(1) |               \
   S_0286D0_PERSP_CENTER_ENA(1) |               \
   S_0286D0_PERSP_CENTROID_ENA(1) |             \
   S_0286D0_LINEAR_SAMPLE_ENA(1) |              \
   S_0286D0_LINEAR_CENTER_ENA(1) |              \
   S_0286D0_LINEAR_CENTROID_ENA(1) |            \
   S_0286D0_FRONT_FACE_ENA(1) |                 \
   S_0286D0_ANCILLARY_ENA(1) |                  \
   S_0286D0_SAMPLE_COVERAGE_ENA(1) |            \
   S_0286D0_POS_FIXED_PT_ENA(1))

struct util_debug_callback;

struct si_shader_args {
   struct ac_shader_args ac;

   struct ac_arg const_and_shader_buffers;
   struct ac_arg samplers_and_images;

   /* For merged shaders, the per-stage descriptors for the stage other
    * than the one we're processing, used to pass them through from the
    * first stage to the second.
    */
   struct ac_arg other_const_and_shader_buffers;
   struct ac_arg other_samplers_and_images;

   struct ac_arg internal_bindings;
   struct ac_arg bindless_samplers_and_images;
   struct ac_arg small_prim_cull_info;
   struct ac_arg gs_attr_address;
   /* API VS */
   struct ac_arg vb_descriptors[5];
   struct ac_arg vertex_index0;
   /* VS state bits. See the VS_STATE_* and GS_STATE_* definitions. */
   struct ac_arg vs_state_bits;
   struct ac_arg vs_blit_inputs;

   /* API TCS & TES */
   /* Layout of TCS outputs in the offchip buffer
    * # 6 bits
    *   [0:5] = the number of patches per threadgroup - 1, max = 63
    * # 5 bits
    *   [6:10] = the number of output vertices per patch - 1, max = 31
    * # 5 bits
    *   [11:15] = the number of input vertices per patch - 1, max = 31 (TCS only)
    * # 16 bits
    *   [16:31] = the offset of per patch attributes in the buffer in bytes.
    *       64 outputs are implied by SI_UNIQUE_SLOT_* values.
    *       max = 32(CPs) * 64(outputs) * 16(vec4) * 64(num_patches) = 2M,
    *       clamped to 32K(LDS limit) = 32K
    */
   struct ac_arg tcs_offchip_layout;

   /* API TCS & TES */
   struct ac_arg tes_offchip_addr;
   /* PS */
   struct ac_arg alpha_reference;
   struct ac_arg color_start;
   /* CS */
   struct ac_arg block_size;
   struct ac_arg cs_user_data;
   struct ac_arg cs_shaderbuf[3];
   struct ac_arg cs_image[3];
};

struct ac_nir_gs_output_info;
typedef struct ac_nir_gs_output_info ac_nir_gs_output_info;

struct nir_builder;
typedef struct nir_builder nir_builder;

struct nir_shader;
typedef struct nir_shader nir_shader;

/* si_shader.c */
bool si_is_multi_part_shader(struct si_shader *shader);
bool si_is_merged_shader(struct si_shader *shader);
void si_add_arg_checked(struct ac_shader_args *args, enum ac_arg_regfile file, unsigned registers,
                        enum ac_arg_type type, struct ac_arg *arg, unsigned idx);
void si_init_shader_args(struct si_shader *shader, struct si_shader_args *args);
unsigned si_get_max_workgroup_size(const struct si_shader *shader);
bool si_vs_needs_prolog(const struct si_shader_selector *sel,
                        const struct si_vs_prolog_bits *prolog_key);
void si_get_vs_prolog_key(const struct si_shader_info *info, unsigned num_input_sgprs,
                          const struct si_vs_prolog_bits *prolog_key,
                          struct si_shader *shader_out, union si_shader_part_key *key);
struct nir_shader *si_get_nir_shader(struct si_shader *shader, struct si_shader_args *args,
                                     bool *free_nir, uint64_t tcs_vgpr_only_inputs,
                                     ac_nir_gs_output_info *output_info);
void si_get_tcs_epilog_key(struct si_shader *shader, union si_shader_part_key *key);
bool si_need_ps_prolog(const union si_shader_part_key *key);
void si_get_ps_prolog_key(struct si_shader *shader, union si_shader_part_key *key);
void si_get_ps_epilog_key(struct si_shader *shader, union si_shader_part_key *key);
enum ac_hw_stage si_select_hw_stage(const gl_shader_stage stage, const union si_shader_key *const key,
                                    const enum amd_gfx_level gfx_level);
nir_shader *si_get_prev_stage_nir_shader(struct si_shader *shader,
                                         struct si_shader *prev_shader,
                                         struct si_shader_args *args,
                                         bool *free_nir);
unsigned si_get_tcs_out_patch_stride(const struct si_shader_info *info);
void si_get_tcs_epilog_args(enum amd_gfx_level gfx_level,
                            struct si_shader_args *args,
                            struct ac_arg *rel_patch_id,
                            struct ac_arg *invocation_id,
                            struct ac_arg *tf_lds_offset,
                            struct ac_arg tess_factors[6]);
void si_get_vs_prolog_args(enum amd_gfx_level gfx_level,
                           struct si_shader_args *args,
                           const union si_shader_part_key *key);
void si_get_ps_prolog_args(struct si_shader_args *args,
                           const union si_shader_part_key *key);
void si_get_ps_epilog_args(struct si_shader_args *args,
                           const union si_shader_part_key *key,
                           struct ac_arg colors[MAX_DRAW_BUFFERS],
                           struct ac_arg *depth, struct ac_arg *stencil,
                           struct ac_arg *sample_mask);

/* gfx10_shader_ngg.c */
unsigned gfx10_ngg_get_vertices_per_prim(struct si_shader *shader);
bool gfx10_ngg_export_prim_early(struct si_shader *shader);
unsigned gfx10_ngg_get_scratch_dw_size(struct si_shader *shader);
bool gfx10_ngg_calculate_subgroup_info(struct si_shader *shader);

/* si_nir_lower_abi.c */
nir_def *si_nir_load_internal_binding(nir_builder *b, struct si_shader_args *args,
                                          unsigned slot, unsigned num_components);
bool si_nir_lower_abi(nir_shader *nir, struct si_shader *shader, struct si_shader_args *args);

/* si_nir_lower_resource.c */
bool si_nir_lower_resource(nir_shader *nir, struct si_shader *shader,
                           struct si_shader_args *args);

/* si_nir_lower_vs_inputs.c */
bool si_nir_lower_vs_inputs(nir_shader *nir, struct si_shader *shader,
                            struct si_shader_args *args);

/* si_shader_llvm.c */
bool si_llvm_compile_shader(struct si_screen *sscreen, struct ac_llvm_compiler *compiler,
                            struct si_shader *shader, struct si_shader_args *args,
                            struct util_debug_callback *debug, struct nir_shader *nir);
bool si_llvm_build_shader_part(struct si_screen *sscreen, gl_shader_stage stage,
                               bool prolog, struct ac_llvm_compiler *compiler,
                               struct util_debug_callback *debug, const char *name,
                               struct si_shader_part *result);

/* si_shader_aco.c */
bool si_aco_compile_shader(struct si_shader *shader,
                           struct si_shader_args *args,
                           struct nir_shader *nir,
                           struct util_debug_callback *debug);
void si_aco_resolve_symbols(struct si_shader *shader, uint32_t *code_for_write,
                            const uint32_t *code_for_read, uint64_t scratch_va,
                            uint32_t const_offset);
bool si_aco_build_shader_part(struct si_screen *screen, gl_shader_stage stage, bool prolog,
                              struct util_debug_callback *debug, const char *name,
                              struct si_shader_part *result);

#endif
