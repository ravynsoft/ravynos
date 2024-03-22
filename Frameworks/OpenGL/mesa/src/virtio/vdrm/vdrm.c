/*
 * Copyright © 2023 Google, Inc.
 * SPDX-License-Identifier: MIT
 */

#include "util/u_math.h"
#include "util/perf/cpu_trace.h"

#include "vdrm.h"

struct vdrm_device * vdrm_virtgpu_connect(int fd, uint32_t context_type);

struct vdrm_device *
vdrm_device_connect(int fd, uint32_t context_type)
{
   struct vdrm_device *vdev;

   // TODO vtest vs virtio..
   vdev = vdrm_virtgpu_connect(fd, context_type);
   if (!vdev)
      return NULL;

   simple_mtx_init(&vdev->rsp_lock, mtx_plain);
   simple_mtx_init(&vdev->eb_lock, mtx_plain);

   return vdev;
}

void
vdrm_device_close(struct vdrm_device *vdev)
{
   vdev->funcs->close(vdev);
   free(vdev);
}

uint32_t
vdrm_bo_create(struct vdrm_device *vdev, size_t size, uint32_t blob_flags,
               uint64_t blob_id, struct vdrm_ccmd_req *req)
{
   uint32_t handle;

   simple_mtx_lock(&vdev->eb_lock);

   /* flush any buffered cmds so they are seen by the host *prior* to
    * the cmds associated with bo creation.
    */
   vdev->funcs->flush_locked(vdev, NULL);

   req->seqno = ++vdev->next_seqno;

   handle = vdev->funcs->bo_create(vdev, size, blob_flags, blob_id, req);

   simple_mtx_unlock(&vdev->eb_lock);

   return handle;
}

void *
vdrm_alloc_rsp(struct vdrm_device *vdev, struct vdrm_ccmd_req *req, uint32_t sz)
{
   unsigned off;

   simple_mtx_lock(&vdev->rsp_lock);

   sz = align(sz, 8);

   if ((vdev->next_rsp_off + sz) >= vdev->rsp_mem_len)
      vdev->next_rsp_off = 0;

   off = vdev->next_rsp_off;
   vdev->next_rsp_off += sz;

   simple_mtx_unlock(&vdev->rsp_lock);

   req->rsp_off = off;

   struct vdrm_ccmd_rsp *rsp = (void *)&vdev->rsp_mem[off];
   rsp->len = sz;

   return rsp;
}

static int
enqueue_req(struct vdrm_device *vdev, struct vdrm_ccmd_req *req)
{
   simple_mtx_assert_locked(&vdev->eb_lock);

   req->seqno = ++vdev->next_seqno;

   if ((vdev->reqbuf_len + req->len) > sizeof(vdev->reqbuf)) {
      int ret = vdev->funcs->flush_locked(vdev, NULL);
      if (ret)
         return ret;
   }

   memcpy(&vdev->reqbuf[vdev->reqbuf_len], req, req->len);
   vdev->reqbuf_len += req->len;
   vdev->reqbuf_cnt++;

   return 0;
}

int
vdrm_execbuf(struct vdrm_device *vdev, struct vdrm_execbuf_params *p)
{
   int ret = 0;

   MESA_TRACE_FUNC();

   simple_mtx_lock(&vdev->eb_lock);

   ret = vdev->funcs->flush_locked(vdev, NULL);
   if (ret)
      goto out_unlock;

   ret = vdev->funcs->execbuf_locked(vdev, p, p->req, p->req->len);

out_unlock:
   simple_mtx_unlock(&vdev->eb_lock);

   return ret;
}

/**
 * Buffer/send a request cmd to host
 */
int
vdrm_send_req(struct vdrm_device *vdev, struct vdrm_ccmd_req *req, bool sync)
{
   MESA_TRACE_FUNC();

   uintptr_t fence = 0;
   int ret = 0;

   simple_mtx_lock(&vdev->eb_lock);
   ret = enqueue_req(vdev, req);

   if (ret || !sync)
      goto out_unlock;

   ret = vdev->funcs->flush_locked(vdev, &fence);

out_unlock:
   simple_mtx_unlock(&vdev->eb_lock);

   if (ret)
      return ret;

   if (sync) {
      MESA_TRACE_SCOPE("vdrm_execbuf sync");
      vdev->funcs->wait_fence(vdev, fence);
      vdrm_host_sync(vdev, req);
   }

   return 0;
}

int
vdrm_flush(struct vdrm_device *vdev)
{
   int ret = 0;

   MESA_TRACE_FUNC();

   simple_mtx_lock(&vdev->eb_lock);
   ret = vdev->funcs->flush_locked(vdev, NULL);
   simple_mtx_unlock(&vdev->eb_lock);

   return ret;
}

/**
 * Helper for fence/seqno comparisions which deals properly with rollover.
 * Returns true if fence 'a' is before fence 'b'
 */
static bool
fence_before(uint32_t a, uint32_t b)
{
   return (int32_t)(a - b) < 0;
}

/**
 * Wait until host has processed the specified request.
 */
void
vdrm_host_sync(struct vdrm_device *vdev, const struct vdrm_ccmd_req *req)
{
   while (fence_before(vdev->shmem->seqno, req->seqno))
      sched_yield();
}
