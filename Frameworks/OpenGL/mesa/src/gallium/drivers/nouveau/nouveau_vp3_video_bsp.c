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

struct strparm_bsp {
   uint32_t w0[4]; // bits 0-23 length, bits 24-31 addr_hi
   uint32_t w1[4]; // bit 8-24 addr_lo
   uint32_t unk20; // should be idx * 0x8000000, bitstream offset
   uint32_t do_crypto_crap; // set to 0
};

struct mpeg12_picparm_bsp {
   uint16_t width;
   uint16_t height;
   uint8_t picture_structure;
   uint8_t picture_coding_type;
   uint8_t intra_dc_precision;
   uint8_t frame_pred_frame_dct;
   uint8_t concealment_motion_vectors;
   uint8_t intra_vlc_format;
   uint16_t pad;
   uint8_t f_code[2][2];
};

struct mpeg4_picparm_bsp {
   uint16_t width;
   uint16_t height;
   uint8_t vop_time_increment_size;
   uint8_t interlaced;
   uint8_t resync_marker_disable;
};

struct vc1_picparm_bsp {
   uint16_t width;
   uint16_t height;
   uint8_t profile; // 04 0 simple, 1 main, 2 advanced
   uint8_t postprocflag; // 05
   uint8_t pulldown; // 06
   uint8_t interlaced; // 07
   uint8_t tfcntrflag; // 08
   uint8_t finterpflag; // 09
   uint8_t psf; // 0a
   uint8_t pad; // 0b
   uint8_t multires; // 0c
   uint8_t syncmarker; // 0d
   uint8_t rangered; // 0e
   uint8_t maxbframes; // 0f
   uint8_t dquant; // 10
   uint8_t panscan_flag; // 11
   uint8_t refdist_flag; // 12
   uint8_t quantizer; // 13
   uint8_t extended_mv; // 14
   uint8_t extended_dmv; // 15
   uint8_t overlap; // 16
   uint8_t vstransform; // 17
};

struct h264_picparm_bsp {
   // 00
   uint32_t unk00;
   // 04
   uint32_t log2_max_frame_num_minus4; // 04 checked
   uint32_t pic_order_cnt_type; // 08 checked
   uint32_t log2_max_pic_order_cnt_lsb_minus4; // 0c checked
   uint32_t delta_pic_order_always_zero_flag; // 10, or unknown

   uint32_t frame_mbs_only_flag; // 14, always 1?
   uint32_t direct_8x8_inference_flag; // 18, always 1?
   uint32_t width_mb; // 1c checked
   uint32_t height_mb; // 20 checked
   // 24
   //struct picparm2
   uint32_t entropy_coding_mode_flag; // 00, checked
   uint32_t pic_order_present_flag; // 04 checked
   uint32_t unk; // 08 seems to be 0?
   uint32_t pad1; // 0c seems to be 0?
   uint32_t pad2; // 10 always 0 ?
   uint32_t num_ref_idx_l0_active_minus1; // 14 always 0?
   uint32_t num_ref_idx_l1_active_minus1; // 18 always 0?
   uint32_t weighted_pred_flag; // 1c checked
   uint32_t weighted_bipred_idc; // 20 checked
   uint32_t pic_init_qp_minus26; // 24 checked
   uint32_t deblocking_filter_control_present_flag; // 28 always 1?
   uint32_t redundant_pic_cnt_present_flag; // 2c always 0?
   uint32_t transform_8x8_mode_flag; // 30 checked
   uint32_t mb_adaptive_frame_field_flag; // 34 checked-ish
   uint8_t field_pic_flag; // 38 checked
   uint8_t bottom_field_flag; // 39 checked
   uint8_t real_pad[0x1b]; // XX why?
};

static uint32_t
nouveau_vp3_fill_picparm_mpeg12_bsp(struct nouveau_vp3_decoder *dec,
                                    struct pipe_mpeg12_picture_desc *desc,
                                    char *map)
{
   struct mpeg12_picparm_bsp *pic_bsp = (struct mpeg12_picparm_bsp *)map;
   int i;
   pic_bsp->width = dec->base.width;
   pic_bsp->height = dec->base.height;
   pic_bsp->picture_structure = desc->picture_structure;
   pic_bsp->picture_coding_type = desc->picture_coding_type;
   pic_bsp->intra_dc_precision = desc->intra_dc_precision;
   pic_bsp->frame_pred_frame_dct = desc->frame_pred_frame_dct;
   pic_bsp->concealment_motion_vectors = desc->concealment_motion_vectors;
   pic_bsp->intra_vlc_format = desc->intra_vlc_format;
   pic_bsp->pad = 0;
   for (i = 0; i < 4; ++i)
      pic_bsp->f_code[i/2][i%2] = desc->f_code[i/2][i%2] + 1; // FU

   return (desc->num_slices << 4) | (dec->base.profile != PIPE_VIDEO_PROFILE_MPEG1);
}

