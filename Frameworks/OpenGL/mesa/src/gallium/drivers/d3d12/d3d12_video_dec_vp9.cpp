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
#include "d3d12_video_dec_vp9.h"
#include <cmath>

void
d3d12_video_decoder_refresh_dpb_active_references_vp9(struct d3d12_video_decoder *pD3D12Dec)
{
// Method overview
   // 1. Codec specific strategy in switch statement regarding reference frames eviction policy. Should only mark active
   // DPB references, leaving evicted ones as unused
   // 2. Call release_unused_references_texture_memory(); at the end of this method. Any references (and texture
   // allocations associated)
   //    that were left not marked as used in m_spDPBManager by step (2) are lost.

   // Assign DXVA original Index indices to current frame and references
   DXVA_PicParams_VP9 *pCurrPicParams = d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_VP9>(pD3D12Dec);

   for (uint8_t i = 0; i < _countof(pCurrPicParams->ref_frame_map); i++) {
      if (pD3D12Dec->m_pCurrentReferenceTargets[i]) {
         pCurrPicParams->ref_frame_map[i].Index7Bits =
            pD3D12Dec->m_spDPBManager->get_index7bits(pD3D12Dec->m_pCurrentReferenceTargets[i]);
      }
   }

   for (uint8_t i = 0; i < _countof(pCurrPicParams->frame_refs); i++) {
      if(!pCurrPicParams->frame_refs[i].AssociatedFlag)
         pCurrPicParams->frame_refs[i].Index7Bits = pCurrPicParams->ref_frame_map[pCurrPicParams->frame_refs[i].Index7Bits].Index7Bits;
   }

   pD3D12Dec->m_spDPBManager->mark_all_references_as_unused();
   pD3D12Dec->m_spDPBManager->mark_references_in_use(pCurrPicParams->ref_frame_map);

   // Releases the underlying reference picture texture objects of all references that were not marked as used in this
   // method.
   pD3D12Dec->m_spDPBManager->release_unused_references_texture_memory();

   pCurrPicParams->CurrPic.Index7Bits = pD3D12Dec->m_spDPBManager->get_index7bits(pD3D12Dec->m_pCurrentDecodeTarget);
}

void
d3d12_video_decoder_get_frame_info_vp9(
   struct d3d12_video_decoder *pD3D12Dec, uint32_t *pWidth, uint32_t *pHeight, uint16_t *pMaxDPB, bool *pIsInterlaced)
{
   auto pPicParams = d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_VP9>(pD3D12Dec);
   *pWidth = pPicParams->width;
   *pHeight = pPicParams->height;
   
   /*
      – The VP9 decoder maintains a pool (ref_frame_map[]) of 8 reference pictures at all times. Each 
      frame picks 3 reference frames (frame_refs[]) from the pool to use for inter prediction (known as Last, 
      Golden, and AltRef) of the current frame.
   */
   *pMaxDPB = 8 + 1 /*current picture*/;
   *pIsInterlaced = false;
}

void
d3d12_video_decoder_prepare_current_frame_references_vp9(struct d3d12_video_decoder *pD3D12Dec,
                                                          ID3D12Resource *pTexture2D,
                                                          uint32_t subresourceIndex)
{
   DXVA_PicParams_VP9 *pPicParams = d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_VP9>(pD3D12Dec);
   pPicParams->CurrPic.Index7Bits = pD3D12Dec->m_spDPBManager->store_future_reference(pPicParams->CurrPic.Index7Bits,
                                                                                      pD3D12Dec->m_spVideoDecoderHeap,
                                                                                      pTexture2D,
                                                                                      subresourceIndex);
   pD3D12Dec->m_spDPBManager->update_entries(
      d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_VP9>(pD3D12Dec)->frame_refs,
      pD3D12Dec->m_transitionsStorage);

   pD3D12Dec->m_spDPBManager->update_entries(
      d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_VP9>(pD3D12Dec)->ref_frame_map,
      pD3D12Dec->m_transitionsStorage);

   pD3D12Dec->m_spDecodeCommandList->ResourceBarrier(pD3D12Dec->m_transitionsStorage.size(), pD3D12Dec->m_transitionsStorage.data());

   // Schedule reverse (back to common) transitions before command list closes for current frame
   for (auto BarrierDesc : pD3D12Dec->m_transitionsStorage) {
      std::swap(BarrierDesc.Transition.StateBefore, BarrierDesc.Transition.StateAfter);
      pD3D12Dec->m_transitionsBeforeCloseCmdList.push_back(BarrierDesc);
   }

   debug_printf(
      "[d3d12_video_decoder_prepare_current_frame_references_vp9] DXVA_PicParams_VP9 after index remapping)\n");
   d3d12_video_decoder_log_pic_params_vp9(
      d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_VP9>(pD3D12Dec));
}

