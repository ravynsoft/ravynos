/*
 * Copyright © Yonggang Luo
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

#ifndef D3D12_COMMON_STATE_H
#define D3D12_COMMON_STATE_H

#pragma once

#include <wsl/winadapter.h>

#define D3D12_IGNORE_SDK_LAYERS
#ifndef _GAMING_XBOX
#include <directx/d3d12.h>
#include <directx/d3d12video.h>
#elif defined(__cplusplus)
#ifdef _GAMING_XBOX_SCARLETT
#include <d3d12_xs.h>
#include <d3d12video_xs.h>
#else
#include <d3d12_x.h>
#include <d3d12video_x.h>
#endif
#ifdef IID_PPV_ARGS
#undef IID_PPV_ARGS
#endif
#define IID_PPV_ARGS IID_GRAPHICS_PPV_ARGS
#define D3D12_HEAP_FLAG_CREATE_NOT_RESIDENT (D3D12_HEAP_FLAGS) 0x800
#endif /* _GAMING_XBOX */

#ifndef D3D12_TEXTURE_DATA_PITCH_ALIGNMENT
#define D3D12_TEXTURE_DATA_PITCH_ALIGNMENT D3D12XBOX_TEXTURE_DATA_PITCH_ALIGNMENT
#endif /* D3D12_TEXTURE_DATA_PITCH_ALIGNMENT */

#if defined(__cplusplus)
#if !defined(_WIN32) || defined(_MSC_VER)
inline D3D12_CPU_DESCRIPTOR_HANDLE
GetCPUDescriptorHandleForHeapStart(ID3D12DescriptorHeap *heap)
{
   return heap->GetCPUDescriptorHandleForHeapStart();
}
inline D3D12_GPU_DESCRIPTOR_HANDLE
GetGPUDescriptorHandleForHeapStart(ID3D12DescriptorHeap *heap)
{
   return heap->GetGPUDescriptorHandleForHeapStart();
}
inline D3D12_RESOURCE_DESC
GetDesc(ID3D12Resource *res)
{
   return res->GetDesc();
}
inline D3D12_HEAP_PROPERTIES
GetCustomHeapProperties(ID3D12Device *dev, D3D12_HEAP_TYPE type)
{
   return dev->GetCustomHeapProperties(0, type);
}
inline LUID
GetAdapterLuid(ID3D12Device *dev)
{
   return dev->GetAdapterLuid();
}
inline D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC
GetOutputStreamDesc(ID3D12VideoProcessor *proc)
{
   return proc->GetOutputStreamDesc();
}
#else
inline D3D12_CPU_DESCRIPTOR_HANDLE
GetCPUDescriptorHandleForHeapStart(ID3D12DescriptorHeap *heap)
{
   D3D12_CPU_DESCRIPTOR_HANDLE ret;
   heap->GetCPUDescriptorHandleForHeapStart(&ret);
   return ret;
}
inline D3D12_GPU_DESCRIPTOR_HANDLE
GetGPUDescriptorHandleForHeapStart(ID3D12DescriptorHeap *heap)
{
   D3D12_GPU_DESCRIPTOR_HANDLE ret;
   heap->GetGPUDescriptorHandleForHeapStart(&ret);
   return ret;
}
inline D3D12_RESOURCE_DESC
GetDesc(ID3D12Resource *res)
{
   D3D12_RESOURCE_DESC ret;
   res->GetDesc(&ret);
   return ret;
}
inline D3D12_HEAP_PROPERTIES
GetCustomHeapProperties(ID3D12Device *dev, D3D12_HEAP_TYPE type)
{
   D3D12_HEAP_PROPERTIES ret;
   dev->GetCustomHeapProperties(&ret, 0, type);
   return ret;
}
inline LUID
GetAdapterLuid(ID3D12Device *dev)
{
   LUID ret;
   dev->GetAdapterLuid(&ret);
   return ret;
}
inline D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC
GetOutputStreamDesc(ID3D12VideoProcessor *proc)
{
   D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC ret;
   proc->GetOutputStreamDesc(&ret);
   return ret;
}
#endif
#endif

#endif
