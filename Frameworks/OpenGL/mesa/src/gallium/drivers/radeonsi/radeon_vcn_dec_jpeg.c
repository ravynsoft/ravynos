/**************************************************************************
 *
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#include "pipe/p_video_codec.h"
#include "radeon_vcn_dec.h"
#include "radeon_video.h"
#include "radeonsi/si_pipe.h"
#include "util/u_memory.h"
#include "util/u_video.h"

#include <assert.h>
#include <stdio.h>

static struct pb_buffer_lean *radeon_jpeg_get_decode_param(struct radeon_decoder *dec,
                                                           struct pipe_video_buffer *target,
                                                           struct pipe_picture_desc *picture)
{
   struct si_texture *luma = (struct si_texture *)((struct vl_video_buffer *)target)->resources[0];
   struct si_texture *chroma, *chromav;

   dec->jpg.bsd_size = align(dec->bs_size, 128);
   dec->jpg.dt_luma_top_offset = luma->surface.u.gfx9.surf_offset;
   dec->jpg.dt_chroma_top_offset = 0;
   dec->jpg.dt_chromav_top_offset = 0;

   switch (target->buffer_format) {
      case PIPE_FORMAT_IYUV:
      case PIPE_FORMAT_YV12:
      case PIPE_FORMAT_Y8_U8_V8_444_UNORM:
      case PIPE_FORMAT_R8_G8_B8_UNORM:
         chromav = (struct si_texture *)((struct vl_video_buffer *)target)->resources[2];
         dec->jpg.dt_chromav_top_offset = chromav->surface.u.gfx9.surf_offset;
         chroma = (struct si_texture *)((struct vl_video_buffer*)target)->resources[1];
         dec->jpg.dt_chroma_top_offset = chroma->surface.u.gfx9.surf_offset;
         break;
      case PIPE_FORMAT_NV12:
      case PIPE_FORMAT_P010:
      case PIPE_FORMAT_P016:
         chroma = (struct si_texture *)((struct vl_video_buffer*)target)->resources[1];
         dec->jpg.dt_chroma_top_offset = chroma->surface.u.gfx9.surf_offset;
         break;
      default:
         break;
   }
   dec->jpg.dt_pitch = luma->surface.u.gfx9.surf_pitch * luma->surface.blk_w;
   dec->jpg.dt_uv_pitch = dec->jpg.dt_pitch / 2;

   return luma->buffer.buf;
}

/* add a new set register command to the IB */
static void set_reg_jpeg(struct radeon_decoder *dec, unsigned reg, unsigned cond, unsigned type,
                         uint32_t val)
{
   radeon_emit(&dec->jcs[dec->cb_idx], RDECODE_PKTJ(reg, cond, type));
   radeon_emit(&dec->jcs[dec->cb_idx], val);
}

/* send a bitstream buffer command */
static void send_cmd_bitstream(struct radeon_decoder *dec, struct pb_buffer_lean *buf, uint32_t off,
                               unsigned usage, enum radeon_bo_domain domain)
{
   uint64_t addr;

   // jpeg soft reset
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_CNTL), COND0, TYPE0, 1);

   // ensuring the Reset is asserted in SCLK domain
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C2);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, 0x01400200);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C3);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, (1 << 9));
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_SOFT_RESET), COND0, TYPE3, (1 << 9));

   // wait mem
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_CNTL), COND0, TYPE0, 0);

   // ensuring the Reset is de-asserted in SCLK domain
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C3);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, (0 << 9));
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_SOFT_RESET), COND0, TYPE3, (1 << 9));

   dec->ws->cs_add_buffer(&dec->jcs[dec->cb_idx], buf, usage | RADEON_USAGE_SYNCHRONIZED, domain);
   addr = dec->ws->buffer_get_virtual_address(buf);
   addr = addr + off;

   // set UVD_LMI_JPEG_READ_64BIT_BAR_LOW/HIGH based on bitstream buffer address
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_LMI_JPEG_READ_64BIT_BAR_HIGH), COND0, TYPE0,
                (addr >> 32));
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_LMI_JPEG_READ_64BIT_BAR_LOW), COND0, TYPE0, addr);

   // set jpeg_rb_base
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_RB_BASE), COND0, TYPE0, 0);

   // set jpeg_rb_base
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_RB_SIZE), COND0, TYPE0, 0xFFFFFFF0);

   // set jpeg_rb_wptr
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_RB_WPTR), COND0, TYPE0, (dec->jpg.bsd_size >> 2));
}

