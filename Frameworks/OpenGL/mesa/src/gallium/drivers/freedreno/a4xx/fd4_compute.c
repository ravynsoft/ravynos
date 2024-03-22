/*
 * Copyright (C) 2021 Ilia Mirkin <imirkin@alum.mit.edu>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Ilia Mirkin <imirkin@alum.mit.edu>
 */

#include "pipe/p_state.h"

#include "freedreno_resource.h"

#include "fd4_compute.h"
#include "fd4_context.h"
#include "fd4_emit.h"

/* maybe move to fd4_program? */
static void
cs_program_emit(struct fd_ringbuffer *ring, struct ir3_shader_variant *v)
{
   const struct ir3_info *i = &v->info;
   enum a3xx_threadsize thrsz = i->double_threadsize ? FOUR_QUADS : TWO_QUADS;
   unsigned instrlen = v->instrlen;

   /* XXX verify that this is the case on a4xx */
   /* if shader is more than 32*16 instructions, don't preload it.  Similar
    * to the combined restriction of 64*16 for VS+FS
    */
   if (instrlen > 32)
      instrlen = 0;

   OUT_PKT0(ring, REG_A4XX_SP_SP_CTRL_REG, 1);
   OUT_RING(ring, 0x00860010); /* SP_SP_CTRL_REG */

   OUT_PKT0(ring, REG_A4XX_HLSQ_CONTROL_0_REG, 1);
   OUT_RING(ring, A4XX_HLSQ_CONTROL_0_REG_FSTHREADSIZE(TWO_QUADS) |
                     A4XX_HLSQ_CONTROL_0_REG_SINGLECONTEXT |
                     0x000001a0 /* XXX */);

   OUT_PKT0(ring, REG_A4XX_SP_CS_CTRL_REG0, 1);
   OUT_RING(ring, A4XX_SP_CS_CTRL_REG0_THREADSIZE(thrsz) |
                     A4XX_SP_CS_CTRL_REG0_SUPERTHREADMODE |
                     A4XX_SP_CS_CTRL_REG0_HALFREGFOOTPRINT(i->max_half_reg + 1) |
                     A4XX_SP_CS_CTRL_REG0_FULLREGFOOTPRINT(i->max_reg + 1));

   OUT_PKT0(ring, REG_A4XX_HLSQ_UPDATE_CONTROL, 1);
   OUT_RING(ring, 0x00000038); /* HLSQ_UPDATE_CONTROL */

   OUT_PKT0(ring, REG_A4XX_HLSQ_CS_CONTROL_REG, 1);
   OUT_RING(ring, A4XX_HLSQ_CS_CONTROL_REG_CONSTOBJECTOFFSET(0) |
                     A4XX_HLSQ_CS_CONTROL_REG_SHADEROBJOFFSET(0) |
                     A4XX_HLSQ_CS_CONTROL_REG_ENABLED |
                     A4XX_HLSQ_CS_CONTROL_REG_INSTRLENGTH(1) |
                     COND(v->has_ssbo, A4XX_HLSQ_CS_CONTROL_REG_SSBO_ENABLE) |
                     A4XX_HLSQ_CS_CONTROL_REG_CONSTLENGTH(v->constlen / 4));

   uint32_t driver_param_base = v->const_state->offsets.driver_param * 4;
   uint32_t local_invocation_id, work_group_id, local_group_size_id,
      num_wg_id, work_dim_id, unused_id;
   local_invocation_id =
      ir3_find_sysval_regid(v, SYSTEM_VALUE_LOCAL_INVOCATION_ID);
   work_group_id = driver_param_base + IR3_DP_WORKGROUP_ID_X;
   num_wg_id = driver_param_base + IR3_DP_NUM_WORK_GROUPS_X;
   local_group_size_id = driver_param_base + IR3_DP_LOCAL_GROUP_SIZE_X;
   work_dim_id = driver_param_base + IR3_DP_WORK_DIM;
   /* NOTE: At some point we'll want to use this, it's probably WGOFFSETCONSTID */
   unused_id = driver_param_base + IR3_DP_BASE_GROUP_X;

   OUT_PKT0(ring, REG_A4XX_HLSQ_CL_CONTROL_0, 2);
   OUT_RING(ring, A4XX_HLSQ_CL_CONTROL_0_WGIDCONSTID(work_group_id) |
                     A4XX_HLSQ_CL_CONTROL_0_KERNELDIMCONSTID(work_dim_id) |
                     A4XX_HLSQ_CL_CONTROL_0_LOCALIDREGID(local_invocation_id));
   OUT_RING(ring, A4XX_HLSQ_CL_CONTROL_1_UNK0CONSTID(unused_id) |
                     A4XX_HLSQ_CL_CONTROL_1_WORKGROUPSIZECONSTID(local_group_size_id));

   OUT_PKT0(ring, REG_A4XX_HLSQ_CL_KERNEL_CONST, 1);
   OUT_RING(ring, A4XX_HLSQ_CL_KERNEL_CONST_UNK0CONSTID(unused_id) |
                     A4XX_HLSQ_CL_KERNEL_CONST_NUMWGCONSTID(num_wg_id));

   OUT_PKT0(ring, REG_A4XX_HLSQ_CL_WG_OFFSET, 1);
   OUT_RING(ring, A4XX_HLSQ_CL_WG_OFFSET_UNK0CONSTID(unused_id));

   OUT_PKT0(ring, REG_A4XX_HLSQ_MODE_CONTROL, 1);
   OUT_RING(ring, 0x00000003); /* HLSQ_MODE_CONTROL */

   OUT_PKT0(ring, REG_A4XX_HLSQ_UPDATE_CONTROL, 1);
   OUT_RING(ring, 0x00000000); /* HLSQ_UPDATE_CONTROL */

   OUT_PKT0(ring, REG_A4XX_SP_CS_OBJ_START, 1);
   OUT_RELOC(ring, v->bo, 0, 0, 0); /* SP_CS_OBJ_START */

   OUT_PKT0(ring, REG_A4XX_SP_CS_LENGTH_REG, 1);
   OUT_RING(ring, v->instrlen);

   if (instrlen > 0)
      fd4_emit_shader(ring, v);
}

