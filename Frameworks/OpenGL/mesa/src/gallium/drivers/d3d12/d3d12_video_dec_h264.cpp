/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "d3d12_video_dec.h"
#include "d3d12_video_dec_h264.h"
#include "vl/vl_zscan.h"

#include <cmath>

void
d3d12_video_decoder_refresh_dpb_active_references_h264(struct d3d12_video_decoder *pD3D12Dec)
{
   // Method overview
   // 1. Codec specific strategy in switch statement regarding reference frames eviction policy. Should only mark active
   // DPB references, leaving evicted ones as unused
   // 2. Call release_unused_references_texture_memory(); at the end of this method. Any references (and texture
   // allocations associated)
   //    that were left not marked as used in m_spDPBManager by step (2) are lost.

   // Assign DXVA original Index7Bits indices to current frame and references
   DXVA_PicParams_H264 *pCurrPicParams = d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_H264>(pD3D12Dec);
   for (uint8_t i = 0; i < 16; i++) {
      // From H264 DXVA spec:
      // Index7Bits
      //     An index that identifies an uncompressed surface for the CurrPic or RefFrameList member of the picture
      //     parameters structure(section 4.0) or the RefPicList member of the slice control data
      //     structure(section 6.0) When Index7Bits is used in the CurrPic and RefFrameList members of the picture
      //     parameters structure, the value directly specifies the DXVA index of an uncompressed surface. When
      //     Index7Bits is used in the RefPicList member of the slice control data structure, the value identifies
      //     the surface indirectly, as an index into the RefFrameList array of the associated picture parameters
      //     structure.For more information, see section 6.2. In all cases, when Index7Bits does not contain a valid
      //     index, the value is 127.
      if (pCurrPicParams->RefFrameList[i].bPicEntry != DXVA_H264_INVALID_PICTURE_ENTRY_VALUE) {
         pCurrPicParams->RefFrameList[i].Index7Bits =
            pD3D12Dec->m_spDPBManager->get_index7bits(pD3D12Dec->m_pCurrentReferenceTargets[i]);
      }
   }

   pD3D12Dec->m_spDPBManager->mark_all_references_as_unused();
   pD3D12Dec->m_spDPBManager->mark_references_in_use(pCurrPicParams->RefFrameList);

   // Releases the underlying reference picture texture objects of all references that were not marked as used in this
   // method.
   pD3D12Dec->m_spDPBManager->release_unused_references_texture_memory();

   pCurrPicParams->CurrPic.Index7Bits = pD3D12Dec->m_spDPBManager->get_index7bits(pD3D12Dec->m_pCurrentDecodeTarget);

   debug_printf("[d3d12_video_decoder_store_converted_dxva_picparams_from_pipe_input] DXVA_PicParams_H264 converted "
                 "from pipe_h264_picture_desc (No reference index remapping)\n");
   d3d12_video_decoder_log_pic_params_h264(pCurrPicParams);
}

void
d3d12_video_decoder_get_frame_info_h264(
   struct d3d12_video_decoder *pD3D12Dec, uint32_t *pWidth, uint32_t *pHeight, uint16_t *pMaxDPB, bool &isInterlaced)
{
   auto pPicParams = d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_H264>(pD3D12Dec);
   // wFrameWidthInMbsMinus1 Width of the frame containing this picture, in units of macroblocks, minus 1. (The width in
   // macroblocks is wFrameWidthInMbsMinus1 plus 1.) wFrameHeightInMbsMinus1 Height of the frame containing this
   // picture, in units of macroblocks, minus 1. (The height in macroblocks is wFrameHeightInMbsMinus1 plus 1.) When the
   // picture is a field, the height of the frame is twice the height of the picture and is an integer multiple of 2 in
   // units of macroblocks.
   *pWidth = (pPicParams->wFrameWidthInMbsMinus1 + 1) * 16;
   *pHeight = (pPicParams->wFrameHeightInMbsMinus1 + 1) / (pPicParams->frame_mbs_only_flag ? 1 : 2);
   *pHeight = (2 - pPicParams->frame_mbs_only_flag) * *pHeight;
   *pHeight = *pHeight * 16;
   *pMaxDPB = pPicParams->num_ref_frames + 1;
   isInterlaced = !pPicParams->frame_mbs_only_flag;
}

