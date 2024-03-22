/*
 * Copyright © 2022 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "common/intel_gem.h"

#include "drm-uapi/i915_drm.h"

bool i915_gem_create_context(int fd, uint32_t *context_id);
bool i915_gem_destroy_context(int fd, uint32_t context_id);
bool i915_gem_create_context_engines(int fd,
                                     enum intel_gem_create_context_flags flags,
                                     const struct intel_query_engine_info *info,
                                     int num_engines, enum intel_engine_class *engine_classes,
                                     uint32_t vm_id,
                                     uint32_t *context_id);
bool i915_gem_set_context_param(int fd, uint32_t context, uint32_t param,
                                uint64_t value);
bool i915_gem_get_context_param(int fd, uint32_t context, uint32_t param,
                                uint64_t *value);
bool i915_gem_read_render_timestamp(int fd, uint64_t *value);
bool
i915_gem_create_context_ext(int fd,
                            enum intel_gem_create_context_flags flags,
                            uint32_t *ctx_id);
bool i915_gem_supports_protected_context(int fd);
bool i915_gem_get_param(int fd, uint32_t param, int *value);
bool i915_gem_can_render_on_fd(int fd);

/**
 * A wrapper around DRM_IOCTL_I915_QUERY
 *
 * Unfortunately, the error semantics of this ioctl are rather annoying so
 * it's better to have a common helper.
 */
static inline int
intel_i915_query_flags(int fd, uint64_t query_id, uint32_t flags,
                       void *buffer, int32_t *buffer_len)
{
   struct drm_i915_query_item item = {
      .query_id = query_id,
      .length = *buffer_len,
      .flags = flags,
      .data_ptr = (uintptr_t)buffer,
   };

   struct drm_i915_query args = {
      .num_items = 1,
      .flags = 0,
      .items_ptr = (uintptr_t)&item,
   };

   int ret = intel_ioctl(fd, DRM_IOCTL_I915_QUERY, &args);
   if (ret != 0)
      return -errno;
   else if (item.length < 0)
      return item.length;

   *buffer_len = item.length;
   return 0;
}

static inline int
intel_i915_query(int fd, uint64_t query_id, void *buffer,
                 int32_t *buffer_len)
{
   return intel_i915_query_flags(fd, query_id, 0, buffer, buffer_len);
}

/**
 * Query for the given data, allocating as needed
 *
 * The caller is responsible for freeing the returned pointer.
 */
static inline void *
intel_i915_query_alloc(int fd, uint64_t query_id, int32_t *query_length)
{
   if (query_length)
      *query_length = 0;

   int32_t length = 0;
   int ret = intel_i915_query(fd, query_id, NULL, &length);
   if (ret < 0)
      return NULL;

   void *data = calloc(1, length);
   assert(data != NULL); /* This shouldn't happen in practice */
   if (data == NULL)
      return NULL;

   ret = intel_i915_query(fd, query_id, data, &length);
   assert(ret == 0); /* We should have caught the error above */
   if (ret < 0) {
      free(data);
      return NULL;
   }

   if (query_length)
      *query_length = length;

   return data;
}

static inline void
intel_i915_gem_add_ext(__u64 *ptr, uint32_t ext_name,
                  struct i915_user_extension *ext)
{
   __u64 *iter = ptr;

   while (*iter != 0) {
      iter = (__u64 *) &((struct i915_user_extension *)(uintptr_t)*iter)->next_extension;
   }

   ext->name = ext_name;

   *iter = (uintptr_t) ext;
}
