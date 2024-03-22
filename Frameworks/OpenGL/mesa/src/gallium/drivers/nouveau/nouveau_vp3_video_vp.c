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

#include "nouveau_vp3_video.h"

struct mpeg12_picparm_vp {
   uint16_t width; // 00 in mb units
   uint16_t height; // 02 in mb units

   uint32_t unk04; // 04 stride for Y?
   uint32_t unk08; // 08 stride for CbCr?

   uint32_t ofs[6]; // 1c..20 ofs
   uint32_t bucket_size; // 24
   uint32_t inter_ring_data_size; // 28
   uint16_t unk2c; // 2c
   uint16_t alternate_scan; // 2e
   uint16_t unk30; // 30 not seen set yet
   uint16_t picture_structure; // 32
   uint16_t pad2[3];
   uint16_t unk3a; // 3a set on I frame?

   uint32_t f_code[4]; // 3c
   uint32_t picture_coding_type; // 4c
   uint32_t intra_dc_precision; // 50
   uint32_t q_scale_type; // 54
   uint32_t top_field_first; // 58
   uint32_t full_pel_forward_vector; // 5c
   uint32_t full_pel_backward_vector; // 60
   uint8_t intra_quantizer_matrix[0x40]; // 64
   uint8_t non_intra_quantizer_matrix[0x40]; // a4
};

struct mpeg4_picparm_vp {
   uint32_t width; // 00 in normal units
   uint32_t height; // 04 in normal units
   uint32_t unk08; // stride 1
   uint32_t unk0c; // stride 2
   uint32_t ofs[6]; // 10..24 ofs
   uint32_t bucket_size; // 28
   uint32_t pad1; // 2c, pad
   uint32_t pad2; // 30
   uint32_t inter_ring_data_size; // 34

   uint32_t trd[2]; // 38, 3c
   uint32_t trb[2]; // 40, 44
   uint32_t u48; // XXX codec selection? Should test with different values of VdpDecoderProfile
   uint16_t f_code_fw; // 4c
   uint16_t f_code_bw; // 4e
   uint8_t interlaced; // 50

   uint8_t quant_type; // bool, written to 528
   uint8_t quarter_sample; // bool, written to 548
   uint8_t short_video_header; // bool, negated written to 528 shifted by 1
   uint8_t u54; // bool, written to 0x740
   uint8_t vop_coding_type; // 55
   uint8_t rounding_control; // 56
   uint8_t alternate_vertical_scan_flag; // 57 bool
   uint8_t top_field_first; // bool, written to vuc

   uint8_t pad4[3]; // 59, 5a, 5b, contains garbage on blob

   uint32_t intra[0x10]; // 5c
   uint32_t non_intra[0x10]; // 9c
   uint32_t pad5[0x10]; // bc what does this do?
   // udc..uff pad?
};

// Full version, with data pumped from BSP
struct vc1_picparm_vp {
   uint32_t bucket_size; // 00
   uint32_t pad; // 04

   uint32_t inter_ring_data_size; // 08
   uint32_t unk0c; // stride 1
   uint32_t unk10; // stride 2
   uint32_t ofs[6]; // 14..28 ofs

   uint16_t width; // 2c
   uint16_t height; // 2e

   uint8_t profile; // 30 0 = simple, 1 = main, 2 = advanced
   uint8_t loopfilter; // 31 written into vuc
   uint8_t fastuvmc; // 32, written into vuc
   uint8_t dquant; // 33

   uint8_t overlap; // 34
   uint8_t quantizer; // 35
   uint8_t u36; // 36, bool
   uint8_t pad2; // 37, to align to 0x38
};

struct h264_picparm_vp { // 700..a00
   uint16_t width, height;
   uint32_t stride1, stride2; // 04 08
   uint32_t ofs[6]; // 0c..24 in-image offset

   uint32_t tmp_stride;
   uint32_t bucket_size; // 28 bucket size
   uint32_t inter_ring_data_size; // 2c

   unsigned mb_adaptive_frame_field_flag : 1; // 0
   unsigned direct_8x8_inference_flag : 1; // 1 0x02: into vuc ofs 56
   unsigned weighted_pred_flag : 1; // 2 0x04
   unsigned constrained_intra_pred_flag : 1; // 3 0x08: into vuc ofs 68
   unsigned is_reference : 1; // 4
   unsigned interlace : 1; // 5 field_pic_flag
   unsigned bottom_field_flag : 1; // 6
   unsigned second_field : 1; // 7 0x80: nfi yet

