/*
 * Copyright © 2021 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_nir.h"
#include "amdgfxregs.h"
#include "nir_builder.h"
#include "nir_xfb_info.h"
#include "util/u_math.h"
#include "util/u_vector.h"

#define SPECIAL_MS_OUT_MASK \
   (BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_COUNT) | \
    BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_INDICES) | \
    BITFIELD64_BIT(VARYING_SLOT_CULL_PRIMITIVE))

#define MS_PRIM_ARG_EXP_MASK \
   (VARYING_BIT_LAYER | \
    VARYING_BIT_VIEWPORT | \
    VARYING_BIT_PRIMITIVE_SHADING_RATE)

#define MS_VERT_ARG_EXP_MASK \
   (VARYING_BIT_CULL_DIST0 | \
    VARYING_BIT_CULL_DIST1 | \
    VARYING_BIT_CLIP_DIST0 | \
    VARYING_BIT_CLIP_DIST1 | \
    VARYING_BIT_PSIZ)

enum {
   nggc_passflag_used_by_pos = 1,
   nggc_passflag_used_by_other = 2,
   nggc_passflag_used_by_both = nggc_passflag_used_by_pos | nggc_passflag_used_by_other,
};

typedef struct
{
   nir_def *ssa;
   nir_variable *var;
} reusable_nondeferred_variable;

typedef struct
{
   gl_varying_slot slot;
   nir_def *chan[4];
} vs_output;

typedef struct
{
   nir_alu_type types[VARYING_SLOT_MAX][4];
   nir_alu_type types_16bit_lo[16][4];
   nir_alu_type types_16bit_hi[16][4];
} shader_output_types;

typedef struct
{
   const ac_nir_lower_ngg_options *options;

   nir_variable *position_value_var;
   nir_variable *prim_exp_arg_var;
   nir_variable *es_accepted_var;
   nir_variable *gs_accepted_var;
   nir_variable *gs_exported_var;
   nir_variable *gs_vtx_indices_vars[3];

   nir_def *vtx_addr[3];

   struct u_vector reusable_nondeferred_variables;

   bool early_prim_export;
   bool streamout_enabled;
   bool has_user_edgeflags;
   unsigned max_num_waves;

   /* LDS params */
   unsigned pervertex_lds_bytes;

   uint64_t inputs_needed_by_pos;
   uint64_t inputs_needed_by_others;

   nir_instr *compact_arg_stores[4];
   nir_intrinsic_instr *overwrite_args;
   nir_variable *repacked_rel_patch_id;

   /* clip distance */
   nir_variable *clip_vertex_var;
   nir_variable *clipdist_neg_mask_var;
   bool has_clipdist;

   /* outputs */
   nir_def *outputs[VARYING_SLOT_MAX][4];
   nir_def *outputs_16bit_lo[16][4];
   nir_def *outputs_16bit_hi[16][4];
   shader_output_types output_types;
} lower_ngg_nogs_state;

typedef struct
{
   /* output stream index, 2 bit per component */
   uint8_t stream;
   /* Bitmask of components used: 4 bits per slot, 1 bit per component. */
   uint8_t components_mask : 4;
} gs_output_info;

typedef struct
{
   const ac_nir_lower_ngg_options *options;

   nir_function_impl *impl;
   int const_out_vtxcnt[4];
   int const_out_prmcnt[4];
   unsigned max_num_waves;
   unsigned num_vertices_per_primitive;
   nir_def *lds_addr_gs_out_vtx;
   nir_def *lds_addr_gs_scratch;
   unsigned lds_bytes_per_gs_out_vertex;
   unsigned lds_offs_primflags;
   bool output_compile_time_known;
   bool streamout_enabled;
   /* 32 bit outputs */
   nir_def *outputs[VARYING_SLOT_MAX][4];
   gs_output_info output_info[VARYING_SLOT_MAX];
   /* 16 bit outputs */
   nir_def *outputs_16bit_hi[16][4];
   nir_def *outputs_16bit_lo[16][4];
   gs_output_info output_info_16bit_hi[16];
   gs_output_info output_info_16bit_lo[16];
   /* output types for both 32bit and 16bit */
   shader_output_types output_types;
   /* Count per stream. */
   nir_def *vertex_count[4];
   nir_def *primitive_count[4];
} lower_ngg_gs_state;

/* LDS layout of Mesh Shader workgroup info. */
enum {
   /* DW0: number of primitives */
   lds_ms_num_prims = 0,
   /* DW1: number of vertices */
   lds_ms_num_vtx = 4,
   /* DW2: workgroup index within the current dispatch */
   lds_ms_wg_index = 8,
   /* DW3: number of API workgroups in flight */
   lds_ms_num_api_waves = 12,
};

/* Potential location for Mesh Shader outputs. */
typedef enum {
   ms_out_mode_lds,
   ms_out_mode_scratch_ring,
   ms_out_mode_attr_ring,
   ms_out_mode_var,
} ms_out_mode;

typedef struct
{
   uint64_t mask; /* Mask of output locations */
   uint32_t addr; /* Base address */
} ms_out_part;

typedef struct
{
   /* Mesh shader LDS layout. For details, see ms_calculate_output_layout. */
   struct {
      uint32_t workgroup_info_addr;
      ms_out_part vtx_attr;
      ms_out_part prm_attr;
      uint32_t indices_addr;
      uint32_t cull_flags_addr;
      uint32_t total_size;
   } lds;

   /* VRAM "mesh shader scratch ring" layout for outputs that don't fit into the LDS.
    * Not to be confused with scratch memory.
    */
   struct {
      ms_out_part vtx_attr;
      ms_out_part prm_attr;
   } scratch_ring;

   /* VRAM attributes ring (GFX11 only) for all non-position outputs.
    * GFX11 doesn't have to reload attributes from this ring at the end of the shader.
    */
   struct {
      ms_out_part vtx_attr;
      ms_out_part prm_attr;
   } attr_ring;

   /* Outputs without cross-invocation access can be stored in variables. */
   struct {
      ms_out_part vtx_attr;
      ms_out_part prm_attr;
   } var;
} ms_out_mem_layout;

typedef struct
{
   enum amd_gfx_level gfx_level;
   bool fast_launch_2;
   bool vert_multirow_export;
   bool prim_multirow_export;

   ms_out_mem_layout layout;
   uint64_t per_vertex_outputs;
   uint64_t per_primitive_outputs;
   unsigned vertices_per_prim;

   unsigned wave_size;
   unsigned api_workgroup_size;
   unsigned hw_workgroup_size;

   nir_def *workgroup_index;
   nir_variable *out_variables[VARYING_SLOT_MAX * 4];
   nir_variable *primitive_count_var;
   nir_variable *vertex_count_var;

   /* True if the lowering needs to insert the layer output. */
   bool insert_layer_output;
   /* True if cull flags are used */
   bool uses_cull_flags;

   struct {
      /* Bitmask of components used: 4 bits per slot, 1 bit per component. */
      uint32_t components_mask;
   } output_info[VARYING_SLOT_MAX];

   /* Used by outputs export. */
   nir_def *outputs[VARYING_SLOT_MAX][4];
   uint32_t clipdist_enable_mask;
   const uint8_t *vs_output_param_offset;
   bool has_param_exports;

   /* True if the lowering needs to insert shader query. */
   bool has_query;
} lower_ngg_ms_state;

/* Per-vertex LDS layout of culling shaders */
enum {
   /* Position of the ES vertex (at the beginning for alignment reasons) */
   lds_es_pos_x = 0,
   lds_es_pos_y = 4,
   lds_es_pos_z = 8,
   lds_es_pos_w = 12,

   /* 1 when the vertex is accepted, 0 if it should be culled */
   lds_es_vertex_accepted = 16,
   /* ID of the thread which will export the current thread's vertex */
   lds_es_exporter_tid = 17,
   /* bit i is set when the i'th clip distance of a vertex is negative */
   lds_es_clipdist_neg_mask = 18,
   /* TES only, relative patch ID, less than max workgroup size */
   lds_es_tes_rel_patch_id = 19,

   /* Repacked arguments - also listed separately for VS and TES */
   lds_es_arg_0 = 20,
};

typedef struct {
   nir_def *num_repacked_invocations;
   nir_def *repacked_invocation_index;
} wg_repack_result;

/**
 * Computes a horizontal sum of 8-bit packed values loaded from LDS.
 *
 * Each lane N will sum packed bytes 0 to N-1.
 * We only care about the results from up to wave_id+1 lanes.
 * (Other lanes are not deactivated but their calculation is not used.)
 */
static nir_def *
summarize_repack(nir_builder *b, nir_def *packed_counts, unsigned num_lds_dwords)
{
   /* We'll use shift to filter out the bytes not needed by the current lane.
    *
    * Need to shift by: num_lds_dwords * 4 - lane_id (in bytes).
    * However, two shifts are needed because one can't go all the way,
    * so the shift amount is half that (and in bits).
    *
    * When v_dot4_u32_u8 is available, we right-shift a series of 0x01 bytes.
    * This will yield 0x01 at wanted byte positions and 0x00 at unwanted positions,
    * therefore v_dot can get rid of the unneeded values.
    * This sequence is preferable because it better hides the latency of the LDS.
    *
    * If the v_dot instruction can't be used, we left-shift the packed bytes.
    * This will shift out the unneeded bytes and shift in zeroes instead,
    * then we sum them using v_msad_u8.
    */

   nir_def *lane_id = nir_load_subgroup_invocation(b);
   nir_def *shift = nir_iadd_imm(b, nir_imul_imm(b, lane_id, -4u), num_lds_dwords * 16);
   bool use_dot = b->shader->options->has_udot_4x8;

   if (num_lds_dwords == 1) {
      nir_def *dot_op = !use_dot ? NULL : nir_ushr(b, nir_ushr(b, nir_imm_int(b, 0x01010101), shift), shift);

      /* Broadcast the packed data we read from LDS (to the first 16 lanes, but we only care up to num_waves). */
      nir_def *packed = nir_lane_permute_16_amd(b, packed_counts, nir_imm_int(b, 0), nir_imm_int(b, 0));

      /* Horizontally add the packed bytes. */
      if (use_dot) {
         return nir_udot_4x8_uadd(b, packed, dot_op, nir_imm_int(b, 0));
      } else {
         nir_def *sad_op = nir_ishl(b, nir_ishl(b, packed, shift), shift);
         return nir_msad_4x8(b, sad_op, nir_imm_int(b, 0), nir_imm_int(b, 0));
      }
   } else if (num_lds_dwords == 2) {
      nir_def *dot_op = !use_dot ? NULL : nir_ushr(b, nir_ushr(b, nir_imm_int64(b, 0x0101010101010101), shift), shift);

      /* Broadcast the packed data we read from LDS (to the first 16 lanes, but we only care up to num_waves). */
      nir_def *packed_dw0 = nir_lane_permute_16_amd(b, nir_unpack_64_2x32_split_x(b, packed_counts), nir_imm_int(b, 0), nir_imm_int(b, 0));
      nir_def *packed_dw1 = nir_lane_permute_16_amd(b, nir_unpack_64_2x32_split_y(b, packed_counts), nir_imm_int(b, 0), nir_imm_int(b, 0));

      /* Horizontally add the packed bytes. */
      if (use_dot) {
         nir_def *sum = nir_udot_4x8_uadd(b, packed_dw0, nir_unpack_64_2x32_split_x(b, dot_op), nir_imm_int(b, 0));
         return nir_udot_4x8_uadd(b, packed_dw1, nir_unpack_64_2x32_split_y(b, dot_op), sum);
      } else {
         nir_def *sad_op = nir_ishl(b, nir_ishl(b, nir_pack_64_2x32_split(b, packed_dw0, packed_dw1), shift), shift);
         nir_def *sum = nir_msad_4x8(b, nir_unpack_64_2x32_split_x(b, sad_op), nir_imm_int(b, 0), nir_imm_int(b, 0));
         return nir_msad_4x8(b, nir_unpack_64_2x32_split_y(b, sad_op), nir_imm_int(b, 0), sum);
      }
   } else {
      unreachable("Unimplemented NGG wave count");
   }
}

/**
 * Repacks invocations in the current workgroup to eliminate gaps between them.
 *
 * Uses 1 dword of LDS per 4 waves (1 byte of LDS per wave).
 * Assumes that all invocations in the workgroup are active (exec = -1).
 */
static wg_repack_result
repack_invocations_in_workgroup(nir_builder *b, nir_def *input_bool,
                                nir_def *lds_addr_base, unsigned max_num_waves,
                                unsigned wave_size)
{
   /* Input boolean: 1 if the current invocation should survive the repack. */
   assert(input_bool->bit_size == 1);

   /* STEP 1. Count surviving invocations in the current wave.
    *
    * Implemented by a scalar instruction that simply counts the number of bits set in a 32/64-bit mask.
    */

   nir_def *input_mask = nir_ballot(b, 1, wave_size, input_bool);
   nir_def *surviving_invocations_in_current_wave = nir_bit_count(b, input_mask);

   /* If we know at compile time that the workgroup has only 1 wave, no further steps are necessary. */
   if (max_num_waves == 1) {
      wg_repack_result r = {
         .num_repacked_invocations = surviving_invocations_in_current_wave,
         .repacked_invocation_index = nir_mbcnt_amd(b, input_mask, nir_imm_int(b, 0)),
      };
      return r;
   }

   /* STEP 2. Waves tell each other their number of surviving invocations.
    *
    * Each wave activates only its first lane (exec = 1), which stores the number of surviving
    * invocations in that wave into the LDS, then reads the numbers from every wave.
    *
    * The workgroup size of NGG shaders is at most 256, which means
    * the maximum number of waves is 4 in Wave64 mode and 8 in Wave32 mode.
    * Each wave writes 1 byte, so it's up to 8 bytes, so at most 2 dwords are necessary.
    */

   const unsigned num_lds_dwords = DIV_ROUND_UP(max_num_waves, 4);
   assert(num_lds_dwords <= 2);

   nir_def *wave_id = nir_load_subgroup_id(b);
   nir_def *lds_offset = nir_iadd(b, lds_addr_base, wave_id);
   nir_def *dont_care = nir_undef(b, 1, num_lds_dwords * 32);
   nir_if *if_first_lane = nir_push_if(b, nir_elect(b, 1));

   nir_store_shared(b, nir_u2u8(b, surviving_invocations_in_current_wave), lds_offset);

   nir_barrier(b, .execution_scope=SCOPE_WORKGROUP, .memory_scope=SCOPE_WORKGROUP,
                         .memory_semantics=NIR_MEMORY_ACQ_REL, .memory_modes=nir_var_mem_shared);

   nir_def *packed_counts =
      nir_load_shared(b, 1, num_lds_dwords * 32, lds_addr_base, .align_mul = 8u);

   nir_pop_if(b, if_first_lane);

   packed_counts = nir_if_phi(b, packed_counts, dont_care);

   /* STEP 3. Compute the repacked invocation index and the total number of surviving invocations.
    *
    * By now, every wave knows the number of surviving invocations in all waves.
    * Each number is 1 byte, and they are packed into up to 2 dwords.
    *
    * Each lane N will sum the number of surviving invocations from waves 0 to N-1.
    * If the workgroup has M waves, then each wave will use only its first M+1 lanes for this.
    * (Other lanes are not deactivated but their calculation is not used.)
    *
    * - We read the sum from the lane whose id is the current wave's id.
    *   Add the masked bitcount to this, and we get the repacked invocation index.
    * - We read the sum from the lane whose id is the number of waves in the workgroup.
    *   This is the total number of surviving invocations in the workgroup.
    */

   nir_def *num_waves = nir_load_num_subgroups(b);
   nir_def *sum = summarize_repack(b, packed_counts, num_lds_dwords);

   nir_def *wg_repacked_index_base = nir_read_invocation(b, sum, wave_id);
   nir_def *wg_num_repacked_invocations = nir_read_invocation(b, sum, num_waves);
   nir_def *wg_repacked_index = nir_mbcnt_amd(b, input_mask, wg_repacked_index_base);

   wg_repack_result r = {
      .num_repacked_invocations = wg_num_repacked_invocations,
      .repacked_invocation_index = wg_repacked_index,
   };

   return r;
}

static nir_def *
pervertex_lds_addr(nir_builder *b, nir_def *vertex_idx, unsigned per_vtx_bytes)
{
   return nir_imul_imm(b, vertex_idx, per_vtx_bytes);
}

static nir_def *
emit_pack_ngg_prim_exp_arg(nir_builder *b, unsigned num_vertices_per_primitives,
                           nir_def *vertex_indices[3], nir_def *is_null_prim)
{
   nir_def *arg = nir_load_initial_edgeflags_amd(b);

   for (unsigned i = 0; i < num_vertices_per_primitives; ++i) {
      assert(vertex_indices[i]);
      arg = nir_ior(b, arg, nir_ishl_imm(b, vertex_indices[i], 10u * i));
   }

   if (is_null_prim) {
      if (is_null_prim->bit_size == 1)
         is_null_prim = nir_b2i32(b, is_null_prim);
      assert(is_null_prim->bit_size == 32);
      arg = nir_ior(b, arg, nir_ishl_imm(b, is_null_prim, 31u));
   }

   return arg;
}

static void
alloc_vertices_and_primitives(nir_builder *b,
                              nir_def *num_vtx,
                              nir_def *num_prim)
{
   /* The caller should only call this conditionally on wave 0.
    *
    * Send GS Alloc Request message from the first wave of the group to SPI.
    * Message payload (in the m0 register) is:
    * - bits 0..10: number of vertices in group
    * - bits 12..22: number of primitives in group
    */

   nir_def *m0 = nir_ior(b, nir_ishl_imm(b, num_prim, 12), num_vtx);
   nir_sendmsg_amd(b, m0, .base = AC_SENDMSG_GS_ALLOC_REQ);
}

static void
alloc_vertices_and_primitives_gfx10_workaround(nir_builder *b,
                                               nir_def *num_vtx,
                                               nir_def *num_prim)
{
   /* HW workaround for a GPU hang with 100% culling on GFX10.
    * We always have to export at least 1 primitive.
    * Export a degenerate triangle using vertex 0 for all 3 vertices.
    *
    * NOTE: We rely on the caller to set the vertex count also to 0 when the primitive count is 0.
    */
   nir_def *is_prim_cnt_0 = nir_ieq_imm(b, num_prim, 0);
   nir_if *if_prim_cnt_0 = nir_push_if(b, is_prim_cnt_0);
   {
      nir_def *one = nir_imm_int(b, 1);
      alloc_vertices_and_primitives(b, one, one);

      nir_def *tid = nir_load_subgroup_invocation(b);
      nir_def *is_thread_0 = nir_ieq_imm(b, tid, 0);
      nir_if *if_thread_0 = nir_push_if(b, is_thread_0);
      {
         /* The vertex indices are 0, 0, 0. */
         nir_export_amd(b, nir_imm_zero(b, 4, 32),
                        .base = V_008DFC_SQ_EXP_PRIM,
                        .flags = AC_EXP_FLAG_DONE,
                        .write_mask = 1);

         /* The HW culls primitives with NaN. -1 is also NaN and can save
          * a dword in binary code by inlining constant.
          */
         nir_export_amd(b, nir_imm_ivec4(b, -1, -1, -1, -1),
                        .base = V_008DFC_SQ_EXP_POS,
                        .flags = AC_EXP_FLAG_DONE,
                        .write_mask = 0xf);
      }
      nir_pop_if(b, if_thread_0);
   }
   nir_push_else(b, if_prim_cnt_0);
   {
      alloc_vertices_and_primitives(b, num_vtx, num_prim);
   }
   nir_pop_if(b, if_prim_cnt_0);
}

static void
ngg_nogs_init_vertex_indices_vars(nir_builder *b, nir_function_impl *impl, lower_ngg_nogs_state *s)
{
   for (unsigned v = 0; v < s->options->num_vertices_per_primitive; ++v) {
      s->gs_vtx_indices_vars[v] = nir_local_variable_create(impl, glsl_uint_type(), "gs_vtx_addr");

      nir_def *vtx = s->options->passthrough ?
         nir_ubfe_imm(b, nir_load_packed_passthrough_primitive_amd(b),
                      10 * v, 9) :
         nir_ubfe_imm(b, nir_load_gs_vertex_offset_amd(b, .base = v / 2u),
                      (v & 1u) * 16u, 16u);

      nir_store_var(b, s->gs_vtx_indices_vars[v], vtx, 0x1);
   }
}

static nir_def *
emit_ngg_nogs_prim_exp_arg(nir_builder *b, lower_ngg_nogs_state *s)
{
   if (s->options->passthrough) {
      return nir_load_packed_passthrough_primitive_amd(b);
   } else {
      nir_def *vtx_idx[3] = {0};

      for (unsigned v = 0; v < s->options->num_vertices_per_primitive; ++v)
         vtx_idx[v] = nir_load_var(b, s->gs_vtx_indices_vars[v]);

      return emit_pack_ngg_prim_exp_arg(b, s->options->num_vertices_per_primitive, vtx_idx, NULL);
   }
}

static nir_def *
has_input_vertex(nir_builder *b)
{
   return nir_is_subgroup_invocation_lt_amd(b, nir_load_merged_wave_info_amd(b));
}

static nir_def *
has_input_primitive(nir_builder *b)
{
   return nir_is_subgroup_invocation_lt_amd(b,
                                            nir_ushr_imm(b, nir_load_merged_wave_info_amd(b), 8));
}

static void
nogs_prim_gen_query(nir_builder *b, lower_ngg_nogs_state *s)
{
   if (!s->options->has_gen_prim_query)
      return;

   nir_if *if_shader_query = nir_push_if(b, nir_load_prim_gen_query_enabled_amd(b));
   {
      /* Activate only 1 lane and add the number of primitives to query result. */
      nir_if *if_elected = nir_push_if(b, nir_elect(b, 1));
      {
         /* Number of input primitives in the current wave. */
         nir_def *num_input_prims = nir_ubfe_imm(b, nir_load_merged_wave_info_amd(b),
                                                     8, 8);

         /* Add to stream 0 primitive generated counter. */
         nir_atomic_add_gen_prim_count_amd(b, num_input_prims, .stream_id = 0);
      }
      nir_pop_if(b, if_elected);
   }
   nir_pop_if(b, if_shader_query);
}

static void
emit_ngg_nogs_prim_export(nir_builder *b, lower_ngg_nogs_state *s, nir_def *arg)
{
   nir_if *if_gs_thread = nir_push_if(b, nir_load_var(b, s->gs_exported_var));
   {
      if (!arg)
         arg = emit_ngg_nogs_prim_exp_arg(b, s);

      /* pack user edge flag info into arg */
      if (s->has_user_edgeflags) {
         /* Workgroup barrier: wait for ES threads store user edge flags to LDS */
         nir_barrier(b, .execution_scope = SCOPE_WORKGROUP,
                            .memory_scope = SCOPE_WORKGROUP,
                            .memory_semantics = NIR_MEMORY_ACQ_REL,
                            .memory_modes = nir_var_mem_shared);

         unsigned edge_flag_bits = ac_get_all_edge_flag_bits();
         nir_def *mask = nir_imm_intN_t(b, ~edge_flag_bits, 32);

         unsigned edge_flag_offset = 0;
         if (s->streamout_enabled) {
            unsigned packed_location =
               util_bitcount64(b->shader->info.outputs_written &
                               BITFIELD64_MASK(VARYING_SLOT_EDGE));
            edge_flag_offset = packed_location * 16;
         }

         for (int i = 0; i < s->options->num_vertices_per_primitive; i++) {
            nir_def *vtx_idx = nir_load_var(b, s->gs_vtx_indices_vars[i]);
            nir_def *addr = pervertex_lds_addr(b, vtx_idx, s->pervertex_lds_bytes);
            nir_def *edge = nir_load_shared(b, 1, 32, addr, .base = edge_flag_offset);
            mask = nir_ior(b, mask, nir_ishl_imm(b, edge, 9 + i * 10));
         }
         arg = nir_iand(b, arg, mask);
      }

      ac_nir_export_primitive(b, arg, NULL);
   }
   nir_pop_if(b, if_gs_thread);
}

static void
emit_ngg_nogs_prim_id_store_shared(nir_builder *b, lower_ngg_nogs_state *s)
{
   nir_def *gs_thread =
      s->gs_accepted_var ? nir_load_var(b, s->gs_accepted_var) : has_input_primitive(b);

   nir_if *if_gs_thread = nir_push_if(b, gs_thread);
   {
      /* Copy Primitive IDs from GS threads to the LDS address
       * corresponding to the ES thread of the provoking vertex.
       * It will be exported as a per-vertex attribute.
       */
      nir_def *gs_vtx_indices[3];
      for (unsigned i = 0; i < s->options->num_vertices_per_primitive; i++)
         gs_vtx_indices[i] = nir_load_var(b, s->gs_vtx_indices_vars[i]);

      nir_def *provoking_vertex = nir_load_provoking_vtx_in_prim_amd(b);
      nir_def *provoking_vtx_idx = nir_select_from_ssa_def_array(
         b, gs_vtx_indices, s->options->num_vertices_per_primitive, provoking_vertex);

      nir_def *prim_id = nir_load_primitive_id(b);
      nir_def *addr = pervertex_lds_addr(b, provoking_vtx_idx, s->pervertex_lds_bytes);

      /* primitive id is always at last of a vertex */
      nir_store_shared(b, prim_id, addr, .base = s->pervertex_lds_bytes - 4);
   }
   nir_pop_if(b, if_gs_thread);
}

static void
emit_store_ngg_nogs_es_primitive_id(nir_builder *b, lower_ngg_nogs_state *s)
{
   nir_def *prim_id = NULL;

   if (b->shader->info.stage == MESA_SHADER_VERTEX) {
      /* LDS address where the primitive ID is stored */
      nir_def *thread_id_in_threadgroup = nir_load_local_invocation_index(b);
      nir_def *addr =
         pervertex_lds_addr(b, thread_id_in_threadgroup, s->pervertex_lds_bytes);

      /* Load primitive ID from LDS */
      prim_id = nir_load_shared(b, 1, 32, addr, .base = s->pervertex_lds_bytes - 4);
   } else if (b->shader->info.stage == MESA_SHADER_TESS_EVAL) {
      /* Just use tess eval primitive ID, which is the same as the patch ID. */
      prim_id = nir_load_primitive_id(b);
   }

   s->outputs[VARYING_SLOT_PRIMITIVE_ID][0] = prim_id;

   /* Update outputs_written to reflect that the pass added a new output. */
   b->shader->info.outputs_written |= VARYING_BIT_PRIMITIVE_ID;
}

