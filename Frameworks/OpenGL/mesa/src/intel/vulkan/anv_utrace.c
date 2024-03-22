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
#include "anv_internal_kernels.h"

#include "ds/intel_tracepoints.h"
#include "genxml/gen9_pack.h"
#include "perf/intel_perf.h"
#include "util/perf/cpu_trace.h"

#include "vulkan/runtime/vk_common_entrypoints.h"

/** Timestamp structure format */
union anv_utrace_timestamp {
   /* Timestamp writtem by either 2 * MI_STORE_REGISTER_MEM or
    * PIPE_CONTROL.
    */
   uint64_t timestamp;

   /* Timestamp written by COMPUTE_WALKER::PostSync
    *
    * Layout is described in PRMs.
    * ATSM PRMs, Volume 2d: Command Reference: Structures, POSTSYNC_DATA:
    *
    *    "The timestamp layout :
    *        [0] = 32b Context Timestamp Start
    *        [1] = 32b Global Timestamp Start
    *        [2] = 32b Context Timestamp End
    *        [3] = 32b Global Timestamp End"
    */
   uint32_t compute_walker[4];
};

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
anv_utrace_delete_submit(struct u_trace_context *utctx, void *submit_data)
{
   struct anv_device *device =
      container_of(utctx, struct anv_device, ds.trace_context);
   struct anv_utrace_submit *submit = submit_data;

   intel_ds_flush_data_fini(&submit->ds);

   anv_state_stream_finish(&submit->dynamic_state_stream);
   anv_state_stream_finish(&submit->general_state_stream);

   if (submit->trace_bo)
      anv_bo_pool_free(&device->utrace_bo_pool, submit->trace_bo);

   util_dynarray_foreach(&submit->batch_bos, struct anv_bo *, bo)
      anv_bo_pool_free(&device->utrace_bo_pool, *bo);
   util_dynarray_fini(&submit->batch_bos);

   vk_sync_destroy(&device->vk, submit->sync);

   vk_free(&device->vk.alloc, submit);
}

static void
anv_device_utrace_emit_gfx_copy_ts_buffer(struct u_trace_context *utctx,
                                          void *cmdstream,
                                          void *ts_from, uint32_t from_offset,
                                          void *ts_to, uint32_t to_offset,
                                          uint32_t count)
{
   struct anv_device *device =
      container_of(utctx, struct anv_device, ds.trace_context);
   struct anv_utrace_submit *submit = cmdstream;
   struct anv_address from_addr = (struct anv_address) {
      .bo = ts_from, .offset = from_offset * sizeof(union anv_utrace_timestamp) };
   struct anv_address to_addr = (struct anv_address) {
      .bo = ts_to, .offset = to_offset * sizeof(union anv_utrace_timestamp) };

   anv_genX(device->info, emit_so_memcpy)(&submit->memcpy_state,
                                          to_addr, from_addr,
                                          count * sizeof(union anv_utrace_timestamp));
}

static void
anv_device_utrace_emit_cs_copy_ts_buffer(struct u_trace_context *utctx,
                                         void *cmdstream,
                                         void *ts_from, uint32_t from_offset,
                                         void *ts_to, uint32_t to_offset,
                                         uint32_t count)
{
   struct anv_device *device =
      container_of(utctx, struct anv_device, ds.trace_context);
   struct anv_utrace_submit *submit = cmdstream;
   struct anv_address from_addr = (struct anv_address) {
      .bo = ts_from, .offset = from_offset * sizeof(union anv_utrace_timestamp) };
   struct anv_address to_addr = (struct anv_address) {
      .bo = ts_to, .offset = to_offset * sizeof(union anv_utrace_timestamp) };

   struct anv_state push_data_state =
      anv_genX(device->info, simple_shader_alloc_push)(
         &submit->simple_state, sizeof(struct anv_memcpy_params));
   struct anv_memcpy_params *params = push_data_state.map;

   *params = (struct anv_memcpy_params) {
      .copy = {
         .num_dwords = count * sizeof(union anv_utrace_timestamp) / 4,
      },
      .src_addr = anv_address_physical(from_addr),
      .dst_addr = anv_address_physical(to_addr),
   };

   anv_genX(device->info, emit_simple_shader_dispatch)(
      &submit->simple_state, DIV_ROUND_UP(params->copy.num_dwords, 4),
      push_data_state);
}

