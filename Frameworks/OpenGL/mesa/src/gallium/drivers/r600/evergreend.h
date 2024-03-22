/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *      Jerome Glisse
 */
#ifndef EVERGREEND_H
#define EVERGREEND_H

/* evergreen values */
#define EVERGREEN_CONFIG_REG_OFFSET                 0X00008000
#define EVERGREEN_CONFIG_REG_END                    0X0000AC00
#define EVERGREEN_CONTEXT_REG_OFFSET                0X00028000
#define EVERGREEN_CONTEXT_REG_END                   0X00029000
#define EVERGREEN_RESOURCE_OFFSET                   0x00030000
#define EVERGREEN_RESOURCE_END                      0x00038000
#define EVERGREEN_LOOP_CONST_OFFSET                 0x0003A200
#define EVERGREEN_LOOP_CONST_END                    0x0003A500
#define EVERGREEN_BOOL_CONST_OFFSET                 0x0003A500
#define EVERGREEN_BOOL_CONST_END                    0x0003A518
#define EVERGREEN_SAMPLER_OFFSET                    0X0003C000
#define EVERGREEN_SAMPLER_END                       0X0003C600

#define EVERGREEN_CTL_CONST_OFFSET                  0x0003CFF0
#define EVERGREEN_CTL_CONST_END                     0x0003FF0C

#define EVENT_TYPE_CS_PARTIAL_FLUSH            0x07
#define EVENT_TYPE_PS_PARTIAL_FLUSH            0x10
#define EVENT_TYPE_ZPASS_DONE                  0x15
#define EVENT_TYPE_CACHE_FLUSH_AND_INV_EVENT   0x16
#define EVENT_TYPE_SO_VGTSTREAMOUT_FLUSH       0x1f
#define EVENT_TYPE_VGT_FLUSH                   0x24
#define EVENT_TYPE_FLUSH_AND_INV_DB_META       0x2c
#define EVENT_TYPE_CS_DONE                     0x2f
#define EVENT_TYPE_PS_DONE                     0x30

#define		EVENT_TYPE(x)                           ((x) << 0)
#define		EVENT_INDEX(x)                          ((x) << 8)
                /* 0 - any non-TS event
		 * 1 - ZPASS_DONE
		 * 2 - SAMPLE_PIPELINESTAT
		 * 3 - SAMPLE_STREAMOUTSTAT*
		 * 4 - *S_PARTIAL_FLUSH
		 * 5 - TS events
		 * 6 - EOS events
		 */

#define R600_TEXEL_PITCH_ALIGNMENT_MASK        0x7

#define PKT3_NOP                               0x10
#define PKT3_DEALLOC_STATE                     0x14
#define PKT3_DISPATCH_DIRECT                   0x15
#define PKT3_DISPATCH_INDIRECT                 0x16
#define PKT3_INDIRECT_BUFFER_END               0x17
#define PKT3_SET_PREDICATION                   0x20
#define PKT3_REG_RMW                           0x21
#define PKT3_COND_EXEC                         0x22
#define PKT3_PRED_EXEC                         0x23
#define PKT3_DRAW_INDEX_2                      0x27
#define PKT3_CONTEXT_CONTROL                   0x28
#define PKT3_DRAW_INDEX_IMMD_BE                0x29
#define PKT3_INDEX_TYPE                        0x2A
#define PKT3_DRAW_INDEX                        0x2B
#define PKT3_DRAW_INDEX_AUTO                   0x2D
#define PKT3_DRAW_INDEX_IMMD                   0x2E
#define PKT3_NUM_INSTANCES                     0x2F
#define PKT3_STRMOUT_BUFFER_UPDATE             0x34
#define PKT3_INDIRECT_BUFFER_MP                0x38
#define PKT3_MEM_SEMAPHORE                     0x39
#define PKT3_MPEG_INDEX                        0x3A
#define PKT3_WAIT_REG_MEM                      0x3C
#define		WAIT_REG_MEM_EQUAL		3
#define PKT3_MEM_WRITE                         0x3D
#define		MEM_WRITE_CONFIRM		(1 << 17)
#define		MEM_WRITE_32_BITS		(1 << 18)
#define PKT3_INDIRECT_BUFFER                   0x32
#define PKT3_PFP_SYNC_ME		       0x42
#define PKT3_SURFACE_SYNC                      0x43
#define PKT3_ME_INITIALIZE                     0x44
#define PKT3_COND_WRITE                        0x45
#define PKT3_EVENT_WRITE                       0x46
#define PKT3_EVENT_WRITE_EOP                   0x47
#define PKT3_EVENT_WRITE_EOS                   0x48
#define PKT3_ONE_REG_WRITE                     0x57
#define PKT3_SET_CONFIG_REG                    0x68
#define PKT3_SET_CONTEXT_REG                   0x69
#define PKT3_SET_ALU_CONST                     0x6A
#define PKT3_SET_BOOL_CONST                    0x6B
#define PKT3_SET_LOOP_CONST                    0x6C
#define PKT3_SET_RESOURCE                      0x6D
#define PKT3_SET_SAMPLER                       0x6E
#define PKT3_SET_CTL_CONST                     0x6F
#define PKT3_SURFACE_BASE_UPDATE               0x73

#define PKT_TYPE_S(x)                   (((unsigned)(x) & 0x3) << 30)
#define PKT_TYPE_G(x)                   (((x) >> 30) & 0x3)
#define PKT_TYPE_C                      0x3FFFFFFF
#define PKT_COUNT_S(x)                  (((unsigned)(x) & 0x3FFF) << 16)
#define PKT_COUNT_G(x)                  (((x) >> 16) & 0x3FFF)
#define PKT_COUNT_C                     0xC000FFFF
#define PKT0_BASE_INDEX_S(x)            (((unsigned)(x) & 0xFFFF) << 0)
#define PKT0_BASE_INDEX_G(x)            (((x) >> 0) & 0xFFFF)
#define PKT0_BASE_INDEX_C               0xFFFF0000
#define PKT3_IT_OPCODE_S(x)             (((unsigned)(x) & 0xFF) << 8)
#define PKT3_IT_OPCODE_G(x)             (((x) >> 8) & 0xFF)
#define PKT3_IT_OPCODE_C                0xFFFF00FF
#define PKT3_PREDICATE(x)               (((x) >> 0) & 0x1)
#define PKT0(index, count) (PKT_TYPE_S(0) | PKT0_BASE_INDEX_S(index) | PKT_COUNT_S(count))

#define PKT3_CP_DMA					0x41
/* 1. header
 * 2. SRC_ADDR_LO [31:0] or DATA [31:0]
 * 3. CP_SYNC [31] | SRC_SEL [30:29] | ENGINE [27] | DST_SEL [21:20] | SRC_ADDR_HI [7:0]
 * 4. DST_ADDR_LO [31:0]
 * 5. DST_ADDR_HI [7:0]
 * 6. COMMAND [29:22] | BYTE_COUNT [20:0]
 */
#define PKT3_CP_DMA_CP_SYNC       (1 << 31)
#define PKT3_CP_DMA_SRC_SEL(x)       ((x) << 29)
/* 0 - SRC_ADDR
 * 1 - GDS (program SAS to 1 as well)
 * 2 - DATA
 */
#define PKT3_CP_DMA_DST_SEL(x)       ((x) << 20)
/* 0 - DST_ADDR
 * 1 - GDS (program DAS to 1 as well)
 */
/* COMMAND */
#define PKT3_CP_DMA_CMD_SRC_SWAP(x) ((x) << 23)
/* 0 - none
 * 1 - 8 in 16
 * 2 - 8 in 32
 * 3 - 8 in 64
 */
#define PKT3_CP_DMA_CMD_DST_SWAP(x) ((x) << 24)
/* 0 - none
 * 1 - 8 in 16
 * 2 - 8 in 32
 * 3 - 8 in 64
 */
#define PKT3_CP_DMA_CMD_SAS       (1 << 26)
/* 0 - memory
 * 1 - register
 */
#define PKT3_CP_DMA_CMD_DAS       (1 << 27)
/* 0 - memory
 * 1 - register
 */
#define PKT3_CP_DMA_CMD_SAIC      (1 << 28)
#define PKT3_CP_DMA_CMD_DAIC      (1 << 29)

#define PKT3_SET_APPEND_CNT                    0x75
/* 1. header
 * 2. COMMAND
 *  1:0 - SOURCE SEL
 *  15:2 - Reserved
 *  31:16 - WR_REG_OFFSET - context register to write source data to.
 *          (one of R_02872C_GDS_APPEND_COUNT_0-11)
 * 3. CONTROL
 *  (for source == mem)
 *  31:2 SRC_ADDRESS_LO
 *  0:1 SWAP
 *  (for source == GDS)
 *  31:0 GDS offset
 *  (for source == DATA)
 *  31:0 DATA
 *  (for source == REG)
 *  31:0 REG
 * 4. SRC_ADDRESS_HI[7:0]
 * kernel driver 2.44 only supports SRC == MEM.
 */
#define PKT3_SET_APPEND_CNT_SRC_SELECT(x) ((x) << 0)
/* source is from the data in CONTROL */
#define PKT3_SAC_SRC_SEL_DATA 0x0
/* source is from register */
#define PKT3_SAC_SRC_SEL_REG  0x1
/* source is from GDS offset in CONTROL */
#define PKT3_SAC_SRC_SEL_GDS  0x2
/* source is from memory address */
#define PKT3_SAC_SRC_SEL_MEM  0x3

/* Registers */
#define R_0084FC_CP_STRMOUT_CNTL		     0x0084FC
#define   S_0084FC_OFFSET_UPDATE_DONE(x)		(((unsigned)(x) & 0x1) << 0)
#define R_028B94_VGT_STRMOUT_CONFIG                                     0x028B94
#define   S_028B94_STREAMOUT_0_EN(x)                                  (((unsigned)(x) & 0x1) << 0)
#define   G_028B94_STREAMOUT_0_EN(x)                                  (((x) >> 0) & 0x1)
#define   C_028B94_STREAMOUT_0_EN                                     0xFFFFFFFE
#define   S_028B94_STREAMOUT_1_EN(x)                                  (((unsigned)(x) & 0x1) << 1)
#define   G_028B94_STREAMOUT_1_EN(x)                                  (((x) >> 1) & 0x1)
#define   C_028B94_STREAMOUT_1_EN                                     0xFFFFFFFD
#define   S_028B94_STREAMOUT_2_EN(x)                                  (((unsigned)(x) & 0x1) << 2)
#define   G_028B94_STREAMOUT_2_EN(x)                                  (((x) >> 2) & 0x1)
#define   C_028B94_STREAMOUT_2_EN                                     0xFFFFFFFB
#define   S_028B94_STREAMOUT_3_EN(x)                                  (((unsigned)(x) & 0x1) << 3)
#define   G_028B94_STREAMOUT_3_EN(x)                                  (((x) >> 3) & 0x1)
#define   C_028B94_STREAMOUT_3_EN                                     0xFFFFFFF7
#define   S_028B94_RAST_STREAM(x)                                     (((unsigned)(x) & 0x07) << 4)
#define   G_028B94_RAST_STREAM(x)                                     (((x) >> 4) & 0x07)
#define   C_028B94_RAST_STREAM                                        0xFFFFFF8F
#define   S_028B94_RAST_STREAM_MASK(x)                                (((unsigned)(x) & 0x0F) << 8) /* SI+ */
#define   G_028B94_RAST_STREAM_MASK(x)                                (((x) >> 8) & 0x0F)
#define   C_028B94_RAST_STREAM_MASK                                   0xFFFFF0FF
#define   S_028B94_USE_RAST_STREAM_MASK(x)                            (((unsigned)(x) & 0x1) << 31) /* SI+ */
#define   G_028B94_USE_RAST_STREAM_MASK(x)                            (((x) >> 31) & 0x1)
#define   C_028B94_USE_RAST_STREAM_MASK                               0x7FFFFFFF
#define R_028B98_VGT_STRMOUT_BUFFER_CONFIG                              0x028B98
#define   S_028B98_STREAM_0_BUFFER_EN(x)                              (((unsigned)(x) & 0x0F) << 0)
#define   G_028B98_STREAM_0_BUFFER_EN(x)                              (((x) >> 0) & 0x0F)
#define   C_028B98_STREAM_0_BUFFER_EN                                 0xFFFFFFF0
#define   S_028B98_STREAM_1_BUFFER_EN(x)                              (((unsigned)(x) & 0x0F) << 4)
#define   G_028B98_STREAM_1_BUFFER_EN(x)                              (((x) >> 4) & 0x0F)
#define   C_028B98_STREAM_1_BUFFER_EN                                 0xFFFFFF0F
#define   S_028B98_STREAM_2_BUFFER_EN(x)                              (((unsigned)(x) & 0x0F) << 8)
#define   G_028B98_STREAM_2_BUFFER_EN(x)                              (((x) >> 8) & 0x0F)
#define   C_028B98_STREAM_2_BUFFER_EN                                 0xFFFFF0FF
#define   S_028B98_STREAM_3_BUFFER_EN(x)                              (((unsigned)(x) & 0x0F) << 12)
#define   G_028B98_STREAM_3_BUFFER_EN(x)                              (((x) >> 12) & 0x0F)
#define   C_028B98_STREAM_3_BUFFER_EN                                 0xFFFF0FFF

#define EG_R_028A4C_PA_SC_MODE_CNTL_1                0x028A4C
#define   EG_S_028A4C_PS_ITER_SAMPLE(x)                 (((unsigned)(x) & 0x1) << 16)
#define   EG_S_028A4C_FORCE_EOV_CNTDWN_ENABLE(x)        (((unsigned)(x) & 0x1) << 25)
#define   EG_S_028A4C_FORCE_EOV_REZ_ENABLE(x)           (((unsigned)(x) & 0x1) << 26)
#define CM_R_028804_DB_EQAA                          0x00028804
#define   S_028804_MAX_ANCHOR_SAMPLES(x)                (((unsigned)(x) & 0x07) << 0)
#define   G_028804_MAX_ANCHOR_SAMPLES(x)                (((x) >> 0) & 0x07)
#define   C_028804_MAX_ANCHOR_SAMPLES                   0xFFFFFFF8
#define   S_028804_PS_ITER_SAMPLES(x)                   (((unsigned)(x) & 0x07) << 4)
#define   G_028804_PS_ITER_SAMPLES(x)                   (((x) >> 4) & 0x07)
#define   C_028804_PS_ITER_SAMPLES                      0xFFFFFF8F
#define   S_028804_MASK_EXPORT_NUM_SAMPLES(x)           (((unsigned)(x) & 0x07) << 8)
#define   G_028804_MASK_EXPORT_NUM_SAMPLES(x)           (((x) >> 8) & 0x07)
#define   C_028804_MASK_EXPORT_NUM_SAMPLES              0xFFFFF8FF
#define   S_028804_ALPHA_TO_MASK_NUM_SAMPLES(x)         (((unsigned)(x) & 0x07) << 12)
#define   G_028804_ALPHA_TO_MASK_NUM_SAMPLES(x)         (((x) >> 12) & 0x07)
#define   C_028804_ALPHA_TO_MASK_NUM_SAMPLES            0xFFFF8FFF
#define   S_028804_HIGH_QUALITY_INTERSECTIONS(x)        (((unsigned)(x) & 0x1) << 16)
#define   G_028804_HIGH_QUALITY_INTERSECTIONS(x)        (((x) >> 16) & 0x1)
#define   C_028804_HIGH_QUALITY_INTERSECTIONS           0xFFFEFFFF
#define   S_028804_INCOHERENT_EQAA_READS(x)             (((unsigned)(x) & 0x1) << 17)
#define   G_028804_INCOHERENT_EQAA_READS(x)             (((x) >> 17) & 0x1)
#define   C_028804_INCOHERENT_EQAA_READS                0xFFFDFFFF
#define   S_028804_INTERPOLATE_COMP_Z(x)                (((unsigned)(x) & 0x1) << 18)
#define   G_028804_INTERPOLATE_COMP_Z(x)                (((x) >> 18) & 0x1)
#define   C_028804_INTERPOLATE_COMP_Z                   0xFFFBFFFF
#define   S_028804_INTERPOLATE_SRC_Z(x)                 (((unsigned)(x) & 0x1) << 19)
#define   G_028804_INTERPOLATE_SRC_Z(x)                 (((x) >> 19) & 0x1)
#define   C_028804_INTERPOLATE_SRC_Z                    0xFFF7FFFF
#define   S_028804_STATIC_ANCHOR_ASSOCIATIONS(x)        (((unsigned)(x) & 0x1) << 20)
#define   G_028804_STATIC_ANCHOR_ASSOCIATIONS(x)        (((x) >> 20) & 0x1)
#define   C_028804_STATIC_ANCHOR_ASSOCIATIONS           0xFFEFFFFF
#define   S_028804_ALPHA_TO_MASK_EQAA_DISABLE(x)        (((unsigned)(x) & 0x1) << 21)
#define   G_028804_ALPHA_TO_MASK_EQAA_DISABLE(x)        (((x) >> 21) & 0x1)
#define   C_028804_ALPHA_TO_MASK_EQAA_DISABLE           0xFFDFFFFF
#define   S_028804_OVERRASTERIZATION_AMOUNT(x)          (((unsigned)(x) & 0x07) << 24)
#define   G_028804_OVERRASTERIZATION_AMOUNT(x)          (((x) >> 24) & 0x07)
#define   C_028804_OVERRASTERIZATION_AMOUNT             0xF8FFFFFF
#define   S_028804_ENABLE_POSTZ_OVERRASTERIZATION(x)    (((unsigned)(x) & 0x1) << 27)
#define   G_028804_ENABLE_POSTZ_OVERRASTERIZATION(x)    (((x) >> 27) & 0x1)
#define   C_028804_ENABLE_POSTZ_OVERRASTERIZATION       0xF7FFFFFF
#define CM_R_028BDC_PA_SC_LINE_CNTL                  0x28bdc
#define   S_028BDC_EXPAND_LINE_WIDTH(x)                (((unsigned)(x) & 0x1) << 9)
#define   G_028BDC_EXPAND_LINE_WIDTH(x)                (((x) >> 9) & 0x1)
#define   C_028BDC_EXPAND_LINE_WIDTH                   0xFFFFFDFF
#define   S_028BDC_LAST_PIXEL(x)                       (((unsigned)(x) & 0x1) << 10)
#define   G_028BDC_LAST_PIXEL(x)                       (((x) >> 10) & 0x1)
#define   C_028BDC_LAST_PIXEL                          0xFFFFFBFF
#define   S_028BDC_PERPENDICULAR_ENDCAP_ENA(x)         (((unsigned)(x) & 0x1) << 11)
#define   G_028BDC_PERPENDICULAR_ENDCAP_ENA(x)         (((x) >> 11) & 0x1)
#define   C_028BDC_PERPENDICULAR_ENDCAP_ENA            0xFFFFF7FF
#define   S_028BDC_DX10_DIAMOND_TEST_ENA(x)            (((unsigned)(x) & 0x1) << 12)
#define   G_028BDC_DX10_DIAMOND_TEST_ENA(x)            (((x) >> 12) & 0x1)
#define   C_028BDC_DX10_DIAMOND_TEST_ENA               0xFFFFEFFF
#define CM_R_028BE0_PA_SC_AA_CONFIG                  0x28be0
#define   S_028BE0_MSAA_NUM_SAMPLES(x)                 (((unsigned)(x) & 0x07) << 0)
#define   G_028BE0_MSAA_NUM_SAMPLES(x)                 (((x) >> 0) & 0x07)
#define   C_028BE0_MSAA_NUM_SAMPLES                    0xFFFFFFF8
#define   S_028BE0_AA_MASK_CENTROID_DTMN(x)            (((unsigned)(x) & 0x1) << 4)
#define   G_028BE0_AA_MASK_CENTROID_DTMN(x)            (((x) >> 4) & 0x1)
#define   C_028BE0_AA_MASK_CENTROID_DTMN               0xFFFFFFEF
#define   S_028BE0_MAX_SAMPLE_DIST(x)                  (((unsigned)(x) & 0x0F) << 13)
#define   G_028BE0_MAX_SAMPLE_DIST(x)                  (((x) >> 13) & 0x0F)
#define   C_028BE0_MAX_SAMPLE_DIST                     0xFFFE1FFF
#define   S_028BE0_MSAA_EXPOSED_SAMPLES(x)             (((unsigned)(x) & 0x07) << 20)
#define   G_028BE0_MSAA_EXPOSED_SAMPLES(x)             (((x) >> 20) & 0x07)
#define   C_028BE0_MSAA_EXPOSED_SAMPLES                0xFF8FFFFF
#define   S_028BE0_DETAIL_TO_EXPOSED_MODE(x)           (((unsigned)(x) & 0x03) << 24)
#define   G_028BE0_DETAIL_TO_EXPOSED_MODE(x)           (((x) >> 24) & 0x03)
#define   C_028BE0_DETAIL_TO_EXPOSED_MODE              0xFCFFFFFF
#define CM_R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0 0x28bf8
#define CM_R_028C08_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0 0x28c08
#define CM_R_028C18_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0 0x28c18
#define CM_R_028C28_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0 0x28c28
#define R_008960_VGT_STRMOUT_BUFFER_FILLED_SIZE_0    0x008960 /* read-only */
#define R_008964_VGT_STRMOUT_BUFFER_FILLED_SIZE_1    0x008964 /* read-only */
#define R_008968_VGT_STRMOUT_BUFFER_FILLED_SIZE_2    0x008968 /* read-only */
#define R_00896C_VGT_STRMOUT_BUFFER_FILLED_SIZE_3    0x00896C /* read-only */
#define R_008C00_SQ_CONFIG                           0x00008C00
#define   S_008C00_VC_ENABLE(x)                        (((unsigned)(x) & 0x1) << 0)
#define   G_008C00_VC_ENABLE(x)                        (((x) >> 0) & 0x1)
#define   C_008C00_VC_ENABLE(x)                        0xFFFFFFFE
#define   S_008C00_EXPORT_SRC_C(x)                     (((unsigned)(x) & 0x1) << 1)
#define   G_008C00_EXPORT_SRC_C(x)                     (((x) >> 1) & 0x1)
#define   C_008C00_EXPORT_SRC_C(x)                     0xFFFFFFFD
/* different */
#define   S_008C00_CS_PRIO(x)                          (((unsigned)(x) & 0x3) << 18)
#define   G_008C00_CS_PRIO(x)                          (((x) >> 18) & 0x3)
#define   C_008C00_CS_PRIO(x)                          0xFFF3FFFF
#define   S_008C00_LS_PRIO(x)                          (((unsigned)(x) & 0x3) << 20)
#define   G_008C00_LS_PRIO(x)                          (((x) >> 20) & 0x3)
#define   C_008C00_LS_PRIO(x)                          0xFFCFFFFF
#define   S_008C00_HS_PRIO(x)                          (((unsigned)(x) & 0x3) << 22)
#define   G_008C00_HS_PRIO(x)                          (((x) >> 22) & 0x3)
#define   C_008C00_HS_PRIO(x)                          0xFF3FFFFF
#define   S_008C00_PS_PRIO(x)                          (((unsigned)(x) & 0x3) << 24)
#define   G_008C00_PS_PRIO(x)                          (((x) >> 24) & 0x3)
#define   C_008C00_PS_PRIO(x)                          0xFCFFFFFF
#define   S_008C00_VS_PRIO(x)                          (((unsigned)(x) & 0x3) << 26)
#define   G_008C00_VS_PRIO(x)                          (((x) >> 26) & 0x3)
#define   C_008C00_VS_PRIO(x)                          0xF3FFFFFF
#define   S_008C00_GS_PRIO(x)                          (((unsigned)(x) & 0x3) << 28)
#define   G_008C00_GS_PRIO(x)                          (((x) >> 28) & 0x3)
#define   C_008C00_GS_PRIO(x)                          0xCFFFFFFF
#define   S_008C00_ES_PRIO(x)                          (((unsigned)(x) & 0x3) << 30)
#define   G_008C00_ES_PRIO(x)                          (((x) >> 30) & 0x3)
#define   C_008C00_ES_PRIO(x)                          0x3FFFFFFF
#define R_008C04_SQ_GPR_RESOURCE_MGMT_1              0x00008C04
#define   S_008C04_NUM_PS_GPRS(x)                      (((unsigned)(x) & 0xFF) << 0)
#define   G_008C04_NUM_PS_GPRS(x)                      (((x) >> 0) & 0xFF)
#define   C_008C04_NUM_PS_GPRS(x)                      0xFFFFFF00
#define   S_008C04_NUM_VS_GPRS(x)                      (((unsigned)(x) & 0xFF) << 16)
#define   G_008C04_NUM_VS_GPRS(x)                      (((x) >> 16) & 0xFF)
#define   C_008C04_NUM_VS_GPRS(x)                      0xFF00FFFF
#define   S_008C04_NUM_CLAUSE_TEMP_GPRS(x)             (((unsigned)(x) & 0xF) << 28)
#define   G_008C04_NUM_CLAUSE_TEMP_GPRS(x)             (((x) >> 28) & 0xF)
#define   C_008C04_NUM_CLAUSE_TEMP_GPRS(x)             0x0FFFFFFF
#define R_008C08_SQ_GPR_RESOURCE_MGMT_2              0x00008C08
#define   S_008C08_NUM_GS_GPRS(x)                      (((unsigned)(x) & 0xFF) << 0)
#define   G_008C08_NUM_GS_GPRS(x)                      (((x) >> 0) & 0xFF)
#define   C_008C08_NUM_GS_GPRS(x)                      0xFFFFFF00
#define   S_008C08_NUM_ES_GPRS(x)                      (((unsigned)(x) & 0xFF) << 16)
#define   G_008C08_NUM_ES_GPRS(x)                      (((x) >> 16) & 0xFF)
#define   C_008C08_NUM_ES_GPRS(x)                      0xFF00FFFF
#define R_008C0C_SQ_GPR_RESOURCE_MGMT_3              0x00008C0C
#define   S_008C0C_NUM_HS_GPRS(x)                      (((unsigned)(x) & 0xFF) << 0)
#define   G_008C0C_NUM_HS_GPRS(x)                      (((x) >> 0) & 0xFF)
#define   C_008C0C_NUM_HS_GPRS(x)                      0xFFFFFF00
#define   S_008C0C_NUM_LS_GPRS(x)                      (((unsigned)(x) & 0xFF) << 16)
#define   G_008C0C_NUM_LS_GPRS(x)                      (((x) >> 16) & 0xFF)
#define   C_008C0C_NUM_LS_GPRS(x)                      0xFF00FFFF

#define R_008C10_SQ_GLOBAL_GPR_RESOURCE_MGMT_1       0x00008C10
#define R_008C14_SQ_GLOBAL_GPR_RESOURCE_MGMT_2       0x00008C14

#define R_008C18_SQ_THREAD_RESOURCE_MGMT_1           0x00008C18
#define   S_008C18_NUM_PS_THREADS(x)                   (((unsigned)(x) & 0xFF) << 0)
#define   G_008C18_NUM_PS_THREADS(x)                   (((x) >> 0) & 0xFF)
#define   C_008C18_NUM_PS_THREADS(x)                   0xFFFFFF00
#define   S_008C18_NUM_VS_THREADS(x)                   (((unsigned)(x) & 0xFF) << 8)
#define   G_008C18_NUM_VS_THREADS(x)                   (((x) >> 8) & 0xFF)
#define   C_008C18_NUM_VS_THREADS(x)                   0xFFFF00FF
#define   S_008C18_NUM_GS_THREADS(x)                   (((unsigned)(x) & 0xFF) << 16)
#define   G_008C18_NUM_GS_THREADS(x)                   (((x) >> 16) & 0xFF)
#define   C_008C18_NUM_GS_THREADS(x)                   0xFF00FFFF
#define   S_008C18_NUM_ES_THREADS(x)                   (((unsigned)(x) & 0xFF) << 24)
#define   G_008C18_NUM_ES_THREADS(x)                   (((x) >> 24) & 0xFF)
#define   C_008C18_NUM_ES_THREADS(x)                   0x00FFFFFF
#define R_008C1C_SQ_THREAD_RESOURCE_MGMT_2             0x00008C1C
#define   S_008C1C_NUM_HS_THREADS(x)                   (((unsigned)(x) & 0xFF) << 0)
#define   G_008C1C_NUM_HS_THREADS(x)                   (((x) >> 0) & 0xFF)
#define   C_008C1C_NUM_HS_THREADS(x)                   0xFFFFFF00
#define   S_008C1C_NUM_LS_THREADS(x)                   (((unsigned)(x) & 0xFF) << 8)
#define   G_008C1C_NUM_LS_THREADS(x)                   (((x) >> 8) & 0xFF)
#define   C_008C1C_NUM_LS_THREADS(x)                   0xFFFF00FF
#define R_008C20_SQ_STACK_RESOURCE_MGMT_1            0x00008C20
#define   S_008C20_NUM_PS_STACK_ENTRIES(x)             (((unsigned)(x) & 0xFFF) << 0)
#define   G_008C20_NUM_PS_STACK_ENTRIES(x)             (((x) >> 0) & 0xFFF)
#define   C_008C20_NUM_PS_STACK_ENTRIES(x)             0xFFFFF000
#define   S_008C20_NUM_VS_STACK_ENTRIES(x)             (((unsigned)(x) & 0xFFF) << 16)
#define   G_008C20_NUM_VS_STACK_ENTRIES(x)             (((x) >> 16) & 0xFFF)
#define   C_008C20_NUM_VS_STACK_ENTRIES(x)             0xF000FFFF
#define R_008C24_SQ_STACK_RESOURCE_MGMT_2            0x00008C24
#define   S_008C24_NUM_GS_STACK_ENTRIES(x)             (((unsigned)(x) & 0xFFF) << 0)
#define   G_008C24_NUM_GS_STACK_ENTRIES(x)             (((x) >> 0) & 0xFFF)
#define   C_008C24_NUM_GS_STACK_ENTRIES(x)             0xFFFFF000
#define   S_008C24_NUM_ES_STACK_ENTRIES(x)             (((unsigned)(x) & 0xFFF) << 16)
#define   G_008C24_NUM_ES_STACK_ENTRIES(x)             (((x) >> 16) & 0xFFF)
#define   C_008C24_NUM_ES_STACK_ENTRIES(x)             0xF000FFFF
#define R_008C28_SQ_STACK_RESOURCE_MGMT_3            0x00008C28
#define   S_008C28_NUM_HS_STACK_ENTRIES(x)             (((unsigned)(x) & 0xFFF) << 0)
#define   G_008C28_NUM_HS_STACK_ENTRIES(x)             (((x) >> 0) & 0xFFF)
#define   C_008C28_NUM_HS_STACK_ENTRIES(x)             0xFFFFF000
#define   S_008C28_NUM_LS_STACK_ENTRIES(x)             (((unsigned)(x) & 0xFFF) << 16)
#define   G_008C28_NUM_LS_STACK_ENTRIES(x)             (((x) >> 16) & 0xFFF)
#define   C_008C28_NUM_LS_STACK_ENTRIES(x)             0xF000FFFF
#define R_008E2C_SQ_LDS_RESOURCE_MGMT                0x00008E2C
#define   S_008E2C_NUM_PS_LDS(x)                       (((unsigned)(x) & 0xFFFF) << 0)
#define   G_008E2C_NUM_PS_LDS(x)                       (((x) >> 0) & 0xFFFF)
#define   C_008E2C_NUM_PS_LDS(x)                       0x0000FFFF
#define   S_008E2C_NUM_LS_LDS(x)                       (((unsigned)(x) & 0xFFFF) << 16)
#define   G_008E2C_NUM_LS_LDS(x)                       (((x) >> 16) & 0xFFFF)
#define   C_008E2C_NUM_LS_LDS(x)                       0xFFFF0000

