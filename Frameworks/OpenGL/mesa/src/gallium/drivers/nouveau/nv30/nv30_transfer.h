#ifndef __NV30_TRANSFER_H__
#define __NV30_TRANSFER_H__

struct nv30_rect {
   struct nouveau_bo *bo;
   unsigned offset;
   unsigned domain;
   unsigned pitch;
   unsigned cpp;
   unsigned w;
   unsigned h;
   unsigned d;
   unsigned z;
   unsigned x0;
   unsigned x1;
   unsigned y0;
   unsigned y1;
};

enum nv30_transfer_filter {
   NEAREST = 0,
   BILINEAR
};

void
nv30_transfer_rect(struct nv30_context *, enum nv30_transfer_filter filter,
                   struct nv30_rect *, struct nv30_rect *);

void
nv30_transfer_push_data(struct nouveau_context *,
                        struct nouveau_bo *, unsigned offset, unsigned domain,
                        unsigned size, void *data);

void
nv30_transfer_copy_data(struct nouveau_context *,
                        struct nouveau_bo *, unsigned dstoff, unsigned dstdom,
                        struct nouveau_bo *, unsigned srcoff, unsigned srcdom,
                        unsigned size);

#endif
