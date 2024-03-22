/**************************************************************************
 *
 * Copyright 2011-2012 Advanced Micro Devices, Inc.
 * Copyright 2009 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * @file
 * TGSI to LLVM IR translation.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 * @author Tom Stellard <thomas.stellard@amd.com>
 */

#ifndef LP_BLD_TGSI_H
#define LP_BLD_TGSI_H

#include "gallivm/lp_bld.h"
#include "gallivm/lp_bld_tgsi_action.h"
#include "gallivm/lp_bld_limits.h"
#include "gallivm/lp_bld_sample.h"
#include "gallivm/lp_bld_ir_common.h"
#include "lp_bld_type.h"
#include "util/compiler.h"
#include "pipe/p_state.h"
#include "tgsi/tgsi_exec.h"
#include "tgsi/tgsi_scan.h"
#include "tgsi/tgsi_info.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LP_CHAN_ALL ~0u

struct tgsi_full_declaration;
struct tgsi_full_immediate;
struct tgsi_full_instruction;
struct tgsi_full_src_register;
struct tgsi_full_dst_register;
struct tgsi_opcode_info;
struct tgsi_token;
struct tgsi_shader_info;
struct lp_build_mask_context;
struct gallivm_state;
struct lp_derivatives;
struct lp_build_gs_iface;

enum lp_build_tex_modifier {
   LP_BLD_TEX_MODIFIER_NONE = 0,
   LP_BLD_TEX_MODIFIER_PROJECTED,
   LP_BLD_TEX_MODIFIER_LOD_BIAS,
   LP_BLD_TEX_MODIFIER_EXPLICIT_LOD,
   LP_BLD_TEX_MODIFIER_EXPLICIT_DERIV,
   LP_BLD_TEX_MODIFIER_LOD_ZERO
};


/**
 * Describe a channel of a register.
 *
 * The value can be a:
 * - immediate value (i.e. derived from a IMM register)
 * - CONST[n].x/y/z/w
 * - IN[n].x/y/z/w
 * - undetermined (when .file == TGSI_FILE_NULL)
 *
 * This is one of the analysis results, and is used to described
 * the output color in terms of inputs.
 */
struct lp_tgsi_channel_info
{
   unsigned file:4; /* TGSI_FILE_* */
   unsigned swizzle:3; /* PIPE_SWIZZLE_x */
   union {
      uint32_t index;
      float value; /* for TGSI_FILE_IMMEDIATE */
   } u;
};


/**
 * Describe a texture sampler interpolator.
 *
 * The interpolation is described in terms of regular inputs.
 */
struct lp_tgsi_texture_info
{
   struct lp_tgsi_channel_info coord[4];
   unsigned target:8; /* TGSI_TEXTURE_* */
   unsigned sampler_unit:8;  /* Sampler unit */
   unsigned texture_unit:8;  /* Texture unit */
   unsigned modifier:8; /* LP_BLD_TEX_MODIFIER_* */
};


struct lp_tgsi_info
{
   struct tgsi_shader_info base;

   /*
    * Whether any of the texture opcodes access a register file other than
    * TGSI_FILE_INPUT.
    *
    * We could also handle TGSI_FILE_CONST/IMMEDIATE here, but there is little
    * benefit.
    */
   unsigned indirect_textures:1;

   /*
    * Whether any of the texture (sample) ocpodes use different sampler
    * and sampler view unit.
    */
   unsigned sampler_texture_units_different:1;

   /*
    * Whether any immediate values are outside the range of 0 and 1
    */
   unsigned unclamped_immediates:1;

   /*
    * Texture opcode description. Aimed at detecting and described direct
    * texture opcodes.
    */
   unsigned num_texs;
   struct lp_tgsi_texture_info tex[PIPE_MAX_SAMPLERS];

   /*
    * Output description. Aimed at detecting and describing simple blit
    * shaders.
    */
   struct lp_tgsi_channel_info output[PIPE_MAX_SHADER_OUTPUTS][4];

