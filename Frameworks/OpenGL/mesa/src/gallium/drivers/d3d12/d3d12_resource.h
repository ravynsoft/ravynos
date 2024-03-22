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

#ifndef D3D12_RESOURCE_H
#define D3D12_RESOURCE_H

struct pipe_screen;
#include "d3d12_bufmgr.h"
#include "util/u_range.h"
#include "util/u_transfer.h"
#include "util/u_threaded_context.h"

#include "d3d12_common.h"

enum d3d12_resource_binding_type {
   D3D12_RESOURCE_BINDING_TYPE_SRV,
   D3D12_RESOURCE_BINDING_TYPE_CBV,
   D3D12_RESOURCE_BINDING_TYPE_SSBO,
   D3D12_RESOURCE_BINDING_TYPE_IMAGE,
   D3D12_RESOURCE_BINDING_TYPES
};

struct d3d12_resource {
   struct threaded_resource base;
   struct d3d12_bo *bo;
   DXGI_FORMAT dxgi_format;
   enum pipe_format overall_format;
   unsigned int plane_slice;
   struct pipe_resource* first_plane;
   unsigned mip_levels;
   struct sw_displaytarget *dt;
   unsigned dt_refcount; /* For planar resources sharing the dt pointer */
   unsigned dt_stride;
   struct util_range valid_buffer_range;
   uint32_t bind_counts[PIPE_SHADER_TYPES][D3D12_RESOURCE_BINDING_TYPES];
   unsigned generation_id;
};

struct d3d12_memory_object {
   struct pipe_memory_object base;
   ID3D12Resource *res;
   ID3D12Heap *heap;
};

struct d3d12_transfer {
   struct threaded_transfer base;
   struct pipe_resource *staging_res;
   void *data;
   unsigned zs_cpu_copy_stride;
   unsigned zs_cpu_copy_layer_stride;
};

static inline struct d3d12_resource *
d3d12_resource(struct pipe_resource *r)
{
   return (struct d3d12_resource *)r;
}

static inline struct d3d12_memory_object *
d3d12_memory_object(struct pipe_memory_object *m)
{
   return (struct d3d12_memory_object *)m;
}

/* Returns the underlying ID3D12Resource and offset for this resource */
static inline ID3D12Resource *
d3d12_resource_underlying(struct d3d12_resource *res, uint64_t *offset)
{
   if (!res->bo)
      return NULL;

   return d3d12_bo_get_base(res->bo, offset)->res;
}

/* Returns the underlying ID3D12Resource for this resource. */
static inline ID3D12Resource *
d3d12_resource_resource(struct d3d12_resource *res)
{
   ID3D12Resource *ret;
   uint64_t offset;
   ret = d3d12_resource_underlying(res, &offset);
   return ret;
}

static inline D3D12_GPU_VIRTUAL_ADDRESS
d3d12_resource_gpu_virtual_address(struct d3d12_resource *res)
{
   uint64_t offset;
   ID3D12Resource *base_res = d3d12_resource_underlying(res, &offset);
   return base_res->GetGPUVirtualAddress() + offset;
}

static inline bool
d3d12_subresource_id_uses_layer(enum pipe_texture_target target)
{
   return target == PIPE_TEXTURE_CUBE ||
          target == PIPE_TEXTURE_CUBE_ARRAY ||
          target == PIPE_TEXTURE_1D_ARRAY ||
          target == PIPE_TEXTURE_2D_ARRAY;
}

void
d3d12_resource_release(struct d3d12_resource *res);

void
d3d12_resource_wait_idle(struct d3d12_context *ctx,
                         struct d3d12_resource *res,
                         bool want_to_write);

void
d3d12_screen_resource_init(struct pipe_screen *pscreen);

void
d3d12_context_resource_init(struct pipe_context *pctx);

struct pipe_resource *
d3d12_resource_from_resource(struct pipe_screen *pscreen,
                              ID3D12Resource* inputRes);

#endif
