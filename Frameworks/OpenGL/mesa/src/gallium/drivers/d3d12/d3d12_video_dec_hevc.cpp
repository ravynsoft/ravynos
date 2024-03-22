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
#include "d3d12_video_dec_hevc.h"
#include "d3d12_resource.h"
#include "d3d12_video_buffer.h"
#include <cmath>

void
d3d12_video_decoder_refresh_dpb_active_references_hevc(struct d3d12_video_decoder *pD3D12Dec)
{
   // Method overview
   // 1. Codec specific strategy in switch statement regarding reference frames eviction policy. Should only mark active
   // DPB references, leaving evicted ones as unused
   // 2. Call release_unused_references_texture_memory(); at the end of this method. Any references (and texture
   // allocations associated)
   //    that were left not marked as used in m_spDPBManager by step (2) are lost.

   // Assign DXVA original Index7Bits indices to current frame and references
   DXVA_PicParams_HEVC *pCurrPicParams = d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_HEVC>(pD3D12Dec);
   for (uint8_t i = 0; i < _countof(pCurrPicParams->RefPicList); i++) {
      // From HEVC DXVA spec:
      // Index7Bits
      //     An index that identifies an uncompressed surface for the CurrPic or RefPicList member of the picture parameters structure (section 4.0).
      //     When Index7Bits is used in the CurrPic and RefPicList members of the picture parameters structure, the value directly specifies the DXVA index of an uncompressed surface.
      //     When Index7Bits is 127 (0x7F), this indicates that it does not contain a valid index.

      //     AssociatedFlag
      //     Optional 1-bit flag associated with the surface. It specifies whether the reference picture is a long-term reference or a short-term reference for RefPicList, and it has no meaning when used for CurrPic.
      //     bPicEntry
      //     Accesses the entire 8 bits of the union.

      if (pCurrPicParams->RefPicList[i].bPicEntry != DXVA_HEVC_INVALID_PICTURE_ENTRY_VALUE) {
         pCurrPicParams->RefPicList[i].Index7Bits =
            pD3D12Dec->m_spDPBManager->get_index7bits(pD3D12Dec->m_pCurrentReferenceTargets[i]);
      }
   }

   pD3D12Dec->m_spDPBManager->mark_all_references_as_unused();
   pD3D12Dec->m_spDPBManager->mark_references_in_use(pCurrPicParams->RefPicList);

   // Releases the underlying reference picture texture objects of all references that were not marked as used in this
   // method.
   pD3D12Dec->m_spDPBManager->release_unused_references_texture_memory();

   pCurrPicParams->CurrPic.Index7Bits = pD3D12Dec->m_spDPBManager->get_index7bits(pD3D12Dec->m_pCurrentDecodeTarget);
}

inline int
LengthFromMinCb(int length, int cbsize)
{
   return length * (1 << cbsize);
}

void
d3d12_video_decoder_get_frame_info_hevc(
   struct d3d12_video_decoder *pD3D12Dec, uint32_t *pWidth, uint32_t *pHeight, uint16_t *pMaxDPB, bool &isInterlaced)
{
   auto pPicParams = d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_HEVC>(pD3D12Dec);
   UINT log2_min_luma_coding_block_size = pPicParams->log2_min_luma_coding_block_size_minus3 + 3;
   *pWidth = LengthFromMinCb(pPicParams->PicWidthInMinCbsY, log2_min_luma_coding_block_size);
   *pHeight = LengthFromMinCb(pPicParams->PicHeightInMinCbsY, log2_min_luma_coding_block_size);
   *pMaxDPB = pPicParams->sps_max_dec_pic_buffering_minus1 + 1;
   isInterlaced = false;
}

///
/// Pushes the current frame as next reference, updates the DXVA HEVC structure with the indices of the DPB and
/// transitions the references
///
void
d3d12_video_decoder_prepare_current_frame_references_hevc(struct d3d12_video_decoder *pD3D12Dec,
                                                          ID3D12Resource *pTexture2D,
                                                          uint32_t subresourceIndex)
{
   DXVA_PicParams_HEVC *pPicParams = d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_HEVC>(pD3D12Dec);
   pPicParams->CurrPic.Index7Bits = pD3D12Dec->m_spDPBManager->store_future_reference(pPicParams->CurrPic.Index7Bits,
                                                                                      pD3D12Dec->m_spVideoDecoderHeap,
                                                                                      pTexture2D,
                                                                                      subresourceIndex);
   // From HEVC DXVA spec:
   // Index7Bits
   //     An index that identifies an uncompressed surface for the CurrPic or RefPicList member of the picture parameters structure (section 4.0).
   //     When Index7Bits is used in the CurrPic and RefPicList members of the picture parameters structure, the value directly specifies the DXVA index of an uncompressed surface.
   //     When Index7Bits is 127 (0x7F), this indicates that it does not contain a valid index.

   pD3D12Dec->m_spDPBManager->update_entries(
      d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_HEVC>(pD3D12Dec)->RefPicList,
      pD3D12Dec->m_transitionsStorage);

   pD3D12Dec->m_spDecodeCommandList->ResourceBarrier(pD3D12Dec->m_transitionsStorage.size(), pD3D12Dec->m_transitionsStorage.data());

   // Schedule reverse (back to common) transitions before command list closes for current frame
   for (auto BarrierDesc : pD3D12Dec->m_transitionsStorage) {
      std::swap(BarrierDesc.Transition.StateBefore, BarrierDesc.Transition.StateAfter);
      pD3D12Dec->m_transitionsBeforeCloseCmdList.push_back(BarrierDesc);
   }

   debug_printf(
      "[d3d12_video_decoder_prepare_current_frame_references_hevc] DXVA_PicParams_HEVC after index remapping)\n");
   d3d12_video_decoder_log_pic_params_hevc(
      d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_HEVC>(pD3D12Dec));
}