static void
add_clipdist_bit(nir_builder *b, nir_def *dist, unsigned index, nir_variable *mask)
{
   nir_def *is_neg = nir_flt_imm(b, dist, 0);
   nir_def *neg_mask = nir_ishl_imm(b, nir_b2i32(b, is_neg), index);
   neg_mask = nir_ior(b, neg_mask, nir_load_var(b, mask));
   nir_store_var(b, mask, neg_mask, 1);
}

static bool
remove_culling_shader_output(nir_builder *b, nir_instr *instr, void *state)
{
   lower_ngg_nogs_state *s = (lower_ngg_nogs_state *) state;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   /* These are not allowed in VS / TES */
   assert(intrin->intrinsic != nir_intrinsic_store_per_vertex_output &&
          intrin->intrinsic != nir_intrinsic_load_per_vertex_input);

   /* We are only interested in output stores now */
   if (intrin->intrinsic != nir_intrinsic_store_output)
      return false;

   b->cursor = nir_before_instr(instr);

   /* no indirect output */
   assert(nir_src_is_const(intrin->src[1]) && nir_src_as_uint(intrin->src[1]) == 0);

   unsigned writemask = nir_intrinsic_write_mask(intrin);
   unsigned component = nir_intrinsic_component(intrin);
   nir_def *store_val = intrin->src[0].ssa;

   /* Position output - store the value to a variable, remove output store */
   nir_io_semantics io_sem = nir_intrinsic_io_semantics(intrin);
   switch (io_sem.location) {
   case VARYING_SLOT_POS:
      ac_nir_store_var_components(b, s->position_value_var, store_val, component, writemask);
      break;
   case VARYING_SLOT_CLIP_DIST0:
   case VARYING_SLOT_CLIP_DIST1: {
      unsigned base = io_sem.location == VARYING_SLOT_CLIP_DIST1 ? 4 : 0;
      base += component;

      /* valid clipdist component mask */
      unsigned mask = (s->options->clipdist_enable_mask >> base) & writemask;
      u_foreach_bit(i, mask) {
         add_clipdist_bit(b, nir_channel(b, store_val, i), base + i,
                          s->clipdist_neg_mask_var);
         s->has_clipdist = true;
      }
      break;
   }
   case VARYING_SLOT_CLIP_VERTEX:
      ac_nir_store_var_components(b, s->clip_vertex_var, store_val, component, writemask);
      break;
   default:
      break;
   }

   /* Remove all output stores */
   nir_instr_remove(instr);
   return true;
}

static void
remove_culling_shader_outputs(nir_shader *culling_shader, lower_ngg_nogs_state *s)
{
   nir_shader_instructions_pass(culling_shader, remove_culling_shader_output,
                                nir_metadata_block_index | nir_metadata_dominance, s);

   /* Remove dead code resulting from the deleted outputs. */
   bool progress;
   do {
      progress = false;
      NIR_PASS(progress, culling_shader, nir_opt_dead_write_vars);
      NIR_PASS(progress, culling_shader, nir_opt_dce);
      NIR_PASS(progress, culling_shader, nir_opt_dead_cf);
   } while (progress);
}

static void
rewrite_uses_to_var(nir_builder *b, nir_def *old_def, nir_variable *replacement_var, unsigned replacement_var_channel)
{
   if (old_def->parent_instr->type == nir_instr_type_load_const)
      return;

   b->cursor = nir_after_instr(old_def->parent_instr);
   if (b->cursor.instr->type == nir_instr_type_phi)
      b->cursor = nir_after_phis(old_def->parent_instr->block);

   nir_def *pos_val_rep = nir_load_var(b, replacement_var);
   nir_def *replacement = nir_channel(b, pos_val_rep, replacement_var_channel);

   if (old_def->num_components > 1) {
      /* old_def uses a swizzled vector component.
       * There is no way to replace the uses of just a single vector component,
       * so instead create a new vector and replace all uses of the old vector.
       */
      nir_def *old_def_elements[NIR_MAX_VEC_COMPONENTS] = {0};
      for (unsigned j = 0; j < old_def->num_components; ++j)
         old_def_elements[j] = nir_channel(b, old_def, j);
      replacement = nir_vec(b, old_def_elements, old_def->num_components);
   }

   nir_def_rewrite_uses_after(old_def, replacement, replacement->parent_instr);
}

static bool
remove_extra_pos_output(nir_builder *b, nir_instr *instr, void *state)
{
   lower_ngg_nogs_state *s = (lower_ngg_nogs_state *) state;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   /* These are not allowed in VS / TES */
   assert(intrin->intrinsic != nir_intrinsic_store_per_vertex_output &&
          intrin->intrinsic != nir_intrinsic_load_per_vertex_input);

   /* We are only interested in output stores now */
   if (intrin->intrinsic != nir_intrinsic_store_output)
      return false;

   nir_io_semantics io_sem = nir_intrinsic_io_semantics(intrin);
   if (io_sem.location != VARYING_SLOT_POS)
      return false;

   b->cursor = nir_before_instr(instr);

   /* In case other outputs use what we calculated for pos,
    * try to avoid calculating it again by rewriting the usages
    * of the store components here.
    */
   nir_def *store_val = intrin->src[0].ssa;
   unsigned store_pos_component = nir_intrinsic_component(intrin);

   nir_instr_remove(instr);

   if (store_val->parent_instr->type == nir_instr_type_alu) {
      nir_alu_instr *alu = nir_instr_as_alu(store_val->parent_instr);
      if (nir_op_is_vec_or_mov(alu->op)) {
         /* Output store uses a vector, we can easily rewrite uses of each vector element. */

         unsigned num_vec_src = 0;
         if (alu->op == nir_op_mov)
            num_vec_src = 1;
         else if (alu->op == nir_op_vec2)
            num_vec_src = 2;
         else if (alu->op == nir_op_vec3)
            num_vec_src = 3;
         else if (alu->op == nir_op_vec4)
            num_vec_src = 4;
         assert(num_vec_src);

         /* Remember the current components whose uses we wish to replace.
          * This is needed because rewriting one source can affect the others too.
          */
         nir_def *vec_comps[NIR_MAX_VEC_COMPONENTS] = {0};
         for (unsigned i = 0; i < num_vec_src; i++)
            vec_comps[i] = alu->src[i].src.ssa;

         for (unsigned i = 0; i < num_vec_src; i++)
            rewrite_uses_to_var(b, vec_comps[i], s->position_value_var, store_pos_component + i);
      } else {
         rewrite_uses_to_var(b, store_val, s->position_value_var, store_pos_component);
      }
   } else {
      rewrite_uses_to_var(b, store_val, s->position_value_var, store_pos_component);
   }

   return true;
}

static void
remove_extra_pos_outputs(nir_shader *shader, lower_ngg_nogs_state *s)
{
   nir_shader_instructions_pass(shader, remove_extra_pos_output,
                                nir_metadata_block_index | nir_metadata_dominance,
                                s);
}

static bool
remove_compacted_arg(lower_ngg_nogs_state *s, nir_builder *b, unsigned idx)
{
   nir_instr *store_instr = s->compact_arg_stores[idx];
   if (!store_instr)
      return false;

   /* Simply remove the store. */
   nir_instr_remove(store_instr);

   /* Find the intrinsic that overwrites the shader arguments,
    * and change its corresponding source.
    * This will cause NIR's DCE to recognize the load and its phis as dead.
    */
   b->cursor = nir_before_instr(&s->overwrite_args->instr);
   nir_def *undef_arg = nir_undef(b, 1, 32);
   nir_def_rewrite_uses(s->overwrite_args->src[idx].ssa, undef_arg);

   s->compact_arg_stores[idx] = NULL;
   return true;
}

static bool
cleanup_culling_shader_after_dce(nir_shader *shader,
                                 nir_function_impl *function_impl,
                                 lower_ngg_nogs_state *s)
{
   bool uses_vs_vertex_id = false;
   bool uses_vs_instance_id = false;
   bool uses_tes_u = false;
   bool uses_tes_v = false;
   bool uses_tes_rel_patch_id = false;
   bool uses_tes_patch_id = false;

   bool progress = false;
   nir_builder b = nir_builder_create(function_impl);

   nir_foreach_block_reverse_safe(block, function_impl) {
      nir_foreach_instr_reverse_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

         switch (intrin->intrinsic) {
         case nir_intrinsic_sendmsg_amd:
            goto cleanup_culling_shader_after_dce_done;
         case nir_intrinsic_load_vertex_id:
         case nir_intrinsic_load_vertex_id_zero_base:
            uses_vs_vertex_id = true;
            break;
         case nir_intrinsic_load_instance_id:
            uses_vs_instance_id = true;
            break;
         case nir_intrinsic_load_input:
            if (s->options->instance_rate_inputs & BITFIELD_BIT(nir_intrinsic_base(intrin)))
               uses_vs_instance_id = true;
            else
               uses_vs_vertex_id = true;
            break;
         case nir_intrinsic_load_tess_coord:
            uses_tes_u = uses_tes_v = true;
            break;
         case nir_intrinsic_load_tess_rel_patch_id_amd:
            uses_tes_rel_patch_id = true;
            break;
         case nir_intrinsic_load_primitive_id:
            if (shader->info.stage == MESA_SHADER_TESS_EVAL)
               uses_tes_patch_id = true;
            break;
         default:
            break;
         }
      }
   }

   cleanup_culling_shader_after_dce_done:

   if (shader->info.stage == MESA_SHADER_VERTEX) {
      if (!uses_vs_vertex_id)
         progress |= remove_compacted_arg(s, &b, 0);
      if (!uses_vs_instance_id)
         progress |= remove_compacted_arg(s, &b, 1);
   } else if (shader->info.stage == MESA_SHADER_TESS_EVAL) {
      if (!uses_tes_u)
         progress |= remove_compacted_arg(s, &b, 0);
      if (!uses_tes_v)
         progress |= remove_compacted_arg(s, &b, 1);
      if (!uses_tes_rel_patch_id)
         progress |= remove_compacted_arg(s, &b, 3);
      if (!uses_tes_patch_id)
         progress |= remove_compacted_arg(s, &b, 2);
   }

   return progress;
}

/**
 * Perform vertex compaction after culling.
 *
 * 1. Repack surviving ES invocations (this determines which lane will export which vertex)
 * 2. Surviving ES vertex invocations store their data to LDS
 * 3. Emit GS_ALLOC_REQ
 * 4. Repacked invocations load the vertex data from LDS
 * 5. GS threads update their vertex indices
 */
static void
compact_vertices_after_culling(nir_builder *b,
                               lower_ngg_nogs_state *s,
                               nir_variable **repacked_variables,
                               nir_variable **gs_vtxaddr_vars,
                               nir_def *invocation_index,
                               nir_def *es_vertex_lds_addr,
                               nir_def *es_exporter_tid,
                               nir_def *num_live_vertices_in_workgroup,
                               unsigned pervertex_lds_bytes,
                               unsigned num_repacked_variables)
{
   nir_variable *es_accepted_var = s->es_accepted_var;
   nir_variable *gs_accepted_var = s->gs_accepted_var;
   nir_variable *position_value_var = s->position_value_var;
   nir_variable *prim_exp_arg_var = s->prim_exp_arg_var;

   nir_if *if_es_accepted = nir_push_if(b, nir_load_var(b, es_accepted_var));
   {
      nir_def *exporter_addr = pervertex_lds_addr(b, es_exporter_tid, pervertex_lds_bytes);

      /* Store the exporter thread's index to the LDS space of the current thread so GS threads can load it */
      nir_store_shared(b, nir_u2u8(b, es_exporter_tid), es_vertex_lds_addr, .base = lds_es_exporter_tid);

      /* Store the current thread's position output to the exporter thread's LDS space */
      nir_def *pos = nir_load_var(b, position_value_var);
      nir_store_shared(b, pos, exporter_addr, .base = lds_es_pos_x);

      /* Store the current thread's repackable arguments to the exporter thread's LDS space */
      for (unsigned i = 0; i < num_repacked_variables; ++i) {
         nir_def *arg_val = nir_load_var(b, repacked_variables[i]);
         nir_intrinsic_instr *store = nir_store_shared(b, arg_val, exporter_addr, .base = lds_es_arg_0 + 4u * i);

         s->compact_arg_stores[i] = &store->instr;
      }

      /* TES rel patch id does not cost extra dword */
      if (b->shader->info.stage == MESA_SHADER_TESS_EVAL) {
         nir_def *arg_val = nir_load_var(b, s->repacked_rel_patch_id);
         nir_intrinsic_instr *store =
            nir_store_shared(b, nir_u2u8(b, arg_val), exporter_addr,
                             .base = lds_es_tes_rel_patch_id);

         s->compact_arg_stores[3] = &store->instr;
      }
   }
   nir_pop_if(b, if_es_accepted);

   /* TODO: Consider adding a shortcut exit.
    * Waves that have no vertices and primitives left can s_endpgm right here.
    */

   nir_barrier(b, .execution_scope=SCOPE_WORKGROUP, .memory_scope=SCOPE_WORKGROUP,
                         .memory_semantics=NIR_MEMORY_ACQ_REL, .memory_modes=nir_var_mem_shared);

   nir_def *es_survived = nir_ilt(b, invocation_index, num_live_vertices_in_workgroup);
   nir_if *if_packed_es_thread = nir_push_if(b, es_survived);
   {
      /* Read position from the current ES thread's LDS space (written by the exported vertex's ES thread) */
      nir_def *exported_pos = nir_load_shared(b, 4, 32, es_vertex_lds_addr, .base = lds_es_pos_x);
      nir_store_var(b, position_value_var, exported_pos, 0xfu);

      /* Read the repacked arguments */
      for (unsigned i = 0; i < num_repacked_variables; ++i) {
         nir_def *arg_val = nir_load_shared(b, 1, 32, es_vertex_lds_addr, .base = lds_es_arg_0 + 4u * i);
         nir_store_var(b, repacked_variables[i], arg_val, 0x1u);
      }

      if (b->shader->info.stage == MESA_SHADER_TESS_EVAL) {
         nir_def *arg_val = nir_load_shared(b, 1, 8, es_vertex_lds_addr,
                                                .base = lds_es_tes_rel_patch_id);
         nir_store_var(b, s->repacked_rel_patch_id, nir_u2u32(b, arg_val), 0x1u);
      }
   }
   nir_push_else(b, if_packed_es_thread);
   {
      nir_store_var(b, position_value_var, nir_undef(b, 4, 32), 0xfu);
      for (unsigned i = 0; i < num_repacked_variables; ++i)
         nir_store_var(b, repacked_variables[i], nir_undef(b, 1, 32), 0x1u);
   }
   nir_pop_if(b, if_packed_es_thread);

   nir_if *if_gs_accepted = nir_push_if(b, nir_load_var(b, gs_accepted_var));
   {
      nir_def *exporter_vtx_indices[3] = {0};

      /* Load the index of the ES threads that will export the current GS thread's vertices */
      for (unsigned v = 0; v < s->options->num_vertices_per_primitive; ++v) {
         nir_def *vtx_addr = nir_load_var(b, gs_vtxaddr_vars[v]);
         nir_def *exporter_vtx_idx = nir_load_shared(b, 1, 8, vtx_addr, .base = lds_es_exporter_tid);
         exporter_vtx_indices[v] = nir_u2u32(b, exporter_vtx_idx);
         nir_store_var(b, s->gs_vtx_indices_vars[v], exporter_vtx_indices[v], 0x1);
      }

      nir_def *prim_exp_arg =
         emit_pack_ngg_prim_exp_arg(b, s->options->num_vertices_per_primitive,
                                    exporter_vtx_indices, NULL);
      nir_store_var(b, prim_exp_arg_var, prim_exp_arg, 0x1u);
   }
   nir_pop_if(b, if_gs_accepted);

   nir_store_var(b, es_accepted_var, es_survived, 0x1u);
}

static void
analyze_shader_before_culling_walk(nir_def *ssa,
                                   uint8_t flag,
                                   lower_ngg_nogs_state *s)
{
   nir_instr *instr = ssa->parent_instr;
   uint8_t old_pass_flags = instr->pass_flags;
   instr->pass_flags |= flag;

   if (instr->pass_flags == old_pass_flags)
      return; /* Already visited. */

   switch (instr->type) {
   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

      /* VS input loads and SSBO loads are actually VRAM reads on AMD HW. */
      switch (intrin->intrinsic) {
      case nir_intrinsic_load_input: {
         nir_io_semantics in_io_sem = nir_intrinsic_io_semantics(intrin);
         uint64_t in_mask = UINT64_C(1) << (uint64_t) in_io_sem.location;
         if (instr->pass_flags & nggc_passflag_used_by_pos)
            s->inputs_needed_by_pos |= in_mask;
         else if (instr->pass_flags & nggc_passflag_used_by_other)
            s->inputs_needed_by_others |= in_mask;
         break;
      }
      default:
         break;
      }

      break;
   }
   case nir_instr_type_alu: {
      nir_alu_instr *alu = nir_instr_as_alu(instr);
      unsigned num_srcs = nir_op_infos[alu->op].num_inputs;

      for (unsigned i = 0; i < num_srcs; ++i) {
         analyze_shader_before_culling_walk(alu->src[i].src.ssa, flag, s);
      }

      break;
   }
   case nir_instr_type_tex: {
      nir_tex_instr *tex = nir_instr_as_tex(instr);
      unsigned num_srcs = tex->num_srcs;

      for (unsigned i = 0; i < num_srcs; ++i) {
         analyze_shader_before_culling_walk(tex->src[i].src.ssa, flag, s);
      }

      break;
   }
   case nir_instr_type_phi: {
      nir_phi_instr *phi = nir_instr_as_phi(instr);
      nir_foreach_phi_src_safe(phi_src, phi) {
         analyze_shader_before_culling_walk(phi_src->src.ssa, flag, s);
      }

      break;
   }
   default:
      break;
   }
}

static void
analyze_shader_before_culling(nir_shader *shader, lower_ngg_nogs_state *s)
{
   /* LCSSA is needed to get correct results from divergence analysis. */
   nir_convert_to_lcssa(shader, true, true);
   /* We need divergence info for culling shaders. */
   nir_divergence_analysis(shader);

   nir_foreach_function_impl(impl, shader) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr(instr, block) {
            instr->pass_flags = 0;

            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            if (intrin->intrinsic != nir_intrinsic_store_output)
               continue;

            nir_io_semantics io_sem = nir_intrinsic_io_semantics(intrin);
            nir_def *store_val = intrin->src[0].ssa;
            uint8_t flag = io_sem.location == VARYING_SLOT_POS ? nggc_passflag_used_by_pos : nggc_passflag_used_by_other;
            analyze_shader_before_culling_walk(store_val, flag, s);
         }
      }
   }
}

static nir_def *
find_reusable_ssa_def(nir_instr *instr)
{
   /* Find instructions whose SSA definitions are used by both
    * the top and bottom parts of the shader (before and after culling).
    * Only in this case, it makes sense for the bottom part
    * to try to reuse these from the top part.
    */
   if ((instr->pass_flags & nggc_passflag_used_by_both) != nggc_passflag_used_by_both)
      return NULL;

   switch (instr->type) {
   case nir_instr_type_alu: {
      nir_alu_instr *alu = nir_instr_as_alu(instr);
      if (alu->def.divergent)
         return NULL;
      /* Ignore uniform floats because they regress VGPR usage too much */
      if (nir_op_infos[alu->op].output_type & nir_type_float)
         return NULL;
      return &alu->def;
   }
   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      if (!nir_intrinsic_can_reorder(intrin) ||
            !nir_intrinsic_infos[intrin->intrinsic].has_dest ||
            intrin->def.divergent)
         return NULL;
      return &intrin->def;
   }
   case nir_instr_type_phi: {
      nir_phi_instr *phi = nir_instr_as_phi(instr);
      if (phi->def.divergent)
         return NULL;
      return &phi->def;
   }
   default:
      return NULL;
   }
}

static const struct glsl_type *
glsl_uint_type_for_ssa(nir_def *ssa)
{
   enum glsl_base_type base_type = GLSL_TYPE_UINT;
   switch (ssa->bit_size) {
   case 8: base_type = GLSL_TYPE_UINT8; break;
   case 16: base_type = GLSL_TYPE_UINT16; break;
   case 32: base_type = GLSL_TYPE_UINT; break;
   case 64: base_type = GLSL_TYPE_UINT64; break;
   default: return NULL;
   }

   return ssa->num_components == 1
          ? glsl_scalar_type(base_type)
          : glsl_vector_type(base_type, ssa->num_components);
}

/**
 * Save the reusable SSA definitions to variables so that the
 * bottom shader part can reuse them from the top part.
 *
 * 1. We create a new function temporary variable for reusables,
 *    and insert a store+load.
 * 2. The shader is cloned (the top part is created), then the
 *    control flow is reinserted (for the bottom part.)
 * 3. For reusables, we delete the variable stores from the
 *    bottom part. This will make them use the variables from
 *    the top part and DCE the redundant instructions.
 */
static void
save_reusable_variables(nir_builder *b, lower_ngg_nogs_state *s)
{
   ASSERTED int vec_ok = u_vector_init(&s->reusable_nondeferred_variables, 4, sizeof(reusable_nondeferred_variable));
   assert(vec_ok);

   /* Upper limit on reusable uniforms in order to reduce SGPR spilling. */
   unsigned remaining_reusable_uniforms = 48;

   nir_block *block = nir_start_block(b->impl);
   while (block) {
      /* Process the instructions in the current block. */
      nir_foreach_instr_safe(instr, block) {
         /* Determine if we can reuse the current SSA value.
          * When vertex compaction is used, it is possible that the same shader invocation
          * processes a different vertex in the top and bottom part of the shader.
          * Therefore, we only reuse uniform values.
          */
         nir_def *ssa = find_reusable_ssa_def(instr);
         if (!ssa)
            continue;

         /* Determine a suitable type for the SSA value. */
         const struct glsl_type *t = glsl_uint_type_for_ssa(ssa);
         if (!t)
            continue;

         if (!ssa->divergent) {
            if (remaining_reusable_uniforms < ssa->num_components)
               continue;

            remaining_reusable_uniforms -= ssa->num_components;
         }

         reusable_nondeferred_variable *saved = (reusable_nondeferred_variable *) u_vector_add(&s->reusable_nondeferred_variables);
         assert(saved);

         /* Create a new NIR variable where we store the reusable value.
          * Then, we reload the variable and replace the uses of the value
          * with the reloaded variable.
          */
         saved->var = nir_local_variable_create(b->impl, t, NULL);
         saved->ssa = ssa;

         b->cursor = instr->type == nir_instr_type_phi
                     ? nir_after_instr_and_phis(instr)
                     : nir_after_instr(instr);
         nir_store_var(b, saved->var, saved->ssa, BITFIELD_MASK(ssa->num_components));
         nir_def *reloaded = nir_load_var(b, saved->var);
         nir_def_rewrite_uses_after(ssa, reloaded, reloaded->parent_instr);
      }

      /* Look at the next CF node. */
      nir_cf_node *next_cf_node = nir_cf_node_next(&block->cf_node);
      if (next_cf_node) {
         /* It makes no sense to try to reuse things from within loops. */
         bool next_is_loop = next_cf_node->type == nir_cf_node_loop;

         /* Don't reuse if we're in divergent control flow.
          *
          * Thanks to vertex repacking, the same shader invocation may process a different vertex
          * in the top and bottom part, and it's even possible that this different vertex was initially
          * processed in a different wave. So the two parts may take a different divergent code path.
          * Therefore, these variables in divergent control flow may stay undefined.
          *
          * Note that this problem doesn't exist if vertices are not repacked or if the
          * workgroup only has a single wave.
          */
         bool next_is_divergent_if =
            next_cf_node->type == nir_cf_node_if &&
            nir_cf_node_as_if(next_cf_node)->condition.ssa->divergent;

         if (next_is_loop || next_is_divergent_if) {
            block = nir_cf_node_cf_tree_next(next_cf_node);
            continue;
         }
      }

      /* Go to the next block. */
      block = nir_block_cf_tree_next(block);
   }
}

/**
 * Reuses suitable variables from the top part of the shader,
 * by deleting their stores from the bottom part.
 */
static void
apply_reusable_variables(nir_builder *b, lower_ngg_nogs_state *s)
{
   if (!u_vector_length(&s->reusable_nondeferred_variables)) {
      u_vector_finish(&s->reusable_nondeferred_variables);
      return;
   }

   nir_foreach_block_reverse_safe(block, b->impl) {
      nir_foreach_instr_reverse_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;
         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

         /* When we found any of these intrinsics, it means
          * we reached the top part and we must stop.
          */
         if (intrin->intrinsic == nir_intrinsic_sendmsg_amd)
            goto done;

         if (intrin->intrinsic != nir_intrinsic_store_deref)
            continue;
         nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
         if (deref->deref_type != nir_deref_type_var)
            continue;

         reusable_nondeferred_variable *saved;
         u_vector_foreach(saved, &s->reusable_nondeferred_variables) {
            if (saved->var == deref->var) {
               nir_instr_remove(instr);
            }
         }
      }
   }

   done:
   u_vector_finish(&s->reusable_nondeferred_variables);
}

static void
cull_primitive_accepted(nir_builder *b, void *state)
{
   lower_ngg_nogs_state *s = (lower_ngg_nogs_state *)state;

   nir_store_var(b, s->gs_accepted_var, nir_imm_true(b), 0x1u);

   /* Store the accepted state to LDS for ES threads */
   for (unsigned vtx = 0; vtx < s->options->num_vertices_per_primitive; ++vtx)
      nir_store_shared(b, nir_imm_intN_t(b, 1, 8), s->vtx_addr[vtx], .base = lds_es_vertex_accepted);
}

static void
clipdist_culling_es_part(nir_builder *b, lower_ngg_nogs_state *s,
                         nir_def *es_vertex_lds_addr)
{
   /* no gl_ClipDistance used but we have user defined clip plane */
   if (s->options->user_clip_plane_enable_mask && !s->has_clipdist) {
      /* use gl_ClipVertex if defined */
      nir_variable *clip_vertex_var =
         b->shader->info.outputs_written & BITFIELD64_BIT(VARYING_SLOT_CLIP_VERTEX) ?
         s->clip_vertex_var : s->position_value_var;
      nir_def *clip_vertex = nir_load_var(b, clip_vertex_var);

      /* clip against user defined clip planes */
      for (unsigned i = 0; i < 8; i++) {
         if (!(s->options->user_clip_plane_enable_mask & BITFIELD_BIT(i)))
            continue;

         nir_def *plane = nir_load_user_clip_plane(b, .ucp_id = i);
         nir_def *dist = nir_fdot(b, clip_vertex, plane);
         add_clipdist_bit(b, dist, i, s->clipdist_neg_mask_var);
      }

      s->has_clipdist = true;
   }

   /* store clipdist_neg_mask to LDS for culling latter in gs thread */
   if (s->has_clipdist) {
      nir_def *mask = nir_load_var(b, s->clipdist_neg_mask_var);
      nir_store_shared(b, nir_u2u8(b, mask), es_vertex_lds_addr,
                       .base = lds_es_clipdist_neg_mask);
   }
}

