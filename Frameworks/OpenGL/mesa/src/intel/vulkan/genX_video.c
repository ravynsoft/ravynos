/*
 * Copyright © 2021 Red Hat
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "anv_private.h"

#include "genxml/gen_macros.h"
#include "genxml/genX_pack.h"

#include "util/vl_zscan_data.h"

void
genX(CmdBeginVideoCodingKHR)(VkCommandBuffer commandBuffer,
                             const VkVideoBeginCodingInfoKHR *pBeginInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);
   ANV_FROM_HANDLE(anv_video_session, vid, pBeginInfo->videoSession);
   ANV_FROM_HANDLE(anv_video_session_params, params, pBeginInfo->videoSessionParameters);

   cmd_buffer->video.vid = vid;
   cmd_buffer->video.params = params;
}

void
genX(CmdControlVideoCodingKHR)(VkCommandBuffer commandBuffer,
                               const VkVideoCodingControlInfoKHR *pCodingControlInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   if (pCodingControlInfo->flags & VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR) {
      anv_batch_emit(&cmd_buffer->batch, GENX(MI_FLUSH_DW), flush) {
         flush.VideoPipelineCacheInvalidate = 1;
      }
   }
}

void
genX(CmdEndVideoCodingKHR)(VkCommandBuffer commandBuffer,
                           const VkVideoEndCodingInfoKHR *pEndCodingInfo)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   cmd_buffer->video.vid = NULL;
   cmd_buffer->video.params = NULL;
}

static void
scaling_list(struct anv_cmd_buffer *cmd_buffer,
             const StdVideoH265ScalingLists *scaling_list)
{
   /* 4x4, 8x8, 16x16, 32x32 */
   for (uint8_t size = 0; size < 4; size++) {
      /* Intra, Inter */
      for (uint8_t pred = 0; pred < 2; pred++) {
         /* Y, Cb, Cr */
         for (uint8_t color = 0; color < 3; color++) {
            if (size == 3 && color > 0)
               continue;

            anv_batch_emit(&cmd_buffer->batch, GENX(HCP_QM_STATE), qm) {
               qm.SizeID = size;
               qm.PredictionType = pred;
               qm.ColorComponent = color;

               qm.DCCoefficient = size > 1 ?
                  (size == 2 ? scaling_list->ScalingListDCCoef16x16[3 * pred + color] :
                               scaling_list->ScalingListDCCoef32x32[pred]) : 0;

               if (size == 0) {
                  for (uint8_t i = 0; i < 4; i++)
                     for (uint8_t j = 0; j < 4; j++)
                        qm.QuantizerMatrix8x8[4 * i + j] =
                           scaling_list->ScalingList4x4[3 * pred + color][4 * i + j];
               } else if (size == 1) {
                  for (uint8_t i = 0; i < 8; i++)
                     for (uint8_t j = 0; j < 8; j++)
                        qm.QuantizerMatrix8x8[8 * i + j] =
                           scaling_list->ScalingList8x8[3 * pred + color][8 * i + j];
               } else if (size == 2) {
                  for (uint8_t i = 0; i < 8; i++)
                     for (uint8_t j = 0; j < 8; j++)
                        qm.QuantizerMatrix8x8[8 * i + j] =
                           scaling_list->ScalingList16x16[3 * pred + color][8 * i + j];
               } else if (size == 3) {
                  for (uint8_t i = 0; i < 8; i++)
                     for (uint8_t j = 0; j < 8; j++)
                        qm.QuantizerMatrix8x8[8 * i + j] =
                           scaling_list->ScalingList32x32[pred][8 * i + j];
               }
            }
         }
      }
   }
}

static void
anv_h265_decode_video(struct anv_cmd_buffer *cmd_buffer,
                      const VkVideoDecodeInfoKHR *frame_info)
{
   ANV_FROM_HANDLE(anv_buffer, src_buffer, frame_info->srcBuffer);
   struct anv_video_session *vid = cmd_buffer->video.vid;
   struct anv_video_session_params *params = cmd_buffer->video.params;

   const struct VkVideoDecodeH265PictureInfoKHR *h265_pic_info =
      vk_find_struct_const(frame_info->pNext, VIDEO_DECODE_H265_PICTURE_INFO_KHR);

   const StdVideoH265SequenceParameterSet *sps =
      vk_video_find_h265_dec_std_sps(&params->vk, h265_pic_info->pStdPictureInfo->pps_seq_parameter_set_id);
   const StdVideoH265PictureParameterSet *pps =
      vk_video_find_h265_dec_std_pps(&params->vk, h265_pic_info->pStdPictureInfo->pps_pic_parameter_set_id);

   struct vk_video_h265_reference ref_slots[2][8] = { 0 };
   uint8_t dpb_idx[ANV_VIDEO_H265_MAX_NUM_REF_FRAME] = { 0,};
   bool is_10bit = sps->bit_depth_chroma_minus8 || sps->bit_depth_luma_minus8;

   anv_batch_emit(&cmd_buffer->batch, GENX(MI_FLUSH_DW), flush) {
      flush.VideoPipelineCacheInvalidate = 1;
   };

#if GFX_VER >= 12
   anv_batch_emit(&cmd_buffer->batch, GENX(MI_FORCE_WAKEUP), wake) {
      wake.HEVCPowerWellControl = 1;
      wake.MaskBits = 768;
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(VD_CONTROL_STATE), cs) {
      cs.PipelineInitialization = true;
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_WAIT), mfx) {
      mfx.MFXSyncControlFlag = 1;
   }
#endif

   anv_batch_emit(&cmd_buffer->batch, GENX(HCP_PIPE_MODE_SELECT), sel) {
      sel.CodecSelect = Decode;
      sel.CodecStandardSelect = HEVC;
   }

#if GFX_VER >= 12
   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_WAIT), mfx) {
      mfx.MFXSyncControlFlag = 1;
   }
#endif

   const struct anv_image_view *iv =
      anv_image_view_from_handle(frame_info->dstPictureResource.imageViewBinding);
   const struct anv_image *img = iv->image;

   anv_batch_emit(&cmd_buffer->batch, GENX(HCP_SURFACE_STATE), ss) {
      ss.SurfacePitch = img->planes[0].primary_surface.isl.row_pitch_B - 1;
      ss.SurfaceID = HCP_CurrentDecodedPicture;
      ss.SurfaceFormat = is_10bit ? P010 : PLANAR_420_8;

      ss.YOffsetforUCb = img->planes[1].primary_surface.memory_range.offset /
         img->planes[0].primary_surface.isl.row_pitch_B;

#if GFX_VER >= 11
      ss.DefaultAlphaValue = 0xffff;
#endif
   }

