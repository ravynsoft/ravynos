/**************************************************************************
 *
 * Copyright 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#ifndef _AC_VCN_DEC_H
#define _AC_VCN_DEC_H

/* VCN programming information shared between gallium/vulkan */
#define RDECODE_PKT_TYPE_S(x)        (((unsigned)(x)&0x3) << 30)
#define RDECODE_PKT_TYPE_G(x)        (((x) >> 30) & 0x3)
#define RDECODE_PKT_TYPE_C           0x3FFFFFFF
#define RDECODE_PKT_COUNT_S(x)       (((unsigned)(x)&0x3FFF) << 16)
#define RDECODE_PKT_COUNT_G(x)       (((x) >> 16) & 0x3FFF)
#define RDECODE_PKT_COUNT_C          0xC000FFFF
#define RDECODE_PKT0_BASE_INDEX_S(x) (((unsigned)(x)&0xFFFF) << 0)
#define RDECODE_PKT0_BASE_INDEX_G(x) (((x) >> 0) & 0xFFFF)
#define RDECODE_PKT0_BASE_INDEX_C    0xFFFF0000
#define RDECODE_PKT0(index, count)                                                                 \
   (RDECODE_PKT_TYPE_S(0) | RDECODE_PKT0_BASE_INDEX_S(index) | RDECODE_PKT_COUNT_S(count))

#define RDECODE_PKT2() (RDECODE_PKT_TYPE_S(2))

#define RDECODE_PKT_REG_J(x)  ((unsigned)(x)&0x3FFFF)
#define RDECODE_PKT_RES_J(x)  (((unsigned)(x)&0x3F) << 18)
#define RDECODE_PKT_COND_J(x) (((unsigned)(x)&0xF) << 24)
#define RDECODE_PKT_TYPE_J(x) (((unsigned)(x)&0xF) << 28)
#define RDECODE_PKTJ(reg, cond, type)                                                              \
   (RDECODE_PKT_REG_J(reg) | RDECODE_PKT_RES_J(0) | RDECODE_PKT_COND_J(cond) |                     \
    RDECODE_PKT_TYPE_J(type))

#define RDECODE_IB_PARAM_DECODE_BUFFER                               (0x00000001)
#define RDECODE_IB_PARAM_QUERY_BUFFER                                (0x00000002)
#define RDECODE_IB_PARAM_PREDICATION_BUFFER                          (0x00000003)
#define RDECODE_IB_PARAM_UMD_64BIT_FENCE                             (0x00000005)
#define RDECODE_IB_PARAM_UMD_RECORD_TIMESTAMP                        (0x00000006)
#define RDECODE_IB_PARAM_UMD_REPORT_EVENT_STATUS                     (0x00000007)
#define RDECODE_IB_PARAM_UMD_COPY_MEMORY                             (0x00000008)
#define RDECODE_IB_PARAM_UMD_WRITE_MEMORY                            (0x00000009)
#define RDECODE_IB_PARAM_FEEDBACK_BUFFER                             (0x0000000A)

#define RDECODE_CMDBUF_FLAGS_MSG_BUFFER                              (0x00000001)
#define RDECODE_CMDBUF_FLAGS_DPB_BUFFER                              (0x00000002)
#define RDECODE_CMDBUF_FLAGS_BITSTREAM_BUFFER                        (0x00000004)
#define RDECODE_CMDBUF_FLAGS_DECODING_TARGET_BUFFER                  (0x00000008)
#define RDECODE_CMDBUF_FLAGS_FEEDBACK_BUFFER                         (0x00000010)
#define RDECODE_CMDBUF_FLAGS_PICTURE_PARAM_BUFFER                    (0x00000020)
#define RDECODE_CMDBUF_FLAGS_MB_CONTROL_BUFFER                       (0x00000040)
#define RDECODE_CMDBUF_FLAGS_IDCT_COEF_BUFFER                        (0x00000080)
#define RDECODE_CMDBUF_FLAGS_PREEMPT_BUFFER                          (0x00000100)
#define RDECODE_CMDBUF_FLAGS_IT_SCALING_BUFFER                       (0x00000200)
#define RDECODE_CMDBUF_FLAGS_SCALER_TARGET_BUFFER                    (0x00000400)
#define RDECODE_CMDBUF_FLAGS_CONTEXT_BUFFER                          (0x00000800)
#define RDECODE_CMDBUF_FLAGS_PROB_TBL_BUFFER                         (0x00001000)
#define RDECODE_CMDBUF_FLAGS_QUERY_BUFFER                            (0x00002000)
#define RDECODE_CMDBUF_FLAGS_PREDICATION_BUFFER                      (0x00004000)
#define RDECODE_CMDBUF_FLAGS_SCLR_COEF_BUFFER                        (0x00008000)
#define RDECODE_CMDBUF_FLAGS_RECORD_TIMESTAMP                        (0x00010000)
#define RDECODE_CMDBUF_FLAGS_REPORT_EVENT_STATUS                     (0x00020000)
#define RDECODE_CMDBUF_FLAGS_RESERVED_SIZE_INFO_BUFFER               (0x00040000)
#define RDECODE_CMDBUF_FLAGS_LUMA_HIST_BUFFER                        (0x00080000)
#define RDECODE_CMDBUF_FLAGS_SESSION_CONTEXT_BUFFER                  (0x00100000)

#define RDECODE_CMD_MSG_BUFFER                              0x00000000
#define RDECODE_CMD_DPB_BUFFER                              0x00000001
#define RDECODE_CMD_DECODING_TARGET_BUFFER                  0x00000002
#define RDECODE_CMD_FEEDBACK_BUFFER                         0x00000003
#define RDECODE_CMD_PROB_TBL_BUFFER                         0x00000004
#define RDECODE_CMD_SESSION_CONTEXT_BUFFER                  0x00000005
#define RDECODE_CMD_BITSTREAM_BUFFER                        0x00000100
#define RDECODE_CMD_IT_SCALING_TABLE_BUFFER                 0x00000204
#define RDECODE_CMD_CONTEXT_BUFFER                          0x00000206

#define RDECODE_MSG_CREATE                                  0x00000000
#define RDECODE_MSG_DECODE                                  0x00000001
#define RDECODE_MSG_DESTROY                                 0x00000002

#define RDECODE_CODEC_H264                                  0x00000000
#define RDECODE_CODEC_VC1                                   0x00000001
#define RDECODE_CODEC_MPEG2_VLD                             0x00000003
#define RDECODE_CODEC_MPEG4                                 0x00000004
#define RDECODE_CODEC_H264_PERF                             0x00000007
#define RDECODE_CODEC_JPEG                                  0x00000008
#define RDECODE_CODEC_H265                                  0x00000010
#define RDECODE_CODEC_VP9                                   0x00000011
#define RDECODE_CODEC_AV1                                   0x00000013
#define RDECODE_MESSAGE_HEVC_DIRECT_REF_LIST                0x00000015

#define RDECODE_ARRAY_MODE_LINEAR                           0x00000000
#define RDECODE_ARRAY_MODE_MACRO_LINEAR_MICRO_TILED         0x00000001
#define RDECODE_ARRAY_MODE_1D_THIN                          0x00000002
#define RDECODE_ARRAY_MODE_2D_THIN                          0x00000004
#define RDECODE_ARRAY_MODE_MACRO_TILED_MICRO_LINEAR         0x00000004
#define RDECODE_ARRAY_MODE_MACRO_TILED_MICRO_TILED          0x00000005

#define RDECODE_ARRAY_MODE_ADDRLIB_SEL_GFX10                0x00000000
#define RDECODE_ARRAY_MODE_ADDRLIB_SEL_GFX9                 0x00000001
#define RDECODE_ARRAY_MODE_ADDRLIB_SEL_GFX8                 0x00000002
#define RDECODE_ARRAY_MODE_ADDRLIB_SEL_GFX11                0x00000003