static unsigned
ngg_nogs_get_culling_pervertex_lds_size(gl_shader_stage stage,
                                        bool uses_instance_id,
                                        bool uses_primitive_id,
                                        unsigned *num_repacked_variables)
{
   /* Culling shaders must repack some variables because
    * the same shader invocation may process different vertices
    * before and after the culling algorithm.
    */

   unsigned num_repacked;
   if (stage == MESA_SHADER_VERTEX) {
      /* Vertex shaders repack:
       * - Vertex ID
       * - Instance ID (only if used)
       */
      num_repacked = uses_instance_id ? 2 : 1;
   } else {
      /* Tess eval shaders repack:
       * - U, V coordinates
       * - primitive ID (aka. patch id, only if used)
       * - relative patch id (not included here because doesn't need a dword)
       */
      assert(stage == MESA_SHADER_TESS_EVAL);
      num_repacked = uses_primitive_id ? 3 : 2;
   }

   if (num_repacked_variables)
      *num_repacked_variables = num_repacked;

   /* one odd dword to reduce LDS bank conflict */
   return (lds_es_arg_0 + num_repacked * 4u) | 4u;
}

static void
add_deferred_attribute_culling(nir_builder *b, nir_cf_list *original_extracted_cf, lower_ngg_nogs_state *s)
{
   bool uses_instance_id = BITSET_TEST(b->shader->info.system_values_read, SYSTEM_VALUE_INSTANCE_ID);
   bool uses_tess_primitive_id = BITSET_TEST(b->shader->info.system_values_read, SYSTEM_VALUE_PRIMITIVE_ID);

   unsigned num_repacked_variables;
   unsigned pervertex_lds_bytes =
      ngg_nogs_get_culling_pervertex_lds_size(b->shader->info.stage,
                                              uses_instance_id,
                                              uses_tess_primitive_id,
                                              &num_repacked_variables);

   nir_function_impl *impl = nir_shader_get_entrypoint(b->shader);

   /* Create some helper variables. */
   nir_variable *gs_vtxaddr_vars[3] = {
      nir_local_variable_create(impl, glsl_uint_type(), "gs_vtx0_addr"),
      nir_local_variable_create(impl, glsl_uint_type(), "gs_vtx1_addr"),
      nir_local_variable_create(impl, glsl_uint_type(), "gs_vtx2_addr"),
   };

   nir_variable *repacked_variables[3] = {
      nir_local_variable_create(impl, glsl_uint_type(), "repacked_var_0"),
      nir_local_variable_create(impl, glsl_uint_type(), "repacked_var_1"),
      nir_local_variable_create(impl, glsl_uint_type(), "repacked_var_2"),
   };

   /* Relative patch ID is a special case because it doesn't need an extra dword, repack separately. */
   s->repacked_rel_patch_id = nir_local_variable_create(impl, glsl_uint_type(), "repacked_rel_patch_id");

   if (s->options->clipdist_enable_mask ||
       s->options->user_clip_plane_enable_mask) {
      s->clip_vertex_var =
         nir_local_variable_create(impl, glsl_vec4_type(), "clip_vertex");
      s->clipdist_neg_mask_var =
         nir_local_variable_create(impl, glsl_uint_type(), "clipdist_neg_mask");

      /* init mask to 0 */
      nir_store_var(b, s->clipdist_neg_mask_var, nir_imm_int(b, 0), 1);
   }

   /* Top part of the culling shader (aka. position shader part)
    *
    * We clone the full ES shader and emit it here, but we only really care
    * about its position output, so we delete every other output from this part.
    * The position output is stored into a temporary variable, and reloaded later.
    */

   nir_def *es_thread = has_input_vertex(b);
   nir_if *if_es_thread = nir_push_if(b, es_thread);
   {
      /* Initialize the position output variable to zeroes, in case not all VS/TES invocations store the output.
       * The spec doesn't require it, but we use (0, 0, 0, 1) because some games rely on that.
       */
      nir_store_var(b, s->position_value_var, nir_imm_vec4(b, 0.0f, 0.0f, 0.0f, 1.0f), 0xfu);

      /* Now reinsert a clone of the shader code */
      struct hash_table *remap_table = _mesa_pointer_hash_table_create(NULL);
      nir_cf_list_clone_and_reinsert(original_extracted_cf, &if_es_thread->cf_node, b->cursor, remap_table);
      _mesa_hash_table_destroy(remap_table, NULL);
      b->cursor = nir_after_cf_list(&if_es_thread->then_list);

      /* Remember the current thread's shader arguments */
      if (b->shader->info.stage == MESA_SHADER_VERTEX) {
         nir_store_var(b, repacked_variables[0], nir_load_vertex_id_zero_base(b), 0x1u);
         if (uses_instance_id)
            nir_store_var(b, repacked_variables[1], nir_load_instance_id(b), 0x1u);
      } else if (b->shader->info.stage == MESA_SHADER_TESS_EVAL) {
         nir_store_var(b, s->repacked_rel_patch_id, nir_load_tess_rel_patch_id_amd(b), 0x1u);
         nir_def *tess_coord = nir_load_tess_coord(b);
         nir_store_var(b, repacked_variables[0], nir_channel(b, tess_coord, 0), 0x1u);
         nir_store_var(b, repacked_variables[1], nir_channel(b, tess_coord, 1), 0x1u);
         if (uses_tess_primitive_id)
            nir_store_var(b, repacked_variables[2], nir_load_primitive_id(b), 0x1u);
      } else {
         unreachable("Should be VS or TES.");
      }
   }
   nir_pop_if(b, if_es_thread);

   nir_store_var(b, s->es_accepted_var, es_thread, 0x1u);
   nir_def *gs_thread = has_input_primitive(b);
   nir_store_var(b, s->gs_accepted_var, gs_thread, 0x1u);

   /* Remove all non-position outputs, and put the position output into the variable. */
   nir_metadata_preserve(impl, nir_metadata_none);
   remove_culling_shader_outputs(b->shader, s);
   b->cursor = nir_after_impl(impl);

   nir_def *lds_scratch_base = nir_load_lds_ngg_scratch_base_amd(b);

   /* Run culling algorithms if culling is enabled.
    *
    * NGG culling can be enabled or disabled in runtime.
    * This is determined by a SGPR shader argument which is accessed
    * by the following NIR intrinsic.
    */

   nir_if *if_cull_en = nir_push_if(b, nir_load_cull_any_enabled_amd(b));
   {
      nir_def *invocation_index = nir_load_local_invocation_index(b);
      nir_def *es_vertex_lds_addr = pervertex_lds_addr(b, invocation_index, pervertex_lds_bytes);

      /* ES invocations store their vertex data to LDS for GS threads to read. */
      if_es_thread = nir_push_if(b, es_thread);
      if_es_thread->control = nir_selection_control_divergent_always_taken;
      {
         /* Store position components that are relevant to culling in LDS */
         nir_def *pre_cull_pos = nir_load_var(b, s->position_value_var);
         nir_def *pre_cull_w = nir_channel(b, pre_cull_pos, 3);
         nir_store_shared(b, pre_cull_w, es_vertex_lds_addr, .base = lds_es_pos_w);
         nir_def *pre_cull_x_div_w = nir_fdiv(b, nir_channel(b, pre_cull_pos, 0), pre_cull_w);
         nir_def *pre_cull_y_div_w = nir_fdiv(b, nir_channel(b, pre_cull_pos, 1), pre_cull_w);
         nir_store_shared(b, nir_vec2(b, pre_cull_x_div_w, pre_cull_y_div_w), es_vertex_lds_addr, .base = lds_es_pos_x);

         /* Clear out the ES accepted flag in LDS */
         nir_store_shared(b, nir_imm_zero(b, 1, 8), es_vertex_lds_addr, .align_mul = 4, .base = lds_es_vertex_accepted);

         /* For clipdist culling */
         clipdist_culling_es_part(b, s, es_vertex_lds_addr);
      }
      nir_pop_if(b, if_es_thread);

      nir_barrier(b, .execution_scope=SCOPE_WORKGROUP, .memory_scope=SCOPE_WORKGROUP,
                            .memory_semantics=NIR_MEMORY_ACQ_REL, .memory_modes=nir_var_mem_shared);

      nir_store_var(b, s->gs_accepted_var, nir_imm_false(b), 0x1u);
      nir_store_var(b, s->prim_exp_arg_var, nir_imm_int(b, 1u << 31), 0x1u);

      /* GS invocations load the vertex data and perform the culling. */
      nir_if *if_gs_thread = nir_push_if(b, gs_thread);
      {
         /* Load vertex indices from input VGPRs */
         nir_def *vtx_idx[3] = {0};
         for (unsigned vertex = 0; vertex < s->options->num_vertices_per_primitive;
              ++vertex)
            vtx_idx[vertex] = nir_load_var(b, s->gs_vtx_indices_vars[vertex]);

         nir_def *pos[3][4] = {0};

         /* Load W positions of vertices first because the culling code will use these first */
         for (unsigned vtx = 0; vtx < s->options->num_vertices_per_primitive; ++vtx) {
            s->vtx_addr[vtx] = pervertex_lds_addr(b, vtx_idx[vtx], pervertex_lds_bytes);
            pos[vtx][3] = nir_load_shared(b, 1, 32, s->vtx_addr[vtx], .base = lds_es_pos_w);
            nir_store_var(b, gs_vtxaddr_vars[vtx], s->vtx_addr[vtx], 0x1u);
         }

         /* Load the X/W, Y/W positions of vertices */
         for (unsigned vtx = 0; vtx < s->options->num_vertices_per_primitive; ++vtx) {
            nir_def *xy = nir_load_shared(b, 2, 32, s->vtx_addr[vtx], .base = lds_es_pos_x);
            pos[vtx][0] = nir_channel(b, xy, 0);
            pos[vtx][1] = nir_channel(b, xy, 1);
         }

         nir_def *accepted_by_clipdist;
         if (s->has_clipdist) {
            nir_def *clipdist_neg_mask = nir_imm_intN_t(b, 0xff, 8);
            for (unsigned vtx = 0; vtx < s->options->num_vertices_per_primitive; ++vtx) {
               nir_def *mask =
                  nir_load_shared(b, 1, 8, s->vtx_addr[vtx],
                                  .base = lds_es_clipdist_neg_mask);
               clipdist_neg_mask = nir_iand(b, clipdist_neg_mask, mask);
            }
            /* primitive is culled if any plane's clipdist of all vertices are negative */
            accepted_by_clipdist = nir_ieq_imm(b, clipdist_neg_mask, 0);
         } else {
            accepted_by_clipdist = nir_imm_true(b);
         }

         /* See if the current primitive is accepted */
         ac_nir_cull_primitive(b, accepted_by_clipdist, pos,
                               s->options->num_vertices_per_primitive,
                               cull_primitive_accepted, s);
      }
      nir_pop_if(b, if_gs_thread);

      nir_barrier(b, .execution_scope=SCOPE_WORKGROUP, .memory_scope=SCOPE_WORKGROUP,
                            .memory_semantics=NIR_MEMORY_ACQ_REL, .memory_modes=nir_var_mem_shared);

      nir_store_var(b, s->es_accepted_var, nir_imm_false(b), 0x1u);

      /* ES invocations load their accepted flag from LDS. */
      if_es_thread = nir_push_if(b, es_thread);
      if_es_thread->control = nir_selection_control_divergent_always_taken;
      {
         nir_def *accepted = nir_load_shared(b, 1, 8u, es_vertex_lds_addr, .base = lds_es_vertex_accepted, .align_mul = 4u);
         nir_def *accepted_bool = nir_ine_imm(b, nir_u2u32(b, accepted), 0);
         nir_store_var(b, s->es_accepted_var, accepted_bool, 0x1u);
      }
      nir_pop_if(b, if_es_thread);

      nir_def *es_accepted = nir_load_var(b, s->es_accepted_var);

      /* Repack the vertices that survived the culling. */
      wg_repack_result rep = repack_invocations_in_workgroup(b, es_accepted, lds_scratch_base,
                                                             s->max_num_waves,
                                                             s->options->wave_size);
      nir_def *num_live_vertices_in_workgroup = rep.num_repacked_invocations;
      nir_def *es_exporter_tid = rep.repacked_invocation_index;

      /* If all vertices are culled, set primitive count to 0 as well. */
      nir_def *num_exported_prims = nir_load_workgroup_num_input_primitives_amd(b);
      nir_def *fully_culled = nir_ieq_imm(b, num_live_vertices_in_workgroup, 0u);
      num_exported_prims = nir_bcsel(b, fully_culled, nir_imm_int(b, 0u), num_exported_prims);
      nir_store_var(b, s->gs_exported_var, nir_iand(b, nir_inot(b, fully_culled), has_input_primitive(b)), 0x1u);

      nir_if *if_wave_0 = nir_push_if(b, nir_ieq_imm(b, nir_load_subgroup_id(b), 0));
      {
         /* Tell the final vertex and primitive count to the HW. */
         if (s->options->gfx_level == GFX10) {
            alloc_vertices_and_primitives_gfx10_workaround(
               b, num_live_vertices_in_workgroup, num_exported_prims);
         } else {
            alloc_vertices_and_primitives(
               b, num_live_vertices_in_workgroup, num_exported_prims);
         }
      }
      nir_pop_if(b, if_wave_0);

      /* Vertex compaction. */
      compact_vertices_after_culling(b, s,
                                     repacked_variables, gs_vtxaddr_vars,
                                     invocation_index, es_vertex_lds_addr,
                                     es_exporter_tid, num_live_vertices_in_workgroup,
                                     pervertex_lds_bytes, num_repacked_variables);
   }
   nir_push_else(b, if_cull_en);
   {
      /* When culling is disabled, we do the same as we would without culling. */
      nir_if *if_wave_0 = nir_push_if(b, nir_ieq_imm(b, nir_load_subgroup_id(b), 0));
      {
         nir_def *vtx_cnt = nir_load_workgroup_num_input_vertices_amd(b);
         nir_def *prim_cnt = nir_load_workgroup_num_input_primitives_amd(b);
         alloc_vertices_and_primitives(b, vtx_cnt, prim_cnt);
      }
      nir_pop_if(b, if_wave_0);
      nir_store_var(b, s->prim_exp_arg_var, emit_ngg_nogs_prim_exp_arg(b, s), 0x1u);
   }
   nir_pop_if(b, if_cull_en);

   /* Update shader arguments.
    *
    * The registers which hold information about the subgroup's
    * vertices and primitives are updated here, so the rest of the shader
    * doesn't need to worry about the culling.
    *
    * These "overwrite" intrinsics must be at top level control flow,
    * otherwise they can mess up the backend (eg. ACO's SSA).
    *
    * TODO:
    * A cleaner solution would be to simply replace all usages of these args
    * with the load of the variables.
    * However, this wouldn't work right now because the backend uses the arguments
    * for purposes not expressed in NIR, eg. VS input loads, etc.
    * This can change if VS input loads and other stuff are lowered to eg. load_buffer_amd.
    */

   if (b->shader->info.stage == MESA_SHADER_VERTEX)
      s->overwrite_args =
         nir_overwrite_vs_arguments_amd(b,
            nir_load_var(b, repacked_variables[0]), nir_load_var(b, repacked_variables[1]));
   else if (b->shader->info.stage == MESA_SHADER_TESS_EVAL)
      s->overwrite_args =
         nir_overwrite_tes_arguments_amd(b,
            nir_load_var(b, repacked_variables[0]), nir_load_var(b, repacked_variables[1]),
            nir_load_var(b, repacked_variables[2]), nir_load_var(b, s->repacked_rel_patch_id));
   else
      unreachable("Should be VS or TES.");
}

static void
ngg_nogs_store_edgeflag_to_lds(nir_builder *b, lower_ngg_nogs_state *s)
{
   if (!s->outputs[VARYING_SLOT_EDGE][0])
      return;

   /* clamp user edge flag to 1 for latter bit operations */
   nir_def *edgeflag = s->outputs[VARYING_SLOT_EDGE][0];
   edgeflag = nir_umin(b, edgeflag, nir_imm_int(b, 1));

   /* user edge flag is stored at the beginning of a vertex if streamout is not enabled */
   unsigned offset = 0;
   if (s->streamout_enabled) {
      unsigned packed_location =
         util_bitcount64(b->shader->info.outputs_written & BITFIELD64_MASK(VARYING_SLOT_EDGE));
      offset = packed_location * 16;
   }

   nir_def *tid = nir_load_local_invocation_index(b);
   nir_def *addr = pervertex_lds_addr(b, tid, s->pervertex_lds_bytes);

   nir_store_shared(b, edgeflag, addr, .base = offset);
}

static void
ngg_nogs_store_xfb_outputs_to_lds(nir_builder *b, lower_ngg_nogs_state *s)
{
   nir_xfb_info *info = b->shader->xfb_info;

   uint64_t xfb_outputs = 0;
   unsigned xfb_outputs_16bit = 0;
   uint8_t xfb_mask[VARYING_SLOT_MAX] = {0};
   uint8_t xfb_mask_16bit_lo[16] = {0};
   uint8_t xfb_mask_16bit_hi[16] = {0};

   /* Get XFB output mask for each slot. */
   for (int i = 0; i < info->output_count; i++) {
      nir_xfb_output_info *out = info->outputs + i;

      if (out->location < VARYING_SLOT_VAR0_16BIT) {
         xfb_outputs |= BITFIELD64_BIT(out->location);
         xfb_mask[out->location] |= out->component_mask;
      } else {
         unsigned index = out->location - VARYING_SLOT_VAR0_16BIT;
         xfb_outputs_16bit |= BITFIELD_BIT(index);

         if (out->high_16bits)
            xfb_mask_16bit_hi[index] |= out->component_mask;
         else
            xfb_mask_16bit_lo[index] |= out->component_mask;
      }
   }

   nir_def *tid = nir_load_local_invocation_index(b);
   nir_def *addr = pervertex_lds_addr(b, tid, s->pervertex_lds_bytes);

   u_foreach_bit64(slot, xfb_outputs) {
      unsigned packed_location =
         util_bitcount64(b->shader->info.outputs_written & BITFIELD64_MASK(slot));

      unsigned mask = xfb_mask[slot];

      /* Clear unused components. */
      for (unsigned i = 0; i < 4; i++) {
         if (!s->outputs[slot][i])
            mask &= ~BITFIELD_BIT(i);
      }

      while (mask) {
         int start, count;
         u_bit_scan_consecutive_range(&mask, &start, &count);
         /* Outputs here are sure to be 32bit.
          *
          * 64bit outputs have been lowered to two 32bit. As 16bit outputs:
          *   Vulkan does not allow streamout outputs less than 32bit.
          *   OpenGL puts 16bit outputs in VARYING_SLOT_VAR0_16BIT.
          */
         nir_def *store_val = nir_vec(b, &s->outputs[slot][start], (unsigned)count);
         nir_store_shared(b, store_val, addr, .base = packed_location * 16 + start * 4);
      }
   }

   unsigned num_32bit_outputs = util_bitcount64(b->shader->info.outputs_written);
   u_foreach_bit64(slot, xfb_outputs_16bit) {
      unsigned packed_location = num_32bit_outputs +
         util_bitcount(b->shader->info.outputs_written_16bit & BITFIELD_MASK(slot));

      unsigned mask_lo = xfb_mask_16bit_lo[slot];
      unsigned mask_hi = xfb_mask_16bit_hi[slot];

      /* Clear unused components. */
      for (unsigned i = 0; i < 4; i++) {
         if (!s->outputs_16bit_lo[slot][i])
            mask_lo &= ~BITFIELD_BIT(i);
         if (!s->outputs_16bit_hi[slot][i])
            mask_hi &= ~BITFIELD_BIT(i);
      }

      nir_def **outputs_lo = s->outputs_16bit_lo[slot];
      nir_def **outputs_hi = s->outputs_16bit_hi[slot];
      nir_def *undef = nir_undef(b, 1, 16);

      unsigned mask = mask_lo | mask_hi;
      while (mask) {
         int start, count;
         u_bit_scan_consecutive_range(&mask, &start, &count);

         nir_def *values[4] = {0};
         for (int c = start; c < start + count; ++c) {
            nir_def *lo = mask_lo & BITFIELD_BIT(c) ? outputs_lo[c] : undef;
            nir_def *hi = mask_hi & BITFIELD_BIT(c) ? outputs_hi[c] : undef;

            /* extend 8/16 bit to 32 bit, 64 bit has been lowered */
            values[c - start] = nir_pack_32_2x16_split(b, lo, hi);
         }

         nir_def *store_val = nir_vec(b, values, (unsigned)count);
         nir_store_shared(b, store_val, addr, .base = packed_location * 16 + start * 4);
      }
   }
}

static void
ngg_build_streamout_buffer_info(nir_builder *b,
                                nir_xfb_info *info,
                                bool has_xfb_prim_query,
                                nir_def *scratch_base,
                                nir_def *tid_in_tg,
                                nir_def *gen_prim[4],
                                nir_def *prim_stride_ret[4],
                                nir_def *so_buffer_ret[4],
                                nir_def *buffer_offsets_ret[4],
                                nir_def *emit_prim_ret[4])
{
   nir_def *undef = nir_undef(b, 1, 32);

   /* For radeonsi which pass this value by arg when VS. Streamout need accurate
    * num-vert-per-prim for writing correct amount of data to buffer.
    */
   nir_def *num_vert_per_prim = nir_load_num_vertices_per_primitive_amd(b);
   for (unsigned buffer = 0; buffer < 4; buffer++) {
      if (!(info->buffers_written & BITFIELD_BIT(buffer)))
         continue;

      assert(info->buffers[buffer].stride);

      prim_stride_ret[buffer] =
         nir_imul_imm(b, num_vert_per_prim, info->buffers[buffer].stride);
      so_buffer_ret[buffer] = nir_load_streamout_buffer_amd(b, .base = buffer);
   }

   nir_if *if_invocation_0 = nir_push_if(b, nir_ieq_imm(b, tid_in_tg, 0));
   {
      nir_def *workgroup_buffer_sizes[4];
      for (unsigned buffer = 0; buffer < 4; buffer++) {
         if (info->buffers_written & BITFIELD_BIT(buffer)) {
            nir_def *buffer_size = nir_channel(b, so_buffer_ret[buffer], 2);
            /* In radeonsi, we may not know if a feedback buffer has been bound when
             * compile time, so have to check buffer size in runtime to disable the
             * GDS update for unbind buffer to prevent the case that previous draw
             * compiled with streamout but does not bind feedback buffer miss update
             * GDS which will affect current draw's streamout.
             */
            nir_def *buffer_valid = nir_ine_imm(b, buffer_size, 0);
            nir_def *inc_buffer_size =
               nir_imul(b, gen_prim[info->buffer_to_stream[buffer]], prim_stride_ret[buffer]);
            workgroup_buffer_sizes[buffer] =
               nir_bcsel(b, buffer_valid, inc_buffer_size, nir_imm_int(b, 0));
         } else
            workgroup_buffer_sizes[buffer] = undef;
      }

      nir_def *ordered_id = nir_load_ordered_id_amd(b);
      /* Get current global offset of buffer and increase by amount of
       * workgroup buffer size. This is an ordered operation sorted by
       * ordered_id; Each buffer info is in a channel of a vec4.
       */
      nir_def *buffer_offsets =
         nir_ordered_xfb_counter_add_amd(b, ordered_id, nir_vec(b, workgroup_buffer_sizes, 4),
                                         /* mask of buffers to update */
                                         .write_mask = info->buffers_written);

      nir_def *emit_prim[4];
      memcpy(emit_prim, gen_prim, 4 * sizeof(nir_def *));

      nir_def *any_overflow = nir_imm_false(b);
      nir_def *overflow_amount[4] = {undef, undef, undef, undef};

      for (unsigned buffer = 0; buffer < 4; buffer++) {
         if (!(info->buffers_written & BITFIELD_BIT(buffer)))
            continue;

         nir_def *buffer_size = nir_channel(b, so_buffer_ret[buffer], 2);

         /* Only consider overflow for valid feedback buffers because
          * otherwise the ordered operation above (GDS atomic return) might
          * return non-zero offsets for invalid buffers.
          */
         nir_def *buffer_valid = nir_ine_imm(b, buffer_size, 0);
         nir_def *buffer_offset = nir_channel(b, buffer_offsets, buffer);
         buffer_offset = nir_bcsel(b, buffer_valid, buffer_offset, nir_imm_int(b, 0));

         nir_def *remain_size = nir_isub(b, buffer_size, buffer_offset);
         nir_def *remain_prim = nir_idiv(b, remain_size, prim_stride_ret[buffer]);
         nir_def *overflow = nir_ilt(b, buffer_size, buffer_offset);

         any_overflow = nir_ior(b, any_overflow, overflow);
         overflow_amount[buffer] = nir_imax(b, nir_imm_int(b, 0),
                                            nir_isub(b, buffer_offset, buffer_size));

         unsigned stream = info->buffer_to_stream[buffer];
         /* when previous workgroup overflow, we can't emit any primitive */
         emit_prim[stream] = nir_bcsel(
            b, overflow, nir_imm_int(b, 0),
            /* we can emit part primitives, limited by smallest buffer */
            nir_imin(b, emit_prim[stream], remain_prim));

         /* Save to LDS for being accessed by other waves in this workgroup. */
         nir_store_shared(b, buffer_offset, scratch_base, .base = buffer * 4);
      }

      /* We have to fix up the streamout offsets if we overflowed because they determine
       * the vertex count for DrawTransformFeedback.
       */
      nir_if *if_any_overflow = nir_push_if(b, any_overflow);
      {
         nir_xfb_counter_sub_amd(b, nir_vec(b, overflow_amount, 4),
                                 /* mask of buffers to update */
                                 .write_mask = info->buffers_written);
      }
      nir_pop_if(b, if_any_overflow);

      /* Save to LDS for being accessed by other waves in this workgroup. */
      for (unsigned stream = 0; stream < 4; stream++) {
         if (!(info->streams_written & BITFIELD_BIT(stream)))
            continue;

         nir_store_shared(b, emit_prim[stream], scratch_base, .base = 16 + stream * 4);
      }

      /* Update shader query. */
      if (has_xfb_prim_query) {
         nir_if *if_shader_query = nir_push_if(b, nir_load_prim_xfb_query_enabled_amd(b));
         {
            for (unsigned stream = 0; stream < 4; stream++) {
               if (info->streams_written & BITFIELD_BIT(stream))
                  nir_atomic_add_xfb_prim_count_amd(b, emit_prim[stream], .stream_id = stream);
            }
         }
         nir_pop_if(b, if_shader_query);
      }
   }
   nir_pop_if(b, if_invocation_0);

   nir_barrier(b, .execution_scope = SCOPE_WORKGROUP,
                      .memory_scope = SCOPE_WORKGROUP,
                      .memory_semantics = NIR_MEMORY_ACQ_REL,
                      .memory_modes = nir_var_mem_shared);

   /* Fetch the per-buffer offsets in all waves. */
   for (unsigned buffer = 0; buffer < 4; buffer++) {
      if (!(info->buffers_written & BITFIELD_BIT(buffer)))
         continue;

      buffer_offsets_ret[buffer] =
         nir_load_shared(b, 1, 32, scratch_base, .base = buffer * 4);
   }

   /* Fetch the per-stream emit prim in all waves. */
   for (unsigned stream = 0; stream < 4; stream++) {
      if (!(info->streams_written & BITFIELD_BIT(stream)))
            continue;

      emit_prim_ret[stream] =
         nir_load_shared(b, 1, 32, scratch_base, .base = 16 + stream * 4);
   }
}

