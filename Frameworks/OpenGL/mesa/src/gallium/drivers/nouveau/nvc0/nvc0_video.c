/*
 * Copyright 2011-2013 Maarten Lankhorst
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

#include "nvc0/nvc0_video.h"

#include "util/u_sampler.h"
#include "util/format/u_format.h"

static void
nvc0_decoder_begin_frame(struct pipe_video_codec *decoder,
                         struct pipe_video_buffer *target,
                         struct pipe_picture_desc *picture)
{
   struct nouveau_vp3_decoder *dec = (struct nouveau_vp3_decoder *)decoder;
   uint32_t comm_seq = ++dec->fence_seq;
   ASSERTED unsigned ret = 0; /* used in debug checks */

   assert(dec);
   assert(target);
   assert(target->buffer_format == PIPE_FORMAT_NV12);

   ret = nvc0_decoder_bsp_begin(dec, comm_seq);

   assert(ret == 2);
}

static void
nvc0_decoder_decode_bitstream(struct pipe_video_codec *decoder,
                              struct pipe_video_buffer *video_target,
                              struct pipe_picture_desc *picture,
                              unsigned num_buffers,
                              const void *const *data,
                              const unsigned *num_bytes)
{
   struct nouveau_vp3_decoder *dec = (struct nouveau_vp3_decoder *)decoder;
   uint32_t comm_seq = dec->fence_seq;
   ASSERTED unsigned ret = 0; /* used in debug checks */

   assert(decoder);

   ret = nvc0_decoder_bsp_next(dec, comm_seq, num_buffers, data, num_bytes);

   assert(ret == 2);
}

static void
nvc0_decoder_end_frame(struct pipe_video_codec *decoder,
                       struct pipe_video_buffer *video_target,
                       struct pipe_picture_desc *picture)
{
   struct nouveau_vp3_decoder *dec = (struct nouveau_vp3_decoder *)decoder;
   struct nouveau_vp3_video_buffer *target = (struct nouveau_vp3_video_buffer *)video_target;
   uint32_t comm_seq = dec->fence_seq;
   union pipe_desc desc;

   unsigned vp_caps, is_ref;
   ASSERTED unsigned ret; /* used in debug checks */
   struct nouveau_vp3_video_buffer *refs[16] = {};

   desc.base = picture;

   ret = nvc0_decoder_bsp_end(dec, desc, target, comm_seq, &vp_caps, &is_ref, refs);

   /* did we decode bitstream correctly? */
   assert(ret == 2);

   nvc0_decoder_vp(dec, desc, target, comm_seq, vp_caps, is_ref, refs);
   nvc0_decoder_ppp(dec, desc, target, comm_seq);
}

