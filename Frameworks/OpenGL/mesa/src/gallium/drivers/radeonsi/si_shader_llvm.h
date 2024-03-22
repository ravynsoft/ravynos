/*
 * Copyright 2016 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SI_SHADER_LLVM_H
#define SI_SHADER_LLVM_H

#include "ac_shader_abi.h"
#include "ac_llvm_build.h"
#include "si_shader.h"

struct si_shader_args;

struct si_shader_context {
   struct ac_llvm_context ac;
   struct si_shader *shader;
   struct si_screen *screen;

   gl_shader_stage stage;

   /* For clamping the non-constant index in resource indexing: */
   unsigned num_const_buffers;
   unsigned num_shader_buffers;
   unsigned num_images;
   unsigned num_samplers;

   struct si_shader_args *args;
   struct ac_shader_abi abi;

   LLVMBasicBlockRef merged_wrap_if_entry_block;
   int merged_wrap_if_label;

   struct ac_llvm_pointer main_fn;
   LLVMTypeRef return_type;

   struct ac_llvm_compiler *compiler;

   /* Preloaded descriptors. */
   LLVMValueRef instance_divisor_constbuf;

   LLVMValueRef gs_ngg_emit;
   struct ac_llvm_pointer gs_ngg_scratch;
   LLVMValueRef return_value;
};

static inline struct si_shader_context *si_shader_context_from_abi(struct ac_shader_abi *abi)
{
   return container_of(abi, struct si_shader_context, abi);
}

/* si_shader_llvm.c */
bool si_compile_llvm(struct si_screen *sscreen, struct si_shader_binary *binary,
                     struct ac_shader_config *conf, struct ac_llvm_compiler *compiler,
                     struct ac_llvm_context *ac, struct util_debug_callback *debug,
                     gl_shader_stage stage, const char *name, bool less_optimized);
void si_llvm_context_init(struct si_shader_context *ctx, struct si_screen *sscreen,
                          struct ac_llvm_compiler *compiler, unsigned wave_size,
                          bool exports_color_null, bool exports_mrtz,
                          enum ac_float_mode float_mode);
void si_llvm_create_func(struct si_shader_context *ctx, const char *name, LLVMTypeRef *return_types,
                         unsigned num_return_elems, unsigned max_workgroup_size);
void si_llvm_create_main_func(struct si_shader_context *ctx);
void si_llvm_optimize_module(struct si_shader_context *ctx);
void si_llvm_dispose(struct si_shader_context *ctx);
LLVMValueRef si_buffer_load_const(struct si_shader_context *ctx, LLVMValueRef resource,
                                  LLVMValueRef offset);
void si_llvm_build_ret(struct si_shader_context *ctx, LLVMValueRef ret);
LLVMValueRef si_insert_input_ret(struct si_shader_context *ctx, LLVMValueRef ret,
                                 struct ac_arg param, unsigned return_index);
LLVMValueRef si_insert_input_ret_float(struct si_shader_context *ctx, LLVMValueRef ret,
                                       struct ac_arg param, unsigned return_index);
LLVMValueRef si_insert_input_ptr(struct si_shader_context *ctx, LLVMValueRef ret,
                                 struct ac_arg param, unsigned return_index);
LLVMValueRef si_prolog_get_internal_binding_slot(struct si_shader_context *ctx, unsigned slot);
LLVMValueRef si_unpack_param(struct si_shader_context *ctx, struct ac_arg param, unsigned rshift,
                             unsigned bitwidth);

/* si_shader_llvm_gs.c */
LLVMValueRef si_is_es_thread(struct si_shader_context *ctx);
LLVMValueRef si_is_gs_thread(struct si_shader_context *ctx);
void si_llvm_es_build_end(struct si_shader_context *ctx);
void si_llvm_gs_build_end(struct si_shader_context *ctx);

/* si_shader_llvm_tess.c */
LLVMValueRef si_get_rel_patch_id(struct si_shader_context *ctx);
void si_llvm_ls_build_end(struct si_shader_context *ctx);
void si_llvm_build_tcs_epilog(struct si_shader_context *ctx, union si_shader_part_key *key);
void si_llvm_tcs_build_end(struct si_shader_context *ctx);
void si_llvm_init_tcs_callbacks(struct si_shader_context *ctx);

/* si_shader_llvm_ps.c */
void si_llvm_build_ps_prolog(struct si_shader_context *ctx, union si_shader_part_key *key);
void si_llvm_build_ps_epilog(struct si_shader_context *ctx, union si_shader_part_key *key);
void si_llvm_ps_build_end(struct si_shader_context *ctx);

/* si_shader_llvm_vs.c */
void si_llvm_build_vs_prolog(struct si_shader_context *ctx, union si_shader_part_key *key);

#endif