/* send a target buffer command */
static void send_cmd_target(struct radeon_decoder *dec, struct pb_buffer_lean *buf, uint32_t off,
                            unsigned usage, enum radeon_bo_domain domain)
{
   uint64_t addr;

   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_PITCH), COND0, TYPE0, (dec->jpg.dt_pitch >> 4));
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_UV_PITCH), COND0, TYPE0,
                ((dec->jpg.dt_uv_pitch * 2) >> 4));

   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_TILING_CTRL), COND0, TYPE0, 0);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_UV_TILING_CTRL), COND0, TYPE0, 0);

   dec->ws->cs_add_buffer(&dec->jcs[dec->cb_idx], buf, usage | RADEON_USAGE_SYNCHRONIZED, domain);
   addr = dec->ws->buffer_get_virtual_address(buf);
   addr = addr + off;

   // set UVD_LMI_JPEG_WRITE_64BIT_BAR_LOW/HIGH based on target buffer address
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_LMI_JPEG_WRITE_64BIT_BAR_HIGH), COND0, TYPE0,
                (addr >> 32));
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_LMI_JPEG_WRITE_64BIT_BAR_LOW), COND0, TYPE0, addr);

   // set output buffer data address
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_INDEX), COND0, TYPE0, 0);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_DATA), COND0, TYPE0, dec->jpg.dt_luma_top_offset);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_INDEX), COND0, TYPE0, 1);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_DATA), COND0, TYPE0, dec->jpg.dt_chroma_top_offset);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_TIER_CNTL2), COND0, TYPE3, 0);

   // set output buffer read pointer
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_OUTBUF_RPTR), COND0, TYPE0, 0);

   // enable error interrupts
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_INT_EN), COND0, TYPE0, 0xFFFFFFFE);

   // start engine command
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_CNTL), COND0, TYPE0, 0x6);

   // wait for job completion, wait for job JBSI fetch done
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C3);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, (dec->jpg.bsd_size >> 2));
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C2);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, 0x01400200);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_RB_RPTR), COND0, TYPE3, 0xFFFFFFFF);

   // wait for job jpeg outbuf idle
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C3);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, 0xFFFFFFFF);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_OUTBUF_WPTR), COND0, TYPE3, 0x00000001);

   // stop engine
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_CNTL), COND0, TYPE0, 0x4);

   // asserting jpeg lmi drop
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x0005);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, (1 << 23 | 1 << 0));
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE1, 0);

   // asserting jpeg reset
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_CNTL), COND0, TYPE0, 1);

   // ensure reset is asserted in sclk domain
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C3);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, (1 << 9));
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_SOFT_RESET), COND0, TYPE3, (1 << 9));

   // de-assert jpeg reset
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_JPEG_CNTL), COND0, TYPE0, 0);

   // ensure reset is de-asserted in sclk domain
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C3);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, (0 << 9));
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_SOFT_RESET), COND0, TYPE3, (1 << 9));

   // de-asserting jpeg lmi drop
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x0005);
   set_reg_jpeg(dec, SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, 0);
}

/* send a bitstream buffer command */
static void send_cmd_bitstream_direct(struct radeon_decoder *dec, struct pb_buffer_lean *buf,
                                      uint32_t off, unsigned usage,
                                      enum radeon_bo_domain domain)
{
   uint64_t addr;

