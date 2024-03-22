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
#include <sys/mman.h>

#if NOUVEAU_VP3_DEBUG_FENCE
static void dump_comm_vp(struct nouveau_vp3_decoder *dec, struct comm *comm, u32 comm_seq,
                         struct nouveau_bo *inter_bo, unsigned slice_size)
{
   unsigned i, idx = comm->pvp_cur_index & 0xf;
   debug_printf("Status: %08x, stage: %08x\n", comm->status_vp[idx], comm->pvp_stage);
#if 0
   debug_printf("Acked byte ofs: %x, bsp byte ofs: %x\n", comm->acked_byte_ofs, comm->byte_ofs);
   debug_printf("Irq/parse indexes: %i %i\n", comm->irq_index, comm->parse_endpos_index);

   for (i = 0; i != comm->irq_index; ++i)
      debug_printf("irq[%i] = { @ %08x -> %04x }\n", i, comm->irq_pos[i], comm->irq_470[i]);
   for (i = 0; i != comm->parse_endpos_index; ++i)
      debug_printf("parse_endpos[%i] = { @ %08x}\n", i, comm->parse_endpos[i]);
#endif
   debug_printf("mb_y = %u\n", comm->mb_y[idx]);
   if (comm->status_vp[idx] <= 1)
      return;

   if ((comm->pvp_stage & 0xff) != 0xff) {
      unsigned *map;
      int ret = BO_MAP(nouveau_screen(dec->base.context->screen), inter_bo,
                       NOUVEAU_BO_RD|NOUVEAU_BO_NOBLOCK, dec->client);
      assert(ret >= 0);
      map = inter_bo->map;
      for (i = 0; i < comm->byte_ofs + slice_size; i += 0x10) {
         debug_printf("%05x: %08x %08x %08x %08x\n", i, map[i/4], map[i/4+1], map[i/4+2], map[i/4+3]);
      }
      munmap(inter_bo->map, inter_bo->size);
      inter_bo->map = NULL;
   }
   assert((comm->pvp_stage & 0xff) == 0xff);
}
#endif

static void
nvc0_decoder_kick_ref(struct nouveau_vp3_decoder *dec, struct nouveau_vp3_video_buffer *target)
{
   dec->refs[target->valid_ref].last_used = 0;
//   debug_printf("Unreffed %p\n", target);
}

void
nvc0_decoder_vp(struct nouveau_vp3_decoder *dec, union pipe_desc desc,
                struct nouveau_vp3_video_buffer *target, unsigned comm_seq,
                unsigned caps, unsigned is_ref,
                struct nouveau_vp3_video_buffer *refs[16])
{
   struct nouveau_pushbuf *push = dec->pushbuf[1];
   uint32_t bsp_addr, comm_addr, inter_addr, ucode_addr, pic_addr[17], last_addr, null_addr;
   uint32_t slice_size, bucket_size, ring_size, i;
   enum pipe_video_format codec = u_reduce_video_profile(dec->base.profile);
   struct nouveau_bo *bsp_bo = dec->bsp_bo[comm_seq % NOUVEAU_VP3_VIDEO_QDEPTH];
   struct nouveau_bo *inter_bo = dec->inter_bo[comm_seq & 1];
   u32 codec_extra = 0;
   struct nouveau_pushbuf_refn bo_refs[] = {
      { inter_bo, NOUVEAU_BO_WR | NOUVEAU_BO_VRAM },
      { dec->ref_bo, NOUVEAU_BO_WR | NOUVEAU_BO_VRAM },
      { bsp_bo, NOUVEAU_BO_RD | NOUVEAU_BO_VRAM },
#if NOUVEAU_VP3_DEBUG_FENCE
      { dec->fence_bo, NOUVEAU_BO_WR | NOUVEAU_BO_GART },
#endif
      { dec->fw_bo, NOUVEAU_BO_RD | NOUVEAU_BO_VRAM },
   };
   int num_refs = ARRAY_SIZE(bo_refs) - !dec->fw_bo;

   if (codec == PIPE_VIDEO_FORMAT_MPEG4_AVC) {
      nouveau_vp3_inter_sizes(dec, desc.h264->slice_count, &slice_size, &bucket_size, &ring_size);
      codec_extra += 2;
   } else
      nouveau_vp3_inter_sizes(dec, 1, &slice_size, &bucket_size, &ring_size);

   if (dec->base.max_references > 2)
      codec_extra += 1 + (dec->base.max_references - 2);

   pic_addr[16] = nouveau_vp3_video_addr(dec, target) >> 8;
   last_addr = null_addr = nouveau_vp3_video_addr(dec, NULL) >> 8;

