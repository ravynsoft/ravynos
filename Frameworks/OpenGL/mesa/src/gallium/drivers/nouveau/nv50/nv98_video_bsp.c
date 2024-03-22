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

#if NOUVEAU_VP3_DEBUG_FENCE
static void dump_comm_bsp(struct comm *comm)
{
   unsigned idx = comm->bsp_cur_index & 0xf;
   debug_printf("Cur seq: %x, bsp byte ofs: %x\n", comm->bsp_cur_index, comm->byte_ofs);
   debug_printf("Status: %08x, pos: %08x\n", comm->status[idx], comm->pos[idx]);
}
#endif

unsigned
nv98_decoder_bsp(struct nouveau_vp3_decoder *dec, union pipe_desc desc,
                 struct nouveau_vp3_video_buffer *target,
                 unsigned comm_seq, unsigned num_buffers,
                 const void *const *data, const unsigned *num_bytes,
                 unsigned *vp_caps, unsigned *is_ref,
                 struct nouveau_vp3_video_buffer *refs[16])
{
   struct nouveau_screen *screen = nouveau_screen(dec->base.context->screen);
   struct nouveau_pushbuf *push = dec->pushbuf[0];
   enum pipe_video_format codec = u_reduce_video_profile(dec->base.profile);
   uint32_t bsp_addr, comm_addr, inter_addr;
   uint32_t slice_size, bucket_size, ring_size, bsp_size;
   uint32_t caps, i;
   int ret;
   struct nouveau_bo *bsp_bo = dec->bsp_bo[comm_seq % NOUVEAU_VP3_VIDEO_QDEPTH];
   struct nouveau_bo *inter_bo = dec->inter_bo[comm_seq & 1];
   struct nouveau_pushbuf_refn bo_refs[] = {
      { bsp_bo, NOUVEAU_BO_RD | NOUVEAU_BO_VRAM },
      { inter_bo, NOUVEAU_BO_WR | NOUVEAU_BO_VRAM },
#if NOUVEAU_VP3_DEBUG_FENCE
      { dec->fence_bo, NOUVEAU_BO_WR | NOUVEAU_BO_GART },
#endif
      { dec->bitplane_bo, NOUVEAU_BO_RDWR | NOUVEAU_BO_VRAM },
   };
   int num_refs = ARRAY_SIZE(bo_refs);

   if (!dec->bitplane_bo)
      num_refs--;

   bsp_size = NOUVEAU_VP3_BSP_RESERVED_SIZE;
   for (i = 0; i < num_buffers; i++)
      bsp_size += num_bytes[i];
   bsp_size += 256; /* the 4 end markers */

   if (!bsp_bo || bsp_size > bsp_bo->size) {
      struct nouveau_bo *tmp_bo = NULL;

      /* round up to the nearest mb */
      bsp_size += (1 << 20) - 1;
      bsp_size &= ~((1 << 20) - 1);

      ret = nouveau_bo_new(dec->client->device, NOUVEAU_BO_VRAM, 0, bsp_size, NULL, &tmp_bo);
      if (ret) {
         debug_printf("reallocating bsp %u -> %u failed with %i\n",
                      bsp_bo ? (unsigned)bsp_bo->size : 0, bsp_size, ret);
         return -1;
      }
      nouveau_bo_ref(NULL, &bsp_bo);
      bo_refs[0].bo = dec->bsp_bo[comm_seq % NOUVEAU_VP3_VIDEO_QDEPTH] = bsp_bo = tmp_bo;
   }

   if (!inter_bo || bsp_bo->size * 4 > inter_bo->size) {
      struct nouveau_bo *tmp_bo = NULL;

      ret = nouveau_bo_new(dec->client->device, NOUVEAU_BO_VRAM, 0, bsp_bo->size * 4, NULL, &tmp_bo);
      if (ret) {
         debug_printf("reallocating inter %u -> %u failed with %i\n",
                      inter_bo ? (unsigned)inter_bo->size : 0, (unsigned)bsp_bo->size * 4, ret);
         return -1;
      }
      nouveau_bo_ref(NULL, &inter_bo);
      bo_refs[1].bo = dec->inter_bo[comm_seq & 1] = inter_bo = tmp_bo;
   }

   ret = BO_MAP(screen, bsp_bo, NOUVEAU_BO_WR, dec->client);
   if (ret) {
      debug_printf("map failed: %i %s\n", ret, strerror(-ret));
      return -1;
   }

