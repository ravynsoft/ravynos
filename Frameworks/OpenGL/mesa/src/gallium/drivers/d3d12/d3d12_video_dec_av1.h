
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

#ifndef D3D12_VIDEO_DEC_AV1_H
#define D3D12_VIDEO_DEC_AV1_H

#include "d3d12_video_types.h"

// From DXVA AV1 spec
// Entries that will not be used for decoding the current picture,
// or any subsequent pictures, are indicated by setting this value to 0xFF. 
// Also, When DXVA_PicEntry_AV1. Index does not contain an index to a valid 
// reference, the value shall be set to 255, to indicate that the index is invalid.
constexpr uint16_t DXVA_AV1_INVALID_PICTURE_INDEX = 0xFF;

// Frame Restoration types (section 6.10.15)
enum {
    AV1_RESTORE_NONE       = 0,
    AV1_RESTORE_WIENER     = 1,
    AV1_RESTORE_SGRPROJ    = 2,
    AV1_RESTORE_SWITCHABLE = 3,
};

#pragma pack(push, BeforeDXVApacking, 1)

#ifndef _DIRECTX_AV1_VA_
#define _DIRECTX_AV1_VA_

/* AV1 picture entry data structure */
typedef struct _DXVA_PicEntry_AV1 {

    uint32_t width;
    uint32_t height;

    // Global motion parameters
    int32_t wmmat[6];
    union {
        struct {
            uint8_t wminvalid : 1;
            uint8_t wmtype : 2;
            uint8_t Reserved : 5;
        };
        uint8_t GlobalMotionFlags;
    };

    uint8_t Index;
    uint16_t Reserved16Bits;

} DXVA_PicEntry_AV1, *LPDXVA_PicEntry_AV1;

/* AV1 picture parameters structure */
typedef struct _DXVA_PicParams_AV1 {
    uint32_t width;
    uint32_t height;

    uint32_t max_width;
    uint32_t max_height;

    uint8_t CurrPicTextureIndex;
    uint8_t superres_denom;
    uint8_t bitdepth;
    uint8_t seq_profile;

    // Tiles:
    struct {
        uint8_t cols;
        uint8_t rows;
        uint16_t context_update_id;
        uint16_t widths[64];
        uint16_t heights[64];
    } tiles;

    // Coding Tools
    union {
        struct {
            uint32_t use_128x128_superblock : 1;
            uint32_t intra_edge_filter : 1;
            uint32_t interintra_compound : 1;
            uint32_t masked_compound : 1;
            uint32_t warped_motion : 1;
            uint32_t dual_filter : 1;
            uint32_t jnt_comp : 1;
            uint32_t screen_content_tools : 1;
            uint32_t integer_mv : 1;
            uint32_t cdef : 1;
            uint32_t restoration : 1;
            uint32_t film_grain : 1;
            uint32_t intrabc : 1;
            uint32_t high_precision_mv : 1;
            uint32_t switchable_motion_mode : 1;
            uint32_t filter_intra : 1;
            uint32_t disable_frame_end_update_cdf : 1;
            uint32_t disable_cdf_update : 1;
            uint32_t reference_mode : 1;
            uint32_t skip_mode : 1;
            uint32_t reduced_tx_set : 1;
            uint32_t superres : 1;
            uint32_t tx_mode : 2;
            uint32_t use_ref_frame_mvs : 1;
            uint32_t enable_ref_frame_mvs : 1;
            uint32_t reference_frame_update : 1;
            uint32_t Reserved : 5;
        };
        uint32_t CodingParamToolFlags;
    } coding;

    // Format & Picture Info flags
    union {
        struct {
            uint8_t frame_type : 2;
            uint8_t show_frame : 1;
            uint8_t showable_frame : 1;
            uint8_t subsampling_x : 1;
            uint8_t subsampling_y : 1;
            uint8_t mono_chrome : 1;
            uint8_t Reserved : 1;
        };
        uint8_t FormatAndPictureInfoFlags;
    } format;

    // References
    uint8_t primary_ref_frame;
    uint8_t order_hint;
    uint8_t order_hint_bits;

    DXVA_PicEntry_AV1 frame_refs[7];
    uint8_t RefFrameMapTextureIndex[8];

    // Loop filter parameters
    struct {
        uint8_t filter_level[2];
        uint8_t filter_level_u;
        uint8_t filter_level_v;

        uint8_t sharpness_level;
        union {
            struct {
                uint8_t mode_ref_delta_enabled : 1;
                uint8_t mode_ref_delta_update : 1;
                uint8_t delta_lf_multi : 1;
                uint8_t delta_lf_present : 1;
                uint8_t Reserved : 4;
            };
            uint8_t ControlFlags;
        };
        char ref_deltas[8];
        char mode_deltas[2];
        uint8_t delta_lf_res;
        uint8_t frame_restoration_type[3];
        uint16_t log2_restoration_unit_size[3];
        uint16_t Reserved16Bits;
    } loop_filter;

    // Quantization
    struct {
        union {
            struct {
                uint8_t delta_q_present : 1;
                uint8_t delta_q_res : 2;
                uint8_t Reserved : 5;
            };
            uint8_t ControlFlags;
        };

        uint8_t base_qindex;
        char y_dc_delta_q;
        char u_dc_delta_q;
        char v_dc_delta_q;
        char u_ac_delta_q;
        char v_ac_delta_q;
        // using_qmatrix:
        uint8_t qm_y;
        uint8_t qm_u;
        uint8_t qm_v;
        uint16_t Reserved16Bits;
    } quantization;

    // Cdef parameters
    struct {
        union {
            struct {
                uint8_t damping : 2;
                uint8_t bits : 2;
                uint8_t Reserved : 4;
            };
            uint8_t ControlFlags;
        };

        union {
            struct {
                uint8_t primary : 6;
                uint8_t secondary : 2;
            };
            uint8_t combined;
        } y_strengths[8];

        union {
            struct {
                uint8_t primary : 6;
                uint8_t secondary : 2;
            };
            uint8_t combined;
        } uv_strengths[8];

    } cdef;

    uint8_t interp_filter;

    // Segmentation
    struct {
        union {
            struct {
                uint8_t enabled : 1;
                uint8_t update_map : 1;
                uint8_t update_data : 1;
                uint8_t temporal_update : 1;
                uint8_t Reserved : 4;
            };
            uint8_t ControlFlags;
        };
        uint8_t Reserved24Bits[3];

        union {
            struct {
                uint8_t alt_q : 1;
                uint8_t alt_lf_y_v : 1;
                uint8_t alt_lf_y_h : 1;
                uint8_t alt_lf_u : 1;
                uint8_t alt_lf_v : 1;
                uint8_t ref_frame : 1;
                uint8_t skip : 1;
                uint8_t globalmv : 1;
            };
            uint8_t mask;
        } feature_mask[8];

        int16_t feature_data[8][8];

    } segmentation;

    struct {
        union {
            struct {
                uint16_t apply_grain : 1;
                uint16_t scaling_shift_minus8 : 2;
                uint16_t chroma_scaling_from_luma : 1;
                uint16_t ar_coeff_lag : 2;
                uint16_t ar_coeff_shift_minus6 : 2;
                uint16_t grain_scale_shift : 2;
                uint16_t overlap_flag : 1;
                uint16_t clip_to_restricted_range : 1;
                uint16_t matrix_coeff_is_identity : 1;
                uint16_t Reserved : 3;
            };
            uint16_t ControlFlags;
        };

        uint16_t grain_seed;
        uint8_t scaling_points_y[14][2];
        uint8_t num_y_points;
        uint8_t scaling_points_cb[10][2];
        uint8_t num_cb_points;
        uint8_t scaling_points_cr[10][2];
        uint8_t num_cr_points;
        uint8_t ar_coeffs_y[24];
        uint8_t ar_coeffs_cb[25];
        uint8_t ar_coeffs_cr[25];
        uint8_t cb_mult;
        uint8_t cb_luma_mult;
        uint8_t cr_mult;
        uint8_t cr_luma_mult;
        uint8_t Reserved8Bits;
        int16_t cb_offset;
        int16_t cr_offset;
    } film_grain;

    uint32_t   Reserved32Bits;
    uint32_t   StatusReportFeedbackNumber;
} DXVA_PicParams_AV1, *LPDXVA_PicParams_AV1;

