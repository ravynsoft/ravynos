#ifndef __NV50_QUERY_H__
#define __NV50_QUERY_H__

#include "pipe/p_context.h"

#include "nouveau_context.h"

struct nv50_context;
struct nv50_query;

struct nv50_query_funcs {
   void (*destroy_query)(struct nv50_context *, struct nv50_query *);
   bool (*begin_query)(struct nv50_context *, struct nv50_query *);
   void (*end_query)(struct nv50_context *, struct nv50_query *);
   bool (*get_query_result)(struct nv50_context *, struct nv50_query *,
                            bool, union pipe_query_result *);
};

struct nv50_query {
   const struct nv50_query_funcs *funcs;
   uint16_t type;
   uint16_t index;
};

static inline struct nv50_query *
nv50_query(struct pipe_query *pipe)
{
   return (struct nv50_query *)pipe;
}

/*
 * Driver queries groups:
 */
#define NV50_HW_SM_QUERY_GROUP       0
#define NV50_HW_METRIC_QUERY_GROUP   1

void nv50_init_query_functions(struct nv50_context *);

#endif