static void
ngg_build_streamout_vertex(nir_builder *b, nir_xfb_info *info,
                           unsigned stream, nir_def *so_buffer[4],
                           nir_def *buffer_offsets[4],
                           nir_def *vtx_buffer_idx, nir_def *vtx_lds_addr,
                           shader_output_types *output_types)
{
   nir_def *vtx_buffer_offsets[4];
   for (unsigned buffer = 0; buffer < 4; buffer++) {
      if (!(info->buffers_written & BITFIELD_BIT(buffer)))
         continue;

      nir_def *offset = nir_imul_imm(b, vtx_buffer_idx, info->buffers[buffer].stride);
      vtx_buffer_offsets[buffer] = nir_iadd(b, buffer_offsets[buffer], offset);
   }

   for (unsigned i = 0; i < info->output_count; i++) {
      nir_xfb_output_info *out = info->outputs + i;
      if (!out->component_mask || info->buffer_to_stream[out->buffer] != stream)
         continue;

      unsigned base;
      if (out->location >= VARYING_SLOT_VAR0_16BIT) {
         base =
            util_bitcount64(b->shader->info.outputs_written) +
            util_bitcount(b->shader->info.outputs_written_16bit &
                          BITFIELD_MASK(out->location - VARYING_SLOT_VAR0_16BIT));
      } else {
         base =
            util_bitcount64(b->shader->info.outputs_written &
                            BITFIELD64_MASK(out->location));
      }

      unsigned offset = (base * 4 + out->component_offset) * 4;
      unsigned count = util_bitcount(out->component_mask);

      assert(u_bit_consecutive(out->component_offset, count) == out->component_mask);

      nir_def *out_data =
         nir_load_shared(b, count, 32, vtx_lds_addr, .base = offset);

      /* Up-scaling 16bit outputs to 32bit.
       *
       * OpenGL ES will put 16bit medium precision varyings to VARYING_SLOT_VAR0_16BIT.
       * We need to up-scaling them to 32bit when streamout to buffer.
       *
       * Vulkan does not allow 8/16bit varyings to be streamout.
       */
      if (out->location >= VARYING_SLOT_VAR0_16BIT) {
         unsigned index = out->location - VARYING_SLOT_VAR0_16BIT;
         nir_def *values[4];

         for (int j = 0; j < count; j++) {
            unsigned c = out->component_offset + j;
            nir_def *v = nir_channel(b, out_data, j);
            nir_alu_type t;

            if (out->high_16bits) {
               v = nir_unpack_32_2x16_split_y(b, v);
               t = output_types->types_16bit_hi[index][c];
            } else {
               v = nir_unpack_32_2x16_split_x(b, v);
               t = output_types->types_16bit_lo[index][c];
            }

            t = nir_alu_type_get_base_type(t);
            values[j] = nir_convert_to_bit_size(b, v, t, 32);
         }

         out_data = nir_vec(b, values, count);
      }

      nir_def *zero = nir_imm_int(b, 0);
      nir_store_buffer_amd(b, out_data, so_buffer[out->buffer],
                           vtx_buffer_offsets[out->buffer],
                           zero, zero,
                           .base = out->offset,
                           .memory_modes = nir_var_mem_ssbo,
                           .access = ACCESS_NON_TEMPORAL);
   }
}

static void
ngg_nogs_build_streamout(nir_builder *b, lower_ngg_nogs_state *s)
{
   nir_xfb_info *info = b->shader->xfb_info;

   nir_def *lds_scratch_base = nir_load_lds_ngg_scratch_base_amd(b);

   /* Get global buffer offset where this workgroup will stream out data to. */
   nir_def *generated_prim = nir_load_workgroup_num_input_primitives_amd(b);
   nir_def *gen_prim_per_stream[4] = {generated_prim, 0, 0, 0};
   nir_def *emit_prim_per_stream[4] = {0};
   nir_def *buffer_offsets[4] = {0};
   nir_def *so_buffer[4] = {0};
   nir_def *prim_stride[4] = {0};
   nir_def *tid_in_tg = nir_load_local_invocation_index(b);
   ngg_build_streamout_buffer_info(b, info, s->options->has_xfb_prim_query,
                                   lds_scratch_base, tid_in_tg,
                                   gen_prim_per_stream, prim_stride,
                                   so_buffer, buffer_offsets,
                                   emit_prim_per_stream);

   /* Write out primitive data */
   nir_if *if_emit = nir_push_if(b, nir_ilt(b, tid_in_tg, emit_prim_per_stream[0]));
   {
      unsigned vtx_lds_stride = (b->shader->num_outputs * 4 + 1) * 4;
      nir_def *num_vert_per_prim = nir_load_num_vertices_per_primitive_amd(b);
      nir_def *vtx_buffer_idx = nir_imul(b, tid_in_tg, num_vert_per_prim);

      for (unsigned i = 0; i < s->options->num_vertices_per_primitive; i++) {
         nir_if *if_valid_vertex =
            nir_push_if(b, nir_igt_imm(b, num_vert_per_prim, i));
         {
            nir_def *vtx_lds_idx = nir_load_var(b, s->gs_vtx_indices_vars[i]);
            nir_def *vtx_lds_addr = pervertex_lds_addr(b, vtx_lds_idx, vtx_lds_stride);
            ngg_build_streamout_vertex(b, info, 0, so_buffer, buffer_offsets,
                                       nir_iadd_imm(b, vtx_buffer_idx, i),
                                       vtx_lds_addr, &s->output_types);
         }
         nir_pop_if(b, if_valid_vertex);
      }
   }
   nir_pop_if(b, if_emit);

   /* Wait streamout memory ops done before export primitive, otherwise it
    * may not finish when shader ends.
    *
    * If a shader has no param exports, rasterization can start before
    * the shader finishes and thus memory stores might not finish before
    * the pixel shader starts.
    *
    * TODO: we only need this when no param exports.
    *
    * TODO: not sure if we need this barrier when late prim export, as I
    *       can't observe test fail without this barrier.
    */
   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_RELEASE, nir_var_mem_ssbo);
}

static unsigned
ngg_nogs_get_pervertex_lds_size(gl_shader_stage stage,
                                unsigned shader_num_outputs,
                                bool streamout_enabled,
                                bool export_prim_id,
                                bool has_user_edgeflags)
{
   unsigned pervertex_lds_bytes = 0;

   if (streamout_enabled) {
      /* The extra dword is used to avoid LDS bank conflicts and store the primitive id.
       * TODO: only alloc space for outputs that really need streamout.
       */
      pervertex_lds_bytes = (shader_num_outputs * 4 + 1) * 4;
   }

   bool need_prim_id_store_shared = export_prim_id && stage == MESA_SHADER_VERTEX;
   if (need_prim_id_store_shared || has_user_edgeflags) {
      unsigned size = 0;
      if (need_prim_id_store_shared)
         size += 4;
      if (has_user_edgeflags)
         size += 4;

      /* pad to odd dwords to avoid LDS bank conflict */
      size |= 4;

      pervertex_lds_bytes = MAX2(pervertex_lds_bytes, size);
   }

   return pervertex_lds_bytes;
}

static void
ngg_nogs_gather_outputs(nir_builder *b, struct exec_list *cf_list, lower_ngg_nogs_state *s)
{
   /* Assume:
    * - the shader used nir_lower_io_to_temporaries
    * - 64-bit outputs are lowered
    * - no indirect indexing is present
    */
   struct nir_cf_node *first_node =
      exec_node_data(nir_cf_node, exec_list_get_head(cf_list), node);

   for (nir_block *block = nir_cf_node_cf_tree_first(first_node); block != NULL;
        block = nir_block_cf_tree_next(block)) {
      nir_foreach_instr_safe (instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic != nir_intrinsic_store_output)
            continue;

         assert(nir_src_is_const(intrin->src[1]) && !nir_src_as_uint(intrin->src[1]));

         nir_io_semantics sem = nir_intrinsic_io_semantics(intrin);
         unsigned slot = sem.location;

         nir_def **output;
         nir_alu_type *type;
         if (slot >= VARYING_SLOT_VAR0_16BIT) {
            unsigned index = slot - VARYING_SLOT_VAR0_16BIT;
            if (sem.high_16bits) {
               output = s->outputs_16bit_hi[index];
               type = s->output_types.types_16bit_hi[index];
            } else {
               output = s->outputs_16bit_lo[index];
               type = s->output_types.types_16bit_lo[index];
            }
         } else {
            output = s->outputs[slot];
            type = s->output_types.types[slot];
         }

         unsigned component = nir_intrinsic_component(intrin);
         unsigned write_mask = nir_intrinsic_write_mask(intrin);
         nir_alu_type src_type = nir_intrinsic_src_type(intrin);

         u_foreach_bit (i, write_mask) {
            unsigned c = component + i;
            output[c] = nir_channel(b, intrin->src[0].ssa, i);
            type[c] = src_type;
         }

         /* remove all store output instructions */
         nir_instr_remove(instr);
      }
   }
}

static unsigned
gather_vs_outputs(nir_builder *b, vs_output *outputs,
                  const uint8_t *param_offsets,
                  nir_def *(*data)[4],
                  nir_def *(*data_16bit_lo)[4],
                  nir_def *(*data_16bit_hi)[4])
{
   unsigned num_outputs = 0;
   u_foreach_bit64 (slot, b->shader->info.outputs_written) {
      if (param_offsets[slot] > AC_EXP_PARAM_OFFSET_31)
         continue;

      nir_def **output = data[slot];

      /* skip output if no one written before */
      if (!output[0] && !output[1] && !output[2] && !output[3])
         continue;

      outputs[num_outputs].slot = slot;
      for (int i = 0; i < 4; i++) {
         nir_def *chan = output[i];
         /* RADV implements 16-bit outputs as 32-bit with VARYING_SLOT_VAR0-31. */
         outputs[num_outputs].chan[i] = chan && chan->bit_size == 16 ? nir_u2u32(b, chan) : chan;
      }
      num_outputs++;
   }

   u_foreach_bit (i, b->shader->info.outputs_written_16bit) {
      unsigned slot = VARYING_SLOT_VAR0_16BIT + i;
      if (param_offsets[slot] > AC_EXP_PARAM_OFFSET_31)
         continue;

      nir_def **output_lo = data_16bit_lo[i];
      nir_def **output_hi = data_16bit_hi[i];

      /* skip output if no one written before */
      if (!output_lo[0] && !output_lo[1] && !output_lo[2] && !output_lo[3] &&
          !output_hi[0] && !output_hi[1] && !output_hi[2] && !output_hi[3])
         continue;

      vs_output *output = &outputs[num_outputs++];
      output->slot = slot;

      nir_def *undef = nir_undef(b, 1, 16);
      for (int j = 0; j < 4; j++) {
         nir_def *lo = output_lo[j] ? output_lo[j] : undef;
         nir_def *hi = output_hi[j] ? output_hi[j] : undef;
         if (output_lo[j] || output_hi[j])
            output->chan[j] = nir_pack_32_2x16_split(b, lo, hi);
         else
            output->chan[j] = NULL;
      }
   }

   return num_outputs;
}

static void
create_vertex_param_phis(nir_builder *b, unsigned num_outputs, vs_output *outputs)
{
   nir_def *undef = nir_undef(b, 1, 32); /* inserted at the start of the shader */

   for (unsigned i = 0; i < num_outputs; i++) {
      for (unsigned j = 0; j < 4; j++) {
         if (outputs[i].chan[j])
            outputs[i].chan[j] = nir_if_phi(b, outputs[i].chan[j], undef);
      }
   }
}

static void
export_vertex_params_gfx11(nir_builder *b, nir_def *export_tid, nir_def *num_export_threads,
                           unsigned num_outputs, vs_output *outputs,
                           const uint8_t *vs_output_param_offset)
{
   nir_def *attr_rsrc = nir_load_ring_attr_amd(b);

   /* We should always store full vec4s in groups of 8 lanes for the best performance even if
    * some of them are garbage or have unused components, so align the number of export threads
    * to 8.
    */
   num_export_threads = nir_iand_imm(b, nir_iadd_imm(b, num_export_threads, 7), ~7);
   if (!export_tid)
      nir_push_if(b, nir_is_subgroup_invocation_lt_amd(b, num_export_threads));
   else
      nir_push_if(b, nir_ult(b, export_tid, num_export_threads));

   nir_def *attr_offset = nir_load_ring_attr_offset_amd(b);
   nir_def *vindex = nir_load_local_invocation_index(b);
   nir_def *voffset = nir_imm_int(b, 0);
   nir_def *undef = nir_undef(b, 1, 32);

   uint32_t exported_params = 0;

   for (unsigned i = 0; i < num_outputs; i++) {
      gl_varying_slot slot = outputs[i].slot;
      unsigned offset = vs_output_param_offset[slot];

      /* Since vs_output_param_offset[] can map multiple varying slots to
       * the same param export index (that's radeonsi-specific behavior),
       * we need to do this so as not to emit duplicated exports.
       */
      if (exported_params & BITFIELD_BIT(offset))
         continue;

      nir_def *comp[4];
      for (unsigned j = 0; j < 4; j++)
         comp[j] = outputs[i].chan[j] ? outputs[i].chan[j] : undef;
      nir_store_buffer_amd(b, nir_vec(b, comp, 4), attr_rsrc, voffset, attr_offset, vindex,
                           .base = offset * 16,
                           .memory_modes = nir_var_shader_out,
                           .access = ACCESS_COHERENT | ACCESS_IS_SWIZZLED_AMD);
      exported_params |= BITFIELD_BIT(offset);
   }

   nir_pop_if(b, NULL);
}

static bool must_wait_attr_ring(enum amd_gfx_level gfx_level, bool has_param_exports)
{
   return (gfx_level == GFX11 || gfx_level == GFX11_5) && has_param_exports;
}

static void
export_pos0_wait_attr_ring(nir_builder *b, nir_if *if_es_thread, nir_def *outputs[VARYING_SLOT_MAX][4], const ac_nir_lower_ngg_options *options)
{
   b->cursor = nir_after_cf_node(&if_es_thread->cf_node);

   /* Create phi for the position output values. */
   vs_output pos_output = {
      .slot = VARYING_SLOT_POS,
      .chan = {
         outputs[VARYING_SLOT_POS][0],
         outputs[VARYING_SLOT_POS][1],
         outputs[VARYING_SLOT_POS][2],
         outputs[VARYING_SLOT_POS][3],
      },
   };
   create_vertex_param_phis(b, 1, &pos_output);

   b->cursor = nir_after_cf_list(&b->impl->body);

   /* Wait for attribute stores to finish. */
   nir_barrier(b, .execution_scope = SCOPE_SUBGROUP,
                  .memory_scope = SCOPE_DEVICE,
                  .memory_semantics = NIR_MEMORY_RELEASE,
                  .memory_modes = nir_var_mem_ssbo | nir_var_shader_out | nir_var_mem_global | nir_var_image);

   /* Export just the pos0 output. */
   nir_if *if_export_empty_pos = nir_push_if(b, if_es_thread->condition.ssa);
   {
      nir_def *pos_output_array[VARYING_SLOT_MAX][4] = {0};
      memcpy(pos_output_array[VARYING_SLOT_POS], pos_output.chan, sizeof(pos_output.chan));

      ac_nir_export_position(b, options->gfx_level,
                             options->clipdist_enable_mask,
                             !options->has_param_exports,
                             options->force_vrs, true,
                             VARYING_BIT_POS, pos_output_array, NULL);
   }
   nir_pop_if(b, if_export_empty_pos);
}

static void
nogs_export_vertex_params(nir_builder *b, nir_function_impl *impl,
                          nir_if *if_es_thread, nir_def *num_es_threads,
                          lower_ngg_nogs_state *s)
{
   if (!s->options->has_param_exports)
      return;

   if (s->options->gfx_level >= GFX11) {
      /* Export varyings for GFX11+ */
      vs_output outputs[64];
      const unsigned num_outputs =
         gather_vs_outputs(b, outputs,
                           s->options->vs_output_param_offset,
                           s->outputs,
                           s->outputs_16bit_lo,
                           s->outputs_16bit_hi);

      if (!num_outputs)
         return;

      b->cursor = nir_after_cf_node(&if_es_thread->cf_node);
      create_vertex_param_phis(b, num_outputs, outputs);

      b->cursor = nir_after_impl(impl);
      if (!num_es_threads)
         num_es_threads = nir_load_merged_wave_info_amd(b);

      export_vertex_params_gfx11(b, NULL, num_es_threads, num_outputs, outputs,
                                 s->options->vs_output_param_offset);
   } else {
      ac_nir_export_parameters(b, s->options->vs_output_param_offset,
                                 b->shader->info.outputs_written,
                                 b->shader->info.outputs_written_16bit,
                                 s->outputs, s->outputs_16bit_lo,
                                 s->outputs_16bit_hi);
   }
}

void
ac_nir_lower_ngg_nogs(nir_shader *shader, const ac_nir_lower_ngg_options *options)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   assert(impl);
   assert(options->max_workgroup_size && options->wave_size);
   assert(!(options->can_cull && options->passthrough));

   nir_variable *position_value_var = nir_local_variable_create(impl, glsl_vec4_type(), "position_value");
   nir_variable *prim_exp_arg_var = nir_local_variable_create(impl, glsl_uint_type(), "prim_exp_arg");
   nir_variable *es_accepted_var =
      options->can_cull ? nir_local_variable_create(impl, glsl_bool_type(), "es_accepted") : NULL;
   nir_variable *gs_accepted_var =
      options->can_cull ? nir_local_variable_create(impl, glsl_bool_type(), "gs_accepted") : NULL;
   nir_variable *gs_exported_var = nir_local_variable_create(impl, glsl_bool_type(), "gs_exported");

   bool streamout_enabled = shader->xfb_info && !options->disable_streamout;
   bool has_user_edgeflags =
      options->use_edgeflags && (shader->info.outputs_written & VARYING_BIT_EDGE);
   /* streamout need to be done before either prim or vertex export. Because when no
    * param export, rasterization can start right after prim and vertex export,
    * which left streamout buffer writes un-finished.
    *
    * Always use late prim export when user edge flags are enabled.
    * This is because edge flags are written by ES threads but they
    * are exported by GS threads as part of th primitive export.
    */
   bool early_prim_export =
      options->early_prim_export && !(streamout_enabled || has_user_edgeflags);

   lower_ngg_nogs_state state = {
      .options = options,
      .early_prim_export = early_prim_export,
      .streamout_enabled = streamout_enabled,
      .position_value_var = position_value_var,
      .prim_exp_arg_var = prim_exp_arg_var,
      .es_accepted_var = es_accepted_var,
      .gs_accepted_var = gs_accepted_var,
      .gs_exported_var = gs_exported_var,
      .max_num_waves = DIV_ROUND_UP(options->max_workgroup_size, options->wave_size),
      .has_user_edgeflags = has_user_edgeflags,
   };

   const bool need_prim_id_store_shared =
      options->export_primitive_id && shader->info.stage == MESA_SHADER_VERTEX;

   if (options->export_primitive_id) {
      nir_variable *prim_id_var = nir_variable_create(shader, nir_var_shader_out, glsl_uint_type(), "ngg_prim_id");
      prim_id_var->data.location = VARYING_SLOT_PRIMITIVE_ID;
      prim_id_var->data.driver_location = VARYING_SLOT_PRIMITIVE_ID;
      prim_id_var->data.interpolation = INTERP_MODE_NONE;
      shader->info.outputs_written |= VARYING_BIT_PRIMITIVE_ID;
   }

   nir_builder builder = nir_builder_create(impl);
   nir_builder *b = &builder; /* This is to avoid the & */

   if (options->can_cull) {
      analyze_shader_before_culling(shader, &state);
      save_reusable_variables(b, &state);
   }

   nir_cf_list extracted;
   nir_cf_extract(&extracted, nir_before_impl(impl),
                  nir_after_impl(impl));
   b->cursor = nir_before_impl(impl);

   ngg_nogs_init_vertex_indices_vars(b, impl, &state);

   /* Emit primitives generated query code here, so that
    * it executes before culling and isn't in the extracted CF.
    */
   nogs_prim_gen_query(b, &state);

   /* Whether a shader invocation should export a primitive,
    * initialize to all invocations that have an input primitive.
    */
   nir_store_var(b, gs_exported_var, has_input_primitive(b), 0x1u);

   if (!options->can_cull) {
      /* Newer chips can use PRIMGEN_PASSTHRU_NO_MSG to skip gs_alloc_req for NGG passthrough. */
      if (!(options->passthrough && options->family >= CHIP_NAVI23)) {
         /* Allocate export space on wave 0 - confirm to the HW that we want to use all possible space */
         nir_if *if_wave_0 = nir_push_if(b, nir_ieq_imm(b, nir_load_subgroup_id(b), 0));
         {
            nir_def *vtx_cnt = nir_load_workgroup_num_input_vertices_amd(b);
            nir_def *prim_cnt = nir_load_workgroup_num_input_primitives_amd(b);
            alloc_vertices_and_primitives(b, vtx_cnt, prim_cnt);
         }
         nir_pop_if(b, if_wave_0);
      }

      /* Take care of early primitive export, otherwise just pack the primitive export argument */
      if (state.early_prim_export)
         emit_ngg_nogs_prim_export(b, &state, NULL);
      else
         nir_store_var(b, prim_exp_arg_var, emit_ngg_nogs_prim_exp_arg(b, &state), 0x1u);
   } else {
      add_deferred_attribute_culling(b, &extracted, &state);
      b->cursor = nir_after_impl(impl);

      if (state.early_prim_export)
         emit_ngg_nogs_prim_export(b, &state, nir_load_var(b, state.prim_exp_arg_var));

      /* Wait for culling to finish using LDS. */
      if (need_prim_id_store_shared || has_user_edgeflags) {
         nir_barrier(b, .execution_scope = SCOPE_WORKGROUP,
                               .memory_scope = SCOPE_WORKGROUP,
                               .memory_semantics = NIR_MEMORY_ACQ_REL,
                               .memory_modes = nir_var_mem_shared);
      }
   }

   /* determine the LDS vertex stride */
   state.pervertex_lds_bytes =
      ngg_nogs_get_pervertex_lds_size(shader->info.stage,
                                      shader->num_outputs,
                                      state.streamout_enabled,
                                      options->export_primitive_id,
                                      state.has_user_edgeflags);

   if (need_prim_id_store_shared) {
      emit_ngg_nogs_prim_id_store_shared(b, &state);

      /* Wait for GS threads to store primitive ID in LDS. */
      nir_barrier(b, .execution_scope = SCOPE_WORKGROUP, .memory_scope = SCOPE_WORKGROUP,
                            .memory_semantics = NIR_MEMORY_ACQ_REL, .memory_modes = nir_var_mem_shared);
   }

   nir_def *es_thread =
      options->can_cull ? nir_load_var(b, es_accepted_var) : has_input_vertex(b);

   /* Calculate the bit count here instead of below for lower SGPR usage and better ALU
    * scheduling.
    */
   nir_def *num_es_threads = NULL;
   if (state.options->gfx_level >= GFX11 && options->can_cull) {
      nir_def *es_accepted_mask =
         nir_ballot(b, 1, options->wave_size, nir_load_var(b, es_accepted_var));
      num_es_threads = nir_bit_count(b, es_accepted_mask);
   }

   nir_if *if_es_thread = nir_push_if(b, es_thread);
   {
      /* Run the actual shader */
      nir_cf_reinsert(&extracted, b->cursor);
      b->cursor = nir_after_cf_list(&if_es_thread->then_list);

      if (options->export_primitive_id)
         emit_store_ngg_nogs_es_primitive_id(b, &state);
   }
   nir_pop_if(b, if_es_thread);

   if (options->can_cull) {
      /* Replace uniforms. */
      apply_reusable_variables(b, &state);

      /* Remove the redundant position output. */
      remove_extra_pos_outputs(shader, &state);

      /* After looking at the performance in apps eg. Doom Eternal, and The Witcher 3,
       * it seems that it's best to put the position export always at the end, and
       * then let ACO schedule it up (slightly) only when early prim export is used.
       */
      b->cursor = nir_after_cf_list(&if_es_thread->then_list);

      nir_def *pos_val = nir_load_var(b, state.position_value_var);
      for (int i = 0; i < 4; i++)
         state.outputs[VARYING_SLOT_POS][i] = nir_channel(b, pos_val, i);
   }

   /* Gather outputs data and types */
   b->cursor = nir_after_cf_list(&if_es_thread->then_list);
   ngg_nogs_gather_outputs(b, &if_es_thread->then_list, &state);

   if (state.has_user_edgeflags)
      ngg_nogs_store_edgeflag_to_lds(b, &state);

   if (state.streamout_enabled) {
      /* TODO: support culling after streamout. */
      assert(!options->can_cull);

      ngg_nogs_store_xfb_outputs_to_lds(b, &state);

      b->cursor = nir_after_impl(impl);
      ngg_nogs_build_streamout(b, &state);
   }

   /* Take care of late primitive export */
   if (!state.early_prim_export) {
      b->cursor = nir_after_impl(impl);
      emit_ngg_nogs_prim_export(b, &state, nir_load_var(b, prim_exp_arg_var));
   }

   uint64_t export_outputs = shader->info.outputs_written | VARYING_BIT_POS;
   if (options->kill_pointsize)
      export_outputs &= ~VARYING_BIT_PSIZ;
   if (options->kill_layer)
      export_outputs &= ~VARYING_BIT_LAYER;

   const bool wait_attr_ring = must_wait_attr_ring(options->gfx_level, options->has_param_exports);
   if (wait_attr_ring)
      export_outputs &= ~VARYING_BIT_POS;

   b->cursor = nir_after_cf_list(&if_es_thread->then_list);

   ac_nir_export_position(b, options->gfx_level,
                          options->clipdist_enable_mask,
                          !options->has_param_exports,
                          options->force_vrs, !wait_attr_ring,
                          export_outputs, state.outputs, NULL);

   nogs_export_vertex_params(b, impl, if_es_thread, num_es_threads, &state);

   if (wait_attr_ring)
      export_pos0_wait_attr_ring(b, if_es_thread, state.outputs, options);

   nir_metadata_preserve(impl, nir_metadata_none);
   nir_validate_shader(shader, "after emitting NGG VS/TES");

   /* Cleanup */
   nir_opt_dead_write_vars(shader);
   nir_lower_vars_to_ssa(shader);
   nir_remove_dead_variables(shader, nir_var_function_temp, NULL);
   nir_lower_alu_to_scalar(shader, NULL, NULL);
   nir_lower_phis_to_scalar(shader, true);

   if (options->can_cull) {
      /* It's beneficial to redo these opts after splitting the shader. */
      nir_opt_sink(shader, nir_move_load_input | nir_move_const_undef | nir_move_copies);
      nir_opt_move(shader, nir_move_load_input | nir_move_copies | nir_move_const_undef);
   }

   bool progress;
   do {
      progress = false;
      NIR_PASS(progress, shader, nir_opt_undef);
      NIR_PASS(progress, shader, nir_opt_dce);
      NIR_PASS(progress, shader, nir_opt_dead_cf);

      if (options->can_cull)
         progress |= cleanup_culling_shader_after_dce(shader, b->impl, &state);
   } while (progress);
}

