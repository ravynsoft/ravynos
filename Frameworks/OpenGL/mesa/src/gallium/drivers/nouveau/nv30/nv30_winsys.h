#ifndef __NV30_WINSYS_H__
#define __NV30_WINSYS_H__

#include <string.h>
#include "nouveau_winsys.h"
#include "nouveau_context.h"
#include "nouveau_buffer.h"
#include "nv30_context.h"

/*XXX: rnn */
#define NV40_3D_VTXTEX_OFFSET(i) (0x0900 + ((i) * 0x20)) // 401e80
#define NV40_3D_VTXTEX_FORMAT(i) (0x0904 + ((i) * 0x20)) // 401e90
#define NV40_3D_VTXTEX_WRAP(i)   (0x0908 + ((i) * 0x20)) // 401ea0
#define NV40_3D_VTXTEX_ENABLE(i) (0x090c + ((i) * 0x20)) // 401eb0
#define NV40_3D_VTXTEX_SWZ(i)    (0x0910 + ((i) * 0x20)) // 401ec0
#define NV40_3D_VTXTEX_FILTER(i) (0x0914 + ((i) * 0x20)) // 401ed0
#define NV40_3D_VTXTEX_SIZE(i)   (0x0918 + ((i) * 0x20)) // 401ee0
#define NV40_3D_VTXTEX_BCOL(i)   (0x091c + ((i) * 0x20)) // 401ef0
#define NV30_3D_VTX_CACHE_INVALIDATE_1710 0x1710
#define NV30_3D_R1718 0x1718
#define NV40_3D_PRIM_RESTART_ENABLE 0x1dac
#define NV40_3D_PRIM_RESTART_INDEX  0x1db0

static inline void
PUSH_RELOC(struct nouveau_pushbuf *push, struct nouveau_bo *bo, uint32_t offset,
      uint32_t flags, uint32_t vor, uint32_t tor)
{
   nouveau_pushbuf_reloc(push, bo, offset, flags, vor, tor);
}

static inline struct nouveau_bufctx *
bufctx(struct nouveau_pushbuf *push)
{
   struct nouveau_pushbuf_priv *p = push->user_priv;
   return nv30_context(&p->context->pipe)->bufctx;
}

static inline void
PUSH_RESET(struct nouveau_pushbuf *push, int bin)
{
   nouveau_bufctx_reset(bufctx(push), bin);
}

static inline void
PUSH_MTHDl(struct nouveau_pushbuf *push, int subc, int mthd, int bin,
      struct nouveau_bo *bo, uint32_t offset, uint32_t access)
{
   nouveau_bufctx_mthd(bufctx(push), bin, (1 << 18) | (subc << 13) | mthd,
                       bo, offset, access | NOUVEAU_BO_LOW, 0, 0)->priv = NULL;
   PUSH_DATA(push, bo->offset + offset);
}

static inline void
PUSH_MTHDo(struct nouveau_pushbuf *push, int subc, int mthd, int bin,
      struct nouveau_bo *bo, uint32_t access, uint32_t vor, uint32_t tor)
{
   nouveau_bufctx_mthd(bufctx(push), bin, (1 << 18) | (subc << 13) | mthd,
                       bo, 0, access | NOUVEAU_BO_OR, vor, tor)->priv = NULL;
   if (bo->flags & NOUVEAU_BO_VRAM)
      PUSH_DATA(push, vor);
   else
      PUSH_DATA(push, tor);
}

static inline void
PUSH_MTHDs(struct nouveau_pushbuf *push, int subc, int mthd, int bin,
      struct nouveau_bo *bo, uint32_t data, uint32_t access,
      uint32_t vor, uint32_t tor)
{
   nouveau_bufctx_mthd(bufctx(push), bin, (1 << 18) | (subc << 13) | mthd,
                       bo, data, access | NOUVEAU_BO_OR, vor, tor)->priv = NULL;
   if (bo->flags & NOUVEAU_BO_VRAM)
      PUSH_DATA(push, data | vor);
   else
      PUSH_DATA(push, data | tor);
}

static inline struct nouveau_bufref *
PUSH_MTHD(struct nouveau_pushbuf *push, int subc, int mthd, int bin,
     struct nouveau_bo *bo, uint32_t data, uint32_t access,
     uint32_t vor, uint32_t tor)
{
   struct nouveau_bufref *bref =
   nouveau_bufctx_mthd(bufctx(push), bin, (1 << 18) | (subc << 13) | mthd,
                       bo, data, access | NOUVEAU_BO_OR, vor, tor);
   if (access & NOUVEAU_BO_LOW)
      data += bo->offset;
   if (bo->flags & NOUVEAU_BO_VRAM)
      data |= vor;
   else
      data |= tor;
   PUSH_DATA(push, data);
   bref->priv = NULL;
   return bref;
}

static inline void
PUSH_RESRC(struct nouveau_pushbuf *push, int subc, int mthd, int bin,
           struct nv04_resource *r, uint32_t data, uint32_t access,
           uint32_t vor, uint32_t tor)
{
   PUSH_MTHD(push, subc, mthd, bin, r->bo, r->offset + data,
             r->domain | access, vor, tor)->priv = r;
}

/* subchannel assignment
 *
 * 0: <1.0.0 - used by kernel for m2mf
 *     1.0.0 - used by kernel for nvsw
 *
 * 1: <1.0.0 - used by kernel for nvsw
 *     1.0.0 - free for userspace
 *
 * 2-7: free for userspace on all kernel versions
 */

#define SUBC_M2MF(mthd)  2, (mthd)
#define NV03_M2MF(mthd)  SUBC_M2MF(NV03_M2MF_##mthd)

#define SUBC_SF2D(mthd)  3, (mthd)
#define NV04_SF2D(mthd)  SUBC_SF2D(NV04_SURFACE_2D_##mthd)

#define SUBC_SSWZ(mthd)  4, (mthd)
#define NV04_SSWZ(mthd)  SUBC_SSWZ(NV04_SURFACE_SWZ_##mthd)

#define SUBC_SIFM(mthd)  5, (mthd)
#define NV03_SIFM(mthd)  SUBC_SIFM(NV03_SIFM_##mthd)
#define NV05_SIFM(mthd)  SUBC_SIFM(NV05_SIFM_##mthd)

#define SUBC_3D(mthd)    7, (mthd)
#define NV30_3D(mthd)    SUBC_3D(NV30_3D_##mthd)
#define NV40_3D(mthd)    SUBC_3D(NV40_3D_##mthd)

#define NV01_SUBC(subc, mthd) SUBC_##subc((NV01_SUBCHAN_##mthd))
#define NV11_SUBC(subc, mthd) SUBC_##subc((NV11_SUBCHAN_##mthd))

#define NV04_GRAPH(subc, mthd) SUBC_##subc((NV04_GRAPH_##mthd))

#endif
