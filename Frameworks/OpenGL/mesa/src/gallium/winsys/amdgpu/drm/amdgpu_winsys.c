/*
 * Copyright © 2009 Corbin Simpson <MostAwesomeDude@gmail.com>
 * Copyright © 2009 Joakim Sindholt <opensource@zhasha.com>
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 * Copyright © 2015 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "amdgpu_cs.h"

#include "util/os_file.h"
#include "util/os_misc.h"
#include "util/u_cpu_detect.h"
#include "util/u_hash_table.h"
#include "util/hash_table.h"
#include "util/xmlconfig.h"
#include "drm-uapi/amdgpu_drm.h"
#include <xf86drm.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sid.h"

static struct hash_table *dev_tab = NULL;
static simple_mtx_t dev_tab_mutex = SIMPLE_MTX_INITIALIZER;

#if DEBUG
DEBUG_GET_ONCE_BOOL_OPTION(all_bos, "RADEON_ALL_BOS", false)
#endif

/* Helper function to do the ioctls needed for setup and init. */
static bool do_winsys_init(struct amdgpu_winsys *ws,
                           const struct pipe_screen_config *config,
                           int fd)
{
   if (!ac_query_gpu_info(fd, ws->dev, &ws->info, false))
      goto fail;

   /* TODO: Enable this once the kernel handles it efficiently. */
   if (ws->info.has_dedicated_vram)
      ws->info.has_local_buffers = false;

   ws->addrlib = ac_addrlib_create(&ws->info, &ws->info.max_alignment);
   if (!ws->addrlib) {
      fprintf(stderr, "amdgpu: Cannot create addrlib.\n");
      goto fail;
   }

   ws->check_vm = strstr(debug_get_option("R600_DEBUG", ""), "check_vm") != NULL ||
                  strstr(debug_get_option("AMD_DEBUG", ""), "check_vm") != NULL;
   ws->noop_cs = ws->info.family_overridden || debug_get_bool_option("RADEON_NOOP", false);
#if DEBUG
   ws->debug_all_bos = debug_get_option_all_bos();
#endif
   ws->reserve_vmid = strstr(debug_get_option("R600_DEBUG", ""), "reserve_vmid") != NULL ||
                      strstr(debug_get_option("AMD_DEBUG", ""), "reserve_vmid") != NULL ||
                      strstr(debug_get_option("AMD_DEBUG", ""), "sqtt") != NULL;
   ws->zero_all_vram_allocs = strstr(debug_get_option("R600_DEBUG", ""), "zerovram") != NULL ||
                              driQueryOptionb(config->options, "radeonsi_zerovram");

   return true;

fail:
   amdgpu_device_deinitialize(ws->dev);
   ws->dev = NULL;
   return false;
}

static void do_winsys_deinit(struct amdgpu_winsys *ws)
{
   if (ws->reserve_vmid)
      amdgpu_vm_unreserve_vmid(ws->dev, 0);

   for (unsigned i = 0; i < ARRAY_SIZE(ws->queues); i++) {
      for (unsigned j = 0; j < ARRAY_SIZE(ws->queues[i].fences); j++)
         amdgpu_fence_reference(&ws->queues[i].fences[j], NULL);

      amdgpu_ctx_reference(&ws->queues[i].last_ctx, NULL);
   }

   if (util_queue_is_initialized(&ws->cs_queue))
      util_queue_destroy(&ws->cs_queue);

   simple_mtx_destroy(&ws->bo_fence_lock);
   if (ws->bo_slabs.groups)
      pb_slabs_deinit(&ws->bo_slabs);
   pb_cache_deinit(&ws->bo_cache);
   _mesa_hash_table_destroy(ws->bo_export_table, NULL);
   simple_mtx_destroy(&ws->sws_list_lock);
#if DEBUG
   simple_mtx_destroy(&ws->global_bo_list_lock);
#endif
   simple_mtx_destroy(&ws->bo_export_table_lock);

   ac_addrlib_destroy(ws->addrlib);
   amdgpu_device_deinitialize(ws->dev);
   FREE(ws);
}

