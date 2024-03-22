/*
 * Copyright © 2015 Intel Corporation
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

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "anv_private.h"

#include "util/os_time.h"

#include "genxml/gen_macros.h"
#include "genxml/genX_pack.h"

/* We reserve :
 *    - GPR 14 for perf queries
 *    - GPR 15 for conditional rendering
 */
#define MI_BUILDER_NUM_ALLOC_GPRS 14
#define MI_BUILDER_CAN_WRITE_BATCH GFX_VER >= 8
#define __gen_get_batch_dwords anv_batch_emit_dwords
#define __gen_address_offset anv_address_add
#define __gen_get_batch_address(b, a) anv_batch_address(b, a)
#include "common/mi_builder.h"
#include "perf/intel_perf.h"
#include "perf/intel_perf_mdapi.h"
#include "perf/intel_perf_regs.h"

#include "vk_util.h"

static struct anv_address
anv_query_address(struct anv_query_pool *pool, uint32_t query)
{
   return (struct anv_address) {
      .bo = pool->bo,
      .offset = query * pool->stride,
   };
}

VkResult genX(CreateQueryPool)(
    VkDevice                                    _device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   const struct anv_physical_device *pdevice = device->physical;
#if GFX_VER >= 8
   const VkQueryPoolPerformanceCreateInfoKHR *perf_query_info = NULL;
   struct intel_perf_counter_pass *counter_pass;
   struct intel_perf_query_info **pass_query;
   uint32_t n_passes = 0;
#endif
   uint32_t data_offset = 0;
   VK_MULTIALLOC(ma);
   VkResult result;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO);

   /* Query pool slots are made up of some number of 64-bit values packed
    * tightly together. For most query types have the first 64-bit value is
    * the "available" bit which is 0 when the query is unavailable and 1 when
    * it is available. The 64-bit values that follow are determined by the
    * type of query.
    *
    * For performance queries, we have a requirement to align OA reports at
    * 64bytes so we put those first and have the "available" bit behind
    * together with some other counters.
    */
   uint32_t uint64s_per_slot = 0;

   VK_MULTIALLOC_DECL(&ma, struct anv_query_pool, pool, 1);

   VkQueryPipelineStatisticFlags pipeline_statistics = 0;
   switch (pCreateInfo->queryType) {
   case VK_QUERY_TYPE_OCCLUSION:
      /* Occlusion queries have two values: begin and end. */
      uint64s_per_slot = 1 + 2;
      break;
   case VK_QUERY_TYPE_TIMESTAMP:
      /* Timestamps just have the one timestamp value */
      uint64s_per_slot = 1 + 1;
      break;
   case VK_QUERY_TYPE_PIPELINE_STATISTICS:
      pipeline_statistics = pCreateInfo->pipelineStatistics;
      /* We're going to trust this field implicitly so we need to ensure that
       * no unhandled extension bits leak in.
       */
      pipeline_statistics &= ANV_PIPELINE_STATISTICS_MASK;

      /* Statistics queries have a min and max for every statistic */
      uint64s_per_slot = 1 + 2 * util_bitcount(pipeline_statistics);
      break;
   case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
      /* Transform feedback queries are 4 values, begin/end for
       * written/available.
       */
      uint64s_per_slot = 1 + 4;
      break;
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL: {
      const struct intel_perf_query_field_layout *layout =
         &pdevice->perf->query_layout;

      uint64s_per_slot = 2; /* availability + marker */
      /* Align to the requirement of the layout */
      uint64s_per_slot = align(uint64s_per_slot,
                               DIV_ROUND_UP(layout->alignment, sizeof(uint64_t)));
      data_offset = uint64s_per_slot * sizeof(uint64_t);
      /* Add the query data for begin & end commands */
      uint64s_per_slot += 2 * DIV_ROUND_UP(layout->size, sizeof(uint64_t));
      break;
   }
#if GFX_VER >= 8
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
      const struct intel_perf_query_field_layout *layout =
         &pdevice->perf->query_layout;

      perf_query_info = vk_find_struct_const(pCreateInfo->pNext,
                                             QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR);
      n_passes = intel_perf_get_n_passes(pdevice->perf,
                                         perf_query_info->pCounterIndices,
                                         perf_query_info->counterIndexCount,
                                         NULL);
      vk_multialloc_add(&ma, &counter_pass, struct intel_perf_counter_pass,
                             perf_query_info->counterIndexCount);
      vk_multialloc_add(&ma, &pass_query, struct intel_perf_query_info *,
                             n_passes);
      uint64s_per_slot = 4 /* availability + small batch */;
      /* Align to the requirement of the layout */
      uint64s_per_slot = align(uint64s_per_slot,
                               DIV_ROUND_UP(layout->alignment, sizeof(uint64_t)));
      data_offset = uint64s_per_slot * sizeof(uint64_t);
      /* Add the query data for begin & end commands */
      uint64s_per_slot += 2 * DIV_ROUND_UP(layout->size, sizeof(uint64_t));
      /* Multiply by the number of passes */
      uint64s_per_slot *= n_passes;
      break;
   }
#endif
   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
      /* Query has two values: begin and end. */
      uint64s_per_slot = 1 + 2;
      break;
   default:
      assert(!"Invalid query type");
   }

   if (!vk_object_multialloc(&device->vk, &ma, pAllocator,
                             VK_OBJECT_TYPE_QUERY_POOL))
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pool->type = pCreateInfo->queryType;
   pool->pipeline_statistics = pipeline_statistics;
   pool->stride = uint64s_per_slot * sizeof(uint64_t);
   pool->slots = pCreateInfo->queryCount;

   if (pool->type == VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL) {
      pool->data_offset = data_offset;
      pool->snapshot_size = (pool->stride - data_offset) / 2;
   }
#if GFX_VER >= 8
   else if (pool->type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
      pool->pass_size = pool->stride / n_passes;
      pool->data_offset = data_offset;
      pool->snapshot_size = (pool->pass_size - data_offset) / 2;
      pool->n_counters = perf_query_info->counterIndexCount;
      pool->counter_pass = counter_pass;
      intel_perf_get_counters_passes(pdevice->perf,
                                     perf_query_info->pCounterIndices,
                                     perf_query_info->counterIndexCount,
                                     pool->counter_pass);
      pool->n_passes = n_passes;
      pool->pass_query = pass_query;
      intel_perf_get_n_passes(pdevice->perf,
                              perf_query_info->pCounterIndices,
                              perf_query_info->counterIndexCount,
                              pool->pass_query);
   }
