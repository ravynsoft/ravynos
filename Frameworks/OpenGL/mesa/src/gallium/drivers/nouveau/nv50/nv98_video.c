/*
 * Copyright 2011-2013 Maarten Lankhorst, Ilia Mirkin
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
 */

#include "nv50/nv98_video.h"

#include "util/u_sampler.h"
#include "util/format/u_format.h"

#include <nvif/class.h>

static void
nv98_decoder_decode_bitstream(struct pipe_video_codec *decoder,
                              struct pipe_video_buffer *video_target,
                              struct pipe_picture_desc *picture,
                              unsigned num_buffers,
                              const void *const *data,
                              const unsigned *num_bytes)
{
   struct nouveau_vp3_decoder *dec = (struct nouveau_vp3_decoder *)decoder;
   struct nouveau_vp3_video_buffer *target = (struct nouveau_vp3_video_buffer *)video_target;
   uint32_t comm_seq = ++dec->fence_seq;
   union pipe_desc desc;

   unsigned vp_caps, is_ref;
   ASSERTED unsigned ret; /* used in debug checks */
   struct nouveau_vp3_video_buffer *refs[16] = {};

   desc.base = picture;

   assert(target->base.buffer_format == PIPE_FORMAT_NV12);

   ret = nv98_decoder_bsp(dec, desc, target, comm_seq,
                          num_buffers, data, num_bytes,
                          &vp_caps, &is_ref, refs);

   /* did we decode bitstream correctly? */
   assert(ret == 2);

   nv98_decoder_vp(dec, desc, target, comm_seq, vp_caps, is_ref, refs);
   nv98_decoder_ppp(dec, desc, target, comm_seq);
}

static const struct nouveau_mclass
nv98_decoder_msvld[] = {
   { G98_MSVLD, -1 },
   { IGT21A_MSVLD, -1 },
   { GT212_MSVLD, -1 },
   {}
};

static const struct nouveau_mclass
nv98_decoder_mspdec[] = {
   { G98_MSPDEC, -1 },
   { GT212_MSPDEC, -1 },
   {}
};

static const struct nouveau_mclass
nv98_decoder_msppp[] = {
   { G98_MSPPP, -1 },
   { GT212_MSPPP, -1 },
   {}
};