static void amdgpu_winsys_destroy_locked(struct radeon_winsys *rws, bool locked)
{
   struct amdgpu_screen_winsys *sws = amdgpu_screen_winsys(rws);
   struct amdgpu_winsys *ws = sws->aws;
   bool destroy;

   /* When the reference counter drops to zero, remove the device pointer
    * from the table.
    * This must happen while the mutex is locked, so that
    * amdgpu_winsys_create in another thread doesn't get the winsys
    * from the table when the counter drops to 0.
    */
   if (!locked)
      simple_mtx_lock(&dev_tab_mutex);

   destroy = pipe_reference(&ws->reference, NULL);
   if (destroy && dev_tab) {
      _mesa_hash_table_remove_key(dev_tab, ws->dev);
      if (_mesa_hash_table_num_entries(dev_tab) == 0) {
         _mesa_hash_table_destroy(dev_tab, NULL);
         dev_tab = NULL;
      }
   }

   if (!locked)
      simple_mtx_unlock(&dev_tab_mutex);

   if (destroy)
      do_winsys_deinit(ws);

   close(sws->fd);
   FREE(rws);
}

static void amdgpu_winsys_destroy(struct radeon_winsys *rws)
{
   amdgpu_winsys_destroy_locked(rws, false);
}

static void amdgpu_winsys_query_info(struct radeon_winsys *rws, struct radeon_info *info)
{
   struct amdgpu_winsys *ws = amdgpu_winsys(rws);

   *info = ws->info;
}

static bool amdgpu_cs_request_feature(struct radeon_cmdbuf *rcs,
                                      enum radeon_feature_id fid,
                                      bool enable)
{
   return false;
}

static uint64_t amdgpu_query_value(struct radeon_winsys *rws,
                                   enum radeon_value_id value)
{
   struct amdgpu_winsys *ws = amdgpu_winsys(rws);
   struct amdgpu_heap_info heap;
   uint64_t retval = 0;

   switch (value) {
   case RADEON_REQUESTED_VRAM_MEMORY:
      return ws->allocated_vram;
   case RADEON_REQUESTED_GTT_MEMORY:
      return ws->allocated_gtt;
   case RADEON_MAPPED_VRAM:
      return ws->mapped_vram;
   case RADEON_MAPPED_GTT:
      return ws->mapped_gtt;
   case RADEON_SLAB_WASTED_VRAM:
      return ws->slab_wasted_vram;
   case RADEON_SLAB_WASTED_GTT:
      return ws->slab_wasted_gtt;
   case RADEON_BUFFER_WAIT_TIME_NS:
      return ws->buffer_wait_time;
   case RADEON_NUM_MAPPED_BUFFERS:
      return ws->num_mapped_buffers;
   case RADEON_TIMESTAMP:
      amdgpu_query_info(ws->dev, AMDGPU_INFO_TIMESTAMP, 8, &retval);
      return retval;
   case RADEON_NUM_GFX_IBS:
      return ws->num_gfx_IBs;
   case RADEON_NUM_SDMA_IBS:
      return ws->num_sdma_IBs;
   case RADEON_GFX_BO_LIST_COUNTER:
      return ws->gfx_bo_list_counter;
   case RADEON_GFX_IB_SIZE_COUNTER:
      return ws->gfx_ib_size_counter;
   case RADEON_NUM_BYTES_MOVED:
      amdgpu_query_info(ws->dev, AMDGPU_INFO_NUM_BYTES_MOVED, 8, &retval);
      return retval;
   case RADEON_NUM_EVICTIONS:
      amdgpu_query_info(ws->dev, AMDGPU_INFO_NUM_EVICTIONS, 8, &retval);
      return retval;
   case RADEON_NUM_VRAM_CPU_PAGE_FAULTS:
      amdgpu_query_info(ws->dev, AMDGPU_INFO_NUM_VRAM_CPU_PAGE_FAULTS, 8, &retval);
      return retval;
   case RADEON_VRAM_USAGE:
      amdgpu_query_heap_info(ws->dev, AMDGPU_GEM_DOMAIN_VRAM, 0, &heap);
      return heap.heap_usage;
   case RADEON_VRAM_VIS_USAGE:
      amdgpu_query_heap_info(ws->dev, AMDGPU_GEM_DOMAIN_VRAM,
                             AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED, &heap);
      return heap.heap_usage;
   case RADEON_GTT_USAGE:
      amdgpu_query_heap_info(ws->dev, AMDGPU_GEM_DOMAIN_GTT, 0, &heap);
      return heap.heap_usage;
   case RADEON_GPU_TEMPERATURE:
      amdgpu_query_sensor_info(ws->dev, AMDGPU_INFO_SENSOR_GPU_TEMP, 4, &retval);
      return retval;
   case RADEON_CURRENT_SCLK:
      amdgpu_query_sensor_info(ws->dev, AMDGPU_INFO_SENSOR_GFX_SCLK, 4, &retval);
      return retval;
   case RADEON_CURRENT_MCLK:
      amdgpu_query_sensor_info(ws->dev, AMDGPU_INFO_SENSOR_GFX_MCLK, 4, &retval);
      return retval;
   case RADEON_CS_THREAD_TIME:
      return util_queue_get_thread_time_nano(&ws->cs_queue, 0);
   }
   return 0;
}