/**
 * Return the address of the LDS storage reserved for the N'th vertex,
 * where N is in emit order, meaning:
 * - during the finale, N is the invocation_index (within the workgroup)
 * - during vertex emit, i.e. while the API GS shader invocation is running,
 *   N = invocation_index * gs_max_out_vertices + emit_idx
 *   where emit_idx is the vertex index in the current API GS invocation.
 *
 * Goals of the LDS memory layout:
 * 1. Eliminate bank conflicts on write for geometry shaders that have all emits
 *    in uniform control flow
 * 2. Eliminate bank conflicts on read for export if, additionally, there is no
 *    culling
 * 3. Agnostic to the number of waves (since we don't know it before compiling)
 * 4. Allow coalescing of LDS instructions (ds_write_b128 etc.)
 * 5. Avoid wasting memory.
 *
 * We use an AoS layout due to point 4 (this also helps point 3). In an AoS
 * layout, elimination of bank conflicts requires that each vertex occupy an
 * odd number of dwords. We use the additional dword to store the output stream
 * index as well as a flag to indicate whether this vertex ends a primitive
 * for rasterization.
 *
 * Swizzling is required to satisfy points 1 and 2 simultaneously.
 *
 * Vertices are stored in export order (gsthread * gs_max_out_vertices + emitidx).
 * Indices are swizzled in groups of 32, which ensures point 1 without
 * disturbing point 2.
 *
 * \return an LDS pointer to type {[N x i32], [4 x i8]}
 */
static nir_def *
ngg_gs_out_vertex_addr(nir_builder *b, nir_def *out_vtx_idx, lower_ngg_gs_state *s)
{
   unsigned write_stride_2exp = ffs(MAX2(b->shader->info.gs.vertices_out, 1)) - 1;

   /* gs_max_out_vertices = 2^(write_stride_2exp) * some odd number */
   if (write_stride_2exp) {
      nir_def *row = nir_ushr_imm(b, out_vtx_idx, 5);
      nir_def *swizzle = nir_iand_imm(b, row, (1u << write_stride_2exp) - 1u);
      out_vtx_idx = nir_ixor(b, out_vtx_idx, swizzle);
   }

   nir_def *out_vtx_offs = nir_imul_imm(b, out_vtx_idx, s->lds_bytes_per_gs_out_vertex);
   return nir_iadd_nuw(b, out_vtx_offs, s->lds_addr_gs_out_vtx);
}

static nir_def *
ngg_gs_emit_vertex_addr(nir_builder *b, nir_def *gs_vtx_idx, lower_ngg_gs_state *s)
{
   nir_def *tid_in_tg = nir_load_local_invocation_index(b);
   nir_def *gs_out_vtx_base = nir_imul_imm(b, tid_in_tg, b->shader->info.gs.vertices_out);
   nir_def *out_vtx_idx = nir_iadd_nuw(b, gs_out_vtx_base, gs_vtx_idx);

   return ngg_gs_out_vertex_addr(b, out_vtx_idx, s);
}

static void
ngg_gs_clear_primflags(nir_builder *b, nir_def *num_vertices, unsigned stream, lower_ngg_gs_state *s)
{
   char name[32];
   snprintf(name, sizeof(name), "clear_primflag_idx_%u", stream);
   nir_variable *clear_primflag_idx_var = nir_local_variable_create(b->impl, glsl_uint_type(), name);

   nir_def *zero_u8 = nir_imm_zero(b, 1, 8);
   nir_store_var(b, clear_primflag_idx_var, num_vertices, 0x1u);

   nir_loop *loop = nir_push_loop(b);
   {
      nir_def *clear_primflag_idx = nir_load_var(b, clear_primflag_idx_var);
      nir_if *if_break = nir_push_if(b, nir_uge_imm(b, clear_primflag_idx, b->shader->info.gs.vertices_out));
      {
         nir_jump(b, nir_jump_break);
      }
      nir_push_else(b, if_break);
      {
         nir_def *emit_vtx_addr = ngg_gs_emit_vertex_addr(b, clear_primflag_idx, s);
         nir_store_shared(b, zero_u8, emit_vtx_addr, .base = s->lds_offs_primflags + stream);
         nir_store_var(b, clear_primflag_idx_var, nir_iadd_imm_nuw(b, clear_primflag_idx, 1), 0x1u);
      }
      nir_pop_if(b, if_break);
   }
   nir_pop_loop(b, loop);
}

static bool
lower_ngg_gs_store_output(nir_builder *b, nir_intrinsic_instr *intrin, lower_ngg_gs_state *s)
{
   assert(nir_src_is_const(intrin->src[1]) && !nir_src_as_uint(intrin->src[1]));
   b->cursor = nir_before_instr(&intrin->instr);

   unsigned writemask = nir_intrinsic_write_mask(intrin);
   unsigned component_offset = nir_intrinsic_component(intrin);
   nir_io_semantics io_sem = nir_intrinsic_io_semantics(intrin);

   unsigned location = io_sem.location;

   nir_def *store_val = intrin->src[0].ssa;
   nir_alu_type src_type = nir_intrinsic_src_type(intrin);

   /* Small bitsize components consume the same amount of space as 32-bit components,
    * but 64-bit ones consume twice as many. (Vulkan spec 15.1.5)
    *
    * 64-bit IO has been lowered to multi 32-bit IO.
    */
   assert(store_val->bit_size <= 32);
   assert(nir_alu_type_get_type_size(src_type) == store_val->bit_size);

   /* Get corresponding output variable and usage info. */
   nir_def **output;
   nir_alu_type *type;
   gs_output_info *info;
   if (location >= VARYING_SLOT_VAR0_16BIT) {
      unsigned index = location - VARYING_SLOT_VAR0_16BIT;
      assert(index < 16);

      if (io_sem.high_16bits) {
         output = s->outputs_16bit_hi[index];
         type = s->output_types.types_16bit_hi[index];
         info = s->output_info_16bit_hi + index;
      } else {
         output = s->outputs_16bit_lo[index];
         type = s->output_types.types_16bit_lo[index];
         info = s->output_info_16bit_lo + index;
      }
   } else {
      assert(location < VARYING_SLOT_MAX);
      output = s->outputs[location];
      type = s->output_types.types[location];
      info = s->output_info + location;
   }

   for (unsigned comp = 0; comp < store_val->num_components; ++comp) {
      if (!(writemask & (1 << comp)))
         continue;
      unsigned stream = (io_sem.gs_streams >> (comp * 2)) & 0x3;
      if (!(b->shader->info.gs.active_stream_mask & (1 << stream)))
         continue;

      unsigned component = component_offset + comp;

      /* The same output component should always belong to the same stream. */
      assert(!(info->components_mask & (1 << component)) ||
             ((info->stream >> (component * 2)) & 3) == stream);

      /* Components of the same output slot may belong to different streams. */
      info->stream |= stream << (component * 2);
      info->components_mask |= BITFIELD_BIT(component);

      /* If type is set multiple times, the value must be same. */
      assert(type[component] == nir_type_invalid || type[component] == src_type);
      type[component] = src_type;

      /* Assume we have called nir_lower_io_to_temporaries which store output in the
       * same block as EmitVertex, so we don't need to use nir_variable for outputs.
       */
      output[component] = nir_channel(b, store_val, comp);
   }

   nir_instr_remove(&intrin->instr);
   return true;
}

static unsigned
gs_output_component_mask_with_stream(gs_output_info *info, unsigned stream)
{
   unsigned mask = info->components_mask;
   if (!mask)
      return 0;

   /* clear component when not requested stream */
   for (int i = 0; i < 4; i++) {
      if (((info->stream >> (i * 2)) & 3) != stream)
         mask &= ~(1 << i);
   }

   return mask;
}

static bool
lower_ngg_gs_emit_vertex_with_counter(nir_builder *b, nir_intrinsic_instr *intrin, lower_ngg_gs_state *s)
{
   b->cursor = nir_before_instr(&intrin->instr);

   unsigned stream = nir_intrinsic_stream_id(intrin);
   if (!(b->shader->info.gs.active_stream_mask & (1 << stream))) {
      nir_instr_remove(&intrin->instr);
      return true;
   }

   nir_def *gs_emit_vtx_idx = intrin->src[0].ssa;
   nir_def *current_vtx_per_prim = intrin->src[1].ssa;
   nir_def *gs_emit_vtx_addr = ngg_gs_emit_vertex_addr(b, gs_emit_vtx_idx, s);

   u_foreach_bit64(slot, b->shader->info.outputs_written) {
      unsigned packed_location = util_bitcount64((b->shader->info.outputs_written & BITFIELD64_MASK(slot)));
      gs_output_info *info = &s->output_info[slot];
      nir_def **output = s->outputs[slot];

      unsigned mask = gs_output_component_mask_with_stream(info, stream);
      while (mask) {
         int start, count;
         u_bit_scan_consecutive_range(&mask, &start, &count);
         nir_def *values[4] = {0};
         for (int c = start; c < start + count; ++c) {
            if (!output[c]) {
               /* no one write to this output before */
               values[c - start] = nir_undef(b, 1, 32);
               continue;
            }

            /* extend 8/16 bit to 32 bit, 64 bit has been lowered */
            values[c - start] = nir_u2uN(b, output[c], 32);
         }

         nir_def *store_val = nir_vec(b, values, (unsigned)count);
         nir_store_shared(b, store_val, gs_emit_vtx_addr,
                          .base = packed_location * 16 + start * 4,
                          .align_mul = 4);
      }

      /* Clear all outputs (they are undefined after emit_vertex) */
      memset(s->outputs[slot], 0, sizeof(s->outputs[slot]));
   }

   /* Store 16bit outputs to LDS. */
   unsigned num_32bit_outputs = util_bitcount64(b->shader->info.outputs_written);
   u_foreach_bit(slot, b->shader->info.outputs_written_16bit) {
      unsigned packed_location = num_32bit_outputs +
         util_bitcount(b->shader->info.outputs_written_16bit & BITFIELD_MASK(slot));

      unsigned mask_lo = gs_output_component_mask_with_stream(s->output_info_16bit_lo + slot, stream);
      unsigned mask_hi = gs_output_component_mask_with_stream(s->output_info_16bit_hi + slot, stream);
      unsigned mask = mask_lo | mask_hi;

      nir_def **output_lo = s->outputs_16bit_lo[slot];
      nir_def **output_hi = s->outputs_16bit_hi[slot];
      nir_def *undef = nir_undef(b, 1, 16);

      while (mask) {
         int start, count;
         u_bit_scan_consecutive_range(&mask, &start, &count);
         nir_def *values[4] = {0};
         for (int c = start; c < start + count; ++c) {
            nir_def *lo = output_lo[c] ? output_lo[c] : undef;
            nir_def *hi = output_hi[c] ? output_hi[c] : undef;

            values[c - start] = nir_pack_32_2x16_split(b, lo, hi);
         }

         nir_def *store_val = nir_vec(b, values, (unsigned)count);
         nir_store_shared(b, store_val, gs_emit_vtx_addr,
                          .base = packed_location * 16 + start * 4,
                          .align_mul = 4);
      }

      /* Clear all outputs (they are undefined after emit_vertex) */
      memset(s->outputs_16bit_lo[slot], 0, sizeof(s->outputs_16bit_lo[slot]));
      memset(s->outputs_16bit_hi[slot], 0, sizeof(s->outputs_16bit_hi[slot]));
   }

   /* Calculate and store per-vertex primitive flags based on vertex counts:
    * - bit 0: whether this vertex finishes a primitive (a real primitive, not the strip)
    * - bit 1: whether the primitive index is odd (if we are emitting triangle strips, otherwise always 0)
    *          only set when the vertex also finishes the primitive
    * - bit 2: whether vertex is live (if culling is enabled: set after culling, otherwise always 1)
    */

   nir_def *vertex_live_flag =
      !stream && s->options->can_cull
         ? nir_ishl_imm(b, nir_b2i32(b, nir_inot(b, nir_load_cull_any_enabled_amd(b))), 2)
         : nir_imm_int(b, 0b100);

   nir_def *completes_prim = nir_ige_imm(b, current_vtx_per_prim, s->num_vertices_per_primitive - 1);
   nir_def *complete_flag = nir_b2i32(b, completes_prim);

   nir_def *prim_flag = nir_ior(b, vertex_live_flag, complete_flag);
   if (s->num_vertices_per_primitive == 3) {
      nir_def *odd = nir_iand(b, current_vtx_per_prim, complete_flag);
      nir_def *odd_flag = nir_ishl_imm(b, odd, 1);
      prim_flag = nir_ior(b, prim_flag, odd_flag);
   }

   nir_store_shared(b, nir_u2u8(b, prim_flag), gs_emit_vtx_addr,
                    .base = s->lds_offs_primflags + stream,
                    .align_mul = 4, .align_offset = stream);

   nir_instr_remove(&intrin->instr);
   return true;
}

static bool
lower_ngg_gs_end_primitive_with_counter(nir_builder *b, nir_intrinsic_instr *intrin, UNUSED lower_ngg_gs_state *s)
{
   b->cursor = nir_before_instr(&intrin->instr);

   /* These are not needed, we can simply remove them */
   nir_instr_remove(&intrin->instr);
   return true;
}

static bool
lower_ngg_gs_set_vertex_and_primitive_count(nir_builder *b, nir_intrinsic_instr *intrin, lower_ngg_gs_state *s)
{
   b->cursor = nir_before_instr(&intrin->instr);

   unsigned stream = nir_intrinsic_stream_id(intrin);
   if (stream > 0 && !(b->shader->info.gs.active_stream_mask & (1 << stream))) {
      nir_instr_remove(&intrin->instr);
      return true;
   }

   s->vertex_count[stream] = intrin->src[0].ssa;
   s->primitive_count[stream] = intrin->src[1].ssa;

   /* Clear the primitive flags of non-emitted vertices */
   if (!nir_src_is_const(intrin->src[0]) || nir_src_as_uint(intrin->src[0]) < b->shader->info.gs.vertices_out)
      ngg_gs_clear_primflags(b, intrin->src[0].ssa, stream, s);

   nir_instr_remove(&intrin->instr);
   return true;
}

static bool
lower_ngg_gs_intrinsic(nir_builder *b, nir_instr *instr, void *state)
{
   lower_ngg_gs_state *s = (lower_ngg_gs_state *) state;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   if (intrin->intrinsic == nir_intrinsic_store_output)
      return lower_ngg_gs_store_output(b, intrin, s);
   else if (intrin->intrinsic == nir_intrinsic_emit_vertex_with_counter)
      return lower_ngg_gs_emit_vertex_with_counter(b, intrin, s);
   else if (intrin->intrinsic == nir_intrinsic_end_primitive_with_counter)
      return lower_ngg_gs_end_primitive_with_counter(b, intrin, s);
   else if (intrin->intrinsic == nir_intrinsic_set_vertex_and_primitive_count)
      return lower_ngg_gs_set_vertex_and_primitive_count(b, intrin, s);

   return false;
}

static void
lower_ngg_gs_intrinsics(nir_shader *shader, lower_ngg_gs_state *s)
{
   nir_shader_instructions_pass(shader, lower_ngg_gs_intrinsic, nir_metadata_none, s);
}

static void
ngg_gs_export_primitives(nir_builder *b, nir_def *max_num_out_prims, nir_def *tid_in_tg,
                         nir_def *exporter_tid_in_tg, nir_def *primflag_0,
                         lower_ngg_gs_state *s)
{
   nir_if *if_prim_export_thread = nir_push_if(b, nir_ilt(b, tid_in_tg, max_num_out_prims));

   /* Only bit 0 matters here - set it to 1 when the primitive should be null */
   nir_def *is_null_prim = nir_ixor(b, primflag_0, nir_imm_int(b, -1u));

   nir_def *vtx_indices[3] = {0};
   vtx_indices[s->num_vertices_per_primitive - 1] = exporter_tid_in_tg;
   if (s->num_vertices_per_primitive >= 2)
      vtx_indices[s->num_vertices_per_primitive - 2] = nir_iadd_imm(b, exporter_tid_in_tg, -1);
   if (s->num_vertices_per_primitive == 3)
      vtx_indices[s->num_vertices_per_primitive - 3] = nir_iadd_imm(b, exporter_tid_in_tg, -2);

   if (s->num_vertices_per_primitive == 3) {
      /* API GS outputs triangle strips, but NGG HW understands triangles.
       * We already know the triangles due to how we set the primitive flags, but we need to
       * make sure the vertex order is so that the front/back is correct, and the provoking vertex is kept.
       */

      nir_def *is_odd = nir_ubfe_imm(b, primflag_0, 1, 1);
      nir_def *provoking_vertex_index = nir_load_provoking_vtx_in_prim_amd(b);
      nir_def *provoking_vertex_first = nir_ieq_imm(b, provoking_vertex_index, 0);

      vtx_indices[0] = nir_bcsel(b, provoking_vertex_first, vtx_indices[0],
                                 nir_iadd(b, vtx_indices[0], is_odd));
      vtx_indices[1] = nir_bcsel(b, provoking_vertex_first,
                                 nir_iadd(b, vtx_indices[1], is_odd),
                                 nir_isub(b, vtx_indices[1], is_odd));
      vtx_indices[2] = nir_bcsel(b, provoking_vertex_first,
                                 nir_isub(b, vtx_indices[2], is_odd), vtx_indices[2]);
   }

   nir_def *arg = emit_pack_ngg_prim_exp_arg(b, s->num_vertices_per_primitive, vtx_indices,
                                                 is_null_prim);
   ac_nir_export_primitive(b, arg, NULL);
   nir_pop_if(b, if_prim_export_thread);
}

static void
ngg_gs_export_vertices(nir_builder *b, nir_def *max_num_out_vtx, nir_def *tid_in_tg,
                       nir_def *out_vtx_lds_addr, lower_ngg_gs_state *s)
{
   nir_if *if_vtx_export_thread = nir_push_if(b, nir_ilt(b, tid_in_tg, max_num_out_vtx));
   nir_def *exported_out_vtx_lds_addr = out_vtx_lds_addr;

   if (!s->output_compile_time_known) {
      /* Vertex compaction.
       * The current thread will export a vertex that was live in another invocation.
       * Load the index of the vertex that the current thread will have to export.
       */
      nir_def *exported_vtx_idx = nir_load_shared(b, 1, 8, out_vtx_lds_addr, .base = s->lds_offs_primflags + 1);
      exported_out_vtx_lds_addr = ngg_gs_out_vertex_addr(b, nir_u2u32(b, exported_vtx_idx), s);
   }

   u_foreach_bit64(slot, b->shader->info.outputs_written) {
      unsigned packed_location =
         util_bitcount64((b->shader->info.outputs_written & BITFIELD64_MASK(slot)));

      gs_output_info *info = &s->output_info[slot];
      unsigned mask = gs_output_component_mask_with_stream(info, 0);

      while (mask) {
         int start, count;
         u_bit_scan_consecutive_range(&mask, &start, &count);
         nir_def *load =
            nir_load_shared(b, count, 32, exported_out_vtx_lds_addr,
                            .base = packed_location * 16 + start * 4,
                            .align_mul = 4);

         for (int i = 0; i < count; i++)
            s->outputs[slot][start + i] = nir_channel(b, load, i);
      }
   }

   /* 16bit outputs */
   unsigned num_32bit_outputs = util_bitcount64(b->shader->info.outputs_written);
   u_foreach_bit(i, b->shader->info.outputs_written_16bit) {
      unsigned packed_location = num_32bit_outputs +
         util_bitcount(b->shader->info.outputs_written_16bit & BITFIELD_MASK(i));

      gs_output_info *info_lo = s->output_info_16bit_lo + i;
      gs_output_info *info_hi = s->output_info_16bit_hi + i;
      unsigned mask_lo = gs_output_component_mask_with_stream(info_lo, 0);
      unsigned mask_hi = gs_output_component_mask_with_stream(info_hi, 0);
      unsigned mask = mask_lo | mask_hi;

      while (mask) {
         int start, count;
         u_bit_scan_consecutive_range(&mask, &start, &count);
         nir_def *load =
            nir_load_shared(b, count, 32, exported_out_vtx_lds_addr,
                            .base = packed_location * 16 + start * 4,
                            .align_mul = 4);

         for (int j = 0; j < count; j++) {
            nir_def *val = nir_channel(b, load, j);
            unsigned comp = start + j;

            if (mask_lo & BITFIELD_BIT(comp))
               s->outputs_16bit_lo[i][comp] = nir_unpack_32_2x16_split_x(b, val);

            if (mask_hi & BITFIELD_BIT(comp))
               s->outputs_16bit_hi[i][comp] = nir_unpack_32_2x16_split_y(b, val);
         }
      }
   }

   uint64_t export_outputs = b->shader->info.outputs_written | VARYING_BIT_POS;
   if (s->options->kill_pointsize)
      export_outputs &= ~VARYING_BIT_PSIZ;
   if (s->options->kill_layer)
      export_outputs &= ~VARYING_BIT_LAYER;

   const bool wait_attr_ring = must_wait_attr_ring(s->options->gfx_level, s->options->has_param_exports);
   if (wait_attr_ring)
      export_outputs &= ~VARYING_BIT_POS;

   ac_nir_export_position(b, s->options->gfx_level,
                          s->options->clipdist_enable_mask,
                          !s->options->has_param_exports,
                          s->options->force_vrs, !wait_attr_ring,
                          export_outputs, s->outputs, NULL);

   nir_pop_if(b, if_vtx_export_thread);

   if (s->options->has_param_exports) {
      b->cursor = nir_after_cf_list(&if_vtx_export_thread->then_list);

      if (s->options->gfx_level >= GFX11) {
         vs_output outputs[64];
         unsigned num_outputs = gather_vs_outputs(b, outputs,
                                                  s->options->vs_output_param_offset,
                                                  s->outputs, s->outputs_16bit_lo,
                                                  s->outputs_16bit_hi);

         if (num_outputs) {
            b->cursor = nir_after_impl(s->impl);
            create_vertex_param_phis(b, num_outputs, outputs);

            export_vertex_params_gfx11(b, tid_in_tg, max_num_out_vtx, num_outputs, outputs,
                                       s->options->vs_output_param_offset);
         }
      } else {
         ac_nir_export_parameters(b, s->options->vs_output_param_offset,
                                  b->shader->info.outputs_written,
                                  b->shader->info.outputs_written_16bit,
                                  s->outputs, s->outputs_16bit_lo,
                                  s->outputs_16bit_hi);
      }
   }

   if (wait_attr_ring)
      export_pos0_wait_attr_ring(b, if_vtx_export_thread, s->outputs, s->options);
}

static void
ngg_gs_setup_vertex_compaction(nir_builder *b, nir_def *vertex_live, nir_def *tid_in_tg,
                               nir_def *exporter_tid_in_tg, lower_ngg_gs_state *s)
{
   assert(vertex_live->bit_size == 1);
   nir_if *if_vertex_live = nir_push_if(b, vertex_live);
   {
      /* Setup the vertex compaction.
       * Save the current thread's id for the thread which will export the current vertex.
       * We reuse stream 1 of the primitive flag of the other thread's vertex for storing this.
       */

      nir_def *exporter_lds_addr = ngg_gs_out_vertex_addr(b, exporter_tid_in_tg, s);
      nir_def *tid_in_tg_u8 = nir_u2u8(b, tid_in_tg);
      nir_store_shared(b, tid_in_tg_u8, exporter_lds_addr, .base = s->lds_offs_primflags + 1);
   }
   nir_pop_if(b, if_vertex_live);
}

static nir_def *
ngg_gs_load_out_vtx_primflag(nir_builder *b, unsigned stream, nir_def *tid_in_tg,
                             nir_def *vtx_lds_addr, nir_def *max_num_out_vtx,
                             lower_ngg_gs_state *s)
{
   nir_def *zero = nir_imm_int(b, 0);

   nir_if *if_outvtx_thread = nir_push_if(b, nir_ilt(b, tid_in_tg, max_num_out_vtx));
   nir_def *primflag = nir_load_shared(b, 1, 8, vtx_lds_addr,
                                           .base = s->lds_offs_primflags + stream);
   primflag = nir_u2u32(b, primflag);
   nir_pop_if(b, if_outvtx_thread);

   return nir_if_phi(b, primflag, zero);
}

static void
ngg_gs_out_prim_all_vtxptr(nir_builder *b, nir_def *last_vtxidx, nir_def *last_vtxptr,
                           nir_def *last_vtx_primflag, lower_ngg_gs_state *s,
                           nir_def *vtxptr[3])
{
   unsigned last_vtx = s->num_vertices_per_primitive - 1;
   vtxptr[last_vtx]= last_vtxptr;

   bool primitive_is_triangle = s->num_vertices_per_primitive == 3;
   nir_def *is_odd = primitive_is_triangle ?
      nir_ubfe_imm(b, last_vtx_primflag, 1, 1) : NULL;

   for (unsigned i = 0; i < s->num_vertices_per_primitive - 1; i++) {
      nir_def *vtxidx = nir_iadd_imm(b, last_vtxidx, -(last_vtx - i));

      /* Need to swap vertex 0 and vertex 1 when vertex 2 index is odd to keep
       * CW/CCW order for correct front/back face culling.
       */
      if (primitive_is_triangle)
         vtxidx = i == 0 ? nir_iadd(b, vtxidx, is_odd) : nir_isub(b, vtxidx, is_odd);

      vtxptr[i] = ngg_gs_out_vertex_addr(b, vtxidx, s);
   }
}