   /*
    * Shortcut pointers into the above (for fragment shaders).
    */
   const struct lp_tgsi_channel_info *cbuf[PIPE_MAX_COLOR_BUFS];
};

/**
 * Reference to system values.
 */
struct lp_bld_tgsi_system_values {
   LLVMValueRef instance_id;
   LLVMValueRef base_instance;
   LLVMValueRef vertex_id;
   LLVMValueRef vertex_id_nobase;
   LLVMValueRef prim_id;
   LLVMValueRef basevertex;
   LLVMValueRef firstvertex;
   LLVMValueRef invocation_id;
   LLVMValueRef draw_id;
   LLVMValueRef thread_id[3];
   LLVMValueRef block_id[3];
   LLVMValueRef grid_size[3];
   LLVMValueRef front_facing;
   LLVMValueRef work_dim;
   LLVMValueRef block_size[3];
   LLVMValueRef tess_coord;
   LLVMValueRef tess_outer;
   LLVMValueRef tess_inner;
   LLVMValueRef vertices_in;
   LLVMValueRef sample_id;
   LLVMTypeRef sample_pos_type;
   LLVMValueRef sample_pos;
   LLVMValueRef sample_mask_in;
   LLVMValueRef view_index;
   LLVMValueRef subgroup_id;
   LLVMValueRef num_subgroups;
};


/**
 * Sampler code generation interface.
 *
 * Although texture sampling is a requirement for TGSI translation, it is
 * a very different problem with several different approaches to it. This
 * structure establishes an interface for texture sampling code generation, so
 * that we can easily use different texture sampling strategies.
 */
struct lp_build_sampler_soa
{
   void
   (*emit_tex_sample)(const struct lp_build_sampler_soa *sampler,
                      struct gallivm_state *gallivm,
                      const struct lp_sampler_params *params);

   void
   (*emit_size_query)(const struct lp_build_sampler_soa *sampler,
                      struct gallivm_state *gallivm,
                      const struct lp_sampler_size_query_params *params);
};


struct lp_build_sampler_aos
{
   LLVMValueRef
   (*emit_fetch_texel)(const struct lp_build_sampler_aos *sampler,
                       struct lp_build_context *bld,
                       enum tgsi_texture_type target,
                       unsigned unit,
                       LLVMValueRef coords,
                       const struct lp_derivatives derivs,
                       enum lp_build_tex_modifier modifier);
};

struct lp_img_params;

struct lp_build_image_soa
{
   void
   (*emit_op)(const struct lp_build_image_soa *image,
              struct gallivm_state *gallivm,
              const struct lp_img_params *params);

   void
   (*emit_size_query)(const struct lp_build_image_soa *sampler,
                      struct gallivm_state *gallivm,
                      const struct lp_sampler_size_query_params *params);
};

struct lp_build_fs_iface;
struct lp_build_fs_iface {
   LLVMValueRef (*interp_fn)(const struct lp_build_fs_iface *iface,
                             struct lp_build_context *bld,
                             unsigned attrib, unsigned chan,
                             bool centroid, bool sample,
                             LLVMValueRef indir_index, LLVMValueRef offsets[2]);

   void (*fb_fetch)(const struct lp_build_fs_iface *iface,
                    struct lp_build_context *bld,
                    int location,
                    LLVMValueRef result[4]);
};

void
lp_build_tgsi_info(const struct tgsi_token *tokens,
                   struct lp_tgsi_info *info);


