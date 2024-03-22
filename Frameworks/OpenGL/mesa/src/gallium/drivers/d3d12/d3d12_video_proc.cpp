/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "d3d12_context.h"
#include "d3d12_screen.h"
#include "d3d12_video_proc.h"
#include "d3d12_residency.h"
#include "d3d12_util.h"
#include "d3d12_resource.h"
#include "d3d12_video_buffer.h"
#include "d3d12_format.h"

void
d3d12_video_processor_begin_frame(struct pipe_video_codec * codec,
                                struct pipe_video_buffer *target,
                                struct pipe_picture_desc *picture)
{
    struct d3d12_video_processor * pD3D12Proc = (struct d3d12_video_processor *) codec;
    debug_printf("[d3d12_video_processor] d3d12_video_processor_begin_frame - "
                "fenceValue: %d\n",
                pD3D12Proc->m_fenceValue);

    ///
    /// Wait here to make sure the next in flight resource set is empty before using it
    ///
    uint64_t fenceValueToWaitOn = static_cast<uint64_t>(std::max(static_cast<int64_t>(0l), static_cast<int64_t>(pD3D12Proc->m_fenceValue) - static_cast<int64_t>(D3D12_VIDEO_PROC_ASYNC_DEPTH) ));

    debug_printf("[d3d12_video_processor] d3d12_video_processor_begin_frame Waiting for completion of in flight resource sets with previous work with fenceValue: %" PRIu64 "\n",
                    fenceValueToWaitOn);

    ASSERTED bool wait_res = d3d12_video_processor_sync_completion(codec, fenceValueToWaitOn, OS_TIMEOUT_INFINITE);
    assert(wait_res);

    HRESULT hr = pD3D12Proc->m_spCommandList->Reset(pD3D12Proc->m_spCommandAllocators[d3d12_video_processor_pool_current_index(pD3D12Proc)].Get());
    if (FAILED(hr)) {
        debug_printf(
            "[d3d12_video_processor] resetting ID3D12GraphicsCommandList failed with HR %x\n",
            hr);
        assert(false);
    }

    // Setup process frame arguments for output/target texture.
    struct d3d12_video_buffer *pOutputVideoBuffer = (struct d3d12_video_buffer *) target;

    ID3D12Resource *pDstD3D12Res = d3d12_resource_resource(pOutputVideoBuffer->texture);    
    auto dstDesc = GetDesc(pDstD3D12Res);
    pD3D12Proc->m_OutputArguments = {
        //struct D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS args;
        {
            {
                {
                        pDstD3D12Res, // ID3D12Resource *pTexture2D;
                        0, // UINT Subresource;
                },
                {
                        NULL, // ID3D12Resource *pTexture2D;
                        0 // UINT Subresource;
                }
            },
            { 0, 0, (int) dstDesc.Width, (int) dstDesc.Height }
        },
        // struct d3d12_resource* buffer;
        pOutputVideoBuffer,
    };
    
    debug_printf("d3d12_video_processor_begin_frame: Beginning new scene with Output ID3D12Resource: %p (%d %d)\n", pDstD3D12Res, (int) dstDesc.Width, (int) dstDesc.Height);
}

void
d3d12_video_processor_end_frame(struct pipe_video_codec * codec,
                              struct pipe_video_buffer *target,
                              struct pipe_picture_desc *picture)
{
    struct d3d12_video_processor * pD3D12Proc = (struct d3d12_video_processor *) codec;
    debug_printf("[d3d12_video_processor] d3d12_video_processor_end_frame - "
                "fenceValue: %d\n",
                pD3D12Proc->m_fenceValue);

    if(pD3D12Proc->m_ProcessInputs.size() > pD3D12Proc->m_vpMaxInputStreams.MaxInputStreams) {
      debug_printf("[d3d12_video_processor] ERROR: Requested number of input surfaces (%" PRIu64 ") exceeds underlying D3D12 driver capabilities (%d)\n", (uint64_t) pD3D12Proc->m_ProcessInputs.size(), pD3D12Proc->m_vpMaxInputStreams.MaxInputStreams);
      assert(false);
    }

    auto curOutputDesc = GetOutputStreamDesc(pD3D12Proc->m_spVideoProcessor.Get());
    auto curOutputTexFmt = GetDesc(pD3D12Proc->m_OutputArguments.args.OutputStream[0].pTexture2D).Format;
    
    bool inputFmtsMatch = pD3D12Proc->m_inputStreamDescs.size() == pD3D12Proc->m_ProcessInputs.size();
    unsigned curInputIdx = 0;
    while( (curInputIdx < pD3D12Proc->m_inputStreamDescs.size()) && inputFmtsMatch)    
    {
        inputFmtsMatch = inputFmtsMatch && (pD3D12Proc->m_inputStreamDescs[curInputIdx].Format == GetDesc(pD3D12Proc->m_ProcessInputs[curInputIdx].InputStream[0].pTexture2D).Format);
        curInputIdx++;
    }

    bool inputCountMatches = (pD3D12Proc->m_ProcessInputs.size() == pD3D12Proc->m_spVideoProcessor->GetNumInputStreamDescs());
    bool outputFmtMatches = (curOutputDesc.Format == curOutputTexFmt);
    bool needsVPRecreation = (
        !inputCountMatches // Requested batch has different number of Inputs to be blit'd
        || !outputFmtMatches // output texture format different than vid proc object expects
        || !inputFmtsMatch // inputs texture formats different than vid proc object expects
    );