static VkResult
anv_utrace_submit_extend_batch(struct anv_batch *batch, uint32_t size,
                               void *user_data)
{
   struct anv_utrace_submit *submit = user_data;

   uint32_t alloc_size = 0;
   util_dynarray_foreach(&submit->batch_bos, struct anv_bo *, bo)
      alloc_size += (*bo)->size;
   alloc_size = MAX2(alloc_size * 2, 8192);

   struct anv_bo *bo;
   VkResult result = anv_bo_pool_alloc(&submit->queue->device->utrace_bo_pool,
                                       align(alloc_size, 4096),
                                       &bo);
   if (result != VK_SUCCESS)
      return result;

   util_dynarray_append(&submit->batch_bos, struct anv_bo *, bo);

   batch->end += 4 * GFX9_MI_BATCH_BUFFER_START_length;

   anv_batch_emit(batch, GFX9_MI_BATCH_BUFFER_START, bbs) {
      bbs.DWordLength               = GFX9_MI_BATCH_BUFFER_START_length -
                                      GFX9_MI_BATCH_BUFFER_START_length_bias;
      bbs.SecondLevelBatchBuffer    = Firstlevelbatch;
      bbs.AddressSpaceIndicator     = ASI_PPGTT;
      bbs.BatchBufferStartAddress   = (struct anv_address) { bo, 0 };
   }

   anv_batch_set_storage(batch,
                         (struct anv_address) { .bo = bo, },
                         bo->map,
                         bo->size - 4 * GFX9_MI_BATCH_BUFFER_START_length);

   return VK_SUCCESS;
}

VkResult
anv_device_utrace_flush_cmd_buffers(struct anv_queue *queue,
                                    uint32_t cmd_buffer_count,
                                    struct anv_cmd_buffer **cmd_buffers,
                                    struct anv_utrace_submit **out_submit)
{
   struct anv_device *device = queue->device;
   uint32_t utrace_copies = 0;
   uint32_t utraces = command_buffers_count_utraces(device,
                                                    cmd_buffer_count,
                                                    cmd_buffers,
                                                    &utrace_copies);
   if (!utraces) {
      *out_submit = NULL;
      return VK_SUCCESS;
   }

   VkResult result;
   struct anv_utrace_submit *submit =
      vk_zalloc(&device->vk.alloc, sizeof(struct anv_utrace_submit),
                8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!submit)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   submit->queue = queue;

   intel_ds_flush_data_init(&submit->ds, &queue->ds, queue->ds.submission_id);

   result = vk_sync_create(&device->vk, &device->physical->sync_syncobj_type,
                           0, 0, &submit->sync);
   if (result != VK_SUCCESS)
      goto error_sync;

   util_dynarray_init(&submit->batch_bos, NULL);

