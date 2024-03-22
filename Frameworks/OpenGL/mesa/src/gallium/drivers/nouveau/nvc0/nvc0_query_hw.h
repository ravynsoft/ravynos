#ifndef __NVC0_QUERY_HW_H__
#define __NVC0_QUERY_HW_H__

#include "nouveau_fence.h"
#include "nouveau_mm.h"

#include "nvc0_query.h"

#define NVC0_HW_QUERY_STATE_READY   0
#define NVC0_HW_QUERY_STATE_ACTIVE  1
#define NVC0_HW_QUERY_STATE_ENDED   2
#define NVC0_HW_QUERY_STATE_FLUSHED 3

#define NVC0_HW_QUERY_TFB_BUFFER_OFFSET (PIPE_QUERY_TYPES + 0)

struct nvc0_hw_query;

struct nvc0_hw_query_funcs {
   void (*destroy_query)(struct nvc0_context *, struct nvc0_hw_query *);
   bool (*begin_query)(struct nvc0_context *, struct nvc0_hw_query *);
   void (*end_query)(struct nvc0_context *, struct nvc0_hw_query *);
   bool (*get_query_result)(struct nvc0_context *, struct nvc0_hw_query *,
                            bool, union pipe_query_result *);
};

struct nvc0_hw_query {
   struct nvc0_query base;
   const struct nvc0_hw_query_funcs *funcs;
   uint32_t *data;
   uint32_t sequence;
   struct nouveau_bo *bo;
   uint32_t base_offset;
   uint32_t offset; /* base_offset + i * rotate */
   uint8_t state;
   bool is64bit;
   uint8_t rotate;
   struct nouveau_mm_allocation *mm;
   struct nouveau_fence *fence;
};

static inline struct nvc0_hw_query *
nvc0_hw_query(struct nvc0_query *q)
{
   return (struct nvc0_hw_query *)q;
}

struct nvc0_query *
nvc0_hw_create_query(struct nvc0_context *, unsigned, unsigned);
int
nvc0_hw_get_driver_query_info(struct nvc0_screen *, unsigned,
                              struct pipe_driver_query_info *);
bool
nvc0_hw_query_allocate(struct nvc0_context *, struct nvc0_query *, int);
void
nvc0_hw_query_pushbuf_submit(struct nouveau_pushbuf *, struct nvc0_query *,
                             unsigned);
void
nvc0_hw_query_fifo_wait(struct nvc0_context *, struct nvc0_query *);

#endif