#define R_008C40_SQ_ESGS_RING_BASE                    0x00008C40
#define R_008C44_SQ_ESGS_RING_SIZE                    0x00008C44
#define R_008C48_SQ_GSVS_RING_BASE                    0x00008C48
#define R_008C4C_SQ_GSVS_RING_SIZE                    0x00008C4C

#define R_008CF0_SQ_MS_FIFO_SIZES                     0x00008CF0
#define   S_008CF0_CACHE_FIFO_SIZE(x)                  (((unsigned)(x) & 0xFF) << 0)
#define   G_008CF0_CACHE_FIFO_SIZE(x)                  (((x) >> 0) & 0xFF)
#define   C_008CF0_CACHE_FIFO_SIZE(x)                  0xFFFFFF00
#define   S_008CF0_FETCH_FIFO_HIWATER(x)               (((unsigned)(x) & 0x1F) << 8)
#define   G_008CF0_FETCH_FIFO_HIWATER(x)               (((x) >> 8) & 0x1F)
#define   C_008CF0_FETCH_FIFO_HIWATER(x)               0xFFFFE0FF
#define   S_008CF0_DONE_FIFO_HIWATER(x)                (((unsigned)(x) & 0xFF) << 16)
#define   G_008CF0_DONE_FIFO_HIWATER(x)                (((x) >> 16) & 0xFF)
#define   C_008CF0_DONE_FIFO_HIWATER(x)                0xFF00FFFF
#define   S_008CF0_ALU_UPDATE_FIFO_HIWATER(x)          (((unsigned)(x) & 0x1F) << 24)
#define   G_008CF0_ALU_UPDATE_FIFO_HIWATER(x)          (((x) >> 24) & 0x1F)
#define   C_008CF0_ALU_UPDATE_FIFO_HIWATER(x)          0xE0FFFFFF

#define R_008E20_SQ_STATIC_THREAD_MGMT1               0x8E20
#define R_008E24_SQ_STATIC_THREAD_MGMT2               0x8E24
#define R_008E28_SQ_STATIC_THREAD_MGMT3               0x8E28

#define   R_00899C_VGT_COMPUTE_START_X                 0x0000899C
#define   R_0089A0_VGT_COMPUTE_START_Y                 0x000089A0
#define   R_0089A4_VGT_COMPUTE_START_Z                 0x000089A4
#define   R_0089AC_VGT_COMPUTE_THREAD_GROUP_SIZE       0x000089AC

#define R_009100_SPI_CONFIG_CNTL                      0x00009100
#define R_00913C_SPI_CONFIG_CNTL_1                    0x0000913C
#define   S_00913C_VTX_DONE_DELAY(x)                (((unsigned)(x) & 0xF) << 0)
#define   G_00913C_VTX_DONE_DELAY(x)                (((x) >> 0) & 0xF )
#define   C_00913C_VTX_DONE_DELAY(x)                0xFFFFFFF0


#define R_028C64_CB_COLOR0_PITCH                      0x028C64
#define   S_028C64_PITCH_TILE_MAX(x)                   (((unsigned)(x) & 0x7FF) << 0)
#define   G_028C64_PITCH_TILE_MAX(x)                   (((x) >> 0) & 0x7FF)
#define   C_028C64_PITCH_TILE_MAX                      0xFFFFF800
#define R_028C68_CB_COLOR0_SLICE                      0x028C68
#define   S_028C68_SLICE_TILE_MAX(x)                   (((unsigned)(x) & 0x3FFFFF) << 0)
#define   G_028C68_SLICE_TILE_MAX(x)                   (((x) >> 0) & 0x3FFFFF)
#define   C_028C68_SLICE_TILE_MAX                      0xFFC00000
#define R_028C70_CB_COLOR0_INFO                      0x028C70
#define   S_028C70_ENDIAN(x)                           (((unsigned)(x) & 0x3) << 0)
#define   G_028C70_ENDIAN(x)                           (((x) >> 0) & 0x3)
#define   C_028C70_ENDIAN                              0xFFFFFFFC
#define   S_028C70_FORMAT(x)                           (((unsigned)(x) & 0x3F) << 2)
#define   G_028C70_FORMAT(x)                           (((x) >> 2) & 0x3F)
#define   C_028C70_FORMAT                              0xFFFFFF03
#define     V_028C70_COLOR_INVALID                     0x00000000
#define     V_028C70_COLOR_8                           0x00000001
#define     V_028C70_COLOR_4_4                         0x00000002
#define     V_028C70_COLOR_3_3_2                       0x00000003
#define     V_028C70_COLOR_16                          0x00000005
#define     V_028C70_COLOR_16_FLOAT                    0x00000006
#define     V_028C70_COLOR_8_8                         0x00000007
#define     V_028C70_COLOR_5_6_5                       0x00000008
#define     V_028C70_COLOR_6_5_5                       0x00000009
#define     V_028C70_COLOR_1_5_5_5                     0x0000000A
#define     V_028C70_COLOR_4_4_4_4                     0x0000000B
#define     V_028C70_COLOR_5_5_5_1                     0x0000000C
#define     V_028C70_COLOR_32                          0x0000000D
#define     V_028C70_COLOR_32_FLOAT                    0x0000000E
#define     V_028C70_COLOR_16_16                       0x0000000F
#define     V_028C70_COLOR_16_16_FLOAT                 0x00000010
#define     V_028C70_COLOR_8_24                        0x00000011
#define     V_028C70_COLOR_8_24_FLOAT                  0x00000012
#define     V_028C70_COLOR_24_8                        0x00000013
#define     V_028C70_COLOR_24_8_FLOAT                  0x00000014
#define     V_028C70_COLOR_10_11_11                    0x00000015
#define     V_028C70_COLOR_10_11_11_FLOAT              0x00000016
#define     V_028C70_COLOR_11_11_10                    0x00000017
#define     V_028C70_COLOR_11_11_10_FLOAT              0x00000018
#define     V_028C70_COLOR_2_10_10_10                  0x00000019
#define     V_028C70_COLOR_8_8_8_8                     0x0000001A
#define     V_028C70_COLOR_10_10_10_2                  0x0000001B
#define     V_028C70_COLOR_X24_8_32_FLOAT              0x0000001C
#define     V_028C70_COLOR_32_32                       0x0000001D
#define     V_028C70_COLOR_32_32_FLOAT                 0x0000001E
#define     V_028C70_COLOR_16_16_16_16                 0x0000001F
#define     V_028C70_COLOR_16_16_16_16_FLOAT           0x00000020
#define     V_028C70_COLOR_32_32_32_32                 0x00000022
#define     V_028C70_COLOR_32_32_32_32_FLOAT           0x00000023
#define     V_028C70_COLOR_32_32_32_FLOAT              0x00000030
#define   S_028C70_ARRAY_MODE(x)                       (((unsigned)(x) & 0xF) << 8)
#define   G_028C70_ARRAY_MODE(x)                       (((x) >> 8) & 0xF)
#define   C_028C70_ARRAY_MODE                          0xFFFFF0FF
#define     V_028C70_ARRAY_LINEAR_GENERAL              0x00000000
#define     V_028C70_ARRAY_LINEAR_ALIGNED              0x00000001
#define     V_028C70_ARRAY_1D_TILED_THIN1              0x00000002
#define     V_028C70_ARRAY_2D_TILED_THIN1              0x00000004
#define   S_028C70_NUMBER_TYPE(x)                      (((unsigned)(x) & 0x7) << 12)
#define   G_028C70_NUMBER_TYPE(x)                      (((x) >> 12) & 0x7)
#define   C_028C70_NUMBER_TYPE                         0xFFFF8FFF
#define     V_028C70_NUMBER_UNORM                      0x00000000
#define     V_028C70_NUMBER_SNORM                      0x00000001
#define     V_028C70_NUMBER_USCALED                    0x00000002
#define     V_028C70_NUMBER_SSCALED                    0x00000003
#define     V_028C70_NUMBER_UINT                       0x00000004
#define     V_028C70_NUMBER_SINT                       0x00000005
#define     V_028C70_NUMBER_SRGB                       0x00000006
#define     V_028C70_NUMBER_FLOAT                      0x00000007
#define   S_028C70_COMP_SWAP(x)                        (((unsigned)(x) & 0x3) << 15)
#define   G_028C70_COMP_SWAP(x)                        (((x) >> 15) & 0x3)
#define   C_028C70_COMP_SWAP                           0xFFFE7FFF
#define     V_028C70_SWAP_STD                          0x00000000
#define     V_028C70_SWAP_ALT                          0x00000001
#define     V_028C70_SWAP_STD_REV                      0x00000002
#define     V_028C70_SWAP_ALT_REV                      0x00000003
#define   S_028C70_FAST_CLEAR(x)                       (((unsigned)(x) & 0x1) << 17)
#define   G_028C70_FAST_CLEAR(x)                       (((x) >> 17) & 0x1)
#define   C_028C70_FAST_CLEAR                          0xFFFDFFFF
#define   S_028C70_COMPRESSION(x)                      (((unsigned)(x) & 0x1) << 18)
#define   G_028C70_COMPRESSION(x)                      (((x) >> 18) & 0x1)
#define   C_028C70_COMPRESSION                         0xFFFBFFFF
#define   S_028C70_BLEND_CLAMP(x)                      (((unsigned)(x) & 0x1) << 19)
#define   G_028C70_BLEND_CLAMP(x)                      (((x) >> 19) & 0x1)
#define   C_028C70_BLEND_CLAMP                         0xFFF7FFFF
#define   S_028C70_BLEND_BYPASS(x)                     (((unsigned)(x) & 0x1) << 20)
#define   G_028C70_BLEND_BYPASS(x)                     (((x) >> 20) & 0x1)
#define   C_028C70_BLEND_BYPASS                        0xFFEFFFFF
#define   S_028C70_SIMPLE_FLOAT(x)                     (((unsigned)(x) & 0x1) << 21)
#define   G_028C70_SIMPLE_FLOAT(x)                     (((x) >> 21) & 0x1)
#define   C_028C70_SIMPLE_FLOAT                        0xFFDFFFFF
#define   S_028C70_ROUND_MODE(x)                       (((unsigned)(x) & 0x1) << 22)
#define   G_028C70_ROUND_MODE(x)                       (((x) >> 22) & 0x1)
#define   C_028C70_ROUND_MODE                          0xFFBFFFFF
#define   S_028C70_TILE_COMPACT(x)                     (((unsigned)(x) & 0x1) << 23)
#define   G_028C70_TILE_COMPACT(x)                     (((x) >> 23) & 0x1)
#define   C_028C70_TILE_COMPACT                        0xFF7FFFFF
#define   S_028C70_SOURCE_FORMAT(x)                    (((unsigned)(x) & 0x3) << 24)
#define   G_028C70_SOURCE_FORMAT(x)                    (((x) >> 24) & 0x3)
#define   C_028C70_SOURCE_FORMAT                       0xFCFFFFFF
#define     V_028C70_EXPORT_4C_32BPC                   0x0
#define     V_028C70_EXPORT_4C_16BPC                   0x1
#define     V_028C70_EXPORT_2C_32BPC                   0x2 /* Do not use */
#define   S_028C70_RAT(x)                              (((unsigned)(x) & 0x1) << 26)
#define   G_028C70_RAT(x)                              (((x) >> 26) & 0x1)
#define   C_028C70_RAT                                 0xFBFFFFFF
/* RESOURCE_TYPE is only used for compute shaders */
#define   S_028C70_RESOURCE_TYPE(x)                    (((unsigned)(x) & 0x7) << 27)
#define   G_028C70_RESOURCE_TYPE(x)                    (((x) >> 27) & 0x7)
#define   C_028C70_RESOURCE_TYPE                       0xC7FFFFFF
#define     V_028C70_BUFFER                            0x0
#define     V_028C70_TEXTURE1D                         0x1
#define     V_028C70_TEXTURE1DARRAY                    0x2
#define     V_028C70_TEXTURE2D                         0x3
#define     V_028C70_TEXTURE2DARRAY                    0x4
#define     V_028C70_TEXTURE3D                         0x5

#define R_028C74_CB_COLOR0_ATTRIB                      0x028C74
#define   S_028C74_NON_DISP_TILING_ORDER(x)            (((unsigned)(x) & 0x1) << 4)
#define   G_028C74_NON_DISP_TILING_ORDER(x)            (((x) >> 4) & 0x1)
#define   C_028C74_NON_DISP_TILING_ORDER               0xFFFFFFEF
#define   S_028C74_TILE_SPLIT(x)                       (((unsigned)(x) & 0xf) << 5)
#define   S_028C74_NUM_BANKS(x)                        (((unsigned)(x) & 0x3) << 10)
#define   S_028C74_BANK_WIDTH(x)                       (((unsigned)(x) & 0x3) << 13)
#define   S_028C74_BANK_HEIGHT(x)                      (((unsigned)(x) & 0x3) << 16)
#define   S_028C74_MACRO_TILE_ASPECT(x)                (((unsigned)(x) & 0x3) << 19)
#define   S_028C74_FMASK_BANK_HEIGHT(x)                (((unsigned)(x) & 0x3) << 22)
#define   S_028C74_NUM_SAMPLES(x)                      (((unsigned)(x) & 0x7) << 24) /* cayman only */
#define   S_028C74_NUM_FRAGMENTS(x)                    (((unsigned)(x) & 0x3) << 27) /* cayman only */
#define   S_028C74_FORCE_DST_ALPHA_1(x)                (((unsigned)(x) & 0x1) << 31) /* cayman only */

#define R_028C78_CB_COLOR0_DIM                         0x028C78
#define   S_028C78_WIDTH_MAX(x)                        (((unsigned)(x) & 0xFFFF) << 0)
#define   G_028C78_WIDTH_MAX(x)                        (((x) >> 0) & 0xFFFF)
#define   C_028C78_WIDTH_MAX                           0xFFFF0000
#define   S_028C78_HEIGHT_MAX(x)                       (((unsigned)(x) & 0xFFFF) << 16)
#define   G_028C78_HEIGHT_MAX(x)                       (((x) >> 16) & 0xFFFF)
#define   C_028C78_HEIGHT_MAX                          0x0000FFFF


/* alpha same */
#define R_028410_SX_ALPHA_TEST_CONTROL               0x028410
#define   S_028410_ALPHA_FUNC(x)                       (((unsigned)(x) & 0x7) << 0)
#define   G_028410_ALPHA_FUNC(x)                       (((x) >> 0) & 0x7)
#define   C_028410_ALPHA_FUNC                          0xFFFFFFF8
#define   S_028410_ALPHA_TEST_ENABLE(x)                (((unsigned)(x) & 0x1) << 3)
#define   G_028410_ALPHA_TEST_ENABLE(x)                (((x) >> 3) & 0x1)
#define   C_028410_ALPHA_TEST_ENABLE                   0xFFFFFFF7
#define   S_028410_ALPHA_TEST_BYPASS(x)                (((unsigned)(x) & 0x1) << 8)
#define   G_028410_ALPHA_TEST_BYPASS(x)                (((x) >> 8) & 0x1)
#define   C_028410_ALPHA_TEST_BYPASS                   0xFFFFFEFF

#define R_0286EC_SPI_COMPUTE_NUM_THREAD_X            0x0286EC
#define R_0286F0_SPI_COMPUTE_NUM_THREAD_Y            0x0286F0
#define R_0286F4_SPI_COMPUTE_NUM_THREAD_Z            0x0286F4
#define R_028B6C_VGT_TF_PARAM                        0x028B6C
#define   S_028B6C_TYPE(x)                                            (((unsigned)(x) & 0x03) << 0)
#define   G_028B6C_TYPE(x)                                            (((x) >> 0) & 0x03)
#define   C_028B6C_TYPE                                               0xFFFFFFFC
#define     V_028B6C_TESS_ISOLINE                                   0x00
#define     V_028B6C_TESS_TRIANGLE                                  0x01
#define     V_028B6C_TESS_QUAD                                      0x02
#define   S_028B6C_PARTITIONING(x)                                    (((unsigned)(x) & 0x07) << 2)
#define   G_028B6C_PARTITIONING(x)                                    (((x) >> 2) & 0x07)
#define   C_028B6C_PARTITIONING                                       0xFFFFFFE3
#define     V_028B6C_PART_INTEGER                                   0x00
#define     V_028B6C_PART_POW2                                      0x01
#define     V_028B6C_PART_FRAC_ODD                                  0x02
#define     V_028B6C_PART_FRAC_EVEN                                 0x03
#define   S_028B6C_TOPOLOGY(x)                                        (((unsigned)(x) & 0x07) << 5)
#define   G_028B6C_TOPOLOGY(x)                                        (((x) >> 5) & 0x07)
#define   C_028B6C_TOPOLOGY                                           0xFFFFFF1F
#define     V_028B6C_OUTPUT_POINT                                   0x00
#define     V_028B6C_OUTPUT_LINE                                    0x01
#define     V_028B6C_OUTPUT_TRIANGLE_CW                             0x02
#define     V_028B6C_OUTPUT_TRIANGLE_CCW                            0x03
#define   S_028B6C_RESERVED_REDUC_AXIS(x)                             (((unsigned)(x) & 0x1) << 8)
#define   G_028B6C_RESERVED_REDUC_AXIS(x)                             (((x) >> 8) & 0x1)
#define   C_028B6C_RESERVED_REDUC_AXIS                                0xFFFFFEFF
#define   S_028B6C_BUFFER_ACCESS_MODE(x)                              (((unsigned)(x) & 0x1) << 9)
#define   G_028B6C_BUFFER_ACCESS_MODE(x)                              (((x) >> 9) & 0x1)
#define   C_028B6C_BUFFER_ACCESS_MODE                                 0xFFFFFDFF
#define     V_028B6C_PATCH_MAJOR                                    0x00
#define     V_028B6C_TF_MAJOR                                       0x01
#define   S_028B6C_NUM_DS_WAVES_PER_SIMD(x)                           (((unsigned)(x) & 0xf) << 10)
#define   G_028B6C_NUM_DS_WAVES_PER_SIMD(x)                           (((x) >> 10) & 0xF)
#define   C_028B6C_NUM_DS_WAVES_PER_SIMD                              0xFFFFC3FF

#define R_028B74_VGT_DISPATCH_INITIATOR              0x028B74

#define R_028800_DB_DEPTH_CONTROL                    0x028800
#define   S_028800_STENCIL_ENABLE(x)                   (((unsigned)(x) & 0x1) << 0)
#define   G_028800_STENCIL_ENABLE(x)                   (((x) >> 0) & 0x1)
#define   C_028800_STENCIL_ENABLE                      0xFFFFFFFE
#define   S_028800_Z_ENABLE(x)                         (((unsigned)(x) & 0x1) << 1)
#define   G_028800_Z_ENABLE(x)                         (((x) >> 1) & 0x1)
#define   C_028800_Z_ENABLE                            0xFFFFFFFD
#define   S_028800_Z_WRITE_ENABLE(x)                   (((unsigned)(x) & 0x1) << 2)
#define   G_028800_Z_WRITE_ENABLE(x)                   (((x) >> 2) & 0x1)
#define   C_028800_Z_WRITE_ENABLE                      0xFFFFFFFB
#define   S_028800_ZFUNC(x)                            (((unsigned)(x) & 0x7) << 4)
#define   G_028800_ZFUNC(x)                            (((x) >> 4) & 0x7)
#define   C_028800_ZFUNC                               0xFFFFFF8F
#define   S_028800_BACKFACE_ENABLE(x)                  (((unsigned)(x) & 0x1) << 7)
#define   G_028800_BACKFACE_ENABLE(x)                  (((x) >> 7) & 0x1)
#define   C_028800_BACKFACE_ENABLE                     0xFFFFFF7F
#define   S_028800_STENCILFUNC(x)                      (((unsigned)(x) & 0x7) << 8)
#define   G_028800_STENCILFUNC(x)                      (((x) >> 8) & 0x7)
#define   C_028800_STENCILFUNC                         0xFFFFF8FF
#define     V_028800_STENCILFUNC_NEVER                 0x00000000
#define     V_028800_STENCILFUNC_LESS                  0x00000001
#define     V_028800_STENCILFUNC_EQUAL                 0x00000002
#define     V_028800_STENCILFUNC_LEQUAL                0x00000003
#define     V_028800_STENCILFUNC_GREATER               0x00000004
#define     V_028800_STENCILFUNC_NOTEQUAL              0x00000005
#define     V_028800_STENCILFUNC_GEQUAL                0x00000006
#define     V_028800_STENCILFUNC_ALWAYS                0x00000007
#define   S_028800_STENCILFAIL(x)                      (((unsigned)(x) & 0x7) << 11)
#define   G_028800_STENCILFAIL(x)                      (((x) >> 11) & 0x7)
#define   C_028800_STENCILFAIL                         0xFFFFC7FF
#define     V_028800_STENCIL_KEEP                      0x00000000
#define     V_028800_STENCIL_ZERO                      0x00000001
#define     V_028800_STENCIL_REPLACE                   0x00000002
#define     V_028800_STENCIL_INCR                      0x00000003
#define     V_028800_STENCIL_DECR                      0x00000004
#define     V_028800_STENCIL_INVERT                    0x00000005
#define     V_028800_STENCIL_INCR_WRAP                 0x00000006
#define     V_028800_STENCIL_DECR_WRAP                 0x00000007
#define   S_028800_STENCILZPASS(x)                     (((unsigned)(x) & 0x7) << 14)
#define   G_028800_STENCILZPASS(x)                     (((x) >> 14) & 0x7)
#define   C_028800_STENCILZPASS                        0xFFFE3FFF
#define   S_028800_STENCILZFAIL(x)                     (((unsigned)(x) & 0x7) << 17)
#define   G_028800_STENCILZFAIL(x)                     (((x) >> 17) & 0x7)
#define   C_028800_STENCILZFAIL                        0xFFF1FFFF
#define   S_028800_STENCILFUNC_BF(x)                   (((unsigned)(x) & 0x7) << 20)
#define   G_028800_STENCILFUNC_BF(x)                   (((x) >> 20) & 0x7)
#define   C_028800_STENCILFUNC_BF                      0xFF8FFFFF
#define   S_028800_STENCILFAIL_BF(x)                   (((unsigned)(x) & 0x7) << 23)
#define   G_028800_STENCILFAIL_BF(x)                   (((x) >> 23) & 0x7)
#define   C_028800_STENCILFAIL_BF                      0xFC7FFFFF
#define   S_028800_STENCILZPASS_BF(x)                  (((unsigned)(x) & 0x7) << 26)
#define   G_028800_STENCILZPASS_BF(x)                  (((x) >> 26) & 0x7)
#define   C_028800_STENCILZPASS_BF                     0xE3FFFFFF
#define   S_028800_STENCILZFAIL_BF(x)                  (((unsigned)(x) & 0x7) << 29)
#define   G_028800_STENCILZFAIL_BF(x)                  (((x) >> 29) & 0x7)
#define   C_028800_STENCILZFAIL_BF                     0x1FFFFFFF

#define R_028808_CB_COLOR_CONTROL                    0x028808
#define   S_028808_DEGAMMA_ENABLE(x)                   (((unsigned)(x) & 0x1) << 3)
#define   G_028808_DEGAMMA_ENABLE(x)                   (((x) >> 3) & 0x1)
#define   C_028808_DEGAMMA_ENABLE                      0xFFFFFFF7
#define   S_028808_MODE(x)                             (((unsigned)(x) & 0x7) << 4)
#define   G_028808_MODE(x)                             (((x) >> 4) & 0x7)
#define   C_028808_MODE                                0xFFFFFF8F
#define      V_028808_CB_DISABLE                       0x00000000
#define      V_028808_CB_NORMAL                        0x00000001
#define      V_028808_CB_ELIMINATE_FAST_CLEAR          0x00000002
#define      V_028808_CB_RESOLVE                       0x00000003
#define      V_028808_CB_DECOMPRESS                    0x00000004
#define      V_028808_CB_FMASK_DECOMPRESS              0x00000005
#define   S_028808_ROP3(x)                             (((unsigned)(x) & 0xFF) << 16)
#define   G_028808_ROP3(x)                             (((x) >> 16) & 0xFF)
#define   C_028808_ROP3                                0xFF00FFFF
#define R_028810_PA_CL_CLIP_CNTL                     0x028810
#define   S_028810_UCP_ENA_0(x)                        (((unsigned)(x) & 0x1) << 0)
#define   G_028810_UCP_ENA_0(x)                        (((x) >> 0) & 0x1)
#define   C_028810_UCP_ENA_0                           0xFFFFFFFE
#define   S_028810_UCP_ENA_1(x)                        (((unsigned)(x) & 0x1) << 1)
#define   G_028810_UCP_ENA_1(x)                        (((x) >> 1) & 0x1)
#define   C_028810_UCP_ENA_1                           0xFFFFFFFD
#define   S_028810_UCP_ENA_2(x)                        (((unsigned)(x) & 0x1) << 2)
#define   G_028810_UCP_ENA_2(x)                        (((x) >> 2) & 0x1)
#define   C_028810_UCP_ENA_2                           0xFFFFFFFB
#define   S_028810_UCP_ENA_3(x)                        (((unsigned)(x) & 0x1) << 3)
#define   G_028810_UCP_ENA_3(x)                        (((x) >> 3) & 0x1)
#define   C_028810_UCP_ENA_3                           0xFFFFFFF7
#define   S_028810_UCP_ENA_4(x)                        (((unsigned)(x) & 0x1) << 4)
#define   G_028810_UCP_ENA_4(x)                        (((x) >> 4) & 0x1)
#define   C_028810_UCP_ENA_4                           0xFFFFFFEF
#define   S_028810_UCP_ENA_5(x)                        (((unsigned)(x) & 0x1) << 5)
#define   G_028810_UCP_ENA_5(x)                        (((x) >> 5) & 0x1)
#define   C_028810_UCP_ENA_5                           0xFFFFFFDF
#define   S_028810_PS_UCP_Y_SCALE_NEG(x)               (((unsigned)(x) & 0x1) << 13)
#define   G_028810_PS_UCP_Y_SCALE_NEG(x)               (((x) >> 13) & 0x1)
#define   C_028810_PS_UCP_Y_SCALE_NEG                  0xFFFFDFFF
#define   S_028810_PS_UCP_MODE(x)                      (((unsigned)(x) & 0x3) << 14)
#define   G_028810_PS_UCP_MODE(x)                      (((x) >> 14) & 0x3)
#define   C_028810_PS_UCP_MODE                         0xFFFF3FFF
#define   S_028810_CLIP_DISABLE(x)                     (((unsigned)(x) & 0x1) << 16)
#define   G_028810_CLIP_DISABLE(x)                     (((x) >> 16) & 0x1)
#define   C_028810_CLIP_DISABLE                        0xFFFEFFFF
#define   S_028810_UCP_CULL_ONLY_ENA(x)                (((unsigned)(x) & 0x1) << 17)
#define   G_028810_UCP_CULL_ONLY_ENA(x)                (((x) >> 17) & 0x1)
#define   C_028810_UCP_CULL_ONLY_ENA                   0xFFFDFFFF
#define   S_028810_BOUNDARY_EDGE_FLAG_ENA(x)           (((unsigned)(x) & 0x1) << 18)
#define   G_028810_BOUNDARY_EDGE_FLAG_ENA(x)           (((x) >> 18) & 0x1)
#define   C_028810_BOUNDARY_EDGE_FLAG_ENA              0xFFFBFFFF
#define   S_028810_DX_CLIP_SPACE_DEF(x)                (((unsigned)(x) & 0x1) << 19)
#define   G_028810_DX_CLIP_SPACE_DEF(x)                (((x) >> 19) & 0x1)
#define   C_028810_DX_CLIP_SPACE_DEF                   0xFFF7FFFF
#define   S_028810_DIS_CLIP_ERR_DETECT(x)              (((unsigned)(x) & 0x1) << 20)
#define   G_028810_DIS_CLIP_ERR_DETECT(x)              (((x) >> 20) & 0x1)
#define   C_028810_DIS_CLIP_ERR_DETECT                 0xFFEFFFFF
#define   S_028810_VTX_KILL_OR(x)                      (((unsigned)(x) & 0x1) << 21)
#define   G_028810_VTX_KILL_OR(x)                      (((x) >> 21) & 0x1)
#define   C_028810_VTX_KILL_OR                         0xFFDFFFFF
#define   S_028810_DX_RASTERIZATION_KILL(x)            (((unsigned)(x) & 0x1) << 22)
#define   G_028810_DX_RASTERIZATION_KILL(x)            (((x) >> 22) & 0x1)
#define   C_028810_DX_RASTERIZATION_KILL               0xFFBFFFFF
#define   S_028810_DX_LINEAR_ATTR_CLIP_ENA(x)          (((unsigned)(x) & 0x1) << 24)
#define   G_028810_DX_LINEAR_ATTR_CLIP_ENA(x)          (((x) >> 24) & 0x1)
#define   C_028810_DX_LINEAR_ATTR_CLIP_ENA             0xFEFFFFFF
#define   S_028810_VTE_VPORT_PROVOKE_DISABLE(x)        (((unsigned)(x) & 0x1) << 25)
#define   G_028810_VTE_VPORT_PROVOKE_DISABLE(x)        (((x) >> 25) & 0x1)
#define   C_028810_VTE_VPORT_PROVOKE_DISABLE           0xFDFFFFFF
#define   S_028810_ZCLIP_NEAR_DISABLE(x)               (((unsigned)(x) & 0x1) << 26)
#define   G_028810_ZCLIP_NEAR_DISABLE(x)               (((x) >> 26) & 0x1)
#define   C_028810_ZCLIP_NEAR_DISABLE                  0xFBFFFFFF
#define   S_028810_ZCLIP_FAR_DISABLE(x)                (((unsigned)(x) & 0x1) << 27)
#define   G_028810_ZCLIP_FAR_DISABLE(x)                (((x) >> 27) & 0x1)
#define   C_028810_ZCLIP_FAR_DISABLE                   0xF7FFFFFF

