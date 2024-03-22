/**************************************************************************
 *
 * Copyright 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#ifndef _RADEON_VCN_DEC_H
#define _RADEON_VCN_DEC_H

#include "radeon_vcn.h"
#include "util/list.h"

#include "ac_vcn_dec.h"

#define NUM_BUFFERS                                         4

struct rvcn_dec_dynamic_dpb_t2 {
   struct list_head list;
   uint8_t index;
   struct rvid_buffer dpb;
};

struct jpeg_registers {
   #define RDECODE_JPEG_REG_VER_V1 0
   #define RDECODE_JPEG_REG_VER_V2 1
   #define RDECODE_JPEG_REG_VER_V3 2
   unsigned version;
   unsigned jpeg_dec_soft_rst;
   unsigned jrbc_ib_cond_rd_timer;
   unsigned jrbc_ib_ref_data;
   unsigned lmi_jpeg_read_64bit_bar_high;
   unsigned lmi_jpeg_read_64bit_bar_low;
   unsigned jpeg_rb_base;
   unsigned jpeg_rb_size;
   unsigned jpeg_rb_wptr;
   unsigned jpeg_pitch;
   unsigned jpeg_uv_pitch;
   unsigned dec_addr_mode;
   unsigned dec_y_gfx10_tiling_surface;
   unsigned dec_uv_gfx10_tiling_surface;
   unsigned lmi_jpeg_write_64bit_bar_high;
   unsigned lmi_jpeg_write_64bit_bar_low;
   unsigned jpeg_tier_cntl2;
   unsigned jpeg_outbuf_rptr;
   unsigned jpeg_outbuf_cntl;
   unsigned jpeg_int_en;
   unsigned jpeg_cntl;
   unsigned jpeg_rb_rptr;
   unsigned jpeg_outbuf_wptr;
   unsigned jpeg_luma_base0_0;
   unsigned jpeg_chroma_base0_0;
   unsigned jpeg_chromav_base0_0;
   unsigned jpeg_index;
   unsigned jpeg_data;
};

struct radeon_decoder {
   struct pipe_video_codec base;

   unsigned stream_handle;
   unsigned stream_type;
   unsigned frame_number;
   unsigned db_alignment;
   unsigned dpb_size;
   unsigned last_width;
   unsigned last_height;
   unsigned max_width;
   unsigned max_height;
   unsigned addr_gfx_mode;

   struct pipe_screen *screen;
   struct radeon_winsys *ws;
   struct radeon_cmdbuf cs;

   void *msg;
   uint32_t *fb;
   uint8_t *it;
   uint8_t *probs;
   void *bs_ptr;
   rvcn_decode_buffer_t *decode_buffer;
   bool vcn_dec_sw_ring;
   struct rvcn_sq_var sq;

   struct rvid_buffer *msg_fb_it_probs_buffers;
   unsigned num_dec_bufs;
   struct rvid_buffer *bs_buffers;
   struct rvid_buffer dpb;
   struct rvid_buffer ctx;
   struct rvid_buffer sessionctx;

   unsigned bs_size;
   unsigned cur_buffer;
   void *render_pic_list[32];
   unsigned h264_valid_ref_num[17];
   unsigned h264_valid_poc_num[34];
   unsigned av1_version;
   bool show_frame;
   unsigned ref_idx;
   bool tmz_ctx;
   struct {
      unsigned data0;
      unsigned data1;
      unsigned cmd;
      unsigned cntl;
   } reg;
   struct jpeg_params jpg;
   struct jpeg_registers jpg_reg;
   enum {
      DPB_MAX_RES = 0,
      DPB_DYNAMIC_TIER_1,
      DPB_DYNAMIC_TIER_2
   } dpb_type;

   struct {
      enum {
         CODEC_8_BITS = 0,
         CODEC_10_BITS
      } bts;
      uint8_t index;
      unsigned ref_size;
      uint8_t ref_list[16];
   } ref_codec;

   struct list_head dpb_ref_list;
   struct list_head dpb_unref_list;

   void (*send_cmd)(struct radeon_decoder *dec, struct pipe_video_buffer *target,
                    struct pipe_picture_desc *picture);
   /* Additional contexts for mJPEG */
   struct radeon_cmdbuf *jcs;
   struct radeon_winsys_ctx **jctx;
   unsigned cb_idx;
   unsigned njctx;
   struct pipe_fence_handle *prev_fence;
   struct pipe_fence_handle *destroy_fence;
};

void send_cmd_dec(struct radeon_decoder *dec, struct pipe_video_buffer *target,
                  struct pipe_picture_desc *picture);

void send_cmd_jpeg(struct radeon_decoder *dec, struct pipe_video_buffer *target,
                   struct pipe_picture_desc *picture);

struct pipe_video_codec *radeon_create_decoder(struct pipe_context *context,
                                               const struct pipe_video_codec *templat);

#endif
