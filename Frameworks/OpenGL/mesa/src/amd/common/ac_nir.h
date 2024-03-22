/*
 * Copyright © 2021 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */


#ifndef AC_NIR_H
#define AC_NIR_H

#include "ac_hw_stage.h"
#include "ac_shader_args.h"
#include "ac_shader_util.h"
#include "nir.h"
#include "nir_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

enum
{
   /* SPI_PS_INPUT_CNTL_i.OFFSET[0:4] */
   AC_EXP_PARAM_OFFSET_0 = 0,
   AC_EXP_PARAM_OFFSET_31 = 31,
   /* SPI_PS_INPUT_CNTL_i.DEFAULT_VAL[0:1] */
   AC_EXP_PARAM_DEFAULT_VAL_0000 = 64,
   AC_EXP_PARAM_DEFAULT_VAL_0001,
   AC_EXP_PARAM_DEFAULT_VAL_1110,
   AC_EXP_PARAM_DEFAULT_VAL_1111,
   AC_EXP_PARAM_UNDEFINED = 255, /* deprecated, use AC_EXP_PARAM_DEFAULT_VAL_0000 instead */
};

enum {
   AC_EXP_FLAG_COMPRESSED = (1 << 0),
   AC_EXP_FLAG_DONE       = (1 << 1),
   AC_EXP_FLAG_VALID_MASK = (1 << 2),
};

/* Maps I/O semantics to the actual location used by the lowering pass. */
typedef unsigned (*ac_nir_map_io_driver_location)(unsigned semantic);

/* Forward declaration of nir_builder so we don't have to include nir_builder.h here */
struct nir_builder;
typedef struct nir_builder nir_builder;

/* Executed by ac_nir_cull when the current primitive is accepted. */
typedef void (*ac_nir_cull_accepted)(nir_builder *b, void *state);

nir_def *
ac_nir_load_arg_at_offset(nir_builder *b, const struct ac_shader_args *ac_args,
                          struct ac_arg arg, unsigned relative_index);

static inline nir_def *
ac_nir_load_arg(nir_builder *b, const struct ac_shader_args *ac_args, struct ac_arg arg)
{
   return ac_nir_load_arg_at_offset(b, ac_args, arg, 0);
}

void ac_nir_store_arg(nir_builder *b, const struct ac_shader_args *ac_args, struct ac_arg arg,
                      nir_def *val);

nir_def *
ac_nir_unpack_arg(nir_builder *b, const struct ac_shader_args *ac_args, struct ac_arg arg,
                  unsigned rshift, unsigned bitwidth);

bool ac_nir_lower_sin_cos(nir_shader *shader);

bool ac_nir_lower_intrinsics_to_args(nir_shader *shader, const enum amd_gfx_level gfx_level,
                                     const enum ac_hw_stage hw_stage,
                                     const struct ac_shader_args *ac_args);

void
ac_nir_store_var_components(nir_builder *b, nir_variable *var, nir_def *value,
                            unsigned component, unsigned writemask);

void
ac_nir_export_primitive(nir_builder *b, nir_def *prim, nir_def *row);

void
ac_nir_export_position(nir_builder *b,
                       enum amd_gfx_level gfx_level,
                       uint32_t clip_cull_mask,
                       bool no_param_export,
                       bool force_vrs,
                       bool done,
                       uint64_t outputs_written,
                       nir_def *(*outputs)[4],
                       nir_def *row);

void
ac_nir_export_parameters(nir_builder *b,
                         const uint8_t *param_offsets,
                         uint64_t outputs_written,
                         uint16_t outputs_written_16bit,
                         nir_def *(*outputs)[4],
                         nir_def *(*outputs_16bit_lo)[4],
                         nir_def *(*outputs_16bit_hi)[4]);

nir_def *
ac_nir_calc_io_offset(nir_builder *b,
                      nir_intrinsic_instr *intrin,
                      nir_def *base_stride,
                      unsigned component_stride,
                      ac_nir_map_io_driver_location map_io);

bool ac_nir_optimize_outputs(nir_shader *nir, bool sprite_tex_disallowed,
                             int8_t slot_remap[NUM_TOTAL_VARYING_SLOTS],
                             uint8_t param_export_index[NUM_TOTAL_VARYING_SLOTS]);

