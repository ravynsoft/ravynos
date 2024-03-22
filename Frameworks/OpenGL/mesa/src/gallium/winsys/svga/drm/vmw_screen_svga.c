/**********************************************************
 * Copyright 2009-2023 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

/**
 * @file
 * This file implements the SVGA interface into this winsys, defined
 * in drivers/svga/svga_winsys.h.
 *
 * @author Keith Whitwell
 * @author Jose Fonseca
 */

#include <libsync.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "svga_cmd.h"
#include "svga3d_caps.h"

#include "c11/threads.h"
#include "util/os_file.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "pipebuffer/pb_buffer.h"
#include "pipebuffer/pb_bufmgr.h"
#include "svga_winsys.h"
#include "vmw_context.h"
#include "vmw_screen.h"
#include "vmw_surface.h"
#include "vmw_buffer.h"
#include "vmw_fence.h"
#include "vmw_msg.h"
#include "vmw_shader.h"
#include "vmw_query.h"
#include "vmwgfx_drm.h"
#include "svga3d_surfacedefs.h"
#include "xf86drm.h"

/**
 * Try to get a surface backing buffer from the cache
 * if it's this size or smaller.
 */
#define VMW_TRY_CACHED_SIZE (2*1024*1024)

#ifdef VMX86_STATS
static const char* const vmw_svga_winsys_stats_count_names[] = {
   SVGA_STATS_COUNT_NAMES
};

static const char* const vmw_svga_winsys_stats_time_names[] = {
   SVGA_STATS_TIME_NAMES
};

/*
 * It's imperative that the above two arrays are const, so that the next
 * function can be optimized to a constant.
 */
static inline size_t
vmw_svga_winsys_stats_names_len(void)
{
   size_t i, res = 0;
   for (i = 0; i < ARRAY_SIZE(vmw_svga_winsys_stats_count_names); ++i)
      res += strlen(vmw_svga_winsys_stats_count_names[i]) + 1;
   for (i = 0; i < ARRAY_SIZE(vmw_svga_winsys_stats_time_names); ++i)
      res += strlen(vmw_svga_winsys_stats_time_names[i]) + 1;
   return res;
}

typedef struct Atomic_uint64 {
   uint64_t value;
} Atomic_uint64;

typedef struct MKSGuestStatCounter {
   Atomic_uint64 count;
} MKSGuestStatCounter;

typedef struct MKSGuestStatCounterTime {
   MKSGuestStatCounter counter;
   Atomic_uint64 selfCycles;
   Atomic_uint64 totalCycles;
} MKSGuestStatCounterTime;

#define MKS_GUEST_STAT_FLAG_NONE    0
#define MKS_GUEST_STAT_FLAG_TIME    (1U << 0)

typedef struct MKSGuestStatInfoEntry {
   alignas(32) union {
      const char *s;
      uint64_t u;
   } name;
   union {
      const char *s;
      uint64_t u;
   } description;
   uint64_t flags;
   union {
      MKSGuestStatCounter *counter;
      MKSGuestStatCounterTime *counterTime;
      uint64_t u;
   } stat;
} MKSGuestStatInfoEntry;
static_assert(alignof(struct MKSGuestStatInfoEntry) == 32, "");

static thread_local struct svga_winsys_stats_timeframe *mksstat_tls_global = NULL;

#define ALIGN(x, power_of_two) (((x) + (power_of_two) - 1) & ~((power_of_two) - 1))

static const size_t mksstat_area_size_info = sizeof(MKSGuestStatInfoEntry) * (SVGA_STATS_COUNT_MAX + SVGA_STATS_TIME_MAX);
static const size_t mksstat_area_size_stat = sizeof(MKSGuestStatCounter) * SVGA_STATS_COUNT_MAX +
                                             sizeof(MKSGuestStatCounterTime) * SVGA_STATS_TIME_MAX;

size_t
vmw_svga_winsys_stats_len(void)
{
   const size_t pg_size = getpagesize();
   const size_t area_size_stat_pg = align_uintptr(mksstat_area_size_stat, pg_size);
   const size_t area_size_info_pg = align_uintptr(mksstat_area_size_info, pg_size);
   const size_t area_size_strs = vmw_svga_winsys_stats_names_len();
   const size_t area_size = area_size_stat_pg + area_size_info_pg + area_size_strs;

   return area_size;
}

