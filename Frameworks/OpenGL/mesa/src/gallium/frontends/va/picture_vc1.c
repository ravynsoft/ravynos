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

void vlVaHandlePictureParameterBufferVC1(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VAPictureParameterBufferVC1 * vc1 = buf->data;

   assert(buf->size >= sizeof(VAPictureParameterBufferVC1) && buf->num_elements == 1);
   context->desc.vc1.slice_count = 0;
   vlVaGetReferenceFrame(drv, vc1->forward_reference_picture, &context->desc.vc1.ref[0]);
   vlVaGetReferenceFrame(drv, vc1->backward_reference_picture, &context->desc.vc1.ref[1]);
   context->desc.vc1.picture_type = vc1->picture_fields.bits.picture_type;
   context->desc.vc1.frame_coding_mode = vc1->picture_fields.bits.frame_coding_mode;
   context->desc.vc1.postprocflag = vc1->post_processing != 0;
   context->desc.vc1.pulldown = vc1->sequence_fields.bits.pulldown;
   context->desc.vc1.interlace = vc1->sequence_fields.bits.interlace;
   context->desc.vc1.tfcntrflag = vc1->sequence_fields.bits.tfcntrflag;
   context->desc.vc1.finterpflag = vc1->sequence_fields.bits.finterpflag;
   context->desc.vc1.psf = vc1->sequence_fields.bits.psf;
   context->desc.vc1.dquant = vc1->pic_quantizer_fields.bits.dquant;
   context->desc.vc1.panscan_flag = vc1->entrypoint_fields.bits.panscan_flag;
   context->desc.vc1.refdist_flag =
      vc1->reference_fields.bits.reference_distance_flag;
   context->desc.vc1.quantizer = vc1->pic_quantizer_fields.bits.quantizer;
   context->desc.vc1.extended_mv = vc1->mv_fields.bits.extended_mv_flag;
   context->desc.vc1.extended_dmv = vc1->mv_fields.bits.extended_dmv_flag;
   context->desc.vc1.overlap = vc1->sequence_fields.bits.overlap;
   context->desc.vc1.vstransform =
      vc1->transform_fields.bits.variable_sized_transform_flag;
   context->desc.vc1.loopfilter = vc1->entrypoint_fields.bits.loopfilter;
   context->desc.vc1.fastuvmc = vc1->fast_uvmc_flag;
   context->desc.vc1.range_mapy_flag = vc1->range_mapping_fields.bits.luma_flag;
   context->desc.vc1.range_mapy = vc1->range_mapping_fields.bits.luma;
   context->desc.vc1.range_mapuv_flag = vc1->range_mapping_fields.bits.chroma_flag;
   context->desc.vc1.range_mapuv = vc1->range_mapping_fields.bits.chroma;
   context->desc.vc1.multires = vc1->sequence_fields.bits.multires;
   context->desc.vc1.syncmarker = vc1->sequence_fields.bits.syncmarker;
   context->desc.vc1.rangered = vc1->sequence_fields.bits.rangered;
   context->desc.vc1.maxbframes = vc1->sequence_fields.bits.max_b_frames;
   context->desc.vc1.deblockEnable = vc1->post_processing != 0;
   context->desc.vc1.pquant = vc1->pic_quantizer_fields.bits.pic_quantizer_scale;
}

void vlVaHandleSliceParameterBufferVC1(vlVaContext *context, vlVaBuffer *buf)
{
   assert(buf->size >= sizeof(VASliceParameterBufferVC1) && buf->num_elements == 1);
   context->desc.vc1.slice_count += buf->num_elements;
}