#define R_028040_DB_Z_INFO                       0x028040
#define   S_028040_FORMAT(x)                           (((unsigned)(x) & 0x3) << 0)
#define   G_028040_FORMAT(x)                           (((x) >> 0) & 0x3)
#define   C_028040_FORMAT                              0xFFFFFFFC
#define     V_028040_Z_INVALID                     0x00000000
#define     V_028040_Z_16                          0x00000001
#define     V_028040_Z_24                          0x00000002
#define     V_028040_Z_32_FLOAT                    0x00000003
#define   S_028040_NUM_SAMPLES(x)                      (((unsigned)(x) & 0x3) << 2) /* cayman only */
#define   S_028040_ARRAY_MODE(x)                       (((unsigned)(x) & 0xF) << 4)
#define   G_028040_ARRAY_MODE(x)                       (((x) >> 4) & 0xF)
#define   C_028040_ARRAY_MODE                          0xFFFFFF0F
#define   S_028040_READ_SIZE(x)                        (((unsigned)(x) & 0x1) << 28)
#define   G_028040_READ_SIZE(x)                        (((x) >> 28) & 0x1)
#define   C_028040_READ_SIZE                           0xEFFFFFFF
#define   S_028040_TILE_SURFACE_ENABLE(x)              (((unsigned)(x) & 0x1) << 29)
#define   G_028040_TILE_SURFACE_ENABLE(x)              (((x) >> 29) & 0x1)
#define   C_028040_TILE_SURFACE_ENABLE                 0xDFFFFFFF
#define   S_028040_ZRANGE_PRECISION(x)                 (((unsigned)(x) & 0x1) << 31)
#define   G_028040_ZRANGE_PRECISION(x)                 (((x) >> 31) & 0x1)
#define   C_028040_ZRANGE_PRECISION                    0x7FFFFFFF
#define   S_028040_TILE_SPLIT(x)                       (((unsigned)(x) & 0x7) << 8)
#define   S_028040_NUM_BANKS(x)                        (((unsigned)(x) & 0x3) << 12)
#define   S_028040_BANK_WIDTH(x)                       (((unsigned)(x) & 0x3) << 16)
#define   S_028040_BANK_HEIGHT(x)                      (((unsigned)(x) & 0x3) << 20)
#define   S_028040_MACRO_TILE_ASPECT(x)                (((unsigned)(x) & 0x3) << 24)

#define R_028044_DB_STENCIL_INFO                     0x028044
#define   S_028044_FORMAT(x)                           (((unsigned)(x) & 0x1) << 0)
#define     V_028044_STENCIL_INVALID			0
#define     V_028044_STENCIL_8				1
#define   G_028044_FORMAT(x)                           (((x) >> 0) & 0x1)
#define   C_028044_FORMAT                              0xFFFFFFFE
#define   S_028044_TILE_SPLIT(x)                       (((unsigned)(x) & 0x7) << 8)

#define R_028058_DB_DEPTH_SIZE                       0x028058
#define   S_028058_PITCH_TILE_MAX(x)                   (((unsigned)(x) & 0x7FF) << 0)
#define   G_028058_PITCH_TILE_MAX(x)                   (((x) >> 0) & 0x7FF)
#define   C_028058_PITCH_TILE_MAX                      0xFFFFF800
#define   S_028058_HEIGHT_TILE_MAX(x)                   (((unsigned)(x) & 0x7FF) << 11)
#define   G_028058_HEIGHT_TILE_MAX(x)                   (((x) >> 11) & 0x7FF)
#define   C_028058_HEIGHT_TILE_MAX                      0xFFC007FF

#define R_02805C_DB_DEPTH_SLICE                      0x02805C
#define   S_02805C_SLICE_TILE_MAX(x)                   (((unsigned)(x) & 0x3FFFFF) << 0)
#define   G_02805C_SLICE_TILE_MAX(x)                   (((x) >> 0) & 0x3FFFFF)
#define   C_02805C_SLICE_TILE_MAX                      0xFFC00000

#define R_028430_DB_STENCILREFMASK                   0x028430
#define   S_028430_STENCILREF(x)                       (((unsigned)(x) & 0xFF) << 0)
#define   G_028430_STENCILREF(x)                       (((x) >> 0) & 0xFF)
#define   C_028430_STENCILREF                          0xFFFFFF00
#define   S_028430_STENCILMASK(x)                      (((unsigned)(x) & 0xFF) << 8)
#define   G_028430_STENCILMASK(x)                      (((x) >> 8) & 0xFF)
#define   C_028430_STENCILMASK                         0xFFFF00FF
#define   S_028430_STENCILWRITEMASK(x)                 (((unsigned)(x) & 0xFF) << 16)
#define   G_028430_STENCILWRITEMASK(x)                 (((x) >> 16) & 0xFF)
#define   C_028430_STENCILWRITEMASK                    0xFF00FFFF
#define R_028434_DB_STENCILREFMASK_BF                0x028434
#define   S_028434_STENCILREF_BF(x)                    (((unsigned)(x) & 0xFF) << 0)
#define   G_028434_STENCILREF_BF(x)                    (((x) >> 0) & 0xFF)
#define   C_028434_STENCILREF_BF                       0xFFFFFF00
#define   S_028434_STENCILMASK_BF(x)                   (((unsigned)(x) & 0xFF) << 8)
#define   G_028434_STENCILMASK_BF(x)                   (((x) >> 8) & 0xFF)
#define   C_028434_STENCILMASK_BF                      0xFFFF00FF
#define   S_028434_STENCILWRITEMASK_BF(x)              (((unsigned)(x) & 0xFF) << 16)
#define   G_028434_STENCILWRITEMASK_BF(x)              (((x) >> 16) & 0xFF)
#define   C_028434_STENCILWRITEMASK_BF                 0xFF00FFFF
#define R_028780_CB_BLEND0_CONTROL                   0x028780
#define   S_028780_COLOR_SRCBLEND(x)                   (((unsigned)(x) & 0x1F) << 0)
#define   G_028780_COLOR_SRCBLEND(x)                   (((x) >> 0) & 0x1F)
#define   C_028780_COLOR_SRCBLEND                      0xFFFFFFE0
#define     V_028780_BLEND_ZERO                        0x00000000
#define     V_028780_BLEND_ONE                         0x00000001
#define     V_028780_BLEND_SRC_COLOR                   0x00000002
#define     V_028780_BLEND_ONE_MINUS_SRC_COLOR         0x00000003
#define     V_028780_BLEND_SRC_ALPHA                   0x00000004
#define     V_028780_BLEND_ONE_MINUS_SRC_ALPHA         0x00000005
#define     V_028780_BLEND_DST_ALPHA                   0x00000006
#define     V_028780_BLEND_ONE_MINUS_DST_ALPHA         0x00000007
#define     V_028780_BLEND_DST_COLOR                   0x00000008
#define     V_028780_BLEND_ONE_MINUS_DST_COLOR         0x00000009
#define     V_028780_BLEND_SRC_ALPHA_SATURATE          0x0000000A
#define     V_028780_BLEND_BOTH_SRC_ALPHA              0x0000000B
#define     V_028780_BLEND_BOTH_INV_SRC_ALPHA          0x0000000C
#define     V_028780_BLEND_CONST_COLOR                 0x0000000D
#define     V_028780_BLEND_ONE_MINUS_CONST_COLOR       0x0000000E
#define     V_028780_BLEND_SRC1_COLOR                  0x0000000F
#define     V_028780_BLEND_INV_SRC1_COLOR              0x00000010
#define     V_028780_BLEND_SRC1_ALPHA                  0x00000011
#define     V_028780_BLEND_INV_SRC1_ALPHA              0x00000012
#define     V_028780_BLEND_CONST_ALPHA                 0x00000013
#define     V_028780_BLEND_ONE_MINUS_CONST_ALPHA       0x00000014
#define   S_028780_COLOR_COMB_FCN(x)                   (((unsigned)(x) & 0x7) << 5)
#define   G_028780_COLOR_COMB_FCN(x)                   (((x) >> 5) & 0x7)
#define   C_028780_COLOR_COMB_FCN                      0xFFFFFF1F
#define     V_028780_COMB_DST_PLUS_SRC                 0x00000000
#define     V_028780_COMB_SRC_MINUS_DST                0x00000001
#define     V_028780_COMB_MIN_DST_SRC                  0x00000002
#define     V_028780_COMB_MAX_DST_SRC                  0x00000003
#define     V_028780_COMB_DST_MINUS_SRC                0x00000004
#define   S_028780_COLOR_DESTBLEND(x)                  (((unsigned)(x) & 0x1F) << 8)
#define   G_028780_COLOR_DESTBLEND(x)                  (((x) >> 8) & 0x1F)
#define   C_028780_COLOR_DESTBLEND                     0xFFFFE0FF
#define   S_028780_OPACITY_WEIGHT(x)                   (((unsigned)(x) & 0x1) << 13)
#define   G_028780_OPACITY_WEIGHT(x)                   (((x) >> 13) & 0x1)
#define   C_028780_OPACITY_WEIGHT                      0xFFFFDFFF
#define   S_028780_ALPHA_SRCBLEND(x)                   (((unsigned)(x) & 0x1F) << 16)
#define   G_028780_ALPHA_SRCBLEND(x)                   (((x) >> 16) & 0x1F)
#define   C_028780_ALPHA_SRCBLEND                      0xFFE0FFFF
#define   S_028780_ALPHA_COMB_FCN(x)                   (((unsigned)(x) & 0x7) << 21)
#define   G_028780_ALPHA_COMB_FCN(x)                   (((x) >> 21) & 0x7)
#define   C_028780_ALPHA_COMB_FCN                      0xFF1FFFFF
#define   S_028780_ALPHA_DESTBLEND(x)                  (((unsigned)(x) & 0x1F) << 24)
#define   G_028780_ALPHA_DESTBLEND(x)                  (((x) >> 24) & 0x1F)
#define   C_028780_ALPHA_DESTBLEND                     0xE0FFFFFF
#define   S_028780_SEPARATE_ALPHA_BLEND(x)             (((unsigned)(x) & 0x1) << 29)
#define   G_028780_SEPARATE_ALPHA_BLEND(x)             (((x) >> 29) & 0x1)
#define   C_028780_SEPARATE_ALPHA_BLEND                0xDFFFFFFF
#define   S_028780_BLEND_CONTROL_ENABLE(x)             (((unsigned)(x) & 0x1) << 30)
#define   G_028780_BLEND_CONTROL_ENABLE(x)             (((x) >> 30) & 0x1)
#define   C_028780_BLEND_CONTROL_ENABLE                0xEFFFFFFF
#define R_028814_PA_SU_SC_MODE_CNTL                  0x028814
#define   S_028814_CULL_FRONT(x)                       (((unsigned)(x) & 0x1) << 0)
#define   G_028814_CULL_FRONT(x)                       (((x) >> 0) & 0x1)
#define   C_028814_CULL_FRONT                          0xFFFFFFFE
#define   S_028814_CULL_BACK(x)                        (((unsigned)(x) & 0x1) << 1)
#define   G_028814_CULL_BACK(x)                        (((x) >> 1) & 0x1)
#define   C_028814_CULL_BACK                           0xFFFFFFFD
#define   S_028814_FACE(x)                             (((unsigned)(x) & 0x1) << 2)
#define   G_028814_FACE(x)                             (((x) >> 2) & 0x1)
#define   C_028814_FACE                                0xFFFFFFFB
#define   S_028814_POLY_MODE(x)                        (((unsigned)(x) & 0x3) << 3)
#define   G_028814_POLY_MODE(x)                        (((x) >> 3) & 0x3)
#define   C_028814_POLY_MODE                           0xFFFFFFE7
#define   S_028814_POLYMODE_FRONT_PTYPE(x)             (((unsigned)(x) & 0x7) << 5)
#define   G_028814_POLYMODE_FRONT_PTYPE(x)             (((x) >> 5) & 0x7)
#define   C_028814_POLYMODE_FRONT_PTYPE                0xFFFFFF1F
#define   S_028814_POLYMODE_BACK_PTYPE(x)              (((unsigned)(x) & 0x7) << 8)
#define   G_028814_POLYMODE_BACK_PTYPE(x)              (((x) >> 8) & 0x7)
#define   C_028814_POLYMODE_BACK_PTYPE                 0xFFFFF8FF
#define   S_028814_POLY_OFFSET_FRONT_ENABLE(x)         (((unsigned)(x) & 0x1) << 11)
#define   G_028814_POLY_OFFSET_FRONT_ENABLE(x)         (((x) >> 11) & 0x1)
#define   C_028814_POLY_OFFSET_FRONT_ENABLE            0xFFFFF7FF
#define   S_028814_POLY_OFFSET_BACK_ENABLE(x)          (((unsigned)(x) & 0x1) << 12)
#define   G_028814_POLY_OFFSET_BACK_ENABLE(x)          (((x) >> 12) & 0x1)
#define   C_028814_POLY_OFFSET_BACK_ENABLE             0xFFFFEFFF
#define   S_028814_POLY_OFFSET_PARA_ENABLE(x)          (((unsigned)(x) & 0x1) << 13)
#define   G_028814_POLY_OFFSET_PARA_ENABLE(x)          (((x) >> 13) & 0x1)
#define   C_028814_POLY_OFFSET_PARA_ENABLE             0xFFFFDFFF
#define   S_028814_VTX_WINDOW_OFFSET_ENABLE(x)         (((unsigned)(x) & 0x1) << 16)
#define   G_028814_VTX_WINDOW_OFFSET_ENABLE(x)         (((x) >> 16) & 0x1)
#define   C_028814_VTX_WINDOW_OFFSET_ENABLE            0xFFFEFFFF
#define   S_028814_PROVOKING_VTX_LAST(x)               (((unsigned)(x) & 0x1) << 19)
#define   G_028814_PROVOKING_VTX_LAST(x)               (((x) >> 19) & 0x1)
#define   C_028814_PROVOKING_VTX_LAST                  0xFFF7FFFF
#define   S_028814_PERSP_CORR_DIS(x)                   (((unsigned)(x) & 0x1) << 20)
#define   G_028814_PERSP_CORR_DIS(x)                   (((x) >> 20) & 0x1)
#define   C_028814_PERSP_CORR_DIS                      0xFFEFFFFF
#define   S_028814_MULTI_PRIM_IB_ENA(x)                (((unsigned)(x) & 0x1) << 21)
#define   G_028814_MULTI_PRIM_IB_ENA(x)                (((x) >> 21) & 0x1)
#define   C_028814_MULTI_PRIM_IB_ENA                   0xFFDFFFFF

#define R_028ABC_DB_HTILE_SURFACE                    0x028ABC
#define   S_028ABC_HTILE_WIDTH(x)                      (((unsigned)(x) & 0x1) << 0)
#define   G_028ABC_HTILE_WIDTH(x)                      (((x) >> 0) & 0x1)
#define   C_028ABC_HTILE_WIDTH                         0xFFFFFFFE
#define   S_028ABC_HTILE_HEIGHT(x)                     (((unsigned)(x) & 0x1) << 1)
#define   G_028ABC_HTILE_HEIGHT(x)                     (((x) >> 1) & 0x1)
#define   C_028ABC_HTILE_HEIGHT                        0xFFFFFFFD
#define   S_028ABC_LINEAR(x)                           (((unsigned)(x) & 0x1) << 2)
#define   G_028ABC_LINEAR(x)                           (((x) >> 2) & 0x1)
#define   C_028ABC_LINEAR                              0xFFFFFFFB
#define   S_028ABC_FULL_CACHE(x)                       (((unsigned)(x) & 0x1) << 3)
#define   G_028ABC_FULL_CACHE(x)                       (((x) >> 3) & 0x1)
#define   C_028ABC_FULL_CACHE                          0xFFFFFFF7
#define   S_028ABC_HTILE_USES_PRELOAD_WIN(x)           (((unsigned)(x) & 0x1) << 4)
#define   G_028ABC_HTILE_USES_PRELOAD_WIN(x)           (((x) >> 4) & 0x1)
#define   C_028ABC_HTILE_USES_PRELOAD_WIN              0xFFFFFFEF
#define   S_028ABC_PRELOAD(x)                          (((unsigned)(x) & 0x1) << 5)
#define   G_028ABC_PRELOAD(x)                          (((x) >> 5) & 0x1)
#define   C_028ABC_PRELOAD                             0xFFFFFFDF
#define   S_028ABC_PREFETCH_WIDTH(x)                   (((unsigned)(x) & 0x3F) << 6)
#define   G_028ABC_PREFETCH_WIDTH(x)                   (((x) >> 6) & 0x3F)
#define   C_028ABC_PREFETCH_WIDTH                      0xFFFFF03F
#define   S_028ABC_PREFETCH_HEIGHT(x)                  (((unsigned)(x) & 0x3F) << 12)
#define   G_028ABC_PREFETCH_HEIGHT(x)                  (((x) >> 12) & 0x3F)
#define   C_028ABC_PREFETCH_HEIGHT                     0xFFFC0FFF
#define R_02880C_DB_SHADER_CONTROL                    0x02880C
#define   S_02880C_Z_EXPORT_ENABLE(x)                  (((unsigned)(x) & 0x1) << 0)
#define   G_02880C_Z_EXPORT_ENABLE(x)                  (((x) >> 0) & 0x1)
#define   C_02880C_Z_EXPORT_ENABLE                     0xFFFFFFFE
#define   S_02880C_STENCIL_EXPORT_ENABLE(x)            (((unsigned)(x) & 0x1) << 1)
#define   G_02880C_STENCIL_EXPORT_ENABLE(x)            (((x) >> 1) & 0x1)
#define   C_02880C_STENCIL_EXPORT_ENABLE               0xFFFFFFFD
#define   S_02880C_Z_ORDER(x)                          (((unsigned)(x) & 0x3) << 4)
#define   G_02880C_Z_ORDER(x)                          (((x) >> 4) & 0x3)
#define   C_02880C_Z_ORDER                             0xFFFFFCFF
#define     V_02880C_LATE_Z                            0
#define     V_02880C_EARLY_Z_THEN_LATE_Z               1
#define     V_02880C_RE_Z                              2
#define     V_02880C_EARLY_Z_THEN_RE_Z                 3
#define   S_02880C_KILL_ENABLE(x)                      (((unsigned)(x) & 0x1) << 6)
#define   G_02880C_KILL_ENABLE(x)                      (((x) >> 6) & 0x1)
#define   C_02880C_KILL_ENABLE                         0xFFFFFFBF
#define   S_02880C_MASK_EXPORT_ENABLE(x)               (((unsigned)(x) & 0x1) << 8)
#define   G_02880C_MASK_EXPORT_ENABLE(x)               (((x) >> 8) & 0x1)
#define   C_02880C_MASK_EXPORT_ENABLE                  0XFFFFFEFF
#define   S_02880C_DUAL_EXPORT_ENABLE(x)               (((unsigned)(x) & 0x1) << 9)
#define   G_02880C_DUAL_EXPORT_ENABLE(x)               (((x) >> 9) & 0x1)
#define   C_02880C_DUAL_EXPORT_ENABLE                  0xFFFFFDFF
#define   S_02880C_EXEC_ON_HIER_FAIL(x)                (((unsigned)(x) & 0x1) << 10)
#define   G_02880C_EXEC_ON_HIER_FAIL(x)                (((x) >> 10) & 0x1)
#define   C_02880C_EXEC_ON_HIER_FAIL                   0xFFFFFBFF
#define   S_02880C_EXEC_ON_NOOP(x)                     (((unsigned)(x) & 0x1) << 11)
#define   G_02880C_EXEC_ON_NOOP(x)                     (((x) >> 11) & 0x1)
#define   C_02880C_EXEC_ON_NOOP                        0xFFFFF7FF
#define   S_02880C_DB_SOURCE_FORMAT(x)                 (((unsigned)(x) & 0x3) << 13)
#define   G_02880C_DB_SOURCE_FORMAT(x)                 (((x) >> 13) & 0x3)
#define   C_02880C_DB_SOURCE_FORMAT                    0xFFFF9FFF
#define     V_02880C_EXPORT_DB_FULL                    0x00
#define     V_02880C_EXPORT_DB_FOUR16                  0x01
#define     V_02880C_EXPORT_DB_TWO                     0x02
#define   S_02880C_ALPHA_TO_MASK_DISABLE(x)            (((unsigned)(x) & 0x1) << 12)
#define   S_02880C_DEPTH_BEFORE_SHADER(x)              (((unsigned)(x) & 0x1) << 15)
#define   S_02880C_CONSERVATIVE_Z_EXPORT(x)            (((unsigned)(x) & 0x03) << 16)
#define   G_02880C_CONSERVATIVE_Z_EXPORT(x)            (((x) >> 16) & 0x03)
#define   C_02880C_CONSERVATIVE_Z_EXPORT               0xFFFCFFFF
#define     V_02880C_EXPORT_ANY_Z                      0
#define     V_02880C_EXPORT_LESS_THAN_Z                1
#define     V_02880C_EXPORT_GREATER_THAN_Z             2
#define     V_02880C_EXPORT_RESERVED                   3

#define R_028A00_PA_SU_POINT_SIZE                    0x028A00
#define   S_028A00_HEIGHT(x)                           (((unsigned)(x) & 0xFFFF) << 0)
#define   G_028A00_HEIGHT(x)                           (((x) >> 0) & 0xFFFF)
#define   C_028A00_HEIGHT                              0xFFFF0000
#define   S_028A00_WIDTH(x)                            (((unsigned)(x) & 0xFFFF) << 16)
#define   G_028A00_WIDTH(x)                            (((x) >> 16) & 0xFFFF)
#define   C_028A00_WIDTH                               0x0000FFFF
#define R_028A0C_PA_SC_LINE_STIPPLE                  0x028A0C
#define   S_028A0C_LINE_PATTERN(x)                     (((unsigned)(x) & 0xFFFF) << 0)
#define   S_028A0C_REPEAT_COUNT(x)                     (((unsigned)(x) & 0xFF) << 16)
#define   S_028A0C_PATTERN_BIT_ORDER(x)                (((unsigned)(x) & 0x1) << 28)
#define   S_028A0C_AUTO_RESET_CNTL(x)                  (((unsigned)(x) & 0x3) << 29)
#define R_028A40_VGT_GS_MODE                         0x028A40
#define   S_028A40_MODE(x)                             (((unsigned)(x) & 0x3) << 0)
#define   G_028A40_MODE(x)                             (((x) >> 0) & 0x3)
#define   C_028A40_MODE                                0xFFFFFFFC
#define     V_028A40_GS_OFF                            0
#define     V_028A40_GS_SCENARIO_A                     1
#define     V_028A40_GS_SCENARIO_B                     2
#define     V_028A40_GS_SCENARIO_G                     3
#define     V_028A40_GS_SCENARIO_C                     4
#define     V_028A40_SPRITE_EN                         5
#define   S_028A40_ES_PASSTHRU(x)                      (((unsigned)(x) & 0x1) << 2)
#define   G_028A40_ES_PASSTHRU(x)                      (((x) >> 2) & 0x1)
#define   C_028A40_ES_PASSTHRU                         0xFFFFFFFB
#define   S_028A40_CUT_MODE(x)                         (((unsigned)(x) & 0x3) << 3)
#define   G_028A40_CUT_MODE(x)                         (((x) >> 3) & 0x3)
#define   C_028A40_CUT_MODE                            0xFFFFFFE7
#define     V_028A40_GS_CUT_1024                       0
#define     V_028A40_GS_CUT_512                        1
#define     V_028A40_GS_CUT_256                        2
#define     V_028A40_GS_CUT_128                        3
#define   S_028A40_COMPUTE_MODE(x)                     (x << 14)
#define   S_028A40_PARTIAL_THD_AT_EOI(x)               (x << 17)
#define R_028A6C_VGT_GS_OUT_PRIM_TYPE                0x028A6C
#define   S_028A6C_OUTPRIM_TYPE(x)                     (((unsigned)(x) & 0x3F) << 0)
#define     V_028A6C_OUTPRIM_TYPE_POINTLIST            0
#define     V_028A6C_OUTPRIM_TYPE_LINESTRIP            1
#define     V_028A6C_OUTPRIM_TYPE_TRISTRIP             2
#define R_008040_WAIT_UNTIL                          0x008040
#define   S_008040_WAIT_CP_DMA_IDLE(x)                 (((unsigned)(x) & 0x1) << 8)
#define   G_008040_WAIT_CP_DMA_IDLE(x)                 (((x) >> 8) & 0x1)
#define   C_008040_WAIT_CP_DMA_IDLE                    0xFFFFFEFF
#define   S_008040_WAIT_CMDFIFO(x)                     (((unsigned)(x) & 0x1) << 10)
#define   G_008040_WAIT_CMDFIFO(x)                     (((x) >> 10) & 0x1)
#define   C_008040_WAIT_CMDFIFO                        0xFFFFFBFF
#define   S_008040_WAIT_2D_IDLE(x)                     (((unsigned)(x) & 0x1) << 14)
#define   G_008040_WAIT_2D_IDLE(x)                     (((x) >> 14) & 0x1)
#define   C_008040_WAIT_2D_IDLE                        0xFFFFBFFF
#define   S_008040_WAIT_3D_IDLE(x)                     (((unsigned)(x) & 0x1) << 15)
#define   G_008040_WAIT_3D_IDLE(x)                     (((x) >> 15) & 0x1)
#define   C_008040_WAIT_3D_IDLE                        0xFFFF7FFF
#define   S_008040_WAIT_2D_IDLECLEAN(x)                (((unsigned)(x) & 0x1) << 16)
#define   G_008040_WAIT_2D_IDLECLEAN(x)                (((x) >> 16) & 0x1)
#define   C_008040_WAIT_2D_IDLECLEAN                   0xFFFEFFFF
#define   S_008040_WAIT_3D_IDLECLEAN(x)                (((unsigned)(x) & 0x1) << 17)
#define   G_008040_WAIT_3D_IDLECLEAN(x)                (((x) >> 17) & 0x1)
#define   C_008040_WAIT_3D_IDLECLEAN                   0xFFFDFFFF
#define   S_008040_WAIT_EXTERN_SIG(x)                  (((unsigned)(x) & 0x1) << 19)
#define   G_008040_WAIT_EXTERN_SIG(x)                  (((x) >> 19) & 0x1)
#define   C_008040_WAIT_EXTERN_SIG                     0xFFF7FFFF
#define   S_008040_CMDFIFO_ENTRIES(x)                  (((unsigned)(x) & 0x1F) << 20)
#define   G_008040_CMDFIFO_ENTRIES(x)                  (((x) >> 20) & 0x1F)
#define   C_008040_CMDFIFO_ENTRIES                     0xFE0FFFFF

