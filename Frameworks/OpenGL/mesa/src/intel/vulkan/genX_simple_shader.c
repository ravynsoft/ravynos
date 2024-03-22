/*
 * Copyright © 2023 Intel Corporation
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

#include <assert.h>
#include <stdbool.h>

#include "util/macros.h"

#include "anv_private.h"

#include "genxml/gen_macros.h"
#include "genxml/genX_pack.h"
#include "common/intel_genX_state.h"

static void
genX(emit_simpler_shader_init_fragment)(struct anv_simple_shader *state)
{
   assert(state->cmd_buffer == NULL ||
          state->cmd_buffer->state.current_pipeline == _3D);

   struct anv_batch *batch = state->batch;
   struct anv_device *device = state->device;
   const struct brw_wm_prog_data *prog_data =
      brw_wm_prog_data_const(state->kernel->prog_data);

   uint32_t *dw = anv_batch_emitn(batch,
                                  1 + 2 * GENX(VERTEX_ELEMENT_STATE_length),
                                  GENX(3DSTATE_VERTEX_ELEMENTS));
   /* You might think there is some shady stuff going here and you would be
    * right. We're setting up 2 VERTEX_ELEMENT_STATE yet we're only providing
    * 1 (positions) VERTEX_BUFFER_STATE later.
    *
    * Find more about how to set up a 3D pipeline with a fragment shader but
    * without a vertex shader in blorp_emit_vertex_elements() in
    * blorp_genX_exec.h.
    */
   GENX(VERTEX_ELEMENT_STATE_pack)(
      batch, dw + 1, &(struct GENX(VERTEX_ELEMENT_STATE)) {
         .VertexBufferIndex = 1,
         .Valid = true,
         .SourceElementFormat = ISL_FORMAT_R32G32B32A32_FLOAT,
         .SourceElementOffset = 0,
         .Component0Control = VFCOMP_STORE_SRC,
         .Component1Control = VFCOMP_STORE_0,
         .Component2Control = VFCOMP_STORE_0,
         .Component3Control = VFCOMP_STORE_0,
      });
   GENX(VERTEX_ELEMENT_STATE_pack)(
      batch, dw + 3, &(struct GENX(VERTEX_ELEMENT_STATE)) {
         .VertexBufferIndex   = 0,
         .Valid               = true,
         .SourceElementFormat = ISL_FORMAT_R32G32B32_FLOAT,
         .SourceElementOffset = 0,
         .Component0Control   = VFCOMP_STORE_SRC,
         .Component1Control   = VFCOMP_STORE_SRC,
         .Component2Control   = VFCOMP_STORE_SRC,
         .Component3Control   = VFCOMP_STORE_1_FP,
      });

   anv_batch_emit(batch, GENX(3DSTATE_VF_STATISTICS), vf);
   anv_batch_emit(batch, GENX(3DSTATE_VF_SGVS), sgvs) {
      sgvs.InstanceIDEnable = true;
      sgvs.InstanceIDComponentNumber = COMP_1;
      sgvs.InstanceIDElementOffset = 0;
   }
#if GFX_VER >= 11
   anv_batch_emit(batch, GENX(3DSTATE_VF_SGVS_2), sgvs);
#endif
   anv_batch_emit(batch, GENX(3DSTATE_VF_INSTANCING), vfi) {
      vfi.InstancingEnable   = false;
      vfi.VertexElementIndex = 0;
   }
   anv_batch_emit(batch, GENX(3DSTATE_VF_INSTANCING), vfi) {
      vfi.InstancingEnable   = false;
      vfi.VertexElementIndex = 1;
   }

   anv_batch_emit(batch, GENX(3DSTATE_VF_TOPOLOGY), topo) {
      topo.PrimitiveTopologyType = _3DPRIM_RECTLIST;
   }

   /* Emit URB setup.  We tell it that the VS is active because we want it to
    * allocate space for the VS.  Even though one isn't run, we need VUEs to
    * store the data that VF is going to pass to SOL.
    */
   const unsigned entry_size[4] = { DIV_ROUND_UP(32, 64), 1, 1, 1 };

   genX(emit_l3_config)(batch, device, state->l3_config);

   state->cmd_buffer->state.current_l3_config = state->l3_config;

   enum intel_urb_deref_block_size deref_block_size;
   genX(emit_urb_setup)(device, batch, state->l3_config,
                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                        entry_size, &deref_block_size);

   anv_batch_emit(batch, GENX(3DSTATE_PS_BLEND), ps_blend) {
      ps_blend.HasWriteableRT = true;
   }

   anv_batch_emit(batch, GENX(3DSTATE_WM_DEPTH_STENCIL), wm);

