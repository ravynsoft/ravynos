/*
 * Copyright © 2021 Intel Corporation
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

#include "anv_private.h"

#include "perf/intel_perf.h"

static uint32_t
command_buffers_count_utraces(struct anv_device *device,
                              uint32_t cmd_buffer_count,
                              struct anv_cmd_buffer **cmd_buffers,
                              uint32_t *utrace_copies)
{
   if (!u_trace_should_process(&device->ds.trace_context))
      return 0;

   uint32_t utraces = 0;
   for (uint32_t i = 0; i < cmd_buffer_count; i++) {
      if (u_trace_has_points(&cmd_buffers[i]->trace)) {
         utraces++;
         if (!(cmd_buffers[i]->usage_flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT))
            *utrace_copies += list_length(&cmd_buffers[i]->trace.trace_chunks);
      }
   }

   return utraces;
}

static void
anv_utrace_delete_flush_data(struct u_trace_context *utctx,
                             void *flush_data)
{
   struct anv_device *device =
      container_of(utctx, struct anv_device, ds.trace_context);
   struct anv_utrace_flush_copy *flush = flush_data;

   intel_ds_flush_data_fini(&flush->ds);

   if (flush->trace_bo) {
      assert(flush->batch_bo);
      anv_reloc_list_finish(&flush->relocs, &device->vk.alloc);
      anv_device_release_bo(device, flush->batch_bo);
      anv_device_release_bo(device, flush->trace_bo);
   }

   vk_sync_destroy(&device->vk, flush->sync);

   vk_free(&device->vk.alloc, flush);
}

static void
anv_device_utrace_emit_copy_ts_buffer(struct u_trace_context *utctx,
                                      void *cmdstream,
                                      void *ts_from, uint32_t from_offset,
                                      void *ts_to, uint32_t to_offset,
                                      uint32_t count)
{
   struct anv_device *device =
      container_of(utctx, struct anv_device, ds.trace_context);
   struct anv_utrace_flush_copy *flush = cmdstream;
   struct anv_address from_addr = (struct anv_address) {
      .bo = ts_from, .offset = from_offset * sizeof(uint64_t) };
   struct anv_address to_addr = (struct anv_address) {
      .bo = ts_to, .offset = to_offset * sizeof(uint64_t) };

   anv_genX(device->info, emit_so_memcpy)(&flush->memcpy_state,
                                           to_addr, from_addr, count * sizeof(uint64_t));
}

VkResult
anv_device_utrace_flush_cmd_buffers(struct anv_queue *queue,
                                    uint32_t cmd_buffer_count,
                                    struct anv_cmd_buffer **cmd_buffers,
                                    struct anv_utrace_flush_copy **out_flush_data)
{
   struct anv_device *device = queue->device;
   uint32_t utrace_copies = 0;
   uint32_t utraces = command_buffers_count_utraces(device,
                                                    cmd_buffer_count,
                                                    cmd_buffers,
                                                    &utrace_copies);
   if (!utraces) {
      *out_flush_data = NULL;
      return VK_SUCCESS;
   }

   VkResult result;
   struct anv_utrace_flush_copy *flush =
      vk_zalloc(&device->vk.alloc, sizeof(struct anv_utrace_flush_copy),
                8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!flush)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   intel_ds_flush_data_init(&flush->ds, &queue->ds, queue->ds.submission_id);

   result = vk_sync_create(&device->vk, &device->physical->sync_syncobj_type,
                           0, 0, &flush->sync);
   if (result != VK_SUCCESS)
      goto error_sync;

   if (utrace_copies > 0) {
      result = anv_bo_pool_alloc(&device->utrace_bo_pool,
                                 utrace_copies * 4096,
                                 &flush->trace_bo);
      if (result != VK_SUCCESS)
         goto error_trace_buf;

      result = anv_bo_pool_alloc(&device->utrace_bo_pool,
                                 /* 128 dwords of setup + 64 dwords per copy */
                                 align(512 + 64 * utrace_copies, 4096),
                                 &flush->batch_bo);
      if (result != VK_SUCCESS)
         goto error_batch_buf;

      result = anv_reloc_list_init(&flush->relocs, &device->vk.alloc);
      if (result != VK_SUCCESS)
         goto error_reloc_list;

      flush->batch.alloc = &device->vk.alloc;
      flush->batch.relocs = &flush->relocs;
      anv_batch_set_storage(&flush->batch,
                            (struct anv_address) { .bo = flush->batch_bo, },
                            flush->batch_bo->map, flush->batch_bo->size);

      /* Emit the copies */
      anv_genX(device->info, emit_so_memcpy_init)(&flush->memcpy_state,
                                                   device,
                                                   &flush->batch);
      for (uint32_t i = 0; i < cmd_buffer_count; i++) {
         if (cmd_buffers[i]->usage_flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) {
           intel_ds_queue_flush_data(&queue->ds, &cmd_buffers[i]->trace,
                                     &flush->ds, false);
         } else {
            u_trace_clone_append(u_trace_begin_iterator(&cmd_buffers[i]->trace),
                                 u_trace_end_iterator(&cmd_buffers[i]->trace),
                                 &flush->ds.trace,
                                 flush,
                                 anv_device_utrace_emit_copy_ts_buffer);
         }
      }
      anv_genX(device->info, emit_so_memcpy_fini)(&flush->memcpy_state);

      intel_ds_queue_flush_data(&queue->ds, &flush->ds.trace, &flush->ds, true);

      if (flush->batch.status != VK_SUCCESS) {
         result = flush->batch.status;
         goto error_batch;
      }
   } else {
      for (uint32_t i = 0; i < cmd_buffer_count; i++) {
         assert(cmd_buffers[i]->usage_flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
         intel_ds_queue_flush_data(&queue->ds, &cmd_buffers[i]->trace,
                                   &flush->ds, i == (cmd_buffer_count - 1));
      }
   }

   flush->queue = queue;

   *out_flush_data = flush;

   return VK_SUCCESS;

 error_batch:
   anv_reloc_list_finish(&flush->relocs, &device->vk.alloc);
 error_reloc_list:
   anv_bo_pool_free(&device->utrace_bo_pool, flush->batch_bo);
 error_batch_buf:
   anv_bo_pool_free(&device->utrace_bo_pool, flush->trace_bo);
 error_trace_buf:
   vk_sync_destroy(&device->vk, flush->sync);
 error_sync:
   vk_free(&device->vk.alloc, flush);
   return result;
}

