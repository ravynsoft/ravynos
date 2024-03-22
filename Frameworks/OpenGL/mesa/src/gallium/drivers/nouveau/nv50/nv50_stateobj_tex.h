
#ifndef __NV50_STATEOBJ_TEX_H__
#define __NV50_STATEOBJ_TEX_H__

#include "pipe/p_state.h"

struct nv50_tsc_entry {
   int id;
   uint32_t tsc[8];
   bool seamless_cube_map;
};

static inline struct nv50_tsc_entry *
nv50_tsc_entry(void *hwcso)
{
   return (struct nv50_tsc_entry *)hwcso;
}

struct nv50_tic_entry {
   struct pipe_sampler_view pipe;
   int id;
   uint32_t tic[8];
   uint32_t bindless;
};

static inline struct nv50_tic_entry *
nv50_tic_entry(struct pipe_sampler_view *view)
{
   return (struct nv50_tic_entry *)view;
}

extern void *
nv50_sampler_state_create(struct pipe_context *,
                          const struct pipe_sampler_state *);

#endif /* __NV50_STATEOBJ_TEX_H__ */