/**
 * vmw_mksstat_get_pstat: Computes the address of the MKSGuestStatCounter
 * array from the address of the base page.
 *
 * @page_addr: Pointer to the base page.
 * @page_size: Size of page.
 * Return: Pointer to the MKSGuestStatCounter array.
 */

static inline MKSGuestStatCounter *
vmw_mksstat_get_pstat(uint8_t *page_addr, size_t page_size)
{
   return (MKSGuestStatCounter *)page_addr;
}

/**
 * vmw_mksstat_get_pstat_time: Computes the address of the MKSGuestStatCounterTime
 * array from the address of the base page.
 *
 * @page_addr: Pointer to the base page.
 * @page_size: Size of page.
 * Return: Pointer to the MKSGuestStatCounterTime array.
 */

static inline MKSGuestStatCounterTime *
vmw_mksstat_get_pstat_time(uint8_t *page_addr, size_t page_size)
{
   return (MKSGuestStatCounterTime *)(page_addr + sizeof(MKSGuestStatCounter) * SVGA_STATS_COUNT_MAX);
}

/**
 * vmw_mksstat_get_pinfo: Computes the address of the MKSGuestStatInfoEntry
 * array from the address of the base page.
 *
 * @page_addr: Pointer to the base page.
 * @page_size: Size of page.
 * Return: Pointer to the MKSGuestStatInfoEntry array.
 */

static inline MKSGuestStatInfoEntry *
vmw_mksstat_get_pinfo(uint8_t *page_addr, size_t page_size)
{
   const size_t area_size_stat_pg = align_uintptr(mksstat_area_size_stat, page_size);
   return (MKSGuestStatInfoEntry *)(page_addr + area_size_stat_pg);
}

/**
 * vmw_mksstat_get_pstrs: Computes the address of the mksGuestStat strings
 * sequence from the address of the base page.
 *
 * @page_addr: Pointer to the base page.
 * @page_size: Size of page.
 * Return: Pointer to the mksGuestStat strings sequence.
 */

static inline char *
vmw_mksstat_get_pstrs(uint8_t *page_addr, const size_t page_size)
{
   const size_t area_size_info_pg = align_uintptr(mksstat_area_size_info, page_size);
   const size_t area_size_stat_pg = align_uintptr(mksstat_area_size_stat, page_size);
   return (char *)(page_addr + area_size_info_pg + area_size_stat_pg);
}

/**
 * Add all known mksGuestStats counters for tracking by the host.
 */
