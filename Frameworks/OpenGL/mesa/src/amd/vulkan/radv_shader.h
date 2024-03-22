/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
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

#ifndef RADV_SHADER_H
#define RADV_SHADER_H

#include "util/mesa-blake3.h"
#include "util/u_math.h"
#include "vulkan/runtime/vk_pipeline_cache.h"
#include "vulkan/vulkan.h"
#include "ac_binary.h"
#include "ac_shader_util.h"
#include "amd_family.h"
#include "radv_constants.h"

#include "aco_shader_info.h"

#define RADV_VERT_ATTRIB_MAX MAX2(VERT_ATTRIB_MAX, VERT_ATTRIB_GENERIC0 + MAX_VERTEX_ATTRIBS)

struct radv_physical_device;
struct radv_device;
struct radv_pipeline;
struct radv_ray_tracing_pipeline;
struct radv_pipeline_key;
struct radv_shader_args;
struct radv_vs_input_state;
struct radv_shader_args;
struct radv_serialized_shader_arena_block;

enum {
   RADV_GRAPHICS_STAGE_BITS =
      (VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT),
   RADV_RT_STAGE_BITS =
      (VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
       VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR)
};

#define RADV_STAGE_MASK ((1 << MESA_VULKAN_SHADER_STAGES) - 1)

#define radv_foreach_stage(stage, stage_bits)                                                                          \
   for (gl_shader_stage stage, __tmp = (gl_shader_stage)((stage_bits)&RADV_STAGE_MASK); stage = ffs(__tmp) - 1, __tmp; \
        __tmp &= ~(1 << (stage)))

enum radv_nggc_settings {
   radv_nggc_none = 0,
   radv_nggc_front_face = 1 << 0,
   radv_nggc_back_face = 1 << 1,
   radv_nggc_face_is_ccw = 1 << 2,
   radv_nggc_small_primitives = 1 << 3,
};

enum radv_shader_query_state {
   radv_shader_query_none = 0,
   radv_shader_query_pipeline_stat = 1 << 0,
   radv_shader_query_prim_gen = 1 << 1,
   radv_shader_query_prim_xfb = 1 << 2,
};

enum radv_required_subgroup_size {
   RADV_REQUIRED_NONE = 0,
   RADV_REQUIRED_WAVE32 = 1,
   RADV_REQUIRED_WAVE64 = 2,
};

struct radv_shader_stage_key {
   uint8_t subgroup_required_size : 2; /* radv_required_subgroup_size */
   uint8_t subgroup_require_full : 1;  /* whether full subgroups are required */

   uint8_t storage_robustness2 : 1;
   uint8_t uniform_robustness2 : 1;
};

struct radv_ps_epilog_key {
   uint32_t spi_shader_col_format;
   uint32_t spi_shader_z_format;

   /* Bitmasks, each bit represents one of the 8 MRTs. */
   uint8_t color_is_int8;
   uint8_t color_is_int10;
   uint8_t enable_mrt_output_nan_fixup;

   uint32_t colors_written;
   bool mrt0_is_dual_src;
   bool export_depth;
   bool export_stencil;
   bool export_sample_mask;
   bool alpha_to_coverage_via_mrtz;
};

struct radv_pipeline_key {
   uint32_t lib_flags : 4; /* VkGraphicsPipelineLibraryFlagBitsEXT */

   uint32_t has_multiview_view_index : 1;
   uint32_t optimisations_disabled : 1;
   uint32_t adjust_frag_coord_z : 1;
   uint32_t dynamic_patch_control_points : 1;
   uint32_t dynamic_rasterization_samples : 1;
   uint32_t dynamic_provoking_vtx_mode : 1;
   uint32_t dynamic_line_rast_mode : 1;
   uint32_t enable_remove_point_size : 1;
   uint32_t unknown_rast_prim : 1;

   uint32_t vertex_robustness1 : 1;
   uint32_t mesh_fast_launch_2 : 1;

   uint32_t keep_statistic_info : 1;

   /* Pipeline shader version (up to 8) to force re-compilation when RADV_BUILD_ID_OVERRIDE is enabled. */
   uint32_t shader_version : 3;

   struct radv_shader_stage_key stage_info[MESA_VULKAN_SHADER_STAGES];

   struct {
      uint32_t instance_rate_inputs;
      uint32_t instance_rate_divisors[MAX_VERTEX_ATTRIBS];
      uint8_t vertex_attribute_formats[MAX_VERTEX_ATTRIBS];
      uint32_t vertex_attribute_bindings[MAX_VERTEX_ATTRIBS];
      uint32_t vertex_attribute_offsets[MAX_VERTEX_ATTRIBS];
      uint32_t vertex_attribute_strides[MAX_VERTEX_ATTRIBS];
      uint8_t vertex_binding_align[MAX_VBS];
      uint32_t provoking_vtx_last : 1;
      uint32_t has_prolog : 1;
      uint8_t topology;
   } vs;

