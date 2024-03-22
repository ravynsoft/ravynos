/*
 * Copyright 2011 Maarten Lankhorst
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

#include "vl/vl_decoder.h"
#include "vl/vl_video_buffer.h"

#include "nouveau_screen.h"
#include "nouveau_context.h"
#include "nouveau_video.h"

#include "nouveau_buffer.h"
#include "util/u_video.h"
#include "util/format/u_format.h"
#include "util/u_sampler.h"

static int
nouveau_vpe_init(struct nouveau_decoder *dec) {
   int ret;
   if (dec->cmds)
      return 0;
   ret = BO_MAP(dec->screen, dec->cmd_bo, NOUVEAU_BO_RDWR, dec->client);
   if (ret) {
      debug_printf("Mapping cmd bo: %s\n", strerror(-ret));
      return ret;
   }
   ret = BO_MAP(dec->screen, dec->data_bo, NOUVEAU_BO_RDWR, dec->client);
   if (ret) {
      debug_printf("Mapping data bo: %s\n", strerror(-ret));
      return ret;
   }
   dec->cmds = dec->cmd_bo->map;
   dec->data = dec->data_bo->map;
   return ret;
}

static void
nouveau_vpe_synch(struct nouveau_decoder *dec) {
   struct nouveau_pushbuf *push = dec->push;
#if 0
   if (dec->fence_map) {
      BEGIN_NV04(push, NV84_MPEG(QUERY_COUNTER), 1);
      PUSH_DATA (push, ++dec->fence_seq);
      PUSH_KICK (push);
      while (dec->fence_map[0] != dec->fence_seq)
         usleep(1000);
   } else
#endif
      PUSH_KICK(push);
}

static void
nouveau_vpe_fini(struct nouveau_decoder *dec) {
   struct nouveau_pushbuf *push = dec->push;
   if (!dec->cmds)
      return;

   PUSH_SPACE_EX(push, 16, 2, 0);
   nouveau_bufctx_reset(dec->bufctx, NV31_VIDEO_BIND_CMD);

#define BCTX_ARGS dec->bufctx, NV31_VIDEO_BIND_CMD, NOUVEAU_BO_RD

   BEGIN_NV04(push, NV31_MPEG(CMD_OFFSET), 2);
   PUSH_MTHDl(push, NV31_MPEG(CMD_OFFSET), dec->cmd_bo, 0, BCTX_ARGS);
   PUSH_DATA (push, dec->ofs * 4);

   BEGIN_NV04(push, NV31_MPEG(DATA_OFFSET), 2);
   PUSH_MTHDl(push, NV31_MPEG(DATA_OFFSET), dec->data_bo, 0, BCTX_ARGS);
   PUSH_DATA (push, dec->data_pos * 4);

#undef BCTX_ARGS

   if (unlikely(PUSH_VAL(dec->push)))
      return;

   BEGIN_NV04(push, NV31_MPEG(EXEC), 1);
   PUSH_DATA (push, 1);

   nouveau_vpe_synch(dec);
   dec->ofs = dec->data_pos = dec->num_surfaces = 0;
   dec->cmds = dec->data = NULL;
   dec->current = dec->future = dec->past = 8;
}

static inline void
nouveau_vpe_mb_dct_blocks(struct nouveau_decoder *dec, const struct pipe_mpeg12_macroblock *mb)
{
   int cbb;
   unsigned cbp = mb->coded_block_pattern;
   short *db = mb->blocks;
   for (cbb = 0x20; cbb > 0; cbb >>= 1) {
      if (cbb & cbp) {
         int i, found = 0;
         for (i = 0; i < 64; ++i) {
            if (!db[i]) continue;
            dec->data[dec->data_pos++] = (db[i] << 16) | (i * 2);
            found = 1;
         }
         if (found)
            dec->data[dec->data_pos - 1] |= 1;
         else
            dec->data[dec->data_pos++] = 1;
         db += 64;
      } else if (mb->macroblock_type & PIPE_MPEG12_MB_TYPE_INTRA) {
         dec->data[dec->data_pos++] = 1;
      }
   }
}

static inline void
nouveau_vpe_mb_data_blocks(struct nouveau_decoder *dec, const struct pipe_mpeg12_macroblock *mb)
{
   int cbb;
   unsigned cbp = mb->coded_block_pattern;
   short *db = mb->blocks;
   for (cbb = 0x20; cbb > 0; cbb >>= 1) {
      if (cbb & cbp) {
         memcpy(&dec->data[dec->data_pos], db, 128);
         dec->data_pos += 32;
         db += 64;
      } else if (mb->macroblock_type & PIPE_MPEG12_MB_TYPE_INTRA) {
         memset(&dec->data[dec->data_pos], 0, 128);
         dec->data_pos += 32;
      }
   }
}

static inline void
nouveau_vpe_mb_dct_header(struct nouveau_decoder *dec,
                          const struct pipe_mpeg12_macroblock *mb,
                          bool luma)
{
   unsigned base_dct, cbp;
   bool intra = mb->macroblock_type & PIPE_MPEG12_MB_TYPE_INTRA;
   unsigned x = mb->x * 16;
   unsigned y = luma ? mb->y * 16 : mb->y * 8;

   /* Setup the base dct header */
   base_dct = dec->current << NV17_MPEG_CMD_CHROMA_MB_HEADER_SURFACE__SHIFT;
   base_dct |= NV17_MPEG_CMD_CHROMA_MB_HEADER_RUN_SINGLE;

   if (!(mb->x & 1))
      base_dct |= NV17_MPEG_CMD_CHROMA_MB_HEADER_X_COORD_EVEN;
   if (intra)
      cbp = 0x3f;
   else
      cbp = mb->coded_block_pattern;

   if (dec->picture_structure == PIPE_MPEG12_PICTURE_STRUCTURE_FRAME) {
      base_dct |= NV17_MPEG_CMD_CHROMA_MB_HEADER_TYPE_FRAME;
      if (luma && mb->macroblock_modes.bits.dct_type == PIPE_MPEG12_DCT_TYPE_FIELD)
         base_dct |= NV17_MPEG_CMD_CHROMA_MB_HEADER_FRAME_DCT_TYPE_FIELD;
   } else {
      if (dec->picture_structure == PIPE_MPEG12_PICTURE_STRUCTURE_FIELD_BOTTOM)
         base_dct |= NV17_MPEG_CMD_CHROMA_MB_HEADER_FIELD_BOTTOM;
      if (!intra)
         y *= 2;
   }

   if (luma) {
      base_dct |= NV17_MPEG_CMD_LUMA_MB_HEADER_OP_LUMA_MB_HEADER;
      base_dct |= (cbp >> 2) << NV17_MPEG_CMD_LUMA_MB_HEADER_CBP__SHIFT;
   } else {
      base_dct |= NV17_MPEG_CMD_CHROMA_MB_HEADER_OP_CHROMA_MB_HEADER;
      base_dct |= (cbp & 3) << NV17_MPEG_CMD_CHROMA_MB_HEADER_CBP__SHIFT;
   }
   nouveau_vpe_write(dec, base_dct);
   nouveau_vpe_write(dec, NV17_MPEG_CMD_MB_COORDS_OP_MB_COORDS |
                     x | (y << NV17_MPEG_CMD_MB_COORDS_Y__SHIFT));
}

