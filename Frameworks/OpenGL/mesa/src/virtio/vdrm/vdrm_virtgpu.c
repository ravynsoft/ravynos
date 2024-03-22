/*
 * Copyright © 2023 Google, Inc.
 * SPDX-License-Identifier: MIT
 */

#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <xf86drm.h>

#include "vdrm.h"

#include "drm-uapi/virtgpu_drm.h"
#include "util/libsync.h"
#include "util/log.h"
#include "util/perf/cpu_trace.h"


#define SHMEM_SZ 0x4000

#define virtgpu_ioctl(fd, name, args...) ({                          \
      MESA_TRACE_SCOPE(#name);                                       \
      int ret = drmIoctl((fd), DRM_IOCTL_ ## name, (args));          \
      ret;                                                           \
   })

struct virtgpu_device {
   struct vdrm_device base;
   uint32_t shmem_handle;
   int fd;
};
DEFINE_CAST(vdrm_device, virtgpu_device)

static int
virtgpu_execbuf_locked(struct vdrm_device *vdev, struct vdrm_execbuf_params *p,
                       void *command, unsigned size)
{
   struct virtgpu_device *vgdev = to_virtgpu_device(vdev);

   simple_mtx_assert_locked(&vdev->eb_lock);

   assert(size);

#define COND(bool, val) ((bool) ? (val) : 0)
   struct drm_virtgpu_execbuffer eb = {
         .flags = COND(p->needs_out_fence_fd, VIRTGPU_EXECBUF_FENCE_FD_OUT) |
                  COND(p->has_in_fence_fd, VIRTGPU_EXECBUF_FENCE_FD_IN) |
                  VIRTGPU_EXECBUF_RING_IDX,
         .size  = size,
         .command = (uintptr_t)command,
         .bo_handles = (uintptr_t)p->handles,
         .num_bo_handles = p->num_handles,
         .fence_fd = p->fence_fd,
         .ring_idx = p->ring_idx,
         .syncobj_stride = sizeof(struct drm_virtgpu_execbuffer_syncobj),
         .num_in_syncobjs = p->num_in_syncobjs,
         .num_out_syncobjs = p->num_out_syncobjs,
         .in_syncobjs = (uintptr_t)p->in_syncobjs,
         .out_syncobjs = (uintptr_t)p->out_syncobjs,
   };

   int ret = virtgpu_ioctl(vgdev->fd, VIRTGPU_EXECBUFFER, &eb);
   if (ret) {
      mesa_loge("EXECBUFFER failed: %s", strerror(errno));
      return ret;
   }

   if (p->needs_out_fence_fd)
      p->fence_fd = eb.fence_fd;

   return 0;
}

static int
virtgpu_flush_locked(struct vdrm_device *vdev, uintptr_t *fencep)
{
   int ret;

   simple_mtx_assert_locked(&vdev->eb_lock);

   if (!vdev->reqbuf_len)
      return 0;

   struct vdrm_execbuf_params p = {
      .needs_out_fence_fd = !!fencep,
   };
   ret = virtgpu_execbuf_locked(vdev, &p, vdev->reqbuf, vdev->reqbuf_len);
   if (ret)
      return ret;

   vdev->reqbuf_len = 0;
   vdev->reqbuf_cnt = 0;

   if (fencep)
      *fencep = p.fence_fd;

   return 0;
}

static void
virtgpu_wait_fence(struct vdrm_device *vdev, uintptr_t fence)
{
   int fence_fd = fence;

   sync_wait(fence_fd, -1);
   close(fence_fd);
}

static void
gem_close(struct virtgpu_device *vgdev, uint32_t handle)
{
   virtgpu_ioctl(vgdev->fd, GEM_CLOSE, &(struct drm_gem_close){
      .handle = handle,
   });
}

/**
 * Note, does _not_ de-duplicate handles
 */
static uint32_t
virtgpu_dmabuf_to_handle(struct vdrm_device *vdev, int fd)
{
   struct virtgpu_device *vgdev = to_virtgpu_device(vdev);
   uint32_t handle;
   int ret;

   ret = drmPrimeFDToHandle(vgdev->fd, fd, &handle);
   if (ret) {
      mesa_loge("dmabuf import failed: %s", strerror(errno));
      return 0;
   }

   return handle;
}

