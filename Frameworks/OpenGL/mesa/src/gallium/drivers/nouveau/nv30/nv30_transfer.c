/*
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 *
 */

#define XFER_ARGS                                                              \
   struct nv30_context *nv30, enum nv30_transfer_filter filter,                \
   struct nv30_rect *src, struct nv30_rect *dst

#include "util/u_math.h"

#include "nv_object.xml.h"
#include "nv_m2mf.xml.h"
#include "nv30/nv01_2d.xml.h"
#include "nv30/nv30-40_3d.xml.h"

#include "nv30/nv30_context.h"
#include "nv30/nv30_transfer.h"
#include "nv30/nv30_winsys.h"

/* Various helper functions to transfer different types of data in a number
 * of different ways.
 */

static inline bool
nv30_transfer_scaled(struct nv30_rect *src, struct nv30_rect *dst)
{
   if (src->x1 - src->x0 != dst->x1 - dst->x0)
      return true;
   if (src->y1 - src->y0 != dst->y1 - dst->y0)
      return true;
   return false;
}

static inline bool
nv30_transfer_blit(XFER_ARGS)
{
   if (nv30->screen->eng3d->oclass < NV40_3D_CLASS)
      return false;
   if (dst->offset & 63 || dst->pitch & 63 || dst->d > 1)
      return false;
   if (dst->w < 2 || dst->h < 2)
      return false;
   if (dst->cpp > 4 || (dst->cpp == 1 && !dst->pitch))
      return false;
   if (src->cpp > 4)
      return false;
   return true;
}

static inline struct nouveau_heap *
nv30_transfer_rect_vertprog(struct nv30_context *nv30)
{
   struct nouveau_heap *heap = nv30->screen->vp_exec_heap;
   struct nouveau_heap *vp;

   vp = nv30->blit_vp;
   if (!vp) {
      if (nouveau_heap_alloc(heap, 2, &nv30->blit_vp, &nv30->blit_vp)) {
         while (heap->next && heap->size < 2) {
            struct nouveau_heap **evict = heap->next->priv;
            nouveau_heap_free(evict);
         }

         if (nouveau_heap_alloc(heap, 2, &nv30->blit_vp, &nv30->blit_vp))
            return NULL;
      }

      vp = nv30->blit_vp;
      if (vp) {
         struct nouveau_pushbuf *push = nv30->base.pushbuf;

         BEGIN_NV04(push, NV30_3D(VP_UPLOAD_FROM_ID), 1);
         PUSH_DATA (push, vp->start);
         BEGIN_NV04(push, NV30_3D(VP_UPLOAD_INST(0)), 4);
         PUSH_DATA (push, 0x401f9c6c); /* mov o[hpos], a[0]; */
         PUSH_DATA (push, 0x0040000d);
         PUSH_DATA (push, 0x8106c083);
         PUSH_DATA (push, 0x6041ff80);
         BEGIN_NV04(push, NV30_3D(VP_UPLOAD_INST(0)), 4);
         PUSH_DATA (push, 0x401f9c6c); /* mov o[tex0], a[8]; end; */
         PUSH_DATA (push, 0x0040080d);
         PUSH_DATA (push, 0x8106c083);
         PUSH_DATA (push, 0x6041ff9d);
      }
   }

   return vp;
}


static inline struct nv04_resource *
nv30_transfer_rect_fragprog(struct nv30_context *nv30)
{
   struct nv04_resource *fp = nv04_resource(nv30->blit_fp);
   struct pipe_context *pipe = &nv30->base.pipe;

   if (!fp) {
      nv30->blit_fp =
         pipe_buffer_create(pipe->screen, 0, PIPE_USAGE_STAGING, 12 * 4);
      if (nv30->blit_fp) {
         struct pipe_transfer *transfer;
         u32 *map = pipe_buffer_map(pipe, nv30->blit_fp,
                                    PIPE_MAP_WRITE, &transfer);
         if (map) {
            map[0] = 0x17009e00; /* texr r0, i[tex0], texture[0]; end; */
            map[1] = 0x1c9dc801;
            map[2] = 0x0001c800;
            map[3] = 0x3fe1c800;
            map[4] = 0x01401e81; /* end; */
            map[5] = 0x1c9dc800;
            map[6] = 0x0001c800;
            map[7] = 0x0001c800;
            pipe_buffer_unmap(pipe, transfer);
         }

         fp = nv04_resource(nv30->blit_fp);
         nouveau_buffer_migrate(&nv30->base, fp, NOUVEAU_BO_VRAM);
      }
   }

   return fp;
}