    if(needsVPRecreation) {
        debug_printf("[d3d12_video_processor] d3d12_video_processor_end_frame - Attempting to re-create ID3D12VideoProcessor "
                      "input count matches %d inputFmtsMatch: %d outputFmtsMatch %d \n", inputCountMatches, inputFmtsMatch, outputFmtMatches);
        
        DXGI_COLOR_SPACE_TYPE OutputColorSpace = d3d12_convert_from_legacy_color_space(
          !util_format_is_yuv(d3d12_get_pipe_format(curOutputTexFmt)),
          util_format_get_blocksize(d3d12_get_pipe_format(curOutputTexFmt)) * 8 /*bytes to bits conversion*/,
          /* StudioRGB= */ false,
          /* P709= */ true,
          /* StudioYUV= */ true);
        
        std::vector<DXGI_FORMAT> InputFormats;
        for(D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS1 curInput : pD3D12Proc->m_ProcessInputs)
        {
            InputFormats.push_back(GetDesc(curInput.InputStream[0].pTexture2D).Format);
        }
        DXGI_COLOR_SPACE_TYPE InputColorSpace = d3d12_convert_from_legacy_color_space(
          !util_format_is_yuv(d3d12_get_pipe_format(InputFormats[0])),
          util_format_get_blocksize(d3d12_get_pipe_format(InputFormats[0])) * 8 /*bytes to bits conversion*/,
          /* StudioRGB= */ false,
          /* P709= */ true,
          /* StudioYUV= */ true);

        // Release previous allocation
        pD3D12Proc->m_spVideoProcessor.Reset();
        if(!d3d12_video_processor_check_caps_and_create_processor(pD3D12Proc, InputFormats, InputColorSpace, curOutputTexFmt, OutputColorSpace))
        {
            debug_printf("[d3d12_video_processor] d3d12_video_processor_end_frame - Failure when "
                      " trying to re-create the ID3D12VideoProcessor for current batch streams configuration\n");
            assert(false);
        }      
    }

    // Schedule barrier transitions
    std::vector<D3D12_RESOURCE_BARRIER> barrier_transitions;
    barrier_transitions.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
                                pD3D12Proc->m_OutputArguments.args.OutputStream[0].pTexture2D,
                                D3D12_RESOURCE_STATE_COMMON,
                                D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE));

    for(D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS1 curInput : pD3D12Proc->m_ProcessInputs)
        barrier_transitions.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
                                    curInput.InputStream[0].pTexture2D,
                                    D3D12_RESOURCE_STATE_COMMON,
                                    D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ));
    
    pD3D12Proc->m_spCommandList->ResourceBarrier(static_cast<uint32_t>(barrier_transitions.size()), barrier_transitions.data());

    // Schedule process operation

    pD3D12Proc->m_spCommandList->ProcessFrames1(pD3D12Proc->m_spVideoProcessor.Get(), &pD3D12Proc->m_OutputArguments.args, pD3D12Proc->m_ProcessInputs.size(), pD3D12Proc->m_ProcessInputs.data());

    // Schedule reverse (back to common) transitions before command list closes for current frame

    for (auto &BarrierDesc : barrier_transitions)
        std::swap(BarrierDesc.Transition.StateBefore, BarrierDesc.Transition.StateAfter);

    pD3D12Proc->m_spCommandList->ResourceBarrier(static_cast<uint32_t>(barrier_transitions.size()), barrier_transitions.data());

    pD3D12Proc->m_PendingFences[d3d12_video_processor_pool_current_index(pD3D12Proc)].value = pD3D12Proc->m_fenceValue;
    pD3D12Proc->m_PendingFences[d3d12_video_processor_pool_current_index(pD3D12Proc)].cmdqueue_fence = pD3D12Proc->m_spFence.Get();
    *picture->fence = (pipe_fence_handle*) &pD3D12Proc->m_PendingFences[d3d12_video_processor_pool_current_index(pD3D12Proc)];
}

