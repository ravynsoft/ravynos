/*
 * Copyright © 2016 Intel Corporation
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

#include "anv_private.h"

#include "genxml/gen_macros.h"
#include "genxml/genX_pack.h"

#include "common/intel_l3_config.h"

/**
 * This file implements some lightweight memcpy/memset operations on the GPU
 * using a vertex buffer and streamout.
 */

/**
 * Returns the greatest common divisor of a and b that is a power of two.
 */
static uint64_t
gcd_pow2_u64(uint64_t a, uint64_t b)
{
   assert(a > 0 || b > 0);

   unsigned a_log2 = ffsll(a) - 1;
   unsigned b_log2 = ffsll(b) - 1;

   /* If either a or b is 0, then a_log2 or b_log2 will be UINT_MAX in which
    * case, the MIN2() will take the other one.  If both are 0 then we will
    * hit the assert above.
    */
   return 1 << MIN2(a_log2, b_log2);
}

static void
emit_common_so_memcpy(struct anv_batch *batch, struct anv_device *device,
                      const struct intel_l3_config *l3_config)
{
   anv_batch_emit(batch, GENX(3DSTATE_VF_INSTANCING), vfi) {
      vfi.InstancingEnable = false;
      vfi.VertexElementIndex = 0;
   }
   anv_batch_emit(batch, GENX(3DSTATE_VF_SGVS), sgvs);
#if GFX_VER >= 11
   anv_batch_emit(batch, GENX(3DSTATE_VF_SGVS_2), sgvs);
#endif

   /* Disable all shader stages */
   anv_batch_emit(batch, GENX(3DSTATE_VS), vs);
   anv_batch_emit(batch, GENX(3DSTATE_HS), hs);
   anv_batch_emit(batch, GENX(3DSTATE_TE), te);
   anv_batch_emit(batch, GENX(3DSTATE_DS), DS);
   anv_batch_emit(batch, GENX(3DSTATE_GS), gs);
   anv_batch_emit(batch, GENX(3DSTATE_PS), gs);

#if GFX_VERx10 >= 125
   /* Disable Mesh, we can't have this and streamout enabled at the same
    * time.
    */
   if (device->vk.enabled_extensions.EXT_mesh_shader) {
      anv_batch_emit(batch, GENX(3DSTATE_MESH_CONTROL), mesh);
      anv_batch_emit(batch, GENX(3DSTATE_TASK_CONTROL), task);
   }
#endif

#if INTEL_WA_16013994831_GFX_VER
   /* Wa_16013994831 - Disable preemption during streamout. */
   if (intel_needs_workaround(device->info, 16013994831))
      genX(batch_set_preemption)(batch, device->info, _3D, false);
#endif

   anv_batch_emit(batch, GENX(3DSTATE_SBE), sbe) {
      sbe.VertexURBEntryReadOffset = 1;
      sbe.NumberofSFOutputAttributes = 1;
      sbe.VertexURBEntryReadLength = 1;
      sbe.ForceVertexURBEntryReadLength = true;
      sbe.ForceVertexURBEntryReadOffset = true;

      for (unsigned i = 0; i < 32; i++)
         sbe.AttributeActiveComponentFormat[i] = ACF_XYZW;
   }

   /* Emit URB setup.  We tell it that the VS is active because we want it to
    * allocate space for the VS.  Even though one isn't run, we need VUEs to
    * store the data that VF is going to pass to SOL.
    */
   const unsigned entry_size[4] = { DIV_ROUND_UP(32, 64), 1, 1, 1 };

   genX(emit_urb_setup)(device, batch, l3_config,
                        VK_SHADER_STAGE_VERTEX_BIT, entry_size, NULL);

#if GFX_VER >= 12
   /* Disable Primitive Replication. */
   anv_batch_emit(batch, GENX(3DSTATE_PRIMITIVE_REPLICATION), pr);
#endif

   anv_batch_emit(batch, GENX(3DSTATE_VF_TOPOLOGY), topo) {
      topo.PrimitiveTopologyType = _3DPRIM_POINTLIST;
   }

   anv_batch_emit(batch, GENX(3DSTATE_VF_STATISTICS), vf) {
      vf.StatisticsEnable = false;
   }
}