   struct {
      unsigned tess_input_vertices;
   } tcs;

   struct {
      struct radv_ps_epilog_key epilog;

      uint8_t num_samples;
      bool sample_shading_enable;

      bool force_vrs_enabled;

      /* Used to export alpha through MRTZ for alpha-to-coverage (GFX11+). */
      bool alpha_to_coverage_via_mrtz;
      bool exports_mrtz_via_epilog;

      bool has_epilog;

      bool line_smooth_enabled;
   } ps;
};

struct radv_nir_compiler_options {
   struct radv_pipeline_key key;
   bool robust_buffer_access_llvm;
   bool dump_shader;
   bool dump_preoptir;
   bool record_ir;
   bool record_stats;
   bool check_ir;
   uint8_t enable_mrt_output_nan_fixup;
   bool wgp_mode;
   const struct radeon_info *info;

   struct {
      void (*func)(void *private_data, enum aco_compiler_debug_level level, const char *message);
      void *private_data;
   } debug;
};

enum radv_ud_index {
   AC_UD_SCRATCH_RING_OFFSETS = 0,
   AC_UD_PUSH_CONSTANTS = 1,
   AC_UD_INLINE_PUSH_CONSTANTS = 2,
   AC_UD_INDIRECT_DESCRIPTOR_SETS = 3,
   AC_UD_VIEW_INDEX = 4,
   AC_UD_STREAMOUT_BUFFERS = 5,
   AC_UD_SHADER_QUERY_STATE = 6,
   AC_UD_NGG_PROVOKING_VTX = 7,
   AC_UD_NGG_CULLING_SETTINGS = 8,
   AC_UD_NGG_VIEWPORT = 9,
   AC_UD_FORCE_VRS_RATES = 10,
   AC_UD_TASK_RING_ENTRY = 11,
   AC_UD_NUM_VERTS_PER_PRIM = 12,
   AC_UD_NEXT_STAGE_PC = 13,
   AC_UD_SHADER_START = 14,
   AC_UD_VS_VERTEX_BUFFERS = AC_UD_SHADER_START,
   AC_UD_VS_BASE_VERTEX_START_INSTANCE,
   AC_UD_VS_PROLOG_INPUTS,
   AC_UD_VS_MAX_UD,
   AC_UD_PS_EPILOG_PC,
   AC_UD_PS_STATE,
   AC_UD_PS_MAX_UD,
   AC_UD_CS_GRID_SIZE = AC_UD_SHADER_START,
   AC_UD_CS_SBT_DESCRIPTORS,
   AC_UD_CS_RAY_LAUNCH_SIZE_ADDR,
   AC_UD_CS_RAY_DYNAMIC_CALLABLE_STACK_BASE,
   AC_UD_CS_TRAVERSAL_SHADER_ADDR,
   AC_UD_CS_TASK_RING_OFFSETS,
   AC_UD_CS_TASK_DRAW_ID,
   AC_UD_CS_TASK_IB,
   AC_UD_CS_MAX_UD,
   AC_UD_GS_MAX_UD,
   AC_UD_TCS_OFFCHIP_LAYOUT = AC_UD_VS_MAX_UD,
   AC_UD_TCS_EPILOG_PC,
   AC_UD_TCS_MAX_UD,
   AC_UD_TES_STATE = AC_UD_SHADER_START,
   AC_UD_TES_MAX_UD,
   AC_UD_MAX_UD = AC_UD_CS_MAX_UD,
};

#define SET_SGPR_FIELD(field, value) (((unsigned)(value)&field##__MASK) << field##__SHIFT)

#define TCS_OFFCHIP_LAYOUT_PATCH_CONTROL_POINTS__SHIFT 0
#define TCS_OFFCHIP_LAYOUT_PATCH_CONTROL_POINTS__MASK  0x3f
#define TCS_OFFCHIP_LAYOUT_NUM_PATCHES__SHIFT          6
#define TCS_OFFCHIP_LAYOUT_NUM_PATCHES__MASK           0x3f
#define TCS_OFFCHIP_LAYOUT_LSHS_VERTEX_STRIDE__SHIFT   12
#define TCS_OFFCHIP_LAYOUT_LSHS_VERTEX_STRIDE__MASK    0xff /* max 32 * 4 + 1 (to reduce LDS bank conflicts) */

#define TES_STATE_NUM_PATCHES__SHIFT      0
#define TES_STATE_NUM_PATCHES__MASK       0xff
#define TES_STATE_TCS_VERTICES_OUT__SHIFT 8
#define TES_STATE_TCS_VERTICES_OUT__MASK  0xff
#define TES_STATE_NUM_TCS_OUTPUTS__SHIFT  16
#define TES_STATE_NUM_TCS_OUTPUTS__MASK   0xff

