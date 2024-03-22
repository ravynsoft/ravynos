/*
 * Copyright 2023 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#include "xe/intel_device_query.h"

#include "drm-uapi/xe_drm.h"

#include "common/intel_gem.h"

void *
xe_device_query_alloc_fetch(int fd, uint32_t query_id, uint32_t *len)
{
   struct drm_xe_device_query query = {
      .query = query_id,
   };
   if (intel_ioctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query))
      return NULL;

   void *data = calloc(1, query.size);
   if (!data)
      return NULL;

   query.data = (uintptr_t)data;
   if (intel_ioctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query)) {
      free(data);
      return NULL;
   }

   if (len)
      *len = query.size;
   return data;
}