static void
emit_so_memcpy(struct anv_batch *batch, struct anv_device *device,
               struct anv_address dst, struct anv_address src,
               uint32_t size)
{
   /* The maximum copy block size is 4 32-bit components at a time. */
   assert(size % 4 == 0);
   unsigned bs = gcd_pow2_u64(16, size);

   enum isl_format format;
   switch (bs) {
   case 4:  format = ISL_FORMAT_R32_UINT;          break;
   case 8:  format = ISL_FORMAT_R32G32_UINT;       break;
   case 16: format = ISL_FORMAT_R32G32B32A32_UINT; break;
   default:
      unreachable("Invalid size");
   }

   uint32_t *dw;
   dw = anv_batch_emitn(batch, 5, GENX(3DSTATE_VERTEX_BUFFERS));
   GENX(VERTEX_BUFFER_STATE_pack)(batch, dw + 1,
      &(struct GENX(VERTEX_BUFFER_STATE)) {
         .VertexBufferIndex = 32, /* Reserved for this */
         .AddressModifyEnable = true,
         .BufferStartingAddress = src,
         .BufferPitch = bs,
         .MOCS = anv_mocs(device, src.bo, 0),
#if GFX_VER >= 12
         .L3BypassDisable = true,
#endif
         .BufferSize = size,
      });

   dw = anv_batch_emitn(batch, 3, GENX(3DSTATE_VERTEX_ELEMENTS));
   GENX(VERTEX_ELEMENT_STATE_pack)(batch, dw + 1,
      &(struct GENX(VERTEX_ELEMENT_STATE)) {
         .VertexBufferIndex = 32,
         .Valid = true,
         .SourceElementFormat = format,
         .SourceElementOffset = 0,
         .Component0Control = (bs >= 4) ? VFCOMP_STORE_SRC : VFCOMP_STORE_0,
         .Component1Control = (bs >= 8) ? VFCOMP_STORE_SRC : VFCOMP_STORE_0,
         .Component2Control = (bs >= 12) ? VFCOMP_STORE_SRC : VFCOMP_STORE_0,
         .Component3Control = (bs >= 16) ? VFCOMP_STORE_SRC : VFCOMP_STORE_0,
      });


   /* Wa_16011411144:
    *
    * SW must insert a PIPE_CONTROL cmd before and after the
    * 3dstate_so_buffer_index_0/1/2/3 states to ensure so_buffer_index_*
    * state is not combined with other state changes.
    */
   if (intel_needs_workaround(device->info, 16011411144))
      genx_batch_emit_pipe_control(batch, device->info, _3D, ANV_PIPE_CS_STALL_BIT);

   anv_batch_emit(batch, GENX(3DSTATE_SO_BUFFER), sob) {
#if GFX_VER < 12
      sob.SOBufferIndex = 0;
#else
      sob._3DCommandOpcode = 0;
      sob._3DCommandSubOpcode = SO_BUFFER_INDEX_0_CMD;
#endif
      sob.MOCS = anv_mocs(device, dst.bo, ISL_SURF_USAGE_STREAM_OUT_BIT),
      sob.SurfaceBaseAddress = dst;

      sob.SOBufferEnable = true;
      sob.SurfaceSize = size / 4 - 1;

      /* As SOL writes out data, it updates the SO_WRITE_OFFSET registers with
       * the end position of the stream.  We need to reset this value to 0 at
       * the beginning of the run or else SOL will start at the offset from
       * the previous draw.
       */
      sob.StreamOffsetWriteEnable = true;
      sob.StreamOffset = 0;
   }

   /* Wa_16011411144: also CS_STALL after touching SO_BUFFER change */
   if (intel_needs_workaround(device->info, 16011411144))
      genx_batch_emit_pipe_control(batch, device->info, _3D, ANV_PIPE_CS_STALL_BIT);

   dw = anv_batch_emitn(batch, 5, GENX(3DSTATE_SO_DECL_LIST),
                        .StreamtoBufferSelects0 = (1 << 0),
                        .NumEntries0 = 1);
   GENX(SO_DECL_ENTRY_pack)(batch, dw + 3,
      &(struct GENX(SO_DECL_ENTRY)) {
         .Stream0Decl = {
            .OutputBufferSlot = 0,
            .RegisterIndex = 0,
            .ComponentMask = (1 << (bs / 4)) - 1,
         },
      });

#if GFX_VERx10 == 125
      /* Wa_14015946265: Send PC with CS stall after SO_DECL. */
      genx_batch_emit_pipe_control(batch, device->info, _3D, ANV_PIPE_CS_STALL_BIT);
#endif

   anv_batch_emit(batch, GENX(3DSTATE_STREAMOUT), so) {
      so.SOFunctionEnable = true;
      so.RenderingDisable = true;
      so.Stream0VertexReadOffset = 0;
      so.Stream0VertexReadLength = DIV_ROUND_UP(32, 64);
      so.Buffer0SurfacePitch = bs;
   }

   genX(emit_breakpoint)(batch, device, true);
   anv_batch_emit(batch, GENX(3DPRIMITIVE), prim) {
      prim.VertexAccessType         = SEQUENTIAL;
      prim.VertexCountPerInstance   = size / bs;
      prim.StartVertexLocation      = 0;
      prim.InstanceCount            = 1;
      prim.StartInstanceLocation    = 0;
      prim.BaseVertexLocation       = 0;
   }

   genX(batch_emit_post_3dprimitive_was)(batch,
                                         device,
                                         _3DPRIM_POINTLIST, size / bs);

   genX(emit_breakpoint)(batch, device, false);
}

