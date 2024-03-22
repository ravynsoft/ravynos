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

#include "d3d12_bufmgr.h"
#include "d3d12_context.h"
#include "d3d12_format.h"
#include "d3d12_screen.h"

#include "pipebuffer/pb_buffer.h"
#include "pipebuffer/pb_bufmgr.h"

#include "util/format/u_format.h"
#include "util/u_memory.h"

#include <dxguids/dxguids.h>

struct d3d12_bufmgr {
   struct pb_manager base;

   struct d3d12_screen *screen;
};

extern const struct pb_vtbl d3d12_buffer_vtbl;

static inline struct d3d12_bufmgr *
d3d12_bufmgr(struct pb_manager *mgr)
{
   assert(mgr);

   return (struct d3d12_bufmgr *)mgr;
}

static void
describe_direct_bo(char *buf, struct d3d12_bo *ptr)
{
   sprintf(buf, "d3d12_bo<direct,%p,0x%x>", ptr->res, (unsigned)ptr->estimated_size);
}

static void
describe_suballoc_bo(char *buf, struct d3d12_bo *ptr)
{
   char res[128];
   uint64_t offset;
   d3d12_bo *base = d3d12_bo_get_base(ptr, &offset);
   describe_direct_bo(res, base);
   sprintf(buf, "d3d12_bo<suballoc<%s>,0x%x,0x%x>", res,
           (unsigned)ptr->buffer->base.size, (unsigned)offset);
}

void
d3d12_debug_describe_bo(char *buf, struct d3d12_bo *ptr)
{
   if (ptr->buffer)
      describe_suballoc_bo(buf, ptr);
   else
      describe_direct_bo(buf, ptr);
}

struct d3d12_bo *
d3d12_bo_wrap_res(struct d3d12_screen *screen, ID3D12Resource *res, enum d3d12_residency_status residency)
{
   struct d3d12_bo *bo;

   bo = MALLOC_STRUCT(d3d12_bo);
   if (!bo)
      return NULL;
   memset(bo, 0, offsetof(d3d12_bo, local_context_states));

   D3D12_RESOURCE_DESC desc = GetDesc(res);
   unsigned array_size = desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? 1 : desc.DepthOrArraySize;
   unsigned total_subresources = desc.MipLevels * array_size * d3d12_non_opaque_plane_count(desc.Format);
   bool supports_simultaneous_access = d3d12_resource_supports_simultaneous_access(&desc);

   pipe_reference_init(&bo->reference, 1);
   bo->screen = screen;
   bo->res = res;
   bo->unique_id = p_atomic_inc_return(&screen->resource_id_generator);
   if (!supports_simultaneous_access)
      d3d12_resource_state_init(&bo->global_state, total_subresources, false);

   bo->residency_status = residency;
   bo->last_used_timestamp = 0;
   desc.Flags &= ~D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
   screen->dev->GetCopyableFootprints(&desc, 0, total_subresources, 0, nullptr, nullptr, nullptr, &bo->estimated_size);
   if (residency == d3d12_resident) {
      mtx_lock(&screen->submit_mutex);
      list_add(&bo->residency_list_entry, &screen->residency_list);
      mtx_unlock(&screen->submit_mutex);
   }

   return bo;
}