   signed log2_max_frame_num_minus4 : 4; // 31 0..3
   unsigned chroma_format_idc : 2; // 31 4..5
   unsigned pic_order_cnt_type : 2; // 31 6..7
   signed pic_init_qp_minus26 : 6; // 32 0..5
   signed chroma_qp_index_offset : 5; // 32 6..10
   signed second_chroma_qp_index_offset : 5; // 32 11..15

   unsigned weighted_bipred_idc : 2; // 34 0..1
   unsigned fifo_dec_index : 7; // 34 2..8
   unsigned tmp_idx : 5; // 34 9..13
   unsigned frame_number : 16; // 34 14..29
   unsigned u34_3030 : 1; // 34 30..30 pp.u34[30:30]
   unsigned u34_3131 : 1; // 34 31..31 pad?

   uint32_t field_order_cnt[2]; // 38, 3c

   struct { // 40
      unsigned fifo_idx : 7; // 00 0..6
      unsigned tmp_idx : 5; // 00 7..11
      unsigned top_is_reference : 1; // 00 12
      unsigned bottom_is_reference : 1; // 00 13
      unsigned is_long_term : 1; // 00 14
      unsigned notseenyet : 1; // 00 15 pad?
      unsigned field_pic_flag : 1; // 00 16
      unsigned top_field_marking : 4; // 00 17..20
      unsigned bottom_field_marking : 4; // 00 21..24
      unsigned pad : 7; // 00 d25..31

      uint32_t field_order_cnt[2]; // 04,08
      uint32_t frame_idx; // 0c
   } refs[0x10];

   uint8_t m4x4[6][16]; // 140
   uint8_t m8x8[2][64]; // 1a0
   uint32_t u220; // 220 number of extra reorder_list to append?
   uint8_t u224[0x20]; // 224..244 reorder_list append ?
   uint8_t nfi244[0xb0]; // add some pad to make sure nulls are read
};

static void
nouveau_vp3_handle_references(struct nouveau_vp3_decoder *dec, struct nouveau_vp3_video_buffer *refs[16], unsigned seq, struct nouveau_vp3_video_buffer *target)
{
   unsigned i, idx, empty_spot = ~0;

   for (i = 0; i < dec->base.max_references; ++i) {
      if (!refs[i])
         continue;

      idx = refs[i]->valid_ref;
      //debug_printf("ref[%i] %p in slot %i\n", i, refs[i], idx);

      if (dec->refs[idx].vidbuf != refs[i]) {
         debug_printf("%p is not a real ref\n", refs[i]);
         // FIXME: Maybe do m2mf copy here if a application really depends on it?
         continue;
      }

      assert(dec->refs[idx].vidbuf == refs[i]);
      dec->refs[idx].last_used = seq;
   }

   if (dec->refs[target->valid_ref].vidbuf == target) {
      dec->refs[target->valid_ref].last_used = seq;
      return;
   }

   /* Try to find a real empty spot first, there should be one..
    */
   for (i = 0; i < dec->base.max_references + 1; ++i) {
      if (dec->refs[i].vidbuf == target) {
         empty_spot = i;
         break;
      } else if (!dec->refs[i].last_used) {
         empty_spot = i;
      } else if (empty_spot == ~0U && dec->refs[i].last_used != seq)
         empty_spot = i;
   }

   assert(empty_spot < dec->base.max_references+1);
   dec->refs[empty_spot].last_used = seq;
//   debug_printf("Kicked %p to add %p to slot %i\n", dec->refs[empty_spot].vidbuf, target, empty_spot);
   dec->refs[empty_spot].vidbuf = target;
   dec->refs[empty_spot].decoded_bottom = dec->refs[empty_spot].decoded_top = 0;
   target->valid_ref = empty_spot;
}