static inline unsigned int
nouveau_vpe_mb_mv_flags(bool luma, int mv_h, int mv_v, bool forward, bool first, bool vert)
{
   unsigned mc_header = 0;
   if (luma)
      mc_header |= NV17_MPEG_CMD_LUMA_MV_HEADER_OP_LUMA_MV_HEADER;
   else
      mc_header |= NV17_MPEG_CMD_CHROMA_MV_HEADER_OP_CHROMA_MV_HEADER;
   if (mv_h & 1)
      mc_header |= NV17_MPEG_CMD_CHROMA_MV_HEADER_X_HALF;
   if (mv_v & 1)
      mc_header |= NV17_MPEG_CMD_CHROMA_MV_HEADER_Y_HALF;
   if (!forward)
      mc_header |= NV17_MPEG_CMD_CHROMA_MV_HEADER_DIRECTION_BACKWARD;
   if (!first)
      mc_header |= NV17_MPEG_CMD_CHROMA_MV_HEADER_IDX;
   if (vert)
      mc_header |= NV17_MPEG_CMD_LUMA_MV_HEADER_FIELD_BOTTOM;
   return mc_header;
}

static unsigned pos(int pos, int mov, int max) {
   int ret = pos + mov;
   if (pos < 0)
      return 0;
   if (pos >= max)
      return max-1;
   return ret;
}

/* because we want -1 / 2 = -1 */
static int div_down(int val, int mult) {
   val &= ~(mult - 1);
   return val / mult;
}

static int div_up(int val, int mult) {
   val += mult - 1;
   return val / mult;
}