static int
vmw_svga_winsys_add_stats(struct vmw_winsys_screen *vws, int slot)
{
   const size_t pg_size = getpagesize();
   const size_t area_size = vmw_svga_winsys_stats_len();

   MKSGuestStatInfoEntry *pinfo;
   MKSGuestStatCounter *pstat;
   MKSGuestStatCounterTime *pstatTime;
   char *pstrs;
   uint64_t id;
   size_t i;

   /* Allocate a contiguous area of pages for all info entries, counters and strings. */
   void *area = mmap(NULL, area_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED | MAP_NORESERVE, -1, 0);

   if (area == MAP_FAILED) {
      fprintf(stderr, "%s could not mmap memory: %s\n", __func__, strerror(errno));
      return -1;
   }

   pinfo = vmw_mksstat_get_pinfo(area, pg_size);
   pstat = vmw_mksstat_get_pstat(area, pg_size);
   pstrs = vmw_mksstat_get_pstrs(area, pg_size);
   pstatTime = vmw_mksstat_get_pstat_time(area, pg_size);

   if (mlock(area, area_size)) {
      fprintf(stderr, "%s could not mlock memory: %s\n", __func__, strerror(errno));
      goto error;
   }

   /* Suppress pages copy-on-write; for MAP_SHARED this should not really matter; it would if we go MAP_PRIVATE */
   if (madvise(area, area_size, MADV_DONTFORK)) {
      fprintf(stderr, "%s could not madvise memory: %s\n", __func__, strerror(errno));
      goto error;
   }

   /* Set up regular counters first */
   for (i = 0; i < SVGA_STATS_COUNT_MAX; ++i) {
      pinfo->name.s = pstrs;
      pinfo->description.s = pstrs;
      pinfo->flags = MKS_GUEST_STAT_FLAG_NONE;
      pinfo->stat.counter = pstat + i;
      pinfo++;

      memcpy(pstrs, vmw_svga_winsys_stats_count_names[i], strlen(vmw_svga_winsys_stats_count_names[i]));
      pstrs += strlen(vmw_svga_winsys_stats_count_names[i]) + 1;
   }

   /* Set up time counters second */
   for (i = 0; i < SVGA_STATS_TIME_MAX; ++i) {
      pinfo->name.s = pstrs;
      pinfo->description.s = pstrs;
      pinfo->flags = MKS_GUEST_STAT_FLAG_TIME;
      pinfo->stat.counterTime = pstatTime + i;
      pinfo++;

      memcpy(pstrs, vmw_svga_winsys_stats_time_names[i], strlen(vmw_svga_winsys_stats_time_names[i]));
      pstrs += strlen(vmw_svga_winsys_stats_time_names[i]) + 1;
   }

   { /* ioctl(DRM_VMW_MKSSTAT_ADD) */
      char desc[64];
      snprintf(desc, sizeof(desc) - 1, "vmw_winsys_screen=%p pid=%d", vws, gettid());

      struct drm_vmw_mksstat_add_arg arg = {
         .stat = (uintptr_t)pstat,
         .info = (uintptr_t)vmw_mksstat_get_pinfo(area, pg_size),
         .strs = (uintptr_t)vmw_mksstat_get_pstrs(area, pg_size),
         .stat_len = mksstat_area_size_stat,
         .info_len = mksstat_area_size_info,
         .strs_len = vmw_svga_winsys_stats_names_len(),
         .description = (uintptr_t)desc,
         .id = -1U
      };
      if (drmCommandWriteRead(vws->ioctl.drm_fd, DRM_VMW_MKSSTAT_ADD, &arg, sizeof(arg))) {
         fprintf(stderr, "%s could not ioctl: %s\n", __func__, strerror(errno));
         goto error;
      }
      id = arg.id;
   }

   vws->mksstat_tls[slot].stat_pages = area;
   vws->mksstat_tls[slot].stat_id = id;
   /* Don't update vws->mksstat_tls[].pid as it's reserved. */
   return 0;

error:
   munmap(area, area_size);
   return -1;
}

/**
 * Acquire a mksstat TLS slot making it immutable by other parties.
 */