#endif

   uint64_t size = pool->slots * (uint64_t)pool->stride;
   result = anv_device_alloc_bo(device, "query-pool", size,
                                ANV_BO_ALLOC_MAPPED |
                                ANV_BO_ALLOC_SNOOPED,
                                0 /* explicit_address */,
                                &pool->bo);
   if (result != VK_SUCCESS)
      goto fail;

#if GFX_VER >= 8
   if (pool->type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
      for (uint32_t p = 0; p < pool->n_passes; p++) {
         struct mi_builder b;
         struct anv_batch batch = {
            .start = pool->bo->map + khr_perf_query_preamble_offset(pool, p),
            .end = pool->bo->map + khr_perf_query_preamble_offset(pool, p) + pool->data_offset,
         };
         batch.next = batch.start;

         mi_builder_init(&b, device->info, &batch);
         mi_store(&b, mi_reg64(ANV_PERF_QUERY_OFFSET_REG),
                      mi_imm(p * (uint64_t)pool->pass_size));
         anv_batch_emit(&batch, GENX(MI_BATCH_BUFFER_END), bbe);
      }
   }
#endif

   *pQueryPool = anv_query_pool_to_handle(pool);

   return VK_SUCCESS;

 fail:
   vk_free2(&device->vk.alloc, pAllocator, pool);

   return result;
}

void genX(DestroyQueryPool)(
    VkDevice                                    _device,
    VkQueryPool                                 _pool,
    const VkAllocationCallbacks*                pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_query_pool, pool, _pool);

   if (!pool)
      return;

   anv_device_release_bo(device, pool->bo);
   vk_object_free(&device->vk, pAllocator, pool);
}

#if GFX_VER >= 8
/**
 * VK_KHR_performance_query layout  :
 *
 * --------------------------------------------
 * |       availability (8b)       | |        |
 * |-------------------------------| |        |
 * |      Small batch loading      | |        |
 * |   ANV_PERF_QUERY_OFFSET_REG   | |        |
 * |            (24b)              | | Pass 0 |
 * |-------------------------------| |        |
 * |       some padding (see       | |        |
 * | query_field_layout:alignment) | |        |
 * |-------------------------------| |        |
 * |           query data          | |        |
 * | (2 * query_field_layout:size) | |        |
 * |-------------------------------|--        | Query 0
 * |       availability (8b)       | |        |
 * |-------------------------------| |        |
 * |      Small batch loading      | |        |
 * |   ANV_PERF_QUERY_OFFSET_REG   | |        |
 * |            (24b)              | | Pass 1 |
 * |-------------------------------| |        |
 * |       some padding (see       | |        |
 * | query_field_layout:alignment) | |        |
 * |-------------------------------| |        |
 * |           query data          | |        |
 * | (2 * query_field_layout:size) | |        |
 * |-------------------------------|-----------
 * |       availability (8b)       | |        |
 * |-------------------------------| |        |
 * |      Small batch loading      | |        |
 * |   ANV_PERF_QUERY_OFFSET_REG   | |        |
 * |            (24b)              | | Pass 0 |
 * |-------------------------------| |        |
 * |       some padding (see       | |        |
 * | query_field_layout:alignment) | |        |
 * |-------------------------------| |        |
 * |           query data          | |        |
 * | (2 * query_field_layout:size) | |        |
 * |-------------------------------|--        | Query 1
 * |               ...             | |        |
 * --------------------------------------------
 */

static uint64_t
khr_perf_query_availability_offset(struct anv_query_pool *pool, uint32_t query, uint32_t pass)
{
   return query * (uint64_t)pool->stride + pass * (uint64_t)pool->pass_size;
}

static uint64_t
khr_perf_query_data_offset(struct anv_query_pool *pool, uint32_t query, uint32_t pass, bool end)
{
   return query * (uint64_t)pool->stride + pass * (uint64_t)pool->pass_size +
      pool->data_offset + (end ? pool->snapshot_size : 0);
}

static struct anv_address
khr_perf_query_availability_address(struct anv_query_pool *pool, uint32_t query, uint32_t pass)
{
   return anv_address_add(
      (struct anv_address) { .bo = pool->bo, },
      khr_perf_query_availability_offset(pool, query, pass));
}

static struct anv_address
khr_perf_query_data_address(struct anv_query_pool *pool, uint32_t query, uint32_t pass, bool end)
{
   return anv_address_add(
      (struct anv_address) { .bo = pool->bo, },
      khr_perf_query_data_offset(pool, query, pass, end));
}