#if GFX_VER >= 12
   anv_batch_emit(batch, GENX(3DSTATE_DEPTH_BOUNDS), db) {
      db.DepthBoundsTestEnable = false;
      db.DepthBoundsTestMinValue = 0.0;
      db.DepthBoundsTestMaxValue = 1.0;
   }
#endif

   anv_batch_emit(batch, GENX(3DSTATE_MULTISAMPLE), ms);
   anv_batch_emit(batch, GENX(3DSTATE_SAMPLE_MASK), sm) {
      sm.SampleMask = 0x1;
   }

   anv_batch_emit(batch, GENX(3DSTATE_VS), vs);
   anv_batch_emit(batch, GENX(3DSTATE_HS), hs);
   anv_batch_emit(batch, GENX(3DSTATE_TE), te);
   anv_batch_emit(batch, GENX(3DSTATE_DS), DS);

#if GFX_VERx10 >= 125
   if (device->vk.enabled_extensions.EXT_mesh_shader) {
      anv_batch_emit(batch, GENX(3DSTATE_MESH_CONTROL), mesh);
      anv_batch_emit(batch, GENX(3DSTATE_TASK_CONTROL), task);
   }
#endif

   anv_batch_emit(batch, GENX(3DSTATE_STREAMOUT), so);

   anv_batch_emit(batch, GENX(3DSTATE_GS), gs);

   anv_batch_emit(batch, GENX(3DSTATE_CLIP), clip) {
      clip.PerspectiveDivideDisable = true;
   }

   anv_batch_emit(batch, GENX(3DSTATE_SF), sf) {
#if GFX_VER >= 12
      sf.DerefBlockSize = deref_block_size;
#endif
   }

   anv_batch_emit(batch, GENX(3DSTATE_RASTER), raster) {
      raster.CullMode = CULLMODE_NONE;
   }

   anv_batch_emit(batch, GENX(3DSTATE_SBE), sbe) {
      sbe.VertexURBEntryReadOffset = 1;
      sbe.NumberofSFOutputAttributes = prog_data->num_varying_inputs;
      sbe.VertexURBEntryReadLength = MAX2((prog_data->num_varying_inputs + 1) / 2, 1);
      sbe.ConstantInterpolationEnable = prog_data->flat_inputs;
      sbe.ForceVertexURBEntryReadLength = true;
      sbe.ForceVertexURBEntryReadOffset = true;
      for (unsigned i = 0; i < 32; i++)
         sbe.AttributeActiveComponentFormat[i] = ACF_XYZW;
   }

   anv_batch_emit(batch, GENX(3DSTATE_WM), wm);

   anv_batch_emit(batch, GENX(3DSTATE_PS), ps) {
      intel_set_ps_dispatch_state(&ps, device->info, prog_data,
                                  1 /* rasterization_samples */,
                                  0 /* msaa_flags */);

      ps.VectorMaskEnable       = prog_data->uses_vmask;

      ps.BindingTableEntryCount = GFX_VER == 9 ? 1 : 0;
#if GFX_VER < 20
      ps.PushConstantEnable     = prog_data->base.nr_params > 0 ||
                                  prog_data->base.ubo_ranges[0].length;
#endif

      ps.DispatchGRFStartRegisterForConstantSetupData0 =
         brw_wm_prog_data_dispatch_grf_start_reg(prog_data, ps, 0);
      ps.DispatchGRFStartRegisterForConstantSetupData1 =
         brw_wm_prog_data_dispatch_grf_start_reg(prog_data, ps, 1);
#if GFX_VER < 20
      ps.DispatchGRFStartRegisterForConstantSetupData2 =
         brw_wm_prog_data_dispatch_grf_start_reg(prog_data, ps, 2);
#endif

      ps.KernelStartPointer0 = state->kernel->kernel.offset +
         brw_wm_prog_data_prog_offset(prog_data, ps, 0);
      ps.KernelStartPointer1 = state->kernel->kernel.offset +
         brw_wm_prog_data_prog_offset(prog_data, ps, 1);
#if GFX_VER < 20
      ps.KernelStartPointer2 = state->kernel->kernel.offset +
         brw_wm_prog_data_prog_offset(prog_data, ps, 2);
#endif

      ps.MaximumNumberofThreadsPerPSD = device->info->max_threads_per_psd - 1;
   }

   anv_batch_emit(batch, GENX(3DSTATE_PS_EXTRA), psx) {
      psx.PixelShaderValid = true;
#if GFX_VER < 20
      psx.AttributeEnable = prog_data->num_varying_inputs > 0;
#endif
      psx.PixelShaderIsPerSample = prog_data->persample_dispatch;
      psx.PixelShaderComputedDepthMode = prog_data->computed_depth_mode;
      psx.PixelShaderComputesStencil = prog_data->computed_stencil;
   }

   anv_batch_emit(batch, GENX(3DSTATE_VIEWPORT_STATE_POINTERS_CC), cc) {
      struct anv_state cc_state =
         anv_state_stream_alloc(state->dynamic_state_stream,
                                4 * GENX(CC_VIEWPORT_length), 32);
      if (cc_state.map == NULL)
         return;

      struct GENX(CC_VIEWPORT) cc_viewport = {
         .MinimumDepth = 0.0f,
         .MaximumDepth = 1.0f,
      };
      GENX(CC_VIEWPORT_pack)(NULL, cc_state.map, &cc_viewport);
      cc.CCViewportPointer = cc_state.offset;
   }

