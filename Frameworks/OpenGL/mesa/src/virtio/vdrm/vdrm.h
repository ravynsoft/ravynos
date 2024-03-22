/*
 * Copyright © 2023 Google, Inc.
 * SPDX-License-Identifier: MIT
 */

/* A simple helper layer for virtgpu drm native context, which also
 * abstracted the differences between vtest (communicating via socket
 * with vtest server) vs virtgpu (communicating via drm/virtio driver
 * in the guest).
 */

#ifndef __VDRM_H__
#define __VDRM_H__

#include <stdint.h>

#include "util/simple_mtx.h"

#define VIRGL_RENDERER_UNSTABLE_APIS 1
#include "virglrenderer_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vdrm_device;
struct vdrm_execbuf_params;

struct vdrm_device_funcs {
   /* Note flush_locked and execbuf_locked are similar, and on top of virtgpu
    * guest kernel driver are basically the same.  But with vtest, only cmds
    * that result in host kernel cmd submission can take and/or return fence
    * and/or syncobj fd's.
    */
   int (*execbuf_locked)(struct vdrm_device *vdev, struct vdrm_execbuf_params *p,
                         void *command, unsigned size);
   int (*flush_locked)(struct vdrm_device *vdev, uintptr_t *fencep);

   void (*wait_fence)(struct vdrm_device *vdev, uintptr_t fence);

   uint32_t (*dmabuf_to_handle)(struct vdrm_device *vdev, int fd);
   uint32_t (*handle_to_res_id)(struct vdrm_device *vdev, uint32_t handle);

   uint32_t (*bo_create)(struct vdrm_device *vdev, size_t size, uint32_t blob_flags,
                         uint64_t blob_id, struct vdrm_ccmd_req *req);
   int (*bo_wait)(struct vdrm_device *vdev, uint32_t handle);
   void *(*bo_map)(struct vdrm_device *vdev, uint32_t handle, size_t size);
   int (*bo_export_dmabuf)(struct vdrm_device *vdev, uint32_t handle);
   void (*bo_close)(struct vdrm_device *vdev, uint32_t handle);

   void (*close)(struct vdrm_device *vdev);
};

struct vdrm_device {
   const struct vdrm_device_funcs *funcs;

   struct virgl_renderer_capset_drm caps;
   struct vdrm_shmem *shmem;
   uint8_t *rsp_mem;
   uint32_t rsp_mem_len;
   uint32_t next_rsp_off;
   simple_mtx_t rsp_lock;
   simple_mtx_t eb_lock;

   uint32_t next_seqno;

   /*
    * Buffering for requests to host:
    */
   uint32_t reqbuf_len;
   uint32_t reqbuf_cnt;
   uint8_t reqbuf[0x4000];
};

struct vdrm_device *vdrm_device_connect(int fd, uint32_t context_type);
void vdrm_device_close(struct vdrm_device *vdev);

void * vdrm_alloc_rsp(struct vdrm_device *vdev, struct vdrm_ccmd_req *req, uint32_t sz);
int vdrm_send_req(struct vdrm_device *vdev, struct vdrm_ccmd_req *req, bool sync);
int vdrm_flush(struct vdrm_device *vdev);

struct vdrm_execbuf_params {
   int ring_idx;

   struct vdrm_ccmd_req *req;     /* Note, must be host kernel cmd submit */

   uint32_t *handles;
   uint32_t num_handles;

   struct drm_virtgpu_execbuffer_syncobj *in_syncobjs;
   struct drm_virtgpu_execbuffer_syncobj *out_syncobjs;

   bool has_in_fence_fd : 1;
   bool needs_out_fence_fd : 1;

   int fence_fd;                  /* in/out fence */

   uint32_t num_in_syncobjs;
   uint32_t num_out_syncobjs;
};

/**
 * Note, must be a host cmd submission, which specified in/out fence/syncobj
 * can be passed to.  In the vtest case, we can't get fences/syncobjs for
 * other host cmds.
 */
int vdrm_execbuf(struct vdrm_device *vdev, struct vdrm_execbuf_params *p);

void vdrm_host_sync(struct vdrm_device *vdev, const struct vdrm_ccmd_req *req);

/**
 * Import dmabuf fd returning a GEM handle
 */
static inline uint32_t
vdrm_dmabuf_to_handle(struct vdrm_device *vdev, int fd)
{
   return vdev->funcs->dmabuf_to_handle(vdev, fd);
}

static inline uint32_t
vdrm_handle_to_res_id(struct vdrm_device *vdev, uint32_t handle)
{
   return vdev->funcs->handle_to_res_id(vdev, handle);
}

uint32_t vdrm_bo_create(struct vdrm_device *vdev, size_t size,
                        uint32_t blob_flags, uint64_t blob_id,
                        struct vdrm_ccmd_req *req);

static inline int
vdrm_bo_wait(struct vdrm_device *vdev, uint32_t handle)
{
   return vdev->funcs->bo_wait(vdev, handle);
}

static inline void *
vdrm_bo_map(struct vdrm_device *vdev, uint32_t handle, size_t size)
{
   return vdev->funcs->bo_map(vdev, handle, size);
}

static inline int
vdrm_bo_export_dmabuf(struct vdrm_device *vdev, uint32_t handle)
{
   return vdev->funcs->bo_export_dmabuf(vdev, handle);
}

static inline void
vdrm_bo_close(struct vdrm_device *vdev, uint32_t handle)
{
   vdev->funcs->bo_close(vdev, handle);
}

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* __VDRM_H__ */