static void
nv30_transfer_rect_blit(XFER_ARGS)
{
   struct nv04_resource *fp = nv30_transfer_rect_fragprog(nv30);
   struct nouveau_heap *vp = nv30_transfer_rect_vertprog(nv30);
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct nouveau_pushbuf_refn refs[] = {
      { fp->bo, fp->domain | NOUVEAU_BO_RD },
      { src->bo, src->domain | NOUVEAU_BO_RD },
      { dst->bo, NOUVEAU_BO_VRAM | NOUVEAU_BO_WR },
   };
   u32 texfmt, texswz;
   u32 format, stride;

   if (!PUSH_SPACE_EX(push, 512, 8, 0) ||
       PUSH_REFN(push, refs, ARRAY_SIZE(refs)))
      return;

   /* various switches depending on cpp of the transfer */
   switch (dst->cpp) {
   case 4:
      format = NV30_3D_RT_FORMAT_COLOR_A8R8G8B8 |
               NV30_3D_RT_FORMAT_ZETA_Z24S8;
      texfmt = NV40_3D_TEX_FORMAT_FORMAT_A8R8G8B8;
      texswz = 0x0000aae4;
      break;
   case 2:
      format = NV30_3D_RT_FORMAT_COLOR_R5G6B5 |
               NV30_3D_RT_FORMAT_ZETA_Z16;
      texfmt = NV40_3D_TEX_FORMAT_FORMAT_R5G6B5;
      texswz = 0x0000a9e4;
      break;
   case 1:
      format = NV30_3D_RT_FORMAT_COLOR_B8 |
               NV30_3D_RT_FORMAT_ZETA_Z16;
      texfmt = NV40_3D_TEX_FORMAT_FORMAT_L8;
      texswz = 0x0000aaff;
      break;
   default:
      assert(0);
      return;
   }

   /* render target */
   if (!dst->pitch) {
      format |= NV30_3D_RT_FORMAT_TYPE_SWIZZLED;
      format |= util_logbase2(dst->w) << 16;
      format |= util_logbase2(dst->h) << 24;
      stride  = 64;
   } else {
      format |= NV30_3D_RT_FORMAT_TYPE_LINEAR;
      stride  = dst->pitch;
   }

   BEGIN_NV04(push, NV30_3D(VIEWPORT_HORIZ), 2);
   PUSH_DATA (push, dst->w << 16);
   PUSH_DATA (push, dst->h << 16);
   BEGIN_NV04(push, NV30_3D(RT_HORIZ), 5);
   PUSH_DATA (push, dst->w << 16);
   PUSH_DATA (push, dst->h << 16);
   PUSH_DATA (push, format);
   PUSH_DATA (push, stride);
   PUSH_RELOC(push, dst->bo, dst->offset, NOUVEAU_BO_LOW, 0, 0);
   BEGIN_NV04(push, NV30_3D(RT_ENABLE), 1);
   PUSH_DATA (push, NV30_3D_RT_ENABLE_COLOR0);

   nv30->dirty |= NV30_NEW_FRAMEBUFFER;

   /* viewport state */
   BEGIN_NV04(push, NV30_3D(VIEWPORT_TRANSLATE_X), 8);
   PUSH_DATAf(push, 0.0);
   PUSH_DATAf(push, 0.0);
   PUSH_DATAf(push, 0.0);
   PUSH_DATAf(push, 0.0);
   PUSH_DATAf(push, 1.0);
   PUSH_DATAf(push, 1.0);
   PUSH_DATAf(push, 1.0);
   PUSH_DATAf(push, 1.0);
   BEGIN_NV04(push, NV30_3D(DEPTH_RANGE_NEAR), 2);
   PUSH_DATAf(push, 0.0);
   PUSH_DATAf(push, 1.0);

   nv30->dirty |= NV30_NEW_VIEWPORT;

   /* blend state */
   BEGIN_NV04(push, NV30_3D(COLOR_LOGIC_OP_ENABLE), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV30_3D(DITHER_ENABLE), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV30_3D(BLEND_FUNC_ENABLE), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV30_3D(COLOR_MASK), 1);
   PUSH_DATA (push, 0x01010101);

   nv30->dirty |= NV30_NEW_BLEND;

   /* depth-stencil-alpha state */
   BEGIN_NV04(push, NV30_3D(DEPTH_WRITE_ENABLE), 2);
   PUSH_DATA (push, 0);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV30_3D(STENCIL_ENABLE(0)), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV30_3D(STENCIL_ENABLE(1)), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV30_3D(ALPHA_FUNC_ENABLE), 1);
   PUSH_DATA (push, 0);

   nv30->dirty |= NV30_NEW_ZSA;

   /* rasterizer state */
   BEGIN_NV04(push, NV30_3D(SHADE_MODEL), 1);
   PUSH_DATA (push, NV30_3D_SHADE_MODEL_FLAT);
   BEGIN_NV04(push, NV30_3D(CULL_FACE_ENABLE), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV30_3D(POLYGON_MODE_FRONT), 2);
   PUSH_DATA (push, NV30_3D_POLYGON_MODE_FRONT_FILL);
   PUSH_DATA (push, NV30_3D_POLYGON_MODE_BACK_FILL);
   BEGIN_NV04(push, NV30_3D(POLYGON_OFFSET_FILL_ENABLE), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV30_3D(POLYGON_STIPPLE_ENABLE), 1);
   PUSH_DATA (push, 0);

   nv30->state.scissor_off = 0;
   nv30->dirty |= NV30_NEW_RASTERIZER;

   /* vertex program */
   BEGIN_NV04(push, NV30_3D(VP_START_FROM_ID), 1);
   PUSH_DATA (push, vp->start);
   BEGIN_NV04(push, NV40_3D(VP_ATTRIB_EN), 2);
   PUSH_DATA (push, 0x00000101); /* attrib: 0, 8 */
   PUSH_DATA (push, 0x00004000); /* result: hpos, tex0 */
   BEGIN_NV04(push, NV30_3D(ENGINE), 1);
   PUSH_DATA (push, 0x00000103);
   BEGIN_NV04(push, NV30_3D(VP_CLIP_PLANES_ENABLE), 1);
   PUSH_DATA (push, 0x00000000);

   nv30->dirty |= NV30_NEW_VERTPROG;
   nv30->dirty |= NV30_NEW_CLIP;

   /* fragment program */
   BEGIN_NV04(push, NV30_3D(FP_ACTIVE_PROGRAM), 1);
   PUSH_RELOC(push, fp->bo, fp->offset, fp->domain |
                    NOUVEAU_BO_LOW | NOUVEAU_BO_OR,
                    NV30_3D_FP_ACTIVE_PROGRAM_DMA0,
                    NV30_3D_FP_ACTIVE_PROGRAM_DMA1);
   BEGIN_NV04(push, NV30_3D(FP_CONTROL), 1);
   PUSH_DATA (push, 0x02000000);

   nv30->state.fragprog = NULL;
   nv30->dirty |= NV30_NEW_FRAGPROG;

   /* texture */
   texfmt |= 1 << NV40_3D_TEX_FORMAT_MIPMAP_COUNT__SHIFT;
   texfmt |= NV30_3D_TEX_FORMAT_NO_BORDER;
   texfmt |= NV40_3D_TEX_FORMAT_RECT;
   texfmt |= 0x00008000;
   if (src->d < 2)
      texfmt |= NV30_3D_TEX_FORMAT_DIMS_2D;
   else
      texfmt |= NV30_3D_TEX_FORMAT_DIMS_3D;
   if (src->pitch)
      texfmt |= NV40_3D_TEX_FORMAT_LINEAR;

   BEGIN_NV04(push, NV30_3D(TEX_OFFSET(0)), 8);
   PUSH_RELOC(push, src->bo, src->offset, NOUVEAU_BO_LOW, 0, 0);
   PUSH_RELOC(push, src->bo, texfmt, NOUVEAU_BO_OR,
                    NV30_3D_TEX_FORMAT_DMA0, NV30_3D_TEX_FORMAT_DMA1);
   PUSH_DATA (push, NV30_3D_TEX_WRAP_S_CLAMP_TO_EDGE |
                    NV30_3D_TEX_WRAP_T_CLAMP_TO_EDGE |
                    NV30_3D_TEX_WRAP_R_CLAMP_TO_EDGE);
   PUSH_DATA (push, NV40_3D_TEX_ENABLE_ENABLE);
   PUSH_DATA (push, texswz);
   switch (filter) {
   case BILINEAR:
      PUSH_DATA (push, NV30_3D_TEX_FILTER_MIN_LINEAR |
                       NV30_3D_TEX_FILTER_MAG_LINEAR | 0x00002000);
      break;
   default:
      PUSH_DATA (push, NV30_3D_TEX_FILTER_MIN_NEAREST |
                       NV30_3D_TEX_FILTER_MAG_NEAREST | 0x00002000);
      break;
   }
   PUSH_DATA (push, (src->w << 16) | src->h);
   PUSH_DATA (push, 0x00000000);
   BEGIN_NV04(push, NV40_3D(TEX_SIZE1(0)), 1);
   PUSH_DATA (push, 0x00100000 | src->pitch);
   BEGIN_NV04(push, SUBC_3D(0x0b40), 1);
   PUSH_DATA (push, src->d < 2 ? 0x00000001 : 0x00000000);
   BEGIN_NV04(push, NV40_3D(TEX_CACHE_CTL), 1);
   PUSH_DATA (push, 1);

   nv30->fragprog.dirty_samplers |= 1;
   nv30->dirty |= NV30_NEW_FRAGTEX;

   /* blit! */
   BEGIN_NV04(push, NV30_3D(SCISSOR_HORIZ), 2);
   PUSH_DATA (push, (dst->x1 - dst->x0) << 16 | dst->x0);
   PUSH_DATA (push, (dst->y1 - dst->y0) << 16 | dst->y0);
   BEGIN_NV04(push, NV30_3D(VERTEX_BEGIN_END), 1);
   PUSH_DATA (push, NV30_3D_VERTEX_BEGIN_END_QUADS);
   BEGIN_NV04(push, NV30_3D(VTX_ATTR_3F(8)), 3);
   PUSH_DATAf(push, src->x0);
   PUSH_DATAf(push, src->y0);
   PUSH_DATAf(push, src->z);
   BEGIN_NV04(push, NV30_3D(VTX_ATTR_2I(0)), 1);
   PUSH_DATA (push, (dst->y0 << 16) | dst->x0);
   BEGIN_NV04(push, NV30_3D(VTX_ATTR_3F(8)), 3);
   PUSH_DATAf(push, src->x1);
   PUSH_DATAf(push, src->y0);
   PUSH_DATAf(push, src->z);
   BEGIN_NV04(push, NV30_3D(VTX_ATTR_2I(0)), 1);
   PUSH_DATA (push, (dst->y0 << 16) | dst->x1);
   BEGIN_NV04(push, NV30_3D(VTX_ATTR_3F(8)), 3);
   PUSH_DATAf(push, src->x1);
   PUSH_DATAf(push, src->y1);
   PUSH_DATAf(push, src->z);
   BEGIN_NV04(push, NV30_3D(VTX_ATTR_2I(0)), 1);
   PUSH_DATA (push, (dst->y1 << 16) | dst->x1);
   BEGIN_NV04(push, NV30_3D(VTX_ATTR_3F(8)), 3);
   PUSH_DATAf(push, src->x0);
   PUSH_DATAf(push, src->y1);
   PUSH_DATAf(push, src->z);
   BEGIN_NV04(push, NV30_3D(VTX_ATTR_2I(0)), 1);
   PUSH_DATA (push, (dst->y1 << 16) | dst->x0);
   BEGIN_NV04(push, NV30_3D(VERTEX_BEGIN_END), 1);
   PUSH_DATA (push, NV30_3D_VERTEX_BEGIN_END_STOP);
}