static uint32_t
virtgpu_handle_to_res_id(struct vdrm_device *vdev, uint32_t handle)
{
   struct virtgpu_device *vgdev = to_virtgpu_device(vdev);
   struct drm_virtgpu_resource_info args = {
         .bo_handle = handle,
   };
   int ret;

   ret = virtgpu_ioctl(vgdev->fd, VIRTGPU_RESOURCE_INFO, &args);
   if (ret) {
      mesa_loge("failed to get resource info: %s", strerror(errno));
      return 0;
   }

   return args.res_handle;
}

static uint32_t
virtgpu_bo_create(struct vdrm_device *vdev, size_t size, uint32_t blob_flags,
                  uint64_t blob_id, struct vdrm_ccmd_req *req)
{
   struct virtgpu_device *vgdev = to_virtgpu_device(vdev);
   struct drm_virtgpu_resource_create_blob args = {
         .blob_mem   = VIRTGPU_BLOB_MEM_HOST3D,
         .blob_flags = blob_flags,
         .size       = size,
         .cmd_size   = req->len,
         .cmd        = (uintptr_t)req,
         .blob_id    = blob_id,
   };
   int ret;

   simple_mtx_assert_locked(&vdev->eb_lock);

   ret = virtgpu_ioctl(vgdev->fd, VIRTGPU_RESOURCE_CREATE_BLOB, &args);
   if (ret) {
      mesa_loge("buffer allocation failed: %s", strerror(errno));
      return 0;
   }

   return args.bo_handle;
}

static int
map_handle(int fd, uint32_t handle, size_t size, void **map)
{
   struct drm_virtgpu_map req = {
      .handle = handle,
   };
   int ret;

   ret = virtgpu_ioctl(fd, VIRTGPU_MAP, &req);
   if (ret) {
      mesa_loge("VIRTGPU_MAP failed: %s", strerror(errno));
      return ret;
   }

   *map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, req.offset);
   if (*map == MAP_FAILED) {
      mesa_loge("failed to map handle: %s", strerror(errno));
      return -1;
   }

   return 0;
}

static int
virtgpu_bo_wait(struct vdrm_device *vdev, uint32_t handle)
{
   struct virtgpu_device *vgdev = to_virtgpu_device(vdev);
   struct drm_virtgpu_3d_wait args = {
         .handle = handle,
   };
   int ret;

   /* Side note, this ioctl is defined as IO_WR but should be IO_W: */
   ret = virtgpu_ioctl(vgdev->fd, VIRTGPU_WAIT, &args);
   if (ret && errno == EBUSY)
      return -EBUSY;

   return 0;
}

static void *
virtgpu_bo_map(struct vdrm_device *vdev, uint32_t handle, size_t size)
{
   struct virtgpu_device *vgdev = to_virtgpu_device(vdev);
   void *map;
   int ret;

   ret = map_handle(vgdev->fd, handle, size, &map);
   if (ret)
      return NULL;

   return map;
}

static int
virtgpu_bo_export_dmabuf(struct vdrm_device *vdev, uint32_t handle)
{
   struct virtgpu_device *vgdev = to_virtgpu_device(vdev);
   int ret, fd;

   ret = drmPrimeHandleToFD(vgdev->fd, handle, DRM_CLOEXEC | DRM_RDWR, &fd);
   if (ret) {
      mesa_loge("dmabuf export failed: %s", strerror(errno));
      return ret;
   }

   return fd;
}

static void
virtgpu_bo_close(struct vdrm_device *vdev, uint32_t handle)
{
   /* Flush any buffered commands first, so the detach_resource doesn't
    * overtake any buffered ccmd which references the resource:
    */
   if (vdev->reqbuf_len) {
      simple_mtx_lock(&vdev->eb_lock);
      virtgpu_flush_locked(vdev, NULL);
      simple_mtx_unlock(&vdev->eb_lock);
   }

   gem_close(to_virtgpu_device(vdev), handle);
}

static void
virtgpu_close(struct vdrm_device *vdev)
{
   struct virtgpu_device *vgdev = to_virtgpu_device(vdev);

   munmap(vdev->shmem, SHMEM_SZ);
   gem_close(vgdev, vgdev->shmem_handle);
}