struct lp_build_tgsi_params {
   struct lp_type type;
   struct lp_build_mask_context *mask;
   LLVMValueRef consts_ptr;
   LLVMValueRef const_sizes_ptr;
   const struct lp_bld_tgsi_system_values *system_values;
   const LLVMValueRef (*inputs)[4];
   int num_inputs;
   LLVMTypeRef context_type;
   LLVMValueRef context_ptr;
   LLVMTypeRef resources_type;
   LLVMValueRef resources_ptr;
   LLVMTypeRef thread_data_type;
   LLVMValueRef thread_data_ptr;
   const struct lp_build_sampler_soa *sampler;
   const struct tgsi_shader_info *info;
   const struct lp_build_gs_iface *gs_iface;
   const struct lp_build_tcs_iface *tcs_iface;
   const struct lp_build_tes_iface *tes_iface;
   const struct lp_build_mesh_iface *mesh_iface;
   LLVMValueRef ssbo_ptr;
   LLVMValueRef ssbo_sizes_ptr;
   const struct lp_build_image_soa *image;
   LLVMValueRef shared_ptr;
   LLVMValueRef payload_ptr;
   const struct lp_build_coro_suspend_info *coro;
   LLVMValueRef kernel_args;
   const struct lp_build_fs_iface *fs_iface;
   unsigned gs_vertex_streams;
   LLVMValueRef aniso_filter_table;
   LLVMValueRef current_func;
   struct hash_table *fns;
   LLVMValueRef scratch_ptr;
   LLVMValueRef call_context_ptr;
};

void
lp_build_tgsi_soa(struct gallivm_state *gallivm,
                  const struct tgsi_token *tokens,
                  const struct lp_build_tgsi_params *params,
                  LLVMValueRef (*outputs)[4]);

struct lp_build_tgsi_inst_list
{
   struct tgsi_full_instruction *instructions;
   unsigned max_instructions;
   unsigned num_instructions;
};

unsigned lp_bld_tgsi_list_init(struct lp_build_tgsi_context * bld_base);


unsigned lp_bld_tgsi_add_instruction(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_full_instruction *inst_to_add);


struct lp_build_tgsi_context;


typedef LLVMValueRef (*lp_build_emit_fetch_fn)(struct lp_build_tgsi_context *,
                                        const struct tgsi_full_src_register *,
                                        enum tgsi_opcode_type,
                                        unsigned);

typedef void (*lp_build_emit_store_reg_fn)(struct lp_build_tgsi_context *,
                              enum tgsi_opcode_type,
                              const struct tgsi_full_dst_register *,
                              unsigned,
                              unsigned,
                              LLVMValueRef,
                              LLVMValueRef);

struct lp_build_tgsi_context
{
   struct lp_build_context base;

   struct lp_build_context uint_bld;
   struct lp_build_context int_bld;

   struct lp_build_context dbl_bld;

   struct lp_build_context uint64_bld;
   struct lp_build_context int64_bld;

   /** This array stores functions that are used to transform TGSI opcodes to
     * LLVM instructions.
     */
   struct lp_build_tgsi_action op_actions[TGSI_OPCODE_LAST];

   /* TGSI_OPCODE_RSQ is defined as 1 / sqrt( abs(src0.x) ), rsq_action
    * should compute 1 / sqrt (src0.x) */
   struct lp_build_tgsi_action rsq_action;

   struct lp_build_tgsi_action sqrt_action;

   struct lp_build_tgsi_action drsq_action;

   struct lp_build_tgsi_action dsqrt_action;
   const struct tgsi_shader_info *info;

   lp_build_emit_fetch_fn emit_fetch_funcs[TGSI_FILE_COUNT];
   lp_build_emit_store_reg_fn emit_store_reg_funcs[TGSI_FILE_COUNT];

   LLVMValueRef (*emit_swizzle)(struct lp_build_tgsi_context *,
                         LLVMValueRef, unsigned, unsigned, unsigned, unsigned);


   void (*emit_debug)(struct lp_build_tgsi_context *,
                      const struct tgsi_full_instruction *,
                      const struct tgsi_opcode_info *);

   void (*emit_store)(struct lp_build_tgsi_context *,
                      const struct tgsi_full_instruction *,
                      const struct tgsi_opcode_info *,
                      unsigned index,
                      LLVMValueRef dst[4]);

   void (*emit_declaration)(struct lp_build_tgsi_context *,
                             const struct tgsi_full_declaration *decl);