///
/// Pushes the current frame as next reference, updates the DXVA H264 structure with the indices of the DPB and
/// transitions the references
///
void
d3d12_video_decoder_prepare_current_frame_references_h264(struct d3d12_video_decoder *pD3D12Dec,
                                                          ID3D12Resource *pTexture2D,
                                                          uint32_t subresourceIndex)
{
   DXVA_PicParams_H264 *pPicParams = d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_H264>(pD3D12Dec);
   pPicParams->CurrPic.Index7Bits = pD3D12Dec->m_spDPBManager->store_future_reference(pPicParams->CurrPic.Index7Bits,
                                                                                      pD3D12Dec->m_spVideoDecoderHeap,
                                                                                      pTexture2D,
                                                                                      subresourceIndex);

   // From H264 DXVA spec:
   // Index7Bits
   //     An index that identifies an uncompressed surface for the CurrPic or RefFrameList member of the picture
   //     parameters structure(section 4.0) or the RefPicList member of the slice control data structure(section 6.0)
   //     When Index7Bits is used in the CurrPic and RefFrameList members of the picture parameters structure, the value
   //     directly specifies the DXVA index of an uncompressed surface. When Index7Bits is used in the RefPicList member
   //     of the slice control data structure, the value identifies the surface indirectly, as an index into the
   //     RefFrameList array of the associated picture parameters structure.For more information, see section 6.2. In
   //     all cases, when Index7Bits does not contain a valid index, the value is 127.

   pD3D12Dec->m_spDPBManager->update_entries(
      d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_H264>(pD3D12Dec)->RefFrameList,
      pD3D12Dec->m_transitionsStorage);

   pD3D12Dec->m_spDecodeCommandList->ResourceBarrier(pD3D12Dec->m_transitionsStorage.size(), pD3D12Dec->m_transitionsStorage.data());

   // Schedule reverse (back to common) transitions before command list closes for current frame
   for (auto BarrierDesc : pD3D12Dec->m_transitionsStorage) {
      std::swap(BarrierDesc.Transition.StateBefore, BarrierDesc.Transition.StateAfter);
      pD3D12Dec->m_transitionsBeforeCloseCmdList.push_back(BarrierDesc);
   }

   debug_printf(
      "[d3d12_video_decoder_prepare_current_frame_references_h264] DXVA_PicParams_H264 after index remapping)\n");
   d3d12_video_decoder_log_pic_params_h264(
      d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_H264>(pD3D12Dec));
}

void
d3d12_video_decoder_prepare_dxva_slices_control_h264(struct d3d12_video_decoder *pD3D12Dec,
                                                     std::vector<uint8_t> &vecOutSliceControlBuffers,
                                                     struct pipe_h264_picture_desc *picture_h264)
{
   uint64_t TotalSlicesDXVAArrayByteSize = picture_h264->slice_count * sizeof(DXVA_Slice_H264_Short);
   vecOutSliceControlBuffers.resize(TotalSlicesDXVAArrayByteSize);
   uint8_t* pData = vecOutSliceControlBuffers.data();
   assert(picture_h264->slice_parameter.slice_info_present);
   debug_printf("[d3d12_video_decoder_h264] Upper layer reported %d slices for this frame...\n",
                  picture_h264->slice_count);

   static const uint32_t start_code_size = 3;
   uint32_t acum_slice_offset = (picture_h264->slice_count > 0) ? picture_h264->slice_parameter.slice_data_offset[0] : 0;
   for (uint32_t sliceIdx = 0; sliceIdx < picture_h264->slice_count; sliceIdx++)
   {
      DXVA_Slice_H264_Short* currentSliceEntry = (DXVA_Slice_H264_Short*) pData;
      // From H264 DXVA Spec
      // wBadSliceChopping
      // 0	All bits for the slice are located within the corresponding bitstream data buffer.
      // 1	The bitstream data buffer contains the start of the slice, but not the entire slice, because the buffer is full.
      // 2	The bitstream data buffer contains the end of the slice. It does not contain the start of the slice, because the start of the slice was located in the previous bitstream data buffer.
      // 3	The bitstream data buffer does not contain the start of the slice (because the start of the slice was located in the previous bitstream data buffer),
      //     and it does not contain the end of the slice (because the current bitstream data buffer is also full).

      switch (picture_h264->slice_parameter.slice_data_flag[sliceIdx]) {
         /* whole slice is in the buffer */
         case PIPE_SLICE_BUFFER_PLACEMENT_TYPE_WHOLE:
            currentSliceEntry->wBadSliceChopping = 0u;
            break;
         /* The beginning of the slice is in the buffer but the end is not */
         case PIPE_SLICE_BUFFER_PLACEMENT_TYPE_BEGIN:
            currentSliceEntry->wBadSliceChopping = 1u;
            break;
         /* Neither beginning nor end of the slice is in the buffer */
         case PIPE_SLICE_BUFFER_PLACEMENT_TYPE_MIDDLE:
            currentSliceEntry->wBadSliceChopping = 3u;
            break;
         /* end of the slice is in the buffer */
         case PIPE_SLICE_BUFFER_PLACEMENT_TYPE_END:
            currentSliceEntry->wBadSliceChopping = 2u;
            break;
         default:
         {
            unreachable("Unsupported pipe_slice_buffer_placement_type");
         } break;
      }

      /* slice_data_size from pipe/va does not include the NAL unit size, DXVA requires it */
      currentSliceEntry->SliceBytesInBuffer = picture_h264->slice_parameter.slice_data_size[sliceIdx] + start_code_size;

      /* slice_data_offset from pipe/va are relative to the current slice, and in DXVA they are absolute within the frame source buffer */
      currentSliceEntry->BSNALunitDataLocation = acum_slice_offset;
      acum_slice_offset += (currentSliceEntry->SliceBytesInBuffer + picture_h264->slice_parameter.slice_data_offset[sliceIdx]);

      debug_printf("[d3d12_video_decoder_h264] Reported slice index %" PRIu32 " with SliceBytesInBuffer %d - BSNALunitDataLocation %d - wBadSliceChopping: %" PRIu16
                  " for frame with "
                  "fenceValue: %d\n",
                  sliceIdx,
                  currentSliceEntry->SliceBytesInBuffer,
                  currentSliceEntry->BSNALunitDataLocation,
                  currentSliceEntry->wBadSliceChopping,
                  pD3D12Dec->m_fenceValue);

      pData += sizeof(DXVA_Slice_H264_Short);
   }
   assert(vecOutSliceControlBuffers.size() == TotalSlicesDXVAArrayByteSize);
}