   if (utrace_copies > 0) {
      result = anv_bo_pool_alloc(&device->utrace_bo_pool,
                                 utrace_copies * 4096,
                                 &submit->trace_bo);
      if (result != VK_SUCCESS)
         goto error_trace_buf;

      const bool uses_relocs = device->physical->uses_relocs;
      result = anv_reloc_list_init(&submit->relocs, &device->vk.alloc, uses_relocs);
      if (result != VK_SUCCESS)
         goto error_reloc_list;

      anv_state_stream_init(&submit->dynamic_state_stream,
                            &device->dynamic_state_pool, 16384);
      anv_state_stream_init(&submit->general_state_stream,
                            &device->general_state_pool, 16384);

      submit->batch = (struct anv_batch) {
         .alloc = &device->vk.alloc,
         .relocs = &submit->relocs,
         .user_data = submit,
         .extend_cb = anv_utrace_submit_extend_batch,
      };

      /* Only engine class where we support timestamp copies
       *
       * TODO: add INTEL_ENGINE_CLASS_COPY support (should be trivial ;)
       */
      assert(queue->family->engine_class == INTEL_ENGINE_CLASS_RENDER ||
             queue->family->engine_class == INTEL_ENGINE_CLASS_COMPUTE);
      if (queue->family->engine_class == INTEL_ENGINE_CLASS_RENDER) {

         trace_intel_begin_trace_copy_cb(&submit->ds.trace, &submit->batch);

         anv_genX(device->info, emit_so_memcpy_init)(&submit->memcpy_state,
                                                     device,
                                                     &submit->batch);
         uint32_t num_traces = 0;
         for (uint32_t i = 0; i < cmd_buffer_count; i++) {
            if (cmd_buffers[i]->usage_flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) {
               intel_ds_queue_flush_data(&queue->ds, &cmd_buffers[i]->trace,
                                         &submit->ds, false);
            } else {
               num_traces += cmd_buffers[i]->trace.num_traces;
               u_trace_clone_append(u_trace_begin_iterator(&cmd_buffers[i]->trace),
                                    u_trace_end_iterator(&cmd_buffers[i]->trace),
                                    &submit->ds.trace,
                                    submit,
                                    anv_device_utrace_emit_gfx_copy_ts_buffer);
            }
         }
         anv_genX(device->info, emit_so_memcpy_fini)(&submit->memcpy_state);

         trace_intel_end_trace_copy_cb(&submit->ds.trace, &submit->batch,
                                       num_traces);

         anv_genX(device->info, emit_so_memcpy_end)(&submit->memcpy_state);
      } else {
         trace_intel_begin_trace_copy_cb(&submit->ds.trace, &submit->batch);

         submit->simple_state = (struct anv_simple_shader) {
            .device               = device,
            .dynamic_state_stream = &submit->dynamic_state_stream,
            .general_state_stream = &submit->general_state_stream,
            .batch                = &submit->batch,
            .kernel               = device->internal_kernels[
               ANV_INTERNAL_KERNEL_MEMCPY_COMPUTE],
            .l3_config            = device->internal_kernels_l3_config,
         };
         anv_genX(device->info, emit_simple_shader_init)(&submit->simple_state);

         uint32_t num_traces = 0;
         for (uint32_t i = 0; i < cmd_buffer_count; i++) {
            num_traces += cmd_buffers[i]->trace.num_traces;
            if (cmd_buffers[i]->usage_flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) {
               intel_ds_queue_flush_data(&queue->ds, &cmd_buffers[i]->trace,
                                         &submit->ds, false);
            } else {
               num_traces += cmd_buffers[i]->trace.num_traces;
               u_trace_clone_append(u_trace_begin_iterator(&cmd_buffers[i]->trace),
                                    u_trace_end_iterator(&cmd_buffers[i]->trace),
                                    &submit->ds.trace,
                                    submit,
                                    anv_device_utrace_emit_cs_copy_ts_buffer);
            }
         }

         trace_intel_end_trace_copy_cb(&submit->ds.trace, &submit->batch,
                                       num_traces);

         anv_genX(device->info, emit_simple_shader_end)(&submit->simple_state);
      }

      intel_ds_queue_flush_data(&queue->ds, &submit->ds.trace, &submit->ds, true);

      if (submit->batch.status != VK_SUCCESS) {
         result = submit->batch.status;
         goto error_batch;
      }
   } else {
      for (uint32_t i = 0; i < cmd_buffer_count; i++) {
         assert(cmd_buffers[i]->usage_flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
         intel_ds_queue_flush_data(&queue->ds, &cmd_buffers[i]->trace,
                                   &submit->ds, i == (cmd_buffer_count - 1));
      }
   }

   *out_submit = submit;

   return VK_SUCCESS;

 error_batch:
   anv_reloc_list_finish(&submit->relocs);
   util_dynarray_foreach(&submit->batch_bos, struct anv_bo *, bo)
      anv_bo_pool_free(&device->utrace_bo_pool, *bo);
 error_reloc_list:
   anv_bo_pool_free(&device->utrace_bo_pool, submit->trace_bo);
 error_trace_buf:
   vk_sync_destroy(&device->vk, submit->sync);
 error_sync:
   intel_ds_flush_data_fini(&submit->ds);
   vk_free(&device->vk.alloc, submit);
   return result;
}

static void *
anv_utrace_create_ts_buffer(struct u_trace_context *utctx, uint32_t size_b)
{
   struct anv_device *device =
      container_of(utctx, struct anv_device, ds.trace_context);

   uint32_t anv_ts_size_b = (size_b / sizeof(uint64_t)) *
      sizeof(union anv_utrace_timestamp);

   struct anv_bo *bo = NULL;
   UNUSED VkResult result =
      anv_bo_pool_alloc(&device->utrace_bo_pool,
                        align(anv_ts_size_b, 4096),
                        &bo);
   assert(result == VK_SUCCESS);

   memset(bo->map, 0, bo->size);
#ifdef SUPPORT_INTEL_INTEGRATED_GPUS
   if (device->physical->memory.need_flush &&
       anv_bo_needs_host_cache_flush(bo->alloc_flags))
      intel_flush_range(bo->map, bo->size);
#endif

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
   struct anv_device *device =
      container_of(ut->utctx, struct anv_device, ds.trace_context);
   struct anv_cmd_buffer *cmd_buffer =
      container_of(ut, struct anv_cmd_buffer, trace);
   /* cmd_buffer is only valid if cs == NULL */
   struct anv_batch *batch = cs != NULL ? cs : &cmd_buffer->batch;
   struct anv_bo *bo = timestamps;

   struct anv_address ts_address = (struct anv_address) {
      .bo = bo,
      .offset = idx * sizeof(union anv_utrace_timestamp)
   };

   /* Is this a end of compute trace point? */
   const bool is_end_compute =
      cs == NULL &&
      (cmd_buffer->last_compute_walker != NULL ||
       cmd_buffer->last_indirect_dispatch != NULL) &&
      end_of_pipe;

   enum anv_timestamp_capture_type capture_type = end_of_pipe ?
      (is_end_compute ?
       (cmd_buffer->last_indirect_dispatch != NULL ?
        ANV_TIMESTAMP_REWRITE_INDIRECT_DISPATCH : ANV_TIMESTAMP_REWRITE_COMPUTE_WALKER) :
       ANV_TIMESTAMP_CAPTURE_END_OF_PIPE) : ANV_TIMESTAMP_CAPTURE_TOP_OF_PIPE;

   void *addr = capture_type ==  ANV_TIMESTAMP_REWRITE_INDIRECT_DISPATCH ?
                cmd_buffer->last_indirect_dispatch :
                capture_type ==  ANV_TIMESTAMP_REWRITE_COMPUTE_WALKER ?
                cmd_buffer->last_compute_walker : NULL;

   device->physical->cmd_emit_timestamp(batch, device, ts_address,
                                        capture_type,
                                        addr);
   if (is_end_compute) {
      cmd_buffer->last_compute_walker = NULL;
      cmd_buffer->last_indirect_dispatch = NULL;
   }
}

static uint64_t
anv_utrace_read_ts(struct u_trace_context *utctx,
                   void *timestamps, unsigned idx, void *flush_data)
{
   struct anv_device *device =
      container_of(utctx, struct anv_device, ds.trace_context);
   struct anv_bo *bo = timestamps;
   struct anv_utrace_submit *submit = flush_data;

   /* Only need to stall on results for the first entry: */
   if (idx == 0) {
      MESA_TRACE_SCOPE("anv utrace wait timestamps");
      UNUSED VkResult result =
         vk_sync_wait(&device->vk,
                      submit->sync,
                      0,
                      VK_SYNC_WAIT_COMPLETE,
                      os_time_get_absolute_timeout(OS_TIMEOUT_INFINITE));
      assert(result == VK_SUCCESS);
   }

   union anv_utrace_timestamp *ts = (union anv_utrace_timestamp *)bo->map;

   /* Don't translate the no-timestamp marker: */
   if (ts[idx].timestamp == U_TRACE_NO_TIMESTAMP)
      return U_TRACE_NO_TIMESTAMP;

   /* Detect a 16bytes timestamp write */
   if (ts[idx].compute_walker[2] != 0 || ts[idx].compute_walker[3] != 0) {
      /* The timestamp written by COMPUTE_WALKER::PostSync only as 32bits. We
       * need to rebuild the full 64bits using the previous timestamp. We
       * assume that utrace is reading the timestamp in order. Anyway
       * timestamp rollover on 32bits in a few minutes so in most cases that
       * should be correct.
       */
      uint64_t timestamp =
         (submit->last_full_timestamp & 0xffffffff00000000) |
         (uint64_t) ts[idx].compute_walker[3];

      return intel_device_info_timebase_scale(device->info, timestamp);
   }

   submit->last_full_timestamp = ts[idx].timestamp;

   return intel_device_info_timebase_scale(device->info, ts[idx].timestamp);
}

void
anv_device_utrace_init(struct anv_device *device)
{
   anv_bo_pool_init(&device->utrace_bo_pool, device, "utrace",
                    ANV_BO_ALLOC_MAPPED | ANV_BO_ALLOC_HOST_CACHED_COHERENT);
   intel_ds_device_init(&device->ds, device->info, device->fd,
                        device->physical->local_minor,
                        INTEL_DS_API_VULKAN);
   u_trace_context_init(&device->ds.trace_context,
                        &device->ds,
                        anv_utrace_create_ts_buffer,
                        anv_utrace_destroy_ts_buffer,
                        anv_utrace_record_ts,
                        anv_utrace_read_ts,
                        anv_utrace_delete_submit);

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
      { .anv = ANV_PIPE_PSS_STALL_SYNC_BIT,               .ds = INTEL_DS_PSS_STALL_SYNC_BIT, },
      { .anv = ANV_PIPE_END_OF_PIPE_SYNC_BIT,             .ds = INTEL_DS_END_OF_PIPE_BIT, },
      { .anv = ANV_PIPE_CCS_CACHE_FLUSH_BIT,              .ds = INTEL_DS_CCS_CACHE_FLUSH_BIT, },
   };