void
d3d12_video_decoder_prepare_dxva_slices_control_hevc(struct d3d12_video_decoder *pD3D12Dec,
                                                     std::vector<uint8_t> &vecOutSliceControlBuffers,
                                                     struct pipe_h265_picture_desc *picture_hevc)
{
   
   if(!picture_hevc->slice_parameter.slice_info_present)
   {
      unreachable("Unsupported - need pipe_h265_picture_desc.slice_parameter.slice_info_present");
   }

   debug_printf("[d3d12_video_decoder_hevc] Upper layer reported %d slices for this frame, parsing them below...\n",
                  picture_hevc->slice_parameter.slice_count);

   uint64_t TotalSlicesDXVAArrayByteSize = picture_hevc->slice_parameter.slice_count * sizeof(DXVA_Slice_HEVC_Short);
   vecOutSliceControlBuffers.resize(TotalSlicesDXVAArrayByteSize);

   uint8_t* pData = vecOutSliceControlBuffers.data();
   static const uint32_t start_code_size = 3;
   uint32_t acum_slice_offset = (picture_hevc->slice_parameter.slice_count > 0) ? picture_hevc->slice_parameter.slice_data_offset[0] : 0;
   for (uint32_t sliceIdx = 0; sliceIdx < picture_hevc->slice_parameter.slice_count; sliceIdx++)
   {
      DXVA_Slice_HEVC_Short currentSliceEntry = {};
      // From HEVC DXVA Spec
      // wBadSliceChopping
      // 0	All bits for the slice are located within the corresponding bitstream data buffer. 
      // 1	The bitstream data buffer contains the start of the slice, but not the entire slice, because the buffer is full. 
      // 2	The bitstream data buffer contains the end of the slice. It does not contain the start of the slice, because the start of the slice was located in the previous bitstream data buffer. 
      // 3	The bitstream data buffer does not contain the start of the slice (because the start of the slice was located in the previous bitstream data buffer),
      //     and it does not contain the end of the slice (because the current bitstream data buffer is also full). 

      switch (picture_hevc->slice_parameter.slice_data_flag[sliceIdx]) {
         /* whole slice is in the buffer */
         case PIPE_SLICE_BUFFER_PLACEMENT_TYPE_WHOLE:
            currentSliceEntry.wBadSliceChopping = 0u;
            break;
         /* The beginning of the slice is in the buffer but the end is not */
         case PIPE_SLICE_BUFFER_PLACEMENT_TYPE_BEGIN:
            currentSliceEntry.wBadSliceChopping = 1u;
            break;
         /* Neither beginning nor end of the slice is in the buffer */
         case PIPE_SLICE_BUFFER_PLACEMENT_TYPE_MIDDLE:
            currentSliceEntry.wBadSliceChopping = 3u;
            break;
         /* end of the slice is in the buffer */
         case PIPE_SLICE_BUFFER_PLACEMENT_TYPE_END:
            currentSliceEntry.wBadSliceChopping = 2u;
            break;
         default:
         {
            unreachable("Unsupported pipe_slice_buffer_placement_type");
         } break;
      }

      /* slice_data_size from pipe/va does not include the NAL unit size, DXVA requires it */
      currentSliceEntry.SliceBytesInBuffer = picture_hevc->slice_parameter.slice_data_size[sliceIdx] + start_code_size;

      /* slice_data_offset from pipe/va are relative to the current slice, and in DXVA they are absolute within the frame source buffer */
      currentSliceEntry.BSNALunitDataLocation = acum_slice_offset;
      acum_slice_offset += (currentSliceEntry.SliceBytesInBuffer + picture_hevc->slice_parameter.slice_data_offset[sliceIdx]);

      debug_printf("[d3d12_video_decoder_hevc] Detected slice index %" PRIu32 " with SliceBytesInBuffer %d - BSNALunitDataLocation %d - wBadSliceChopping: %" PRIu16
                  " for frame with "
                  "fenceValue: %d\n",
                  sliceIdx,
                  currentSliceEntry.SliceBytesInBuffer,
                  currentSliceEntry.BSNALunitDataLocation,
                  currentSliceEntry.wBadSliceChopping,
                  pD3D12Dec->m_fenceValue);

      memcpy(pData, &currentSliceEntry, sizeof(DXVA_Slice_HEVC_Short));
      pData += sizeof(DXVA_Slice_HEVC_Short);
   }
   assert(vecOutSliceControlBuffers.size() == TotalSlicesDXVAArrayByteSize);
}

static void
d3d12_video_decoder_log_pic_entry_hevc(DXVA_PicEntry_HEVC &picEntry)
{
   debug_printf("\t\tIndex7Bits: %d\n"
                 "\t\tAssociatedFlag: %d\n"
                 "\t\tbPicEntry: %d\n",
                 picEntry.Index7Bits,
                 picEntry.AssociatedFlag,
                 picEntry.bPicEntry);
}