static uint32_t
nouveau_vp3_fill_picparm_mpeg4_bsp(struct nouveau_vp3_decoder *dec,
                                   struct pipe_mpeg4_picture_desc *desc,
                                   char *map)
{
   struct mpeg4_picparm_bsp *pic_bsp = (struct mpeg4_picparm_bsp *)map;
   uint32_t t, bits = 0;
   pic_bsp->width = dec->base.width;
   pic_bsp->height = dec->base.height;
   assert(desc->vop_time_increment_resolution > 0);

   t = desc->vop_time_increment_resolution - 1;
   while (t) {
      bits++;
      t /= 2;
   }
   if (!bits)
      bits = 1;
   t = desc->vop_time_increment_resolution - 1;
   pic_bsp->vop_time_increment_size = bits;
   pic_bsp->interlaced = desc->interlaced;
   pic_bsp->resync_marker_disable = desc->resync_marker_disable;
   return 4;
}

static uint32_t
nouveau_vp3_fill_picparm_vc1_bsp(struct nouveau_vp3_decoder *dec,
                                 struct pipe_vc1_picture_desc *d,
                                 char *map)
{
   struct vc1_picparm_bsp *vc = (struct vc1_picparm_bsp *)map;
   uint32_t caps = (d->slice_count << 4)&0xfff0;
   vc->width = dec->base.width;
   vc->height = dec->base.height;
   vc->profile = dec->base.profile - PIPE_VIDEO_PROFILE_VC1_SIMPLE; // 04
   vc->postprocflag = d->postprocflag;
   vc->pulldown = d->pulldown;
   vc->interlaced = d->interlace;
   vc->tfcntrflag = d->tfcntrflag; // 08
   vc->finterpflag = d->finterpflag;
   vc->psf = d->psf;
   vc->pad = 0;
   vc->multires = d->multires; // 0c
   vc->syncmarker = d->syncmarker;
   vc->rangered = d->rangered;
   vc->maxbframes = d->maxbframes;
   vc->dquant = d->dquant; // 10
   vc->panscan_flag = d->panscan_flag;
   vc->refdist_flag = d->refdist_flag;
   vc->quantizer = d->quantizer;
   vc->extended_mv = d->extended_mv; // 14
   vc->extended_dmv = d->extended_dmv;
   vc->overlap = d->overlap;
   vc->vstransform = d->vstransform;
   return caps | 2;
}

static uint32_t
nouveau_vp3_fill_picparm_h264_bsp(struct nouveau_vp3_decoder *dec,
                                  struct pipe_h264_picture_desc *d,
                                  char *map)
{
   struct h264_picparm_bsp stub_h = {}, *h = &stub_h;
   uint32_t caps = (d->slice_count << 4)&0xfff0;

   assert(!(d->slice_count & ~0xfff));
   if (d->slice_count & 0x1000)
      caps |= 1 << 20;

   assert(offsetof(struct h264_picparm_bsp, bottom_field_flag) == (0x39 + 0x24));
   h->unk00 = 1;
   h->pad1 = h->pad2 = 0;
   h->unk = 0;
   h->log2_max_frame_num_minus4 = d->pps->sps->log2_max_frame_num_minus4;
   h->frame_mbs_only_flag = d->pps->sps->frame_mbs_only_flag;
   h->direct_8x8_inference_flag = d->pps->sps->direct_8x8_inference_flag;
   h->width_mb = mb(dec->base.width);
   h->height_mb = mb(dec->base.height);
   h->entropy_coding_mode_flag = d->pps->entropy_coding_mode_flag;
   h->pic_order_present_flag = d->pps->bottom_field_pic_order_in_frame_present_flag;
   h->pic_order_cnt_type = d->pps->sps->pic_order_cnt_type;
   h->log2_max_pic_order_cnt_lsb_minus4 = d->pps->sps->log2_max_pic_order_cnt_lsb_minus4;
   h->delta_pic_order_always_zero_flag = d->pps->sps->delta_pic_order_always_zero_flag;
   h->num_ref_idx_l0_active_minus1 = d->num_ref_idx_l0_active_minus1;
   h->num_ref_idx_l1_active_minus1 = d->num_ref_idx_l1_active_minus1;
   h->weighted_pred_flag = d->pps->weighted_pred_flag;
   h->weighted_bipred_idc = d->pps->weighted_bipred_idc;
   h->pic_init_qp_minus26 = d->pps->pic_init_qp_minus26;
   h->deblocking_filter_control_present_flag = d->pps->deblocking_filter_control_present_flag;
   h->redundant_pic_cnt_present_flag = d->pps->redundant_pic_cnt_present_flag;
   h->transform_8x8_mode_flag = d->pps->transform_8x8_mode_flag;
   h->mb_adaptive_frame_field_flag = d->pps->sps->mb_adaptive_frame_field_flag;
   h->field_pic_flag = d->field_pic_flag;
   h->bottom_field_flag = d->bottom_field_flag;
   memset(h->real_pad, 0, sizeof(h->real_pad));
   *(struct h264_picparm_bsp *)map = *h;
   return caps | 3;
}

