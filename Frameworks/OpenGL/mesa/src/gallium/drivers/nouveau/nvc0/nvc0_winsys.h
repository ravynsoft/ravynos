
#ifndef __NVC0_WINSYS_H__
#define __NVC0_WINSYS_H__

#include <stdint.h>
#include <unistd.h>

#include "pipe/p_defines.h"

#include "nouveau_winsys.h"
#include "nouveau_buffer.h"

#ifndef NV04_PFIFO_MAX_PACKET_LEN
#define NV04_PFIFO_MAX_PACKET_LEN 2047
#endif


static inline void
nv50_add_bufctx_resident_bo(struct nouveau_bufctx *bufctx, int bin,
                            unsigned flags, struct nouveau_bo *bo)
{
   nouveau_bufctx_refn(bufctx, bin, bo, flags)->priv = NULL;
}

static inline void
nvc0_add_resident(struct nouveau_bufctx *bufctx, int bin,
                  struct nv04_resource *res, unsigned flags)
{
   struct nouveau_bufref *ref =
      nouveau_bufctx_refn(bufctx, bin, res->bo, flags | res->domain);
   ref->priv = res;
   ref->priv_data = flags;
}

#define BCTX_REFN_bo(ctx, bin, fl, bo) \
   nv50_add_bufctx_resident_bo(ctx, NVC0_BIND_##bin, fl, bo);

#define BCTX_REFN(bctx, bin, res, acc) \
   nvc0_add_resident(bctx, NVC0_BIND_##bin, res, NOUVEAU_BO_##acc)

#define SUBC_3D(m) 0, (m)
#define NVC0_3D(n) SUBC_3D(NVC0_3D_##n)
#define NVE4_3D(n) SUBC_3D(NVE4_3D_##n)

#define SUBC_CP(m) 1, (m)
#define NVC0_CP(n) SUBC_CP(NVC0_COMPUTE_##n)
#define NVE4_CP(n) SUBC_CP(NVE4_COMPUTE_##n)

#define SUBC_M2MF(m) 2, (m)
#define SUBC_P2MF(m) 2, (m)
#define NVC0_M2MF(n) SUBC_M2MF(NVC0_M2MF_##n)
#define NVE4_P2MF(n) SUBC_P2MF(NVE4_P2MF_##n)

#define SUBC_2D(m) 3, (m)
#define NVC0_2D(n) SUBC_2D(NV50_2D_##n)

#define SUBC_COPY(m) 4, (m)
#define NVE4_COPY(n) SUBC_COPY(NVE4_COPY_##n)

#define SUBC_SW(m) 7, (m)

#define NVC0_3D_SERIALIZE NV50_GRAPH_SERIALIZE
#define NVC0_IB_ENTRY_1_NO_PREFETCH (1 << (31 - 8))

static inline uint32_t
NVC0_FIFO_PKHDR_SQ(int subc, int mthd, unsigned size)
{
   return 0x20000000 | (size << 16) | (subc << 13) | (mthd >> 2);
}

static inline uint32_t
NVC0_FIFO_PKHDR_NI(int subc, int mthd, unsigned size)
{
   return 0x60000000 | (size << 16) | (subc << 13) | (mthd >> 2);
}

static inline uint32_t
NVC0_FIFO_PKHDR_IL(int subc, int mthd, uint16_t data)
{
   assert(data < 0x2000);
   return 0x80000000 | (data << 16) | (subc << 13) | (mthd >> 2);
}

static inline uint32_t
NVC0_FIFO_PKHDR_1I(int subc, int mthd, unsigned size)
{
   return 0xa0000000 | (size << 16) | (subc << 13) | (mthd >> 2);
}


static inline uint8_t
nouveau_bo_memtype(const struct nouveau_bo *bo)
{
   return bo->config.nvc0.memtype;
}


static inline void
PUSH_DATAh(struct nouveau_pushbuf *push, uint64_t data)
{
   *push->cur++ = (uint32_t)(data >> 32);
}

static inline void
BEGIN_NVC0(struct nouveau_pushbuf *push, int subc, int mthd, unsigned size)
{
#ifndef NVC0_PUSH_EXPLICIT_SPACE_CHECKING
   PUSH_SPACE(push, size + 1);
#endif
   PUSH_DATA (push, NVC0_FIFO_PKHDR_SQ(subc, mthd, size));
}

static inline void
BEGIN_NIC0(struct nouveau_pushbuf *push, int subc, int mthd, unsigned size)
{
#ifndef NVC0_PUSH_EXPLICIT_SPACE_CHECKING
   PUSH_SPACE(push, size + 1);
#endif
   PUSH_DATA (push, NVC0_FIFO_PKHDR_NI(subc, mthd, size));
}

static inline void
BEGIN_1IC0(struct nouveau_pushbuf *push, int subc, int mthd, unsigned size)
{
#ifndef NVC0_PUSH_EXPLICIT_SPACE_CHECKING
   PUSH_SPACE(push, size + 1);
#endif
   PUSH_DATA (push, NVC0_FIFO_PKHDR_1I(subc, mthd, size));
}

static inline void
IMMED_NVC0(struct nouveau_pushbuf *push, int subc, int mthd, uint16_t data)
{
#ifndef NVC0_PUSH_EXPLICIT_SPACE_CHECKING
   PUSH_SPACE(push, 1);
#endif
   PUSH_DATA (push, NVC0_FIFO_PKHDR_IL(subc, mthd, data));
}

#endif /* __NVC0_WINSYS_H__ */