static inline int
vmw_winsys_screen_mksstat_acq_slot(struct vmw_winsys_screen *vws)
{
   const pid_t pid = gettid();
   const size_t base = (size_t)pid % ARRAY_SIZE(vws->mksstat_tls);
   size_t i;

   if (mksstat_tls_global && vmw_winsys_screen(mksstat_tls_global->sws) == vws) {
      const size_t slot = mksstat_tls_global->slot;
      uint32_t expecpid = pid;
      if (__atomic_compare_exchange_n(&vws->mksstat_tls[slot].pid, &expecpid, -1U, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
         return (int)slot;
   }

   for (i = 0; i < ARRAY_SIZE(vws->mksstat_tls); ++i) {
      const size_t slot = (i + base) % ARRAY_SIZE(vws->mksstat_tls);
      uint32_t expecpid = pid;
      uint32_t expected = 0;

      /* Check if pid is already present */
      if (__atomic_compare_exchange_n(&vws->mksstat_tls[slot].pid, &expecpid, -1U, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
         return (int)slot;

      /* Try to set up a new mksstat for this pid */
      if (__atomic_compare_exchange_n(&vws->mksstat_tls[slot].pid, &expected, -1U, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
         const int ret = vmw_svga_winsys_add_stats(vws, slot);

         if (!ret)
            return (int)slot;

         __atomic_store_n(&vws->mksstat_tls[slot].pid, 0, __ATOMIC_RELEASE);
         return ret;
      }
   }

   return -1;
}

/**
 * Release a mksstat TLS slot -- caller still owns the slot but now it is erasable by other parties.
 */
static inline void
vmw_winsys_screen_mksstat_rel_slot(struct vmw_winsys_screen *vws, int slot)
{
   assert(slot < ARRAY_SIZE(vws->mksstat_tls));

   __atomic_store_n(&vws->mksstat_tls[slot].pid, gettid(), __ATOMIC_RELEASE);
}

static inline uint64_t
rdtsc(void)
{
   uint32_t hi, lo;
   __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
   return (uint64_t)lo | ((uint64_t)hi << 32);
}

#endif /* VMX86_STATS */

static struct svga_winsys_buffer *
vmw_svga_winsys_buffer_create(struct svga_winsys_screen *sws,
                              unsigned alignment,
                              unsigned usage,
                              unsigned size)
{
   struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);
   struct vmw_buffer_desc desc;
   struct pb_manager *provider;
   struct pb_buffer *buffer;

   memset(&desc, 0, sizeof desc);
   desc.pb_desc.alignment = alignment;
   desc.pb_desc.usage = usage;

   if (usage == SVGA_BUFFER_USAGE_PINNED) {
      if (vws->pools.query_fenced == NULL && !vmw_query_pools_init(vws))
	 return NULL;
      provider = vws->pools.query_fenced;
   } else if (usage == SVGA_BUFFER_USAGE_SHADER) {
      provider = vws->pools.dma_slab_fenced;
   } else {
      if (size > VMW_GMR_POOL_SIZE)
         return NULL;
      provider = vws->pools.dma_fenced;
   }

   assert(provider);
   buffer = provider->create_buffer(provider, size, &desc.pb_desc);

   if(!buffer && provider == vws->pools.dma_fenced) {

      assert(provider);
      provider = vws->pools.dma_slab_fenced;
      buffer = provider->create_buffer(provider, size, &desc.pb_desc);
   }

   if (!buffer)
      return NULL;

   return vmw_svga_winsys_buffer_wrap(buffer);
}


static void
vmw_svga_winsys_fence_reference(struct svga_winsys_screen *sws,
                                struct pipe_fence_handle **pdst,
                                struct pipe_fence_handle *src)
{
    struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);

    vmw_fence_reference(vws, pdst, src);
}


static int
vmw_svga_winsys_fence_signalled(struct svga_winsys_screen *sws,
                                struct pipe_fence_handle *fence,
                                unsigned flag)
{
   struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);

   return vmw_fence_signalled(vws, fence, flag);
}


static int
vmw_svga_winsys_fence_finish(struct svga_winsys_screen *sws,
                             struct pipe_fence_handle *fence,
                             uint64_t timeout,
                             unsigned flag)
{
   struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);

   return vmw_fence_finish(vws, fence, timeout, flag);
}


static int
vmw_svga_winsys_fence_get_fd(struct svga_winsys_screen *sws,
                             struct pipe_fence_handle *fence,
                             bool duplicate)
{
   if (duplicate)
      return os_dupfd_cloexec(vmw_fence_get_fd(fence));
   else
      return vmw_fence_get_fd(fence);
}


static void
vmw_svga_winsys_fence_create_fd(struct svga_winsys_screen *sws,
                                struct pipe_fence_handle **fence,
                                int32_t fd)
{
   *fence = vmw_fence_create(NULL, 0, 0, 0, os_dupfd_cloexec(fd));
}

static int
vmw_svga_winsys_fence_server_sync(struct svga_winsys_screen *sws,
                                  int32_t *context_fd,
                                  struct pipe_fence_handle *fence)
{
   int32_t fd = sws->fence_get_fd(sws, fence, false);

   /* If we don't have fd, we don't need to merge fd into the context's fd. */
   if (fd == -1)
      return 0;

   return sync_accumulate("vmwgfx", context_fd, fd);
}


static struct svga_winsys_surface *
vmw_svga_winsys_surface_create(struct svga_winsys_screen *sws,
                               SVGA3dSurfaceAllFlags flags,
                               SVGA3dSurfaceFormat format,
                               unsigned usage,
                               SVGA3dSize size,
                               uint32 numLayers,
                               uint32 numMipLevels,
                               unsigned sampleCount)
{
   struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);
   struct vmw_svga_winsys_surface *surface;
   struct vmw_buffer_desc desc;
   struct pb_manager *provider;
   uint32_t buffer_size;
   uint32_t num_samples = 1;
   SVGA3dMSPattern multisample_pattern = SVGA3D_MS_PATTERN_NONE;
   SVGA3dMSQualityLevel quality_level = SVGA3D_MS_QUALITY_NONE;

   memset(&desc, 0, sizeof(desc));
   surface = CALLOC_STRUCT(vmw_svga_winsys_surface);
   if(!surface)
      goto no_surface;

   pipe_reference_init(&surface->refcnt, 1);
   p_atomic_set(&surface->validated, 0);
   surface->screen = vws;
   (void) mtx_init(&surface->mutex, mtx_plain);
   surface->shared = !!(usage & SVGA_SURFACE_USAGE_SHARED);
   provider = (surface->shared) ? vws->pools.dma_base : vws->pools.dma_fenced;

   /*
    * When multisampling is not supported sample count received is 0,
    * otherwise should have a valid sample count.
    */
   if ((flags & SVGA3D_SURFACE_MULTISAMPLE) != 0) {
      if (sampleCount == 0)
         goto no_sid;
      num_samples = sampleCount;
      multisample_pattern = SVGA3D_MS_PATTERN_STANDARD;
      quality_level = SVGA3D_MS_QUALITY_FULL;
   }

   /*
    * Used for the backing buffer GB surfaces, and to approximate
    * when to flush on non-GB hosts.
    */
   buffer_size = svga3dsurface_get_serialized_size_extended(format, size,
                                                            numMipLevels,
                                                            numLayers,
                                                            num_samples);
   if (flags & SVGA3D_SURFACE_BIND_STREAM_OUTPUT)
      buffer_size += sizeof(SVGA3dDXSOState);

   if (buffer_size > vws->ioctl.max_texture_size) {
      goto no_sid;
   }

   if (sws->have_gb_objects) {
      SVGAGuestPtr ptr = {0,0};

      /*
       * If the backing buffer size is small enough, try to allocate a
       * buffer out of the buffer cache. Otherwise, let the kernel allocate
       * a suitable buffer for us.
       */
      if (buffer_size < VMW_TRY_CACHED_SIZE && !surface->shared) {
         struct pb_buffer *pb_buf;

         surface->size = buffer_size;
         desc.pb_desc.alignment = 4096;
         desc.pb_desc.usage = 0;
         pb_buf = provider->create_buffer(provider, buffer_size, &desc.pb_desc);
         surface->buf = vmw_svga_winsys_buffer_wrap(pb_buf);
         if (surface->buf && !vmw_dma_bufmgr_region_ptr(pb_buf, &ptr))
            assert(0);
      }

      surface->sid = vmw_ioctl_gb_surface_create(vws, flags, format, usage,
                                                 size, numLayers,
                                                 numMipLevels, sampleCount,
                                                 ptr.gmrId,
                                                 multisample_pattern,
                                                 quality_level,
                                                 surface->buf ? NULL :
                                                 &desc.region);

      if (surface->sid == SVGA3D_INVALID_ID) {
         if (surface->buf == NULL) {
            goto no_sid;
         } else {
            /*
             * Kernel refused to allocate a surface for us.
             * Perhaps something was wrong with our buffer?
             * This is really a guard against future new size requirements
             * on the backing buffers.
             */
            vmw_svga_winsys_buffer_destroy(sws, surface->buf);
            surface->buf = NULL;
            surface->sid = vmw_ioctl_gb_surface_create(vws, flags, format, usage,
                                                       size, numLayers,
                                                       numMipLevels, sampleCount,
                                                       0, multisample_pattern,
                                                       quality_level,
                                                       &desc.region);
            if (surface->sid == SVGA3D_INVALID_ID)
               goto no_sid;
         }
      }

      /*
       * If the kernel created the buffer for us, wrap it into a
       * vmw_svga_winsys_buffer.
       */
      if (surface->buf == NULL) {
         struct pb_buffer *pb_buf;

         surface->size = vmw_region_size(desc.region);
         desc.pb_desc.alignment = 4096;
         desc.pb_desc.usage = VMW_BUFFER_USAGE_SHARED;
         pb_buf = provider->create_buffer(provider, surface->size,
                                          &desc.pb_desc);
         surface->buf = vmw_svga_winsys_buffer_wrap(pb_buf);
         if (surface->buf == NULL) {
            vmw_ioctl_region_destroy(desc.region);
            vmw_ioctl_surface_destroy(vws, surface->sid);
            goto no_sid;
         }
      }
   } else {
      /* Legacy surface only support 32-bit svga3d flags */
      surface->sid = vmw_ioctl_surface_create(vws, (SVGA3dSurface1Flags)flags,
                                              format, usage, size, numLayers,
                                              numMipLevels, sampleCount);
      if(surface->sid == SVGA3D_INVALID_ID)
         goto no_sid;

      /* Best estimate for surface size, used for early flushing. */
      surface->size = buffer_size;
      surface->buf = NULL;
   }

   return svga_winsys_surface(surface);

no_sid:
   if (surface->buf)
      vmw_svga_winsys_buffer_destroy(sws, surface->buf);

   FREE(surface);
no_surface:
   return NULL;
}