static bool
nv30_transfer_sifm(XFER_ARGS)
{
   if (!src->pitch || src->w > 1024 || src->h > 1024 || src->w < 2 || src->h < 2)
      return false;

   if (src->d > 1 || dst->d > 1)
      return false;

   if (dst->offset & 63)
      return false;

   if (!dst->pitch) {
      if (dst->w > 2048 || dst->h > 2048 || dst->w < 2 || dst->h < 2)
         return false;
   } else {
      if (dst->domain != NOUVEAU_BO_VRAM)
         return false;
      if (dst->pitch & 63)
         return false;
   }

   return true;
}

static void
nv30_transfer_rect_sifm(XFER_ARGS)

{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct nouveau_pushbuf_refn refs[] = {
      { src->bo, src->domain | NOUVEAU_BO_RD },
      { dst->bo, dst->domain | NOUVEAU_BO_WR },
   };
   struct nv04_fifo *fifo = push->channel->data;
   unsigned si_fmt, si_arg;
   unsigned ss_fmt;

   switch (dst->cpp) {
   case 4: ss_fmt = NV04_SURFACE_SWZ_FORMAT_COLOR_A8R8G8B8; break;
   case 2: ss_fmt = NV04_SURFACE_SWZ_FORMAT_COLOR_R5G6B5; break;
   default:
      ss_fmt = NV04_SURFACE_SWZ_FORMAT_COLOR_Y8;
      break;
   }

   switch (src->cpp) {
   case 4: si_fmt = NV03_SIFM_COLOR_FORMAT_A8R8G8B8; break;
   case 2: si_fmt = NV03_SIFM_COLOR_FORMAT_R5G6B5; break;
   default:
      si_fmt = NV03_SIFM_COLOR_FORMAT_AY8;
      break;
   }

   if (filter == NEAREST) {
      si_arg  = NV03_SIFM_FORMAT_ORIGIN_CENTER;
      si_arg |= NV03_SIFM_FORMAT_FILTER_POINT_SAMPLE;
   } else {
      si_arg  = NV03_SIFM_FORMAT_ORIGIN_CORNER;
      si_arg |= NV03_SIFM_FORMAT_FILTER_BILINEAR;
   }

   if (!PUSH_SPACE_EX(push, 64, 6, 0) ||
       PUSH_REFN(push, refs, 2))
      return;

   if (dst->pitch) {
      BEGIN_NV04(push, NV04_SF2D(DMA_IMAGE_SOURCE), 2);
      PUSH_RELOC(push, dst->bo, 0, NOUVEAU_BO_OR, fifo->vram, fifo->gart);
      PUSH_RELOC(push, dst->bo, 0, NOUVEAU_BO_OR, fifo->vram, fifo->gart);
      BEGIN_NV04(push, NV04_SF2D(FORMAT), 4);
      PUSH_DATA (push, ss_fmt);
      PUSH_DATA (push, dst->pitch << 16 | dst->pitch);
      PUSH_RELOC(push, dst->bo, dst->offset, NOUVEAU_BO_LOW, 0, 0);
      PUSH_RELOC(push, dst->bo, dst->offset, NOUVEAU_BO_LOW, 0, 0);
      BEGIN_NV04(push, NV05_SIFM(SURFACE), 1);
      PUSH_DATA (push, nv30->screen->surf2d->handle);
   } else {
      BEGIN_NV04(push, NV04_SSWZ(DMA_IMAGE), 1);
      PUSH_RELOC(push, dst->bo, 0, NOUVEAU_BO_OR, fifo->vram, fifo->gart);
      BEGIN_NV04(push, NV04_SSWZ(FORMAT), 2);
      PUSH_DATA (push, ss_fmt | (util_logbase2(dst->w) << 16) |
                                (util_logbase2(dst->h) << 24));
      PUSH_RELOC(push, dst->bo, dst->offset, NOUVEAU_BO_LOW, 0, 0);
      BEGIN_NV04(push, NV05_SIFM(SURFACE), 1);
      PUSH_DATA (push, nv30->screen->swzsurf->handle);
   }

   BEGIN_NV04(push, NV03_SIFM(DMA_IMAGE), 1);
   PUSH_RELOC(push, src->bo, 0, NOUVEAU_BO_OR, fifo->vram, fifo->gart);
   BEGIN_NV04(push, NV03_SIFM(COLOR_FORMAT), 8);
   PUSH_DATA (push, si_fmt);
   PUSH_DATA (push, NV03_SIFM_OPERATION_SRCCOPY);
   PUSH_DATA (push, (           dst->y0  << 16) |            dst->x0);
   PUSH_DATA (push, ((dst->y1 - dst->y0) << 16) | (dst->x1 - dst->x0));
   PUSH_DATA (push, (           dst->y0  << 16) |            dst->x0);
   PUSH_DATA (push, ((dst->y1 - dst->y0) << 16) | (dst->x1 - dst->x0));
   PUSH_DATA (push, ((src->x1 - src->x0) << 20) / (dst->x1 - dst->x0));
   PUSH_DATA (push, ((src->y1 - src->y0) << 20) / (dst->y1 - dst->y0));
   BEGIN_NV04(push, NV03_SIFM(SIZE), 4);
   PUSH_DATA (push, align(src->h, 2) << 16 | align(src->w, 2));
   PUSH_DATA (push, src->pitch | si_arg);
   PUSH_RELOC(push, src->bo, src->offset, NOUVEAU_BO_LOW, 0, 0);
   PUSH_DATA (push, (src->y0 << 20) | src->x0 << 4);
}