   // jpeg soft reset
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_dec_soft_rst, COND0, TYPE0, 1);

   // ensuring the Reset is asserted in SCLK domain
   set_reg_jpeg(dec, dec->jpg_reg.jrbc_ib_cond_rd_timer, COND0, TYPE0, 0x01400200);
   set_reg_jpeg(dec, dec->jpg_reg.jrbc_ib_ref_data, COND0, TYPE0, (0x1 << 0x10));
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_dec_soft_rst, COND3, TYPE3, (0x1 << 0x10));

   // wait mem
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_dec_soft_rst, COND0, TYPE0, 0);

   // ensuring the Reset is de-asserted in SCLK domain
   set_reg_jpeg(dec, dec->jpg_reg.jrbc_ib_ref_data, COND0, TYPE0, (0 << 0x10));
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_dec_soft_rst, COND3, TYPE3, (0x1 << 0x10));

   dec->ws->cs_add_buffer(&dec->jcs[dec->cb_idx], buf, usage | RADEON_USAGE_SYNCHRONIZED, domain);
   addr = dec->ws->buffer_get_virtual_address(buf);
   addr = addr + off;

   // set UVD_LMI_JPEG_READ_64BIT_BAR_LOW/HIGH based on bitstream buffer address
   set_reg_jpeg(dec, dec->jpg_reg.lmi_jpeg_read_64bit_bar_high, COND0, TYPE0, (addr >> 32));
   set_reg_jpeg(dec, dec->jpg_reg.lmi_jpeg_read_64bit_bar_low, COND0, TYPE0, addr);

   // set jpeg_rb_base
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_rb_base, COND0, TYPE0, 0);

   // set jpeg_rb_base
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_rb_size, COND0, TYPE0, 0xFFFFFFF0);

   // set jpeg_rb_wptr
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_rb_wptr, COND0, TYPE0, (dec->jpg.bsd_size >> 2));
}

/* send a target buffer command */
static void send_cmd_target_direct(struct radeon_decoder *dec, struct pb_buffer_lean *buf, uint32_t off,
                                   unsigned usage, enum radeon_bo_domain domain,
                                   enum pipe_format buffer_format)
{
   uint64_t addr;
   uint32_t val;
   bool format_convert = false;
   uint32_t fc_sps_info_val = 0;

   switch (buffer_format) {
      case PIPE_FORMAT_R8G8B8A8_UNORM:
         format_convert = true;
         fc_sps_info_val = 1 | (1 << 4) | (0xff << 8);
         break;
      case PIPE_FORMAT_A8R8G8B8_UNORM:
         format_convert = true;
         fc_sps_info_val = 1 | (1 << 4) | (1 << 5) | (0xff << 8);
         break;
      case PIPE_FORMAT_R8_G8_B8_UNORM:
         format_convert = true;
         fc_sps_info_val = 1 | (1 << 5) | (0xff << 8);
         break;
      default:
         break;
   }

   if (dec->jpg_reg.version == RDECODE_JPEG_REG_VER_V3 && format_convert) {
      set_reg_jpeg(dec, dec->jpg_reg.jpeg_pitch, COND0, TYPE0, dec->jpg.dt_pitch);
      set_reg_jpeg(dec, dec->jpg_reg.jpeg_uv_pitch, COND0, TYPE0, (dec->jpg.dt_uv_pitch * 2));
   } else {
      set_reg_jpeg(dec, dec->jpg_reg.jpeg_pitch, COND0, TYPE0, (dec->jpg.dt_pitch >> 4));
      set_reg_jpeg(dec, dec->jpg_reg.jpeg_uv_pitch, COND0, TYPE0, ((dec->jpg.dt_uv_pitch * 2) >> 4));
   }

   set_reg_jpeg(dec, dec->jpg_reg.dec_addr_mode, COND0, TYPE0, 0);
   set_reg_jpeg(dec, dec->jpg_reg.dec_y_gfx10_tiling_surface, COND0, TYPE0, 0);
   set_reg_jpeg(dec, dec->jpg_reg.dec_uv_gfx10_tiling_surface, COND0, TYPE0, 0);