static inline void
nouveau_vpe_mb_mv(struct nouveau_decoder *dec, unsigned mc_header,
                   bool luma, bool frame, bool forward, bool vert,
                   int x, int y, const short motions[2],
                   unsigned surface, bool first)
{
   unsigned mc_vector;
   int mv_horizontal = motions[0];
   int mv_vertical = motions[1];
   int mv2 = mc_header & NV17_MPEG_CMD_CHROMA_MV_HEADER_COUNT_2;
   unsigned width = dec->base.width;
   unsigned height = dec->base.height;
   if (mv2)
      mv_vertical = div_down(mv_vertical, 2);
   assert(frame); // Untested for non-frames
   if (!frame)
      height *= 2;

   mc_header |= surface << NV17_MPEG_CMD_CHROMA_MV_HEADER_SURFACE__SHIFT;
   if (!luma) {
      mv_vertical = div_up(mv_vertical, 2);
      mv_horizontal = div_up(mv_horizontal, 2);
      height /= 2;
   }
   mc_header |= nouveau_vpe_mb_mv_flags(luma, mv_horizontal, mv_vertical, forward, first, vert);
   nouveau_vpe_write(dec, mc_header);

   mc_vector = NV17_MPEG_CMD_MV_COORDS_OP_MV_COORDS;
   if (luma)
      mc_vector |= pos(x, div_down(mv_horizontal, 2), width);
   else
      mc_vector |= pos(x, mv_horizontal & ~1, width);
   if (!mv2)
      mc_vector |= pos(y, div_down(mv_vertical, 2), height) << NV17_MPEG_CMD_MV_COORDS_Y__SHIFT;
   else
      mc_vector |= pos(y, mv_vertical & ~1, height) << NV17_MPEG_CMD_MV_COORDS_Y__SHIFT;
   nouveau_vpe_write(dec, mc_vector);
}

static void
nouveau_vpe_mb_mv_header(struct nouveau_decoder *dec,
                         const struct pipe_mpeg12_macroblock *mb,
                         bool luma)
{
   bool frame = dec->picture_structure == PIPE_MPEG12_PICTURE_STRUCTURE_FRAME;
   unsigned base;
   bool forward, backward;
   int y, y2, x = mb->x * 16;
   if (luma)
      y = mb->y * (frame ? 16 : 32);
   else
      y = mb->y * (frame ? 8 : 16);
   if (frame)
      y2 = y;
   else
      y2 = y + (luma ? 16 : 8);

   forward = mb->macroblock_type & PIPE_MPEG12_MB_TYPE_MOTION_FORWARD;
   backward = mb->macroblock_type & PIPE_MPEG12_MB_TYPE_MOTION_BACKWARD;
   assert(!forward || dec->past < 8);
   assert(!backward || dec->future < 8);
   if (frame) {
      switch (mb->macroblock_modes.bits.frame_motion_type) {
      case PIPE_MPEG12_MO_TYPE_FRAME: goto mv1;
      case PIPE_MPEG12_MO_TYPE_FIELD: goto mv2;
      case PIPE_MPEG12_MO_TYPE_DUAL_PRIME: {
         base = NV17_MPEG_CMD_CHROMA_MV_HEADER_COUNT_2;
         if (forward) {
            nouveau_vpe_mb_mv(dec, base, luma, frame, true, false,
                              x, y, mb->PMV[0][0], dec->past, true);
            nouveau_vpe_mb_mv(dec, base, luma, frame, true, true,
                              x, y2, mb->PMV[0][0], dec->past, false);
         }
         if (backward && forward) {
            nouveau_vpe_mb_mv(dec, base, luma, frame, !forward, true,
                              x, y, mb->PMV[1][0], dec->future, true);
            nouveau_vpe_mb_mv(dec, base, luma, frame, !forward, false,
                              x, y2, mb->PMV[1][1], dec->future, false);
         } else assert(!backward);
         break;
      }
      default: assert(0);
      }
   } else {
      switch (mb->macroblock_modes.bits.field_motion_type) {
      case PIPE_MPEG12_MO_TYPE_FIELD: goto mv1;
      case PIPE_MPEG12_MO_TYPE_16x8: goto mv2;
      case PIPE_MPEG12_MO_TYPE_DUAL_PRIME: {
      base = NV17_MPEG_CMD_CHROMA_MV_HEADER_MV_SPLIT_HALF_MB;
         if (frame)
            base |= NV17_MPEG_CMD_CHROMA_MV_HEADER_TYPE_FRAME;
         if (forward)
            nouveau_vpe_mb_mv(dec, base, luma, frame, true,
                              dec->picture_structure != PIPE_MPEG12_PICTURE_STRUCTURE_FIELD_TOP,
                              x, y, mb->PMV[0][0], dec->past, true);
         if (backward && forward)
            nouveau_vpe_mb_mv(dec, base, luma, frame, false,
                              dec->picture_structure == PIPE_MPEG12_PICTURE_STRUCTURE_FIELD_TOP,
                              x, y, mb->PMV[0][1], dec->future, true);
         else assert(!backward);
         break;
      }
      default: assert(0);
      }
   }
   return;

mv1:
   base = NV17_MPEG_CMD_CHROMA_MV_HEADER_MV_SPLIT_HALF_MB;
   if (frame)
       base |= NV17_MPEG_CMD_CHROMA_MV_HEADER_TYPE_FRAME;
    /* frame 16x16 */
   if (forward)
       nouveau_vpe_mb_mv(dec, base, luma, frame, true, false,
                         x, y, mb->PMV[0][0], dec->past, true);
   if (backward)
       nouveau_vpe_mb_mv(dec, base, luma, frame, !forward, false,
                         x, y, mb->PMV[0][1], dec->future, true);
    return;

mv2:
   base = NV17_MPEG_CMD_CHROMA_MV_HEADER_COUNT_2;
   if (!frame)
      base |= NV17_MPEG_CMD_CHROMA_MV_HEADER_MV_SPLIT_HALF_MB;
   if (forward) {
      nouveau_vpe_mb_mv(dec, base, luma, frame, true,
                        mb->motion_vertical_field_select & PIPE_MPEG12_FS_FIRST_FORWARD,
                        x, y, mb->PMV[0][0], dec->past, true);
      nouveau_vpe_mb_mv(dec, base, luma, frame, true,
                        mb->motion_vertical_field_select & PIPE_MPEG12_FS_SECOND_FORWARD,
                        x, y2, mb->PMV[1][0], dec->past, false);
   }
   if (backward) {
      nouveau_vpe_mb_mv(dec, base, luma, frame, !forward,
                        mb->motion_vertical_field_select & PIPE_MPEG12_FS_FIRST_BACKWARD,
                        x, y, mb->PMV[0][1], dec->future, true);
      nouveau_vpe_mb_mv(dec, base, luma, frame, !forward,
                        mb->motion_vertical_field_select & PIPE_MPEG12_FS_SECOND_BACKWARD,
                        x, y2, mb->PMV[1][1], dec->future, false);
   }
}