   void (*emit_immediate)(struct lp_build_tgsi_context *,
                          const struct tgsi_full_immediate *imm);


   /* Allow the user to store data in this structure rather than passing it
    * to every function. */
   void * userdata;

   bool soa;

   int pc;

   struct tgsi_full_instruction *instructions;
   unsigned max_instructions;
   unsigned num_instructions;

   /** This function allows the user to insert some instructions at the
     * beginning of the program.  It is optional and does not need to be
     * implemented.
     */
   void (*emit_prologue)(struct lp_build_tgsi_context*);

   /** This function allows the user to insert some instructions after
     * declarations section, but before any other code.
     * It is optional and does not need to be implemented.
     */
   void (*emit_prologue_post_decl)(struct lp_build_tgsi_context*);

   /** This function allows the user to insert some instructions at the end of
     * the program.  This callback is intended to be used for emitting
     * instructions to handle the export for the output registers, but it can
     * be used for any purpose.  Implementing this function is optiona, but
     * recommended.
     */
   void (*emit_epilogue)(struct lp_build_tgsi_context*);
};

struct lp_build_gs_iface
{
   LLVMValueRef (*fetch_input)(const struct lp_build_gs_iface *gs_iface,
                               struct lp_build_context * bld,
                               bool is_vindex_indirect,
                               LLVMValueRef vertex_index,
                               bool is_aindex_indirect,
                               LLVMValueRef attrib_index,
                               LLVMValueRef swizzle_index);
   void (*emit_vertex)(const struct lp_build_gs_iface *gs_iface,
                       struct lp_build_context * bld,
                       LLVMValueRef (*outputs)[4],
                       LLVMValueRef emitted_vertices_vec,
                       LLVMValueRef mask_vec, LLVMValueRef stream_id);
   void (*end_primitive)(const struct lp_build_gs_iface *gs_iface,
                         struct lp_build_context * bld,
                         LLVMValueRef total_emitted_vertices_vec,
                         LLVMValueRef verts_per_prim_vec,
                         LLVMValueRef emitted_prims_vec,
                         LLVMValueRef mask_vec, unsigned stream);
   void (*gs_epilogue)(const struct lp_build_gs_iface *gs_iface,
                       LLVMValueRef total_emitted_vertices_vec,
                       LLVMValueRef emitted_prims_vec, unsigned stream);
};

struct lp_build_tcs_iface
{
   void (*emit_prologue)(struct lp_build_context * bld);
   void (*emit_epilogue)(struct lp_build_context * bld);
   void (*emit_barrier)(struct lp_build_context *bld_base);

   void (*emit_store_output)(const struct lp_build_tcs_iface *tcs_iface,
                             struct lp_build_context * bld,
                             unsigned name,
                             bool is_vindex_indirect,
                             LLVMValueRef vertex_index,
                             bool is_aindex_indirect,
                             LLVMValueRef attrib_index,
                             bool is_sindex_indirect,
                             LLVMValueRef swizzle_index,
                             LLVMValueRef value,
                             LLVMValueRef mask_vec);

   LLVMValueRef (*emit_fetch_input)(const struct lp_build_tcs_iface *tcs_iface,
                                    struct lp_build_context * bld,
                                    bool is_vindex_indirect,
                                    LLVMValueRef vertex_index,
                                    bool is_aindex_indirect,
                                    LLVMValueRef attrib_index,
                                    bool is_sindex_indirect,
                                    LLVMValueRef swizzle_index);

   LLVMValueRef (*emit_fetch_output)(const struct lp_build_tcs_iface *tcs_iface,
                                    struct lp_build_context * bld,
                                    bool is_vindex_indirect,
                                    LLVMValueRef vertex_index,
                                    bool is_aindex_indirect,
                                    LLVMValueRef attrib_index,
                                    bool is_sindex_indirect,
                                    LLVMValueRef swizzle_index,
                                    uint32_t name);
};

