
#ifndef __NV50_WINSYS_H__
#define __NV50_WINSYS_H__

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
nv50_add_bufctx_resident(struct nouveau_bufctx *bufctx, int bin,
                         struct nv04_resource *res, unsigned flags)
{
   struct nouveau_bufref *ref =
      nouveau_bufctx_refn(bufctx, bin, res->bo, flags | res->domain);
   ref->priv = res;
   ref->priv_data = flags;
}

#define BCTX_REFN_bo(ctx, bin, fl, bo) \
   nv50_add_bufctx_resident_bo(ctx, NV50_BIND_##bin, fl, bo);

#define BCTX_REFN(bctx, bin, res, acc) \
   nv50_add_bufctx_resident(bctx, NV50_BIND_##bin, res, NOUVEAU_BO_##acc)

#define SUBC_3D(m) 3, (m)
#define NV50_3D(n) SUBC_3D(NV50_3D_##n)
#define NV84_3D(n) SUBC_3D(NV84_3D_##n)
#define NVA0_3D(n) SUBC_3D(NVA0_3D_##n)

#define SUBC_2D(m) 4, (m)
#define NV50_2D(n) SUBC_2D(NV50_2D_##n)

#define SUBC_M2MF(m) 5, (m)
#define NV50_M2MF(n) SUBC_M2MF(NV50_M2MF_##n)

#define SUBC_CP(m) 6, (m)
#define NV50_CP(n) SUBC_CP(NV50_COMPUTE_##n)


static inline uint32_t
NV50_FIFO_PKHDR(int subc, int mthd, unsigned size)
{
   return 0x00000000 | (size << 18) | (subc << 13) | mthd;
}

static inline uint32_t
NV50_FIFO_PKHDR_NI(int subc, int mthd, unsigned size)
{
   return 0x40000000 | (size << 18) | (subc << 13) | mthd;
}

static inline uint32_t
NV50_FIFO_PKHDR_L(int subc, int mthd)
{
   return 0x00030000 | (subc << 13) | mthd;
}


static inline uint32_t
nouveau_bo_memtype(const struct nouveau_bo *bo)
{
   return bo->config.nv50.memtype;
}


static inline void
PUSH_DATAh(struct nouveau_pushbuf *push, uint64_t data)
{
   *push->cur++ = (uint32_t)(data >> 32);
}

/* long, non-incremental, nv50-only */
static inline void
BEGIN_NL50(struct nouveau_pushbuf *push, int subc, int mthd, uint32_t size)
{
#ifndef NV50_PUSH_EXPLICIT_SPACE_CHECKING
   PUSH_SPACE(push, 2);
#endif
   PUSH_DATA (push, NV50_FIFO_PKHDR_L(subc, mthd));
   PUSH_DATA (push, size);
}

#endif /* __NV50_WINSYS_H__ */
