
#include "util/format/u_format.h"

#include "nvc0/nvc0_context.h"

struct nvc0_transfer {
   struct pipe_transfer base;
   struct nv50_m2mf_rect rect[2];
   uint32_t nblocksx;
   uint16_t nblocksy;
   uint16_t nlayers;
};

static void
nvc0_m2mf_transfer_rect(struct nvc0_context *nvc0,
                        const struct nv50_m2mf_rect *dst,
                        const struct nv50_m2mf_rect *src,
                        uint32_t nblocksx, uint32_t nblocksy)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nouveau_bufctx *bctx = nvc0->bufctx;
   const int cpp = dst->cpp;
   uint32_t src_ofst = src->base;
   uint32_t dst_ofst = dst->base;
   uint32_t height = nblocksy;
   uint32_t sy = src->y;
   uint32_t dy = dst->y;
   uint32_t exec = (1 << 20);

   assert(dst->cpp == src->cpp);

   nouveau_bufctx_refn(bctx, 0, src->bo, src->domain | NOUVEAU_BO_RD);
   nouveau_bufctx_refn(bctx, 0, dst->bo, dst->domain | NOUVEAU_BO_WR);
   nouveau_pushbuf_bufctx(push, bctx);
   PUSH_VAL(push);

   if (nouveau_bo_memtype(src->bo)) {
      BEGIN_NVC0(push, NVC0_M2MF(TILING_MODE_IN), 5);
      PUSH_DATA (push, src->tile_mode);
      PUSH_DATA (push, src->width * cpp);
      PUSH_DATA (push, src->height);
      PUSH_DATA (push, src->depth);
      PUSH_DATA (push, src->z);
   } else {
      src_ofst += src->y * src->pitch + src->x * cpp;

      BEGIN_NVC0(push, NVC0_M2MF(PITCH_IN), 1);
      PUSH_DATA (push, src->width * cpp);

      exec |= NVC0_M2MF_EXEC_LINEAR_IN;
   }

   if (nouveau_bo_memtype(dst->bo)) {
      BEGIN_NVC0(push, NVC0_M2MF(TILING_MODE_OUT), 5);
      PUSH_DATA (push, dst->tile_mode);
      PUSH_DATA (push, dst->width * cpp);
      PUSH_DATA (push, dst->height);
      PUSH_DATA (push, dst->depth);
      PUSH_DATA (push, dst->z);
   } else {
      dst_ofst += dst->y * dst->pitch + dst->x * cpp;

      BEGIN_NVC0(push, NVC0_M2MF(PITCH_OUT), 1);
      PUSH_DATA (push, dst->width * cpp);

      exec |= NVC0_M2MF_EXEC_LINEAR_OUT;
   }

   while (height) {
      int line_count = height > 2047 ? 2047 : height;

      BEGIN_NVC0(push, NVC0_M2MF(OFFSET_IN_HIGH), 2);
      PUSH_DATAh(push, src->bo->offset + src_ofst);
      PUSH_DATA (push, src->bo->offset + src_ofst);

      BEGIN_NVC0(push, NVC0_M2MF(OFFSET_OUT_HIGH), 2);
      PUSH_DATAh(push, dst->bo->offset + dst_ofst);
      PUSH_DATA (push, dst->bo->offset + dst_ofst);

      if (!(exec & NVC0_M2MF_EXEC_LINEAR_IN)) {
         BEGIN_NVC0(push, NVC0_M2MF(TILING_POSITION_IN_X), 2);
         PUSH_DATA (push, src->x * cpp);
         PUSH_DATA (push, sy);
      } else {
         src_ofst += line_count * src->pitch;
      }
      if (!(exec & NVC0_M2MF_EXEC_LINEAR_OUT)) {
         BEGIN_NVC0(push, NVC0_M2MF(TILING_POSITION_OUT_X), 2);
         PUSH_DATA (push, dst->x * cpp);
         PUSH_DATA (push, dy);
      } else {
         dst_ofst += line_count * dst->pitch;
      }

      BEGIN_NVC0(push, NVC0_M2MF(LINE_LENGTH_IN), 2);
      PUSH_DATA (push, nblocksx * cpp);
      PUSH_DATA (push, line_count);
      BEGIN_NVC0(push, NVC0_M2MF(EXEC), 1);
      PUSH_DATA (push, exec);

      height -= line_count;
      sy += line_count;
      dy += line_count;
   }

   nouveau_bufctx_reset(bctx, 0);
}