void
d3d12_video_decoder_log_pic_params_hevc(DXVA_PicParams_HEVC *pPicParams)
{
   debug_printf("\n=============================================\n");
   debug_printf("PicWidthInMinCbsY = %d\n", pPicParams->PicWidthInMinCbsY);
   debug_printf("PicHeightInMinCbsY = %d\n", pPicParams->PicHeightInMinCbsY);
   debug_printf("chroma_format_idc = %d\n", pPicParams->chroma_format_idc);
   debug_printf("separate_colour_plane_flag = %d\n", pPicParams->separate_colour_plane_flag);
   debug_printf("bit_depth_luma_minus8 = %d\n", pPicParams->bit_depth_luma_minus8);
   debug_printf("bit_depth_chroma_minus8 = %d\n", pPicParams->bit_depth_chroma_minus8);
   debug_printf("log2_max_pic_order_cnt_lsb_minus4 = %d\n", pPicParams->log2_max_pic_order_cnt_lsb_minus4);
   debug_printf("NoPicReorderingFlag = %d\n", pPicParams->NoPicReorderingFlag);
   debug_printf("NoBiPredFlag = %d\n", pPicParams->NoBiPredFlag);
   debug_printf("ReservedBits1 = %d\n", pPicParams->ReservedBits1);
   debug_printf("wFormatAndSequenceInfoFlags = %d\n", pPicParams->wFormatAndSequenceInfoFlags);
   debug_printf("CurrPic.Index7Bits = %d\n", pPicParams->CurrPic.Index7Bits);
   debug_printf("CurrPic.AssociatedFlag = %d\n", pPicParams->CurrPic.AssociatedFlag);
   debug_printf("sps_max_dec_pic_buffering_minus1 = %d\n", pPicParams->sps_max_dec_pic_buffering_minus1);
   debug_printf("log2_min_luma_coding_block_size_minus3 = %d\n", pPicParams->log2_min_luma_coding_block_size_minus3);
   debug_printf("log2_diff_max_min_luma_coding_block_size = %d\n", pPicParams->log2_diff_max_min_luma_coding_block_size);
   debug_printf("log2_min_transform_block_size_minus2 = %d\n", pPicParams->log2_min_transform_block_size_minus2);
   debug_printf("log2_diff_max_min_transform_block_size = %d\n", pPicParams->log2_diff_max_min_transform_block_size);
   debug_printf("max_transform_hierarchy_depth_inter = %d\n", pPicParams->max_transform_hierarchy_depth_inter);
   debug_printf("max_transform_hierarchy_depth_intra = %d\n", pPicParams->max_transform_hierarchy_depth_intra);
   debug_printf("num_short_term_ref_pic_sets = %d\n", pPicParams->num_short_term_ref_pic_sets);
   debug_printf("num_long_term_ref_pics_sps = %d\n", pPicParams->num_long_term_ref_pics_sps);
   debug_printf("num_ref_idx_l0_default_active_minus1 = %d\n", pPicParams->num_ref_idx_l0_default_active_minus1);
   debug_printf("num_ref_idx_l1_default_active_minus1 = %d\n", pPicParams->num_ref_idx_l1_default_active_minus1);
   debug_printf("init_qp_minus26 = %d\n", pPicParams->init_qp_minus26);
   debug_printf("ucNumDeltaPocsOfRefRpsIdx = %d\n", pPicParams->ucNumDeltaPocsOfRefRpsIdx);
   debug_printf("wNumBitsForShortTermRPSInSlice = %d\n", pPicParams->wNumBitsForShortTermRPSInSlice);
   debug_printf("ReservedBits2 = %d\n", pPicParams->ReservedBits2);
   debug_printf("scaling_list_enabled_flag = %d\n", pPicParams->scaling_list_enabled_flag);
   debug_printf("amp_enabled_flag = %d\n", pPicParams->amp_enabled_flag);
   debug_printf("sample_adaptive_offset_enabled_flag = %d\n", pPicParams->sample_adaptive_offset_enabled_flag);
   debug_printf("pcm_enabled_flag = %d\n", pPicParams->pcm_enabled_flag);
   debug_printf("pcm_sample_bit_depth_luma_minus1 = %d\n", pPicParams->pcm_sample_bit_depth_luma_minus1);
   debug_printf("pcm_sample_bit_depth_chroma_minus1 = %d\n", pPicParams->pcm_sample_bit_depth_chroma_minus1);
   debug_printf("log2_min_pcm_luma_coding_block_size_minus3 = %d\n", pPicParams->log2_min_pcm_luma_coding_block_size_minus3);
   debug_printf("log2_diff_max_min_pcm_luma_coding_block_size = %d\n", pPicParams->log2_diff_max_min_pcm_luma_coding_block_size);
   debug_printf("pcm_loop_filter_disabled_flag = %d\n", pPicParams->pcm_loop_filter_disabled_flag);
   debug_printf("long_term_ref_pics_present_flag = %d\n", pPicParams->long_term_ref_pics_present_flag);
   debug_printf("sps_temporal_mvp_enabled_flag = %d\n", pPicParams->sps_temporal_mvp_enabled_flag);
   debug_printf("strong_intra_smoothing_enabled_flag = %d\n", pPicParams->strong_intra_smoothing_enabled_flag);
   debug_printf("dependent_slice_segments_enabled_flag = %d\n", pPicParams->dependent_slice_segments_enabled_flag);
   debug_printf("output_flag_present_flag = %d\n", pPicParams->output_flag_present_flag);
   debug_printf("num_extra_slice_header_bits = %d\n", pPicParams->num_extra_slice_header_bits);
   debug_printf("sign_data_hiding_enabled_flag = %d\n", pPicParams->sign_data_hiding_enabled_flag);
   debug_printf("cabac_init_present_flag = %d\n", pPicParams->cabac_init_present_flag);
   debug_printf("ReservedBits3 = %d\n", pPicParams->ReservedBits3);
   debug_printf("dwCodingParamToolFlags = %d\n", pPicParams->dwCodingParamToolFlags);
   debug_printf("constrained_intra_pred_flag = %d\n", pPicParams->constrained_intra_pred_flag);
   debug_printf("transform_skip_enabled_flag = %d\n", pPicParams->transform_skip_enabled_flag);
   debug_printf("cu_qp_delta_enabled_flag = %d\n", pPicParams->cu_qp_delta_enabled_flag);
   debug_printf("pps_slice_chroma_qp_offsets_present_flag = %d\n", pPicParams->pps_slice_chroma_qp_offsets_present_flag);
   debug_printf("weighted_pred_flag = %d\n", pPicParams->weighted_pred_flag);
   debug_printf("weighted_bipred_flag = %d\n", pPicParams->weighted_bipred_flag);
   debug_printf("transquant_bypass_enabled_flag = %d\n", pPicParams->transquant_bypass_enabled_flag);
   debug_printf("tiles_enabled_flag = %d\n", pPicParams->tiles_enabled_flag);
   debug_printf("entropy_coding_sync_enabled_flag = %d\n", pPicParams->entropy_coding_sync_enabled_flag);
   debug_printf("uniform_spacing_flag = %d\n", pPicParams->uniform_spacing_flag);
   debug_printf("loop_filter_across_tiles_enabled_flag = %d\n", pPicParams->loop_filter_across_tiles_enabled_flag);
   debug_printf("pps_loop_filter_across_slices_enabled_flag = %d\n", pPicParams->pps_loop_filter_across_slices_enabled_flag);
   debug_printf("deblocking_filter_override_enabled_flag = %d\n", pPicParams->deblocking_filter_override_enabled_flag);
   debug_printf("pps_deblocking_filter_disabled_flag = %d\n", pPicParams->pps_deblocking_filter_disabled_flag);
   debug_printf("lists_modification_present_flag = %d\n", pPicParams->lists_modification_present_flag);
   debug_printf("slice_segment_header_extension_present_flag = %d\n", pPicParams->slice_segment_header_extension_present_flag);
   debug_printf("IrapPicFlag = %d\n", pPicParams->IrapPicFlag);
   debug_printf("IdrPicFlag = %d\n", pPicParams->IdrPicFlag);
   debug_printf("IntraPicFlag = %d\n", pPicParams->IntraPicFlag);
   debug_printf("ReservedBits4 = %d\n", pPicParams->ReservedBits4);
   debug_printf("dwCodingSettingPicturePropertyFlags = %d\n", pPicParams->dwCodingSettingPicturePropertyFlags);
   debug_printf("pps_cb_qp_offset = %d\n", pPicParams->pps_cb_qp_offset);
   debug_printf("pps_cr_qp_offset = %d\n", pPicParams->pps_cr_qp_offset);
   debug_printf("num_tile_columns_minus1 = %d\n", pPicParams->num_tile_columns_minus1);
   debug_printf("num_tile_rows_minus1 = %d\n", pPicParams->num_tile_rows_minus1);
   for (uint32_t i = 0; i < std::min((unsigned) pPicParams->num_tile_columns_minus1 + 1u, (unsigned) _countof(DXVA_PicParams_HEVC::column_width_minus1)); i++) {
      debug_printf("column_width_minus1[%d]; = %d\n", i, pPicParams->column_width_minus1[i]);
   }
   for (uint32_t i = 0; i < std::min((unsigned) pPicParams->num_tile_rows_minus1 + 1u, (unsigned) _countof(DXVA_PicParams_HEVC::row_height_minus1)); i++) {
      debug_printf("row_height_minus1[%d]; = %d\n", i, pPicParams->row_height_minus1[i]);
   }
   debug_printf("diff_cu_qp_delta_depth = %d\n", pPicParams->diff_cu_qp_delta_depth);
   debug_printf("pps_beta_offset_div2 = %d\n", pPicParams->pps_beta_offset_div2);
   debug_printf("pps_tc_offset_div2 = %d\n", pPicParams->pps_tc_offset_div2);
   debug_printf("log2_parallel_merge_level_minus2 = %d\n", pPicParams->log2_parallel_merge_level_minus2);
   debug_printf("CurrPicOrderCntVal = %d\n", pPicParams->CurrPicOrderCntVal);
   debug_printf("ReservedBits5 = %d\n", pPicParams->ReservedBits5);
   debug_printf("ReservedBits6 = %d\n", pPicParams->ReservedBits6);
   debug_printf("ReservedBits7 = %d\n", pPicParams->ReservedBits7);
   debug_printf("StatusReportFeedbackNumber = %d\n", pPicParams->StatusReportFeedbackNumber);

   debug_printf("[D3D12 Video Decoder HEVC DXVA PicParams info]\n"
                 "\t[Current Picture Entry]\n");
   d3d12_video_decoder_log_pic_entry_hevc(pPicParams->CurrPic);

   debug_printf("[D3D12 Video Decoder HEVC DXVA PicParams info]\n"
                 "\t[Current Picture Reference sets, hiding entries with bPicEntry 0xFF]\n");

   for (uint32_t refIdx = 0; refIdx < _countof(DXVA_PicParams_HEVC::RefPicSetStCurrBefore); refIdx++) {
      if(pPicParams->RefPicSetStCurrBefore[refIdx] != DXVA_HEVC_INVALID_PICTURE_ENTRY_VALUE) {
         debug_printf("\tRefPicSetStCurrBefore[%d] = %d \n PicEntry RefPicList[%d]\n", refIdx, pPicParams->RefPicSetStCurrBefore[refIdx], pPicParams->RefPicSetStCurrBefore[refIdx]);
         d3d12_video_decoder_log_pic_entry_hevc(pPicParams->RefPicList[pPicParams->RefPicSetStCurrBefore[refIdx]]);
         debug_printf("\t\tPicOrderCntValList: %d\n",
                     pPicParams->PicOrderCntValList[pPicParams->RefPicSetStCurrBefore[refIdx]]);
      }   
   }
   for (uint32_t refIdx = 0; refIdx < _countof(DXVA_PicParams_HEVC::RefPicSetStCurrAfter); refIdx++) {
      if(pPicParams->RefPicSetStCurrAfter[refIdx] != DXVA_HEVC_INVALID_PICTURE_ENTRY_VALUE) {
         debug_printf("\tRefPicSetStCurrAfter[%d] = %d \n PicEntry RefPicList[%d]\n", refIdx, pPicParams->RefPicSetStCurrAfter[refIdx], pPicParams->RefPicSetStCurrAfter[refIdx]);
         d3d12_video_decoder_log_pic_entry_hevc(pPicParams->RefPicList[pPicParams->RefPicSetStCurrAfter[refIdx]]);
         debug_printf("\t\tPicOrderCntValList: %d\n",
                     pPicParams->PicOrderCntValList[pPicParams->RefPicSetStCurrAfter[refIdx]]);
      }   
   }
   for (uint32_t refIdx = 0; refIdx < _countof(DXVA_PicParams_HEVC::RefPicSetLtCurr); refIdx++) {
      if(pPicParams->RefPicSetLtCurr[refIdx] != DXVA_HEVC_INVALID_PICTURE_ENTRY_VALUE) {
         debug_printf("\tRefPicSetLtCurr[%d] = %d \n PicEntry RefPicList[%d]\n", refIdx, pPicParams->RefPicSetLtCurr[refIdx], pPicParams->RefPicSetLtCurr[refIdx]);
         d3d12_video_decoder_log_pic_entry_hevc(pPicParams->RefPicList[pPicParams->RefPicSetLtCurr[refIdx]]);
         debug_printf("\t\tPicOrderCntValList: %d\n",
                     pPicParams->PicOrderCntValList[pPicParams->RefPicSetLtCurr[refIdx]]);
      }   
   }
}