/* diff */
#define R_0286CC_SPI_PS_IN_CONTROL_0                 0x0286CC
#define   S_0286CC_NUM_INTERP(x)                       (((unsigned)(x) & 0x3F) << 0)
#define   G_0286CC_NUM_INTERP(x)                       (((x) >> 0) & 0x3F)
#define   C_0286CC_NUM_INTERP                          0xFFFFFFC0
#define   S_0286CC_POSITION_ENA(x)                     (((unsigned)(x) & 0x1) << 8)
#define   G_0286CC_POSITION_ENA(x)                     (((x) >> 8) & 0x1)
#define   C_0286CC_POSITION_ENA                        0xFFFFFEFF
#define   S_0286CC_POSITION_CENTROID(x)                (((unsigned)(x) & 0x1) << 9)
#define   G_0286CC_POSITION_CENTROID(x)                (((x) >> 9) & 0x1)
#define   C_0286CC_POSITION_CENTROID                   0xFFFFFDFF
#define   S_0286CC_POSITION_ADDR(x)                    (((unsigned)(x) & 0x1F) << 10)
#define   G_0286CC_POSITION_ADDR(x)                    (((x) >> 10) & 0x1F)
#define   C_0286CC_POSITION_ADDR                       0xFFFF83FF
#define   S_0286CC_PARAM_GEN(x)                        (((unsigned)(x) & 0xF) << 15)
#define   G_0286CC_PARAM_GEN(x)                        (((x) >> 15) & 0xF)
#define   C_0286CC_PARAM_GEN                           0xFFF87FFF
#define   S_0286CC_PERSP_GRADIENT_ENA(x)               (((unsigned)(x) & 0x1) << 28)
#define   G_0286CC_PERSP_GRADIENT_ENA(x)               (((x) >> 28) & 0x1)
#define   C_0286CC_PERSP_GRADIENT_ENA                  0xEFFFFFFF
#define   S_0286CC_LINEAR_GRADIENT_ENA(x)              (((unsigned)(x) & 0x1) << 29)
#define   G_0286CC_LINEAR_GRADIENT_ENA(x)              (((x) >> 29) & 0x1)
#define   C_0286CC_LINEAR_GRADIENT_ENA                 0xDFFFFFFF
#define   S_0286CC_POSITION_SAMPLE(x)                  (((unsigned)(x) & 0x1) << 30)
#define   G_0286CC_POSITION_SAMPLE(x)                  (((x) >> 30) & 0x1)
#define   C_0286CC_POSITION_SAMPLE                     0xBFFFFFFF
#define R_0286D0_SPI_PS_IN_CONTROL_1                 0x0286D0
#define   S_0286D0_FRONT_FACE_ENA(x)                   (((unsigned)(x) & 0x1) << 8)
#define   G_0286D0_FRONT_FACE_ENA(x)                   (((x) >> 8) & 0x1)
#define   C_0286D0_FRONT_FACE_ENA                      0xFFFFFEFF
#define   S_0286D0_FRONT_FACE_CHAN(x)                  (((unsigned)(x) & 0x3) << 9)
#define   G_0286D0_FRONT_FACE_CHAN(x)                  (((x) >> 9) & 0x3)
#define   C_0286D0_FRONT_FACE_CHAN                     0xFFFFF9FF
#define   S_0286D0_FRONT_FACE_ALL_BITS(x)              (((unsigned)(x) & 0x1) << 11)
#define   G_0286D0_FRONT_FACE_ALL_BITS(x)              (((x) >> 11) & 0x1)
#define   C_0286D0_FRONT_FACE_ALL_BITS                 0xFFFFF7FF
#define   S_0286D0_FRONT_FACE_ADDR(x)                  (((unsigned)(x) & 0x1F) << 12)
#define   G_0286D0_FRONT_FACE_ADDR(x)                  (((x) >> 12) & 0x1F)
#define   C_0286D0_FRONT_FACE_ADDR                     0xFFFE0FFF
#define   S_0286D0_FOG_ADDR(x)                         (((unsigned)(x) & 0x7F) << 17)
#define   G_0286D0_FOG_ADDR(x)                         (((x) >> 17) & 0x7F)
#define   C_0286D0_FOG_ADDR                            0xFF01FFFF
#define   S_0286D0_FIXED_PT_POSITION_ENA(x)            (((unsigned)(x) & 0x1) << 24)
#define   G_0286D0_FIXED_PT_POSITION_ENA(x)            (((x) >> 24) & 0x1)
#define   C_0286D0_FIXED_PT_POSITION_ENA               0xFEFFFFFF
#define   S_0286D0_FIXED_PT_POSITION_ADDR(x)           (((unsigned)(x) & 0x1F) << 25)
#define   G_0286D0_FIXED_PT_POSITION_ADDR(x)           (((x) >> 25) & 0x1F)
#define   C_0286D0_FIXED_PT_POSITION_ADDR              0xC1FFFFFF
#define R_0286C4_SPI_VS_OUT_CONFIG                   0x0286C4
#define   S_0286C4_VS_PER_COMPONENT(x)                 (((unsigned)(x) & 0x1) << 0)
#define   G_0286C4_VS_PER_COMPONENT(x)                 (((x) >> 0) & 0x1)
#define   C_0286C4_VS_PER_COMPONENT                    0xFFFFFFFE
#define   S_0286C4_VS_EXPORT_COUNT(x)                  (((unsigned)(x) & 0x1F) << 1)
#define   G_0286C4_VS_EXPORT_COUNT(x)                  (((x) >> 1) & 0x1F)
#define   C_0286C4_VS_EXPORT_COUNT                     0xFFFFFFC1
#define   S_0286C4_VS_EXPORTS_FOG(x)                   (((unsigned)(x) & 0x1) << 8)
#define   G_0286C4_VS_EXPORTS_FOG(x)                   (((x) >> 8) & 0x1)
#define   C_0286C4_VS_EXPORTS_FOG                      0xFFFFFEFF
#define   S_0286C4_VS_OUT_FOG_VEC_ADDR(x)              (((unsigned)(x) & 0x1F) << 9)
#define   G_0286C4_VS_OUT_FOG_VEC_ADDR(x)              (((x) >> 9) & 0x1F)
#define   C_0286C4_VS_OUT_FOG_VEC_ADDR                 0xFFFFC1FF

#define R_0286E0_SPI_BARYC_CNTL                     0x0286E0
#define   S_0286E0_PERSP_CENTER_ENA(x)                (((unsigned)(x) & 0x3) << 0)
#define   G_0286E0_PERSP_CENTER_ENA(x)                (((x) >> 0) & 0x3)
#define   C_0286E0_PERSP_CENTER_ENA                   0xFFFFFFFC
#define   S_0286E0_PERSP_CENTROID_ENA(x)              (((unsigned)(x) & 0x3) << 4)
#define   G_0286E0_PERSP_CENTROID_ENA(x)              (((x) >> 4) & 0x3)
#define   C_0286E0_PERSP_CENTROID_ENA                 0xFFFFFFCF
#define   S_0286E0_PERSP_SAMPLE_ENA(x)                (((unsigned)(x) & 0x3) << 8)
#define   G_0286E0_PERSP_SAMPLE_ENA(x)                (((x) >> 8) & 0x3)
#define   C_0286E0_PERSP_SAMPLE_ENA                   0xFFFFFCFF
#define   S_0286E0_PERSP_PULL_MODEL_ENA(x)            (((unsigned)(x) & 0x3) << 12)
#define   G_0286E0_PERSP_PULL_MODEL_ENA(x)            (((x) >> 12) & 0x3)
#define   C_0286E0_PERSP_PULL_MODEL_ENA               0xFFFFCFFF
#define   S_0286E0_LINEAR_CENTER_ENA(x)               (((unsigned)(x) & 0x3) << 16)
#define   G_0286E0_LINEAR_CENTER_ENA(x)               (((x) >> 16) & 0x3)
#define   C_0286E0_LINEAR_CENTER_ENA                  0xFFFCFFFF
#define   S_0286E0_LINEAR_CENTROID_ENA(x)             (((unsigned)(x) & 0x3) << 20)
#define   G_0286E0_LINEAR_CENTROID_ENA(x)             (((x) >> 20) & 0x3)
#define   C_0286E0_LINEAR_CENTROID_ENA                0xFFCFFFFF
#define   S_0286E0_LINEAR_SAMPLE_ENA(x)               (((unsigned)(x) & 0x3) << 24)
#define   G_0286E0_LINEAR_SAMPLE_ENA(x)               (((x) >> 24) & 0x3)
#define   C_0286E0_LINEAR_SAMPLE_ENA                  0xFCFFFFFF


/* new - diff */
#define R_028250_PA_SC_VPORT_SCISSOR_0_TL            0x028250
#define   S_028250_TL_X(x)                             (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028250_TL_X(x)                             (((x) >> 0) & 0x7FFF)
#define   C_028250_TL_X                                0xFFFF8000
#define   S_028250_TL_Y(x)                             (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028250_TL_Y(x)                             (((x) >> 16) & 0x7FFF)
#define   C_028250_TL_Y                                0x8000FFFF
#define   S_028250_WINDOW_OFFSET_DISABLE(x)            (((unsigned)(x) & 0x1) << 31)
#define   G_028250_WINDOW_OFFSET_DISABLE(x)            (((x) >> 31) & 0x1)
#define   C_028250_WINDOW_OFFSET_DISABLE               0x7FFFFFFF
#define R_028254_PA_SC_VPORT_SCISSOR_0_BR            0x028254
#define   S_028254_BR_X(x)                             (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028254_BR_X(x)                             (((x) >> 0) & 0x7FFF)
#define   C_028254_BR_X                                0xFFFF8000
#define   S_028254_BR_Y(x)                             (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028254_BR_Y(x)                             (((x) >> 16) & 0x7FFF)
#define   C_028254_BR_Y                                0x8000FFFF
/* diff */
#define R_028240_PA_SC_GENERIC_SCISSOR_TL            0x028240
#define   S_028240_TL_X(x)                             (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028240_TL_X(x)                             (((x) >> 0) & 0x7FFF)
#define   C_028240_TL_X                                0xFFFF8000
#define   S_028240_TL_Y(x)                             (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028240_TL_Y(x)                             (((x) >> 16) & 0x7FFF)
#define   C_028240_TL_Y                                0x8000FFFF
#define   S_028240_WINDOW_OFFSET_DISABLE(x)            (((unsigned)(x) & 0x1) << 31)
#define   G_028240_WINDOW_OFFSET_DISABLE(x)            (((x) >> 31) & 0x1)
#define   C_028240_WINDOW_OFFSET_DISABLE               0x7FFFFFFF
#define R_028244_PA_SC_GENERIC_SCISSOR_BR            0x028244
#define   S_028244_BR_X(x)                             (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028244_BR_X(x)                             (((x) >> 0) & 0x7FFF)
#define   C_028244_BR_X                                0xFFFF8000
#define   S_028244_BR_Y(x)                             (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028244_BR_Y(x)                             (((x) >> 16) & 0x7FFF)
#define   C_028244_BR_Y                                0x8000FFFF
/* diff */
#define R_028030_PA_SC_SCREEN_SCISSOR_TL             0x028030
#define   S_028030_TL_X(x)                             (((unsigned)(x) & 0xFFFF) << 0)
#define   G_028030_TL_X(x)                             (((x) >> 0) & 0xFFFF)
#define   C_028030_TL_X                                0xFFFF0000
#define   S_028030_TL_Y(x)                             (((unsigned)(x) & 0xFFFF) << 16)
#define   G_028030_TL_Y(x)                             (((x) >> 16) & 0xFFFF)
#define   C_028030_TL_Y                                0x0000FFFF
#define R_028034_PA_SC_SCREEN_SCISSOR_BR             0x028034
#define   S_028034_BR_X(x)                             (((unsigned)(x) & 0xFFFF) << 0)
#define   G_028034_BR_X(x)                             (((x) >> 0) & 0xFFFF)
#define   C_028034_BR_X                                0xFFFF0000
#define   S_028034_BR_Y(x)                             (((unsigned)(x) & 0xFFFF) << 16)
#define   G_028034_BR_Y(x)                             (((x) >> 16) & 0xFFFF)
#define   C_028034_BR_Y                                0x0000FFFF
/* diff */
#define R_028204_PA_SC_WINDOW_SCISSOR_TL             0x028204
#define   S_028204_TL_X(x)                             (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028204_TL_X(x)                             (((x) >> 0) & 0x7FFF)
#define   C_028204_TL_X                                0xFFFF8000
#define   S_028204_TL_Y(x)                             (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028204_TL_Y(x)                             (((x) >> 16) & 0x7FFF)
#define   C_028204_TL_Y                                0x8000FFFF
#define   S_028204_WINDOW_OFFSET_DISABLE(x)            (((unsigned)(x) & 0x1) << 31)
#define   G_028204_WINDOW_OFFSET_DISABLE(x)            (((x) >> 31) & 0x1)
#define   C_028204_WINDOW_OFFSET_DISABLE               0x7FFFFFFF
#define R_028208_PA_SC_WINDOW_SCISSOR_BR             0x028208
#define   S_028208_BR_X(x)                             (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028208_BR_X(x)                             (((x) >> 0) & 0x7FFF)
#define   C_028208_BR_X                                0xFFFF8000
#define   S_028208_BR_Y(x)                             (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028208_BR_Y(x)                             (((x) >> 16) & 0x7FFF)
#define   C_028208_BR_Y                                0x8000FFFF

#define R_028A78_VGT_DMA_MAX_SIZE                    0x028A78
#define R_028A7C_VGT_DMA_INDEX_TYPE                  0x028A7C
#define R_028A88_VGT_NUM_INSTANCES                   0x028A88
#define R_0287E4_VGT_DMA_BASE_HI                     0x0287E4
#define R_0287E8_VGT_DMA_BASE                        0x0287E8
#define R_0287F0_VGT_DRAW_INITIATOR                  0x0287F0
#define   S_0287F0_SOURCE_SELECT(x)                    (((unsigned)(x) & 0x3) << 0)
#define   G_0287F0_SOURCE_SELECT(x)                    (((x) >> 0) & 0x3)
#define   C_0287F0_SOURCE_SELECT                       0xFFFFFFFC
#define   S_0287F0_MAJOR_MODE(x)                       (((unsigned)(x) & 0x3) << 2)
#define   G_0287F0_MAJOR_MODE(x)                       (((x) >> 2) & 0x3)
#define   C_0287F0_MAJOR_MODE                          0xFFFFFFF3
#define   S_0287F0_SPRITE_EN(x)                        (((unsigned)(x) & 0x1) << 4)
#define   G_0287F0_SPRITE_EN(x)                        (((x) >> 4) & 0x1)
#define   C_0287F0_SPRITE_EN                           0xFFFFFFEF
#define   S_0287F0_NOT_EOP(x)                          (((unsigned)(x) & 0x1) << 5)
#define   G_0287F0_NOT_EOP(x)                          (((x) >> 5) & 0x1)
#define   C_0287F0_NOT_EOP                             0xFFFFFFDF
#define   S_0287F0_USE_OPAQUE(x)                       (((unsigned)(x) & 0x1) << 6)
#define   G_0287F0_USE_OPAQUE(x)                       (((x) >> 6) & 0x1)
#define   C_0287F0_USE_OPAQUE                          0xFFFFFFBF

#define R_030000_SQ_TEX_RESOURCE_WORD0_0             0x030000
#define   S_030000_DIM(x)                              (((unsigned)(x) & 0x7) << 0)
#define   G_030000_DIM(x)                              (((x) >> 0) & 0x7)
#define   C_030000_DIM                                 0xFFFFFFF8
#define     V_030000_SQ_TEX_DIM_1D                     0x00000000
#define     V_030000_SQ_TEX_DIM_2D                     0x00000001
#define     V_030000_SQ_TEX_DIM_3D                     0x00000002
#define     V_030000_SQ_TEX_DIM_CUBEMAP                0x00000003
#define     V_030000_SQ_TEX_DIM_1D_ARRAY               0x00000004
#define     V_030000_SQ_TEX_DIM_2D_ARRAY               0x00000005
#define     V_030000_SQ_TEX_DIM_2D_MSAA                0x00000006
#define     V_030000_SQ_TEX_DIM_2D_ARRAY_MSAA          0x00000007
#define   S_030000_NON_DISP_TILING_ORDER(x)            (((unsigned)(x) & 0x1) << 5)
#define   G_030000_NON_DISP_TILING_ORDER(x)            (((x) >> 5) & 0x1)
#define   C_030000_NON_DISP_TILING_ORDER               0xFFFFFFDF
#define   CM_S_030000_NON_DISP_TILING_ORDER(x)         (((unsigned)(x) & 0x3) << 4)
#define   CM_G_030000_NON_DISP_TILING_ORDER(x)         (((x) >> 4) & 0x3)
#define   CM_C_030000_NON_DISP_TILING_ORDER            0xFFFFFFCF
#define   S_030000_PITCH(x)                            (((unsigned)(x) & 0xFFF) << 6)
#define   G_030000_PITCH(x)                            (((x) >> 6) & 0xFFF)
#define   C_030000_PITCH                               0xFFFC003F
#define   S_030000_TEX_WIDTH(x)                        (((unsigned)(x) & 0x3FFF) << 18)
#define   G_030000_TEX_WIDTH(x)                        (((x) >> 18) & 0x3FFF)
#define   C_030000_TEX_WIDTH                           0x0003FFFF
#define R_030004_SQ_TEX_RESOURCE_WORD1_0             0x030004
#define   S_030004_TEX_HEIGHT(x)                       (((unsigned)(x) & 0x3FFF) << 0)
#define   G_030004_TEX_HEIGHT(x)                       (((x) >> 0) & 0x3FFF)
#define   C_030004_TEX_HEIGHT                          0xFFFFC000
#define   S_030004_TEX_DEPTH(x)                        (((unsigned)(x) & 0x1FFF) << 14)
#define   G_030004_TEX_DEPTH(x)                        (((x) >> 14) & 0x1FFF)
#define   C_030004_TEX_DEPTH                           0xF8003FFF
#define   S_030004_ARRAY_MODE(x)                       (((unsigned)(x) & 0xF) << 28)
#define   G_030004_ARRAY_MODE(x)                       (((x) >> 28) & 0xF)
#define   C_030004_ARRAY_MODE                          0x0FFFFFFF
#define R_030008_SQ_TEX_RESOURCE_WORD2_0             0x030008
#define   S_030008_BASE_ADDRESS(x)                     (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_030008_BASE_ADDRESS(x)                     (((x) >> 0) & 0xFFFFFFFF)
#define   C_030008_BASE_ADDRESS                        0x00000000
#define R_03000C_SQ_TEX_RESOURCE_WORD3_0             0x03000C
#define   S_03000C_MIP_ADDRESS(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_03000C_MIP_ADDRESS(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_03000C_MIP_ADDRESS                         0x00000000
#define R_030010_SQ_TEX_RESOURCE_WORD4_0             0x030010
#define   S_030010_FORMAT_COMP_X(x)                    (((unsigned)(x) & 0x3) << 0)
#define   G_030010_FORMAT_COMP_X(x)                    (((x) >> 0) & 0x3)
#define   C_030010_FORMAT_COMP_X                       0xFFFFFFFC
#define     V_030010_SQ_FORMAT_COMP_UNSIGNED           0x00000000
#define     V_030010_SQ_FORMAT_COMP_SIGNED             0x00000001
#define     V_030010_SQ_FORMAT_COMP_UNSIGNED_BIASED    0x00000002
#define   S_030010_FORMAT_COMP_Y(x)                    (((unsigned)(x) & 0x3) << 2)
#define   G_030010_FORMAT_COMP_Y(x)                    (((x) >> 2) & 0x3)
#define   C_030010_FORMAT_COMP_Y                       0xFFFFFFF3
#define   S_030010_FORMAT_COMP_Z(x)                    (((unsigned)(x) & 0x3) << 4)
#define   G_030010_FORMAT_COMP_Z(x)                    (((x) >> 4) & 0x3)
#define   C_030010_FORMAT_COMP_Z                       0xFFFFFFCF
#define   S_030010_FORMAT_COMP_W(x)                    (((unsigned)(x) & 0x3) << 6)
#define   G_030010_FORMAT_COMP_W(x)                    (((x) >> 6) & 0x3)
#define   C_030010_FORMAT_COMP_W                       0xFFFFFF3F
#define   S_030010_NUM_FORMAT_ALL(x)                   (((unsigned)(x) & 0x3) << 8)
#define   G_030010_NUM_FORMAT_ALL(x)                   (((x) >> 8) & 0x3)
#define   C_030010_NUM_FORMAT_ALL                      0xFFFFFCFF
#define     V_030010_SQ_NUM_FORMAT_NORM                0x00000000
#define     V_030010_SQ_NUM_FORMAT_INT                 0x00000001
#define     V_030010_SQ_NUM_FORMAT_SCALED              0x00000002
#define   S_030010_SRF_MODE_ALL(x)                     (((unsigned)(x) & 0x1) << 10)
#define   G_030010_SRF_MODE_ALL(x)                     (((x) >> 10) & 0x1)
#define   C_030010_SRF_MODE_ALL                        0xFFFFFBFF
#define     V_030010_SRF_MODE_ZERO_CLAMP_MINUS_ONE     0x00000000
#define     V_030010_SRF_MODE_NO_ZERO                  0x00000001
#define   S_030010_FORCE_DEGAMMA(x)                    (((unsigned)(x) & 0x1) << 11)
#define   G_030010_FORCE_DEGAMMA(x)                    (((x) >> 11) & 0x1)
#define   C_030010_FORCE_DEGAMMA                       0xFFFFF7FF
#define   S_030010_ENDIAN_SWAP(x)                      (((unsigned)(x) & 0x3) << 12)
#define   G_030010_ENDIAN_SWAP(x)                      (((x) >> 12) & 0x3)
#define   C_030010_ENDIAN_SWAP                         0xFFFFCFFF
#define   S_030010_LOG2_NUM_FRAGMENTS(x)               (((unsigned)(x) & 0x3) << 14) /* cayman only */
#define   S_030010_DST_SEL_X(x)                        (((unsigned)(x) & 0x7) << 16)
#define   G_030010_DST_SEL_X(x)                        (((x) >> 16) & 0x7)
#define   C_030010_DST_SEL_X                           0xFFF8FFFF
#define     V_030010_SQ_SEL_X                          0x00000000
#define     V_030010_SQ_SEL_Y                          0x00000001
#define     V_030010_SQ_SEL_Z                          0x00000002
#define     V_030010_SQ_SEL_W                          0x00000003
#define     V_030010_SQ_SEL_0                          0x00000004
#define     V_030010_SQ_SEL_1                          0x00000005
#define   S_030010_DST_SEL_Y(x)                        (((unsigned)(x) & 0x7) << 19)
#define   G_030010_DST_SEL_Y(x)                        (((x) >> 19) & 0x7)
#define   C_030010_DST_SEL_Y                           0xFFC7FFFF
#define   S_030010_DST_SEL_Z(x)                        (((unsigned)(x) & 0x7) << 22)
#define   G_030010_DST_SEL_Z(x)                        (((x) >> 22) & 0x7)
#define   C_030010_DST_SEL_Z                           0xFE3FFFFF
#define   S_030010_DST_SEL_W(x)                        (((unsigned)(x) & 0x7) << 25)
#define   G_030010_DST_SEL_W(x)                        (((x) >> 25) & 0x7)
#define   C_030010_DST_SEL_W                           0xF1FFFFFF
#define   S_030010_BASE_LEVEL(x)                       (((unsigned)(x) & 0xF) << 28)
#define   G_030010_BASE_LEVEL(x)                       (((x) >> 28) & 0xF)
#define   C_030010_BASE_LEVEL                          0x0FFFFFFF
#define R_030014_SQ_TEX_RESOURCE_WORD5_0             0x030014
#define   S_030014_LAST_LEVEL(x)                       (((unsigned)(x) & 0xF) << 0)
#define   G_030014_LAST_LEVEL(x)                       (((x) >> 0) & 0xF)
#define   C_030014_LAST_LEVEL                          0xFFFFFFF0
#define   S_030014_BASE_ARRAY(x)                       (((unsigned)(x) & 0x1FFF) << 4)
#define   G_030014_BASE_ARRAY(x)                       (((x) >> 4) & 0x1FFF)
#define   C_030014_BASE_ARRAY                          0xFFFE000F
#define   S_030014_LAST_ARRAY(x)                       (((unsigned)(x) & 0x1FFF) << 17)
#define   G_030014_LAST_ARRAY(x)                       (((x) >> 17) & 0x1FFF)
#define   C_030014_LAST_ARRAY                          0xC001FFFF
#define R_030018_SQ_TEX_RESOURCE_WORD6_0             0x030018
/* FMASK_BANK_HEIGHT and MAX_ANISO_RATIO share the first two bits.
 * The former is only used with MSAA textures. */
#define   S_030018_MAX_ANISO_RATIO(x)                  (((unsigned)(x) & 0x7) << 0)
#define   G_030018_MAX_ANISO_RATIO(x)                  (((x) >> 0) & 0x7)
#define   C_030018_MAX_ANISO_RATIO                     0xFFFFFFF8
#define   S_030018_FMASK_BANK_HEIGHT(x)                (((unsigned)(x) & 0x3) << 0)
#define   S_030018_PERF_MODULATION(x)                  (((unsigned)(x) & 0x7) << 3)
#define   G_030018_PERF_MODULATION(x)                  (((x) >> 3) & 0x7)
#define   C_030018_PERF_MODULATION                     0xFFFFFFC7
#define   S_030018_INTERLACED(x)                       (((unsigned)(x) & 0x1) << 6)
#define   G_030018_INTERLACED(x)                       (((x) >> 6) & 0x1)
#define   C_030018_INTERLACED                          0xFFFFFFBF
#define   S_030018_TILE_SPLIT(x)                       (((unsigned)(x) & 0x7) << 29)
#define R_03001C_SQ_TEX_RESOURCE_WORD7_0             0x03001C
#define   S_03001C_DATA_FORMAT(x)                      (((unsigned)(x) & 0x3F) << 0)
#define   G_03001C_DATA_FORMAT(x)                      (((x) >> 0) & 0x3F)
#define   C_03001C_DATA_FORMAT                         0xFFFFFFC0
#define   S_03001C_MACRO_TILE_ASPECT(x)                (((unsigned)(x) & 0x3) << 6)
#define   S_03001C_BANK_WIDTH(x)                       (((unsigned)(x) & 0x3) << 8)
#define   S_03001C_BANK_HEIGHT(x)                      (((unsigned)(x) & 0x3) << 10)
#define   S_03001C_DEPTH_SAMPLE_ORDER(x)               (((unsigned)(x) & 0x1) << 15)
#define   S_03001C_NUM_BANKS(x)                        (((unsigned)(x) & 0x3) << 16)
#define   S_03001C_TYPE(x)                             (((unsigned)(x) & 0x3) << 30)
#define   G_03001C_TYPE(x)                             (((x) >> 30) & 0x3)
#define   C_03001C_TYPE                                0x3FFFFFFF
#define     V_03001C_SQ_TEX_VTX_INVALID_TEXTURE        0x00000000
#define     V_03001C_SQ_TEX_VTX_INVALID_BUFFER         0x00000001
#define     V_03001C_SQ_TEX_VTX_VALID_TEXTURE          0x00000002
#define     V_03001C_SQ_TEX_VTX_VALID_BUFFER           0x00000003

#define R_030008_SQ_VTX_CONSTANT_WORD2_0             0x030008
#define   S_030008_BASE_ADDRESS_HI(x)                  (((unsigned)(x) & 0xFF) << 0)
#define   G_030008_BASE_ADDRESS_HI(x)                  (((x) >> 0) & 0xFF)
#define   C_030008_BASE_ADDRESS_HI                     0xFFFFFF00
#define   S_030008_STRIDE(x)                           (((unsigned)(x) & 0x7FF) << 8)
#define   G_030008_STRIDE(x)                           (((x) >> 8) & 0x7FF)
#define   C_030008_STRIDE                              0xFFF800FF
#define   S_030008_CLAMP_X(x)                          (((unsigned)(x) & 0x1) << 19)
#define   G_030008_CLAMP_X(x)                          (((x) >> 19) & 0x1)
#define   C_030008_CLAMP_X                             0xFFF7FFFF
#define   S_030008_DATA_FORMAT(x)                      (((unsigned)(x) & 0x3F) << 20)
#define   G_030008_DATA_FORMAT(x)                      (((x) >> 20) & 0x3F)
#define   C_030008_DATA_FORMAT                         0xFC0FFFFF
#define   S_030008_NUM_FORMAT_ALL(x)                   (((unsigned)(x) & 0x3) << 26)
#define   G_030008_NUM_FORMAT_ALL(x)                   (((x) >> 26) & 0x3)
#define   C_030008_NUM_FORMAT_ALL                      0xF3FFFFFF
#define     V_030008_SQ_NUM_FORMAT_NORM                0x00000000
#define     V_030008_SQ_NUM_FORMAT_INT                 0x00000001
#define     V_030008_SQ_NUM_FORMAT_SCALED              0x00000002
#define   S_030008_FORMAT_COMP_ALL(x)                  (((unsigned)(x) & 0x1) << 28)
#define   G_030008_FORMAT_COMP_ALL(x)                  (((x) >> 28) & 0x1)
#define   C_030008_FORMAT_COMP_ALL                     0xEFFFFFFF
#define   S_030008_SRF_MODE_ALL(x)                     (((unsigned)(x) & 0x1) << 29)
#define   G_030008_SRF_MODE_ALL(x)                     (((x) >> 29) & 0x1)
#define   C_030008_SRF_MODE_ALL                        0xDFFFFFFF
#define   S_030008_ENDIAN_SWAP(x)                      (((unsigned)(x) & 0x3) << 30)
#define   G_030008_ENDIAN_SWAP(x)                      (((x) >> 30) & 0x3)
#define   C_030008_ENDIAN_SWAP                         0x3FFFFFFF

#define R_03000C_SQ_VTX_CONSTANT_WORD3_0             0x03000C
#define   S_03000C_UNCACHED(x)                         (((unsigned)(x) & 0x1) << 2)
#define   S_03000C_DST_SEL_X(x)                        (((unsigned)(x) & 0x7) << 3)
#define   G_03000C_DST_SEL_X(x)                        (((x) >> 3) & 0x7)
#define     V_03000C_SQ_SEL_X                          0x00000000
#define     V_03000C_SQ_SEL_Y                          0x00000001
#define     V_03000C_SQ_SEL_Z                          0x00000002
#define     V_03000C_SQ_SEL_W                          0x00000003
#define     V_03000C_SQ_SEL_0                          0x00000004
#define     V_03000C_SQ_SEL_1                          0x00000005
#define   S_03000C_DST_SEL_Y(x)                        (((unsigned)(x) & 0x7) << 6)
#define   G_03000C_DST_SEL_Y(x)                        (((x) >> 6) & 0x7)
#define   S_03000C_DST_SEL_Z(x)                        (((unsigned)(x) & 0x7) << 9)
#define   G_03000C_DST_SEL_Z(x)                        (((x) >> 9) & 0x7)
#define   S_03000C_DST_SEL_W(x)                        (((unsigned)(x) & 0x7) << 12)
#define   G_03000C_DST_SEL_W(x)                        (((x) >> 12) & 0x7)