#define RDECODE_H264_PROFILE_BASELINE                       0x00000000
#define RDECODE_H264_PROFILE_MAIN                           0x00000001
#define RDECODE_H264_PROFILE_HIGH                           0x00000002
#define RDECODE_H264_PROFILE_STEREO_HIGH                    0x00000003
#define RDECODE_H264_PROFILE_MVC                            0x00000004

#define RDECODE_VC1_PROFILE_SIMPLE                          0x00000000
#define RDECODE_VC1_PROFILE_MAIN                            0x00000001
#define RDECODE_VC1_PROFILE_ADVANCED                        0x00000002

#define RDECODE_SW_MODE_LINEAR                              0x00000000
#define RDECODE_256B_S                                      0x00000001
#define RDECODE_256B_D                                      0x00000002
#define RDECODE_4KB_S                                       0x00000005
#define RDECODE_4KB_D                                       0x00000006
#define RDECODE_64KB_S                                      0x00000009
#define RDECODE_64KB_D                                      0x0000000A
#define RDECODE_4KB_S_X                                     0x00000015
#define RDECODE_4KB_D_X                                     0x00000016
#define RDECODE_64KB_S_X                                    0x00000019
#define RDECODE_64KB_D_X                                    0x0000001A

#define RDECODE_MESSAGE_NOT_SUPPORTED                       0x00000000
#define RDECODE_MESSAGE_CREATE                              0x00000001
#define RDECODE_MESSAGE_DECODE                              0x00000002
#define RDECODE_MESSAGE_DRM                                 0x00000003
#define RDECODE_MESSAGE_AVC                                 0x00000006
#define RDECODE_MESSAGE_VC1                                 0x00000007
#define RDECODE_MESSAGE_MPEG2_VLD                           0x0000000A
#define RDECODE_MESSAGE_MPEG4_ASP_VLD                       0x0000000B
#define RDECODE_MESSAGE_HEVC                                0x0000000D
#define RDECODE_MESSAGE_VP9                                 0x0000000E
#define RDECODE_MESSAGE_DYNAMIC_DPB                         0x00000010
#define RDECODE_MESSAGE_AV1                                 0x00000011

#define RDECODE_FEEDBACK_PROFILING                          0x00000001

#define RDECODE_SPS_INFO_H264_EXTENSION_SUPPORT_FLAG_SHIFT  7

#define RDECODE_VP9_PROBS_DATA_SIZE                         2304

/* *** decode flags *** */
#define RDECODE_FLAGS_USE_DYNAMIC_DPB_MASK                  0x00000001
#define RDECODE_FLAGS_USE_PAL_MASK                          0x00000008
#define RDECODE_FLAGS_DPB_RESIZE_MASK                       0x00000100

#define mmUVD_JPEG_CNTL                                     0x0200
#define mmUVD_JPEG_CNTL_BASE_IDX                            1
#define mmUVD_JPEG_RB_BASE                                  0x0201
#define mmUVD_JPEG_RB_BASE_BASE_IDX                         1
#define mmUVD_JPEG_RB_WPTR                                  0x0202
#define mmUVD_JPEG_RB_WPTR_BASE_IDX                         1
#define mmUVD_JPEG_RB_RPTR                                  0x0203
#define mmUVD_JPEG_RB_RPTR_BASE_IDX                         1
#define mmUVD_JPEG_RB_SIZE                                  0x0204
#define mmUVD_JPEG_RB_SIZE_BASE_IDX                         1
#define mmUVD_JPEG_TIER_CNTL2                               0x021a
#define mmUVD_JPEG_TIER_CNTL2_BASE_IDX                      1
#define mmUVD_JPEG_UV_TILING_CTRL                           0x021c
#define mmUVD_JPEG_UV_TILING_CTRL_BASE_IDX                  1
#define mmUVD_JPEG_TILING_CTRL                              0x021e
#define mmUVD_JPEG_TILING_CTRL_BASE_IDX                     1
#define mmUVD_JPEG_OUTBUF_RPTR                              0x0220
#define mmUVD_JPEG_OUTBUF_RPTR_BASE_IDX                     1
#define mmUVD_JPEG_OUTBUF_WPTR                              0x0221
#define mmUVD_JPEG_OUTBUF_WPTR_BASE_IDX                     1
#define mmUVD_JPEG_PITCH                                    0x0222
#define mmUVD_JPEG_PITCH_BASE_IDX                           1
#define mmUVD_JPEG_INT_EN                                   0x0229
#define mmUVD_JPEG_INT_EN_BASE_IDX                          1
#define mmUVD_JPEG_UV_PITCH                                 0x022b
#define mmUVD_JPEG_UV_PITCH_BASE_IDX                        1
#define mmUVD_JPEG_INDEX                                    0x023e
#define mmUVD_JPEG_INDEX_BASE_IDX                           1
#define mmUVD_JPEG_DATA                                     0x023f
#define mmUVD_JPEG_DATA_BASE_IDX                            1
#define mmUVD_LMI_JPEG_WRITE_64BIT_BAR_HIGH                 0x0438
#define mmUVD_LMI_JPEG_WRITE_64BIT_BAR_HIGH_BASE_IDX        1
#define mmUVD_LMI_JPEG_WRITE_64BIT_BAR_LOW                  0x0439
#define mmUVD_LMI_JPEG_WRITE_64BIT_BAR_LOW_BASE_IDX         1
#define mmUVD_LMI_JPEG_READ_64BIT_BAR_HIGH                  0x045a
#define mmUVD_LMI_JPEG_READ_64BIT_BAR_HIGH_BASE_IDX         1
#define mmUVD_LMI_JPEG_READ_64BIT_BAR_LOW                   0x045b
#define mmUVD_LMI_JPEG_READ_64BIT_BAR_LOW_BASE_IDX          1
#define mmUVD_CTX_INDEX                                     0x0528
#define mmUVD_CTX_INDEX_BASE_IDX                            1
#define mmUVD_CTX_DATA                                      0x0529
#define mmUVD_CTX_DATA_BASE_IDX                             1
#define mmUVD_SOFT_RESET                                    0x05a0
#define mmUVD_SOFT_RESET_BASE_IDX                           1

