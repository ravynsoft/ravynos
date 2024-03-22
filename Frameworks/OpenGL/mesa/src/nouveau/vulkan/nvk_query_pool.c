/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_query_pool.h"

#include "nvk_buffer.h"
#include "nvk_cmd_buffer.h"
#include "nvk_device.h"
#include "nvk_entrypoints.h"
#include "nvk_event.h"
#include "nvk_mme.h"
#include "nvk_physical_device.h"
#include "nvk_pipeline.h"

#include "vk_meta.h"
#include "vk_pipeline.h"

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"

#include "nouveau_bo.h"
#include "nouveau_context.h"

#include "util/os_time.h"

#include "nvk_cl906f.h"
#include "nvk_cl9097.h"
#include "nvk_cla0c0.h"
#include "nvk_clc597.h"

struct nvk_query_report {
   uint64_t value;
   uint64_t timestamp;
};

VKAPI_ATTR VkResult VKAPI_CALL
nvk_CreateQueryPool(VkDevice device,
                    const VkQueryPoolCreateInfo *pCreateInfo,
                    const VkAllocationCallbacks *pAllocator,
                    VkQueryPool *pQueryPool)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   struct nvk_query_pool *pool;

   pool = vk_query_pool_create(&dev->vk, pCreateInfo,
                               pAllocator, sizeof(*pool));
   if (!pool)
      return vk_error(dev, VK_ERROR_OUT_OF_HOST_MEMORY);

   /* We place the availability first and then data */
   pool->query_start = align(pool->vk.query_count * sizeof(uint32_t),
                             sizeof(struct nvk_query_report));

   uint32_t reports_per_query;
   switch (pCreateInfo->queryType) {
   case VK_QUERY_TYPE_OCCLUSION:
   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
      reports_per_query = 2;
      break;
   case VK_QUERY_TYPE_TIMESTAMP:
      reports_per_query = 1;
      break;
   case VK_QUERY_TYPE_PIPELINE_STATISTICS:
      reports_per_query = 2 * util_bitcount(pool->vk.pipeline_statistics);
      break;
   case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
      // 2 for primitives succeeded 2 for primitives needed
      reports_per_query = 4;
      break;
   default:
      unreachable("Unsupported query type");
   }
   pool->query_stride = reports_per_query * sizeof(struct nvk_query_report);

   if (pool->vk.query_count > 0) {
      uint32_t bo_size = pool->query_start +
                         pool->query_stride * pool->vk.query_count;
      pool->bo = nouveau_ws_bo_new_mapped(dev->ws_dev, bo_size, 0,
                                          NOUVEAU_WS_BO_GART |
                                          NOUVEAU_WS_BO_NO_SHARE,
                                          NOUVEAU_WS_BO_RDWR,
                                          &pool->bo_map);
      if (!pool->bo) {
         vk_query_pool_destroy(&dev->vk, pAllocator, &pool->vk);
         return vk_error(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY);
      }

      if (dev->ws_dev->debug_flags & NVK_DEBUG_ZERO_MEMORY)
         memset(pool->bo_map, 0, bo_size);
   }

   *pQueryPool = nvk_query_pool_to_handle(pool);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
nvk_DestroyQueryPool(VkDevice device,
                     VkQueryPool queryPool,
                     const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   VK_FROM_HANDLE(nvk_query_pool, pool, queryPool);

   if (!pool)
      return;

   if (pool->bo) {
      nouveau_ws_bo_unmap(pool->bo, pool->bo_map);
      nouveau_ws_bo_destroy(pool->bo);
   }
   vk_query_pool_destroy(&dev->vk, pAllocator, &pool->vk);
}

static uint64_t
nvk_query_available_addr(struct nvk_query_pool *pool, uint32_t query)
{
   assert(query < pool->vk.query_count);
   return pool->bo->offset + query * sizeof(uint32_t);
}

static nir_def *
nvk_nir_available_addr(nir_builder *b, nir_def *pool_addr,
                       nir_def *query)
{
   nir_def *offset = nir_imul_imm(b, query, sizeof(uint32_t));
   return nir_iadd(b, pool_addr, nir_u2u64(b, offset));
}

static uint32_t *
nvk_query_available_map(struct nvk_query_pool *pool, uint32_t query)
{
   assert(query < pool->vk.query_count);
   return (uint32_t *)pool->bo_map + query;
}

static uint64_t
nvk_query_offset(struct nvk_query_pool *pool, uint32_t query)
{
   assert(query < pool->vk.query_count);
   return pool->query_start + query * pool->query_stride;
}

static uint64_t
nvk_query_report_addr(struct nvk_query_pool *pool, uint32_t query)
{
   return pool->bo->offset + nvk_query_offset(pool, query);
}

static nir_def *
nvk_nir_query_report_addr(nir_builder *b, nir_def *pool_addr,
                          nir_def *query_start, nir_def *query_stride,
                          nir_def *query)
{
   nir_def *offset =
      nir_iadd(b, query_start, nir_umul_2x32_64(b, query, query_stride));
   return nir_iadd(b, pool_addr, offset);
}

static struct nvk_query_report *
nvk_query_report_map(struct nvk_query_pool *pool, uint32_t query)
{
   return (void *)((char *)pool->bo_map + nvk_query_offset(pool, query));
}

/**
 * Goes through a series of consecutive query indices in the given pool,
 * setting all element values to 0 and emitting them as available.
 */