static unsigned
nouveau_decoder_surface_index(struct nouveau_decoder *dec,
                              struct pipe_video_buffer *buffer)
{
   struct nouveau_video_buffer *buf = (struct nouveau_video_buffer *)buffer;
   struct nouveau_pushbuf *push = dec->push;
   struct nouveau_bo *bo_y = nv04_resource(buf->resources[0])->bo;
   struct nouveau_bo *bo_c = nv04_resource(buf->resources[1])->bo;

   unsigned i;

   for (i = 0; i < dec->num_surfaces; ++i) {
      if (dec->surfaces[i] == buf)
         return i;
   }
   assert(i < 8);
   dec->surfaces[i] = buf;
   dec->num_surfaces++;

   nouveau_bufctx_reset(dec->bufctx, NV31_VIDEO_BIND_IMG(i));

#define BCTX_ARGS dec->bufctx, NV31_VIDEO_BIND_IMG(i), NOUVEAU_BO_RDWR
   BEGIN_NV04(push, NV31_MPEG(IMAGE_Y_OFFSET(i)), 2);
   PUSH_MTHDl(push, NV31_MPEG(IMAGE_Y_OFFSET(i)), bo_y, 0, BCTX_ARGS);
   PUSH_MTHDl(push, NV31_MPEG(IMAGE_C_OFFSET(i)), bo_c, 0, BCTX_ARGS);
#undef BCTX_ARGS

   return i;
}

static void
nouveau_decoder_begin_frame(struct pipe_video_codec *decoder,
                            struct pipe_video_buffer *target,
                            struct pipe_picture_desc *picture)
{
}