static bool
vmw_svga_winsys_surface_can_create(struct svga_winsys_screen *sws,
                               SVGA3dSurfaceFormat format,
                               SVGA3dSize size,
                               uint32 numLayers,
                               uint32 numMipLevels,
                               uint32 numSamples)
{
   struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);
   uint32_t buffer_size;

   buffer_size = svga3dsurface_get_serialized_size(format, size,
                                                   numMipLevels,
                                                   numLayers);
   if (numSamples > 1)
      buffer_size *= numSamples;

   if (buffer_size > vws->ioctl.max_texture_size) {
	return false;
   }
   return true;
}


static bool
vmw_svga_winsys_surface_is_flushed(struct svga_winsys_screen *sws,
                                   struct svga_winsys_surface *surface)
{
   struct vmw_svga_winsys_surface *vsurf = vmw_svga_winsys_surface(surface);
   return (p_atomic_read(&vsurf->validated) == 0);
}


static void
vmw_svga_winsys_surface_ref(struct svga_winsys_screen *sws,
			    struct svga_winsys_surface **pDst,
			    struct svga_winsys_surface *src)
{
   struct vmw_svga_winsys_surface *d_vsurf = vmw_svga_winsys_surface(*pDst);
   struct vmw_svga_winsys_surface *s_vsurf = vmw_svga_winsys_surface(src);