void
d3d12_video_processor_process_frame(struct pipe_video_codec *codec,
                        struct pipe_video_buffer *input_texture,
                        const struct pipe_vpp_desc *process_properties)
{
    struct d3d12_video_processor * pD3D12Proc = (struct d3d12_video_processor *) codec;

    // begin_frame gets only called once so wouldn't update process_properties->src_surface_fence correctly
    pD3D12Proc->input_surface_fence = (struct d3d12_fence*) process_properties->src_surface_fence;

    // Get the underlying resources from the pipe_video_buffers
    struct d3d12_video_buffer *pInputVideoBuffer = (struct d3d12_video_buffer *) input_texture;

    ID3D12Resource *pSrcD3D12Res = d3d12_resource_resource(pInputVideoBuffer->texture);

    // y0 = top
    // x0 = left
    // x1 = right
    // y1 = bottom

    debug_printf("d3d12_video_processor_process_frame: Adding Input ID3D12Resource: %p to scene (Output target %p)\n", pSrcD3D12Res, pD3D12Proc->m_OutputArguments.args.OutputStream[0].pTexture2D);
    debug_printf("d3d12_video_processor_process_frame: Input box: top: %d left: %d right: %d bottom: %d\n", process_properties->src_region.y0, process_properties->src_region.x0, process_properties->src_region.x1, process_properties->src_region.y1);
    debug_printf("d3d12_video_processor_process_frame: Output box: top: %d left: %d right: %d bottom: %d\n", process_properties->dst_region.y0, process_properties->dst_region.x0, process_properties->dst_region.x1, process_properties->dst_region.y1);
    debug_printf("d3d12_video_processor_process_frame: Requested alpha blend mode %d global alpha: %f \n", process_properties->blend.mode, process_properties->blend.global_alpha);

    // Setup process frame arguments for current input texture.

    D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS1 InputArguments = {
        {
        { // D3D12_VIDEO_PROCESS_INPUT_STREAM InputStream[0];
                pSrcD3D12Res, // ID3D12Resource *pTexture2D;
                0, // UINT Subresource
                {//D3D12_VIDEO_PROCESS_REFERENCE_SET ReferenceSet;
                    0, //UINT NumPastFrames;
                    NULL, //ID3D12Resource **ppPastFrames;
                    NULL, // UINT *pPastSubresources;
                    0, //UINT NumFutureFrames;
                    NULL, //ID3D12Resource **ppFutureFrames;
                    NULL //UINT *pFutureSubresources;
                }
        },
        { // D3D12_VIDEO_PROCESS_INPUT_STREAM InputStream[1];
                NULL, //ID3D12Resource *pTexture2D;
                0, //UINT Subresource;
                {//D3D12_VIDEO_PROCESS_REFERENCE_SET ReferenceSet;
                    0, //UINT NumPastFrames;
                    NULL, //ID3D12Resource **ppPastFrames;
                    NULL, // UINT *pPastSubresources;
                    0, //UINT NumFutureFrames;
                    NULL, //ID3D12Resource **ppFutureFrames;
                    NULL //UINT *pFutureSubresources;
                }
        }
        },
        { // D3D12_VIDEO_PROCESS_TRANSFORM Transform;
            // y0 = top
            // x0 = left
            // x1 = right
            // y1 = bottom
            // typedef struct _RECT
            // {
            //     int left;
            //     int top;
            //     int right;
            //     int bottom;
            // } RECT;
            { process_properties->src_region.x0/*left*/, process_properties->src_region.y0/*top*/, process_properties->src_region.x1/*right*/, process_properties->src_region.y1/*bottom*/ },
            { process_properties->dst_region.x0/*left*/, process_properties->dst_region.y0/*top*/, process_properties->dst_region.x1/*right*/, process_properties->dst_region.y1/*bottom*/ }, // D3D12_RECT DestinationRectangle;
            pD3D12Proc->m_inputStreamDescs[0].EnableOrientation ? d3d12_video_processor_convert_pipe_rotation(process_properties->orientation) : D3D12_VIDEO_PROCESS_ORIENTATION_DEFAULT, // D3D12_VIDEO_PROCESS_ORIENTATION Orientation;
        },
        D3D12_VIDEO_PROCESS_INPUT_STREAM_FLAG_NONE,
        { // D3D12_VIDEO_PROCESS_INPUT_STREAM_RATE RateInfo;
            0,
            0,
        },
        // INT                                    FilterLevels[32];
        {
            0, // Trailing zeroes on the rest
        },
        //D3D12_VIDEO_PROCESS_ALPHA_BLENDING;
        { 
            (process_properties->blend.mode == PIPE_VIDEO_VPP_BLEND_MODE_GLOBAL_ALPHA),
            process_properties->blend.global_alpha
        },
        // D3D12_VIDEO_FIELD_TYPE FieldType
        D3D12_VIDEO_FIELD_TYPE_NONE,
    };

    debug_printf("ProcessFrame InArgs Orientation %d \n\tSrc top: %d left: %d right: %d bottom: %d\n\tDst top: %d left: %d right: %d bottom: %d\n", InputArguments.Transform.Orientation, 
        InputArguments.Transform.SourceRectangle.top, InputArguments.Transform.SourceRectangle.left, InputArguments.Transform.SourceRectangle.right, InputArguments.Transform.SourceRectangle.bottom,
        InputArguments.Transform.DestinationRectangle.top, InputArguments.Transform.DestinationRectangle.left, InputArguments.Transform.DestinationRectangle.right, InputArguments.Transform.DestinationRectangle.bottom);

    pD3D12Proc->m_ProcessInputs.push_back(InputArguments);    
    pD3D12Proc->m_InputBuffers.push_back(pInputVideoBuffer);
    
    ///
    /// Flush work to the GPU and blocking wait until GPU finishes
    ///
    pD3D12Proc->m_needsGPUFlush = true;
}

void
d3d12_video_processor_destroy(struct pipe_video_codec * codec)
{
    if (codec == nullptr) {
        return;
    }
    // Flush pending work before destroying.
    struct d3d12_video_processor *pD3D12Proc = (struct d3d12_video_processor *) codec;

    uint64_t curBatchFence = pD3D12Proc->m_fenceValue;
    if (pD3D12Proc->m_needsGPUFlush)
    {
        d3d12_video_processor_flush(codec);
        d3d12_video_processor_sync_completion(codec, curBatchFence, OS_TIMEOUT_INFINITE);
    }

    // Call dtor to make ComPtr work
    delete pD3D12Proc;
}

