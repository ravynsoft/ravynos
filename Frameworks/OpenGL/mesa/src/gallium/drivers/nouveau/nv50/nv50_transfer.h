
#ifndef __NV50_TRANSFER_H__
#define __NV50_TRANSFER_H__

#include "pipe/p_state.h"

struct nv50_m2mf_rect {
   struct nouveau_bo *bo;
   uint32_t base;
   unsigned domain;
   uint32_t pitch;
   uint32_t width;
   uint32_t x;
   uint32_t height;
   uint32_t y;
   uint16_t depth;
   uint16_t z;
   uint16_t tile_mode;
   uint16_t cpp;
};

void
nv50_m2mf_rect_setup(struct nv50_m2mf_rect *rect,
                     struct pipe_resource *restrict res, unsigned l,
                     unsigned x, unsigned y, unsigned z);

#endif