   vmw_svga_winsys_surface_reference(&d_vsurf, s_vsurf);
   *pDst = svga_winsys_surface(d_vsurf);
}


static void
vmw_svga_winsys_destroy(struct svga_winsys_screen *sws)
{
   struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);

   vmw_winsys_destroy(vws);
}


static SVGA3dHardwareVersion
vmw_svga_winsys_get_hw_version(struct svga_winsys_screen *sws)
{
   struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);

   if (sws->have_gb_objects)
      return SVGA3D_HWVERSION_WS8_B1;

   return (SVGA3dHardwareVersion) vws->ioctl.hwversion;
}


static bool
vmw_svga_winsys_get_cap(struct svga_winsys_screen *sws,
                        SVGA3dDevCapIndex index,
                        SVGA3dDevCapResult *result)
{
   struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);

   if (index > vws->ioctl.num_cap_3d ||
       index >= SVGA3D_DEVCAP_MAX ||
       !vws->ioctl.cap_3d[index].has_cap)
      return false;

   *result = vws->ioctl.cap_3d[index].result;
   return true;
}

struct svga_winsys_gb_shader *
vmw_svga_winsys_shader_create(struct svga_winsys_screen *sws,
			      SVGA3dShaderType type,
			      const uint32 *bytecode,
			      uint32 bytecodeLen)
{
   struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);
   struct vmw_svga_winsys_shader *shader;
   void *code;

   shader = CALLOC_STRUCT(vmw_svga_winsys_shader);
   if(!shader)
      goto out_no_shader;

   pipe_reference_init(&shader->refcnt, 1);
   p_atomic_set(&shader->validated, 0);
   shader->screen = vws;
   shader->buf = vmw_svga_winsys_buffer_create(sws, 64,
					       SVGA_BUFFER_USAGE_SHADER,
					       bytecodeLen);
   if (!shader->buf)
      goto out_no_buf;

   code = vmw_svga_winsys_buffer_map(sws, shader->buf, PIPE_MAP_WRITE);
   if (!code)
      goto out_no_buf;

   memcpy(code, bytecode, bytecodeLen);
   vmw_svga_winsys_buffer_unmap(sws, shader->buf);

   if (!sws->have_vgpu10) {
      shader->shid = vmw_ioctl_shader_create(vws, type, bytecodeLen);
      if (shader->shid == SVGA3D_INVALID_ID)
         goto out_no_shid;
   }

   return svga_winsys_shader(shader);

