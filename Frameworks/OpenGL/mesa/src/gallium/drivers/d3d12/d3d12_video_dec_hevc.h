
/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of uint8_tge, to any person obtaining a
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

#ifndef D3D12_VIDEO_DEC_HEVC_H
#define D3D12_VIDEO_DEC_HEVC_H

#include "d3d12_video_types.h"

// From DXVA spec regarding DXVA_PicEntry_HEVC entries:
// Entries that will not be used for decoding the current picture, or any subsequent pictures,
// are indicated by setting bPicEntry to 0xFF.
// If bPicEntry is not 0xFF, the entry may be used as a reference surface for decoding the current picture or
// a subsequent picture (in decoding order).
constexpr uint16_t DXVA_HEVC_INVALID_PICTURE_INDEX =
   0x7F;   // This corresponds to DXVA_PicEntry_HEVC.Index7Bits ; Not to be confused with the invalid value for
           // DXVA_PicEntry_HEVC.bPicEntry full uint8_t value
constexpr uint16_t DXVA_HEVC_INVALID_PICTURE_ENTRY_VALUE = 0xFF;   // This corresponds to DXVA_PicEntry_HEVC.bPicEntry

constexpr unsigned int DXVA_HEVC_START_CODE          = 0x000001;   // 3 byte start code
constexpr unsigned int DXVA_HEVC_START_CODE_LEN_BITS = 24;         // 3 byte start code
#define DXVA_RPS_COUNT 8

/* HEVC picture entry data structure */
/* If including new DXVA structs in this header, check the byte-alignment packing pragma declarations that need to be included with them */
#pragma pack(push, BeforeDXVApacking, 1)
typedef struct _DXVA_PicEntry_HEVC
{
   union
   {
      struct
      {
         uint8_t Index7Bits : 7;
         uint8_t AssociatedFlag : 1;
      };
      uint8_t bPicEntry;
   };
} DXVA_PicEntry_HEVC, *LPDXVA_PicEntry_HEVC; /* 1 byte */
#pragma pack(pop, BeforeDXVApacking)