   nouveau_vp3_bsp_begin(dec);
   nouveau_vp3_bsp_next(dec, num_buffers, data, num_bytes);
   caps = nouveau_vp3_bsp_end(dec, desc);

   nouveau_vp3_vp_caps(dec, desc, target, comm_seq, vp_caps, is_ref, refs);

   PUSH_SPACE_EX(push, 32, num_refs, 0);
   PUSH_REFN(push, bo_refs, num_refs);

   bsp_addr = bsp_bo->offset >> 8;
   inter_addr = inter_bo->offset >> 8;

#if NOUVEAU_VP3_DEBUG_FENCE
   memset(dec->comm, 0, 0x200);
   comm_addr = (dec->fence_bo->offset + COMM_OFFSET) >> 8;
#else
   comm_addr = bsp_addr + (COMM_OFFSET>>8);
#endif

   BEGIN_NV04(push, SUBC_BSP(0x700), 5);
   PUSH_DATA (push, caps); // 700 cmd
   PUSH_DATA (push, bsp_addr + 1); // 704 strparm_bsp
   PUSH_DATA (push, bsp_addr + 7); // 708 str addr
   PUSH_DATA (push, comm_addr); // 70c comm
   PUSH_DATA (push, comm_seq); // 710 seq

   if (codec != PIPE_VIDEO_FORMAT_MPEG4_AVC) {
      u32 bitplane_addr;
      int mpeg12 = (codec == PIPE_VIDEO_FORMAT_MPEG12);

      bitplane_addr = dec->bitplane_bo->offset >> 8;

      nouveau_vp3_inter_sizes(dec, 1, &slice_size, &bucket_size, &ring_size);
      BEGIN_NV04(push, SUBC_BSP(0x400), mpeg12 ? 5 : 7);
      PUSH_DATA (push, bsp_addr); // 400 picparm addr
      PUSH_DATA (push, inter_addr); // 404 interparm addr
      PUSH_DATA (push, inter_addr + slice_size + bucket_size); // 408 interdata addr
      PUSH_DATA (push, ring_size << 8); // 40c interdata_size
      if (!mpeg12) {
         PUSH_DATA (push, bitplane_addr); // 410 BITPLANE_DATA
         PUSH_DATA (push, 0x400); // 414 BITPLANE_DATA_SIZE
      }
      PUSH_DATA (push, 0); // dma idx
   } else {
      nouveau_vp3_inter_sizes(dec, desc.h264->slice_count, &slice_size, &bucket_size, &ring_size);
      BEGIN_NV04(push, SUBC_BSP(0x400), 8);
      PUSH_DATA (push, bsp_addr); // 400 picparm addr
      PUSH_DATA (push, inter_addr); // 404 interparm addr
      PUSH_DATA (push, slice_size << 8); // 408 interparm size?
      PUSH_DATA (push, inter_addr + slice_size + bucket_size); // 40c interdata addr
      PUSH_DATA (push, ring_size << 8); // 410 interdata size
      PUSH_DATA (push, inter_addr + slice_size); // 414 bucket?
      PUSH_DATA (push, bucket_size << 8); // 418 bucket size? unshifted..
      PUSH_DATA (push, 0); // 41c targets
      // TODO: Double check 414 / 418 with nvidia trace
   }

#if NOUVEAU_VP3_DEBUG_FENCE
   BEGIN_NV04(push, SUBC_BSP(0x240), 3);
   PUSH_DATAh(push, dec->fence_bo->offset);
   PUSH_DATA (push, dec->fence_bo->offset);
   PUSH_DATA (push, dec->fence_seq);

   BEGIN_NV04(push, SUBC_BSP(0x300), 1);
   PUSH_DATA (push, 1);
   PUSH_KICK (push);

   {
      unsigned spin = 0;
      do {
         usleep(100);
         if ((spin++ & 0xff) == 0xff) {
            debug_printf("b%u: %u\n", dec->fence_seq, dec->fence_map[0]);
            dump_comm_bsp(dec->comm);
         }
      } while (dec->fence_seq > dec->fence_map[0]);
   }

   dump_comm_bsp(dec->comm);
   return dec->comm->status[comm_seq & 0xf];
#else
   BEGIN_NV04(push, SUBC_BSP(0x300), 1);
   PUSH_DATA (push, 0);
   PUSH_KICK (push);
   return 2;
#endif
}