/* The NOP+OFFSET_OUT stuff after each M2MF transfer *is* actually required
 * to prevent some odd things from happening, easily reproducible by
 * attempting to do conditional rendering that has a M2MF transfer done
 * some time before it.  0x1e98 will fail with a DMA_W_PROTECTION (assuming
 * that name is still accurate on nv4x) error.
 */

static bool
nv30_transfer_m2mf(XFER_ARGS)
{
   if (!src->pitch || !dst->pitch)
      return false;
   if (nv30_transfer_scaled(src, dst))
      return false;
   return true;
}

static void
nv30_transfer_rect_m2mf(XFER_ARGS)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct nouveau_pushbuf_refn refs[] = {
      { src->bo, src->domain | NOUVEAU_BO_RD },
      { dst->bo, dst->domain | NOUVEAU_BO_WR },
   };
   struct nv04_fifo *fifo = push->channel->data;
   unsigned src_offset = src->offset;
   unsigned dst_offset = dst->offset;
   unsigned w = dst->x1 - dst->x0;
   unsigned h = dst->y1 - dst->y0;

   src_offset += (src->y0 * src->pitch) + (src->x0 * src->cpp);
   dst_offset += (dst->y0 * dst->pitch) + (dst->x0 * dst->cpp);

   BEGIN_NV04(push, NV03_M2MF(DMA_BUFFER_IN), 2);
   PUSH_DATA (push, (src->domain == NOUVEAU_BO_VRAM) ? fifo->vram : fifo->gart);
   PUSH_DATA (push, (dst->domain == NOUVEAU_BO_VRAM) ? fifo->vram : fifo->gart);

   while (h) {
      unsigned lines = (h > 2047) ? 2047 : h;

      if (!PUSH_SPACE_EX(push, 32, 2, 0) ||
          PUSH_REFN(push, refs, 2))
         return;

      BEGIN_NV04(push, NV03_M2MF(OFFSET_IN), 8);
      PUSH_RELOC(push, src->bo, src_offset, NOUVEAU_BO_LOW, 0, 0);
      PUSH_RELOC(push, dst->bo, dst_offset, NOUVEAU_BO_LOW, 0, 0);
      PUSH_DATA (push, src->pitch);
      PUSH_DATA (push, dst->pitch);
      PUSH_DATA (push, w * src->cpp);
      PUSH_DATA (push, lines);
      PUSH_DATA (push, NV03_M2MF_FORMAT_INPUT_INC_1 |
                       NV03_M2MF_FORMAT_OUTPUT_INC_1);
      PUSH_DATA (push, 0x00000000);
      BEGIN_NV04(push, NV04_GRAPH(M2MF, NOP), 1);
      PUSH_DATA (push, 0x00000000);
      BEGIN_NV04(push, NV03_M2MF(OFFSET_OUT), 1);
      PUSH_DATA (push, 0x00000000);

      h -= lines;
      src_offset += src->pitch * lines;
      dst_offset += dst->pitch * lines;
   }
}

