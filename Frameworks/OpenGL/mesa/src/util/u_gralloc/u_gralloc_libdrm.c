/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2023 Roman Stratiienko (r.stratiienko@gmail.com)
 * SPDX-License-Identifier: MIT
 */

#include "u_gralloc_libdrm.h"

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <hardware/gralloc.h>

#include "util/log.h"
#include "util/u_memory.h"

#include "u_gralloc_internal.h"

/* Despite minigbm becoming standard gralloc for many distributions, some
 * users still rely on legacy grallocs, like gbm_gralloc that use a native
 * handle header that is located at libdrm/android/gralloc_handle.h.
 * Using this gralloc is not recommended for new distributions.
 */

struct libdrm_gralloc {
   struct u_gralloc base;
   gralloc_module_t *gralloc_module;
   struct u_gralloc *fallback_gralloc;
};

static const char gbm_gralloc_module_name[] = "GBM Memory Allocator";

static int
get_buffer_info(struct u_gralloc *gralloc,
                struct u_gralloc_buffer_handle *hnd,
                struct u_gralloc_buffer_basic_info *out)
{
   struct libdrm_gralloc *gr = (struct libdrm_gralloc *)gralloc;
   struct gralloc_handle_t *handle = (struct gralloc_handle_t *)hnd->handle;
   assert(handle->base.numFds == GRALLOC_HANDLE_NUM_FDS);
   assert(handle->base.numInts == GRALLOC_HANDLE_NUM_INTS);
   assert(handle->magic == GRALLOC_HANDLE_MAGIC);

   if (handle->version != GRALLOC_HANDLE_VERSION)
      mesa_loge("Unexpected gralloc handle version %d", handle->version);

   assert(handle->version == GRALLOC_HANDLE_VERSION);

   /* Query basic information using fallback gralloc */
   u_gralloc_get_buffer_basic_info(gr->fallback_gralloc, hnd, out);

   /* Fill the known data using libdrm gralloc handle */
   out->modifier = handle->modifier;
   out->strides[0] = handle->stride;

   return 0;
}

static int
destroy(struct u_gralloc *gralloc)
{
   struct libdrm_gralloc *gr = (struct libdrm_gralloc *)gralloc;
   if (gr->gralloc_module)
      dlclose(gr->gralloc_module->common.dso);

   if (gr->fallback_gralloc)
      gr->fallback_gralloc->ops.destroy(gr->fallback_gralloc);

   FREE(gr);

   return 0;
}

struct u_gralloc *
u_gralloc_libdrm_create()
{
   struct libdrm_gralloc *gr = CALLOC_STRUCT(libdrm_gralloc);
   int err = 0;

   err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID,
                       (const hw_module_t **)&gr->gralloc_module);

   if (err)
      goto fail;

   if (strcmp(gr->gralloc_module->common.name, gbm_gralloc_module_name) != 0)
      goto fail;

   gr->base.ops.get_buffer_basic_info = get_buffer_info;
   gr->base.ops.destroy = destroy;

   mesa_logw("Using gralloc header from libdrm/android/gralloc_handle.h. "
             " This is not recommended for new distributions. "
             " Initializing a fallback gralloc as a helper:");

   gr->fallback_gralloc = u_gralloc_fallback_create();

   return &gr->base;

fail:
   destroy(&gr->base);

   return NULL;
}