#define R_00A400_TD_PS_SAMPLER0_BORDER_INDEX         0x00A400
#define R_00A404_TD_PS_SAMPLER0_BORDER_RED           0x00A404
#define R_00A408_TD_PS_SAMPLER0_BORDER_GREEN         0x00A408
#define R_00A40C_TD_PS_SAMPLER0_BORDER_BLUE          0x00A40C
#define R_00A410_TD_PS_SAMPLER0_BORDER_ALPHA         0x00A410
#define R_00A414_TD_VS_SAMPLER0_BORDER_INDEX         0x00A414
#define R_00A418_TD_VS_SAMPLER0_BORDER_RED           0x00A418
#define R_00A41C_TD_VS_SAMPLER0_BORDER_GREEN         0x00A41C
#define R_00A420_TD_VS_SAMPLER0_BORDER_BLUE          0x00A420
#define R_00A424_TD_VS_SAMPLER0_BORDER_ALPHA         0x00A424
#define R_00A428_TD_GS_SAMPLER0_BORDER_INDEX         0x00A428
#define R_00A42C_TD_GS_SAMPLER0_BORDER_RED           0x00A42C
#define R_00A430_TD_GS_SAMPLER0_BORDER_GREEN         0x00A430
#define R_00A434_TD_GS_SAMPLER0_BORDER_BLUE          0x00A434
#define R_00A438_TD_GS_SAMPLER0_BORDER_ALPHA         0x00A438
#define R_00A43C_TD_HS_SAMPLER0_BORDER_COLOR_INDEX   0x00A43C
#define R_00A440_TD_HS_SAMPLER0_BORDER_COLOR_RED     0x00A440
#define R_00A444_TD_HS_SAMPLER0_BORDER_COLOR_GREEN   0x00A444
#define R_00A448_TD_HS_SAMPLER0_BORDER_COLOR_BLUE    0x00A448
#define R_00A44C_TD_HS_SAMPLER0_BORDER_COLOR_ALPHA   0x00A44C
#define R_00A450_TD_LS_SAMPLER0_BORDER_COLOR_INDEX   0x00A450
#define R_00A454_TD_LS_SAMPLER0_BORDER_COLOR_RED     0x00A454
#define R_00A458_TD_LS_SAMPLER0_BORDER_COLOR_GREEN   0x00A458
#define R_00A45C_TD_LS_SAMPLER0_BORDER_COLOR_BLUE    0x00A45C
#define R_00A460_TD_LS_SAMPLER0_BORDER_COLOR_ALPHA   0x00A460
#define R_00A464_TD_CS_SAMPLER0_BORDER_INDEX         0x00A464
#define R_00A468_TD_CS_SAMPLER0_BORDER_RED           0x00A468
#define R_00A46C_TD_CS_SAMPLER0_BORDER_GREEN         0x00A46C
#define R_00A470_TD_CS_SAMPLER0_BORDER_BLUE          0x00A470
#define R_00A474_TD_CS_SAMPLER0_BORDER_ALPHA         0x00A474

#define R_03C000_SQ_TEX_SAMPLER_WORD0_0              0x03C000
#define   S_03C000_CLAMP_X(x)                          (((unsigned)(x) & 0x7) << 0)
#define   G_03C000_CLAMP_X(x)                          (((x) >> 0) & 0x7)
#define   C_03C000_CLAMP_X                             0xFFFFFFF8
#define     V_03C000_SQ_TEX_WRAP                       0x00000000
#define     V_03C000_SQ_TEX_MIRROR                     0x00000001
#define     V_03C000_SQ_TEX_CLAMP_LAST_TEXEL           0x00000002
#define     V_03C000_SQ_TEX_MIRROR_ONCE_LAST_TEXEL     0x00000003
#define     V_03C000_SQ_TEX_CLAMP_HALF_BORDER          0x00000004
#define     V_03C000_SQ_TEX_MIRROR_ONCE_HALF_BORDER    0x00000005
#define     V_03C000_SQ_TEX_CLAMP_BORDER               0x00000006
#define     V_03C000_SQ_TEX_MIRROR_ONCE_BORDER         0x00000007
#define   S_03C000_CLAMP_Y(x)                          (((unsigned)(x) & 0x7) << 3)
#define   G_03C000_CLAMP_Y(x)                          (((x) >> 3) & 0x7)
#define   C_03C000_CLAMP_Y                             0xFFFFFFC7
#define   S_03C000_CLAMP_Z(x)                          (((unsigned)(x) & 0x7) << 6)
#define   G_03C000_CLAMP_Z(x)                          (((x) >> 6) & 0x7)
#define   C_03C000_CLAMP_Z                             0xFFFFFE3F
#define   S_03C000_XY_MAG_FILTER(x)                    (((unsigned)(x) & 0x3) << 9)
#define   G_03C000_XY_MAG_FILTER(x)                    (((x) >> 9) & 0x3)
#define   C_03C000_XY_MAG_FILTER                       0xFFFFF9FF
#define     V_03C000_SQ_TEX_XY_FILTER_POINT            0x00000000
#define     V_03C000_SQ_TEX_XY_FILTER_BILINEAR         0x00000001
#define   S_03C000_XY_MIN_FILTER(x)                    (((unsigned)(x) & 0x3) << 11)
#define   G_03C000_XY_MIN_FILTER(x)                    (((x) >> 11) & 0x3)
#define   C_03C000_XY_MIN_FILTER                       0xFFFFE7FF
#define   S_03C000_Z_FILTER(x)                         (((unsigned)(x) & 0x3) << 13)
#define   G_03C000_Z_FILTER(x)                         (((x) >> 13) & 0x3)
#define   C_03C000_Z_FILTER                            0xFFFF9FFF
#define     V_03C000_SQ_TEX_Z_FILTER_NONE              0x00000000
#define     V_03C000_SQ_TEX_Z_FILTER_POINT             0x00000001
#define     V_03C000_SQ_TEX_Z_FILTER_LINEAR            0x00000002
#define   S_03C000_MIP_FILTER(x)                       (((unsigned)(x) & 0x3) << 15)
#define   G_03C000_MIP_FILTER(x)                       (((x) >> 15) & 0x3)
#define   C_03C000_MIP_FILTER                          0xFFFE7FFF
#define   S_03C000_MAX_ANISO_RATIO(x)                  (((unsigned)(x) & 0x7) << 17)
#define   G_03C000_MAX_ANISO_RATIO(x)                  (((x) >> 17) & 0x7)
#define   C_03C000_MAX_ANISO_RATIO                     0xFFF1FFFF
#define   S_03C000_BORDER_COLOR_TYPE(x)                (((unsigned)(x) & 0x3) << 20)
#define   G_03C000_BORDER_COLOR_TYPE(x)                (((x) >> 20) & 0x3)
#define   C_03C000_BORDER_COLOR_TYPE                   0xFFCFFFFF
#define     V_03C000_SQ_TEX_BORDER_COLOR_TRANS_BLACK   0x00000000
#define     V_03C000_SQ_TEX_BORDER_COLOR_OPAQUE_BLACK  0x00000001
#define     V_03C000_SQ_TEX_BORDER_COLOR_OPAQUE_WHITE  0x00000002
#define     V_03C000_SQ_TEX_BORDER_COLOR_REGISTER      0x00000003
#define   S_03C000_DEPTH_COMPARE_FUNCTION(x)           (((unsigned)(x) & 0x7) << 22)
#define   G_03C000_DEPTH_COMPARE_FUNCTION(x)           (((x) >> 22) & 0x7)
#define   C_03C000_DEPTH_COMPARE_FUNCTION              0xFE3FFFFF
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_NEVER        0x00000000
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_LESS         0x00000001
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_EQUAL        0x00000002
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_LESSEQUAL    0x00000003
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_GREATER      0x00000004
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_NOTEQUAL     0x00000005
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_GREATEREQUAL 0x00000006
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_ALWAYS       0x00000007
#define   S_03C000_CHROMA_KEY(x)                       (((unsigned)(x) & 0x3) << 25)
#define   G_03C000_CHROMA_KEY(x)                       (((x) >> 25) & 0x3)
#define   C_03C000_CHROMA_KEY                          0xF9FFFFFF
#define     V_03C000_SQ_TEX_CHROMA_KEY_DISABLE         0x00000000
#define     V_03C000_SQ_TEX_CHROMA_KEY_KILL            0x00000001
#define     V_03C000_SQ_TEX_CHROMA_KEY_BLEND           0x00000002

#define R_03C004_SQ_TEX_SAMPLER_WORD1_0              0x03C004
#define   S_03C004_MIN_LOD(x)                          (((unsigned)(x) & 0xFFF) << 0)
#define   G_03C004_MIN_LOD(x)                          (((x) >> 0) & 0xFFF)
#define   C_03C004_MIN_LOD                             0xFFFFF000
#define   S_03C004_MAX_LOD(x)                          (((unsigned)(x) & 0xFFF) << 12)
#define   G_03C004_MAX_LOD(x)                          (((x) >> 12) & 0xFFF)
#define   C_03C004_MAX_LOD                             0xFF000FFF

#define   S_03C004_PERF_MIP(x)                         (((unsigned)(x) & 0xF) << 24)
#define   G_03C004_PERF_MIP(x)                         (((x) >> 24) & 0xF)
#define   C_03C004_PERF_MIP                            0xF0FFFFFF
#define   S_03C004_PERF_Z(x)                           (((unsigned)(x) & 0xF) << 28)
#define   G_03C004_PERF_Z(x)                           (((x) >> 24) & 0xF)
#define   C_03C004_PERF_Z                              0x0FFFFFFF

#define R_03C008_SQ_TEX_SAMPLER_WORD2_0              0x03C008
#define   S_03C008_LOD_BIAS(x)                         (((unsigned)(x) & 0x3FFF) << 0)
#define   G_03C008_LOD_BIAS(x)                         (((x) >> 0) & 0x3FFF)
#define   C_03C008_LOD_BIAS                            0xFFFFC000
#define   S_03C008_LOD_BIAS_SEC(x)                     (((unsigned)(x) & 0x3F) << 14)
#define   G_03C008_LOD_BIAS_SEC(x)                     (((x) >> 14) & 0x3F)
#define   C_03C008_LOD_BIAS_SEC                        0xFFF03FFF
#define   S_03C008_MC_COORD_TRUNCATE(x)                (((unsigned)(x) & 0x1) << 20)
#define   G_03C008_MC_COORD_TRUNCATE(x)                (((x) >> 20) & 0x1)
#define   C_03C008_MC_COORD_TRUNCATE                   0xFFEFFFFF
#define   S_03C008_FORCE_DEGAMMA(x)                    (((unsigned)(x) & 0x1) << 21)
#define   G_03C008_FORCE_DEGAMMA(x)                    (((x) >> 21) & 0x1)
#define   C_03C008_FORCE_DEGAMMA                       0xFFDFFFFF
#define   S_03C008_ANISO_BIAS(x)                       (((unsigned)(x) & 0x3f) << 22)
#define   G_03C008_ANISO_BIAS(x)                       (((x) >> 22) & 0x3f)
#define   C_03C008_ANISO_BIAS                          (~(0x3f << 22))
#define   S_03C008_TRUNCATE_COORD(x)                   (((unsigned)(x) & 0x1) << 28)
#define   G_03C008_TRUNCATE_COORD(x)                   (((x) >> 28) & 0x1)
#define   C_03C008_TRUNCATE_COORD                      (~(1 << 28))
#define   S_03C008_DISABLE_CUBE_WRAP(x)                (((unsigned)(x) & 0x1) << 29)
#define   G_03C008_DISABLE_CUBE_WRAP(x)                (((x) >> 29) & 0x1)
#define   C_03C008_DISABLE_CUBE_WRAP                   (~(1 << 29))
#define   S_03C008_TYPE(x)                             (((unsigned)(x) & 0x1) << 31)
#define   G_03C008_TYPE(x)                             (((x) >> 31) & 0x1)
#define   C_03C008_TYPE                                0x7FFFFFFF

#define R_008958_VGT_PRIMITIVE_TYPE                  0x008958
#define   S_008958_PRIM_TYPE(x)                        (((unsigned)(x) & 0x3F) << 0)
#define   G_008958_PRIM_TYPE(x)                        (((x) >> 0) & 0x3F)
#define   C_008958_PRIM_TYPE                           0xFFFFFFC0
#define     V_008958_DI_PT_NONE                        0x00000000
#define     V_008958_DI_PT_POINTLIST                   0x00000001
#define     V_008958_DI_PT_LINELIST                    0x00000002
#define     V_008958_DI_PT_LINESTRIP                   0x00000003
#define     V_008958_DI_PT_TRILIST                     0x00000004
#define     V_008958_DI_PT_TRIFAN                      0x00000005
#define     V_008958_DI_PT_TRISTRIP                    0x00000006
#define     V_008958_DI_PT_UNUSED_0                    0x00000007
#define     V_008958_DI_PT_UNUSED_1                    0x00000008
#define     V_008958_DI_PT_PATCH                       0x00000009
#define     V_008958_DI_PT_LINELIST_ADJ                0x0000000A
#define     V_008958_DI_PT_LINESTRIP_ADJ               0x0000000B
#define     V_008958_DI_PT_TRILIST_ADJ                 0x0000000C
#define     V_008958_DI_PT_TRISTRIP_ADJ                0x0000000D
#define     V_008958_DI_PT_UNUSED_3                    0x0000000E
#define     V_008958_DI_PT_UNUSED_4                    0x0000000F
#define     V_008958_DI_PT_TRI_WITH_WFLAGS             0x00000010
#define     V_008958_DI_PT_RECTLIST                    0x00000011
#define     V_008958_DI_PT_LINELOOP                    0x00000012
#define     V_008958_DI_PT_QUADLIST                    0x00000013
#define     V_008958_DI_PT_QUADSTRIP                   0x00000014
#define     V_008958_DI_PT_POLYGON                     0x00000015
#define     V_008958_DI_PT_2D_COPY_RECT_LIST_V0        0x00000016
#define     V_008958_DI_PT_2D_COPY_RECT_LIST_V1        0x00000017
#define     V_008958_DI_PT_2D_COPY_RECT_LIST_V2        0x00000018
#define     V_008958_DI_PT_2D_COPY_RECT_LIST_V3        0x00000019
#define     V_008958_DI_PT_2D_FILL_RECT_LIST           0x0000001A
#define     V_008958_DI_PT_2D_LINE_STRIP               0x0000001B
#define     V_008958_DI_PT_2D_TRI_STRIP                0x0000001C
#define R_02881C_PA_CL_VS_OUT_CNTL                   0x02881C
#define   S_02881C_CLIP_DIST_ENA_0(x)                  (((unsigned)(x) & 0x1) << 0)
#define   G_02881C_CLIP_DIST_ENA_0(x)                  (((x) >> 0) & 0x1)
#define   C_02881C_CLIP_DIST_ENA_0                     0xFFFFFFFE
#define   S_02881C_CLIP_DIST_ENA_1(x)                  (((unsigned)(x) & 0x1) << 1)
#define   G_02881C_CLIP_DIST_ENA_1(x)                  (((x) >> 1) & 0x1)
#define   C_02881C_CLIP_DIST_ENA_1                     0xFFFFFFFD
#define   S_02881C_CLIP_DIST_ENA_2(x)                  (((unsigned)(x) & 0x1) << 2)
#define   G_02881C_CLIP_DIST_ENA_2(x)                  (((x) >> 2) & 0x1)
#define   C_02881C_CLIP_DIST_ENA_2                     0xFFFFFFFB
#define   S_02881C_CLIP_DIST_ENA_3(x)                  (((unsigned)(x) & 0x1) << 3)
#define   G_02881C_CLIP_DIST_ENA_3(x)                  (((x) >> 3) & 0x1)
#define   C_02881C_CLIP_DIST_ENA_3                     0xFFFFFFF7
#define   S_02881C_CLIP_DIST_ENA_4(x)                  (((unsigned)(x) & 0x1) << 4)
#define   G_02881C_CLIP_DIST_ENA_4(x)                  (((x) >> 4) & 0x1)
#define   C_02881C_CLIP_DIST_ENA_4                     0xFFFFFFEF
#define   S_02881C_CLIP_DIST_ENA_5(x)                  (((unsigned)(x) & 0x1) << 5)
#define   G_02881C_CLIP_DIST_ENA_5(x)                  (((x) >> 5) & 0x1)
#define   C_02881C_CLIP_DIST_ENA_5                     0xFFFFFFDF
#define   S_02881C_CLIP_DIST_ENA_6(x)                  (((unsigned)(x) & 0x1) << 6)
#define   G_02881C_CLIP_DIST_ENA_6(x)                  (((x) >> 6) & 0x1)
#define   C_02881C_CLIP_DIST_ENA_6                     0xFFFFFFBF
#define   S_02881C_CLIP_DIST_ENA_7(x)                  (((unsigned)(x) & 0x1) << 7)
#define   G_02881C_CLIP_DIST_ENA_7(x)                  (((x) >> 7) & 0x1)
#define   C_02881C_CLIP_DIST_ENA_7                     0xFFFFFF7F
#define   S_02881C_CULL_DIST_ENA_0(x)                  (((unsigned)(x) & 0x1) << 8)
#define   G_02881C_CULL_DIST_ENA_0(x)                  (((x) >> 8) & 0x1)
#define   C_02881C_CULL_DIST_ENA_0                     0xFFFFFEFF
#define   S_02881C_CULL_DIST_ENA_1(x)                  (((unsigned)(x) & 0x1) << 9)
#define   G_02881C_CULL_DIST_ENA_1(x)                  (((x) >> 9) & 0x1)
#define   C_02881C_CULL_DIST_ENA_1                     0xFFFFFDFF
#define   S_02881C_CULL_DIST_ENA_2(x)                  (((unsigned)(x) & 0x1) << 10)
#define   G_02881C_CULL_DIST_ENA_2(x)                  (((x) >> 10) & 0x1)
#define   C_02881C_CULL_DIST_ENA_2                     0xFFFFFBFF
#define   S_02881C_CULL_DIST_ENA_3(x)                  (((unsigned)(x) & 0x1) << 11)
#define   G_02881C_CULL_DIST_ENA_3(x)                  (((x) >> 11) & 0x1)
#define   C_02881C_CULL_DIST_ENA_3                     0xFFFFF7FF
#define   S_02881C_CULL_DIST_ENA_4(x)                  (((unsigned)(x) & 0x1) << 12)
#define   G_02881C_CULL_DIST_ENA_4(x)                  (((x) >> 12) & 0x1)
#define   C_02881C_CULL_DIST_ENA_4                     0xFFFFEFFF
#define   S_02881C_CULL_DIST_ENA_5(x)                  (((unsigned)(x) & 0x1) << 13)
#define   G_02881C_CULL_DIST_ENA_5(x)                  (((x) >> 13) & 0x1)
#define   C_02881C_CULL_DIST_ENA_5                     0xFFFFDFFF
#define   S_02881C_CULL_DIST_ENA_6(x)                  (((unsigned)(x) & 0x1) << 14)
#define   G_02881C_CULL_DIST_ENA_6(x)                  (((x) >> 14) & 0x1)
#define   C_02881C_CULL_DIST_ENA_6                     0xFFFFBFFF
#define   S_02881C_CULL_DIST_ENA_7(x)                  (((unsigned)(x) & 0x1) << 15)
#define   G_02881C_CULL_DIST_ENA_7(x)                  (((x) >> 15) & 0x1)
#define   C_02881C_CULL_DIST_ENA_7                     0xFFFF7FFF
#define   S_02881C_USE_VTX_POINT_SIZE(x)               (((unsigned)(x) & 0x1) << 16)
#define   G_02881C_USE_VTX_POINT_SIZE(x)               (((x) >> 16) & 0x1)
#define   C_02881C_USE_VTX_POINT_SIZE                  0xFFFEFFFF
#define   S_02881C_USE_VTX_EDGE_FLAG(x)                (((unsigned)(x) & 0x1) << 17)
#define   G_02881C_USE_VTX_EDGE_FLAG(x)                (((x) >> 17) & 0x1)
#define   C_02881C_USE_VTX_EDGE_FLAG                   0xFFFDFFFF
#define   S_02881C_USE_VTX_RENDER_TARGET_INDX(x)       (((unsigned)(x) & 0x1) << 18)
#define   G_02881C_USE_VTX_RENDER_TARGET_INDX(x)       (((x) >> 18) & 0x1)
#define   C_02881C_USE_VTX_RENDER_TARGET_INDX          0xFFFBFFFF
#define   S_02881C_USE_VTX_VIEWPORT_INDX(x)            (((unsigned)(x) & 0x1) << 19)
#define   G_02881C_USE_VTX_VIEWPORT_INDX(x)            (((x) >> 19) & 0x1)
#define   C_02881C_USE_VTX_VIEWPORT_INDX               0xFFF7FFFF
#define   S_02881C_USE_VTX_KILL_FLAG(x)                (((unsigned)(x) & 0x1) << 20)
#define   G_02881C_USE_VTX_KILL_FLAG(x)                (((x) >> 20) & 0x1)
#define   C_02881C_USE_VTX_KILL_FLAG                   0xFFEFFFFF
#define   S_02881C_VS_OUT_MISC_VEC_ENA(x)              (((unsigned)(x) & 0x1) << 21)
#define   G_02881C_VS_OUT_MISC_VEC_ENA(x)              (((x) >> 21) & 0x1)
#define   C_02881C_VS_OUT_MISC_VEC_ENA                 0xFFDFFFFF
#define   S_02881C_VS_OUT_CCDIST0_VEC_ENA(x)           (((unsigned)(x) & 0x1) << 22)
#define   G_02881C_VS_OUT_CCDIST0_VEC_ENA(x)           (((x) >> 22) & 0x1)
#define   C_02881C_VS_OUT_CCDIST0_VEC_ENA              0xFFBFFFFF
#define   S_02881C_VS_OUT_CCDIST1_VEC_ENA(x)           (((unsigned)(x) & 0x1) << 23)
#define   G_02881C_VS_OUT_CCDIST1_VEC_ENA(x)           (((x) >> 23) & 0x1)
#define   C_02881C_VS_OUT_CCDIST1_VEC_ENA              0xFF7FFFFF
/* diff */
#define R_028860_SQ_PGM_RESOURCES_VS                 0x028860
#define   S_028860_NUM_GPRS(x)                         (((unsigned)(x) & 0xFF) << 0)
#define   G_028860_NUM_GPRS(x)                         (((x) >> 0) & 0xFF)
#define   C_028860_NUM_GPRS                            0xFFFFFF00
#define   S_028860_STACK_SIZE(x)                       (((unsigned)(x) & 0xFF) << 8)
#define   G_028860_STACK_SIZE(x)                       (((x) >> 8) & 0xFF)
#define   C_028860_STACK_SIZE                          0xFFFF00FF
#define   S_028860_DX10_CLAMP(x)                       (((unsigned)(x) & 0x1) << 21)
#define   G_028860_DX10_CLAMP(x)                       (((x) >> 21) & 0x1)
#define   C_028860_DX10_CLAMP                          0xFFDFFFFF
#define   S_028860_UNCACHED_FIRST_INST(x)              (((unsigned)(x) & 0x1) << 28)
#define   G_028860_UNCACHED_FIRST_INST(x)              (((x) >> 28) & 0x1)
#define   C_028860_UNCACHED_FIRST_INST                 0xEFFFFFFF

#define R_028878_SQ_PGM_RESOURCES_GS                 0x028878
#define   S_028878_NUM_GPRS(x)                         (((unsigned)(x) & 0xFF) << 0)
#define   G_028878_NUM_GPRS(x)                         (((x) >> 0) & 0xFF)
#define   C_028878_NUM_GPRS                            0xFFFFFF00
#define   S_028878_STACK_SIZE(x)                       (((unsigned)(x) & 0xFF) << 8)
#define   G_028878_STACK_SIZE(x)                       (((x) >> 8) & 0xFF)
#define   C_028878_STACK_SIZE                          0xFFFF00FF
#define   S_028878_DX10_CLAMP(x)                       (((unsigned)(x) & 0x1) << 21)
#define   G_028878_DX10_CLAMP(x)                       (((x) >> 21) & 0x1)
#define   C_028878_DX10_CLAMP                          0xFFDFFFFF
#define   S_028878_UNCACHED_FIRST_INST(x)              (((unsigned)(x) & 0x1) << 28)
#define   G_028878_UNCACHED_FIRST_INST(x)              (((x) >> 28) & 0x1)
#define   C_028878_UNCACHED_FIRST_INST                 0xEFFFFFFF
#define R_02887C_SQ_PGM_RESOURCES_2_GS                 0x02887C

#define R_028890_SQ_PGM_RESOURCES_ES                 0x028890
#define   S_028890_NUM_GPRS(x)                         (((unsigned)(x) & 0xFF) << 0)
#define   G_028890_NUM_GPRS(x)                         (((x) >> 0) & 0xFF)
#define   C_028890_NUM_GPRS                            0xFFFFFF00
#define   S_028890_STACK_SIZE(x)                       (((unsigned)(x) & 0xFF) << 8)
#define   G_028890_STACK_SIZE(x)                       (((x) >> 8) & 0xFF)
#define   C_028890_STACK_SIZE                          0xFFFF00FF
#define   S_028890_DX10_CLAMP(x)                       (((unsigned)(x) & 0x1) << 21)
#define   G_028890_DX10_CLAMP(x)                       (((x) >> 21) & 0x1)
#define   C_028890_DX10_CLAMP                          0xFFDFFFFF
#define   S_028890_UNCACHED_FIRST_INST(x)              (((unsigned)(x) & 0x1) << 28)
#define   G_028890_UNCACHED_FIRST_INST(x)              (((x) >> 28) & 0x1)
#define   C_028890_UNCACHED_FIRST_INST                 0xEFFFFFFF
#define R_028894_SQ_PGM_RESOURCES_2_ES                 0x028894

#define R_028864_SQ_PGM_RESOURCES_2_VS               0x028864
#define   S_028864_SINGLE_ROUND(x)                     (((unsigned)(x) & 0x3) << 0)
#define   G_028864_SINGLE_ROUND(x)                     (((x) >> 0) & 0x3)
#define   C_028864_SINGLE_ROUND                        0xFFFFFFFC
#define     V_SQ_ROUND_NEAREST_EVEN                    0x00
#define     V_SQ_ROUND_PLUS_INFINITY                   0x01
#define     V_SQ_ROUND_MINUS_INFINITY                  0x02
#define     V_SQ_ROUND_TO_ZERO                         0x03
#define   S_028864_DOUBLE_ROUND(x)                     (((unsigned)(x) & 0x3) << 2)
#define   G_028864_DOUBLE_ROUND(x)                     (((x) >> 2) & 0x3)
#define   C_028864_DOUBLE_ROUND                        0xFFFFFFF3
#define   S_028864_ALLOW_SINGLE_DENORM_IN(x)           (((unsigned)(x) & 0x1) << 4)
#define   G_028864_ALLOW_SINGLE_DENORM_IN(x)           (((x) >> 4) & 0x1)
#define   C_028864_ALLOW_SINGLE_DENORM_IN              0xFFFFFFEF
#define   S_028864_ALLOW_SINGLE_DENORM_OUT(x)          (((unsigned)(x) & 0x1) << 5)
#define   G_028864_ALLOW_SINGLE_DENORM_OUT(x)          (((x) >> 5) & 0x1)
#define   C_028864_ALLOW_SINGLE_DENORM_OUT             0xFFFFFFDF
#define   S_028864_ALLOW_DOUBLE_DENORM_IN(x)           (((unsigned)(x) & 0x1) << 6)
#define   G_028864_ALLOW_DOUBLE_DENORM_IN(x)           (((x) >> 6) & 0x1)
#define   C_028864_ALLOW_DOUBLE_DENORM_IN              0xFFFFFFBF
#define   S_028864_ALLOW_DOUBLE_DENORM_OUT(x)          (((unsigned)(x) & 0x1) << 7)
#define   G_028864_ALLOW_DOUBLE_DENORM_OUT(x)          (((x) >> 7) & 0x1)
#define   C_028864_ALLOW_DOUBLE_DENORM_OUT             0xFFFFFF7F

#define R_028844_SQ_PGM_RESOURCES_PS                 0x028844
#define   S_028844_NUM_GPRS(x)                         (((unsigned)(x) & 0xFF) << 0)
#define   G_028844_NUM_GPRS(x)                         (((x) >> 0) & 0xFF)
#define   C_028844_NUM_GPRS                            0xFFFFFF00
#define   S_028844_STACK_SIZE(x)                       (((unsigned)(x) & 0xFF) << 8)
#define   G_028844_STACK_SIZE(x)                       (((x) >> 8) & 0xFF)
#define   C_028844_STACK_SIZE                          0xFFFF00FF
#define   S_028844_DX10_CLAMP(x)                       (((unsigned)(x) & 0x1) << 21)
#define   G_028844_DX10_CLAMP(x)                       (((x) >> 21) & 0x1)
#define   C_028844_DX10_CLAMP                          0xFFDFFFFF
#define   S_028844_PRIME_CACHE_ON_DRAW(x)              (((unsigned)(x) & 0x1) << 23)
#define   G_028844_PRIME_CACHE_ON_DRAW(x)              (((x) >> 23) & 0x1)
#define   C_028844_PRIME_CACHE_ON_DRAW                 0xFF7FFFFF
#define   S_028844_UNCACHED_FIRST_INST(x)              (((unsigned)(x) & 0x1) << 28)
#define   G_028844_UNCACHED_FIRST_INST(x)              (((x) >> 28) & 0x1)
#define   C_028844_UNCACHED_FIRST_INST                 0xEFFFFFFF
#define   S_028844_CLAMP_CONSTS(x)                     (((unsigned)(x) & 0x1) << 31)
#define   G_028844_CLAMP_CONSTS(x)                     (((x) >> 31) & 0x1)
#define   C_028844_CLAMP_CONSTS                        0x7FFFFFFF