/* AV1 tile structure */
typedef struct _DXVA_Tile_AV1 {
    uint32_t   DataOffset;
    uint32_t   DataSize;
    uint16_t row;
    uint16_t column;
    uint16_t Reserved16Bits;
    uint8_t anchor_frame;
    uint8_t Reserved8Bits;
} DXVA_Tile_AV1, *LPDXVA_Tile_AV1;

/* AV1 status reporting data structure */
typedef struct _DXVA_Status_AV1 {
    uint32_t  StatusReportFeedbackNumber;
    DXVA_PicEntry_AV1 CurrPic;
    uint8_t  BufType;
    uint8_t  Status;
    uint8_t  Reserved8Bits;
    uint16_t NumMbsAffected;
} DXVA_Status_AV1, *LPDXVA_Status_AV1;

#endif // _DIRECTX_AV1_VA_

#pragma pack(pop, BeforeDXVApacking)

void 
d3d12_video_decoder_prepare_current_frame_references_av1(struct d3d12_video_decoder *pD3D12Dec,
                                                          ID3D12Resource *pTexture2D,
                                                          uint32_t subresourceIndex);

void
d3d12_video_decoder_refresh_dpb_active_references_av1(struct d3d12_video_decoder *pD3D12Dec);

void
d3d12_video_decoder_get_frame_info_av1(
   struct d3d12_video_decoder *pD3D12Dec, uint32_t *pWidth, uint32_t *pHeight, uint16_t *pMaxDPB, bool &isInterlaced);

DXVA_PicParams_AV1
d3d12_video_decoder_dxva_picparams_from_pipe_picparams_av1(
   uint32_t frameNum,
   pipe_video_profile profile,
   pipe_av1_picture_desc *pPipeDesc);

void
d3d12_video_decoder_log_pic_params_av1(DXVA_PicParams_AV1 *pPicParams);

void
d3d12_video_decoder_prepare_dxva_slices_control_av1(struct d3d12_video_decoder *pD3D12Dec,
                                                     std::vector<uint8_t> &vecOutSliceControlBuffers,
                                                     struct pipe_av1_picture_desc *picture_av1);

#endif