static bool
nv30_transfer_cpu(XFER_ARGS)
{
   if (nv30_transfer_scaled(src, dst))
      return false;
   return true;
}

static char *
linear_ptr(struct nv30_rect *rect, char *base, int x, int y, int z)
{
   return base + (y * rect->pitch) + (x * rect->cpp);
}

static inline unsigned
swizzle2d(unsigned v, unsigned s)
{
   v = (v | (v << 8)) & 0x00ff00ff;
   v = (v | (v << 4)) & 0x0f0f0f0f;
   v = (v | (v << 2)) & 0x33333333;
   v = (v | (v << 1)) & 0x55555555;
   return v << s;
}

static char *
swizzle2d_ptr(struct nv30_rect *rect, char *base, int x, int y, int z)
{
   unsigned k = util_logbase2(MIN2(rect->w, rect->h));
   unsigned km = (1 << k) - 1;
   unsigned nx = rect->w >> k;
   unsigned tx = x >> k;
   unsigned ty = y >> k;
   unsigned m;

   m  = swizzle2d(x & km, 0);
   m |= swizzle2d(y & km, 1);
   m += ((ty * nx) + tx) << k << k;

   return base + (m * rect->cpp);
}

static char *
swizzle3d_ptr(struct nv30_rect *rect, char *base, int x, int y, int z)
{
   unsigned w = rect->w >> 1;
   unsigned h = rect->h >> 1;
   unsigned d = rect->d >> 1;
   unsigned i = 0, o;
   unsigned v = 0;

   do {
      o = i;
      if (w) {
         v |= (x & 1) << i++;
         x >>= 1;
         w >>= 1;
      }
      if (h) {
         v |= (y & 1) << i++;
         y >>= 1;
         h >>= 1;
      }
      if (d) {
         v |= (z & 1) << i++;
         z >>= 1;
         d >>= 1;
      }
   } while(o != i);

   return base + (v * rect->cpp);
}