static void
d3d12_video_decoder_log_pic_entry_h264(DXVA_PicEntry_H264 &picEntry)
{
   debug_printf("\t\tIndex7Bits: %d\n"
                 "\t\tAssociatedFlag: %d\n"
                 "\t\tbPicEntry: %d\n",
                 picEntry.Index7Bits,
                 picEntry.AssociatedFlag,
                 picEntry.bPicEntry);
}

void
d3d12_video_decoder_log_pic_params_h264(DXVA_PicParams_H264 *pPicParams)
{
   debug_printf("\n=============================================\n");
   debug_printf("wFrameWidthInMbsMinus1 = %d\n", pPicParams->wFrameWidthInMbsMinus1);
   debug_printf("wFrameHeightInMbsMinus1 = %d\n", pPicParams->wFrameHeightInMbsMinus1);
   debug_printf("CurrPic.Index7Bits = %d\n", pPicParams->CurrPic.Index7Bits);
   debug_printf("CurrPic.AssociatedFlag = %d\n", pPicParams->CurrPic.AssociatedFlag);
   debug_printf("num_ref_frames = %d\n", pPicParams->num_ref_frames);
   debug_printf("sp_for_switch_flag = %d\n", pPicParams->sp_for_switch_flag);
   debug_printf("field_pic_flag = %d\n", pPicParams->field_pic_flag);
   debug_printf("MbaffFrameFlag = %d\n", pPicParams->MbaffFrameFlag);
   debug_printf("residual_colour_transform_flag = %d\n", pPicParams->residual_colour_transform_flag);
   debug_printf("chroma_format_idc = %d\n", pPicParams->chroma_format_idc);
   debug_printf("RefPicFlag = %d\n", pPicParams->RefPicFlag);
   debug_printf("IntraPicFlag = %d\n", pPicParams->IntraPicFlag);
   debug_printf("constrained_intra_pred_flag = %d\n", pPicParams->constrained_intra_pred_flag);
   debug_printf("MinLumaBipredSize8x8Flag = %d\n", pPicParams->MinLumaBipredSize8x8Flag);
   debug_printf("weighted_pred_flag = %d\n", pPicParams->weighted_pred_flag);
   debug_printf("weighted_bipred_idc = %d\n", pPicParams->weighted_bipred_idc);
   debug_printf("MbsConsecutiveFlag = %d\n", pPicParams->MbsConsecutiveFlag);
   debug_printf("frame_mbs_only_flag = %d\n", pPicParams->frame_mbs_only_flag);
   debug_printf("transform_8x8_mode_flag = %d\n", pPicParams->transform_8x8_mode_flag);
   debug_printf("StatusReportFeedbackNumber = %d\n", pPicParams->StatusReportFeedbackNumber);
   debug_printf("CurrFieldOrderCnt[0] = %d\n", pPicParams->CurrFieldOrderCnt[0]);
   debug_printf("CurrFieldOrderCnt[1] = %d\n", pPicParams->CurrFieldOrderCnt[1]);
   debug_printf("chroma_qp_index_offset = %d\n", pPicParams->chroma_qp_index_offset);
   debug_printf("second_chroma_qp_index_offset = %d\n", pPicParams->second_chroma_qp_index_offset);
   debug_printf("ContinuationFlag = %d\n", pPicParams->ContinuationFlag);
   debug_printf("pic_init_qp_minus26 = %d\n", pPicParams->pic_init_qp_minus26);
   debug_printf("pic_init_qs_minus26 = %d\n", pPicParams->pic_init_qs_minus26);
   debug_printf("num_ref_idx_l0_active_minus1 = %d\n", pPicParams->num_ref_idx_l0_active_minus1);
   debug_printf("num_ref_idx_l1_active_minus1 = %d\n", pPicParams->num_ref_idx_l1_active_minus1);
   debug_printf("frame_num = %d\n", pPicParams->frame_num);
   debug_printf("log2_max_frame_num_minus4 = %d\n", pPicParams->log2_max_frame_num_minus4);
   debug_printf("pic_order_cnt_type = %d\n", pPicParams->pic_order_cnt_type);
   debug_printf("log2_max_pic_order_cnt_lsb_minus4 = %d\n", pPicParams->log2_max_pic_order_cnt_lsb_minus4);
   debug_printf("delta_pic_order_always_zero_flag = %d\n", pPicParams->delta_pic_order_always_zero_flag);
   debug_printf("direct_8x8_inference_flag = %d\n", pPicParams->direct_8x8_inference_flag);
   debug_printf("entropy_coding_mode_flag = %d\n", pPicParams->entropy_coding_mode_flag);
   debug_printf("pic_order_present_flag = %d\n", pPicParams->pic_order_present_flag);
   debug_printf("deblocking_filter_control_present_flag = %d\n", pPicParams->deblocking_filter_control_present_flag);
   debug_printf("redundant_pic_cnt_present_flag = %d\n", pPicParams->redundant_pic_cnt_present_flag);
   debug_printf("num_slice_groups_minus1 = %d\n", pPicParams->num_slice_groups_minus1);
   debug_printf("slice_group_map_type = %d\n", pPicParams->slice_group_map_type);
   debug_printf("slice_group_change_rate_minus1 = %d\n", pPicParams->slice_group_change_rate_minus1);
   debug_printf("Reserved8BitsB = %d\n", pPicParams->Reserved8BitsB);
   debug_printf("UsedForReferenceFlags 0x%08x\n", pPicParams->UsedForReferenceFlags);
   debug_printf("NonExistingFrameFlags 0x%08x\n", pPicParams->NonExistingFrameFlags);

   const UINT16 RefPicListLength = _countof(DXVA_PicParams_H264::RefFrameList);

   debug_printf("[D3D12 Video Decoder H264 DXVA PicParams info]\n"
                 "\t[Current Picture Entry]\n");
   d3d12_video_decoder_log_pic_entry_h264(pPicParams->CurrPic);

   debug_printf("[Decode RefFrameList Pic_Entry list] Entries where bPicEntry == "
                 "DXVA_H264_INVALID_PICTURE_ENTRY_VALUE are not printed\n");
   for (uint32_t refIdx = 0; refIdx < RefPicListLength; refIdx++) {
      if (DXVA_H264_INVALID_PICTURE_ENTRY_VALUE != pPicParams->RefFrameList[refIdx].bPicEntry) {
         debug_printf("\t[Reference PicEntry %d]\n", refIdx);
         d3d12_video_decoder_log_pic_entry_h264(pPicParams->RefFrameList[refIdx]);
         debug_printf("\t\tFrameNumList: %d\n"
                       "\t\tFieldOrderCntList[0]: %d\n"
                       "\t\tFieldOrderCntList[1]: %d\n",
                       pPicParams->FrameNumList[refIdx],
                       pPicParams->FieldOrderCntList[refIdx][0],
                       pPicParams->FieldOrderCntList[refIdx][1]);
      }
   }
}

