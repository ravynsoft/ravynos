/*
 * Copyright 2020 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_nir.h"
#include "si_pipe.h"
#include "si_shader_internal.h"
#include "si_shader_llvm.h"
#include "si_query.h"
#include "sid.h"
#include "util/u_memory.h"

LLVMValueRef si_is_es_thread(struct si_shader_context *ctx)
{
   /* Return true if the current thread should execute an ES thread. */
   return LLVMBuildICmp(ctx->ac.builder, LLVMIntULT, ac_get_thread_id(&ctx->ac),
                        si_unpack_param(ctx, ctx->args->ac.merged_wave_info, 0, 8), "");
}

LLVMValueRef si_is_gs_thread(struct si_shader_context *ctx)
{
   /* Return true if the current thread should execute a GS thread. */
   return LLVMBuildICmp(ctx->ac.builder, LLVMIntULT, ac_get_thread_id(&ctx->ac),
                        si_unpack_param(ctx, ctx->args->ac.merged_wave_info, 8, 8), "");
}

void si_llvm_es_build_end(struct si_shader_context *ctx)
{
   if (ctx->screen->info.gfx_level < GFX9 || ctx->shader->is_monolithic)
      return;

   ac_build_endif(&ctx->ac, ctx->merged_wrap_if_label);

   LLVMValueRef ret = ctx->return_value;

   ret = si_insert_input_ptr(ctx, ret, ctx->args->other_const_and_shader_buffers, 0);
   ret = si_insert_input_ptr(ctx, ret, ctx->args->other_samplers_and_images, 1);
   if (ctx->shader->key.ge.as_ngg)
      ret = si_insert_input_ptr(ctx, ret, ctx->args->ac.gs_tg_info, 2);
   else
      ret = si_insert_input_ret(ctx, ret, ctx->args->ac.gs2vs_offset, 2);
   ret = si_insert_input_ret(ctx, ret, ctx->args->ac.merged_wave_info, 3);
   if (ctx->screen->info.gfx_level >= GFX11)
      ret = si_insert_input_ret(ctx, ret, ctx->args->ac.gs_attr_offset, 5);
   else
      ret = si_insert_input_ret(ctx, ret, ctx->args->ac.scratch_offset, 5);
   ret = si_insert_input_ptr(ctx, ret, ctx->args->internal_bindings, 8 + SI_SGPR_INTERNAL_BINDINGS);
   ret = si_insert_input_ptr(ctx, ret, ctx->args->bindless_samplers_and_images,
                             8 + SI_SGPR_BINDLESS_SAMPLERS_AND_IMAGES);
   ret = si_insert_input_ptr(ctx, ret, ctx->args->vs_state_bits, 8 + SI_SGPR_VS_STATE_BITS);
   if (ctx->screen->use_ngg) {
      ret = si_insert_input_ptr(ctx, ret, ctx->args->small_prim_cull_info, 8 + GFX9_SGPR_SMALL_PRIM_CULL_INFO);
      if (ctx->screen->info.gfx_level >= GFX11)
         ret = si_insert_input_ptr(ctx, ret, ctx->args->gs_attr_address, 8 + GFX9_SGPR_ATTRIBUTE_RING_ADDR);
   }

   unsigned vgpr = 8 + GFX9_GS_NUM_USER_SGPR;

   ret = si_insert_input_ret_float(ctx, ret, ctx->args->ac.gs_vtx_offset[0], vgpr++);
   ret = si_insert_input_ret_float(ctx, ret, ctx->args->ac.gs_vtx_offset[1], vgpr++);
   ret = si_insert_input_ret_float(ctx, ret, ctx->args->ac.gs_prim_id, vgpr++);
   ret = si_insert_input_ret_float(ctx, ret, ctx->args->ac.gs_invocation_id, vgpr++);
   ret = si_insert_input_ret_float(ctx, ret, ctx->args->ac.gs_vtx_offset[2], vgpr++);
   ctx->return_value = ret;
}

void si_llvm_gs_build_end(struct si_shader_context *ctx)
{
   if (ctx->screen->info.gfx_level >= GFX9)
      ac_build_endif(&ctx->ac, ctx->merged_wrap_if_label);
}
