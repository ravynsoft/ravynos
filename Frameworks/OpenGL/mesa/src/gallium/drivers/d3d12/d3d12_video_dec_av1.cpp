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
#include "d3d12_video_dec_av1.h"
#include <cmath>

void
d3d12_video_decoder_refresh_dpb_active_references_av1(struct d3d12_video_decoder *pD3D12Dec)
{
// Method overview
   // 1. Codec specific strategy in switch statement regarding reference frames eviction policy. Should only mark active
   // DPB references, leaving evicted ones as unused
   // 2. Call release_unused_references_texture_memory(); at the end of this method. Any references (and texture
   // allocations associated)
   //    that were left not marked as used in m_spDPBManager by step (2) are lost.

   // Assign DXVA original Index indices to current frame and references
   DXVA_PicParams_AV1 *pCurrPicParams = d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_AV1>(pD3D12Dec);

   for (uint8_t i = 0; i < _countof(pCurrPicParams->RefFrameMapTextureIndex); i++) {
      if (pD3D12Dec->m_pCurrentReferenceTargets[i]) {
         pCurrPicParams->RefFrameMapTextureIndex[i] =
            pD3D12Dec->m_spDPBManager->get_index7bits(pD3D12Dec->m_pCurrentReferenceTargets[i]);
      }
   }

   pD3D12Dec->m_spDPBManager->mark_all_references_as_unused();
   pD3D12Dec->m_spDPBManager->mark_references_in_use_av1(pCurrPicParams->RefFrameMapTextureIndex);

   // Releases the underlying reference picture texture objects of all references that were not marked as used in this
   // method.
   pD3D12Dec->m_spDPBManager->release_unused_references_texture_memory();

   pCurrPicParams->CurrPicTextureIndex = pD3D12Dec->m_spDPBManager->get_index7bits(pD3D12Dec->m_pCurrentDecodeTarget);
}

void
d3d12_video_decoder_get_frame_info_av1(
   struct d3d12_video_decoder *pD3D12Dec, uint32_t *pWidth, uint32_t *pHeight, uint16_t *pMaxDPB, bool &isInterlaced)
{
   auto pPicParams = d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_AV1>(pD3D12Dec);
   /* width, height
      Specify the coded width and height of the current frame. 
      These correspond to the syntax elements of: frame_width_minus_1 and frame_height_minus_1.  
      If these values are derived for the frame (for example via the frame_size_override_flag), 
      the host decoder will derive the appropriate values and store the result here.
      If superres is enabled these values represent the post-scaled frame resolution 
      (referred to in the specification as UpscaledWidth). */
   *pWidth = pPicParams->width;
   *pHeight = pPicParams->height;
   
   /*
      Note – The AV1 decoder maintains a pool (RefFrameMapTextureIndex[]) of 8 reference pictures at all times.
      Each frame may pick up to 7 reference frames (frame_refs[]) from the pool to use for inter prediction of the current frame. 
   */
   *pMaxDPB = 8 + 1 /*current picture*/;
   isInterlaced = false;
}

void
d3d12_video_decoder_prepare_current_frame_references_av1(struct d3d12_video_decoder *pD3D12Dec,
                                                          ID3D12Resource *pTexture2D,
                                                          uint32_t subresourceIndex)
{
   DXVA_PicParams_AV1 *pPicParams = d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_AV1>(pD3D12Dec);
   pPicParams->CurrPicTextureIndex = pD3D12Dec->m_spDPBManager->store_future_reference(pPicParams->CurrPicTextureIndex,
                                                                                      pD3D12Dec->m_spVideoDecoderHeap,
                                                                                      pTexture2D,
                                                                                      subresourceIndex);
   pD3D12Dec->m_spDPBManager->update_entries_av1(
      d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_AV1>(pD3D12Dec)->RefFrameMapTextureIndex,
      pD3D12Dec->m_transitionsStorage);

   pD3D12Dec->m_spDecodeCommandList->ResourceBarrier(pD3D12Dec->m_transitionsStorage.size(), pD3D12Dec->m_transitionsStorage.data());

   // Schedule reverse (back to common) transitions before command list closes for current frame
   for (auto BarrierDesc : pD3D12Dec->m_transitionsStorage) {
      std::swap(BarrierDesc.Transition.StateBefore, BarrierDesc.Transition.StateAfter);
      pD3D12Dec->m_transitionsBeforeCloseCmdList.push_back(BarrierDesc);
   }

   debug_printf(
      "[d3d12_video_decoder_prepare_current_frame_references_av1] DXVA_PicParams_AV1 after index remapping)\n");
   d3d12_video_decoder_log_pic_params_av1(
      d3d12_video_decoder_get_current_dxva_picparams<DXVA_PicParams_AV1>(pD3D12Dec));
}