#define PS_STATE_NUM_SAMPLES__SHIFT    0
#define PS_STATE_NUM_SAMPLES__MASK     0xf
#define PS_STATE_LINE_RAST_MODE__SHIFT 4
#define PS_STATE_LINE_RAST_MODE__MASK  0x3
#define PS_STATE_PS_ITER_MASK__SHIFT   6
#define PS_STATE_PS_ITER_MASK__MASK    0xffff
#define PS_STATE_RAST_PRIM__SHIFT      22
#define PS_STATE_RAST_PRIM__MASK       0x3

struct radv_streamout_info {
   uint16_t num_outputs;
   uint16_t strides[MAX_SO_BUFFERS];
   uint32_t enabled_stream_buffers_mask;
};

struct radv_userdata_info {
   int8_t sgpr_idx;
   uint8_t num_sgprs;
};

struct radv_userdata_locations {
   struct radv_userdata_info descriptor_sets[MAX_SETS];
   struct radv_userdata_info shader_data[AC_UD_MAX_UD];
   uint32_t descriptor_sets_enabled;
};

struct radv_vs_output_info {
   uint8_t vs_output_param_offset[VARYING_SLOT_MAX];
   uint8_t clip_dist_mask;
   uint8_t cull_dist_mask;
   uint8_t param_exports;
   uint8_t prim_param_exports;
   bool writes_pointsize;
   bool writes_layer;
   bool writes_layer_per_primitive;
   bool writes_viewport_index;
   bool writes_viewport_index_per_primitive;
   bool writes_primitive_shading_rate;
   bool writes_primitive_shading_rate_per_primitive;
   bool export_prim_id;
   unsigned pos_exports;
};

struct radv_legacy_gs_info {
   uint32_t vgt_gs_onchip_cntl;
   uint32_t vgt_gs_max_prims_per_subgroup;
   uint32_t vgt_esgs_ring_itemsize;
   uint32_t lds_size;
   uint32_t esgs_ring_size;
   uint32_t gsvs_ring_size;
};

struct gfx10_ngg_info {
   uint16_t ngg_emit_size; /* in dwords */
   uint32_t hw_max_esverts;
   uint32_t max_gsprims;
   uint32_t max_out_verts;
   uint32_t prim_amp_factor;
   uint32_t vgt_esgs_ring_itemsize;
   uint32_t esgs_ring_size;
   uint32_t scratch_lds_base;
   bool max_vert_out_per_gs_instance;
};

enum radv_shader_type {
   RADV_SHADER_TYPE_DEFAULT = 0,
   RADV_SHADER_TYPE_GS_COPY,
   RADV_SHADER_TYPE_TRAP_HANDLER,
};

struct radv_shader_info {
   uint64_t inline_push_constant_mask;
   bool can_inline_all_push_constants;
   bool loads_push_constants;
   bool loads_dynamic_offsets;
   uint32_t desc_set_used_mask;
   bool uses_view_index;
   bool uses_invocation_id;
   bool uses_prim_id;
   uint8_t wave_size;
   uint8_t ballot_bit_size;
   struct radv_userdata_locations user_sgprs_locs;
   bool is_ngg;
   bool is_ngg_passthrough;
   bool has_ngg_culling;
   bool has_ngg_early_prim_export;
   bool has_prim_query;
   bool has_xfb_query;
   uint32_t num_tess_patches;
   uint32_t esgs_itemsize; /* Only for VS or TES as ES */
   struct radv_vs_output_info outinfo;
   unsigned workgroup_size;
   bool force_vrs_per_vertex;
   gl_shader_stage stage;
   gl_shader_stage next_stage;
   enum radv_shader_type type;
   uint32_t user_data_0;
   bool inputs_linked;
   bool outputs_linked;
   bool has_epilog;                        /* Only for TCS or PS */
   bool merged_shader_compiled_separately; /* GFX9+ */