static nir_def *
ngg_gs_cull_primitive(nir_builder *b, nir_def *tid_in_tg, nir_def *max_vtxcnt,
                      nir_def *out_vtx_lds_addr, nir_def *out_vtx_primflag_0,
                      lower_ngg_gs_state *s)
{
   /* we haven't enabled point culling, if enabled this function could be further optimized */
   assert(s->num_vertices_per_primitive > 1);

   /* save the primflag so that we don't need to load it from LDS again */
   nir_variable *primflag_var = nir_local_variable_create(s->impl, glsl_uint_type(), "primflag");
   nir_store_var(b, primflag_var, out_vtx_primflag_0, 1);

   /* last bit of primflag indicate if this is the final vertex of a primitive */
   nir_def *is_end_prim_vtx = nir_i2b(b, nir_iand_imm(b, out_vtx_primflag_0, 1));
   nir_def *has_output_vertex = nir_ilt(b, tid_in_tg, max_vtxcnt);
   nir_def *prim_enable = nir_iand(b, is_end_prim_vtx, has_output_vertex);

   nir_if *if_prim_enable = nir_push_if(b, prim_enable);
   {
      /* Calculate the LDS address of every vertex in the current primitive. */
      nir_def *vtxptr[3];
      ngg_gs_out_prim_all_vtxptr(b, tid_in_tg, out_vtx_lds_addr, out_vtx_primflag_0, s, vtxptr);

      /* Load the positions from LDS. */
      nir_def *pos[3][4];
      for (unsigned i = 0; i < s->num_vertices_per_primitive; i++) {
         /* VARYING_SLOT_POS == 0, so base won't count packed location */
         pos[i][3] = nir_load_shared(b, 1, 32, vtxptr[i], .base = 12); /* W */
         nir_def *xy = nir_load_shared(b, 2, 32, vtxptr[i], .base = 0, .align_mul = 4);
         pos[i][0] = nir_channel(b, xy, 0);
         pos[i][1] = nir_channel(b, xy, 1);

         pos[i][0] = nir_fdiv(b, pos[i][0], pos[i][3]);
         pos[i][1] = nir_fdiv(b, pos[i][1], pos[i][3]);
      }

      /* TODO: support clipdist culling in GS */
      nir_def *accepted_by_clipdist = nir_imm_true(b);

      nir_def *accepted = ac_nir_cull_primitive(
         b, accepted_by_clipdist, pos, s->num_vertices_per_primitive, NULL, NULL);

      nir_if *if_rejected = nir_push_if(b, nir_inot(b, accepted));
      {
         /* clear the primflag if rejected */
         nir_store_shared(b, nir_imm_zero(b, 1, 8), out_vtx_lds_addr,
                          .base = s->lds_offs_primflags);

         nir_store_var(b, primflag_var, nir_imm_int(b, 0), 1);
      }
      nir_pop_if(b, if_rejected);
   }
   nir_pop_if(b, if_prim_enable);

   /* Wait for LDS primflag access done. */
   nir_barrier(b, .execution_scope = SCOPE_WORKGROUP,
                         .memory_scope = SCOPE_WORKGROUP,
                         .memory_semantics = NIR_MEMORY_ACQ_REL,
                         .memory_modes = nir_var_mem_shared);

   /* only dead vertex need a chance to relive */
   nir_def *vtx_is_dead = nir_ieq_imm(b, nir_load_var(b, primflag_var), 0);
   nir_def *vtx_update_primflag = nir_iand(b, vtx_is_dead, has_output_vertex);
   nir_if *if_update_primflag = nir_push_if(b, vtx_update_primflag);
   {
      /* get succeeding vertices' primflag to detect this vertex's liveness */
      for (unsigned i = 1; i < s->num_vertices_per_primitive; i++) {
         nir_def *vtxidx = nir_iadd_imm(b, tid_in_tg, i);
         nir_def *not_overflow = nir_ilt(b, vtxidx, max_vtxcnt);
         nir_if *if_not_overflow = nir_push_if(b, not_overflow);
         {
            nir_def *vtxptr = ngg_gs_out_vertex_addr(b, vtxidx, s);
            nir_def *vtx_primflag =
               nir_load_shared(b, 1, 8, vtxptr, .base = s->lds_offs_primflags);
            vtx_primflag = nir_u2u32(b, vtx_primflag);

            /* if succeeding vertex is alive end of primitive vertex, need to set current
             * thread vertex's liveness flag (bit 2)
             */
            nir_def *has_prim = nir_i2b(b, nir_iand_imm(b, vtx_primflag, 1));
            nir_def *vtx_live_flag =
               nir_bcsel(b, has_prim, nir_imm_int(b, 0b100), nir_imm_int(b, 0));

            /* update this vertex's primflag */
            nir_def *primflag = nir_load_var(b, primflag_var);
            primflag = nir_ior(b, primflag, vtx_live_flag);
            nir_store_var(b, primflag_var, primflag, 1);
         }
         nir_pop_if(b, if_not_overflow);
      }
   }
   nir_pop_if(b, if_update_primflag);

   return nir_load_var(b, primflag_var);
}

static void
ngg_gs_build_streamout(nir_builder *b, lower_ngg_gs_state *s)
{
   nir_xfb_info *info = b->shader->xfb_info;

   nir_def *tid_in_tg = nir_load_local_invocation_index(b);
   nir_def *max_vtxcnt = nir_load_workgroup_num_input_vertices_amd(b);
   nir_def *out_vtx_lds_addr = ngg_gs_out_vertex_addr(b, tid_in_tg, s);
   nir_def *prim_live[4] = {0};
   nir_def *gen_prim[4] = {0};
   nir_def *export_seq[4] = {0};
   nir_def *out_vtx_primflag[4] = {0};
   for (unsigned stream = 0; stream < 4; stream++) {
      if (!(info->streams_written & BITFIELD_BIT(stream)))
         continue;

      out_vtx_primflag[stream] =
         ngg_gs_load_out_vtx_primflag(b, stream, tid_in_tg, out_vtx_lds_addr, max_vtxcnt, s);

      /* Check bit 0 of primflag for primitive alive, it's set for every last
       * vertex of a primitive.
       */
      prim_live[stream] = nir_i2b(b, nir_iand_imm(b, out_vtx_primflag[stream], 1));

      unsigned scratch_stride = ALIGN(s->max_num_waves, 4);
      nir_def *scratch_base =
         nir_iadd_imm(b, s->lds_addr_gs_scratch, stream * scratch_stride);

      /* We want to export primitives to streamout buffer in sequence,
       * but not all vertices are alive or mark end of a primitive, so
       * there're "holes". We don't need continuous invocations to write
       * primitives to streamout buffer like final vertex export, so
       * just repack to get the sequence (export_seq) is enough, no need
       * to do compaction.
       *
       * Use separate scratch space for each stream to avoid barrier.
       * TODO: we may further reduce barriers by writing to all stream
       * LDS at once, then we only need one barrier instead of one each
       * stream..
       */
      wg_repack_result rep =
         repack_invocations_in_workgroup(b, prim_live[stream], scratch_base,
                                         s->max_num_waves, s->options->wave_size);

      /* nir_intrinsic_set_vertex_and_primitive_count can also get primitive count of
       * current wave, but still need LDS to sum all wave's count to get workgroup count.
       * And we need repack to export primitive to streamout buffer anyway, so do here.
       */
      gen_prim[stream] = rep.num_repacked_invocations;
      export_seq[stream] = rep.repacked_invocation_index;
   }

   /* Workgroup barrier: wait for LDS scratch reads finish. */
   nir_barrier(b, .execution_scope = SCOPE_WORKGROUP,
                      .memory_scope = SCOPE_WORKGROUP,
                      .memory_semantics = NIR_MEMORY_ACQ_REL,
                      .memory_modes = nir_var_mem_shared);

   /* Get global buffer offset where this workgroup will stream out data to. */
   nir_def *emit_prim[4] = {0};
   nir_def *buffer_offsets[4] = {0};
   nir_def *so_buffer[4] = {0};
   nir_def *prim_stride[4] = {0};
   ngg_build_streamout_buffer_info(b, info, s->options->has_xfb_prim_query,
                                   s->lds_addr_gs_scratch, tid_in_tg, gen_prim,
                                   prim_stride, so_buffer, buffer_offsets, emit_prim);

   for (unsigned stream = 0; stream < 4; stream++) {
      if (!(info->streams_written & BITFIELD_BIT(stream)))
         continue;

      nir_def *can_emit = nir_ilt(b, export_seq[stream], emit_prim[stream]);
      nir_if *if_emit = nir_push_if(b, nir_iand(b, can_emit, prim_live[stream]));
      {
         /* Get streamout buffer vertex index for the first vertex of this primitive. */
         nir_def *vtx_buffer_idx =
            nir_imul_imm(b, export_seq[stream], s->num_vertices_per_primitive);

         /* Get all vertices' lds address of this primitive. */
         nir_def *exported_vtx_lds_addr[3];
         ngg_gs_out_prim_all_vtxptr(b, tid_in_tg, out_vtx_lds_addr,
                                    out_vtx_primflag[stream], s,
                                    exported_vtx_lds_addr);

         /* Write all vertices of this primitive to streamout buffer. */
         for (unsigned i = 0; i < s->num_vertices_per_primitive; i++) {
            ngg_build_streamout_vertex(b, info, stream, so_buffer,
                                       buffer_offsets,
                                       nir_iadd_imm(b, vtx_buffer_idx, i),
                                       exported_vtx_lds_addr[i],
                                       &s->output_types);
         }
      }
      nir_pop_if(b, if_emit);
   }
}

static void
ngg_gs_finale(nir_builder *b, lower_ngg_gs_state *s)
{
   nir_def *tid_in_tg = nir_load_local_invocation_index(b);
   nir_def *max_vtxcnt = nir_load_workgroup_num_input_vertices_amd(b);
   nir_def *max_prmcnt = max_vtxcnt; /* They are currently practically the same; both RADV and RadeonSI do this. */
   nir_def *out_vtx_lds_addr = ngg_gs_out_vertex_addr(b, tid_in_tg, s);

   if (s->output_compile_time_known) {
      /* When the output is compile-time known, the GS writes all possible vertices and primitives it can.
       * The gs_alloc_req needs to happen on one wave only, otherwise the HW hangs.
       */
      nir_if *if_wave_0 = nir_push_if(b, nir_ieq_imm(b, nir_load_subgroup_id(b), 0));
      alloc_vertices_and_primitives(b, max_vtxcnt, max_prmcnt);
      nir_pop_if(b, if_wave_0);
   }

   /* Workgroup barrier already emitted, we can assume all GS output stores are done by now. */

   nir_def *out_vtx_primflag_0 = ngg_gs_load_out_vtx_primflag(b, 0, tid_in_tg, out_vtx_lds_addr, max_vtxcnt, s);

   if (s->output_compile_time_known) {
      ngg_gs_export_primitives(b, max_vtxcnt, tid_in_tg, tid_in_tg, out_vtx_primflag_0, s);
      ngg_gs_export_vertices(b, max_vtxcnt, tid_in_tg, out_vtx_lds_addr, s);
      return;
   }

   /* cull primitives */
   if (s->options->can_cull) {
      nir_if *if_cull_en = nir_push_if(b, nir_load_cull_any_enabled_amd(b));

      /* culling code will update the primflag */
      nir_def *updated_primflag =
         ngg_gs_cull_primitive(b, tid_in_tg, max_vtxcnt, out_vtx_lds_addr,
                               out_vtx_primflag_0, s);

      nir_pop_if(b, if_cull_en);

      out_vtx_primflag_0 = nir_if_phi(b, updated_primflag, out_vtx_primflag_0);
   }

   /* When the output vertex count is not known at compile time:
    * There may be gaps between invocations that have live vertices, but NGG hardware
    * requires that the invocations that export vertices are packed (ie. compact).
    * To ensure this, we need to repack invocations that have a live vertex.
    */
   nir_def *vertex_live = nir_ine_imm(b, out_vtx_primflag_0, 0);
   wg_repack_result rep = repack_invocations_in_workgroup(b, vertex_live, s->lds_addr_gs_scratch,
                                                          s->max_num_waves, s->options->wave_size);

   nir_def *workgroup_num_vertices = rep.num_repacked_invocations;
   nir_def *exporter_tid_in_tg = rep.repacked_invocation_index;

   /* When the workgroup emits 0 total vertices, we also must export 0 primitives (otherwise the HW can hang). */
   nir_def *any_output = nir_ine_imm(b, workgroup_num_vertices, 0);
   max_prmcnt = nir_bcsel(b, any_output, max_prmcnt, nir_imm_int(b, 0));

   /* Allocate export space. We currently don't compact primitives, just use the maximum number. */
   nir_if *if_wave_0 = nir_push_if(b, nir_ieq_imm(b, nir_load_subgroup_id(b), 0));
   {
      if (s->options->gfx_level == GFX10)
         alloc_vertices_and_primitives_gfx10_workaround(b, workgroup_num_vertices, max_prmcnt);
      else
         alloc_vertices_and_primitives(b, workgroup_num_vertices, max_prmcnt);
   }
   nir_pop_if(b, if_wave_0);

   /* Vertex compaction. This makes sure there are no gaps between threads that export vertices. */
   ngg_gs_setup_vertex_compaction(b, vertex_live, tid_in_tg, exporter_tid_in_tg, s);

   /* Workgroup barrier: wait for all LDS stores to finish. */
   nir_barrier(b, .execution_scope=SCOPE_WORKGROUP, .memory_scope=SCOPE_WORKGROUP,
                        .memory_semantics=NIR_MEMORY_ACQ_REL, .memory_modes=nir_var_mem_shared);

   ngg_gs_export_primitives(b, max_prmcnt, tid_in_tg, exporter_tid_in_tg, out_vtx_primflag_0, s);
   ngg_gs_export_vertices(b, workgroup_num_vertices, tid_in_tg, out_vtx_lds_addr, s);
}

void
ac_nir_lower_ngg_gs(nir_shader *shader, const ac_nir_lower_ngg_options *options)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   assert(impl);

   lower_ngg_gs_state state = {
      .options = options,
      .impl = impl,
      .max_num_waves = DIV_ROUND_UP(options->max_workgroup_size, options->wave_size),
      .lds_offs_primflags = options->gs_out_vtx_bytes,
      .lds_bytes_per_gs_out_vertex = options->gs_out_vtx_bytes + 4u,
      .streamout_enabled = shader->xfb_info && !options->disable_streamout,
   };

   if (!options->can_cull) {
      nir_gs_count_vertices_and_primitives(shader, state.const_out_vtxcnt,
                                           state.const_out_prmcnt, NULL, 4u);
      state.output_compile_time_known =
         state.const_out_vtxcnt[0] == shader->info.gs.vertices_out &&
         state.const_out_prmcnt[0] != -1;
   }

   if (shader->info.gs.output_primitive == MESA_PRIM_POINTS)
      state.num_vertices_per_primitive = 1;
   else if (shader->info.gs.output_primitive == MESA_PRIM_LINE_STRIP)
      state.num_vertices_per_primitive = 2;
   else if (shader->info.gs.output_primitive == MESA_PRIM_TRIANGLE_STRIP)
      state.num_vertices_per_primitive = 3;
   else
      unreachable("Invalid GS output primitive.");

   /* Extract the full control flow. It is going to be wrapped in an if statement. */
   nir_cf_list extracted;
   nir_cf_extract(&extracted, nir_before_impl(impl),
                  nir_after_impl(impl));

   nir_builder builder = nir_builder_at(nir_before_impl(impl));
   nir_builder *b = &builder; /* This is to avoid the & */

   /* Workgroup barrier: wait for ES threads */
   nir_barrier(b, .execution_scope=SCOPE_WORKGROUP, .memory_scope=SCOPE_WORKGROUP,
                         .memory_semantics=NIR_MEMORY_ACQ_REL, .memory_modes=nir_var_mem_shared);

   state.lds_addr_gs_out_vtx = nir_load_lds_ngg_gs_out_vertex_base_amd(b);
   state.lds_addr_gs_scratch = nir_load_lds_ngg_scratch_base_amd(b);

   /* Wrap the GS control flow. */
   nir_if *if_gs_thread = nir_push_if(b, has_input_primitive(b));

   nir_cf_reinsert(&extracted, b->cursor);
   b->cursor = nir_after_cf_list(&if_gs_thread->then_list);
   nir_pop_if(b, if_gs_thread);

   /* Workgroup barrier: wait for all GS threads to finish */
   nir_barrier(b, .execution_scope=SCOPE_WORKGROUP, .memory_scope=SCOPE_WORKGROUP,
                         .memory_semantics=NIR_MEMORY_ACQ_REL, .memory_modes=nir_var_mem_shared);

   if (state.streamout_enabled)
      ngg_gs_build_streamout(b, &state);

   /* Lower the GS intrinsics */
   lower_ngg_gs_intrinsics(shader, &state);

   if (!state.vertex_count[0]) {
      fprintf(stderr, "Could not find set_vertex_and_primitive_count for stream 0. This would hang your GPU.");
      abort();
   }

   /* Emit shader queries */
   b->cursor = nir_after_cf_list(&if_gs_thread->then_list);
   ac_nir_gs_shader_query(b,
                          state.options->has_gen_prim_query,
                          state.options->gfx_level < GFX11,
                          state.num_vertices_per_primitive,
                          state.options->wave_size,
                          state.vertex_count,
                          state.primitive_count);

   b->cursor = nir_after_impl(impl);

   /* Emit the finale sequence */
   ngg_gs_finale(b, &state);
   nir_validate_shader(shader, "after emitting NGG GS");

   /* Cleanup */
   nir_lower_vars_to_ssa(shader);
   nir_remove_dead_variables(shader, nir_var_function_temp, NULL);
   nir_metadata_preserve(impl, nir_metadata_none);
}

unsigned
ac_ngg_nogs_get_pervertex_lds_size(gl_shader_stage stage,
                                   unsigned shader_num_outputs,
                                   bool streamout_enabled,
                                   bool export_prim_id,
                                   bool has_user_edgeflags,
                                   bool can_cull,
                                   bool uses_instance_id,
                                   bool uses_primitive_id)
{
   /* for culling time lds layout only */
   unsigned culling_pervertex_lds_bytes = can_cull ?
      ngg_nogs_get_culling_pervertex_lds_size(
         stage, uses_instance_id, uses_primitive_id, NULL) : 0;

   unsigned pervertex_lds_bytes =
      ngg_nogs_get_pervertex_lds_size(stage, shader_num_outputs, streamout_enabled,
                                      export_prim_id, has_user_edgeflags);

   return MAX2(culling_pervertex_lds_bytes, pervertex_lds_bytes);
}

unsigned
ac_ngg_get_scratch_lds_size(gl_shader_stage stage,
                            unsigned workgroup_size,
                            unsigned wave_size,
                            bool streamout_enabled,
                            bool can_cull)
{
   unsigned scratch_lds_size = 0;
   unsigned max_num_waves = DIV_ROUND_UP(workgroup_size, wave_size);

   if (stage == MESA_SHADER_VERTEX || stage == MESA_SHADER_TESS_EVAL) {
      if (streamout_enabled) {
         /* 4 dwords for 4 streamout buffer offset, 1 dword for emit prim count */
         scratch_lds_size = 20;
      } else if (can_cull) {
         scratch_lds_size = ALIGN(max_num_waves, 4u);
      }
   } else {
      assert(stage == MESA_SHADER_GEOMETRY);

      scratch_lds_size = ALIGN(max_num_waves, 4u);
      /* streamout take 8 dwords for buffer offset and emit vertex per stream */
      if (streamout_enabled)
         scratch_lds_size = MAX2(scratch_lds_size, 32);
   }

   return scratch_lds_size;
}

static void
ms_store_prim_indices(nir_builder *b,
                      nir_def *val,
                      nir_def *offset_src,
                      lower_ngg_ms_state *s)
{
   assert(val->num_components <= 3);

   if (s->layout.var.prm_attr.mask & BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_INDICES)) {
      for (unsigned c = 0; c < s->vertices_per_prim; ++c)
         nir_store_var(b, s->out_variables[VARYING_SLOT_PRIMITIVE_INDICES * 4 + c], nir_channel(b, val, c), 0x1);
      return;
   }

   if (!offset_src)
      offset_src = nir_imm_int(b, 0);

   nir_store_shared(b, nir_u2u8(b, val), offset_src, .base = s->layout.lds.indices_addr);
}

static void
ms_store_cull_flag(nir_builder *b,
                   nir_def *val,
                   nir_def *offset_src,
                   lower_ngg_ms_state *s)
{
   assert(val->num_components == 1);
   assert(val->bit_size == 1);

   if (s->layout.var.prm_attr.mask & BITFIELD64_BIT(VARYING_SLOT_CULL_PRIMITIVE)) {
      nir_store_var(b, s->out_variables[VARYING_SLOT_CULL_PRIMITIVE * 4], nir_b2i32(b, val), 0x1);
      return;
   }

   if (!offset_src)
      offset_src = nir_imm_int(b, 0);

   nir_store_shared(b, nir_b2i8(b, val), offset_src, .base = s->layout.lds.cull_flags_addr);
}

static nir_def *
ms_arrayed_output_base_addr(nir_builder *b,
                            nir_def *arr_index,
                            unsigned driver_location,
                            unsigned num_arrayed_outputs)
{
   /* Address offset of the array item (vertex or primitive). */
   unsigned arr_index_stride = num_arrayed_outputs * 16u;
   nir_def *arr_index_off = nir_imul_imm(b, arr_index, arr_index_stride);

   /* IO address offset within the vertex or primitive data. */
   unsigned io_offset = driver_location * 16u;
   nir_def *io_off = nir_imm_int(b, io_offset);

   return nir_iadd_nuw(b, arr_index_off, io_off);
}

static void
update_ms_output_info_slot(lower_ngg_ms_state *s,
                           unsigned slot, unsigned base_off,
                           uint32_t components_mask)
{
   while (components_mask) {
      s->output_info[slot + base_off].components_mask |= components_mask & 0xF;

      components_mask >>= 4;
      base_off++;
   }
}

static void
update_ms_output_info(nir_intrinsic_instr *intrin,
                      const ms_out_part *out,
                      lower_ngg_ms_state *s)
{
   nir_io_semantics io_sem = nir_intrinsic_io_semantics(intrin);
   nir_src *base_offset_src = nir_get_io_offset_src(intrin);
   uint32_t write_mask = nir_intrinsic_write_mask(intrin);
   unsigned component_offset = nir_intrinsic_component(intrin);

   nir_def *store_val = intrin->src[0].ssa;
   write_mask = util_widen_mask(write_mask, DIV_ROUND_UP(store_val->bit_size, 32));
   uint32_t components_mask = write_mask << component_offset;

   if (nir_src_is_const(*base_offset_src)) {
      /* Simply mark the components of the current slot as used. */
      unsigned base_off = nir_src_as_uint(*base_offset_src);
      update_ms_output_info_slot(s, io_sem.location, base_off, components_mask);
   } else {
      /* Indirect offset: mark the components of all slots as used. */
      for (unsigned base_off = 0; base_off < io_sem.num_slots; ++base_off)
         update_ms_output_info_slot(s, io_sem.location, base_off, components_mask);
   }
}

static nir_def *
regroup_store_val(nir_builder *b, nir_def *store_val)
{
   /* Vulkan spec 15.1.4-15.1.5:
    *
    * The shader interface consists of output slots with 4x 32-bit components.
    * Small bitsize components consume the same space as 32-bit components,
    * but 64-bit ones consume twice as much.
    *
    * The same output slot may consist of components of different bit sizes.
    * Therefore for simplicity we don't store small bitsize components
    * contiguously, but pad them instead. In practice, they are converted to
    * 32-bit and then stored contiguously.
    */

   if (store_val->bit_size < 32) {
      assert(store_val->num_components <= 4);
      nir_def *comps[4] = {0};
      for (unsigned c = 0; c < store_val->num_components; ++c)
         comps[c] = nir_u2u32(b, nir_channel(b, store_val, c));
      return nir_vec(b, comps, store_val->num_components);
   }

   return store_val;
}

static nir_def *
regroup_load_val(nir_builder *b, nir_def *load, unsigned dest_bit_size)
{
   if (dest_bit_size == load->bit_size)
      return load;

   /* Small bitsize components are not stored contiguously, take care of that here. */
   unsigned num_components = load->num_components;
   assert(num_components <= 4);
   nir_def *components[4] = {0};
   for (unsigned i = 0; i < num_components; ++i)
      components[i] = nir_u2uN(b, nir_channel(b, load, i), dest_bit_size);

   return nir_vec(b, components, num_components);
}

static const ms_out_part *
ms_get_out_layout_part(unsigned location,
                       shader_info *info,
                       ms_out_mode *out_mode,
                       lower_ngg_ms_state *s)
{
   uint64_t mask = BITFIELD64_BIT(location);

   if (info->per_primitive_outputs & mask) {
      if (mask & s->layout.lds.prm_attr.mask) {
         *out_mode = ms_out_mode_lds;
         return &s->layout.lds.prm_attr;
      } else if (mask & s->layout.scratch_ring.prm_attr.mask) {
         *out_mode = ms_out_mode_scratch_ring;
         return &s->layout.scratch_ring.prm_attr;
      } else if (mask & s->layout.attr_ring.prm_attr.mask) {
         *out_mode = ms_out_mode_attr_ring;
         return &s->layout.attr_ring.prm_attr;
      } else if (mask & s->layout.var.prm_attr.mask) {
         *out_mode = ms_out_mode_var;
         return &s->layout.var.prm_attr;
      }
   } else {
      if (mask & s->layout.lds.vtx_attr.mask) {
         *out_mode = ms_out_mode_lds;
         return &s->layout.lds.vtx_attr;
      } else if (mask & s->layout.scratch_ring.vtx_attr.mask) {
         *out_mode = ms_out_mode_scratch_ring;
         return &s->layout.scratch_ring.vtx_attr;
      } else if (mask & s->layout.attr_ring.vtx_attr.mask) {
         *out_mode = ms_out_mode_attr_ring;
         return &s->layout.attr_ring.vtx_attr;
      } else if (mask & s->layout.var.vtx_attr.mask) {
         *out_mode = ms_out_mode_var;
         return &s->layout.var.vtx_attr;
      }
   }

   unreachable("Couldn't figure out mesh shader output mode.");
}