static bool amdgpu_read_registers(struct radeon_winsys *rws,
                                  unsigned reg_offset,
                                  unsigned num_registers, uint32_t *out)
{
   struct amdgpu_winsys *ws = amdgpu_winsys(rws);

   return amdgpu_read_mm_registers(ws->dev, reg_offset / 4, num_registers,
                                   0xffffffff, 0, out) == 0;
}

static bool amdgpu_winsys_unref(struct radeon_winsys *rws)
{
   struct amdgpu_screen_winsys *sws = amdgpu_screen_winsys(rws);
   struct amdgpu_winsys *aws = sws->aws;
   bool ret;

   simple_mtx_lock(&aws->sws_list_lock);

   ret = pipe_reference(&sws->reference, NULL);
   if (ret) {
      struct amdgpu_screen_winsys **sws_iter;
      struct amdgpu_winsys *aws = sws->aws;

      /* Remove this amdgpu_screen_winsys from amdgpu_winsys' list, so that
       * amdgpu_winsys_create can't re-use it anymore
       */
      for (sws_iter = &aws->sws_list; *sws_iter; sws_iter = &(*sws_iter)->next) {
         if (*sws_iter == sws) {
            *sws_iter = sws->next;
            break;
         }
      }
   }

   simple_mtx_unlock(&aws->sws_list_lock);

   if (ret && sws->kms_handles) {
      struct drm_gem_close args;

      hash_table_foreach(sws->kms_handles, entry) {
         args.handle = (uintptr_t)entry->data;
         drmIoctl(sws->fd, DRM_IOCTL_GEM_CLOSE, &args);
      }
      _mesa_hash_table_destroy(sws->kms_handles, NULL);
   }

   return ret;
}

static void amdgpu_pin_threads_to_L3_cache(struct radeon_winsys *rws,
                                           unsigned cache)
{
   struct amdgpu_winsys *ws = amdgpu_winsys(rws);

   util_set_thread_affinity(ws->cs_queue.threads[0],
                            util_get_cpu_caps()->L3_affinity_mask[cache],
                            NULL, util_get_cpu_caps()->num_cpu_mask_bits);
}

static uint32_t kms_handle_hash(const void *key)
{
   const struct amdgpu_bo_real *bo = key;

   return bo->kms_handle;
}

static bool kms_handle_equals(const void *a, const void *b)
{
   return a == b;
}

static bool amdgpu_cs_is_secure(struct radeon_cmdbuf *rcs)
{
   struct amdgpu_cs *cs = amdgpu_cs(rcs);
   return cs->csc->secure;
}

static uint32_t
radeon_to_amdgpu_pstate(enum radeon_ctx_pstate pstate)
{
   switch (pstate) {
   case RADEON_CTX_PSTATE_NONE:
      return AMDGPU_CTX_STABLE_PSTATE_NONE;
   case RADEON_CTX_PSTATE_STANDARD:
      return AMDGPU_CTX_STABLE_PSTATE_STANDARD;
   case RADEON_CTX_PSTATE_MIN_SCLK:
      return AMDGPU_CTX_STABLE_PSTATE_MIN_SCLK;
   case RADEON_CTX_PSTATE_MIN_MCLK:
      return AMDGPU_CTX_STABLE_PSTATE_MIN_MCLK;
   case RADEON_CTX_PSTATE_PEAK:
      return AMDGPU_CTX_STABLE_PSTATE_PEAK;
   default:
      unreachable("Invalid pstate");
   }
}

static bool
amdgpu_cs_set_pstate(struct radeon_cmdbuf *rcs, enum radeon_ctx_pstate pstate)
{
   struct amdgpu_cs *cs = amdgpu_cs(rcs);
   uint32_t amdgpu_pstate = radeon_to_amdgpu_pstate(pstate);
   return amdgpu_cs_ctx_stable_pstate(cs->ctx->ctx,
      AMDGPU_CTX_OP_SET_STABLE_PSTATE, amdgpu_pstate, NULL) == 0;
}