   struct {
      uint8_t input_usage_mask[RADV_VERT_ATTRIB_MAX];
      uint8_t output_usage_mask[VARYING_SLOT_VAR31 + 1];
      bool needs_draw_id;
      bool needs_instance_id;
      bool as_es;
      bool as_ls;
      bool tcs_in_out_eq;
      uint64_t tcs_temp_only_input_mask;
      uint8_t num_linked_outputs;
      bool needs_base_instance;
      bool use_per_attribute_vb_descs;
      uint32_t vb_desc_usage_mask;
      uint32_t input_slot_usage_mask;
      bool has_prolog;
      bool dynamic_inputs;
      bool dynamic_num_verts_per_prim;
   } vs;
   struct {
      uint8_t output_usage_mask[VARYING_SLOT_VAR31 + 1];
      uint8_t num_stream_output_components[4];
      uint8_t output_streams[VARYING_SLOT_VAR31 + 1];
      uint8_t max_stream;
      unsigned gsvs_vertex_size;
      unsigned max_gsvs_emit_size;
      unsigned vertices_in;
      unsigned vertices_out;
      unsigned input_prim;
      unsigned output_prim;
      unsigned invocations;
      unsigned es_type; /* GFX9: VS or TES */
      uint8_t num_linked_inputs;
      bool has_pipeline_stat_query;
   } gs;
   struct {
      uint8_t output_usage_mask[VARYING_SLOT_VAR31 + 1];
      bool as_es;
      enum tess_primitive_mode _primitive_mode;
      enum gl_tess_spacing spacing;
      bool ccw;
      bool point_mode;
      bool reads_tess_factors;
      unsigned tcs_vertices_out;
      uint8_t num_linked_inputs;
      uint8_t num_linked_outputs;
   } tes;
   struct {
      bool uses_sample_shading;
      bool needs_sample_positions;
      bool needs_poly_line_smooth;
      bool writes_memory;
      bool writes_z;
      bool writes_stencil;
      bool writes_sample_mask;
      bool writes_mrt0_alpha;
      bool exports_mrtz_via_epilog;
      bool has_pcoord;
      bool prim_id_input;
      bool layer_input;
      bool viewport_index_input;
      uint8_t num_input_clips_culls;
      uint32_t input_mask;
      uint32_t input_per_primitive_mask;
      uint32_t flat_shaded_mask;
      uint32_t explicit_shaded_mask;
      uint32_t per_vertex_shaded_mask;
      uint32_t float16_shaded_mask;
      uint32_t num_interp;
      uint32_t num_prim_interp;
      bool can_discard;
      bool early_fragment_test;
      bool post_depth_coverage;
      bool reads_sample_mask_in;
      bool reads_front_face;
      bool reads_sample_id;
      bool reads_frag_shading_rate;
      bool reads_barycentric_model;
      bool reads_persp_sample;
      bool reads_persp_center;
      bool reads_persp_centroid;
      bool reads_linear_sample;
      bool reads_linear_center;
      bool reads_linear_centroid;
      bool reads_fully_covered;
      uint8_t reads_frag_coord_mask;
      uint8_t reads_sample_pos_mask;
      uint8_t depth_layout;
      bool allow_flat_shading;
      bool pops; /* Uses Primitive Ordered Pixel Shading (fragment shader interlock) */
      bool pops_is_per_sample;
      bool mrt0_is_dual_src;
      unsigned spi_ps_input;
      unsigned colors_written;
      unsigned spi_shader_col_format;
      uint8_t color0_written;
      bool load_provoking_vtx;
      bool load_rasterization_prim;
      bool force_sample_iter_shading_rate;
      uint32_t db_shader_control; /* DB_SHADER_CONTROL without intrinsic rate overrides */
   } ps;
   struct {
      bool uses_grid_size;
      bool uses_block_id[3];
      bool uses_thread_id[3];
      bool uses_local_invocation_idx;
      unsigned block_size[3];

      bool is_rt_shader;
      bool uses_ray_launch_size;
      bool uses_dynamic_rt_callable_stack;
      bool uses_rt;
      bool uses_full_subgroups;
      bool linear_taskmesh_dispatch;
      bool has_query; /* Task shader only */

      bool regalloc_hang_bug;
   } cs;
   struct {
      uint64_t tes_inputs_read;
      uint64_t tes_patch_inputs_read;
      unsigned tcs_vertices_out;
      uint32_t num_lds_blocks;
      uint8_t num_linked_inputs;
      uint8_t num_linked_outputs;
      uint8_t num_linked_patch_outputs;
      bool tes_reads_tess_factors : 1;
   } tcs;
   struct {
      enum mesa_prim output_prim;
      bool needs_ms_scratch_ring;
      bool has_task; /* If mesh shader is used together with a task shader. */
      bool has_query;
   } ms;

   struct radv_streamout_info so;

   struct radv_legacy_gs_info gs_ring_info;
   struct gfx10_ngg_info ngg_info;
};

struct radv_vs_input_state {
   uint32_t attribute_mask;

   uint32_t instance_rate_inputs;
   uint32_t nontrivial_divisors;
   uint32_t zero_divisors;
   uint32_t post_shuffle;
   /* Having two separate fields instead of a single uint64_t makes it easier to remove attributes
    * using bitwise arithmetic.
    */
   uint32_t alpha_adjust_lo;
   uint32_t alpha_adjust_hi;
   uint32_t nontrivial_formats;

   uint8_t bindings[MAX_VERTEX_ATTRIBS];
   uint32_t divisors[MAX_VERTEX_ATTRIBS];
   uint32_t offsets[MAX_VERTEX_ATTRIBS];
   uint8_t formats[MAX_VERTEX_ATTRIBS];
   uint8_t format_align_req_minus_1[MAX_VERTEX_ATTRIBS];
   uint8_t format_sizes[MAX_VERTEX_ATTRIBS];

