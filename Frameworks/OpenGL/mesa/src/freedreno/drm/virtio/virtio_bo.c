/*
 * Copyright © 2022 Google, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "util/libsync.h"

#include "virtio_priv.h"

static void *
virtio_bo_mmap(struct fd_bo *bo)
{
   struct vdrm_device *vdrm = to_virtio_device(bo->dev)->vdrm;
   struct virtio_bo *virtio_bo = to_virtio_bo(bo);

   /* If we have uploaded, we need to wait for host to handle that
    * before we can allow guest-side CPU access:
    */
   if (virtio_bo->has_upload_seqno) {

      virtio_bo->has_upload_seqno = false;

      vdrm_flush(vdrm);
      vdrm_host_sync(vdrm, &(struct vdrm_ccmd_req) {
         .seqno = virtio_bo->upload_seqno,
      });
   }

   return vdrm_bo_map(vdrm, bo->handle, bo->size);
}

static int
virtio_bo_cpu_prep(struct fd_bo *bo, struct fd_pipe *pipe, uint32_t op)
{
   MESA_TRACE_FUNC();
   struct vdrm_device *vdrm = to_virtio_device(bo->dev)->vdrm;
   int ret;

   /*
    * Wait first in the guest, to avoid a blocking call in host.
    * If implicit sync it used, we still need to *also* wait in
    * host, if it is a shared buffer, because the guest doesn't
    * know about usage of the bo in the host (or other guests).
    */

   ret = vdrm_bo_wait(vdrm, bo->handle);
   if (ret)
      goto out;

   /*
    * The buffer could be shared with other things on the host side
    * so have to poll the host.  But we only get here with the shared
    * buffers plus implicit sync.  Hopefully that is rare enough.
    */

   struct msm_ccmd_gem_cpu_prep_req req = {
         .hdr = MSM_CCMD(GEM_CPU_PREP, sizeof(req)),
         .res_id = to_virtio_bo(bo)->res_id,
         .op = op,
   };
   struct msm_ccmd_gem_cpu_prep_rsp *rsp;

   /* We can't do a blocking wait in the host, so we have to poll: */
   do {
      rsp = vdrm_alloc_rsp(vdrm, &req.hdr, sizeof(*rsp));

      ret = vdrm_send_req(vdrm, &req.hdr, true);
      if (ret)
         goto out;

      ret = rsp->ret;
   } while (ret == -EBUSY);

out:
   return ret;
}

static int
virtio_bo_madvise(struct fd_bo *bo, int willneed)
{
   /* TODO:
    * Currently unsupported, synchronous WILLNEED calls would introduce too
    * much latency.. ideally we'd keep state in the guest and only flush
    * down to host when host is under memory pressure.  (Perhaps virtio-balloon
    * could signal this?)
    */
   return willneed;
}

static uint64_t
virtio_bo_iova(struct fd_bo *bo)
{
   /* The shmem bo is allowed to have no iova, as it is only used for
    * guest<->host communications:
    */
   assert(bo->iova || (to_virtio_bo(bo)->blob_id == 0));
   return bo->iova;
}

static void
virtio_bo_set_name(struct fd_bo *bo, const char *fmt, va_list ap)
{
   char name[32];
   int sz;

   /* Note, we cannot set name on the host for the shmem bo, as
    * that isn't a real gem obj on the host side.. not having
    * an iova is a convenient way to detect this case:
    */
   if (!bo->iova)
      return;

   sz = vsnprintf(name, sizeof(name), fmt, ap);
   sz = MIN2(sz, sizeof(name));

   unsigned req_len = sizeof(struct msm_ccmd_gem_set_name_req) + align(sz, 4);

   uint8_t buf[req_len];
   struct msm_ccmd_gem_set_name_req *req = (void *)buf;

   req->hdr = MSM_CCMD(GEM_SET_NAME, req_len);
   req->res_id = to_virtio_bo(bo)->res_id;
   req->len = sz;

   memcpy(req->payload, name, sz);

   vdrm_send_req(to_virtio_device(bo->dev)->vdrm, &req->hdr, false);
}

static int
virtio_bo_dmabuf(struct fd_bo *bo)
{
   struct virtio_device *virtio_dev = to_virtio_device(bo->dev);

   return vdrm_bo_export_dmabuf(virtio_dev->vdrm, bo->handle);
}

static void
bo_upload(struct fd_bo *bo, unsigned off, void *src, unsigned len)
{
   MESA_TRACE_FUNC();
   unsigned req_len = sizeof(struct msm_ccmd_gem_upload_req) + align(len, 4);
   struct virtio_bo *virtio_bo = to_virtio_bo(bo);

   uint8_t buf[req_len];
   struct msm_ccmd_gem_upload_req *req = (void *)buf;

   req->hdr = MSM_CCMD(GEM_UPLOAD, req_len);
   req->res_id = virtio_bo->res_id;
   req->pad = 0;
   req->off = off;
   req->len = len;

   memcpy(req->payload, src, len);

   vdrm_send_req(to_virtio_device(bo->dev)->vdrm, &req->hdr, false);

   virtio_bo->upload_seqno = req->hdr.seqno;
   virtio_bo->has_upload_seqno = true;
}

static void
virtio_bo_upload(struct fd_bo *bo, void *src, unsigned off, unsigned len)
{
   while (len > 0) {
      unsigned sz = MIN2(len, 0x1000);
      bo_upload(bo, off, src, sz);
      off += sz;
      src += sz;
      len -= sz;
   }
}

/**
 * For recently allocated buffers, an immediate mmap would stall waiting
 * for the host to handle the allocation and map to the guest, which
 * could take a few ms.  So for small transfers to recently allocated
 * buffers, we'd prefer to use the upload path instead.
 */