void
d3d12_video_decoder_log_pic_params_vp9(DXVA_PicParams_VP9 *pPicParams)
   {
   debug_printf("\n=============================================\n");
   debug_printf("[D3D12 Video Decoder VP9 DXVA PicParams info]\n");

   debug_printf("CurrPic.Index7Bits = %d\n", pPicParams->CurrPic.Index7Bits);
   debug_printf("CurrPic.AssociatedFlag = %d\n", pPicParams->CurrPic.AssociatedFlag);
   debug_printf("profile = %d\n", pPicParams->profile);
   debug_printf("frame_type = %d\n", pPicParams->frame_type);
   debug_printf("show_frame = %d\n", pPicParams->show_frame);
   debug_printf("error_resilient_mode = %d\n", pPicParams->error_resilient_mode);
   debug_printf("subsampling_x = %d\n", pPicParams->subsampling_x);
   debug_printf("subsampling_y = %d\n", pPicParams->subsampling_y);
   debug_printf("extra_plane = %d\n", pPicParams->extra_plane);
   debug_printf("refresh_frame_context = %d\n", pPicParams->refresh_frame_context);
   debug_printf("frame_parallel_decoding_mode = %d\n", pPicParams->frame_parallel_decoding_mode);
   debug_printf("intra_only = %d\n", pPicParams->intra_only);
   debug_printf("frame_context_idx = %d\n", pPicParams->frame_context_idx);
   debug_printf("reset_frame_context = %d\n", pPicParams->reset_frame_context);
   debug_printf("allow_high_precision_mv = %d\n", pPicParams->allow_high_precision_mv);
   debug_printf("ReservedFormatInfo2Bits = %d\n", pPicParams->ReservedFormatInfo2Bits);
   debug_printf("wFormatAndPictureInfoFlags = %d\n", pPicParams->wFormatAndPictureInfoFlags);
   debug_printf("width = %d\n", pPicParams->width);
   debug_printf("height = %d\n", pPicParams->height);
   debug_printf("BitDepthMinus8Luma = %d\n", pPicParams->BitDepthMinus8Luma);
   debug_printf("BitDepthMinus8Chroma = %d\n", pPicParams->BitDepthMinus8Chroma);
   debug_printf("interp_filter = %d\n", pPicParams->interp_filter);
   debug_printf("Reserved8Bits = %d\n", pPicParams->Reserved8Bits);

   for (uint32_t i = 0; i < _countof(pPicParams->ref_frame_map); i++) {
      debug_printf("ref_frame_map[%d].Index7Bits = %d\n", i, pPicParams->ref_frame_map[i].Index7Bits);
      debug_printf("ref_frame_map[%d].AssociatedFlag = %d\n", i, pPicParams->ref_frame_map[i].AssociatedFlag);
   }

   for (uint32_t i = 0; i < _countof(pPicParams->ref_frame_coded_width); i++)
      debug_printf("ref_frame_coded_width[%d] = %d\n", i, pPicParams->ref_frame_coded_width[i]);

   for (uint32_t i = 0; i < _countof(pPicParams->ref_frame_coded_height); i++)
      debug_printf("ref_frame_coded_height[%d] = %d\n", i, pPicParams->ref_frame_coded_height[i]);

   for (uint32_t i = 0; i < _countof(pPicParams->frame_refs); i++) {
      debug_printf("frame_refs[%d].Index7Bits = %d\n", i, pPicParams->frame_refs[i].Index7Bits);
      debug_printf("frame_refs[%d].AssociatedFlag = %d\n", i, pPicParams->frame_refs[i].AssociatedFlag);
   }

   for (uint32_t i = 0; i < _countof(pPicParams->ref_frame_sign_bias); i++)
      debug_printf("ref_frame_sign_bias[%d] = %d\n", i, pPicParams->ref_frame_sign_bias[i]);

   debug_printf("filter_level = %d\n", pPicParams->filter_level);
   debug_printf("sharpness_level = %d\n", pPicParams->sharpness_level);
   debug_printf("mode_ref_delta_enabled = %d\n", pPicParams->mode_ref_delta_enabled);
   debug_printf("mode_ref_delta_update = %d\n", pPicParams->mode_ref_delta_update);
   debug_printf("use_prev_in_find_mv_refs = %d\n", pPicParams->use_prev_in_find_mv_refs);
   debug_printf("ReservedControlInfo5Bits = %d\n", pPicParams->ReservedControlInfo5Bits);
   debug_printf("wControlInfoFlags; = %d\n", pPicParams->wControlInfoFlags);

   for (uint32_t i = 0; i < _countof(pPicParams->ref_deltas); i++)
      debug_printf("ref_deltas[%d] = %d\n", i, pPicParams->ref_deltas[i]);
   for (uint32_t i = 0; i < _countof(pPicParams->mode_deltas); i++)
      debug_printf("mode_deltas[%d] = %d\n", i, pPicParams->mode_deltas[i]);

   debug_printf("base_qindex = %d\n", pPicParams->base_qindex);
   debug_printf("y_dc_delta_q = %d\n", pPicParams->y_dc_delta_q);
   debug_printf("uv_dc_delta_q = %d\n", pPicParams->uv_dc_delta_q);
   debug_printf("uv_ac_delta_q = %d\n", pPicParams->uv_ac_delta_q);

   debug_printf("stVP9Segments.enabled = %d\n", pPicParams->stVP9Segments.enabled);
   debug_printf("stVP9Segments.update_map = %d\n", pPicParams->stVP9Segments.update_map);
   debug_printf("stVP9Segments.temporal_update = %d\n", pPicParams->stVP9Segments.temporal_update);
   debug_printf("stVP9Segments.abs_delta = %d\n", pPicParams->stVP9Segments.abs_delta);
   debug_printf("stVP9Segments.ReservedSegmentFlags4Bits = %d\n", pPicParams->stVP9Segments.ReservedSegmentFlags4Bits);
   debug_printf("stVP9Segments.wSegmentInfoFlags = %d\n", pPicParams->stVP9Segments.wSegmentInfoFlags);

   for (uint32_t i = 0; i < _countof(pPicParams->stVP9Segments.tree_probs); i++)
      debug_printf("tree_probs[%d] = %d\n", i, pPicParams->stVP9Segments.tree_probs[i]);

   for (uint32_t i = 0; i < _countof(pPicParams->stVP9Segments.pred_probs); i++)
      debug_printf("pred_probs[%d] = %d\n", i, pPicParams->stVP9Segments.pred_probs[i]);

   for (uint32_t i = 0; i < _countof(pPicParams->stVP9Segments.feature_data); i++)
      for (uint32_t j = 0; j < _countof(pPicParams->stVP9Segments.feature_data[0]); j++)
         debug_printf("feature_data[%d][%d] = %d\n", i, j, pPicParams->stVP9Segments.feature_data[i][j]);

   for (uint32_t i = 0; i < _countof(pPicParams->stVP9Segments.feature_mask); i++)
      debug_printf("feature_mask[%d] = %d\n", i, pPicParams->stVP9Segments.feature_mask[i]);

   debug_printf("log2_tile_cols = %d\n", pPicParams->log2_tile_cols);
   debug_printf("log2_tile_rows = %d\n", pPicParams->log2_tile_rows);
   debug_printf("uncompressed_header_size_byte_aligned = %d\n", pPicParams->uncompressed_header_size_byte_aligned);
   debug_printf("first_partition_size = %d\n", pPicParams->first_partition_size);
   debug_printf("Reserved16Bits = %d\n", pPicParams->Reserved16Bits);
   debug_printf("Reserved32Bits = %d\n", pPicParams->Reserved32Bits);
   debug_printf("StatusReportFeedbackNumber = %d\n", pPicParams->StatusReportFeedbackNumber);
}