static uint32_t
nouveau_vp3_fill_picparm_mpeg12_vp(struct nouveau_vp3_decoder *dec,
                                   struct pipe_mpeg12_picture_desc *desc,
                                   struct nouveau_vp3_video_buffer *refs[16],
                                   unsigned *is_ref,
                                   char *map)
{
   struct mpeg12_picparm_vp pic_vp_stub = {}, *pic_vp = &pic_vp_stub;
   uint32_t i, ret = 0x01010, ring; // !async_shutdown << 16 | watchdog << 12 | irq_record << 4 | unk;
   assert(!(dec->base.width & 0xf));
   *is_ref = desc->picture_coding_type <= 2;

   if (dec->base.profile == PIPE_VIDEO_PROFILE_MPEG1)
      pic_vp->picture_structure = 3;
   else
      pic_vp->picture_structure = desc->picture_structure;

   assert(desc->picture_structure != 4);
   if (desc->picture_structure == 4) // Untested, but should work
      ret |= 0x100;
   pic_vp->width = mb(dec->base.width);
   pic_vp->height = mb(dec->base.height);
   pic_vp->unk08 = pic_vp->unk04 = (dec->base.width+0xf)&~0xf; // Stride

   nouveau_vp3_ycbcr_offsets(dec, &pic_vp->ofs[1], &pic_vp->ofs[3], &pic_vp->ofs[4]);
   pic_vp->ofs[5] = pic_vp->ofs[3];
   pic_vp->ofs[0] = pic_vp->ofs[2] = 0;
   nouveau_vp3_inter_sizes(dec, 1, &ring, &pic_vp->bucket_size, &pic_vp->inter_ring_data_size);

   pic_vp->alternate_scan = desc->alternate_scan;
   pic_vp->pad2[0] = pic_vp->pad2[1] = pic_vp->pad2[2] = 0;
   pic_vp->unk30 = desc->picture_structure < 3 && (desc->picture_structure == 2 - desc->top_field_first);
   pic_vp->unk3a = (desc->picture_coding_type == 1);
   for (i = 0; i < 4; ++i)
      pic_vp->f_code[i] = desc->f_code[i/2][i%2] + 1; // FU
   pic_vp->picture_coding_type = desc->picture_coding_type;
   pic_vp->intra_dc_precision = desc->intra_dc_precision;
   pic_vp->q_scale_type = desc->q_scale_type;
   pic_vp->top_field_first = desc->top_field_first;
   pic_vp->full_pel_forward_vector = desc->full_pel_forward_vector;
   pic_vp->full_pel_backward_vector = desc->full_pel_backward_vector;
   memcpy(pic_vp->intra_quantizer_matrix, desc->intra_matrix, 0x40);
   memcpy(pic_vp->non_intra_quantizer_matrix, desc->non_intra_matrix, 0x40);
   memcpy(map, pic_vp, sizeof(*pic_vp));
   refs[0] = (struct nouveau_vp3_video_buffer *)desc->ref[0];
   refs[!!refs[0]] = (struct nouveau_vp3_video_buffer *)desc->ref[1];
   return ret | (dec->base.profile != PIPE_VIDEO_PROFILE_MPEG1);
}