#define R_028848_SQ_PGM_RESOURCES_2_PS               0x028848
#define   S_028848_SINGLE_ROUND(x)                     (((unsigned)(x) & 0x3) << 0)
#define   G_028848_SINGLE_ROUND(x)                     (((x) >> 0) & 0x3)
#define   C_028848_SINGLE_ROUND                        0xFFFFFFFC
#define   S_028848_DOUBLE_ROUND(x)                     (((unsigned)(x) & 0x3) << 2)
#define   G_028848_DOUBLE_ROUND(x)                     (((x) >> 2) & 0x3)
#define   C_028848_DOUBLE_ROUND                        0xFFFFFFF3
#define   S_028848_ALLOW_SINGLE_DENORM_IN(x)           (((unsigned)(x) & 0x1) << 4)
#define   G_028848_ALLOW_SINGLE_DENORM_IN(x)           (((x) >> 4) & 0x1)
#define   C_028848_ALLOW_SINGLE_DENORM_IN              0xFFFFFFEF
#define   S_028848_ALLOW_SINGLE_DENORM_OUT(x)          (((unsigned)(x) & 0x1) << 5)
#define   G_028848_ALLOW_SINGLE_DENORM_OUT(x)          (((x) >> 5) & 0x1)
#define   C_028848_ALLOW_SINGLE_DENORM_OUT             0xFFFFFFDF
#define   S_028848_ALLOW_DOUBLE_DENORM_IN(x)           (((unsigned)(x) & 0x1) << 6)
#define   G_028848_ALLOW_DOUBLE_DENORM_IN(x)           (((x) >> 6) & 0x1)
#define   C_028848_ALLOW_DOUBLE_DENORM_IN              0xFFFFFFBF
#define   S_028848_ALLOW_DOUBLE_DENORM_OUT(x)          (((unsigned)(x) & 0x1) << 7)
#define   G_028848_ALLOW_DOUBLE_DENORM_OUT(x)          (((x) >> 7) & 0x1)
#define   C_028848_ALLOW_DOUBLE_DENORM_OUT             0xFFFFFF7F

#define R_0288BC_SQ_PGM_RESOURCES_HS                 0x0288BC
#define   S_0288BC_NUM_GPRS(x)                         (((unsigned)(x) & 0xFF) << 0)
#define   G_0288BC_NUM_GPRS(x)                         (((x) >> 0) & 0xFF)
#define   C_0288BC_NUM_GPRS                            0xFFFFFF00
#define   S_0288BC_STACK_SIZE(x)                       (((unsigned)(x) & 0xFF) << 8)
#define   G_0288BC_STACK_SIZE(x)                       (((x) >> 8) & 0xFF)
#define   C_0288BC_STACK_SIZE                          0xFFFF00FF
#define   S_0288BC_DX10_CLAMP(x)                       (((unsigned)(x) & 0x1) << 21)
#define   G_0288BC_DX10_CLAMP(x)                       (((x) >> 21) & 0x1)
#define   C_0288BC_DX10_CLAMP                          0xFFDFFFFF
#define   S_0288BC_PRIME_CACHE_ON_DRAW(x)              (((unsigned)(x) & 0x1) << 23)
#define   G_0288BC_PRIME_CACHE_ON_DRAW(x)              (((x) >> 23) & 0x1)
#define   C_028844_PRIME_CACHE_ON_DRAW                 0xFF7FFFFF
#define   S_0288BC_UNCACHED_FIRST_INST(x)              (((unsigned)(x) & 0x1) << 28)
#define   G_0288BC_UNCACHED_FIRST_INST(x)              (((x) >> 28) & 0x1)
#define   C_0288BC_UNCACHED_FIRST_INST                 0xEFFFFFFF

#define R_0288C0_SQ_PGM_RESOURCES_2_HS               0x0288c0

#define R_0288D4_SQ_PGM_RESOURCES_LS                 0x0288d4
#define   S_0288D4_NUM_GPRS(x)                         (((unsigned)(x) & 0xFF) << 0)
#define   G_0288D4_NUM_GPRS(x)                         (((x) >> 0) & 0xFF)
#define   C_0288D4_NUM_GPRS                            0xFFFFFF00
#define   S_0288D4_STACK_SIZE(x)                       (((unsigned)(x) & 0xFF) << 8)
#define   G_0288D4_STACK_SIZE(x)                       (((x) >> 8) & 0xFF)
#define   C_0288D4_STACK_SIZE                          0xFFFF00FF
#define   S_0288D4_DX10_CLAMP(x)                       (((unsigned)(x) & 0x1) << 21)
#define   G_0288D4_DX10_CLAMP(x)                       (((x) >> 21) & 0x1)
#define   C_0288D4_DX10_CLAMP                          0xFFDFFFFF
#define   S_0288D4_PRIME_CACHE_ON_DRAW(x)              (((unsigned)(x) & 0x1) << 23)
#define   G_0288D4_PRIME_CACHE_ON_DRAW(x)              (((x) >> 23) & 0x1)
#define   S_0288D4_UNCACHED_FIRST_INST(x)              (((unsigned)(x) & 0x1) << 28)
#define   G_0288D4_UNCACHED_FIRST_INST(x)              (((x) >> 28) & 0x1)
#define   C_0288D4_UNCACHED_FIRST_INST                 0xEFFFFFFF

#define R_0288D8_SQ_PGM_RESOURCES_2_LS               0x0288d8

#define R_028644_SPI_PS_INPUT_CNTL_0                 0x028644
#define   S_028644_SEMANTIC(x)                         (((unsigned)(x) & 0xFF) << 0)
#define   G_028644_SEMANTIC(x)                         (((x) >> 0) & 0xFF)
#define   C_028644_SEMANTIC                            0xFFFFFF00
#define   S_028644_DEFAULT_VAL(x)                      (((unsigned)(x) & 0x3) << 8)
#define   G_028644_DEFAULT_VAL(x)                      (((x) >> 8) & 0x3)
#define   C_028644_DEFAULT_VAL                         0xFFFFFCFF
#define   S_028644_FLAT_SHADE(x)                       (((unsigned)(x) & 0x1) << 10)
#define   G_028644_FLAT_SHADE(x)                       (((x) >> 10) & 0x1)
#define   C_028644_FLAT_SHADE                          0xFFFFFBFF
#define   S_028644_SEL_CENTROID(x)                     (((unsigned)(x) & 0x1) << 11)
#define   G_028644_SEL_CENTROID(x)                     (((x) >> 11) & 0x1)
#define   C_028644_SEL_CENTROID                        0xFFFFF7FF
#define   S_028644_SEL_LINEAR(x)                       (((unsigned)(x) & 0x1) << 12)
#define   G_028644_SEL_LINEAR(x)                       (((x) >> 12) & 0x1)
#define   C_028644_SEL_LINEAR                          0xFFFFEFFF
#define   S_028644_CYL_WRAP(x)                         (((unsigned)(x) & 0xF) << 13)
#define   G_028644_CYL_WRAP(x)                         (((x) >> 13) & 0xF)
#define   C_028644_CYL_WRAP                            0xFFFE1FFF
#define   S_028644_PT_SPRITE_TEX(x)                    (((unsigned)(x) & 0x1) << 17)
#define   G_028644_PT_SPRITE_TEX(x)                    (((x) >> 17) & 0x1)
#define   C_028644_PT_SPRITE_TEX                       0xFFFDFFFF
#define   S_028644_SEL_SAMPLE(x)                       (((unsigned)(x) & 0x1) << 18)
#define   G_028644_SEL_SAMPLE(x)                       (((x) >> 18) & 0x1)
#define   C_028644_SEL_SAMPLE                          0xFFFBFFFF
#define R_0286D4_SPI_INTERP_CONTROL_0                0x0286D4
#define   S_0286D4_FLAT_SHADE_ENA(x)                   (((unsigned)(x) & 0x1) << 0)
#define   G_0286D4_FLAT_SHADE_ENA(x)                   (((x) >> 0) & 0x1)
#define   C_0286D4_FLAT_SHADE_ENA                      0xFFFFFFFE
#define   S_0286D4_PNT_SPRITE_ENA(x)                   (((unsigned)(x) & 0x1) << 1)
#define   G_0286D4_PNT_SPRITE_ENA(x)                   (((x) >> 1) & 0x1)
#define   C_0286D4_PNT_SPRITE_ENA                      0xFFFFFFFD
#define   S_0286D4_PNT_SPRITE_OVRD_X(x)                (((unsigned)(x) & 0x7) << 2)
#define   G_0286D4_PNT_SPRITE_OVRD_X(x)                (((x) >> 2) & 0x7)
#define   C_0286D4_PNT_SPRITE_OVRD_X                   0xFFFFFFE3
#define   S_0286D4_PNT_SPRITE_OVRD_Y(x)                (((unsigned)(x) & 0x7) << 5)
#define   G_0286D4_PNT_SPRITE_OVRD_Y(x)                (((x) >> 5) & 0x7)
#define   C_0286D4_PNT_SPRITE_OVRD_Y                   0xFFFFFF1F
#define   S_0286D4_PNT_SPRITE_OVRD_Z(x)                (((unsigned)(x) & 0x7) << 8)
#define   G_0286D4_PNT_SPRITE_OVRD_Z(x)                (((x) >> 8) & 0x7)
#define   C_0286D4_PNT_SPRITE_OVRD_Z                   0xFFFFF8FF
#define   S_0286D4_PNT_SPRITE_OVRD_W(x)                (((unsigned)(x) & 0x7) << 11)
#define   G_0286D4_PNT_SPRITE_OVRD_W(x)                (((x) >> 11) & 0x7)
#define   C_0286D4_PNT_SPRITE_OVRD_W                   0xFFFFC7FF
#define   S_0286D4_PNT_SPRITE_TOP_1(x)                 (((unsigned)(x) & 0x1) << 14)
#define   G_0286D4_PNT_SPRITE_TOP_1(x)                 (((x) >> 14) & 0x1)
#define   C_0286D4_PNT_SPRITE_TOP_1                    0xFFFFBFFF

#define SQ_TEX_INST_LD 0x03
#define SQ_TEX_INST_GET_GRADIENTS_H 0x7
#define SQ_TEX_INST_GET_GRADIENTS_V 0x8

#define SQ_TEX_INST_SAMPLE 0x10
#define SQ_TEX_INST_SAMPLE_L 0x11
#define SQ_TEX_INST_SAMPLE_C 0x18

#define R_008A14_PA_CL_ENHANCE                       0x00008A14
#define R_008D8C_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ        0x00008D8C
#define R_028000_DB_RENDER_CONTROL                   0x00028000
#define   S_028000_DEPTH_CLEAR_ENABLE(x)               (((unsigned)(x) & 0x1) << 0)
#define   S_028000_STENCIL_CLEAR_ENABLE(x)             (((unsigned)(x) & 0x1) << 1)
#define   S_028000_DEPTH_COPY_ENABLE(x)                (((unsigned)(x) & 0x1) << 2)
#define   S_028000_STENCIL_COPY_ENABLE(x)              (((unsigned)(x) & 0x1) << 3)
#define   S_028000_RESUMMARIZE_ENABLE(x)               (((unsigned)(x) & 0x1) << 4)
#define   S_028000_STENCIL_COMPRESS_DISABLE(x)         (((unsigned)(x) & 0x1) << 5)
#define   S_028000_DEPTH_COMPRESS_DISABLE(x)           (((unsigned)(x) & 0x1) << 6)
#define   S_028000_COPY_CENTROID(x)                    (((unsigned)(x) & 0x1) << 7)
#define   S_028000_COPY_SAMPLE(x)                      (((unsigned)(x) & 0x7) << 8)
#define   S_028000_COLOR_DISABLE(x)                    (((unsigned)(x) & 0x1) << 12)
#define R_028004_DB_COUNT_CONTROL                    0x00028004
#define   S_028004_ZPASS_INCREMENT_DISABLE(x)     (((unsigned)(x) & 0x1) << 0)
#define   S_028004_PERFECT_ZPASS_COUNTS(x)        (((unsigned)(x) & 0x1) << 1)
#define   S_028004_SAMPLE_RATE(x)                 (((unsigned)(x) & 0x7) << 4) /* cayman only */
#define R_028008_DB_DEPTH_VIEW                       0x00028008
#define   S_028008_SLICE_START(x)                      (((unsigned)(x) & 0x7FF) << 0)
#define   G_028008_SLICE_START(x)                      (((x) >> 0) & 0x7FF)
#define   C_028008_SLICE_START                         0xFFFFF800
#define   S_028008_SLICE_MAX(x)                        (((unsigned)(x) & 0x7FF) << 13)
#define   G_028008_SLICE_MAX(x)                        (((x) >> 13) & 0x7FF)
#define   C_028008_SLICE_MAX                           0xFF001FFF
#define R_02800C_DB_RENDER_OVERRIDE                  0x0002800C
#define   V_02800C_FORCE_OFF                         0
#define   V_02800C_FORCE_ENABLE                      1
#define   V_02800C_FORCE_DISABLE                     2
#define   S_02800C_FORCE_HIZ_ENABLE(x)                 (((unsigned)(x) & 0x3) << 0)
#define   G_02800C_FORCE_HIZ_ENABLE(x)                 (((x) >> 0) & 0x3)
#define   C_02800C_FORCE_HIZ_ENABLE                    0xFFFFFFFC
#define   S_02800C_FORCE_HIS_ENABLE0(x)                (((unsigned)(x) & 0x3) << 2)
#define   G_02800C_FORCE_HIS_ENABLE0(x)                (((x) >> 2) & 0x3)
#define   C_02800C_FORCE_HIS_ENABLE0                   0xFFFFFFF3
#define   S_02800C_FORCE_HIS_ENABLE1(x)                (((unsigned)(x) & 0x3) << 4)
#define   G_02800C_FORCE_HIS_ENABLE1(x)                (((x) >> 4) & 0x3)
#define   C_02800C_FORCE_HIS_ENABLE1                   0xFFFFFFCF
#define   S_02800C_FORCE_SHADER_Z_ORDER(x)             (((unsigned)(x) & 0x1) << 6)
#define   G_02800C_FORCE_SHADER_Z_ORDER(x)             (((x) >> 6) & 0x1)
#define   C_02800C_FORCE_SHADER_Z_ORDER                0xFFFFFFBF
#define   S_02800C_FAST_Z_DISABLE(x)                   (((unsigned)(x) & 0x1) << 7)
#define   G_02800C_FAST_Z_DISABLE(x)                   (((x) >> 7) & 0x1)
#define   C_02800C_FAST_Z_DISABLE                      0xFFFFFF7F
#define   S_02800C_FAST_STENCIL_DISABLE(x)             (((unsigned)(x) & 0x1) << 8)
#define   G_02800C_FAST_STENCIL_DISABLE(x)             (((x) >> 8) & 0x1)
#define   C_02800C_FAST_STENCIL_DISABLE                0xFFFFFEFF
#define   S_02800C_NOOP_CULL_DISABLE(x)                (((unsigned)(x) & 0x1) << 9)
#define   G_02800C_NOOP_CULL_DISABLE(x)                (((x) >> 9) & 0x1)
#define   C_02800C_NOOP_CULL_DISABLE                   0xFFFFFDFF
#define   S_02800C_FORCE_COLOR_KILL(x)                 (((unsigned)(x) & 0x1) << 10)
#define   G_02800C_FORCE_COLOR_KILL(x)                 (((x) >> 10) & 0x1)
#define   C_02800C_FORCE_COLOR_KILL                    0xFFFFFBFF
#define   S_02800C_FORCE_Z_READ(x)                     (((unsigned)(x) & 0x1) << 11)
#define   G_02800C_FORCE_Z_READ(x)                     (((x) >> 11) & 0x1)
#define   C_02800C_FORCE_Z_READ                        0xFFFFF7FF
#define   S_02800C_FORCE_STENCIL_READ(x)               (((unsigned)(x) & 0x1) << 12)
#define   G_02800C_FORCE_STENCIL_READ(x)               (((x) >> 12) & 0x1)
#define   C_02800C_FORCE_STENCIL_READ                  0xFFFFEFFF
#define   S_02800C_FORCE_FULL_Z_RANGE(x)               (((unsigned)(x) & 0x3) << 13)
#define   G_02800C_FORCE_FULL_Z_RANGE(x)               (((x) >> 13) & 0x3)
#define   C_02800C_FORCE_FULL_Z_RANGE                  0xFFFF9FFF
#define   S_02800C_FORCE_QC_SMASK_CONFLICT(x)          (((unsigned)(x) & 0x1) << 15)
#define   G_02800C_FORCE_QC_SMASK_CONFLICT(x)          (((x) >> 15) & 0x1)
#define   C_02800C_FORCE_QC_SMASK_CONFLICT             0xFFFF7FFF
#define   S_02800C_DISABLE_VIEWPORT_CLAMP(x)           (((unsigned)(x) & 0x1) << 16)
#define   G_02800C_DISABLE_VIEWPORT_CLAMP(x)           (((x) >> 16) & 0x1)
#define   C_02800C_DISABLE_VIEWPORT_CLAMP              0xFFFEFFFF
#define   S_02800C_IGNORE_SC_ZRANGE(x)                 (((unsigned)(x) & 0x1) << 17)
#define   G_02800C_IGNORE_SC_ZRANGE(x)                 (((x) >> 17) & 0x1)
#define   C_02800C_IGNORE_SC_ZRANGE                    0xFFFDFFFF
#define   S_02800C_DISABLE_PIXEL_RATE_TILES(x)         (((unsigned)(x) & 0x1) << 26)
#define   G_02800C_DISABLE_PIXEL_RATE_TILES(x)         (((x) >> 26) & 0x1)
#define   C_02800C_DISABLE_PIXEL_RATE_TILES            0xFFFDFFFF
#define R_028010_DB_RENDER_OVERRIDE2                 0x00028010
#define R_028014_DB_HTILE_DATA_BASE                  0x00028014
#define R_028028_DB_STENCIL_CLEAR                    0x00028028
#define R_02802C_DB_DEPTH_CLEAR                      0x0002802C
#define R_028048_DB_Z_READ_BASE                      0x00028048
#define R_02804C_DB_STENCIL_READ_BASE                0x0002804C
#define R_028050_DB_Z_WRITE_BASE                     0x00028050
#define R_028054_DB_STENCIL_WRITE_BASE               0x00028054
#define R_028140_ALU_CONST_BUFFER_SIZE_PS_0          0x00028140
#define R_028144_ALU_CONST_BUFFER_SIZE_PS_1          0x00028144
#define R_028180_ALU_CONST_BUFFER_SIZE_VS_0          0x00028180
#define R_028184_ALU_CONST_BUFFER_SIZE_VS_1          0x00028184
#define R_0281C0_ALU_CONST_BUFFER_SIZE_GS_0          0x000281C0
#define R_028F80_ALU_CONST_BUFFER_SIZE_HS_0          0x00028F80
#define R_028FC0_ALU_CONST_BUFFER_SIZE_LS_0          0x00028FC0
#define R_028200_PA_SC_WINDOW_OFFSET                 0x00028200
#define R_02820C_PA_SC_CLIPRECT_RULE                 0x0002820C
#define R_028210_PA_SC_CLIPRECT_0_TL                 0x00028210
#define R_028214_PA_SC_CLIPRECT_0_BR                 0x00028214
#define R_028218_PA_SC_CLIPRECT_1_TL                 0x00028218
#define R_02821C_PA_SC_CLIPRECT_1_BR                 0x0002821C
#define R_028220_PA_SC_CLIPRECT_2_TL                 0x00028220
#define R_028224_PA_SC_CLIPRECT_2_BR                 0x00028224
#define R_028228_PA_SC_CLIPRECT_3_TL                 0x00028228
#define R_02822C_PA_SC_CLIPRECT_3_BR                 0x0002822C
#define R_028230_PA_SC_EDGERULE                      0x00028230
#define R_028234_PA_SU_HARDWARE_SCREEN_OFFSET        0x00028234
#define R_028238_CB_TARGET_MASK                      0x00028238
#define R_02823C_CB_SHADER_MASK                      0x0002823C
#define R_028350_SX_MISC                             0x00028350
#define   S_028350_MULTIPASS(x)                        (((unsigned)(x) & 0x1) << 0)
#define   G_028350_MULTIPASS(x)                        (((x) >> 0) & 0x1)
#define   C_028350_MULTIPASS                           0xFFFFFFFE
#define R_028354_SX_SURFACE_SYNC                     0x00028354
#define   S_028354_SURFACE_SYNC_MASK(x)                (((unsigned)(x) & 0x1FF) << 0)
#define R_028380_SQ_VTX_SEMANTIC_0                   0x00028380
#define R_028384_SQ_VTX_SEMANTIC_1                   0x00028384
#define R_028388_SQ_VTX_SEMANTIC_2                   0x00028388
#define R_02838C_SQ_VTX_SEMANTIC_3                   0x0002838C
#define R_028390_SQ_VTX_SEMANTIC_4                   0x00028390
#define R_028394_SQ_VTX_SEMANTIC_5                   0x00028394
#define R_028398_SQ_VTX_SEMANTIC_6                   0x00028398
#define R_02839C_SQ_VTX_SEMANTIC_7                   0x0002839C
#define R_0283A0_SQ_VTX_SEMANTIC_8                   0x000283A0
#define R_0283A4_SQ_VTX_SEMANTIC_9                   0x000283A4
#define R_0283A8_SQ_VTX_SEMANTIC_10                  0x000283A8
#define R_0283AC_SQ_VTX_SEMANTIC_11                  0x000283AC
#define R_0283B0_SQ_VTX_SEMANTIC_12                  0x000283B0
#define R_0283B4_SQ_VTX_SEMANTIC_13                  0x000283B4
#define R_0283B8_SQ_VTX_SEMANTIC_14                  0x000283B8
#define R_0283BC_SQ_VTX_SEMANTIC_15                  0x000283BC
#define R_0283C0_SQ_VTX_SEMANTIC_16                  0x000283C0
#define R_0283C4_SQ_VTX_SEMANTIC_17                  0x000283C4
#define R_0283C8_SQ_VTX_SEMANTIC_18                  0x000283C8
#define R_0283CC_SQ_VTX_SEMANTIC_19                  0x000283CC
#define R_0283D0_SQ_VTX_SEMANTIC_20                  0x000283D0
#define R_0283D4_SQ_VTX_SEMANTIC_21                  0x000283D4
#define R_0283D8_SQ_VTX_SEMANTIC_22                  0x000283D8
#define R_0283DC_SQ_VTX_SEMANTIC_23                  0x000283DC
#define R_0283E0_SQ_VTX_SEMANTIC_24                  0x000283E0
#define R_0283E4_SQ_VTX_SEMANTIC_25                  0x000283E4
#define R_0283E8_SQ_VTX_SEMANTIC_26                  0x000283E8
#define R_0283EC_SQ_VTX_SEMANTIC_27                  0x000283EC
#define R_0283F0_SQ_VTX_SEMANTIC_28                  0x000283F0
#define R_0283F4_SQ_VTX_SEMANTIC_29                  0x000283F4
#define R_0283F8_SQ_VTX_SEMANTIC_30                  0x000283F8
#define R_0283FC_SQ_VTX_SEMANTIC_31                  0x000283FC
#define R_0288F0_SQ_VTX_SEMANTIC_CLEAR               0x000288F0
#define R_0282D0_PA_SC_VPORT_ZMIN_0                  0x0282D0
#define R_0282D4_PA_SC_VPORT_ZMAX_0                  0x0282D4
#define R_028400_VGT_MAX_VTX_INDX                    0x00028400
#define R_028404_VGT_MIN_VTX_INDX                    0x00028404
#define R_028408_VGT_INDX_OFFSET                     0x00028408
#define R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX        0x0002840C
#define R_028414_CB_BLEND_RED                        0x00028414
#define R_028418_CB_BLEND_GREEN                      0x00028418
#define R_02841C_CB_BLEND_BLUE                       0x0002841C
#define R_028420_CB_BLEND_ALPHA                      0x00028420
#define R_028438_SX_ALPHA_REF                        0x00028438
#define R_02843C_PA_CL_VPORT_XSCALE_0                0x0002843C
#define R_028440_PA_CL_VPORT_XOFFSET_0               0x00028440
#define R_028444_PA_CL_VPORT_YSCALE_0                0x00028444
#define R_028448_PA_CL_VPORT_YOFFSET_0               0x00028448
#define R_02844C_PA_CL_VPORT_ZSCALE_0                0x0002844C
#define R_028450_PA_CL_VPORT_ZOFFSET_0               0x00028450
#define R_0285BC_PA_CL_UCP0_X                        0x000285BC
#define R_0285C0_PA_CL_UCP0_Y                        0x000285C0
#define R_0285C4_PA_CL_UCP0_Z                        0x000285C4
#define R_0285C8_PA_CL_UCP0_W                        0x000285C8
#define R_0285CC_PA_CL_UCP1_X                        0x000285CC
#define R_0285D0_PA_CL_UCP1_Y                        0x000285D0
#define R_0285D4_PA_CL_UCP1_Z                        0x000285D4
#define R_0285D8_PA_CL_UCP1_W                        0x000285D8
#define R_0285DC_PA_CL_UCP2_X                        0x000285DC
#define R_0285E0_PA_CL_UCP2_Y                        0x000285E0
#define R_0285E4_PA_CL_UCP2_Z                        0x000285E4
#define R_0285E8_PA_CL_UCP2_W                        0x000285E8
#define R_0285EC_PA_CL_UCP3_X                        0x000285EC
#define R_0285F0_PA_CL_UCP3_Y                        0x000285F0
#define R_0285F4_PA_CL_UCP3_Z                        0x000285F4
#define R_0285F8_PA_CL_UCP3_W                        0x000285F8
#define R_0285FC_PA_CL_UCP4_X                        0x000285FC
#define R_028600_PA_CL_UCP4_Y                        0x00028600
#define R_028604_PA_CL_UCP4_Z                        0x00028604
#define R_028608_PA_CL_UCP4_W                        0x00028608
#define R_02860C_PA_CL_UCP5_X                        0x0002860C
#define R_028610_PA_CL_UCP5_Y                        0x00028610
#define R_028614_PA_CL_UCP5_Z                        0x00028614
#define R_028618_PA_CL_UCP5_W                        0x00028618
#define R_02861C_SPI_VS_OUT_ID_0                     0x0002861C
#define R_028620_SPI_VS_OUT_ID_1                     0x00028620
#define R_028624_SPI_VS_OUT_ID_2                     0x00028624
#define R_028628_SPI_VS_OUT_ID_3                     0x00028628
#define R_02862C_SPI_VS_OUT_ID_4                     0x0002862C
#define R_028630_SPI_VS_OUT_ID_5                     0x00028630
#define R_028634_SPI_VS_OUT_ID_6                     0x00028634
#define R_028638_SPI_VS_OUT_ID_7                     0x00028638
#define R_02863C_SPI_VS_OUT_ID_8                     0x0002863C
#define R_028640_SPI_VS_OUT_ID_9                     0x00028640
#define R_028648_SPI_PS_INPUT_CNTL_1                 0x00028648
#define R_02864C_SPI_PS_INPUT_CNTL_2                 0x0002864C
#define R_028650_SPI_PS_INPUT_CNTL_3                 0x00028650
#define R_028654_SPI_PS_INPUT_CNTL_4                 0x00028654
#define R_028658_SPI_PS_INPUT_CNTL_5                 0x00028658
#define R_02865C_SPI_PS_INPUT_CNTL_6                 0x0002865C
#define R_028660_SPI_PS_INPUT_CNTL_7                 0x00028660
#define R_028664_SPI_PS_INPUT_CNTL_8                 0x00028664
#define R_028668_SPI_PS_INPUT_CNTL_9                 0x00028668
#define R_02866C_SPI_PS_INPUT_CNTL_10                0x0002866C
#define R_028670_SPI_PS_INPUT_CNTL_11                0x00028670
#define R_028674_SPI_PS_INPUT_CNTL_12                0x00028674
#define R_028678_SPI_PS_INPUT_CNTL_13                0x00028678
#define R_02867C_SPI_PS_INPUT_CNTL_14                0x0002867C
#define R_028680_SPI_PS_INPUT_CNTL_15                0x00028680
#define R_028684_SPI_PS_INPUT_CNTL_16                0x00028684
#define R_028688_SPI_PS_INPUT_CNTL_17                0x00028688
#define R_02868C_SPI_PS_INPUT_CNTL_18                0x0002868C
#define R_028690_SPI_PS_INPUT_CNTL_19                0x00028690
#define R_028694_SPI_PS_INPUT_CNTL_20                0x00028694
#define R_028698_SPI_PS_INPUT_CNTL_21                0x00028698
#define R_02869C_SPI_PS_INPUT_CNTL_22                0x0002869C
#define R_0286A0_SPI_PS_INPUT_CNTL_23                0x000286A0
#define R_0286A4_SPI_PS_INPUT_CNTL_24                0x000286A4
#define R_0286A8_SPI_PS_INPUT_CNTL_25                0x000286A8
#define R_0286AC_SPI_PS_INPUT_CNTL_26                0x000286AC
#define R_0286B0_SPI_PS_INPUT_CNTL_27                0x000286B0
#define R_0286B4_SPI_PS_INPUT_CNTL_28                0x000286B4
#define R_0286B8_SPI_PS_INPUT_CNTL_29                0x000286B8
#define R_0286BC_SPI_PS_INPUT_CNTL_30                0x000286BC
#define R_0286C0_SPI_PS_INPUT_CNTL_31                0x000286C0
#define R_0286C8_SPI_THREAD_GROUPING                 0x000286C8
#define R_0286D8_SPI_INPUT_Z                         0x000286D8
#define   S_0286D8_PROVIDE_Z_TO_SPI(x)			(((unsigned)(x) & 0x1) << 0)
#define R_0286DC_SPI_FOG_CNTL                        0x000286DC
#define R_0286E4_SPI_PS_IN_CONTROL_2                 0x000286E4
#define R_0286E8_SPI_COMPUTE_INPUT_CNTL              0x000286E8
#define   S_0286E8_TID_IN_GROUP_ENA(x)                  (((unsigned)(x) & 0x1) << 0)
#define   S_0286E8_TGID_ENA(x)                          (((unsigned)(x) & 0x1) << 1)
#define   S_0286E8_DISABLE_INDEX_PACK(x)                (((unsigned)(x) & 0x1) << 2)
#define R_028720_GDS_ADDR_BASE                       0x00028720
#define R_028724_GDS_ADDR_SIZE                       0x00028724
#define R_028728_GDS_ORDERED_WAVE_PER_SE             0x00028728
#define R_02872C_GDS_APPEND_COUNT_0                  0x0002872C
#define R_028730_GDS_APPEND_COUNT_1                  0x00028730
#define R_028734_GDS_APPEND_COUNT_2                  0x00028734
#define R_028738_GDS_APPEND_COUNT_3                  0x00028738
#define R_02873C_GDS_APPEND_COUNT_4                  0x0002873C
#define R_028740_GDS_APPEND_COUNT_5                  0x00028740
#define R_028748_GDS_APPEND_COUNT_6                  0x00028744
#define R_028744_GDS_APPEND_COUNT_7                  0x00028748
#define R_028744_GDS_APPEND_COUNT_8                  0x0002874C
#define R_028744_GDS_APPEND_COUNT_9                  0x00028750
#define R_028744_GDS_APPEND_COUNT_10                 0x00028754
#define R_028744_GDS_APPEND_COUNT_11                 0x00028758