void
d3d12_video_decoder_log_pic_params_av1(DXVA_PicParams_AV1 *pPicParams)
{

   debug_printf("\n=============================================\n");
   debug_printf("width = %d\n", pPicParams->width);
   debug_printf("height = %d\n", pPicParams->height);

   debug_printf("max_width = %d\n", pPicParams->max_width);
   debug_printf("max_height = %d\n", pPicParams->max_height);

   debug_printf("CurrPicTextureIndex = %d\n", pPicParams->CurrPicTextureIndex);
   debug_printf("superres_denom = %d\n", pPicParams->superres_denom);
   debug_printf("bitdepth = %d\n", pPicParams->bitdepth);
   debug_printf("seq_profile = %d\n", pPicParams->seq_profile);

   debug_printf("cols = %d\n", pPicParams->tiles.cols);
   debug_printf("rows = %d\n", pPicParams->tiles.rows);
   debug_printf("context_update_id = %d\n", pPicParams->tiles.context_update_id);
   for (uint32_t i = 0; i < pPicParams->tiles.cols; i++)
      debug_printf("widths[%d] = %d\n", i, pPicParams->tiles.widths[i]);
   for (uint32_t i = 0; i < pPicParams->tiles.rows; i++)
      debug_printf("heights[%d] = %d\n", i, pPicParams->tiles.heights[i]);

   debug_printf("coding.use_128x128_superblock = %d\n", pPicParams->coding.use_128x128_superblock);
   debug_printf("coding.intra_edge_filter = %d\n", pPicParams->coding.intra_edge_filter);
   debug_printf("coding.interintra_compound = %d\n", pPicParams->coding.interintra_compound);
   debug_printf("coding.masked_compound = %d\n", pPicParams->coding.masked_compound);
   debug_printf("coding.warped_motion = %d\n", pPicParams->coding.warped_motion);
   debug_printf("coding.dual_filter = %d\n", pPicParams->coding.dual_filter);
   debug_printf("coding.jnt_comp = %d\n", pPicParams->coding.jnt_comp);
   debug_printf("coding.screen_content_tools = %d\n", pPicParams->coding.screen_content_tools);
   debug_printf("coding.integer_mv = %d\n", pPicParams->coding.integer_mv);
   debug_printf("coding.cdef = %d\n", pPicParams->coding.cdef);
   debug_printf("coding.restoration = %d\n", pPicParams->coding.restoration);
   debug_printf("coding.film_grain = %d\n", pPicParams->coding.film_grain);
   debug_printf("coding.intrabc = %d\n", pPicParams->coding.intrabc);
   debug_printf("coding.high_precision_mv = %d\n", pPicParams->coding.high_precision_mv);
   debug_printf("coding.switchable_motion_mode = %d\n", pPicParams->coding.switchable_motion_mode);
   debug_printf("coding.filter_intra = %d\n", pPicParams->coding.filter_intra);
   debug_printf("coding.disable_frame_end_update_cdf = %d\n", pPicParams->coding.disable_frame_end_update_cdf);
   debug_printf("coding.disable_cdf_update = %d\n", pPicParams->coding.disable_cdf_update);
   debug_printf("coding.reference_mode = %d\n", pPicParams->coding.reference_mode);
   debug_printf("coding.skip_mode = %d\n", pPicParams->coding.skip_mode);
   debug_printf("coding.reduced_tx_set = %d\n", pPicParams->coding.reduced_tx_set);
   debug_printf("coding.superres = %d\n", pPicParams->coding.superres);
   debug_printf("coding.tx_mode = %d\n", pPicParams->coding.tx_mode);
   debug_printf("coding.use_ref_frame_mvs = %d\n", pPicParams->coding.use_ref_frame_mvs);
   debug_printf("coding.enable_ref_frame_mvs = %d\n", pPicParams->coding.enable_ref_frame_mvs);
   debug_printf("coding.reference_frame_update = %d\n", pPicParams->coding.reference_frame_update);
   debug_printf("coding.Reserved = %d\n", pPicParams->coding.Reserved);
   debug_printf("coding.CodingParamToolFlags = %d\n", pPicParams->coding.CodingParamToolFlags);

   debug_printf("format.frame_type = %d\n", pPicParams->format.frame_type);
   debug_printf("format.show_frame = %d\n", pPicParams->format.show_frame);
   debug_printf("format.showable_frame = %d\n", pPicParams->format.showable_frame);
   debug_printf("format.subsampling_x = %d\n", pPicParams->format.subsampling_x);
   debug_printf("format.subsampling_y = %d\n", pPicParams->format.subsampling_y);
   debug_printf("format.mono_chrome = %d\n", pPicParams->format.mono_chrome);
   debug_printf("format.Reserved = %d\n", pPicParams->format.Reserved);

   debug_printf("primary_ref_frame = %d\n", pPicParams->primary_ref_frame);
   debug_printf("order_hint = %d\n", pPicParams->order_hint);
   debug_printf("order_hint_bits = %d\n", pPicParams->order_hint_bits);

   for (uint32_t i = 0; i < _countof(pPicParams->frame_refs); i++) {
      debug_printf("frame_refs[%d]\n", i);
      debug_printf("\twidth = %d\n", pPicParams->frame_refs[i].width);
      debug_printf("\theight = %d\n", pPicParams->frame_refs[i].height);

      // Global motion parameters
      for (uint32_t j = 0; j < 6; j++)
         debug_printf("\t\twmmat[%d] = %d\n", j, pPicParams->frame_refs[i].wmmat[j]);
      debug_printf("\twminvalid = %d\n", pPicParams->frame_refs[i].wminvalid);
      debug_printf("\twmtype = %d\n", pPicParams->frame_refs[i].wmtype);
      debug_printf("\tGlobalMotionFlags = %d\n", pPicParams->frame_refs[i].GlobalMotionFlags);
      debug_printf("\tIndex = %d\n", pPicParams->frame_refs[i].Index);
   }

   for (uint32_t i = 0; i < _countof(pPicParams->RefFrameMapTextureIndex); i++)
      debug_printf("RefFrameMapTextureIndex[%d] = %d\n", i, pPicParams->RefFrameMapTextureIndex[i]);

   // Loop filter parameters
   debug_printf("filter_level[0] = %d\n", pPicParams->loop_filter.filter_level[0]);
   debug_printf("filter_level[1] = %d\n", pPicParams->loop_filter.filter_level[1]);
   debug_printf("filter_level_u = %d\n", pPicParams->loop_filter.filter_level_u);
   debug_printf("filter_level_v = %d\n", pPicParams->loop_filter.filter_level_v);
   debug_printf("sharpness_level = %d\n", pPicParams->loop_filter.sharpness_level);
   debug_printf("mode_ref_delta_enabled = %d\n", pPicParams->loop_filter.mode_ref_delta_enabled);
   debug_printf("mode_ref_delta_update = %d\n", pPicParams->loop_filter.mode_ref_delta_update);
   debug_printf("delta_lf_multi = %d\n", pPicParams->loop_filter.delta_lf_multi);
   debug_printf("delta_lf_present = %d\n", pPicParams->loop_filter.delta_lf_present);
   debug_printf("ControlFlags = %d\n", pPicParams->loop_filter.ControlFlags);

   for (uint32_t i = 0; i < _countof(pPicParams->loop_filter.ref_deltas); i++)
      debug_printf("loop_filter.ref_deltas[%d] = %d\n", i, pPicParams->loop_filter.ref_deltas[i]);
   for (uint32_t i = 0; i < _countof(pPicParams->loop_filter.mode_deltas); i++)
      debug_printf("loop_filter.mode_deltas[%d] = %d\n", i, pPicParams->loop_filter.mode_deltas[i]);
   
   debug_printf("delta_lf_res = %d\n", pPicParams->loop_filter.delta_lf_res);
   
   for (uint32_t i = 0; i < _countof(pPicParams->loop_filter.frame_restoration_type); i++)
      debug_printf("loop_filter.frame_restoration_type[%d] = %d\n", i, pPicParams->loop_filter.frame_restoration_type[i]);

   for (uint32_t i = 0; i < _countof(pPicParams->loop_filter.log2_restoration_unit_size); i++)
      debug_printf("loop_filter.log2_restoration_unit_size[%d] = %d\n", i, pPicParams->loop_filter.log2_restoration_unit_size[i]);

   // Quantization
   debug_printf("delta_q_present = %d\n", pPicParams->quantization.delta_q_present);
   debug_printf("delta_q_res = %d\n", pPicParams->quantization.delta_q_res);
   debug_printf("ControlFlags = %d\n", pPicParams->quantization.ControlFlags);
   debug_printf("quantization.base_qindex = %d\n", pPicParams->quantization.base_qindex);
   debug_printf("quantization.y_dc_delta_q = %d\n", pPicParams->quantization.y_dc_delta_q);
   debug_printf("quantization.u_dc_delta_q = %d\n", pPicParams->quantization.u_dc_delta_q);
   debug_printf("quantization.v_dc_delta_q = %d\n", pPicParams->quantization.v_dc_delta_q);
   debug_printf("quantization.u_ac_delta_q = %d\n", pPicParams->quantization.u_ac_delta_q);
   debug_printf("quantization.v_ac_delta_q = %d\n", pPicParams->quantization.v_ac_delta_q);
   // using_qmatrix:
   debug_printf("quantization.qm_y = %d\n", pPicParams->quantization.qm_y);
   debug_printf("quantization.qm_u = %d\n", pPicParams->quantization.qm_u);
   debug_printf("quantization.qm_v = %d\n", pPicParams->quantization.qm_v);

    // Cdef parameters
   debug_printf("cdef.damping = %d\n", pPicParams->cdef.damping);
   debug_printf("cdef.bits = %d\n", pPicParams->cdef.bits);
   debug_printf("cdef.Reserved = %d\n", pPicParams->cdef.Reserved);
   debug_printf("cdef.ControlFlags = %d\n", pPicParams->cdef.ControlFlags);
   
   for (uint32_t i = 0; i < _countof(pPicParams->cdef.y_strengths); i++) {
      debug_printf("cdef.primaryuv_strengthsy%d] = %d\n", i, pPicParams->cdef.y_strengths[i].primary);
      debug_printf("cdef.y_strengths.secondary[%d] = %d\n", i, pPicParams->cdef.y_strengths[i].secondary);
      debug_printf("cdef.y_strengths.combined[%d] = %d\n", i, pPicParams->cdef.y_strengths[i].combined);
   }      

   for (uint32_t i = 0; i < _countof(pPicParams->cdef.uv_strengths); i++) {
      debug_printf("cdef.uv_strengths.primary[%d] = %d\n", i, pPicParams->cdef.uv_strengths[i].primary);
      debug_printf("cdef.uv_strengths.secondary[%d] = %d\n", i, pPicParams->cdef.uv_strengths[i].secondary);
      debug_printf("cdef.uv_strengths.combined[%d] = %d\n", i, pPicParams->cdef.uv_strengths[i].combined);
   }      

   debug_printf("interp_filter = %d\n", pPicParams->interp_filter);

    // Segmentation
   debug_printf("segmentation.enabled = %d\n", pPicParams->segmentation.enabled);
   debug_printf("segmentation.update_map = %d\n", pPicParams->segmentation.update_map);
   debug_printf("segmentation.update_data = %d\n", pPicParams->segmentation.update_data);
   debug_printf("segmentation.temporal_update = %d\n", pPicParams->segmentation.temporal_update);
   debug_printf("segmentation.Reserved = %d\n", pPicParams->segmentation.Reserved);
   debug_printf("segmentation.ControlFlags = %d\n", pPicParams->segmentation.ControlFlags);

   for (uint32_t i = 0; i < _countof(pPicParams->segmentation.feature_mask); i++) {
      debug_printf("segmentation.feature_mask[%d].alt_q = %d\n", i, pPicParams->segmentation.feature_mask[i].alt_q);
      debug_printf("segmentation.feature_mask[%d].alt_lf_y_v = %d\n", i, pPicParams->segmentation.feature_mask[i].alt_lf_y_v);
      debug_printf("segmentation.feature_mask[%d].alt_lf_y_h = %d\n", i, pPicParams->segmentation.feature_mask[i].alt_lf_y_h);
      debug_printf("segmentation.feature_mask[%d].alt_lf_u = %d\n", i, pPicParams->segmentation.feature_mask[i].alt_lf_u);
      debug_printf("segmentation.feature_mask[%d].alt_lf_v = %d\n", i, pPicParams->segmentation.feature_mask[i].alt_lf_v);
      debug_printf("segmentation.feature_mask[%d].ref_frame = %d\n", i, pPicParams->segmentation.feature_mask[i].ref_frame);
      debug_printf("segmentation.feature_mask[%d].skip = %d\n", i, pPicParams->segmentation.feature_mask[i].skip);
      debug_printf("segmentation.feature_mask[%d].globalmv = %d\n", i, pPicParams->segmentation.feature_mask[i].globalmv);
      debug_printf("segmentation.feature_mask[%d].mask = %d\n", i, pPicParams->segmentation.feature_mask[i].mask);
   }

   for (uint32_t i = 0; i < 8; i++)
      for (uint32_t j = 0; j < 8; j++)
        debug_printf("segmentation.feature_data[%d][%d] = %d\n", i, j, pPicParams->segmentation.feature_data[i][j]);

   debug_printf("film_grain.apply_grain = %d\n", pPicParams->film_grain.apply_grain);
   debug_printf("film_grain.scaling_shift_minus8 = %d\n", pPicParams->film_grain.scaling_shift_minus8);
   debug_printf("film_grain.chroma_scaling_from_luma = %d\n", pPicParams->film_grain.chroma_scaling_from_luma);
   debug_printf("film_grain.ar_coeff_lag = %d\n", pPicParams->film_grain.ar_coeff_lag);
   debug_printf("film_grain.ar_coeff_shift_minus6 = %d\n", pPicParams->film_grain.ar_coeff_shift_minus6);
   debug_printf("film_grain.grain_scale_shift = %d\n", pPicParams->film_grain.grain_scale_shift);
   debug_printf("film_grain.overlap_flag = %d\n", pPicParams->film_grain.overlap_flag);
   debug_printf("film_grain.clip_to_restricted_range = %d\n", pPicParams->film_grain.clip_to_restricted_range);
   debug_printf("film_grain.matrix_coeff_is_identity = %d\n", pPicParams->film_grain.matrix_coeff_is_identity);
   debug_printf("film_grain.Reserved = %d\n", pPicParams->film_grain.Reserved);
   debug_printf("film_grain.ControlFlags = %d\n", pPicParams->film_grain.ControlFlags);

   debug_printf("film_grain.grain_seed = %d\n", pPicParams->film_grain.grain_seed);
                        
   for (uint32_t i = 0; i < 14; i++)
      for (uint32_t j = 0; j < 2; j++)
        debug_printf("film_grain.scaling_points_y[%d][%d] = %d\n", i, j, pPicParams->film_grain.scaling_points_y[i][j]);

   debug_printf("film_grain.num_y_points = %d\n", pPicParams->film_grain.num_y_points);
                        
   for (uint32_t i = 0; i < 10; i++)
      for (uint32_t j = 0; j < 2; j++)
        debug_printf("film_grain.scaling_points_cb[%d][%d] = %d\n", i, j, pPicParams->film_grain.scaling_points_cb[i][j]);

   debug_printf("film_grain.num_cb_points = %d\n", pPicParams->film_grain.num_cb_points);
                        
   for (uint32_t i = 0; i < 10; i++)
      for (uint32_t j = 0; j < 2; j++)
        debug_printf("film_grain.scaling_points_cr[%d][%d] = %d\n", i, j, pPicParams->film_grain.scaling_points_cr[i][j]);

   debug_printf("film_grain.num_cr_points = %d\n", pPicParams->film_grain.num_cr_points);
   for (uint32_t i = 0; i < _countof(pPicParams->film_grain.ar_coeffs_y); i++)
      debug_printf("film_grain.ar_coeffs_y[%d] = %d\n", i, pPicParams->film_grain.ar_coeffs_y[i]);

   for (uint32_t i = 0; i < _countof(pPicParams->film_grain.ar_coeffs_cb); i++)
      debug_printf("film_grain.ar_coeffs_cb[%d] = %d\n", i, pPicParams->film_grain.ar_coeffs_cb[i]);

   for (uint32_t i = 0; i < _countof(pPicParams->film_grain.ar_coeffs_cr); i++)
      debug_printf("film_grain.ar_coeffs_cr[%d] = %d\n", i, pPicParams->film_grain.ar_coeffs_cr[i]);

   debug_printf("film_grain.cb_mult = %d\n", pPicParams->film_grain.cb_mult);
   debug_printf("film_grain.cb_luma_mult = %d\n", pPicParams->film_grain.cb_luma_mult);
   debug_printf("film_grain.cr_mult = %d\n", pPicParams->film_grain.cr_mult);
   debug_printf("film_grain.cr_luma_mult = %d\n", pPicParams->film_grain.cr_luma_mult);
   debug_printf("film_grain.Reserved8Bits = %d\n", pPicParams->film_grain.Reserved8Bits);
   debug_printf("film_grain.cb_offset = %d\n", pPicParams->film_grain.cb_offset);
   debug_printf("film_grain.cr_offset = %d\n", pPicParams->film_grain.cr_offset);

   debug_printf("Reserved32Bits = %d\n", pPicParams->Reserved32Bits);
   debug_printf("StatusReportFeedbackNumber = %d\n", pPicParams->StatusReportFeedbackNumber);
}

