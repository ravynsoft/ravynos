/*
 * Copyright 2014, 2015 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef VIRGL_DRM_WINSYS_H
#define VIRGL_DRM_WINSYS_H

#include <stdint.h>
#include "util/u_thread.h"
#include "pipe/p_state.h"
#include "util/list.h"

#include "virgl/virgl_winsys.h"
#include "virgl_resource_cache.h"

struct pipe_fence_handle;
struct hash_table;

struct virgl_hw_res {
   struct pipe_reference reference;
   enum pipe_texture_target target;
   uint32_t res_handle;
   uint32_t bo_handle;
   int num_cs_references;
   uint32_t size;
   void *ptr;

   struct virgl_resource_cache_entry cache_entry;
   uint32_t bind;
   uint32_t flags;
   uint32_t flink_name;

   /* false when the resource is known to be typed */
   bool maybe_untyped;

   /* true when the resource is imported or exported */
   int external;

   /* false when the resource is known to be idle */
   int maybe_busy;
   uint32_t blob_mem;
};


struct param {
   uint64_t param;
   const char *name;
   uint64_t value;
};

enum param_id {
   param_3d_features,
   param_capset_fix,
   param_resource_blob,
   param_host_visible,
   param_cross_device,
   param_context_init,
   param_supported_capset_ids,
   param_max,
};

#define PARAM(x) (struct param) { x, #x, 0 }

struct param params[] = { PARAM(VIRTGPU_PARAM_3D_FEATURES),
                          PARAM(VIRTGPU_PARAM_CAPSET_QUERY_FIX),
                          PARAM(VIRTGPU_PARAM_RESOURCE_BLOB),
                          PARAM(VIRTGPU_PARAM_HOST_VISIBLE),
                          PARAM(VIRTGPU_PARAM_CROSS_DEVICE),
                          PARAM(VIRTGPU_PARAM_CONTEXT_INIT),
                          PARAM(VIRTGPU_PARAM_SUPPORTED_CAPSET_IDs)
};

struct virgl_drm_winsys
{
   struct virgl_winsys base;
   int fd;
   struct virgl_resource_cache cache;
   mtx_t mutex;

   int32_t blob_id;
   struct hash_table *bo_handles;
   struct hash_table *bo_names;
   mtx_t bo_handles_mutex;
};

struct virgl_drm_fence {
   struct pipe_reference reference;
   bool external;
   int fd;
   struct virgl_hw_res *hw_res;
};

struct virgl_drm_cmd_buf {
   struct virgl_cmd_buf base;

   uint32_t *buf;

   int in_fence_fd;

   unsigned nres;
   unsigned cres;
   struct virgl_hw_res **res_bo;
   struct virgl_winsys *ws;
   uint32_t *res_hlist;

   char                        is_handle_added[512];
   unsigned                    reloc_indices_hashlist[512];

};

static inline struct virgl_drm_winsys *
virgl_drm_winsys(struct virgl_winsys *iws)
{
   return (struct virgl_drm_winsys *)iws;
}

static inline struct virgl_drm_fence *
virgl_drm_fence(struct pipe_fence_handle *f)
{
   return (struct virgl_drm_fence *)f;
}

static inline struct virgl_drm_cmd_buf *
virgl_drm_cmd_buf(struct virgl_cmd_buf *cbuf)
{
   return (struct virgl_drm_cmd_buf *)cbuf;
}

#endif