static void
nouveau_decoder_decode_macroblock(struct pipe_video_codec *decoder,
                                  struct pipe_video_buffer *target,
                                  struct pipe_picture_desc *picture,
                                  const struct pipe_macroblock *pipe_mb,
                                  unsigned num_macroblocks)
{
   struct nouveau_decoder *dec = (struct nouveau_decoder *)decoder;
   struct pipe_mpeg12_picture_desc *desc = (struct pipe_mpeg12_picture_desc*)picture;
   const struct pipe_mpeg12_macroblock *mb;
   unsigned i;
   assert(target->width == decoder->width);
   assert(target->height == decoder->height);

   dec->current = nouveau_decoder_surface_index(dec, target);
   assert(dec->current < 8);
   dec->picture_structure = desc->picture_structure;
   if (desc->ref[1])
      dec->future = nouveau_decoder_surface_index(dec, desc->ref[1]);
   if (desc->ref[0])
      dec->past = nouveau_decoder_surface_index(dec, desc->ref[0]);

   if (nouveau_vpe_init(dec)) return;

   /* initialize scan order */
   nouveau_vpe_write(dec, 0x720000c0);
   nouveau_vpe_write(dec, dec->data_pos);

   mb = (const struct pipe_mpeg12_macroblock *)pipe_mb;
   for (i = 0; i < num_macroblocks; ++i, mb++) {
      if (mb->macroblock_type & PIPE_MPEG12_MB_TYPE_INTRA) {
         nouveau_vpe_mb_dct_header(dec, mb, true);
         nouveau_vpe_mb_dct_header(dec, mb, false);
      } else {
         nouveau_vpe_mb_mv_header(dec, mb, true);
         nouveau_vpe_mb_dct_header(dec, mb, true);

         nouveau_vpe_mb_mv_header(dec, mb, false);
         nouveau_vpe_mb_dct_header(dec, mb, false);
      }
      if (dec->base.entrypoint <= PIPE_VIDEO_ENTRYPOINT_IDCT)
         nouveau_vpe_mb_dct_blocks(dec, mb);
      else
         nouveau_vpe_mb_data_blocks(dec, mb);
   }
}

static void
nouveau_decoder_end_frame(struct pipe_video_codec *decoder,
                          struct pipe_video_buffer *target,
                          struct pipe_picture_desc *picture)
{
}

static void
nouveau_decoder_flush(struct pipe_video_codec *decoder)
{
   struct nouveau_decoder *dec = (struct nouveau_decoder *)decoder;
   if (dec->ofs)
      nouveau_vpe_fini(dec);
}

static void
nouveau_decoder_destroy(struct pipe_video_codec *decoder)
{
   struct nouveau_decoder *dec = (struct nouveau_decoder*)decoder;

   if (dec->data_bo)
      nouveau_bo_ref(NULL, &dec->data_bo);
   if (dec->cmd_bo)
      nouveau_bo_ref(NULL, &dec->cmd_bo);
   if (dec->fence_bo)
      nouveau_bo_ref(NULL, &dec->fence_bo);

   nouveau_object_del(&dec->mpeg);

   if (dec->bufctx)
      nouveau_bufctx_del(&dec->bufctx);
   if (dec->push)
      nouveau_pushbuf_destroy(&dec->push);
   if (dec->client)
      nouveau_client_del(&dec->client);
   if (dec->chan)
      nouveau_object_del(&dec->chan);

   FREE(dec);
}

static struct pipe_video_codec *
nouveau_create_decoder(struct pipe_context *context,
                       const struct pipe_video_codec *templ,
                       struct nouveau_screen *screen)
{
   struct nv04_fifo nv04_data = { .vram = 0xbeef0201, .gart = 0xbeef0202 };
   unsigned width = templ->width, height = templ->height;
   struct nouveau_object *mpeg = NULL;
   struct nouveau_decoder *dec;
   struct nouveau_pushbuf *push;
   int ret;
   bool is8274 = screen->device->chipset > 0x80;

   debug_printf("Acceleration level: %s\n", templ->entrypoint <= PIPE_VIDEO_ENTRYPOINT_BITSTREAM ? "bit":
                                            templ->entrypoint == PIPE_VIDEO_ENTRYPOINT_IDCT ? "IDCT" : "MC");

   if (u_reduce_video_profile(templ->profile) != PIPE_VIDEO_FORMAT_MPEG12)
      goto vl;
   if (screen->device->chipset >= 0x98 && screen->device->chipset != 0xa0)
      goto vl;
   if (screen->device->chipset < 0x40)
      goto vl;

   dec = CALLOC_STRUCT(nouveau_decoder);
   if (!dec)
      return NULL;

   ret = nouveau_object_new(&screen->device->object, 0,
                            NOUVEAU_FIFO_CHANNEL_CLASS,
                            &nv04_data, sizeof(nv04_data), &dec->chan);
   if (ret)
      goto fail;
   ret = nouveau_client_new(screen->device, &dec->client);
   if (ret)
      goto fail;
   ret = nouveau_pushbuf_create(screen, nouveau_context(context), dec->client, dec->chan, 2, 4096, 1, &dec->push);
   if (ret)
      goto fail;
   ret = nouveau_bufctx_new(dec->client, NV31_VIDEO_BIND_COUNT, &dec->bufctx);
   if (ret)
      goto fail;
   push = dec->push;

   width = align(width, 64);
   height = align(height, 64);

