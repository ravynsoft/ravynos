/*
 * Copyright (C) 2016 Christian Gmeiner <christian.gmeiner@gmail.com>
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
 *
 * Authors:
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#ifndef RENDERONLY_H
#define RENDERONLY_H

#include <stdint.h>
#include "frontend/drm_driver.h"
#include "pipe/p_state.h"
#include "util/simple_mtx.h"
#include "util/sparse_array.h"

struct renderonly_scanout {
   uint32_t handle;
   uint32_t stride;
   int32_t refcnt;
};

struct renderonly {
   /**
    * Create a renderonly_scanout object for scanout resource.
    *
    * This function creates a renderonly_scanout object based on the provided
    * resource. The library is designed that the driver specific pipe_resource
    * struct holds a pointer to a renderonly_scanout struct.
    *
    * struct driver_resource {
    *    struct pipe_resource base;
    *    struct renderonly_scanout *scanout;
    *   ...
    * };
    *
    * The renderonly_scanout object exits for two reasons:
    * - Do any special treatment for a scanout resource like importing the GPU
    *   resource into the scanout hw.
    * - Make it easier for a gallium driver to detect if anything special needs
    *   to be done in flush_resource(..) like a resolve to linear.
    *
    * When the screen has renderonly enabled, drivers need to follow these
    * rules:
    * - Create the scanout resource in resource_create and
    *   resource_create_with_modifiers if PIPE_BIND_SCANOUT is set. Drivers
    *   can fail if the scanout resource couldn't be created.
    * - Try to import the scanout resource in resource_from_handle with
    *   renderonly_create_gpu_import_for_resource. Drivers MUST NOT fail if
    *   the scanout resource couldn't be created.
    * - In a resource_get_handle call for WINSYS_HANDLE_TYPE_KMS, use
    *   renderonly_get_handle with the scanout resource, even if the scanout
    *   resource is NULL. Drivers MUST NOT return their own resource here,
    *   because the GEM handle will not be valid for the caller's DRM FD.
    * - Implement resource_get_params for at least PIPE_RESOURCE_PARAM_STRIDE,
    *   PIPE_RESOURCE_PARAM_OFFSET and PIPE_RESOURCE_PARAM_MODIFIER.
    */
   struct renderonly_scanout *(*create_for_resource)(struct pipe_resource *rsc,
                                                     struct renderonly *ro,
                                                     struct winsys_handle *out_handle);
   void (*destroy)(struct renderonly *ro);
   int kms_fd;
   int gpu_fd;

   simple_mtx_t bo_map_lock;
   struct util_sparse_array bo_map;
};

static inline struct renderonly_scanout *
renderonly_scanout_for_resource(struct pipe_resource *rsc,
                                struct renderonly *ro,
                                struct winsys_handle *out_handle)
{
   return ro->create_for_resource(rsc, ro, out_handle);
}

void
renderonly_scanout_destroy(struct renderonly_scanout *scanout,
			   struct renderonly *ro);

static inline bool
renderonly_get_handle(struct renderonly_scanout *scanout,
      struct winsys_handle *handle)
{
   if (!scanout)
      return false;

   assert(handle->type == WINSYS_HANDLE_TYPE_KMS);
   handle->handle = scanout->handle;
   handle->stride = scanout->stride;

   return true;
}

/**
 * Create a dumb buffer object for a resource at scanout hw.
 */
struct renderonly_scanout *
renderonly_create_kms_dumb_buffer_for_resource(struct pipe_resource *rsc,
                                               struct renderonly *ro,
                                               struct winsys_handle *out_handle);

/**
 * Import GPU resource into scanout hw.
 */
struct renderonly_scanout *
renderonly_create_gpu_import_for_resource(struct pipe_resource *rsc,
                                          struct renderonly *ro,
                                          struct winsys_handle *out_handle);

#endif /* RENDERONLY_H_ */