#define R_028784_CB_BLEND1_CONTROL                   0x00028784
#define R_028788_CB_BLEND2_CONTROL                   0x00028788
#define R_02878C_CB_BLEND3_CONTROL                   0x0002878C
#define R_028790_CB_BLEND4_CONTROL                   0x00028790
#define R_028794_CB_BLEND5_CONTROL                   0x00028794
#define R_028798_CB_BLEND6_CONTROL                   0x00028798
#define R_02879C_CB_BLEND7_CONTROL                   0x0002879C
#define R_028818_PA_CL_VTE_CNTL                      0x00028818
#define   S_028818_VPORT_X_SCALE_ENA(x)                (((unsigned)(x) & 0x1) << 0)
#define   G_028818_VPORT_X_SCALE_ENA(x)                (((x) >> 0 & 0x1)
#define   C_028818_VPORT_X_SCALE_ENA                   0xFFFFFFFE
#define   S_028818_VPORT_X_OFFSET_ENA(x)               (((unsigned)(x) & 0x1) << 1)
#define   G_028818_VPORT_X_OFFSET_ENA(x)               (((x) >> 1 & 0x1)
#define   C_028818_VPORT_X_OFFSET_ENA                  0xFFFFFFFD
#define   S_028818_VPORT_Y_SCALE_ENA(x)                (((unsigned)(x) & 0x1) << 2)
#define   G_028818_VPORT_Y_SCALE_ENA(x)                (((x) >> 2 & 0x1)
#define   C_028818_VPORT_Y_SCALE_ENA                   0xFFFFFFFB
#define   S_028818_VPORT_Y_OFFSET_ENA(x)               (((unsigned)(x) & 0x1) << 3)
#define   G_028818_VPORT_Y_OFFSET_ENA(x)               (((x) >> 3 & 0x1)
#define   C_028818_VPORT_Y_OFFSET_ENA                  0xFFFFFFF7
#define   S_028818_VPORT_Z_SCALE_ENA(x)                (((unsigned)(x) & 0x1) << 4)
#define   G_028818_VPORT_Z_SCALE_ENA(x)                (((x) >> 4 & 0x1)
#define   C_028818_VPORT_Z_SCALE_ENA                   0xFFFFFFEF
#define   S_028818_VPORT_Z_OFFSET_ENA(x)               (((unsigned)(x) & 0x1) << 5)
#define   G_028818_VPORT_Z_OFFSET_ENA(x)               (((x) >> 5 & 0x1)
#define   C_028818_VPORT_Z_OFFSET_ENA                  0xFFFFFFDF
#define   S_028818_VTX_XY_FMT(x)                       (((unsigned)(x) & 0x1) << 8)
#define   G_028818_VTX_XY_FMT(x)                       (((x) >> 8) & 0x1)
#define   C_028818_VTX_XY_FMT                          0xFFFFFEFF
#define   S_028818_VTX_Z_FMT(x)                        (((unsigned)(x) & 0x1) << 9)
#define   G_028818_VTX_Z_FMT(x)                        (((x) >> 9) & 0x1)
#define   C_028818_VTX_Z_FMT                           0xFFFFFDFF
#define   S_028818_VTX_W0_FMT(x)                       (((unsigned)(x) & 0x1) << 10)
#define   G_028818_VTX_W0_FMT(x)                       (((x) >> 10) & 0x1)
#define   C_028818_VTX_W0_FMT                          0xFFFFFBFF

#define R_028820_PA_CL_NANINF_CNTL                   0x00028820
#define R_028830_SQ_LSTMP_RING_ITEMSIZE              0x00028830
#define R_028838_SQ_DYN_GPR_RESOURCE_LIMIT_1         0x00028838
#define   S_028838_PS_GPRS(x)                          (((unsigned)(x) & 0x1F) << 0)
#define   S_028838_VS_GPRS(x)                          (((unsigned)(x) & 0x1F) << 5)
#define   S_028838_GS_GPRS(x)                          (((unsigned)(x) & 0x1F) << 10)
#define   S_028838_ES_GPRS(x)                          (((unsigned)(x) & 0x1F) << 15)
#define   S_028838_HS_GPRS(x)                          (((unsigned)(x) & 0x1F) << 20)
#define   S_028838_LS_GPRS(x)                          (((unsigned)(x) & 0x1F) << 25)
#define R_028840_SQ_PGM_START_PS                     0x00028840
#define R_02884C_SQ_PGM_EXPORTS_PS                   0x0002884C
#define   S_02884C_EXPORT_COLORS(x)                    (((unsigned)(x) & 0xF) << 1)
#define   G_02884C_EXPORT_COLORS(x)                    (((x) >> 1) & 0xF)
#define   C_02884C_EXPORT_COLORS                       0xFFFFFFE1
#define   S_02884C_EXPORT_Z(x)                         (((unsigned)(x) & 0x1) << 0)
#define   G_02884C_EXPORT_Z(x)                         (((x) >> 0) & 0x1)
#define   C_02884C_EXPORT_Z                            0xFFFFFFFE
#define R_02885C_SQ_PGM_START_VS                     0x0002885C
#define R_028874_SQ_PGM_START_GS                     0x00028874
#define R_02888C_SQ_PGM_START_ES                     0x0002888C
#define R_0288A4_SQ_PGM_START_FS                     0x000288A4
#define R_0288B8_SQ_PGM_START_HS                     0x000288B8
#define R_0288D0_SQ_PGM_START_LS                     0x000288D0
#define R_0288A8_SQ_PGM_RESOURCES_FS                 0x000288A8
#define R_0288E8_SQ_LDS_ALLOC                        0x000288E8
#define R_0288EC_SQ_LDS_ALLOC_PS                     0x000288EC
#define R_028900_SQ_ESGS_RING_ITEMSIZE               0x00028900
#define R_028904_SQ_GSVS_RING_ITEMSIZE               0x00028904
#define R_008C50_SQ_ESTMP_RING_BASE                  0x00008C50
#define R_028908_SQ_ESTMP_RING_ITEMSIZE              0x00028908
#define R_008C54_SQ_ESTMP_RING_SIZE                  0x00008C54
#define R_008C58_SQ_GSTMP_RING_BASE                  0x00008C58
#define R_02890C_SQ_GSTMP_RING_ITEMSIZE              0x0002890C
#define R_008C5C_SQ_GSTMP_RING_SIZE                  0x00008C5C
#define R_008C60_SQ_VSTMP_RING_BASE                  0x00008C60
#define R_028910_SQ_VSTMP_RING_ITEMSIZE              0x00028910
#define R_008C64_SQ_VSTMP_RING_SIZE                  0x00008C64
#define R_008C68_SQ_PSTMP_RING_BASE                  0x00008C68
#define R_028914_SQ_PSTMP_RING_ITEMSIZE              0x00028914
#define R_008C6C_SQ_PSTMP_RING_SIZE                  0x00008C6C
#define R_008E10_SQ_LSTMP_RING_BASE                  0x00008E10
#define R_028830_SQ_LSTMP_RING_ITEMSIZE              0x00028830
#define R_008E14_SQ_LSTMP_RING_SIZE                  0x00008E14
#define R_008E18_SQ_HSTMP_RING_BASE                  0x00008E18
#define R_028834_SQ_HSTMP_RING_ITEMSIZE              0x00028834
#define R_008E1C_SQ_HSTMP_RING_SIZE                  0x00008E1C
#define R_02891C_SQ_GS_VERT_ITEMSIZE                 0x0002891C
#define R_028920_SQ_GS_VERT_ITEMSIZE_1               0x00028920
#define R_028924_SQ_GS_VERT_ITEMSIZE_2               0x00028924
#define R_028928_SQ_GS_VERT_ITEMSIZE_3               0x00028928
#define R_02892C_SQ_GSVS_RING_OFFSET_1               0x0002892C
#define R_028930_SQ_GSVS_RING_OFFSET_2               0x00028930
#define R_028934_SQ_GSVS_RING_OFFSET_3               0x00028934
#define R_028940_ALU_CONST_CACHE_PS_0                0x00028940
#define R_028944_ALU_CONST_CACHE_PS_1                0x00028944
#define R_028980_ALU_CONST_CACHE_VS_0                0x00028980
#define R_028984_ALU_CONST_CACHE_VS_1                0x00028984
#define R_0289C0_ALU_CONST_CACHE_GS_0                0x000289C0
#define R_028F00_ALU_CONST_CACHE_HS_0                0x00028F00
#define R_028F40_ALU_CONST_CACHE_LS_0                0x00028F40
#define R_028A04_PA_SU_POINT_MINMAX                  0x00028A04
#define   S_028A04_MIN_SIZE(x)                         (((unsigned)(x) & 0xFFFF) << 0)
#define   G_028A04_MIN_SIZE(x)                         (((x) >> 0) & 0xFFFF)
#define   C_028A04_MIN_SIZE                            0xFFFF0000
#define   S_028A04_MAX_SIZE(x)                         (((unsigned)(x) & 0xFFFF) << 16)
#define   G_028A04_MAX_SIZE(x)                         (((x) >> 16) & 0xFFFF)
#define   C_028A04_MAX_SIZE                            0x0000FFFF
#define R_028A08_PA_SU_LINE_CNTL                     0x00028A08
#define   S_028A08_WIDTH(x)                            (((unsigned)(x) & 0xFFFF) << 0)
#define   G_028A08_WIDTH(x)                            (((x) >> 0) & 0xFFFF)
#define   C_028A08_WIDTH                               0xFFFF0000
#define R_028A10_VGT_OUTPUT_PATH_CNTL                0x00028A10
#define R_028A14_VGT_HOS_CNTL                        0x00028A14
#define R_028A18_VGT_HOS_MAX_TESS_LEVEL              0x00028A18
#define R_028A1C_VGT_HOS_MIN_TESS_LEVEL              0x00028A1C
#define R_028A20_VGT_HOS_REUSE_DEPTH                 0x00028A20
#define R_028A24_VGT_GROUP_PRIM_TYPE                 0x00028A24
#define R_028A28_VGT_GROUP_FIRST_DECR                0x00028A28
#define R_028A2C_VGT_GROUP_DECR                      0x00028A2C
#define R_028A30_VGT_GROUP_VECT_0_CNTL               0x00028A30
#define R_028A34_VGT_GROUP_VECT_1_CNTL               0x00028A34
#define R_028A38_VGT_GROUP_VECT_0_FMT_CNTL           0x00028A38
#define R_028A3C_VGT_GROUP_VECT_1_FMT_CNTL           0x00028A3C
#define R_028A48_PA_SC_MODE_CNTL_0                   0x00028A48
#define   S_028A48_MSAA_ENABLE(x)                      (((unsigned)(x) & 0x1) << 0)
#define   S_028A48_VPORT_SCISSOR_ENABLE(x)             (((unsigned)(x) & 0x1) << 1)
#define   S_028A48_LINE_STIPPLE_ENABLE(x)              (((unsigned)(x) & 0x1) << 2)
#define R_028A4C_PA_SC_MODE_CNTL_1                   0x00028A4C

#define R_028A54_GS_PER_ES                           0x00028A54
#define R_028A58_ES_PER_GS                           0x00028A58
#define R_028A5C_GS_PER_VS                           0x00028A5C