DXVA_PicParams_H264
d3d12_video_decoder_dxva_picparams_from_pipe_picparams_h264(
   uint32_t frameNum,
   pipe_video_profile profile,
   uint32_t decodeWidth,    // pipe_h264_picture_desc doesn't have the size of the frame for H264, but it does for other
                            // codecs.
   uint32_t decodeHeight,   // pipe_h264_picture_desc doesn't have the size of the frame for H264, but it does for other
                            // codecs.
   pipe_h264_picture_desc *pPipeDesc)
{
   DXVA_PicParams_H264 dxvaStructure = {};

   // uint16_t  wFrameWidthInMbsMinus1;
   uint width_in_mb = decodeWidth / D3D12_VIDEO_H264_MB_IN_PIXELS;
   dxvaStructure.wFrameWidthInMbsMinus1 = width_in_mb - 1;
   // uint16_t  wFrameHeightInMbsMinus1;
   uint height_in_mb = static_cast<uint>(std::ceil(decodeHeight / D3D12_VIDEO_H264_MB_IN_PIXELS));
   dxvaStructure.wFrameHeightInMbsMinus1 = height_in_mb - 1;

   // CurrPic.Index7Bits is handled by d3d12_video_decoder_refresh_dpb_active_references_h264
   // CurrPic.AssociatedFlag
   // If field_pic_flag is 1, the AssociatedFlag field in CurrPic is interpreted as follows:
   // 0 -> The current picture is the top field of the uncompressed destination frame surface.
   // 1 -> The current picture is the bottom field of the uncompressed destination frame surface.
   // If field_pic_flag is 0, AssociatedFlag has no meaning and shall be 0, and the accelerator shall ignore the value.
   if (pPipeDesc->field_pic_flag) {
      dxvaStructure.CurrPic.AssociatedFlag = (pPipeDesc->bottom_field_flag == 0) ? 0 : 1;
   } else {
      dxvaStructure.CurrPic.AssociatedFlag = 0;
   }

   // uint8_t   num_ref_frames;
   dxvaStructure.num_ref_frames = pPipeDesc->num_ref_frames;
   // union {
   // struct {
   // uint16_t  field_pic_flag                 : 1;
   dxvaStructure.field_pic_flag = pPipeDesc->field_pic_flag;
   // From H264 codec spec
   // The variable MbaffFrameFlag is derived as
   // MbaffFrameFlag = ( mb_adaptive_frame_field_flag && !field_pic_flag )
   dxvaStructure.MbaffFrameFlag = (pPipeDesc->pps->sps->mb_adaptive_frame_field_flag && !pPipeDesc->field_pic_flag);
   // uint16_t  residual_colour_transform_flag :1
   dxvaStructure.residual_colour_transform_flag = pPipeDesc->pps->sps->separate_colour_plane_flag;
   // uint16_t sp_for_switch_flag // switch slices are not supported by VA
   dxvaStructure.sp_for_switch_flag = 0;
   // uint16_t  chroma_format_idc              : 2;
   assert(pPipeDesc->pps->sps->chroma_format_idc == 1);   // Not supported otherwise
   dxvaStructure.chroma_format_idc = 1;   // This is always 4:2:0 for D3D12 Video. NV12/P010 DXGI formats only.
   // uint16_t  RefPicFlag                     : 1;
   dxvaStructure.RefPicFlag = pPipeDesc->is_reference;

   // uint16_t  constrained_intra_pred_flag    : 1;
   dxvaStructure.constrained_intra_pred_flag = pPipeDesc->pps->constrained_intra_pred_flag;
   // uint16_t  weighted_pred_flag             : 1;
   dxvaStructure.weighted_pred_flag = pPipeDesc->pps->weighted_pred_flag;
   // uint16_t  weighted_bipred_idc            : 2;
   dxvaStructure.weighted_bipred_idc = pPipeDesc->pps->weighted_bipred_idc;
   // From DXVA spec:
   // The value shall be 1 unless the restricted-mode profile in use explicitly supports the value 0.
   // FMO is not supported by VAAPI
   dxvaStructure.MbsConsecutiveFlag = 1;
   // uint16_t  frame_mbs_only_flag            : 1;
   dxvaStructure.frame_mbs_only_flag = pPipeDesc->pps->sps->frame_mbs_only_flag;
   // uint16_t  transform_8x8_mode_flag        : 1;
   dxvaStructure.transform_8x8_mode_flag = pPipeDesc->pps->transform_8x8_mode_flag;
   // };
   // uint16_t  wBitFields;
   // };
   // uint8_t  bit_depth_luma_minus8;
   dxvaStructure.bit_depth_luma_minus8 = pPipeDesc->pps->sps->bit_depth_luma_minus8;
   assert(dxvaStructure.bit_depth_luma_minus8 == 0);   // Only support for NV12 now
   // uint8_t  bit_depth_chroma_minus8;
   dxvaStructure.bit_depth_chroma_minus8 = pPipeDesc->pps->sps->bit_depth_chroma_minus8;
   assert(dxvaStructure.bit_depth_chroma_minus8 == 0);   // Only support for NV12 now
   // uint16_t MinLumaBipredSize8x8Flag
   dxvaStructure.MinLumaBipredSize8x8Flag = pPipeDesc->pps->sps->MinLumaBiPredSize8x8;
   // char pic_init_qs_minus26
   dxvaStructure.pic_init_qs_minus26 = pPipeDesc->pps->pic_init_qs_minus26;
   // uint8_t   chroma_qp_index_offset;   /* also used for QScb */
   dxvaStructure.chroma_qp_index_offset = pPipeDesc->pps->chroma_qp_index_offset;
   // uint8_t   second_chroma_qp_index_offset; /* also for QScr */
   dxvaStructure.second_chroma_qp_index_offset = pPipeDesc->pps->second_chroma_qp_index_offset;

   /* remainder for parsing */
   // uint8_t   pic_init_qp_minus26;
   dxvaStructure.pic_init_qp_minus26 = pPipeDesc->pps->pic_init_qp_minus26;
   // uint8_t  num_ref_idx_l0_active_minus1;
   dxvaStructure.num_ref_idx_l0_active_minus1 = pPipeDesc->num_ref_idx_l0_active_minus1;
   // uint8_t  num_ref_idx_l1_active_minus1;
   dxvaStructure.num_ref_idx_l1_active_minus1 = pPipeDesc->num_ref_idx_l1_active_minus1;

   // uint16_t frame_num;
   dxvaStructure.frame_num = pPipeDesc->frame_num;

   // uint8_t  log2_max_frame_num_minus4;
   dxvaStructure.log2_max_frame_num_minus4 = pPipeDesc->pps->sps->log2_max_frame_num_minus4;
   // uint8_t  pic_order_cnt_type;
   dxvaStructure.pic_order_cnt_type = pPipeDesc->pps->sps->pic_order_cnt_type;
   // uint8_t  log2_max_pic_order_cnt_lsb_minus4;
   dxvaStructure.log2_max_pic_order_cnt_lsb_minus4 = pPipeDesc->pps->sps->log2_max_pic_order_cnt_lsb_minus4;
   // uint8_t  delta_pic_order_always_zero_flag;
   dxvaStructure.delta_pic_order_always_zero_flag = pPipeDesc->pps->sps->delta_pic_order_always_zero_flag;
   // uint8_t  direct_8x8_inference_flag;
   dxvaStructure.direct_8x8_inference_flag = pPipeDesc->pps->sps->direct_8x8_inference_flag;
   // uint8_t  entropy_coding_mode_flag;
   dxvaStructure.entropy_coding_mode_flag = pPipeDesc->pps->entropy_coding_mode_flag;
   // uint8_t  num_slice_groups_minus1;
   dxvaStructure.num_slice_groups_minus1 = pPipeDesc->pps->num_slice_groups_minus1;
   assert(dxvaStructure.num_slice_groups_minus1 == 0);   // FMO Not supported by VA

   // uint8_t  slice_group_map_type;
   dxvaStructure.slice_group_map_type = pPipeDesc->pps->slice_group_map_type;
   // uint8_t  deblocking_filter_control_present_flag;
   dxvaStructure.deblocking_filter_control_present_flag = pPipeDesc->pps->deblocking_filter_control_present_flag;
   // uint8_t  redundant_pic_cnt_present_flag;
   dxvaStructure.redundant_pic_cnt_present_flag = pPipeDesc->pps->redundant_pic_cnt_present_flag;
   // uint16_t slice_group_change_rate_minus1;
   dxvaStructure.slice_group_change_rate_minus1 = pPipeDesc->pps->slice_group_change_rate_minus1;

   // int32_t    CurrFieldOrderCnt[2];
   dxvaStructure.CurrFieldOrderCnt[0] = pPipeDesc->field_order_cnt[0];
   dxvaStructure.CurrFieldOrderCnt[1] = pPipeDesc->field_order_cnt[1];

   // DXVA_PicEntry_H264  RefFrameList[16]; /* DXVA_PicEntry_H264.AssociatedFlag 1 means LongTermRef */
   // From DXVA spec:
   // RefFrameList
   // Contains a list of 16 uncompressed frame buffer surfaces.  All uncompressed surfaces that correspond to pictures
   // currently marked as "used for reference" must appear in the RefFrameList array. Non-reference surfaces (those
   // which only contain pictures for which the value of RefPicFlag was 0 when the picture was decoded) shall not appear
   // in RefFrameList for a subsequent picture. In addition, surfaces that contain only pictures marked as "unused for
   // reference" shall not appear in RefFrameList for a subsequent picture.

   dxvaStructure.UsedForReferenceFlags = 0;   // initialize to zero and set only the appropiate values below

   bool frameUsesAnyRefPicture = false;
   for (uint i = 0; i < 16; i++) {
      // Fix ad-hoc behaviour from the VA upper layer which always marks short term references as top_is_reference and
      // bottom_is_reference as true and then differenciates using INT_MAX in field_order_cnt_list[i][0]/[1] to indicate
      // not used convert to expected
      if (pPipeDesc->field_order_cnt_list[i][0] == INT_MAX) {
         pPipeDesc->top_is_reference[i] = false;
         pPipeDesc->field_order_cnt_list[i][0] = 0;   // DXVA Spec says this has to be zero if unused
      }

      if (pPipeDesc->field_order_cnt_list[i][1] == INT_MAX) {
         pPipeDesc->bottom_is_reference[i] = false;
         pPipeDesc->field_order_cnt_list[i][1] = 0;   // DXVA Spec says this has to be zero if unused
      }

      // If both top and bottom reference flags are false, this is an invalid entry
      bool validEntry = (pPipeDesc->top_is_reference[i] || pPipeDesc->bottom_is_reference[i] || pPipeDesc->is_long_term[i]);
      if (!validEntry) {
         // From DXVA spec:
         // Entries that will not be used for decoding the current picture, or any subsequent pictures, are indicated by
         // setting bPicEntry to 0xFF. If bPicEntry is not 0xFF, the entry may be used as a reference surface for
         // decoding the current picture or a subsequent picture (in decoding order).
         dxvaStructure.RefFrameList[i].bPicEntry = DXVA_H264_INVALID_PICTURE_ENTRY_VALUE;
         dxvaStructure.FieldOrderCntList[i][0] = 0;
         dxvaStructure.FieldOrderCntList[i][1] = 0;
         dxvaStructure.FrameNumList[i] = 0;
      } else {
         frameUsesAnyRefPicture = true;
         // From DXVA spec:
         // For each entry whose value is not 0xFF, the value of AssociatedFlag is interpreted as follows:
         // 0 - Not a long-term reference frame.
         // 1 - Long-term reference frame. The uncompressed frame buffer contains a reference frame or one or more
         // reference fields marked as "used for long-term reference." If field_pic_flag is 1, the current uncompressed
         // frame surface may appear in the list for the purpose of decoding the second field of a complementary
         // reference field pair.
         dxvaStructure.RefFrameList[i].AssociatedFlag = pPipeDesc->is_long_term[i] ? 1u : 0u;

         // dxvaStructure.RefFrameList[i].Index7Bits is handled by d3d12_video_decoder_refresh_dpb_active_references_h264

         // uint16_t FrameNumList[16];
         // 	 FrameNumList
         // For each entry in RefFrameList, the corresponding entry in FrameNumList
         // contains the value of FrameNum or LongTermFrameIdx, depending on the value of
         // AssociatedFlag in the RefFrameList entry. (FrameNum is assigned to short-term
         // reference pictures, and LongTermFrameIdx is assigned to long-term reference
         // pictures.)
         // If an element in the list of frames is not relevent (for example, if the corresponding
         // entry in RefFrameList is empty or is marked as "not used for reference"), the value
         // of the FrameNumList entry shall be 0. Accelerators can rely on this constraint being
         // fulfilled.
         dxvaStructure.FrameNumList[i] = pPipeDesc->frame_num_list[i];

         // int32_t    FieldOrderCntList[16][2];
         // Contains the picture order counts for the reference frames listed in RefFrameList.
         // For each entry i in the RefFrameList array, FieldOrderCntList[i][0] contains the
         // value of TopFieldOrderCnt for entry i, and FieldOrderCntList[i][1] contains the
         // value of BottomFieldOrderCnt for entry i.
         //
         // If an element of the list is not relevent (for example, if the corresponding entry in
         // RefFrameList is empty or is marked as "not used for reference"), the value of
         // TopFieldOrderCnt or BottomFieldOrderCnt in FieldOrderCntList shall be 0.
         // Accelerators can rely on this constraint being fulfilled.

         dxvaStructure.FieldOrderCntList[i][0] = pPipeDesc->field_order_cnt_list[i][0];
         dxvaStructure.FieldOrderCntList[i][1] = pPipeDesc->field_order_cnt_list[i][1];

         // From DXVA spec
         // UsedForReferenceFlags
         // Contains two 1-bit flags for each entry in RefFrameList. For the ith entry in RefFrameList, the two flags
         // are accessed as follows:  Flag1i = (UsedForReferenceFlags >> (2 * i)) & 1  Flag2i = (UsedForReferenceFlags
         // >> (2 * i + 1)) & 1 If Flag1i is 1, the top field of frame number i is marked as "used for reference," as
         // defined by the H.264/AVC specification. If Flag2i is 1, the bottom field of frame number i is marked as
         // "used for reference." (Otherwise, if either flag is 0, that field is not marked as "used for reference.") If
         // an element in the list of frames is not relevent (for example, if the corresponding entry in RefFrameList is
         // empty), the value of both flags for that entry shall be 0. Accelerators may rely on this constraint being
         // fulfilled.

         if (pPipeDesc->top_is_reference[i] || pPipeDesc->is_long_term[i]) {
            dxvaStructure.UsedForReferenceFlags |= (1 << (2 * i));
         }

         if (pPipeDesc->bottom_is_reference[i] || pPipeDesc->is_long_term[i]) {
            dxvaStructure.UsedForReferenceFlags |= (1 << (2 * i + 1));
         }
      }
   }

   // frame type (I, P, B, etc) is not included in pipeDesc data, let's try to derive it
   // from the reference list...if frame doesn't use any references, it should be an I frame.
   dxvaStructure.IntraPicFlag = !frameUsesAnyRefPicture;

   // uint8_t  pic_order_present_flag; /* Renamed to bottom_field_pic_order_in_frame_present_flag in newer standard
   // versions. */
   dxvaStructure.pic_order_present_flag = pPipeDesc->pps->bottom_field_pic_order_in_frame_present_flag;

   // Software decoders should be implemented, as soon as feasible, to set the value of
   // Reserved16Bits to 3. The value 0 was previously assigned for uses prior to July 20,
   // 2007. The value 1 was previously assigned for uses prior to October 12, 2007. The
   // value 2 was previously assigned for uses prior to January 15, 2009. Software
   // decoders shall not set Reserved16Bits to any value other than those listed here.
   // Note Software decoders that set Reserved16Bits to 3 should ensure that any aspects of software decoder operation
   // that were previously not in conformance with this version of the specification have been corrected in the current
   // implementation. One particular aspect of conformance that should be checked is the ordering of quantization
   // scaling list data, as specified in section 5.2. In addition, the ReservedIntraBit flag in the macroblock control
   // buffer must use the semantics described in section 7.2 (this flag was previously reserved). The semantics of
   // Index7Bits and RefPicList have also been clarified in updates to this specification.
   dxvaStructure.Reserved16Bits = 3;

   // DXVA spec: Arbitrary number set by the host decoder to use as a tag in the status report
   // feedback data. The value should not equal 0, and should be different in each call to
   // Execute. For more information, see section 12.0, Status Report Data Structure.
   dxvaStructure.StatusReportFeedbackNumber = frameNum;
   assert(dxvaStructure.StatusReportFeedbackNumber > 0);

   // from DXVA spec
   // ContinuationFlag
   // If this flag is 1, the remainder of this structure is present in the buffer and contains valid values. If this
   // flag is 0, the structure might be truncated at this point in the buffer, or the remaining fields may be set to 0
   // and shall be ignored by the accelerator. The remaining members of this structure are needed only for off-host
   // bitstream parsing. If the host decoder parses the bitstream, the decoder can truncate the picture parameters data
   // structure buffer after the ContinuationFlag or set the remaining members to zero. uint8_t  ContinuationFlag;
   dxvaStructure.ContinuationFlag =
      1;   // DXVA destination struct does contain members from the slice section of pipeDesc...

   return dxvaStructure;
}