static bool
khr_perf_query_ensure_relocs(struct anv_cmd_buffer *cmd_buffer)
{
   if (anv_batch_has_error(&cmd_buffer->batch))
      return false;

   if (cmd_buffer->self_mod_locations)
      return true;

   struct anv_device *device = cmd_buffer->device;
   const struct anv_physical_device *pdevice = device->physical;

   cmd_buffer->self_mod_locations =
      vk_alloc(&cmd_buffer->vk.pool->alloc,
               pdevice->n_perf_query_commands * sizeof(*cmd_buffer->self_mod_locations), 8,
               VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

   if (!cmd_buffer->self_mod_locations) {
      anv_batch_set_error(&cmd_buffer->batch, VK_ERROR_OUT_OF_HOST_MEMORY);
      return false;
   }

   return true;
}
#endif

/**
 * VK_INTEL_performance_query layout :
 *
 * ---------------------------------
 * |       availability (8b)       |
 * |-------------------------------|
 * |          marker (8b)          |
 * |-------------------------------|
 * |       some padding (see       |
 * | query_field_layout:alignment) |
 * |-------------------------------|
 * |           query data          |
 * | (2 * query_field_layout:size) |
 * ---------------------------------
 */

static uint32_t
intel_perf_marker_offset(void)
{
   return 8;
}

static uint32_t
intel_perf_query_data_offset(struct anv_query_pool *pool, bool end)
{
   return pool->data_offset + (end ? pool->snapshot_size : 0);
}

static void
cpu_write_query_result(void *dst_slot, VkQueryResultFlags flags,
                       uint32_t value_index, uint64_t result)
{
   if (flags & VK_QUERY_RESULT_64_BIT) {
      uint64_t *dst64 = dst_slot;
      dst64[value_index] = result;
   } else {
      uint32_t *dst32 = dst_slot;
      dst32[value_index] = result;
   }
}

static void *
query_slot(struct anv_query_pool *pool, uint32_t query)
{
   return pool->bo->map + query * pool->stride;
}

static bool
query_is_available(struct anv_query_pool *pool, uint32_t query)
{
#if GFX_VER >= 8
   if (pool->type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
      for (uint32_t p = 0; p < pool->n_passes; p++) {
         volatile uint64_t *slot =
            pool->bo->map + khr_perf_query_availability_offset(pool, query, p);
         if (!slot[0])
            return false;
      }
      return true;
   }
#endif

   return *(volatile uint64_t *)query_slot(pool, query);
}

static VkResult
wait_for_available(struct anv_device *device,
                   struct anv_query_pool *pool, uint32_t query)
{
   uint64_t abs_timeout_ns = os_time_get_absolute_timeout(2 * NSEC_PER_SEC);

   while (os_time_get_nano() < abs_timeout_ns) {
      if (query_is_available(pool, query))
         return VK_SUCCESS;
      VkResult status = vk_device_check_status(&device->vk);
      if (status != VK_SUCCESS)
         return status;
   }

   return vk_device_set_lost(&device->vk, "query timeout");
}

VkResult genX(GetQueryPoolResults)(
    VkDevice                                    _device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_query_pool, pool, queryPool);

   assert(pool->type == VK_QUERY_TYPE_OCCLUSION ||
          pool->type == VK_QUERY_TYPE_PIPELINE_STATISTICS ||
          pool->type == VK_QUERY_TYPE_TIMESTAMP ||
          pool->type == VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT ||
          pool->type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR ||
          pool->type == VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL ||
          pool->type == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT);

   if (vk_device_is_lost(&device->vk))
      return VK_ERROR_DEVICE_LOST;

   if (pData == NULL)
      return VK_SUCCESS;

   void *data_end = pData + dataSize;

   VkResult status = VK_SUCCESS;
   for (uint32_t i = 0; i < queryCount; i++) {
      bool available = query_is_available(pool, firstQuery + i);

      if (!available && (flags & VK_QUERY_RESULT_WAIT_BIT)) {
         status = wait_for_available(device, pool, firstQuery + i);
         if (status != VK_SUCCESS) {
            return status;
         }

         available = true;
      }

      /* From the Vulkan 1.0.42 spec:
       *
       *    "If VK_QUERY_RESULT_WAIT_BIT and VK_QUERY_RESULT_PARTIAL_BIT are
       *    both not set then no result values are written to pData for
       *    queries that are in the unavailable state at the time of the call,
       *    and vkGetQueryPoolResults returns VK_NOT_READY. However,
       *    availability state is still written to pData for those queries if
       *    VK_QUERY_RESULT_WITH_AVAILABILITY_BIT is set."
       *
       * From VK_KHR_performance_query :
       *
       *    "VK_QUERY_RESULT_PERFORMANCE_QUERY_RECORDED_COUNTERS_BIT_KHR specifies
       *     that the result should contain the number of counters that were recorded
       *     into a query pool of type ename:VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR"
       */
      bool write_results = available || (flags & VK_QUERY_RESULT_PARTIAL_BIT);

      uint32_t idx = 0;
      switch (pool->type) {
      case VK_QUERY_TYPE_OCCLUSION:
      case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT: {
         uint64_t *slot = query_slot(pool, firstQuery + i);
         if (write_results) {
            /* From the Vulkan 1.2.132 spec:
             *
             *    "If VK_QUERY_RESULT_PARTIAL_BIT is set,
             *    VK_QUERY_RESULT_WAIT_BIT is not set, and the query’s status
             *    is unavailable, an intermediate result value between zero and
             *    the final result value is written to pData for that query."
             */
            uint64_t result = available ? slot[2] - slot[1] : 0;
            cpu_write_query_result(pData, flags, idx, result);
         }
         idx++;
         break;
      }

      case VK_QUERY_TYPE_PIPELINE_STATISTICS: {
         uint64_t *slot = query_slot(pool, firstQuery + i);
         uint32_t statistics = pool->pipeline_statistics;
         while (statistics) {
            uint32_t stat = u_bit_scan(&statistics);
            if (write_results) {
               uint64_t result = slot[idx * 2 + 2] - slot[idx * 2 + 1];

               /* WaDividePSInvocationCountBy4:HSW,BDW */
               if ((device->info->ver == 8 || device->info->verx10 == 75) &&
                   (1 << stat) == VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT)
                  result >>= 2;

               cpu_write_query_result(pData, flags, idx, result);
            }
            idx++;
         }
         assert(idx == util_bitcount(pool->pipeline_statistics));
         break;
      }

      case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT: {
         uint64_t *slot = query_slot(pool, firstQuery + i);
         if (write_results)
            cpu_write_query_result(pData, flags, idx, slot[2] - slot[1]);
         idx++;
         if (write_results)
            cpu_write_query_result(pData, flags, idx, slot[4] - slot[3]);
         idx++;
         break;
      }

      case VK_QUERY_TYPE_TIMESTAMP: {
         uint64_t *slot = query_slot(pool, firstQuery + i);
         if (write_results)
            cpu_write_query_result(pData, flags, idx, slot[1]);
         idx++;
         break;
      }

#if GFX_VER >= 8
      case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
         const struct anv_physical_device *pdevice = device->physical;
         assert((flags & (VK_QUERY_RESULT_WITH_AVAILABILITY_BIT |
                          VK_QUERY_RESULT_PARTIAL_BIT)) == 0);
         for (uint32_t p = 0; p < pool->n_passes; p++) {
            const struct intel_perf_query_info *query = pool->pass_query[p];
            struct intel_perf_query_result result;
            intel_perf_query_result_clear(&result);
            intel_perf_query_result_accumulate_fields(&result, query,
                                                      pool->bo->map + khr_perf_query_data_offset(pool, firstQuery + i, p, false),
                                                      pool->bo->map + khr_perf_query_data_offset(pool, firstQuery + i, p, true),
                                                      false /* no_oa_accumulate */);
            anv_perf_write_pass_results(pdevice->perf, pool, p, &result, pData);
         }
         break;
      }
#endif

      case VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL: {
         if (!write_results)
            break;
         const void *query_data = query_slot(pool, firstQuery + i);
         const struct intel_perf_query_info *query = &device->physical->perf->queries[0];
         struct intel_perf_query_result result;
         intel_perf_query_result_clear(&result);
         intel_perf_query_result_accumulate_fields(&result, query,
                                                   query_data + intel_perf_query_data_offset(pool, false),
                                                   query_data + intel_perf_query_data_offset(pool, true),
                                                   false /* no_oa_accumulate */);
         intel_perf_query_result_write_mdapi(pData, stride,
                                             device->info,
                                             query, &result);
         const uint64_t *marker = query_data + intel_perf_marker_offset();
         intel_perf_query_mdapi_write_marker(pData, stride, device->info, *marker);
         break;
      }

      default:
         unreachable("invalid pool type");
      }

      if (!write_results)
         status = VK_NOT_READY;

      if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)
         cpu_write_query_result(pData, flags, idx, available);

      pData += stride;
      if (pData >= data_end)
         break;
   }

   return status;
}