   bool bindings_match_attrib;
};

struct radv_vs_prolog_key {
   /* All the fields are pre-masked with BITFIELD_MASK(num_attributes).
    * Some of the fields are pre-masked by other conditions. See lookup_vs_prolog.
    */
   uint32_t instance_rate_inputs;
   uint32_t nontrivial_divisors;
   uint32_t zero_divisors;
   uint32_t post_shuffle;
   /* Having two separate fields instead of a single uint64_t makes it easier to remove attributes
    * using bitwise arithmetic.
    */
   uint32_t alpha_adjust_lo;
   uint32_t alpha_adjust_hi;
   uint8_t formats[MAX_VERTEX_ATTRIBS];
   unsigned num_attributes;
   uint32_t misaligned_mask;
   bool as_ls;
   bool is_ngg;
   bool wave32;
   gl_shader_stage next_stage;
};

struct radv_tcs_epilog_key {
   enum tess_primitive_mode primitive_mode;
   bool tes_reads_tessfactors;
   bool tcs_out_patch_fits_subgroup;
};

enum radv_shader_binary_type { RADV_BINARY_TYPE_LEGACY, RADV_BINARY_TYPE_RTLD };

struct radv_shader_binary {
   uint32_t type; /* enum radv_shader_binary_type */

   struct ac_shader_config config;
   struct radv_shader_info info;

   /* Self-referential size so we avoid consistency issues. */
   uint32_t total_size;
};

struct radv_shader_binary_legacy {
   struct radv_shader_binary base;
   uint32_t code_size;
   uint32_t exec_size;
   uint32_t ir_size;
   uint32_t disasm_size;
   uint32_t stats_size;
   uint32_t padding;

   /* data has size of stats_size + code_size + ir_size + disasm_size + 2,
    * where the +2 is for 0 of the ir strings. */
   uint8_t data[0];
};
static_assert(sizeof(struct radv_shader_binary_legacy) == offsetof(struct radv_shader_binary_legacy, data),
              "Unexpected padding");

struct radv_shader_binary_rtld {
   struct radv_shader_binary base;
   unsigned elf_size;
   unsigned llvm_ir_size;
   uint8_t data[0];
};

struct radv_shader_part_binary {
   struct {
      uint32_t spi_shader_col_format;
      uint32_t spi_shader_z_format;
   } info;

   uint8_t num_sgprs;
   uint8_t num_vgprs;
   unsigned code_size;
   unsigned disasm_size;

   /* Self-referential size so we avoid consistency issues. */
   uint32_t total_size;

   uint8_t data[0];
};

enum radv_shader_arena_type { RADV_SHADER_ARENA_DEFAULT, RADV_SHADER_ARENA_REPLAYABLE, RADV_SHADER_ARENA_REPLAYED };

struct radv_shader_arena {
   struct list_head list;
   struct list_head entries;
   uint32_t size;
   struct radeon_winsys_bo *bo;
   char *ptr;
   enum radv_shader_arena_type type;
};

union radv_shader_arena_block {
   struct list_head pool;
   struct {
      /* List of blocks in the arena, sorted by address. */
      struct list_head list;
      /* For holes, a list_head for the free-list. For allocations, freelist.prev=NULL and
       * freelist.next is a pointer associated with the allocation.
       */
      struct list_head freelist;
      struct radv_shader_arena *arena;
      uint32_t offset;
      uint32_t size;
   };
};

struct radv_shader_free_list {
   uint8_t size_mask;
   struct list_head free_lists[RADV_SHADER_ALLOC_NUM_FREE_LISTS];
};

struct radv_serialized_shader_arena_block {
   uint32_t offset;
   uint32_t size;
   uint64_t arena_va;
   uint32_t arena_size;
};

struct radv_shader {
   struct vk_pipeline_cache_object base;

   simple_mtx_t replay_mtx;
   bool has_replay_alloc;

   struct radeon_winsys_bo *bo;
   union radv_shader_arena_block *alloc;
   uint64_t va;

   uint64_t upload_seq;

   struct ac_shader_config config;
   uint32_t code_size;
   uint32_t exec_size;
   struct radv_shader_info info;
   uint32_t max_waves;

   blake3_hash hash;
   void *code;

   /* debug only */
   char *spirv;
   uint32_t spirv_size;
   char *nir_string;
   char *disasm_string;
   char *ir_string;
   uint32_t *statistics;
};

struct radv_shader_part {
   uint32_t ref_count;

   union {
      struct radv_vs_prolog_key vs;
      struct radv_ps_epilog_key ps;
      struct radv_tcs_epilog_key tcs;
   } key;

   uint64_t va;