#if GFX_VER >= 12
   /* Disable Primitive Replication. */
   anv_batch_emit(batch, GENX(3DSTATE_PRIMITIVE_REPLICATION), pr);
#endif

   anv_batch_emit(batch, GENX(3DSTATE_PUSH_CONSTANT_ALLOC_VS), alloc);
   anv_batch_emit(batch, GENX(3DSTATE_PUSH_CONSTANT_ALLOC_HS), alloc);
   anv_batch_emit(batch, GENX(3DSTATE_PUSH_CONSTANT_ALLOC_DS), alloc);
   anv_batch_emit(batch, GENX(3DSTATE_PUSH_CONSTANT_ALLOC_GS), alloc);
   anv_batch_emit(batch, GENX(3DSTATE_PUSH_CONSTANT_ALLOC_PS), alloc) {
      alloc.ConstantBufferOffset = 0;
      alloc.ConstantBufferSize   = device->info->max_constant_urb_size_kb;
   }

#if GFX_VERx10 == 125
   /* DG2: Wa_22011440098
    * MTL: Wa_18022330953
    *
    * In 3D mode, after programming push constant alloc command immediately
    * program push constant command(ZERO length) without any commit between
    * them.
    *
    * Note that Wa_16011448509 isn't needed here as all address bits are zero.
    */
   anv_batch_emit(batch, GENX(3DSTATE_CONSTANT_ALL), c) {
      /* Update empty push constants for all stages (bitmask = 11111b) */
      c.ShaderUpdateEnable = 0x1f;
      c.MOCS = anv_mocs(device, NULL, 0);
   }
#endif

#if GFX_VER == 9
   /* Allocate a binding table for Gfx9 for 2 reason :
    *
    *   1. we need a to emit a 3DSTATE_BINDING_TABLE_POINTERS_PS to make the
    *      HW apply the preceeding 3DSTATE_CONSTANT_PS
    *
    *   2. Emitting an empty 3DSTATE_BINDING_TABLE_POINTERS_PS would cause RT
    *      writes (even though they're empty) to disturb later writes
    *      (probably due to RT cache)
    *
    * Our binding table only has one entry to the null surface.
    */
   uint32_t bt_offset;
   state->bt_state =
      anv_cmd_buffer_alloc_binding_table(state->cmd_buffer, 1, &bt_offset);
   if (state->bt_state.map == NULL) {
      VkResult result = anv_cmd_buffer_new_binding_table_block(state->cmd_buffer);
      if (result != VK_SUCCESS)
         return;

      /* Re-emit state base addresses so we get the new surface state base
       * address before we start emitting binding tables etc.
       */
      genX(cmd_buffer_emit_state_base_address)(state->cmd_buffer);

      state->bt_state =
         anv_cmd_buffer_alloc_binding_table(state->cmd_buffer, 1, &bt_offset);
      assert(state->bt_state.map != NULL);
   }

   uint32_t *bt_map = state->bt_state.map;
   bt_map[0] = anv_bindless_state_for_binding_table(
      device,
      device->null_surface_state).offset + bt_offset;

   state->cmd_buffer->state.descriptors_dirty |= VK_SHADER_STAGE_FRAGMENT_BIT;