   enum intel_ds_stall_flag ret = 0;
   for (uint32_t i = 0; i < ARRAY_SIZE(anv_to_ds_flags); i++) {
      if (anv_to_ds_flags[i].anv & bits)
         ret |= anv_to_ds_flags[i].ds;
   }

   return ret;
}

void anv_CmdBeginDebugUtilsLabelEXT(
   VkCommandBuffer _commandBuffer,
   const VkDebugUtilsLabelEXT *pLabelInfo)
{
   VK_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, _commandBuffer);

   vk_common_CmdBeginDebugUtilsLabelEXT(_commandBuffer, pLabelInfo);

   trace_intel_begin_cmd_buffer_annotation(&cmd_buffer->trace);
}

void anv_CmdEndDebugUtilsLabelEXT(VkCommandBuffer _commandBuffer)
{
   VK_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, _commandBuffer);

   if (cmd_buffer->vk.labels.size > 0) {
      const VkDebugUtilsLabelEXT *label =
         util_dynarray_top_ptr(&cmd_buffer->vk.labels, VkDebugUtilsLabelEXT);

      trace_intel_end_cmd_buffer_annotation(&cmd_buffer->trace,
                                            strlen(label->pLabelName),
                                            label->pLabelName);
   }

   vk_common_CmdEndDebugUtilsLabelEXT(_commandBuffer);
}

