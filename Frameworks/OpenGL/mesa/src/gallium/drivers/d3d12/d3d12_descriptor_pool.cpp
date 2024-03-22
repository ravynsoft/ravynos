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
#include "d3d12_descriptor_pool.h"
#include "d3d12_screen.h"

#include "pipe/p_context.h"
#include "pipe/p_state.h"

#include "util/list.h"
#include "util/u_dynarray.h"
#include "util/u_memory.h"

#include <dxguids/dxguids.h>

struct d3d12_descriptor_pool {
   ID3D12Device *dev;
   D3D12_DESCRIPTOR_HEAP_TYPE type;
   uint32_t num_descriptors;
   list_head heaps;
};

struct d3d12_descriptor_heap {
   struct d3d12_descriptor_pool *pool;

   D3D12_DESCRIPTOR_HEAP_DESC desc;
   ID3D12Device *dev;
   ID3D12DescriptorHeap *heap;
   uint32_t desc_size;
   uint64_t cpu_base;
   uint64_t gpu_base;
   uint32_t size;
   uint32_t next;
   util_dynarray free_list;
   list_head link;
};

struct d3d12_descriptor_heap*
d3d12_descriptor_heap_new(ID3D12Device *dev,
                          D3D12_DESCRIPTOR_HEAP_TYPE type,
                          D3D12_DESCRIPTOR_HEAP_FLAGS flags,
                          uint32_t num_descriptors)
{
   struct d3d12_descriptor_heap *heap = CALLOC_STRUCT(d3d12_descriptor_heap);

   heap->desc.NumDescriptors = num_descriptors;
   heap->desc.Type = type;
   heap->desc.Flags = flags;
   if (FAILED(dev->CreateDescriptorHeap(&heap->desc,
                                        IID_PPV_ARGS(&heap->heap)))) {
      FREE(heap);
      return NULL;
   }

   heap->dev = dev;
   heap->desc_size = dev->GetDescriptorHandleIncrementSize(type);
   heap->size = num_descriptors * heap->desc_size;
   heap->cpu_base = GetCPUDescriptorHandleForHeapStart(heap->heap).ptr;
   if (flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
      heap->gpu_base = GetGPUDescriptorHandleForHeapStart(heap->heap).ptr;
   util_dynarray_init(&heap->free_list, NULL);

   return heap;
}

void
d3d12_descriptor_heap_free(struct d3d12_descriptor_heap *heap)
{
   heap->heap->Release();
   util_dynarray_fini(&heap->free_list);
   FREE(heap);
}

ID3D12DescriptorHeap*
d3d12_descriptor_heap_get(struct d3d12_descriptor_heap *heap)
{
   return heap->heap;
}

static uint32_t
d3d12_descriptor_heap_is_online(struct d3d12_descriptor_heap *heap)
{
   return (heap->desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) ? 1 : 0;
}

static uint32_t
d3d12_descriptor_heap_can_allocate(struct d3d12_descriptor_heap *heap)
{
   return (heap->free_list.size > 0 ||
           heap->size >= heap->next + heap->desc_size);
}

uint32_t
d3d12_descriptor_heap_get_remaining_handles(struct d3d12_descriptor_heap *heap)
{
   return (heap->size - heap->next) / heap->desc_size;
}

void
d2d12_descriptor_heap_get_next_handle(struct d3d12_descriptor_heap *heap,
                                      struct d3d12_descriptor_handle *handle)
{
   handle->heap = heap;
   handle->cpu_handle.ptr = heap->cpu_base + heap->next;
   handle->gpu_handle.ptr = d3d12_descriptor_heap_is_online(heap) ?
                            heap->gpu_base + heap->next : 0;
}

uint32_t
d3d12_descriptor_heap_alloc_handle(struct d3d12_descriptor_heap *heap,
                                   struct d3d12_descriptor_handle *handle)
{
   uint32_t offset = 0;

   assert(handle != NULL);

   if (heap->free_list.size > 0) {
      offset = util_dynarray_pop(&heap->free_list, uint32_t);
   } else if (heap->size >= heap->next + heap->desc_size) {
      offset = heap->next;
      heap->next += heap->desc_size;
   } else {
      /* Todo: we should add a new descriptor heap to get more handles */
      assert(0 && "No handles available in descriptor heap");
      return 0;
   }

   handle->heap = heap;
   handle->cpu_handle.ptr = heap->cpu_base + offset;
   handle->gpu_handle.ptr = d3d12_descriptor_heap_is_online(heap) ?
                            heap->gpu_base + offset : 0;

   return 1;
}

void
d3d12_descriptor_handle_free(struct d3d12_descriptor_handle *handle)
{
   const uint32_t index = handle->cpu_handle.ptr - handle->heap->cpu_base;
   if (index + handle->heap->desc_size == handle->heap->next) {
      handle->heap->next = index;
   } else {
      util_dynarray_append(&handle->heap->free_list, uint32_t, index);
   }

   handle->heap = NULL;
   handle->cpu_handle.ptr = 0;
   handle->gpu_handle.ptr = 0;
}

void
d3d12_descriptor_heap_append_handles(struct d3d12_descriptor_heap *heap,
                                     D3D12_CPU_DESCRIPTOR_HANDLE *handles,
                                     unsigned num_handles)
{
   D3D12_CPU_DESCRIPTOR_HANDLE dst;

   assert(heap->next + (num_handles * heap->desc_size) <= heap->size);
   dst.ptr = heap->cpu_base + heap->next;
   heap->dev->CopyDescriptors(1, &dst, &num_handles,
                              num_handles, handles, NULL,
                              heap->desc.Type);
   heap->next += num_handles * heap->desc_size;
}

void
d3d12_descriptor_heap_clear(struct d3d12_descriptor_heap *heap)
{
   heap->next = 0;
   util_dynarray_clear(&heap->free_list);
}

struct d3d12_descriptor_pool*
d3d12_descriptor_pool_new(struct d3d12_screen *screen,
                          D3D12_DESCRIPTOR_HEAP_TYPE type,
                          uint32_t num_descriptors)
{
   struct d3d12_descriptor_pool *pool = CALLOC_STRUCT(d3d12_descriptor_pool);
   if (!pool)
      return NULL;

   pool->dev = screen->dev;
   pool->type = type;
   pool->num_descriptors = num_descriptors;
   list_inithead(&pool->heaps);

   return pool;
}

void
d3d12_descriptor_pool_free(struct d3d12_descriptor_pool *pool)
{
   list_for_each_entry_safe(struct d3d12_descriptor_heap, heap, &pool->heaps, link) {
      list_del(&heap->link);
      d3d12_descriptor_heap_free(heap);
   }
   FREE(pool);
}

uint32_t
d3d12_descriptor_pool_alloc_handle(struct d3d12_descriptor_pool *pool,
                                   struct d3d12_descriptor_handle *handle)
{
   struct d3d12_descriptor_heap *valid_heap = NULL;

   list_for_each_entry(struct d3d12_descriptor_heap, heap, &pool->heaps, link) {
      if (d3d12_descriptor_heap_can_allocate(heap)) {
         valid_heap = heap;
         break;
      }
   }

   if (!valid_heap) {
      valid_heap = d3d12_descriptor_heap_new(pool->dev,
                                             pool->type,
                                             D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                                             pool->num_descriptors);
      list_addtail(&valid_heap->link, &pool->heaps);
   }

   return d3d12_descriptor_heap_alloc_handle(valid_heap, handle);
}