static void
nve4_m2mf_transfer_rect(struct nvc0_context *nvc0,
                        const struct nv50_m2mf_rect *dst,
                        const struct nv50_m2mf_rect *src,
                        uint32_t nblocksx, uint32_t nblocksy)
{
   static const struct {
      int cs;
      int nc;
   } cpbs[] = {
      [ 1] = { 1, 1 },
      [ 2] = { 1, 2 },
      [ 3] = { 1, 3 },
      [ 4] = { 1, 4 },
      [ 6] = { 2, 3 },
      [ 8] = { 2, 4 },
      [ 9] = { 3, 3 },
      [12] = { 3, 4 },
      [16] = { 4, 4 },
   };
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nouveau_bufctx *bctx = nvc0->bufctx;
   uint32_t exec;
   uint32_t src_base = src->base;
   uint32_t dst_base = dst->base;

   assert(dst->cpp < ARRAY_SIZE(cpbs) && cpbs[dst->cpp].cs);
   assert(dst->cpp == src->cpp);

   nouveau_bufctx_refn(bctx, 0, dst->bo, dst->domain | NOUVEAU_BO_WR);
   nouveau_bufctx_refn(bctx, 0, src->bo, src->domain | NOUVEAU_BO_RD);
   nouveau_pushbuf_bufctx(push, bctx);
   PUSH_VAL(push);

   exec = NVE4_COPY_EXEC_SWIZZLE_ENABLE | NVE4_COPY_EXEC_2D_ENABLE | NVE4_COPY_EXEC_FLUSH | NVE4_COPY_EXEC_COPY_MODE_NON_PIPELINED;

   BEGIN_NVC0(push, NVE4_COPY(SWIZZLE), 1);
   PUSH_DATA (push, (cpbs[dst->cpp].nc - 1) << 24 |
                    (cpbs[src->cpp].nc - 1) << 20 |
                    (cpbs[src->cpp].cs - 1) << 16 |
                    3 << 12 /* DST_W = SRC_W */ |
                    2 <<  8 /* DST_Z = SRC_Z */ |
                    1 <<  4 /* DST_Y = SRC_Y */ |
                    0 <<  0 /* DST_X = SRC_X */);

   if (nouveau_bo_memtype(dst->bo)) {
      BEGIN_NVC0(push, NVE4_COPY(DST_BLOCK_DIMENSIONS), 6);
      PUSH_DATA (push, dst->tile_mode | NVE4_COPY_SRC_BLOCK_DIMENSIONS_GOB_HEIGHT_FERMI_8);
      PUSH_DATA (push, dst->width);
      PUSH_DATA (push, dst->height);
      PUSH_DATA (push, dst->depth);
      PUSH_DATA (push, dst->z);
      PUSH_DATA (push, (dst->y << 16) | dst->x);
   } else {
      assert(!dst->z);
      dst_base += dst->y * dst->pitch + dst->x * dst->cpp;
      exec |= NVE4_COPY_EXEC_DST_LAYOUT_BLOCKLINEAR;
   }

   if (nouveau_bo_memtype(src->bo)) {
      BEGIN_NVC0(push, NVE4_COPY(SRC_BLOCK_DIMENSIONS), 6);
      PUSH_DATA (push, src->tile_mode | NVE4_COPY_SRC_BLOCK_DIMENSIONS_GOB_HEIGHT_FERMI_8);
      PUSH_DATA (push, src->width);
      PUSH_DATA (push, src->height);
      PUSH_DATA (push, src->depth);
      PUSH_DATA (push, src->z);
      PUSH_DATA (push, (src->y << 16) | src->x);
   } else {
      assert(!src->z);
      src_base += src->y * src->pitch + src->x * src->cpp;
      exec |= NVE4_COPY_EXEC_SRC_LAYOUT_BLOCKLINEAR;
   }

   BEGIN_NVC0(push, NVE4_COPY(SRC_ADDRESS_HIGH), 8);
   PUSH_DATAh(push, src->bo->offset + src_base);
   PUSH_DATA (push, src->bo->offset + src_base);
   PUSH_DATAh(push, dst->bo->offset + dst_base);
   PUSH_DATA (push, dst->bo->offset + dst_base);
   PUSH_DATA (push, src->pitch);
   PUSH_DATA (push, dst->pitch);
   PUSH_DATA (push, nblocksx);
   PUSH_DATA (push, nblocksy);

   BEGIN_NVC0(push, NVE4_COPY(EXEC), 1);
   PUSH_DATA (push, exec);

   nouveau_bufctx_reset(bctx, 0);
}