static void
emit_zero_queries(struct nvk_cmd_buffer *cmd, struct nvk_query_pool *pool,
                  uint32_t first_index, uint32_t num_queries)
{
   switch (pool->vk.query_type) {
   case VK_QUERY_TYPE_OCCLUSION:
   case VK_QUERY_TYPE_TIMESTAMP:
   case VK_QUERY_TYPE_PIPELINE_STATISTICS:
   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT: {
      for (uint32_t i = 0; i < num_queries; i++) {
         uint64_t addr = nvk_query_available_addr(pool, first_index + i);

         struct nv_push *p = nvk_cmd_buffer_push(cmd, 5);
         P_MTHD(p, NV9097, SET_REPORT_SEMAPHORE_A);
         P_NV9097_SET_REPORT_SEMAPHORE_A(p, addr >> 32);
         P_NV9097_SET_REPORT_SEMAPHORE_B(p, addr);
         P_NV9097_SET_REPORT_SEMAPHORE_C(p, 1);
         P_NV9097_SET_REPORT_SEMAPHORE_D(p, {
            .operation = OPERATION_RELEASE,
            .release = RELEASE_AFTER_ALL_PRECEEDING_WRITES_COMPLETE,
            .pipeline_location = PIPELINE_LOCATION_ALL,
            .structure_size = STRUCTURE_SIZE_ONE_WORD,
         });
      }
      break;
   }
   default:
      unreachable("Unsupported query type");
   }
}

VKAPI_ATTR void VKAPI_CALL
nvk_ResetQueryPool(VkDevice device,
                   VkQueryPool queryPool,
                   uint32_t firstQuery,
                   uint32_t queryCount)
{
   VK_FROM_HANDLE(nvk_query_pool, pool, queryPool);

   uint32_t *available = nvk_query_available_map(pool, firstQuery);
   memset(available, 0, queryCount * sizeof(*available));
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdResetQueryPool(VkCommandBuffer commandBuffer,
                      VkQueryPool queryPool,
                      uint32_t firstQuery,
                      uint32_t queryCount)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_query_pool, pool, queryPool);

   for (uint32_t i = 0; i < queryCount; i++) {
      uint64_t addr = nvk_query_available_addr(pool, firstQuery + i);

      struct nv_push *p = nvk_cmd_buffer_push(cmd, 5);
      P_MTHD(p, NV9097, SET_REPORT_SEMAPHORE_A);
      P_NV9097_SET_REPORT_SEMAPHORE_A(p, addr >> 32);
      P_NV9097_SET_REPORT_SEMAPHORE_B(p, addr);
      P_NV9097_SET_REPORT_SEMAPHORE_C(p, 0);
      P_NV9097_SET_REPORT_SEMAPHORE_D(p, {
         .operation = OPERATION_RELEASE,
         .release = RELEASE_AFTER_ALL_PRECEEDING_WRITES_COMPLETE,
         .pipeline_location = PIPELINE_LOCATION_ALL,
         .structure_size = STRUCTURE_SIZE_ONE_WORD,
      });
   }

   /* Wait for the above writes to complete.  This prevents WaW hazards on any
    * later query availability updates and ensures vkCmdCopyQueryPoolResults
    * will see the query as unavailable if it happens before the query is
    * completed again.
    */
   for (uint32_t i = 0; i < queryCount; i++) {
      uint64_t addr = nvk_query_available_addr(pool, firstQuery + i);

      struct nv_push *p = nvk_cmd_buffer_push(cmd, 5);
      __push_mthd(p, SUBC_NV9097, NV906F_SEMAPHOREA);
      P_NV906F_SEMAPHOREA(p, addr >> 32);
      P_NV906F_SEMAPHOREB(p, (addr & UINT32_MAX) >> 2);
      P_NV906F_SEMAPHOREC(p, 0);
      P_NV906F_SEMAPHORED(p, {
         .operation = OPERATION_ACQUIRE,
         .acquire_switch = ACQUIRE_SWITCH_ENABLED,
         .release_size = RELEASE_SIZE_4BYTE,
      });
   }
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdWriteTimestamp2(VkCommandBuffer commandBuffer,
                       VkPipelineStageFlags2 stage,
                       VkQueryPool queryPool,
                       uint32_t query)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_query_pool, pool, queryPool);

   struct nv_push *p = nvk_cmd_buffer_push(cmd, 10);

   uint64_t report_addr = nvk_query_report_addr(pool, query);
   P_MTHD(p, NV9097, SET_REPORT_SEMAPHORE_A);
   P_NV9097_SET_REPORT_SEMAPHORE_A(p, report_addr >> 32);
   P_NV9097_SET_REPORT_SEMAPHORE_B(p, report_addr);
   P_NV9097_SET_REPORT_SEMAPHORE_C(p, 0);
   P_NV9097_SET_REPORT_SEMAPHORE_D(p, {
      .operation = OPERATION_REPORT_ONLY,
      .pipeline_location = vk_stage_flags_to_nv9097_pipeline_location(stage),
      .structure_size = STRUCTURE_SIZE_FOUR_WORDS,
   });

   uint64_t available_addr = nvk_query_available_addr(pool, query);
   P_MTHD(p, NV9097, SET_REPORT_SEMAPHORE_A);
   P_NV9097_SET_REPORT_SEMAPHORE_A(p, available_addr >> 32);
   P_NV9097_SET_REPORT_SEMAPHORE_B(p, available_addr);
   P_NV9097_SET_REPORT_SEMAPHORE_C(p, 1);
   P_NV9097_SET_REPORT_SEMAPHORE_D(p, {
      .operation = OPERATION_RELEASE,
      .release = RELEASE_AFTER_ALL_PRECEEDING_WRITES_COMPLETE,
      .pipeline_location = PIPELINE_LOCATION_ALL,
      .structure_size = STRUCTURE_SIZE_ONE_WORD,
   });

   /* From the Vulkan spec:
    *
    *   "If vkCmdWriteTimestamp2 is called while executing a render pass
    *    instance that has multiview enabled, the timestamp uses N consecutive
    *    query indices in the query pool (starting at query) where N is the
    *    number of bits set in the view mask of the subpass the command is
    *    executed in. The resulting query values are determined by an
    *    implementation-dependent choice of one of the following behaviors:"
    *
    * In our case, only the first query is used, so we emit zeros for the
    * remaining queries, as described in the first behavior listed in the
    * Vulkan spec:
    *
    *   "The first query is a timestamp value and (if more than one bit is set
    *   in the view mask) zero is written to the remaining queries."
    */
   if (cmd->state.gfx.render.view_mask != 0) {
      const uint32_t num_queries =
         util_bitcount(cmd->state.gfx.render.view_mask);
      if (num_queries > 1)
         emit_zero_queries(cmd, pool, query + 1, num_queries - 1);
   }
}