static uint32_t
nouveau_vp3_fill_picparm_mpeg4_vp(struct nouveau_vp3_decoder *dec,
                                  struct pipe_mpeg4_picture_desc *desc,
                                  struct nouveau_vp3_video_buffer *refs[16],
                                  unsigned *is_ref,
                                  char *map)
{
   struct mpeg4_picparm_vp pic_vp_stub = {}, *pic_vp = &pic_vp_stub;
   uint32_t ring, ret = 0x01014; // !async_shutdown << 16 | watchdog << 12 | irq_record << 4 | unk;
   *is_ref = desc->vop_coding_type <= 1;

   pic_vp->width = dec->base.width;
   pic_vp->height = mb(dec->base.height)<<4;
   pic_vp->unk0c = pic_vp->unk08 = mb(dec->base.width)<<4; // Stride

   nouveau_vp3_ycbcr_offsets(dec, &pic_vp->ofs[1], &pic_vp->ofs[3], &pic_vp->ofs[4]);
   pic_vp->ofs[5] = pic_vp->ofs[3];
   pic_vp->ofs[0] = pic_vp->ofs[2] = 0;
   pic_vp->pad1 = pic_vp->pad2 = 0;
   nouveau_vp3_inter_sizes(dec, 1, &ring, &pic_vp->bucket_size, &pic_vp->inter_ring_data_size);

   pic_vp->trd[0] = desc->trd[0];
   pic_vp->trd[1] = desc->trd[1];
   pic_vp->trb[0] = desc->trb[0];
   pic_vp->trb[1] = desc->trb[1];
   pic_vp->u48 = 0; // Codec?
   pic_vp->pad1 = pic_vp->pad2 = 0;
   pic_vp->f_code_fw = desc->vop_fcode_forward;
   pic_vp->f_code_bw = desc->vop_fcode_backward;
   pic_vp->interlaced = desc->interlaced;
   pic_vp->quant_type = desc->quant_type;
   pic_vp->quarter_sample = desc->quarter_sample;
   pic_vp->short_video_header = desc->short_video_header;
   pic_vp->u54 = 0;
   pic_vp->vop_coding_type = desc->vop_coding_type;
   pic_vp->rounding_control = desc->rounding_control;
   pic_vp->alternate_vertical_scan_flag = desc->alternate_vertical_scan_flag;
   pic_vp->top_field_first = desc->top_field_first;

   memcpy(pic_vp->intra, desc->intra_matrix, 0x40);
   memcpy(pic_vp->non_intra, desc->non_intra_matrix, 0x40);
   memcpy(map, pic_vp, sizeof(*pic_vp));
   refs[0] = (struct nouveau_vp3_video_buffer *)desc->ref[0];
   refs[!!refs[0]] = (struct nouveau_vp3_video_buffer *)desc->ref[1];
   return ret;
}

static uint32_t
nouveau_vp3_fill_picparm_h264_vp(struct nouveau_vp3_decoder *dec,
                                 const struct pipe_h264_picture_desc *d,
                                 struct nouveau_vp3_video_buffer *refs[16],
                                 unsigned *is_ref,
                                 char *map)
{
   struct h264_picparm_vp stub_h = {}, *h = &stub_h;
   unsigned ring, i, j = 0;
   assert(offsetof(struct h264_picparm_vp, u224) == 0x224);
   *is_ref = d->is_reference;
   dec->last_frame_num = d->frame_num;

   h->width = mb(dec->base.width);
   h->height = mb(dec->base.height);
   h->stride1 = h->stride2 = mb(dec->base.width)*16;
   nouveau_vp3_ycbcr_offsets(dec, &h->ofs[1], &h->ofs[3], &h->ofs[4]);
   h->ofs[5] = h->ofs[3];
   h->ofs[0] = h->ofs[2] = 0;
   h->tmp_stride = dec->tmp_stride >> 8;
   assert(h->tmp_stride);
   nouveau_vp3_inter_sizes(dec, d->slice_count, &ring, &h->bucket_size, &h->inter_ring_data_size);

   h->u220 = 0;
   h->mb_adaptive_frame_field_flag = d->pps->sps->mb_adaptive_frame_field_flag;
   h->direct_8x8_inference_flag = d->pps->sps->direct_8x8_inference_flag;
   h->weighted_pred_flag = d->pps->weighted_pred_flag;
   h->constrained_intra_pred_flag = d->pps->constrained_intra_pred_flag;
   h->is_reference = d->is_reference;
   h->interlace = d->field_pic_flag;
   h->bottom_field_flag = d->bottom_field_flag;
   h->second_field = 0; // set in nouveau_vp3_fill_picparm_h264_vp_refs

   h->log2_max_frame_num_minus4 = d->pps->sps->log2_max_frame_num_minus4;
   h->chroma_format_idc = 1;

   h->pic_order_cnt_type = d->pps->sps->pic_order_cnt_type;
   h->pic_init_qp_minus26 = d->pps->pic_init_qp_minus26;
   h->chroma_qp_index_offset = d->pps->chroma_qp_index_offset;
   h->second_chroma_qp_index_offset = d->pps->second_chroma_qp_index_offset;
   h->weighted_bipred_idc = d->pps->weighted_bipred_idc;
   h->tmp_idx = 0; // set in h264_vp_refs below
   h->fifo_dec_index = 0; // always set to 0 to be fifo compatible with other codecs
   h->frame_number = d->frame_num;
   h->u34_3030 = h->u34_3131 = 0;
   h->field_order_cnt[0] = d->field_order_cnt[0];
   h->field_order_cnt[1] = d->field_order_cnt[1];
   memcpy(h->m4x4, d->pps->ScalingList4x4, sizeof(h->m4x4));
   memcpy(h->m8x8, d->pps->ScalingList8x8, sizeof(h->m8x8));
   h->u220 = 0;
   for (i = 0; i < d->num_ref_frames; ++i) {
      if (!d->ref[i])
         break;
      refs[j] = (struct nouveau_vp3_video_buffer *)d->ref[i];
      h->refs[j].fifo_idx = j + 1;
      h->refs[j].tmp_idx = refs[j]->valid_ref;
      assert(dec->refs[refs[j]->valid_ref].vidbuf == refs[j]);
      h->refs[j].field_order_cnt[0] = d->field_order_cnt_list[i][0];
      h->refs[j].field_order_cnt[1] = d->field_order_cnt_list[i][1];
      h->refs[j].frame_idx = d->frame_num_list[i];
      if (!dec->refs[refs[j]->valid_ref].field_pic_flag) {
         h->refs[j].top_is_reference = d->top_is_reference[i];
         h->refs[j].bottom_is_reference = d->bottom_is_reference[i];
      }
      h->refs[j].is_long_term = d->is_long_term[i];
      h->refs[j].notseenyet = 0;
      h->refs[j].field_pic_flag = dec->refs[refs[j]->valid_ref].field_pic_flag;
      h->refs[j].top_field_marking =
         dec->refs[refs[j]->valid_ref].decoded_top && d->top_is_reference[i] ?
         1 + d->is_long_term[i] : 0;
      h->refs[j].bottom_field_marking =
         dec->refs[refs[j]->valid_ref].decoded_bottom && d->bottom_is_reference[i] ?
         1 + d->is_long_term[i] : 0;
      h->refs[j].pad = 0;
      j++;
   }
   for (; i < 16; ++i)
      assert(!d->ref[i]);
   assert(d->num_ref_frames <= dec->base.max_references);

   for (; i < d->num_ref_frames; ++i)
      h->refs[j].field_pic_flag = d->field_pic_flag;
   *(struct h264_picparm_vp *)map = *h;

   return 0x1113;
}

