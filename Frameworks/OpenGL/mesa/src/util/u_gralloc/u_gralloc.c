/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2022 Roman Stratiienko (r.stratiienko@gmail.com)
 * SPDX-License-Identifier: MIT
 */

#include "u_gralloc_internal.h"

#include <assert.h>
#include <errno.h>

#include "drm-uapi/drm_fourcc.h"
#include "util/log.h"
#include "util/macros.h"
#include "util/simple_mtx.h"
#include "util/u_atomic.h"
#include "util/u_memory.h"

static simple_mtx_t u_gralloc_mutex = SIMPLE_MTX_INITIALIZER;

static const struct u_grallocs {
   enum u_gralloc_type type;
   struct u_gralloc *(*create)();
} u_grallocs[] = {
   /* Prefer the CrOS API as it is significantly faster than IMapper4 */
   {.type = U_GRALLOC_TYPE_CROS, .create = u_gralloc_cros_api_create},
#ifdef USE_IMAPPER4_METADATA_API
   {.type = U_GRALLOC_TYPE_GRALLOC4, .create = u_gralloc_imapper_api_create},
#endif /* USE_IMAPPER4_METADATA_API */
   {.type = U_GRALLOC_TYPE_LIBDRM, .create = u_gralloc_libdrm_create},
   {.type = U_GRALLOC_TYPE_QCOM, .create = u_gralloc_qcom_create},
   {.type = U_GRALLOC_TYPE_FALLBACK, .create = u_gralloc_fallback_create},
};

static struct u_gralloc_cache {
   struct u_gralloc *u_gralloc;
   int refcount;
} u_gralloc_cache[U_GRALLOC_TYPE_COUNT] = {0};

struct u_gralloc *
u_gralloc_create(enum u_gralloc_type type)
{
   struct u_gralloc *out_gralloc = NULL;

   simple_mtx_lock(&u_gralloc_mutex);

   if (u_gralloc_cache[type].u_gralloc != NULL) {
      u_gralloc_cache[type].refcount++;
      out_gralloc = u_gralloc_cache[type].u_gralloc;
      goto out;
   }

   for (int i = 0; i < ARRAY_SIZE(u_grallocs); i++) {
      if (u_grallocs[i].type != type && type != U_GRALLOC_TYPE_AUTO)
         continue;

      u_gralloc_cache[type].u_gralloc = u_grallocs[i].create();
      if (u_gralloc_cache[type].u_gralloc) {
         assert(u_gralloc_cache[type].u_gralloc->ops.get_buffer_basic_info);
         assert(u_gralloc_cache[type].u_gralloc->ops.destroy);

         u_gralloc_cache[type].u_gralloc->type = u_grallocs[i].type;
         u_gralloc_cache[type].refcount = 1;

         out_gralloc = u_gralloc_cache[type].u_gralloc;
         goto out;
      }
   }

out:
   simple_mtx_unlock(&u_gralloc_mutex);

   return out_gralloc;
}

void
u_gralloc_destroy(struct u_gralloc **gralloc)
{
   int i;

   if (*gralloc == NULL)
      return;

   simple_mtx_lock(&u_gralloc_mutex);

   for (i = 0; i < ARRAY_SIZE(u_gralloc_cache); i++) {
      if (u_gralloc_cache[i].u_gralloc == *gralloc) {
         u_gralloc_cache[i].refcount--;
         if (u_gralloc_cache[i].refcount == 0) {
            u_gralloc_cache[i].u_gralloc->ops.destroy(
               u_gralloc_cache[i].u_gralloc);
            u_gralloc_cache[i].u_gralloc = NULL;
         }
         break;
      }
   }

   simple_mtx_unlock(&u_gralloc_mutex);

   assert(i < ARRAY_SIZE(u_grallocs));

   *gralloc = NULL;
}

int
u_gralloc_get_buffer_basic_info(struct u_gralloc *gralloc,
                                struct u_gralloc_buffer_handle *hnd,
                                struct u_gralloc_buffer_basic_info *out)
{
   struct u_gralloc_buffer_basic_info info = {0};
   int ret;

   ret = gralloc->ops.get_buffer_basic_info(gralloc, hnd, &info);

   if (ret)
      return ret;

   *out = info;

   return 0;
}

int
u_gralloc_get_buffer_color_info(struct u_gralloc *gralloc,
                                struct u_gralloc_buffer_handle *hnd,
                                struct u_gralloc_buffer_color_info *out)
{
   struct u_gralloc_buffer_color_info info = {0};
   int ret;

   if (!gralloc->ops.get_buffer_color_info)
      return -ENOTSUP;

   ret = gralloc->ops.get_buffer_color_info(gralloc, hnd, &info);

   if (ret)
      return ret;

   *out = info;

   return 0;
}

int
u_gralloc_get_front_rendering_usage(struct u_gralloc *gralloc,
                                    uint64_t *out_usage)
{
   if (!gralloc->ops.get_front_rendering_usage)
      return -ENOTSUP;

   return gralloc->ops.get_front_rendering_usage(gralloc, out_usage);
}

int
u_gralloc_get_type(struct u_gralloc *gralloc)
{
   return gralloc->type;
}