void
d3d12_video_processor_flush(struct pipe_video_codec * codec)
{
    struct d3d12_video_processor * pD3D12Proc = (struct d3d12_video_processor *) codec;
    assert(pD3D12Proc);
    assert(pD3D12Proc->m_spD3D12VideoDevice);
    assert(pD3D12Proc->m_spCommandQueue);

    debug_printf("[d3d12_video_processor] d3d12_video_processor_flush started. Will flush video queue work and CPU wait on "
                    "fenceValue: %d\n",
                    pD3D12Proc->m_fenceValue);

    if (!pD3D12Proc->m_needsGPUFlush) {
        debug_printf("[d3d12_video_processor] d3d12_video_processor_flush started. Nothing to flush, all up to date.\n");
    } else {
        debug_printf("[d3d12_video_processor] d3d12_video_processor_flush - Promoting the output texture %p to d3d12_permanently_resident.\n", 
                     pD3D12Proc->m_OutputArguments.buffer->texture);

        // Make the resources permanently resident for video use
        d3d12_promote_to_permanent_residency(pD3D12Proc->m_pD3D12Screen, pD3D12Proc->m_OutputArguments.buffer->texture);

        for(auto curInput : pD3D12Proc->m_InputBuffers)
        {
            debug_printf("[d3d12_video_processor] d3d12_video_processor_flush - Promoting the input texture %p to d3d12_permanently_resident.\n", 
                         curInput->texture);
            // Make the resources permanently resident for video use
            d3d12_promote_to_permanent_residency(pD3D12Proc->m_pD3D12Screen, curInput->texture);
        }

        HRESULT hr = pD3D12Proc->m_pD3D12Screen->dev->GetDeviceRemovedReason();
        if (hr != S_OK) {
            debug_printf("[d3d12_video_processor] d3d12_video_processor_flush"
                            " - D3D12Device was removed BEFORE commandlist "
                            "execution with HR %x.\n",
                            hr);
            goto flush_fail;
        }

        // Close and execute command list and wait for idle on CPU blocking
        // this method before resetting list and allocator for next submission.

        if (pD3D12Proc->m_transitionsBeforeCloseCmdList.size() > 0) {
            pD3D12Proc->m_spCommandList->ResourceBarrier(pD3D12Proc->m_transitionsBeforeCloseCmdList.size(),
                                                            pD3D12Proc->m_transitionsBeforeCloseCmdList.data());
            pD3D12Proc->m_transitionsBeforeCloseCmdList.clear();
        }

        hr = pD3D12Proc->m_spCommandList->Close();
        if (FAILED(hr)) {
            debug_printf("[d3d12_video_processor] d3d12_video_processor_flush - Can't close command list with HR %x\n", hr);
            goto flush_fail;
        }

        // Flush any work batched in the d3d12_screen and Wait on the m_spCommandQueue
        struct pipe_fence_handle *completion_fence = NULL;
        pD3D12Proc->base.context->flush(pD3D12Proc->base.context, &completion_fence, PIPE_FLUSH_ASYNC | PIPE_FLUSH_HINT_FINISH);
        struct d3d12_fence *casted_completion_fence = d3d12_fence(completion_fence);
        pD3D12Proc->m_spCommandQueue->Wait(casted_completion_fence->cmdqueue_fence, casted_completion_fence->value);
        pD3D12Proc->m_pD3D12Screen->base.fence_reference(&pD3D12Proc->m_pD3D12Screen->base, &completion_fence, NULL);

        struct d3d12_fence *input_surface_fence = pD3D12Proc->input_surface_fence;
        if (input_surface_fence)
            pD3D12Proc->m_spCommandQueue->Wait(input_surface_fence->cmdqueue_fence, input_surface_fence->value);

        ID3D12CommandList *ppCommandLists[1] = { pD3D12Proc->m_spCommandList.Get() };
        pD3D12Proc->m_spCommandQueue->ExecuteCommandLists(1, ppCommandLists);
        pD3D12Proc->m_spCommandQueue->Signal(pD3D12Proc->m_spFence.Get(), pD3D12Proc->m_fenceValue);

        // Validate device was not removed
        hr = pD3D12Proc->m_pD3D12Screen->dev->GetDeviceRemovedReason();
        if (hr != S_OK) {
            debug_printf("[d3d12_video_processor] d3d12_video_processor_flush"
                            " - D3D12Device was removed AFTER commandlist "
                            "execution with HR %x, but wasn't before.\n",
                            hr);
            goto flush_fail;
        }

        debug_printf(
            "[d3d12_video_processor] d3d12_video_processor_flush - GPU signaled execution finalized for fenceValue: %d\n",
            pD3D12Proc->m_fenceValue);

        pD3D12Proc->m_fenceValue++;
        pD3D12Proc->m_needsGPUFlush = false;
    }
    pD3D12Proc->m_ProcessInputs.clear();
    pD3D12Proc->m_InputBuffers.clear();
    // Free the fence after completion finished

    return;

flush_fail:
    debug_printf("[d3d12_video_processor] d3d12_video_processor_flush failed for fenceValue: %d\n", pD3D12Proc->m_fenceValue);
    assert(false);
}

