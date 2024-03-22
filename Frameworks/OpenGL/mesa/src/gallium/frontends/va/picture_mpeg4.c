/**************************************************************************
 *
 * Copyright 2014 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "va_private.h"

void vlVaHandlePictureParameterBufferMPEG4(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   static const uint8_t default_intra_quant_matrix[64] = { 0 };
   static const uint8_t default_non_intra_quant_matrix[64] = { 0 };

   VAPictureParameterBufferMPEG4 *mpeg4 = buf->data;
   unsigned i;

   assert(buf->size >= sizeof(VAPictureParameterBufferMPEG4) && buf->num_elements == 1);

   context->mpeg4.pps = *mpeg4;

   /* vop_width */
   /* vop_height */
   /* forward_reference_picture */
   /* backward_reference_picture */
   context->desc.mpeg4.short_video_header =
         mpeg4->vol_fields.bits.short_video_header;
   /* chroma_format */
   context->desc.mpeg4.interlaced = mpeg4->vol_fields.bits.interlaced;
   /* obmc_disable */
   /* sprite_enable */
   /* sprite_warping_accuracy */
   context->desc.mpeg4.quant_type = mpeg4->vol_fields.bits.quant_type;
   context->desc.mpeg4.quarter_sample = mpeg4->vol_fields.bits.quarter_sample;
   /* data_partitioned */
   /* reversible_vlc */
   context->desc.mpeg4.resync_marker_disable =
         mpeg4->vol_fields.bits.resync_marker_disable;
   /* no_of_sprite_warping_points */
   /* sprite_trajectory_du */
   /* sprite_trajectory_dv */
   /* quant_precision */
   context->desc.mpeg4.vop_coding_type = mpeg4->vop_fields.bits.vop_coding_type;
   /* backward_reference_vop_coding_type */
   /* vop_rounding_type */
   /* intra_dc_vlc_thr */
   context->desc.mpeg4.top_field_first =
         mpeg4->vop_fields.bits.top_field_first;
   context->desc.mpeg4.alternate_vertical_scan_flag =
         mpeg4->vop_fields.bits.alternate_vertical_scan_flag;
   context->desc.mpeg4.vop_fcode_forward = mpeg4->vop_fcode_forward;
   context->desc.mpeg4.vop_fcode_backward = mpeg4->vop_fcode_backward;
   context->desc.mpeg4.vop_time_increment_resolution =
         mpeg4->vop_time_increment_resolution;
   /* num_gobs_in_vop */
   /* num_macroblocks_in_gob */
   context->desc.mpeg4.trb[0] = mpeg4->TRB;
   context->desc.mpeg4.trb[1] = mpeg4->TRB;
   context->desc.mpeg4.trd[0] = mpeg4->TRD;
   context->desc.mpeg4.trd[1] = mpeg4->TRD;

   /* default [non-]intra quant matrix because mpv does not set these
      matrices */
   if (!context->desc.mpeg4.intra_matrix)
      context->desc.mpeg4.intra_matrix = default_intra_quant_matrix;
   if (!context->desc.mpeg4.non_intra_matrix)
      context->desc.mpeg4.non_intra_matrix = default_non_intra_quant_matrix;

   vlVaGetReferenceFrame(drv, mpeg4->forward_reference_picture, &context->desc.mpeg4.ref[0]);
   vlVaGetReferenceFrame(drv, mpeg4->backward_reference_picture, &context->desc.mpeg4.ref[1]);

   context->mpeg4.vti_bits = 0;
   for (i = context->desc.mpeg4.vop_time_increment_resolution; i > 0; i /= 2)
      ++context->mpeg4.vti_bits;
}

void vlVaHandleIQMatrixBufferMPEG4(vlVaContext *context, vlVaBuffer *buf)
{
   VAIQMatrixBufferMPEG4 *mpeg4 = buf->data;

   assert(buf->size >= sizeof(VAIQMatrixBufferMPEG4) && buf->num_elements == 1);
   if (mpeg4->load_intra_quant_mat)
      context->desc.mpeg4.intra_matrix = mpeg4->intra_quant_mat;
   else
      context->desc.mpeg4.intra_matrix = NULL;

   if (mpeg4->load_non_intra_quant_mat)
      context->desc.mpeg4.non_intra_matrix = mpeg4->non_intra_quant_mat;
   else
      context->desc.mpeg4.non_intra_matrix = NULL;
}

void vlVaHandleSliceParameterBufferMPEG4(vlVaContext *context, vlVaBuffer *buf)
{
   VASliceParameterBufferMPEG4 *mpeg4 = buf->data;

   assert(buf->size >= sizeof(VASliceParameterBufferMPEG4) && buf->num_elements == 1);
   context->mpeg4.quant_scale = mpeg4->quant_scale;
}

