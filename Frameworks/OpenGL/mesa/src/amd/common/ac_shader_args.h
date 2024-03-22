/*
 * Copyright 2019 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AC_SHADER_ARGS_H
#define AC_SHADER_ARGS_H

#include <stdbool.h>
#include <stdint.h>

/* Maximum dwords of inline push constants when the indirect path is still used */
#define AC_MAX_INLINE_PUSH_CONSTS_WITH_INDIRECT 8
/* Maximum dwords of inline push constants when the indirect path is not used */
#define AC_MAX_INLINE_PUSH_CONSTS 32

enum ac_arg_regfile
{
   AC_ARG_SGPR,
   AC_ARG_VGPR,
};

enum ac_arg_type
{
   AC_ARG_INVALID = -1,
   AC_ARG_FLOAT,
   AC_ARG_INT,
   AC_ARG_CONST_PTR,       /* Pointer to i8 array */
   AC_ARG_CONST_FLOAT_PTR, /* Pointer to f32 array */
   AC_ARG_CONST_PTR_PTR,   /* Pointer to pointer to i8 array */
   AC_ARG_CONST_DESC_PTR,  /* Pointer to v4i32 array */
   AC_ARG_CONST_IMAGE_PTR, /* Pointer to v8i32 array */
};

struct ac_arg {
   uint16_t arg_index;
   bool used;
};

#define AC_MAX_ARGS 384 /* including all VS->TCS IO */

struct ac_shader_args {
   /* Info on how to declare arguments */
   struct {
      enum ac_arg_type type;
      enum ac_arg_regfile file;
      uint8_t offset;
      uint8_t size;
      bool skip : 1;
      bool pending_vmem : 1; /* Loaded from VMEM and needs waitcnt before use. */
      bool preserved : 1;
   } args[AC_MAX_ARGS];

   uint16_t arg_count;
   uint16_t num_sgprs_used;
   uint16_t num_vgprs_used;

   uint16_t return_count;
   uint16_t num_sgprs_returned;
   uint16_t num_vgprs_returned;

   /* User data 0/1. GFX: descriptor list, Compute: scratch BO. These are the SGPRs used by RADV for
    * scratch and have to be accessed using llvm.amdgcn.implicit.buffer.ptr for LLVM in that case.
    */
   struct ac_arg ring_offsets;

   /* VS */
   struct ac_arg base_vertex;
   struct ac_arg start_instance;
   struct ac_arg draw_id;
   struct ac_arg vertex_buffers;
   struct ac_arg vertex_id;
   struct ac_arg vs_rel_patch_id;
   struct ac_arg vs_prim_id;
   struct ac_arg instance_id;

   /* Merged shaders */
   struct ac_arg tess_offchip_offset;
   struct ac_arg merged_wave_info;
   /* On gfx10:
    *  - bits 0..11: ordered_wave_id
    *  - bits 12..20: number of vertices in group
    *  - bits 22..30: number of primitives in group
    */
   struct ac_arg gs_tg_info;
   struct ac_arg scratch_offset;

   /* TCS */
   struct ac_arg tcs_factor_offset;
   struct ac_arg tcs_wave_id; /* gfx11+ */
   struct ac_arg tcs_patch_id;
   struct ac_arg tcs_rel_ids;

   /* TES */
   struct ac_arg tes_u;
   struct ac_arg tes_v;
   struct ac_arg tes_rel_patch_id;
   struct ac_arg tes_patch_id;

   /* GS */
   struct ac_arg es2gs_offset;      /* separate legacy ES */
   struct ac_arg gs2vs_offset;      /* legacy GS */
   struct ac_arg gs_wave_id;        /* legacy GS */
   struct ac_arg gs_attr_offset;    /* gfx11+: attribute ring offset in 512B increments */

   /* GS vertex indices/offsets:
    *
    * GFX6-8: [0-5] 6x uint32, multiplied by VGT_ESGS_RING_ITEMSIZE by hw
    * GFX9-11 non-passthrough: [0-2] 6x packed uint16, multiplied by VGT_ESGS_RING_ITEMSIZE by hw
    *
    * GFX10-11 passthrough: [0] 1x uint32 with the following bitfields matching the prim export:
    *    [0:8]    vertex index 0
    *    [9]      edgeflag 0
    *    [10:18]  vertex index 1
    *    [19]     edgeflag 1
    *    [20:28]  vertex index 2
    *    [29]     edgeflag 2
    *    [31]     0 (valid prim)
    */
   struct ac_arg gs_vtx_offset[6];
   struct ac_arg gs_prim_id;
   struct ac_arg gs_invocation_id;

   /* Streamout */
   struct ac_arg streamout_config;
   struct ac_arg streamout_write_index;
   struct ac_arg streamout_offset[4];

   /* PS */
   struct ac_arg frag_pos[4];
   struct ac_arg front_face;
   struct ac_arg ancillary;
   struct ac_arg sample_coverage;
   struct ac_arg prim_mask;
   struct ac_arg pops_collision_wave_id;
   struct ac_arg load_provoking_vtx;
   struct ac_arg persp_sample;
   struct ac_arg persp_center;
   struct ac_arg persp_centroid;
   struct ac_arg pull_model;
   struct ac_arg linear_sample;
   struct ac_arg linear_center;
   struct ac_arg linear_centroid;
   struct ac_arg pos_fixed_pt;

   /* CS */
   struct ac_arg local_invocation_ids;
   struct ac_arg num_work_groups;
   struct ac_arg workgroup_ids[3];
   struct ac_arg tg_size;

   /* Mesh and task shaders */
   struct ac_arg task_ring_entry; /* Pointer into the draw and payload rings. */

   /* Vulkan only */
   struct ac_arg push_constants;
   struct ac_arg inline_push_consts[AC_MAX_INLINE_PUSH_CONSTS];
   uint64_t inline_push_const_mask;
   struct ac_arg view_index;
   struct ac_arg force_vrs_rates;

   /* RT */
   struct {
      struct ac_arg uniform_shader_addr;
      struct ac_arg sbt_descriptors;
      struct ac_arg launch_size;
      struct ac_arg launch_size_addr;
      struct ac_arg launch_id;
      struct ac_arg dynamic_callable_stack_base;
      struct ac_arg traversal_shader_addr;
      struct ac_arg shader_addr;
      struct ac_arg shader_record;
      struct ac_arg payload_offset;
      struct ac_arg ray_origin;
      struct ac_arg ray_tmin;
      struct ac_arg ray_direction;
      struct ac_arg ray_tmax;
      struct ac_arg cull_mask_and_flags;
      struct ac_arg sbt_offset;
      struct ac_arg sbt_stride;
      struct ac_arg miss_index;
      struct ac_arg accel_struct;
      struct ac_arg primitive_id;
      struct ac_arg instance_addr;
      struct ac_arg geometry_id_and_flags;
      struct ac_arg hit_kind;
   } rt;
};

void ac_add_arg(struct ac_shader_args *info, enum ac_arg_regfile regfile, unsigned registers,
                enum ac_arg_type type, struct ac_arg *arg);
void ac_add_return(struct ac_shader_args *info, enum ac_arg_regfile regfile);
void ac_add_preserved(struct ac_shader_args *info, const struct ac_arg *arg);
void ac_compact_ps_vgpr_args(struct ac_shader_args *info, uint32_t spi_ps_input);

#endif