   struct radeon_winsys_bo *bo;
   union radv_shader_arena_block *alloc;
   uint32_t code_size;
   uint32_t rsrc1;
   bool nontrivial_divisors;
   uint32_t spi_shader_col_format;
   uint32_t spi_shader_z_format;
   uint64_t upload_seq;

   /* debug only */
   char *disasm_string;
};

struct radv_shader_part_cache_ops {
   uint32_t (*hash)(const void *key);
   bool (*equals)(const void *a, const void *b);
   struct radv_shader_part *(*create)(struct radv_device *device, const void *key);
};

struct radv_shader_part_cache {
   simple_mtx_t lock;
   struct radv_shader_part_cache_ops *ops;
   struct set entries;
};

struct radv_shader_dma_submission {
   struct list_head list;

   struct radeon_cmdbuf *cs;
   struct radeon_winsys_bo *bo;
   uint64_t bo_size;
   char *ptr;

   /* The semaphore value to wait for before reusing this submission. */
   uint64_t seq;
};

struct radv_pipeline_layout;
struct radv_shader_stage;

void radv_optimize_nir(struct nir_shader *shader, bool optimize_conservatively);
void radv_optimize_nir_algebraic(nir_shader *shader, bool opt_offsets);

void radv_postprocess_nir(struct radv_device *device, const struct radv_pipeline_key *pipeline_key,
                          struct radv_shader_stage *stage);

bool radv_shader_should_clear_lds(const struct radv_device *device, const nir_shader *shader);

nir_shader *radv_parse_rt_stage(struct radv_device *device, const VkPipelineShaderStageCreateInfo *sinfo,
                                const struct radv_pipeline_key *key,
                                const struct radv_pipeline_layout *pipeline_layout);

void radv_nir_lower_rt_abi(nir_shader *shader, const VkRayTracingPipelineCreateInfoKHR *pCreateInfo,
                           const struct radv_shader_args *args, const struct radv_shader_info *info,
                           uint32_t *stack_size, bool resume_shader, struct radv_device *device,
                           struct radv_ray_tracing_pipeline *pipeline, bool monolithic);

struct radv_shader_stage;

nir_shader *radv_shader_spirv_to_nir(struct radv_device *device, const struct radv_shader_stage *stage,
                                     const struct radv_pipeline_key *key, bool is_internal);

void radv_init_shader_arenas(struct radv_device *device);
void radv_destroy_shader_arenas(struct radv_device *device);
VkResult radv_init_shader_upload_queue(struct radv_device *device);
void radv_destroy_shader_upload_queue(struct radv_device *device);

struct radv_shader_args;

struct radv_shader *radv_shader_create(struct radv_device *device, struct vk_pipeline_cache *cache,
                                       const struct radv_shader_binary *binary, bool skip_cache);

VkResult radv_shader_create_uncached(struct radv_device *device, const struct radv_shader_binary *binary,
                                     bool replayable, struct radv_serialized_shader_arena_block *replay_block,
                                     struct radv_shader **out_shader);

struct radv_shader_binary *radv_shader_nir_to_asm(struct radv_device *device, struct radv_shader_stage *pl_stage,
                                                  struct nir_shader *const *shaders, int shader_count,
                                                  const struct radv_pipeline_key *key, bool keep_shader_info,
                                                  bool keep_statistic_info);

void radv_shader_generate_debug_info(struct radv_device *device, bool dump_shader, bool keep_shader_info,
                                     struct radv_shader_binary *binary, struct radv_shader *shader,
                                     struct nir_shader *const *shaders, int shader_count,
                                     struct radv_shader_info *info);

VkResult radv_shader_wait_for_upload(struct radv_device *device, uint64_t seq);

struct radv_shader_dma_submission *radv_shader_dma_pop_submission(struct radv_device *device);

void radv_shader_dma_push_submission(struct radv_device *device, struct radv_shader_dma_submission *submission,
                                     uint64_t seq);

struct radv_shader_dma_submission *
radv_shader_dma_get_submission(struct radv_device *device, struct radeon_winsys_bo *bo, uint64_t va, uint64_t size);

bool radv_shader_dma_submit(struct radv_device *device, struct radv_shader_dma_submission *submission,
                            uint64_t *upload_seq_out);

union radv_shader_arena_block *radv_alloc_shader_memory(struct radv_device *device, uint32_t size, bool replayable,
                                                        void *ptr);

union radv_shader_arena_block *radv_replay_shader_arena_block(struct radv_device *device,
                                                              const struct radv_serialized_shader_arena_block *src,
                                                              void *ptr);

struct radv_serialized_shader_arena_block radv_serialize_shader_arena_block(union radv_shader_arena_block *block);

void radv_free_shader_memory(struct radv_device *device, union radv_shader_arena_block *alloc);

struct radv_shader *radv_create_trap_handler_shader(struct radv_device *device);

struct radv_shader *radv_create_rt_prolog(struct radv_device *device);