#if GFX_VER >= 12
   /* Seems to need to set same states to ref as decode on gen12 */
   anv_batch_emit(&cmd_buffer->batch, GENX(HCP_SURFACE_STATE), ss) {
      ss.SurfacePitch = img->planes[0].primary_surface.isl.row_pitch_B - 1;
      ss.SurfaceID = HCP_ReferencePicture;
      ss.SurfaceFormat = is_10bit ? P010 : PLANAR_420_8;

      ss.YOffsetforUCb = img->planes[1].primary_surface.memory_range.offset /
         img->planes[0].primary_surface.isl.row_pitch_B;

      ss.DefaultAlphaValue = 0xffff;
   }
#endif

   anv_batch_emit(&cmd_buffer->batch, GENX(HCP_PIPE_BUF_ADDR_STATE), buf) {
      buf.DecodedPictureAddress =
         anv_image_address(img, &img->planes[0].primary_surface.memory_range);

      buf.DecodedPictureMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.DecodedPictureAddress.bo, 0),
      };

      buf.DeblockingFilterLineBufferAddress = (struct anv_address) {
         vid->vid_mem[ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_LINE].mem->bo,
         vid->vid_mem[ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_LINE].offset
      };

      buf.DeblockingFilterLineBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.DeblockingFilterLineBufferAddress.bo, 0),
      };

      buf.DeblockingFilterTileLineBufferAddress = (struct anv_address) {
         vid->vid_mem[ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_TILE_LINE].mem->bo,
         vid->vid_mem[ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_TILE_LINE].offset
      };

      buf.DeblockingFilterTileLineBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.DeblockingFilterTileLineBufferAddress.bo, 0),
      };

      buf.DeblockingFilterTileColumnBufferAddress = (struct anv_address) {
         vid->vid_mem[ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_TILE_COLUMN].mem->bo,
         vid->vid_mem[ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_TILE_COLUMN].offset
      };

      buf.DeblockingFilterTileColumnBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.DeblockingFilterTileColumnBufferAddress.bo, 0),
      };

      buf.MetadataLineBufferAddress = (struct anv_address) {
         vid->vid_mem[ANV_VID_MEM_H265_METADATA_LINE].mem->bo,
         vid->vid_mem[ANV_VID_MEM_H265_METADATA_LINE].offset
      };

      buf.MetadataLineBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.MetadataLineBufferAddress.bo, 0),
      };

      buf.MetadataTileLineBufferAddress = (struct anv_address) {
         vid->vid_mem[ANV_VID_MEM_H265_METADATA_TILE_LINE].mem->bo,
         vid->vid_mem[ANV_VID_MEM_H265_METADATA_TILE_LINE].offset
      };

      buf.MetadataTileLineBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.MetadataTileLineBufferAddress.bo, 0),
      };

      buf.MetadataTileColumnBufferAddress = (struct anv_address) {
         vid->vid_mem[ANV_VID_MEM_H265_METADATA_TILE_COLUMN].mem->bo,
         vid->vid_mem[ANV_VID_MEM_H265_METADATA_TILE_COLUMN].offset
      };

      buf.MetadataTileColumnBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.MetadataTileColumnBufferAddress.bo, 0),
      };

      buf.SAOLineBufferAddress = (struct anv_address) {
         vid->vid_mem[ANV_VID_MEM_H265_SAO_LINE].mem->bo,
         vid->vid_mem[ANV_VID_MEM_H265_SAO_LINE].offset
      };

      buf.SAOLineBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.SAOLineBufferAddress.bo, 0),
      };

      buf.SAOTileLineBufferAddress = (struct anv_address) {
         vid->vid_mem[ANV_VID_MEM_H265_SAO_TILE_LINE].mem->bo,
         vid->vid_mem[ANV_VID_MEM_H265_SAO_TILE_LINE].offset
      };

      buf.SAOTileLineBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.SAOTileLineBufferAddress.bo, 0),
      };

      buf.SAOTileColumnBufferAddress = (struct anv_address) {
         vid->vid_mem[ANV_VID_MEM_H265_SAO_TILE_COLUMN].mem->bo,
         vid->vid_mem[ANV_VID_MEM_H265_SAO_TILE_COLUMN].offset
      };

      buf.SAOTileColumnBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.SAOTileColumnBufferAddress.bo, 0),
      };

      buf.CurrentMVTemporalBufferAddress = anv_image_address(img, &img->vid_dmv_top_surface);

      buf.CurrentMVTemporalBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.CurrentMVTemporalBufferAddress.bo, 0),
      };

      for (unsigned i = 0; i < frame_info->referenceSlotCount; i++) {
         const struct anv_image_view *ref_iv =
            anv_image_view_from_handle(frame_info->pReferenceSlots[i].pPictureResource->imageViewBinding);
         int slot_idx = frame_info->pReferenceSlots[i].slotIndex;

         assert(slot_idx < ANV_VIDEO_H265_MAX_NUM_REF_FRAME);
         dpb_idx[slot_idx] = i;

         buf.ReferencePictureAddress[i] =
            anv_image_address(ref_iv->image, &ref_iv->image->planes[0].primary_surface.memory_range);
      }

      buf.ReferencePictureMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };

      buf.OriginalUncompressedPictureSourceMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };

      buf.StreamOutDataDestinationMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };

      buf.DecodedPictureStatusBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };

      buf.LCUILDBStreamOutBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };

      for (unsigned i = 0; i < frame_info->referenceSlotCount; i++) {
         const struct anv_image_view *ref_iv =
            anv_image_view_from_handle(frame_info->pReferenceSlots[i].pPictureResource->imageViewBinding);

         buf.CollocatedMVTemporalBufferAddress[i] =
            anv_image_address(ref_iv->image, &ref_iv->image->vid_dmv_top_surface);
      }

      buf.CollocatedMVTemporalBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.CollocatedMVTemporalBufferAddress[0].bo, 0),
      };

      buf.VP9ProbabilityBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };

      buf.VP9SegmentIDBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };

      buf.VP9HVDLineRowStoreBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };

      buf.VP9HVDTileRowStoreBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
#if GFX_VER >= 11
      buf.SAOStreamOutDataDestinationBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      buf.FrameStatisticsStreamOutDataDestinationBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      buf.SSESourcePixelRowStoreBufferMemoryAddressAttributesReadWrite = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      buf.HCPScalabilitySliceStateBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      buf.HCPScalabilityCABACDecodedSyntaxElementsBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      buf.MVUpperRightColumnStoreBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      buf.IntraPredictionUpperRightColumnStoreBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      buf.IntraPredictionLeftReconColumnStoreBufferMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
#endif
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(HCP_IND_OBJ_BASE_ADDR_STATE), indirect) {
      indirect.HCPIndirectBitstreamObjectBaseAddress =
         anv_address_add(src_buffer->address, frame_info->srcBufferOffset & ~4095);

      indirect.HCPIndirectBitstreamObjectMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, src_buffer->address.bo, 0),
      };

      indirect.HCPIndirectBitstreamObjectAccessUpperBound =
         anv_address_add(src_buffer->address, align64(frame_info->srcBufferRange, 4096));

      indirect.HCPIndirectCUObjectMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };

      indirect.HCPPAKBSEObjectMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };

#if GFX_VER >= 11
      indirect.HCPVP9PAKCompressedHeaderSyntaxStreamInMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      indirect.HCPVP9PAKProbabilityCounterStreamOutMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      indirect.HCPVP9PAKProbabilityDeltasStreamInMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      indirect.HCPVP9PAKTileRecordStreamOutMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      indirect.HCPVP9PAKCULevelStatisticStreamOutMemoryAddressAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
#endif
   }

   if (sps->flags.scaling_list_enabled_flag) {
      if (pps->flags.pps_scaling_list_data_present_flag) {
         scaling_list(cmd_buffer, pps->pScalingLists);
      } else if (sps->flags.sps_scaling_list_data_present_flag) {
         scaling_list(cmd_buffer, sps->pScalingLists);
      }
   } else {
      for (uint8_t size = 0; size < 4; size++) {
         for (uint8_t pred = 0; pred < 2; pred++) {
            for (uint8_t color = 0; color < 3; color++) {

               if (size == 3 && color > 0)
                  continue;

               anv_batch_emit(&cmd_buffer->batch, GENX(HCP_QM_STATE), qm) {
                  qm.SizeID = size;
                  qm.PredictionType = pred;
                  qm.ColorComponent = color;
                  qm.DCCoefficient = (size > 1) ? 16 : 0;
                  unsigned len = (size == 0) ? 16 : 64;

                  for (uint8_t q = 0; q < len; q++)
                     qm.QuantizerMatrix8x8[q] = 0x10;
               }
            }
         }
      }
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(HCP_PIC_STATE), pic) {
      pic.FrameWidthInMinimumCodingBlockSize =
         sps->pic_width_in_luma_samples / (1 << (sps->log2_min_luma_coding_block_size_minus3 + 3)) - 1;
      pic.FrameHeightInMinimumCodingBlockSize =
         sps->pic_height_in_luma_samples / (1 << (sps->log2_min_luma_coding_block_size_minus3 + 3))  - 1;

      pic.MinCUSize = sps->log2_min_luma_coding_block_size_minus3 & 0x3;
      pic.LCUSize = (sps->log2_diff_max_min_luma_coding_block_size +
                     sps->log2_min_luma_coding_block_size_minus3) & 0x3;

      pic.MinTUSize = sps->log2_min_luma_transform_block_size_minus2 & 0x3;
      pic.MaxTUSize = (sps->log2_diff_max_min_luma_transform_block_size + sps->log2_min_luma_transform_block_size_minus2) & 0x3;
      pic.MinPCMSize = sps->log2_min_pcm_luma_coding_block_size_minus3 & 0x3;
      pic.MaxPCMSize = (sps->log2_diff_max_min_pcm_luma_coding_block_size + sps->log2_min_pcm_luma_coding_block_size_minus3) & 0x3;

#if GFX_VER >= 11
      pic.Log2SAOOffsetScaleLuma = pps->log2_sao_offset_scale_luma;
      pic.Log2SAOOffsetScaleChroma = pps->log2_sao_offset_scale_chroma;
      pic.ChromaQPOffsetListLength = pps->chroma_qp_offset_list_len_minus1;
      pic.DiffCUChromaQPOffsetDepth = pps->diff_cu_chroma_qp_offset_depth;
      pic.ChromaQPOffsetListEnable = pps->flags.chroma_qp_offset_list_enabled_flag;
      pic.ChromaSubsampling = sps->chroma_format_idc;

      pic.HighPrecisionOffsetsEnable = sps->flags.high_precision_offsets_enabled_flag;
      pic.Log2MaxTransformSkipSize = pps->log2_max_transform_skip_block_size_minus2 + 2;
      pic.CrossComponentPredictionEnable = pps->flags.cross_component_prediction_enabled_flag;
      pic.CABACBypassAlignmentEnable = sps->flags.cabac_bypass_alignment_enabled_flag;
      pic.PersistentRiceAdaptationEnable = sps->flags.persistent_rice_adaptation_enabled_flag;
      pic.IntraSmoothingDisable = sps->flags.intra_smoothing_disabled_flag;
      pic.ExplicitRDPCMEnable = sps->flags.explicit_rdpcm_enabled_flag;
      pic.ImplicitRDPCMEnable = sps->flags.implicit_rdpcm_enabled_flag;
      pic.TransformSkipContextEnable = sps->flags.transform_skip_context_enabled_flag;
      pic.TransformSkipRotationEnable = sps->flags.transform_skip_rotation_enabled_flag;
      pic.SPSRangeExtensionEnable = sps->flags.sps_range_extension_flag;
#endif

      pic.CollocatedPictureIsISlice = false;
      pic.CurrentPictureIsISlice = false;
      pic.SampleAdaptiveOffsetEnable = sps->flags.sample_adaptive_offset_enabled_flag;
      pic.PCMEnable = sps->flags.pcm_enabled_flag;
      pic.CUQPDeltaEnable = pps->flags.cu_qp_delta_enabled_flag;
      pic.MaxDQPDepth = pps->diff_cu_qp_delta_depth;
      pic.PCMLoopFilterDisable = sps->flags.pcm_loop_filter_disabled_flag;
      pic.ConstrainedIntraPrediction = pps->flags.constrained_intra_pred_flag;
      pic.Log2ParallelMergeLevel = pps->log2_parallel_merge_level_minus2;
      pic.SignDataHiding = pps->flags.sign_data_hiding_enabled_flag;
      pic.LoopFilterEnable = pps->flags.loop_filter_across_tiles_enabled_flag;
      pic.EntropyCodingSyncEnable = pps->flags.entropy_coding_sync_enabled_flag;
      pic.TilingEnable = pps->flags.tiles_enabled_flag;
      pic.WeightedBiPredicationEnable = pps->flags.weighted_bipred_flag;
      pic.WeightedPredicationEnable = pps->flags.weighted_pred_flag;
      pic.FieldPic = 0;
      pic.TopField = true;
      pic.TransformSkipEnable = pps->flags.transform_skip_enabled_flag;
      pic.AMPEnable = sps->flags.amp_enabled_flag;
      pic.TransquantBypassEnable = pps->flags.transquant_bypass_enabled_flag;
      pic.StrongIntraSmoothingEnable = sps->flags.strong_intra_smoothing_enabled_flag;
      pic.CUPacketStructure = 0;

      pic.PictureCbQPOffset = pps->pps_cb_qp_offset;
      pic.PictureCrQPOffset = pps->pps_cr_qp_offset;
      pic.IntraMaxTransformHierarchyDepth = sps->max_transform_hierarchy_depth_intra;
      pic.InterMaxTransformHierarchyDepth = sps->max_transform_hierarchy_depth_inter;
      pic.ChromaPCMSampleBitDepth = sps->pcm_sample_bit_depth_chroma_minus1 & 0xf;
      pic.LumaPCMSampleBitDepth = sps->pcm_sample_bit_depth_luma_minus1 & 0xf;

      pic.ChromaBitDepth = sps->bit_depth_chroma_minus8;
      pic.LumaBitDepth = sps->bit_depth_luma_minus8;

#if GFX_VER >= 11
      pic.CbQPOffsetList0 = pps->cb_qp_offset_list[0];
      pic.CbQPOffsetList1 = pps->cb_qp_offset_list[1];
      pic.CbQPOffsetList2 = pps->cb_qp_offset_list[2];
      pic.CbQPOffsetList3 = pps->cb_qp_offset_list[3];
      pic.CbQPOffsetList4 = pps->cb_qp_offset_list[4];
      pic.CbQPOffsetList5 = pps->cb_qp_offset_list[5];

      pic.CrQPOffsetList0 = pps->cr_qp_offset_list[0];
      pic.CrQPOffsetList1 = pps->cr_qp_offset_list[1];
      pic.CrQPOffsetList2 = pps->cr_qp_offset_list[2];
      pic.CrQPOffsetList3 = pps->cr_qp_offset_list[3];
      pic.CrQPOffsetList4 = pps->cr_qp_offset_list[4];
      pic.CrQPOffsetList5 = pps->cr_qp_offset_list[5];
#endif
   }

   if (pps->flags.tiles_enabled_flag) {
      int cum = 0;
      anv_batch_emit(&cmd_buffer->batch, GENX(HCP_TILE_STATE), tile) {
         tile.NumberofTileColumns = pps->num_tile_columns_minus1;
         tile.NumberofTileRows = pps->num_tile_rows_minus1;
         for (unsigned i = 0; i < 5; i++) {
            tile.ColumnPosition[i].CtbPos0i = cum;
            if ((4 * i) == pps->num_tile_columns_minus1)
               break;

            cum += pps->column_width_minus1[4 * i] + 1;
            tile.ColumnPosition[i].CtbPos1i = cum;

            if ((4 * i + 1) == pps->num_tile_columns_minus1)
               break;
            cum += pps->column_width_minus1[4 * i + 1] + 1;
            tile.ColumnPosition[i].CtbPos2i = cum;

            if ((4 * i + 2) == pps->num_tile_columns_minus1)
               break;
            cum += pps->column_width_minus1[4 * i + 2] + 1;
            tile.ColumnPosition[i].CtbPos3i = cum;

            if ((4 * i + 3) >= MIN2(pps->num_tile_columns_minus1,
                                    ARRAY_SIZE(pps->column_width_minus1)))
               break;

            cum += pps->column_width_minus1[4 * i + 3] + 1;
         }

         cum = 0;

         for (unsigned i = 0; i < 5; i++) {
            tile.Rowposition[i].CtbPos0i = cum;
            if ((4 * i) == pps->num_tile_rows_minus1)
               break;

            cum += pps->row_height_minus1[4 * i] + 1;
            tile.Rowposition[i].CtbPos1i = cum;

            if ((4 * i + 1) == pps->num_tile_rows_minus1)
               break;
            cum += pps->row_height_minus1[4 * i + 1] + 1;
            tile.Rowposition[i].CtbPos2i = cum;

            if ((4 * i + 2) == pps->num_tile_rows_minus1)
               break;
            cum += pps->row_height_minus1[4 * i + 2] + 1;
            tile.Rowposition[i].CtbPos3i = cum;

            if ((4 * i + 3) == pps->num_tile_rows_minus1)
               break;

            cum += pps->row_height_minus1[4 * i + 3] + 1;
         }

         if (pps->num_tile_rows_minus1 == 20) {
            tile.Rowposition[5].CtbPos0i = cum;
         }
         if (pps->num_tile_rows_minus1 == 20) {
            tile.Rowposition[5].CtbPos0i = cum;
            cum += pps->row_height_minus1[20] + 1;
            tile.Rowposition[5].CtbPos1i = cum;
         }
      }
   }

   /* Slice parsing */
   uint32_t last_slice = h265_pic_info->sliceSegmentCount - 1;
   void *slice_map = anv_gem_mmap(cmd_buffer->device, src_buffer->address.bo,
                                  src_buffer->address.offset, frame_info->srcBufferRange);

   struct vk_video_h265_slice_params slice_params[h265_pic_info->sliceSegmentCount];

   /* All slices should be parsed in advance to collect information necessary */
   for (unsigned s = 0; s < h265_pic_info->sliceSegmentCount; s++) {
      uint32_t current_offset = h265_pic_info->pSliceSegmentOffsets[s];
      void *map = slice_map + current_offset;
      uint32_t slice_size = 0;

      if (s == last_slice)
         slice_size = frame_info->srcBufferRange - current_offset;
      else
         slice_size = h265_pic_info->pSliceSegmentOffsets[s + 1] - current_offset;

      vk_video_parse_h265_slice_header(frame_info, h265_pic_info, sps, pps, map, slice_size, &slice_params[s]);
      vk_fill_video_h265_reference_info(frame_info, h265_pic_info, &slice_params[s], ref_slots);
   }

   anv_gem_munmap(cmd_buffer->device, slice_map, frame_info->srcBufferRange);

   for (unsigned s = 0; s < h265_pic_info->sliceSegmentCount; s++) {
      uint32_t ctb_size = 1 << (sps->log2_diff_max_min_luma_coding_block_size +
          sps->log2_min_luma_coding_block_size_minus3 + 3);
      uint32_t pic_width_in_min_cbs_y = sps->pic_width_in_luma_samples /
         (1 << (sps->log2_min_luma_coding_block_size_minus3 + 3));
      uint32_t width_in_pix = (1 << (sps->log2_min_luma_coding_block_size_minus3 + 3)) *
         pic_width_in_min_cbs_y;
      uint32_t ctb_w = DIV_ROUND_UP(width_in_pix, ctb_size);
      bool is_last = (s == last_slice);
      int slice_qp = (slice_params[s].slice_qp_delta + pps->init_qp_minus26 + 26) & 0x3f;

      anv_batch_emit(&cmd_buffer->batch, GENX(HCP_SLICE_STATE), slice) {
         slice.SliceHorizontalPosition = slice_params[s].slice_segment_address % ctb_w;
         slice.SliceVerticalPosition = slice_params[s].slice_segment_address / ctb_w;

         if (is_last) {
            slice.NextSliceHorizontalPosition = 0;
            slice.NextSliceVerticalPosition = 0;
         } else {
            slice.NextSliceHorizontalPosition = (slice_params[s + 1].slice_segment_address) % ctb_w;
            slice.NextSliceVerticalPosition = (slice_params[s + 1].slice_segment_address) / ctb_w;
         }

         slice.SliceType = slice_params[s].slice_type;
         slice.LastSlice = is_last;
         slice.DependentSlice = slice_params[s].dependent_slice_segment;
         slice.SliceTemporalMVPEnable = slice_params[s].temporal_mvp_enable;
         slice.SliceQP = abs(slice_qp);
         slice.SliceQPSign = slice_qp >= 0 ? 0 : 1;
         slice.SliceCbQPOffset = slice_params[s].slice_cb_qp_offset;
         slice.SliceCrQPOffset = slice_params[s].slice_cr_qp_offset;
         slice.SliceHeaderDisableDeblockingFilter = pps->flags.deblocking_filter_override_enabled_flag ?
               slice_params[s].disable_deblocking_filter_idc : pps->flags.pps_deblocking_filter_disabled_flag;
         slice.SliceTCOffsetDiv2 = slice_params[s].tc_offset_div2;
         slice.SliceBetaOffsetDiv2 = slice_params[s].beta_offset_div2;
         slice.SliceLoopFilterEnable = slice_params[s].loop_filter_across_slices_enable;
         slice.SliceSAOChroma = slice_params[s].sao_chroma_flag;
         slice.SliceSAOLuma = slice_params[s].sao_luma_flag;
         slice.MVDL1Zero = slice_params[s].mvd_l1_zero_flag;

         uint8_t low_delay = true;

         if (slice_params[s].slice_type == STD_VIDEO_H265_SLICE_TYPE_I) {
            low_delay = false;
         } else {
            for (unsigned i = 0; i < slice_params[s].num_ref_idx_l0_active; i++) {
               int slot_idx = ref_slots[0][i].slot_index;

               if (vk_video_h265_poc_by_slot(frame_info, slot_idx) >
                     h265_pic_info->pStdPictureInfo->PicOrderCntVal) {
                  low_delay = false;
                  break;
               }
            }

            for (unsigned i = 0; i < slice_params[s].num_ref_idx_l1_active; i++) {
               int slot_idx = ref_slots[1][i].slot_index;
               if (vk_video_h265_poc_by_slot(frame_info, slot_idx) >
                     h265_pic_info->pStdPictureInfo->PicOrderCntVal) {
                  low_delay = false;
                  break;
               }
            }
         }

         slice.LowDelay = low_delay;
         slice.CollocatedFromL0 = slice_params[s].collocated_list == 0 ? true : false;
         slice.Log2WeightDenominatorChroma = slice_params[s].luma_log2_weight_denom +
            (slice_params[s].chroma_log2_weight_denom - slice_params[s].luma_log2_weight_denom);
         slice.Log2WeightDenominatorLuma = slice_params[s].luma_log2_weight_denom;
         slice.CABACInit = slice_params[s].cabac_init_idc;
         slice.MaxMergeIndex = slice_params[s].max_num_merge_cand - 1;
         slice.CollocatedMVTemporalBufferIndex =
            dpb_idx[ref_slots[slice_params[s].collocated_list][slice_params[s].collocated_ref_idx].slot_index];
         assert(slice.CollocatedMVTemporalBufferIndex < ANV_VIDEO_H265_HCP_NUM_REF_FRAME);

         slice.SliceHeaderLength = slice_params[s].slice_data_bytes_offset;
         slice.CABACZeroWordInsertionEnable = false;
         slice.EmulationByteSliceInsertEnable = false;
         slice.TailInsertionPresent = false;
         slice.SliceDataInsertionPresent = false;
         slice.HeaderInsertionPresent = false;

         slice.IndirectPAKBSEDataStartOffset = 0;
         slice.TransformSkipLambda = 0;
         slice.TransformSkipNumberofNonZeroCoeffsFactor0 = 0;
         slice.TransformSkipNumberofZeroCoeffsFactor0 = 0;
         slice.TransformSkipNumberofNonZeroCoeffsFactor1 = 0;
         slice.TransformSkipNumberofZeroCoeffsFactor1 = 0;

#if GFX_VER >= 12
         slice.OriginalSliceStartCtbX = slice_params[s].slice_segment_address % ctb_w;
         slice.OriginalSliceStartCtbY = slice_params[s].slice_segment_address / ctb_w;
#endif
      }

      if (slice_params[s].slice_type != STD_VIDEO_H265_SLICE_TYPE_I) {
         anv_batch_emit(&cmd_buffer->batch, GENX(HCP_REF_IDX_STATE), ref) {
            ref.ReferencePictureListSelect = 0;
            ref.NumberofReferenceIndexesActive = slice_params[s].num_ref_idx_l0_active - 1;

            for (unsigned i = 0; i < ref.NumberofReferenceIndexesActive + 1; i++) {
               int slot_idx = ref_slots[0][i].slot_index;
               unsigned poc = ref_slots[0][i].pic_order_cnt;
               int32_t diff_poc = h265_pic_info->pStdPictureInfo->PicOrderCntVal - poc;

               assert(dpb_idx[slot_idx] < ANV_VIDEO_H265_HCP_NUM_REF_FRAME);

               ref.ReferenceListEntry[i].ListEntry = dpb_idx[slot_idx];
               ref.ReferenceListEntry[i].ReferencePicturetbValue = CLAMP(diff_poc, -128, 127) & 0xff;
               ref.ReferenceListEntry[i].TopField = true;
            }
         }
      }

      if (slice_params[s].slice_type == STD_VIDEO_H265_SLICE_TYPE_B) {
         anv_batch_emit(&cmd_buffer->batch, GENX(HCP_REF_IDX_STATE), ref) {
            ref.ReferencePictureListSelect = 1;
            ref.NumberofReferenceIndexesActive = slice_params[s].num_ref_idx_l1_active - 1;

            for (unsigned i = 0; i < ref.NumberofReferenceIndexesActive + 1; i++) {
               int slot_idx = ref_slots[1][i].slot_index;;
               unsigned poc = ref_slots[1][i].pic_order_cnt;
               int32_t diff_poc = h265_pic_info->pStdPictureInfo->PicOrderCntVal - poc;

               assert(dpb_idx[slot_idx] < ANV_VIDEO_H265_HCP_NUM_REF_FRAME);

               ref.ReferenceListEntry[i].ListEntry = dpb_idx[slot_idx];
               ref.ReferenceListEntry[i].ReferencePicturetbValue = CLAMP(diff_poc, -128, 127) & 0xff;
               ref.ReferenceListEntry[i].TopField = true;
            }
         }
      }

      if ((pps->flags.weighted_pred_flag && (slice_params[s].slice_type == STD_VIDEO_H265_SLICE_TYPE_P)) ||
            (pps->flags.weighted_bipred_flag && (slice_params[s].slice_type == STD_VIDEO_H265_SLICE_TYPE_B))) {
         anv_batch_emit(&cmd_buffer->batch, GENX(HCP_WEIGHTOFFSET_STATE), w) {
            w.ReferencePictureListSelect = 0;

            for (unsigned i = 0; i < ANV_VIDEO_H265_MAX_NUM_REF_FRAME; i++) {
               w.LumaOffsets[i].DeltaLumaWeightLX = slice_params[s].delta_luma_weight_l0[i] & 0xff;
               w.LumaOffsets[i].LumaOffsetLX = slice_params[s].luma_offset_l0[i] & 0xff;
               w.ChromaOffsets[i].DeltaChromaWeightLX0 = slice_params[s].delta_chroma_weight_l0[i][0] & 0xff;
               w.ChromaOffsets[i].ChromaOffsetLX0 = slice_params[s].chroma_offset_l0[i][0] & 0xff;
               w.ChromaOffsets[i].DeltaChromaWeightLX1 = slice_params[s].delta_chroma_weight_l0[i][1] & 0xff;
               w.ChromaOffsets[i].ChromaOffsetLX1 = slice_params[s].chroma_offset_l0[i][1] & 0xff;
            }
         }

         if (slice_params[s].slice_type == STD_VIDEO_H265_SLICE_TYPE_B) {
            anv_batch_emit(&cmd_buffer->batch, GENX(HCP_WEIGHTOFFSET_STATE), w) {
               w.ReferencePictureListSelect = 1;

               for (unsigned i = 0; i < ANV_VIDEO_H265_MAX_NUM_REF_FRAME; i++) {
                  w.LumaOffsets[i].DeltaLumaWeightLX = slice_params[s].delta_luma_weight_l1[i] & 0xff;
                  w.LumaOffsets[i].LumaOffsetLX = slice_params[s].luma_offset_l1[i] & 0xff;
                  w.ChromaOffsets[i].DeltaChromaWeightLX0 = slice_params[s].delta_chroma_weight_l1[i][0] & 0xff;
                  w.ChromaOffsets[i].DeltaChromaWeightLX1 = slice_params[s].delta_chroma_weight_l1[i][1] & 0xff;
                  w.ChromaOffsets[i].ChromaOffsetLX0 = slice_params[s].chroma_offset_l1[i][0] & 0xff;
                  w.ChromaOffsets[i].ChromaOffsetLX1 = slice_params[s].chroma_offset_l1[i][1] & 0xff;
               }
            }
         }
      }

      uint32_t buffer_offset = frame_info->srcBufferOffset & 4095;

      anv_batch_emit(&cmd_buffer->batch, GENX(HCP_BSD_OBJECT), bsd) {
         bsd.IndirectBSDDataLength = slice_params[s].slice_size - 3;
         bsd.IndirectBSDDataStartAddress = buffer_offset + h265_pic_info->pSliceSegmentOffsets[s] + 3;
      }
   }

