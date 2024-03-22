 /**************************************************************************
 *
 * Copyright 2010 Thomas Balling Sørensen & Orasanu Lucian.
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

#include "util/u_video.h"
#include "util/u_handle_table.h"
#include "va_private.h"

static void resetReferencePictureDesc(struct pipe_h264_picture_desc *h264,
                                      unsigned int i)
{
   h264->ref[i] = NULL;
   h264->frame_num_list[i] = 0;
   h264->is_long_term[i] = 0;
   h264->top_is_reference[i] = 0;
   h264->bottom_is_reference[i] = 0;
   h264->field_order_cnt_list[i][0] = 0;
   h264->field_order_cnt_list[i][1] = 0;
}

void vlVaHandlePictureParameterBufferH264(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VAPictureParameterBufferH264 *h264 = buf->data;
   unsigned int top_or_bottom_field;
   bool is_ref;
   unsigned i;

   assert(buf->size >= sizeof(VAPictureParameterBufferH264) && buf->num_elements == 1);
   context->desc.h264.slice_count = 0;
   /*CurrPic*/
   context->desc.h264.field_order_cnt[0] = h264->CurrPic.TopFieldOrderCnt;
   context->desc.h264.field_order_cnt[1] = h264->CurrPic.BottomFieldOrderCnt;
   /*ReferenceFrames[16]*/
   /*picture_width_in_mbs_minus1*/
   /*picture_height_in_mbs_minus1*/
   context->desc.h264.pps->sps->bit_depth_luma_minus8 = h264->bit_depth_luma_minus8;
   context->desc.h264.pps->sps->bit_depth_chroma_minus8 = h264->bit_depth_chroma_minus8;
   context->desc.h264.num_ref_frames = h264->num_ref_frames;
   context->desc.h264.pps->sps->chroma_format_idc = h264->seq_fields.bits.chroma_format_idc;
   /*residual_colour_transform_flag*/
   /*gaps_in_frame_num_value_allowed_flag*/
   context->desc.h264.pps->sps->frame_mbs_only_flag =
      h264->seq_fields.bits.frame_mbs_only_flag;
   context->desc.h264.pps->sps->mb_adaptive_frame_field_flag =
      h264->seq_fields.bits.mb_adaptive_frame_field_flag;
   context->desc.h264.pps->sps->direct_8x8_inference_flag =
      h264->seq_fields.bits.direct_8x8_inference_flag;
   context->desc.h264.pps->sps->MinLumaBiPredSize8x8 =
      h264->seq_fields.bits.MinLumaBiPredSize8x8;
   context->desc.h264.pps->sps->log2_max_frame_num_minus4 =
      h264->seq_fields.bits.log2_max_frame_num_minus4;
   context->desc.h264.pps->sps->pic_order_cnt_type =
      h264->seq_fields.bits.pic_order_cnt_type;
   context->desc.h264.pps->sps->log2_max_pic_order_cnt_lsb_minus4 =
      h264->seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4;
   context->desc.h264.pps->sps->delta_pic_order_always_zero_flag =
      h264->seq_fields.bits.delta_pic_order_always_zero_flag;
   /*num_slice_groups_minus1*/
   /*slice_group_map_type*/
   /*slice_group_change_rate_minus1*/
   context->desc.h264.pps->pic_init_qp_minus26 =
      h264->pic_init_qp_minus26;
   context->desc.h264.pps->pic_init_qs_minus26 =
      h264->pic_init_qs_minus26;
   context->desc.h264.pps->chroma_qp_index_offset =
      h264->chroma_qp_index_offset;
   context->desc.h264.pps->second_chroma_qp_index_offset =
      h264->second_chroma_qp_index_offset;
   context->desc.h264.pps->entropy_coding_mode_flag =
      h264->pic_fields.bits.entropy_coding_mode_flag;
   context->desc.h264.pps->weighted_pred_flag =
      h264->pic_fields.bits.weighted_pred_flag;
   context->desc.h264.pps->weighted_bipred_idc =
      h264->pic_fields.bits.weighted_bipred_idc;
   context->desc.h264.pps->transform_8x8_mode_flag =
      h264->pic_fields.bits.transform_8x8_mode_flag;
   context->desc.h264.field_pic_flag =
      h264->pic_fields.bits.field_pic_flag;
   context->desc.h264.pps->constrained_intra_pred_flag =
      h264->pic_fields.bits.constrained_intra_pred_flag;
   context->desc.h264.pps->bottom_field_pic_order_in_frame_present_flag =
      h264->pic_fields.bits.pic_order_present_flag;
   context->desc.h264.pps->deblocking_filter_control_present_flag =
      h264->pic_fields.bits.deblocking_filter_control_present_flag;
   context->desc.h264.pps->redundant_pic_cnt_present_flag =
      h264->pic_fields.bits.redundant_pic_cnt_present_flag;
   /*reference_pic_flag*/
   context->desc.h264.frame_num = h264->frame_num;
   context->desc.h264.is_reference = h264->pic_fields.bits.reference_pic_flag;
   context->desc.h264.bottom_field_flag =
      h264->pic_fields.bits.field_pic_flag &&
      (h264->CurrPic.flags & VA_PICTURE_H264_BOTTOM_FIELD) != 0;

   if (context->decoder && (context->templat.max_references != context->desc.h264.num_ref_frames)) {
      context->templat.max_references = MIN2(context->desc.h264.num_ref_frames, 16);
      context->decoder->destroy(context->decoder);
      context->decoder = NULL;
   } else if (!context->decoder && context->desc.h264.num_ref_frames > 0)
      context->templat.max_references = MIN2(context->desc.h264.num_ref_frames, 16);

   for (i = 0; i < context->templat.max_references; ++i) {
      if ((h264->ReferenceFrames[i].flags & VA_PICTURE_H264_INVALID) ||
          (h264->ReferenceFrames[i].picture_id == VA_INVALID_SURFACE)) {
         resetReferencePictureDesc(&context->desc.h264, i);
         break;
      }

      vlVaGetReferenceFrame(drv, h264->ReferenceFrames[i].picture_id, &context->desc.h264.ref[i]);
      context->desc.h264.frame_num_list[i] = h264->ReferenceFrames[i].frame_idx;

      top_or_bottom_field = h264->ReferenceFrames[i].flags &
         (VA_PICTURE_H264_TOP_FIELD | VA_PICTURE_H264_BOTTOM_FIELD);
      is_ref = !!(h264->ReferenceFrames[i].flags &
         (VA_PICTURE_H264_SHORT_TERM_REFERENCE | VA_PICTURE_H264_LONG_TERM_REFERENCE));
      context->desc.h264.is_long_term[i] = !!(h264->ReferenceFrames[i].flags &
          VA_PICTURE_H264_LONG_TERM_REFERENCE);
      context->desc.h264.top_is_reference[i] =
         !!(h264->ReferenceFrames[i].flags & VA_PICTURE_H264_TOP_FIELD) ||
         ((!top_or_bottom_field) && is_ref);
      context->desc.h264.bottom_is_reference[i] =
         !!(h264->ReferenceFrames[i].flags & VA_PICTURE_H264_BOTTOM_FIELD) ||
         ((!top_or_bottom_field) && is_ref);
      context->desc.h264.field_order_cnt_list[i][0] =
         top_or_bottom_field != VA_PICTURE_H264_BOTTOM_FIELD ?
         h264->ReferenceFrames[i].TopFieldOrderCnt: INT_MAX;
      context->desc.h264.field_order_cnt_list[i][1] =
         top_or_bottom_field != VA_PICTURE_H264_TOP_FIELD ?
         h264->ReferenceFrames[i].BottomFieldOrderCnt: INT_MAX;
   }

   /* Make sure remaining elements are clean */
   for (; i < 16; ++i)
      resetReferencePictureDesc(&context->desc.h264, i);

   context->desc.h264.slice_parameter.slice_info_present = false;
   memset(context->desc.h264.slice_parameter.slice_data_flag, 0, sizeof(context->desc.h264.slice_parameter.slice_data_flag));
   memset(context->desc.h264.slice_parameter.slice_data_offset, 0, sizeof(context->desc.h264.slice_parameter.slice_data_offset));
   memset(context->desc.h264.slice_parameter.slice_data_size, 0, sizeof(context->desc.h264.slice_parameter.slice_data_size));

}