static void
ms_store_arrayed_output_intrin(nir_builder *b,
                               nir_intrinsic_instr *intrin,
                               lower_ngg_ms_state *s)
{
   unsigned location = nir_intrinsic_io_semantics(intrin).location;

   if (location == VARYING_SLOT_PRIMITIVE_INDICES) {
      /* EXT_mesh_shader primitive indices: array of vectors.
       * They don't count as per-primitive outputs, but the array is indexed
       * by the primitive index, so they are practically per-primitive.
       *
       * The max vertex count is 256, so these indices always fit 8 bits.
       * To reduce LDS use, store these as a flat array of 8-bit values.
       */
      assert(nir_src_is_const(*nir_get_io_offset_src(intrin)));
      assert(nir_src_as_uint(*nir_get_io_offset_src(intrin)) == 0);
      assert(nir_intrinsic_component(intrin) == 0);

      nir_def *store_val = intrin->src[0].ssa;
      nir_def *arr_index = nir_get_io_arrayed_index_src(intrin)->ssa;
      nir_def *offset = nir_imul_imm(b, arr_index, s->vertices_per_prim);
      ms_store_prim_indices(b, store_val, offset, s);
      return;
   } else if (location == VARYING_SLOT_CULL_PRIMITIVE) {
      /* EXT_mesh_shader cull primitive: per-primitive bool.
       * To reduce LDS use, store these as an array of 8-bit values.
       */
      assert(nir_src_is_const(*nir_get_io_offset_src(intrin)));
      assert(nir_src_as_uint(*nir_get_io_offset_src(intrin)) == 0);
      assert(nir_intrinsic_component(intrin) == 0);
      assert(nir_intrinsic_write_mask(intrin) == 1);

      nir_def *store_val = intrin->src[0].ssa;
      nir_def *arr_index = nir_get_io_arrayed_index_src(intrin)->ssa;
      nir_def *offset = nir_imul_imm(b, arr_index, s->vertices_per_prim);
      ms_store_cull_flag(b, store_val, offset, s);
      return;
   }

   ms_out_mode out_mode;
   const ms_out_part *out = ms_get_out_layout_part(location, &b->shader->info, &out_mode, s);
   update_ms_output_info(intrin, out, s);

   /* We compact the LDS size (we don't reserve LDS space for outputs which can
    * be stored in variables), so we can't rely on the original driver_location.
    * Instead, we compute the first free location based on the output mask.
    */
   unsigned driver_location = util_bitcount64(out->mask & u_bit_consecutive64(0, location));
   unsigned component_offset = nir_intrinsic_component(intrin);
   unsigned write_mask = nir_intrinsic_write_mask(intrin);
   unsigned num_outputs = util_bitcount64(out->mask);
   unsigned const_off = out->addr + component_offset * 4;

   nir_def *store_val = regroup_store_val(b, intrin->src[0].ssa);
   nir_def *arr_index = nir_get_io_arrayed_index_src(intrin)->ssa;
   nir_def *base_addr = ms_arrayed_output_base_addr(b, arr_index, driver_location, num_outputs);
   nir_def *base_offset = nir_get_io_offset_src(intrin)->ssa;
   nir_def *base_addr_off = nir_imul_imm(b, base_offset, 16u);
   nir_def *addr = nir_iadd_nuw(b, base_addr, base_addr_off);

   if (out_mode == ms_out_mode_lds) {
      nir_store_shared(b, store_val, addr, .base = const_off,
                     .write_mask = write_mask, .align_mul = 16,
                     .align_offset = const_off % 16);
   } else if (out_mode == ms_out_mode_scratch_ring) {
      nir_def *ring = nir_load_ring_mesh_scratch_amd(b);
      nir_def *off = nir_load_ring_mesh_scratch_offset_amd(b);
      nir_def *zero = nir_imm_int(b, 0);
      nir_store_buffer_amd(b, store_val, ring, addr, off, zero,
                           .base = const_off,
                           .write_mask = write_mask,
                           .memory_modes = nir_var_shader_out,
                           .access = ACCESS_COHERENT);
   } else if (out_mode == ms_out_mode_attr_ring) {
      /* GFX11+: Store params straight to the attribute ring.
       *
       * Even though the access pattern may not be the most optimal,
       * this is still much better than reserving LDS and losing waves.
       * (Also much better than storing and reloading from the scratch ring.)
       */
      const nir_io_semantics io_sem = nir_intrinsic_io_semantics(intrin);
      unsigned param_offset = s->vs_output_param_offset[io_sem.location];
      nir_def *ring = nir_load_ring_attr_amd(b);
      nir_def *soffset = nir_load_ring_attr_offset_amd(b);
      nir_store_buffer_amd(b, store_val, ring, base_addr_off, soffset, arr_index,
                           .base = const_off + param_offset * 16,
                           .write_mask = write_mask,
                           .memory_modes = nir_var_shader_out,
                           .access = ACCESS_COHERENT | ACCESS_IS_SWIZZLED_AMD);
   } else if (out_mode == ms_out_mode_var) {
      if (store_val->bit_size > 32) {
         /* Split 64-bit store values to 32-bit components. */
         store_val = nir_bitcast_vector(b, store_val, 32);
         /* Widen the write mask so it is in 32-bit components. */
         write_mask = util_widen_mask(write_mask, store_val->bit_size / 32);
      }

      u_foreach_bit(comp, write_mask) {
         nir_def *val = nir_channel(b, store_val, comp);
         unsigned idx = location * 4 + comp + component_offset;
         nir_store_var(b, s->out_variables[idx], val, 0x1);
      }
   } else {
      unreachable("Invalid MS output mode for store");
   }
}

static nir_def *
ms_load_arrayed_output(nir_builder *b,
                       nir_def *arr_index,
                       nir_def *base_offset,
                       unsigned location,
                       unsigned component_offset,
                       unsigned num_components,
                       unsigned load_bit_size,
                       lower_ngg_ms_state *s)
{
   ms_out_mode out_mode;
   const ms_out_part *out = ms_get_out_layout_part(location, &b->shader->info, &out_mode, s);

   unsigned component_addr_off = component_offset * 4;
   unsigned num_outputs = util_bitcount64(out->mask);
   unsigned const_off = out->addr + component_offset * 4;

   /* Use compacted driver location instead of the original. */
   unsigned driver_location = util_bitcount64(out->mask & u_bit_consecutive64(0, location));

   nir_def *base_addr = ms_arrayed_output_base_addr(b, arr_index, driver_location, num_outputs);
   nir_def *base_addr_off = nir_imul_imm(b, base_offset, 16);
   nir_def *addr = nir_iadd_nuw(b, base_addr, base_addr_off);

   if (out_mode == ms_out_mode_lds) {
      return nir_load_shared(b, num_components, load_bit_size, addr, .align_mul = 16,
                             .align_offset = component_addr_off % 16,
                             .base = const_off);
   } else if (out_mode == ms_out_mode_scratch_ring) {
      nir_def *ring = nir_load_ring_mesh_scratch_amd(b);
      nir_def *off = nir_load_ring_mesh_scratch_offset_amd(b);
      nir_def *zero = nir_imm_int(b, 0);
      return nir_load_buffer_amd(b, num_components, load_bit_size, ring, addr, off, zero,
                                 .base = const_off,
                                 .memory_modes = nir_var_shader_out,
                                 .access = ACCESS_COHERENT);
   } else if (out_mode == ms_out_mode_var) {
      nir_def *arr[8] = {0};
      unsigned num_32bit_components = num_components * load_bit_size / 32;
      for (unsigned comp = 0; comp < num_32bit_components; ++comp) {
         unsigned idx = location * 4 + comp + component_addr_off;
         arr[comp] = nir_load_var(b, s->out_variables[idx]);
      }
      if (load_bit_size > 32)
         return nir_extract_bits(b, arr, 1, 0, num_components, load_bit_size);
      return nir_vec(b, arr, num_components);
   } else {
      unreachable("Invalid MS output mode for load");
   }
}

static nir_def *
ms_load_arrayed_output_intrin(nir_builder *b,
                              nir_intrinsic_instr *intrin,
                              lower_ngg_ms_state *s)
{
   nir_def *arr_index = nir_get_io_arrayed_index_src(intrin)->ssa;
   nir_def *base_offset = nir_get_io_offset_src(intrin)->ssa;

   unsigned location = nir_intrinsic_io_semantics(intrin).location;
   unsigned component_offset = nir_intrinsic_component(intrin);
   unsigned bit_size = intrin->def.bit_size;
   unsigned num_components = intrin->def.num_components;
   unsigned load_bit_size = MAX2(bit_size, 32);

   nir_def *load =
      ms_load_arrayed_output(b, arr_index, base_offset, location, component_offset,
                             num_components, load_bit_size, s);

   return regroup_load_val(b, load, bit_size);
}

static nir_def *
lower_ms_load_workgroup_index(nir_builder *b,
                              UNUSED nir_intrinsic_instr *intrin,
                              lower_ngg_ms_state *s)
{
   return s->workgroup_index;
}

static nir_def *
lower_ms_set_vertex_and_primitive_count(nir_builder *b,
                                        nir_intrinsic_instr *intrin,
                                        lower_ngg_ms_state *s)
{
   /* If either the number of vertices or primitives is zero, set both of them to zero. */
   nir_def *num_vtx = nir_read_first_invocation(b, intrin->src[0].ssa);
   nir_def *num_prm = nir_read_first_invocation(b, intrin->src[1].ssa);
   nir_def *zero = nir_imm_int(b, 0);
   nir_def *is_either_zero = nir_ieq(b, nir_umin(b, num_vtx, num_prm), zero);
   num_vtx = nir_bcsel(b, is_either_zero, zero, num_vtx);
   num_prm = nir_bcsel(b, is_either_zero, zero, num_prm);

   nir_store_var(b, s->vertex_count_var, num_vtx, 0x1);
   nir_store_var(b, s->primitive_count_var, num_prm, 0x1);

   return NIR_LOWER_INSTR_PROGRESS_REPLACE;
}

static nir_def *
update_ms_barrier(nir_builder *b,
                         nir_intrinsic_instr *intrin,
                         lower_ngg_ms_state *s)
{
   /* Output loads and stores are lowered to shared memory access,
    * so we have to update the barriers to also reflect this.
    */
   unsigned mem_modes = nir_intrinsic_memory_modes(intrin);
   if (mem_modes & nir_var_shader_out)
      mem_modes |= nir_var_mem_shared;
   else
      return NULL;

   nir_intrinsic_set_memory_modes(intrin, mem_modes);

   return NIR_LOWER_INSTR_PROGRESS;
}

static nir_def *
lower_ms_intrinsic(nir_builder *b, nir_instr *instr, void *state)
{
   lower_ngg_ms_state *s = (lower_ngg_ms_state *) state;

   if (instr->type != nir_instr_type_intrinsic)
      return NULL;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   switch (intrin->intrinsic) {
   case nir_intrinsic_store_per_vertex_output:
   case nir_intrinsic_store_per_primitive_output:
      ms_store_arrayed_output_intrin(b, intrin, s);
      return NIR_LOWER_INSTR_PROGRESS_REPLACE;
   case nir_intrinsic_load_per_vertex_output:
   case nir_intrinsic_load_per_primitive_output:
      return ms_load_arrayed_output_intrin(b, intrin, s);
   case nir_intrinsic_barrier:
      return update_ms_barrier(b, intrin, s);
   case nir_intrinsic_load_workgroup_index:
      return lower_ms_load_workgroup_index(b, intrin, s);
   case nir_intrinsic_set_vertex_and_primitive_count:
      return lower_ms_set_vertex_and_primitive_count(b, intrin, s);
   default:
      unreachable("Not a lowerable mesh shader intrinsic.");
   }
}

static bool
filter_ms_intrinsic(const nir_instr *instr,
                    UNUSED const void *s)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   return intrin->intrinsic == nir_intrinsic_store_output ||
          intrin->intrinsic == nir_intrinsic_load_output ||
          intrin->intrinsic == nir_intrinsic_store_per_vertex_output ||
          intrin->intrinsic == nir_intrinsic_load_per_vertex_output ||
          intrin->intrinsic == nir_intrinsic_store_per_primitive_output ||
          intrin->intrinsic == nir_intrinsic_load_per_primitive_output ||
          intrin->intrinsic == nir_intrinsic_barrier ||
          intrin->intrinsic == nir_intrinsic_load_workgroup_index ||
          intrin->intrinsic == nir_intrinsic_set_vertex_and_primitive_count;
}

static void
lower_ms_intrinsics(nir_shader *shader, lower_ngg_ms_state *s)
{
   nir_shader_lower_instructions(shader, filter_ms_intrinsic, lower_ms_intrinsic, s);
}

static void
ms_emit_arrayed_outputs(nir_builder *b,
                        nir_def *invocation_index,
                        uint64_t mask,
                        lower_ngg_ms_state *s)
{
   nir_def *zero = nir_imm_int(b, 0);

   u_foreach_bit64(slot, mask) {
      /* Should not occur here, handled separately. */
      assert(slot != VARYING_SLOT_PRIMITIVE_COUNT && slot != VARYING_SLOT_PRIMITIVE_INDICES);

      unsigned component_mask = s->output_info[slot].components_mask;

      while (component_mask) {
         int start_comp = 0, num_components = 1;
         u_bit_scan_consecutive_range(&component_mask, &start_comp, &num_components);

         nir_def *load =
            ms_load_arrayed_output(b, invocation_index, zero, slot, start_comp,
                                   num_components, 32, s);

         for (int i = 0; i < num_components; i++)
            s->outputs[slot][start_comp + i] = nir_channel(b, load, i);
      }
   }
}

static void
ms_create_same_invocation_vars(nir_builder *b, lower_ngg_ms_state *s)
{
   /* Initialize NIR variables for same-invocation outputs. */
   uint64_t same_invocation_output_mask = s->layout.var.prm_attr.mask | s->layout.var.vtx_attr.mask;

   u_foreach_bit64(slot, same_invocation_output_mask) {
      for (unsigned comp = 0; comp < 4; ++comp) {
         unsigned idx = slot * 4 + comp;
         s->out_variables[idx] = nir_local_variable_create(b->impl, glsl_uint_type(), "ms_var_output");
      }
   }
}

static void
ms_emit_legacy_workgroup_index(nir_builder *b, lower_ngg_ms_state *s)
{
   /* Workgroup ID should have been lowered to workgroup index. */
   assert(!BITSET_TEST(b->shader->info.system_values_read, SYSTEM_VALUE_WORKGROUP_ID));

   /* No need to do anything if the shader doesn't use the workgroup index. */
   if (!BITSET_TEST(b->shader->info.system_values_read, SYSTEM_VALUE_WORKGROUP_INDEX))
      return;

   b->cursor = nir_before_impl(b->impl);

   /* Legacy fast launch mode (FAST_LAUNCH=1):
    *
    * The HW doesn't support a proper workgroup index for vertex processing stages,
    * so we use the vertex ID which is equivalent to the index of the current workgroup
    * within the current dispatch.
    *
    * Due to the register programming of mesh shaders, this value is only filled for
    * the first invocation of the first wave. To let other waves know, we use LDS.
    */
   nir_def *workgroup_index = nir_load_vertex_id_zero_base(b);

   if (s->api_workgroup_size <= s->wave_size) {
      /* API workgroup is small, so we don't need to use LDS. */
      s->workgroup_index = nir_read_first_invocation(b, workgroup_index);
      return;
   }

   unsigned workgroup_index_lds_addr = s->layout.lds.workgroup_info_addr + lds_ms_wg_index;

   nir_def *zero = nir_imm_int(b, 0);
   nir_def *dont_care = nir_undef(b, 1, 32);
   nir_def *loaded_workgroup_index = NULL;

   /* Use elect to make sure only 1 invocation uses LDS. */
   nir_if *if_elected = nir_push_if(b, nir_elect(b, 1));
   {
      nir_def *wave_id = nir_load_subgroup_id(b);
      nir_if *if_wave_0 = nir_push_if(b, nir_ieq_imm(b, wave_id, 0));
      {
         nir_store_shared(b, workgroup_index, zero, .base = workgroup_index_lds_addr);
         nir_barrier(b, .execution_scope = SCOPE_WORKGROUP,
                               .memory_scope = SCOPE_WORKGROUP,
                               .memory_semantics = NIR_MEMORY_ACQ_REL,
                               .memory_modes = nir_var_mem_shared);
      }
      nir_push_else(b, if_wave_0);
      {
         nir_barrier(b, .execution_scope = SCOPE_WORKGROUP,
                               .memory_scope = SCOPE_WORKGROUP,
                               .memory_semantics = NIR_MEMORY_ACQ_REL,
                               .memory_modes = nir_var_mem_shared);
         loaded_workgroup_index = nir_load_shared(b, 1, 32, zero, .base = workgroup_index_lds_addr);
      }
      nir_pop_if(b, if_wave_0);

      workgroup_index = nir_if_phi(b, workgroup_index, loaded_workgroup_index);
   }
   nir_pop_if(b, if_elected);

   workgroup_index = nir_if_phi(b, workgroup_index, dont_care);
   s->workgroup_index = nir_read_first_invocation(b, workgroup_index);
}

static void
set_ms_final_output_counts(nir_builder *b,
                           lower_ngg_ms_state *s,
                           nir_def **out_num_prm,
                           nir_def **out_num_vtx)
{
   /* The spec allows the numbers to be divergent, and in that case we need to
    * use the values from the first invocation. Also the HW requires us to set
    * both to 0 if either was 0.
    *
    * These are already done by the lowering.
    */
   nir_def *num_prm = nir_load_var(b, s->primitive_count_var);
   nir_def *num_vtx = nir_load_var(b, s->vertex_count_var);

   if (s->hw_workgroup_size <= s->wave_size) {
      /* Single-wave mesh shader workgroup. */
      alloc_vertices_and_primitives(b, num_vtx, num_prm);
      *out_num_prm = num_prm;
      *out_num_vtx = num_vtx;
      return;
   }

   /* Multi-wave mesh shader workgroup:
    * We need to use LDS to distribute the correct values to the other waves.
    *
    * TODO:
    * If we can prove that the values are workgroup-uniform, we can skip this
    * and just use whatever the current wave has. However, NIR divergence analysis
    * currently doesn't support this.
    */

   nir_def *zero = nir_imm_int(b, 0);

   nir_if *if_wave_0 = nir_push_if(b, nir_ieq_imm(b, nir_load_subgroup_id(b), 0));
   {
      nir_if *if_elected = nir_push_if(b, nir_elect(b, 1));
      {
         nir_store_shared(b, nir_vec2(b, num_prm, num_vtx), zero,
                          .base = s->layout.lds.workgroup_info_addr + lds_ms_num_prims);
      }
      nir_pop_if(b, if_elected);

      nir_barrier(b, .execution_scope = SCOPE_WORKGROUP,
                            .memory_scope = SCOPE_WORKGROUP,
                            .memory_semantics = NIR_MEMORY_ACQ_REL,
                            .memory_modes = nir_var_mem_shared);

      alloc_vertices_and_primitives(b, num_vtx, num_prm);
   }
   nir_push_else(b, if_wave_0);
   {
      nir_barrier(b, .execution_scope = SCOPE_WORKGROUP,
                            .memory_scope = SCOPE_WORKGROUP,
                            .memory_semantics = NIR_MEMORY_ACQ_REL,
                            .memory_modes = nir_var_mem_shared);

      nir_def *prm_vtx = NULL;
      nir_def *dont_care_2x32 = nir_undef(b, 2, 32);
      nir_if *if_elected = nir_push_if(b, nir_elect(b, 1));
      {
         prm_vtx = nir_load_shared(b, 2, 32, zero,
                                   .base = s->layout.lds.workgroup_info_addr + lds_ms_num_prims);
      }
      nir_pop_if(b, if_elected);

      prm_vtx = nir_if_phi(b, prm_vtx, dont_care_2x32);
      num_prm = nir_read_first_invocation(b, nir_channel(b, prm_vtx, 0));
      num_vtx = nir_read_first_invocation(b, nir_channel(b, prm_vtx, 1));

      nir_store_var(b, s->primitive_count_var, num_prm, 0x1);
      nir_store_var(b, s->vertex_count_var, num_vtx, 0x1);
   }
   nir_pop_if(b, if_wave_0);

   *out_num_prm = nir_load_var(b, s->primitive_count_var);
   *out_num_vtx = nir_load_var(b, s->vertex_count_var);
}

static void
ms_emit_attribute_ring_output_stores(nir_builder *b, const uint64_t outputs_mask,
                                     nir_def *idx, lower_ngg_ms_state *s)
{
   if (!outputs_mask)
      return;

   nir_def *ring = nir_load_ring_attr_amd(b);
   nir_def *off = nir_load_ring_attr_offset_amd(b);
   nir_def *zero = nir_imm_int(b, 0);

   u_foreach_bit64 (slot, outputs_mask) {
      if (s->vs_output_param_offset[slot] > AC_EXP_PARAM_OFFSET_31)
         continue;

      nir_def *soffset = nir_iadd_imm(b, off, s->vs_output_param_offset[slot] * 16 * 32);
      nir_def *store_val = nir_undef(b, 4, 32);
      unsigned store_val_components = 0;
      for (unsigned c = 0; c < 4; ++c) {
         if (s->outputs[slot][c]) {
            store_val = nir_vector_insert_imm(b, store_val, s->outputs[slot][c], c);
            store_val_components = c + 1;
         }
      }

      store_val = nir_trim_vector(b, store_val, store_val_components);
      nir_store_buffer_amd(b, store_val, ring, zero, soffset, idx,
                           .memory_modes = nir_var_shader_out,
                           .access = ACCESS_COHERENT | ACCESS_IS_SWIZZLED_AMD);
   }
}

static nir_def *
ms_prim_exp_arg_ch1(nir_builder *b, nir_def *invocation_index, nir_def *num_vtx, lower_ngg_ms_state *s)
{
   /* Primitive connectivity data: describes which vertices the primitive uses. */
   nir_def *prim_idx_addr = nir_imul_imm(b, invocation_index, s->vertices_per_prim);
   nir_def *indices_loaded = NULL;
   nir_def *cull_flag = NULL;

   if (s->layout.var.prm_attr.mask & BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_INDICES)) {
      nir_def *indices[3] = {0};
      for (unsigned c = 0; c < s->vertices_per_prim; ++c)
         indices[c] = nir_load_var(b, s->out_variables[VARYING_SLOT_PRIMITIVE_INDICES * 4 + c]);
      indices_loaded = nir_vec(b, indices, s->vertices_per_prim);
   } else {
      indices_loaded = nir_load_shared(b, s->vertices_per_prim, 8, prim_idx_addr, .base = s->layout.lds.indices_addr);
      indices_loaded = nir_u2u32(b, indices_loaded);
   }

   if (s->uses_cull_flags) {
      nir_def *loaded_cull_flag = NULL;
      if (s->layout.var.prm_attr.mask & BITFIELD64_BIT(VARYING_SLOT_CULL_PRIMITIVE))
         loaded_cull_flag = nir_load_var(b, s->out_variables[VARYING_SLOT_CULL_PRIMITIVE * 4]);
      else
         loaded_cull_flag = nir_u2u32(b, nir_load_shared(b, 1, 8, prim_idx_addr, .base = s->layout.lds.cull_flags_addr));

      cull_flag = nir_i2b(b, loaded_cull_flag);
   }

   nir_def *indices[3];
   nir_def *max_vtx_idx = nir_iadd_imm(b, num_vtx, -1u);

   for (unsigned i = 0; i < s->vertices_per_prim; ++i) {
      indices[i] = nir_channel(b, indices_loaded, i);
      indices[i] = nir_umin(b, indices[i], max_vtx_idx);
   }

   return emit_pack_ngg_prim_exp_arg(b, s->vertices_per_prim, indices, cull_flag);
}

static nir_def *
ms_prim_exp_arg_ch2(nir_builder *b, uint64_t outputs_mask, lower_ngg_ms_state *s)
{
   nir_def *prim_exp_arg_ch2 = NULL;

   if (outputs_mask) {
      /* When layer, viewport etc. are per-primitive, they need to be encoded in
       * the primitive export instruction's second channel. The encoding is:
       *
       * --- GFX10.3 ---
       * bits 31..30: VRS rate Y
       * bits 29..28: VRS rate X
       * bits 23..20: viewport
       * bits 19..17: layer
       *
       * --- GFX11 ---
       * bits 31..28: VRS rate enum
       * bits 23..20: viewport
       * bits 12..00: layer
       */
      prim_exp_arg_ch2 = nir_imm_int(b, 0);

      if (outputs_mask & VARYING_BIT_LAYER) {
         nir_def *layer =
            nir_ishl_imm(b, s->outputs[VARYING_SLOT_LAYER][0], s->gfx_level >= GFX11 ? 0 : 17);
         prim_exp_arg_ch2 = nir_ior(b, prim_exp_arg_ch2, layer);
      }

      if (outputs_mask & VARYING_BIT_VIEWPORT) {
         nir_def *view = nir_ishl_imm(b, s->outputs[VARYING_SLOT_VIEWPORT][0], 20);
         prim_exp_arg_ch2 = nir_ior(b, prim_exp_arg_ch2, view);
      }

      if (outputs_mask & VARYING_BIT_PRIMITIVE_SHADING_RATE) {
         nir_def *rate = s->outputs[VARYING_SLOT_PRIMITIVE_SHADING_RATE][0];
         prim_exp_arg_ch2 = nir_ior(b, prim_exp_arg_ch2, rate);
      }
   }

   return prim_exp_arg_ch2;
}

static void
ms_prim_gen_query(nir_builder *b,
                  nir_def *invocation_index,
                  nir_def *num_prm,
                  lower_ngg_ms_state *s)
{
   if (!s->has_query)
      return;

   nir_if *if_invocation_index_zero = nir_push_if(b, nir_ieq_imm(b, invocation_index, 0));
   {
      nir_if *if_shader_query = nir_push_if(b, nir_load_prim_gen_query_enabled_amd(b));
      {
         nir_atomic_add_gen_prim_count_amd(b, num_prm, .stream_id = 0);
      }
      nir_pop_if(b, if_shader_query);
   }
   nir_pop_if(b, if_invocation_index_zero);
}