void
nvc0_m2mf_push_linear(struct nouveau_context *nv,
                      struct nouveau_bo *dst, unsigned offset, unsigned domain,
                      unsigned size, const void *data)
{
   struct nvc0_context *nvc0 = nvc0_context(&nv->pipe);
   struct nouveau_pushbuf *push = nv->pushbuf;
   uint32_t *src = (uint32_t *)data;
   unsigned count = (size + 3) / 4;

   nouveau_bufctx_refn(nvc0->bufctx, 0, dst, domain | NOUVEAU_BO_WR);
   nouveau_pushbuf_bufctx(push, nvc0->bufctx);
   PUSH_VAL(push);

   while (count) {
      unsigned nr = MIN2(count, NV04_PFIFO_MAX_PACKET_LEN);

      if (!PUSH_SPACE(push, nr + 9))
         break;

      BEGIN_NVC0(push, NVC0_M2MF(OFFSET_OUT_HIGH), 2);
      PUSH_DATAh(push, dst->offset + offset);
      PUSH_DATA (push, dst->offset + offset);
      BEGIN_NVC0(push, NVC0_M2MF(LINE_LENGTH_IN), 2);
      PUSH_DATA (push, MIN2(size, nr * 4));
      PUSH_DATA (push, 1);
      BEGIN_NVC0(push, NVC0_M2MF(EXEC), 1);
      PUSH_DATA (push, 0x100111);

      /* must not be interrupted (trap on QUERY fence, 0x50 works however) */
      BEGIN_NIC0(push, NVC0_M2MF(DATA), nr);
      PUSH_DATAp(push, src, nr);

      count -= nr;
      src += nr;
      offset += nr * 4;
      size -= nr * 4;
   }

   nouveau_bufctx_reset(nvc0->bufctx, 0);
}

void
nve4_p2mf_push_linear(struct nouveau_context *nv,
                      struct nouveau_bo *dst, unsigned offset, unsigned domain,
                      unsigned size, const void *data)
{
   struct nvc0_context *nvc0 = nvc0_context(&nv->pipe);
   struct nouveau_pushbuf *push = nv->pushbuf;
   uint32_t *src = (uint32_t *)data;
   unsigned count = (size + 3) / 4;

   nouveau_bufctx_refn(nvc0->bufctx, 0, dst, domain | NOUVEAU_BO_WR);
   nouveau_pushbuf_bufctx(push, nvc0->bufctx);
   PUSH_VAL(push);

   while (count) {
      unsigned nr = MIN2(count, (NV04_PFIFO_MAX_PACKET_LEN - 1));

      if (!PUSH_SPACE(push, nr + 10))
         break;

      BEGIN_NVC0(push, NVE4_P2MF(UPLOAD_DST_ADDRESS_HIGH), 2);
      PUSH_DATAh(push, dst->offset + offset);
      PUSH_DATA (push, dst->offset + offset);
      BEGIN_NVC0(push, NVE4_P2MF(UPLOAD_LINE_LENGTH_IN), 2);
      PUSH_DATA (push, MIN2(size, nr * 4));
      PUSH_DATA (push, 1);
      /* must not be interrupted (trap on QUERY fence, 0x50 works however) */
      BEGIN_1IC0(push, NVE4_P2MF(UPLOAD_EXEC), nr + 1);
      PUSH_DATA (push, 0x1001);
      PUSH_DATAp(push, src, nr);

      count -= nr;
      src += nr;
      offset += nr * 4;
      size -= nr * 4;
   }

   nouveau_bufctx_reset(nvc0->bufctx, 0);
}