struct radv_shader_part *radv_shader_part_create(struct radv_device *device, struct radv_shader_part_binary *binary,
                                                 unsigned wave_size);

struct radv_shader_part *radv_create_vs_prolog(struct radv_device *device, const struct radv_vs_prolog_key *key);

struct radv_shader_part *radv_create_ps_epilog(struct radv_device *device, const struct radv_ps_epilog_key *key,
                                               struct radv_shader_part_binary **binary_out);

struct radv_shader_part *radv_create_tcs_epilog(struct radv_device *device, const struct radv_tcs_epilog_key *key);

void radv_shader_part_destroy(struct radv_device *device, struct radv_shader_part *shader_part);

bool radv_shader_part_cache_init(struct radv_shader_part_cache *cache, struct radv_shader_part_cache_ops *ops);
void radv_shader_part_cache_finish(struct radv_device *device, struct radv_shader_part_cache *cache);
struct radv_shader_part *radv_shader_part_cache_get(struct radv_device *device, struct radv_shader_part_cache *cache,
                                                    struct set *local_entries, const void *key);

uint64_t radv_shader_get_va(const struct radv_shader *shader);
struct radv_shader *radv_find_shader(struct radv_device *device, uint64_t pc);

unsigned radv_get_max_waves(const struct radv_device *device, const struct ac_shader_config *conf,
                            const struct radv_shader_info *info);

unsigned radv_get_max_scratch_waves(const struct radv_device *device, struct radv_shader *shader);

const char *radv_get_shader_name(const struct radv_shader_info *info, gl_shader_stage stage);

unsigned radv_compute_spi_ps_input(const struct radv_pipeline_key *pipeline_key, const struct radv_shader_info *info);

bool radv_can_dump_shader(struct radv_device *device, nir_shader *nir, bool meta_shader);

bool radv_can_dump_shader_stats(struct radv_device *device, nir_shader *nir);

VkResult radv_dump_shader_stats(struct radv_device *device, struct radv_pipeline *pipeline, struct radv_shader *shader,
                                gl_shader_stage stage, FILE *output);

/* Returns true on success and false on failure */
bool radv_shader_reupload(struct radv_device *device, struct radv_shader *shader);

enum ac_hw_stage radv_select_hw_stage(const struct radv_shader_info *const info, const enum amd_gfx_level gfx_level);

extern const struct vk_pipeline_cache_object_ops radv_shader_ops;

static inline struct radv_shader *
radv_shader_ref(struct radv_shader *shader)
{
   vk_pipeline_cache_object_ref(&shader->base);
   return shader;
}

static inline void
radv_shader_unref(struct radv_device *device, struct radv_shader *shader)
{
   vk_pipeline_cache_object_unref((struct vk_device *)device, &shader->base);
}

static inline struct radv_shader_part *
radv_shader_part_ref(struct radv_shader_part *shader_part)
{
   assert(shader_part && shader_part->ref_count >= 1);
   p_atomic_inc(&shader_part->ref_count);
   return shader_part;
}

static inline void
radv_shader_part_unref(struct radv_device *device, struct radv_shader_part *shader_part)
{
   assert(shader_part && shader_part->ref_count >= 1);
   if (p_atomic_dec_zero(&shader_part->ref_count))
      radv_shader_part_destroy(device, shader_part);
}

static inline struct radv_shader_part *
radv_shader_part_from_cache_entry(const void *key)
{
   return container_of(key, struct radv_shader_part, key);
}

static inline unsigned
get_tcs_input_vertex_stride(unsigned tcs_num_inputs)
{
   unsigned stride = tcs_num_inputs * 16;

   /* Add 1 dword to reduce LDS bank conflicts. */
   if (stride)
      stride += 4;

   return stride;
}

static inline unsigned
calculate_tess_lds_size(enum amd_gfx_level gfx_level, unsigned tcs_num_input_vertices, unsigned tcs_num_output_vertices,
                        unsigned tcs_num_inputs, unsigned tcs_num_patches, unsigned tcs_num_outputs,
                        unsigned tcs_num_patch_outputs)
{
   unsigned input_vertex_size = get_tcs_input_vertex_stride(tcs_num_inputs);
   unsigned output_vertex_size = tcs_num_outputs * 16;

   unsigned input_patch_size = tcs_num_input_vertices * input_vertex_size;

   unsigned pervertex_output_patch_size = tcs_num_output_vertices * output_vertex_size;
   unsigned output_patch_size = pervertex_output_patch_size + tcs_num_patch_outputs * 16;

   unsigned output_patch0_offset = input_patch_size * tcs_num_patches;

   unsigned lds_size = output_patch0_offset + output_patch_size * tcs_num_patches;

   if (gfx_level >= GFX7) {
      assert(lds_size <= 65536);
      lds_size = align(lds_size, 512) / 512;
   } else {
      assert(lds_size <= 32768);
      lds_size = align(lds_size, 256) / 256;
   }

   return lds_size;
}