#if GFX_VER >= 12
   anv_batch_emit(&cmd_buffer->batch, GENX(VD_CONTROL_STATE), cs) {
      cs.MemoryImplicitFlush = true;
   }
#endif

   anv_batch_emit(&cmd_buffer->batch, GENX(VD_PIPELINE_FLUSH), flush) {
      flush.HEVCPipelineDone = true;
      flush.HEVCPipelineCommandFlush = true;
      flush.VDCommandMessageParserDone = true;
   }
}

static void
anv_h264_decode_video(struct anv_cmd_buffer *cmd_buffer,
                      const VkVideoDecodeInfoKHR *frame_info)
{
   ANV_FROM_HANDLE(anv_buffer, src_buffer, frame_info->srcBuffer);
   struct anv_video_session *vid = cmd_buffer->video.vid;
   struct anv_video_session_params *params = cmd_buffer->video.params;
   const struct VkVideoDecodeH264PictureInfoKHR *h264_pic_info =
      vk_find_struct_const(frame_info->pNext, VIDEO_DECODE_H264_PICTURE_INFO_KHR);
   const StdVideoH264SequenceParameterSet *sps = vk_video_find_h264_dec_std_sps(&params->vk, h264_pic_info->pStdPictureInfo->seq_parameter_set_id);
   const StdVideoH264PictureParameterSet *pps = vk_video_find_h264_dec_std_pps(&params->vk, h264_pic_info->pStdPictureInfo->pic_parameter_set_id);

   anv_batch_emit(&cmd_buffer->batch, GENX(MI_FLUSH_DW), flush) {
      flush.DWordLength = 2;
      flush.VideoPipelineCacheInvalidate = 1;
   };

#if GFX_VER >= 12
   anv_batch_emit(&cmd_buffer->batch, GENX(MI_FORCE_WAKEUP), wake) {
      wake.MFXPowerWellControl = 1;
      wake.MaskBits = 768;
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_WAIT), mfx) {
      mfx.MFXSyncControlFlag = 1;
   }