static void
emit_ps_depth_count(struct anv_cmd_buffer *cmd_buffer,
                    struct anv_address addr)
{
   cmd_buffer->state.pending_pipe_bits |= ANV_PIPE_POST_SYNC_BIT;
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
      pc.DestinationAddressType  = DAT_PPGTT;
      pc.PostSyncOperation       = WritePSDepthCount;
      pc.DepthStallEnable        = true;
      pc.Address                 = addr;

      if (GFX_VER == 9 && cmd_buffer->device->info->gt == 4)
         pc.CommandStreamerStallEnable = true;
   }
}

static void
emit_query_mi_availability(struct mi_builder *b,
                           struct anv_address addr,
                           bool available)
{
   mi_store(b, mi_mem64(addr), mi_imm(available));
}

static void
emit_query_pc_availability(struct anv_cmd_buffer *cmd_buffer,
                           struct anv_address addr,
                           bool available)
{
   cmd_buffer->state.pending_pipe_bits |= ANV_PIPE_POST_SYNC_BIT;
   genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
      pc.DestinationAddressType  = DAT_PPGTT;
      pc.PostSyncOperation       = WriteImmediateData;
      pc.Address                 = addr;
      pc.ImmediateData           = available;
   }
}

/**
 * Goes through a series of consecutive query indices in the given pool
 * setting all element values to 0 and emitting them as available.
 */
static void
emit_zero_queries(struct anv_cmd_buffer *cmd_buffer,
                  struct mi_builder *b, struct anv_query_pool *pool,
                  uint32_t first_index, uint32_t num_queries)
{
   switch (pool->type) {
   case VK_QUERY_TYPE_OCCLUSION:
   case VK_QUERY_TYPE_TIMESTAMP:
      /* These queries are written with a PIPE_CONTROL so clear them using the
       * PIPE_CONTROL as well so we don't have to synchronize between 2 types
       * of operations.
       */
      assert((pool->stride % 8) == 0);
      for (uint32_t i = 0; i < num_queries; i++) {
         struct anv_address slot_addr =
            anv_query_address(pool, first_index + i);

         for (uint32_t qword = 1; qword < (pool->stride / 8); qword++) {
            emit_query_pc_availability(cmd_buffer,
                                       anv_address_add(slot_addr, qword * 8),
                                       false);
         }
         emit_query_pc_availability(cmd_buffer, slot_addr, true);
      }
      break;

   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
   case VK_QUERY_TYPE_PIPELINE_STATISTICS:
   case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
      for (uint32_t i = 0; i < num_queries; i++) {
         struct anv_address slot_addr =
            anv_query_address(pool, first_index + i);
         mi_memset(b, anv_address_add(slot_addr, 8), 0, pool->stride - 8);
         emit_query_mi_availability(b, slot_addr, true);
      }
      break;

#if GFX_VER >= 8
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
      for (uint32_t i = 0; i < num_queries; i++) {
         for (uint32_t p = 0; p < pool->n_passes; p++) {
            mi_memset(b, khr_perf_query_data_address(pool, first_index + i, p, false),
                         0, 2 * pool->snapshot_size);
            emit_query_mi_availability(b,
                                       khr_perf_query_availability_address(pool, first_index + i, p),
                                       true);
         }
      }
      break;
   }
#endif

   case VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL:
      for (uint32_t i = 0; i < num_queries; i++) {
         struct anv_address slot_addr =
            anv_query_address(pool, first_index + i);
         mi_memset(b, anv_address_add(slot_addr, 8), 0, pool->stride - 8);
         emit_query_mi_availability(b, slot_addr, true);
      }
      break;

   default:
      unreachable("Unsupported query type");
   }
}

void genX(CmdResetQueryPool)(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_query_pool, pool, queryPool);

   switch (pool->type) {
   case VK_QUERY_TYPE_OCCLUSION:
      for (uint32_t i = 0; i < queryCount; i++) {
         emit_query_pc_availability(cmd_buffer,
                                    anv_query_address(pool, firstQuery + i),
                                    false);
      }
      break;

   case VK_QUERY_TYPE_TIMESTAMP: {
      for (uint32_t i = 0; i < queryCount; i++) {
         emit_query_pc_availability(cmd_buffer,
                                    anv_query_address(pool, firstQuery + i),
                                    false);
      }

      /* Add a CS stall here to make sure the PIPE_CONTROL above has
       * completed. Otherwise some timestamps written later with MI_STORE_*
       * commands might race with the PIPE_CONTROL in the loop above.
       */
      anv_add_pending_pipe_bits(cmd_buffer, ANV_PIPE_CS_STALL_BIT,
                                "vkCmdResetQueryPool of timestamps");
      genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);
      break;
   }

   case VK_QUERY_TYPE_PIPELINE_STATISTICS:
   case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT: {
      struct mi_builder b;
      mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

      for (uint32_t i = 0; i < queryCount; i++)
         emit_query_mi_availability(&b, anv_query_address(pool, firstQuery + i), false);
      break;
   }

#if GFX_VER >= 8
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
      struct mi_builder b;
      mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

      for (uint32_t i = 0; i < queryCount; i++) {
         for (uint32_t p = 0; p < pool->n_passes; p++) {
            emit_query_mi_availability(
               &b,
               khr_perf_query_availability_address(pool, firstQuery + i, p),
               false);
         }
      }
      break;
   }
#endif

   case VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL: {
      struct mi_builder b;
      mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

      for (uint32_t i = 0; i < queryCount; i++)
         emit_query_mi_availability(&b, anv_query_address(pool, firstQuery + i), false);
      break;
   }

   default:
      unreachable("Unsupported query type");
   }
}