static bool
virtio_bo_prefer_upload(struct fd_bo *bo, unsigned len)
{
   struct virtio_bo *virtio_bo = to_virtio_bo(bo);

   /* If we've already taken the hit of mmap'ing the buffer, then no reason
    * to take the upload path:
    */
   if (bo->map)
      return false;

   if (len > 0x4000)
      return false;

   int64_t age_ns = os_time_get_nano() - virtio_bo->alloc_time_ns;
   if (age_ns > 5000000)
      return false;

   return true;
}

static void
set_iova(struct fd_bo *bo, uint64_t iova)
{
   struct msm_ccmd_gem_set_iova_req req = {
         .hdr = MSM_CCMD(GEM_SET_IOVA, sizeof(req)),
         .res_id = to_virtio_bo(bo)->res_id,
         .iova = iova,
   };

   vdrm_send_req(to_virtio_device(bo->dev)->vdrm, &req.hdr, false);
}

static void
virtio_bo_finalize(struct fd_bo *bo)
{
   /* Release iova by setting to zero: */
   if (bo->iova) {
      set_iova(bo, 0);

      virtio_dev_free_iova(bo->dev, bo->iova, bo->size);
   }
}

static const struct fd_bo_funcs funcs = {
   .map = virtio_bo_mmap,
   .cpu_prep = virtio_bo_cpu_prep,
   .madvise = virtio_bo_madvise,
   .iova = virtio_bo_iova,
   .set_name = virtio_bo_set_name,
   .dmabuf = virtio_bo_dmabuf,
   .upload = virtio_bo_upload,
   .prefer_upload = virtio_bo_prefer_upload,
   .finalize = virtio_bo_finalize,
   .destroy = fd_bo_fini_common,
};

static struct fd_bo *
bo_from_handle(struct fd_device *dev, uint32_t size, uint32_t handle)
{
   struct virtio_device *virtio_dev = to_virtio_device(dev);
   struct virtio_bo *virtio_bo;
   struct fd_bo *bo;

   virtio_bo = calloc(1, sizeof(*virtio_bo));
   if (!virtio_bo)
      return NULL;

   virtio_bo->alloc_time_ns = os_time_get_nano();

   bo = &virtio_bo->base;

   /* Note we need to set these because allocation_wait_execute() could
    * run before bo_init_commont():
    */
   bo->dev = dev;
   p_atomic_set(&bo->refcnt, 1);

   bo->size = size;
   bo->funcs = &funcs;
   bo->handle = handle;

   /* Don't assume we can mmap an imported bo: */
   bo->alloc_flags = FD_BO_NOMAP;

   virtio_bo->res_id = vdrm_handle_to_res_id(virtio_dev->vdrm, handle);

   fd_bo_init_common(bo, dev);

   return bo;
}

/* allocate a new buffer object from existing handle */
struct fd_bo *
virtio_bo_from_handle(struct fd_device *dev, uint32_t size, uint32_t handle)
{
   struct fd_bo *bo = bo_from_handle(dev, size, handle);

   if (!bo)
      return NULL;

   bo->iova = virtio_dev_alloc_iova(dev, size);
   if (!bo->iova)
      goto fail;

   set_iova(bo, bo->iova);

   return bo;

fail:
   virtio_bo_finalize(bo);
   fd_bo_fini_common(bo);
   return NULL;
}

/* allocate a buffer object: */
struct fd_bo *
virtio_bo_new(struct fd_device *dev, uint32_t size, uint32_t flags)
{
   struct virtio_device *virtio_dev = to_virtio_device(dev);
   struct msm_ccmd_gem_new_req req = {
         .hdr = MSM_CCMD(GEM_NEW, sizeof(req)),
         .size = size,
   };

   if (flags & FD_BO_SCANOUT)
      req.flags |= MSM_BO_SCANOUT;

   if (flags & FD_BO_GPUREADONLY)
      req.flags |= MSM_BO_GPU_READONLY;

   if (flags & FD_BO_CACHED_COHERENT) {
      req.flags |= MSM_BO_CACHED_COHERENT;
   } else {
      req.flags |= MSM_BO_WC;
   }

   uint32_t blob_flags = 0;
   if (flags & (FD_BO_SHARED | FD_BO_SCANOUT)) {
      blob_flags = VIRTGPU_BLOB_FLAG_USE_CROSS_DEVICE |
            VIRTGPU_BLOB_FLAG_USE_SHAREABLE;
   }

   if (!(flags & FD_BO_NOMAP)) {
      blob_flags |= VIRTGPU_BLOB_FLAG_USE_MAPPABLE;
   }

   uint32_t blob_id = p_atomic_inc_return(&virtio_dev->next_blob_id);

   /* tunneled cmds are processed separately on host side,
    * before the renderer->get_blob() callback.. the blob_id
    * is used to link the created bo to the get_blob() call
    */
   req.blob_id = blob_id;
   req.iova = virtio_dev_alloc_iova(dev, size);
   if (!req.iova)
      goto fail;

   uint32_t handle =
      vdrm_bo_create(virtio_dev->vdrm, size, blob_flags, blob_id, &req.hdr);
   if (!handle)
      goto fail;

   struct fd_bo *bo = bo_from_handle(dev, size, handle);
   struct virtio_bo *virtio_bo = to_virtio_bo(bo);

   virtio_bo->blob_id = blob_id;
   bo->iova = req.iova;

   return bo;

fail:
   if (req.iova)
      virtio_dev_free_iova(dev, req.iova, size);
   return NULL;
}