   dec->ws->cs_add_buffer(&dec->jcs[dec->cb_idx], buf, usage | RADEON_USAGE_SYNCHRONIZED, domain);
   addr = dec->ws->buffer_get_virtual_address(buf);
   addr = addr + off;

   // set UVD_LMI_JPEG_WRITE_64BIT_BAR_LOW/HIGH based on target buffer address
   set_reg_jpeg(dec, dec->jpg_reg.lmi_jpeg_write_64bit_bar_high, COND0, TYPE0, (addr >> 32));
   set_reg_jpeg(dec, dec->jpg_reg.lmi_jpeg_write_64bit_bar_low, COND0, TYPE0, addr);

   // set output buffer data address
   if (dec->jpg_reg.version == RDECODE_JPEG_REG_VER_V2) {
      set_reg_jpeg(dec, dec->jpg_reg.jpeg_index, COND0, TYPE0, 0);
      set_reg_jpeg(dec, dec->jpg_reg.jpeg_data, COND0, TYPE0, dec->jpg.dt_luma_top_offset);
      set_reg_jpeg(dec, dec->jpg_reg.jpeg_index, COND0, TYPE0, 1);
      set_reg_jpeg(dec, dec->jpg_reg.jpeg_data, COND0, TYPE0, dec->jpg.dt_chroma_top_offset);
      if (dec->jpg.dt_chromav_top_offset) {
         set_reg_jpeg(dec, dec->jpg_reg.jpeg_index, COND0, TYPE0, 2);
         set_reg_jpeg(dec, dec->jpg_reg.jpeg_data, COND0, TYPE0, dec->jpg.dt_chromav_top_offset);
      }
   } else {
      set_reg_jpeg(dec, dec->jpg_reg.jpeg_luma_base0_0, COND0, TYPE0, dec->jpg.dt_luma_top_offset);
      set_reg_jpeg(dec, dec->jpg_reg.jpeg_chroma_base0_0, COND0, TYPE0, dec->jpg.dt_chroma_top_offset);
      set_reg_jpeg(dec, dec->jpg_reg.jpeg_chromav_base0_0, COND0, TYPE0, dec->jpg.dt_chromav_top_offset);
      if (dec->jpg.crop_width && dec->jpg.crop_height) {
         set_reg_jpeg(dec, vcnipUVD_JPEG_ROI_CROP_POS_START, COND0, TYPE0,
                      ((dec->jpg.crop_y << 16) | dec->jpg.crop_x));
         set_reg_jpeg(dec, vcnipUVD_JPEG_ROI_CROP_POS_STRIDE, COND0, TYPE0,
                      ((dec->jpg.crop_height << 16) | dec->jpg.crop_width));
      } else {
         set_reg_jpeg(dec, vcnipUVD_JPEG_ROI_CROP_POS_START, COND0, TYPE0,
                      ((0 << 16) | 0));
         set_reg_jpeg(dec, vcnipUVD_JPEG_ROI_CROP_POS_STRIDE, COND0, TYPE0,
                      ((1 << 16) | 1));
      }
      if (format_convert) {
         /* set fc timeout control */
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_TMEOUT_CNT, COND0, TYPE0,(4244373504));
         /* set alpha position and packed format */
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_SPS_INFO, COND0, TYPE0, fc_sps_info_val);
         /* coefs */
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_R_COEF, COND0, TYPE0, 256 | (0 << 10) | (403 << 20));
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_G_COEF, COND0, TYPE0, 256 | (976 << 10) | (904 << 20));
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_B_COEF, COND0, TYPE0, 256 | (475 << 10) | (0 << 20));
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_VUP_COEF_CNTL0, COND0, TYPE0, 128 | (384 << 16));
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_VUP_COEF_CNTL1, COND0, TYPE0, 384 | (128 << 16));
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_VUP_COEF_CNTL2, COND0, TYPE0, 128 | (384 << 16));
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_VUP_COEF_CNTL3, COND0, TYPE0, 384 | (128 << 16));
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_HUP_COEF_CNTL0, COND0, TYPE0, 128 | (384 << 16));
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_HUP_COEF_CNTL1, COND0, TYPE0, 384 | (128 << 16));
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_HUP_COEF_CNTL2, COND0, TYPE0, 128 | (384 << 16));
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_HUP_COEF_CNTL3, COND0, TYPE0, 384 | (128 << 16));
      } else
         set_reg_jpeg(dec, vcnipUVD_JPEG_FC_SPS_INFO, COND0, TYPE0, 1 | (1 << 5) | (255 << 8));
   }
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_tier_cntl2, COND0, 0, 0);

   // set output buffer read pointer
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_outbuf_rptr, COND0, TYPE0, 0);
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_outbuf_cntl, COND0, TYPE0,
                ((0x00001587 & (~0x00000180L)) | (0x1 << 0x7) | (0x1 << 0x6)));

   // enable error interrupts
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_int_en, COND0, TYPE0, 0xFFFFFFFE);

   // start engine command
   val = 0x6;
   if (dec->jpg_reg.version == RDECODE_JPEG_REG_VER_V3) {
      if (dec->jpg.crop_width && dec->jpg.crop_height)
         val = val | (0x1 << 24);
      if (format_convert)
         val = val |  (1 << 16) | (1 << 18);
   }
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_cntl, COND0, TYPE0, val);

   // wait for job completion, wait for job JBSI fetch done
   set_reg_jpeg(dec, dec->jpg_reg.jrbc_ib_ref_data, COND0, TYPE0, (dec->jpg.bsd_size >> 2));
   set_reg_jpeg(dec, dec->jpg_reg.jrbc_ib_cond_rd_timer, COND0, TYPE0, 0x01400200);
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_rb_rptr, COND3, TYPE3, 0xFFFFFFFF);

   // wait for job jpeg outbuf idle
   set_reg_jpeg(dec, dec->jpg_reg.jrbc_ib_ref_data, COND0, TYPE0, 0xFFFFFFFF);
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_outbuf_wptr, COND3, TYPE3, 0x00000001);

   if (dec->jpg_reg.version == RDECODE_JPEG_REG_VER_V3 && format_convert) {
      val = val | (0x7 << 16);
      set_reg_jpeg(dec, dec->jpg_reg.jrbc_ib_ref_data, COND0, TYPE0, 0);
      set_reg_jpeg(dec, vcnipUVD_JPEG_INT_STAT, COND3, TYPE3, val);
   }

   // stop engine
   set_reg_jpeg(dec, dec->jpg_reg.jpeg_cntl, COND0, TYPE0, 0x4);
}