static void
nouveau_vp3_fill_picparm_h264_vp_refs(struct nouveau_vp3_decoder *dec,
                                      struct pipe_h264_picture_desc *d,
                                      struct nouveau_vp3_video_buffer *refs[16],
                                      struct nouveau_vp3_video_buffer *target,
                                      char *map)
{
   struct h264_picparm_vp *h = (struct h264_picparm_vp *)map;
   assert(dec->refs[target->valid_ref].vidbuf == target);
//    debug_printf("Target: %p\n", target);

   if (!dec->refs[target->valid_ref].decoded_top &&
       !dec->refs[target->valid_ref].decoded_bottom)
      dec->refs[target->valid_ref].decoded_first = d->bottom_field_flag;
   else if (dec->refs[target->valid_ref].decoded_first != d->bottom_field_flag)
      h->second_field = 1;

   h->tmp_idx = target->valid_ref;
   dec->refs[target->valid_ref].field_pic_flag = d->field_pic_flag;
   if (!d->field_pic_flag || d->bottom_field_flag)
      dec->refs[target->valid_ref].decoded_bottom = 1;
   if (!d->field_pic_flag || !d->bottom_field_flag)
      dec->refs[target->valid_ref].decoded_top = 1;
}

static uint32_t
nouveau_vp3_fill_picparm_vc1_vp(struct nouveau_vp3_decoder *dec,
                                struct pipe_vc1_picture_desc *d,
                                struct nouveau_vp3_video_buffer *refs[16],
                                unsigned *is_ref,
                                char *map)
{
   struct vc1_picparm_vp *vc = (struct vc1_picparm_vp *)map;
   unsigned ring;
   assert(dec->base.profile != PIPE_VIDEO_PROFILE_VC1_SIMPLE);
   *is_ref = d->picture_type <= 1;

   nouveau_vp3_ycbcr_offsets(dec, &vc->ofs[1], &vc->ofs[3], &vc->ofs[4]);
   vc->ofs[5] = vc->ofs[3];
   vc->ofs[0] = vc->ofs[2] = 0;
   vc->width = dec->base.width;
   vc->height = mb(dec->base.height)<<4;
   vc->unk0c = vc->unk10 = mb(dec->base.width)<<4; // Stride
   vc->pad = vc->pad2 = 0;
   nouveau_vp3_inter_sizes(dec, 1, &ring, &vc->bucket_size, &vc->inter_ring_data_size);
   vc->profile = dec->base.profile - PIPE_VIDEO_PROFILE_VC1_SIMPLE;
   vc->loopfilter = d->loopfilter;
   vc->fastuvmc = d->fastuvmc;
   vc->dquant = d->dquant;
   vc->overlap = d->overlap;
   vc->quantizer = d->quantizer;
   vc->u36 = 0; // ? No idea what this one is..
   refs[0] = (struct nouveau_vp3_video_buffer *)d->ref[0];
   refs[!!refs[0]] = (struct nouveau_vp3_video_buffer *)d->ref[1];
   return 0x12;
}

