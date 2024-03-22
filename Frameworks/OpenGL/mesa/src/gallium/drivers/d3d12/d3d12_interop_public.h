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

#include <stdint.h>

#ifndef D3D12_INTEROP_PUBLIC_H
#define D3D12_INTEROP_PUBLIC_H

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D12Device;
struct ID3D12CommandQueue;
struct ID3D12Resource;

struct d3d12_interop_device_info {
   uint64_t adapter_luid;
   ID3D12Device *device;
   ID3D12CommandQueue *queue;
};

struct d3d12_interop_resource_info {
   ID3D12Resource *resource;
   uint64_t buffer_offset;
};

#ifdef __cplusplus
}
#endif

#endif /* D3D12_INTEROP_PUBLIC_H */