struct nvk_3d_stat_query {
   VkQueryPipelineStatisticFlagBits flag;
   uint8_t loc;
   uint8_t report;
};

/* This must remain sorted in flag order */
static const struct nvk_3d_stat_query nvk_3d_stat_queries[] = {{
   .flag    = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT,
   .loc     = NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_DATA_ASSEMBLER,
   .report  = NV9097_SET_REPORT_SEMAPHORE_D_REPORT_DA_VERTICES_GENERATED,
}, {
   .flag    = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT,
   .loc     = NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_DATA_ASSEMBLER,
   .report  = NV9097_SET_REPORT_SEMAPHORE_D_REPORT_DA_PRIMITIVES_GENERATED,
}, {
   .flag    = VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT,
   .loc     = NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_VERTEX_SHADER,
   .report  = NV9097_SET_REPORT_SEMAPHORE_D_REPORT_VS_INVOCATIONS,
}, {
   .flag    = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT,
   .loc     = NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_GEOMETRY_SHADER,
   .report  = NV9097_SET_REPORT_SEMAPHORE_D_REPORT_GS_INVOCATIONS,
}, {
   .flag    = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT,
   .loc     = NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_GEOMETRY_SHADER,
   .report  = NV9097_SET_REPORT_SEMAPHORE_D_REPORT_GS_PRIMITIVES_GENERATED,
}, {
   .flag    = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT,
   .loc     = NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_VPC, /* TODO */
   .report  = NV9097_SET_REPORT_SEMAPHORE_D_REPORT_CLIPPER_INVOCATIONS,
}, {
   .flag    = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT,
   .loc     = NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_VPC, /* TODO */
   .report  = NV9097_SET_REPORT_SEMAPHORE_D_REPORT_CLIPPER_PRIMITIVES_GENERATED,
}, {
   .flag    = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT,
   .loc     = NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_PIXEL_SHADER,
   .report  = NV9097_SET_REPORT_SEMAPHORE_D_REPORT_PS_INVOCATIONS,
}, {
   .flag    = VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT,
   .loc     = NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_TESSELATION_INIT_SHADER,
   .report  = NV9097_SET_REPORT_SEMAPHORE_D_REPORT_TI_INVOCATIONS,
}, {
   .flag    = VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT,
   .loc     = NV9097_SET_REPORT_SEMAPHORE_D_PIPELINE_LOCATION_TESSELATION_SHADER,
   .report  = NV9097_SET_REPORT_SEMAPHORE_D_REPORT_TS_INVOCATIONS,
}, {
   .flag    = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT,
   .loc     = UINT8_MAX,
   .report  = UINT8_MAX,
}};

static void
mme_store_global(struct mme_builder *b,
                 struct mme_value64 addr,
                 struct mme_value v)
{
   mme_mthd(b, NV9097_SET_REPORT_SEMAPHORE_A);
   mme_emit_addr64(b, addr);
   mme_emit(b, v);
   mme_emit(b, mme_imm(0x10000000));
}

void
nvk_mme_write_cs_invocations(struct mme_builder *b)
{
   struct mme_value64 dst_addr = mme_load_addr64(b);

   struct mme_value accum_hi = mme_state(b,
      NVC597_SET_MME_SHADOW_SCRATCH(NVK_MME_SCRATCH_CS_INVOCATIONS_HI));
   struct mme_value accum_lo = mme_state(b,
      NVC597_SET_MME_SHADOW_SCRATCH(NVK_MME_SCRATCH_CS_INVOCATIONS_LO));
   struct mme_value64 accum = mme_value64(accum_lo, accum_hi);

   mme_store_global(b, dst_addr, accum.lo);
   mme_store_global(b, mme_add64(b, dst_addr, mme_imm64(4)), accum.hi);
}