typedef char *(*get_ptr_t)(struct nv30_rect *, char *, int, int, int);

static inline get_ptr_t
get_ptr(struct nv30_rect *rect)
{
   if (rect->pitch)
      return linear_ptr;

   if (rect->d <= 1)
      return swizzle2d_ptr;

   return swizzle3d_ptr;
}

static void
nv30_transfer_rect_cpu(XFER_ARGS)
{
   get_ptr_t sp = get_ptr(src);
   get_ptr_t dp = get_ptr(dst);
   char *srcmap, *dstmap;
   int x, y;

   BO_MAP(nv30->base.screen, src->bo, NOUVEAU_BO_RD, nv30->base.client);
   BO_MAP(nv30->base.screen, dst->bo, NOUVEAU_BO_WR, nv30->base.client);
   srcmap = src->bo->map + src->offset;
   dstmap = dst->bo->map + dst->offset;

   for (y = 0; y < (dst->y1 - dst->y0); y++) {
      for (x = 0; x < (dst->x1 - dst->x0); x++) {
         memcpy(dp(dst, dstmap, dst->x0 + x, dst->y0 + y, dst->z),
                sp(src, srcmap, src->x0 + x, src->y0 + y, src->z), dst->cpp);
      }
   }
}

void
nv30_transfer_rect(struct nv30_context *nv30, enum nv30_transfer_filter filter,
                   struct nv30_rect *src, struct nv30_rect *dst)
{
   static const struct {
      char *name;
      bool (*possible)(XFER_ARGS);
      void (*execute)(XFER_ARGS);
   } *method, methods[] = {
      { "m2mf", nv30_transfer_m2mf, nv30_transfer_rect_m2mf },
      { "sifm", nv30_transfer_sifm, nv30_transfer_rect_sifm },
      { "blit", nv30_transfer_blit, nv30_transfer_rect_blit },
      { "rect", nv30_transfer_cpu, nv30_transfer_rect_cpu },
      {}
   };

