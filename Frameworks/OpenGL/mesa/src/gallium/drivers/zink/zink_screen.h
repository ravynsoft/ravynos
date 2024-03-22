/*
 * Copyright 2018 Collabora Ltd.
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

#ifndef ZINK_SCREEN_H
#define ZINK_SCREEN_H

#include "zink_types.h"


#ifdef __cplusplus
extern "C" {
#endif

struct util_dl_library;

void
zink_init_screen_pipeline_libs(struct zink_screen *screen);


/* update last_finished to account for batch_id wrapping */
static inline void
zink_screen_update_last_finished(struct zink_screen *screen, uint64_t batch_id)
{
   const uint32_t check_id = (uint32_t)batch_id;
   /* last_finished may have wrapped */
   if (screen->last_finished < UINT_MAX / 2) {
      /* last_finished has wrapped, batch_id has not */
      if (check_id > UINT_MAX / 2)
         return;
   } else if (check_id < UINT_MAX / 2) {
      /* batch_id has wrapped, last_finished has not */
      screen->last_finished = check_id;
      return;
   }
   /* neither have wrapped */
   screen->last_finished = MAX2(check_id, screen->last_finished);
}

/* check a batch_id against last_finished while accounting for wrapping */
static inline bool
zink_screen_check_last_finished(struct zink_screen *screen, uint32_t batch_id)
{
   const uint32_t check_id = (uint32_t)batch_id;
   assert(check_id);
   /* last_finished may have wrapped */
   if (screen->last_finished < UINT_MAX / 2) {
      /* last_finished has wrapped, batch_id has not */
      if (check_id > UINT_MAX / 2)
         return true;
   } else if (check_id < UINT_MAX / 2) {
      /* batch_id has wrapped, last_finished has not */
      return false;
   }
   return screen->last_finished >= check_id;
}

bool
zink_screen_init_semaphore(struct zink_screen *screen);

static inline bool
zink_screen_handle_vkresult(struct zink_screen *screen, VkResult ret)
{
   bool success = false;
   switch (ret) {
   case VK_SUCCESS:
      success = true;
      break;
   case VK_ERROR_DEVICE_LOST:
      screen->device_lost = true;
      mesa_loge("zink: DEVICE LOST!\n");
      /* if nothing can save us, abort */
      if (screen->abort_on_hang && !screen->robust_ctx_count)
         abort();
      FALLTHROUGH;
   default:
      success = false;
      break;
   }
   return success;
}

typedef const char *(*zink_vkflags_func)(uint64_t);

static inline unsigned
zink_string_vkflags_unroll(char *buf, size_t bufsize, uint64_t flags, zink_vkflags_func func)
{
   bool first = true;
   unsigned idx = 0;
   u_foreach_bit64(bit, flags) {
      if (!first)
         buf[idx++] = '|';
      idx += snprintf(&buf[idx], bufsize - idx, "%s", func((BITFIELD64_BIT(bit))));
      first = false;
   }
   return idx;
}

#define VRAM_ALLOC_LOOP(RET, DOIT, ...) \
   do { \
      unsigned _us[] = {0, 1000, 10000, 500000, 1000000}; \
      for (unsigned _i = 0; _i < ARRAY_SIZE(_us); _i++) { \
         RET = DOIT; \
         if (RET == VK_SUCCESS || RET != VK_ERROR_OUT_OF_DEVICE_MEMORY) \
            break; \
         os_time_sleep(_us[_i]); \
      } \
      __VA_ARGS__ \
   } while (0)

VkSemaphore
zink_create_semaphore(struct zink_screen *screen);

void
zink_screen_lock_context(struct zink_screen *screen);
void
zink_screen_unlock_context(struct zink_screen *screen);

VkSemaphore
zink_create_exportable_semaphore(struct zink_screen *screen);
VkSemaphore
zink_screen_export_dmabuf_semaphore(struct zink_screen *screen, struct zink_resource *res);
bool
zink_screen_import_dmabuf_semaphore(struct zink_screen *screen, struct zink_resource *res, VkSemaphore sem);

VkFormat
zink_get_format(struct zink_screen *screen, enum pipe_format format);

void
zink_convert_color(const struct zink_screen *screen, enum pipe_format format,
                   union pipe_color_union *dst,
                   const union pipe_color_union *src);

bool
zink_screen_timeline_wait(struct zink_screen *screen, uint64_t batch_id, uint64_t timeout);

bool
zink_is_depth_format_supported(struct zink_screen *screen, VkFormat format);

#define GET_PROC_ADDR_INSTANCE_LOCAL(screen, instance, x) PFN_vk##x vk_##x = (PFN_vk##x)(screen)->vk_GetInstanceProcAddr(instance, "vk"#x)

void
zink_screen_update_pipeline_cache(struct zink_screen *screen, struct zink_program *pg, bool in_thread);

void
zink_screen_get_pipeline_cache(struct zink_screen *screen, struct zink_program *pg, bool in_thread);

void
zink_stub_function_not_loaded(void);

bool
zink_screen_debug_marker_begin(struct zink_screen *screen, const char *fmt, ...);
void
zink_screen_debug_marker_end(struct zink_screen *screen, bool emitted);

#define warn_missing_feature(warned, feat) \
   do { \
      if (!warned) { \
         if (!(zink_debug & ZINK_DEBUG_QUIET)) \
            mesa_logw("WARNING: Incorrect rendering will happen " \
                           "because the Vulkan device doesn't support " \
                           "the '%s' feature\n", feat); \
         warned = true; \
      } \
   } while (0)

#ifdef __cplusplus
}
#endif

#endif