#endif

   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_PIPE_MODE_SELECT), sel) {
      sel.StandardSelect = SS_AVC;
      sel.CodecSelect = Decode;
      sel.DecoderShortFormatMode = ShortFormatDriverInterface;
      sel.DecoderModeSelect = VLDMode; // Hardcoded

      sel.PreDeblockingOutputEnable = 0;
      sel.PostDeblockingOutputEnable = 1;
   }

#if GFX_VER >= 12
   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_WAIT), mfx) {
      mfx.MFXSyncControlFlag = 1;
   }
#endif

   const struct anv_image_view *iv = anv_image_view_from_handle(frame_info->dstPictureResource.imageViewBinding);
   const struct anv_image *img = iv->image;
   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_SURFACE_STATE), ss) {
      ss.Width = img->vk.extent.width - 1;
      ss.Height = img->vk.extent.height - 1;
      ss.SurfaceFormat = PLANAR_420_8; // assert on this?
      ss.InterleaveChroma = 1;
      ss.SurfacePitch = img->planes[0].primary_surface.isl.row_pitch_B - 1;
      ss.TiledSurface = img->planes[0].primary_surface.isl.tiling != ISL_TILING_LINEAR;
      ss.TileWalk = TW_YMAJOR;

      ss.YOffsetforUCb = ss.YOffsetforVCr =
         img->planes[1].primary_surface.memory_range.offset / img->planes[0].primary_surface.isl.row_pitch_B;
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_PIPE_BUF_ADDR_STATE), buf) {
      bool use_pre_deblock = false;
      if (use_pre_deblock) {
         buf.PreDeblockingDestinationAddress = anv_image_address(img,
                                                                 &img->planes[0].primary_surface.memory_range);
      } else {
         buf.PostDeblockingDestinationAddress = anv_image_address(img,
                                                                  &img->planes[0].primary_surface.memory_range);
      }
      buf.PreDeblockingDestinationAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.PreDeblockingDestinationAddress.bo, 0),
      };
      buf.PostDeblockingDestinationAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.PostDeblockingDestinationAddress.bo, 0),
      };

      buf.IntraRowStoreScratchBufferAddress = (struct anv_address) { vid->vid_mem[ANV_VID_MEM_H264_INTRA_ROW_STORE].mem->bo, vid->vid_mem[ANV_VID_MEM_H264_INTRA_ROW_STORE].offset };
      buf.IntraRowStoreScratchBufferAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.IntraRowStoreScratchBufferAddress.bo, 0),
      };
      buf.DeblockingFilterRowStoreScratchAddress = (struct anv_address) { vid->vid_mem[ANV_VID_MEM_H264_DEBLOCK_FILTER_ROW_STORE].mem->bo, vid->vid_mem[ANV_VID_MEM_H264_DEBLOCK_FILTER_ROW_STORE].offset };
      buf.DeblockingFilterRowStoreScratchAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, buf.DeblockingFilterRowStoreScratchAddress.bo, 0),
      };
      buf.MBStatusBufferAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      buf.MBILDBStreamOutBufferAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      buf.SecondMBILDBStreamOutBufferAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      buf.ScaledReferenceSurfaceAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      buf.OriginalUncompressedPictureSourceAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      buf.StreamOutDataDestinationAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };

      struct anv_bo *ref_bo = NULL;
      for (unsigned i = 0; i < frame_info->referenceSlotCount; i++) {
         const struct anv_image_view *ref_iv = anv_image_view_from_handle(frame_info->pReferenceSlots[i].pPictureResource->imageViewBinding);
         int idx = frame_info->pReferenceSlots[i].slotIndex;
         buf.ReferencePictureAddress[idx] = anv_image_address(ref_iv->image,
                                                              &ref_iv->image->planes[0].primary_surface.memory_range);

         if (i == 0) {
            ref_bo = ref_iv->image->bindings[0].address.bo;
         }
      }
      buf.ReferencePictureAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, ref_bo, 0),
      };
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_IND_OBJ_BASE_ADDR_STATE), index_obj) {
      index_obj.MFXIndirectBitstreamObjectAddress = anv_address_add(src_buffer->address,
                                                                    frame_info->srcBufferOffset & ~4095);
      index_obj.MFXIndirectBitstreamObjectAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, src_buffer->address.bo, 0),
      };
      index_obj.MFXIndirectMVObjectAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      index_obj.MFDIndirectITCOEFFObjectAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      index_obj.MFDIndirectITDBLKObjectAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
      index_obj.MFCIndirectPAKBSEObjectAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_BSP_BUF_BASE_ADDR_STATE), bsp) {
      bsp.BSDMPCRowStoreScratchBufferAddress = (struct anv_address) { vid->vid_mem[ANV_VID_MEM_H264_BSD_MPC_ROW_SCRATCH].mem->bo,
         vid->vid_mem[ANV_VID_MEM_H264_BSD_MPC_ROW_SCRATCH].offset };

      bsp.BSDMPCRowStoreScratchBufferAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, bsp.BSDMPCRowStoreScratchBufferAddress.bo, 0),
      };
      bsp.MPRRowStoreScratchBufferAddress = (struct anv_address) { vid->vid_mem[ANV_VID_MEM_H264_MPR_ROW_SCRATCH].mem->bo,
         vid->vid_mem[ANV_VID_MEM_H264_BSD_MPC_ROW_SCRATCH].offset };

      bsp.MPRRowStoreScratchBufferAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, bsp.MPRRowStoreScratchBufferAddress.bo, 0),
      };
      bsp.BitplaneReadBufferAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, NULL, 0),
      };
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(MFD_AVC_DPB_STATE), avc_dpb) {
      for (unsigned i = 0; i < frame_info->referenceSlotCount; i++) {
         const struct VkVideoDecodeH264DpbSlotInfoKHR *dpb_slot =
            vk_find_struct_const(frame_info->pReferenceSlots[i].pNext, VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR);
         const StdVideoDecodeH264ReferenceInfo *ref_info = dpb_slot->pStdReferenceInfo;
         int idx = frame_info->pReferenceSlots[i].slotIndex;
         avc_dpb.NonExistingFrame[idx] = ref_info->flags.is_non_existing;
         avc_dpb.LongTermFrame[idx] = ref_info->flags.used_for_long_term_reference;
         if (!ref_info->flags.top_field_flag && !ref_info->flags.bottom_field_flag)
            avc_dpb.UsedforReference[idx] = 3;
         else
            avc_dpb.UsedforReference[idx] = ref_info->flags.top_field_flag | (ref_info->flags.bottom_field_flag << 1);
         avc_dpb.LTSTFrameNumberList[idx] = ref_info->FrameNum;
      }
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(MFD_AVC_PICID_STATE), picid) {
      picid.PictureIDRemappingDisable = true;
   }

   uint32_t pic_height = sps->pic_height_in_map_units_minus1 + 1;
   if (!sps->flags.frame_mbs_only_flag)
      pic_height *= 2;
   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_AVC_IMG_STATE), avc_img) {
      avc_img.FrameWidth = sps->pic_width_in_mbs_minus1;
      avc_img.FrameHeight = pic_height - 1;
      avc_img.FrameSize = (sps->pic_width_in_mbs_minus1 + 1) * pic_height;

      if (!h264_pic_info->pStdPictureInfo->flags.field_pic_flag)
         avc_img.ImageStructure = FramePicture;
      else if (h264_pic_info->pStdPictureInfo->flags.bottom_field_flag)
         avc_img.ImageStructure = BottomFieldPicture;
      else
         avc_img.ImageStructure = TopFieldPicture;

      avc_img.WeightedBiPredictionIDC = pps->weighted_bipred_idc;
      avc_img.WeightedPredictionEnable = pps->flags.weighted_pred_flag;
      avc_img.FirstChromaQPOffset = pps->chroma_qp_index_offset;
      avc_img.SecondChromaQPOffset = pps->second_chroma_qp_index_offset;
      avc_img.FieldPicture = h264_pic_info->pStdPictureInfo->flags.field_pic_flag;
      avc_img.MBAFFMode = (sps->flags.mb_adaptive_frame_field_flag &&
                           !h264_pic_info->pStdPictureInfo->flags.field_pic_flag);
      avc_img.FrameMBOnly = sps->flags.frame_mbs_only_flag;
      avc_img._8x8IDCTTransformMode = pps->flags.transform_8x8_mode_flag;
      avc_img.Direct8x8Inference = sps->flags.direct_8x8_inference_flag;
      avc_img.ConstrainedIntraPrediction = pps->flags.constrained_intra_pred_flag;
      avc_img.NonReferencePicture = !h264_pic_info->pStdPictureInfo->flags.is_reference;
      avc_img.EntropyCodingSyncEnable = pps->flags.entropy_coding_mode_flag;
      avc_img.ChromaFormatIDC = sps->chroma_format_idc;
      avc_img.TrellisQuantizationChromaDisable = true;
      avc_img.NumberofReferenceFrames = frame_info->referenceSlotCount;
      avc_img.NumberofActiveReferencePicturesfromL0 = pps->num_ref_idx_l0_default_active_minus1 + 1;
      avc_img.NumberofActiveReferencePicturesfromL1 = pps->num_ref_idx_l1_default_active_minus1 + 1;
      avc_img.InitialQPValue = pps->pic_init_qp_minus26;
      avc_img.PicOrderPresent = pps->flags.bottom_field_pic_order_in_frame_present_flag;
      avc_img.DeltaPicOrderAlwaysZero = sps->flags.delta_pic_order_always_zero_flag;
      avc_img.PicOrderCountType = sps->pic_order_cnt_type;
      avc_img.DeblockingFilterControlPresent = pps->flags.deblocking_filter_control_present_flag;
      avc_img.RedundantPicCountPresent = pps->flags.redundant_pic_cnt_present_flag;
      avc_img.Log2MaxFrameNumber = sps->log2_max_frame_num_minus4;
      avc_img.Log2MaxPicOrderCountLSB = sps->log2_max_pic_order_cnt_lsb_minus4;
      avc_img.CurrentPictureFrameNumber = h264_pic_info->pStdPictureInfo->frame_num;
   }

   StdVideoH264ScalingLists scaling_lists;
   vk_video_derive_h264_scaling_list(sps, pps, &scaling_lists);
   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_QM_STATE), qm) {
      qm.DWordLength = 16;
      qm.AVC = AVC_4x4_Intra_MATRIX;
      for (unsigned m = 0; m < 3; m++)
         for (unsigned q = 0; q < 16; q++)
            qm.ForwardQuantizerMatrix[m * 16 + vl_zscan_normal_16[q]] = scaling_lists.ScalingList4x4[m][q];
   }
   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_QM_STATE), qm) {
      qm.DWordLength = 16;
      qm.AVC = AVC_4x4_Inter_MATRIX;
      for (unsigned m = 0; m < 3; m++)
         for (unsigned q = 0; q < 16; q++)
            qm.ForwardQuantizerMatrix[m * 16 + vl_zscan_normal_16[q]] = scaling_lists.ScalingList4x4[m + 3][q];
   }
   if (pps->flags.transform_8x8_mode_flag) {
      anv_batch_emit(&cmd_buffer->batch, GENX(MFX_QM_STATE), qm) {
         qm.DWordLength = 16;
         qm.AVC = AVC_8x8_Intra_MATRIX;
         for (unsigned q = 0; q < 64; q++)
            qm.ForwardQuantizerMatrix[vl_zscan_normal[q]] = scaling_lists.ScalingList8x8[0][q];
      }
      anv_batch_emit(&cmd_buffer->batch, GENX(MFX_QM_STATE), qm) {
         qm.DWordLength = 16;
         qm.AVC = AVC_8x8_Inter_MATRIX;
         for (unsigned q = 0; q < 64; q++)
            qm.ForwardQuantizerMatrix[vl_zscan_normal[q]] = scaling_lists.ScalingList8x8[1][q];
      }
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(MFX_AVC_DIRECTMODE_STATE), avc_directmode) {
      /* bind reference frame DMV */
      struct anv_bo *dmv_bo = NULL;
      for (unsigned i = 0; i < frame_info->referenceSlotCount; i++) {
         int idx = frame_info->pReferenceSlots[i].slotIndex;
         const struct VkVideoDecodeH264DpbSlotInfoKHR *dpb_slot =
            vk_find_struct_const(frame_info->pReferenceSlots[i].pNext, VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR);
         const struct anv_image_view *ref_iv = anv_image_view_from_handle(frame_info->pReferenceSlots[i].pPictureResource->imageViewBinding);
         const StdVideoDecodeH264ReferenceInfo *ref_info = dpb_slot->pStdReferenceInfo;
         avc_directmode.DirectMVBufferAddress[idx] = anv_image_address(ref_iv->image,
                                                                     &ref_iv->image->vid_dmv_top_surface);
         if (i == 0) {
            dmv_bo = ref_iv->image->bindings[0].address.bo;
         }
         avc_directmode.POCList[2 * idx] = ref_info->PicOrderCnt[0];
         avc_directmode.POCList[2 * idx + 1] = ref_info->PicOrderCnt[1];
      }
      avc_directmode.DirectMVBufferAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, dmv_bo, 0),
      };

      avc_directmode.DirectMVBufferWriteAddress = anv_image_address(img,
                                                                    &img->vid_dmv_top_surface);
      avc_directmode.DirectMVBufferWriteAttributes = (struct GENX(MEMORYADDRESSATTRIBUTES)) {
         .MOCS = anv_mocs(cmd_buffer->device, img->bindings[0].address.bo, 0),
      };
      avc_directmode.POCList[32] = h264_pic_info->pStdPictureInfo->PicOrderCnt[0];
      avc_directmode.POCList[33] = h264_pic_info->pStdPictureInfo->PicOrderCnt[1];
   }

   uint32_t buffer_offset = frame_info->srcBufferOffset & 4095;
