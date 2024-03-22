/*
 * Copyright 2013 Ilia Mirkin
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

#include "nv50/nv84_video.h"

struct iparm {
   struct iseqparm {
      uint32_t chroma_format_idc; // 00
      uint32_t pad[(0x128 - 0x4) / 4];
      uint32_t log2_max_frame_num_minus4; // 128
      uint32_t pic_order_cnt_type; // 12c
      uint32_t log2_max_pic_order_cnt_lsb_minus4; // 130
      uint32_t delta_pic_order_always_zero_flag; // 134
      uint32_t num_ref_frames; // 138
      uint32_t pic_width_in_mbs_minus1; // 13c
      uint32_t pic_height_in_map_units_minus1; // 140
      uint32_t frame_mbs_only_flag; // 144
      uint32_t mb_adaptive_frame_field_flag; // 148
      uint32_t direct_8x8_inference_flag; // 14c
   } iseqparm; // 000
   struct ipicparm {
      uint32_t entropy_coding_mode_flag; // 00
      uint32_t pic_order_present_flag; // 04
      uint32_t num_slice_groups_minus1; // 08
      uint32_t slice_group_map_type; // 0c
      uint32_t pad1[0x60 / 4];
      uint32_t u70; // 70
      uint32_t u74; // 74
      uint32_t u78; // 78
      uint32_t num_ref_idx_l0_active_minus1; // 7c
      uint32_t num_ref_idx_l1_active_minus1; // 80
      uint32_t weighted_pred_flag; // 84
      uint32_t weighted_bipred_idc; // 88
      uint32_t pic_init_qp_minus26; // 8c
      uint32_t chroma_qp_index_offset; // 90
      uint32_t deblocking_filter_control_present_flag; // 94
      uint32_t constrained_intra_pred_flag; // 98
      uint32_t redundant_pic_cnt_present_flag; // 9c
      uint32_t transform_8x8_mode_flag; // a0
      uint32_t pad2[(0x1c8 - 0xa0 - 4) / 4];
      uint32_t second_chroma_qp_index_offset; // 1c8
      uint32_t u1cc; // 1cc
      uint32_t curr_pic_order_cnt; // 1d0
      uint32_t field_order_cnt[2]; // 1d4
      uint32_t curr_mvidx; // 1dc
      struct iref {
         uint32_t u00; // 00
         uint32_t field_is_ref; // 04 // bit0: top, bit1: bottom
         uint8_t is_long_term; // 08
         uint8_t non_existing; // 09
         uint8_t u0a; // 0a
         uint8_t u0b; // 0b
         uint32_t frame_idx; // 0c
         uint32_t field_order_cnt[2]; // 10
         uint32_t mvidx; // 18
         uint8_t field_pic_flag; // 1c
         uint8_t u1d; // 1d
         uint8_t u1e; // 1e
         uint8_t u1f; // 1f
         // 20
      } refs[0x10]; // 1e0
   } ipicparm; // 150
};

int
nv84_decoder_bsp(struct nv84_decoder *dec,
                 struct pipe_h264_picture_desc *desc,
                 unsigned num_buffers,
                 const void *const *data,
                 const unsigned *num_bytes,
                 struct nv84_video_buffer *dest)
{
   struct nouveau_screen *screen = nouveau_screen(dec->base.context->screen);
   struct iparm params;
   uint32_t more_params[0x44 / 4] = {0};
   unsigned total_bytes = 0;
   int i;
   static const uint32_t end[] = {0x0b010000, 0, 0x0b010000, 0};
   char indexes[17] = {0};
   struct nouveau_pushbuf *push = dec->bsp_pushbuf;
   struct nouveau_pushbuf_refn bo_refs[] = {
      { dec->vpring, NOUVEAU_BO_RDWR | NOUVEAU_BO_VRAM },
      { dec->mbring, NOUVEAU_BO_RDWR | NOUVEAU_BO_VRAM },
      { dec->bitstream, NOUVEAU_BO_RDWR | NOUVEAU_BO_GART },
      { dec->fence, NOUVEAU_BO_RDWR | NOUVEAU_BO_VRAM },
   };

   BO_WAIT(screen, dec->fence, NOUVEAU_BO_RDWR, dec->client);

   STATIC_ASSERT(sizeof(struct iparm) == 0x530);

   memset(&params, 0, sizeof(params));

   dest->frame_num = dest->frame_num_max = desc->frame_num;

   for (i = 0; i < 16; i++) {
      struct iref *ref = &params.ipicparm.refs[i];
      struct nv84_video_buffer *frame = (struct nv84_video_buffer *)desc->ref[i];
      if (!frame) break;
      /* The frame index is relative to the last IDR frame. So once the frame
       * num goes back to 0, previous reference frames need to have a negative
       * index.
       */
      if (desc->frame_num >= frame->frame_num_max) {
         frame->frame_num_max = desc->frame_num;
      } else {
         frame->frame_num -= frame->frame_num_max + 1;
         frame->frame_num_max = desc->frame_num;
      }
      ref->non_existing = 0;
      ref->field_is_ref = (desc->top_is_reference[i] ? 1 : 0) |
         (desc->bottom_is_reference[i] ? 2 : 0);
      ref->is_long_term = desc->is_long_term[i];
      ref->field_order_cnt[0] = desc->field_order_cnt_list[i][0];
      ref->field_order_cnt[1] = desc->field_order_cnt_list[i][1];
      ref->frame_idx = frame->frame_num;
      ref->u00 = ref->mvidx = frame->mvidx;
      ref->field_pic_flag = desc->field_pic_flag;
      indexes[frame->mvidx] = 1;
   }

   /* Needs to be adjusted if we ever support non-4:2:0 videos */
   params.iseqparm.chroma_format_idc = 1;

   params.iseqparm.pic_width_in_mbs_minus1 = mb(dec->base.width) - 1;
   if (desc->field_pic_flag || desc->pps->sps->mb_adaptive_frame_field_flag)
      params.iseqparm.pic_height_in_map_units_minus1 = mb_half(dec->base.height) - 1;
   else
      params.iseqparm.pic_height_in_map_units_minus1 = mb(dec->base.height) - 1;

   if (desc->bottom_field_flag)
      params.ipicparm.curr_pic_order_cnt = desc->field_order_cnt[1];
   else
      params.ipicparm.curr_pic_order_cnt = desc->field_order_cnt[0];
   params.ipicparm.field_order_cnt[0] = desc->field_order_cnt[0];
   params.ipicparm.field_order_cnt[1] = desc->field_order_cnt[1];
   if (desc->is_reference) {
      if (dest->mvidx < 0) {
         for (i = 0; i < desc->num_ref_frames + 1; i++) {
            if (!indexes[i]) {
               dest->mvidx = i;
               break;
            }
         }
         assert(i != desc->num_ref_frames + 1);
      }

      params.ipicparm.u1cc = params.ipicparm.curr_mvidx = dest->mvidx;
   }

   params.iseqparm.num_ref_frames = desc->num_ref_frames;
   params.iseqparm.mb_adaptive_frame_field_flag = desc->pps->sps->mb_adaptive_frame_field_flag;
   params.ipicparm.constrained_intra_pred_flag = desc->pps->constrained_intra_pred_flag;
   params.ipicparm.weighted_pred_flag = desc->pps->weighted_pred_flag;
   params.ipicparm.weighted_bipred_idc = desc->pps->weighted_bipred_idc;
   params.iseqparm.frame_mbs_only_flag = desc->pps->sps->frame_mbs_only_flag;
   params.ipicparm.transform_8x8_mode_flag = desc->pps->transform_8x8_mode_flag;
   params.ipicparm.chroma_qp_index_offset = desc->pps->chroma_qp_index_offset;
   params.ipicparm.second_chroma_qp_index_offset = desc->pps->second_chroma_qp_index_offset;
   params.ipicparm.pic_init_qp_minus26 = desc->pps->pic_init_qp_minus26;
   params.ipicparm.num_ref_idx_l0_active_minus1 = desc->num_ref_idx_l0_active_minus1;
   params.ipicparm.num_ref_idx_l1_active_minus1 = desc->num_ref_idx_l1_active_minus1;
   params.iseqparm.log2_max_frame_num_minus4 = desc->pps->sps->log2_max_frame_num_minus4;
   params.iseqparm.pic_order_cnt_type = desc->pps->sps->pic_order_cnt_type;
   params.iseqparm.log2_max_pic_order_cnt_lsb_minus4 = desc->pps->sps->log2_max_pic_order_cnt_lsb_minus4;
   params.iseqparm.delta_pic_order_always_zero_flag = desc->pps->sps->delta_pic_order_always_zero_flag;
   params.iseqparm.direct_8x8_inference_flag = desc->pps->sps->direct_8x8_inference_flag;
   params.ipicparm.entropy_coding_mode_flag = desc->pps->entropy_coding_mode_flag;
   params.ipicparm.pic_order_present_flag = desc->pps->bottom_field_pic_order_in_frame_present_flag;
   params.ipicparm.deblocking_filter_control_present_flag = desc->pps->deblocking_filter_control_present_flag;
   params.ipicparm.redundant_pic_cnt_present_flag = desc->pps->redundant_pic_cnt_present_flag;

   memcpy(dec->bitstream->map, &params, sizeof(params));
   for (i = 0; i < num_buffers; i++) {
      assert(total_bytes + num_bytes[i] < dec->bitstream->size / 2 - 0x700);
      memcpy(dec->bitstream->map + 0x700 + total_bytes, data[i], num_bytes[i]);
      total_bytes += num_bytes[i];
   }
   memcpy(dec->bitstream->map + 0x700 + total_bytes, end, sizeof(end));
   total_bytes += sizeof(end);
   more_params[1] = total_bytes;
   memcpy(dec->bitstream->map + 0x600, more_params, sizeof(more_params));

   PUSH_SPACE(push, 5 + 21 + 3 + 2 + 4 + 2);
   PUSH_REFN(push, bo_refs, ARRAY_SIZE(bo_refs));

   /* Wait for the fence = 1 */
   BEGIN_NV04(push, SUBC_BSP(0x10), 4);
   PUSH_DATAh(push, dec->fence->offset);
   PUSH_DATA (push, dec->fence->offset);
   PUSH_DATA (push, 1);
   PUSH_DATA (push, 1);

   /* TODO: Use both halves of bitstream/vpring for alternating frames */

   /* Kick off the BSP */
   BEGIN_NV04(push, SUBC_BSP(0x400), 20);
   PUSH_DATA (push, dec->bitstream->offset >> 8);
   PUSH_DATA (push, (dec->bitstream->offset >> 8) + 7);
   PUSH_DATA (push, dec->bitstream->size / 2 - 0x700);
   PUSH_DATA (push, (dec->bitstream->offset >> 8) + 6);
   PUSH_DATA (push, 1);
   PUSH_DATA (push, dec->mbring->offset >> 8);
   PUSH_DATA (push, dec->frame_size);
   PUSH_DATA (push, (dec->mbring->offset + dec->frame_size) >> 8);
   PUSH_DATA (push, dec->vpring->offset >> 8);
   PUSH_DATA (push, dec->vpring->size / 2);
   PUSH_DATA (push, dec->vpring_residual);
   PUSH_DATA (push, dec->vpring_ctrl);
   PUSH_DATA (push, 0);
   PUSH_DATA (push, dec->vpring_residual);
   PUSH_DATA (push, dec->vpring_residual + dec->vpring_ctrl);
   PUSH_DATA (push, dec->vpring_deblock);
   PUSH_DATA (push, (dec->vpring->offset + dec->vpring_ctrl +
                     dec->vpring_residual + dec->vpring_deblock) >> 8);
   PUSH_DATA (push, 0x654321);
   PUSH_DATA (push, 0);
   PUSH_DATA (push, 0x100008);

   BEGIN_NV04(push, SUBC_BSP(0x620), 2);
   PUSH_DATA (push, 0);
   PUSH_DATA (push, 0);

   BEGIN_NV04(push, SUBC_BSP(0x300), 1);
   PUSH_DATA (push, 0);

   /* Write fence = 2, intr */
   BEGIN_NV04(push, SUBC_BSP(0x610), 3);
   PUSH_DATAh(push, dec->fence->offset);
   PUSH_DATA (push, dec->fence->offset);
   PUSH_DATA (push, 2);

   BEGIN_NV04(push, SUBC_BSP(0x304), 1);
   PUSH_DATA (push, 0x101);
   PUSH_KICK (push);
   return 0;
}