static bool
are_file_descriptions_equal(int fd1, int fd2)
{
   int r = os_same_file_description(fd1, fd2);

   if (r == 0)
      return true;

   if (r < 0) {
      static bool logged;

      if (!logged) {
         os_log_message("amdgpu: os_same_file_description couldn't "
                        "determine if two DRM fds reference the same "
                        "file description.\n"
                        "If they do, bad things may happen!\n");
         logged = true;
      }
   }
   return false;
}

static int
amdgpu_drm_winsys_get_fd(struct radeon_winsys *rws)
{
   struct amdgpu_screen_winsys *sws = amdgpu_screen_winsys(rws);

   return sws->fd;
}

PUBLIC struct radeon_winsys *
amdgpu_winsys_create(int fd, const struct pipe_screen_config *config,
		     radeon_screen_create_t screen_create)
{
   struct amdgpu_screen_winsys *ws;
   struct amdgpu_winsys *aws;
   amdgpu_device_handle dev;
   uint32_t drm_major, drm_minor;
   int r;

   ws = CALLOC_STRUCT(amdgpu_screen_winsys);
   if (!ws)
      return NULL;

   pipe_reference_init(&ws->reference, 1);
   ws->fd = os_dupfd_cloexec(fd);

   /* Look up the winsys from the dev table. */
   simple_mtx_lock(&dev_tab_mutex);
   if (!dev_tab)
      dev_tab = util_hash_table_create_ptr_keys();

   /* Initialize the amdgpu device. This should always return the same pointer
    * for the same fd. */
   r = amdgpu_device_initialize(ws->fd, &drm_major, &drm_minor, &dev);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_device_initialize failed.\n");
      goto fail;
   }

   /* Lookup a winsys if we have already created one for this device. */
   aws = util_hash_table_get(dev_tab, dev);
   if (aws) {
      struct amdgpu_screen_winsys *sws_iter;

      /* Release the device handle, because we don't need it anymore.
       * This function is returning an existing winsys instance, which
       * has its own device handle.
       */
      amdgpu_device_deinitialize(dev);

      simple_mtx_lock(&aws->sws_list_lock);
      for (sws_iter = aws->sws_list; sws_iter; sws_iter = sws_iter->next) {
         if (are_file_descriptions_equal(sws_iter->fd, ws->fd)) {
            close(ws->fd);
            FREE(ws);
            ws = sws_iter;
            pipe_reference(NULL, &ws->reference);
            simple_mtx_unlock(&aws->sws_list_lock);
            goto unlock;
         }
      }
      simple_mtx_unlock(&aws->sws_list_lock);

      ws->kms_handles = _mesa_hash_table_create(NULL, kms_handle_hash,
                                                kms_handle_equals);
      if (!ws->kms_handles)
         goto fail;

      pipe_reference(NULL, &aws->reference);
   } else {
      /* Create a new winsys. */
      aws = CALLOC_STRUCT(amdgpu_winsys);
      if (!aws)
         goto fail;

      aws->dev = dev;
      /* The device fd might be different from the one we passed because of
       * libdrm_amdgpu device dedup logic. This can happen if radv is initialized
       * first.
       * Get the correct fd or the buffer sharing will not work (see #3424).
       */
      int device_fd = amdgpu_device_get_fd(dev);
      if (!are_file_descriptions_equal(device_fd, fd)) {
         ws->kms_handles = _mesa_hash_table_create(NULL, kms_handle_hash,
                                                   kms_handle_equals);
         if (!ws->kms_handles)
            goto fail;
         /* We could avoid storing the fd and use amdgpu_device_get_fd() where
          * we need it but we'd have to use os_same_file_description() to
          * compare the fds.
          */
         aws->fd = device_fd;
      } else {
         aws->fd = ws->fd;
      }
      aws->info.drm_major = drm_major;
      aws->info.drm_minor = drm_minor;

      /* Only aws and buffer functions are used. */
      aws->dummy_ws.aws = aws;
      amdgpu_bo_init_functions(&aws->dummy_ws);

      if (!do_winsys_init(aws, config, fd))
         goto fail_alloc;

      /* Create managers. */
      pb_cache_init(&aws->bo_cache, RADEON_NUM_HEAPS,
                    500000, aws->check_vm ? 1.0f : 1.5f, 0,
                    ((uint64_t)aws->info.vram_size_kb + aws->info.gart_size_kb) * 1024 / 8,
                    offsetof(struct amdgpu_bo_real_reusable, cache_entry), aws,
                    /* Cast to void* because one of the function parameters
                     * is a struct pointer instead of void*. */
                    (void*)amdgpu_bo_destroy, (void*)amdgpu_bo_can_reclaim);

      if (!pb_slabs_init(&aws->bo_slabs,
                         8,  /* min slab entry size: 256 bytes */
                         20, /* max slab entry size: 1 MB (slab size = 2 MB) */
                         RADEON_NUM_HEAPS, true,
                         aws,
                         amdgpu_bo_can_reclaim_slab,
                         amdgpu_bo_slab_alloc,
                         /* Cast to void* because one of the function parameters
                          * is a struct pointer instead of void*. */
                         (void*)amdgpu_bo_slab_free)) {
         amdgpu_winsys_destroy(&ws->base);
         simple_mtx_unlock(&dev_tab_mutex);
         return NULL;
      }

      aws->info.min_alloc_size = 1 << aws->bo_slabs.min_order;

      /* init reference */
      pipe_reference_init(&aws->reference, 1);
#if DEBUG
      list_inithead(&aws->global_bo_list);
#endif
      aws->bo_export_table = util_hash_table_create_ptr_keys();

      (void) simple_mtx_init(&aws->sws_list_lock, mtx_plain);
#if DEBUG
      (void) simple_mtx_init(&aws->global_bo_list_lock, mtx_plain);
#endif
      (void) simple_mtx_init(&aws->bo_fence_lock, mtx_plain);
      (void) simple_mtx_init(&aws->bo_export_table_lock, mtx_plain);

      if (!util_queue_init(&aws->cs_queue, "cs", 8, 1,
                           UTIL_QUEUE_INIT_RESIZE_IF_FULL, NULL)) {
         amdgpu_winsys_destroy(&ws->base);
         simple_mtx_unlock(&dev_tab_mutex);
         return NULL;
      }

      _mesa_hash_table_insert(dev_tab, dev, aws);

      if (aws->reserve_vmid) {
         r = amdgpu_vm_reserve_vmid(dev, 0);
         if (r) {
            amdgpu_winsys_destroy(&ws->base);
            simple_mtx_unlock(&dev_tab_mutex);
            return NULL;
         }
      }
   }

   ws->aws = aws;

   /* Set functions. */
   ws->base.unref = amdgpu_winsys_unref;
   ws->base.destroy = amdgpu_winsys_destroy;
   ws->base.get_fd = amdgpu_drm_winsys_get_fd;
   ws->base.query_info = amdgpu_winsys_query_info;
   ws->base.cs_request_feature = amdgpu_cs_request_feature;
   ws->base.query_value = amdgpu_query_value;
   ws->base.read_registers = amdgpu_read_registers;
   ws->base.pin_threads_to_L3_cache = amdgpu_pin_threads_to_L3_cache;
   ws->base.cs_is_secure = amdgpu_cs_is_secure;
   ws->base.cs_set_pstate = amdgpu_cs_set_pstate;

   amdgpu_bo_init_functions(ws);
   amdgpu_cs_init_functions(ws);
   amdgpu_surface_init_functions(ws);

   simple_mtx_lock(&aws->sws_list_lock);
   ws->next = aws->sws_list;
   aws->sws_list = ws;
   simple_mtx_unlock(&aws->sws_list_lock);

   /* Create the screen at the end. The winsys must be initialized
    * completely.
    *
    * Alternatively, we could create the screen based on "ws->gen"
    * and link all drivers into one binary blob. */
   ws->base.screen = screen_create(&ws->base, config);
   if (!ws->base.screen) {
      amdgpu_winsys_destroy_locked(&ws->base, true);
      simple_mtx_unlock(&dev_tab_mutex);
      return NULL;
   }

unlock:
   /* We must unlock the mutex once the winsys is fully initialized, so that
    * other threads attempting to create the winsys from the same fd will
    * get a fully initialized winsys and not just half-way initialized. */
   simple_mtx_unlock(&dev_tab_mutex);

   return &ws->base;

fail_alloc:
   FREE(aws);
fail:
   if (ws->kms_handles)
      _mesa_hash_table_destroy(ws->kms_handles, NULL);
   close(ws->fd);
   FREE(ws);
   simple_mtx_unlock(&dev_tab_mutex);
   return NULL;
}