#define HEADER_OFFSET 3
   for (unsigned s = 0; s < h264_pic_info->sliceCount; s++) {
      bool last_slice = s == (h264_pic_info->sliceCount - 1);
      uint32_t current_offset = h264_pic_info->pSliceOffsets[s];
      uint32_t this_end;
      if (!last_slice) {
         uint32_t next_offset = h264_pic_info->pSliceOffsets[s + 1];
         uint32_t next_end = h264_pic_info->pSliceOffsets[s + 2];
         if (s == h264_pic_info->sliceCount - 2)
            next_end = frame_info->srcBufferRange;
         anv_batch_emit(&cmd_buffer->batch, GENX(MFD_AVC_SLICEADDR), sliceaddr) {
            sliceaddr.IndirectBSDDataLength = next_end - next_offset - HEADER_OFFSET;
            /* start decoding after the 3-byte header. */
            sliceaddr.IndirectBSDDataStartAddress = buffer_offset + next_offset + HEADER_OFFSET;
         };
         this_end = next_offset;
      } else
         this_end = frame_info->srcBufferRange;
      anv_batch_emit(&cmd_buffer->batch, GENX(MFD_AVC_BSD_OBJECT), avc_bsd) {
         avc_bsd.IndirectBSDDataLength = this_end - current_offset - HEADER_OFFSET;
         /* start decoding after the 3-byte header. */
         avc_bsd.IndirectBSDDataStartAddress = buffer_offset + current_offset + HEADER_OFFSET;
         avc_bsd.InlineData.LastSlice = last_slice;
         avc_bsd.InlineData.FixPrevMBSkipped = 1;
         avc_bsd.InlineData.IntraPredictionErrorControl = 1;
         avc_bsd.InlineData.Intra8x84x4PredictionErrorConcealmentControl = 1;
         avc_bsd.InlineData.ISliceConcealmentMode = 1;
      };
   }
}

void
genX(CmdDecodeVideoKHR)(VkCommandBuffer commandBuffer,
                        const VkVideoDecodeInfoKHR *frame_info)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, cmd_buffer, commandBuffer);

   switch (cmd_buffer->video.vid->vk.op) {
   case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
      anv_h264_decode_video(cmd_buffer, frame_info);
      break;
   case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
      anv_h265_decode_video(cmd_buffer, frame_info);
      break;
   default:
      assert(0);
   }
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
void
genX(CmdEncodeVideoKHR)(VkCommandBuffer commandBuffer,
                        const VkVideoEncodeInfoKHR *pEncodeInfo)
{
}
#endif