static const struct vdrm_device_funcs funcs = {
   .flush_locked = virtgpu_flush_locked,
   .wait_fence = virtgpu_wait_fence,
   .execbuf_locked = virtgpu_execbuf_locked,
   .dmabuf_to_handle = virtgpu_dmabuf_to_handle,
   .handle_to_res_id = virtgpu_handle_to_res_id,
   .bo_create = virtgpu_bo_create,
   .bo_wait = virtgpu_bo_wait,
   .bo_map = virtgpu_bo_map,
   .bo_export_dmabuf = virtgpu_bo_export_dmabuf,
   .bo_close = virtgpu_bo_close,
   .close = virtgpu_close,
};

static int
get_capset(int fd, struct virgl_renderer_capset_drm *caps)
{
   struct drm_virtgpu_get_caps args = {
         .cap_set_id = VIRGL_RENDERER_CAPSET_DRM,
         .cap_set_ver = 0,
         .addr = (uintptr_t)caps,
         .size = sizeof(*caps),
   };

   memset(caps, 0, sizeof(*caps));

   return virtgpu_ioctl(fd, VIRTGPU_GET_CAPS, &args);
}

static int
set_context(int fd)
{
   struct drm_virtgpu_context_set_param params[] = {
         { VIRTGPU_CONTEXT_PARAM_CAPSET_ID, VIRGL_RENDERER_CAPSET_DRM },
         { VIRTGPU_CONTEXT_PARAM_NUM_RINGS, 64 },
   };
   struct drm_virtgpu_context_init args = {
      .num_params = ARRAY_SIZE(params),
      .ctx_set_params = (uintptr_t)params,
   };

   return virtgpu_ioctl(fd, VIRTGPU_CONTEXT_INIT, &args);
}

static int
init_shmem(struct virtgpu_device *vgdev)
{
   struct vdrm_device *vdev = &vgdev->base;
   struct drm_virtgpu_resource_create_blob args = {
         .blob_mem   = VIRTGPU_BLOB_MEM_HOST3D,
         .blob_flags = VIRTGPU_BLOB_FLAG_USE_MAPPABLE,
         .size       = SHMEM_SZ,
         .blob_id    = 0,
   };
   int ret;

   ret = virtgpu_ioctl(vgdev->fd, VIRTGPU_RESOURCE_CREATE_BLOB, &args);
   if (ret) {
      mesa_logi("failed to allocate shmem buffer: %s", strerror(errno));
      return ret;
   }

   vgdev->shmem_handle = args.bo_handle;

   ret = map_handle(vgdev->fd, vgdev->shmem_handle, args.size, (void **)&vdev->shmem);
   if (ret) {
      gem_close(vgdev, vgdev->shmem_handle);
      vgdev->shmem_handle = 0;
      return ret;
   }

   uint32_t offset = vdev->shmem->rsp_mem_offset;
   vdev->rsp_mem_len = args.size - offset;
   vdev->rsp_mem = &((uint8_t *)vdev->shmem)[offset];

   return 0;
}

struct vdrm_device * vdrm_virtgpu_connect(int fd, uint32_t context_type);

struct vdrm_device *
vdrm_virtgpu_connect(int fd, uint32_t context_type)
{
   struct virgl_renderer_capset_drm caps;
   struct virtgpu_device *vgdev;
   struct vdrm_device *vdev;
   int ret;

   ret = get_capset(fd, &caps);
   if (ret) {
      mesa_logi("could not get caps: %s", strerror(errno));
      return NULL;
   }

   if (caps.context_type != context_type) {
      mesa_logi("wrong context_type: %u", caps.context_type);
      return NULL;
   }

   ret = set_context(fd);
   if (ret) {
      mesa_logi("Could not set context type: %s", strerror(errno));
      return NULL;
   }

   vgdev = calloc(1, sizeof(*vgdev));
   if (!vgdev)
      return NULL;

   vgdev->fd = fd;

   ret = init_shmem(vgdev);
   if (ret) {
      free(vgdev);
      return NULL;
   }

   vdev = &vgdev->base;
   vdev->caps = caps;
   vdev->funcs = &funcs;

   return vdev;
}