   for (i = 0; i < dec->base.max_references; ++i) {
      if (!refs[i])
         pic_addr[i] = last_addr;
      else if (dec->refs[refs[i]->valid_ref].vidbuf == refs[i])
         last_addr = pic_addr[i] = nouveau_vp3_video_addr(dec, refs[i]) >> 8;
      else
         pic_addr[i] = null_addr;
   }
   if (!is_ref && (dec->refs[target->valid_ref].decoded_top && dec->refs[target->valid_ref].decoded_bottom))
      nvc0_decoder_kick_ref(dec, target);

   PUSH_SPACE_EX(push, 32 + codec_extra, num_refs, 0);

   PUSH_REFN(push, bo_refs, num_refs);

   bsp_addr = bsp_bo->offset >> 8;
#if NOUVEAU_VP3_DEBUG_FENCE
   comm_addr = (dec->fence_bo->offset + COMM_OFFSET)>>8;
#else
   comm_addr = bsp_addr + (COMM_OFFSET>>8);
#endif
   inter_addr = inter_bo->offset >> 8;
   if (dec->fw_bo)
      ucode_addr = dec->fw_bo->offset >> 8;
   else
      ucode_addr = 0;

   BEGIN_NVC0(push, SUBC_VP(0x700), 7);
   PUSH_DATA (push, caps); // 700
   PUSH_DATA (push, comm_seq); // 704
   PUSH_DATA (push, 0); // 708 fuc targets, ignored for nvc0
   PUSH_DATA (push, dec->fw_sizes); // 70c
   PUSH_DATA (push, bsp_addr+(VP_OFFSET>>8)); // 710 picparm_addr
   PUSH_DATA (push, inter_addr); // 714 inter_parm
   PUSH_DATA (push, inter_addr + slice_size + bucket_size); // 718 inter_data_ofs

   if (bucket_size) {
      uint64_t tmpimg_addr = dec->ref_bo->offset + dec->ref_stride * (dec->base.max_references+2);

      BEGIN_NVC0(push, SUBC_VP(0x71c), 2);
      PUSH_DATA (push, tmpimg_addr >> 8); // 71c
      PUSH_DATA (push, inter_addr + slice_size); // 720 bucket_ofs
   }

   BEGIN_NVC0(push, SUBC_VP(0x724), 5);
   PUSH_DATA (push, comm_addr); // 724
   PUSH_DATA (push, ucode_addr); // 728
   PUSH_DATA (push, pic_addr[16]); // 734
   PUSH_DATA (push, pic_addr[0]); // 72c
   PUSH_DATA (push, pic_addr[1]); // 730

   if (dec->base.max_references > 2) {
      int i;

      BEGIN_NVC0(push, SUBC_VP(0x400), dec->base.max_references - 2);
      for (i = 2; i < dec->base.max_references; ++i) {
         assert(0x400 + (i - 2) * 4 < 0x438);
         PUSH_DATA (push, pic_addr[i]);
      }
   }

   if (codec == PIPE_VIDEO_FORMAT_MPEG4_AVC) {
      BEGIN_NVC0(push, SUBC_VP(0x438), 1);
      PUSH_DATA (push, desc.h264->slice_count);
   }

   //debug_printf("Decoding %08lx with %08lx and %08lx\n", pic_addr[16], pic_addr[0], pic_addr[1]);

#if NOUVEAU_VP3_DEBUG_FENCE
   BEGIN_NVC0(push, SUBC_VP(0x240), 3);
   PUSH_DATAh(push, (dec->fence_bo->offset + 0x10));
   PUSH_DATA (push, (dec->fence_bo->offset + 0x10));
   PUSH_DATA (push, dec->fence_seq);

   BEGIN_NVC0(push, SUBC_VP(0x300), 1);
   PUSH_DATA (push, 1);
   PUSH_KICK(push);

   {
      unsigned spin = 0;
      do {
         usleep(100);
         if ((spin++ & 0xff) == 0xff) {
            debug_printf("v%u: %u\n", dec->fence_seq, dec->fence_map[4]);
            dump_comm_vp(dec, dec->comm, comm_seq, inter_bo, slice_size << 8);
         }
      } while (dec->fence_seq > dec->fence_map[4]);
   }
   dump_comm_vp(dec, dec->comm, comm_seq, inter_bo, slice_size << 8);
#else
   BEGIN_NVC0(push, SUBC_VP(0x300), 1);
   PUSH_DATA (push, 0);
   PUSH_KICK (push);
#endif
}