#define vcnipUVD_JPEG_DEC_SOFT_RST                          0x402f
#define vcnipUVD_JRBC_IB_COND_RD_TIMER                      0x408e
#define vcnipUVD_JRBC_IB_REF_DATA                           0x408f
#define vcnipUVD_LMI_JPEG_READ_64BIT_BAR_HIGH               0x40e1
#define vcnipUVD_LMI_JPEG_READ_64BIT_BAR_LOW                0x40e0
#define vcnipUVD_JPEG_RB_BASE                               0x4001
#define vcnipUVD_JPEG_RB_SIZE                               0x4004
#define vcnipUVD_JPEG_RB_WPTR                               0x4002
#define vcnipUVD_JPEG_PITCH                                 0x401f
#define vcnipUVD_JPEG_UV_PITCH                              0x4020
#define vcnipJPEG_DEC_ADDR_MODE                             0x4027
#define vcnipJPEG_DEC_Y_GFX10_TILING_SURFACE                0x4024
#define vcnipJPEG_DEC_UV_GFX10_TILING_SURFACE               0x4025
#define vcnipUVD_LMI_JPEG_WRITE_64BIT_BAR_HIGH              0x40e3
#define vcnipUVD_LMI_JPEG_WRITE_64BIT_BAR_LOW               0x40e2
#define vcnipUVD_JPEG_INDEX                                 0x402c
#define vcnipUVD_JPEG_DATA                                  0x402d
#define vcnipUVD_JPEG_TIER_CNTL2                            0x400f
#define vcnipUVD_JPEG_OUTBUF_RPTR                           0x401e
#define vcnipUVD_JPEG_OUTBUF_CNTL                           0x401c
#define vcnipUVD_JPEG_INT_EN                                0x400a
#define vcnipUVD_JPEG_CNTL                                  0x4000
#define vcnipUVD_JPEG_RB_RPTR                               0x4003
#define vcnipUVD_JPEG_OUTBUF_WPTR                           0x401d
#define vcnipUVD_JPEG_DEC_SOFT_RST_1                        0x4051
#define vcnipUVD_JPEG_PITCH_1                               0x4043
#define vcnipUVD_JPEG_UV_PITCH_1                            0x4044
#define vcnipJPEG_DEC_ADDR_MODE_1                           0x404B
#define vcnipUVD_JPEG_TIER_CNTL2_1                          0x400E
#define vcnipUVD_JPEG_OUTBUF_CNTL_1                         0x4040
#define vcnipUVD_JPEG_OUTBUF_WPTR_1                         0x4041
#define vcnipUVD_JPEG_OUTBUF_RPTR_1                         0x4042
#define vcnipUVD_JPEG_LUMA_BASE0_0                          0x41C0
#define vcnipUVD_JPEG_CHROMA_BASE0_0                        0x41C1
#define vcnipUVD_JPEG_CHROMAV_BASE0_0                       0x41C2
#define vcnipJPEG_DEC_Y_GFX10_TILING_SURFACE_1              0x4048
#define vcnipJPEG_DEC_UV_GFX10_TILING_SURFACE_1             0x4049
#define vcnipUVD_LMI_JPEG_WRITE_64BIT_BAR_HIGH_1            0x40B5
#define vcnipUVD_LMI_JPEG_WRITE_64BIT_BAR_LOW_1             0x40B4
#define vcnipUVD_LMI_JPEG_READ_64BIT_BAR_HIGH_1             0x40B3
#define vcnipUVD_LMI_JPEG_READ_64BIT_BAR_LOW_1              0x40B2
#define vcnipUVD_JPEG_ROI_CROP_POS_START                    0x401B
#define vcnipUVD_JPEG_ROI_CROP_POS_STRIDE                   0x401C
#define vcnipUVD_JPEG_INT_STAT                              0x400B
#define vcnipUVD_JPEG_FC_SPS_INFO                           0x4052
#define vcnipUVD_JPEG_SPS_INFO                              0x4006
#define vcnipUVD_JPEG_FC_R_COEF                             0x4018
#define vcnipUVD_JPEG_FC_G_COEF                             0x4019
#define vcnipUVD_JPEG_FC_B_COEF                             0x401A
#define vcnipUVD_JPEG_FC_VUP_COEF_CNTL0                     0x4010
#define vcnipUVD_JPEG_FC_VUP_COEF_CNTL1                     0x4011
#define vcnipUVD_JPEG_FC_VUP_COEF_CNTL2                     0x4012
#define vcnipUVD_JPEG_FC_VUP_COEF_CNTL3                     0x4013
#define vcnipUVD_JPEG_FC_HUP_COEF_CNTL0                     0x4014
#define vcnipUVD_JPEG_FC_HUP_COEF_CNTL1                     0x4015
#define vcnipUVD_JPEG_FC_HUP_COEF_CNTL2                     0x4016
#define vcnipUVD_JPEG_FC_HUP_COEF_CNTL3                     0x4017
#define vcnipUVD_JPEG_FC_TMEOUT_CNT                         0x4183
#define vcnipUVD_JPEG_SPS1_INFO                             0x4007

#define UVD_BASE_INST0_SEG0                                 0x00007800
#define UVD_BASE_INST0_SEG1                                 0x00007E00
#define UVD_BASE_INST0_SEG2                                 0
#define UVD_BASE_INST0_SEG3                                 0
#define UVD_BASE_INST0_SEG4                                 0

#define SOC15_REG_ADDR(reg) (UVD_BASE_INST0_SEG1 + reg)

#define COND0 0
#define COND1 1
#define COND2 2
#define COND3 3
#define COND4 4
#define COND5 5
#define COND6 6
#define COND7 7

#define TYPE0 0
#define TYPE1 1
#define TYPE2 2
#define TYPE3 3
#define TYPE4 4
#define TYPE5 5
#define TYPE6 6
#define TYPE7 7

/* VP9 Frame header flags */
#define RDECODE_FRAME_HDR_INFO_VP9_USE_UNCOMPRESSED_HEADER_SHIFT      (14)
#define RDECODE_FRAME_HDR_INFO_VP9_USE_PREV_IN_FIND_MV_REFS_SHIFT     (13)
#define RDECODE_FRAME_HDR_INFO_VP9_MODE_REF_DELTA_UPDATE_SHIFT        (12)
#define RDECODE_FRAME_HDR_INFO_VP9_MODE_REF_DELTA_ENABLED_SHIFT       (11)
#define RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_UPDATE_DATA_SHIFT     (10)
#define RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_TEMPORAL_UPDATE_SHIFT (9)
#define RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_UPDATE_MAP_SHIFT      (8)
#define RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_ENABLED_SHIFT         (7)
#define RDECODE_FRAME_HDR_INFO_VP9_FRAME_PARALLEL_DECODING_MODE_SHIFT (6)
#define RDECODE_FRAME_HDR_INFO_VP9_REFRESH_FRAME_CONTEXT_SHIFT        (5)
#define RDECODE_FRAME_HDR_INFO_VP9_ALLOW_HIGH_PRECISION_MV_SHIFT      (4)
#define RDECODE_FRAME_HDR_INFO_VP9_INTRA_ONLY_SHIFT                   (3)
#define RDECODE_FRAME_HDR_INFO_VP9_ERROR_RESILIENT_MODE_SHIFT         (2)
#define RDECODE_FRAME_HDR_INFO_VP9_FRAME_TYPE_SHIFT                   (1)
#define RDECODE_FRAME_HDR_INFO_VP9_SHOW_EXISTING_FRAME_SHIFT          (0)


#define RDECODE_FRAME_HDR_INFO_VP9_USE_UNCOMPRESSED_HEADER_MASK      (0x00004000)
#define RDECODE_FRAME_HDR_INFO_VP9_USE_PREV_IN_FIND_MV_REFS_MASK     (0x00002000)
#define RDECODE_FRAME_HDR_INFO_VP9_MODE_REF_DELTA_UPDATE_MASK        (0x00001000)
#define RDECODE_FRAME_HDR_INFO_VP9_MODE_REF_DELTA_ENABLED_MASK       (0x00000800)
#define RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_UPDATE_DATA_MASK     (0x00000400)
#define RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_TEMPORAL_UPDATE_MASK (0x00000200)
#define RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_UPDATE_MAP_MASK      (0x00000100)
#define RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_ENABLED_MASK         (0x00000080)
#define RDECODE_FRAME_HDR_INFO_VP9_FRAME_PARALLEL_DECODING_MODE_MASK (0x00000040)
#define RDECODE_FRAME_HDR_INFO_VP9_REFRESH_FRAME_CONTEXT_MASK        (0x00000020)
#define RDECODE_FRAME_HDR_INFO_VP9_ALLOW_HIGH_PRECISION_MV_MASK      (0x00000010)
#define RDECODE_FRAME_HDR_INFO_VP9_INTRA_ONLY_MASK                   (0x00000008)
#define RDECODE_FRAME_HDR_INFO_VP9_ERROR_RESILIENT_MODE_MASK         (0x00000004)
#define RDECODE_FRAME_HDR_INFO_VP9_FRAME_TYPE_MASK                   (0x00000002)
#define RDECODE_FRAME_HDR_INFO_VP9_SHOW_EXISTING_FRAME_MASK          (0x00000001)

