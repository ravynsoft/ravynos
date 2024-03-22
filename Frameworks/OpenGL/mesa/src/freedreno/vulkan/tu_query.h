/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#ifndef TU_QUERY_H
#define TU_QUERY_H

#include "tu_common.h"

#define PERF_CNTRS_REG 4

struct tu_perf_query_data
{
   uint32_t gid;      /* group-id */
   uint32_t cid;      /* countable-id within the group */
   uint32_t cntr_reg; /* counter register within the group */
   uint32_t pass;     /* pass index that countables can be requested */
   uint32_t app_idx;  /* index provided by apps */
};

struct tu_query_pool
{
   struct vk_object_base base;

   VkQueryType type;
   uint32_t stride;
   uint64_t size;
   uint32_t pipeline_statistics;
   struct tu_bo *bo;

   /* For performance query */
   const struct fd_perfcntr_group *perf_group;
   uint32_t perf_group_count;
   uint32_t counter_index_count;
   struct tu_perf_query_data perf_query_data[0];
};
VK_DEFINE_NONDISP_HANDLE_CASTS(tu_query_pool, base, VkQueryPool,
                               VK_OBJECT_TYPE_QUERY_POOL)

#endif /* TU_QUERY_H */