struct pipe_video_codec *
d3d12_video_processor_create(struct pipe_context *context, const struct pipe_video_codec *codec)
{
   ///
   /// Initialize d3d12_video_processor
   ///

   // Not using new doesn't call ctor and the initializations in the class declaration are lost
   struct d3d12_video_processor *pD3D12Proc = new d3d12_video_processor;

   pD3D12Proc->m_PendingFences.resize(D3D12_VIDEO_PROC_ASYNC_DEPTH);
   pD3D12Proc->base = *codec;

   pD3D12Proc->base.context = context;
   pD3D12Proc->base.width = codec->width;
   pD3D12Proc->base.height = codec->height;
   pD3D12Proc->base.destroy = d3d12_video_processor_destroy;
   pD3D12Proc->base.begin_frame = d3d12_video_processor_begin_frame;
   pD3D12Proc->base.process_frame = d3d12_video_processor_process_frame;
   pD3D12Proc->base.end_frame = d3d12_video_processor_end_frame;
   pD3D12Proc->base.flush = d3d12_video_processor_flush;
   pD3D12Proc->base.get_processor_fence = d3d12_video_processor_get_processor_fence;

   ///

   ///
   /// Try initializing D3D12 Video device and check for device caps
   ///

   struct d3d12_context *pD3D12Ctx = (struct d3d12_context *) context;
   pD3D12Proc->m_pD3D12Context = pD3D12Ctx;
   pD3D12Proc->m_pD3D12Screen = d3d12_screen(pD3D12Ctx->base.screen);

    // Assume defaults for now, can re-create if necessary when d3d12_video_processor_end_frame kicks off the processing
    DXGI_COLOR_SPACE_TYPE InputColorSpace = DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709;
    std::vector<DXGI_FORMAT> InputFormats = { DXGI_FORMAT_NV12 };
    DXGI_FORMAT OutputFormat = DXGI_FORMAT_NV12;
    DXGI_COLOR_SPACE_TYPE OutputColorSpace = DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709;

   ///
   /// Create processor objects
   ///
   if (FAILED(pD3D12Proc->m_pD3D12Screen->dev->QueryInterface(
          IID_PPV_ARGS(pD3D12Proc->m_spD3D12VideoDevice.GetAddressOf())))) {
      debug_printf("[d3d12_video_processor] d3d12_video_create_processor - D3D12 Device has no Video support\n");
      goto failed;
   }

   if (FAILED(pD3D12Proc->m_spD3D12VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_PROCESS_MAX_INPUT_STREAMS, &pD3D12Proc->m_vpMaxInputStreams, sizeof(pD3D12Proc->m_vpMaxInputStreams)))) {
      debug_printf("[d3d12_video_processor] d3d12_video_create_processor - Failed to query D3D12_FEATURE_VIDEO_PROCESS_MAX_INPUT_STREAMS\n");
      goto failed;
   }

   if (!d3d12_video_processor_check_caps_and_create_processor(pD3D12Proc, InputFormats, InputColorSpace, OutputFormat, OutputColorSpace)) {
      debug_printf("[d3d12_video_processor] d3d12_video_create_processor - Failure on "
                      "d3d12_video_processor_check_caps_and_create_processor\n");
      goto failed;
   }

   if (!d3d12_video_processor_create_command_objects(pD3D12Proc)) {
      debug_printf(
         "[d3d12_video_processor] d3d12_video_create_processor - Failure on d3d12_video_processor_create_command_objects\n");
      goto failed;
   }

    debug_printf("[d3d12_video_processor] d3d12_video_create_processor - Created successfully!\n");

   return &pD3D12Proc->base;

failed:
   if (pD3D12Proc != nullptr) {
      d3d12_video_processor_destroy(&pD3D12Proc->base);
   }

   return nullptr;
}