void genX(ResetQueryPool)(
    VkDevice                                    _device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount)
{
   ANV_FROM_HANDLE(anv_query_pool, pool, queryPool);

   for (uint32_t i = 0; i < queryCount; i++) {
      if (pool->type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
#if GFX_VER >= 8
         for (uint32_t p = 0; p < pool->n_passes; p++) {
            uint64_t *pass_slot = pool->bo->map +
               khr_perf_query_availability_offset(pool, firstQuery + i, p);
            *pass_slot = 0;
         }
#endif
      } else {
         uint64_t *slot = query_slot(pool, firstQuery + i);
         *slot = 0;
      }
   }
}

static const uint32_t vk_pipeline_stat_to_reg[] = {
   GENX(IA_VERTICES_COUNT_num),
   GENX(IA_PRIMITIVES_COUNT_num),
   GENX(VS_INVOCATION_COUNT_num),
   GENX(GS_INVOCATION_COUNT_num),
   GENX(GS_PRIMITIVES_COUNT_num),
   GENX(CL_INVOCATION_COUNT_num),
   GENX(CL_PRIMITIVES_COUNT_num),
   GENX(PS_INVOCATION_COUNT_num),
   GENX(HS_INVOCATION_COUNT_num),
   GENX(DS_INVOCATION_COUNT_num),
   GENX(CS_INVOCATION_COUNT_num),
};

static void
emit_pipeline_stat(struct mi_builder *b, uint32_t stat,
                   struct anv_address addr)
{
   STATIC_ASSERT(ANV_PIPELINE_STATISTICS_MASK ==
                 (1 << ARRAY_SIZE(vk_pipeline_stat_to_reg)) - 1);

   assert(stat < ARRAY_SIZE(vk_pipeline_stat_to_reg));
   mi_store(b, mi_mem64(addr), mi_reg64(vk_pipeline_stat_to_reg[stat]));
}

static void
emit_xfb_query(struct mi_builder *b, uint32_t stream,
               struct anv_address addr)
{
   assert(stream < MAX_XFB_STREAMS);

   mi_store(b, mi_mem64(anv_address_add(addr, 0)),
               mi_reg64(GENX(SO_NUM_PRIMS_WRITTEN0_num) + stream * 8));
   mi_store(b, mi_mem64(anv_address_add(addr, 16)),
               mi_reg64(GENX(SO_PRIM_STORAGE_NEEDED0_num) + stream * 8));
}

static void
emit_perf_intel_query(struct anv_cmd_buffer *cmd_buffer,
                      struct anv_query_pool *pool,
                      struct mi_builder *b,
                      struct anv_address query_addr,
                      bool end)
{
   const struct intel_perf_query_field_layout *layout =
      &cmd_buffer->device->physical->perf->query_layout;
   struct anv_address data_addr =
      anv_address_add(query_addr, intel_perf_query_data_offset(pool, end));

   for (uint32_t f = 0; f < layout->n_fields; f++) {
      const struct intel_perf_query_field *field =
         &layout->fields[end ? f : (layout->n_fields - 1 - f)];

      switch (field->type) {
      case INTEL_PERF_QUERY_FIELD_TYPE_MI_RPC:
         anv_batch_emit(&cmd_buffer->batch, GENX(MI_REPORT_PERF_COUNT), rpc) {
            rpc.MemoryAddress = anv_address_add(data_addr, field->location);
         }
         break;

      case INTEL_PERF_QUERY_FIELD_TYPE_SRM_PERFCNT:
      case INTEL_PERF_QUERY_FIELD_TYPE_SRM_RPSTAT:
      case INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_A:
      case INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_B:
      case INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_C: {
         struct anv_address addr = anv_address_add(data_addr, field->location);
         struct mi_value src = field->size == 8 ?
            mi_reg64(field->mmio_offset) :
            mi_reg32(field->mmio_offset);
         struct mi_value dst = field->size == 8 ?
            mi_mem64(addr) : mi_mem32(addr);
         mi_store(b, dst, src);
         break;
      }

      default:
         unreachable("Invalid query field");
         break;
      }
   }
}

void genX(CmdBeginQuery)(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags)
{
   genX(CmdBeginQueryIndexedEXT)(commandBuffer, queryPool, query, flags, 0);
}

void genX(CmdBeginQueryIndexedEXT)(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_query_pool, pool, queryPool);
   struct anv_address query_addr = anv_query_address(pool, query);

   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

   switch (pool->type) {
   case VK_QUERY_TYPE_OCCLUSION:
      emit_ps_depth_count(cmd_buffer, anv_address_add(query_addr, 8));
      break;

   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.CommandStreamerStallEnable = true;
         pc.StallAtPixelScoreboard = true;
      }
      mi_store(&b, mi_mem64(anv_address_add(query_addr, 8)),
                   mi_reg64(GENX(CL_INVOCATION_COUNT_num)));
      break;

   case VK_QUERY_TYPE_PIPELINE_STATISTICS: {
      /* TODO: This might only be necessary for certain stats */
      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.CommandStreamerStallEnable = true;
         pc.StallAtPixelScoreboard = true;
      }

      uint32_t statistics = pool->pipeline_statistics;
      uint32_t offset = 8;
      while (statistics) {
         uint32_t stat = u_bit_scan(&statistics);
         emit_pipeline_stat(&b, stat, anv_address_add(query_addr, offset));
         offset += 16;
      }
      break;
   }

   case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.CommandStreamerStallEnable = true;
         pc.StallAtPixelScoreboard = true;
      }
      emit_xfb_query(&b, index, anv_address_add(query_addr, 8));
      break;

