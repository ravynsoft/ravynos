#ifndef __NV50_QUERY_HW_H__
#define __NV50_QUERY_HW_H__

#include "nouveau_fence.h"
#include "nouveau_mm.h"

#include "nv50_query.h"

#define NV50_HW_QUERY_STATE_READY   0
#define NV50_HW_QUERY_STATE_ACTIVE  1
#define NV50_HW_QUERY_STATE_ENDED   2
#define NV50_HW_QUERY_STATE_FLUSHED 3

#define NVA0_HW_QUERY_STREAM_OUTPUT_BUFFER_OFFSET (PIPE_QUERY_TYPES + 0)

struct nv50_hw_query;

struct nv50_hw_query_funcs {
   void (*destroy_query)(struct nv50_context *, struct nv50_hw_query *);
   bool (*begin_query)(struct nv50_context *, struct nv50_hw_query *);
   void (*end_query)(struct nv50_context *, struct nv50_hw_query *);
   bool (*get_query_result)(struct nv50_context *, struct nv50_hw_query *,
                            bool, union pipe_query_result *);
};

struct nv50_hw_query {
   struct nv50_query base;
   const struct nv50_hw_query_funcs *funcs;
   uint32_t *data;
   uint32_t sequence;
   struct nouveau_bo *bo;
   uint32_t base_offset;
   uint32_t offset; /* base + i * rotate */
   uint8_t state;
   bool is64bit;
   uint8_t rotate;
   struct nouveau_mm_allocation *mm;
   struct nouveau_fence *fence;
};

static inline struct nv50_hw_query *
nv50_hw_query(struct nv50_query *q)
{
   return (struct nv50_hw_query *)q;
}

struct nv50_query *
nv50_hw_create_query(struct nv50_context *, unsigned, unsigned);
int
nv50_hw_get_driver_query_info(struct nv50_screen *, unsigned,
                              struct pipe_driver_query_info *);
bool
nv50_hw_query_allocate(struct nv50_context *, struct nv50_query *, int);
void
nv50_hw_query_pushbuf_submit(struct nv50_context *, uint16_t,
                             struct nv50_query *, unsigned);
void
nv84_hw_query_fifo_wait(struct nouveau_pushbuf *, struct nv50_query *);

#endif
