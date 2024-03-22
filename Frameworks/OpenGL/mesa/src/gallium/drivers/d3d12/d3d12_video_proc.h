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

#ifndef D3D12_VIDEO_PROC_H
#define D3D12_VIDEO_PROC_H
#include "d3d12_video_types.h"


///
/// Pipe video interface starts
///

/**
 * creates a video processor
 */
struct pipe_video_codec *
d3d12_video_create_processor(struct pipe_context *context, const struct pipe_video_codec *templ);

/**
 * destroy this video processor
 */
void
d3d12_video_processor_destroy(struct pipe_video_codec *codec);

/**
 * start processing of a new frame
 */
void
d3d12_video_processor_begin_frame(struct pipe_video_codec * codec,
                                struct pipe_video_buffer *target,
                                struct pipe_picture_desc *picture);

/**
 * Perform post-process effect
 */
void
d3d12_video_processor_process_frame(struct pipe_video_codec *codec,
                        struct pipe_video_buffer *input_texture,
                        const struct pipe_vpp_desc *process_properties);
/**
 * end processing of the current frame
 */
void
d3d12_video_processor_end_frame(struct pipe_video_codec * codec,
                              struct pipe_video_buffer *target,
                              struct pipe_picture_desc *picture);
/**

 * flush any outstanding command buffers to the hardware
 * should be called before a video_buffer is acessed by the gallium frontend again
 */
void
d3d12_video_processor_flush(struct pipe_video_codec *codec);

///
/// Pipe video interface ends
///

///
/// d3d12_video_processor functions starts
///

struct d3d12_video_processor_output_context
{
   struct D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS args;
   struct d3d12_video_buffer* buffer;
};

struct d3d12_video_processor
{
   struct pipe_video_codec base;
   struct d3d12_screen  *m_pD3D12Screen;
   struct d3d12_context *m_pD3D12Context;

   ///
   /// D3D12 objects and context info
   ///

   const uint m_NodeMask  = 0u;
   const uint m_NodeIndex = 0u;

   ComPtr<ID3D12Fence> m_spFence;
   uint                m_fenceValue = 1u;

   ComPtr<ID3D12VideoDevice>             m_spD3D12VideoDevice;
   
   D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT           m_SupportCaps;
   D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC             m_outputStreamDesc;
   std::vector<D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC> m_inputStreamDescs;
   ComPtr<ID3D12VideoProcessor1>                      m_spVideoProcessor;
   ComPtr<ID3D12CommandQueue>                         m_spCommandQueue;
   std::vector<ComPtr<ID3D12CommandAllocator>>        m_spCommandAllocators;
   std::vector<struct d3d12_fence>                    m_PendingFences;
   ComPtr<ID3D12VideoProcessCommandList1>             m_spCommandList;

   std::vector<D3D12_RESOURCE_BARRIER> m_transitionsBeforeCloseCmdList;

   // Current state between begin and end frame
   d3d12_video_processor_output_context m_OutputArguments;
   std::vector<D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS1> m_ProcessInputs;
   std::vector<struct d3d12_video_buffer*> m_InputBuffers;

   // Indicates if GPU commands have not been flushed and are pending.
   bool m_needsGPUFlush = false;

   D3D12_FEATURE_DATA_VIDEO_PROCESS_MAX_INPUT_STREAMS m_vpMaxInputStreams = { };

   struct d3d12_fence* input_surface_fence = NULL;
};

struct pipe_video_codec *
d3d12_video_processor_create(struct pipe_context *context, const struct pipe_video_codec *codec);

bool
d3d12_video_processor_check_caps_and_create_processor(struct d3d12_video_processor *pD3D12Proc,
                                                        std::vector<DXGI_FORMAT> InputFormats,
                                                        DXGI_COLOR_SPACE_TYPE InputColorSpace,
                                                        DXGI_FORMAT OutputFormat,
                                                        DXGI_COLOR_SPACE_TYPE OutputColorSpace);

bool
d3d12_video_processor_create_command_objects(struct d3d12_video_processor *pD3D12Proc);

D3D12_VIDEO_PROCESS_ORIENTATION
d3d12_video_processor_convert_pipe_rotation(enum pipe_video_vpp_orientation orientation);

bool
d3d12_video_processor_ensure_fence_finished(struct pipe_video_codec *codec, uint64_t fenceValueToWaitOn, uint64_t timeout_ns);

bool
d3d12_video_processor_sync_completion(struct pipe_video_codec *codec, uint64_t fenceValueToWaitOn, uint64_t timeout_ns);

uint64_t
d3d12_video_processor_pool_current_index(struct d3d12_video_processor *codec);

int d3d12_video_processor_get_processor_fence(struct pipe_video_codec *codec,
                                              struct pipe_fence_handle *fence,
                                              uint64_t timeout);

// We need enough to so next item in pipeline doesn't ask for a fence value we lost
const uint64_t D3D12_VIDEO_PROC_ASYNC_DEPTH = 36;

///
/// d3d12_video_processor functions ends
///

#endif