static void
nvc0_m2mf_copy_linear(struct nouveau_context *nv,
                      struct nouveau_bo *dst, unsigned dstoff, unsigned dstdom,
                      struct nouveau_bo *src, unsigned srcoff, unsigned srcdom,
                      unsigned size)
{
   struct nouveau_pushbuf *push = nv->pushbuf;
   struct nouveau_bufctx *bctx = nvc0_context(&nv->pipe)->bufctx;

   nouveau_bufctx_refn(bctx, 0, src, srcdom | NOUVEAU_BO_RD);
   nouveau_bufctx_refn(bctx, 0, dst, dstdom | NOUVEAU_BO_WR);
   nouveau_pushbuf_bufctx(push, bctx);
   PUSH_VAL(push);

   while (size) {
      unsigned bytes = MIN2(size, 1 << 17);

      BEGIN_NVC0(push, NVC0_M2MF(OFFSET_OUT_HIGH), 2);
      PUSH_DATAh(push, dst->offset + dstoff);
      PUSH_DATA (push, dst->offset + dstoff);
      BEGIN_NVC0(push, NVC0_M2MF(OFFSET_IN_HIGH), 2);
      PUSH_DATAh(push, src->offset + srcoff);
      PUSH_DATA (push, src->offset + srcoff);
      BEGIN_NVC0(push, NVC0_M2MF(LINE_LENGTH_IN), 2);
      PUSH_DATA (push, bytes);
      PUSH_DATA (push, 1);
      BEGIN_NVC0(push, NVC0_M2MF(EXEC), 1);
      PUSH_DATA (push, NVC0_M2MF_EXEC_QUERY_SHORT |
                 NVC0_M2MF_EXEC_LINEAR_IN | NVC0_M2MF_EXEC_LINEAR_OUT);

      srcoff += bytes;
      dstoff += bytes;
      size -= bytes;
   }

   nouveau_bufctx_reset(bctx, 0);
}

static void
nve4_m2mf_copy_linear(struct nouveau_context *nv,
                      struct nouveau_bo *dst, unsigned dstoff, unsigned dstdom,
                      struct nouveau_bo *src, unsigned srcoff, unsigned srcdom,
                      unsigned size)
{
   struct nouveau_pushbuf *push = nv->pushbuf;
   struct nouveau_bufctx *bctx = nvc0_context(&nv->pipe)->bufctx;

   nouveau_bufctx_refn(bctx, 0, src, srcdom | NOUVEAU_BO_RD);
   nouveau_bufctx_refn(bctx, 0, dst, dstdom | NOUVEAU_BO_WR);
   nouveau_pushbuf_bufctx(push, bctx);
   PUSH_VAL(push);

   BEGIN_NVC0(push, NVE4_COPY(SRC_ADDRESS_HIGH), 4);
   PUSH_DATAh(push, src->offset + srcoff);
   PUSH_DATA (push, src->offset + srcoff);
   PUSH_DATAh(push, dst->offset + dstoff);
   PUSH_DATA (push, dst->offset + dstoff);
   BEGIN_NVC0(push, NVE4_COPY(X_COUNT), 1);
   PUSH_DATA (push, size);
   BEGIN_NVC0(push, NVE4_COPY(EXEC), 1);
   PUSH_DATA (push, NVE4_COPY_EXEC_COPY_MODE_NON_PIPELINED |
		    NVE4_COPY_EXEC_FLUSH |
		    NVE4_COPY_EXEC_SRC_LAYOUT_BLOCKLINEAR |
		    NVE4_COPY_EXEC_DST_LAYOUT_BLOCKLINEAR);

   nouveau_bufctx_reset(bctx, 0);
}


static inline bool
nvc0_mt_transfer_can_map_directly(struct nv50_miptree *mt)
{
   if (mt->base.domain == NOUVEAU_BO_VRAM)
      return false;
   if (mt->base.base.usage != PIPE_USAGE_STAGING)
      return false;
   return !nouveau_bo_memtype(mt->base.bo);
}

static inline bool
nvc0_mt_sync(struct nvc0_context *nvc0, struct nv50_miptree *mt, unsigned usage)
{
   if (!mt->base.mm) {
      uint32_t access = (usage & PIPE_MAP_WRITE) ?
         NOUVEAU_BO_WR : NOUVEAU_BO_RD;
      return !BO_WAIT(&nvc0->screen->base, mt->base.bo, access, nvc0->base.client);
   }
   if (usage & PIPE_MAP_WRITE)
      return !mt->base.fence || nouveau_fence_wait(mt->base.fence, &nvc0->base.debug);
   return !mt->base.fence_wr || nouveau_fence_wait(mt->base.fence_wr, &nvc0->base.debug);
}

void *
nvc0_miptree_transfer_map(struct pipe_context *pctx,
                          struct pipe_resource *res,
                          unsigned level,
                          unsigned usage,
                          const struct pipe_box *box,
                          struct pipe_transfer **ptransfer)
{
   struct nvc0_context *nvc0 = nvc0_context(pctx);
   struct nouveau_device *dev = nvc0->screen->base.device;
   struct nv50_miptree *mt = nv50_miptree(res);
   struct nvc0_transfer *tx;
   uint32_t size;
   int ret;
   unsigned flags = 0;

