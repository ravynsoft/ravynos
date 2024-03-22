/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#include "vn_common.h"

#include <stdarg.h>

#include "util/log.h"
#include "util/os_misc.h"
#include "util/u_debug.h"
#include "venus-protocol/vn_protocol_driver_info.h"
#include "vk_enum_to_str.h"

#include "vn_instance.h"
#include "vn_ring.h"

#define VN_RELAX_MIN_BASE_SLEEP_US (160)

static const struct debug_control vn_debug_options[] = {
   /* clang-format off */
   { "init", VN_DEBUG_INIT },
   { "result", VN_DEBUG_RESULT },
   { "vtest", VN_DEBUG_VTEST },
   { "wsi", VN_DEBUG_WSI },
   { "no_abort", VN_DEBUG_NO_ABORT },
   { "log_ctx_info", VN_DEBUG_LOG_CTX_INFO },
   { "cache", VN_DEBUG_CACHE },
   { "no_sparse", VN_DEBUG_NO_SPARSE },
   { "gpl", VN_DEBUG_GPL },
   { NULL, 0 },
   /* clang-format on */
};

static const struct debug_control vn_perf_options[] = {
   /* clang-format off */
   { "no_async_set_alloc", VN_PERF_NO_ASYNC_SET_ALLOC },
   { "no_async_buffer_create", VN_PERF_NO_ASYNC_BUFFER_CREATE },
   { "no_async_queue_submit", VN_PERF_NO_ASYNC_QUEUE_SUBMIT },
   { "no_event_feedback", VN_PERF_NO_EVENT_FEEDBACK },
   { "no_fence_feedback", VN_PERF_NO_FENCE_FEEDBACK },
   { "no_memory_suballoc", VN_PERF_NO_MEMORY_SUBALLOC },
   { "no_cmd_batching", VN_PERF_NO_CMD_BATCHING },
   { "no_timeline_sem_feedback", VN_PERF_NO_TIMELINE_SEM_FEEDBACK },
   { "no_query_feedback", VN_PERF_NO_QUERY_FEEDBACK },
   { "no_async_mem_alloc", VN_PERF_NO_ASYNC_MEM_ALLOC },
   { "no_tiled_wsi_image", VN_PERF_NO_TILED_WSI_IMAGE },
   { "no_multi_ring", VN_PERF_NO_MULTI_RING },
   { "no_async_image_create", VN_PERF_NO_ASYNC_IMAGE_CREATE },
   { NULL, 0 },
   /* clang-format on */
};

struct vn_env vn_env;

static void
vn_env_init_once(void)
{
   vn_env.debug =
      parse_debug_string(os_get_option("VN_DEBUG"), vn_debug_options);
   vn_env.perf =
      parse_debug_string(os_get_option("VN_PERF"), vn_perf_options);
   vn_env.draw_cmd_batch_limit =
      debug_get_num_option("VN_DRAW_CMD_BATCH_LIMIT", UINT32_MAX);
   if (!vn_env.draw_cmd_batch_limit)
      vn_env.draw_cmd_batch_limit = UINT32_MAX;
   vn_env.relax_base_sleep_us = debug_get_num_option(
      "VN_RELAX_BASE_SLEEP_US", VN_RELAX_MIN_BASE_SLEEP_US);
}

void
vn_env_init(void)
{
   static once_flag once = ONCE_FLAG_INIT;
   call_once(&once, vn_env_init_once);

   /* log per VkInstance creation */
   if (VN_DEBUG(INIT)) {
      vn_log(NULL,
             "vn_env is as below:"
             "\n\tdebug = 0x%" PRIx64 ""
             "\n\tperf = 0x%" PRIx64 ""
             "\n\tdraw_cmd_batch_limit = %u"
             "\n\trelax_base_sleep_us = %u",
             vn_env.debug, vn_env.perf, vn_env.draw_cmd_batch_limit,
             vn_env.relax_base_sleep_us);
   }
}

void
vn_trace_init(void)
{
#ifdef ANDROID
   atrace_init();
#else
   util_cpu_trace_init();
#endif
}