static inline struct strparm_bsp *strparm_bsp(struct nouveau_vp3_decoder *dec)
{
   unsigned comm_seq = dec->fence_seq;
   struct nouveau_bo *bsp_bo = dec->bsp_bo[comm_seq % NOUVEAU_VP3_VIDEO_QDEPTH];
   return (struct strparm_bsp *)(bsp_bo->map + 0x100);
}

void
nouveau_vp3_bsp_begin(struct nouveau_vp3_decoder *dec)
{
   struct strparm_bsp *str_bsp = strparm_bsp(dec);

   dec->bsp_ptr = (void *)str_bsp;
   memset(str_bsp, 0, 0x80);
   dec->bsp_ptr += 0x100;
   /* Reserved for picparm_vp */
   dec->bsp_ptr += 0x300;
   /* Reserved for comm */
#if !NOUVEAU_VP3_DEBUG_FENCE
   memset(dec->bsp_ptr, 0, 0x200);
#endif
   dec->bsp_ptr += 0x200;
}

void
nouveau_vp3_bsp_next(struct nouveau_vp3_decoder *dec, unsigned num_buffers,
                     const void *const *data, const unsigned *num_bytes)
{
#ifndef NDEBUG
   unsigned comm_seq = dec->fence_seq;
   struct nouveau_bo *bsp_bo = dec->bsp_bo[comm_seq % NOUVEAU_VP3_VIDEO_QDEPTH];
#endif
   struct strparm_bsp *str_bsp = strparm_bsp(dec);
   int i;

   for (i = 0; i < num_buffers; ++i) {
#ifndef NDEBUG
      assert(bsp_bo->size >= str_bsp->w0[0] + num_bytes[i]);
#endif
      memcpy(dec->bsp_ptr, data[i], num_bytes[i]);
      dec->bsp_ptr += num_bytes[i];
      str_bsp->w0[0] += num_bytes[i];
   }
}

uint32_t
nouveau_vp3_bsp_end(struct nouveau_vp3_decoder *dec, union pipe_desc desc)
{
   enum pipe_video_format codec = u_reduce_video_profile(dec->base.profile);
   unsigned comm_seq = dec->fence_seq;
   struct nouveau_bo *bsp_bo = dec->bsp_bo[comm_seq % NOUVEAU_VP3_VIDEO_QDEPTH];
   uint32_t endmarker, caps;
   struct strparm_bsp *str_bsp = strparm_bsp(dec);
   char *bsp = bsp_bo->map;
   /*
    * 0x000..0x100: picparm_bsp
    * 0x200..0x500: picparm_vp
    * 0x500..0x700: comm
    * 0x700..onward: raw bitstream
    */

   switch (codec){
   case PIPE_VIDEO_FORMAT_MPEG12:
      endmarker = 0xb7010000;
      caps = nouveau_vp3_fill_picparm_mpeg12_bsp(dec, desc.mpeg12, bsp);
      break;
   case PIPE_VIDEO_FORMAT_MPEG4:
      endmarker = 0xb1010000;
      caps = nouveau_vp3_fill_picparm_mpeg4_bsp(dec, desc.mpeg4, bsp);
      break;
   case PIPE_VIDEO_FORMAT_VC1: {
      endmarker = 0x0a010000;
      caps = nouveau_vp3_fill_picparm_vc1_bsp(dec, desc.vc1, bsp);
      break;
   }
   case PIPE_VIDEO_FORMAT_MPEG4_AVC: {
      endmarker = 0x0b010000;
      caps = nouveau_vp3_fill_picparm_h264_bsp(dec, desc.h264, bsp);
      break;
   }
   default: assert(0); return -1;
   }

   caps |= 0 << 16; // reset struct comm if flag is set
   caps |= 1 << 17; // enable watchdog
   caps |= 0 << 18; // do not report error to VP, so it can continue decoding what we have
   caps |= 0 << 19; // if enabled, use crypto crap?

   str_bsp = strparm_bsp(dec);
   str_bsp->w1[0] = 0x1;

   /* Append end sequence */
   *(uint32_t *)dec->bsp_ptr = endmarker;
   dec->bsp_ptr += 4;
   *(uint32_t *)dec->bsp_ptr = 0x00000000;
   dec->bsp_ptr += 4;
   *(uint32_t *)dec->bsp_ptr = endmarker;
   dec->bsp_ptr += 4;
   *(uint32_t *)dec->bsp_ptr = 0x00000000;
   str_bsp->w0[0] += 16;

   dec->bsp_ptr = NULL;

   return caps;
}