void
d3d12_video_decoder_prepare_dxva_slices_control_av1(struct d3d12_video_decoder *pD3D12Dec,
                                                     std::vector<uint8_t> &vecOutSliceControlBuffers,
                                                     struct pipe_av1_picture_desc *picture_av1)
{
   uint32_t tileCount = picture_av1->picture_parameter.tile_cols * picture_av1->picture_parameter.tile_rows;
   debug_printf("[d3d12_video_decoder_av1] Upper layer reported %d tiles for this frame, parsing them below...\n",
                  tileCount);

   uint64_t totalSlicesDXVAArrayByteSize = tileCount * sizeof(DXVA_Tile_AV1);
   vecOutSliceControlBuffers.resize(totalSlicesDXVAArrayByteSize);

   uint8_t* pData = vecOutSliceControlBuffers.data();
   for (uint32_t tileIdx = 0; tileIdx < tileCount; tileIdx++)
   {
      DXVA_Tile_AV1 currentTileEntry = {};
      currentTileEntry.DataOffset   = picture_av1->slice_parameter.slice_data_offset[tileIdx];
      currentTileEntry.DataSize     = picture_av1->slice_parameter.slice_data_size[tileIdx];
      currentTileEntry.row          = picture_av1->slice_parameter.slice_data_row[tileIdx];
      currentTileEntry.column       = picture_av1->slice_parameter.slice_data_col[tileIdx];
      // From va_dec_av1.h `anchor_frame_idx` valid only when large_scale_tile equals 1.
      currentTileEntry.anchor_frame = (picture_av1->picture_parameter.pic_info_fields.large_scale_tile == 1) ? picture_av1->slice_parameter.slice_data_anchor_frame_idx[tileIdx] : 0xFF;

      debug_printf("[d3d12_video_decoder_av1] Detected tile index %" PRIu32
                  " with DataOffset %" PRIu32
                  " - DataSize %" PRIu32
                  " - row: %" PRIu16
                  " - col: %" PRIu16
                  " - large_scale_tile: %" PRIu32
                  " - anchor_frame_idx: %" PRIu8
                  " for frame with "
                  "fenceValue: %d\n",
                  tileIdx,
                  currentTileEntry.DataOffset,
                  currentTileEntry.DataSize,
                  currentTileEntry.row,
                  currentTileEntry.column,
                  picture_av1->picture_parameter.pic_info_fields.large_scale_tile,
                  currentTileEntry.anchor_frame,
                  pD3D12Dec->m_fenceValue);

      memcpy(pData, &currentTileEntry, sizeof(DXVA_Tile_AV1));
      pData += sizeof(DXVA_Tile_AV1);
   }
   assert(vecOutSliceControlBuffers.size() == totalSlicesDXVAArrayByteSize);
}