/* HEVC picture parameters structure */
/* If including new DXVA structs in this header, check the byte-alignment packing pragma declarations that need to be included with them */
#pragma pack(push, BeforeDXVApacking, 1)
/* HEVC Picture Parameter structure */
typedef struct _DXVA_PicParams_HEVC {
    uint16_t      PicWidthInMinCbsY;
    uint16_t      PicHeightInMinCbsY;
    union {
        struct {
            uint16_t  chroma_format_idc                       : 2;
            uint16_t  separate_colour_plane_flag              : 1;
            uint16_t  bit_depth_luma_minus8                   : 3; 
            uint16_t  bit_depth_chroma_minus8                 : 3;
            uint16_t  log2_max_pic_order_cnt_lsb_minus4       : 4;
            uint16_t  NoPicReorderingFlag                     : 1;
            uint16_t  NoBiPredFlag                            : 1;
            uint16_t  ReservedBits1                            : 1;
        };
        uint16_t wFormatAndSequenceInfoFlags;
    };
    DXVA_PicEntry_HEVC  CurrPic;
    uint8_t   sps_max_dec_pic_buffering_minus1;
    uint8_t   log2_min_luma_coding_block_size_minus3;
    uint8_t   log2_diff_max_min_luma_coding_block_size;
    uint8_t   log2_min_transform_block_size_minus2;
    uint8_t   log2_diff_max_min_transform_block_size;
    uint8_t   max_transform_hierarchy_depth_inter;
    uint8_t   max_transform_hierarchy_depth_intra;
    uint8_t   num_short_term_ref_pic_sets;
    uint8_t   num_long_term_ref_pics_sps;
    uint8_t   num_ref_idx_l0_default_active_minus1;
    uint8_t   num_ref_idx_l1_default_active_minus1;
    uint8_t    init_qp_minus26;
    uint8_t   ucNumDeltaPocsOfRefRpsIdx;
    uint16_t  wNumBitsForShortTermRPSInSlice;
    uint16_t  ReservedBits2;

    union {
        struct {
            uint32_t  scaling_list_enabled_flag                    : 1;
            uint32_t  amp_enabled_flag                            : 1;
            uint32_t  sample_adaptive_offset_enabled_flag         : 1;
            uint32_t  pcm_enabled_flag                            : 1;
            uint32_t  pcm_sample_bit_depth_luma_minus1            : 4;
            uint32_t  pcm_sample_bit_depth_chroma_minus1          : 4;
            uint32_t  log2_min_pcm_luma_coding_block_size_minus3  : 2;
            uint32_t  log2_diff_max_min_pcm_luma_coding_block_size : 2;
            uint32_t  pcm_loop_filter_disabled_flag                : 1;
            uint32_t  long_term_ref_pics_present_flag             : 1;
            uint32_t  sps_temporal_mvp_enabled_flag               : 1;
            uint32_t  strong_intra_smoothing_enabled_flag         : 1;
            uint32_t  dependent_slice_segments_enabled_flag       : 1;
            uint32_t  output_flag_present_flag                    : 1;
            uint32_t  num_extra_slice_header_bits                 : 3;
            uint32_t  sign_data_hiding_enabled_flag               : 1;
            uint32_t  cabac_init_present_flag                     : 1;
            uint32_t  ReservedBits3                               : 5;
        };
        uint32_t dwCodingParamToolFlags;
    };

    union {
        struct {
            uint32_t  constrained_intra_pred_flag                 : 1;
            uint32_t  transform_skip_enabled_flag                 : 1;
            uint32_t  cu_qp_delta_enabled_flag                    : 1;
            uint32_t  pps_slice_chroma_qp_offsets_present_flag    : 1;
            uint32_t  weighted_pred_flag                          : 1;
            uint32_t  weighted_bipred_flag                        : 1;
            uint32_t  transquant_bypass_enabled_flag              : 1;
            uint32_t  tiles_enabled_flag                          : 1;
            uint32_t  entropy_coding_sync_enabled_flag            : 1;
            uint32_t  uniform_spacing_flag                        : 1;
            uint32_t  loop_filter_across_tiles_enabled_flag       : 1;
            uint32_t  pps_loop_filter_across_slices_enabled_flag  : 1;
            uint32_t  deblocking_filter_override_enabled_flag     : 1;
            uint32_t  pps_deblocking_filter_disabled_flag         : 1;
            uint32_t  lists_modification_present_flag             : 1;
            uint32_t  slice_segment_header_extension_present_flag : 1;
            uint32_t  IrapPicFlag                                 : 1;
            uint32_t  IdrPicFlag                                  : 1;
            uint32_t  IntraPicFlag                                : 1;
            uint32_t  ReservedBits4                               : 13;
        };
        uint32_t dwCodingSettingPicturePropertyFlags;
    };
    uint8_t    pps_cb_qp_offset;
    uint8_t    pps_cr_qp_offset;
    uint8_t   num_tile_columns_minus1;
    uint8_t   num_tile_rows_minus1;
    uint16_t  column_width_minus1[19];
    uint16_t  row_height_minus1[21];
    uint8_t   diff_cu_qp_delta_depth;
    uint8_t    pps_beta_offset_div2;
    uint8_t    pps_tc_offset_div2;
    uint8_t   log2_parallel_merge_level_minus2;
    int32_t     CurrPicOrderCntVal;
    DXVA_PicEntry_HEVC	RefPicList[15];
    uint8_t   ReservedBits5;
    int32_t     PicOrderCntValList[15];
    uint8_t   RefPicSetStCurrBefore[8];
    uint8_t   RefPicSetStCurrAfter[8];
    uint8_t   RefPicSetLtCurr[8];
    uint16_t  ReservedBits6;
    uint16_t  ReservedBits7;
    uint32_t    StatusReportFeedbackNumber;
} DXVA_PicParams_HEVC, *LPDXVA_PicParams_HEVC;
#pragma pack(pop, BeforeDXVApacking)

static_assert(_countof(DXVA_PicParams_HEVC::RefPicSetStCurrBefore) == DXVA_RPS_COUNT, 
    "DXVA_PicParams_HEVC::RefPicSetStCurrBefore must have DXVA_RPS_COUNT elements");
static_assert(_countof(DXVA_PicParams_HEVC::RefPicSetStCurrAfter) == DXVA_RPS_COUNT, 
    "DXVA_PicParams_HEVC::RefPicSetStCurrAfter must have DXVA_RPS_COUNT elements");