static void *
anv_utrace_create_ts_buffer(struct u_trace_context *utctx, uint32_t size_b)
{
   struct anv_device *device =
      container_of(utctx, struct anv_device, ds.trace_context);

   struct anv_bo *bo = NULL;
   UNUSED VkResult result =
      anv_bo_pool_alloc(&device->utrace_bo_pool,
                        align(size_b, 4096),
                        &bo);
   assert(result == VK_SUCCESS);

   return bo;
}

static void
anv_utrace_destroy_ts_buffer(struct u_trace_context *utctx, void *timestamps)
{
   struct anv_device *device =
      container_of(utctx, struct anv_device, ds.trace_context);
   struct anv_bo *bo = timestamps;

   anv_bo_pool_free(&device->utrace_bo_pool, bo);
}

static void
anv_utrace_record_ts(struct u_trace *ut, void *cs,
                     void *timestamps, unsigned idx,
                     bool end_of_pipe)
{
   struct anv_cmd_buffer *cmd_buffer =
      container_of(ut, struct anv_cmd_buffer, trace);
   struct anv_device *device = cmd_buffer->device;
   struct anv_bo *bo = timestamps;

   enum anv_timestamp_capture_type capture_type =
      (end_of_pipe) ? ANV_TIMESTAMP_CAPTURE_END_OF_PIPE
                    : ANV_TIMESTAMP_CAPTURE_TOP_OF_PIPE;
   device->physical->cmd_emit_timestamp(&cmd_buffer->batch, device,
                                        (struct anv_address) {
                                           .bo = bo,
                                           .offset = idx * sizeof(uint64_t) },
                                        capture_type);
}

static uint64_t
anv_utrace_read_ts(struct u_trace_context *utctx,
                   void *timestamps, unsigned idx, void *flush_data)
{
   struct anv_device *device =
      container_of(utctx, struct anv_device, ds.trace_context);
   struct anv_bo *bo = timestamps;
   struct anv_utrace_flush_copy *flush = flush_data;

   /* Only need to stall on results for the first entry: */
   if (idx == 0) {
      UNUSED VkResult result =
         vk_sync_wait(&device->vk,
                      flush->sync,
                      0,
                      VK_SYNC_WAIT_COMPLETE,
                      os_time_get_absolute_timeout(OS_TIMEOUT_INFINITE));
      assert(result == VK_SUCCESS);
   }

   uint64_t *ts = bo->map;

   /* Don't translate the no-timestamp marker: */
   if (ts[idx] == U_TRACE_NO_TIMESTAMP)
      return U_TRACE_NO_TIMESTAMP;

   return intel_device_info_timebase_scale(device->info, ts[idx]);
}