struct d3d12_bo *
d3d12_bo_new(struct d3d12_screen *screen, uint64_t size, const pb_desc *pb_desc)
{
   ID3D12Device *dev = screen->dev;
   ID3D12Resource *res;

   D3D12_RESOURCE_DESC res_desc;
   res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
   res_desc.Format = DXGI_FORMAT_UNKNOWN;
   res_desc.Alignment = 0;
   res_desc.Width = size;
   res_desc.Height = 1;
   res_desc.DepthOrArraySize = 1;
   res_desc.MipLevels = 1;
   res_desc.SampleDesc.Count = 1;
   res_desc.SampleDesc.Quality = 0;
   res_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
   res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

   D3D12_HEAP_TYPE heap_type = D3D12_HEAP_TYPE_DEFAULT;
   if (pb_desc->usage & PB_USAGE_CPU_READ)
      heap_type = D3D12_HEAP_TYPE_READBACK;
   else if (pb_desc->usage & PB_USAGE_CPU_WRITE)
      heap_type = D3D12_HEAP_TYPE_UPLOAD;

   D3D12_HEAP_FLAGS heap_flags = screen->support_create_not_resident ?
      D3D12_HEAP_FLAG_CREATE_NOT_RESIDENT : D3D12_HEAP_FLAG_NONE;
   enum d3d12_residency_status init_residency = screen->support_create_not_resident ?
      d3d12_evicted : d3d12_resident;

   D3D12_HEAP_PROPERTIES heap_pris = GetCustomHeapProperties(dev, heap_type);
   HRESULT hres = dev->CreateCommittedResource(&heap_pris,
                                               heap_flags,
                                               &res_desc,
                                               D3D12_RESOURCE_STATE_COMMON,
                                               NULL,
                                               IID_PPV_ARGS(&res));

   if (FAILED(hres))
      return NULL;

   return d3d12_bo_wrap_res(screen, res, init_residency);
}

struct d3d12_bo *
d3d12_bo_wrap_buffer(struct d3d12_screen *screen, struct pb_buffer *buf)
{
   struct d3d12_bo *bo;

   bo = MALLOC_STRUCT(d3d12_bo);
   if (!bo)
      return NULL;
   memset(bo, 0, offsetof(d3d12_bo, local_context_states));

   pipe_reference_init(&bo->reference, 1);
   bo->screen = screen;
   bo->buffer = buf;
   bo->unique_id = p_atomic_inc_return(&screen->resource_id_generator);
   bo->residency_status = d3d12_evicted;

   return bo;
}

void
d3d12_bo_unreference(struct d3d12_bo *bo)
{
   if (bo == NULL)
      return;

   assert(pipe_is_referenced(&bo->reference));

   if (pipe_reference_described(&bo->reference, NULL,
                                (debug_reference_descriptor)
                                d3d12_debug_describe_bo)) {
      if (bo->buffer)
         pb_reference(&bo->buffer, NULL);

      mtx_lock(&bo->screen->submit_mutex);

      if (bo->residency_status == d3d12_resident)
         list_del(&bo->residency_list_entry);

      /* MSVC's offsetof fails when the name is ambiguous between struct and function */
      typedef struct d3d12_context d3d12_context_type;
      list_for_each_entry(d3d12_context_type, ctx, &bo->screen->context_list, context_list_entry)
         if (ctx->id == D3D12_CONTEXT_NO_ID)
            util_dynarray_append(&ctx->recently_destroyed_bos, uint64_t, bo->unique_id);

      mtx_unlock(&bo->screen->submit_mutex);

      d3d12_resource_state_cleanup(&bo->global_state);
      if (bo->res)
         bo->res->Release();

      uint64_t mask = bo->local_context_state_mask;
      while (mask) {
         int ctxid = u_bit_scan64(&mask);
         d3d12_destroy_context_state_table_entry(&bo->local_context_states[ctxid]);
      }

      FREE(bo);
   }
}

void *
d3d12_bo_map(struct d3d12_bo *bo, D3D12_RANGE *range)
{
   struct d3d12_bo *base_bo;
   D3D12_RANGE offset_range = {0, 0};
   uint64_t offset;
   void *ptr;

   base_bo = d3d12_bo_get_base(bo, &offset);

   if (!range || range->Begin >= range->End) {
      offset_range.Begin = offset;
      offset_range.End = offset + d3d12_bo_get_size(bo);
      range = &offset_range;
   } else {
      offset_range.Begin = range->Begin + offset;
      offset_range.End = range->End + offset;
      range = &offset_range;
   }

   if (FAILED(base_bo->res->Map(0, range, &ptr)))
      return NULL;

   return (uint8_t *)ptr + (range ? range->Begin : 0);
}

void
d3d12_bo_unmap(struct d3d12_bo *bo, D3D12_RANGE *range)
{
   struct d3d12_bo *base_bo;
   D3D12_RANGE offset_range = {0, 0};
   uint64_t offset;

   base_bo = d3d12_bo_get_base(bo, &offset);

   if (!range || range->Begin >= range->End) {
      offset_range.Begin = offset;
      offset_range.End = offset + d3d12_bo_get_size(bo);
      range = &offset_range;
   } else {
      offset_range.Begin = range->Begin + offset;
      offset_range.End = range->End + offset;
      range = &offset_range;
   }

   base_bo->res->Unmap(0, range);
}