void nouveau_vp3_vp_caps(struct nouveau_vp3_decoder *dec, union pipe_desc desc,
                         struct nouveau_vp3_video_buffer *target, unsigned comm_seq,
                         unsigned *caps, unsigned *is_ref,
                         struct nouveau_vp3_video_buffer *refs[16])
{
   struct nouveau_bo *bsp_bo = dec->bsp_bo[comm_seq % NOUVEAU_VP3_VIDEO_QDEPTH];
   enum pipe_video_format codec = u_reduce_video_profile(dec->base.profile);
   char *vp = bsp_bo->map + VP_OFFSET;

   switch (codec){
   case PIPE_VIDEO_FORMAT_MPEG12:
      *caps = nouveau_vp3_fill_picparm_mpeg12_vp(dec, desc.mpeg12, refs, is_ref, vp);
      nouveau_vp3_handle_references(dec, refs, dec->fence_seq, target);
      switch (desc.mpeg12->picture_structure) {
      case PIPE_MPEG12_PICTURE_STRUCTURE_FIELD_TOP:
         dec->refs[target->valid_ref].decoded_top = 1;
         break;
      case PIPE_MPEG12_PICTURE_STRUCTURE_FIELD_BOTTOM:
         dec->refs[target->valid_ref].decoded_bottom = 1;
         break;
      default:
         dec->refs[target->valid_ref].decoded_top = 1;
         dec->refs[target->valid_ref].decoded_bottom = 1;
         break;
      }
      return;
   case PIPE_VIDEO_FORMAT_MPEG4:
      *caps = nouveau_vp3_fill_picparm_mpeg4_vp(dec, desc.mpeg4, refs, is_ref, vp);
      nouveau_vp3_handle_references(dec, refs, dec->fence_seq, target);
      // XXX: Correct?
      if (!desc.mpeg4->interlaced) {
         dec->refs[target->valid_ref].decoded_top = 1;
         dec->refs[target->valid_ref].decoded_bottom = 1;
      } else if (desc.mpeg4->top_field_first) {
         if (!dec->refs[target->valid_ref].decoded_top)
            dec->refs[target->valid_ref].decoded_top = 1;
         else
            dec->refs[target->valid_ref].decoded_bottom = 1;
      } else {
         if (!dec->refs[target->valid_ref].decoded_bottom)
            dec->refs[target->valid_ref].decoded_bottom = 1;
         else
            dec->refs[target->valid_ref].decoded_top = 1;
      }
      return;
   case PIPE_VIDEO_FORMAT_VC1: {
      *caps = nouveau_vp3_fill_picparm_vc1_vp(dec, desc.vc1, refs, is_ref, vp);
      nouveau_vp3_handle_references(dec, refs, dec->fence_seq, target);
      if (desc.vc1->frame_coding_mode == 3)
         debug_printf("Field-Interlaced possibly incorrectly handled\n");
      dec->refs[target->valid_ref].decoded_top = 1;
      dec->refs[target->valid_ref].decoded_bottom = 1;
      return;
   }
   case PIPE_VIDEO_FORMAT_MPEG4_AVC: {
      *caps = nouveau_vp3_fill_picparm_h264_vp(dec, desc.h264, refs, is_ref, vp);
      nouveau_vp3_handle_references(dec, refs, dec->fence_seq, target);
      nouveau_vp3_fill_picparm_h264_vp_refs(dec, desc.h264, refs, target, vp);
      return;
   }
   default: assert(0); return;
   }
}