void
d3d12_video_decoder_sort_rps_lists_by_refpoc(struct d3d12_video_decoder *pD3D12Dec, DXVA_PicParams_HEVC* pDXVAStruct, pipe_h265_picture_desc *pPipeDesc)
{
   // Sort the RPS lists in pDXVAStruct in order by pPipeDesc->PicOrderCntVal for DXVA expectations.
   // Both arrays have parallel indices

   pD3D12Dec->m_ReferencesConversionStorage.clear();
   for (uint8_t i = 0; i < pPipeDesc->NumPocStCurrBefore; i++)
      pD3D12Dec->m_ReferencesConversionStorage.push_back({ pDXVAStruct->RefPicSetStCurrBefore[i], pPipeDesc->PicOrderCntVal[pDXVAStruct->RefPicSetStCurrBefore[i]] });

   std::sort(std::begin(pD3D12Dec->m_ReferencesConversionStorage), std::end(pD3D12Dec->m_ReferencesConversionStorage),
      [](d3d12_video_decoder_reference_poc_entry entryI, d3d12_video_decoder_reference_poc_entry entryJ)
                                                    { return entryI.poc_value /*desc order*/ > entryJ.poc_value; });
   for (uint8_t i = 0; i < pPipeDesc->NumPocStCurrBefore; i++)
      pDXVAStruct->RefPicSetStCurrBefore[i] = pD3D12Dec->m_ReferencesConversionStorage[i].refpicset_index;

   pD3D12Dec->m_ReferencesConversionStorage.clear();
   for (uint8_t i = 0; i < pPipeDesc->NumPocStCurrAfter; i++)
      pD3D12Dec->m_ReferencesConversionStorage.push_back({ pDXVAStruct->RefPicSetStCurrAfter[i], pPipeDesc->PicOrderCntVal[pDXVAStruct->RefPicSetStCurrAfter[i]] });

   std::sort(std::begin(pD3D12Dec->m_ReferencesConversionStorage), std::end(pD3D12Dec->m_ReferencesConversionStorage), 
      [](d3d12_video_decoder_reference_poc_entry entryI, d3d12_video_decoder_reference_poc_entry entryJ)
                                                    { return entryI.poc_value /*ascending order*/ < entryJ.poc_value; });
   for (uint8_t i = 0; i < pPipeDesc->NumPocStCurrAfter; i++)
      pDXVAStruct->RefPicSetStCurrAfter[i] = pD3D12Dec->m_ReferencesConversionStorage[i].refpicset_index;

   pD3D12Dec->m_ReferencesConversionStorage.clear();
   for (uint8_t i = 0; i < pPipeDesc->NumPocLtCurr; i++)
      pD3D12Dec->m_ReferencesConversionStorage.push_back({ pDXVAStruct->RefPicSetLtCurr[i], pPipeDesc->PicOrderCntVal[pDXVAStruct->RefPicSetLtCurr[i]] });

   // The ordering of RefPicSetLtCurr is unclear from the DXVA spec, might need to be changed
   std::sort(std::begin(pD3D12Dec->m_ReferencesConversionStorage), std::end(pD3D12Dec->m_ReferencesConversionStorage), 
      [](d3d12_video_decoder_reference_poc_entry entryI, d3d12_video_decoder_reference_poc_entry entryJ)
                                                    { return entryI.poc_value /*ascending order*/ < entryJ.poc_value; });
   for (uint8_t i = 0; i < pPipeDesc->NumPocLtCurr; i++)
      pDXVAStruct->RefPicSetLtCurr[i] = pD3D12Dec->m_ReferencesConversionStorage[i].refpicset_index;
}