#endif

   /* Flag all the instructions emitted by the memcpy. */
   struct anv_gfx_dynamic_state *hw_state =
      &state->cmd_buffer->state.gfx.dyn_state;

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
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_STREAMOUT);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VIEWPORT_CC);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_CLIP);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_RASTER);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SAMPLE_MASK);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_MULTISAMPLE);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_DEPTH_BOUNDS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_WM);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_WM_DEPTH_STENCIL);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SF);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SBE);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_HS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_DS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_TE);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_GS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_PS);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_PS_EXTRA);
   BITSET_SET(hw_state->dirty, ANV_GFX_STATE_PS_BLEND);
   if (device->vk.enabled_extensions.EXT_mesh_shader) {
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_MESH_CONTROL);
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_TASK_CONTROL);
   }

   state->cmd_buffer->state.gfx.vb_dirty = BITFIELD_BIT(0);
   state->cmd_buffer->state.gfx.dirty |= ~(ANV_CMD_DIRTY_INDEX_BUFFER |
                                           ANV_CMD_DIRTY_XFB_ENABLE);
   state->cmd_buffer->state.push_constants_dirty |= VK_SHADER_STAGE_FRAGMENT_BIT;
   state->cmd_buffer->state.gfx.push_constant_stages = VK_SHADER_STAGE_FRAGMENT_BIT;
}

static void
genX(emit_simpler_shader_init_compute)(struct anv_simple_shader *state)
{
   assert(state->cmd_buffer == NULL ||
          state->cmd_buffer->state.current_pipeline == GPGPU);

#if GFX_VERx10 >= 125
   struct anv_shader_bin *cs_bin = state->kernel;
   const struct brw_cs_prog_data *prog_data =
      (const struct brw_cs_prog_data *) cs_bin->prog_data;
   /* Currently our simple shaders are simple enough that they never spill. */
   assert(prog_data->base.total_scratch == 0);
   if (state->cmd_buffer != NULL) {
      genX(cmd_buffer_ensure_cfe_state)(state->cmd_buffer, 0);
   } else {
      anv_batch_emit(state->batch, GENX(CFE_STATE), cfe) {
         cfe.MaximumNumberofThreads =
            state->device->info->max_cs_threads *
            state->device->info->subslice_total;
      }
   }
#endif
}

/** Initialize a simple shader emission */
void
genX(emit_simple_shader_init)(struct anv_simple_shader *state)
{
   assert(state->kernel->stage == MESA_SHADER_FRAGMENT ||
          state->kernel->stage == MESA_SHADER_COMPUTE);

   if (state->kernel->stage == MESA_SHADER_FRAGMENT)
      genX(emit_simpler_shader_init_fragment)(state);
   else
      genX(emit_simpler_shader_init_compute)(state);
}