void
ac_nir_lower_ls_outputs_to_mem(nir_shader *ls,
                               ac_nir_map_io_driver_location map,
                               bool tcs_in_out_eq,
                               uint64_t tcs_temp_only_inputs);

void
ac_nir_lower_hs_inputs_to_mem(nir_shader *shader,
                              ac_nir_map_io_driver_location map,
                              bool tcs_in_out_eq);

void
ac_nir_lower_hs_outputs_to_mem(nir_shader *shader,
                               ac_nir_map_io_driver_location map,
                               enum amd_gfx_level gfx_level,
                               bool tes_reads_tessfactors,
                               uint64_t tes_inputs_read,
                               uint64_t tes_patch_inputs_read,
                               unsigned num_reserved_tcs_outputs,
                               unsigned num_reserved_tcs_patch_outputs,
                               unsigned wave_size,
                               bool no_inputs_in_lds,
                               bool pass_tessfactors_by_reg,
                               bool emit_tess_factor_write);

void
ac_nir_lower_tes_inputs_to_mem(nir_shader *shader,
                               ac_nir_map_io_driver_location map);

void
ac_nir_lower_es_outputs_to_mem(nir_shader *shader,
                               ac_nir_map_io_driver_location map,
                               enum amd_gfx_level gfx_level,
                               unsigned esgs_itemsize);

void
ac_nir_lower_gs_inputs_to_mem(nir_shader *shader,
                              ac_nir_map_io_driver_location map,
                              enum amd_gfx_level gfx_level,
                              bool triangle_strip_adjacency_fix);

bool
ac_nir_lower_indirect_derefs(nir_shader *shader,
                             enum amd_gfx_level gfx_level);

typedef struct {
   enum radeon_family family;
   enum amd_gfx_level gfx_level;

   unsigned max_workgroup_size;
   unsigned wave_size;
   uint32_t clipdist_enable_mask;
   const uint8_t *vs_output_param_offset; /* GFX11+ */
   bool has_param_exports;
   bool can_cull;
   bool disable_streamout;
   bool has_gen_prim_query;
   bool has_xfb_prim_query;
   bool kill_pointsize;
   bool kill_layer;
   bool force_vrs;

   /* VS */
   unsigned num_vertices_per_primitive;
   bool early_prim_export;
   bool passthrough;
   bool use_edgeflags;
   bool export_primitive_id;
   uint32_t instance_rate_inputs;
   uint32_t user_clip_plane_enable_mask;

   /* GS */
   unsigned gs_out_vtx_bytes;
} ac_nir_lower_ngg_options;

void
ac_nir_lower_ngg_nogs(nir_shader *shader, const ac_nir_lower_ngg_options *options);

void
ac_nir_lower_ngg_gs(nir_shader *shader, const ac_nir_lower_ngg_options *options);

void
ac_nir_lower_ngg_ms(nir_shader *shader,
                    enum amd_gfx_level gfx_level,
                    uint32_t clipdist_enable_mask,
                    const uint8_t *vs_output_param_offset,
                    bool has_param_exports,
                    bool *out_needs_scratch_ring,
                    unsigned wave_size,
                    unsigned workgroup_size,
                    bool multiview,
                    bool has_query,
                    bool fast_launch_2);

void
ac_nir_lower_task_outputs_to_mem(nir_shader *shader,
                                 unsigned task_payload_entry_bytes,
                                 unsigned task_num_entries,
                                 bool has_query);

void
ac_nir_lower_mesh_inputs_to_mem(nir_shader *shader,
                                unsigned task_payload_entry_bytes,
                                unsigned task_num_entries);

nir_def *
ac_nir_cull_primitive(nir_builder *b,
                      nir_def *initially_accepted,
                      nir_def *pos[3][4],
                      unsigned num_vertices,
                      ac_nir_cull_accepted accept_func,
                      void *state);

bool
ac_nir_lower_global_access(nir_shader *shader);

bool ac_nir_lower_resinfo(nir_shader *nir, enum amd_gfx_level gfx_level);
bool ac_nir_lower_image_opcodes(nir_shader *nir);