struct lp_build_tes_iface
{
   LLVMValueRef (*fetch_vertex_input)(const struct lp_build_tes_iface *tes_iface,
                                      struct lp_build_context * bld,
                                      bool is_vindex_indirect,
                                      LLVMValueRef vertex_index,
                                      bool is_aindex_indirect,
                                      LLVMValueRef attrib_index,
                                      bool is_sindex_indirect,
                                      LLVMValueRef swizzle_index);

   LLVMValueRef (*fetch_patch_input)(const struct lp_build_tes_iface *tes_iface,
                                     struct lp_build_context * bld,
                                     bool is_aindex_indirect,
                                     LLVMValueRef attrib_index,
                                     LLVMValueRef swizzle_index);
};

struct lp_build_mesh_iface
{
   void (*emit_store_output)(const struct lp_build_mesh_iface *mesh_iface,
                             struct lp_build_context * bld,
                             unsigned name,
                             bool is_vindex_indirect,
                             LLVMValueRef vertex_index,
                             bool is_aindex_indirect,
                             LLVMValueRef attrib_index,
                             bool is_sindex_indirect,
                             LLVMValueRef swizzle_index,
                             LLVMValueRef value,
                             LLVMValueRef mask_vec);
   void (*emit_vertex_and_primitive_count)(const struct lp_build_mesh_iface *mesh_iface,
                                           struct lp_build_context *bld,
                                           LLVMValueRef vertices_count,
                                           LLVMValueRef primitives_count);
};

struct lp_build_tgsi_soa_context
{
   struct lp_build_tgsi_context bld_base;

   /* Builder for scalar elements of shader's data type (float) */
   struct lp_build_context elem_bld;

   const struct lp_build_gs_iface *gs_iface;
   const struct lp_build_tcs_iface *tcs_iface;
   const struct lp_build_tes_iface *tes_iface;

   LLVMValueRef emitted_prims_vec_ptr;
   LLVMValueRef total_emitted_vertices_vec_ptr;
   LLVMValueRef emitted_vertices_vec_ptr;
   LLVMValueRef max_output_vertices_vec;

   LLVMValueRef consts_ptr;
   LLVMValueRef consts[LP_MAX_TGSI_CONST_BUFFERS];
   LLVMValueRef consts_sizes[LP_MAX_TGSI_CONST_BUFFERS];
   const LLVMValueRef (*inputs)[TGSI_NUM_CHANNELS];
   LLVMValueRef (*outputs)[TGSI_NUM_CHANNELS];
   LLVMTypeRef context_type;
   LLVMValueRef context_ptr;
   LLVMTypeRef resources_type;
   LLVMValueRef resources_ptr;
   LLVMTypeRef thread_data_type;
   LLVMValueRef thread_data_ptr;

   LLVMValueRef ssbo_ptr;
   LLVMValueRef ssbos[LP_MAX_TGSI_SHADER_BUFFERS];
   LLVMValueRef ssbo_sizes[LP_MAX_TGSI_SHADER_BUFFERS];

   LLVMValueRef shared_ptr;

   const struct lp_build_coro_suspend_info *coro;

   const struct lp_build_sampler_soa *sampler;
   const struct lp_build_image_soa *image;

   struct tgsi_declaration_sampler_view sv[PIPE_MAX_SHADER_SAMPLER_VIEWS];

   LLVMValueRef immediates[LP_MAX_INLINED_IMMEDIATES][TGSI_NUM_CHANNELS];
   LLVMValueRef temps[LP_MAX_INLINED_TEMPS][TGSI_NUM_CHANNELS];
   LLVMValueRef addr[LP_MAX_TGSI_ADDRS][TGSI_NUM_CHANNELS];

   /* We allocate/use this array of temps if (1 << TGSI_FILE_TEMPORARY) is
    * set in the indirect_files field.
    * The temps[] array above is unused then.
    */
   LLVMTypeRef temps_array_type;
   LLVMValueRef temps_array;