out_no_shid:
   vmw_svga_winsys_buffer_destroy(sws, shader->buf);
out_no_buf:
   FREE(shader);
out_no_shader:
   return NULL;
}

void
vmw_svga_winsys_shader_destroy(struct svga_winsys_screen *sws,
			       struct svga_winsys_gb_shader *shader)
{
   struct vmw_svga_winsys_shader *d_shader =
      vmw_svga_winsys_shader(shader);

   vmw_svga_winsys_shader_reference(&d_shader, NULL);
}

#ifdef VMX86_STATS
static void
vmw_svga_winsys_stats_inc(struct svga_winsys_screen *sws,
                          enum svga_stats_count index)
{
   struct vmw_winsys_screen *const vws = vmw_winsys_screen(sws);
   const int slot = vmw_winsys_screen_mksstat_acq_slot(vws);
   assert(index < SVGA_STATS_COUNT_MAX);

   if (slot >= 0) {
      MKSGuestStatCounter *pstat;
      assert(vws->mksstat_tls[slot].stat_pages);
      assert(vws->mksstat_tls[slot].stat_id != -1UL);

      pstat = vmw_mksstat_get_pstat(vws->mksstat_tls[slot].stat_pages, getpagesize());

      __atomic_fetch_add(&pstat[index].count.value, 1, __ATOMIC_ACQ_REL);

      vmw_winsys_screen_mksstat_rel_slot(vws, slot);
   }
}

static void
vmw_svga_winsys_stats_time_push(struct svga_winsys_screen *sws,
                                enum svga_stats_time index,
                                struct svga_winsys_stats_timeframe *tf)
{
   struct vmw_winsys_screen *const vws = vmw_winsys_screen(sws);
   const int slot = vmw_winsys_screen_mksstat_acq_slot(vws);

   if (slot < 0)
      return;

   assert(vws->mksstat_tls[slot].stat_pages);
   assert(vws->mksstat_tls[slot].stat_id != -1UL);

   tf->counterTime = vmw_mksstat_get_pstat_time(vws->mksstat_tls[slot].stat_pages, getpagesize()) + index;

   vmw_winsys_screen_mksstat_rel_slot(vws, slot);

   tf->startTime = rdtsc();
   tf->enclosing = mksstat_tls_global;
   tf->sws = sws;
   tf->slot = slot;

   mksstat_tls_global = tf;
}

