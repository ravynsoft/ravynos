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

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util/libsync.h"
#include "util/u_process.h"

#include "virtio_priv.h"


static int virtio_execbuf_flush(struct fd_device *dev);

static void
virtio_device_destroy(struct fd_device *dev)
{
   struct virtio_device *virtio_dev = to_virtio_device(dev);

   util_vma_heap_finish(&virtio_dev->address_space);
}

static uint32_t
virtio_handle_from_dmabuf(struct fd_device *dev, int fd)
{
   struct virtio_device *virtio_dev = to_virtio_device(dev);

   return vdrm_dmabuf_to_handle(virtio_dev->vdrm, fd);
}

static void
virtio_close_handle(struct fd_bo *bo)
{
   struct virtio_device *virtio_dev = to_virtio_device(bo->dev);

   vdrm_bo_close(virtio_dev->vdrm, bo->handle);
}

static const struct fd_device_funcs funcs = {
   .bo_new = virtio_bo_new,
   .bo_from_handle = virtio_bo_from_handle,
   .handle_from_dmabuf = virtio_handle_from_dmabuf,
   .bo_from_dmabuf = fd_bo_from_dmabuf_drm,
   .bo_close_handle = virtio_close_handle,
   .pipe_new = virtio_pipe_new,
   .flush = virtio_execbuf_flush,
   .destroy = virtio_device_destroy,
};

static void
set_debuginfo(struct fd_device *dev)
{
   const char *comm = util_get_process_name();
   static char cmdline[0x1000+1];
   int fd = open("/proc/self/cmdline", O_RDONLY);
   if (fd < 0)
      return;

   int n = read(fd, cmdline, sizeof(cmdline) - 1);
   if (n < 0)
      return;

   /* arguments are separated by NULL, convert to spaces: */
   for (int i = 0; i < n; i++) {
      if (cmdline[i] == '\0') {
         cmdline[i] = ' ';
      }
   }

   cmdline[n] = '\0';

   unsigned comm_len = strlen(comm) + 1;
   unsigned cmdline_len = strlen(cmdline) + 1;

   struct msm_ccmd_set_debuginfo_req *req;

   unsigned req_len = align(sizeof(*req) + comm_len + cmdline_len, 4);

   req = malloc(req_len);

   req->hdr         = MSM_CCMD(SET_DEBUGINFO, req_len);
   req->comm_len    = comm_len;
   req->cmdline_len = cmdline_len;

   memcpy(&req->payload[0], comm, comm_len);
   memcpy(&req->payload[comm_len], cmdline, cmdline_len);

   vdrm_send_req(to_virtio_device(dev)->vdrm, &req->hdr, false);

   free(req);
}

struct fd_device *
virtio_device_new(int fd, drmVersionPtr version)
{
   struct virgl_renderer_capset_drm caps;
   struct virtio_device *virtio_dev;
   struct vdrm_device *vdrm;
   struct fd_device *dev;

   STATIC_ASSERT(FD_BO_PREP_READ == MSM_PREP_READ);
   STATIC_ASSERT(FD_BO_PREP_WRITE == MSM_PREP_WRITE);
   STATIC_ASSERT(FD_BO_PREP_NOSYNC == MSM_PREP_NOSYNC);

   /* Debug option to force fallback to virgl: */
   if (debug_get_bool_option("FD_NO_VIRTIO", false))
      return NULL;

   vdrm = vdrm_device_connect(fd, VIRTGPU_DRM_CONTEXT_MSM);
   if (!vdrm) {
      INFO_MSG("could not connect vdrm");
      return NULL;
   }

   caps = vdrm->caps;

   INFO_MSG("wire_format_version: %u", caps.wire_format_version);
   INFO_MSG("version_major:       %u", caps.version_major);
   INFO_MSG("version_minor:       %u", caps.version_minor);
   INFO_MSG("version_patchlevel:  %u", caps.version_patchlevel);
   INFO_MSG("has_cached_coherent: %u", caps.u.msm.has_cached_coherent);
   INFO_MSG("va_start:            0x%0"PRIx64, caps.u.msm.va_start);
   INFO_MSG("va_size:             0x%0"PRIx64, caps.u.msm.va_size);
   INFO_MSG("gpu_id:              %u", caps.u.msm.gpu_id);
   INFO_MSG("gmem_size:           %u", caps.u.msm.gmem_size);
   INFO_MSG("gmem_base:           0x%0" PRIx64, caps.u.msm.gmem_base);
   INFO_MSG("chip_id:             0x%0" PRIx64, caps.u.msm.chip_id);
   INFO_MSG("max_freq:            %u", caps.u.msm.max_freq);

   if (caps.wire_format_version != 2) {
      ERROR_MSG("Unsupported protocol version: %u", caps.wire_format_version);
      goto error;
   }

   if ((caps.version_major != 1) || (caps.version_minor < FD_VERSION_SOFTPIN)) {
      ERROR_MSG("unsupported version: %u.%u.%u", caps.version_major,
                caps.version_minor, caps.version_patchlevel);
      goto error;
   }

   if (!caps.u.msm.va_size) {
      ERROR_MSG("No address space");
      goto error;
   }

   virtio_dev = calloc(1, sizeof(*virtio_dev));
   if (!virtio_dev)
      goto error;

   dev = &virtio_dev->base;
   dev->funcs = &funcs;
   dev->fd = fd;
   dev->version = caps.version_minor;
   dev->has_cached_coherent = caps.u.msm.has_cached_coherent;

   p_atomic_set(&virtio_dev->next_blob_id, 1);
   virtio_dev->shmem = to_msm_shmem(vdrm->shmem);
   virtio_dev->vdrm = vdrm;

   util_queue_init(&dev->submit_queue, "sq", 8, 1, 0, NULL);

   dev->bo_size = sizeof(struct virtio_bo);

   set_debuginfo(dev);

   util_vma_heap_init(&virtio_dev->address_space,
                      caps.u.msm.va_start,
                      caps.u.msm.va_size);
   simple_mtx_init(&virtio_dev->address_space_lock, mtx_plain);

   return dev;

error:
   vdrm_device_close(vdrm);
   return NULL;
}

static int
virtio_execbuf_flush(struct fd_device *dev)
{
   return vdrm_flush(to_virtio_device(dev)->vdrm);
}

/**
 * Helper for simple pass-thru ioctls
 */
int
virtio_simple_ioctl(struct fd_device *dev, unsigned cmd, void *_req)
{
   MESA_TRACE_FUNC();
   struct vdrm_device *vdrm = to_virtio_device(dev)->vdrm;
   unsigned req_len = sizeof(struct msm_ccmd_ioctl_simple_req);
   unsigned rsp_len = sizeof(struct msm_ccmd_ioctl_simple_rsp);

   req_len += _IOC_SIZE(cmd);
   if (cmd & IOC_OUT)
      rsp_len += _IOC_SIZE(cmd);

   uint8_t buf[req_len];
   struct msm_ccmd_ioctl_simple_req *req = (void *)buf;
   struct msm_ccmd_ioctl_simple_rsp *rsp;

   req->hdr = MSM_CCMD(IOCTL_SIMPLE, req_len);
   req->cmd = cmd;
   memcpy(req->payload, _req, _IOC_SIZE(cmd));

   rsp = vdrm_alloc_rsp(vdrm, &req->hdr, rsp_len);

   int ret = vdrm_send_req(vdrm, &req->hdr, true);

   if (cmd & IOC_OUT)
      memcpy(_req, rsp->payload, _IOC_SIZE(cmd));

   ret = rsp->ret;

   return ret;
}