/* Drm definitions */
#define DRM_CMD_KEY_SHIFT              0
#define DRM_CMD_CNT_KEY_SHIFT          1
#define DRM_CMD_CNT_DATA_SHIFT         2
#define DRM_CMD_OFFSET_SHIFT           3
#define DRM_CMD_SESSION_SEL_SHIFT      4
#define DRM_CMD_UNWRAP_KEY_SHIFT       8
#define DRM_CMD_GEN_MASK_SHIFT         9
#define DRM_CMD_ALGORITHM_SHIFT        10
#define DRM_CMD_BYTE_MASK_SHIFT        16
#define DRM_CMD_DRM_BYPASS_SHIFT       31

#define DRM_CMD_KEY_MASK               (0x00000001)
#define DRM_CMD_CNT_KEY_MASK           (0x00000002)
#define DRM_CMD_CNT_DATA_MASK          (0x00000004)
#define DRM_CMD_OFFSET_MASK            (0x00000008)
#define DRM_CMD_SESSION_SEL_MASK       (0x000000F0)
#define DRM_CMD_UNWRAP_KEY_MASK        (0x00000100)
#define DRM_CMD_GEN_MASK_MASK          (0x00000200)
#define DRM_CMD_ALGORITHM_MASK         (0x00000C00)
#define DRM_CMD_BYTE_MASK_MASK         (0x00FF0000)
#define DRM_CMD_DRM_BYPASS_MASK        (0x80000000)

/* Drm_cntl definitions */
#define DRM_CNTL_ENC_BYTECNT_SHIFT     (6)
#define DRM_CNTL_CLR_BYTECNT_SHIFT     (16)
#define DRM_CNTL_BYPASS_SHIFT          (24)
#define DRM_CNTL_PARTIAL_MODE_SHIFT    (25)
#define DRM_CNTL_OFFSET_MODE_SHIFT     (26)
#define DRM_CNTL_HEADER_MODE_SHIFT     (27)
#define DRM_CNTL_HEADER_BYTECNT_SHIFT  (28)

#define DRM_CNTL_ENC_BYTECNT_MASK      (0x00000FC0)
#define DRM_CNTL_CLR_BYTECNT_MASK      (0x003F0000)
#define DRM_CNTL_BYPASS_MASK           (0x01000000)
#define DRM_CNTL_PARTIAL_MODE_MASK     (0x02000000)
#define DRM_CNTL_OFFSET_MODE_MASK      (0x04000000)
#define DRM_CNTL_HEADER_MODE_MASK      (0x08000000)
#define DRM_CNTL_HEADER_BYTECNT_MASK   (0xF0000000)

#define SAMU_DRM_DISABLE 0x00000000
#define SAMU_DRM_ENABLE  0x00000001

/* AV1 Frame header flags */
#define RDECODE_FRAME_HDR_INFO_AV1_DISABLE_REF_FRAME_MVS_SHIFT        (31)
#define RDECODE_FRAME_HDR_INFO_AV1_SKIP_REFERENCE_UPDATE_SHIFT        (30)
#define RDECODE_FRAME_HDR_INFO_AV1_SWITCHABLE_SKIP_MODE_SHIFT         (29)
#define RDECODE_FRAME_HDR_INFO_AV1_DELTA_LF_MULTI_SHIFT               (28)
#define RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_TEMPORAL_UPDATE_SHIFT (27)
#define RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_UPDATE_MAP_SHIFT      (26)
#define RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_ENABLED_SHIFT         (25)
#define RDECODE_FRAME_HDR_INFO_AV1_REDUCED_TX_SET_USED_SHIFT          (24)
#define RDECODE_FRAME_HDR_INFO_AV1_DELTA_LF_PRESENT_FLAG_SHIFT        (23)
#define RDECODE_FRAME_HDR_INFO_AV1_DELTA_Q_PRESENT_FLAG_SHIFT         (22)
#define RDECODE_FRAME_HDR_INFO_AV1_MODE_REF_DELTA_UPDATE_SHIFT        (21)
#define RDECODE_FRAME_HDR_INFO_AV1_MODE_REF_DELTA_ENABLED_SHIFT       (20)
#define RDECODE_FRAME_HDR_INFO_AV1_CUR_FRAME_FORCE_INTEGER_MV_SHIFT   (19)
#define RDECODE_FRAME_HDR_INFO_AV1_ALLOW_SCREEN_CONTENT_TOOLS_SHIFT   (18)
#define RDECODE_FRAME_HDR_INFO_AV1_ALLOW_REF_FRAME_MVS_SHIFT          (17)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_JNT_COMP_SHIFT              (16)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_ORDER_HINT_SHIFT            (15)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_DUAL_FILTER_SHIFT           (14)
#define RDECODE_FRAME_HDR_INFO_AV1_ALLOW_WARPED_MOTION_SHIFT          (13)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_MASKED_COMPOUND_SHIFT       (12)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_INTERINTRA_COMPOUND_SHIFT   (11)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_INTRA_EDGE_FILTER_SHIFT     (10)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_FILTER_INTRA_SHIFT          (9)
#define RDECODE_FRAME_HDR_INFO_AV1_USING_QMATRIX_SHIFT                (8)
#define RDECODE_FRAME_HDR_INFO_AV1_SKIP_MODE_FLAG_SHIFT               (7)
#define RDECODE_FRAME_HDR_INFO_AV1_MONOCHROME_SHIFT                   (6)
#define RDECODE_FRAME_HDR_INFO_AV1_ALLOW_HIGH_PRECISION_MV_SHIFT      (5)
#define RDECODE_FRAME_HDR_INFO_AV1_ALLOW_INTRABC_SHIFT                (4)
#define RDECODE_FRAME_HDR_INFO_AV1_INTRA_ONLY_SHIFT                   (3)
#define RDECODE_FRAME_HDR_INFO_AV1_REFRESH_FRAME_CONTEXT_SHIFT        (2)
#define RDECODE_FRAME_HDR_INFO_AV1_DISABLE_CDF_UPDATE_SHIFT           (1)
#define RDECODE_FRAME_HDR_INFO_AV1_SHOW_FRAME_SHIFT                   (0)