static void
nvk_cmd_begin_end_query(struct nvk_cmd_buffer *cmd,
                        struct nvk_query_pool *pool,
                        uint32_t query, uint32_t index,
                        bool end)
{
   uint64_t report_addr = nvk_query_report_addr(pool, query) +
                          end * sizeof(struct nvk_query_report);

   uint32_t end_size = 7 * end;

   struct nv_push *p;
   switch (pool->vk.query_type) {
   case VK_QUERY_TYPE_OCCLUSION:
      p = nvk_cmd_buffer_push(cmd, 7 + end_size);

      P_IMMD(p, NV9097, SET_ZPASS_PIXEL_COUNT, !end);

      P_MTHD(p, NV9097, SET_REPORT_SEMAPHORE_A);
      P_NV9097_SET_REPORT_SEMAPHORE_A(p, report_addr >> 32);
      P_NV9097_SET_REPORT_SEMAPHORE_B(p, report_addr);
      P_NV9097_SET_REPORT_SEMAPHORE_C(p, 0);
      P_NV9097_SET_REPORT_SEMAPHORE_D(p, {
         .operation = OPERATION_REPORT_ONLY,
         .pipeline_location = PIPELINE_LOCATION_ALL,
         .report = REPORT_ZPASS_PIXEL_CNT64,
         .structure_size = STRUCTURE_SIZE_FOUR_WORDS,
         .flush_disable = true,
      });
      break;

   case VK_QUERY_TYPE_PIPELINE_STATISTICS: {
      uint32_t stat_count = util_bitcount(pool->vk.pipeline_statistics);
      p = nvk_cmd_buffer_push(cmd, stat_count * 5 + end_size);

      ASSERTED uint32_t stats_left = pool->vk.pipeline_statistics;
      for (uint32_t i = 0; i < ARRAY_SIZE(nvk_3d_stat_queries); i++) {
         const struct nvk_3d_stat_query *sq = &nvk_3d_stat_queries[i];
         if (!(stats_left & sq->flag))
            continue;

         /* The 3D stat queries array MUST be sorted */
         assert(!(stats_left & (sq->flag - 1)));

         if (sq->flag == VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT) {
            P_1INC(p, NVC597, CALL_MME_MACRO(NVK_MME_WRITE_CS_INVOCATIONS));
            P_INLINE_DATA(p, report_addr >> 32);
            P_INLINE_DATA(p, report_addr);
         } else {
            P_MTHD(p, NV9097, SET_REPORT_SEMAPHORE_A);
            P_NV9097_SET_REPORT_SEMAPHORE_A(p, report_addr >> 32);
            P_NV9097_SET_REPORT_SEMAPHORE_B(p, report_addr);
            P_NV9097_SET_REPORT_SEMAPHORE_C(p, 0);
            P_NV9097_SET_REPORT_SEMAPHORE_D(p, {
               .operation = OPERATION_REPORT_ONLY,
               .pipeline_location = sq->loc,
               .report = sq->report,
               .structure_size = STRUCTURE_SIZE_FOUR_WORDS,
               .flush_disable = true,
            });
         }

         report_addr += 2 * sizeof(struct nvk_query_report);
         stats_left &= ~sq->flag;
      }
      break;
   }

   case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT: {
      const uint32_t xfb_reports[] = {
         NV9097_SET_REPORT_SEMAPHORE_D_REPORT_STREAMING_PRIMITIVES_SUCCEEDED,
         NV9097_SET_REPORT_SEMAPHORE_D_REPORT_STREAMING_PRIMITIVES_NEEDED,
      };
      p = nvk_cmd_buffer_push(cmd, 5 * ARRAY_SIZE(xfb_reports) + end_size);
      for (uint32_t i = 0; i < ARRAY_SIZE(xfb_reports); ++i) {
         P_MTHD(p, NV9097, SET_REPORT_SEMAPHORE_A);
         P_NV9097_SET_REPORT_SEMAPHORE_A(p, report_addr >> 32);
         P_NV9097_SET_REPORT_SEMAPHORE_B(p, report_addr);
         P_NV9097_SET_REPORT_SEMAPHORE_C(p, 0);
         P_NV9097_SET_REPORT_SEMAPHORE_D(p, {
               .operation = OPERATION_REPORT_ONLY,
               .pipeline_location = PIPELINE_LOCATION_STREAMING_OUTPUT,
               .report = xfb_reports[i],
               .structure_size = STRUCTURE_SIZE_FOUR_WORDS,
               .sub_report = index,
               .flush_disable = true,
               });
         report_addr += 2 * sizeof(struct nvk_query_report);
      }
      break;
   }

   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
      p = nvk_cmd_buffer_push(cmd, 5 + end_size);

      P_MTHD(p, NV9097, SET_REPORT_SEMAPHORE_A);
      P_NV9097_SET_REPORT_SEMAPHORE_A(p, report_addr >> 32);
      P_NV9097_SET_REPORT_SEMAPHORE_B(p, report_addr);
      P_NV9097_SET_REPORT_SEMAPHORE_C(p, 1);
      P_NV9097_SET_REPORT_SEMAPHORE_D(p, {
         .operation = OPERATION_REPORT_ONLY,
         .pipeline_location = PIPELINE_LOCATION_STREAMING_OUTPUT,
         .report = REPORT_VTG_PRIMITIVES_OUT,
         .sub_report = index,
         .structure_size = STRUCTURE_SIZE_FOUR_WORDS,
         .flush_disable = true,
      });
      break;

   default:
      unreachable("Unsupported query type");
   }

   if (end) {
      P_IMMD(p, NV9097, FLUSH_PENDING_WRITES, 0);

      uint64_t available_addr = nvk_query_available_addr(pool, query);
      P_MTHD(p, NV9097, SET_REPORT_SEMAPHORE_A);
      P_NV9097_SET_REPORT_SEMAPHORE_A(p, available_addr >> 32);
      P_NV9097_SET_REPORT_SEMAPHORE_B(p, available_addr);
      P_NV9097_SET_REPORT_SEMAPHORE_C(p, 1);
      P_NV9097_SET_REPORT_SEMAPHORE_D(p, {
         .operation = OPERATION_RELEASE,
         .release = RELEASE_AFTER_ALL_PRECEEDING_WRITES_COMPLETE,
         .pipeline_location = PIPELINE_LOCATION_ALL,
         .structure_size = STRUCTURE_SIZE_ONE_WORD,
      });
   }
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer,
                            VkQueryPool queryPool,
                            uint32_t query,
                            VkQueryControlFlags flags,
                            uint32_t index)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_query_pool, pool, queryPool);

   nvk_cmd_begin_end_query(cmd, pool, query, index, false);
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer,
                          VkQueryPool queryPool,
                          uint32_t query,
                          uint32_t index)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_query_pool, pool, queryPool);

   nvk_cmd_begin_end_query(cmd, pool, query, index, true);

   /* From the Vulkan spec:
    *
    *   "If queries are used while executing a render pass instance that has
    *    multiview enabled, the query uses N consecutive query indices in
    *    the query pool (starting at query) where N is the number of bits set
    *    in the view mask in the subpass the query is used in. How the
    *    numerical results of the query are distributed among the queries is
    *    implementation-dependent."
    *
    * In our case, only the first query is used, so we emit zeros for the
    * remaining queries.
    */
   if (cmd->state.gfx.render.view_mask != 0) {
      const uint32_t num_queries =
         util_bitcount(cmd->state.gfx.render.view_mask);
      if (num_queries > 1)
         emit_zero_queries(cmd, pool, query + 1, num_queries - 1);
   }
}