#if GFX_VER >= 8
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
      if (!khr_perf_query_ensure_relocs(cmd_buffer))
         return;

      const struct anv_physical_device *pdevice = cmd_buffer->device->physical;
      const struct intel_perf_query_field_layout *layout = &pdevice->perf->query_layout;

      uint32_t reloc_idx = 0;
      for (uint32_t end = 0; end < 2; end++) {
         for (uint32_t r = 0; r < layout->n_fields; r++) {
            const struct intel_perf_query_field *field =
               &layout->fields[end ? r : (layout->n_fields - 1 - r)];
            struct mi_value reg_addr =
               mi_iadd(
                  &b,
                  mi_imm(intel_canonical_address(pool->bo->offset +
                                                 khr_perf_query_data_offset(pool, query, 0, end) +
                                                 field->location)),
                  mi_reg64(ANV_PERF_QUERY_OFFSET_REG));
            cmd_buffer->self_mod_locations[reloc_idx++] = mi_store_address(&b, reg_addr);

            if (field->type != INTEL_PERF_QUERY_FIELD_TYPE_MI_RPC &&
                field->size == 8) {
               reg_addr =
                  mi_iadd(
                     &b,
                     mi_imm(intel_canonical_address(pool->bo->offset +
                                                    khr_perf_query_data_offset(pool, query, 0, end) +
                                                    field->location + 4)),
                     mi_reg64(ANV_PERF_QUERY_OFFSET_REG));
               cmd_buffer->self_mod_locations[reloc_idx++] = mi_store_address(&b, reg_addr);
            }
         }
      }

      struct mi_value availability_write_offset =
         mi_iadd(
            &b,
            mi_imm(
               intel_canonical_address(
                  pool->bo->offset +
                  khr_perf_query_availability_offset(pool, query, 0 /* pass */))),
            mi_reg64(ANV_PERF_QUERY_OFFSET_REG));
      cmd_buffer->self_mod_locations[reloc_idx++] =
         mi_store_address(&b, availability_write_offset);

      assert(reloc_idx == pdevice->n_perf_query_commands);

      const struct intel_device_info *devinfo = cmd_buffer->device->info;
      const enum intel_engine_class engine_class = cmd_buffer->queue_family->engine_class;
      mi_self_mod_barrier(&b, devinfo->engine_class_prefetch[engine_class]);

      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.CommandStreamerStallEnable = true;
         pc.StallAtPixelScoreboard = true;
      }
      cmd_buffer->perf_query_pool = pool;

      cmd_buffer->perf_reloc_idx = 0;
      for (uint32_t r = 0; r < layout->n_fields; r++) {
         const struct intel_perf_query_field *field =
            &layout->fields[layout->n_fields - 1 - r];
         void *dws;

         switch (field->type) {
         case INTEL_PERF_QUERY_FIELD_TYPE_MI_RPC:
            dws = anv_batch_emitn(&cmd_buffer->batch,
                                  GENX(MI_REPORT_PERF_COUNT_length),
                                  GENX(MI_REPORT_PERF_COUNT),
                                  .MemoryAddress = query_addr /* Will be overwritten */);
            _mi_resolve_address_token(&b,
                                      cmd_buffer->self_mod_locations[cmd_buffer->perf_reloc_idx++],
                                      dws +
                                      GENX(MI_REPORT_PERF_COUNT_MemoryAddress_start) / 8);
            break;

         case INTEL_PERF_QUERY_FIELD_TYPE_SRM_PERFCNT:
         case INTEL_PERF_QUERY_FIELD_TYPE_SRM_RPSTAT:
         case INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_A:
         case INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_B:
         case INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_C:
            dws =
               anv_batch_emitn(&cmd_buffer->batch,
                               GENX(MI_STORE_REGISTER_MEM_length),
                               GENX(MI_STORE_REGISTER_MEM),
                               .RegisterAddress = field->mmio_offset,
                               .MemoryAddress = query_addr /* Will be overwritten */ );
            _mi_resolve_address_token(&b,
                                      cmd_buffer->self_mod_locations[cmd_buffer->perf_reloc_idx++],
                                      dws +
                                      GENX(MI_STORE_REGISTER_MEM_MemoryAddress_start) / 8);
            if (field->size == 8) {
               dws =
                  anv_batch_emitn(&cmd_buffer->batch,
                                  GENX(MI_STORE_REGISTER_MEM_length),
                                  GENX(MI_STORE_REGISTER_MEM),
                                  .RegisterAddress = field->mmio_offset + 4,
                                  .MemoryAddress = query_addr /* Will be overwritten */ );
               _mi_resolve_address_token(&b,
                                         cmd_buffer->self_mod_locations[cmd_buffer->perf_reloc_idx++],
                                         dws +
                                         GENX(MI_STORE_REGISTER_MEM_MemoryAddress_start) / 8);
            }
            break;

         default:
            unreachable("Invalid query field");
            break;
         }
      }
      break;
   }
#endif

   case VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL: {
      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.CommandStreamerStallEnable = true;
         pc.StallAtPixelScoreboard = true;
      }
      emit_perf_intel_query(cmd_buffer, pool, &b, query_addr, false);
      break;
   }

   default:
      unreachable("");
   }
}

void genX(CmdEndQuery)(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query)
{
   genX(CmdEndQueryIndexedEXT)(commandBuffer, queryPool, query, 0);
}

void genX(CmdEndQueryIndexedEXT)(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_query_pool, pool, queryPool);
   struct anv_address query_addr = anv_query_address(pool, query);

   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

   switch (pool->type) {
   case VK_QUERY_TYPE_OCCLUSION:
      emit_ps_depth_count(cmd_buffer, anv_address_add(query_addr, 16));
      emit_query_pc_availability(cmd_buffer, query_addr, true);
      break;

   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
      /* Ensure previous commands have completed before capturing the register
       * value.
       */
      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.CommandStreamerStallEnable = true;
         pc.StallAtPixelScoreboard = true;
      }

      mi_store(&b, mi_mem64(anv_address_add(query_addr, 16)),
                   mi_reg64(GENX(CL_INVOCATION_COUNT_num)));
      emit_query_mi_availability(&b, query_addr, true);
      break;

   case VK_QUERY_TYPE_PIPELINE_STATISTICS: {
      /* TODO: This might only be necessary for certain stats */
      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.CommandStreamerStallEnable = true;
         pc.StallAtPixelScoreboard = true;
      }

      uint32_t statistics = pool->pipeline_statistics;
      uint32_t offset = 16;
      while (statistics) {
         uint32_t stat = u_bit_scan(&statistics);
         emit_pipeline_stat(&b, stat, anv_address_add(query_addr, offset));
         offset += 16;
      }

      emit_query_mi_availability(&b, query_addr, true);
      break;
   }

   case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.CommandStreamerStallEnable = true;
         pc.StallAtPixelScoreboard = true;
      }

      emit_xfb_query(&b, index, anv_address_add(query_addr, 16));
      emit_query_mi_availability(&b, query_addr, true);
      break;

