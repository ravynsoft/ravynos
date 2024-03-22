#ifndef NOUVEAU_WINSYS_H
#define NOUVEAU_WINSYS_H

#include <stdint.h>
#include <inttypes.h>

#include "pipe/p_defines.h"
#include "util/os_misc.h"

#include "drm-uapi/drm.h"
#include <nouveau.h>

#include "nouveau_screen.h"

#ifndef NV04_PFIFO_MAX_PACKET_LEN
#define NV04_PFIFO_MAX_PACKET_LEN 2047
#endif

#define NOUVEAU_MIN_BUFFER_MAP_ALIGN      64
#define NOUVEAU_MIN_BUFFER_MAP_ALIGN_MASK (NOUVEAU_MIN_BUFFER_MAP_ALIGN - 1)

static inline uint32_t
PUSH_AVAIL(struct nouveau_pushbuf *push)
{
   return push->end - push->cur;
}

static inline bool
PUSH_SPACE_EX(struct nouveau_pushbuf *push, uint32_t size, uint32_t relocs, uint32_t pushes)
{
   struct nouveau_pushbuf_priv *ppush = push->user_priv;
   simple_mtx_lock(&ppush->screen->fence.lock);
   bool res = nouveau_pushbuf_space(push, size, relocs, pushes) == 0;
   simple_mtx_unlock(&ppush->screen->fence.lock);
   return res;
}

static inline bool
PUSH_SPACE(struct nouveau_pushbuf *push, uint32_t size)
{
   /* Provide a buffer so that fences always have room to be emitted */
   size += 8;
   if (PUSH_AVAIL(push) < size)
      return PUSH_SPACE_EX(push, size, 0, 0);
   return true;
}

static inline void
PUSH_DATA(struct nouveau_pushbuf *push, uint32_t data)
{
   *push->cur++ = data;
}

static inline void
PUSH_DATAp(struct nouveau_pushbuf *push, const void *data, uint32_t size)
{
   memcpy(push->cur, data, size * 4);
   push->cur += size;
}

static inline void
PUSH_DATAb(struct nouveau_pushbuf *push, const void *data, uint32_t size)
{
   memcpy(push->cur, data, size);
   push->cur += DIV_ROUND_UP(size, 4);
}

static inline void
PUSH_DATAf(struct nouveau_pushbuf *push, float f)
{
   union { float f; uint32_t i; } u;
   u.f = f;
   PUSH_DATA(push, u.i);
}

static inline int
PUSH_REFN(struct nouveau_pushbuf *push, struct nouveau_pushbuf_refn *refs, int nr)
{
   struct nouveau_pushbuf_priv *ppush = push->user_priv;
   simple_mtx_lock(&ppush->screen->fence.lock);
   int ret = nouveau_pushbuf_refn(push, refs, nr);
   simple_mtx_unlock(&ppush->screen->fence.lock);
   return ret;
}

static inline int
PUSH_REF1(struct nouveau_pushbuf *push, struct nouveau_bo *bo, uint32_t flags)
{
   struct nouveau_pushbuf_refn ref = { bo, flags };
   return PUSH_REFN(push, &ref, 1);
}

static inline void
PUSH_KICK(struct nouveau_pushbuf *push)
{
   struct nouveau_pushbuf_priv *ppush = push->user_priv;
   simple_mtx_lock(&ppush->screen->fence.lock);
   nouveau_pushbuf_kick(push, push->channel);
   simple_mtx_unlock(&ppush->screen->fence.lock);
}

static inline int
PUSH_VAL(struct nouveau_pushbuf *push)
{
   struct nouveau_pushbuf_priv *ppush = push->user_priv;
   simple_mtx_lock(&ppush->screen->fence.lock);
   int res = nouveau_pushbuf_validate(push);
   simple_mtx_unlock(&ppush->screen->fence.lock);
   return res;
}

static inline uint32_t
NV04_FIFO_PKHDR(int subc, int mthd, unsigned size)
{
   return 0x00000000 | (size << 18) | (subc << 13) | mthd;
}

static inline uint32_t
NV04_FIFO_PKHDR_NI(int subc, int mthd, unsigned size)
{
   return 0x40000000 | (size << 18) | (subc << 13) | mthd;
}

static inline void
BEGIN_NV04(struct nouveau_pushbuf *push, int subc, int mthd, unsigned size)
{
#ifndef NV50_PUSH_EXPLICIT_SPACE_CHECKING
   PUSH_SPACE(push, size + 1);
#endif
   PUSH_DATA (push, NV04_FIFO_PKHDR(subc, mthd, size));
}

static inline void
BEGIN_NI04(struct nouveau_pushbuf *push, int subc, int mthd, unsigned size)
{
#ifndef NV50_PUSH_EXPLICIT_SPACE_CHECKING
   PUSH_SPACE(push, size + 1);
#endif
   PUSH_DATA (push, NV04_FIFO_PKHDR_NI(subc, mthd, size));
}

static inline int
BO_MAP(struct nouveau_screen *screen, struct nouveau_bo *bo, uint32_t access, struct nouveau_client *client)
{
   int res;
   simple_mtx_lock(&screen->fence.lock);
   res = nouveau_bo_map(bo, access, client);
   simple_mtx_unlock(&screen->fence.lock);
   return res;
}

static inline int
BO_WAIT(struct nouveau_screen *screen, struct nouveau_bo *bo, uint32_t access, struct nouveau_client *client)
{
   int res;
   simple_mtx_lock(&screen->fence.lock);
   res = nouveau_bo_wait(bo, access, client);
   simple_mtx_unlock(&screen->fence.lock);
   return res;
}

#define NOUVEAU_RESOURCE_FLAG_LINEAR   (PIPE_RESOURCE_FLAG_DRV_PRIV << 0)
#define NOUVEAU_RESOURCE_FLAG_DRV_PRIV (PIPE_RESOURCE_FLAG_DRV_PRIV << 1)

static inline uint32_t
nouveau_screen_transfer_flags(unsigned pipe)
{
   uint32_t flags = 0;

   if (!(pipe & PIPE_MAP_UNSYNCHRONIZED)) {
      if (pipe & PIPE_MAP_READ)
         flags |= NOUVEAU_BO_RD;
      if (pipe & PIPE_MAP_WRITE)
         flags |= NOUVEAU_BO_WR;
      if (pipe & PIPE_MAP_DONTBLOCK)
         flags |= NOUVEAU_BO_NOBLOCK;
   }

   return flags;
}

extern struct nouveau_screen *
nv30_screen_create(struct nouveau_device *);

extern struct nouveau_screen *
nv50_screen_create(struct nouveau_device *);

extern struct nouveau_screen *
nvc0_screen_create(struct nouveau_device *);

static inline uint64_t
nouveau_device_get_global_mem_size(struct nouveau_device *dev)
{
   uint64_t size = dev->vram_size;

   if (!size) {
      os_get_available_system_memory(&size);
      size = MIN2(dev->gart_size, size);
   }

   /* cap to 32 bit on nv50 and older */
   if (dev->chipset < 0xc0)
      size = MIN2(size, 1ull << 32);
   else
      size = MIN2(size, 1ull << 40);

   return size;
}

#endif