   if (nvc0_mt_transfer_can_map_directly(mt)) {
      ret = !nvc0_mt_sync(nvc0, mt, usage);
      if (!ret)
         ret = BO_MAP(nvc0->base.screen, mt->base.bo, 0, NULL);
      if (ret &&
          (usage & PIPE_MAP_DIRECTLY))
         return NULL;
      if (!ret)
         usage |= PIPE_MAP_DIRECTLY;
   } else
   if (usage & PIPE_MAP_DIRECTLY)
      return NULL;

   tx = CALLOC_STRUCT(nvc0_transfer);
   if (!tx)
      return NULL;

   pipe_resource_reference(&tx->base.resource, res);

   tx->base.level = level;
   tx->base.usage = usage;
   tx->base.box = *box;

   if (util_format_is_plain(res->format)) {
      tx->nblocksx = box->width << mt->ms_x;
      tx->nblocksy = box->height << mt->ms_y;
   } else {
      tx->nblocksx = util_format_get_nblocksx(res->format, box->width);
      tx->nblocksy = util_format_get_nblocksy(res->format, box->height);
   }
   tx->nlayers = box->depth;

   if (usage & PIPE_MAP_DIRECTLY) {
      tx->base.stride = mt->level[level].pitch;
      tx->base.layer_stride = mt->layer_stride;
      uint32_t offset = box->y * tx->base.stride +
         util_format_get_stride(res->format, box->x);
      if (!mt->layout_3d)
         offset += mt->layer_stride * box->z;
      else
         offset += nvc0_mt_zslice_offset(mt, level, box->z);
      *ptransfer = &tx->base;
      return mt->base.bo->map + mt->base.offset + offset;
   }

   tx->base.stride = tx->nblocksx * util_format_get_blocksize(res->format);
   tx->base.layer_stride = tx->nblocksy * tx->base.stride;

   nv50_m2mf_rect_setup(&tx->rect[0], res, level, box->x, box->y, box->z);

   size = tx->base.layer_stride;

   ret = nouveau_bo_new(dev, NOUVEAU_BO_GART | NOUVEAU_BO_MAP, 0,
                        size * tx->nlayers, NULL, &tx->rect[1].bo);
   if (ret) {
      pipe_resource_reference(&tx->base.resource, NULL);
      FREE(tx);
      return NULL;
   }

   tx->rect[1].cpp = tx->rect[0].cpp;
   tx->rect[1].width = tx->nblocksx;
   tx->rect[1].height = tx->nblocksy;
   tx->rect[1].depth = 1;
   tx->rect[1].pitch = tx->base.stride;
   tx->rect[1].domain = NOUVEAU_BO_GART;

   if (usage & PIPE_MAP_READ) {
      unsigned base = tx->rect[0].base;
      unsigned z = tx->rect[0].z;
      unsigned i;
      for (i = 0; i < tx->nlayers; ++i) {
         nvc0->m2mf_copy_rect(nvc0, &tx->rect[1], &tx->rect[0],
                              tx->nblocksx, tx->nblocksy);
         if (mt->layout_3d)
            tx->rect[0].z++;
         else
            tx->rect[0].base += mt->layer_stride;
         tx->rect[1].base += size;
      }
      tx->rect[0].z = z;
      tx->rect[0].base = base;
      tx->rect[1].base = 0;
   }

   if (tx->rect[1].bo->map) {
      *ptransfer = &tx->base;
      return tx->rect[1].bo->map;
   }

   if (usage & PIPE_MAP_READ)
      flags = NOUVEAU_BO_RD;
   if (usage & PIPE_MAP_WRITE)
      flags |= NOUVEAU_BO_WR;

   ret = BO_MAP(nvc0->base.screen, tx->rect[1].bo, flags, nvc0->base.client);
   if (ret) {
      pipe_resource_reference(&tx->base.resource, NULL);
      nouveau_bo_ref(NULL, &tx->rect[1].bo);
      FREE(tx);
      return NULL;
   }

   *ptransfer = &tx->base;
   return tx->rect[1].bo->map;
}

void
nvc0_miptree_transfer_unmap(struct pipe_context *pctx,
                            struct pipe_transfer *transfer)
{
   struct nvc0_context *nvc0 = nvc0_context(pctx);
   struct nvc0_transfer *tx = (struct nvc0_transfer *)transfer;
   struct nv50_miptree *mt = nv50_miptree(tx->base.resource);
   unsigned i;

   if (tx->base.usage & PIPE_MAP_DIRECTLY) {
      pipe_resource_reference(&transfer->resource, NULL);

      FREE(tx);
      return;
   }