void vlVaHandleIQMatrixBufferH264(vlVaContext *context, vlVaBuffer *buf)
{
   VAIQMatrixBufferH264 *h264 = buf->data;

   assert(buf->size >= sizeof(VAIQMatrixBufferH264) && buf->num_elements == 1);
   memcpy(&context->desc.h264.pps->ScalingList4x4, h264->ScalingList4x4, 6 * 16);
   memcpy(&context->desc.h264.pps->ScalingList8x8, h264->ScalingList8x8, 2 * 64);
}

void vlVaHandleSliceParameterBufferH264(vlVaContext *context, vlVaBuffer *buf)
{
   VASliceParameterBufferH264 *h264 = buf->data;

   assert(buf->size >= sizeof(VASliceParameterBufferH264) && buf->num_elements == 1);
   context->desc.h264.num_ref_idx_l0_active_minus1 =
      h264->num_ref_idx_l0_active_minus1;
   context->desc.h264.num_ref_idx_l1_active_minus1 =
      h264->num_ref_idx_l1_active_minus1;

   ASSERTED const size_t max_pipe_h264_slices = ARRAY_SIZE(context->desc.h264.slice_parameter.slice_data_offset);
   assert(context->desc.h264.slice_count < max_pipe_h264_slices);

   context->desc.h264.slice_parameter.slice_info_present = true;
   context->desc.h264.slice_parameter.slice_data_size[context->desc.h264.slice_count] = h264->slice_data_size;
   context->desc.h264.slice_parameter.slice_data_offset[context->desc.h264.slice_count] = h264->slice_data_offset;

   switch (h264->slice_data_flag) {
   case VA_SLICE_DATA_FLAG_ALL:
      context->desc.h264.slice_parameter.slice_data_flag[context->desc.h264.slice_count] = PIPE_SLICE_BUFFER_PLACEMENT_TYPE_WHOLE;
      break;
   case VA_SLICE_DATA_FLAG_BEGIN:
      context->desc.h264.slice_parameter.slice_data_flag[context->desc.h264.slice_count] = PIPE_SLICE_BUFFER_PLACEMENT_TYPE_BEGIN;
      break;
   case VA_SLICE_DATA_FLAG_MIDDLE:
      context->desc.h264.slice_parameter.slice_data_flag[context->desc.h264.slice_count] = PIPE_SLICE_BUFFER_PLACEMENT_TYPE_MIDDLE;
      break;
   case VA_SLICE_DATA_FLAG_END:
      context->desc.h264.slice_parameter.slice_data_flag[context->desc.h264.slice_count] = PIPE_SLICE_BUFFER_PLACEMENT_TYPE_END;
      break;
   default:
      break;
   }

   /* assert(buf->num_elements == 1) above; */
   context->desc.h264.slice_count += buf->num_elements;
}