typedef struct ac_nir_gs_output_info {
   const uint8_t *streams;
   const uint8_t *streams_16bit_lo;
   const uint8_t *streams_16bit_hi;

   const uint8_t *usage_mask;
   const uint8_t *usage_mask_16bit_lo;
   const uint8_t *usage_mask_16bit_hi;

   /* type for each 16bit slot component */
   nir_alu_type (*types_16bit_lo)[4];
   nir_alu_type (*types_16bit_hi)[4];
} ac_nir_gs_output_info;

nir_shader *
ac_nir_create_gs_copy_shader(const nir_shader *gs_nir,
                             enum amd_gfx_level gfx_level,
                             uint32_t clip_cull_mask,
                             const uint8_t *param_offsets,
                             bool has_param_exports,
                             bool disable_streamout,
                             bool kill_pointsize,
                             bool kill_layer,
                             bool force_vrs,
                             ac_nir_gs_output_info *output_info);

void
ac_nir_lower_legacy_vs(nir_shader *nir,
                       enum amd_gfx_level gfx_level,
                       uint32_t clip_cull_mask,
                       const uint8_t *param_offsets,
                       bool has_param_exports,
                       bool export_primitive_id,
                       bool disable_streamout,
                       bool kill_pointsize,
                       bool kill_layer,
                       bool force_vrs);

bool
ac_nir_gs_shader_query(nir_builder *b,
                       bool has_gen_prim_query,
                       bool has_pipeline_stats_query,
                       unsigned num_vertices_per_primitive,
                       unsigned wave_size,
                       nir_def *vertex_count[4],
                       nir_def *primitive_count[4]);

void
ac_nir_lower_legacy_gs(nir_shader *nir,
                       bool has_gen_prim_query,
                       bool has_pipeline_stats_query,
                       ac_nir_gs_output_info *output_info);

typedef struct {
   /* Which load instructions to lower depending on whether the number of
    * components being loaded is 1 or more than 1.
    */
   nir_variable_mode modes_1_comp;  /* lower 1-component loads for these */
   nir_variable_mode modes_N_comps; /* lower multi-component loads for these */
} ac_nir_lower_subdword_options;

bool ac_nir_lower_subdword_loads(nir_shader *nir, ac_nir_lower_subdword_options options);

typedef struct {
   enum radeon_family family;
   enum amd_gfx_level gfx_level;

   bool use_aco;
   bool uses_discard;
   bool alpha_to_coverage_via_mrtz;
   bool dual_src_blend_swizzle;
   unsigned spi_shader_col_format;
   unsigned color_is_int8;
   unsigned color_is_int10;

   bool bc_optimize_for_persp;
   bool bc_optimize_for_linear;
   bool force_persp_sample_interp;
   bool force_linear_sample_interp;
   bool force_persp_center_interp;
   bool force_linear_center_interp;
   unsigned ps_iter_samples;

   /* OpenGL only */
   bool clamp_color;
   bool alpha_to_one;
   bool kill_samplemask;
   enum compare_func alpha_func;
   unsigned broadcast_last_cbuf;

   /* Vulkan only */
   unsigned enable_mrt_output_nan_fixup;
   bool no_color_export;
   bool no_depth_export;
} ac_nir_lower_ps_options;

void
ac_nir_lower_ps(nir_shader *nir, const ac_nir_lower_ps_options *options);

typedef struct {
   enum amd_gfx_level gfx_level;

   /* If true, round the layer component of the coordinates source to the nearest
    * integer for all array ops. This is always done for cube array ops.
    */
   bool lower_array_layer_round_even;

   /* Fix derivatives of constants and FS inputs in control flow.
    *
    * Ignores interpolateAtSample()/interpolateAtOffset(), dynamically indexed input loads,
    * pervertexEXT input loads, textureGather() with implicit LOD and 16-bit derivatives and
    * texture samples with nir_tex_src_min_lod.
    *
    * The layer must also be a constant or FS input.
    */
   bool fix_derivs_in_divergent_cf;
   unsigned max_wqm_vgprs;
} ac_nir_lower_tex_options;

bool
ac_nir_lower_tex(nir_shader *nir, const ac_nir_lower_tex_options *options);

#ifdef __cplusplus
}
#endif

#endif /* AC_NIR_H */
