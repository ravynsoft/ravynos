
#ifndef __NOUVEAU_FENCE_H__
#define __NOUVEAU_FENCE_H__

#include "util/u_inlines.h"
#include "util/list.h"
#include "util/simple_mtx.h"

#define NOUVEAU_FENCE_STATE_AVAILABLE 0
#define NOUVEAU_FENCE_STATE_EMITTING  1
#define NOUVEAU_FENCE_STATE_EMITTED   2
#define NOUVEAU_FENCE_STATE_FLUSHED   3
#define NOUVEAU_FENCE_STATE_SIGNALLED 4

struct util_debug_callback;

struct nouveau_fence_work {
   struct list_head list;
   void (*func)(void *);
   void *data;
};

struct nouveau_fence {
   struct nouveau_fence *next;
   struct nouveau_screen *screen;
   struct nouveau_context *context;
   struct nouveau_bo *bo;
   int state;
   int ref;
   uint32_t sequence;
   uint32_t work_count;
   struct list_head work;
};

struct nouveau_fence_list {
   struct nouveau_fence *head;
   struct nouveau_fence *tail;
   uint32_t sequence;
   uint32_t sequence_ack;
   simple_mtx_t lock;
   void (*emit)(struct pipe_context *, uint32_t *sequence, struct nouveau_bo *wait);
   uint32_t (*update)(struct pipe_screen *);
};

static inline void
nouveau_fence_list_init(struct nouveau_fence_list *fence_list)
{
   simple_mtx_init(&fence_list->lock, mtx_plain);
}

static inline void
nouveau_fence_list_destroy(struct nouveau_fence_list *fence_list)
{
   simple_mtx_destroy(&fence_list->lock);
}

/* unlocked versions, use with care */
void _nouveau_fence_update(struct nouveau_screen *, bool flushed);
void _nouveau_fence_next(struct nouveau_context *);
void _nouveau_fence_ref(struct nouveau_fence *, struct nouveau_fence **);

bool nouveau_fence_new(struct nouveau_context *, struct nouveau_fence **);
void nouveau_fence_cleanup(struct nouveau_context *);
bool nouveau_fence_work(struct nouveau_fence *, void (*)(void *), void *);
void nouveau_fence_update(struct nouveau_screen *, bool flushed);
void nouveau_fence_next_if_current(struct nouveau_context *, struct nouveau_fence *);
bool nouveau_fence_wait(struct nouveau_fence *, struct util_debug_callback *);
bool nouveau_fence_signalled(struct nouveau_fence *);
void nouveau_fence_ref(struct nouveau_fence *, struct nouveau_fence **);

void nouveau_fence_unref_bo(void *data); /* generic unref bo callback */

static inline struct nouveau_fence *
nouveau_fence(struct pipe_fence_handle *fence)
{
   return (struct nouveau_fence *)fence;
}

#endif // __NOUVEAU_FENCE_H__