static bool
nvk_query_is_available(struct nvk_query_pool *pool, uint32_t query)
{
   uint32_t *available = nvk_query_available_map(pool, query);
   return p_atomic_read(available) != 0;
}

#define NVK_QUERY_TIMEOUT 2000000000ull

static VkResult
nvk_query_wait_for_available(struct nvk_device *dev,
                             struct nvk_query_pool *pool,
                             uint32_t query)
{
   uint64_t abs_timeout_ns = os_time_get_absolute_timeout(NVK_QUERY_TIMEOUT);

   while (os_time_get_nano() < abs_timeout_ns) {
      if (nvk_query_is_available(pool, query))
         return VK_SUCCESS;

      VkResult status = vk_device_check_status(&dev->vk);
      if (status != VK_SUCCESS)
         return status;
   }

   return vk_device_set_lost(&dev->vk, "query timeout");
}

static void
cpu_write_query_result(void *dst, uint32_t idx,
                       VkQueryResultFlags flags,
                       uint64_t result)
{
   if (flags & VK_QUERY_RESULT_64_BIT) {
      uint64_t *dst64 = dst;
      dst64[idx] = result;
   } else {
      uint32_t *dst32 = dst;
      dst32[idx] = result;
   }
}

static void
cpu_get_query_delta(void *dst, const struct nvk_query_report *src,
                    uint32_t idx, VkQueryResultFlags flags)
{
   uint64_t delta = src[idx * 2 + 1].value - src[idx * 2].value;
   cpu_write_query_result(dst, idx, flags, delta);
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_GetQueryPoolResults(VkDevice device,
                        VkQueryPool queryPool,
                        uint32_t firstQuery,
                        uint32_t queryCount,
                        size_t dataSize,
                        void *pData,
                        VkDeviceSize stride,
                        VkQueryResultFlags flags)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   VK_FROM_HANDLE(nvk_query_pool, pool, queryPool);

   if (vk_device_is_lost(&dev->vk))
      return VK_ERROR_DEVICE_LOST;

   VkResult status = VK_SUCCESS;
   for (uint32_t i = 0; i < queryCount; i++) {
      const uint32_t query = firstQuery + i;

      bool available = nvk_query_is_available(pool, query);

      if (!available && (flags & VK_QUERY_RESULT_WAIT_BIT)) {
         status = nvk_query_wait_for_available(dev, pool, query);
         if (status != VK_SUCCESS)
            return status;

         available = true;
      }

      bool write_results = available || (flags & VK_QUERY_RESULT_PARTIAL_BIT);

      const struct nvk_query_report *src = nvk_query_report_map(pool, query);
      assert(i * stride < dataSize);
      void *dst = (char *)pData + i * stride;

      uint32_t available_dst_idx = 1;
      switch (pool->vk.query_type) {
      case VK_QUERY_TYPE_OCCLUSION:
      case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
         if (write_results)
            cpu_get_query_delta(dst, src, 0, flags);
         break;
      case VK_QUERY_TYPE_PIPELINE_STATISTICS: {
         uint32_t stat_count = util_bitcount(pool->vk.pipeline_statistics);
         available_dst_idx = stat_count;
         if (write_results) {
            for (uint32_t j = 0; j < stat_count; j++)
               cpu_get_query_delta(dst, src, j, flags);
         }
         break;
      }
      case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT: {
         const int prims_succeeded_idx = 0;
         const int prims_needed_idx = 1;
         available_dst_idx = 2;
         if (write_results) {
            cpu_get_query_delta(dst, src, prims_succeeded_idx, flags);
            cpu_get_query_delta(dst, src, prims_needed_idx, flags);
         }
         break;
      }
      case VK_QUERY_TYPE_TIMESTAMP:
         if (write_results)
            cpu_write_query_result(dst, 0, flags, src->timestamp);
         break;
      default:
         unreachable("Unsupported query type");
      }

      if (!write_results)
         status = VK_NOT_READY;

      if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)
         cpu_write_query_result(dst, available_dst_idx, flags, available);
   }

   return status;
}

struct nvk_copy_query_push {
   uint64_t pool_addr;
   uint32_t query_start;
   uint32_t query_stride;
   uint32_t first_query;
   uint32_t query_count;
   uint64_t dst_addr;
   uint64_t dst_stride;
   uint32_t flags;
};

static nir_def *
load_struct_var(nir_builder *b, nir_variable *var, uint32_t field)
{
   nir_deref_instr *deref =
      nir_build_deref_struct(b, nir_build_deref_var(b, var), field);
   return nir_load_deref(b, deref);
}

static void
nir_write_query_result(nir_builder *b, nir_def *dst_addr,
                       nir_def *idx, nir_def *flags,
                       nir_def *result)
{
   assert(result->num_components == 1);
   assert(result->bit_size == 64);

   nir_push_if(b, nir_test_mask(b, flags, VK_QUERY_RESULT_64_BIT));
   {
      nir_def *offset = nir_i2i64(b, nir_imul_imm(b, idx, 8));
      nir_store_global(b, nir_iadd(b, dst_addr, offset), 8, result, 0x1);
   }
   nir_push_else(b, NULL);
   {
      nir_def *result32 = nir_u2u32(b, result);
      nir_def *offset = nir_i2i64(b, nir_imul_imm(b, idx, 4));
      nir_store_global(b, nir_iadd(b, dst_addr, offset), 4, result32, 0x1);
   }
   nir_pop_if(b, NULL);
}