#define RDECODE_FRAME_HDR_INFO_AV1_DISABLE_REF_FRAME_MVS_MASK         (0x80000000)
#define RDECODE_FRAME_HDR_INFO_AV1_SKIP_REFERENCE_UPDATE_MASK         (0x40000000)
#define RDECODE_FRAME_HDR_INFO_AV1_SWITCHABLE_SKIP_MODE_MASK          (0x20000000)
#define RDECODE_FRAME_HDR_INFO_AV1_DELTA_LF_MULTI_MASK                (0x10000000)
#define RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_TEMPORAL_UPDATE_MASK  (0x08000000)
#define RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_UPDATE_MAP_MASK       (0x04000000)
#define RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_ENABLED_MASK          (0x02000000)
#define RDECODE_FRAME_HDR_INFO_AV1_REDUCED_TX_SET_USED_MASK           (0x01000000)
#define RDECODE_FRAME_HDR_INFO_AV1_DELTA_LF_PRESENT_FLAG_MASK         (0x00800000)
#define RDECODE_FRAME_HDR_INFO_AV1_DELTA_Q_PRESENT_FLAG_MASK          (0x00400000)
#define RDECODE_FRAME_HDR_INFO_AV1_MODE_REF_DELTA_UPDATE_MASK         (0x00200000)
#define RDECODE_FRAME_HDR_INFO_AV1_MODE_REF_DELTA_ENABLED_MASK        (0x00100000)
#define RDECODE_FRAME_HDR_INFO_AV1_CUR_FRAME_FORCE_INTEGER_MV_MASK    (0x00080000)
#define RDECODE_FRAME_HDR_INFO_AV1_ALLOW_SCREEN_CONTENT_TOOLS_MASK    (0x00040000)
#define RDECODE_FRAME_HDR_INFO_AV1_ALLOW_REF_FRAME_MVS_MASK           (0x00020000)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_JNT_COMP_MASK               (0x00010000)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_ORDER_HINT_MASK             (0x00008000)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_DUAL_FILTER_MASK            (0x00004000)
#define RDECODE_FRAME_HDR_INFO_AV1_ALLOW_WARPED_MOTION_MASK           (0x00002000)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_MASKED_COMPOUND_MASK        (0x00001000)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_INTERINTRA_COMPOUND_MASK    (0x00000800)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_INTRA_EDGE_FILTER_MASK      (0x00000400)
#define RDECODE_FRAME_HDR_INFO_AV1_ENABLE_FILTER_INTRA_MASK           (0x00000200)
#define RDECODE_FRAME_HDR_INFO_AV1_USING_QMATRIX_MASK                 (0x00000100)
#define RDECODE_FRAME_HDR_INFO_AV1_SKIP_MODE_FLAG_MASK                (0x00000080)
#define RDECODE_FRAME_HDR_INFO_AV1_MONOCHROME_MASK                    (0x08000040)
#define RDECODE_FRAME_HDR_INFO_AV1_ALLOW_HIGH_PRECISION_MV_MASK       (0x00000020)
#define RDECODE_FRAME_HDR_INFO_AV1_ALLOW_INTRABC_MASK                 (0x00000010)
#define RDECODE_FRAME_HDR_INFO_AV1_INTRA_ONLY_MASK                    (0x00000008)
#define RDECODE_FRAME_HDR_INFO_AV1_REFRESH_FRAME_CONTEXT_MASK         (0x00000004)
#define RDECODE_FRAME_HDR_INFO_AV1_DISABLE_CDF_UPDATE_MASK            (0x00000002)
#define RDECODE_FRAME_HDR_INFO_AV1_SHOW_FRAME_MASK                    (0x00000001)

#define RDECODE_AV1_VER_0  0
#define RDECODE_AV1_VER_1  1

typedef struct rvcn_decode_buffer_s {
   unsigned int valid_buf_flag;
   unsigned int msg_buffer_address_hi;
   unsigned int msg_buffer_address_lo;
   unsigned int dpb_buffer_address_hi;
   unsigned int dpb_buffer_address_lo;
   unsigned int target_buffer_address_hi;
   unsigned int target_buffer_address_lo;
   unsigned int session_contex_buffer_address_hi;
   unsigned int session_contex_buffer_address_lo;
   unsigned int bitstream_buffer_address_hi;
   unsigned int bitstream_buffer_address_lo;
   unsigned int context_buffer_address_hi;
   unsigned int context_buffer_address_lo;
   unsigned int feedback_buffer_address_hi;
   unsigned int feedback_buffer_address_lo;
   unsigned int luma_hist_buffer_address_hi;
   unsigned int luma_hist_buffer_address_lo;
   unsigned int prob_tbl_buffer_address_hi;
   unsigned int prob_tbl_buffer_address_lo;
   unsigned int sclr_coeff_buffer_address_hi;
   unsigned int sclr_coeff_buffer_address_lo;
   unsigned int it_sclr_table_buffer_address_hi;
   unsigned int it_sclr_table_buffer_address_lo;
   unsigned int sclr_target_buffer_address_hi;
   unsigned int sclr_target_buffer_address_lo;
   unsigned int reserved_size_info_buffer_address_hi;
   unsigned int reserved_size_info_buffer_address_lo;
   unsigned int mpeg2_pic_param_buffer_address_hi;
   unsigned int mpeg2_pic_param_buffer_address_lo;
   unsigned int mpeg2_mb_control_buffer_address_hi;
   unsigned int mpeg2_mb_control_buffer_address_lo;
   unsigned int mpeg2_idct_coeff_buffer_address_hi;
   unsigned int mpeg2_idct_coeff_buffer_address_lo;
} rvcn_decode_buffer_t;

typedef struct rvcn_decode_ib_package_s {
   unsigned int package_size;
   unsigned int package_type;
} rvcn_decode_ib_package_t;

typedef struct rvcn_dec_message_index_s {
   unsigned int message_id;
   unsigned int offset;
   unsigned int size;
   unsigned int filled;
} rvcn_dec_message_index_t;

typedef struct rvcn_dec_message_header_s {
   unsigned int header_size;
   unsigned int total_size;
   unsigned int num_buffers;
   unsigned int msg_type;
   unsigned int stream_handle;
   unsigned int status_report_feedback_number;

   rvcn_dec_message_index_t index[1];
} rvcn_dec_message_header_t;

typedef struct rvcn_dec_message_create_s {
   unsigned int stream_type;
   unsigned int session_flags;
   unsigned int width_in_samples;
   unsigned int height_in_samples;
} rvcn_dec_message_create_t;

typedef struct rvcn_dec_message_decode_s {
   unsigned int stream_type;
   unsigned int decode_flags;
   unsigned int width_in_samples;
   unsigned int height_in_samples;

   unsigned int bsd_size;
   unsigned int dpb_size;
   unsigned int dt_size;
   unsigned int sct_size;
   unsigned int sc_coeff_size;
   unsigned int hw_ctxt_size;
   unsigned int sw_ctxt_size;
   unsigned int pic_param_size;
   unsigned int mb_cntl_size;
   unsigned int reserved0[4];
   unsigned int decode_buffer_flags;

   unsigned int db_pitch;
   unsigned int db_aligned_height;
   unsigned int db_tiling_mode;
   unsigned int db_swizzle_mode;
   unsigned int db_array_mode;
   unsigned int db_field_mode;
   unsigned int db_surf_tile_config;

   unsigned int dt_pitch;
   unsigned int dt_uv_pitch;
   unsigned int dt_tiling_mode;
   unsigned int dt_swizzle_mode;
   unsigned int dt_array_mode;
   unsigned int dt_field_mode;
   unsigned int dt_out_format;
   unsigned int dt_surf_tile_config;
   unsigned int dt_uv_surf_tile_config;
   unsigned int dt_luma_top_offset;
   unsigned int dt_luma_bottom_offset;
   unsigned int dt_chroma_top_offset;
   unsigned int dt_chroma_bottom_offset;
   unsigned int dt_chromaV_top_offset;
   unsigned int dt_chromaV_bottom_offset;

   unsigned int mif_wrc_en;
   unsigned int db_pitch_uv;

   unsigned char reserved1[20];
} rvcn_dec_message_decode_t;

typedef struct rvcn_dec_message_drm_s {
   unsigned int	drm_key[4];
   unsigned int	drm_counter[4];
   unsigned int	drm_wrapped_key[4];
   unsigned int	drm_offset;
   unsigned int	drm_cmd;
   unsigned int	drm_cntl;
   unsigned int	drm_reserved;
} rvcn_dec_message_drm_t;

typedef struct rvcn_dec_message_dynamic_dpb_s {
   unsigned int dpbConfigFlags;
   unsigned int dpbLumaPitch;
   unsigned int dpbLumaAlignedHeight;
   unsigned int dpbLumaAlignedSize;
   unsigned int dpbChromaPitch;
   unsigned int dpbChromaAlignedHeight;
   unsigned int dpbChromaAlignedSize;

   unsigned char dpbArraySize;
   unsigned char dpbCurArraySlice;
   unsigned char dpbRefArraySlice[16];
   unsigned char dpbReserved0[2];

   unsigned int dpbCurrOffset;
   unsigned int dpbAddrOffset[16];
} rvcn_dec_message_dynamic_dpb_t;

