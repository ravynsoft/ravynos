#ifndef __NV50_QUERY_HW_METRIC_H__
#define __NV50_QUERY_HW_METRIC_H__

#include "nv50_query_hw.h"

struct nv50_hw_metric_query {
   struct nv50_hw_query base;
   struct nv50_hw_query *queries[4];
   unsigned num_queries;
};

static inline struct nv50_hw_metric_query *
nv50_hw_metric_query(struct nv50_hw_query *hq)
{
   return (struct nv50_hw_metric_query *)hq;
}

/*
 * Driver metrics queries:
 */
#define NV50_HW_METRIC_QUERY(i)   (PIPE_QUERY_DRIVER_SPECIFIC + 1024 + (i))
#define NV50_HW_METRIC_QUERY_LAST  NV50_HW_METRIC_QUERY(NV50_HW_METRIC_QUERY_COUNT - 1)
enum nv50_hw_metric_queries
{
    NV50_HW_METRIC_QUERY_BRANCH_EFFICIENCY = 0,
    NV50_HW_METRIC_QUERY_COUNT
};

struct nv50_hw_query *
nv50_hw_metric_create_query(struct nv50_context *, unsigned);
int
nv50_hw_metric_get_driver_query_info(struct nv50_screen *, unsigned,
                                     struct pipe_driver_query_info *);
#endif
