/**********************************************************
 * Copyright 2009-2015 VMware, Inc.  All rights reserved.
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


#include "vmw_screen.h"
#include "vmw_fence.h"
#include "vmw_context.h"
#include "vmwgfx_drm.h"
#include "xf86drm.h"

#include "util/os_file.h"
#include "util/u_memory.h"
#include "util/compiler.h"
#include "util/u_hash_table.h"
#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

static struct hash_table *dev_hash = NULL;

static bool vmw_dev_compare(const void *key1, const void *key2)
{
   return (major(*(dev_t *)key1) == major(*(dev_t *)key2) &&
           minor(*(dev_t *)key1) == minor(*(dev_t *)key2));
}

static uint32_t vmw_dev_hash(const void *key)
{
   return (major(*(dev_t *) key) << 16) | minor(*(dev_t *) key);
}

#ifdef VMX86_STATS
/**
 * Initializes mksstat TLS store.
 */
static void
vmw_winsys_screen_init_mksstat(struct vmw_winsys_screen *vws)
{
   size_t i;

   for (i = 0; i < ARRAY_SIZE(vws->mksstat_tls); ++i) {
      vws->mksstat_tls[i].stat_pages = NULL;
      vws->mksstat_tls[i].stat_id = -1UL;
      vws->mksstat_tls[i].pid = 0;
   }
}

/**
 * Deinits mksstat TLS store.
 */
static void
vmw_winsys_screen_deinit_mksstat(struct vmw_winsys_screen *vws)
{
   size_t i;

   for (i = 0; i < ARRAY_SIZE(vws->mksstat_tls); ++i) {
      uint32_t expected = __atomic_load_n(&vws->mksstat_tls[i].pid, __ATOMIC_ACQUIRE);

      if (expected == -1U) {
         fprintf(stderr, "%s encountered locked mksstat TLS entry at index %lu.\n", __func__, i);
         continue;
      }

      if (expected == 0)
         continue;

      if (__atomic_compare_exchange_n(&vws->mksstat_tls[i].pid, &expected, 0, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
         struct drm_vmw_mksstat_remove_arg arg = {
            .id = vws->mksstat_tls[i].stat_id
         };

         assert(vws->mksstat_tls[i].stat_pages);
         assert(vws->mksstat_tls[i].stat_id != -1UL);

         if (drmCommandWrite(vws->ioctl.drm_fd, DRM_VMW_MKSSTAT_REMOVE, &arg, sizeof(arg))) {
            fprintf(stderr, "%s could not ioctl: %s\n", __func__, strerror(errno));
         } else if (munmap(vws->mksstat_tls[i].stat_pages, vmw_svga_winsys_stats_len())) {
            fprintf(stderr, "%s could not munmap: %s\n", __func__, strerror(errno));
         }
      } else {
         fprintf(stderr, "%s encountered volatile mksstat TLS entry at index %lu.\n", __func__, i);
      }
   }
}

#endif
/* Called from vmw_drm_create_screen(), creates and initializes the
 * vmw_winsys_screen structure, which is the main entity in this
 * module.
 * First, check whether a vmw_winsys_screen object already exists for
 * this device, and in that case return that one, making sure that we
 * have our own file descriptor open to DRM.
 */

struct vmw_winsys_screen *
vmw_winsys_create( int fd )
{
   struct vmw_winsys_screen *vws;
   struct stat stat_buf;
   const char *getenv_val;

   if (dev_hash == NULL) {
      dev_hash = _mesa_hash_table_create(NULL, vmw_dev_hash, vmw_dev_compare);
      if (dev_hash == NULL)
         return NULL;
   }

   if (fstat(fd, &stat_buf))
      return NULL;

   vws = util_hash_table_get(dev_hash, &stat_buf.st_rdev);
   if (vws) {
      vws->open_count++;
      return vws;
   }

   vws = CALLOC_STRUCT(vmw_winsys_screen);
   if (!vws)
      goto out_no_vws;

   vws->device = stat_buf.st_rdev;
   vws->open_count = 1;
   vws->ioctl.drm_fd = os_dupfd_cloexec(fd);
   vws->force_coherent = false;
   if (!vmw_ioctl_init(vws))
      goto out_no_ioctl;

   vws->base.have_gb_dma = !vws->force_coherent;
   vws->base.need_to_rebind_resources = false;
   vws->base.have_transfer_from_buffer_cmd = vws->base.have_vgpu10;
   vws->base.have_constant_buffer_offset_cmd =
      vws->ioctl.have_drm_2_20 && vws->base.have_sm5;
   vws->base.have_index_vertex_buffer_offset_cmd = false;
   vws->base.have_rasterizer_state_v2_cmd =
      vws->ioctl.have_drm_2_20 && vws->base.have_sm5;

   getenv_val = getenv("SVGA_FORCE_KERNEL_UNMAPS");
   vws->cache_maps = !getenv_val || strcmp(getenv_val, "0") == 0;
   vws->fence_ops = vmw_fence_ops_create(vws);
   if (!vws->fence_ops)
      goto out_no_fence_ops;

   if(!vmw_pools_init(vws))
      goto out_no_pools;

   if (!vmw_winsys_screen_init_svga(vws))
      goto out_no_svga;

#ifdef VMX86_STATS
   vmw_winsys_screen_init_mksstat(vws);
#endif
   _mesa_hash_table_insert(dev_hash, &vws->device, vws);

   cnd_init(&vws->cs_cond);
   mtx_init(&vws->cs_mutex, mtx_plain);

   return vws;
out_no_svga:
   vmw_pools_cleanup(vws);
out_no_pools:
   vws->fence_ops->destroy(vws->fence_ops);
out_no_fence_ops:
   vmw_ioctl_cleanup(vws);
out_no_ioctl:
   close(vws->ioctl.drm_fd);
   FREE(vws);
out_no_vws:
   return NULL;
}

void
vmw_winsys_destroy(struct vmw_winsys_screen *vws)
{
   if (--vws->open_count == 0) {
      _mesa_hash_table_remove_key(dev_hash, &vws->device);
      vmw_pools_cleanup(vws);
      vws->fence_ops->destroy(vws->fence_ops);
      vmw_ioctl_cleanup(vws);
#ifdef VMX86_STATS
      vmw_winsys_screen_deinit_mksstat(vws);
#endif
      close(vws->ioctl.drm_fd);
      mtx_destroy(&vws->cs_mutex);
      cnd_destroy(&vws->cs_cond);
      FREE(vws);
   }
}