static void
nir_get_query_delta(nir_builder *b, nir_def *dst_addr,
                    nir_def *report_addr, nir_def *idx,
                    nir_def *flags)
{
   nir_def *offset =
      nir_imul_imm(b, idx, 2 * sizeof(struct nvk_query_report));
   nir_def *begin_addr =
      nir_iadd(b, report_addr, nir_i2i64(b, offset));
   nir_def *end_addr =
      nir_iadd_imm(b, begin_addr, sizeof(struct nvk_query_report));

   /* nvk_query_report::timestamp is the first uint64_t */
   nir_def *begin = nir_load_global(b, begin_addr, 16, 1, 64);
   nir_def *end = nir_load_global(b, end_addr, 16, 1, 64);

   nir_def *delta = nir_isub(b, end, begin);

   nir_write_query_result(b, dst_addr, idx, flags, delta);
}

static void
nvk_nir_copy_query(nir_builder *b, nir_variable *push, nir_def *i)
{
   nir_def *pool_addr = load_struct_var(b, push, 0);
   nir_def *query_start = nir_u2u64(b, load_struct_var(b, push, 1));
   nir_def *query_stride = load_struct_var(b, push, 2);
   nir_def *first_query = load_struct_var(b, push, 3);
   nir_def *dst_addr = load_struct_var(b, push, 5);
   nir_def *dst_stride = load_struct_var(b, push, 6);
   nir_def *flags = load_struct_var(b, push, 7);

   nir_def *query = nir_iadd(b, first_query, i);

   nir_def *avail_addr = nvk_nir_available_addr(b, pool_addr, query);
   nir_def *available =
      nir_i2b(b, nir_load_global(b, avail_addr, 4, 1, 32));

   nir_def *partial = nir_test_mask(b, flags, VK_QUERY_RESULT_PARTIAL_BIT);
   nir_def *write_results = nir_ior(b, available, partial);

   nir_def *report_addr =
      nvk_nir_query_report_addr(b, pool_addr, query_start, query_stride,
                                query);
   nir_def *dst_offset = nir_imul(b, nir_u2u64(b, i), dst_stride);

   /* Timestamp queries are the only ones use a single report */
   nir_def *is_timestamp =
      nir_ieq_imm(b, query_stride, sizeof(struct nvk_query_report));

   nir_def *one = nir_imm_int(b, 1);
   nir_def *num_reports;
   nir_push_if(b, is_timestamp);
   {
      nir_push_if(b, write_results);
      {
         /* This is the timestamp case.  We add 8 because we're loading
          * nvk_query_report::timestamp.
          */
         nir_def *timestamp =
            nir_load_global(b, nir_iadd_imm(b, report_addr, 8), 8, 1, 64);

         nir_write_query_result(b, nir_iadd(b, dst_addr, dst_offset),
                                nir_imm_int(b, 0), flags, timestamp);
      }
      nir_pop_if(b, NULL);
   }
   nir_push_else(b, NULL);
   {
      /* Everything that isn't a timestamp has the invariant that the
       * number of destination entries is equal to the query stride divided
       * by the size of two reports.
       */
      num_reports = nir_udiv_imm(b, query_stride,
                                 2 * sizeof(struct nvk_query_report));

      nir_push_if(b, write_results);
      {
         nir_variable *r =
            nir_local_variable_create(b->impl, glsl_uint_type(), "r");
         nir_store_var(b, r, nir_imm_int(b, 0), 0x1);

         nir_push_loop(b);
         {
            nir_push_if(b, nir_ige(b, nir_load_var(b, r), num_reports));
            {
               nir_jump(b, nir_jump_break);
            }
            nir_pop_if(b, NULL);

            nir_get_query_delta(b, nir_iadd(b, dst_addr, dst_offset),
                                report_addr, nir_load_var(b, r), flags);

            nir_store_var(b, r, nir_iadd_imm(b, nir_load_var(b, r), 1), 0x1);
         }
         nir_pop_loop(b, NULL);
      }
      nir_pop_if(b, NULL);
   }
   nir_pop_if(b, NULL);

   num_reports = nir_if_phi(b, one, num_reports);

   nir_push_if(b, nir_test_mask(b, flags, VK_QUERY_RESULT_WITH_AVAILABILITY_BIT));
   {
      nir_write_query_result(b, nir_iadd(b, dst_addr, dst_offset),
                             num_reports, flags, nir_b2i64(b, available));
   }
   nir_pop_if(b, NULL);
}

static nir_shader *
build_copy_queries_shader(void)
{
   nir_builder build =
      nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, NULL,
                                     "nvk-meta-copy-queries");
   nir_builder *b = &build;

   struct glsl_struct_field push_fields[] = {
      { .type = glsl_uint64_t_type(), .name = "pool_addr", .offset = 0 },
      { .type = glsl_uint_type(), .name = "query_start", .offset = 8 },
      { .type = glsl_uint_type(), .name = "query_stride", .offset = 12 },
      { .type = glsl_uint_type(), .name = "first_query", .offset = 16 },
      { .type = glsl_uint_type(), .name = "query_count", .offset = 20 },
      { .type = glsl_uint64_t_type(), .name = "dst_addr", .offset = 24 },
      { .type = glsl_uint64_t_type(), .name = "dst_stride", .offset = 32 },
      { .type = glsl_uint_type(), .name = "flags", .offset = 40 },
   };
   const struct glsl_type *push_iface_type =
      glsl_interface_type(push_fields, ARRAY_SIZE(push_fields),
                          GLSL_INTERFACE_PACKING_STD140,
                          false /* row_major */, "push");
   nir_variable *push = nir_variable_create(b->shader, nir_var_mem_push_const,
                                            push_iface_type, "push");

   nir_def *query_count = load_struct_var(b, push, 4);

   nir_variable *i = nir_local_variable_create(b->impl, glsl_uint_type(), "i");
   nir_store_var(b, i, nir_imm_int(b, 0), 0x1);

   nir_push_loop(b);
   {
      nir_push_if(b, nir_ige(b, nir_load_var(b, i), query_count));
      {
         nir_jump(b, nir_jump_break);
      }
      nir_pop_if(b, NULL);

      nvk_nir_copy_query(b, push, nir_load_var(b, i));

      nir_store_var(b, i, nir_iadd_imm(b, nir_load_var(b, i), 1), 0x1);
   }
   nir_pop_loop(b, NULL);

   return build.shader;
}