void
anv_device_utrace_init(struct anv_device *device)
{
   anv_bo_pool_init(&device->utrace_bo_pool, device, "utrace");
   intel_ds_device_init(&device->ds, device->info, device->fd,
                        device->physical->local_minor,
                        INTEL_DS_API_VULKAN);
   u_trace_context_init(&device->ds.trace_context,
                        &device->ds,
                        anv_utrace_create_ts_buffer,
                        anv_utrace_destroy_ts_buffer,
                        anv_utrace_record_ts,
                        anv_utrace_read_ts,
                        anv_utrace_delete_flush_data);

   for (uint32_t q = 0; q < device->queue_count; q++) {
      struct anv_queue *queue = &device->queues[q];

      intel_ds_device_init_queue(&device->ds, &queue->ds, "%s%u",
                                 intel_engines_class_to_string(queue->family->engine_class),
                                 queue->vk.index_in_family);
   }
}

void
anv_device_utrace_finish(struct anv_device *device)
{
   intel_ds_device_process(&device->ds, true);
   intel_ds_device_fini(&device->ds);
   anv_bo_pool_finish(&device->utrace_bo_pool);
}

enum intel_ds_stall_flag
anv_pipe_flush_bit_to_ds_stall_flag(enum anv_pipe_bits bits)
{
   static const struct {
      enum anv_pipe_bits anv;
      enum intel_ds_stall_flag ds;
   } anv_to_ds_flags[] = {
      { .anv = ANV_PIPE_DEPTH_CACHE_FLUSH_BIT,            .ds = INTEL_DS_DEPTH_CACHE_FLUSH_BIT, },
      { .anv = ANV_PIPE_DATA_CACHE_FLUSH_BIT,             .ds = INTEL_DS_DATA_CACHE_FLUSH_BIT, },
      { .anv = ANV_PIPE_TILE_CACHE_FLUSH_BIT,             .ds = INTEL_DS_TILE_CACHE_FLUSH_BIT, },
      { .anv = ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT,    .ds = INTEL_DS_RENDER_TARGET_CACHE_FLUSH_BIT, },
      { .anv = ANV_PIPE_STATE_CACHE_INVALIDATE_BIT,       .ds = INTEL_DS_STATE_CACHE_INVALIDATE_BIT, },
      { .anv = ANV_PIPE_CONSTANT_CACHE_INVALIDATE_BIT,    .ds = INTEL_DS_CONST_CACHE_INVALIDATE_BIT, },
      { .anv = ANV_PIPE_VF_CACHE_INVALIDATE_BIT,          .ds = INTEL_DS_VF_CACHE_INVALIDATE_BIT, },
      { .anv = ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT,     .ds = INTEL_DS_TEXTURE_CACHE_INVALIDATE_BIT, },
      { .anv = ANV_PIPE_INSTRUCTION_CACHE_INVALIDATE_BIT, .ds = INTEL_DS_INST_CACHE_INVALIDATE_BIT, },
      { .anv = ANV_PIPE_DEPTH_STALL_BIT,                  .ds = INTEL_DS_DEPTH_STALL_BIT, },
      { .anv = ANV_PIPE_CS_STALL_BIT,                     .ds = INTEL_DS_CS_STALL_BIT, },
      { .anv = ANV_PIPE_HDC_PIPELINE_FLUSH_BIT,           .ds = INTEL_DS_HDC_PIPELINE_FLUSH_BIT, },
      { .anv = ANV_PIPE_STALL_AT_SCOREBOARD_BIT,          .ds = INTEL_DS_STALL_AT_SCOREBOARD_BIT, },
      { .anv = ANV_PIPE_UNTYPED_DATAPORT_CACHE_FLUSH_BIT, .ds = INTEL_DS_UNTYPED_DATAPORT_CACHE_FLUSH_BIT, },
   };

   enum intel_ds_stall_flag ret = 0;
   for (uint32_t i = 0; i < ARRAY_SIZE(anv_to_ds_flags); i++) {
      if (anv_to_ds_flags[i].anv & bits)
         ret |= anv_to_ds_flags[i].ds;
   }

   return ret;
}
