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

#ifndef D3D12_BUFMGR_H
#define D3D12_BUFMGR_H

#include "pipebuffer/pb_buffer.h"
#include "util/u_atomic.h"
#include "util/list.h"

#include "d3d12_common.h"
#include "d3d12_resource_state.h"

struct d3d12_bufmgr;
struct d3d12_screen;
struct pb_manager;

enum d3d12_residency_status {
   d3d12_evicted,
   d3d12_resident,
   d3d12_permanently_resident,
};

enum batch_bo_reference_state {
   batch_bo_reference_none = 0,
   batch_bo_reference_read = (1 << 0),
   batch_bo_reference_written = (1 << 1),
};

struct d3d12_bo {
   struct pipe_reference reference;
   struct d3d12_screen *screen;
   ID3D12Resource *res;
   struct pb_buffer *buffer;
   struct d3d12_resource_state global_state;

   /* Used as a key in per-context resource state maps,
    * to avoid needing to lock them for single-threaded lookups to
    * protect against resource destruction.
    */
   uint64_t unique_id;

   struct list_head residency_list_entry;
   uint64_t estimated_size;
   int64_t last_used_timestamp;
   uint64_t last_used_fence;
   enum d3d12_residency_status residency_status;
   uint16_t local_needs_resolve_state;

   unsigned local_context_state_mask;
   uint8_t local_reference_mask[16];

   d3d12_context_state_table_entry local_context_states[16];
   uint8_t local_reference_state[16][8];
};

struct d3d12_buffer {
   struct pb_buffer base;

   struct d3d12_bo *bo;
   D3D12_RANGE range;
   void *map;
};

static inline struct d3d12_buffer *
d3d12_buffer(struct pb_buffer *buf)
{
   assert(buf);
   return (struct d3d12_buffer *)buf;
}

static inline struct d3d12_bo *
d3d12_bo_get_base(struct d3d12_bo *bo, uint64_t *offset)
{
   if (bo->buffer) {
      struct pb_buffer *base_buffer;
      pb_get_base_buffer(bo->buffer, &base_buffer, offset);
      return d3d12_buffer(base_buffer)->bo;
   } else {
      *offset = 0;
      return bo;
   }
}

static inline uint64_t
d3d12_bo_get_size(struct d3d12_bo *bo)
{
   if (bo->buffer)
      return bo->buffer->base.size;
   else
      return GetDesc(bo->res).Width;
}

static inline bool
d3d12_bo_is_suballocated(struct d3d12_bo *bo)
{
   struct d3d12_bo *base_bo;
   uint64_t offset;

   if (!bo->buffer)
      return false;

   base_bo = d3d12_bo_get_base(bo, &offset);
   return d3d12_bo_get_size(base_bo) != d3d12_bo_get_size(bo);
}

struct d3d12_bo *
d3d12_bo_new(struct d3d12_screen *screen, uint64_t size, uint64_t alignment);

struct d3d12_bo *
d3d12_bo_wrap_res(struct d3d12_screen *screen, ID3D12Resource *res, enum d3d12_residency_status residency);

struct d3d12_bo *
d3d12_bo_wrap_buffer(struct d3d12_screen *screen, struct pb_buffer *buf);

void
d3d12_debug_describe_bo(char* buf, struct d3d12_bo* ptr);

static inline void
d3d12_bo_reference(struct d3d12_bo *bo)
{
   pipe_reference_described(NULL, &bo->reference,
                            (debug_reference_descriptor)
                            d3d12_debug_describe_bo);
}

void
d3d12_bo_unreference(struct d3d12_bo *bo);

void *
d3d12_bo_map(struct d3d12_bo *bo, D3D12_RANGE *range);

void
d3d12_bo_unmap(struct d3d12_bo *bo, D3D12_RANGE *range);

struct pb_manager *
d3d12_bufmgr_create(struct d3d12_screen *screen);

#endif