   if (is8274)
      ret = nouveau_object_new(dec->chan, 0xbeef8274, NV84_MPEG_CLASS, NULL, 0,
                               &mpeg);
   else
      ret = nouveau_object_new(dec->chan, 0xbeef3174, NV31_MPEG_CLASS, NULL, 0,
                               &mpeg);
   if (ret < 0) {
      debug_printf("Creation failed: %s (%i)\n", strerror(-ret), ret);
      goto fail;
   }

   dec->mpeg = mpeg;
   dec->base = *templ;
   dec->base.context = context;
   dec->base.width = width;
   dec->base.height = height;
   dec->base.destroy = nouveau_decoder_destroy;
   dec->base.begin_frame = nouveau_decoder_begin_frame;
   dec->base.decode_macroblock = nouveau_decoder_decode_macroblock;
   dec->base.end_frame = nouveau_decoder_end_frame;
   dec->base.flush = nouveau_decoder_flush;
   dec->screen = screen;

   ret = nouveau_bo_new(dec->screen->device, NOUVEAU_BO_GART | NOUVEAU_BO_MAP,
                        0, 1024 * 1024, NULL, &dec->cmd_bo);
   if (ret)
      goto fail;

   ret = nouveau_bo_new(dec->screen->device, NOUVEAU_BO_GART | NOUVEAU_BO_MAP,
                        0, width * height * 6, NULL, &dec->data_bo);
   if (ret)
      goto fail;

   /* we don't need the fence, the kernel sync's for us */
#if 0
   ret = nouveau_bo_new(dec->screen->device, NOUVEAU_BO_GART | NOUVEAU_BO_MAP,
                        0, 4096, NULL, &dec->fence_bo);
   if (ret)
      goto fail;
   BO_MAP(screen, dec->fence_bo, NOUVEAU_BO_RDWR, NULL);
   dec->fence_map = dec->fence_bo->map;
   dec->fence_map[0] = 0;
#endif

   nouveau_pushbuf_bufctx(dec->push, dec->bufctx);
   PUSH_SPACE_EX(push, 32, 4, 0);

   BEGIN_NV04(push, SUBC_MPEG(NV01_SUBCHAN_OBJECT), 1);
   PUSH_DATA (push, dec->mpeg->handle);

   BEGIN_NV04(push, NV31_MPEG(DMA_CMD), 1);
   PUSH_DATA (push, nv04_data.gart);

   BEGIN_NV04(push, NV31_MPEG(DMA_DATA), 1);
   PUSH_DATA (push, nv04_data.gart);

   BEGIN_NV04(push, NV31_MPEG(DMA_IMAGE), 1);
   PUSH_DATA (push, nv04_data.vram);

   BEGIN_NV04(push, NV31_MPEG(PITCH), 2);
   PUSH_DATA (push, width | NV31_MPEG_PITCH_UNK);
   PUSH_DATA (push, (height << NV31_MPEG_SIZE_H__SHIFT) | width);

   BEGIN_NV04(push, NV31_MPEG(FORMAT), 2);
   PUSH_DATA (push, 0);
   switch (templ->entrypoint) {
      case PIPE_VIDEO_ENTRYPOINT_IDCT: PUSH_DATA (push, 1); break;
      case PIPE_VIDEO_ENTRYPOINT_MC: PUSH_DATA (push, 0); break;
      default: assert(0);
   }

   if (is8274) {
      BEGIN_NV04(push, NV84_MPEG(DMA_QUERY), 1);
      PUSH_DATA (push, nv04_data.vram);
#if 0
      BEGIN_NV04(push, NV84_MPEG(QUERY_OFFSET), 2);
      PUSH_DATA (push, dec->fence_bo->offset);
      PUSH_DATA (push, dec->fence_seq);
#endif
   }

   ret = nouveau_vpe_init(dec);
   if (ret)
      goto fail;
   nouveau_vpe_fini(dec);
   return &dec->base;

fail:
   nouveau_decoder_destroy(&dec->base);
   return NULL;

vl:
   debug_printf("Using g3dvl renderer\n");
   return vl_create_decoder(context, templ);
}

static void
nouveau_video_buffer_resources(struct pipe_video_buffer *buffer,
                               struct pipe_resource **resources)
{
   struct nouveau_video_buffer *buf = (struct nouveau_video_buffer *)buffer;
   unsigned i;

   assert(buf);

   for (i = 0; i < buf->num_planes; ++i) {
      resources[i] = buf->resources[i];
   }
}

