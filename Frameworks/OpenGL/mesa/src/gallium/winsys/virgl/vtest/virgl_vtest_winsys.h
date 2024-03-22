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
#include "util/compiler.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "util/list.h"
#include "util/u_thread.h"

#include "virgl/virgl_winsys.h"
#include "vtest/vtest_protocol.h"
#include "virgl_resource_cache.h"

struct pipe_fence_handle;
struct sw_winsys;
struct sw_displaytarget;

struct virgl_vtest_winsys {
   struct virgl_winsys base;

   struct sw_winsys *sws;

   /* fd to remote renderer */
   int sock_fd;

   struct virgl_resource_cache cache;
   mtx_t mutex;

   unsigned protocol_version;
};

struct virgl_hw_res {
   struct pipe_reference reference;
   uint32_t res_handle;
   int num_cs_references;

   void *ptr;
   int size;

   uint32_t format;
   uint32_t stride;
   uint32_t width;
   uint32_t height;

   struct sw_displaytarget *dt;
   void *mapped;

   uint32_t bind;
   struct virgl_resource_cache_entry cache_entry;
};

struct virgl_vtest_cmd_buf {
   struct virgl_cmd_buf base;
   uint32_t *buf;
   unsigned nres;
   unsigned cres;
   struct virgl_winsys *ws;
   struct virgl_hw_res **res_bo;

   char                        is_handle_added[512];
   unsigned                    reloc_indices_hashlist[512];
};

static inline struct virgl_hw_res *
virgl_hw_res(struct pipe_fence_handle *f)
{
   return (struct virgl_hw_res *)f;
}

static inline struct virgl_vtest_winsys *
virgl_vtest_winsys(struct virgl_winsys *iws)
{
   return (struct virgl_vtest_winsys *)iws;
}

static inline struct virgl_vtest_cmd_buf *
virgl_vtest_cmd_buf(struct virgl_cmd_buf *cbuf)
{
   return (struct virgl_vtest_cmd_buf *)cbuf;
}


int virgl_vtest_connect(struct virgl_vtest_winsys *vws);
int virgl_vtest_send_get_caps(struct virgl_vtest_winsys *vws,
                              struct virgl_drm_caps *caps);

int virgl_vtest_send_resource_create(struct virgl_vtest_winsys *vws,
                                     uint32_t handle,
                                     enum pipe_texture_target target,
                                     uint32_t format,
                                     uint32_t bind,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t depth,
                                     uint32_t array_size,
                                     uint32_t last_level,
                                     uint32_t nr_samples,
                                     uint32_t size,
                                     int *out_fd);

int virgl_vtest_send_resource_unref(struct virgl_vtest_winsys *vws,
                                    uint32_t handle);
int virgl_vtest_submit_cmd(struct virgl_vtest_winsys *vtws,
                           struct virgl_vtest_cmd_buf *cbuf);

int virgl_vtest_send_transfer_get(struct virgl_vtest_winsys *vws,
                                  uint32_t handle,
                                  uint32_t level, uint32_t stride,
                                  uint32_t layer_stride,
                                  const struct pipe_box *box,
                                  uint32_t data_size,
                                  uint32_t offset);

int virgl_vtest_send_transfer_put(struct virgl_vtest_winsys *vws,
                                  uint32_t handle,
                                  uint32_t level, uint32_t stride,
                                  uint32_t layer_stride,
                                  const struct pipe_box *box,
                                  uint32_t data_size,
                                  uint32_t offset);

int virgl_vtest_send_transfer_put_data(struct virgl_vtest_winsys *vws,
                                       void *data,
                                       uint32_t data_size);
int virgl_vtest_recv_transfer_get_data(struct virgl_vtest_winsys *vws,
                                       void *data,
                                       uint32_t data_size,
                                       uint32_t stride,
                                       const struct pipe_box *box,
                                       uint32_t format);

int virgl_vtest_busy_wait(struct virgl_vtest_winsys *vws, int handle,
                          int flags);
#endif