bool
d3d12_video_processor_check_caps_and_create_processor(struct d3d12_video_processor *pD3D12Proc,
                                                        std::vector<DXGI_FORMAT> InputFormats,
                                                        DXGI_COLOR_SPACE_TYPE InputColorSpace,
                                                        DXGI_FORMAT OutputFormat,
                                                        DXGI_COLOR_SPACE_TYPE OutputColorSpace)
{
    HRESULT hr = S_OK;

    D3D12_VIDEO_FIELD_TYPE FieldType = D3D12_VIDEO_FIELD_TYPE_NONE;
    D3D12_VIDEO_FRAME_STEREO_FORMAT StereoFormat = D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE;
    DXGI_RATIONAL FrameRate = { 30, 1 };
    DXGI_RATIONAL AspectRatio = { 1, 1 };

    struct ResolStruct {
        uint Width;
        uint Height;
    };

    ResolStruct resolutionsList[] = {
        { 8192, 8192 },   // 8k
        { 8192, 4320 },   // 8k - alternative
        { 7680, 4800 },   // 8k - alternative
        { 7680, 4320 },   // 8k - alternative
        { 4096, 2304 },   // 2160p (4K)
        { 4096, 2160 },   // 2160p (4K) - alternative
        { 2560, 1440 },   // 1440p
        { 1920, 1200 },   // 1200p
        { 1920, 1080 },   // 1080p
        { 1280, 720 },    // 720p
        { 800, 600 },
    };

    pD3D12Proc->m_SupportCaps = 
    {
        0, // NodeIndex
        { resolutionsList[0].Width, resolutionsList[0].Height, { InputFormats[0], InputColorSpace } },
        FieldType,
        StereoFormat,
        FrameRate,
        { OutputFormat, OutputColorSpace },
        StereoFormat,
        FrameRate,
    };

    uint32_t idxResol = 0;
    bool bSupportsAny = false;
    while ((idxResol < ARRAY_SIZE(resolutionsList)) && !bSupportsAny) {
        pD3D12Proc->m_SupportCaps.InputSample.Width = resolutionsList[idxResol].Width;
        pD3D12Proc->m_SupportCaps.InputSample.Height = resolutionsList[idxResol].Height;
        if (SUCCEEDED(pD3D12Proc->m_spD3D12VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_PROCESS_SUPPORT, &pD3D12Proc->m_SupportCaps, sizeof(pD3D12Proc->m_SupportCaps)))) {
            bSupportsAny = ((pD3D12Proc->m_SupportCaps.SupportFlags & D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED) != 0);
        }
        idxResol++;
    }

    if ((pD3D12Proc->m_SupportCaps.SupportFlags & D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED) != D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED)
    {
        if((pD3D12Proc->m_SupportCaps.SupportFlags & D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED) != D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED) {
        debug_printf("[d3d12_video_processor] d3d12_video_processor_check_caps_and_create_processor - D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED not returned by driver. "
                            "failed with SupportFlags %x\n",
                            pD3D12Proc->m_SupportCaps.SupportFlags);
        }
    }

    D3D12_VIDEO_PROCESS_FILTER_FLAGS enabledFilterFlags = D3D12_VIDEO_PROCESS_FILTER_FLAG_NONE;

    bool enableOrientation = (
        ((pD3D12Proc->m_SupportCaps.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_ROTATION) != 0)
        || ((pD3D12Proc->m_SupportCaps.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_FLIP) != 0)
    );

    D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC inputStreamDesc = {
        InputFormats[0],
        InputColorSpace,
        AspectRatio,                            // SourceAspectRatio;
        AspectRatio,                            // DestinationAspectRatio;
        FrameRate,                              // FrameRate
        pD3D12Proc->m_SupportCaps.ScaleSupport.OutputSizeRange, // SourceSizeRange
        pD3D12Proc->m_SupportCaps.ScaleSupport.OutputSizeRange, // DestinationSizeRange
        enableOrientation,
        enabledFilterFlags,
        StereoFormat,
        FieldType,
        D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_NONE,
        ((pD3D12Proc->m_SupportCaps.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_BLENDING) != 0)
        && ((pD3D12Proc->m_SupportCaps.FeatureSupport & D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_FILL) != 0), // EnableAlphaBlending
        {},                                     // LumaKey
        0,                                      // NumPastFrames
        0,                                      // NumFutureFrames
        false                                   // EnableAutoProcessing
    };

    D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC outputStreamDesc =
    {
        pD3D12Proc->m_SupportCaps.OutputFormat.Format,
        OutputColorSpace,
        D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_OPAQUE, // AlphaFillMode
        0u,                                         // AlphaFillModeSourceStreamIndex
        {0, 0, 0, 0},                               // BackgroundColor
        FrameRate,                                  // FrameRate
        false                                       // EnableStereo
    };
    
    // gets the required past/future frames for VP creation
    {
        D3D12_FEATURE_DATA_VIDEO_PROCESS_REFERENCE_INFO referenceInfo = {};
        referenceInfo.NodeIndex = 0;
        D3D12_VIDEO_PROCESS_FEATURE_FLAGS featureFlags = D3D12_VIDEO_PROCESS_FEATURE_FLAG_NONE;
        featureFlags |= outputStreamDesc.AlphaFillMode ? D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_FILL : D3D12_VIDEO_PROCESS_FEATURE_FLAG_NONE;
        featureFlags |= inputStreamDesc.LumaKey.Enable ? D3D12_VIDEO_PROCESS_FEATURE_FLAG_LUMA_KEY : D3D12_VIDEO_PROCESS_FEATURE_FLAG_NONE;
        featureFlags |= (inputStreamDesc.StereoFormat != D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE || outputStreamDesc.EnableStereo) ? D3D12_VIDEO_PROCESS_FEATURE_FLAG_STEREO : D3D12_VIDEO_PROCESS_FEATURE_FLAG_NONE;
        featureFlags |= inputStreamDesc.EnableOrientation ? D3D12_VIDEO_PROCESS_FEATURE_FLAG_ROTATION | D3D12_VIDEO_PROCESS_FEATURE_FLAG_FLIP : D3D12_VIDEO_PROCESS_FEATURE_FLAG_NONE;
        featureFlags |= inputStreamDesc.EnableAlphaBlending ? D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_BLENDING : D3D12_VIDEO_PROCESS_FEATURE_FLAG_NONE;

        referenceInfo.DeinterlaceMode = inputStreamDesc.DeinterlaceMode;
        referenceInfo.Filters = inputStreamDesc.FilterFlags;
        referenceInfo.FeatureSupport = featureFlags;
        referenceInfo.InputFrameRate = inputStreamDesc.FrameRate;
        referenceInfo.OutputFrameRate = outputStreamDesc.FrameRate;
        referenceInfo.EnableAutoProcessing = inputStreamDesc.EnableAutoProcessing;

        hr = pD3D12Proc->m_spD3D12VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_PROCESS_REFERENCE_INFO, &referenceInfo, sizeof(referenceInfo));
        if (FAILED(hr)) {
        debug_printf("[d3d12_video_processor] d3d12_video_processor_check_caps_and_create_processor - CheckFeatureSupport "
                        "failed with HR %x\n",
                        hr);
        return false;
        }

        inputStreamDesc.NumPastFrames = referenceInfo.PastFrames;
        inputStreamDesc.NumFutureFrames = referenceInfo.FutureFrames;
    }

    pD3D12Proc->m_outputStreamDesc = outputStreamDesc;
    
    debug_printf("[d3d12_video_processor]\t Creating Video Processor\n");
    debug_printf("[d3d12_video_processor]\t NumInputs: %d\n", (int) InputFormats.size());

    pD3D12Proc->m_inputStreamDescs.clear();
    for (unsigned i = 0; i < InputFormats.size(); i++)
    {
        inputStreamDesc.Format = InputFormats[i];
        pD3D12Proc->m_inputStreamDescs.push_back(inputStreamDesc);
        debug_printf("[d3d12_video_processor]\t Input Stream #%d Format: %d\n", i, inputStreamDesc.Format);
    }
    debug_printf("[d3d12_video_processor]\t Output Stream Format: %d\n", pD3D12Proc->m_outputStreamDesc.Format);

    hr = pD3D12Proc->m_spD3D12VideoDevice->CreateVideoProcessor(pD3D12Proc->m_NodeMask,
                                                            &pD3D12Proc->m_outputStreamDesc,
                                                            pD3D12Proc->m_inputStreamDescs.size(),
                                                            pD3D12Proc->m_inputStreamDescs.data(),
                                                            IID_PPV_ARGS(pD3D12Proc->m_spVideoProcessor.GetAddressOf()));
    if (FAILED(hr)) {
        debug_printf("[d3d12_video_processor] d3d12_video_processor_check_caps_and_create_processor - CreateVideoProcessor "
                    "failed with HR %x\n",
                    hr);
        return false;
    }

   return true;
}