static inline unsigned
get_tcs_num_patches(unsigned tcs_num_input_vertices, unsigned tcs_num_output_vertices, unsigned tcs_num_inputs,
                    unsigned tcs_num_outputs, unsigned tcs_num_patch_outputs, unsigned tess_offchip_block_dw_size,
                    enum amd_gfx_level gfx_level, enum radeon_family family)
{
   uint32_t input_vertex_size = get_tcs_input_vertex_stride(tcs_num_inputs);
   uint32_t input_patch_size = tcs_num_input_vertices * input_vertex_size;
   uint32_t output_vertex_size = tcs_num_outputs * 16;
   uint32_t pervertex_output_patch_size = tcs_num_output_vertices * output_vertex_size;
   uint32_t output_patch_size = pervertex_output_patch_size + tcs_num_patch_outputs * 16;

   /* Ensure that we only need one wave per SIMD so we don't need to check
    * resource usage. Also ensures that the number of tcs in and out
    * vertices per threadgroup are at most 256.
    */
   unsigned num_patches = 64 / MAX2(tcs_num_input_vertices, tcs_num_output_vertices) * 4;
   /* Make sure that the data fits in LDS. This assumes the shaders only
    * use LDS for the inputs and outputs.
    */
   unsigned hardware_lds_size = 32768;

   /* Looks like STONEY hangs if we use more than 32 KiB LDS in a single
    * threadgroup, even though there is more than 32 KiB LDS.
    *
    * Test: dEQP-VK.tessellation.shader_input_output.barrier
    */
   if (gfx_level >= GFX7 && family != CHIP_STONEY)
      hardware_lds_size = 65536;

   if (input_patch_size + output_patch_size)
      num_patches = MIN2(num_patches, hardware_lds_size / (input_patch_size + output_patch_size));
   /* Make sure the output data fits in the offchip buffer */
   if (output_patch_size)
      num_patches = MIN2(num_patches, (tess_offchip_block_dw_size * 4) / output_patch_size);
   /* Not necessary for correctness, but improves performance. The
    * specific value is taken from the proprietary driver.
    */
   num_patches = MIN2(num_patches, 40);

   /* GFX6 bug workaround - limit LS-HS threadgroups to only one wave. */
   if (gfx_level == GFX6) {
      unsigned one_wave = 64 / MAX2(tcs_num_input_vertices, tcs_num_output_vertices);
      num_patches = MIN2(num_patches, one_wave);
   }
   return num_patches;
}

void radv_lower_ngg(struct radv_device *device, struct radv_shader_stage *ngg_stage,
                    const struct radv_pipeline_key *pl_key);

bool radv_consider_culling(const struct radv_physical_device *pdevice, struct nir_shader *nir, uint64_t ps_inputs_read,
                           unsigned num_vertices_per_primitive, const struct radv_shader_info *info);

void radv_get_nir_options(struct radv_physical_device *device);

nir_shader *radv_build_traversal_shader(struct radv_device *device, struct radv_ray_tracing_pipeline *pipeline,
                                        const VkRayTracingPipelineCreateInfoKHR *pCreateInfo);

enum radv_rt_priority {
   radv_rt_priority_raygen = 0,
   radv_rt_priority_traversal = 1,
   radv_rt_priority_hit_miss = 2,
   radv_rt_priority_callable = 3,
   radv_rt_priority_mask = 0x3,
};

static inline enum radv_rt_priority
radv_get_rt_priority(gl_shader_stage stage)
{
   switch (stage) {
   case MESA_SHADER_RAYGEN:
      return radv_rt_priority_raygen;
   case MESA_SHADER_INTERSECTION:
   case MESA_SHADER_ANY_HIT:
      return radv_rt_priority_traversal;
   case MESA_SHADER_CLOSEST_HIT:
   case MESA_SHADER_MISS:
      return radv_rt_priority_hit_miss;
   case MESA_SHADER_CALLABLE:
      return radv_rt_priority_callable;
   default:
      unreachable("Unimplemented RT shader stage.");
   }
}

struct radv_shader_layout;
enum radv_pipeline_type;

void radv_nir_shader_info_pass(struct radv_device *device, const struct nir_shader *nir,
                               const struct radv_shader_layout *layout, const struct radv_pipeline_key *pipeline_key,
                               const enum radv_pipeline_type pipeline_type, bool consider_force_vrs,
                               struct radv_shader_info *info);

void radv_nir_shader_info_init(gl_shader_stage stage, gl_shader_stage next_stage, struct radv_shader_info *info);

void radv_nir_shader_info_link(struct radv_device *device, const struct radv_pipeline_key *pipeline_key,
                               struct radv_shader_stage *stages);

#endif
