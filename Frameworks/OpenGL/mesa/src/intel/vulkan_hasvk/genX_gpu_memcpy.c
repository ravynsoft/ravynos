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
#if GFX_VER >= 8
   anv_batch_emit(batch, GENX(3DSTATE_VF_INSTANCING), vfi) {
      vfi.InstancingEnable = false;
      vfi.VertexElementIndex = 0;
   }
   anv_batch_emit(batch, GENX(3DSTATE_VF_SGVS), sgvs);
#endif

   /* Disable all shader stages */
   anv_batch_emit(batch, GENX(3DSTATE_VS), vs);
   anv_batch_emit(batch, GENX(3DSTATE_HS), hs);
   anv_batch_emit(batch, GENX(3DSTATE_TE), te);
   anv_batch_emit(batch, GENX(3DSTATE_DS), DS);
   anv_batch_emit(batch, GENX(3DSTATE_GS), gs);
   anv_batch_emit(batch, GENX(3DSTATE_PS), gs);

   anv_batch_emit(batch, GENX(3DSTATE_SBE), sbe) {
      sbe.VertexURBEntryReadOffset = 1;
      sbe.NumberofSFOutputAttributes = 1;
      sbe.VertexURBEntryReadLength = 1;
#if GFX_VER >= 8
      sbe.ForceVertexURBEntryReadLength = true;
      sbe.ForceVertexURBEntryReadOffset = true;
#endif
   }

   /* Emit URB setup.  We tell it that the VS is active because we want it to
    * allocate space for the VS.  Even though one isn't run, we need VUEs to
    * store the data that VF is going to pass to SOL.
    */
   const unsigned entry_size[4] = { DIV_ROUND_UP(32, 64), 1, 1, 1 };

   genX(emit_urb_setup)(device, batch, l3_config,
                        VK_SHADER_STAGE_VERTEX_BIT, entry_size, NULL);

#if GFX_VER >= 8
   anv_batch_emit(batch, GENX(3DSTATE_VF_TOPOLOGY), topo) {
      topo.PrimitiveTopologyType = _3DPRIM_POINTLIST;
   }
#endif

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
#if (GFX_VER >= 8)
         .BufferSize = size,
#else
         .EndAddress = anv_address_add(src, size - 1),
#endif
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


   anv_batch_emit(batch, GENX(3DSTATE_SO_BUFFER), sob) {
      sob.SOBufferIndex = 0;
      sob.MOCS = anv_mocs(device, dst.bo, ISL_SURF_USAGE_STREAM_OUT_BIT),
      sob.SurfaceBaseAddress = dst;

#if GFX_VER >= 8
      sob.SOBufferEnable = true;
      sob.SurfaceSize = size / 4 - 1;
#else
      sob.SurfacePitch = bs;
      sob.SurfaceEndAddress = anv_address_add(dst, size);
#endif

#if GFX_VER >= 8
      /* As SOL writes out data, it updates the SO_WRITE_OFFSET registers with
       * the end position of the stream.  We need to reset this value to 0 at
       * the beginning of the run or else SOL will start at the offset from
       * the previous draw.
       */
      sob.StreamOffsetWriteEnable = true;
      sob.StreamOffset = 0;
#endif
   }

#if GFX_VER <= 7
   /* The hardware can do this for us on BDW+ (see above) */
   anv_batch_emit(batch, GENX(MI_LOAD_REGISTER_IMM), load) {
      load.RegisterOffset = GENX(SO_WRITE_OFFSET0_num);
      load.DataDWord = 0;
   }
#endif

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

   anv_batch_emit(batch, GENX(3DSTATE_STREAMOUT), so) {
      so.SOFunctionEnable = true;
      so.RenderingDisable = true;
      so.Stream0VertexReadOffset = 0;
      so.Stream0VertexReadLength = DIV_ROUND_UP(32, 64);
#if GFX_VER >= 8
      so.Buffer0SurfacePitch = bs;
#else
      so.SOBufferEnable0 = true;
#endif
   }

   anv_batch_emit(batch, GENX(3DPRIMITIVE), prim) {
      prim.VertexAccessType         = SEQUENTIAL;
      prim.PrimitiveTopologyType    = _3DPRIM_POINTLIST;
      prim.VertexCountPerInstance   = size / bs;
      prim.StartVertexLocation      = 0;
      prim.InstanceCount            = 1;
      prim.StartInstanceLocation    = 0;
      prim.BaseVertexLocation       = 0;
   }
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

   anv_batch_emit(batch, GENX(PIPELINE_SELECT), ps) {
      ps.PipelineSelection = _3D;
   }

   emit_common_so_memcpy(batch, device, cfg);
}

void
genX(emit_so_memcpy_fini)(struct anv_memcpy_state *state)
{
   genX(emit_apply_pipe_flushes)(state->batch, state->device, _3D,
                                 ANV_PIPE_END_OF_PIPE_SYNC_BIT);

   anv_batch_emit(state->batch, GENX(MI_BATCH_BUFFER_END), end);

   if ((state->batch->next - state->batch->start) & 4)
      anv_batch_emit(state->batch, GENX(MI_NOOP), noop);
}

void
genX(emit_so_memcpy)(struct anv_memcpy_state *state,
                     struct anv_address dst, struct anv_address src,
                     uint32_t size)
{
   if (GFX_VER >= 8 && !anv_use_relocations(state->device->physical) &&
       anv_gfx8_9_vb_cache_range_needs_workaround(&state->vb_bound,
                                                  &state->vb_dirty,
                                                  src, size)) {
      genX(emit_apply_pipe_flushes)(state->batch, state->device, _3D,
                                    ANV_PIPE_CS_STALL_BIT |
                                    ANV_PIPE_VF_CACHE_INVALIDATE_BIT);
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

   genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(cmd_buffer, 32, src, size);
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   genX(flush_pipeline_select_3d)(cmd_buffer);

   emit_common_so_memcpy(&cmd_buffer->batch, cmd_buffer->device,
                         cmd_buffer->state.current_l3_config);
   emit_so_memcpy(&cmd_buffer->batch, cmd_buffer->device, dst, src, size);

   genX(cmd_buffer_update_dirty_vbs_for_gfx8_vb_flush)(cmd_buffer, SEQUENTIAL,
                                                       1ull << 32);

   /* Invalidate pipeline & raster discard since we touch
    * 3DSTATE_STREAMOUT.
    */
   cmd_buffer->state.gfx.dirty |= ANV_CMD_DIRTY_PIPELINE;
   BITSET_SET(cmd_buffer->vk.dynamic_graphics_state.dirty,
              MESA_VK_DYNAMIC_RS_RASTERIZER_DISCARD_ENABLE);
}