/**
 * send cmd for vcn jpeg
 */
void send_cmd_jpeg(struct radeon_decoder *dec, struct pipe_video_buffer *target,
                   struct pipe_picture_desc *picture)
{
   struct pb_buffer_lean *dt;
   struct rvid_buffer *bs_buf;

   bs_buf = &dec->bs_buffers[dec->cur_buffer];

   memset(dec->bs_ptr, 0, align(dec->bs_size, 128) - dec->bs_size);
   dec->ws->buffer_unmap(dec->ws, bs_buf->res->buf);
   dec->bs_ptr = NULL;

   dt = radeon_jpeg_get_decode_param(dec, target, picture);

   if (dec->jpg_reg.version == RDECODE_JPEG_REG_VER_V1) {
      send_cmd_bitstream(dec, bs_buf->res->buf, 0, RADEON_USAGE_READ, RADEON_DOMAIN_GTT);
      send_cmd_target(dec, dt, 0, RADEON_USAGE_WRITE, RADEON_DOMAIN_VRAM);
   } else {
      send_cmd_bitstream_direct(dec, bs_buf->res->buf, 0, RADEON_USAGE_READ, RADEON_DOMAIN_GTT);
      send_cmd_target_direct(dec, dt, 0, RADEON_USAGE_WRITE, RADEON_DOMAIN_VRAM, target->buffer_format);
   }
}