bool
d3d12_video_processor_create_command_objects(struct d3d12_video_processor *pD3D12Proc)
{
    assert(pD3D12Proc->m_spD3D12VideoDevice);

    D3D12_COMMAND_QUEUE_DESC commandQueueDesc = { D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS };
    HRESULT hr = pD3D12Proc->m_pD3D12Screen->dev->CreateCommandQueue(
                &commandQueueDesc,
                IID_PPV_ARGS(pD3D12Proc->m_spCommandQueue.GetAddressOf()));

    if (FAILED(hr)) {
        debug_printf("[d3d12_video_processor] d3d12_video_processor_create_command_objects - Call to CreateCommandQueue "
                        "failed with HR %x\n",
                        hr);
        return false;
    }

    hr = pD3D12Proc->m_pD3D12Screen->dev->CreateFence(0,
         D3D12_FENCE_FLAG_SHARED,
         IID_PPV_ARGS(&pD3D12Proc->m_spFence));

    if (FAILED(hr)) {
        debug_printf(
            "[d3d12_video_processor] d3d12_video_processor_create_command_objects - Call to CreateFence failed with HR %x\n",
            hr);
        return false;
    }

    pD3D12Proc->m_spCommandAllocators.resize(D3D12_VIDEO_PROC_ASYNC_DEPTH);
    for (uint32_t i = 0; i < pD3D12Proc->m_spCommandAllocators.size() ; i++) {
        hr = pD3D12Proc->m_pD3D12Screen->dev->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS,
            IID_PPV_ARGS(pD3D12Proc->m_spCommandAllocators[i].GetAddressOf()));

        if (FAILED(hr)) {
            debug_printf("[d3d12_video_processor] d3d12_video_processor_create_command_objects - Call to "
                            "CreateCommandAllocator failed with HR %x\n",
                            hr);
            return false;
        }
    }

    ComPtr<ID3D12Device4> spD3D12Device4;
    if (FAILED(pD3D12Proc->m_pD3D12Screen->dev->QueryInterface(
            IID_PPV_ARGS(spD3D12Device4.GetAddressOf())))) {
        debug_printf(
            "[d3d12_video_processor] d3d12_video_processor_create_processor - D3D12 Device has no ID3D12Device4 support\n");
        return false;
    }

    hr = spD3D12Device4->CreateCommandList1(0,
                                            D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS,
                                            D3D12_COMMAND_LIST_FLAG_NONE,
                                            IID_PPV_ARGS(pD3D12Proc->m_spCommandList.GetAddressOf()));

    if (FAILED(hr)) {
        debug_printf("[d3d12_video_processor] d3d12_video_processor_create_command_objects - Call to CreateCommandList "
                        "failed with HR %x\n",
                        hr);
        return false;
    }

    return true;
}

D3D12_VIDEO_PROCESS_ORIENTATION
d3d12_video_processor_convert_pipe_rotation(enum pipe_video_vpp_orientation orientation_flags)
{
    D3D12_VIDEO_PROCESS_ORIENTATION result = D3D12_VIDEO_PROCESS_ORIENTATION_DEFAULT;

    if(orientation_flags & PIPE_VIDEO_VPP_ROTATION_90)
    {
        result = (orientation_flags & PIPE_VIDEO_VPP_FLIP_HORIZONTAL) ? D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_90_FLIP_HORIZONTAL : D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_90;
        debug_printf("d3d12_video_processor_process_frame: Orientation Mode: %s\n", (orientation_flags & PIPE_VIDEO_VPP_FLIP_HORIZONTAL) ? "D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_90_FLIP_HORIZONTAL" : "D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_90");
    }
    else if(orientation_flags & PIPE_VIDEO_VPP_ROTATION_180)
    {
        result = D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_180;
        debug_printf("d3d12_video_processor_process_frame: Orientation Mode: D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_180\n");
    }
    else if(orientation_flags & PIPE_VIDEO_VPP_ROTATION_270)
    {
        result = (orientation_flags & PIPE_VIDEO_VPP_FLIP_HORIZONTAL) ? D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_270_FLIP_HORIZONTAL : D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_270;
        debug_printf("d3d12_video_processor_process_frame: Orientation Mode: %s\n", (orientation_flags & PIPE_VIDEO_VPP_FLIP_HORIZONTAL) ? "D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_270_FLIP_HORIZONTAL" : "D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_270");
    }
    else if(orientation_flags & PIPE_VIDEO_VPP_FLIP_HORIZONTAL)
    {
        result = D3D12_VIDEO_PROCESS_ORIENTATION_FLIP_HORIZONTAL;
        debug_printf("d3d12_video_processor_process_frame: Orientation Mode: D3D12_VIDEO_PROCESS_ORIENTATION_FLIP_HORIZONTAL\n");
    }
    else if(orientation_flags & PIPE_VIDEO_VPP_FLIP_VERTICAL)
    {
        result = D3D12_VIDEO_PROCESS_ORIENTATION_FLIP_VERTICAL;
        debug_printf("d3d12_video_processor_process_frame: Orientation Mode: D3D12_VIDEO_PROCESS_ORIENTATION_FLIP_VERTICAL\n");
    }

    return result;
}