static struct pipe_sampler_view **
nouveau_video_buffer_sampler_view_planes(struct pipe_video_buffer *buffer)
{
   struct nouveau_video_buffer *buf = (struct nouveau_video_buffer *)buffer;
   struct pipe_sampler_view sv_templ;
   struct pipe_context *pipe;
   unsigned i;

   assert(buf);

   pipe = buf->base.context;

   for (i = 0; i < buf->num_planes; ++i ) {
      if (!buf->sampler_view_planes[i]) {
         memset(&sv_templ, 0, sizeof(sv_templ));
         u_sampler_view_default_template(&sv_templ, buf->resources[i], buf->resources[i]->format);

         if (util_format_get_nr_components(buf->resources[i]->format) == 1)
            sv_templ.swizzle_r = sv_templ.swizzle_g = sv_templ.swizzle_b = sv_templ.swizzle_a = PIPE_SWIZZLE_X;

         buf->sampler_view_planes[i] = pipe->create_sampler_view(pipe, buf->resources[i], &sv_templ);
         if (!buf->sampler_view_planes[i])
            goto error;
      }
   }

   return buf->sampler_view_planes;

error:
   for (i = 0; i < buf->num_planes; ++i )
      pipe_sampler_view_reference(&buf->sampler_view_planes[i], NULL);

   return NULL;
}

static struct pipe_sampler_view **
nouveau_video_buffer_sampler_view_components(struct pipe_video_buffer *buffer)
{
   struct nouveau_video_buffer *buf = (struct nouveau_video_buffer *)buffer;
   struct pipe_sampler_view sv_templ;
   struct pipe_context *pipe;
   unsigned i, j, component;

   assert(buf);

   pipe = buf->base.context;

   for (component = 0, i = 0; i < buf->num_planes; ++i ) {
      unsigned nr_components = util_format_get_nr_components(buf->resources[i]->format);

      for (j = 0; j < nr_components; ++j, ++component) {
         assert(component < VL_NUM_COMPONENTS);

         if (!buf->sampler_view_components[component]) {
            memset(&sv_templ, 0, sizeof(sv_templ));
            u_sampler_view_default_template(&sv_templ, buf->resources[i], buf->resources[i]->format);
            sv_templ.swizzle_r = sv_templ.swizzle_g = sv_templ.swizzle_b = PIPE_SWIZZLE_X + j;
            sv_templ.swizzle_a = PIPE_SWIZZLE_1;
            buf->sampler_view_components[component] = pipe->create_sampler_view(pipe, buf->resources[i], &sv_templ);
            if (!buf->sampler_view_components[component])
               goto error;
         }
      }
   }

   return buf->sampler_view_components;

error:
   for (i = 0; i < 3; ++i )
      pipe_sampler_view_reference(&buf->sampler_view_components[i], NULL);

   return NULL;
}

static struct pipe_surface **
nouveau_video_buffer_surfaces(struct pipe_video_buffer *buffer)
{
   struct nouveau_video_buffer *buf = (struct nouveau_video_buffer *)buffer;
   struct pipe_surface surf_templ;
   struct pipe_context *pipe;
   unsigned i;

   assert(buf);

   pipe = buf->base.context;

   for (i = 0; i < buf->num_planes; ++i ) {
      if (!buf->surfaces[i]) {
         memset(&surf_templ, 0, sizeof(surf_templ));
         surf_templ.format = buf->resources[i]->format;
         buf->surfaces[i] = pipe->create_surface(pipe, buf->resources[i], &surf_templ);
         if (!buf->surfaces[i])
            goto error;
      }
   }

   return buf->surfaces;

error:
   for (i = 0; i < buf->num_planes; ++i )
      pipe_surface_reference(&buf->surfaces[i], NULL);

   return NULL;
}

static void
nouveau_video_buffer_destroy(struct pipe_video_buffer *buffer)
{
   struct nouveau_video_buffer *buf = (struct nouveau_video_buffer *)buffer;
   unsigned i;

   assert(buf);

   for (i = 0; i < buf->num_planes; ++i) {
      pipe_surface_reference(&buf->surfaces[i], NULL);
      pipe_sampler_view_reference(&buf->sampler_view_planes[i], NULL);
      pipe_sampler_view_reference(&buf->sampler_view_components[i], NULL);
      pipe_resource_reference(&buf->resources[i], NULL);
   }
   for (;i < 3;++i)
      pipe_sampler_view_reference(&buf->sampler_view_components[i], NULL);

   FREE(buffer);
}

