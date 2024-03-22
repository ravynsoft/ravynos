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

static inline struct blorp_address
dynamic_state_address(struct blorp_batch *blorp_batch, uint32_t offset)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;

   return (struct blorp_address) {
      .buffer = batch->state.bo,
      .offset = offset,
   };

}

static inline struct blorp_address
instruction_state_address(struct blorp_batch *blorp_batch, uint32_t offset)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;

   return (struct blorp_address) {
      .buffer = batch->ice->shaders.cache_bo,
      .offset = offset,
   };
}

static struct blorp_address
blorp_emit_vs_state(struct blorp_batch *blorp_batch)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;

   uint32_t offset;
   blorp_emit_dynamic(blorp_batch, GENX(VS_STATE), vs, 64, &offset) {
      vs.Enable = false;
      vs.URBEntryAllocationSize = batch->ice->urb.vsize - 1;
#if GFX_VER == 5
      vs.NumberofURBEntries = batch->ice->urb.nr_vs_entries >> 2;
#else
      vs.NumberofURBEntries = batch->ice->urb.nr_vs_entries;
#endif
   }

   return dynamic_state_address(blorp_batch, offset);
}

static struct blorp_address
blorp_emit_sf_state(struct blorp_batch *blorp_batch,
                    const struct blorp_params *params)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;
   const struct brw_sf_prog_data *prog_data = params->sf_prog_data;

   uint32_t offset;
   blorp_emit_dynamic(blorp_batch, GENX(SF_STATE), sf, 64, &offset) {
#if GFX_VER == 4
      sf.KernelStartPointer =
         instruction_state_address(blorp_batch, params->sf_prog_kernel);
#else
      sf.KernelStartPointer = params->sf_prog_kernel;
#endif
      sf.GRFRegisterCount = DIV_ROUND_UP(prog_data->total_grf, 16) - 1;
      sf.VertexURBEntryReadLength = prog_data->urb_read_length;
      sf.VertexURBEntryReadOffset = BRW_SF_URB_ENTRY_READ_OFFSET;
      sf.DispatchGRFStartRegisterForURBData = 3;
      sf.URBEntryAllocationSize = batch->ice->urb.sfsize - 1;
      sf.NumberofURBEntries = batch->ice->urb.nr_sf_entries;

#if GFX_VER == 5
      sf.MaximumNumberofThreads = MIN2(48, batch->ice->urb.nr_sf_entries) - 1;
#else
      sf.MaximumNumberofThreads = MIN2(24, batch->ice->urb.nr_sf_entries) - 1;
#endif
      sf.ViewportTransformEnable = false;

      sf.CullMode = CULLMODE_NONE;
   }

   return dynamic_state_address(blorp_batch, offset);
}

static struct blorp_address
blorp_emit_wm_state(struct blorp_batch *blorp_batch,
                    const struct blorp_params *params)
{
   const struct brw_wm_prog_data *prog_data = params->wm_prog_data;

   uint32_t offset;
   blorp_emit_dynamic(blorp_batch, GENX(WM_STATE), wm, 64, &offset) {
      if (params->src.enabled) {
         /* Iron Lake can't do sampler prefetch */
         wm.SamplerCount = (GFX_VER != 5);
         wm.BindingTableEntryCount = 2;
         uint32_t sampler = blorp_emit_sampler_state(blorp_batch);
         wm.SamplerStatePointer = dynamic_state_address(blorp_batch, sampler);
      }

      if (prog_data) {
         wm.DispatchGRFStartRegisterForConstantSetupData0 =
            prog_data->base.dispatch_grf_start_reg;
         wm.SetupURBEntryReadLength = prog_data->num_varying_inputs * 2;
         wm.SetupURBEntryReadOffset = 0;

         wm.DepthCoefficientURBReadOffset = 1;
         wm.PixelShaderKillsPixel = prog_data->uses_kill;
         wm.ThreadDispatchEnable = true;
         wm.EarlyDepthTestEnable = true;

         wm._8PixelDispatchEnable = prog_data->dispatch_8;
         wm._16PixelDispatchEnable = prog_data->dispatch_16;
         wm._32PixelDispatchEnable = prog_data->dispatch_32;

#if GFX_VER == 4
         wm.KernelStartPointer0 =
            instruction_state_address(blorp_batch, params->wm_prog_kernel);
         wm.GRFRegisterCount0 = brw_wm_prog_data_reg_blocks(prog_data, wm, 0);
#else
         wm.KernelStartPointer0 = params->wm_prog_kernel +
                                  brw_wm_prog_data_prog_offset(prog_data, wm, 0);
         wm.KernelStartPointer1 = params->wm_prog_kernel +
                                  brw_wm_prog_data_prog_offset(prog_data, wm, 1);
         wm.KernelStartPointer2 = params->wm_prog_kernel +
                                  brw_wm_prog_data_prog_offset(prog_data, wm, 2);
         wm.GRFRegisterCount0 = brw_wm_prog_data_reg_blocks(prog_data, wm, 0);
         wm.GRFRegisterCount1 = brw_wm_prog_data_reg_blocks(prog_data, wm, 1);
         wm.GRFRegisterCount2 = brw_wm_prog_data_reg_blocks(prog_data, wm, 2);
#endif
      }

      wm.MaximumNumberofThreads =
         blorp_batch->blorp->compiler->devinfo->max_wm_threads - 1;
   }

   return dynamic_state_address(blorp_batch, offset);
}

static struct blorp_address
blorp_emit_color_calc_state(struct blorp_batch *blorp_batch)
{
   uint32_t cc_viewport = blorp_emit_cc_viewport(blorp_batch);

   uint32_t offset;
   blorp_emit_dynamic(blorp_batch, GENX(COLOR_CALC_STATE), cc, 64, &offset) {
      cc.CCViewportStatePointer = dynamic_state_address(blorp_batch, cc_viewport);
   }

   return dynamic_state_address(blorp_batch, offset);
}

static void
blorp_emit_pipeline(struct blorp_batch *blorp_batch,
                    const struct blorp_params *params)
{
   struct crocus_batch *batch = blorp_batch->driver_batch;

   emit_urb_config(blorp_batch, params, NULL);

   blorp_emit(blorp_batch, GENX(3DSTATE_PIPELINED_POINTERS), pp) {
      pp.PointertoVSState = blorp_emit_vs_state(blorp_batch);
      pp.GSEnable = false;
      pp.ClipEnable = false;
      pp.PointertoSFState = blorp_emit_sf_state(blorp_batch, params);
      pp.PointertoWMState = blorp_emit_wm_state(blorp_batch, params);
      pp.PointertoColorCalcState = blorp_emit_color_calc_state(blorp_batch);
   }

   batch->screen->vtbl.upload_urb_fence(batch);

   blorp_emit(blorp_batch, GENX(CS_URB_STATE), curb);
   blorp_emit(blorp_batch, GENX(CONSTANT_BUFFER), curb);
}