static_assert(_countof(DXVA_PicParams_HEVC::RefPicSetLtCurr) == DXVA_RPS_COUNT, 
    "DXVA_PicParams_HEVC::RefPicSetLtCurr must have DXVA_RPS_COUNT elements");

/* HEVC Quantization Matrix structure */
/* If including new DXVA structs in this header, check the byte-alignment packing pragma declarations that need to be included with them */
#pragma pack(push, BeforeDXVApacking, 1)
typedef struct _DXVA_Qmatrix_HEVC 
{
    uint8_t ucScalingLists0[6][16]; 
    uint8_t ucScalingLists1[6][64]; 
    uint8_t ucScalingLists2[6][64]; 
    uint8_t ucScalingLists3[2][64]; 
    uint8_t ucScalingListDCCoefSizeID2[6];
    uint8_t ucScalingListDCCoefSizeID3[2];
} DXVA_Qmatrix_HEVC, *LPDXVA_Qmatrix_HEVC;
#pragma pack(pop, BeforeDXVApacking)

/* HEVC slice control data structure - short form */
/* If including new DXVA structs in this header, check the byte-alignment packing pragma declarations that need to be included with them */
#pragma pack(push, BeforeDXVApacking, 1)
typedef struct _DXVA_Slice_HEVC_Short
{
   uint32_t BSNALunitDataLocation; /* type 1..5 */
   uint32_t SliceBytesInBuffer;    /* for off-host parse */
   uint16_t wBadSliceChopping;     /* for off-host parse */
} DXVA_Slice_HEVC_Short, *LPDXVA_Slice_HEVC_Short;
#pragma pack(pop, BeforeDXVApacking)

#pragma pack(push, BeforeDXVApacking, 1)
typedef struct _DXVA_Status_HEVC {
    uint16_t StatusReportFeedbackNumber;
    DXVA_PicEntry_HEVC CurrPic;
    uint8_t  bBufType;
    uint8_t  bStatus;
    uint8_t  bReserved8Bits;
    uint16_t wNumMbsAffected;
} DXVA_Status_HEVC, *LPDXVA_Status_HEVC;
#pragma pack(pop, BeforeDXVApacking)

DXVA_PicParams_HEVC
d3d12_video_decoder_dxva_picparams_from_pipe_picparams_hevc(struct d3d12_video_decoder *pD3D12Dec,
                                                            pipe_video_profile          profile,
                                                            pipe_h265_picture_desc *    pipeDesc);
void
d3d12_video_decoder_get_frame_info_hevc(
   struct d3d12_video_decoder *pD3D12Dec, uint32_t *pWidth, uint32_t *pHeight, uint16_t *pMaxDPB, bool &isInterlaced);
void
d3d12_video_decoder_prepare_current_frame_references_hevc(struct d3d12_video_decoder *pD3D12Dec,
                                                          ID3D12Resource *            pTexture2D,
                                                          uint32_t                    subresourceIndex);
void
d3d12_video_decoder_dxva_qmatrix_from_pipe_picparams_hevc(pipe_h265_picture_desc *pPipeDesc,
                                                          DXVA_Qmatrix_HEVC &     outMatrixBuffer,
                                                          bool &outScalingListEnabled);
void
d3d12_video_decoder_refresh_dpb_active_references_hevc(struct d3d12_video_decoder *pD3D12Dec);
bool
d3d12_video_decoder_get_next_slice_size_and_offset_hevc(std::vector<uint8_t> &buf,
                                                   unsigned int          bufferOffset,
                                                   uint32_t &            outSliceSize,
                                                   uint32_t &            outSliceOffset);

uint 
d3d12_video_decoder_get_slice_count_hevc(std::vector<uint8_t> &buf);

void
d3d12_video_decoder_prepare_dxva_slices_control_hevc(struct d3d12_video_decoder *        pD3D12Dec,
                                                     std::vector<uint8_t> &vecOutSliceControlBuffers,
                                                     struct pipe_h265_picture_desc* picture_hevc);

void
d3d12_video_decoder_log_pic_params_hevc(DXVA_PicParams_HEVC * pPicParams);

#endif