   for (method = methods; method->possible; method++) {
      if (method->possible(nv30, filter, src, dst)) {
         method->execute(nv30, filter, src, dst);
         return;
      }
   }

   assert(0);
}

void
nv30_transfer_push_data(struct nouveau_context *nv,
                        struct nouveau_bo *bo, unsigned offset, unsigned domain,
                        unsigned size, void *data)
{
   /* use ifc, or scratch + copy_data? */
   fprintf(stderr, "nv30: push_data not implemented\n");
}

void
nv30_transfer_copy_data(struct nouveau_context *nv,
                        struct nouveau_bo *dst, unsigned d_off, unsigned d_dom,
                        struct nouveau_bo *src, unsigned s_off, unsigned s_dom,
                        unsigned size)
{
   struct nv04_fifo *fifo = nv->screen->channel->data;
   struct nouveau_pushbuf_refn refs[] = {
      { src, s_dom | NOUVEAU_BO_RD },
      { dst, d_dom | NOUVEAU_BO_WR },
   };
   struct nouveau_pushbuf *push = nv->pushbuf;
   unsigned pages, lines;

   pages = size >> 12;
   size -= (pages << 12);

   BEGIN_NV04(push, NV03_M2MF(DMA_BUFFER_IN), 2);
   PUSH_DATA (push, (s_dom == NOUVEAU_BO_VRAM) ? fifo->vram : fifo->gart);
   PUSH_DATA (push, (d_dom == NOUVEAU_BO_VRAM) ? fifo->vram : fifo->gart);

   while (pages) {
      lines  = (pages > 2047) ? 2047 : pages;
      pages -= lines;

      if (!PUSH_SPACE_EX(push, 32, 2, 0) ||
          PUSH_REFN(push, refs, 2))
         return;

      BEGIN_NV04(push, NV03_M2MF(OFFSET_IN), 8);
      PUSH_RELOC(push, src, s_off, NOUVEAU_BO_LOW, 0, 0);
      PUSH_RELOC(push, dst, d_off, NOUVEAU_BO_LOW, 0, 0);
      PUSH_DATA (push, 4096);
      PUSH_DATA (push, 4096);
      PUSH_DATA (push, 4096);
      PUSH_DATA (push, lines);
      PUSH_DATA (push, NV03_M2MF_FORMAT_INPUT_INC_1 |
                       NV03_M2MF_FORMAT_OUTPUT_INC_1);
      PUSH_DATA (push, 0x00000000);
      BEGIN_NV04(push, NV04_GRAPH(M2MF, NOP), 1);
      PUSH_DATA (push, 0x00000000);
      BEGIN_NV04(push, NV03_M2MF(OFFSET_OUT), 1);
      PUSH_DATA (push, 0x00000000);

      s_off += (lines << 12);
      d_off += (lines << 12);
   }

   if (size) {
      if (!PUSH_SPACE_EX(push, 32, 2, 0) ||
          PUSH_REFN(push, refs, 2))
         return;

      BEGIN_NV04(push, NV03_M2MF(OFFSET_IN), 8);
      PUSH_RELOC(push, src, s_off, NOUVEAU_BO_LOW, 0, 0);
      PUSH_RELOC(push, dst, d_off, NOUVEAU_BO_LOW, 0, 0);
      PUSH_DATA (push, size);
      PUSH_DATA (push, size);
      PUSH_DATA (push, size);
      PUSH_DATA (push, 1);
      PUSH_DATA (push, NV03_M2MF_FORMAT_INPUT_INC_1 |
                       NV03_M2MF_FORMAT_OUTPUT_INC_1);
      PUSH_DATA (push, 0x00000000);
      BEGIN_NV04(push, NV04_GRAPH(M2MF, NOP), 1);
      PUSH_DATA (push, 0x00000000);
      BEGIN_NV04(push, NV03_M2MF(OFFSET_OUT), 1);
      PUSH_DATA (push, 0x00000000);
   }
}