DXVA_PicParams_AV1
d3d12_video_decoder_dxva_picparams_from_pipe_picparams_av1(
   uint32_t frameNum,
   pipe_video_profile profile,
   pipe_av1_picture_desc *pipe_av1)
{
   DXVA_PicParams_AV1 dxvaStructure;
   memset(&dxvaStructure, 0, sizeof(dxvaStructure));

   dxvaStructure.width  = pipe_av1->picture_parameter.frame_width;
   dxvaStructure.height = pipe_av1->picture_parameter.frame_height;

   dxvaStructure.max_width  = pipe_av1->picture_parameter.max_width;
   dxvaStructure.max_height = pipe_av1->picture_parameter.max_height;

   uint8_t bit_depths[] = { 8, 10, 12 };

   // dxvaStructure.CurrPicTextureIndex = is set by d3d12_video_decoder_refresh_dpb_active_references_av1
   dxvaStructure.superres_denom      = pipe_av1->picture_parameter.superres_scale_denominator;
   dxvaStructure.bitdepth            = bit_depths[pipe_av1->picture_parameter.bit_depth_idx];
   dxvaStructure.seq_profile         = pipe_av1->picture_parameter.profile;

   /* Tiling info */
   dxvaStructure.tiles.cols = pipe_av1->picture_parameter.tile_cols;
   dxvaStructure.tiles.rows = pipe_av1->picture_parameter.tile_rows;
   dxvaStructure.tiles.context_update_id = pipe_av1->picture_parameter.context_update_tile_id;

   for (uint32_t i = 0; i < dxvaStructure.tiles.cols; i++)
      dxvaStructure.tiles.widths[i] = pipe_av1->picture_parameter.width_in_sbs[i];

   for (uint32_t i = 0; i < dxvaStructure.tiles.rows; i++)
      dxvaStructure.tiles.heights[i] = pipe_av1->picture_parameter.height_in_sbs[i];

   // Fix the last tile rounding when uniform_tile_spacing_flag
   if (pipe_av1->picture_parameter.pic_info_fields.uniform_tile_spacing_flag) {
      uint32_t sbPixSize = pipe_av1->picture_parameter.seq_info_fields.use_128x128_superblock ? 128 : 64;
      if (dxvaStructure.tiles.cols > 1) {
         uint32_t acumSbWMinusLast = 0;
         for (uint32_t i = 0; i < dxvaStructure.tiles.cols - 1u; i++)
            acumSbWMinusLast += dxvaStructure.tiles.widths[i];

         dxvaStructure.tiles.widths[dxvaStructure.tiles.cols-1] =
            std::ceil(pipe_av1->picture_parameter.frame_width / (float)sbPixSize) - acumSbWMinusLast;
      }

      if (dxvaStructure.tiles.rows > 1) {
         uint32_t acumSbHMinusLast = 0;
         for (uint32_t i = 0; i < dxvaStructure.tiles.rows - 1u; i++)
            acumSbHMinusLast += dxvaStructure.tiles.heights[i];

         dxvaStructure.tiles.heights[dxvaStructure.tiles.rows-1] =
            std::ceil(pipe_av1->picture_parameter.frame_width / (float)sbPixSize) - acumSbHMinusLast;
      }
   }

   /* Coding tools */
   dxvaStructure.coding.use_128x128_superblock       = pipe_av1->picture_parameter.seq_info_fields.use_128x128_superblock;
   dxvaStructure.coding.intra_edge_filter            = pipe_av1->picture_parameter.seq_info_fields.enable_intra_edge_filter;
   dxvaStructure.coding.interintra_compound          = pipe_av1->picture_parameter.seq_info_fields.enable_interintra_compound;
   dxvaStructure.coding.masked_compound              = pipe_av1->picture_parameter.seq_info_fields.enable_masked_compound;
   dxvaStructure.coding.warped_motion                = pipe_av1->picture_parameter.pic_info_fields.allow_warped_motion;
   dxvaStructure.coding.dual_filter                  = pipe_av1->picture_parameter.seq_info_fields.enable_dual_filter;
   dxvaStructure.coding.jnt_comp                     = pipe_av1->picture_parameter.seq_info_fields.enable_jnt_comp;
   dxvaStructure.coding.screen_content_tools         = pipe_av1->picture_parameter.pic_info_fields.allow_screen_content_tools;
   dxvaStructure.coding.integer_mv                   = pipe_av1->picture_parameter.pic_info_fields.force_integer_mv || !(pipe_av1->picture_parameter.pic_info_fields.frame_type & 1);
   dxvaStructure.coding.cdef                         = pipe_av1->picture_parameter.seq_info_fields.enable_cdef;
   dxvaStructure.coding.restoration                  = 1; // This indicates the feature MAY be enabled in the sequence
   dxvaStructure.coding.film_grain                   = pipe_av1->picture_parameter.seq_info_fields.film_grain_params_present;
   dxvaStructure.coding.intrabc                      = pipe_av1->picture_parameter.pic_info_fields.allow_intrabc;
   dxvaStructure.coding.high_precision_mv            = pipe_av1->picture_parameter.pic_info_fields.allow_high_precision_mv;
   dxvaStructure.coding.switchable_motion_mode       = pipe_av1->picture_parameter.pic_info_fields.is_motion_mode_switchable;
   dxvaStructure.coding.filter_intra                 = pipe_av1->picture_parameter.seq_info_fields.enable_filter_intra;
   dxvaStructure.coding.disable_frame_end_update_cdf = pipe_av1->picture_parameter.pic_info_fields.disable_frame_end_update_cdf;
   dxvaStructure.coding.disable_cdf_update           = pipe_av1->picture_parameter.pic_info_fields.disable_cdf_update;
   dxvaStructure.coding.reference_mode               = pipe_av1->picture_parameter.mode_control_fields.reference_select;
   dxvaStructure.coding.skip_mode                    = pipe_av1->picture_parameter.mode_control_fields.skip_mode_present;
   dxvaStructure.coding.reduced_tx_set               = pipe_av1->picture_parameter.mode_control_fields.reduced_tx_set_used;
   dxvaStructure.coding.superres                     = pipe_av1->picture_parameter.pic_info_fields.use_superres;
   dxvaStructure.coding.tx_mode                      = pipe_av1->picture_parameter.mode_control_fields.tx_mode;
   dxvaStructure.coding.use_ref_frame_mvs            = pipe_av1->picture_parameter.pic_info_fields.use_ref_frame_mvs;
   dxvaStructure.coding.enable_ref_frame_mvs         = pipe_av1->picture_parameter.seq_info_fields.ref_frame_mvs;
   dxvaStructure.coding.reference_frame_update       = 1;

   /* Format & Picture Info flags */
   dxvaStructure.format.frame_type     = pipe_av1->picture_parameter.pic_info_fields.frame_type;
   dxvaStructure.format.show_frame     = pipe_av1->picture_parameter.pic_info_fields.show_frame;
   dxvaStructure.format.showable_frame = pipe_av1->picture_parameter.pic_info_fields.showable_frame;
   dxvaStructure.format.subsampling_x  = 1; // D3D12 Only Supports 4:2:0 (ie. Profile0)
   dxvaStructure.format.subsampling_y  = 1; // D3D12 Only Supports 4:2:0 (ie. Profile0)
   dxvaStructure.format.mono_chrome    = pipe_av1->picture_parameter.seq_info_fields.mono_chrome;

   /* References */
   dxvaStructure.primary_ref_frame = pipe_av1->picture_parameter.primary_ref_frame;
   dxvaStructure.order_hint        = pipe_av1->picture_parameter.order_hint;
   dxvaStructure.order_hint_bits   = pipe_av1->picture_parameter.seq_info_fields.enable_order_hint ? pipe_av1->picture_parameter.order_hint_bits_minus_1 + 1 : 0;

   // d3d12_video_decoder_refresh_dpb_active_references_av1 will assign the correct Index for
   // RefFrameMapTextureIndex entries where pipe_av1->ref[i] has a surface
   memset(dxvaStructure.RefFrameMapTextureIndex, DXVA_AV1_INVALID_PICTURE_INDEX, sizeof(dxvaStructure.RefFrameMapTextureIndex));
   memset(&dxvaStructure.frame_refs[0], 0, sizeof(dxvaStructure.frame_refs));
   for (uint32_t i = 0; i < _countof(dxvaStructure.frame_refs); i++) {
      debug_printf("libva ref[%d] = %p\n", i, pipe_av1->ref[i]);
      debug_printf("libva ref_frame_idx[%d] = %d\n", i, pipe_av1->picture_parameter.ref_frame_idx[i]);

      uint8_t ref_idx = pipe_av1->picture_parameter.ref_frame_idx[i];
      struct pipe_video_buffer *ref_frame = (ref_idx <= _countof(pipe_av1->ref)) ?  pipe_av1->ref[ref_idx] : NULL;
      if(ref_frame) {
         dxvaStructure.frame_refs[i].width  = ref_frame->width;
         dxvaStructure.frame_refs[i].height = ref_frame->height;
         dxvaStructure.frame_refs[i].Index = ref_idx;

         /* Global Motion */
         dxvaStructure.frame_refs[i].wminvalid = pipe_av1->picture_parameter.wm[i].invalid;
         dxvaStructure.frame_refs[i].wmtype    = pipe_av1->picture_parameter.wm[i].wmtype;
         for (uint32_t j = 0; j < 6; ++j) {
               dxvaStructure.frame_refs[i].wmmat[j] = pipe_av1->picture_parameter.wm[i].wmmat[j];
         }
      }
      else
      {
         dxvaStructure.frame_refs[i].Index = DXVA_AV1_INVALID_PICTURE_INDEX;
      }
   }

   /* Loop filter parameters */
   dxvaStructure.loop_filter.filter_level[0]        = pipe_av1->picture_parameter.filter_level[0];
   dxvaStructure.loop_filter.filter_level[1]        = pipe_av1->picture_parameter.filter_level[1];
   dxvaStructure.loop_filter.filter_level_u         = pipe_av1->picture_parameter.filter_level_u;
   dxvaStructure.loop_filter.filter_level_v         = pipe_av1->picture_parameter.filter_level_v;
   dxvaStructure.loop_filter.sharpness_level        = pipe_av1->picture_parameter.loop_filter_info_fields.sharpness_level;
   dxvaStructure.loop_filter.mode_ref_delta_enabled = pipe_av1->picture_parameter.loop_filter_info_fields.mode_ref_delta_enabled;
   dxvaStructure.loop_filter.mode_ref_delta_update  = pipe_av1->picture_parameter.loop_filter_info_fields.mode_ref_delta_update;
   dxvaStructure.loop_filter.delta_lf_multi         = pipe_av1->picture_parameter.mode_control_fields.delta_lf_multi;
   dxvaStructure.loop_filter.delta_lf_present       = pipe_av1->picture_parameter.mode_control_fields.delta_lf_present_flag;
   if (dxvaStructure.loop_filter.delta_lf_present)
      dxvaStructure.loop_filter.delta_lf_res           = pipe_av1->picture_parameter.mode_control_fields.log2_delta_lf_res; /* no need to convert log2 here */

   for (uint32_t i = 0; i < 8; i++) {
      dxvaStructure.loop_filter.ref_deltas[i] = pipe_av1->picture_parameter.ref_deltas[i];
   }

   dxvaStructure.loop_filter.mode_deltas[0]                = pipe_av1->picture_parameter.mode_deltas[0];
   dxvaStructure.loop_filter.mode_deltas[1]                = pipe_av1->picture_parameter.mode_deltas[1];

   /* VAAPI already receives *frame_restoration_type after the remap_lr_type conversion */
   dxvaStructure.loop_filter.frame_restoration_type[0]     = pipe_av1->picture_parameter.loop_restoration_fields.yframe_restoration_type;
   dxvaStructure.loop_filter.frame_restoration_type[1]     = pipe_av1->picture_parameter.loop_restoration_fields.cbframe_restoration_type;
   dxvaStructure.loop_filter.frame_restoration_type[2]     = pipe_av1->picture_parameter.loop_restoration_fields.crframe_restoration_type;

   bool lr_enabled = dxvaStructure.loop_filter.frame_restoration_type[0] || dxvaStructure.loop_filter.frame_restoration_type[1] || dxvaStructure.loop_filter.frame_restoration_type[2];
   dxvaStructure.loop_filter.log2_restoration_unit_size[0] = lr_enabled ? (6 + pipe_av1->picture_parameter.loop_restoration_fields.lr_unit_shift) : 8;
   dxvaStructure.loop_filter.log2_restoration_unit_size[1] = lr_enabled ? (6 + pipe_av1->picture_parameter.loop_restoration_fields.lr_unit_shift - pipe_av1->picture_parameter.loop_restoration_fields.lr_uv_shift) : 8;
   dxvaStructure.loop_filter.log2_restoration_unit_size[2] = lr_enabled ? (6 + pipe_av1->picture_parameter.loop_restoration_fields.lr_unit_shift - pipe_av1->picture_parameter.loop_restoration_fields.lr_uv_shift) : 8;

   /* Quantization */
   dxvaStructure.quantization.delta_q_present = pipe_av1->picture_parameter.mode_control_fields.delta_q_present_flag;
   if(dxvaStructure.quantization.delta_q_present)
      dxvaStructure.quantization.delta_q_res     = pipe_av1->picture_parameter.mode_control_fields.log2_delta_q_res; /* no need to convert log2 here */
   dxvaStructure.quantization.base_qindex     = pipe_av1->picture_parameter.base_qindex;
   dxvaStructure.quantization.y_dc_delta_q    = pipe_av1->picture_parameter.y_dc_delta_q;
   dxvaStructure.quantization.u_dc_delta_q    = pipe_av1->picture_parameter.u_dc_delta_q;
   dxvaStructure.quantization.v_dc_delta_q    = pipe_av1->picture_parameter.v_dc_delta_q;
   dxvaStructure.quantization.u_ac_delta_q    = pipe_av1->picture_parameter.u_ac_delta_q;
   dxvaStructure.quantization.v_ac_delta_q    = pipe_av1->picture_parameter.v_ac_delta_q;
   
   if(pipe_av1->picture_parameter.qmatrix_fields.using_qmatrix)
   {
      dxvaStructure.quantization.qm_y            = pipe_av1->picture_parameter.qmatrix_fields.qm_y;
      dxvaStructure.quantization.qm_u            = pipe_av1->picture_parameter.qmatrix_fields.qm_u;
      dxvaStructure.quantization.qm_v            = pipe_av1->picture_parameter.qmatrix_fields.qm_v;
   } else {
      dxvaStructure.quantization.qm_y = 0xFF;
      dxvaStructure.quantization.qm_u = 0xFF;
      dxvaStructure.quantization.qm_v = 0xFF;
   }

   /* Cdef parameters */
   dxvaStructure.cdef.damping = pipe_av1->picture_parameter.cdef_damping_minus_3;
   dxvaStructure.cdef.bits    = pipe_av1->picture_parameter.cdef_bits;
   for (uint32_t i = 0; i < 8; i++) {
      dxvaStructure.cdef.y_strengths[i].primary    = (pipe_av1->picture_parameter.cdef_y_strengths[i] >> 2);
      dxvaStructure.cdef.y_strengths[i].secondary  = (pipe_av1->picture_parameter.cdef_y_strengths[i] & 0x03);
      dxvaStructure.cdef.uv_strengths[i].primary   = (pipe_av1->picture_parameter.cdef_uv_strengths[i] >> 2);
      dxvaStructure.cdef.uv_strengths[i].secondary = (pipe_av1->picture_parameter.cdef_uv_strengths[i] & 0x03);
   }

   /* Misc flags */
   dxvaStructure.interp_filter = pipe_av1->picture_parameter.interp_filter;

   /* Segmentation */
   dxvaStructure.segmentation.enabled         = pipe_av1->picture_parameter.seg_info.segment_info_fields.enabled;
   dxvaStructure.segmentation.update_map      = pipe_av1->picture_parameter.seg_info.segment_info_fields.update_map;
   dxvaStructure.segmentation.update_data     = pipe_av1->picture_parameter.seg_info.segment_info_fields.update_data;
   dxvaStructure.segmentation.temporal_update = pipe_av1->picture_parameter.seg_info.segment_info_fields.temporal_update;
   for (uint32_t i = 0; i < 8; i++) {
      dxvaStructure.segmentation.feature_mask[i].mask |= pipe_av1->picture_parameter.seg_info.feature_mask[i];
      for (uint32_t j = 0; j < 8; j++) {
         dxvaStructure.segmentation.feature_data[i][j]    = pipe_av1->picture_parameter.seg_info.feature_data[i][j];
      }
   }

   /* Film grain */
   if (pipe_av1->picture_parameter.film_grain_info.film_grain_info_fields.apply_grain) {
      dxvaStructure.film_grain.apply_grain              = 1;
      dxvaStructure.film_grain.scaling_shift_minus8     = pipe_av1->picture_parameter.film_grain_info.film_grain_info_fields.grain_scaling_minus_8;
      dxvaStructure.film_grain.chroma_scaling_from_luma = pipe_av1->picture_parameter.film_grain_info.film_grain_info_fields.chroma_scaling_from_luma;
      dxvaStructure.film_grain.ar_coeff_lag             = pipe_av1->picture_parameter.film_grain_info.film_grain_info_fields.ar_coeff_lag;
      dxvaStructure.film_grain.ar_coeff_shift_minus6    = pipe_av1->picture_parameter.film_grain_info.film_grain_info_fields.ar_coeff_shift_minus_6;
      dxvaStructure.film_grain.grain_scale_shift        = pipe_av1->picture_parameter.film_grain_info.film_grain_info_fields.grain_scale_shift;
      dxvaStructure.film_grain.overlap_flag             = pipe_av1->picture_parameter.film_grain_info.film_grain_info_fields.overlap_flag;
      dxvaStructure.film_grain.clip_to_restricted_range = pipe_av1->picture_parameter.film_grain_info.film_grain_info_fields.clip_to_restricted_range;

      /* order of coefficients is actually GBR, also IEC 61966-2-1 (sRGB) */
      constexpr uint8_t AVCOL_SPC_RGB = 0;
      dxvaStructure.film_grain.matrix_coeff_is_identity = (pipe_av1->picture_parameter.matrix_coefficients == AVCOL_SPC_RGB);

      dxvaStructure.film_grain.grain_seed               = pipe_av1->picture_parameter.film_grain_info.grain_seed;
      dxvaStructure.film_grain.num_y_points             = pipe_av1->picture_parameter.film_grain_info.num_y_points;
      for (uint32_t i = 0; i < pipe_av1->picture_parameter.film_grain_info.num_y_points; i++) {
         dxvaStructure.film_grain.scaling_points_y[i][0] = pipe_av1->picture_parameter.film_grain_info.point_y_value[i];
         dxvaStructure.film_grain.scaling_points_y[i][1] = pipe_av1->picture_parameter.film_grain_info.point_y_scaling[i];
      }
      dxvaStructure.film_grain.num_cb_points            = pipe_av1->picture_parameter.film_grain_info.num_cb_points;
      for (uint32_t i = 0; i < pipe_av1->picture_parameter.film_grain_info.num_cb_points; i++) {
         dxvaStructure.film_grain.scaling_points_cb[i][0] = pipe_av1->picture_parameter.film_grain_info.point_cb_value[i];
         dxvaStructure.film_grain.scaling_points_cb[i][1] = pipe_av1->picture_parameter.film_grain_info.point_cb_scaling[i];
      }
      dxvaStructure.film_grain.num_cr_points            = pipe_av1->picture_parameter.film_grain_info.num_cr_points;
      for (uint32_t i = 0; i < pipe_av1->picture_parameter.film_grain_info.num_cr_points; i++) {
         dxvaStructure.film_grain.scaling_points_cr[i][0] = pipe_av1->picture_parameter.film_grain_info.point_cr_value[i];
         dxvaStructure.film_grain.scaling_points_cr[i][1] = pipe_av1->picture_parameter.film_grain_info.point_cr_scaling[i];
      }
      for (uint32_t i = 0; i < 24; i++) {
         dxvaStructure.film_grain.ar_coeffs_y[i] = pipe_av1->picture_parameter.film_grain_info.ar_coeffs_y[i];
      }
      for (uint32_t i = 0; i < 25; i++) {
         dxvaStructure.film_grain.ar_coeffs_cb[i] = pipe_av1->picture_parameter.film_grain_info.ar_coeffs_cb[i];
         dxvaStructure.film_grain.ar_coeffs_cr[i] = pipe_av1->picture_parameter.film_grain_info.ar_coeffs_cr[i];
      }
      dxvaStructure.film_grain.cb_mult      = pipe_av1->picture_parameter.film_grain_info.cb_mult;
      dxvaStructure.film_grain.cb_luma_mult = pipe_av1->picture_parameter.film_grain_info.cb_luma_mult;
      dxvaStructure.film_grain.cr_mult      = pipe_av1->picture_parameter.film_grain_info.cr_mult;
      dxvaStructure.film_grain.cr_luma_mult = pipe_av1->picture_parameter.film_grain_info.cr_luma_mult;
      dxvaStructure.film_grain.cb_offset    = pipe_av1->picture_parameter.film_grain_info.cb_offset;
      dxvaStructure.film_grain.cr_offset    = pipe_av1->picture_parameter.film_grain_info.cr_offset;
      dxvaStructure.film_grain.cr_offset    = pipe_av1->picture_parameter.film_grain_info.cr_offset;
   }

   dxvaStructure.StatusReportFeedbackNumber = frameNum;
   assert(dxvaStructure.StatusReportFeedbackNumber > 0);
   return dxvaStructure;
}
