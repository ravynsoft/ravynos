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

#ifndef D3D12_DESCRIPTOR_POOL_H
#define D3D12_DESCRIPTOR_POOL_H

#include "d3d12_common.h"

struct d3d12_descriptor_pool;
struct d3d12_descriptor_heap;

struct d3d12_descriptor_handle {
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
    struct d3d12_descriptor_heap *heap;
};

inline bool
d3d12_descriptor_handle_is_allocated(struct d3d12_descriptor_handle *handle)
{
    return (handle->heap != NULL);
}

void
d3d12_descriptor_handle_free(struct d3d12_descriptor_handle *handle);

/* Offline Descriptor Pool */

struct d3d12_descriptor_pool*
d3d12_descriptor_pool_new(struct d3d12_screen *screen,
                          D3D12_DESCRIPTOR_HEAP_TYPE type,
                          uint32_t num_descriptors);

void
d3d12_descriptor_pool_free(struct d3d12_descriptor_pool *pool);

uint32_t
d3d12_descriptor_pool_alloc_handle(struct d3d12_descriptor_pool *pool,
                                   struct d3d12_descriptor_handle *handle);


/* Online/Offline Descriptor Heaps */

struct d3d12_descriptor_heap*
d3d12_descriptor_heap_new(ID3D12Device *device,
                          D3D12_DESCRIPTOR_HEAP_TYPE type,
                          D3D12_DESCRIPTOR_HEAP_FLAGS flags,
                          uint32_t num_descriptors);

void
d3d12_descriptor_heap_free(struct d3d12_descriptor_heap *heap);

ID3D12DescriptorHeap*
d3d12_descriptor_heap_get(struct d3d12_descriptor_heap *heap);

void
d2d12_descriptor_heap_get_next_handle(struct d3d12_descriptor_heap *heap,
                                      struct d3d12_descriptor_handle *handle);

uint32_t
d3d12_descriptor_heap_get_remaining_handles(struct d3d12_descriptor_heap *heap);

uint32_t
d3d12_descriptor_heap_alloc_handle(struct d3d12_descriptor_heap *heap,
                                   struct d3d12_descriptor_handle *handle);

void
d3d12_descriptor_heap_append_handles(struct d3d12_descriptor_heap *heap,
                                     D3D12_CPU_DESCRIPTOR_HANDLE *handles,
                                     unsigned num_handles);

void
d3d12_descriptor_heap_clear(struct d3d12_descriptor_heap *heap);

#endif
