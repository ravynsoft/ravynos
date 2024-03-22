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

struct anv_device;
struct anv_queue;
struct anv_bo;
struct anv_cmd_buffer;
struct anv_query_pool;
struct anv_utrace_submit;
struct anv_sparse_submission;
struct anv_trtt_batch_bo;

VkResult
xe_execute_simple_batch(struct anv_queue *queue, struct anv_bo *batch_bo,
                        uint32_t batch_bo_size, bool is_companion_rcs_batch);
VkResult
xe_execute_trtt_batch(struct anv_sparse_submission *submit,
                      struct anv_trtt_batch_bo *trtt_bbo);

VkResult
xe_queue_exec_locked(struct anv_queue *queue,
                     uint32_t wait_count,
                     const struct vk_sync_wait *waits,
                     uint32_t cmd_buffer_count,
                     struct anv_cmd_buffer **cmd_buffers,
                     uint32_t signal_count,
                     const struct vk_sync_signal *signals,
                     struct anv_query_pool *perf_query_pool,
                     uint32_t perf_query_pass,
                     struct anv_utrace_submit *utrace_submit);

VkResult
xe_queue_exec_utrace_locked(struct anv_queue *queue,
                            struct anv_utrace_submit *utrace_submit);