struct pipe_video_codec *
nvc0_create_decoder(struct pipe_context *context,
                    const struct pipe_video_codec *templ)
{
   struct nvc0_context *nvc0 = nvc0_context(context);
   struct nouveau_screen *screen = &nvc0->screen->base;
   struct nouveau_vp3_decoder *dec;
   struct nouveau_pushbuf **push;
   union nouveau_bo_config cfg;
   bool kepler = screen->device->chipset >= 0xe0;

   cfg.nvc0.tile_mode = 0x10;
   cfg.nvc0.memtype = 0xfe;

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
   dec->client = nvc0->base.client;
   dec->base = *templ;
   nouveau_vp3_decoder_init_common(&dec->base);

   if (!kepler) {
      dec->bsp_idx = 5;
      dec->vp_idx = 6;
      dec->ppp_idx = 7;
   } else {
      dec->bsp_idx = 2;
      dec->vp_idx = 2;
      dec->ppp_idx = 2;
   }

   for (i = 0; i < 3; ++i)
      if (i && !kepler) {
         dec->channel[i] = dec->channel[0];
         dec->pushbuf[i] = dec->pushbuf[0];
      } else {
         void *data;
         u32 size;
         struct nvc0_fifo nvc0_args = {};
         struct nve0_fifo nve0_args = {};

         if (!kepler) {
            size = sizeof(nvc0_args);
            data = &nvc0_args;
         } else {
            unsigned engine[] = {
               NVE0_FIFO_ENGINE_BSP,
               NVE0_FIFO_ENGINE_VP,
               NVE0_FIFO_ENGINE_PPP
            };

            nve0_args.engine = engine[i];
            size = sizeof(nve0_args);
            data = &nve0_args;
         }

         ret = nouveau_object_new(&screen->device->object, 0,
                                  NOUVEAU_FIFO_CHANNEL_CLASS,
                                  data, size, &dec->channel[i]);

         if (!ret)
            ret = nouveau_pushbuf_create(screen, &nvc0->base, nvc0->base.client, dec->channel[i],
                                         4, 32 * 1024, true, &dec->pushbuf[i]);
         if (ret)
            break;
      }
   push = dec->pushbuf;

   if (!kepler) {
      if (!ret)
         ret = nouveau_object_new(dec->channel[0], 0x390b1, 0x90b1, NULL, 0, &dec->bsp);
      if (!ret)
         ret = nouveau_object_new(dec->channel[1], 0x190b2, 0x90b2, NULL, 0, &dec->vp);
      if (!ret)
         ret = nouveau_object_new(dec->channel[2], 0x290b3, 0x90b3, NULL, 0, &dec->ppp);
   } else {
      if (!ret)
         ret = nouveau_object_new(dec->channel[0], 0x95b1, 0x95b1, NULL, 0, &dec->bsp);
      if (!ret)
         ret = nouveau_object_new(dec->channel[1], 0x95b2, 0x95b2, NULL, 0, &dec->vp);
      if (!ret)
         ret = nouveau_object_new(dec->channel[2], 0x90b3, 0x90b3, NULL, 0, &dec->ppp);
   }
   if (ret)
      goto fail;

   BEGIN_NVC0(push[0], SUBC_BSP(NV01_SUBCHAN_OBJECT), 1);
   PUSH_DATA (push[0], dec->bsp->handle);

   BEGIN_NVC0(push[1], SUBC_VP(NV01_SUBCHAN_OBJECT), 1);
   PUSH_DATA (push[1], dec->vp->handle);

   BEGIN_NVC0(push[2], SUBC_PPP(NV01_SUBCHAN_OBJECT), 1);
   PUSH_DATA (push[2], dec->ppp->handle);

   dec->base.context = context;
   dec->base.begin_frame = nvc0_decoder_begin_frame;
   dec->base.decode_bitstream = nvc0_decoder_decode_bitstream;
   dec->base.end_frame = nvc0_decoder_end_frame;

   for (i = 0; i < NOUVEAU_VP3_VIDEO_QDEPTH && !ret; ++i)
      ret = nouveau_bo_new(screen->device, NOUVEAU_BO_VRAM,
                           0, 1 << 20, &cfg, &dec->bsp_bo[i]);
   if (!ret) {
      /* total fudge factor... just has to be bigger for higher bitrates? */
      unsigned inter_size = align(templ->width * templ->height * 2, 4 << 20);
      ret = nouveau_bo_new(screen->device, NOUVEAU_BO_VRAM,
                           0x100, inter_size, &cfg, &dec->inter_bo[0]);
   }
   if (!ret) {
      ret = nouveau_bo_new(screen->device, NOUVEAU_BO_VRAM,
                           0x100, dec->inter_bo[0]->size, &cfg,
                           &dec->inter_bo[1]);
   }
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

   if (screen->device->chipset < 0xd0) {
      ret = nouveau_bo_new(screen->device, NOUVEAU_BO_VRAM, 0,
                           0x4000, &cfg, &dec->fw_bo);
      if (ret)
         goto fail;

      ret = nouveau_vp3_load_firmware(dec, templ->profile, screen->device->chipset);
      if (ret)
         goto fw_fail;
   }

   if (codec != 3) {
      ret = nouveau_bo_new(screen->device, NOUVEAU_BO_VRAM, 0,
                           0x400, &cfg, &dec->bitplane_bo);
      if (ret)
         goto fail;
   }

   dec->ref_stride = mb(templ->width)*16 * (mb_half(templ->height)*32 + nouveau_vp3_video_align(templ->height)/2);
   ret = nouveau_bo_new(screen->device, NOUVEAU_BO_VRAM, 0,
                        dec->ref_stride * (templ->max_references+2) + tmp_size,
                        &cfg, &dec->ref_bo);
   if (ret)
      goto fail;

   timeout = 0;

   BEGIN_NVC0(push[0], SUBC_BSP(0x200), 2);
   PUSH_DATA (push[0], codec);
   PUSH_DATA (push[0], timeout);

   BEGIN_NVC0(push[1], SUBC_VP(0x200), 2);
   PUSH_DATA (push[1], codec);
   PUSH_DATA (push[1], timeout);

   BEGIN_NVC0(push[2], SUBC_PPP(0x200), 2);
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
   BEGIN_NVC0(push[0], SUBC_BSP(0x240), 3);
   PUSH_DATAh(push[0], dec->fence_bo->offset);
   PUSH_DATA (push[0], dec->fence_bo->offset);
   PUSH_DATA (push[0], dec->fence_seq);

   BEGIN_NVC0(push[0], SUBC_BSP(0x304), 1);
   PUSH_DATA (push[0], 0);
   PUSH_KICK (push[0]);

   PUSH_SPACE_EX(push[1], 16, 1, 0);
   PUSH_REF1 (push[1], dec->fence_bo, NOUVEAU_BO_GART|NOUVEAU_BO_RDWR);
   BEGIN_NVC0(push[1], SUBC_VP(0x240), 3);
   PUSH_DATAh(push[1], (dec->fence_bo->offset + 0x10));
   PUSH_DATA (push[1], (dec->fence_bo->offset + 0x10));
   PUSH_DATA (push[1], dec->fence_seq);

   BEGIN_NVC0(push[1], SUBC_VP(0x304), 1);
   PUSH_DATA (push[1], 0);
   PUSH_KICK (push[1]);

   PUSH_SPACE_EX(push[2], 16, 1, 0);
   PUSH_REF1 (push[2], dec->fence_bo, NOUVEAU_BO_GART|NOUVEAU_BO_RDWR);
   BEGIN_NVC0(push[2], SUBC_PPP(0x240), 3);
   PUSH_DATAh(push[2], (dec->fence_bo->offset + 0x20));
   PUSH_DATA (push[2], (dec->fence_bo->offset + 0x20));
   PUSH_DATA (push[2], dec->fence_seq);

   BEGIN_NVC0(push[2], SUBC_PPP(0x304), 1);
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
nvc0_video_buffer_create(struct pipe_context *pipe,
                         const struct pipe_video_buffer *templat)
{
   return nouveau_vp3_video_buffer_create(
         pipe, templat, NVC0_RESOURCE_FLAG_VIDEO);
}
