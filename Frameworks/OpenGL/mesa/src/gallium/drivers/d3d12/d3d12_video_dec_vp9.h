
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

#ifndef D3D12_VIDEO_DEC_VP9_H
#define D3D12_VIDEO_DEC_VP9_H

#include "d3d12_video_types.h"

constexpr uint16_t DXVA_VP9_INVALID_PICTURE_INDEX = 0x7F;
constexpr uint16_t DXVA_VP9_INVALID_PICTURE_ENTRY = 0xFF;

#pragma pack(push, BeforeDXVApacking, 1)

/* VPx picture entry data structure */
typedef struct _DXVA_PicEntry_VPx {
    union {
        struct {
            uint8_t Index7Bits : 7;
            uint8_t AssociatedFlag : 1;
        };
        uint8_t bPicEntry;
    };
} DXVA_PicEntry_VPx, *LPDXVA_PicEntry_VPx;

/* VP9 segmentation structure */
typedef struct _segmentation_VP9 {
    union {
        struct {
            uint8_t enabled : 1;
            uint8_t update_map : 1;
            uint8_t temporal_update : 1;
            uint8_t abs_delta : 1;
            uint8_t ReservedSegmentFlags4Bits : 4;
        };
        uint8_t wSegmentInfoFlags;
    };
    uint8_t tree_probs[7];
    uint8_t pred_probs[3];
    int16_t feature_data[8][4];
    uint8_t feature_mask[8];
} DXVA_segmentation_VP9;

/* VP9 picture parameters structure */
typedef struct _DXVA_PicParams_VP9 {
    DXVA_PicEntry_VPx    CurrPic;
    uint8_t                profile;
    union {
        struct {
            uint16_t frame_type : 1;
            uint16_t show_frame : 1;
            uint16_t error_resilient_mode : 1;
            uint16_t subsampling_x : 1;
            uint16_t subsampling_y : 1;
            uint16_t extra_plane : 1;
            uint16_t refresh_frame_context : 1;
            uint16_t frame_parallel_decoding_mode : 1;
            uint16_t intra_only : 1;
            uint16_t frame_context_idx : 2;
            uint16_t reset_frame_context : 2;
            uint16_t allow_high_precision_mv : 1;
            uint16_t ReservedFormatInfo2Bits : 2;
        };
        uint16_t wFormatAndPictureInfoFlags;
    };
    uint32_t  width;
    uint32_t  height;
    uint8_t BitDepthMinus8Luma;
    uint8_t BitDepthMinus8Chroma;
    uint8_t interp_filter;
    uint8_t Reserved8Bits;
    DXVA_PicEntry_VPx  ref_frame_map[8];
    uint32_t  ref_frame_coded_width[8];
    uint32_t  ref_frame_coded_height[8];
    DXVA_PicEntry_VPx  frame_refs[3];
    char  ref_frame_sign_bias[4];
    char  filter_level;
    char  sharpness_level;
    union {
        struct {
            uint8_t mode_ref_delta_enabled : 1;
            uint8_t mode_ref_delta_update : 1;
            uint8_t use_prev_in_find_mv_refs : 1;
            uint8_t ReservedControlInfo5Bits : 5;
        };
        uint8_t wControlInfoFlags;
    };
    char   ref_deltas[4];
    char   mode_deltas[2];
    int16_t  base_qindex;
    char   y_dc_delta_q;
    char   uv_dc_delta_q;
    char   uv_ac_delta_q;
    DXVA_segmentation_VP9 stVP9Segments;
    uint8_t  log2_tile_cols;
    uint8_t  log2_tile_rows;
    uint16_t uncompressed_header_size_byte_aligned;
    uint16_t first_partition_size;
    uint16_t Reserved16Bits;
    uint32_t   Reserved32Bits;
    uint32_t   StatusReportFeedbackNumber;
} DXVA_PicParams_VP9, *LPDXVA_PicParams_VP9;

/* VPx slice control data structure - short form */
typedef struct _DXVA_Slice_VPx_Short {
    uint32_t   BSNALunitDataLocation;
    uint32_t   SliceBytesInBuffer;
    uint16_t wBadSliceChopping;
} DXVA_Slice_VPx_Short, *LPDXVA_Slice_VPx_Short;

#pragma pack(pop, BeforeDXVApacking)

void 
d3d12_video_decoder_prepare_current_frame_references_vp9(struct d3d12_video_decoder *pD3D12Dec,
                                                          ID3D12Resource *pTexture2D,
                                                          uint32_t subresourceIndex);

void
d3d12_video_decoder_refresh_dpb_active_references_vp9(struct d3d12_video_decoder *pD3D12Dec);

void
d3d12_video_decoder_get_frame_info_vp9(
   struct d3d12_video_decoder *pD3D12Dec, uint32_t *pWidth, uint32_t *pHeight, uint16_t *pMaxDPB, bool *pIsInterlaced);

DXVA_PicParams_VP9
d3d12_video_decoder_dxva_picparams_from_pipe_picparams_vp9(
   struct d3d12_video_decoder *pD3D12Dec,
   pipe_video_profile profile,
   pipe_vp9_picture_desc *pPipeDesc);

void
d3d12_video_decoder_log_pic_params_vp9(DXVA_PicParams_VP9 *pPicParams);

void
d3d12_video_decoder_prepare_dxva_slices_control_vp9(struct d3d12_video_decoder *pD3D12Dec,
                                                     std::vector<uint8_t> &vecOutSliceControlBuffers,
                                                     struct pipe_vp9_picture_desc *picture_vp9);

#endif