typedef struct rvcn_dec_message_dynamic_dpb_t2_s {
    unsigned int dpbConfigFlags;
    unsigned int dpbLumaPitch;
    unsigned int dpbLumaAlignedHeight;
    unsigned int dpbLumaAlignedSize;
    unsigned int dpbChromaPitch;
    unsigned int dpbChromaAlignedHeight;
    unsigned int dpbChromaAlignedSize;
    unsigned int dpbArraySize;

    unsigned int dpbCurrLo;
    unsigned int dpbCurrHi;
    unsigned int dpbAddrLo[16];
    unsigned int dpbAddrHi[16];
} rvcn_dec_message_dynamic_dpb_t2_t;

typedef struct rvcn_dec_message_hevc_direct_ref_list_s {
   unsigned int num_direct_reflist;
   unsigned char multi_direct_reflist[128][2][15];
} rvcn_dec_message_hevc_direct_ref_list_t;

typedef struct {
   unsigned short viewOrderIndex;
   unsigned short viewId;
   unsigned short numOfAnchorRefsInL0;
   unsigned short viewIdOfAnchorRefsInL0[15];
   unsigned short numOfAnchorRefsInL1;
   unsigned short viewIdOfAnchorRefsInL1[15];
   unsigned short numOfNonAnchorRefsInL0;
   unsigned short viewIdOfNonAnchorRefsInL0[15];
   unsigned short numOfNonAnchorRefsInL1;
   unsigned short viewIdOfNonAnchorRefsInL1[15];
} radeon_mvcElement_t;

typedef struct rvcn_dec_message_avc_s {
   unsigned int profile;
   unsigned int level;

   unsigned int sps_info_flags;
   unsigned int pps_info_flags;
   unsigned char chroma_format;
   unsigned char bit_depth_luma_minus8;
   unsigned char bit_depth_chroma_minus8;
   unsigned char log2_max_frame_num_minus4;

   unsigned char pic_order_cnt_type;
   unsigned char log2_max_pic_order_cnt_lsb_minus4;
   unsigned char num_ref_frames;
   unsigned char reserved_8bit;

   signed char pic_init_qp_minus26;
   signed char pic_init_qs_minus26;
   signed char chroma_qp_index_offset;
   signed char second_chroma_qp_index_offset;

   unsigned char num_slice_groups_minus1;
   unsigned char slice_group_map_type;
   unsigned char num_ref_idx_l0_active_minus1;
   unsigned char num_ref_idx_l1_active_minus1;

   unsigned short slice_group_change_rate_minus1;
   unsigned short reserved_16bit_1;

   unsigned char scaling_list_4x4[6][16];
   unsigned char scaling_list_8x8[2][64];

   unsigned int frame_num;
   unsigned int frame_num_list[16];
   int curr_field_order_cnt_list[2];
   int field_order_cnt_list[16][2];

   unsigned int decoded_pic_idx;
   unsigned int curr_pic_ref_frame_num;
   unsigned char ref_frame_list[16];

   unsigned int reserved[122];

   struct {
      unsigned int numViews;
      unsigned int viewId0;
      radeon_mvcElement_t mvcElements[1];
   } mvc;

   unsigned short non_existing_frame_flags;
   unsigned int used_for_reference_flags;
} rvcn_dec_message_avc_t;

typedef struct rvcn_dec_message_vc1_s {
   unsigned int profile;
   unsigned int level;
   unsigned int sps_info_flags;
   unsigned int pps_info_flags;
   unsigned int pic_structure;
   unsigned int chroma_format;
   unsigned short decoded_pic_idx;
   unsigned short deblocked_pic_idx;
   unsigned short forward_ref_idx;
   unsigned short backward_ref_idx;
   unsigned int cached_frame_flag;
} rvcn_dec_message_vc1_t;

typedef struct rvcn_dec_message_mpeg2_vld_s {
   unsigned int decoded_pic_idx;
   unsigned int forward_ref_pic_idx;
   unsigned int backward_ref_pic_idx;

   unsigned char load_intra_quantiser_matrix;
   unsigned char load_nonintra_quantiser_matrix;
   unsigned char reserved_quantiser_alignement[2];
   unsigned char intra_quantiser_matrix[64];
   unsigned char nonintra_quantiser_matrix[64];

   unsigned char profile_and_level_indication;
   unsigned char chroma_format;

   unsigned char picture_coding_type;

   unsigned char reserved_1;

   unsigned char f_code[2][2];
   unsigned char intra_dc_precision;
   unsigned char pic_structure;
   unsigned char top_field_first;
   unsigned char frame_pred_frame_dct;
   unsigned char concealment_motion_vectors;
   unsigned char q_scale_type;
   unsigned char intra_vlc_format;
   unsigned char alternate_scan;
} rvcn_dec_message_mpeg2_vld_t;

typedef struct rvcn_dec_message_mpeg4_asp_vld_s {
   unsigned int decoded_pic_idx;
   unsigned int forward_ref_pic_idx;
   unsigned int backward_ref_pic_idx;

   unsigned int variant_type;
   unsigned char profile_and_level_indication;

   unsigned char video_object_layer_verid;
   unsigned char video_object_layer_shape;

   unsigned char reserved_1;

   unsigned short video_object_layer_width;
   unsigned short video_object_layer_height;

   unsigned short vop_time_increment_resolution;

   unsigned short reserved_2;

   struct {
      unsigned int short_video_header : 1;
      unsigned int obmc_disable : 1;
      unsigned int interlaced : 1;
      unsigned int load_intra_quant_mat : 1;
      unsigned int load_nonintra_quant_mat : 1;
      unsigned int quarter_sample : 1;
      unsigned int complexity_estimation_disable : 1;
      unsigned int resync_marker_disable : 1;
      unsigned int data_partitioned : 1;
      unsigned int reversible_vlc : 1;
      unsigned int newpred_enable : 1;
      unsigned int reduced_resolution_vop_enable : 1;
      unsigned int scalability : 1;
      unsigned int is_object_layer_identifier : 1;
      unsigned int fixed_vop_rate : 1;
      unsigned int newpred_segment_type : 1;
      unsigned int reserved_bits : 16;
   };

   unsigned char quant_type;
   unsigned char reserved_3[3];
   unsigned char intra_quant_mat[64];
   unsigned char nonintra_quant_mat[64];

   struct {
      unsigned char sprite_enable;

      unsigned char reserved_4[3];

      unsigned short sprite_width;
      unsigned short sprite_height;
      short sprite_left_coordinate;
      short sprite_top_coordinate;

      unsigned char no_of_sprite_warping_points;
      unsigned char sprite_warping_accuracy;
      unsigned char sprite_brightness_change;
      unsigned char low_latency_sprite_enable;
   } sprite_config;

   struct {
      struct {
         unsigned int check_skip : 1;
         unsigned int switch_rounding : 1;
         unsigned int t311 : 1;
         unsigned int reserved_bits : 29;
      };

      unsigned char vol_mode;

      unsigned char reserved_5[3];
   } divx_311_config;

   struct {
      unsigned char vop_data_present;
      unsigned char vop_coding_type;
      unsigned char vop_quant;
      unsigned char vop_coded;
      unsigned char vop_rounding_type;
      unsigned char intra_dc_vlc_thr;
      unsigned char top_field_first;
      unsigned char alternate_vertical_scan_flag;
      unsigned char vop_fcode_forward;
      unsigned char vop_fcode_backward;
      unsigned int TRB[2];
      unsigned int TRD[2];
   } vop;

} rvcn_dec_message_mpeg4_asp_vld_t;