   /* We allocate/use this array of output if (1 << TGSI_FILE_OUTPUT) is
    * set in the indirect_files field.
    * The outputs[] array above is unused then.
    */
   LLVMTypeRef outputs_array_type;
   LLVMValueRef outputs_array;

   /* We allocate/use this array of inputs if (1 << TGSI_FILE_INPUT) is
    * set in the indirect_files field.
    * The inputs[] array above is unused then.
    */
   LLVMValueRef inputs_array;

   /* We allocate/use this array of temps if (1 << TGSI_FILE_IMMEDIATE) is
    * set in the indirect_files field.
    */
   LLVMValueRef imms_array;


   struct lp_bld_tgsi_system_values system_values;

   /** bitmask indicating which register files are accessed indirectly */
   unsigned indirect_files;

   struct lp_build_mask_context *mask;
   struct lp_exec_mask exec_mask;

   unsigned num_immediates;
   bool use_immediates_array;
};

void
lp_emit_declaration_soa(
   struct lp_build_tgsi_context *bld,
   const struct tgsi_full_declaration *decl);

void lp_emit_immediate_soa(
   struct lp_build_tgsi_context *bld_base,
   const struct tgsi_full_immediate *imm);

bool
lp_emit_instruction_soa(
   struct lp_build_tgsi_soa_context *bld,
   const struct tgsi_full_instruction *inst,
   const struct tgsi_opcode_info *info);


LLVMValueRef
lp_get_temp_ptr_soa(
   struct lp_build_tgsi_soa_context *bld,
   unsigned index,
   unsigned chan);

LLVMValueRef
lp_get_output_ptr(
   struct lp_build_tgsi_soa_context *bld,
   unsigned index,
   unsigned chan);

static inline struct lp_build_tgsi_soa_context *
lp_soa_context(struct lp_build_tgsi_context *bld_base)
{
   return (struct lp_build_tgsi_soa_context *)bld_base;
}

void lp_build_fetch_args(
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data);

void
lp_build_tgsi_intrinsic(
 const struct lp_build_tgsi_action * action,
 struct lp_build_tgsi_context * bld_base,
 struct lp_build_emit_data * emit_data);

LLVMValueRef
lp_build_emit_llvm(
   struct lp_build_tgsi_context *bld_base,
   unsigned tgsi_opcode,
   struct lp_build_emit_data * emit_data);

LLVMValueRef
lp_build_emit_llvm_unary(
   struct lp_build_tgsi_context *bld_base,
   unsigned tgsi_opcode,
   LLVMValueRef arg0);

LLVMValueRef
lp_build_emit_llvm_binary(
   struct lp_build_tgsi_context *bld_base,
   unsigned tgsi_opcode,
   LLVMValueRef arg0,
   LLVMValueRef arg1);

LLVMValueRef
lp_build_emit_llvm_ternary(
   struct lp_build_tgsi_context *bld_base,
   unsigned tgsi_opcode,
   LLVMValueRef arg0,
   LLVMValueRef arg1,
   LLVMValueRef arg2);

bool
lp_build_tgsi_inst_llvm(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_full_instruction *inst);

LLVMValueRef
lp_build_emit_fetch_src(
   struct lp_build_tgsi_context *bld_base,
   const struct tgsi_full_src_register *reg,
   enum tgsi_opcode_type stype,
   const unsigned chan_index);

LLVMValueRef
lp_build_emit_fetch(
   struct lp_build_tgsi_context *bld_base,
   const struct tgsi_full_instruction *inst,
   unsigned src_op,
   const unsigned chan_index);


LLVMValueRef
lp_build_emit_fetch_texoffset(
   struct lp_build_tgsi_context *bld_base,
   const struct tgsi_full_instruction *inst,
   unsigned tex_off_op,
   const unsigned chan_index);

bool
lp_build_tgsi_llvm(
   struct lp_build_tgsi_context * bld_base,
   const struct tgsi_token *tokens);

#ifdef __cplusplus
}
#endif

#endif /* LP_BLD_TGSI_H */