static void
fd4_launch_grid(struct fd_context *ctx,
                const struct pipe_grid_info *info) assert_dt
{
   struct fd4_context *fd4_ctx = fd4_context(ctx);
   struct ir3_shader_key key = {
      .has_per_samp = fd4_ctx->castc_srgb,
      .fastc_srgb = fd4_ctx->castc_srgb,
   };
   struct ir3_shader *shader = ir3_get_shader(ctx->compute);
   struct ir3_shader_variant *v;
   struct fd_ringbuffer *ring = ctx->batch->draw;
   unsigned nglobal = 0;

   if (ir3_get_shader_info(ctx->compute)->uses_texture_gather) {
      key.has_per_samp = true;
      memcpy(key.fsampler_swizzles, fd4_ctx->csampler_swizzles,
             sizeof(key.fsampler_swizzles));
   }

   v = ir3_shader_variant(shader, key, false, &ctx->debug);
   if (!v)
      return;

   if (ctx->dirty_shader[PIPE_SHADER_COMPUTE] & FD_DIRTY_SHADER_PROG)
      cs_program_emit(ring, v);

   fd4_emit_cs_state(ctx, ring, v);
   fd4_emit_cs_consts(v, ring, ctx, info);

   u_foreach_bit (i, ctx->global_bindings.enabled_mask)
      nglobal++;

   if (nglobal > 0) {
      /* global resources don't otherwise get an OUT_RELOC(), since
       * the raw ptr address is emitted ir ir3_emit_cs_consts().
       * So to make the kernel aware that these buffers are referenced
       * by the batch, emit dummy reloc's as part of a no-op packet
       * payload:
       */
      OUT_PKT3(ring, CP_NOP, 2 * nglobal);
      u_foreach_bit (i, ctx->global_bindings.enabled_mask) {
         struct pipe_resource *prsc = ctx->global_bindings.buf[i];
         OUT_RELOC(ring, fd_resource(prsc)->bo, 0, 0, 0);
      }
   }

   const unsigned *local_size =
      info->block; // v->shader->nir->info->workgroup_size;
   const unsigned *num_groups = info->grid;
   /* for some reason, mesa/st doesn't set info->work_dim, so just assume 3: */
   const unsigned work_dim = info->work_dim ? info->work_dim : 3;
   OUT_PKT0(ring, REG_A4XX_HLSQ_CL_NDRANGE_0, 7);
   OUT_RING(ring, A4XX_HLSQ_CL_NDRANGE_0_KERNELDIM(work_dim) |
                     A4XX_HLSQ_CL_NDRANGE_0_LOCALSIZEX(local_size[0] - 1) |
                     A4XX_HLSQ_CL_NDRANGE_0_LOCALSIZEY(local_size[1] - 1) |
                     A4XX_HLSQ_CL_NDRANGE_0_LOCALSIZEZ(local_size[2] - 1));
   OUT_RING(ring,
            A4XX_HLSQ_CL_NDRANGE_1_SIZE_X(local_size[0] * num_groups[0]));
   OUT_RING(ring, 0); /* HLSQ_CL_NDRANGE_2_GLOBALOFF_X */
   OUT_RING(ring,
            A4XX_HLSQ_CL_NDRANGE_3_SIZE_Y(local_size[1] * num_groups[1]));
   OUT_RING(ring, 0); /* HLSQ_CL_NDRANGE_4_GLOBALOFF_Y */
   OUT_RING(ring,
            A4XX_HLSQ_CL_NDRANGE_5_SIZE_Z(local_size[2] * num_groups[2]));
   OUT_RING(ring, 0); /* HLSQ_CL_NDRANGE_6_GLOBALOFF_Z */

   if (info->indirect) {
      struct fd_resource *rsc = fd_resource(info->indirect);

      fd_event_write(ctx->batch, ring, CACHE_FLUSH);
      fd_wfi(ctx->batch, ring);

      OUT_PKT3(ring, CP_EXEC_CS_INDIRECT, 3);
      OUT_RING(ring, 0x00000000);
      OUT_RELOC(ring, rsc->bo, info->indirect_offset, 0, 0);
      OUT_RING(ring,
               A4XX_CP_EXEC_CS_INDIRECT_2_LOCALSIZEX(local_size[0] - 1) |
                  A4XX_CP_EXEC_CS_INDIRECT_2_LOCALSIZEY(local_size[1] - 1) |
                  A4XX_CP_EXEC_CS_INDIRECT_2_LOCALSIZEZ(local_size[2] - 1));
   } else {
      OUT_PKT3(ring, CP_EXEC_CS, 4);
      OUT_RING(ring, 0x00000000);
      OUT_RING(ring, CP_EXEC_CS_1_NGROUPS_X(info->grid[0]));
      OUT_RING(ring, CP_EXEC_CS_2_NGROUPS_Y(info->grid[1]));
      OUT_RING(ring, CP_EXEC_CS_3_NGROUPS_Z(info->grid[2]));
   }
}

void
fd4_compute_init(struct pipe_context *pctx) disable_thread_safety_analysis
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->launch_grid = fd4_launch_grid;
   pctx->create_compute_state = ir3_shader_compute_state_create;
   pctx->delete_compute_state = ir3_shader_state_delete;
}