void
d3d12_video_decoder_prepare_dxva_slices_control_vp9(struct d3d12_video_decoder *pD3D12Dec,
                                                     std::vector<uint8_t> &vecOutSliceControlBuffers,
                                                     struct pipe_vp9_picture_desc *picture_vp9)
{
   if(!picture_vp9->slice_parameter.slice_info_present)
   {
      unreachable("Unsupported - need pipe_vp9_picture_desc.slice_parameter.slice_info_present");
   }

   debug_printf("[d3d12_video_decoder_vp9] Upper layer reported %d slices for this frame, parsing them below...\n",
                  picture_vp9->slice_parameter.slice_count);

   uint64_t TotalSlicesDXVAArrayByteSize = picture_vp9->slice_parameter.slice_count * sizeof(DXVA_Slice_VPx_Short);
   vecOutSliceControlBuffers.resize(TotalSlicesDXVAArrayByteSize);

   uint8_t* pData = vecOutSliceControlBuffers.data();
   for (uint32_t sliceIdx = 0; sliceIdx < picture_vp9->slice_parameter.slice_count; sliceIdx++)
   {
      DXVA_Slice_VPx_Short currentSliceEntry = {};
      // From DXVA Spec
      // wBadSliceChopping
      // 0	All bits for the slice are located within the corresponding bitstream data buffer. 
      // 1	The bitstream data buffer contains the start of the slice, but not the entire slice, because the buffer is full. 
      // 2	The bitstream data buffer contains the end of the slice. It does not contain the start of the slice, because the start of the slice was located in the previous bitstream data buffer. 
      // 3	The bitstream data buffer does not contain the start of the slice (because the start of the slice was located in the previous bitstream data buffer),
      //     and it does not contain the end of the slice (because the current bitstream data buffer is also full). 

      switch (picture_vp9->slice_parameter.slice_data_flag[sliceIdx]) {
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

      currentSliceEntry.SliceBytesInBuffer = picture_vp9->slice_parameter.slice_data_size[sliceIdx];
      currentSliceEntry.BSNALunitDataLocation = picture_vp9->slice_parameter.slice_data_offset[sliceIdx];

      debug_printf("[d3d12_video_decoder_vp9] Detected slice index %" PRIu32 " with SliceBytesInBuffer %d - BSNALunitDataLocation %d - wBadSliceChopping: %" PRIu16
                  " for frame with "
                  "fenceValue: %d\n",
                  sliceIdx,
                  currentSliceEntry.SliceBytesInBuffer,
                  currentSliceEntry.BSNALunitDataLocation,
                  currentSliceEntry.wBadSliceChopping,
                  pD3D12Dec->m_fenceValue);

      memcpy(pData, &currentSliceEntry, sizeof(DXVA_Slice_VPx_Short));
      pData += sizeof(DXVA_Slice_VPx_Short);
   }
   assert(vecOutSliceControlBuffers.size() == TotalSlicesDXVAArrayByteSize);
}

DXVA_PicParams_VP9
d3d12_video_decoder_dxva_picparams_from_pipe_picparams_vp9(
   struct d3d12_video_decoder *pD3D12Dec,
   pipe_video_profile profile,
   pipe_vp9_picture_desc *pipe_vp9)
{
   uint32_t frameNum = pD3D12Dec->m_fenceValue;
   DXVA_PicParams_VP9 dxvaStructure;   
   memset(&dxvaStructure, 0, sizeof(dxvaStructure));   

   dxvaStructure.profile = pipe_vp9->picture_parameter.profile;
   dxvaStructure.wFormatAndPictureInfoFlags = ((pipe_vp9->picture_parameter.pic_fields.frame_type != 0)   <<  0) |
                                    ((pipe_vp9->picture_parameter.pic_fields.show_frame != 0)             <<  1) |
                                    (pipe_vp9->picture_parameter.pic_fields.error_resilient_mode          <<  2) |
                                    (pipe_vp9->picture_parameter.pic_fields.subsampling_x                 <<  3) |
                                    (pipe_vp9->picture_parameter.pic_fields.subsampling_y                 <<  4) |
                                    (0                                                                    <<  5) |
                                    (pipe_vp9->picture_parameter.pic_fields.refresh_frame_context         <<  6) |
                                    (pipe_vp9->picture_parameter.pic_fields.frame_parallel_decoding_mode  <<  7) |
                                    (pipe_vp9->picture_parameter.pic_fields.intra_only                    <<  8) |
                                    (pipe_vp9->picture_parameter.pic_fields.frame_context_idx             <<  9) |
                                    (pipe_vp9->picture_parameter.pic_fields.reset_frame_context           << 11) |
                                    ((pipe_vp9->picture_parameter.pic_fields.allow_high_precision_mv)     << 13) |
                                    (0                                                                    << 14);

   dxvaStructure.width  = pipe_vp9->picture_parameter.frame_width;
   dxvaStructure.height = pipe_vp9->picture_parameter.frame_height;
   dxvaStructure.BitDepthMinus8Luma   = pipe_vp9->picture_parameter.bit_depth - 8;
   dxvaStructure.BitDepthMinus8Chroma = pipe_vp9->picture_parameter.bit_depth - 8;
   dxvaStructure.interp_filter = pipe_vp9->picture_parameter.pic_fields.mcomp_filter_type;
   dxvaStructure.Reserved8Bits = 0;
   for (uint32_t i = 0; i < 8; i++) {
      if (pipe_vp9->ref[i]) {
         dxvaStructure.ref_frame_coded_width[i]  = pipe_vp9->ref[i]->width;
         dxvaStructure.ref_frame_coded_height[i] = pipe_vp9->ref[i]->height;
      } else
         dxvaStructure.ref_frame_map[i].bPicEntry = DXVA_VP9_INVALID_PICTURE_ENTRY;
   }

   /* DXVA spec The enums and indices for ref_frame_sign_bias[] are defined */
   const uint8_t signbias_last_index = 1;
   const uint8_t signbias_golden_index = 2;
   const uint8_t signbias_alt_index = 3;

   /* AssociatedFlag When Index7Bits does not contain an index to a valid uncompressed surface, the value shall be set to 127, to indicate that the index is invalid. */
   memset(&dxvaStructure.frame_refs[0], DXVA_VP9_INVALID_PICTURE_ENTRY, sizeof(dxvaStructure.frame_refs));

   if (pipe_vp9->ref[pipe_vp9->picture_parameter.pic_fields.last_ref_frame]) {
      /* AssociatedFlag When Index7Bits does not contain an index to a valid uncompressed surface, the value shall be set to 127, to indicate that the index is invalid. */
      /* Mark AssociatedFlag = 0 so last_ref_frame will be replaced with the correct reference index in d3d12_video_decoder_refresh_dpb_active_references_vp9 */
      dxvaStructure.frame_refs[0].AssociatedFlag = 0;
      dxvaStructure.frame_refs[0].Index7Bits = pipe_vp9->picture_parameter.pic_fields.last_ref_frame;
      dxvaStructure.ref_frame_sign_bias[signbias_last_index] = pipe_vp9->picture_parameter.pic_fields.last_ref_frame_sign_bias;
   }

   if (pipe_vp9->ref[pipe_vp9->picture_parameter.pic_fields.golden_ref_frame]) {
      /* AssociatedFlag When Index7Bits does not contain an index to a valid uncompressed surface, the value shall be set to 127, to indicate that the index is invalid. */
      /* Mark AssociatedFlag = 0 so golden_ref_frame will be replaced with the correct reference index in d3d12_video_decoder_refresh_dpb_active_references_vp9 */
      dxvaStructure.frame_refs[1].AssociatedFlag = 0;
      dxvaStructure.frame_refs[1].Index7Bits = pipe_vp9->picture_parameter.pic_fields.golden_ref_frame;
      dxvaStructure.ref_frame_sign_bias[signbias_golden_index] = pipe_vp9->picture_parameter.pic_fields.golden_ref_frame_sign_bias;
   }

  if (pipe_vp9->ref[pipe_vp9->picture_parameter.pic_fields.alt_ref_frame]) {
      /* AssociatedFlag When Index7Bits does not contain an index to a valid uncompressed surface, the value shall be set to 127, to indicate that the index is invalid. */
      /* Mark AssociatedFlag = 0 so alt_ref_frame will be replaced with the correct reference index in d3d12_video_decoder_refresh_dpb_active_references_vp9 */
      dxvaStructure.frame_refs[2].AssociatedFlag = 0;
      dxvaStructure.frame_refs[2].Index7Bits = pipe_vp9->picture_parameter.pic_fields.alt_ref_frame;
      dxvaStructure.ref_frame_sign_bias[signbias_alt_index] = pipe_vp9->picture_parameter.pic_fields.alt_ref_frame_sign_bias;
   }

   dxvaStructure.filter_level    = pipe_vp9->picture_parameter.filter_level;
   dxvaStructure.sharpness_level = pipe_vp9->picture_parameter.sharpness_level;

   bool use_prev_in_find_mv_refs =
      !pipe_vp9->picture_parameter.pic_fields.error_resilient_mode &&
      !(pipe_vp9->picture_parameter.pic_fields.frame_type == 0 /*KEY_FRAME*/ || pipe_vp9->picture_parameter.pic_fields.intra_only) &&
      pipe_vp9->picture_parameter.pic_fields.prev_show_frame &&
      pipe_vp9->picture_parameter.frame_width == pipe_vp9->picture_parameter.prev_frame_width &&
      pipe_vp9->picture_parameter.frame_height == pipe_vp9->picture_parameter.prev_frame_height;

   dxvaStructure.wControlInfoFlags = (pipe_vp9->picture_parameter.mode_ref_delta_enabled  << 0) |
                           (pipe_vp9->picture_parameter.mode_ref_delta_update             << 1) |
                           (use_prev_in_find_mv_refs                                      << 2) |
                           (0                                                             << 3);

   for (uint32_t i = 0; i < 4; i++)
      dxvaStructure.ref_deltas[i] = pipe_vp9->picture_parameter.ref_deltas[i];

   for (uint32_t i = 0; i < 2; i++)
      dxvaStructure.mode_deltas[i] = pipe_vp9->picture_parameter.mode_deltas[i];

   dxvaStructure.base_qindex   = pipe_vp9->picture_parameter.base_qindex;
   dxvaStructure.y_dc_delta_q  = pipe_vp9->picture_parameter.y_dc_delta_q;
   dxvaStructure.uv_dc_delta_q = pipe_vp9->picture_parameter.uv_ac_delta_q;
   dxvaStructure.uv_ac_delta_q = pipe_vp9->picture_parameter.uv_dc_delta_q;

   /* segmentation data */
   dxvaStructure.stVP9Segments.wSegmentInfoFlags = (pipe_vp9->picture_parameter.pic_fields.segmentation_enabled   << 0) |
                                       (pipe_vp9->picture_parameter.pic_fields.segmentation_update_map            << 1) |
                                       (pipe_vp9->picture_parameter.pic_fields.segmentation_temporal_update       << 2) |
                                       (pipe_vp9->picture_parameter.abs_delta                                     << 3) |
                                       (0                                                                         << 4);

   for (uint32_t i = 0; i < 7; i++)
      dxvaStructure.stVP9Segments.tree_probs[i] = pipe_vp9->picture_parameter.mb_segment_tree_probs[i];

   if (pipe_vp9->picture_parameter.pic_fields.segmentation_temporal_update)
      for (uint32_t i = 0; i < 3; i++)
         dxvaStructure.stVP9Segments.pred_probs[i] = pipe_vp9->picture_parameter.segment_pred_probs[i];
   else
      memset(dxvaStructure.stVP9Segments.pred_probs, 255, sizeof(dxvaStructure.stVP9Segments.pred_probs));

   for (uint32_t i = 0; i < 8; i++) {
      dxvaStructure.stVP9Segments.feature_mask[i] = (pipe_vp9->slice_parameter.seg_param[i].alt_quant_enabled              << 0) |
                                          (pipe_vp9->slice_parameter.seg_param[i].alt_lf_enabled                           << 1) |
                                          (pipe_vp9->slice_parameter.seg_param[i].segment_flags.segment_reference_enabled  << 2) |
                                          (pipe_vp9->slice_parameter.seg_param[i].segment_flags.segment_reference_skipped  << 3);

      dxvaStructure.stVP9Segments.feature_data[i][0] = pipe_vp9->slice_parameter.seg_param[i].alt_quant;
      dxvaStructure.stVP9Segments.feature_data[i][1] = pipe_vp9->slice_parameter.seg_param[i].alt_lf;
      dxvaStructure.stVP9Segments.feature_data[i][2] = pipe_vp9->slice_parameter.seg_param[i].segment_flags.segment_reference;
      dxvaStructure.stVP9Segments.feature_data[i][3] = 0;
   }

   dxvaStructure.log2_tile_cols = pipe_vp9->picture_parameter.log2_tile_columns;
   dxvaStructure.log2_tile_rows = pipe_vp9->picture_parameter.log2_tile_rows;
   dxvaStructure.uncompressed_header_size_byte_aligned = pipe_vp9->picture_parameter.frame_header_length_in_bytes;
   dxvaStructure.first_partition_size = pipe_vp9->picture_parameter.first_partition_size;
   dxvaStructure.StatusReportFeedbackNumber = frameNum;
   assert(dxvaStructure.StatusReportFeedbackNumber > 0);
   return dxvaStructure;
}