DXVA_PicParams_HEVC
d3d12_video_decoder_dxva_picparams_from_pipe_picparams_hevc(
   struct d3d12_video_decoder *pD3D12Dec,
   pipe_video_profile profile,
   pipe_h265_picture_desc *pPipeDesc)
{
   uint32_t frameNum = pD3D12Dec->m_fenceValue;
   pipe_h265_pps *pps = pPipeDesc->pps;
   pipe_h265_sps *sps = pPipeDesc->pps->sps;

   DXVA_PicParams_HEVC dxvaStructure;
   memset(&dxvaStructure, 0, sizeof(dxvaStructure));
   
   uint8_t log2_min_cb_size = sps->log2_min_luma_coding_block_size_minus3 + 3;
   dxvaStructure.PicWidthInMinCbsY = sps->pic_width_in_luma_samples  >> log2_min_cb_size;
   dxvaStructure.PicHeightInMinCbsY = sps->pic_height_in_luma_samples >> log2_min_cb_size;
   dxvaStructure.chroma_format_idc = sps->chroma_format_idc;
   dxvaStructure.separate_colour_plane_flag = sps->separate_colour_plane_flag;
   dxvaStructure.bit_depth_luma_minus8 = sps->bit_depth_luma_minus8;
   dxvaStructure.bit_depth_chroma_minus8 = sps->bit_depth_chroma_minus8;
   dxvaStructure.log2_max_pic_order_cnt_lsb_minus4 = sps->log2_max_pic_order_cnt_lsb_minus4;
   dxvaStructure.NoPicReorderingFlag = sps->no_pic_reordering_flag;
   dxvaStructure.NoBiPredFlag = sps->no_bi_pred_flag;

   dxvaStructure.CurrPic.bPicEntry = 0; // No semantic for this flag in HEVC DXVA spec
   // CurrPic.Index7Bits is handled by d3d12_video_decoder_refresh_dpb_active_references_hevc

   dxvaStructure.sps_max_dec_pic_buffering_minus1         = sps->sps_max_dec_pic_buffering_minus1;
   dxvaStructure.log2_min_luma_coding_block_size_minus3   = sps->log2_min_luma_coding_block_size_minus3;
   dxvaStructure.log2_diff_max_min_luma_coding_block_size = sps->log2_diff_max_min_luma_coding_block_size;
   dxvaStructure.log2_min_transform_block_size_minus2     = sps->log2_min_transform_block_size_minus2;
   dxvaStructure.log2_diff_max_min_transform_block_size   = sps->log2_diff_max_min_transform_block_size;
   dxvaStructure.max_transform_hierarchy_depth_inter      = sps->max_transform_hierarchy_depth_inter;
   dxvaStructure.max_transform_hierarchy_depth_intra      = sps->max_transform_hierarchy_depth_intra;
   dxvaStructure.num_short_term_ref_pic_sets              = sps->num_short_term_ref_pic_sets;
   dxvaStructure.num_long_term_ref_pics_sps               = sps->num_long_term_ref_pics_sps;

   dxvaStructure.num_ref_idx_l0_default_active_minus1     = pps->num_ref_idx_l0_default_active_minus1;
   dxvaStructure.num_ref_idx_l1_default_active_minus1     = pps->num_ref_idx_l1_default_active_minus1;
   dxvaStructure.init_qp_minus26                          = pps->init_qp_minus26;

   // NumDeltaPocsOfRefRpsIdx is not passed from VA to pipe, and VA doesn't have it defined in their va_dec_hevc header.
   // DXVA drivers should use wNumBitsForShortTermRPSInSlice (st_rps_bits in VA) to derive the slice header info instead
   dxvaStructure.ucNumDeltaPocsOfRefRpsIdx            = pPipeDesc->NumDeltaPocsOfRefRpsIdx;
   dxvaStructure.wNumBitsForShortTermRPSInSlice = pps->st_rps_bits;

   dxvaStructure.scaling_list_enabled_flag = sps->scaling_list_enabled_flag;
   dxvaStructure.amp_enabled_flag = sps->amp_enabled_flag;
   dxvaStructure.sample_adaptive_offset_enabled_flag = sps->sample_adaptive_offset_enabled_flag;
   dxvaStructure.pcm_enabled_flag = sps->pcm_enabled_flag;
   dxvaStructure.pcm_sample_bit_depth_luma_minus1 = sps->pcm_sample_bit_depth_luma_minus1;
   dxvaStructure.pcm_sample_bit_depth_chroma_minus1 = sps->pcm_sample_bit_depth_chroma_minus1;
   dxvaStructure.log2_min_pcm_luma_coding_block_size_minus3 = sps->log2_min_pcm_luma_coding_block_size_minus3;
   dxvaStructure.log2_diff_max_min_pcm_luma_coding_block_size = sps->log2_diff_max_min_pcm_luma_coding_block_size;
   dxvaStructure.pcm_loop_filter_disabled_flag = sps->pcm_loop_filter_disabled_flag;
   dxvaStructure.long_term_ref_pics_present_flag = sps->long_term_ref_pics_present_flag;
   dxvaStructure.sps_temporal_mvp_enabled_flag = sps->sps_temporal_mvp_enabled_flag;
   dxvaStructure.strong_intra_smoothing_enabled_flag = sps->strong_intra_smoothing_enabled_flag;
   dxvaStructure.dependent_slice_segments_enabled_flag = pps->dependent_slice_segments_enabled_flag;
   dxvaStructure.output_flag_present_flag = pps->output_flag_present_flag;
   dxvaStructure.num_extra_slice_header_bits = pps->num_extra_slice_header_bits;
   dxvaStructure.sign_data_hiding_enabled_flag = pps->sign_data_hiding_enabled_flag;
   dxvaStructure.cabac_init_present_flag = pps->cabac_init_present_flag;
   dxvaStructure.ReservedBits3 = 0;

   dxvaStructure.constrained_intra_pred_flag = pps->constrained_intra_pred_flag;
   dxvaStructure.transform_skip_enabled_flag = pps->transform_skip_enabled_flag;
   dxvaStructure.cu_qp_delta_enabled_flag = pps->cu_qp_delta_enabled_flag;   
   dxvaStructure.pps_slice_chroma_qp_offsets_present_flag = pps->pps_slice_chroma_qp_offsets_present_flag;
   dxvaStructure.weighted_pred_flag = pps->weighted_pred_flag;         
   dxvaStructure.weighted_bipred_flag = pps->weighted_bipred_flag;       
   dxvaStructure.transquant_bypass_enabled_flag = pps->transquant_bypass_enabled_flag;                 
   dxvaStructure.tiles_enabled_flag = pps->tiles_enabled_flag;         
   dxvaStructure.entropy_coding_sync_enabled_flag = pps->entropy_coding_sync_enabled_flag;              
   dxvaStructure.uniform_spacing_flag = pps->uniform_spacing_flag;       
   dxvaStructure.loop_filter_across_tiles_enabled_flag = (pps->tiles_enabled_flag ? pps->loop_filter_across_tiles_enabled_flag : 0);
   dxvaStructure.pps_loop_filter_across_slices_enabled_flag = pps->pps_loop_filter_across_slices_enabled_flag;    
   dxvaStructure.deblocking_filter_override_enabled_flag = pps->deblocking_filter_override_enabled_flag;       
   dxvaStructure.pps_deblocking_filter_disabled_flag = pps->pps_deblocking_filter_disabled_flag;                
   dxvaStructure.lists_modification_present_flag = pps->lists_modification_present_flag;               
   dxvaStructure.slice_segment_header_extension_present_flag = pps->slice_segment_header_extension_present_flag;           
   dxvaStructure.IrapPicFlag = pPipeDesc->RAPPicFlag;
   dxvaStructure.IdrPicFlag = pPipeDesc->IDRPicFlag;
   dxvaStructure.IntraPicFlag = pPipeDesc->IntraPicFlag;
   dxvaStructure.pps_cb_qp_offset            = pps->pps_cb_qp_offset;
   dxvaStructure.pps_cr_qp_offset            = pps->pps_cr_qp_offset;
   if (pps->tiles_enabled_flag) {
      dxvaStructure.num_tile_columns_minus1 = pps->num_tile_columns_minus1;
      dxvaStructure.num_tile_rows_minus1    = pps->num_tile_rows_minus1;
      if (!pps->uniform_spacing_flag) {
         for (uint8_t i = 0; i < _countof(dxvaStructure.column_width_minus1); i++)
            dxvaStructure.column_width_minus1[i] = pps->column_width_minus1[i];

         for (uint8_t i = 0; i < _countof(dxvaStructure.row_height_minus1); i++)
            dxvaStructure.row_height_minus1[i] = pps->row_height_minus1[i];
      }
   }
   dxvaStructure.diff_cu_qp_delta_depth           = pps->diff_cu_qp_delta_depth;
   dxvaStructure.pps_beta_offset_div2             = pps->pps_beta_offset_div2;
   dxvaStructure.pps_tc_offset_div2               = pps->pps_tc_offset_div2;
   dxvaStructure.log2_parallel_merge_level_minus2 = pps->log2_parallel_merge_level_minus2;
   dxvaStructure.CurrPicOrderCntVal               = pPipeDesc->CurrPicOrderCntVal;

   // Update RefPicList with the DPB pictures to be kept alive for current or future frames
   for (uint8_t refIdx = 0; refIdx < _countof(DXVA_PicParams_HEVC::PicOrderCntValList); refIdx++)
   {
      if (pPipeDesc->ref[refIdx] != nullptr) {
         // Mark as used so d3d12_video_decoder_refresh_dpb_active_references_hevc will assign the correct Index7Bits
         dxvaStructure.RefPicList[refIdx].Index7Bits = 0;
         // Mark refpic as LTR if necessary.
         dxvaStructure.RefPicList[refIdx].AssociatedFlag = pPipeDesc->IsLongTerm[refIdx] ? 1u : 0u;
      }
      else
      {
         dxvaStructure.RefPicList[refIdx].bPicEntry = DXVA_HEVC_INVALID_PICTURE_ENTRY_VALUE;
      }
   }

   // Copy POC values for the references
   memcpy(dxvaStructure.PicOrderCntValList, pPipeDesc->PicOrderCntVal, sizeof(dxvaStructure.PicOrderCntValList));

   // Copy RPS Sets to DXVA or mark them as 0xFF if unused in current frame
   for (uint8_t i = 0; i < DXVA_RPS_COUNT; i++) {
      dxvaStructure.RefPicSetStCurrBefore[i] = (i < pPipeDesc->NumPocStCurrBefore) ? pPipeDesc->RefPicSetStCurrBefore[i] : DXVA_HEVC_INVALID_PICTURE_ENTRY_VALUE;
      dxvaStructure.RefPicSetStCurrAfter[i] = (i < pPipeDesc->NumPocStCurrAfter) ? pPipeDesc->RefPicSetStCurrAfter[i] : DXVA_HEVC_INVALID_PICTURE_ENTRY_VALUE;
      dxvaStructure.RefPicSetLtCurr[i] = (i < pPipeDesc->NumPocLtCurr) ? pPipeDesc->RefPicSetLtCurr[i] : DXVA_HEVC_INVALID_PICTURE_ENTRY_VALUE;
   }

   // DXVA drivers expect these in POC order, VA/pipe sends them out of order.
   d3d12_video_decoder_sort_rps_lists_by_refpoc(pD3D12Dec, &dxvaStructure, pPipeDesc);

   for (uint32_t refIdx = 0; refIdx < DXVA_RPS_COUNT; refIdx++) {
      if ((refIdx < pPipeDesc->NumPocStCurrBefore) && (pPipeDesc->RefPicSetStCurrBefore[refIdx] != DXVA_HEVC_INVALID_PICTURE_ENTRY_VALUE)) {
         debug_printf("pPipeDesc->RefPicSetStCurrBefore[%d]: %d (index into RefPicList) Refs[%d] pipe buffer ptr = %p - d3d12 resource %p POC: %d\n",
            refIdx, pPipeDesc->RefPicSetStCurrBefore[refIdx], pPipeDesc->RefPicSetStCurrBefore[refIdx], pPipeDesc->ref[pPipeDesc->RefPicSetStCurrBefore[refIdx]],
            d3d12_resource_resource(((struct d3d12_video_buffer *)(pPipeDesc->ref[pPipeDesc->RefPicSetStCurrBefore[refIdx]]))->texture),
            dxvaStructure.PicOrderCntValList[pPipeDesc->RefPicSetStCurrBefore[refIdx]]);
      }
      if ((refIdx < pPipeDesc->NumPocStCurrAfter) && (pPipeDesc->RefPicSetStCurrAfter[refIdx] != DXVA_HEVC_INVALID_PICTURE_ENTRY_VALUE)) {
         debug_printf("pPipeDesc->RefPicSetStCurrAfter[%d]: %d (index into RefPicList) Refs[%d] pipe buffer ptr = %p - d3d12 resource %p POC: %d\n",
            refIdx, pPipeDesc->RefPicSetStCurrAfter[refIdx], pPipeDesc->RefPicSetStCurrAfter[refIdx], pPipeDesc->ref[pPipeDesc->RefPicSetStCurrAfter[refIdx]],
            d3d12_resource_resource(((struct d3d12_video_buffer *)(pPipeDesc->ref[pPipeDesc->RefPicSetStCurrAfter[refIdx]]))->texture),
            dxvaStructure.PicOrderCntValList[pPipeDesc->RefPicSetStCurrAfter[refIdx]]);
      }
      if ((refIdx < pPipeDesc->NumPocLtCurr) && (pPipeDesc->RefPicSetLtCurr[refIdx] != DXVA_HEVC_INVALID_PICTURE_ENTRY_VALUE)) {
         debug_printf("pPipeDesc->RefPicSetLtCurr[%d]: %d (index into RefPicList) Refs[%d] pipe buffer ptr = %p - d3d12 resource %p POC: %d\n",
            refIdx, pPipeDesc->RefPicSetLtCurr[refIdx], pPipeDesc->RefPicSetLtCurr[refIdx], pPipeDesc->ref[pPipeDesc->RefPicSetLtCurr[refIdx]],
            d3d12_resource_resource(((struct d3d12_video_buffer *)(pPipeDesc->ref[pPipeDesc->RefPicSetLtCurr[refIdx]]))->texture),
            dxvaStructure.PicOrderCntValList[pPipeDesc->RefPicSetLtCurr[refIdx]]);
      }
   }

   // DXVA spec: Arbitrary number set by the host decoder to use as a tag in the status report
   // feedback data. The value should not equal 0, and should be different in each call to
   // Execute. For more information, see section 12.0, Status Report Data Structure.
   dxvaStructure.StatusReportFeedbackNumber = frameNum;
   assert(dxvaStructure.StatusReportFeedbackNumber > 0);
   return dxvaStructure;
}