/** Allocate push constant data for a simple shader */
struct anv_state
genX(simple_shader_alloc_push)(struct anv_simple_shader *state, uint32_t size)
{
   struct anv_state s;

   if (state->kernel->stage == MESA_SHADER_FRAGMENT) {
      s = anv_state_stream_alloc(state->dynamic_state_stream,
                                 size, ANV_UBO_ALIGNMENT);
   } else {
#if GFX_VERx10 >= 125
      s = anv_state_stream_alloc(state->general_state_stream, align(size, 64), 64);
#else
      s = anv_state_stream_alloc(state->dynamic_state_stream, size, 64);
#endif
   }

   if (s.map == NULL)
      anv_batch_set_error(state->batch, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   return s;
}

/** Get the address of allocated push constant data by
 *  genX(simple_shader_alloc_push)
 */
struct anv_address
genX(simple_shader_push_state_address)(struct anv_simple_shader *state,
                                       struct anv_state push_state)
{
   if (state->kernel->stage == MESA_SHADER_FRAGMENT) {
      return anv_state_pool_state_address(
         &state->device->dynamic_state_pool, push_state);
   } else {
#if GFX_VERx10 >= 125
      return anv_state_pool_state_address(
         &state->device->general_state_pool, push_state);
#else
      return anv_state_pool_state_address(
         &state->device->dynamic_state_pool, push_state);
#endif
   }
}

/** Emit a simple shader dispatch */
void
genX(emit_simple_shader_dispatch)(struct anv_simple_shader *state,
                                  uint32_t num_threads,
                                  struct anv_state push_state)
{
   struct anv_device *device = state->device;
   struct anv_batch *batch = state->batch;
   struct anv_address push_addr =
      anv_state_pool_state_address(&device->dynamic_state_pool, push_state);

   if (state->kernel->stage == MESA_SHADER_FRAGMENT) {
      /* At the moment we require a command buffer associated with this
       * emission as we need to allocate binding tables on Gfx9.
       */
      assert(state->cmd_buffer != NULL);

      struct anv_state vs_data_state =
         anv_state_stream_alloc(state->dynamic_state_stream,
                                9 * sizeof(uint32_t), 32);
      if (vs_data_state.map == NULL)
         return;

      float x0 = 0.0f, x1 = MIN2(num_threads, 8192);
      float y0 = 0.0f, y1 = DIV_ROUND_UP(num_threads, 8192);
      float z = 0.0f;

      float *vertices = vs_data_state.map;
      vertices[0] = x1; vertices[1] = y1; vertices[2] = z; /* v0 */
      vertices[3] = x0; vertices[4] = y1; vertices[5] = z; /* v1 */
      vertices[6] = x0; vertices[7] = y0; vertices[8] = z; /* v2 */

      uint32_t *dw = anv_batch_emitn(batch,
                                     1 + GENX(VERTEX_BUFFER_STATE_length),
                                     GENX(3DSTATE_VERTEX_BUFFERS));
      GENX(VERTEX_BUFFER_STATE_pack)(batch, dw + 1,
                                     &(struct GENX(VERTEX_BUFFER_STATE)) {
                                        .VertexBufferIndex     = 0,
                                        .AddressModifyEnable   = true,
                                        .BufferStartingAddress = (struct anv_address) {
                                           .bo = device->dynamic_state_pool.block_pool.bo,
                                           .offset = vs_data_state.offset,
                                        },
                                        .BufferPitch           = 3 * sizeof(float),
                                        .BufferSize            = 9 * sizeof(float),
                                        .MOCS                  = anv_mocs(device, NULL, 0),
#if GFX_VER >= 12
                                        .L3BypassDisable       = true,
#endif
                                     });

#if GFX_VERx10 > 120
      dw =
         anv_batch_emitn(batch,
                         GENX(3DSTATE_CONSTANT_ALL_length) +
                         GENX(3DSTATE_CONSTANT_ALL_DATA_length),
                         GENX(3DSTATE_CONSTANT_ALL),
                         .ShaderUpdateEnable = BITFIELD_BIT(MESA_SHADER_FRAGMENT),
                         .PointerBufferMask = 0x1,
                         .MOCS = anv_mocs(device, NULL, 0));

      GENX(3DSTATE_CONSTANT_ALL_DATA_pack)(
         batch, dw + GENX(3DSTATE_CONSTANT_ALL_length),
         &(struct GENX(3DSTATE_CONSTANT_ALL_DATA)) {
            .PointerToConstantBuffer = push_addr,
            .ConstantBufferReadLength = DIV_ROUND_UP(push_state.alloc_size, 32),
         });
#else
      /* The Skylake PRM contains the following restriction:
       *
       *    "The driver must ensure The following case does not occur
       *     without a flush to the 3D engine: 3DSTATE_CONSTANT_* with
       *     buffer 3 read length equal to zero committed followed by a
       *     3DSTATE_CONSTANT_* with buffer 0 read length not equal to
       *     zero committed."
       *
       * To avoid this, we program the highest slot.
       */
      anv_batch_emit(batch, GENX(3DSTATE_CONSTANT_PS), c) {
         c.MOCS = anv_mocs(device, NULL, 0);
         c.ConstantBody.ReadLength[3] = DIV_ROUND_UP(push_state.alloc_size, 32);
         c.ConstantBody.Buffer[3] = push_addr;
      }
#endif

#if GFX_VER == 9
      /* Why are the push constants not flushed without a binding table
       * update??
       */
      anv_batch_emit(batch, GENX(3DSTATE_BINDING_TABLE_POINTERS_PS), btp) {
         btp.PointertoPSBindingTable = state->bt_state.offset;
      }
#endif

      genX(emit_breakpoint)(batch, device, true);
      anv_batch_emit(batch, GENX(3DPRIMITIVE), prim) {
         prim.VertexAccessType         = SEQUENTIAL;
         prim.PrimitiveTopologyType    = _3DPRIM_RECTLIST;
         prim.VertexCountPerInstance   = 3;
         prim.InstanceCount            = 1;
      }
      genX(batch_emit_post_3dprimitive_was)(batch, device, _3DPRIM_RECTLIST, 3);
      genX(emit_breakpoint)(batch, device, false);
   } else {
      const struct intel_device_info *devinfo = device->info;
      const struct brw_cs_prog_data *prog_data =
         (const struct brw_cs_prog_data *) state->kernel->prog_data;
      const struct brw_cs_dispatch_info dispatch =
         brw_cs_get_dispatch_info(devinfo, prog_data, NULL);

#if GFX_VERx10 >= 125
      anv_batch_emit(batch, GENX(COMPUTE_WALKER), cw) {
         cw.SIMDSize                       = dispatch.simd_size / 16;
         cw.MessageSIMD                    = dispatch.simd_size / 16,
         cw.IndirectDataStartAddress       = push_state.offset;
         cw.IndirectDataLength             = push_state.alloc_size;
         cw.LocalXMaximum                  = prog_data->local_size[0] - 1;
         cw.LocalYMaximum                  = prog_data->local_size[1] - 1;
         cw.LocalZMaximum                  = prog_data->local_size[2] - 1;
         cw.ThreadGroupIDXDimension        = DIV_ROUND_UP(num_threads,
                                                          dispatch.simd_size);
         cw.ThreadGroupIDYDimension        = 1;
         cw.ThreadGroupIDZDimension        = 1;
         cw.ExecutionMask                  = dispatch.right_mask;
         cw.PostSync.MOCS                  = anv_mocs(device, NULL, 0);

         cw.InterfaceDescriptor = (struct GENX(INTERFACE_DESCRIPTOR_DATA)) {
            .KernelStartPointer                = state->kernel->kernel.offset +
                                                 brw_cs_prog_data_prog_offset(prog_data,
                                                                              dispatch.simd_size),
            .SamplerStatePointer               = 0,
            .BindingTablePointer               = 0,
            .BindingTableEntryCount            = 0,
            .NumberofThreadsinGPGPUThreadGroup = dispatch.threads,
            .SharedLocalMemorySize             = encode_slm_size(GFX_VER,
                                                                 prog_data->base.total_shared),
            .NumberOfBarriers                  = prog_data->uses_barrier,
         };
      }
#else
      const uint32_t vfe_curbe_allocation =
         ALIGN(prog_data->push.per_thread.regs * dispatch.threads +
               prog_data->push.cross_thread.regs, 2);

      /* From the Sky Lake PRM Vol 2a, MEDIA_VFE_STATE:
       *
       *    "A stalling PIPE_CONTROL is required before MEDIA_VFE_STATE unless
       *     the only bits that are changed are scoreboard related: Scoreboard
       *     Enable, Scoreboard Type, Scoreboard Mask, Scoreboard * Delta. For
       *     these scoreboard related states, a MEDIA_STATE_FLUSH is
       *     sufficient."
       */
      enum anv_pipe_bits emitted_bits = 0;
      genX(emit_apply_pipe_flushes)(batch, device, GPGPU, ANV_PIPE_CS_STALL_BIT,
                                    &emitted_bits);

      /* If we have a command buffer allocated with the emission, update the
       * pending bits.
       */
      if (state->cmd_buffer)
         anv_cmd_buffer_update_pending_query_bits(state->cmd_buffer, emitted_bits);

      anv_batch_emit(batch, GENX(MEDIA_VFE_STATE), vfe) {
         vfe.StackSize              = 0;
         vfe.MaximumNumberofThreads =
            devinfo->max_cs_threads * devinfo->subslice_total - 1;
         vfe.NumberofURBEntries     = 2;
#if GFX_VER < 11
         vfe.ResetGatewayTimer      = true;
#endif
         vfe.URBEntryAllocationSize = 2;
         vfe.CURBEAllocationSize    = vfe_curbe_allocation;

         if (prog_data->base.total_scratch) {
            /* Broadwell's Per Thread Scratch Space is in the range [0, 11]
             * where 0 = 1k, 1 = 2k, 2 = 4k, ..., 11 = 2M.
             */
            vfe.PerThreadScratchSpace =
               ffs(prog_data->base.total_scratch) - 11;
            vfe.ScratchSpaceBasePointer =
               (struct anv_address) {
               .bo = anv_scratch_pool_alloc(device,
                                            &device->scratch_pool,
                                            MESA_SHADER_COMPUTE,
                                            prog_data->base.total_scratch),
               .offset = 0,
            };
         }
      }
      struct anv_state iface_desc_state =
         anv_state_stream_alloc(state->dynamic_state_stream,
                                GENX(INTERFACE_DESCRIPTOR_DATA_length) * 4, 64);
      if (iface_desc_state.map == NULL)
         return;

      struct GENX(INTERFACE_DESCRIPTOR_DATA) iface_desc = {
         .KernelStartPointer                    = state->kernel->kernel.offset +
                                                  brw_cs_prog_data_prog_offset(prog_data,
                                                                               dispatch.simd_size),

         .SamplerCount                          = 0,
         .BindingTableEntryCount                = 0,
         .BarrierEnable                         = prog_data->uses_barrier,
         .SharedLocalMemorySize                 = encode_slm_size(GFX_VER,
                                                                  prog_data->base.total_shared),

         .ConstantURBEntryReadOffset            = 0,
         .ConstantURBEntryReadLength            = prog_data->push.per_thread.regs,
         .CrossThreadConstantDataReadLength     = prog_data->push.cross_thread.regs,
#if GFX_VER >= 12
         /* TODO: Check if we are missing workarounds and enable mid-thread
          * preemption.
          *
          * We still have issues with mid-thread preemption (it was already
          * disabled by the kernel on gfx11, due to missing workarounds). It's
          * possible that we are just missing some workarounds, and could
          * enable it later, but for now let's disable it to fix a GPU in
          * compute in Car Chase (and possibly more).
          */
         .ThreadPreemptionDisable               = true,
#endif
         .NumberofThreadsinGPGPUThreadGroup     = dispatch.threads,
      };
      GENX(INTERFACE_DESCRIPTOR_DATA_pack)(batch, iface_desc_state.map, &iface_desc);
      anv_batch_emit(batch, GENX(MEDIA_INTERFACE_DESCRIPTOR_LOAD), mid) {
         mid.InterfaceDescriptorTotalLength        = iface_desc_state.alloc_size;
         mid.InterfaceDescriptorDataStartAddress   = iface_desc_state.offset;
      }
      anv_batch_emit(batch, GENX(MEDIA_CURBE_LOAD), curbe) {
         curbe.CURBEDataStartAddress = push_state.offset;
         curbe.CURBETotalDataLength  = push_state.alloc_size;
      }
      anv_batch_emit(batch, GENX(GPGPU_WALKER), ggw) {
         ggw.SIMDSize                     = dispatch.simd_size / 16;
         ggw.ThreadDepthCounterMaximum    = 0;
         ggw.ThreadHeightCounterMaximum   = 0;
         ggw.ThreadWidthCounterMaximum    = dispatch.threads - 1;
         ggw.ThreadGroupIDXDimension      = DIV_ROUND_UP(num_threads,
                                                         dispatch.simd_size);
         ggw.ThreadGroupIDYDimension      = 1;
         ggw.ThreadGroupIDZDimension      = 1;
         ggw.RightExecutionMask           = dispatch.right_mask;
         ggw.BottomExecutionMask          = 0xffffffff;
      }
#endif
   }
}

void
genX(emit_simple_shader_end)(struct anv_simple_shader *state)
{
   anv_batch_emit(state->batch, GENX(MI_BATCH_BUFFER_END), end);

   if ((state->batch->next - state->batch->start) & 4)
      anv_batch_emit(state->batch, GENX(MI_NOOP), noop);
}