typedef struct rvcn_dec_message_hevc_s {
   unsigned int sps_info_flags;
   unsigned int pps_info_flags;
   unsigned char chroma_format;
   unsigned char bit_depth_luma_minus8;
   unsigned char bit_depth_chroma_minus8;
   unsigned char log2_max_pic_order_cnt_lsb_minus4;

   unsigned char sps_max_dec_pic_buffering_minus1;
   unsigned char log2_min_luma_coding_block_size_minus3;
   unsigned char log2_diff_max_min_luma_coding_block_size;
   unsigned char log2_min_transform_block_size_minus2;

   unsigned char log2_diff_max_min_transform_block_size;
   unsigned char max_transform_hierarchy_depth_inter;
   unsigned char max_transform_hierarchy_depth_intra;
   unsigned char pcm_sample_bit_depth_luma_minus1;

   unsigned char pcm_sample_bit_depth_chroma_minus1;
   unsigned char log2_min_pcm_luma_coding_block_size_minus3;
   unsigned char log2_diff_max_min_pcm_luma_coding_block_size;
   unsigned char num_extra_slice_header_bits;

   unsigned char num_short_term_ref_pic_sets;
   unsigned char num_long_term_ref_pic_sps;
   unsigned char num_ref_idx_l0_default_active_minus1;
   unsigned char num_ref_idx_l1_default_active_minus1;

   signed char pps_cb_qp_offset;
   signed char pps_cr_qp_offset;
   signed char pps_beta_offset_div2;
   signed char pps_tc_offset_div2;

   unsigned char diff_cu_qp_delta_depth;
   unsigned char num_tile_columns_minus1;
   unsigned char num_tile_rows_minus1;
   unsigned char log2_parallel_merge_level_minus2;

   unsigned short column_width_minus1[19];
   unsigned short row_height_minus1[21];

   signed char init_qp_minus26;
   unsigned char num_delta_pocs_ref_rps_idx;
   unsigned char curr_idx;
   unsigned char reserved[1];
   int curr_poc;
   unsigned char ref_pic_list[16];
   int poc_list[16];
   unsigned char ref_pic_set_st_curr_before[8];
   unsigned char ref_pic_set_st_curr_after[8];
   unsigned char ref_pic_set_lt_curr[8];

   unsigned char ucScalingListDCCoefSizeID2[6];
   unsigned char ucScalingListDCCoefSizeID3[2];

   unsigned char highestTid;
   unsigned char isNonRef;

   unsigned char p010_mode;
   unsigned char msb_mode;
   unsigned char luma_10to8;
   unsigned char chroma_10to8;

   unsigned char hevc_reserved[2];

   unsigned char direct_reflist[2][15];
   unsigned int st_rps_bits;
} rvcn_dec_message_hevc_t;

typedef struct rvcn_dec_message_vp9_s {
   unsigned int frame_header_flags;

   unsigned char frame_context_idx;
   unsigned char reset_frame_context;

   unsigned char curr_pic_idx;
   unsigned char interp_filter;

   unsigned char filter_level;
   unsigned char sharpness_level;
   unsigned char lf_adj_level[8][4][2];
   unsigned char base_qindex;
   signed char y_dc_delta_q;
   signed char uv_ac_delta_q;
   signed char uv_dc_delta_q;

   unsigned char log2_tile_cols;
   unsigned char log2_tile_rows;
   unsigned char tx_mode;
   unsigned char reference_mode;
   unsigned char chroma_format;

   unsigned char ref_frame_map[8];

   unsigned char frame_refs[3];
   unsigned char ref_frame_sign_bias[3];
   unsigned char frame_to_show;
   unsigned char bit_depth_luma_minus8;
   unsigned char bit_depth_chroma_minus8;

   unsigned char p010_mode;
   unsigned char msb_mode;
   unsigned char luma_10to8;
   unsigned char chroma_10to8;

   unsigned int vp9_frame_size;
   unsigned int compressed_header_size;
   unsigned int uncompressed_header_size;
} rvcn_dec_message_vp9_t;

typedef enum {
   RVCN_DEC_AV1_IDENTITY = 0,
   RVCN_DEC_AV1_TRANSLATION = 1,
   RVCN_DEC_AV1_ROTZOOM = 2,
   RVCN_DEC_AV1_AFFINE = 3,
   RVCN_DEC_AV1_HORTRAPEZOID = 4,
   RVCN_DEC_AV1_VERTRAPEZOID = 5,
   RVCN_DEC_AV1_HOMOGRAPHY = 6,
   RVCN_DEC_AV1_TRANS_TYPES = 7,
} rvcn_dec_transformation_type_e;

typedef struct {
   rvcn_dec_transformation_type_e wmtype;
   int wmmat[8];
   short alpha, beta, gamma, delta;
} rvcn_dec_warped_motion_params_t;

typedef struct {
   unsigned char apply_grain;
   unsigned char scaling_points_y[14][2];
   unsigned char num_y_points;
   unsigned char scaling_points_cb[10][2];
   unsigned char num_cb_points;
   unsigned char scaling_points_cr[10][2];
   unsigned char num_cr_points;
   unsigned char scaling_shift;
   unsigned char ar_coeff_lag;
   signed char ar_coeffs_y[24];
   signed char ar_coeffs_cb[25];
   signed char ar_coeffs_cr[25];
   unsigned char ar_coeff_shift;
   unsigned char cb_mult;
   unsigned char cb_luma_mult;
   unsigned short cb_offset;
   unsigned char cr_mult;
   unsigned char cr_luma_mult;
   unsigned short cr_offset;
   unsigned char overlap_flag;
   unsigned char clip_to_restricted_range;
   unsigned char bit_depth_minus_8;
   unsigned char chroma_scaling_from_luma;
   unsigned char grain_scale_shift;
   unsigned short random_seed;
} rvcn_dec_film_grain_params_t;

typedef struct rvcn_dec_av1_tile_info_s {
   unsigned int offset;
   unsigned int size;
} rvcn_dec_av1_tile_info_t;

typedef struct rvcn_dec_message_av1_s {
   unsigned int frame_header_flags;
   unsigned int current_frame_id;
   unsigned int frame_offset;

   unsigned char profile;
   unsigned char is_annexb;
   unsigned char frame_type;
   unsigned char primary_ref_frame;
   unsigned char curr_pic_idx;

   unsigned char sb_size;
   unsigned char interp_filter;
   unsigned char filter_level[2];
   unsigned char filter_level_u;
   unsigned char filter_level_v;
   unsigned char sharpness_level;
   signed char ref_deltas[8];
   signed char mode_deltas[2];
   unsigned char base_qindex;
   signed char y_dc_delta_q;
   signed char u_dc_delta_q;
   signed char v_dc_delta_q;
   signed char u_ac_delta_q;
   signed char v_ac_delta_q;
   signed char qm_y;
   signed char qm_u;
   signed char qm_v;
   signed char delta_q_res;
   signed char delta_lf_res;

   unsigned char tile_cols;
   unsigned char tile_rows;
   unsigned char tx_mode;
   unsigned char reference_mode;
   unsigned char chroma_format;
   unsigned int tile_size_bytes;
   unsigned int context_update_tile_id;
   unsigned int tile_col_start_sb[65];
   unsigned int tile_row_start_sb[65];
   unsigned int max_width;
   unsigned int max_height;
   unsigned int width;
   unsigned int height;
   unsigned int superres_upscaled_width;
   unsigned char superres_scale_denominator;
   unsigned char order_hint_bits;

   unsigned char ref_frame_map[8];
   unsigned int ref_frame_offset[8];
   unsigned char frame_refs[7];
   unsigned char ref_frame_sign_bias[7];

   unsigned char bit_depth_luma_minus8;
   unsigned char bit_depth_chroma_minus8;

   int feature_data[8][8];
   unsigned char feature_mask[8];

   unsigned char cdef_damping;
   unsigned char cdef_bits;
   unsigned short cdef_strengths[16];
   unsigned short cdef_uv_strengths[16];
   unsigned char frame_restoration_type[3];
   unsigned char log2_restoration_unit_size_minus5[3];

   unsigned char p010_mode;
   unsigned char msb_mode;
   unsigned char luma_10to8;
   unsigned char chroma_10to8;
   unsigned char preskip_segid;
   unsigned char last_active_segid;
   unsigned char seg_lossless_flag;
   unsigned char coded_lossless;
   rvcn_dec_film_grain_params_t film_grain;
   unsigned int uncompressed_header_size;
   rvcn_dec_warped_motion_params_t global_motion[8];
   rvcn_dec_av1_tile_info_t tile_info[256];
} rvcn_dec_message_av1_t;