void
genX(emit_so_memcpy_init)(struct anv_memcpy_state *state,
                          struct anv_device *device,
                          struct anv_batch *batch)
{
   memset(state, 0, sizeof(*state));

   state->batch = batch;
   state->device = device;

   const struct intel_l3_config *cfg = intel_get_default_l3_config(device->info);
   genX(emit_l3_config)(batch, device, cfg);
   genX(emit_pipeline_select)(batch, _3D, device);

   emit_common_so_memcpy(batch, device, cfg);
}

void
genX(emit_so_memcpy_fini)(struct anv_memcpy_state *state)
{
   genX(emit_apply_pipe_flushes)(state->batch, state->device, _3D,
                                 ANV_PIPE_END_OF_PIPE_SYNC_BIT,
                                 NULL);
}

void
genX(emit_so_memcpy_end)(struct anv_memcpy_state *state)
{
   if (intel_needs_workaround(state->device->info, 16013994831))
      genX(batch_set_preemption)(state->batch, state->device->info, _3D, true);

   anv_batch_emit(state->batch, GENX(MI_BATCH_BUFFER_END), end);

   if ((state->batch->next - state->batch->start) & 4)
      anv_batch_emit(state->batch, GENX(MI_NOOP), noop);
}

void
genX(emit_so_memcpy)(struct anv_memcpy_state *state,
                     struct anv_address dst, struct anv_address src,
                     uint32_t size)
{
   if (GFX_VER == 9 &&
       anv_gfx8_9_vb_cache_range_needs_workaround(&state->vb_bound,
                                                  &state->vb_dirty,
                                                  src, size)) {
      genX(emit_apply_pipe_flushes)(state->batch, state->device, _3D,
                                    ANV_PIPE_CS_STALL_BIT |
                                    ANV_PIPE_VF_CACHE_INVALIDATE_BIT,
                                    NULL);
      memset(&state->vb_dirty, 0, sizeof(state->vb_dirty));
   }

   emit_so_memcpy(state->batch, state->device, dst, src, size);
}

void
genX(cmd_buffer_so_memcpy)(struct anv_cmd_buffer *cmd_buffer,
                           struct anv_address dst, struct anv_address src,
                           uint32_t size)
{
   if (size == 0)
      return;

   if (!cmd_buffer->state.current_l3_config) {
      const struct intel_l3_config *cfg =
         intel_get_default_l3_config(cmd_buffer->device->info);
      genX(cmd_buffer_config_l3)(cmd_buffer, cfg);
   }

#if GFX_VER == 9
   genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(cmd_buffer, 32, src, size);
#endif

   /* Wa_14015814527 */
   genX(apply_task_urb_workaround)(cmd_buffer);

   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   genX(flush_pipeline_select_3d)(cmd_buffer);

   emit_common_so_memcpy(&cmd_buffer->batch, cmd_buffer->device,
                         cmd_buffer->state.current_l3_config);
   emit_so_memcpy(&cmd_buffer->batch, cmd_buffer->device, dst, src, size);

#if GFX_VER == 9
   genX(cmd_buffer_update_dirty_vbs_for_gfx8_vb_flush)(cmd_buffer, SEQUENTIAL,
                                                       1ull << 32);
#endif

   /* Flag all the instructions emitted by the memcpy. */
   struct anv_gfx_dynamic_state *hw_state =
      &cmd_buffer->state.gfx.dyn_state;

   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_URB);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF_STATISTICS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF_TOPOLOGY);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VERTEX_INPUT);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF_SGVS);
#if GFX_VER >= 11
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF_SGVS_2);
#endif
#if GFX_VER >= 12
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_PRIMITIVE_REPLICATION);
#endif
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SO_DECL_LIST);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_STREAMOUT);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SAMPLE_MASK);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_MULTISAMPLE);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SF);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SBE);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_HS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_DS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_TE);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_GS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_PS);
   if (cmd_buffer->device->vk.enabled_extensions.EXT_mesh_shader) {
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_MESH_CONTROL);
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_TASK_CONTROL);
   }

   cmd_buffer->state.gfx.dirty |= ~(ANV_CMD_DIRTY_PIPELINE |
                                    ANV_CMD_DIRTY_INDEX_BUFFER);
}