static struct pipe_video_buffer *
nouveau_video_buffer_create(struct pipe_context *pipe,
                            struct nouveau_screen *screen,
                            const struct pipe_video_buffer *templat)
{
   struct nouveau_video_buffer *buffer;
   struct pipe_resource templ;
   unsigned width, height;

   /* Only do a linear surface when a hardware decoder is used
    * hardware decoder is only supported on some chipsets
    * and it only supports the NV12 format
    */
   if (templat->buffer_format != PIPE_FORMAT_NV12 ||
       (screen->device->chipset >= 0x98 && screen->device->chipset != 0xa0) ||
       screen->device->chipset < 0x40)
      return vl_video_buffer_create(pipe, templat);

   assert(pipe_format_to_chroma_format(templat->buffer_format) == PIPE_VIDEO_CHROMA_FORMAT_420);
   width = align(templat->width, 64);
   height = align(templat->height, 64);

   buffer = CALLOC_STRUCT(nouveau_video_buffer);
   if (!buffer)
      return NULL;

   buffer->base.context = pipe;
   buffer->base.destroy = nouveau_video_buffer_destroy;
   buffer->base.get_resources = nouveau_video_buffer_resources;
   buffer->base.get_sampler_view_planes = nouveau_video_buffer_sampler_view_planes;
   buffer->base.get_sampler_view_components = nouveau_video_buffer_sampler_view_components;
   buffer->base.get_surfaces = nouveau_video_buffer_surfaces;
   buffer->base.buffer_format = templat->buffer_format;
   buffer->base.width = width;
   buffer->base.height = height;
   buffer->num_planes = 2;

   memset(&templ, 0, sizeof(templ));
   templ.target = PIPE_TEXTURE_2D;
   templ.format = PIPE_FORMAT_R8_UNORM;
   templ.width0 = width;
   templ.height0 = height;
   templ.depth0 = 1;
   templ.array_size = 1;
   templ.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET;
   templ.usage = PIPE_USAGE_DEFAULT;
   templ.flags = NOUVEAU_RESOURCE_FLAG_LINEAR;

   buffer->resources[0] = pipe->screen->resource_create(pipe->screen, &templ);
   if (!buffer->resources[0])
      goto error;
   templ.width0 /= 2;
   templ.height0 /= 2;
   templ.format = PIPE_FORMAT_R8G8_UNORM;
   buffer->resources[1] = pipe->screen->resource_create(pipe->screen, &templ);
   if (!buffer->resources[1])
      goto error;
   return &buffer->base;

error:
   nouveau_video_buffer_destroy(&buffer->base);
   return NULL;
}

static int
nouveau_screen_get_video_param(struct pipe_screen *pscreen,
                               enum pipe_video_profile profile,
                               enum pipe_video_entrypoint entrypoint,
                               enum pipe_video_cap param)
{
   switch (param) {
   case PIPE_VIDEO_CAP_SUPPORTED:
      return entrypoint >= PIPE_VIDEO_ENTRYPOINT_IDCT &&
         u_reduce_video_profile(profile) == PIPE_VIDEO_FORMAT_MPEG12;
   case PIPE_VIDEO_CAP_NPOT_TEXTURES:
      return 1;
   case PIPE_VIDEO_CAP_MAX_WIDTH:
   case PIPE_VIDEO_CAP_MAX_HEIGHT:
      return vl_video_buffer_max_size(pscreen);
   case PIPE_VIDEO_CAP_PREFERED_FORMAT:
      return PIPE_FORMAT_NV12;
   case PIPE_VIDEO_CAP_PREFERS_INTERLACED:
      return false;
   case PIPE_VIDEO_CAP_SUPPORTS_INTERLACED:
      return false;
   case PIPE_VIDEO_CAP_SUPPORTS_PROGRESSIVE:
      return true;
   case PIPE_VIDEO_CAP_MAX_LEVEL:
      return vl_level_supported(pscreen, profile);
   default:
      debug_printf("unknown video param: %d\n", param);
      return 0;
   }
}

void
nouveau_screen_init_vdec(struct nouveau_screen *screen)
{
   screen->base.get_video_param = nouveau_screen_get_video_param;
   screen->base.is_video_format_supported = vl_video_buffer_is_format_supported;
}

static struct pipe_video_codec *
nouveau_context_create_decoder(struct pipe_context *context,
                               const struct pipe_video_codec *templ)
{
   struct nouveau_screen *screen = nouveau_context(context)->screen;
   return nouveau_create_decoder(context, templ, screen);
}

static struct pipe_video_buffer *
nouveau_context_video_buffer_create(struct pipe_context *pipe,
                                    const struct pipe_video_buffer *templat)
{
   struct nouveau_screen *screen = nouveau_context(pipe)->screen;
   return nouveau_video_buffer_create(pipe, screen, templat);
}

void
nouveau_context_init_vdec(struct nouveau_context *nv)
{
   nv->pipe.create_video_codec = nouveau_context_create_decoder;
   nv->pipe.create_video_buffer = nouveau_context_video_buffer_create;
}