static void
ms_invocation_query(nir_builder *b,
                    nir_def *invocation_index,
                    lower_ngg_ms_state *s)
{
   if (!s->has_query)
      return;

   nir_if *if_invocation_index_zero = nir_push_if(b, nir_ieq_imm(b, invocation_index, 0));
   {
      nir_if *if_pipeline_query = nir_push_if(b, nir_load_pipeline_stat_query_enabled_amd(b));
      {
         nir_atomic_add_shader_invocation_count_amd(b, nir_imm_int(b, s->api_workgroup_size));
      }
      nir_pop_if(b, if_pipeline_query);
   }
   nir_pop_if(b, if_invocation_index_zero);
}

static void
emit_ms_vertex(nir_builder *b, nir_def *index, nir_def *row, bool exports, bool parameters,
               uint64_t per_vertex_outputs, lower_ngg_ms_state *s)
{
   ms_emit_arrayed_outputs(b, index, per_vertex_outputs, s);

   if (exports) {
      ac_nir_export_position(b, s->gfx_level, s->clipdist_enable_mask,
                             !s->has_param_exports, false, true,
                             s->per_vertex_outputs | VARYING_BIT_POS, s->outputs, row);
   }

   if (parameters) {
      /* Export generic attributes on GFX10.3
       * (On GFX11 they are already stored in the attribute ring.)
       */
      if (s->has_param_exports && s->gfx_level == GFX10_3) {
         ac_nir_export_parameters(b, s->vs_output_param_offset, per_vertex_outputs, 0, s->outputs,
                                  NULL, NULL);
      }

      /* GFX11+: also store special outputs to the attribute ring so PS can load them. */
      if (s->gfx_level >= GFX11 && (per_vertex_outputs & MS_VERT_ARG_EXP_MASK))
         ms_emit_attribute_ring_output_stores(b, per_vertex_outputs & MS_VERT_ARG_EXP_MASK, index, s);
   }
}

static void
emit_ms_primitive(nir_builder *b, nir_def *index, nir_def *row, bool exports, bool parameters,
                  uint64_t per_primitive_outputs, lower_ngg_ms_state *s)
{
   ms_emit_arrayed_outputs(b, index, per_primitive_outputs, s);

   /* Insert layer output store if the pipeline uses multiview but the API shader doesn't write it. */
   if (s->insert_layer_output)
      s->outputs[VARYING_SLOT_LAYER][0] = nir_load_view_index(b);

   if (exports) {
      const uint64_t outputs_mask = per_primitive_outputs & MS_PRIM_ARG_EXP_MASK;
      nir_def *num_vtx = nir_load_var(b, s->vertex_count_var);
      nir_def *prim_exp_arg_ch1 = ms_prim_exp_arg_ch1(b, index, num_vtx, s);
      nir_def *prim_exp_arg_ch2 = ms_prim_exp_arg_ch2(b, outputs_mask, s);

      nir_def *prim_exp_arg = prim_exp_arg_ch2 ?
         nir_vec2(b, prim_exp_arg_ch1, prim_exp_arg_ch2) : prim_exp_arg_ch1;

      ac_nir_export_primitive(b, prim_exp_arg, row);
   }

   if (parameters) {
      /* Export generic attributes on GFX10.3
       * (On GFX11 they are already stored in the attribute ring.)
       */
      if (s->has_param_exports && s->gfx_level == GFX10_3) {
         ac_nir_export_parameters(b, s->vs_output_param_offset, per_primitive_outputs, 0,
                                  s->outputs, NULL, NULL);
      }

      /* GFX11+: also store special outputs to the attribute ring so PS can load them. */
      if (s->gfx_level >= GFX11)
         ms_emit_attribute_ring_output_stores(b, per_primitive_outputs & MS_PRIM_ARG_EXP_MASK, index, s);
   }
}

static void
emit_ms_outputs(nir_builder *b, nir_def *invocation_index, nir_def *row_start,
                nir_def *count, bool exports, bool parameters, uint64_t mask,
                void (*cb)(nir_builder *, nir_def *, nir_def *, bool, bool,
                           uint64_t, lower_ngg_ms_state *),
                lower_ngg_ms_state *s)
{
   if (cb == &emit_ms_primitive ? s->prim_multirow_export : s->vert_multirow_export) {
      assert(s->hw_workgroup_size % s->wave_size == 0);
      const unsigned num_waves = s->hw_workgroup_size / s->wave_size;

      nir_loop *row_loop = nir_push_loop(b);
      {
         nir_block *preheader = nir_cf_node_as_block(nir_cf_node_prev(&row_loop->cf_node));

         nir_phi_instr *index = nir_phi_instr_create(b->shader);
         nir_phi_instr *row = nir_phi_instr_create(b->shader);
         nir_def_init(&index->instr, &index->def, 1, 32);
         nir_def_init(&row->instr, &row->def, 1, 32);

         nir_phi_instr_add_src(index, preheader, invocation_index);
         nir_phi_instr_add_src(row, preheader, row_start);

         nir_if *if_break = nir_push_if(b, nir_uge(b, &index->def, count));
         {
            nir_jump(b, nir_jump_break);
         }
         nir_pop_if(b, if_break);

         cb(b, &index->def, &row->def, exports, parameters, mask, s);

         nir_block *body = nir_cursor_current_block(b->cursor);
         nir_phi_instr_add_src(index, body,
                               nir_iadd_imm(b, &index->def, s->hw_workgroup_size));
         nir_phi_instr_add_src(row, body,
                               nir_iadd_imm(b, &row->def, num_waves));

         nir_instr_insert_before_cf_list(&row_loop->body, &row->instr);
         nir_instr_insert_before_cf_list(&row_loop->body, &index->instr);
      }
      nir_pop_loop(b, row_loop);
   } else {
      nir_def *has_output = nir_ilt(b, invocation_index, count);
      nir_if *if_has_output = nir_push_if(b, has_output);
      {
         cb(b, invocation_index, row_start, exports, parameters, mask, s);
      }
      nir_pop_if(b, if_has_output);
   }
}

static void
emit_ms_finale(nir_builder *b, lower_ngg_ms_state *s)
{
   /* We assume there is always a single end block in the shader. */
   nir_block *last_block = nir_impl_last_block(b->impl);
   b->cursor = nir_after_block(last_block);

   nir_barrier(b, .execution_scope=SCOPE_WORKGROUP, .memory_scope=SCOPE_WORKGROUP,
                         .memory_semantics=NIR_MEMORY_ACQ_REL, .memory_modes=nir_var_shader_out|nir_var_mem_shared);

   nir_def *num_prm;
   nir_def *num_vtx;

   set_ms_final_output_counts(b, s, &num_prm, &num_vtx);

   nir_def *invocation_index = nir_load_local_invocation_index(b);

   ms_prim_gen_query(b, invocation_index, num_prm, s);

   nir_def *row_start = NULL;
   if (s->fast_launch_2)
      row_start = s->hw_workgroup_size <= s->wave_size ? nir_imm_int(b, 0) : nir_load_subgroup_id(b);

   /* Load vertex/primitive attributes from shared memory and
    * emit store_output intrinsics for them.
    *
    * Contrary to the semantics of the API mesh shader, these are now
    * compliant with NGG HW semantics, meaning that these store the
    * current thread's vertex attributes in a way the HW can export.
    */

   uint64_t per_vertex_outputs =
      s->per_vertex_outputs & ~s->layout.attr_ring.vtx_attr.mask;
   uint64_t per_primitive_outputs =
      s->per_primitive_outputs & ~s->layout.attr_ring.prm_attr.mask & ~SPECIAL_MS_OUT_MASK;

   /* Insert layer output store if the pipeline uses multiview but the API shader doesn't write it. */
   if (s->insert_layer_output) {
      b->shader->info.outputs_written |= VARYING_BIT_LAYER;
      b->shader->info.per_primitive_outputs |= VARYING_BIT_LAYER;
      per_primitive_outputs |= VARYING_BIT_LAYER;
   }

   const bool has_special_param_exports =
      (per_vertex_outputs & MS_VERT_ARG_EXP_MASK) ||
      (per_primitive_outputs & MS_PRIM_ARG_EXP_MASK);

   const bool wait_attr_ring = must_wait_attr_ring(s->gfx_level, has_special_param_exports);

   /* Export vertices. */
   if ((per_vertex_outputs & ~VARYING_BIT_POS) || !wait_attr_ring) {
      emit_ms_outputs(b, invocation_index, row_start, num_vtx, !wait_attr_ring, true,
                      per_vertex_outputs, &emit_ms_vertex, s);
   }

   /* Export primitives. */
   if (per_primitive_outputs || !wait_attr_ring) {
      emit_ms_outputs(b, invocation_index, row_start, num_prm, !wait_attr_ring, true,
                      per_primitive_outputs, &emit_ms_primitive, s);
   }

   /* When we need to wait for attribute ring stores, we emit both position and primitive
    * export instructions after a barrier to make sure both per-vertex and per-primitive
    * attribute ring stores are finished before the GPU starts rasterization.
    */
   if (wait_attr_ring) {
      /* Wait for attribute stores to finish. */
      nir_barrier(b, .execution_scope = SCOPE_SUBGROUP,
                     .memory_scope = SCOPE_DEVICE,
                     .memory_semantics = NIR_MEMORY_RELEASE,
                     .memory_modes = nir_var_shader_out);

      /* Position/primitive export only */
      emit_ms_outputs(b, invocation_index, row_start, num_vtx, true, false,
                      per_vertex_outputs, &emit_ms_vertex, s);
      emit_ms_outputs(b, invocation_index, row_start, num_prm, true, false,
                      per_primitive_outputs, &emit_ms_primitive, s);
   }
}

static void
handle_smaller_ms_api_workgroup(nir_builder *b,
                                lower_ngg_ms_state *s)
{
   if (s->api_workgroup_size >= s->hw_workgroup_size)
      return;

   /* Handle barriers manually when the API workgroup
    * size is less than the HW workgroup size.
    *
    * The problem is that the real workgroup launched on NGG HW
    * will be larger than the size specified by the API, and the
    * extra waves need to keep up with barriers in the API waves.
    *
    * There are 2 different cases:
    * 1. The whole API workgroup fits in a single wave.
    *    We can shrink the barriers to subgroup scope and
    *    don't need to insert any extra ones.
    * 2. The API workgroup occupies multiple waves, but not
    *    all. In this case, we emit code that consumes every
    *    barrier on the extra waves.
    */
   assert(s->hw_workgroup_size % s->wave_size == 0);
   bool scan_barriers = ALIGN(s->api_workgroup_size, s->wave_size) < s->hw_workgroup_size;
   bool can_shrink_barriers = s->api_workgroup_size <= s->wave_size;
   bool need_additional_barriers = scan_barriers && !can_shrink_barriers;

   unsigned api_waves_in_flight_addr = s->layout.lds.workgroup_info_addr + lds_ms_num_api_waves;
   unsigned num_api_waves = DIV_ROUND_UP(s->api_workgroup_size, s->wave_size);

   /* Scan the shader for workgroup barriers. */
   if (scan_barriers) {
      bool has_any_workgroup_barriers = false;

      nir_foreach_block(block, b->impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            bool is_workgroup_barrier =
               intrin->intrinsic == nir_intrinsic_barrier &&
               nir_intrinsic_execution_scope(intrin) == SCOPE_WORKGROUP;

            if (!is_workgroup_barrier)
               continue;

            if (can_shrink_barriers) {
               /* Every API invocation runs in the first wave.
                * In this case, we can change the barriers to subgroup scope
                * and avoid adding additional barriers.
                */
               nir_intrinsic_set_memory_scope(intrin, SCOPE_SUBGROUP);
               nir_intrinsic_set_execution_scope(intrin, SCOPE_SUBGROUP);
            } else {
               has_any_workgroup_barriers = true;
            }
         }
      }

      need_additional_barriers &= has_any_workgroup_barriers;
   }

   /* Extract the full control flow of the shader. */
   nir_cf_list extracted;
   nir_cf_extract(&extracted, nir_before_impl(b->impl),
                  nir_after_cf_list(&b->impl->body));
   b->cursor = nir_before_impl(b->impl);

   /* Wrap the shader in an if to ensure that only the necessary amount of lanes run it. */
   nir_def *invocation_index = nir_load_local_invocation_index(b);
   nir_def *zero = nir_imm_int(b, 0);

   if (need_additional_barriers) {
      /* First invocation stores 0 to number of API waves in flight. */
      nir_if *if_first_in_workgroup = nir_push_if(b, nir_ieq_imm(b, invocation_index, 0));
      {
         nir_store_shared(b, nir_imm_int(b, num_api_waves), zero, .base = api_waves_in_flight_addr);
      }
      nir_pop_if(b, if_first_in_workgroup);

      nir_barrier(b, .execution_scope = SCOPE_WORKGROUP,
                            .memory_scope = SCOPE_WORKGROUP,
                            .memory_semantics = NIR_MEMORY_ACQ_REL,
                            .memory_modes = nir_var_shader_out | nir_var_mem_shared);
   }

   nir_def *has_api_ms_invocation = nir_ult_imm(b, invocation_index, s->api_workgroup_size);
   nir_if *if_has_api_ms_invocation = nir_push_if(b, has_api_ms_invocation);
   {
      nir_cf_reinsert(&extracted, b->cursor);
      b->cursor = nir_after_cf_list(&if_has_api_ms_invocation->then_list);

      if (need_additional_barriers) {
         /* One invocation in each API wave decrements the number of API waves in flight. */
         nir_if *if_elected_again = nir_push_if(b, nir_elect(b, 1));
         {
            nir_shared_atomic(b, 32, zero, nir_imm_int(b, -1u),
                              .base = api_waves_in_flight_addr,
                              .atomic_op = nir_atomic_op_iadd);
         }
         nir_pop_if(b, if_elected_again);

         nir_barrier(b, .execution_scope = SCOPE_WORKGROUP,
                               .memory_scope = SCOPE_WORKGROUP,
                               .memory_semantics = NIR_MEMORY_ACQ_REL,
                               .memory_modes = nir_var_shader_out | nir_var_mem_shared);
      }

      ms_invocation_query(b, invocation_index, s);
   }
   nir_pop_if(b, if_has_api_ms_invocation);

   if (need_additional_barriers) {
      /* Make sure that waves that don't run any API invocations execute
       * the same amount of barriers as those that do.
       *
       * We do this by executing a barrier until the number of API waves
       * in flight becomes zero.
       */
      nir_def *has_api_ms_ballot = nir_ballot(b, 1, s->wave_size, has_api_ms_invocation);
      nir_def *wave_has_no_api_ms = nir_ieq_imm(b, has_api_ms_ballot, 0);
      nir_if *if_wave_has_no_api_ms = nir_push_if(b, wave_has_no_api_ms);
      {
         nir_if *if_elected = nir_push_if(b, nir_elect(b, 1));
         {
            nir_loop *loop = nir_push_loop(b);
            {
               nir_barrier(b, .execution_scope = SCOPE_WORKGROUP,
                                     .memory_scope = SCOPE_WORKGROUP,
                                     .memory_semantics = NIR_MEMORY_ACQ_REL,
                                     .memory_modes = nir_var_shader_out | nir_var_mem_shared);

               nir_def *loaded = nir_load_shared(b, 1, 32, zero, .base = api_waves_in_flight_addr);
               nir_if *if_break = nir_push_if(b, nir_ieq_imm(b, loaded, 0));
               {
                  nir_jump(b, nir_jump_break);
               }
               nir_pop_if(b, if_break);
            }
            nir_pop_loop(b, loop);
         }
         nir_pop_if(b, if_elected);
      }
      nir_pop_if(b, if_wave_has_no_api_ms);
   }
}

static void
ms_move_output(ms_out_part *from, ms_out_part *to)
{
   uint64_t loc = util_logbase2_64(from->mask);
   uint64_t bit = BITFIELD64_BIT(loc);
   from->mask ^= bit;
   to->mask |= bit;
}

static void
ms_calculate_arrayed_output_layout(ms_out_mem_layout *l,
                                   unsigned max_vertices,
                                   unsigned max_primitives)
{
   uint32_t lds_vtx_attr_size = util_bitcount64(l->lds.vtx_attr.mask) * max_vertices * 16;
   uint32_t lds_prm_attr_size = util_bitcount64(l->lds.prm_attr.mask) * max_primitives * 16;
   l->lds.prm_attr.addr = ALIGN(l->lds.vtx_attr.addr + lds_vtx_attr_size, 16);
   l->lds.total_size = l->lds.prm_attr.addr + lds_prm_attr_size;

   uint32_t scratch_ring_vtx_attr_size =
      util_bitcount64(l->scratch_ring.vtx_attr.mask) * max_vertices * 16;
   l->scratch_ring.prm_attr.addr =
      ALIGN(l->scratch_ring.vtx_attr.addr + scratch_ring_vtx_attr_size, 16);
}

static ms_out_mem_layout
ms_calculate_output_layout(enum amd_gfx_level gfx_level, unsigned api_shared_size,
                           uint64_t per_vertex_output_mask, uint64_t per_primitive_output_mask,
                           uint64_t cross_invocation_output_access, unsigned max_vertices,
                           unsigned max_primitives, unsigned vertices_per_prim)
{
   /* These outputs always need export instructions and can't use the attributes ring. */
   const uint64_t always_export_mask =
      VARYING_BIT_POS | VARYING_BIT_CULL_DIST0 | VARYING_BIT_CULL_DIST1 | VARYING_BIT_CLIP_DIST0 |
      VARYING_BIT_CLIP_DIST1 | VARYING_BIT_PSIZ | VARYING_BIT_VIEWPORT |
      VARYING_BIT_PRIMITIVE_SHADING_RATE | VARYING_BIT_LAYER |
      BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_COUNT) |
      BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_INDICES) | BITFIELD64_BIT(VARYING_SLOT_CULL_PRIMITIVE);

   const bool use_attr_ring = gfx_level >= GFX11;
   const uint64_t attr_ring_per_vertex_output_mask =
      use_attr_ring ? per_vertex_output_mask & ~always_export_mask : 0;
   const uint64_t attr_ring_per_primitive_output_mask =
      use_attr_ring ? per_primitive_output_mask & ~always_export_mask : 0;

   const uint64_t lds_per_vertex_output_mask =
      per_vertex_output_mask & ~attr_ring_per_vertex_output_mask & cross_invocation_output_access &
      ~SPECIAL_MS_OUT_MASK;
   const uint64_t lds_per_primitive_output_mask =
      per_primitive_output_mask & ~attr_ring_per_primitive_output_mask &
      cross_invocation_output_access & ~SPECIAL_MS_OUT_MASK;

   const bool cross_invocation_indices =
      cross_invocation_output_access & BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_INDICES);
   const bool cross_invocation_cull_primitive =
      cross_invocation_output_access & BITFIELD64_BIT(VARYING_SLOT_CULL_PRIMITIVE);

   /* Shared memory used by the API shader. */
   ms_out_mem_layout l = { .lds = { .total_size = api_shared_size } };

   /* GFX11+: use attribute ring for all generic attributes. */
   l.attr_ring.vtx_attr.mask = attr_ring_per_vertex_output_mask;
   l.attr_ring.prm_attr.mask = attr_ring_per_primitive_output_mask;

   /* Outputs without cross-invocation access can be stored in variables. */
   l.var.vtx_attr.mask =
      per_vertex_output_mask & ~attr_ring_per_vertex_output_mask & ~cross_invocation_output_access;
   l.var.prm_attr.mask = per_primitive_output_mask & ~attr_ring_per_primitive_output_mask &
                         ~cross_invocation_output_access;

   /* Workgroup information, see ms_workgroup_* for the layout. */
   l.lds.workgroup_info_addr = ALIGN(l.lds.total_size, 16);
   l.lds.total_size = l.lds.workgroup_info_addr + 16;

   /* Per-vertex and per-primitive output attributes.
    * Outputs without cross-invocation access are not included here.
    * First, try to put all outputs into LDS (shared memory).
    * If they don't fit, try to move them to VRAM one by one.
    */
   l.lds.vtx_attr.addr = ALIGN(l.lds.total_size, 16);
   l.lds.vtx_attr.mask = lds_per_vertex_output_mask;
   l.lds.prm_attr.mask = lds_per_primitive_output_mask;
   ms_calculate_arrayed_output_layout(&l, max_vertices, max_primitives);

   /* NGG shaders can only address up to 32K LDS memory.
    * The spec requires us to allow the application to use at least up to 28K
    * shared memory. Additionally, we reserve 2K for driver internal use
    * (eg. primitive indices and such, see below).
    *
    * Move the outputs that do not fit LDS, to VRAM.
    * Start with per-primitive attributes, because those are grouped at the end.
    */
   const unsigned usable_lds_kbytes =
      (cross_invocation_cull_primitive || cross_invocation_indices) ? 30 : 31;
   while (l.lds.total_size >= usable_lds_kbytes * 1024) {
      if (l.lds.prm_attr.mask)
         ms_move_output(&l.lds.prm_attr, &l.scratch_ring.prm_attr);
      else if (l.lds.vtx_attr.mask)
         ms_move_output(&l.lds.vtx_attr, &l.scratch_ring.vtx_attr);
      else
         unreachable("API shader uses too much shared memory.");

      ms_calculate_arrayed_output_layout(&l, max_vertices, max_primitives);
   }

   if (cross_invocation_indices) {
      /* Indices: flat array of 8-bit vertex indices for each primitive. */
      l.lds.indices_addr = ALIGN(l.lds.total_size, 16);
      l.lds.total_size = l.lds.indices_addr + max_primitives * vertices_per_prim;
   }

   if (cross_invocation_cull_primitive) {
      /* Cull flags: array of 8-bit cull flags for each primitive, 1=cull, 0=keep. */
      l.lds.cull_flags_addr = ALIGN(l.lds.total_size, 16);
      l.lds.total_size = l.lds.cull_flags_addr + max_primitives;
   }

   /* NGG is only allowed to address up to 32K of LDS. */
   assert(l.lds.total_size <= 32 * 1024);
   return l;
}

void
ac_nir_lower_ngg_ms(nir_shader *shader,
                    enum amd_gfx_level gfx_level,
                    uint32_t clipdist_enable_mask,
                    const uint8_t *vs_output_param_offset,
                    bool has_param_exports,
                    bool *out_needs_scratch_ring,
                    unsigned wave_size,
                    unsigned hw_workgroup_size,
                    bool multiview,
                    bool has_query,
                    bool fast_launch_2)
{
   unsigned vertices_per_prim =
      mesa_vertices_per_prim(shader->info.mesh.primitive_type);

   uint64_t per_vertex_outputs =
      shader->info.outputs_written & ~shader->info.per_primitive_outputs & ~SPECIAL_MS_OUT_MASK;
   uint64_t per_primitive_outputs =
      shader->info.per_primitive_outputs & shader->info.outputs_written;

   /* Whether the shader uses CullPrimitiveEXT */
   bool uses_cull = shader->info.outputs_written & BITFIELD64_BIT(VARYING_SLOT_CULL_PRIMITIVE);
   /* Can't handle indirect register addressing, pretend as if they were cross-invocation. */
   uint64_t cross_invocation_access = shader->info.mesh.ms_cross_invocation_output_access |
                                      shader->info.outputs_accessed_indirectly;

   unsigned max_vertices = shader->info.mesh.max_vertices_out;
   unsigned max_primitives = shader->info.mesh.max_primitives_out;

   ms_out_mem_layout layout = ms_calculate_output_layout(
      gfx_level, shader->info.shared_size, per_vertex_outputs, per_primitive_outputs,
      cross_invocation_access, max_vertices, max_primitives, vertices_per_prim);

   shader->info.shared_size = layout.lds.total_size;
   *out_needs_scratch_ring = layout.scratch_ring.vtx_attr.mask || layout.scratch_ring.prm_attr.mask;

   /* The workgroup size that is specified by the API shader may be different
    * from the size of the workgroup that actually runs on the HW, due to the
    * limitations of NGG: max 0/1 vertex and 0/1 primitive per lane is allowed.
    *
    * Therefore, we must make sure that when the API workgroup size is smaller,
    * we don't run the API shader on more HW invocations than is necessary.
    */
   unsigned api_workgroup_size = shader->info.workgroup_size[0] *
                                 shader->info.workgroup_size[1] *
                                 shader->info.workgroup_size[2];

   lower_ngg_ms_state state = {
      .layout = layout,
      .wave_size = wave_size,
      .per_vertex_outputs = per_vertex_outputs,
      .per_primitive_outputs = per_primitive_outputs,
      .vertices_per_prim = vertices_per_prim,
      .api_workgroup_size = api_workgroup_size,
      .hw_workgroup_size = hw_workgroup_size,
      .insert_layer_output = multiview && !(shader->info.outputs_written & VARYING_BIT_LAYER),
      .uses_cull_flags = uses_cull,
      .gfx_level = gfx_level,
      .fast_launch_2 = fast_launch_2,
      .vert_multirow_export = fast_launch_2 && max_vertices > hw_workgroup_size,
      .prim_multirow_export = fast_launch_2 && max_primitives > hw_workgroup_size,
      .clipdist_enable_mask = clipdist_enable_mask,
      .vs_output_param_offset = vs_output_param_offset,
      .has_param_exports = has_param_exports,
      .has_query = has_query,
   };

   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   assert(impl);

   state.vertex_count_var =
      nir_local_variable_create(impl, glsl_uint_type(), "vertex_count_var");
   state.primitive_count_var =
      nir_local_variable_create(impl, glsl_uint_type(), "primitive_count_var");

   nir_builder builder = nir_builder_at(nir_before_impl(impl));
   nir_builder *b = &builder; /* This is to avoid the & */

   handle_smaller_ms_api_workgroup(b, &state);
   if (!fast_launch_2)
      ms_emit_legacy_workgroup_index(b, &state);
   ms_create_same_invocation_vars(b, &state);
   nir_metadata_preserve(impl, nir_metadata_none);

   lower_ms_intrinsics(shader, &state);

   emit_ms_finale(b, &state);
   nir_metadata_preserve(impl, nir_metadata_none);

   /* Cleanup */
   nir_lower_vars_to_ssa(shader);
   nir_remove_dead_variables(shader, nir_var_function_temp, NULL);
   nir_lower_alu_to_scalar(shader, NULL, NULL);
   nir_lower_phis_to_scalar(shader, true);

   /* Optimize load_local_invocation_index. When the API workgroup is smaller than the HW workgroup,
    * local_invocation_id isn't initialized for all lanes and we can't perform this optimization for
    * all load_local_invocation_index.
    */
   if (fast_launch_2 && api_workgroup_size == hw_workgroup_size &&
       ((shader->info.workgroup_size[0] == 1) + (shader->info.workgroup_size[1] == 1) +
        (shader->info.workgroup_size[2] == 1)) == 2) {
      nir_lower_compute_system_values_options csv_options = {
         .lower_local_invocation_index = true,
      };
      nir_lower_compute_system_values(shader, &csv_options);
   }

   nir_validate_shader(shader, "after emitting NGG MS");
}
