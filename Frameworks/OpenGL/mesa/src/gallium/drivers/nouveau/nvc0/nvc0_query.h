#ifndef __NVC0_QUERY_H__
#define __NVC0_QUERY_H__

#include "pipe/p_context.h"

#include "nouveau_context.h"

struct nvc0_context;
struct nvc0_query;

struct nvc0_query_funcs {
   void (*destroy_query)(struct nvc0_context *, struct nvc0_query *);
   bool (*begin_query)(struct nvc0_context *, struct nvc0_query *);
   void (*end_query)(struct nvc0_context *, struct nvc0_query *);
   bool (*get_query_result)(struct nvc0_context *, struct nvc0_query *,
                            bool, union pipe_query_result *);
   void (*get_query_result_resource)(struct nvc0_context *nvc0,
                                     struct nvc0_query *q,
                                     enum pipe_query_flags flags,
                                     enum pipe_query_value_type result_type,
                                     int index,
                                     struct pipe_resource *resource,
                                     unsigned offset);
};

struct nvc0_query {
   const struct nvc0_query_funcs *funcs;
   uint16_t type;
   uint16_t index;
};

static inline struct nvc0_query *
nvc0_query(struct pipe_query *pipe)
{
   return (struct nvc0_query *)pipe;
}

/*
 * Driver queries groups:
 */
#define NVC0_HW_SM_QUERY_GROUP       0
#define NVC0_HW_METRIC_QUERY_GROUP   1
#define NVC0_SW_QUERY_DRV_STAT_GROUP 2

void nvc0_init_query_functions(struct nvc0_context *);

#endif