#if GFX_VER >= 8
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.CommandStreamerStallEnable = true;
         pc.StallAtPixelScoreboard = true;
      }
      cmd_buffer->perf_query_pool = pool;

      if (!khr_perf_query_ensure_relocs(cmd_buffer))
         return;

      const struct anv_physical_device *pdevice = cmd_buffer->device->physical;
      const struct intel_perf_query_field_layout *layout = &pdevice->perf->query_layout;

      void *dws;
      for (uint32_t r = 0; r < layout->n_fields; r++) {
         const struct intel_perf_query_field *field = &layout->fields[r];

         switch (field->type) {
         case INTEL_PERF_QUERY_FIELD_TYPE_MI_RPC:
            dws = anv_batch_emitn(&cmd_buffer->batch,
                                  GENX(MI_REPORT_PERF_COUNT_length),
                                  GENX(MI_REPORT_PERF_COUNT),
                                  .MemoryAddress = query_addr /* Will be overwritten */);
            _mi_resolve_address_token(&b,
                                      cmd_buffer->self_mod_locations[cmd_buffer->perf_reloc_idx++],
                                      dws +
                                      GENX(MI_REPORT_PERF_COUNT_MemoryAddress_start) / 8);
            break;

         case INTEL_PERF_QUERY_FIELD_TYPE_SRM_PERFCNT:
         case INTEL_PERF_QUERY_FIELD_TYPE_SRM_RPSTAT:
         case INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_A:
         case INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_B:
         case INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_C:
            dws =
               anv_batch_emitn(&cmd_buffer->batch,
                               GENX(MI_STORE_REGISTER_MEM_length),
                               GENX(MI_STORE_REGISTER_MEM),
                               .RegisterAddress = field->mmio_offset,
                               .MemoryAddress = query_addr /* Will be overwritten */ );
            _mi_resolve_address_token(&b,
                                      cmd_buffer->self_mod_locations[cmd_buffer->perf_reloc_idx++],
                                      dws +
                                      GENX(MI_STORE_REGISTER_MEM_MemoryAddress_start) / 8);
            if (field->size == 8) {
               dws =
                  anv_batch_emitn(&cmd_buffer->batch,
                                  GENX(MI_STORE_REGISTER_MEM_length),
                                  GENX(MI_STORE_REGISTER_MEM),
                                  .RegisterAddress = field->mmio_offset + 4,
                                  .MemoryAddress = query_addr /* Will be overwritten */ );
               _mi_resolve_address_token(&b,
                                         cmd_buffer->self_mod_locations[cmd_buffer->perf_reloc_idx++],
                                         dws +
                                         GENX(MI_STORE_REGISTER_MEM_MemoryAddress_start) / 8);
            }
            break;

         default:
            unreachable("Invalid query field");
            break;
         }
      }

      dws =
         anv_batch_emitn(&cmd_buffer->batch,
                         GENX(MI_STORE_DATA_IMM_length),
                         GENX(MI_STORE_DATA_IMM),
                         .ImmediateData = true);
      _mi_resolve_address_token(&b,
                                cmd_buffer->self_mod_locations[cmd_buffer->perf_reloc_idx++],
                                dws +
                                GENX(MI_STORE_DATA_IMM_Address_start) / 8);

      assert(cmd_buffer->perf_reloc_idx == pdevice->n_perf_query_commands);
      break;
   }
#endif

   case VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL: {
      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.CommandStreamerStallEnable = true;
         pc.StallAtPixelScoreboard = true;
      }
      uint32_t marker_offset = intel_perf_marker_offset();
      mi_store(&b, mi_mem64(anv_address_add(query_addr, marker_offset)),
                   mi_imm(cmd_buffer->intel_perf_marker));
      emit_perf_intel_query(cmd_buffer, pool, &b, query_addr, true);
      emit_query_mi_availability(&b, query_addr, true);
      break;
   }

   default:
      unreachable("");
   }

   /* When multiview is active the spec requires that N consecutive query
    * indices are used, where N is the number of active views in the subpass.
    * The spec allows that we only write the results to one of the queries
    * but we still need to manage result availability for all the query indices.
    * Since we only emit a single query for all active views in the
    * first index, mark the other query indices as being already available
    * with result 0.
    */
   if (cmd_buffer->state.gfx.view_mask) {
      const uint32_t num_queries =
         util_bitcount(cmd_buffer->state.gfx.view_mask);
      if (num_queries > 1)
         emit_zero_queries(cmd_buffer, &b, pool, query + 1, num_queries - 1);
   }
}

#define TIMESTAMP 0x2358

void genX(CmdWriteTimestamp2)(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_query_pool, pool, queryPool);
   struct anv_address query_addr = anv_query_address(pool, query);

   assert(pool->type == VK_QUERY_TYPE_TIMESTAMP);

   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);

   if (stage == VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT) {
      mi_store(&b, mi_mem64(anv_address_add(query_addr, 8)),
                   mi_reg64(TIMESTAMP));
      emit_query_mi_availability(&b, query_addr, true);
   } else {
      /* Everything else is bottom-of-pipe */
      cmd_buffer->state.pending_pipe_bits |= ANV_PIPE_POST_SYNC_BIT;
      genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);

      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
         pc.DestinationAddressType  = DAT_PPGTT;
         pc.PostSyncOperation       = WriteTimestamp;
         pc.Address                 = anv_address_add(query_addr, 8);

         if (GFX_VER == 9 && cmd_buffer->device->info->gt == 4)
            pc.CommandStreamerStallEnable = true;
      }
      emit_query_pc_availability(cmd_buffer, query_addr, true);
   }


   /* When multiview is active the spec requires that N consecutive query
    * indices are used, where N is the number of active views in the subpass.
    * The spec allows that we only write the results to one of the queries
    * but we still need to manage result availability for all the query indices.
    * Since we only emit a single query for all active views in the
    * first index, mark the other query indices as being already available
    * with result 0.
    */
   if (cmd_buffer->state.gfx.view_mask) {
      const uint32_t num_queries =
         util_bitcount(cmd_buffer->state.gfx.view_mask);
      if (num_queries > 1)
         emit_zero_queries(cmd_buffer, &b, pool, query + 1, num_queries - 1);
   }
}

#if GFX_VERx10 >= 75

#define MI_PREDICATE_SRC0    0x2400
#define MI_PREDICATE_SRC1    0x2408
#define MI_PREDICATE_RESULT  0x2418

/**
 * Writes the results of a query to dst_addr is the value at poll_addr is equal
 * to the reference value.
 */
static void
gpu_write_query_result_cond(struct anv_cmd_buffer *cmd_buffer,
                            struct mi_builder *b,
                            struct anv_address poll_addr,
                            struct anv_address dst_addr,
                            uint64_t ref_value,
                            VkQueryResultFlags flags,
                            uint32_t value_index,
                            struct mi_value query_result)
{
   mi_store(b, mi_reg64(MI_PREDICATE_SRC0), mi_mem64(poll_addr));
   mi_store(b, mi_reg64(MI_PREDICATE_SRC1), mi_imm(ref_value));
   anv_batch_emit(&cmd_buffer->batch, GENX(MI_PREDICATE), mip) {
      mip.LoadOperation    = LOAD_LOAD;
      mip.CombineOperation = COMBINE_SET;
      mip.CompareOperation = COMPARE_SRCS_EQUAL;
   }