struct pipe_video_codec *
nv98_create_decoder(struct pipe_context *context,
                    const struct pipe_video_codec *templ)
{
   struct nv50_context *nv50 = nv50_context(context);
   struct nouveau_screen *screen = &nv50->screen->base;
   struct nouveau_vp3_decoder *dec;
   struct nouveau_pushbuf **push;
   struct nv04_fifo nv04_data = {.vram = 0xbeef0201, .gart = 0xbeef0202};

   int ret, i;
   uint32_t codec = 1, ppp_codec = 3;
   uint32_t timeout;
   u32 tmp_size = 0;

   if (templ->entrypoint != PIPE_VIDEO_ENTRYPOINT_BITSTREAM) {
      debug_printf("%x\n", templ->entrypoint);
      return NULL;
   }

   dec = CALLOC_STRUCT(nouveau_vp3_decoder);
   if (!dec)
      return NULL;
   dec->client = nv50->base.client;
   dec->base = *templ;
   nouveau_vp3_decoder_init_common(&dec->base);

   dec->bsp_idx = 5;
   dec->vp_idx = 6;
   dec->ppp_idx = 7;

   ret = nouveau_object_new(&screen->device->object, 0,
                            NOUVEAU_FIFO_CHANNEL_CLASS,
                            &nv04_data, sizeof(nv04_data), &dec->channel[0]);

   if (!ret)
      ret = nouveau_pushbuf_create(screen, &nv50->base, nv50->base.client, dec->channel[0],
                                   4, 32 * 1024, true, &dec->pushbuf[0]);

   for (i = 1; i < 3; ++i) {
      dec->channel[i] = dec->channel[0];
      dec->pushbuf[i] = dec->pushbuf[0];
   }
   push = dec->pushbuf;

   if (!ret) {
      ret = nouveau_object_mclass(dec->channel[0], nv98_decoder_msvld);
      if (ret >= 0) {
         ret = nouveau_object_new(dec->channel[0], 0xbeef85b1,
                                  nv98_decoder_msvld[ret].oclass, NULL, 0,
                                  &dec->bsp);
      }
   }

   if (!ret) {
      ret = nouveau_object_mclass(dec->channel[1], nv98_decoder_mspdec);
      if (ret >= 0) {
         ret = nouveau_object_new(dec->channel[1], 0xbeef85b2,
                                  nv98_decoder_mspdec[ret].oclass, NULL, 0,
                                  &dec->vp);
      }
   }

   if (!ret) {
      ret = nouveau_object_mclass(dec->channel[2], nv98_decoder_msppp);
      if (ret >= 0) {
         ret = nouveau_object_new(dec->channel[2], 0xbeef85b3,
                                  nv98_decoder_msppp[ret].oclass, NULL, 0,
                                  &dec->ppp);
      }
   }

   if (ret)
      goto fail;

   BEGIN_NV04(push[0], SUBC_BSP(NV01_SUBCHAN_OBJECT), 1);
   PUSH_DATA (push[0], dec->bsp->handle);

   BEGIN_NV04(push[0], SUBC_BSP(0x180), 5);
   for (i = 0; i < 5; i++)
      PUSH_DATA (push[0], nv04_data.vram);

   BEGIN_NV04(push[1], SUBC_VP(NV01_SUBCHAN_OBJECT), 1);
   PUSH_DATA (push[1], dec->vp->handle);

   BEGIN_NV04(push[1], SUBC_VP(0x180), 6);
   for (i = 0; i < 6; i++)
      PUSH_DATA (push[1], nv04_data.vram);

   BEGIN_NV04(push[2], SUBC_PPP(NV01_SUBCHAN_OBJECT), 1);
   PUSH_DATA (push[2], dec->ppp->handle);

   BEGIN_NV04(push[2], SUBC_PPP(0x180), 5);
   for (i = 0; i < 5; i++)
      PUSH_DATA (push[2], nv04_data.vram);

   dec->base.context = context;
   dec->base.decode_bitstream = nv98_decoder_decode_bitstream;

   for (i = 0; i < NOUVEAU_VP3_VIDEO_QDEPTH && !ret; ++i)
      ret = nouveau_bo_new(screen->device, NOUVEAU_BO_VRAM,
                           0, 1 << 20, NULL, &dec->bsp_bo[i]);
   if (!ret)
      ret = nouveau_bo_new(screen->device, NOUVEAU_BO_VRAM,
                           0x100, 4 << 20, NULL, &dec->inter_bo[0]);
   if (!ret)
      nouveau_bo_ref(dec->inter_bo[0], &dec->inter_bo[1]);
   if (ret)
      goto fail;

   switch (u_reduce_video_profile(templ->profile)) {
   case PIPE_VIDEO_FORMAT_MPEG12: {
      codec = 1;
      assert(templ->max_references <= 2);
      break;
   }
   case PIPE_VIDEO_FORMAT_MPEG4: {
      codec = 4;
      tmp_size = mb(templ->height)*16 * mb(templ->width)*16;
      assert(templ->max_references <= 2);
      break;
   }
   case PIPE_VIDEO_FORMAT_VC1: {
      ppp_codec = codec = 2;
      tmp_size = mb(templ->height)*16 * mb(templ->width)*16;
      assert(templ->max_references <= 2);
      break;
   }
   case PIPE_VIDEO_FORMAT_MPEG4_AVC: {
      codec = 3;
      dec->tmp_stride = 16 * mb_half(templ->width) * nouveau_vp3_video_align(templ->height) * 3 / 2;
      tmp_size = dec->tmp_stride * (templ->max_references + 1);
      assert(templ->max_references <= 16);
      break;
   }
   default:
      fprintf(stderr, "invalid codec\n");
      goto fail;
   }

   ret = nouveau_bo_new(screen->device, NOUVEAU_BO_VRAM, 0,
                           0x4000, NULL, &dec->fw_bo);
   if (ret)
      goto fail;

   ret = nouveau_vp3_load_firmware(dec, templ->profile, screen->device->chipset);
   if (ret)
      goto fw_fail;

   if (codec != 3) {
      ret = nouveau_bo_new(screen->device, NOUVEAU_BO_VRAM, 0,
                           0x400, NULL, &dec->bitplane_bo);
      if (ret)
         goto fail;
   }

   dec->ref_stride = mb(templ->width)*16 * (mb_half(templ->height)*32 + nouveau_vp3_video_align(templ->height)/2);
   ret = nouveau_bo_new(screen->device, NOUVEAU_BO_VRAM, 0,
                        dec->ref_stride * (templ->max_references+2) + tmp_size,
                        NULL, &dec->ref_bo);
   if (ret)
      goto fail;

   timeout = 0;

   BEGIN_NV04(push[0], SUBC_BSP(0x200), 2);
   PUSH_DATA (push[0], codec);
   PUSH_DATA (push[0], timeout);

   BEGIN_NV04(push[1], SUBC_VP(0x200), 2);
   PUSH_DATA (push[1], codec);
   PUSH_DATA (push[1], timeout);

   BEGIN_NV04(push[2], SUBC_PPP(0x200), 2);
   PUSH_DATA (push[2], ppp_codec);
   PUSH_DATA (push[2], timeout);

   ++dec->fence_seq;

#if NOUVEAU_VP3_DEBUG_FENCE
   ret = nouveau_bo_new(screen->device, NOUVEAU_BO_GART|NOUVEAU_BO_MAP,
                        0, 0x1000, NULL, &dec->fence_bo);
   if (ret)
      goto fail;

   BO_MAP(screen, dec->fence_bo, NOUVEAU_BO_RDWR, screen->client);
   dec->fence_map = dec->fence_bo->map;
   dec->fence_map[0] = dec->fence_map[4] = dec->fence_map[8] = 0;
   dec->comm = (struct comm *)(dec->fence_map + (COMM_OFFSET/sizeof(*dec->fence_map)));

   /* So lets test if the fence is working? */
   PUSH_SPACE_EX(push[0], 16, 1, 0);
   PUSH_REF1 (push[0], dec->fence_bo, NOUVEAU_BO_GART|NOUVEAU_BO_RDWR);
   BEGIN_NV04(push[0], SUBC_BSP(0x240), 3);
   PUSH_DATAh(push[0], dec->fence_bo->offset);
   PUSH_DATA (push[0], dec->fence_bo->offset);
   PUSH_DATA (push[0], dec->fence_seq);

   BEGIN_NV04(push[0], SUBC_BSP(0x304), 1);
   PUSH_DATA (push[0], 0);
   PUSH_KICK (push[0]);

   PUSH_SPACE_EX(push[1], 16, 1, 0);
   PUSH_REF1 (push[1], dec->fence_bo, NOUVEAU_BO_GART|NOUVEAU_BO_RDWR);
   BEGIN_NV04(push[1], SUBC_VP(0x240), 3);
   PUSH_DATAh(push[1], (dec->fence_bo->offset + 0x10));
   PUSH_DATA (push[1], (dec->fence_bo->offset + 0x10));
   PUSH_DATA (push[1], dec->fence_seq);

   BEGIN_NV04(push[1], SUBC_VP(0x304), 1);
   PUSH_DATA (push[1], 0);
   PUSH_KICK (push[1]);

   PUSH_SPACE_EX(push[2], 16, 1, 0);
   PUSH_REF1 (push[2], dec->fence_bo, NOUVEAU_BO_GART|NOUVEAU_BO_RDWR);
   BEGIN_NV04(push[2], SUBC_PPP(0x240), 3);
   PUSH_DATAh(push[2], (dec->fence_bo->offset + 0x20));
   PUSH_DATA (push[2], (dec->fence_bo->offset + 0x20));
   PUSH_DATA (push[2], dec->fence_seq);

   BEGIN_NV04(push[2], SUBC_PPP(0x304), 1);
   PUSH_DATA (push[2], 0);
   PUSH_KICK (push[2]);

   usleep(100);
   while (dec->fence_seq > dec->fence_map[0] ||
          dec->fence_seq > dec->fence_map[4] ||
          dec->fence_seq > dec->fence_map[8]) {
      debug_printf("%u: %u %u %u\n", dec->fence_seq, dec->fence_map[0], dec->fence_map[4], dec->fence_map[8]);
      usleep(100);
   }
   debug_printf("%u: %u %u %u\n", dec->fence_seq, dec->fence_map[0], dec->fence_map[4], dec->fence_map[8]);
#endif

   return &dec->base;

fw_fail:
   debug_printf("Cannot create decoder without firmware..\n");
   dec->base.destroy(&dec->base);
   return NULL;

fail:
   debug_printf("Creation failed: %s (%i)\n", strerror(-ret), ret);
   dec->base.destroy(&dec->base);
   return NULL;
}

struct pipe_video_buffer *
nv98_video_buffer_create(struct pipe_context *pipe,
                         const struct pipe_video_buffer *templat)
{
   return nouveau_vp3_video_buffer_create(
         pipe, templat, NV50_RESOURCE_FLAG_VIDEO);
}