void
vn_log(struct vn_instance *instance, const char *format, ...)
{
   va_list ap;

   va_start(ap, format);
   mesa_log_v(MESA_LOG_DEBUG, "MESA-VIRTIO", format, ap);
   va_end(ap);

   /* instance may be NULL or partially initialized */
}

VkResult
vn_log_result(struct vn_instance *instance,
              VkResult result,
              const char *where)
{
   vn_log(instance, "%s: %s", where, vk_Result_to_str(result));
   return result;
}

uint32_t
vn_extension_get_spec_version(const char *name)
{
   const int32_t index = vn_info_extension_index(name);
   return index >= 0 ? vn_info_extension_get(index)->spec_version : 0;
}

static inline bool
vn_watchdog_timeout(const struct vn_watchdog *watchdog)
{
   return !watchdog->alive;
}

static inline void
vn_watchdog_release(struct vn_watchdog *watchdog)
{
   if (vn_gettid() == watchdog->tid) {
      watchdog->tid = 0;
      mtx_unlock(&watchdog->mutex);
   }
}

static bool
vn_watchdog_acquire(struct vn_watchdog *watchdog, bool alive)
{
   pid_t tid = vn_gettid();
   if (!watchdog->tid && tid != watchdog->tid &&
       mtx_trylock(&watchdog->mutex) == thrd_success) {
      /* register as the only waiting thread that monitors the ring. */
      watchdog->tid = tid;
   }

   if (tid != watchdog->tid)
      return false;

   watchdog->alive = alive;
   return true;
}

void
vn_relax_fini(struct vn_relax_state *state)
{
   vn_watchdog_release(&state->instance->ring.watchdog);
}

struct vn_relax_state
vn_relax_init(struct vn_instance *instance, const char *reason)
{
   struct vn_ring *ring = instance->ring.ring;
   struct vn_watchdog *watchdog = &instance->ring.watchdog;
   if (vn_watchdog_acquire(watchdog, true))
      vn_ring_unset_status_bits(ring, VK_RING_STATUS_ALIVE_BIT_MESA);

   return (struct vn_relax_state){
      .instance = instance,
      .iter = 0,
      .reason = reason,
   };
}

void
vn_relax(struct vn_relax_state *state)
{
   uint32_t *iter = &state->iter;
   const char *reason = state->reason;

   /* Yield for the first 2^busy_wait_order times and then sleep for
    * base_sleep_us microseconds for the same number of times.  After that,
    * keep doubling both sleep length and count.
    * Must also update pre-calculated "first_warn_time" in vn_relax_init().
    */
   const uint32_t busy_wait_order = 8;
   const uint32_t base_sleep_us = vn_env.relax_base_sleep_us;
   const uint32_t warn_order = 12;
   const uint32_t abort_order = 16;

   (*iter)++;
   if (*iter < (1 << busy_wait_order)) {
      thrd_yield();
      return;
   }

   /* warn occasionally if we have slept at least 1.28ms for 2048 times (plus
    * another 2047 shorter sleeps)
    */
   if (unlikely(*iter % (1 << warn_order) == 0)) {
      struct vn_instance *instance = state->instance;
      vn_log(instance, "stuck in %s wait with iter at %d", reason, *iter);

      struct vn_ring *ring = instance->ring.ring;
      const uint32_t status = vn_ring_load_status(ring);
      if (status & VK_RING_STATUS_FATAL_BIT_MESA) {
         vn_log(instance, "aborting on ring fatal error at iter %d", *iter);
         abort();
      }

      struct vn_watchdog *watchdog = &instance->ring.watchdog;
      const bool alive = status & VK_RING_STATUS_ALIVE_BIT_MESA;
      if (vn_watchdog_acquire(watchdog, alive))
         vn_ring_unset_status_bits(ring, VK_RING_STATUS_ALIVE_BIT_MESA);

      if (vn_watchdog_timeout(watchdog) && !VN_DEBUG(NO_ABORT)) {
         vn_log(instance, "aborting on expired ring alive status at iter %d",
                *iter);
         abort();
      }

      if (*iter >= (1 << abort_order) && !VN_DEBUG(NO_ABORT)) {
         vn_log(instance, "aborting");
         abort();
      }
   }

   const uint32_t shift = util_last_bit(*iter) - busy_wait_order - 1;
   os_time_sleep(base_sleep_us << shift);
}