   if (flags & VK_QUERY_RESULT_64_BIT) {
      struct anv_address res_addr = anv_address_add(dst_addr, value_index * 8);
      mi_store_if(b, mi_mem64(res_addr), query_result);
   } else {
      struct anv_address res_addr = anv_address_add(dst_addr, value_index * 4);
      mi_store_if(b, mi_mem32(res_addr), query_result);
   }
}

static void
gpu_write_query_result(struct mi_builder *b,
                       struct anv_address dst_addr,
                       VkQueryResultFlags flags,
                       uint32_t value_index,
                       struct mi_value query_result)
{
   if (flags & VK_QUERY_RESULT_64_BIT) {
      struct anv_address res_addr = anv_address_add(dst_addr, value_index * 8);
      mi_store(b, mi_mem64(res_addr), query_result);
   } else {
      struct anv_address res_addr = anv_address_add(dst_addr, value_index * 4);
      mi_store(b, mi_mem32(res_addr), query_result);
   }
}

static struct mi_value
compute_query_result(struct mi_builder *b, struct anv_address addr)
{
   return mi_isub(b, mi_mem64(anv_address_add(addr, 8)),
                     mi_mem64(anv_address_add(addr, 0)));
}

void genX(CmdCopyQueryPoolResults)(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    destBuffer,
    VkDeviceSize                                destOffset,
    VkDeviceSize                                destStride,
    VkQueryResultFlags                          flags)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_query_pool, pool, queryPool);
   ANV_FROM_HANDLE(anv_buffer, buffer, destBuffer);

   struct mi_builder b;
   mi_builder_init(&b, cmd_buffer->device->info, &cmd_buffer->batch);
   struct mi_value result;

   /* If render target writes are ongoing, request a render target cache flush
    * to ensure proper ordering of the commands from the 3d pipe and the
    * command streamer.
    */
   if (cmd_buffer->state.pending_pipe_bits & ANV_PIPE_RENDER_TARGET_BUFFER_WRITES) {
      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_TILE_CACHE_FLUSH_BIT |
                                ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT,
                                "CopyQueryPoolResults");
   }

   if ((flags & VK_QUERY_RESULT_WAIT_BIT) ||
       (cmd_buffer->state.pending_pipe_bits & ANV_PIPE_FLUSH_BITS) ||
       /* Occlusion & timestamp queries are written using a PIPE_CONTROL and
        * because we're about to copy values from MI commands, we need to
        * stall the command streamer to make sure the PIPE_CONTROL values have
        * landed, otherwise we could see inconsistent values & availability.
        *
        *  From the vulkan spec:
        *
        *     "vkCmdCopyQueryPoolResults is guaranteed to see the effect of
        *     previous uses of vkCmdResetQueryPool in the same queue, without
        *     any additional synchronization."
        */
       pool->type == VK_QUERY_TYPE_OCCLUSION ||
       pool->type == VK_QUERY_TYPE_TIMESTAMP) {
      anv_add_pending_pipe_bits(cmd_buffer,
                                ANV_PIPE_CS_STALL_BIT,
                                "CopyQueryPoolResults");
      genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);
   }

   struct anv_address dest_addr = anv_address_add(buffer->address, destOffset);
   for (uint32_t i = 0; i < queryCount; i++) {
      struct anv_address query_addr = anv_query_address(pool, firstQuery + i);
      uint32_t idx = 0;
      switch (pool->type) {
      case VK_QUERY_TYPE_OCCLUSION:
      case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
         result = compute_query_result(&b, anv_address_add(query_addr, 8));
         /* Like in the case of vkGetQueryPoolResults, if the query is
          * unavailable and the VK_QUERY_RESULT_PARTIAL_BIT flag is set,
          * conservatively write 0 as the query result. If the
          * VK_QUERY_RESULT_PARTIAL_BIT isn't set, don't write any value.
          */
         gpu_write_query_result_cond(cmd_buffer, &b, query_addr, dest_addr,
               1 /* available */, flags, idx, result);
         if (flags & VK_QUERY_RESULT_PARTIAL_BIT) {
            gpu_write_query_result_cond(cmd_buffer, &b, query_addr, dest_addr,
                  0 /* unavailable */, flags, idx, mi_imm(0));
         }
         idx++;
         break;

      case VK_QUERY_TYPE_PIPELINE_STATISTICS: {
         uint32_t statistics = pool->pipeline_statistics;
         while (statistics) {
            uint32_t stat = u_bit_scan(&statistics);

            result = compute_query_result(&b, anv_address_add(query_addr,
                                                              idx * 16 + 8));

            /* WaDividePSInvocationCountBy4:HSW,BDW */
            if ((cmd_buffer->device->info->ver == 8 ||
                 cmd_buffer->device->info->verx10 == 75) &&
                (1 << stat) == VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT) {
               result = mi_ushr32_imm(&b, result, 2);
            }

            gpu_write_query_result(&b, dest_addr, flags, idx++, result);
         }
         assert(idx == util_bitcount(pool->pipeline_statistics));
         break;
      }

      case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
         result = compute_query_result(&b, anv_address_add(query_addr, 8));
         gpu_write_query_result(&b, dest_addr, flags, idx++, result);
         result = compute_query_result(&b, anv_address_add(query_addr, 24));
         gpu_write_query_result(&b, dest_addr, flags, idx++, result);
         break;

      case VK_QUERY_TYPE_TIMESTAMP:
         result = mi_mem64(anv_address_add(query_addr, 8));
         gpu_write_query_result(&b, dest_addr, flags, idx++, result);
         break;

#if GFX_VER >= 8
      case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR:
         unreachable("Copy KHR performance query results not implemented");
         break;
#endif

      default:
         unreachable("unhandled query type");
      }

      if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) {
         gpu_write_query_result(&b, dest_addr, flags, idx,
                                mi_mem64(query_addr));
      }

      dest_addr = anv_address_add(dest_addr, destStride);
   }
}

#else
void genX(CmdCopyQueryPoolResults)(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    destBuffer,
    VkDeviceSize                                destOffset,
    VkDeviceSize                                destStride,
    VkQueryResultFlags                          flags)
{
   anv_finishme("Queries not yet supported on Ivy Bridge");
}
#endif