void
d3d12_video_decoder_dxva_qmatrix_from_pipe_picparams_h264(pipe_h264_picture_desc *pPipeDesc,
                                                          DXVA_Qmatrix_H264 &outMatrixBuffer)
{
   // Please note here that the matrices coming from the gallium VA frontend are copied from VAIQMatrixBufferH264
   // which are specified in VAAPI as being in raster scan order (different than zigzag needed by DXVA)
   // also please note that VAIQMatrixBufferH264.ScalingList8x8 is copied into the first two rows of
   // pipe_h264_pps.ScalingList8x8 leaving the upper 4 rows of  pipe_h264_pps.ScalingList8x8[6][64] unmodified
   // Finally, please note that other gallium frontends might decide to copy the scaling lists in other order
   // and this section might have to be extended to add support for them.

   // In DXVA each scaling list is ordered in zig-zag scan order, convert them from raster scan order.
   unsigned i, j;
   for (i = 0; i < 6; i++) {
      for (j = 0; j < 16; j++) {
         outMatrixBuffer.bScalingLists4x4[i][j] = pPipeDesc->pps->ScalingList4x4[i][vl_zscan_normal_16[j]];
      }
   }
   for (i = 0; i < 64; i++) {
      outMatrixBuffer.bScalingLists8x8[0][i] = pPipeDesc->pps->ScalingList8x8[0][vl_zscan_normal[i]];
      outMatrixBuffer.bScalingLists8x8[1][i] = pPipeDesc->pps->ScalingList8x8[1][vl_zscan_normal[i]];
   }
}