uint64_t
d3d12_video_processor_pool_current_index(struct d3d12_video_processor *pD3D12Proc)
{
   return pD3D12Proc->m_fenceValue % D3D12_VIDEO_PROC_ASYNC_DEPTH;
}


bool
d3d12_video_processor_ensure_fence_finished(struct pipe_video_codec *codec,
                                          uint64_t fenceValueToWaitOn,
                                          uint64_t timeout_ns)
{
   bool wait_result = true;
   struct d3d12_video_processor *pD3D12Proc = (struct d3d12_video_processor *) codec;
   HRESULT hr = S_OK;
   uint64_t completedValue = pD3D12Proc->m_spFence->GetCompletedValue();

   debug_printf(
      "[d3d12_video_processor] d3d12_video_processor_ensure_fence_finished - Waiting for fence (with timeout_ns %" PRIu64
      ") to finish with "
      "fenceValue: %" PRIu64 " - Current Fence Completed Value %" PRIu64 "\n",
      timeout_ns,
      fenceValueToWaitOn,
      completedValue);

   if (completedValue < fenceValueToWaitOn) {

      HANDLE event = {};
      int event_fd = 0;
      event = d3d12_fence_create_event(&event_fd);

      hr = pD3D12Proc->m_spFence->SetEventOnCompletion(fenceValueToWaitOn, event);
      if (FAILED(hr)) {
         debug_printf("[d3d12_video_processor] d3d12_video_processor_ensure_fence_finished - SetEventOnCompletion for "
                      "fenceValue %" PRIu64 " failed with HR %x\n",
                      fenceValueToWaitOn,
                      hr);
         goto ensure_fence_finished_fail;
      }

      wait_result = d3d12_fence_wait_event(event, event_fd, timeout_ns);
      d3d12_fence_close_event(event, event_fd);

      debug_printf("[d3d12_video_processor] d3d12_video_processor_ensure_fence_finished - Waiting on fence to be done with "
                   "fenceValue: %" PRIu64 " - current CompletedValue: %" PRIu64 "\n",
                   fenceValueToWaitOn,
                   completedValue);
   } else {
      debug_printf("[d3d12_video_processor] d3d12_video_processor_ensure_fence_finished - Fence already done with "
                   "fenceValue: %" PRIu64 " - current CompletedValue: %" PRIu64 "\n",
                   fenceValueToWaitOn,
                   completedValue);
   }
   return wait_result;

ensure_fence_finished_fail:
   debug_printf("[d3d12_video_processor] d3d12_video_processor_sync_completion failed for fenceValue: %" PRIu64 "\n",
                fenceValueToWaitOn);
   assert(false);
   return false;
}

bool
d3d12_video_processor_sync_completion(struct pipe_video_codec *codec, uint64_t fenceValueToWaitOn, uint64_t timeout_ns)
{
   struct d3d12_video_processor *pD3D12Proc = (struct d3d12_video_processor *) codec;
   assert(pD3D12Proc);
   assert(pD3D12Proc->m_spD3D12VideoDevice);
   assert(pD3D12Proc->m_spCommandQueue);
   HRESULT hr = S_OK;

   ASSERTED bool wait_result = d3d12_video_processor_ensure_fence_finished(codec, fenceValueToWaitOn, timeout_ns);
   assert(wait_result);

   hr =
      pD3D12Proc->m_spCommandAllocators[fenceValueToWaitOn % D3D12_VIDEO_PROC_ASYNC_DEPTH]->Reset();
   if (FAILED(hr)) {
      debug_printf("m_spCommandAllocator->Reset() failed with %x.\n", hr);
      goto sync_with_token_fail;
   }

   // Validate device was not removed
   hr = pD3D12Proc->m_pD3D12Screen->dev->GetDeviceRemovedReason();
   if (hr != S_OK) {
      debug_printf("[d3d12_video_processor] d3d12_video_processor_sync_completion"
                   " - D3D12Device was removed AFTER d3d12_video_processor_ensure_fence_finished "
                   "execution with HR %x, but wasn't before.\n",
                   hr);
      goto sync_with_token_fail;
   }

   debug_printf(
      "[d3d12_video_processor] d3d12_video_processor_sync_completion - GPU execution finalized for fenceValue: %" PRIu64
      "\n",
      fenceValueToWaitOn);

   return wait_result;

sync_with_token_fail:
   debug_printf("[d3d12_video_processor] d3d12_video_processor_sync_completion failed for fenceValue: %" PRIu64 "\n",
                fenceValueToWaitOn);
   assert(false);
   return false;
}

int d3d12_video_processor_get_processor_fence(struct pipe_video_codec *codec,
                                              struct pipe_fence_handle *fence,
                                              uint64_t timeout)
{
   struct d3d12_fence *fenceValueToWaitOn = (struct d3d12_fence *) fence;
   assert(fenceValueToWaitOn);

   ASSERTED bool wait_res = d3d12_video_processor_sync_completion(codec, fenceValueToWaitOn->value, timeout);

   // Return semantics based on p_video_codec interface
   // ret == 0 -> work in progress
   // ret != 0 -> work completed
   return wait_res ? 1 : 0;
}
