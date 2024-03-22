#ifndef __NOUVEAU_CONTEXT_H__
#define __NOUVEAU_CONTEXT_H__

#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include <nouveau.h>

#define NOUVEAU_MAX_SCRATCH_BUFS 4

struct nv04_resource;

struct nouveau_context {
   struct pipe_context pipe;
   struct nouveau_screen *screen;

   struct nouveau_client *client;
   struct nouveau_pushbuf *pushbuf;
   struct nouveau_fence *fence;
   void (*kick_notify)(struct nouveau_context *);
   struct util_debug_callback debug;

   bool vbo_dirty;

   void (*copy_data)(struct nouveau_context *,
                     struct nouveau_bo *dst, unsigned, unsigned,
                     struct nouveau_bo *src, unsigned, unsigned, unsigned);
   void (*push_data)(struct nouveau_context *,
                     struct nouveau_bo *dst, unsigned, unsigned,
                     unsigned, const void *);
   /* base, size refer to the whole constant buffer */
   void (*push_cb)(struct nouveau_context *,
                   struct nv04_resource *,
                   unsigned offset, unsigned words, const uint32_t *);

   /* @return: @ref reduced by nr of references found in context */
   int (*invalidate_resource_storage)(struct nouveau_context *,
                                      struct pipe_resource *,
                                      int ref);

   struct {
      uint8_t *map;
      unsigned id;
      unsigned wrap;
      unsigned offset;
      unsigned end;
      struct nouveau_bo *bo[NOUVEAU_MAX_SCRATCH_BUFS];
      struct nouveau_bo *current;
      struct runout {
         unsigned nr;
         struct nouveau_bo *bo[0];
      } *runout;
      unsigned bo_size;
   } scratch;

   struct {
      uint32_t buf_cache_count;
      uint32_t buf_cache_frame;
   } stats;
};

static inline struct nouveau_context *
nouveau_context(struct pipe_context *pipe)
{
   return (struct nouveau_context *)pipe;
}

void
nouveau_context_init_vdec(struct nouveau_context *);

int MUST_CHECK
nouveau_context_init(struct nouveau_context *, struct nouveau_screen *);

void
nouveau_scratch_runout_release(struct nouveau_context *);

/* This is needed because we don't hold references outside of context::scratch,
 * because we don't want to un-bo_ref each allocation every time. This is less
 * work, and we need the wrap index anyway for extreme situations.
 */
static inline void
nouveau_scratch_done(struct nouveau_context *nv)
{
   nv->scratch.wrap = nv->scratch.id;
   if (unlikely(nv->scratch.runout))
      nouveau_scratch_runout_release(nv);
}

/* Get pointer to scratch buffer.
 * The returned nouveau_bo is only referenced by the context, don't un-ref it !
 */
void *
nouveau_scratch_get(struct nouveau_context *, unsigned size, uint64_t *gpu_addr,
                    struct nouveau_bo **);

static inline void
nouveau_context_destroy(struct nouveau_context *ctx)
{
   int i;

   for (i = 0; i < NOUVEAU_MAX_SCRATCH_BUFS; ++i)
      if (ctx->scratch.bo[i])
         nouveau_bo_ref(NULL, &ctx->scratch.bo[i]);

   nouveau_pushbuf_destroy(&ctx->pushbuf);
   nouveau_client_del(&ctx->client);

   FREE(ctx);
}

static inline  void
nouveau_context_update_frame_stats(struct nouveau_context *nv)
{
   nv->stats.buf_cache_frame <<= 1;
   if (nv->stats.buf_cache_count) {
      nv->stats.buf_cache_count = 0;
      nv->stats.buf_cache_frame |= 1;
      if ((nv->stats.buf_cache_frame & 0xf) == 0xf)
         nv->screen->hint_buf_keep_sysmem_copy = true;
   }
}

#endif