typedef struct rvcn_dec_feature_index_s {
   unsigned int feature_id;
   unsigned int offset;
   unsigned int size;
   unsigned int filled;
} rvcn_dec_feature_index_t;

typedef struct rvcn_dec_feedback_header_s {
   unsigned int header_size;
   unsigned int total_size;
   unsigned int num_buffers;
   unsigned int status_report_feedback_number;
   unsigned int status;
   unsigned int value;
   unsigned int errorBits;
   rvcn_dec_feature_index_t index[1];
} rvcn_dec_feedback_header_t;

typedef struct rvcn_dec_feedback_profiling_s {
   unsigned int size;

   unsigned int decodingTime;
   unsigned int decodePlusOverhead;
   unsigned int masterTimerHits;
   unsigned int uvdLBSIREWaitCount;

   unsigned int avgMPCMemLatency;
   unsigned int maxMPCMemLatency;
   unsigned int uvdMPCLumaHits;
   unsigned int uvdMPCLumaHitPend;
   unsigned int uvdMPCLumaSearch;
   unsigned int uvdMPCChromaHits;
   unsigned int uvdMPCChromaHitPend;
   unsigned int uvdMPCChromaSearch;

   unsigned int uvdLMIPerfCountLo;
   unsigned int uvdLMIPerfCountHi;
   unsigned int uvdLMIAvgLatCntrEnvHit;
   unsigned int uvdLMILatCntr;

   unsigned int frameCRC0;
   unsigned int frameCRC1;
   unsigned int frameCRC2;
   unsigned int frameCRC3;

   unsigned int uvdLMIPerfMonCtrl;
   unsigned int uvdLMILatCtrl;
   unsigned int uvdMPCCntl;
   unsigned int reserved0[4];
   unsigned int decoderID;
   unsigned int codec;

   unsigned int dmaHwCrc32Enable;
   unsigned int dmaHwCrc32Value;
   unsigned int dmaHwCrc32Value2;
} rvcn_dec_feedback_profiling_t;

typedef struct rvcn_dec_vp9_nmv_ctx_mask_s {
   unsigned short classes_mask[2];
   unsigned short bits_mask[2];
   unsigned char joints_mask;
   unsigned char sign_mask[2];
   unsigned char class0_mask[2];
   unsigned char class0_fp_mask[2];
   unsigned char fp_mask[2];
   unsigned char class0_hp_mask[2];
   unsigned char hp_mask[2];
   unsigned char reserve[11];
} rvcn_dec_vp9_nmv_ctx_mask_t;

typedef struct rvcn_dec_vp9_nmv_component_s {
   unsigned char sign;
   unsigned char classes[10];
   unsigned char class0[1];
   unsigned char bits[10];
   unsigned char class0_fp[2][3];
   unsigned char fp[3];
   unsigned char class0_hp;
   unsigned char hp;
} rvcn_dec_vp9_nmv_component_t;

typedef struct rvcn_dec_vp9_probs_s {
   rvcn_dec_vp9_nmv_ctx_mask_t nmvc_mask;
   unsigned char coef_probs[4][2][2][6][6][3];
   unsigned char y_mode_prob[4][9];
   unsigned char uv_mode_prob[10][9];
   unsigned char single_ref_prob[5][2];
   unsigned char switchable_interp_prob[4][2];
   unsigned char partition_prob[16][3];
   unsigned char inter_mode_probs[7][3];
   unsigned char mbskip_probs[3];
   unsigned char intra_inter_prob[4];
   unsigned char comp_inter_prob[5];
   unsigned char comp_ref_prob[5];
   unsigned char tx_probs_32x32[2][3];
   unsigned char tx_probs_16x16[2][2];
   unsigned char tx_probs_8x8[2][1];
   unsigned char mv_joints[3];
   rvcn_dec_vp9_nmv_component_t mv_comps[2];
} rvcn_dec_vp9_probs_t;

typedef struct rvcn_dec_vp9_probs_segment_s {
   union {
      rvcn_dec_vp9_probs_t probs;
      unsigned char probs_data[RDECODE_VP9_PROBS_DATA_SIZE];
   };

   union {
      struct {
         unsigned int feature_data[8];
         unsigned char tree_probs[7];
         unsigned char pred_probs[3];
         unsigned char abs_delta;
         unsigned char feature_mask[8];
      } seg;
      unsigned char segment_data[256];
   };
} rvcn_dec_vp9_probs_segment_t;

struct rvcn_av1_prob_funcs
{
   void (*init_mode_probs)(void * prob);
   void (*init_mv_probs)(void *prob);
   void (*default_coef_probs)(void *prob, int index);
};

typedef struct rvcn_dec_av1_fg_init_buf_s {
   short luma_grain_block[64][96];
   short cb_grain_block[32][48];
   short cr_grain_block[32][48];
   short scaling_lut_y[256];
   short scaling_lut_cb[256];
   short scaling_lut_cr[256];
   unsigned short temp_tile_left_seed[256];
} rvcn_dec_av1_fg_init_buf_t;

typedef struct rvcn_dec_av1_segment_fg_s {
   union {
      struct {
         unsigned char feature_data[128];
         unsigned char feature_mask[8];
      } seg;
      unsigned char segment_data[256];
   };
   rvcn_dec_av1_fg_init_buf_t fg_buf;
} rvcn_dec_av1_segment_fg_t;

struct jpeg_params {
   unsigned bsd_size;
   unsigned dt_pitch;
   unsigned dt_uv_pitch;
   unsigned dt_luma_top_offset;
   unsigned dt_chroma_top_offset;
   unsigned dt_chromav_top_offset;
   uint16_t crop_x;
   uint16_t crop_y;
   uint16_t crop_width;
   uint16_t crop_height;
};

#define RDECODE_VCN1_GPCOM_VCPU_CMD   0x2070c
#define RDECODE_VCN1_GPCOM_VCPU_DATA0 0x20710
#define RDECODE_VCN1_GPCOM_VCPU_DATA1 0x20714
#define RDECODE_VCN1_ENGINE_CNTL      0x20718

#define RDECODE_VCN2_GPCOM_VCPU_CMD   (0x503 << 2)
#define RDECODE_VCN2_GPCOM_VCPU_DATA0 (0x504 << 2)
#define RDECODE_VCN2_GPCOM_VCPU_DATA1 (0x505 << 2)
#define RDECODE_VCN2_ENGINE_CNTL      (0x506 << 2)

#define RDECODE_VCN2_5_GPCOM_VCPU_CMD   0x3c
#define RDECODE_VCN2_5_GPCOM_VCPU_DATA0 0x40
#define RDECODE_VCN2_5_GPCOM_VCPU_DATA1 0x44
#define RDECODE_VCN2_5_ENGINE_CNTL      0x9b4

#define RDECODE_SESSION_CONTEXT_SIZE (128 * 1024)

#endif
