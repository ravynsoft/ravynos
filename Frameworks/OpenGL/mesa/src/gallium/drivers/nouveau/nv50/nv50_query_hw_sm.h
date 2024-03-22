#ifndef __NV50_QUERY_HW_SM_H__
#define __NV50_QUERY_HW_SM_H__

#include "nv50_query_hw.h"

struct nv50_hw_sm_query {
   struct nv50_hw_query base;
   uint8_t ctr[4];
};

static inline struct nv50_hw_sm_query *
nv50_hw_sm_query(struct nv50_hw_query *hq)
{
   return (struct nv50_hw_sm_query *)hq;
}

/*
 * Performance counter queries:
 */
#define NV50_HW_SM_QUERY(i)    (PIPE_QUERY_DRIVER_SPECIFIC + (i))
#define NV50_HW_SM_QUERY_LAST   NV50_HW_SM_QUERY(NV50_HW_SM_QUERY_COUNT - 1)
enum nv50_hw_sm_queries
{
   NV50_HW_SM_QUERY_BRANCH = 0,
   NV50_HW_SM_QUERY_DIVERGENT_BRANCH,
   NV50_HW_SM_QUERY_INSTRUCTIONS,
   NV50_HW_SM_QUERY_PROF_TRIGGER_0,
   NV50_HW_SM_QUERY_PROF_TRIGGER_1,
   NV50_HW_SM_QUERY_PROF_TRIGGER_2,
   NV50_HW_SM_QUERY_PROF_TRIGGER_3,
   NV50_HW_SM_QUERY_PROF_TRIGGER_4,
   NV50_HW_SM_QUERY_PROF_TRIGGER_5,
   NV50_HW_SM_QUERY_PROF_TRIGGER_6,
   NV50_HW_SM_QUERY_PROF_TRIGGER_7,
   NV50_HW_SM_QUERY_SM_CTA_LAUNCHED,
   NV50_HW_SM_QUERY_WARP_SERIALIZE,
   NV50_HW_SM_QUERY_COUNT,
};

struct nv50_hw_query *
nv50_hw_sm_create_query(struct nv50_context *, unsigned);
int
nv50_hw_sm_get_driver_query_info(struct nv50_screen *, unsigned,
                                 struct pipe_driver_query_info *);
#endif