static void
d3d12_buffer_destroy(void *winsys, struct pb_buffer *pbuf)
{
   struct d3d12_buffer *buf = d3d12_buffer(pbuf);

   if (buf->map)
      d3d12_bo_unmap(buf->bo, &buf->range);
   d3d12_bo_unreference(buf->bo);
   FREE(buf);
}

static void *
d3d12_buffer_map(struct pb_buffer *pbuf,
                 enum pb_usage_flags flags,
                 void *flush_ctx)
{
   return d3d12_buffer(pbuf)->map;
}

static void
d3d12_buffer_unmap(struct pb_buffer *pbuf)
{
}

static void
d3d12_buffer_get_base_buffer(struct pb_buffer *buf,
                             struct pb_buffer **base_buf,
                             pb_size *offset)
{
   *base_buf = buf;
   *offset = 0;
}

static enum pipe_error
d3d12_buffer_validate(struct pb_buffer *pbuf,
                      struct pb_validate *vl,
                      enum pb_usage_flags flags )
{
   /* Always pinned */
   return PIPE_OK;
}

static void
d3d12_buffer_fence(struct pb_buffer *pbuf,
                   struct pipe_fence_handle *fence )
{
}

const struct pb_vtbl d3d12_buffer_vtbl = {
   d3d12_buffer_destroy,
   d3d12_buffer_map,
   d3d12_buffer_unmap,
   d3d12_buffer_validate,
   d3d12_buffer_fence,
   d3d12_buffer_get_base_buffer
};

static struct pb_buffer *
d3d12_bufmgr_create_buffer(struct pb_manager *pmgr,
                           pb_size size,
                           const struct pb_desc *pb_desc)
{
   struct d3d12_bufmgr *mgr = d3d12_bufmgr(pmgr);
   struct d3d12_buffer *buf;

   buf = CALLOC_STRUCT(d3d12_buffer);
   if (!buf)
      return NULL;

   pipe_reference_init(&buf->base.base.reference, 1);
   buf->base.base.alignment_log2 = util_logbase2(pb_desc->alignment);
   buf->base.base.usage = pb_desc->usage;
   buf->base.vtbl = &d3d12_buffer_vtbl;
   buf->base.base.size = size;
   buf->range.Begin = 0;
   buf->range.End = size;

   buf->bo = d3d12_bo_new(mgr->screen, size, pb_desc);
   if (!buf->bo) {
      FREE(buf);
      return NULL;
   }

   if (pb_desc->usage & PB_USAGE_CPU_READ_WRITE) {
      buf->map = d3d12_bo_map(buf->bo, &buf->range);
      if (!buf->map) {
         d3d12_bo_unreference(buf->bo);
         FREE(buf);
         return NULL;
      }
   }

   return &buf->base;
}

static void
d3d12_bufmgr_flush(struct pb_manager *mgr)
{
   /* No-op */
}

static void
d3d12_bufmgr_destroy(struct pb_manager *_mgr)
{
   struct d3d12_bufmgr *mgr = d3d12_bufmgr(_mgr);
   FREE(mgr);
}

static bool
d3d12_bufmgr_is_buffer_busy(struct pb_manager *_mgr, struct pb_buffer *_buf)
{
   /* We're only asked this on buffers that are known not busy */
   return false;
}

struct pb_manager *
d3d12_bufmgr_create(struct d3d12_screen *screen)
{
   struct d3d12_bufmgr *mgr;

   mgr = CALLOC_STRUCT(d3d12_bufmgr);
   if (!mgr)
      return NULL;

   mgr->base.destroy = d3d12_bufmgr_destroy;
   mgr->base.create_buffer = d3d12_bufmgr_create_buffer;
   mgr->base.flush = d3d12_bufmgr_flush;
   mgr->base.is_buffer_busy = d3d12_bufmgr_is_buffer_busy;

   mgr->screen = screen;

   return &mgr->base;
}