static VkResult
get_copy_queries_pipeline(struct nvk_device *dev,
                          VkPipelineLayout layout,
                          VkPipeline *pipeline_out)
{
   const char key[] = "nvk-meta-copy-query-pool-results";
   VkPipeline cached = vk_meta_lookup_pipeline(&dev->meta, key, sizeof(key));
   if (cached != VK_NULL_HANDLE) {
      *pipeline_out = cached;
      return VK_SUCCESS;
   }

   const VkPipelineShaderStageNirCreateInfoMESA nir_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NIR_CREATE_INFO_MESA,
      .nir = build_copy_queries_shader(),
   };
   const VkComputePipelineCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .pNext = &nir_info,
         .stage = VK_SHADER_STAGE_COMPUTE_BIT,
         .pName = "main",
      },
      .layout = layout,
   };

   return vk_meta_create_compute_pipeline(&dev->vk, &dev->meta, &info,
                                          key, sizeof(key), pipeline_out);
}

static void
nvk_meta_copy_query_pool_results(struct nvk_cmd_buffer *cmd,
                                 struct nvk_query_pool *pool,
                                 uint32_t first_query,
                                 uint32_t query_count,
                                 uint64_t dst_addr,
                                 uint64_t dst_stride,
                                 VkQueryResultFlags flags)
{
   struct nvk_device *dev = nvk_cmd_buffer_device(cmd);
   struct nvk_descriptor_state *desc = &cmd->state.cs.descriptors;
   VkResult result;

   const struct nvk_copy_query_push push = {
      .pool_addr = pool->bo->offset,
      .query_start = pool->query_start,
      .query_stride = pool->query_stride,
      .first_query = first_query,
      .query_count = query_count,
      .dst_addr = dst_addr,
      .dst_stride = dst_stride,
      .flags = flags,
   };

   const char key[] = "nvk-meta-copy-query-pool-results";
   const VkPushConstantRange push_range = {
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .size = sizeof(push),
   };
   VkPipelineLayout layout;
   result = vk_meta_get_pipeline_layout(&dev->vk, &dev->meta, NULL, &push_range,
                                        key, sizeof(key), &layout);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd->vk, result);
      return;
   }

   VkPipeline pipeline;
   result = get_copy_queries_pipeline(dev, layout, &pipeline);
   if (result != VK_SUCCESS) {
      vk_command_buffer_set_error(&cmd->vk, result);
      return;
   }

   /* Save pipeline and push constants */
   struct nvk_compute_pipeline *pipeline_save = cmd->state.cs.pipeline;
   uint8_t push_save[NVK_MAX_PUSH_SIZE];
   memcpy(push_save, desc->root.push, NVK_MAX_PUSH_SIZE);

   nvk_CmdBindPipeline(nvk_cmd_buffer_to_handle(cmd),
                       VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

   nvk_CmdPushConstants(nvk_cmd_buffer_to_handle(cmd), layout,
                        VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);

   nvk_CmdDispatchBase(nvk_cmd_buffer_to_handle(cmd), 0, 0, 0, 1, 1, 1);

   /* Restore pipeline and push constants */
   if (pipeline_save) {
      nvk_CmdBindPipeline(nvk_cmd_buffer_to_handle(cmd),
                          VK_PIPELINE_BIND_POINT_COMPUTE,
                          nvk_pipeline_to_handle(&pipeline_save->base));
   }
   memcpy(desc->root.push, push_save, NVK_MAX_PUSH_SIZE);
}