void
anv_queue_trace(struct anv_queue *queue, const char *label, bool frame, bool begin)
{
   struct anv_device *device = queue->device;

   VkResult result;
   struct anv_utrace_submit *submit =
      vk_zalloc(&device->vk.alloc, sizeof(struct anv_utrace_submit),
                8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!submit)
      return;

   submit->queue = queue;

   intel_ds_flush_data_init(&submit->ds, &queue->ds, queue->ds.submission_id);

   result = vk_sync_create(&device->vk, &device->physical->sync_syncobj_type,
                           0, 0, &submit->sync);
   if (result != VK_SUCCESS)
      goto error_trace;

   const bool uses_relocs = device->physical->uses_relocs;
   result = anv_reloc_list_init(&submit->relocs, &device->vk.alloc, uses_relocs);
   if (result != VK_SUCCESS)
      goto error_sync;

   submit->batch = (struct anv_batch) {
      .alloc = &device->vk.alloc,
      .relocs = &submit->relocs,
      .user_data = submit,
      .extend_cb = anv_utrace_submit_extend_batch,
   };

   if (frame) {
      if (begin)
         trace_intel_begin_frame(&submit->ds.trace, &submit->batch);
      else
         trace_intel_end_frame(&submit->ds.trace, &submit->batch,
                               device->debug_frame_desc->frame_id);
   } else {
      if (begin) {
         trace_intel_begin_queue_annotation(&submit->ds.trace, &submit->batch);
      } else {
         trace_intel_end_queue_annotation(&submit->ds.trace,
                                          &submit->batch,
                                          strlen(label),
                                          label);
      }
   }

   anv_batch_emit(&submit->batch, GFX9_MI_BATCH_BUFFER_END, bbs);
   anv_batch_emit(&submit->batch, GFX9_MI_NOOP, noop);

   if (submit->batch.status != VK_SUCCESS) {
      result = submit->batch.status;
      goto error_reloc_list;
   }

   intel_ds_queue_flush_data(&queue->ds, &submit->ds.trace, &submit->ds, true);

   pthread_mutex_lock(&device->mutex);
   device->kmd_backend->queue_exec_trace(queue, submit);
   pthread_mutex_unlock(&device->mutex);

   return;

 error_reloc_list:
   anv_reloc_list_finish(&submit->relocs);
   util_dynarray_foreach(&submit->batch_bos, struct anv_bo *, bo)
      anv_bo_pool_free(&device->utrace_bo_pool, *bo);
 error_sync:
   vk_sync_destroy(&device->vk, submit->sync);
 error_trace:
   intel_ds_flush_data_fini(&submit->ds);
   vk_free(&device->vk.alloc, submit);
}

void
anv_QueueBeginDebugUtilsLabelEXT(
   VkQueue _queue,
   const VkDebugUtilsLabelEXT *pLabelInfo)
{
   VK_FROM_HANDLE(anv_queue, queue, _queue);

   vk_common_QueueBeginDebugUtilsLabelEXT(_queue, pLabelInfo);

   anv_queue_trace(queue, pLabelInfo->pLabelName,
                   false /* frame */, true /* begin */);
}

void
anv_QueueEndDebugUtilsLabelEXT(VkQueue _queue)
{
   VK_FROM_HANDLE(anv_queue, queue, _queue);

   if (queue->vk.labels.size > 0) {
      const VkDebugUtilsLabelEXT *label =
         util_dynarray_top_ptr(&queue->vk.labels, VkDebugUtilsLabelEXT);
      anv_queue_trace(queue, label->pLabelName,
                      false /* frame */, false /* begin */);

      intel_ds_device_process(&queue->device->ds, true);
   }

   vk_common_QueueEndDebugUtilsLabelEXT(_queue);
}