struct vn_ring *
vn_tls_get_ring(struct vn_instance *instance)
{
   if (VN_PERF(NO_MULTI_RING))
      return instance->ring.ring;

   struct vn_tls *tls = vn_tls_get();
   if (unlikely(!tls)) {
      /* only allow to fallback on missing tls */
      return instance->ring.ring;
   }

   /* look up tls_ring owned by instance */
   list_for_each_entry(struct vn_tls_ring, tls_ring, &tls->tls_rings,
                       tls_head) {
      mtx_lock(&tls_ring->mutex);
      if (tls_ring->instance == instance) {
         mtx_unlock(&tls_ring->mutex);
         assert(tls_ring->ring);
         return tls_ring->ring;
      }
      mtx_unlock(&tls_ring->mutex);
   }

   struct vn_tls_ring *tls_ring = calloc(1, sizeof(*tls_ring));
   if (!tls_ring)
      return NULL;

   /* keep the extra for potential roundtrip sync on tls ring */
   static const size_t extra_size = sizeof(uint32_t);

   /* only need a small ring for synchronous cmds on tls ring */
   static const size_t buf_size = 16 * 1024;

   struct vn_ring_layout layout;
   vn_ring_get_layout(buf_size, extra_size, &layout);

   tls_ring->ring = vn_ring_create(instance, &layout);
   if (!tls_ring->ring) {
      free(tls_ring);
      return NULL;
   }

   mtx_init(&tls_ring->mutex, mtx_plain);
   tls_ring->instance = instance;
   list_add(&tls_ring->tls_head, &tls->tls_rings);
   list_add(&tls_ring->vk_head, &instance->ring.tls_rings);

   return tls_ring->ring;
}

void
vn_tls_destroy_ring(struct vn_tls_ring *tls_ring)
{
   mtx_lock(&tls_ring->mutex);
   if (tls_ring->ring) {
      vn_ring_destroy(tls_ring->ring);
      tls_ring->ring = NULL;
      tls_ring->instance = NULL;
      mtx_unlock(&tls_ring->mutex);
   } else {
      mtx_unlock(&tls_ring->mutex);
      mtx_destroy(&tls_ring->mutex);
      free(tls_ring);
   }
}

static void
vn_tls_free(void *tls)
{
   if (tls) {
      list_for_each_entry_safe(struct vn_tls_ring, tls_ring,
                               &((struct vn_tls *)tls)->tls_rings, tls_head)
         vn_tls_destroy_ring(tls_ring);
   }
   free(tls);
}

static tss_t vn_tls_key;
static bool vn_tls_key_valid;

static void
vn_tls_key_create_once(void)
{
   vn_tls_key_valid = tss_create(&vn_tls_key, vn_tls_free) == thrd_success;
   if (!vn_tls_key_valid && VN_DEBUG(INIT))
      vn_log(NULL, "WARNING: failed to create vn_tls_key");
}

struct vn_tls *
vn_tls_get(void)
{
   static once_flag once = ONCE_FLAG_INIT;
   call_once(&once, vn_tls_key_create_once);
   if (unlikely(!vn_tls_key_valid))
      return NULL;

   struct vn_tls *tls = tss_get(vn_tls_key);
   if (likely(tls))
      return tls;

   tls = calloc(1, sizeof(*tls));
   if (!tls)
      return NULL;

   /* initialize tls */
   tls->async_pipeline_create = false;
   list_inithead(&tls->tls_rings);

   if (tss_set(vn_tls_key, tls) != thrd_success) {
      free(tls);
      return NULL;
   }

   return tls;
}