struct bit_stream
{
   uint8_t *data;
   unsigned int length; /* bits */
   unsigned int pos;    /* bits */
};

static inline void
write_bit(struct bit_stream *writer, unsigned int bit)
{
   assert(writer->length > (writer)->pos);
   writer->data[writer->pos>>3] |= ((bit & 1)<<(7 - (writer->pos & 7)));
   writer->pos++;
}

static inline void
write_bits(struct bit_stream *writer, unsigned int bits, unsigned int len)
{
   int i;
   assert(len <= sizeof(bits)*8);
   for (i = len - 1; i >= 0; i--)
      write_bit(writer, bits>>i);
}

void vlVaDecoderFixMPEG4Startcode(vlVaContext *context)
{
   uint8_t vop[] = { 0x00, 0x00, 0x01, 0xb6, 0x00, 0x00, 0x00, 0x00, 0x00 };
   struct bit_stream bs_vop = {vop, sizeof(vop)*8, 32};
   unsigned int vop_time_inc;
   int mod_time;
   unsigned int vop_size;
   unsigned int vop_coding_type = context->desc.mpeg4.vop_coding_type;

   context->mpeg4.start_code_size = 0;
   memset(context->mpeg4.start_code, 0, sizeof(context->mpeg4.start_code));
   if (vop_coding_type+1 == PIPE_MPEG12_PICTURE_CODING_TYPE_I) {
      unsigned int vop_time = context->mpeg4.frame_num/
            context->desc.mpeg4.vop_time_increment_resolution;
      unsigned int vop_hour = vop_time / 3600;
      unsigned int vop_minute = (vop_time / 60) % 60;
      unsigned int vop_second = vop_time % 60;
      uint8_t group_of_vop[] = { 0x00, 0x00, 0x01, 0xb3, 0x00, 0x00, 0x00 };
      struct bit_stream bs_gvop = {group_of_vop, sizeof(group_of_vop)*8, 32};

      write_bits(&bs_gvop, vop_hour, 5);
      write_bits(&bs_gvop, vop_minute, 6);
      write_bit(&bs_gvop, 1); /* marker_bit */
      write_bits(&bs_gvop, vop_second, 6);
      write_bit(&bs_gvop, 0); /* closed_gov */ /* TODO replace magic */
      write_bit(&bs_gvop, 0); /* broken_link */
      write_bit(&bs_gvop, 0); /* padding */
      write_bits(&bs_gvop, 7, 3); /* padding */

      memcpy(context->mpeg4.start_code, group_of_vop, sizeof(group_of_vop));
      context->mpeg4.start_code_size += sizeof(group_of_vop);
   }

   write_bits(&bs_vop, vop_coding_type, 2);
   mod_time = context->mpeg4.frame_num %
         context->desc.mpeg4.vop_time_increment_resolution == 0 &&
         vop_coding_type+1 != PIPE_MPEG12_PICTURE_CODING_TYPE_I;
   while (mod_time--)
      write_bit(&bs_vop, 1); /* modulo_time_base */
   write_bit(&bs_vop, 0); /* modulo_time_base */

   write_bit(&bs_vop, 1); /* marker_bit */
   vop_time_inc = context->mpeg4.frame_num %
         context->desc.mpeg4.vop_time_increment_resolution;
   write_bits(&bs_vop, vop_time_inc, context->mpeg4.vti_bits);
   write_bit(&bs_vop, 1); /* marker_bit */
   write_bit(&bs_vop, 1); /* vop_coded */
   if (vop_coding_type+1 == PIPE_MPEG12_PICTURE_CODING_TYPE_P)
      write_bit(&bs_vop, context->mpeg4.pps.vop_fields.bits.vop_rounding_type);
   write_bits(&bs_vop, context->mpeg4.pps.vop_fields.bits.intra_dc_vlc_thr, 3);
   if (context->mpeg4.pps.vol_fields.bits.interlaced) {
      write_bit(&bs_vop, context->mpeg4.pps.vop_fields.bits.top_field_first);
      write_bit(&bs_vop, context->mpeg4.pps.vop_fields.bits.alternate_vertical_scan_flag);
   }

   write_bits(&bs_vop, context->mpeg4.quant_scale, context->mpeg4.pps.quant_precision);
   if (vop_coding_type+1 != PIPE_MPEG12_PICTURE_CODING_TYPE_I)
      write_bits(&bs_vop, context->desc.mpeg4.vop_fcode_forward, 3);
   if (vop_coding_type+1 == PIPE_MPEG12_PICTURE_CODING_TYPE_B)
      write_bits(&bs_vop, context->desc.mpeg4.vop_fcode_backward, 3);

   vop_size = bs_vop.pos/8;
   memcpy(context->mpeg4.start_code + context->mpeg4.start_code_size, vop, vop_size);
   context->mpeg4.start_code_size += vop_size;
}