   if (tx->base.usage & PIPE_MAP_WRITE) {
      for (i = 0; i < tx->nlayers; ++i) {
         nvc0->m2mf_copy_rect(nvc0, &tx->rect[0], &tx->rect[1],
                              tx->nblocksx, tx->nblocksy);
         if (mt->layout_3d)
            tx->rect[0].z++;
         else
            tx->rect[0].base += mt->layer_stride;
         tx->rect[1].base += tx->nblocksy * tx->base.stride;
      }
      NOUVEAU_DRV_STAT(&nvc0->screen->base, tex_transfers_wr, 1);

      /* Allow the copies above to finish executing before freeing the source */
      nouveau_fence_work(nvc0->base.fence,
                         nouveau_fence_unref_bo, tx->rect[1].bo);
   } else {
      nouveau_bo_ref(NULL, &tx->rect[1].bo);
   }
   if (tx->base.usage & PIPE_MAP_READ)
      NOUVEAU_DRV_STAT(&nvc0->screen->base, tex_transfers_rd, 1);

   pipe_resource_reference(&transfer->resource, NULL);

   FREE(tx);
}

/* This happens rather often with DTD9/st. */
static void
nvc0_cb_push(struct nouveau_context *nv,
             struct nv04_resource *res,
             unsigned offset, unsigned words, const uint32_t *data)
{
   struct nvc0_context *nvc0 = nvc0_context(&nv->pipe);
   struct nvc0_constbuf *cb = NULL;
   int s;

   /* Go through all the constbuf binding points of this buffer and try to
    * find one which contains the region to be updated.
    */
   for (s = 0; s < 6 && !cb; s++) {
      uint16_t bindings = res->cb_bindings[s];
      while (bindings) {
         int i = ffs(bindings) - 1;
         uint32_t cb_offset = nvc0->constbuf[s][i].offset;

         bindings &= ~(1 << i);
         if (cb_offset <= offset &&
             cb_offset + nvc0->constbuf[s][i].size >= offset + words * 4) {
            cb = &nvc0->constbuf[s][i];
            break;
         }
      }
   }

   if (cb) {
      nvc0_cb_bo_push(nv, res->bo, res->domain,
                      res->offset + cb->offset, cb->size,
                      offset - cb->offset, words, data);
   } else {
      nv->push_data(nv, res->bo, res->offset + offset, res->domain,
                    words * 4, data);
   }
}

void
nvc0_cb_bo_push(struct nouveau_context *nv,
                struct nouveau_bo *bo, unsigned domain,
                unsigned base, unsigned size,
                unsigned offset, unsigned words, const uint32_t *data)
{
   struct nouveau_pushbuf *push = nv->pushbuf;

   NOUVEAU_DRV_STAT(nv->screen, constbuf_upload_count, 1);
   NOUVEAU_DRV_STAT(nv->screen, constbuf_upload_bytes, words * 4);

   assert(!(offset & 3));
   size = align(size, 0x100);

   assert(offset < size);
   assert(offset + words * 4 <= size);

   BEGIN_NVC0(push, NVC0_3D(CB_SIZE), 3);
   PUSH_DATA (push, size);
   PUSH_DATAh(push, bo->offset + base);
   PUSH_DATA (push, bo->offset + base);

   while (words) {
      unsigned nr = MIN2(words, NV04_PFIFO_MAX_PACKET_LEN - 1);

      PUSH_SPACE(push, nr + 2);
      PUSH_REF1 (push, bo, NOUVEAU_BO_WR | domain);
      BEGIN_1IC0(push, NVC0_3D(CB_POS), nr + 1);
      PUSH_DATA (push, offset);
      PUSH_DATAp(push, data, nr);

      words -= nr;
      data += nr;
      offset += nr * 4;
   }
}

void
nvc0_init_transfer_functions(struct nvc0_context *nvc0)
{
   if (nvc0->screen->base.class_3d >= NVE4_3D_CLASS) {
      nvc0->m2mf_copy_rect = nve4_m2mf_transfer_rect;
      nvc0->base.copy_data = nve4_m2mf_copy_linear;
      nvc0->base.push_data = nve4_p2mf_push_linear;
   } else {
      nvc0->m2mf_copy_rect = nvc0_m2mf_transfer_rect;
      nvc0->base.copy_data = nvc0_m2mf_copy_linear;
      nvc0->base.push_data = nvc0_m2mf_push_linear;
   }
   nvc0->base.push_cb = nvc0_cb_push;
}
