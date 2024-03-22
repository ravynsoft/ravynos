/*
 * Copyright © 2023 Intel Corporation
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

#include <stdint.h>

#include "vulkan/vulkan_core.h"
#include "vk_sync.h"

#include "dev/intel_device_info.h"
#include "dev/intel_kmd.h"

struct anv_bo;
enum anv_bo_alloc_flags;
struct anv_cmd_buffer;
struct anv_device;
struct anv_queue;
struct anv_query_pool;
struct anv_utrace_submit;
struct anv_sparse_submission;
struct anv_trtt_batch_bo;

enum anv_vm_bind_op {
   ANV_VM_BIND,
   ANV_VM_UNBIND,
};

struct anv_vm_bind {
   struct anv_bo *bo;  /* Or NULL in case of a NULL binding. */
   uint64_t address;   /* Includes the resource offset. */
   uint64_t bo_offset; /* Also known as the memory offset. */
   uint64_t size;
   enum anv_vm_bind_op op;
};

struct anv_kmd_backend {
   /*
    * Create a gem buffer.
    * Return the gem handle in case of success otherwise returns 0.
    */
   uint32_t (*gem_create)(struct anv_device *device,
                          const struct intel_memory_class_instance **regions,
                          uint16_t num_regions, uint64_t size,
                          enum anv_bo_alloc_flags alloc_flags,
                          uint64_t *actual_size);
   uint32_t (*gem_create_userptr)(struct anv_device *device, void *mem, uint64_t size);
   void (*gem_close)(struct anv_device *device, struct anv_bo *bo);
   /* Returns MAP_FAILED on error */
   void *(*gem_mmap)(struct anv_device *device, struct anv_bo *bo,
                     uint64_t offset, uint64_t size);
   /* Bind things however you want. */
   int (*vm_bind)(struct anv_device *device,
                  struct anv_sparse_submission *submit);
   /* Fully bind or unbind a BO. */
   int (*vm_bind_bo)(struct anv_device *device, struct anv_bo *bo);
   int (*vm_unbind_bo)(struct anv_device *device, struct anv_bo *bo);
   VkResult (*execute_simple_batch)(struct anv_queue *queue,
                                    struct anv_bo *batch_bo,
                                    uint32_t batch_bo_size,
                                    bool is_companion_rcs_batch);
   VkResult (*execute_trtt_batch)(struct anv_sparse_submission *submit,
                                  struct anv_trtt_batch_bo *trtt_bbo);
   VkResult (*queue_exec_locked)(struct anv_queue *queue,
                                 uint32_t wait_count,
                                 const struct vk_sync_wait *waits,
                                 uint32_t cmd_buffer_count,
                                 struct anv_cmd_buffer **cmd_buffers,
                                 uint32_t signal_count,
                                 const struct vk_sync_signal *signals,
                                 struct anv_query_pool *perf_query_pool,
                                 uint32_t perf_query_pass,
                                 struct anv_utrace_submit *utrace_submit);
   VkResult (*queue_exec_trace)(struct anv_queue *queue,
                                struct anv_utrace_submit *submit);
   uint32_t (*bo_alloc_flags_to_bo_flags)(struct anv_device *device,
                                          enum anv_bo_alloc_flags alloc_flags);
};

const struct anv_kmd_backend *anv_kmd_backend_get(enum intel_kmd_type type);

/* Internal functions, should only be called by anv_kmd_backend_get() */
const struct anv_kmd_backend *anv_i915_kmd_backend_get(void);
const struct anv_kmd_backend *anv_xe_kmd_backend_get(void);
const struct anv_kmd_backend *anv_stub_kmd_backend_get(void);