void
nvk_mme_copy_queries(struct mme_builder *b)
{
   if (b->devinfo->cls_eng3d < TURING_A)
      return;

   struct mme_value64 dst_addr = mme_load_addr64(b);
   struct mme_value64 dst_stride = mme_load_addr64(b);
   struct mme_value64 avail_addr = mme_load_addr64(b);
   struct mme_value64 report_addr = mme_load_addr64(b);

   struct mme_value query_count = mme_load(b);
   struct mme_value control = mme_load(b);

   struct mme_value flags = control;
   struct mme_value write64 =
      mme_and(b, flags, mme_imm(VK_QUERY_RESULT_64_BIT));
   struct mme_value query_stride =
      mme_merge(b, mme_zero(), control, 0, 16, 8);
   struct mme_value is_timestamp =
      mme_merge(b, mme_zero(), control, 0, 1, 24);

   mme_while(b, ugt, query_count, mme_zero()) {
      struct mme_value dw_per_query = mme_srl(b, query_stride, mme_imm(2));
      mme_tu104_read_fifoed(b, report_addr, dw_per_query);
      mme_free_reg(b, dw_per_query);

      struct mme_value64 write_addr = mme_mov64(b, dst_addr);
      struct mme_value report_count = mme_srl(b, query_stride, mme_imm(4));
      mme_while(b, ugt, report_count, mme_zero()) {
         struct mme_value result_lo = mme_alloc_reg(b);
         struct mme_value result_hi = mme_alloc_reg(b);
         struct mme_value64 result = mme_value64(result_lo, result_hi);

         mme_if(b, ine, is_timestamp, mme_zero()) {
            mme_load_to(b, mme_zero());
            mme_load_to(b, mme_zero());
            mme_load_to(b, result_lo);
            mme_load_to(b, result_hi);
            mme_sub_to(b, report_count, report_count, mme_imm(1));
         }
         mme_if(b, ieq, is_timestamp, mme_zero()) {
            struct mme_value begin_lo = mme_load(b);
            struct mme_value begin_hi = mme_load(b);
            struct mme_value64 begin = mme_value64(begin_lo, begin_hi);
            mme_load_to(b, mme_zero());
            mme_load_to(b, mme_zero());

            struct mme_value end_lo = mme_load(b);
            struct mme_value end_hi = mme_load(b);
            struct mme_value64 end = mme_value64(end_lo, end_hi);
            mme_load_to(b, mme_zero());
            mme_load_to(b, mme_zero());

            mme_sub64_to(b, result, end, begin);
            mme_sub_to(b, report_count, report_count, mme_imm(2));

            mme_free_reg64(b, begin);
            mme_free_reg64(b, end);
         }

         mme_store_global(b, write_addr, result_lo);
         mme_add64_to(b, write_addr, write_addr, mme_imm64(4));
         mme_if(b, ine, write64, mme_zero()) {
            mme_store_global(b, write_addr, result_hi);
            mme_add64_to(b, write_addr, write_addr, mme_imm64(4));
         }
      }

      struct mme_value with_availability =
         mme_and(b, flags, mme_imm(VK_QUERY_RESULT_WITH_AVAILABILITY_BIT));
      mme_if(b, ine, with_availability, mme_zero()) {
         mme_tu104_read_fifoed(b, avail_addr, mme_imm(1));
         struct mme_value avail = mme_load(b);
         mme_store_global(b, write_addr, avail);
         mme_if(b, ine, write64, mme_zero()) {
            mme_add64_to(b, write_addr, write_addr, mme_imm64(4));
            mme_store_global(b, write_addr, mme_zero());
         }
      }
      mme_free_reg(b, with_availability);

      mme_add64_to(b, avail_addr, avail_addr, mme_imm64(4));

      mme_add64_to(b, report_addr, report_addr,
                   mme_value64(query_stride, mme_zero()));

      mme_add64_to(b, dst_addr, dst_addr, dst_stride);

      mme_sub_to(b, query_count, query_count, mme_imm(1));
   }
}

static void
nvk_cmd_copy_query_pool_results_mme(struct nvk_cmd_buffer *cmd,
                                    struct nvk_query_pool *pool,
                                    uint32_t first_query,
                                    uint32_t query_count,
                                    uint64_t dst_addr,
                                    uint64_t dst_stride,
                                    VkQueryResultFlags flags)
{
   /* TODO: vkCmdCopyQueryPoolResults() with a compute shader */
   assert(nvk_cmd_buffer_device(cmd)->pdev->info.cls_eng3d >= TURING_A);

   struct nv_push *p = nvk_cmd_buffer_push(cmd, 13);
   P_IMMD(p, NVC597, SET_MME_DATA_FIFO_CONFIG, FIFO_SIZE_SIZE_4KB);
   P_1INC(p, NVC597, CALL_MME_MACRO(NVK_MME_COPY_QUERIES));

   P_INLINE_DATA(p, dst_addr >> 32);
   P_INLINE_DATA(p, dst_addr);
   P_INLINE_DATA(p, dst_stride >> 32);
   P_INLINE_DATA(p, dst_stride);

   uint64_t avail_start = nvk_query_available_addr(pool, first_query);
   P_INLINE_DATA(p, avail_start >> 32);
   P_INLINE_DATA(p, avail_start);

   uint64_t report_start = nvk_query_report_addr(pool, first_query);
   P_INLINE_DATA(p, report_start >> 32);
   P_INLINE_DATA(p, report_start);

   P_INLINE_DATA(p, query_count);

   uint32_t is_timestamp = pool->vk.query_type == VK_QUERY_TYPE_TIMESTAMP;

   uint32_t control = (flags & 0xff) |
                      (pool->query_stride << 8) |
                      (is_timestamp << 24);
   P_INLINE_DATA(p, control);
}

VKAPI_ATTR void VKAPI_CALL
nvk_CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer,
                            VkQueryPool queryPool,
                            uint32_t firstQuery,
                            uint32_t queryCount,
                            VkBuffer dstBuffer,
                            VkDeviceSize dstOffset,
                            VkDeviceSize stride,
                            VkQueryResultFlags flags)
{
   VK_FROM_HANDLE(nvk_cmd_buffer, cmd, commandBuffer);
   VK_FROM_HANDLE(nvk_query_pool, pool, queryPool);
   VK_FROM_HANDLE(nvk_buffer, dst_buffer, dstBuffer);

   if (flags & VK_QUERY_RESULT_WAIT_BIT) {
      for (uint32_t i = 0; i < queryCount; i++) {
         uint64_t avail_addr = nvk_query_available_addr(pool, firstQuery + i);

         struct nv_push *p = nvk_cmd_buffer_push(cmd, 5);
         __push_mthd(p, SUBC_NV9097, NV906F_SEMAPHOREA);
         P_NV906F_SEMAPHOREA(p, avail_addr >> 32);
         P_NV906F_SEMAPHOREB(p, (avail_addr & UINT32_MAX) >> 2);
         P_NV906F_SEMAPHOREC(p, 1);
         P_NV906F_SEMAPHORED(p, {
            .operation = OPERATION_ACQ_GEQ,
            .acquire_switch = ACQUIRE_SWITCH_ENABLED,
            .release_size = RELEASE_SIZE_4BYTE,
         });
      }
   }

   uint64_t dst_addr = nvk_buffer_address(dst_buffer, dstOffset);
   nvk_meta_copy_query_pool_results(cmd, pool, firstQuery, queryCount,
                                    dst_addr, stride, flags);
}
