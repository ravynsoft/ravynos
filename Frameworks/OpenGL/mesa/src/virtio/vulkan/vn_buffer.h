/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#ifndef VN_BUFFER_H
#define VN_BUFFER_H

#include "vn_common.h"

struct vn_buffer_memory_requirements {
   VkMemoryRequirements2 memory;
   VkMemoryDedicatedRequirements dedicated;
};

struct vn_buffer_reqs_cache_entry {
   struct vn_buffer_memory_requirements requirements;
   atomic_bool valid;
};

struct vn_buffer_reqs_cache {
   uint64_t max_buffer_size;
   uint32_t queue_family_count;

   /* cache memory type requirement for AHB backed VkBuffer */
   uint32_t ahb_mem_type_bits;
   atomic_bool ahb_mem_type_bits_valid;

   /* lazily cache memory requirements for native buffer infos */
   struct util_sparse_array entries;

   /* protect both entries and ahb_mem_type_bits */
   simple_mtx_t mutex;

   struct {
      uint32_t cache_skip_count;
      uint32_t cache_hit_count;
      uint32_t cache_miss_count;
   } debug;
};

struct vn_buffer {
   struct vn_object_base base;

   struct vn_buffer_memory_requirements requirements;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_buffer,
                               base.base,
                               VkBuffer,
                               VK_OBJECT_TYPE_BUFFER)

struct vn_buffer_view {
   struct vn_object_base base;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_buffer_view,
                               base.base,
                               VkBufferView,
                               VK_OBJECT_TYPE_BUFFER_VIEW)

VkResult
vn_buffer_create(struct vn_device *dev,
                 const VkBufferCreateInfo *create_info,
                 const VkAllocationCallbacks *alloc,
                 struct vn_buffer **out_buf);

void
vn_buffer_reqs_cache_init(struct vn_device *dev);

void
vn_buffer_reqs_cache_fini(struct vn_device *dev);

#endif /* VN_BUFFER_H */