static void
vmw_svga_winsys_stats_time_pop(struct svga_winsys_screen *sws)
{
   struct svga_winsys_stats_timeframe *const tf = mksstat_tls_global;
   struct vmw_winsys_screen *const vws = vmw_winsys_screen(sws);
   const int slot = tf->slot;
   uint32_t expected = gettid();

   mksstat_tls_global = tf->enclosing;

   if (slot < 0)
      return;

   if (__atomic_compare_exchange_n(&vws->mksstat_tls[slot].pid, &expected, -1U, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
      const uint64_t dt = rdtsc() - tf->startTime;
      MKSGuestStatCounterTime *const counterTime = tf->counterTime;

      assert(vws->mksstat_tls[slot].stat_pages);
      assert(vws->mksstat_tls[slot].stat_id != -1UL);

      __atomic_fetch_add(&counterTime->counter.count.value, 1, __ATOMIC_ACQ_REL);
      __atomic_fetch_add(&counterTime->selfCycles.value, dt, __ATOMIC_ACQ_REL);
      __atomic_fetch_add(&counterTime->totalCycles.value, dt, __ATOMIC_ACQ_REL);

      if (tf->enclosing) {
         MKSGuestStatCounterTime *const counterTime = tf->enclosing->counterTime;

         assert(counterTime);

         __atomic_fetch_sub(&counterTime->selfCycles.value, dt, __ATOMIC_ACQ_REL);
      }

      __atomic_store_n(&vws->mksstat_tls[slot].pid, expected, __ATOMIC_RELEASE);
   }
}

#endif /* VMX86_STATS */
static void
vmw_svga_winsys_stats_inc_noop(struct svga_winsys_screen *sws,
                               enum svga_stats_count index)
{
   /* noop */
}

static void
vmw_svga_winsys_stats_time_push_noop(struct svga_winsys_screen *sws,
                                     enum svga_stats_time index,
                                     struct svga_winsys_stats_timeframe *tf)
{
   /* noop */
}

static void
vmw_svga_winsys_stats_time_pop_noop(struct svga_winsys_screen *sws)
{
   /* noop */
}

static int
vmw_svga_winsys_get_fd(struct svga_winsys_screen *sws)
{
   struct vmw_winsys_screen *const vws = vmw_winsys_screen(sws);

   return vws->ioctl.drm_fd;
}

bool
vmw_winsys_screen_init_svga(struct vmw_winsys_screen *vws)
{
   vws->base.destroy = vmw_svga_winsys_destroy;
   vws->base.get_hw_version = vmw_svga_winsys_get_hw_version;
   vws->base.get_fd = vmw_svga_winsys_get_fd;
   vws->base.get_cap = vmw_svga_winsys_get_cap;
   vws->base.context_create = vmw_svga_winsys_context_create;
   vws->base.surface_create = vmw_svga_winsys_surface_create;
   vws->base.surface_is_flushed = vmw_svga_winsys_surface_is_flushed;
   vws->base.surface_reference = vmw_svga_winsys_surface_ref;
   vws->base.surface_can_create = vmw_svga_winsys_surface_can_create;
   vws->base.buffer_create = vmw_svga_winsys_buffer_create;
   vws->base.buffer_map = vmw_svga_winsys_buffer_map;
   vws->base.buffer_unmap = vmw_svga_winsys_buffer_unmap;
   vws->base.buffer_destroy = vmw_svga_winsys_buffer_destroy;
   vws->base.surface_init = vmw_svga_winsys_surface_init;
   vws->base.fence_reference = vmw_svga_winsys_fence_reference;
   vws->base.fence_signalled = vmw_svga_winsys_fence_signalled;
   vws->base.shader_create = vmw_svga_winsys_shader_create;
   vws->base.shader_destroy = vmw_svga_winsys_shader_destroy;
   vws->base.fence_finish = vmw_svga_winsys_fence_finish;
   vws->base.fence_get_fd = vmw_svga_winsys_fence_get_fd;
   vws->base.fence_create_fd = vmw_svga_winsys_fence_create_fd;
   vws->base.fence_server_sync = vmw_svga_winsys_fence_server_sync;

   vws->base.query_create = vmw_svga_winsys_query_create;
   vws->base.query_init = vmw_svga_winsys_query_init;
   vws->base.query_destroy = vmw_svga_winsys_query_destroy;
   vws->base.query_get_result = vmw_svga_winsys_query_get_result;

#ifdef VMX86_STATS
   if (vws->ioctl.have_drm_2_19) {
      vws->base.stats_inc = vmw_svga_winsys_stats_inc;
      vws->base.stats_time_push = vmw_svga_winsys_stats_time_push;
      vws->base.stats_time_pop = vmw_svga_winsys_stats_time_pop;
   } else {
      vws->base.stats_inc = vmw_svga_winsys_stats_inc_noop;
      vws->base.stats_time_push = vmw_svga_winsys_stats_time_push_noop;
      vws->base.stats_time_pop = vmw_svga_winsys_stats_time_pop_noop;
   }

#else
   vws->base.stats_inc = vmw_svga_winsys_stats_inc_noop;
   vws->base.stats_time_push = vmw_svga_winsys_stats_time_push_noop;
   vws->base.stats_time_pop = vmw_svga_winsys_stats_time_pop_noop;

#endif
   vws->base.host_log = vmw_svga_winsys_host_log;

   return true;
}