void
d3d12_video_decoder_dxva_qmatrix_from_pipe_picparams_hevc(pipe_h265_picture_desc *pPipeDesc,
                                                          DXVA_Qmatrix_HEVC &outMatrixBuffer,
                                                          bool &outScalingListEnabled)
{  
   // VA is already converting hevc scaling lists to zigzag order
   // https://gitlab.freedesktop.org/mesa/mesa/-/commit/63dcfed81f011dae5ca68af3369433be28135415

   outScalingListEnabled = (pPipeDesc->pps->sps->scaling_list_enabled_flag != 0);
   if (outScalingListEnabled) {
      memcpy(outMatrixBuffer.ucScalingLists0, pPipeDesc->pps->sps->ScalingList4x4, 6 * 16);
      memcpy(outMatrixBuffer.ucScalingLists1, pPipeDesc->pps->sps->ScalingList8x8, 6 * 64);
      memcpy(outMatrixBuffer.ucScalingLists2, pPipeDesc->pps->sps->ScalingList16x16, 6 * 64);
      memcpy(outMatrixBuffer.ucScalingLists3, pPipeDesc->pps->sps->ScalingList32x32, 2 * 64);
      memcpy(outMatrixBuffer.ucScalingListDCCoefSizeID2, pPipeDesc->pps->sps->ScalingListDCCoeff16x16, 6);
      memcpy(outMatrixBuffer.ucScalingListDCCoefSizeID3, pPipeDesc->pps->sps->ScalingListDCCoeff32x32, 2);
   } else {
      memset(&outMatrixBuffer, 0, sizeof(outMatrixBuffer));
   }
}