#define R_028A84_VGT_PRIMITIVEID_EN                  0x028A84
#define   S_028A84_PRIMITIVEID_EN(x)                   (((unsigned)(x) & 0x1) << 0)
#define   G_028A84_PRIMITIVEID_EN(x)                   (((x) >> 0) & 0x1)
#define   C_028A84_PRIMITIVEID_EN                      0xFFFFFFFE
#define R_028A94_VGT_MULTI_PRIM_IB_RESET_EN          0x00028A94
#define   S_028A94_RESET_EN(x)                         (((unsigned)(x) & 0x1) << 0)
#define   G_028A94_RESET_EN(x)                         (((x) >> 0) & 0x1)
#define   C_028A94_RESET_EN                            0xFFFFFFFE
#define R_028AB4_VGT_REUSE_OFF                       0x00028AB4
#define R_028AB8_VGT_VTX_CNT_EN                      0x00028AB8
#define R_028AC0_DB_SRESULTS_COMPARE_STATE0          0x00028AC0
#define R_028AC4_DB_SRESULTS_COMPARE_STATE1          0x00028AC4
#define R_028AC8_DB_PRELOAD_CONTROL                  0x00028AC8
#define   S_028AC8_MAX_X(x)                            (((unsigned)(x) & 0xff) << 16)
#define   S_028AC8_MAX_Y(x)                            (((unsigned)(x) & 0xff) << 24)
#define R_028AD0_VGT_STRMOUT_BUFFER_SIZE_0	     0x028AD0
#define R_028AD4_VGT_STRMOUT_VTX_STRIDE_0	     0x028AD4
#define R_028AD8_VGT_STRMOUT_BUFFER_BASE_0	     0x028AD8
#define R_028ADC_VGT_STRMOUT_BUFFER_OFFSET_0	     0x028ADC
#define R_028AE0_VGT_STRMOUT_BUFFER_SIZE_1	     0x028AE0
#define R_028AE4_VGT_STRMOUT_VTX_STRIDE_1	     0x028AE4
#define R_028AE8_VGT_STRMOUT_BUFFER_BASE_1	     0x028AE8
#define R_028AEC_VGT_STRMOUT_BUFFER_OFFSET_1	     0x028AEC
#define R_028AF0_VGT_STRMOUT_BUFFER_SIZE_2	     0x028AF0
#define R_028AF4_VGT_STRMOUT_VTX_STRIDE_2	     0x028AF4
#define R_028AF8_VGT_STRMOUT_BUFFER_BASE_2	     0x028AF8
#define R_028AFC_VGT_STRMOUT_BUFFER_OFFSET_2	     0x028AFC
#define R_028B00_VGT_STRMOUT_BUFFER_SIZE_3	     0x028B00
#define R_028B04_VGT_STRMOUT_VTX_STRIDE_3	     0x028B04
#define R_028B08_VGT_STRMOUT_BUFFER_BASE_3	     0x028B08
#define R_028B0C_VGT_STRMOUT_BUFFER_OFFSET_3	     0x028B0C
#define R_028B10_VGT_STRMOUT_BASE_OFFSET_0	     0x028B10
#define R_028B14_VGT_STRMOUT_BASE_OFFSET_1	     0x028B14
#define R_028B18_VGT_STRMOUT_BASE_OFFSET_2	     0x028B18
#define R_028B1C_VGT_STRMOUT_BASE_OFFSET_3	     0x028B1C
#define R_028B28_VGT_STRMOUT_DRAW_OPAQUE_OFFSET	     0x028B28
#define R_028B2C_VGT_STRMOUT_DRAW_OPAQUE_BUFFER_FILLED_SIZE 0x028B2C
#define R_028B30_VGT_STRMOUT_DRAW_OPAQUE_VERTEX_STRIDE 0x028B30
#define R_028B38_VGT_GS_MAX_VERT_OUT                 0x028B38
#define   S_028B38_MAX_VERT_OUT(x)                      (((unsigned)(x) & 0x7FF) << 0)
#define R_028B44_VGT_STRMOUT_BASE_OFFSET_HI_0	     0x028B44
#define R_028B48_VGT_STRMOUT_BASE_OFFSET_HI_1	     0x028B48
#define R_028B4C_VGT_STRMOUT_BASE_OFFSET_HI_2	     0x028B4C
#define R_028B50_VGT_STRMOUT_BASE_OFFSET_HI_3	     0x028B50
#define R_028B54_VGT_SHADER_STAGES_EN                0x00028B54
#define   S_028B54_LS_EN(x)                             (((unsigned)(x) & 0x3) << 0)
#define     V_028B54_LS_STAGE_OFF                    0x00
#define     V_028B54_LS_STAGE_ON                     0x01
#define     V_028B54_CS_STAGE_ON                     0x02
#define   S_028B54_HS_EN(x)                             (((unsigned)(x) & 0x1) << 2)
#define   S_028B54_ES_EN(x)                             (((unsigned)(x) & 0x3) << 3)
#define     V_028B54_ES_STAGE_OFF                    0x00
#define     V_028B54_ES_STAGE_DS                     0x01
#define     V_028B54_ES_STAGE_REAL                   0x02
#define   S_028B54_GS_EN(x)                             (((unsigned)(x) & 0x1) << 5)
#define   S_028B54_VS_EN(x)                             (((unsigned)(x) & 0x3) << 6)
#define     V_028B54_VS_STAGE_REAL                   0x00
#define     V_028B54_VS_STAGE_DS                     0x01
#define     V_028B54_VS_STAGE_COPY_SHADER            0x02
#define R_028B58_VGT_LS_HS_CONFIG		     0x00028B58
#define   S_028B58_NUM_PATCHES(x)                                     (((unsigned)(x) & 0xFF) << 0)
#define   G_028B58_NUM_PATCHES(x)                                     (((x) >> 0) & 0xFF)
#define   C_028B58_NUM_PATCHES                                        0xFFFFFF00
#define   S_028B58_HS_NUM_INPUT_CP(x)                                 (((unsigned)(x) & 0x3F) << 8)
#define   G_028B58_HS_NUM_INPUT_CP(x)                                 (((x) >> 8) & 0x3F)
#define   C_028B58_HS_NUM_INPUT_CP                                    0xFFFFC0FF
#define   S_028B58_HS_NUM_OUTPUT_CP(x)                                (((unsigned)(x) & 0x3F) << 14)
#define   G_028B58_HS_NUM_OUTPUT_CP(x)                                (((x) >> 14) & 0x3F)
#define   C_028B58_HS_NUM_OUTPUT_CP                                   0xFFF03FFF
#define R_028B5C_VGT_LS_SIZE                         0x00028B5C
#define   S_028B5C_SIZE(x)                                            (((unsigned)(x) & 0xFF) << 0)
#define   G_028B5C_SIZE(x)                                            (((x) >> 0) & 0xFF)
#define   C_028B5C_SIZE                                               0xFFFFFF00
#define   S_028B5C_PATCH_CP_SIZE(x)                                   (((unsigned)(x) & 0x1FFF) << 8)
#define   G_028B5C_PATCH_CP_SIZE(x)                                   (((x) >> 8) & 0x1FFF)
#define   C_028B5C_PATCH_CP_SIZE                                      0xFFE000FF
#define R_028B60_VGT_HS_SIZE                         0x00028B60
#define   S_028B60_SIZE(x)                                            (((unsigned)(x) & 0xFF) << 0)
#define   G_028B60_SIZE(x)                                            (((x) >> 0) & 0xFF)
#define   C_028B60_SIZE                                               0xFFFFFF00
#define   S_028B60_PATCH_CP_SIZE(x)                                   (((unsigned)(x) & 0x1FFF) << 8)
#define   G_028B60_PATCH_CP_SIZE(x)                                   (((x) >> 8) & 0x1FFF)
#define   C_028B60_PATCH_CP_SIZE                                      0xFFE000FF
#define R_028B64_VGT_LS_HS_ALLOC                     0x00028B64
#define   S_028B64_HS_TOTAL_OUTPUT(x)                                 (((unsigned)(x) & 0x1FFF) << 0)
#define   G_028B64_HS_TOTAL_OUTPUT(x)                                 (((x) >> 0) & 0x1FFF)
#define   C_028B64_HS_TOTAL_OUTPUT                                    0xFFFFE000
#define   S_028B64_LS_HS_TOTAL_OUTPUT(x)                              (((unsigned)(x) & 0x1FFF) << 13)
#define   G_028B64_LS_HS_TOTAL_OUTPUT(x)                              (((x) >> 13) & 0x1FFF)
#define   C_028B64_LS_HS_TOTAL_OUTPUT                                 0xFC001FFF
#define R_028B68_VGT_HS_PATCH_CONST                  0x00028B68
#define   S_028B68_SIZE(x)                                            (((unsigned)(x) & 0x1FFF) << 0)
#define   G_028B68_SIZE(x)                                            (((x) >> 0) & 0x1FFF)
#define   C_028B68_SIZE                                               0xFFFFE000
#define   S_028B68_STRIDE(x)                                          (((unsigned)(x) & 0x1FFF) << 13)
#define   G_028B68_STRIDE(x)                                          (((x) >> 13) & 0x1FFF)
#define   C_028B68_STRIDE                                             0xFC001FFF
#define R_028B70_DB_ALPHA_TO_MASK                    0x00028B70
#define   S_028B70_ALPHA_TO_MASK_ENABLE(x)		(((unsigned)(x) & 0x1) << 0)
#define   S_028B70_ALPHA_TO_MASK_OFFSET0(x)		(((unsigned)(x) & 0x3) << 8)
#define   S_028B70_ALPHA_TO_MASK_OFFSET1(x)		(((unsigned)(x) & 0x3) << 10)
#define   S_028B70_ALPHA_TO_MASK_OFFSET2(x)		(((unsigned)(x) & 0x3) << 12)
#define   S_028B70_ALPHA_TO_MASK_OFFSET3(x)		(((unsigned)(x) & 0x3) << 14)
#define   S_028B70_OFFSET_ROUND(x)			(((unsigned)(x) & 0x1) << 16)
#define R_028B78_PA_SU_POLY_OFFSET_DB_FMT_CNTL       0x00028B78
#define   S_028B78_POLY_OFFSET_NEG_NUM_DB_BITS(x)      (((unsigned)(x) & 0xFF) << 0)
#define   G_028B78_POLY_OFFSET_NEG_NUM_DB_BITS(x)      (((x) >> 0) & 0xFF)
#define   C_028B78_POLY_OFFSET_NEG_NUM_DB_BITS         0xFFFFFF00
#define   S_028B78_POLY_OFFSET_DB_IS_FLOAT_FMT(x)      (((unsigned)(x) & 0x1) << 8)
#define   G_028B78_POLY_OFFSET_DB_IS_FLOAT_FMT(x)      (((x) >> 8) & 0x1)
#define   C_028B78_POLY_OFFSET_DB_IS_FLOAT_FMT         0xFFFFFEFF
#define R_028B7C_PA_SU_POLY_OFFSET_CLAMP             0x00028B7C
#define R_028B80_PA_SU_POLY_OFFSET_FRONT_SCALE       0x00028B80
#define   S_028B80_SCALE(x)                            (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028B80_SCALE(x)                            (((x) >> 0) & 0xFFFFFFFF)
#define   C_028B80_SCALE                               0x00000000
#define R_028B84_PA_SU_POLY_OFFSET_FRONT_OFFSET      0x00028B84
#define   S_028B84_OFFSET(x)                           (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028B84_OFFSET(x)                           (((x) >> 0) & 0xFFFFFFFF)
#define   C_028B84_OFFSET                              0x00000000
#define R_028B88_PA_SU_POLY_OFFSET_BACK_SCALE        0x00028B88
#define   S_028B88_SCALE(x)                            (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028B88_SCALE(x)                            (((x) >> 0) & 0xFFFFFFFF)
#define   C_028B88_SCALE                               0x00000000
#define R_028B8C_PA_SU_POLY_OFFSET_BACK_OFFSET       0x00028B8C
#define   S_028B8C_OFFSET(x)                           (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028B8C_OFFSET(x)                           (((x) >> 0) & 0xFFFFFFFF)
#define   C_028B8C_OFFSET                              0x00000000
#define R_028B90_VGT_GS_INSTANCE_CNT                 0x00028B90
#define   S_028B90_ENABLE(x)                           (((unsigned)(x) & 0x1) << 0)
#define   S_028B90_CNT(x)                              (((unsigned)(x) & 0x7F) << 2)
#define R_028B98_VGT_STRMOUT_BUFFER_CONFIG           0x028B98
#define   S_028B98_STREAM_0_BUFFER_EN(x)		(((unsigned)(x) & 0x0F) << 0)
#define   S_028B98_STREAM_1_BUFFER_EN(x)		(((unsigned)(x) & 0x0F) << 4)
#define   S_028B98_STREAM_2_BUFFER_EN(x)		(((unsigned)(x) & 0x0F) << 8)
#define   S_028B98_STREAM_3_BUFFER_EN(x)		(((unsigned)(x) & 0x0F) << 12)
#define R_028B9C_CB_IMMED0_BASE                      0x00028B9C
#define R_028BA0_CB_IMMED1_BASE                      0x00028BA0
#define R_028BA4_CB_IMMED2_BASE                      0x00028BA4
#define R_028BA4_CB_IMMED3_BASE                      0x00028BA8
#define R_028BA4_CB_IMMED4_BASE                      0x00028BAC
#define R_028BA4_CB_IMMED5_BASE                      0x00028BB0
#define R_028BA4_CB_IMMED6_BASE                      0x00028BB4
#define R_028BA4_CB_IMMED7_BASE                      0x00028BB8
#define R_028BA4_CB_IMMED8_BASE                      0x00028BBC
#define R_028BA4_CB_IMMED9_BASE                      0x00028BC0
#define R_028BA4_CB_IMMED10_BASE                     0x00028BC4
#define R_028BA4_CB_IMMED11_BASE                     0x00028BC8
#define R_028C00_PA_SC_LINE_CNTL                     0x00028C00
#define   S_028C00_EXPAND_LINE_WIDTH(x)                (((unsigned)(x) & 0x1) << 9)
#define   G_028C00_EXPAND_LINE_WIDTH(x)                (((x) >> 9) & 0x1)
#define   C_028C00_EXPAND_LINE_WIDTH                   0xFFFFFDFF
#define   S_028C00_LAST_PIXEL(x)                       (((unsigned)(x) & 0x1) << 10)
#define   G_028C00_LAST_PIXEL(x)                       (((x) >> 10) & 0x1)
#define   C_028C00_LAST_PIXEL                          0xFFFFFBFF
#define R_028C04_PA_SC_AA_CONFIG                     0x00028C04
#define   S_028C04_MSAA_NUM_SAMPLES(x)                  (((unsigned)(x) & 0x3) << 0)
#define   S_028C04_AA_MASK_CENTROID_DTMN(x)		(((unsigned)(x) & 0x1) << 4)
#define   S_028C04_MAX_SAMPLE_DIST(x)			(((unsigned)(x) & 0xf) << 13)
#define R_028C08_PA_SU_VTX_CNTL                      0x00028C08
#define   S_028C08_PIX_CENTER_HALF(x)                  (((unsigned)(x) & 0x1) << 0)
#define   G_028C08_PIX_CENTER_HALF(x)                  (((x) >> 0) & 0x1)
#define   C_028C08_PIX_CENTER_HALF                     0xFFFFFFFE
#define   S_028C08_QUANT_MODE(x)                       (((unsigned)(x) & 0x7) << 3)
#define   G_028C08_QUANT_MODE(x)                       (((x) >> 3) & 0x7)
#define   C_028C08_QUANT_MODE                          0xFFFFFFC7
#define     V_028C08_X_1_16TH                          0x00
#define     V_028C08_X_1_8TH                           0x01
#define     V_028C08_X_1_4TH                           0x02
#define     V_028C08_X_1_2                             0x03
#define     V_028C08_X_1                               0x04
#define     V_028C08_X_1_256TH                         0x05
#define     V_028C08_X_1_1024TH                        0x06
#define     V_028C08_X_1_4096TH                        0x07
#define R_028C0C_PA_CL_GB_VERT_CLIP_ADJ              0x00028C0C
#define R_028C10_PA_CL_GB_VERT_DISC_ADJ              0x00028C10
#define R_028C14_PA_CL_GB_HORZ_CLIP_ADJ              0x00028C14
#define R_028C18_PA_CL_GB_HORZ_DISC_ADJ              0x00028C18
#define R_028C1C_PA_SC_AA_SAMPLE_LOCS_0              0x00028C1C
#define R_028C20_PA_SC_AA_SAMPLE_LOCS_1              0x00028C20
#define R_028C24_PA_SC_AA_SAMPLE_LOCS_2              0x00028C24
#define R_028C28_PA_SC_AA_SAMPLE_LOCS_3              0x00028C28
#define R_028C2C_PA_SC_AA_SAMPLE_LOCS_4              0x00028C2C
#define R_028C30_PA_SC_AA_SAMPLE_LOCS_5              0x00028C30
#define R_028C34_PA_SC_AA_SAMPLE_LOCS_6              0x00028C34
#define R_028C38_PA_SC_AA_SAMPLE_LOCS_7              0x00028C38
#define R_028C3C_PA_SC_AA_MASK                       0x00028C3C
#define R_028C60_CB_COLOR0_BASE                      0x00028C60
#define R_028C6C_CB_COLOR0_VIEW                      0x00028C6C
#define   S_028C6C_SLICE_START(x)                      (((unsigned)(x) & 0x7FF) << 0)
#define   G_028C6C_SLICE_START(x)                      (((x) >> 0) & 0x7FF)
#define   C_028C6C_SLICE_START                         0xFFFFF800
#define   S_028C6C_SLICE_MAX(x)                        (((unsigned)(x) & 0x7FF) << 13)
#define   G_028C6C_SLICE_MAX(x)                        (((x) >> 13) & 0x7FF)
#define   C_028C6C_SLICE_MAX                           0xFF001FFF
#define R_028C7C_CB_COLOR0_CMASK                         0x028C7C
#define R_028C80_CB_COLOR0_CMASK_SLICE                   0x028C80
#define   S_028C80_TILE_MAX(x)				(((unsigned)(x) & 0x3FFF) << 0)
#define R_028C84_CB_COLOR0_FMASK                         0x028C84
#define R_028C88_CB_COLOR0_FMASK_SLICE                   0x028C88
#define   S_028C88_TILE_MAX(x)				(((unsigned)(x) & 0x3FFFFF) << 0)
#define R_028C8C_CB_COLOR0_CLEAR_WORD0                   0x028C8C
#define R_028C90_CB_COLOR0_CLEAR_WORD1                   0x028C90
#define R_028C94_CB_COLOR0_CLEAR_WORD2                   0x028C94
#define R_028C98_CB_COLOR0_CLEAR_WORD3                   0x028C98
#define R_028C9C_CB_COLOR1_BASE                      0x00028C9C
#define R_028CA0_CB_COLOR1_PITCH                     0x00028CA0
#define R_028CA4_CB_COLOR1_SLICE                     0x00028CA4
#define R_028CA8_CB_COLOR1_VIEW                      0x00028CA8
#define R_028CAC_CB_COLOR1_INFO                      0x00028CAC
#define R_028CB0_CB_COLOR1_ATTRIB                    0x00028CB0
#define R_028CB4_CB_COLOR1_DIM                       0x00028CB4
#define R_028CB8_CB_COLOR1_CMASK                         0x028CB8
#define R_028CBC_CB_COLOR1_CMASK_SLICE                   0x028CBC
#define R_028CC0_CB_COLOR1_FMASK                         0x028CC0
#define R_028CC4_CB_COLOR1_FMASK_SLICE                   0x028CC4
#define R_028CC8_CB_COLOR1_CLEAR_WORD0                   0x028CC8
#define R_028CCC_CB_COLOR1_CLEAR_WORD1                   0x028CCC
#define R_028CD0_CB_COLOR1_CLEAR_WORD2                   0x028CD0
#define R_028CD4_CB_COLOR1_CLEAR_WORD3                   0x028CD4
#define R_028CD8_CB_COLOR2_BASE                      0x00028CD8
#define R_028CDC_CB_COLOR2_PITCH                     0x00028CDC
#define R_028CE0_CB_COLOR2_SLICE                     0x00028CE0
#define R_028CE4_CB_COLOR2_VIEW                      0x00028CE4
#define R_028CE8_CB_COLOR2_INFO                      0x00028CE8
#define R_028CEC_CB_COLOR2_ATTRIB                    0x00028CEC
#define R_028CF0_CB_COLOR2_DIM                       0x00028CF0
#define R_028CF4_CB_COLOR2_CMASK                         0x028CF4
#define R_028CF8_CB_COLOR2_CMASK_SLICE                   0x028CF8
#define R_028CFC_CB_COLOR2_FMASK                         0x028CFC
#define R_028D00_CB_COLOR2_FMASK_SLICE                   0x028D00
#define R_028D04_CB_COLOR2_CLEAR_WORD0                   0x028D04
#define R_028D08_CB_COLOR2_CLEAR_WORD1                   0x028D08
#define R_028D0C_CB_COLOR2_CLEAR_WORD2                   0x028D0C
#define R_028D10_CB_COLOR2_CLEAR_WORD3                   0x028D10
#define R_028D14_CB_COLOR3_BASE                      0x00028D14
#define R_028D18_CB_COLOR3_PITCH                     0x00028D18
#define R_028D1C_CB_COLOR3_SLICE                     0x00028D1C
#define R_028D20_CB_COLOR3_VIEW                      0x00028D20
#define R_028D24_CB_COLOR3_INFO                      0x00028D24
#define R_028D28_CB_COLOR3_ATTRIB                    0x00028D28
#define R_028D2C_CB_COLOR3_DIM                       0x00028D2C
#define R_028D30_CB_COLOR3_CMASK                         0x028D30
#define R_028D34_CB_COLOR3_CMASK_SLICE                   0x028D34
#define R_028D38_CB_COLOR3_FMASK                         0x028D38
#define R_028D3C_CB_COLOR3_FMASK_SLICE                   0x028D3C
#define R_028D40_CB_COLOR3_CLEAR_WORD0                   0x028D40
#define R_028D44_CB_COLOR3_CLEAR_WORD1                   0x028D44
#define R_028D48_CB_COLOR3_CLEAR_WORD2                   0x028D48
#define R_028D4C_CB_COLOR3_CLEAR_WORD3                   0x028D4C
#define R_028D50_CB_COLOR4_BASE                      0x00028D50
#define R_028D54_CB_COLOR4_PITCH                     0x00028D54
#define R_028D58_CB_COLOR4_SLICE                     0x00028D58
#define R_028D5C_CB_COLOR4_VIEW                      0x00028D5C
#define R_028D60_CB_COLOR4_INFO                      0x00028D60
#define R_028D64_CB_COLOR4_ATTRIB                    0x00028D64
#define R_028D68_CB_COLOR4_DIM                       0x00028D68
#define R_028D6C_CB_COLOR4_CMASK                         0x028D6C
#define R_028D70_CB_COLOR4_CMASK_SLICE                   0x028D70
#define R_028D74_CB_COLOR4_FMASK                         0x028D74
#define R_028D78_CB_COLOR4_FMASK_SLICE                   0x028D78
#define R_028D7C_CB_COLOR4_CLEAR_WORD0                   0x028D7C
#define R_028D80_CB_COLOR4_CLEAR_WORD1                   0x028D80
#define R_028D84_CB_COLOR4_CLEAR_WORD2                   0x028D84
#define R_028D88_CB_COLOR4_CLEAR_WORD3                   0x028D88
#define R_028D8C_CB_COLOR5_BASE                      0x00028D8C
#define R_028D90_CB_COLOR5_PITCH                     0x00028D90
#define R_028D94_CB_COLOR5_SLICE                     0x00028D94
#define R_028D98_CB_COLOR5_VIEW                      0x00028D98
#define R_028D9C_CB_COLOR5_INFO                      0x00028D9C
#define R_028DA0_CB_COLOR5_ATTRIB                    0x00028DA0
#define R_028DA4_CB_COLOR5_DIM                       0x00028DA4
#define R_028DA8_CB_COLOR5_CMASK                         0x028DA8
#define R_028DAC_CB_COLOR5_CMASK_SLICE                   0x028DAC
#define R_028DB0_CB_COLOR5_FMASK                         0x028DB0
#define R_028DB4_CB_COLOR5_FMASK_SLICE                   0x028DB4
#define R_028DB8_CB_COLOR5_CLEAR_WORD0                   0x028DB8
#define R_028DBC_CB_COLOR5_CLEAR_WORD1                   0x028DBC
#define R_028DC0_CB_COLOR5_CLEAR_WORD2                   0x028DC0
#define R_028DC4_CB_COLOR5_CLEAR_WORD3                   0x028DC4
#define R_028DC8_CB_COLOR6_BASE                      0x00028DC8
#define R_028DCC_CB_COLOR6_PITCH                     0x00028DCC
#define R_028DD0_CB_COLOR6_SLICE                     0x00028DD0
#define R_028DD4_CB_COLOR6_VIEW                      0x00028DD4
#define R_028DD8_CB_COLOR6_INFO                      0x00028DD8
#define R_028DDC_CB_COLOR6_ATTRIB                    0x00028DDC
#define R_028DE0_CB_COLOR6_DIM                       0x00028DE0
#define R_028DE4_CB_COLOR6_CMASK                         0x028DE4
#define R_028DE8_CB_COLOR6_CMASK_SLICE                   0x028DE8
#define R_028DEC_CB_COLOR6_FMASK                         0x028DEC
#define R_028DF0_CB_COLOR6_FMASK_SLICE                   0x028DF0
#define R_028DF4_CB_COLOR6_CLEAR_WORD0                   0x028DF4
#define R_028DF8_CB_COLOR6_CLEAR_WORD1                   0x028DF8
#define R_028DFC_CB_COLOR6_CLEAR_WORD2                   0x028DFC
#define R_028E00_CB_COLOR6_CLEAR_WORD3                   0x028E00
#define R_028E04_CB_COLOR7_BASE                      0x00028E04
#define R_028E08_CB_COLOR7_PITCH                     0x00028E08
#define R_028E0C_CB_COLOR7_SLICE                     0x00028E0C
#define R_028E10_CB_COLOR7_VIEW                      0x00028E10
#define R_028E14_CB_COLOR7_INFO                      0x00028E14
#define R_028E18_CB_COLOR7_ATTRIB                    0x00028E18
#define R_028E1C_CB_COLOR7_DIM                       0x00028E1C
#define R_028E20_CB_COLOR7_CMASK                         0x028E20
#define R_028E24_CB_COLOR7_CMASK_SLICE                   0x028E24
#define R_028E28_CB_COLOR7_FMASK                         0x028E28
#define R_028E2C_CB_COLOR7_FMASK_SLICE                   0x028E2C
#define R_028E30_CB_COLOR7_CLEAR_WORD0                   0x028E30
#define R_028E34_CB_COLOR7_CLEAR_WORD1                   0x028E34
#define R_028E38_CB_COLOR7_CLEAR_WORD2                   0x028E38
#define R_028E3C_CB_COLOR7_CLEAR_WORD3                   0x028E3C
#define R_028E40_CB_COLOR8_BASE                      0x00028E40
#define R_028E44_CB_COLOR8_PITCH                     0x00028E44
#define R_028E48_CB_COLOR8_SLICE                     0x00028E48
#define R_028E4C_CB_COLOR8_VIEW                      0x00028E4C
#define R_028E50_CB_COLOR8_INFO                      0x00028E50
#define R_028E54_CB_COLOR8_ATTRIB                    0x00028E54
#define R_028E58_CB_COLOR8_DIM                       0x00028E58
#define R_028E5C_CB_COLOR9_BASE                      0x00028E5C
#define R_028E60_CB_COLOR9_PITCH                     0x00028E60
#define R_028E64_CB_COLOR9_SLICE                     0x00028E64
#define R_028E68_CB_COLOR9_VIEW                      0x00028E68
#define R_028E6C_CB_COLOR9_INFO                      0x00028E6C
#define R_028E70_CB_COLOR9_ATTRIB                    0x00028E70
#define R_028E74_CB_COLOR9_DIM                       0x00028E74
#define R_028E78_CB_COLOR10_BASE                     0x00028E78
#define R_028E7C_CB_COLOR10_PITCH                    0x00028E7C
#define R_028E80_CB_COLOR10_SLICE                    0x00028E80
#define R_028E84_CB_COLOR10_VIEW                     0x00028E84
#define R_028E88_CB_COLOR10_INFO                     0x00028E88
#define R_028E8C_CB_COLOR10_ATTRIB                   0x00028E8C
#define R_028E90_CB_COLOR10_DIM                      0x00028E90
#define R_028E94_CB_COLOR11_BASE                     0x00028E94
#define R_028E98_CB_COLOR11_PITCH                    0x00028E98
#define R_028E9C_CB_COLOR11_SLICE                    0x00028E9C
#define R_028EA0_CB_COLOR11_VIEW                     0x00028EA0
#define R_028EA4_CB_COLOR11_INFO                     0x00028EA4
#define R_028EA8_CB_COLOR11_ATTRIB                   0x00028EA8
#define R_028EAC_CB_COLOR11_DIM                      0x00028EAC
#define R_030000_RESOURCE0_WORD0                     0x00030000
#define R_030004_RESOURCE0_WORD1                     0x00030004
#define R_030008_RESOURCE0_WORD2                     0x00030008
#define R_03000C_RESOURCE0_WORD3                     0x0003000C
#define R_030010_RESOURCE0_WORD4                     0x00030010
#define R_030014_RESOURCE0_WORD5                     0x00030014
#define R_030018_RESOURCE0_WORD6                     0x00030018
#define R_03001C_RESOURCE0_WORD7                     0x0003001C
#define R_0085F0_CP_COHER_CNTL                       0x0085F0
#define   S_0085F0_DEST_BASE_0_ENA(x)                  (((unsigned)(x) & 0x1) << 0)
#define   G_0085F0_DEST_BASE_0_ENA(x)                  (((x) >> 0) & 0x1)
#define   C_0085F0_DEST_BASE_0_ENA                     0xFFFFFFFE
#define   S_0085F0_DEST_BASE_1_ENA(x)                  (((unsigned)(x) & 0x1) << 1)
#define   G_0085F0_DEST_BASE_1_ENA(x)                  (((x) >> 1) & 0x1)
#define   C_0085F0_DEST_BASE_1_ENA                     0xFFFFFFFD
#define   S_0085F0_SO0_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 2)
#define   G_0085F0_SO0_DEST_BASE_ENA(x)                (((x) >> 2) & 0x1)
#define   C_0085F0_SO0_DEST_BASE_ENA                   0xFFFFFFFB
#define   S_0085F0_SO1_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 3)
#define   G_0085F0_SO1_DEST_BASE_ENA(x)                (((x) >> 3) & 0x1)
#define   C_0085F0_SO1_DEST_BASE_ENA                   0xFFFFFFF7
#define   S_0085F0_SO2_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 4)
#define   G_0085F0_SO2_DEST_BASE_ENA(x)                (((x) >> 4) & 0x1)
#define   C_0085F0_SO2_DEST_BASE_ENA                   0xFFFFFFEF
#define   S_0085F0_SO3_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 5)
#define   G_0085F0_SO3_DEST_BASE_ENA(x)                (((x) >> 5) & 0x1)
#define   C_0085F0_SO3_DEST_BASE_ENA                   0xFFFFFFDF
#define   S_0085F0_CB0_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 6)
#define   G_0085F0_CB0_DEST_BASE_ENA(x)                (((x) >> 6) & 0x1)
#define   C_0085F0_CB0_DEST_BASE_ENA                   0xFFFFFFBF
#define   S_0085F0_CB1_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 7)
#define   G_0085F0_CB1_DEST_BASE_ENA(x)                (((x) >> 7) & 0x1)
#define   C_0085F0_CB1_DEST_BASE_ENA                   0xFFFFFF7F
#define   S_0085F0_CB2_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 8)
#define   G_0085F0_CB2_DEST_BASE_ENA(x)                (((x) >> 8) & 0x1)
#define   C_0085F0_CB2_DEST_BASE_ENA                   0xFFFFFEFF
#define   S_0085F0_CB3_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 9)
#define   G_0085F0_CB3_DEST_BASE_ENA(x)                (((x) >> 9) & 0x1)
#define   C_0085F0_CB3_DEST_BASE_ENA                   0xFFFFFDFF
#define   S_0085F0_CB4_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 10)
#define   G_0085F0_CB4_DEST_BASE_ENA(x)                (((x) >> 10) & 0x1)
#define   C_0085F0_CB4_DEST_BASE_ENA                   0xFFFFFBFF
#define   S_0085F0_CB5_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 11)
#define   G_0085F0_CB5_DEST_BASE_ENA(x)                (((x) >> 11) & 0x1)
#define   C_0085F0_CB5_DEST_BASE_ENA                   0xFFFFF7FF
#define   S_0085F0_CB6_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 12)
#define   G_0085F0_CB6_DEST_BASE_ENA(x)                (((x) >> 12) & 0x1)
#define   C_0085F0_CB6_DEST_BASE_ENA                   0xFFFFEFFF
#define   S_0085F0_CB7_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 13)
#define   G_0085F0_CB7_DEST_BASE_ENA(x)                (((x) >> 13) & 0x1)
#define   C_0085F0_CB7_DEST_BASE_ENA                   0xFFFFDFFF
#define   S_0085F0_DB_DEST_BASE_ENA(x)                 (((unsigned)(x) & 0x1) << 14)
#define   G_0085F0_DB_DEST_BASE_ENA(x)                 (((x) >> 14) & 0x1)
#define   C_0085F0_DB_DEST_BASE_ENA                    0xFFFFBFFF
#define   S_0085F0_CB8_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 15)
#define   G_0085F0_CB8_DEST_BASE_ENA(x)                (((x) >> 15) & 0x1)
#define   S_0085F0_CB9_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 16)
#define   G_0085F0_CB9_DEST_BASE_ENA(x)                (((x) >> 16) & 0x1)
#define   S_0085F0_CB10_DEST_BASE_ENA(x)               (((unsigned)(x) & 0x1) << 17)
#define   G_0085F0_CB10_DEST_BASE_ENA(x)               (((x) >> 17) & 0x1)
#define   S_0085F0_CB11_DEST_BASE_ENA(x)               (((unsigned)(x) & 0x1) << 18)
#define   G_0085F0_CB11_DEST_BASE_ENA(x)               (((x) >> 18) & 0x1)
#define   S_0085F0_TC_ACTION_ENA(x)                    (((unsigned)(x) & 0x1) << 23)
#define   G_0085F0_TC_ACTION_ENA(x)                    (((x) >> 23) & 0x1)
#define   C_0085F0_TC_ACTION_ENA                       0xFF7FFFFF
#define   S_0085F0_VC_ACTION_ENA(x)                    (((unsigned)(x) & 0x1) << 24)
#define   G_0085F0_VC_ACTION_ENA(x)                    (((x) >> 24) & 0x1)
#define   C_0085F0_VC_ACTION_ENA                       0xFEFFFFFF
#define   S_0085F0_CB_ACTION_ENA(x)                    (((unsigned)(x) & 0x1) << 25)
#define   G_0085F0_CB_ACTION_ENA(x)                    (((x) >> 25) & 0x1)
#define   C_0085F0_CB_ACTION_ENA                       0xFDFFFFFF
#define   S_0085F0_DB_ACTION_ENA(x)                    (((unsigned)(x) & 0x1) << 26)
#define   G_0085F0_DB_ACTION_ENA(x)                    (((x) >> 26) & 0x1)
#define   C_0085F0_DB_ACTION_ENA                       0xFBFFFFFF
#define   S_0085F0_SH_ACTION_ENA(x)                    (((unsigned)(x) & 0x1) << 27)
#define   G_0085F0_SH_ACTION_ENA(x)                    (((x) >> 27) & 0x1)
#define   C_0085F0_SH_ACTION_ENA                       0xF7FFFFFF
#define   S_0085F0_SMX_ACTION_ENA(x)                   (((unsigned)(x) & 0x1) << 28)
#define   G_0085F0_SMX_ACTION_ENA(x)                   (((x) >> 28) & 0x1)
#define   C_0085F0_SMX_ACTION_ENA                      0xEFFFFFFF
#define   S_0085F0_CR0_ACTION_ENA(x)                   (((unsigned)(x) & 0x1) << 29)
#define   G_0085F0_CR0_ACTION_ENA(x)                   (((x) >> 29) & 0x1)
#define   C_0085F0_CR0_ACTION_ENA                      0xDFFFFFFF
#define   S_0085F0_CR1_ACTION_ENA(x)                   (((unsigned)(x) & 0x1) << 30)
#define   G_0085F0_CR1_ACTION_ENA(x)                   (((x) >> 30) & 0x1)
#define   C_0085F0_CR1_ACTION_ENA                      0xBFFFFFFF
#define   S_0085F0_CR2_ACTION_ENA(x)                   (((unsigned)(x) & 0x1) << 31)
#define   G_0085F0_CR2_ACTION_ENA(x)                   (((x) >> 31) & 0x1)
#define   C_0085F0_CR2_ACTION_ENA                      0x7FFFFFFF
#define R_0085F4_CP_COHER_SIZE                       0x0085F4
#define R_0085F8_CP_COHER_BASE                       0x0085F8
#define R_008970_VGT_NUM_INDICES                     0x008970

#define R_03CFF0_SQ_VTX_BASE_VTX_LOC                    0x03CFF0
#define R_03CFF4_SQ_VTX_START_INST_LOC                  0x03CFF4

#define R_03A200_SQ_LOOP_CONST_0                     0x3A200

#define ENDIAN_NONE	0
#define ENDIAN_8IN16	1
#define ENDIAN_8IN32	2
#define ENDIAN_8IN64	3

#define CM_R_0286F8_SPI_GPR_MGMT                     0x286f8
#define CM_R_0286FC_SPI_LDS_MGMT                     0x286fc
#define   S_0286FC_NUM_PS_LDS(x)                     ((x) & 0xff)
#define   S_0286FC_NUM_LS_LDS(x)                     ((x) & 0xff) << 8
#define CM_R_028700_SPI_STACK_MGMT                   0x28700
#define CM_R_028704_SPI_WAVE_MGMT_1                  0x28704
#define CM_R_028708_SPI_WAVE_MGMT_2                  0x28708

#define CM_R_028804_DB_EQAA                          0x00028804
#define   S_028804_MAX_ANCHOR_SAMPLES(x)                              (((unsigned)(x) & 0x07) << 0)
#define   G_028804_MAX_ANCHOR_SAMPLES(x)                              (((x) >> 0) & 0x07)
#define   C_028804_MAX_ANCHOR_SAMPLES                                 0xFFFFFFF8
#define   S_028804_PS_ITER_SAMPLES(x)                                 (((unsigned)(x) & 0x07) << 4)
#define   G_028804_PS_ITER_SAMPLES(x)                                 (((x) >> 4) & 0x07)
#define   C_028804_PS_ITER_SAMPLES                                    0xFFFFFF8F
#define   S_028804_MASK_EXPORT_NUM_SAMPLES(x)                         (((unsigned)(x) & 0x07) << 8)
#define   G_028804_MASK_EXPORT_NUM_SAMPLES(x)                         (((x) >> 8) & 0x07)
#define   C_028804_MASK_EXPORT_NUM_SAMPLES                            0xFFFFF8FF
#define   S_028804_ALPHA_TO_MASK_NUM_SAMPLES(x)                       (((unsigned)(x) & 0x07) << 12)
#define   G_028804_ALPHA_TO_MASK_NUM_SAMPLES(x)                       (((x) >> 12) & 0x07)
#define   C_028804_ALPHA_TO_MASK_NUM_SAMPLES                          0xFFFF8FFF
#define   S_028804_HIGH_QUALITY_INTERSECTIONS(x)                      (((unsigned)(x) & 0x1) << 16)
#define   G_028804_HIGH_QUALITY_INTERSECTIONS(x)                      (((x) >> 16) & 0x1)
#define   C_028804_HIGH_QUALITY_INTERSECTIONS                         0xFFFEFFFF
#define   S_028804_INCOHERENT_EQAA_READS(x)                           (((unsigned)(x) & 0x1) << 17)
#define   G_028804_INCOHERENT_EQAA_READS(x)                           (((x) >> 17) & 0x1)
#define   C_028804_INCOHERENT_EQAA_READS                              0xFFFDFFFF
#define   S_028804_INTERPOLATE_COMP_Z(x)                              (((unsigned)(x) & 0x1) << 18)
#define   G_028804_INTERPOLATE_COMP_Z(x)                              (((x) >> 18) & 0x1)
#define   C_028804_INTERPOLATE_COMP_Z                                 0xFFFBFFFF
#define   S_028804_INTERPOLATE_SRC_Z(x)                               (((unsigned)(x) & 0x1) << 19)
#define   G_028804_INTERPOLATE_SRC_Z(x)                               (((x) >> 19) & 0x1)
#define   C_028804_INTERPOLATE_SRC_Z                                  0xFFF7FFFF
#define   S_028804_STATIC_ANCHOR_ASSOCIATIONS(x)                      (((unsigned)(x) & 0x1) << 20)
#define   G_028804_STATIC_ANCHOR_ASSOCIATIONS(x)                      (((x) >> 20) & 0x1)
#define   C_028804_STATIC_ANCHOR_ASSOCIATIONS                         0xFFEFFFFF
#define   S_028804_ALPHA_TO_MASK_EQAA_DISABLE(x)                      (((unsigned)(x) & 0x1) << 21)
#define   G_028804_ALPHA_TO_MASK_EQAA_DISABLE(x)                      (((x) >> 21) & 0x1)
#define   C_028804_ALPHA_TO_MASK_EQAA_DISABLE                         0xFFDFFFFF

#define CM_R_028BD4_PA_SC_CENTROID_PRIORITY_0        0x00028BD4
#define CM_R_028BD8_PA_SC_CENTROID_PRIORITY_1        0x00028BD8
#define CM_R_028BDC_PA_SC_LINE_CNTL                  0x28bdc
#define CM_R_028BE0_PA_SC_AA_CONFIG                  0x28be0
#define   S_028BE0_MSAA_NUM_SAMPLES(x)                                (((unsigned)(x) & 0x07) << 0)
#define   G_028BE0_MSAA_NUM_SAMPLES(x)                                (((x) >> 0) & 0x07)
#define   C_028BE0_MSAA_NUM_SAMPLES                                   0xFFFFFFF8
#define   S_028BE0_AA_MASK_CENTROID_DTMN(x)                           (((unsigned)(x) & 0x1) << 4)
#define   G_028BE0_AA_MASK_CENTROID_DTMN(x)                           (((x) >> 4) & 0x1)
#define   C_028BE0_AA_MASK_CENTROID_DTMN                              0xFFFFFFEF
#define   S_028BE0_MAX_SAMPLE_DIST(x)                                 (((unsigned)(x) & 0x0F) << 13)
#define   G_028BE0_MAX_SAMPLE_DIST(x)                                 (((x) >> 13) & 0x0F)
#define   C_028BE0_MAX_SAMPLE_DIST                                    0xFFFE1FFF
#define   S_028BE0_MSAA_EXPOSED_SAMPLES(x)                            (((unsigned)(x) & 0x07) << 20)
#define   G_028BE0_MSAA_EXPOSED_SAMPLES(x)                            (((x) >> 20) & 0x07)
#define   C_028BE0_MSAA_EXPOSED_SAMPLES                               0xFF8FFFFF
#define   S_028BE0_DETAIL_TO_EXPOSED_MODE(x)                          (((unsigned)(x) & 0x03) << 24)
#define   G_028BE0_DETAIL_TO_EXPOSED_MODE(x)                          (((x) >> 24) & 0x03)
#define   C_028BE0_DETAIL_TO_EXPOSED_MODE                             0xFCFFFFFF
#define CM_R_028BE4_PA_SU_VTX_CNTL                   0x28be4
#define CM_R_028BE8_PA_CL_GB_VERT_CLIP_ADJ           0x28be8
#define CM_R_028BEC_PA_CL_GB_VERT_DISC_ADJ           0x28bec
#define CM_R_028BF0_PA_CL_GB_HORZ_CLIP_ADJ           0x28bf0
#define CM_R_028BF4_PA_CL_GB_HORZ_DISC_ADJ           0x28bf4

#define CM_R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0 0x28bf8
#define CM_R_028BFC_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_1 0x28bfc
#define CM_R_028C00_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_2 0x28c00
#define CM_R_028C04_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_3 0x28c04

#define CM_R_028C08_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0 0x28c08
#define CM_R_028C0C_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_1 0x28c0c
#define CM_R_028C10_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_2 0x28c10
#define CM_R_028C14_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_3 0x28c14

#define CM_R_028C18_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0 0x28c18
#define CM_R_028C1C_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_1 0x28c1c
#define CM_R_028C20_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_2 0x28c20
#define CM_R_028C24_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_3 0x28c24

#define CM_R_028C28_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0 0x28c28
#define CM_R_028C2C_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_1 0x28c2c
#define CM_R_028C30_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_2 0x28c30
#define CM_R_028C34_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_3 0x28c34

#define CM_R_028C38_PA_SC_AA_MASK_X0Y0_X1Y0           0x28c38
#define CM_R_028C3C_PA_SC_AA_MASK_X0Y1_X1Y1           0x28c3c

#define CM_R_028AA8_IA_MULTI_VGT_PARAM                0x028AA8
#define   S_028AA8_PRIMGROUP_SIZE(x)                   (((unsigned)(x) & 0xFFFF) << 0)
#define   G_028AA8_PRIMGROUP_SIZE(x)                   (((x) >> 0) & 0xFFFF)
#define   C_028AA8_PRIMGROUP_SIZE                      0xFFFF0000
#define   S_028AA8_PARTIAL_VS_WAVE_ON(x)               (((unsigned)(x) & 0x1) << 16)
#define   G_028AA8_PARTIAL_VS_WAVE_ON(x)               (((x) >> 16) & 0x1)
#define   C_028AA8_PARTIAL_VS_WAVE_ON                  0xFFFEFFFF
#define   S_028AA8_SWITCH_ON_EOP(x)                    (((unsigned)(x) & 0x1) << 17)
#define   G_028AA8_SWITCH_ON_EOP(x)                    (((x) >> 17) & 0x1)
#define   C_028AA8_SWITCH_ON_EOP                       0xFFFDFFFF

/* async DMA packets */
#define DMA_PACKET(cmd, sub_cmd, n) ((((unsigned)(cmd) & 0xF) << 28) |    \
                                    (((unsigned)(sub_cmd) & 0xFF) << 20) |\
                                    (((unsigned)(n) & 0xFFFFF) << 0))
/* async DMA Packet types */
#define    DMA_PACKET_WRITE                     0x2
#define    DMA_PACKET_COPY                      0x3
#define    EG_DMA_COPY_MAX_SIZE			0xfffff
#define    EG_DMA_COPY_DWORD_ALIGNED		0x00
#define    EG_DMA_COPY_BYTE_ALIGNED		0x40
#define    EG_DMA_COPY_TILED			0x8
#define    DMA_PACKET_INDIRECT_BUFFER           0x4
#define    DMA_PACKET_SEMAPHORE                 0x5
#define    DMA_PACKET_FENCE                     0x6
#define    DMA_PACKET_TRAP                      0x7
#define    DMA_PACKET_SRBM_WRITE                0x9
#define    DMA_PACKET_CONSTANT_FILL             0xd
#define    DMA_PACKET_NOP                       0xf

#define EG_FETCH_CONSTANTS_OFFSET_PS 0
#define EG_FETCH_CONSTANTS_OFFSET_VS 176
#define EG_FETCH_CONSTANTS_OFFSET_GS 336
#define EG_FETCH_CONSTANTS_OFFSET_HS 496
#define EG_FETCH_CONSTANTS_OFFSET_LS 656
#define EG_FETCH_CONSTANTS_OFFSET_CS 816
#define EG_FETCH_CONSTANTS_OFFSET_FS 992

#endif
