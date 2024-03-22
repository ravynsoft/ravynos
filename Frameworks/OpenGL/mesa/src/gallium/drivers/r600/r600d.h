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
#ifndef R600D_H
#define R600D_H

#define R600_TEXEL_PITCH_ALIGNMENT_MASK        0x7

/* evergreen values */
#define EG_RESOURCE_OFFSET                 0x00030000
#define EG_RESOURCE_END                    0x00034000
#define EG_LOOP_CONST_OFFSET               0x0003A200
#define EG_LOOP_CONST_END                  0x0003A26C
#define EG_BOOL_CONST_OFFSET               0x0003A500
#define EG_BOOL_CONST_END                  0x0003A506

#define R600_CONFIG_REG_END                    0X0000AC00
#define R600_CONTEXT_REG_END                   0X00029000
#define R600_ALU_CONST_OFFSET                  0X00030000
#define R600_ALU_CONST_END                     0X00032000
#define R600_RESOURCE_OFFSET                   0X00038000
#define R600_RESOURCE_END                      0X0003C000
#define R600_SAMPLER_OFFSET                    0X0003C000
#define R600_SAMPLER_END                       0X0003CFF0
#define R600_CTL_CONST_END                     0X0003E200
#define R600_LOOP_CONST_OFFSET                 0X0003E200
#define R600_LOOP_CONST_END                    0X0003E380
#define R600_BOOL_CONST_OFFSET                 0X0003E380
#define R600_BOOL_CONST_END                    0X00040000


#define PKT3_NOP                               0x10
#define EG_PKT3_SET_BASE                       0x11 /* >= evergreen */
#define     EG_DRAW_INDEX_INDIRECT_PATCH_TABLE_BASE 1 /* DX11 Draw_Index_Indirect Patch Table Base */
#define EG_PKT3_INDEX_BUFFER_SIZE              0x13
#define PKT3_INDIRECT_BUFFER_END               0x17
#define PKT3_SET_PREDICATION                   0x20
#define PKT3_REG_RMW                           0x21
#define PKT3_COND_EXEC                         0x22
#define PKT3_PRED_EXEC                         0x23
#define PKT3_START_3D_CMDBUF                   0x24 /* removed on evergreen */
#define EG_PKT3_DRAW_INDIRECT                  0x24 /* >= evergreen */
#define EG_PKT3_DRAW_INDEX_INDIRECT            0x25
#define EG_PKT3_INDEX_BASE                     0x26
#define PKT3_DRAW_INDEX_2                      0x27
#define PKT3_CONTEXT_CONTROL                   0x28
#define PKT3_DRAW_INDEX_IMMD_BE                0x29
#define PKT3_INDEX_TYPE                        0x2A
#define		VGT_INDEX_16                   0
#define		VGT_INDEX_32                   1
#define         VGT_DMA_SWAP_NONE	       (0 << 2)
#define         VGT_DMA_SWAP_16_BIT	       (1 << 2)
#define         VGT_DMA_SWAP_32_BIT	       (2 << 2)
#define         VGT_DMA_SWAP_WORD	       (3 << 2)
#define PKT3_DRAW_INDEX                        0x2B
#define PKT3_DRAW_INDEX_AUTO                   0x2D
#define PKT3_DRAW_INDEX_IMMD                   0x2E
#define PKT3_NUM_INSTANCES                     0x2F
#define PKT3_STRMOUT_BUFFER_UPDATE             0x34
#define		STRMOUT_STORE_BUFFER_FILLED_SIZE	1
#define		STRMOUT_OFFSET_SOURCE(x)	(((unsigned)(x) & 0x3) << 1)
#define			STRMOUT_OFFSET_FROM_PACKET		0
#define			STRMOUT_OFFSET_FROM_VGT_FILLED_SIZE	1
#define			STRMOUT_OFFSET_FROM_MEM			2
#define			STRMOUT_OFFSET_NONE			3
#define		STRMOUT_SELECT_BUFFER(x)	(((unsigned)(x) & 0x3) << 8)
#define PKT3_INDIRECT_BUFFER_MP                0x38
#define PKT3_MEM_SEMAPHORE                     0x39
#define PKT3_MPEG_INDEX                        0x3A
#define PKT3_COPY_DW			       0x3B
#define		COPY_DW_SRC_IS_REG		(0 << 0)
#define		COPY_DW_SRC_IS_MEM		(1 << 0)
#define		COPY_DW_DST_IS_REG		(0 << 1)
#define		COPY_DW_DST_IS_MEM		(1 << 1)
#define PKT3_WAIT_REG_MEM                      0x3C
#define		WAIT_REG_MEM_EQUAL		3
#define		WAIT_REG_MEM_GEQUAL		5
#define		WAIT_REG_MEM_MEMORY		(1 << 4)
#define		WAIT_REG_MEM_PFP		(1 << 8)
#define PKT3_MEM_WRITE                         0x3D
#define		MEM_WRITE_32_BITS		(1 << 18)
#define PKT3_INDIRECT_BUFFER                   0x32
#define PKT3_PFP_SYNC_ME		       0x42 /* EG+ */
#define PKT3_SURFACE_SYNC                      0x43
#define PKT3_ME_INITIALIZE                     0x44
#define PKT3_COND_WRITE                        0x45
#define PKT3_EVENT_WRITE                       0x46
#define PKT3_EVENT_WRITE_EOP                   0x47
#define PKT3_ONE_REG_WRITE                     0x57
#define PKT3_SET_CONFIG_REG                    0x68
#define PKT3_SET_CONTEXT_REG                   0x69
#define PKT3_SET_ALU_CONST                     0x6A
#define PKT3_SET_BOOL_CONST                    0x6B
#define PKT3_SET_LOOP_CONST                    0x6C
#define PKT3_SET_RESOURCE                      0x6D
#define PKT3_SET_SAMPLER                       0x6E
#define PKT3_SET_CTL_CONST                     0x6F
#define PKT3_STRMOUT_BASE_UPDATE	       0x72
#define PKT3_SURFACE_BASE_UPDATE               0x73
#define		SURFACE_BASE_UPDATE_DEPTH      (1 << 0)
#define		SURFACE_BASE_UPDATE_COLOR(x)   (2 << (x))
#define		SURFACE_BASE_UPDATE_COLOR_NUM(x) (((1 << x) - 1) << 1)
#define		SURFACE_BASE_UPDATE_STRMOUT(x) (0x200 << (x))

#define EVENT_TYPE_CS_PARTIAL_FLUSH            0x07 /* eg+ */
#define EVENT_TYPE_PS_PARTIAL_FLUSH            0x10
#define EVENT_TYPE_CACHE_FLUSH_AND_INV_TS_EVENT 0x14
#define EVENT_TYPE_ZPASS_DONE                  0x15
#define EVENT_TYPE_CACHE_FLUSH_AND_INV_EVENT   0x16
#define EVENT_TYPE_PIPELINESTAT_START		25
#define EVENT_TYPE_PIPELINESTAT_STOP		26
#define EVENT_TYPE_SAMPLE_PIPELINESTAT		30
#define EVENT_TYPE_SO_VGTSTREAMOUT_FLUSH	0x1f
#define EVENT_TYPE_SAMPLE_STREAMOUTSTATS	0x20
#define EVENT_TYPE_FLUSH_AND_INV_DB_META       0x2c /* supported on r700+ */
#define EVENT_TYPE_VGT_FLUSH                   0x24
#define EVENT_TYPE_SQ_NON_EVENT                0x26
#define EVENT_TYPE_FLUSH_AND_INV_CB_META	46 /* supported on r700+ */
#define		EVENT_TYPE(x)                           ((x) << 0)
#define		EVENT_INDEX(x)                          ((x) << 8)
                /* 0 - any non-TS event
		 * 1 - ZPASS_DONE
		 * 2 - SAMPLE_PIPELINESTAT
		 * 3 - SAMPLE_STREAMOUTSTAT*
		 * 4 - *S_PARTIAL_FLUSH
		 * 5 - TS events
		 */

#define PREDICATION_OP_CLEAR 0x0
#define PREDICATION_OP_ZPASS 0x1
#define PREDICATION_OP_PRIMCOUNT 0x2

#define PRED_OP(x) ((x) << 16)

#define PREDICATION_CONTINUE (1 << 31)

#define PREDICATION_HINT_WAIT (0 << 12)
#define PREDICATION_HINT_NOWAIT_DRAW (1 << 12)

#define PREDICATION_DRAW_NOT_VISIBLE (0 << 8)
#define PREDICATION_DRAW_VISIBLE (1 << 8)

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
#define PKT3_PRED_S(x)               (((x) >> 0) & 0x1)
#define PKT0(index, count) (PKT_TYPE_S(0) | PKT0_BASE_INDEX_S(index) | PKT_COUNT_S(count))

#define PKT3_CP_DMA					0x41
/* 1. header
 * 2. SRC_ADDR_LO [31:0]
 * 3. CP_SYNC [31] | SRC_ADDR_HI [7:0]
 * 4. DST_ADDR_LO [31:0]
 * 5. DST_ADDR_HI [7:0]
 * 6. COMMAND [29:22] | BYTE_COUNT [20:0]
 */
#define PKT3_CP_DMA_CP_SYNC       (1 << 31)
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


/* Registers */
#define R_008490_CP_STRMOUT_CNTL		     0x008490
#define   S_008490_OFFSET_UPDATE_DONE(x)		(((unsigned)(x) & 0x1) << 0)
#define R_008C40_SQ_ESGS_RING_BASE                   0x008C40
#define R_008C44_SQ_ESGS_RING_SIZE                   0x008C44
#define R_008C48_SQ_GSVS_RING_BASE                   0x008C48
#define R_008C4C_SQ_GSVS_RING_SIZE                   0x008C4C
#define R_008C50_SQ_ESTMP_RING_BASE                  0x008C50
#define R_008C54_SQ_ESTMP_RING_SIZE                  0x008C54
#define R_008C58_SQ_GSTMP_RING_BASE                  0x008C58
#define R_008C5C_SQ_GSTMP_RING_SIZE                  0x008C5C
#define R_008C68_SQ_PSTMP_RING_BASE                  0x008C68
#define R_008C6C_SQ_PSTMP_RING_SIZE                  0x008C6C
#define R_008C60_SQ_VSTMP_RING_BASE                  0x008C60
#define R_008C64_SQ_VSTMP_RING_SIZE                  0x008C64

#define R_0088C8_VGT_GS_PER_ES                       0x0088C8
#define R_0088CC_VGT_ES_PER_GS                       0x0088CC
#define R_0088E8_VGT_GS_PER_VS                       0x0088E8

#define R_008960_VGT_STRMOUT_BUFFER_FILLED_SIZE_0    0x008960 /* read-only */
#define R_008964_VGT_STRMOUT_BUFFER_FILLED_SIZE_1    0x008964 /* read-only */
#define R_008968_VGT_STRMOUT_BUFFER_FILLED_SIZE_2    0x008968 /* read-only */
#define R_00896C_VGT_STRMOUT_BUFFER_FILLED_SIZE_3    0x00896C /* read-only */
#define R_008B40_PA_SC_AA_SAMPLE_LOCS_2S                0x008B40
#define R_008B44_PA_SC_AA_SAMPLE_LOCS_4S                0x008B44
#define R_008B48_PA_SC_AA_SAMPLE_LOCS_8S_WD0            0x008B48
#define R_008B4C_PA_SC_AA_SAMPLE_LOCS_8S_WD1            0x008B4C
#define R_008C00_SQ_CONFIG                           0x00008C00
#define   S_008C00_VC_ENABLE(x)                        (((unsigned)(x) & 0x1) << 0)
#define   G_008C00_VC_ENABLE(x)                        (((x) >> 0) & 0x1)
#define   C_008C00_VC_ENABLE(x)                        0xFFFFFFFE
#define   S_008C00_EXPORT_SRC_C(x)                     (((unsigned)(x) & 0x1) << 1)
#define   G_008C00_EXPORT_SRC_C(x)                     (((x) >> 1) & 0x1)
#define   C_008C00_EXPORT_SRC_C(x)                     0xFFFFFFFD
#define   S_008C00_DX9_CONSTS(x)                       (((unsigned)(x) & 0x1) << 2)
#define   G_008C00_DX9_CONSTS(x)                       (((x) >> 2) & 0x1)
#define   C_008C00_DX9_CONSTS(x)                       0xFFFFFFFB
#define   S_008C00_ALU_INST_PREFER_VECTOR(x)           (((unsigned)(x) & 0x1) << 3)
#define   G_008C00_ALU_INST_PREFER_VECTOR(x)           (((x) >> 3) & 0x1)
#define   C_008C00_ALU_INST_PREFER_VECTOR(x)           0xFFFFFFF7
#define   S_008C00_DX10_CLAMP(x)                       (((unsigned)(x) & 0x1) << 4)
#define   G_008C00_DX10_CLAMP(x)                       (((x) >> 4) & 0x1)
#define   C_008C00_DX10_CLAMP(x)                       0xFFFFFFEF
#define   S_008C00_CLAUSE_SEQ_PRIO(x)                  (((unsigned)(x) & 0x3) << 8)
#define   G_008C00_CLAUSE_SEQ_PRIO(x)                  (((x) >> 8) & 0x3)
#define   C_008C00_CLAUSE_SEQ_PRIO(x)                  0xFFFFFCFF
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
#define R_008C0C_SQ_THREAD_RESOURCE_MGMT             0x00008C0C
#define   S_008C0C_NUM_PS_THREADS(x)                   (((unsigned)(x) & 0xFF) << 0)
#define   G_008C0C_NUM_PS_THREADS(x)                   (((x) >> 0) & 0xFF)
#define   C_008C0C_NUM_PS_THREADS(x)                   0xFFFFFF00
#define   S_008C0C_NUM_VS_THREADS(x)                   (((unsigned)(x) & 0xFF) << 8)
#define   G_008C0C_NUM_VS_THREADS(x)                   (((x) >> 8) & 0xFF)
#define   C_008C0C_NUM_VS_THREADS(x)                   0xFFFF00FF
#define   S_008C0C_NUM_GS_THREADS(x)                   (((unsigned)(x) & 0xFF) << 16)
#define   G_008C0C_NUM_GS_THREADS(x)                   (((x) >> 16) & 0xFF)
#define   C_008C0C_NUM_GS_THREADS(x)                   0xFF00FFFF
#define   S_008C0C_NUM_ES_THREADS(x)                   (((unsigned)(x) & 0xFF) << 24)
#define   G_008C0C_NUM_ES_THREADS(x)                   (((x) >> 24) & 0xFF)
#define   C_008C0C_NUM_ES_THREADS(x)                   0x00FFFFFF
#define R_008C10_SQ_STACK_RESOURCE_MGMT_1            0x00008C10
#define   S_008C10_NUM_PS_STACK_ENTRIES(x)             (((unsigned)(x) & 0xFFF) << 0)
#define   G_008C10_NUM_PS_STACK_ENTRIES(x)             (((x) >> 0) & 0xFFF)
#define   C_008C10_NUM_PS_STACK_ENTRIES(x)             0xFFFFF000
#define   S_008C10_NUM_VS_STACK_ENTRIES(x)             (((unsigned)(x) & 0xFFF) << 16)
#define   G_008C10_NUM_VS_STACK_ENTRIES(x)             (((x) >> 16) & 0xFFF)
#define   C_008C10_NUM_VS_STACK_ENTRIES(x)             0xF000FFFF
#define R_008C14_SQ_STACK_RESOURCE_MGMT_2            0x00008C14
#define   S_008C14_NUM_GS_STACK_ENTRIES(x)             (((unsigned)(x) & 0xFFF) << 0)
#define   G_008C14_NUM_GS_STACK_ENTRIES(x)             (((x) >> 0) & 0xFFF)
#define   C_008C14_NUM_GS_STACK_ENTRIES(x)             0xFFFFF000
#define   S_008C14_NUM_ES_STACK_ENTRIES(x)             (((unsigned)(x) & 0xFFF) << 16)
#define   G_008C14_NUM_ES_STACK_ENTRIES(x)             (((x) >> 16) & 0xFFF)
#define   C_008C14_NUM_ES_STACK_ENTRIES(x)             0xF000FFFF
#define R_0280A0_CB_COLOR0_INFO                      0x0280A0
#define   S_0280A0_ENDIAN(x)                           (((unsigned)(x) & 0x3) << 0)
#define   G_0280A0_ENDIAN(x)                           (((x) >> 0) & 0x3)
#define   C_0280A0_ENDIAN                              0xFFFFFFFC
#define   S_0280A0_FORMAT(x)                           (((unsigned)(x) & 0x3F) << 2)
#define   G_0280A0_FORMAT(x)                           (((x) >> 2) & 0x3F)
#define   C_0280A0_FORMAT                              0xFFFFFF03
#define     V_0280A0_COLOR_INVALID                     0x00000000
#define     V_0280A0_COLOR_8                           0x00000001
#define     V_0280A0_COLOR_4_4                         0x00000002
#define     V_0280A0_COLOR_3_3_2                       0x00000003
#define     V_0280A0_COLOR_16                          0x00000005
#define     V_0280A0_COLOR_16_FLOAT                    0x00000006
#define     V_0280A0_COLOR_8_8                         0x00000007
#define     V_0280A0_COLOR_5_6_5                       0x00000008
#define     V_0280A0_COLOR_6_5_5                       0x00000009
#define     V_0280A0_COLOR_1_5_5_5                     0x0000000A
#define     V_0280A0_COLOR_4_4_4_4                     0x0000000B
#define     V_0280A0_COLOR_5_5_5_1                     0x0000000C
#define     V_0280A0_COLOR_32                          0x0000000D
#define     V_0280A0_COLOR_32_FLOAT                    0x0000000E
#define     V_0280A0_COLOR_16_16                       0x0000000F
#define     V_0280A0_COLOR_16_16_FLOAT                 0x00000010
#define     V_0280A0_COLOR_8_24                        0x00000011
#define     V_0280A0_COLOR_8_24_FLOAT                  0x00000012
#define     V_0280A0_COLOR_24_8                        0x00000013
#define     V_0280A0_COLOR_24_8_FLOAT                  0x00000014
#define     V_0280A0_COLOR_10_11_11                    0x00000015
#define     V_0280A0_COLOR_10_11_11_FLOAT              0x00000016
#define     V_0280A0_COLOR_11_11_10                    0x00000017
#define     V_0280A0_COLOR_11_11_10_FLOAT              0x00000018
#define     V_0280A0_COLOR_2_10_10_10                  0x00000019
#define     V_0280A0_COLOR_8_8_8_8                     0x0000001A
#define     V_0280A0_COLOR_10_10_10_2                  0x0000001B
#define     V_0280A0_COLOR_X24_8_32_FLOAT              0x0000001C
#define     V_0280A0_COLOR_32_32                       0x0000001D
#define     V_0280A0_COLOR_32_32_FLOAT                 0x0000001E
#define     V_0280A0_COLOR_16_16_16_16                 0x0000001F
#define     V_0280A0_COLOR_16_16_16_16_FLOAT           0x00000020
#define     V_0280A0_COLOR_32_32_32_32                 0x00000022
#define     V_0280A0_COLOR_32_32_32_32_FLOAT           0x00000023
#define     V_0280A0_COLOR_32_32_32_FLOAT              0x00000030
#define   S_0280A0_ARRAY_MODE(x)                       (((unsigned)(x) & 0xF) << 8)
#define   G_0280A0_ARRAY_MODE(x)                       (((x) >> 8) & 0xF)
#define   C_0280A0_ARRAY_MODE                          0xFFFFF0FF
#define     V_0280A0_ARRAY_LINEAR_GENERAL              0x00000000
#define     V_0280A0_ARRAY_LINEAR_ALIGNED              0x00000001
#define     V_0280A0_ARRAY_1D_TILED_THIN1              0x00000002
#define     V_0280A0_ARRAY_2D_TILED_THIN1              0x00000004
#define   S_0280A0_NUMBER_TYPE(x)                      (((unsigned)(x) & 0x7) << 12)
#define   G_0280A0_NUMBER_TYPE(x)                      (((x) >> 12) & 0x7)
#define   C_0280A0_NUMBER_TYPE                         0xFFFF8FFF
#define     V_0280A0_NUMBER_UNORM                      0x00000000
#define     V_0280A0_NUMBER_SNORM                      0x00000001
#define     V_0280A0_NUMBER_USCALED                    0x00000002
#define     V_0280A0_NUMBER_SSCALED                    0x00000003
#define     V_0280A0_NUMBER_UINT                       0x00000004
#define     V_0280A0_NUMBER_SINT                       0x00000005
#define     V_0280A0_NUMBER_SRGB                       0x00000006
#define     V_0280A0_NUMBER_FLOAT                      0x00000007
#define   S_0280A0_READ_SIZE(x)                        (((unsigned)(x) & 0x1) << 15)
#define   G_0280A0_READ_SIZE(x)                        (((x) >> 15) & 0x1)
#define   C_0280A0_READ_SIZE                           0xFFFF7FFF
#define   S_0280A0_COMP_SWAP(x)                        (((unsigned)(x) & 0x3) << 16)
#define   G_0280A0_COMP_SWAP(x)                        (((x) >> 16) & 0x3)
#define   C_0280A0_COMP_SWAP                           0xFFFCFFFF
#define     V_0280A0_SWAP_STD                          0x00000000
#define     V_0280A0_SWAP_ALT                          0x00000001
#define     V_0280A0_SWAP_STD_REV                      0x00000002
#define     V_0280A0_SWAP_ALT_REV                      0x00000003
#define   S_0280A0_TILE_MODE(x)                        (((unsigned)(x) & 0x3) << 18)
#define     V_0280A0_TILE_DISABLE			0
#define     V_0280A0_CLEAR_ENABLE			1
#define     V_0280A0_FRAG_ENABLE			2
#define   G_0280A0_TILE_MODE(x)                        (((x) >> 18) & 0x3)
#define   C_0280A0_TILE_MODE                           0xFFF3FFFF
#define   S_0280A0_BLEND_CLAMP(x)                      (((unsigned)(x) & 0x1) << 20)
#define   G_0280A0_BLEND_CLAMP(x)                      (((x) >> 20) & 0x1)
#define   C_0280A0_BLEND_CLAMP                         0xFFEFFFFF
#define   S_0280A0_CLEAR_COLOR(x)                      (((unsigned)(x) & 0x1) << 21)
#define   G_0280A0_CLEAR_COLOR(x)                      (((x) >> 21) & 0x1)
#define   C_0280A0_CLEAR_COLOR                         0xFFDFFFFF
#define   S_0280A0_BLEND_BYPASS(x)                     (((unsigned)(x) & 0x1) << 22)
#define   G_0280A0_BLEND_BYPASS(x)                     (((x) >> 22) & 0x1)
#define   C_0280A0_BLEND_BYPASS                        0xFFBFFFFF
#define   S_0280A0_BLEND_FLOAT32(x)                    (((unsigned)(x) & 0x1) << 23)
#define   G_0280A0_BLEND_FLOAT32(x)                    (((x) >> 23) & 0x1)
#define   C_0280A0_BLEND_FLOAT32                       0xFF7FFFFF
#define   S_0280A0_SIMPLE_FLOAT(x)                     (((unsigned)(x) & 0x1) << 24)
#define   G_0280A0_SIMPLE_FLOAT(x)                     (((x) >> 24) & 0x1)
#define   C_0280A0_SIMPLE_FLOAT                        0xFEFFFFFF
#define   S_0280A0_ROUND_MODE(x)                       (((unsigned)(x) & 0x1) << 25)
#define   G_0280A0_ROUND_MODE(x)                       (((x) >> 25) & 0x1)
#define   C_0280A0_ROUND_MODE                          0xFDFFFFFF
#define   S_0280A0_TILE_COMPACT(x)                     (((unsigned)(x) & 0x1) << 26)
#define   G_0280A0_TILE_COMPACT(x)                     (((x) >> 26) & 0x1)
#define   C_0280A0_TILE_COMPACT                        0xFBFFFFFF
#define   S_0280A0_SOURCE_FORMAT(x)                    (((unsigned)(x) & 0x1) << 27)
#define   G_0280A0_SOURCE_FORMAT(x)                    (((x) >> 27) & 0x1)
#define   C_0280A0_SOURCE_FORMAT                       0xF7FFFFFF
#define     V_0280A0_EXPORT_FULL                       0
#define     V_0280A0_EXPORT_NORM                       1
#define R_028060_CB_COLOR0_SIZE                      0x028060
#define   S_028060_PITCH_TILE_MAX(x)                   (((unsigned)(x) & 0x3FF) << 0)
#define   G_028060_PITCH_TILE_MAX(x)                   (((x) >> 0) & 0x3FF)
#define   C_028060_PITCH_TILE_MAX                      0xFFFFFC00
#define   S_028060_SLICE_TILE_MAX(x)                   (((unsigned)(x) & 0xFFFFF) << 10)
#define   G_028060_SLICE_TILE_MAX(x)                   (((x) >> 10) & 0xFFFFF)
#define   C_028060_SLICE_TILE_MAX                      0xC00003FF
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
#define   S_028808_FOG_ENABLE(x)                       (((unsigned)(x) & 0x1) << 0)
#define   G_028808_FOG_ENABLE(x)                       (((x) >> 0) & 0x1)
#define   C_028808_FOG_ENABLE                          0xFFFFFFFE
#define   S_028808_MULTIWRITE_ENABLE(x)                (((unsigned)(x) & 0x1) << 1)
#define   G_028808_MULTIWRITE_ENABLE(x)                (((x) >> 1) & 0x1)
#define   C_028808_MULTIWRITE_ENABLE                   0xFFFFFFFD
#define   S_028808_DITHER_ENABLE(x)                    (((unsigned)(x) & 0x1) << 2)
#define   G_028808_DITHER_ENABLE(x)                    (((x) >> 2) & 0x1)
#define   C_028808_DITHER_ENABLE                       0xFFFFFFFB
#define   S_028808_DEGAMMA_ENABLE(x)                   (((unsigned)(x) & 0x1) << 3)
#define   G_028808_DEGAMMA_ENABLE(x)                   (((x) >> 3) & 0x1)
#define   C_028808_DEGAMMA_ENABLE                      0xFFFFFFF7
#define   S_028808_SPECIAL_OP(x)                       (((unsigned)(x) & 0x7) << 4)
#define		V_028808_NORMAL				0
#define		V_028808_DISABLE			1
#define   G_028808_SPECIAL_OP(x)                       (((x) >> 4) & 0x7)
#define   C_028808_SPECIAL_OP                          0xFFFFFF8F
#define     V_028808_SPECIAL_NORMAL                     0x00
#define     V_028808_SPECIAL_DISABLE                    0x01
#define     V_028808_SPECIAL_FAST_CLEAR                 0x02
#define     V_028808_SPECIAL_FORCE_CLEAR                0x03
#define     V_028808_SPECIAL_EXPAND_COLOR               0x04
#define     V_028808_SPECIAL_EXPAND_TEXTURE             0x05
#define     V_028808_SPECIAL_EXPAND_SAMPLES             0x06
#define     V_028808_SPECIAL_RESOLVE_BOX                0x07
#define   S_028808_PER_MRT_BLEND(x)                    (((unsigned)(x) & 0x1) << 7)
#define   G_028808_PER_MRT_BLEND(x)                    (((x) >> 7) & 0x1)
#define   C_028808_PER_MRT_BLEND                       0xFFFFFF7F
#define   S_028808_TARGET_BLEND_ENABLE(x)              (((unsigned)(x) & 0xFF) << 8)
#define   G_028808_TARGET_BLEND_ENABLE(x)              (((x) >> 8) & 0xFF)
#define   C_028808_TARGET_BLEND_ENABLE                 0xFFFF00FF
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
#define   S_028810_DX_RASTERIZATION_KILL(x)            (((unsigned)(x) & 0x1) << 22) /* R700 only? */
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
#define R_028010_DB_DEPTH_INFO                       0x028010
#define   S_028010_FORMAT(x)                           (((unsigned)(x) & 0x7) << 0)
#define   G_028010_FORMAT(x)                           (((x) >> 0) & 0x7)
#define   C_028010_FORMAT                              0xFFFFFFF8
#define     V_028010_DEPTH_INVALID                     0x00000000
#define     V_028010_DEPTH_16                          0x00000001
#define     V_028010_DEPTH_X8_24                       0x00000002
#define     V_028010_DEPTH_8_24                        0x00000003
#define     V_028010_DEPTH_X8_24_FLOAT                 0x00000004
#define     V_028010_DEPTH_8_24_FLOAT                  0x00000005
#define     V_028010_DEPTH_32_FLOAT                    0x00000006
#define     V_028010_DEPTH_X24_8_32_FLOAT              0x00000007
#define   S_028010_READ_SIZE(x)                        (((unsigned)(x) & 0x1) << 3)
#define   G_028010_READ_SIZE(x)                        (((x) >> 3) & 0x1)
#define   C_028010_READ_SIZE                           0xFFFFFFF7
#define   S_028010_ARRAY_MODE(x)                       (((unsigned)(x) & 0xF) << 15)
#define   G_028010_ARRAY_MODE(x)                       (((x) >> 15) & 0xF)
#define   C_028010_ARRAY_MODE                          0xFFF87FFF
#define   S_028010_TILE_SURFACE_ENABLE(x)              (((unsigned)(x) & 0x1) << 25)
#define   G_028010_TILE_SURFACE_ENABLE(x)              (((x) >> 25) & 0x1)
#define   C_028010_TILE_SURFACE_ENABLE                 0xFDFFFFFF
#define   S_028010_TILE_COMPACT(x)                     (((unsigned)(x) & 0x1) << 26)
#define   G_028010_TILE_COMPACT(x)                     (((x) >> 26) & 0x1)
#define   C_028010_TILE_COMPACT                        0xFBFFFFFF
#define   S_028010_ZRANGE_PRECISION(x)                 (((unsigned)(x) & 0x1) << 31)
#define   G_028010_ZRANGE_PRECISION(x)                 (((x) >> 31) & 0x1)
#define   C_028010_ZRANGE_PRECISION                    0x7FFFFFFF
#define R_028014_DB_HTILE_DATA_BASE                  0x00028014
#define R_028414_CB_BLEND_RED                        0x028414
#define   S_028414_BLEND_RED(x)                        (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028414_BLEND_RED(x)                        (((x) >> 0) & 0xFFFFFFFF)
#define   C_028414_BLEND_RED                           0x00000000
#define R_028418_CB_BLEND_GREEN                      0x028418
#define   S_028418_BLEND_GREEN(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028418_BLEND_GREEN(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_028418_BLEND_GREEN                         0x00000000
#define R_02841C_CB_BLEND_BLUE                       0x02841C
#define   S_02841C_BLEND_BLUE(x)                       (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_02841C_BLEND_BLUE(x)                       (((x) >> 0) & 0xFFFFFFFF)
#define   C_02841C_BLEND_BLUE                          0x00000000
#define R_028420_CB_BLEND_ALPHA                      0x028420
#define   S_028420_BLEND_ALPHA(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028420_BLEND_ALPHA(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_028420_BLEND_ALPHA                         0x00000000
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
#define R_028784_CB_BLEND1_CONTROL                   0x028784
#define R_028788_CB_BLEND2_CONTROL                   0x028788
#define R_02878C_CB_BLEND3_CONTROL                   0x02878C
#define R_028790_CB_BLEND4_CONTROL                   0x028790
#define R_028794_CB_BLEND5_CONTROL                   0x028794
#define R_028798_CB_BLEND6_CONTROL                   0x028798
#define R_02879C_CB_BLEND7_CONTROL                   0x02879C
#define R_028804_CB_BLEND_CONTROL                    0x028804
#define   S_028804_COLOR_SRCBLEND(x)                   (((unsigned)(x) & 0x1F) << 0)
#define   G_028804_COLOR_SRCBLEND(x)                   (((x) >> 0) & 0x1F)
#define   C_028804_COLOR_SRCBLEND                      0xFFFFFFE0
#define     V_028804_BLEND_ZERO                        0x00000000
#define     V_028804_BLEND_ONE                         0x00000001
#define     V_028804_BLEND_SRC_COLOR                   0x00000002
#define     V_028804_BLEND_ONE_MINUS_SRC_COLOR         0x00000003
#define     V_028804_BLEND_SRC_ALPHA                   0x00000004
#define     V_028804_BLEND_ONE_MINUS_SRC_ALPHA         0x00000005
#define     V_028804_BLEND_DST_ALPHA                   0x00000006
#define     V_028804_BLEND_ONE_MINUS_DST_ALPHA         0x00000007
#define     V_028804_BLEND_DST_COLOR                   0x00000008
#define     V_028804_BLEND_ONE_MINUS_DST_COLOR         0x00000009
#define     V_028804_BLEND_SRC_ALPHA_SATURATE          0x0000000A
#define     V_028804_BLEND_BOTH_SRC_ALPHA              0x0000000B
#define     V_028804_BLEND_BOTH_INV_SRC_ALPHA          0x0000000C
#define     V_028804_BLEND_CONST_COLOR                 0x0000000D
#define     V_028804_BLEND_ONE_MINUS_CONST_COLOR       0x0000000E
#define     V_028804_BLEND_SRC1_COLOR                  0x0000000F
#define     V_028804_BLEND_INV_SRC1_COLOR              0x00000010
#define     V_028804_BLEND_SRC1_ALPHA                  0x00000011
#define     V_028804_BLEND_INV_SRC1_ALPHA              0x00000012
#define     V_028804_BLEND_CONST_ALPHA                 0x00000013
#define     V_028804_BLEND_ONE_MINUS_CONST_ALPHA       0x00000014
#define   S_028804_COLOR_COMB_FCN(x)                   (((unsigned)(x) & 0x7) << 5)
#define   G_028804_COLOR_COMB_FCN(x)                   (((x) >> 5) & 0x7)
#define   C_028804_COLOR_COMB_FCN                      0xFFFFFF1F
#define     V_028804_COMB_DST_PLUS_SRC                 0x00000000
#define     V_028804_COMB_SRC_MINUS_DST                0x00000001
#define     V_028804_COMB_MIN_DST_SRC                  0x00000002
#define     V_028804_COMB_MAX_DST_SRC                  0x00000003
#define     V_028804_COMB_DST_MINUS_SRC                0x00000004
#define   S_028804_COLOR_DESTBLEND(x)                  (((unsigned)(x) & 0x1F) << 8)
#define   G_028804_COLOR_DESTBLEND(x)                  (((x) >> 8) & 0x1F)
#define   C_028804_COLOR_DESTBLEND                     0xFFFFE0FF
#define   S_028804_OPACITY_WEIGHT(x)                   (((unsigned)(x) & 0x1) << 13)
#define   G_028804_OPACITY_WEIGHT(x)                   (((x) >> 13) & 0x1)
#define   C_028804_OPACITY_WEIGHT                      0xFFFFDFFF
#define   S_028804_ALPHA_SRCBLEND(x)                   (((unsigned)(x) & 0x1F) << 16)
#define   G_028804_ALPHA_SRCBLEND(x)                   (((x) >> 16) & 0x1F)
#define   C_028804_ALPHA_SRCBLEND                      0xFFE0FFFF
#define   S_028804_ALPHA_COMB_FCN(x)                   (((unsigned)(x) & 0x7) << 21)
#define   G_028804_ALPHA_COMB_FCN(x)                   (((x) >> 21) & 0x7)
#define   C_028804_ALPHA_COMB_FCN                      0xFF1FFFFF
#define   S_028804_ALPHA_DESTBLEND(x)                  (((unsigned)(x) & 0x1F) << 24)
#define   G_028804_ALPHA_DESTBLEND(x)                  (((x) >> 24) & 0x1F)
#define   C_028804_ALPHA_DESTBLEND                     0xE0FFFFFF
#define   S_028804_SEPARATE_ALPHA_BLEND(x)             (((unsigned)(x) & 0x1) << 29)
#define   G_028804_SEPARATE_ALPHA_BLEND(x)             (((x) >> 29) & 0x1)
#define   C_028804_SEPARATE_ALPHA_BLEND                0xDFFFFFFF
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
#define R_028000_DB_DEPTH_SIZE                       0x028000
#define   S_028000_PITCH_TILE_MAX(x)                   (((unsigned)(x) & 0x3FF) << 0)
#define   G_028000_PITCH_TILE_MAX(x)                   (((x) >> 0) & 0x3FF)
#define   C_028000_PITCH_TILE_MAX                      0xFFFFFC00
#define   S_028000_SLICE_TILE_MAX(x)                   (((unsigned)(x) & 0xFFFFF) << 10)
#define   G_028000_SLICE_TILE_MAX(x)                   (((x) >> 10) & 0xFFFFF)
#define   C_028000_SLICE_TILE_MAX                      0xC00003FF
#define R_028004_DB_DEPTH_VIEW                       0x028004
#define   S_028004_SLICE_START(x)                      (((unsigned)(x) & 0x7FF) << 0)
#define   G_028004_SLICE_START(x)                      (((x) >> 0) & 0x7FF)
#define   C_028004_SLICE_START                         0xFFFFF800
#define   S_028004_SLICE_MAX(x)                        (((unsigned)(x) & 0x7FF) << 13)
#define   G_028004_SLICE_MAX(x)                        (((x) >> 13) & 0x7FF)
#define   C_028004_SLICE_MAX                           0xFF001FFF
#define R_028D24_DB_HTILE_SURFACE                    0x028D24
#define   S_028D24_HTILE_WIDTH(x)                      (((unsigned)(x) & 0x1) << 0)
#define   G_028D24_HTILE_WIDTH(x)                      (((x) >> 0) & 0x1)
#define   C_028D24_HTILE_WIDTH                         0xFFFFFFFE
#define   S_028D24_HTILE_HEIGHT(x)                     (((unsigned)(x) & 0x1) << 1)
#define   G_028D24_HTILE_HEIGHT(x)                     (((x) >> 1) & 0x1)
#define   C_028D24_HTILE_HEIGHT                        0xFFFFFFFD
#define   S_028D24_LINEAR(x)                           (((unsigned)(x) & 0x1) << 2)
#define   G_028D24_LINEAR(x)                           (((x) >> 2) & 0x1)
#define   C_028D24_LINEAR                              0xFFFFFFFB
#define   S_028D24_FULL_CACHE(x)                       (((unsigned)(x) & 0x1) << 3)
#define   G_028D24_FULL_CACHE(x)                       (((x) >> 3) & 0x1)
#define   C_028D24_FULL_CACHE                          0xFFFFFFF7
#define   S_028D24_HTILE_USES_PRELOAD_WIN(x)           (((unsigned)(x) & 0x1) << 4)
#define   G_028D24_HTILE_USES_PRELOAD_WIN(x)           (((x) >> 4) & 0x1)
#define   C_028D24_HTILE_USES_PRELOAD_WIN              0xFFFFFFEF
#define   S_028D24_PRELOAD(x)                          (((unsigned)(x) & 0x1) << 5)
#define   G_028D24_PRELOAD(x)                          (((x) >> 5) & 0x1)
#define   C_028D24_PRELOAD                             0xFFFFFFDF
#define   S_028D24_PREFETCH_WIDTH(x)                   (((unsigned)(x) & 0x3F) << 6)
#define   G_028D24_PREFETCH_WIDTH(x)                   (((x) >> 6) & 0x3F)
#define   C_028D24_PREFETCH_WIDTH                      0xFFFFF03F
#define   S_028D24_PREFETCH_HEIGHT(x)                  (((unsigned)(x) & 0x3F) << 12)
#define   G_028D24_PREFETCH_HEIGHT(x)                  (((x) >> 12) & 0x3F)
#define   C_028D24_PREFETCH_HEIGHT                     0xFFFC0FFF
#define R_028D34_DB_PREFETCH_LIMIT                   0x028D34
#define   S_028D34_DEPTH_HEIGHT_TILE_MAX(x)            (((unsigned)(x) & 0x3FF) << 0)
#define   G_028D34_DEPTH_HEIGHT_TILE_MAX(x)            (((x) >> 0) & 0x3FF)
#define   C_028D34_DEPTH_HEIGHT_TILE_MAX               0xFFFFFC00
#define R_028D0C_DB_RENDER_CONTROL                   0x028D0C
#define   S_028D0C_DEPTH_CLEAR_ENABLE(x)               (((unsigned)(x) & 0x1) << 0)
#define   S_028D0C_STENCIL_CLEAR_ENABLE(x)             (((unsigned)(x) & 0x1) << 1)
#define   S_028D0C_DEPTH_COPY_ENABLE(x)                (((unsigned)(x) & 0x1) << 2)
#define   S_028D0C_STENCIL_COPY_ENABLE(x)              (((unsigned)(x) & 0x1) << 3)
#define   S_028D0C_RESUMMARIZE_ENABLE(x)               (((unsigned)(x) & 0x1) << 4)
#define   S_028D0C_STENCIL_COMPRESS_DISABLE(x)         (((unsigned)(x) & 0x1) << 5)
#define   S_028D0C_DEPTH_COMPRESS_DISABLE(x)           (((unsigned)(x) & 0x1) << 6)
#define   S_028D0C_COPY_CENTROID(x)                    (((unsigned)(x) & 0x1) << 7)
#define   S_028D0C_COPY_SAMPLE(x)                      (((unsigned)(x) & 0x03) << 8)
#define   S_028D0C_ZPASS_INCREMENT_DISABLE(x)          (((unsigned)(x) & 0x1) << 11)
#define   S_028D0C_R700_PERFECT_ZPASS_COUNTS(x)        (((unsigned)(x) & 0x1) << 15)
#define   S_028D0C_CONSERVATIVE_Z_EXPORT(x)            (((unsigned)(x) & 0x03) << 13)
#define   G_028D0C_CONSERVATIVE_Z_EXPORT(x)            (((x) >> 13) & 0x03)
#define   C_028D0C_CONSERVATIVE_Z_EXPORT               0xFFFF9FFF
#define     V_028D0C_EXPORT_ANY_Z                      0
#define     V_028D0C_EXPORT_LESS_THAN_Z                1
#define     V_028D0C_EXPORT_GREATER_THAN_Z             2
#define     V_028D0C_EXPORT_RESERVED                   3

#define R_028D10_DB_RENDER_OVERRIDE                  0x028D10
#define   V_028D10_FORCE_OFF                         0
#define   V_028D10_FORCE_ENABLE                      1
#define   V_028D10_FORCE_DISABLE                     2
#define   S_028D10_FORCE_HIZ_ENABLE(x)                 (((unsigned)(x) & 0x3) << 0)
#define   G_028D10_FORCE_HIZ_ENABLE(x)                 (((x) >> 0) & 0x3)
#define   C_028D10_FORCE_HIZ_ENABLE                    0xFFFFFFFC
#define   S_028D10_FORCE_HIS_ENABLE0(x)                (((unsigned)(x) & 0x3) << 2)
#define   G_028D10_FORCE_HIS_ENABLE0(x)                (((x) >> 2) & 0x3)
#define   C_028D10_FORCE_HIS_ENABLE0                   0xFFFFFFF3
#define   S_028D10_FORCE_HIS_ENABLE1(x)                (((unsigned)(x) & 0x3) << 4)
#define   G_028D10_FORCE_HIS_ENABLE1(x)                (((x) >> 4) & 0x3)
#define   C_028D10_FORCE_HIS_ENABLE1                   0xFFFFFFCF
#define   S_028D10_FORCE_SHADER_Z_ORDER(x)             (((unsigned)(x) & 0x1) << 6)
#define   G_028D10_FORCE_SHADER_Z_ORDER(x)             (((x) >> 6) & 0x1)
#define   C_028D10_FORCE_SHADER_Z_ORDER                0xFFFFFFBF
#define   S_028D10_FAST_Z_DISABLE(x)                   (((unsigned)(x) & 0x1) << 7)
#define   G_028D10_FAST_Z_DISABLE(x)                   (((x) >> 7) & 0x1)
#define   C_028D10_FAST_Z_DISABLE                      0xFFFFFF7F
#define   S_028D10_FAST_STENCIL_DISABLE(x)             (((unsigned)(x) & 0x1) << 8)
#define   G_028D10_FAST_STENCIL_DISABLE(x)             (((x) >> 8) & 0x1)
#define   C_028D10_FAST_STENCIL_DISABLE                0xFFFFFEFF
#define   S_028D10_NOOP_CULL_DISABLE(x)                (((unsigned)(x) & 0x1) << 9)
#define   G_028D10_NOOP_CULL_DISABLE(x)                (((x) >> 9) & 0x1)
#define   C_028D10_NOOP_CULL_DISABLE                   0xFFFFFDFF
#define   S_028D10_FORCE_COLOR_KILL(x)                 (((unsigned)(x) & 0x1) << 10)
#define   G_028D10_FORCE_COLOR_KILL(x)                 (((x) >> 10) & 0x1)
#define   C_028D10_FORCE_COLOR_KILL                    0xFFFFFBFF
#define   S_028D10_FORCE_Z_READ(x)                     (((unsigned)(x) & 0x1) << 11)
#define   G_028D10_FORCE_Z_READ(x)                     (((x) >> 11) & 0x1)
#define   C_028D10_FORCE_Z_READ                        0xFFFFF7FF
#define   S_028D10_FORCE_STENCIL_READ(x)               (((unsigned)(x) & 0x1) << 12)
#define   G_028D10_FORCE_STENCIL_READ(x)               (((x) >> 12) & 0x1)
#define   C_028D10_FORCE_STENCIL_READ                  0xFFFFEFFF
#define   S_028D10_FORCE_FULL_Z_RANGE(x)               (((unsigned)(x) & 0x3) << 13)
#define   G_028D10_FORCE_FULL_Z_RANGE(x)               (((x) >> 13) & 0x3)
#define   C_028D10_FORCE_FULL_Z_RANGE                  0xFFFF9FFF
#define   S_028D10_FORCE_QC_SMASK_CONFLICT(x)          (((unsigned)(x) & 0x1) << 15)
#define   G_028D10_FORCE_QC_SMASK_CONFLICT(x)          (((x) >> 15) & 0x1)
#define   C_028D10_FORCE_QC_SMASK_CONFLICT             0xFFFF7FFF
#define   S_028D10_DISABLE_VIEWPORT_CLAMP(x)           (((unsigned)(x) & 0x1) << 16)
#define   G_028D10_DISABLE_VIEWPORT_CLAMP(x)           (((x) >> 16) & 0x1)
#define   C_028D10_DISABLE_VIEWPORT_CLAMP              0xFFFEFFFF
#define   S_028D10_IGNORE_SC_ZRANGE(x)                 (((unsigned)(x) & 0x1) << 17)
#define   G_028D10_IGNORE_SC_ZRANGE(x)                 (((x) >> 17) & 0x1)
#define   C_028D10_IGNORE_SC_ZRANGE                    0xFFFDFFFF
#define   S_028D10_MAX_TILES_IN_DTT(x)                 (((unsigned)(x) & 0x1F) << 21)
#define   G_028D10_MAX_TILES_IN_DTT(x)                 (((x) >> 21) & 0x1F)
#define   C_028D10_MAX_TILES_IN_DTT                    0xFC1FFFFF
#define R_02880C_DB_SHADER_CONTROL                    0x02880C
#define   S_02880C_Z_EXPORT_ENABLE(x)                  (((unsigned)(x) & 0x1) << 0)
#define   G_02880C_Z_EXPORT_ENABLE(x)                  (((x) >> 0) & 0x1)
#define   C_02880C_Z_EXPORT_ENABLE                     0xFFFFFFFE
#define   S_02880C_STENCIL_REF_EXPORT_ENABLE(x)        (((unsigned)(x) & 0x1) << 1)
#define   G_02880C_STENCIL_REF_EXPORT_ENABLE(x)        (((x) >> 1) & 0x1)
#define   C_02880C_STENCIL_REF_EXPORT_ENABLE           0xFFFFFFFD
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
#define   C_02880C_MASK_EXPORT_ENABLE                  0xFFFFFEFF
#define   S_02880C_DUAL_EXPORT_ENABLE(x)               (((unsigned)(x) & 0x1) << 9)
#define   G_02880C_DUAL_EXPORT_ENABLE(x)               (((x) >> 9) & 0x1)
#define   C_02880C_DUAL_EXPORT_ENABLE                  0xFFFFFDFF
#define R_028DF8_PA_SU_POLY_OFFSET_DB_FMT_CNTL       0x028DF8
#define   S_028DF8_POLY_OFFSET_NEG_NUM_DB_BITS(x)      (((unsigned)(x) & 0xFF) << 0)
#define   G_028DF8_POLY_OFFSET_NEG_NUM_DB_BITS(x)      (((x) >> 0) & 0xFF)
#define   C_028DF8_POLY_OFFSET_NEG_NUM_DB_BITS         0xFFFFFF00
#define   S_028DF8_POLY_OFFSET_DB_IS_FLOAT_FMT(x)      (((unsigned)(x) & 0x1) << 8)
#define   G_028DF8_POLY_OFFSET_DB_IS_FLOAT_FMT(x)      (((x) >> 8) & 0x1)
#define   C_028DF8_POLY_OFFSET_DB_IS_FLOAT_FMT         0xFFFFFEFF
#define R_028E00_PA_SU_POLY_OFFSET_FRONT_SCALE       0x028E00
#define   S_028E00_SCALE(x)                            (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028E00_SCALE(x)                            (((x) >> 0) & 0xFFFFFFFF)
#define   C_028E00_SCALE                               0x00000000
#define R_028E04_PA_SU_POLY_OFFSET_FRONT_OFFSET      0x028E04
#define   S_028E04_OFFSET(x)                           (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028E04_OFFSET(x)                           (((x) >> 0) & 0xFFFFFFFF)
#define   C_028E04_OFFSET                              0x00000000
#define R_028E08_PA_SU_POLY_OFFSET_BACK_SCALE        0x028E08
#define   S_028E08_SCALE(x)                            (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028E08_SCALE(x)                            (((x) >> 0) & 0xFFFFFFFF)
#define   C_028E08_SCALE                               0x00000000
#define R_028E0C_PA_SU_POLY_OFFSET_BACK_OFFSET       0x028E0C
#define   S_028E0C_OFFSET(x)                           (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028E0C_OFFSET(x)                           (((x) >> 0) & 0xFFFFFFFF)
#define   C_028E0C_OFFSET                              0x00000000
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
#define   S_028A40_ES_PASSTHRU(x)                      (((unsigned)(x) & 0x1) << 2)
#define   G_028A40_ES_PASSTHRU(x)                      (((x) >> 2) & 0x1)
#define   C_028A40_ES_PASSTHRU                         0xFFFFFFFB
#define   S_028A40_CUT_MODE(x)                         (((unsigned)(x) & 0x3) << 3)
#define   G_028A40_CUT_MODE(x)                         (((x) >> 3) & 0x3)
#define   C_028A40_CUT_MODE                            0xFFFFFFE7
#define R_028A50_VGT_ENHANCE                         0x028A50
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
#define   S_0286CC_PARAM_GEN_ADDR(x)                   (((unsigned)(x) & 0x7F) << 19)
#define   G_0286CC_PARAM_GEN_ADDR(x)                   (((x) >> 19) & 0x7F)
#define   C_0286CC_PARAM_GEN_ADDR                      0xFC07FFFF
#define   S_0286CC_BARYC_SAMPLE_CNTL(x)                (((unsigned)(x) & 0x3) << 26)
#define   G_0286CC_BARYC_SAMPLE_CNTL(x)                (((x) >> 26) & 0x3)
#define   C_0286CC_BARYC_SAMPLE_CNTL                   0xF3FFFFFF
#define   S_0286CC_PERSP_GRADIENT_ENA(x)               (((unsigned)(x) & 0x1) << 28)
#define   G_0286CC_PERSP_GRADIENT_ENA(x)               (((x) >> 28) & 0x1)
#define   C_0286CC_PERSP_GRADIENT_ENA                  0xEFFFFFFF
#define   S_0286CC_LINEAR_GRADIENT_ENA(x)              (((unsigned)(x) & 0x1) << 29)
#define   G_0286CC_LINEAR_GRADIENT_ENA(x)              (((x) >> 29) & 0x1)
#define   C_0286CC_LINEAR_GRADIENT_ENA                 0xDFFFFFFF
#define   S_0286CC_POSITION_SAMPLE(x)                  (((unsigned)(x) & 0x1) << 30)
#define   G_0286CC_POSITION_SAMPLE(x)                  (((x) >> 30) & 0x1)
#define   C_0286CC_POSITION_SAMPLE                     0xBFFFFFFF
#define   S_0286CC_BARYC_AT_SAMPLE_ENA(x)              (((unsigned)(x) & 0x1) << 31)
#define   G_0286CC_BARYC_AT_SAMPLE_ENA(x)              (((x) >> 31) & 0x1)
#define   C_0286CC_BARYC_AT_SAMPLE_ENA                 0x7FFFFFFF
#define R_0286D0_SPI_PS_IN_CONTROL_1                 0x0286D0
#define   S_0286D0_GEN_INDEX_PIX(x)                    (((unsigned)(x) & 0x1) << 0)
#define   G_0286D0_GEN_INDEX_PIX(x)                    (((x) >> 0) & 0x1)
#define   C_0286D0_GEN_INDEX_PIX                       0xFFFFFFFE
#define   S_0286D0_GEN_INDEX_PIX_ADDR(x)               (((unsigned)(x) & 0x7F) << 1)
#define   G_0286D0_GEN_INDEX_PIX_ADDR(x)               (((x) >> 1) & 0x7F)
#define   C_0286D0_GEN_INDEX_PIX_ADDR                  0xFFFFFF01
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
#define R_028240_PA_SC_GENERIC_SCISSOR_TL            0x028240
#define   S_028240_TL_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028240_TL_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028240_TL_X                                0xFFFFC000
#define   S_028240_TL_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028240_TL_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028240_TL_Y                                0xC000FFFF
#define   S_028240_WINDOW_OFFSET_DISABLE(x)            (((unsigned)(x) & 0x1) << 31)
#define   G_028240_WINDOW_OFFSET_DISABLE(x)            (((x) >> 31) & 0x1)
#define   C_028240_WINDOW_OFFSET_DISABLE               0x7FFFFFFF
#define R_028244_PA_SC_GENERIC_SCISSOR_BR            0x028244
#define   S_028244_BR_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028244_BR_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028244_BR_X                                0xFFFFC000
#define   S_028244_BR_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028244_BR_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028244_BR_Y                                0xC000FFFF
#define R_028030_PA_SC_SCREEN_SCISSOR_TL             0x028030
#define   S_028030_TL_X(x)                             (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028030_TL_X(x)                             (((x) >> 0) & 0x7FFF)
#define   C_028030_TL_X                                0xFFFF8000
#define   S_028030_TL_Y(x)                             (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028030_TL_Y(x)                             (((x) >> 16) & 0x7FFF)
#define   C_028030_TL_Y                                0x8000FFFF
#define R_028034_PA_SC_SCREEN_SCISSOR_BR             0x028034
#define   S_028034_BR_X(x)                             (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028034_BR_X(x)                             (((x) >> 0) & 0x7FFF)
#define   C_028034_BR_X                                0xFFFF8000
#define   S_028034_BR_Y(x)                             (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028034_BR_Y(x)                             (((x) >> 16) & 0x7FFF)
#define   C_028034_BR_Y                                0x8000FFFF
#define R_028204_PA_SC_WINDOW_SCISSOR_TL             0x028204
#define   S_028204_TL_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028204_TL_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028204_TL_X                                0xFFFFC000
#define   S_028204_TL_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028204_TL_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028204_TL_Y                                0xC000FFFF
#define   S_028204_WINDOW_OFFSET_DISABLE(x)            (((unsigned)(x) & 0x1) << 31)
#define   G_028204_WINDOW_OFFSET_DISABLE(x)            (((x) >> 31) & 0x1)
#define   C_028204_WINDOW_OFFSET_DISABLE               0x7FFFFFFF
#define R_028208_PA_SC_WINDOW_SCISSOR_BR             0x028208
#define   S_028208_BR_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028208_BR_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028208_BR_X                                0xFFFFC000
#define   S_028208_BR_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028208_BR_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028208_BR_Y                                0xC000FFFF
#define R_0287F0_VGT_DRAW_INITIATOR                  0x0287F0
#define   S_0287F0_SOURCE_SELECT(x)                    (((unsigned)(x) & 0x3) << 0)
#define   G_0287F0_SOURCE_SELECT(x)                    (((x) >> 0) & 0x3)
#define   C_0287F0_SOURCE_SELECT                       0xFFFFFFFC
#define     V_0287F0_DI_SRC_SEL_DMA                    0
#define     V_0287F0_DI_SRC_SEL_IMMEDIATE              1
#define     V_0287F0_DI_SRC_SEL_AUTO_INDEX             2
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
#define R_038000_SQ_TEX_RESOURCE_WORD0_0             0x038000
#define   S_038000_DIM(x)                              (((unsigned)(x) & 0x7) << 0)
#define   G_038000_DIM(x)                              (((x) >> 0) & 0x7)
#define   C_038000_DIM                                 0xFFFFFFF8
#define     V_038000_SQ_TEX_DIM_1D                     0x00000000
#define     V_038000_SQ_TEX_DIM_2D                     0x00000001
#define     V_038000_SQ_TEX_DIM_3D                     0x00000002
#define     V_038000_SQ_TEX_DIM_CUBEMAP                0x00000003
#define     V_038000_SQ_TEX_DIM_1D_ARRAY               0x00000004
#define     V_038000_SQ_TEX_DIM_2D_ARRAY               0x00000005
#define     V_038000_SQ_TEX_DIM_2D_MSAA                0x00000006
#define     V_038000_SQ_TEX_DIM_2D_ARRAY_MSAA          0x00000007
#define   S_038000_TILE_MODE(x)                        (((unsigned)(x) & 0xF) << 3)
#define   G_038000_TILE_MODE(x)                        (((x) >> 3) & 0xF)
#define   C_038000_TILE_MODE                           0xFFFFFF87
#define     V_038000_ARRAY_LINEAR_GENERAL              0x00000000
#define     V_038000_ARRAY_LINEAR_ALIGNED              0x00000001
#define     V_038000_ARRAY_1D_TILED_THIN1              0x00000002
#define     V_038000_ARRAY_2D_TILED_THIN1              0x00000004
#define   S_038000_TILE_TYPE(x)                        (((unsigned)(x) & 0x1) << 7)
#define   G_038000_TILE_TYPE(x)                        (((x) >> 7) & 0x1)
#define   C_038000_TILE_TYPE                           0xFFFFFF7F
#define   S_038000_PITCH(x)                            (((unsigned)(x) & 0x7FF) << 8)
#define   G_038000_PITCH(x)                            (((x) >> 8) & 0x7FF)
#define   C_038000_PITCH                               0xFFF800FF
#define   S_038000_TEX_WIDTH(x)                        (((unsigned)(x) & 0x1FFF) << 19)
#define   G_038000_TEX_WIDTH(x)                        (((x) >> 19) & 0x1FFF)
#define   C_038000_TEX_WIDTH                           0x0007FFFF
#define R_038004_SQ_TEX_RESOURCE_WORD1_0             0x038004
#define   S_038004_TEX_HEIGHT(x)                       (((unsigned)(x) & 0x1FFF) << 0)
#define   G_038004_TEX_HEIGHT(x)                       (((x) >> 0) & 0x1FFF)
#define   C_038004_TEX_HEIGHT                          0xFFFFE000
#define   S_038004_TEX_DEPTH(x)                        (((unsigned)(x) & 0x1FFF) << 13)
#define   G_038004_TEX_DEPTH(x)                        (((x) >> 13) & 0x1FFF)
#define   C_038004_TEX_DEPTH                           0xFC001FFF
#define   S_038004_DATA_FORMAT(x)                      (((unsigned)(x) & 0x3F) << 26)
#define   G_038004_DATA_FORMAT(x)                      (((x) >> 26) & 0x3F)
#define   C_038004_DATA_FORMAT                         0x03FFFFFF
#define R_038008_SQ_TEX_RESOURCE_WORD2_0             0x038008
#define   S_038008_BASE_ADDRESS(x)                     (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_038008_BASE_ADDRESS(x)                     (((x) >> 0) & 0xFFFFFFFF)
#define   C_038008_BASE_ADDRESS                        0x00000000
#define R_03800C_SQ_TEX_RESOURCE_WORD3_0             0x03800C
#define   S_03800C_MIP_ADDRESS(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_03800C_MIP_ADDRESS(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_03800C_MIP_ADDRESS                         0x00000000
#define R_038010_SQ_TEX_RESOURCE_WORD4_0             0x038010
#define   S_038010_FORMAT_COMP_X(x)                    (((unsigned)(x) & 0x3) << 0)
#define   G_038010_FORMAT_COMP_X(x)                    (((x) >> 0) & 0x3)
#define   C_038010_FORMAT_COMP_X                       0xFFFFFFFC
#define     V_038010_SQ_FORMAT_COMP_UNSIGNED           0x00000000
#define     V_038010_SQ_FORMAT_COMP_SIGNED             0x00000001
#define     V_038010_SQ_FORMAT_COMP_UNSIGNED_BIASED    0x00000002
#define   S_038010_FORMAT_COMP_Y(x)                    (((unsigned)(x) & 0x3) << 2)
#define   G_038010_FORMAT_COMP_Y(x)                    (((x) >> 2) & 0x3)
#define   C_038010_FORMAT_COMP_Y                       0xFFFFFFF3
#define   S_038010_FORMAT_COMP_Z(x)                    (((unsigned)(x) & 0x3) << 4)
#define   G_038010_FORMAT_COMP_Z(x)                    (((x) >> 4) & 0x3)
#define   C_038010_FORMAT_COMP_Z                       0xFFFFFFCF
#define   S_038010_FORMAT_COMP_W(x)                    (((unsigned)(x) & 0x3) << 6)
#define   G_038010_FORMAT_COMP_W(x)                    (((x) >> 6) & 0x3)
#define   C_038010_FORMAT_COMP_W                       0xFFFFFF3F
#define   S_038010_NUM_FORMAT_ALL(x)                   (((unsigned)(x) & 0x3) << 8)
#define   G_038010_NUM_FORMAT_ALL(x)                   (((x) >> 8) & 0x3)
#define   C_038010_NUM_FORMAT_ALL                      0xFFFFFCFF
#define     V_038010_SQ_NUM_FORMAT_NORM                0x00000000
#define     V_038010_SQ_NUM_FORMAT_INT                 0x00000001
#define     V_038010_SQ_NUM_FORMAT_SCALED              0x00000002
#define   S_038010_SRF_MODE_ALL(x)                     (((unsigned)(x) & 0x1) << 10)
#define   G_038010_SRF_MODE_ALL(x)                     (((x) >> 10) & 0x1)
#define   C_038010_SRF_MODE_ALL                        0xFFFFFBFF
#define     V_038010_SRF_MODE_ZERO_CLAMP_MINUS_ONE     0x00000000
#define     V_038010_SRF_MODE_NO_ZERO                  0x00000001
#define   S_038010_FORCE_DEGAMMA(x)                    (((unsigned)(x) & 0x1) << 11)
#define   G_038010_FORCE_DEGAMMA(x)                    (((x) >> 11) & 0x1)
#define   C_038010_FORCE_DEGAMMA                       0xFFFFF7FF
#define   S_038010_ENDIAN_SWAP(x)                      (((unsigned)(x) & 0x3) << 12)
#define   G_038010_ENDIAN_SWAP(x)                      (((x) >> 12) & 0x3)
#define   C_038010_ENDIAN_SWAP                         0xFFFFCFFF
#define   S_038010_REQUEST_SIZE(x)                     (((unsigned)(x) & 0x3) << 14)
#define   G_038010_REQUEST_SIZE(x)                     (((x) >> 14) & 0x3)
#define   C_038010_REQUEST_SIZE                        0xFFFF3FFF
#define   S_038010_DST_SEL_X(x)                        (((unsigned)(x) & 0x7) << 16)
#define   G_038010_DST_SEL_X(x)                        (((x) >> 16) & 0x7)
#define   C_038010_DST_SEL_X                           0xFFF8FFFF
#define     V_038010_SQ_SEL_X                          0x00000000
#define     V_038010_SQ_SEL_Y                          0x00000001
#define     V_038010_SQ_SEL_Z                          0x00000002
#define     V_038010_SQ_SEL_W                          0x00000003
#define     V_038010_SQ_SEL_0                          0x00000004
#define     V_038010_SQ_SEL_1                          0x00000005
#define   S_038010_DST_SEL_Y(x)                        (((unsigned)(x) & 0x7) << 19)
#define   G_038010_DST_SEL_Y(x)                        (((x) >> 19) & 0x7)
#define   C_038010_DST_SEL_Y                           0xFFC7FFFF
#define   S_038010_DST_SEL_Z(x)                        (((unsigned)(x) & 0x7) << 22)
#define   G_038010_DST_SEL_Z(x)                        (((x) >> 22) & 0x7)
#define   C_038010_DST_SEL_Z                           0xFE3FFFFF
#define   S_038010_DST_SEL_W(x)                        (((unsigned)(x) & 0x7) << 25)
#define   G_038010_DST_SEL_W(x)                        (((x) >> 25) & 0x7)
#define   C_038010_DST_SEL_W                           0xF1FFFFFF
#define   S_038010_BASE_LEVEL(x)                       (((unsigned)(x) & 0xF) << 28)
#define   G_038010_BASE_LEVEL(x)                       (((x) >> 28) & 0xF)
#define   C_038010_BASE_LEVEL                          0x0FFFFFFF
#define R_038014_SQ_TEX_RESOURCE_WORD5_0             0x038014
#define   S_038014_LAST_LEVEL(x)                       (((unsigned)(x) & 0xF) << 0)
#define   G_038014_LAST_LEVEL(x)                       (((x) >> 0) & 0xF)
#define   C_038014_LAST_LEVEL                          0xFFFFFFF0
#define   S_038014_BASE_ARRAY(x)                       (((unsigned)(x) & 0x1FFF) << 4)
#define   G_038014_BASE_ARRAY(x)                       (((x) >> 4) & 0x1FFF)
#define   C_038014_BASE_ARRAY                          0xFFFE000F
#define   S_038014_LAST_ARRAY(x)                       (((unsigned)(x) & 0x1FFF) << 17)
#define   G_038014_LAST_ARRAY(x)                       (((x) >> 17) & 0x1FFF)
#define   C_038014_LAST_ARRAY                          0xC001FFFF
#define R_038018_SQ_TEX_RESOURCE_WORD6_0             0x038018
#define   S_038018_MPEG_CLAMP(x)                       (((unsigned)(x) & 0x3) << 0)
#define   G_038018_MPEG_CLAMP(x)                       (((x) >> 0) & 0x3)
#define   C_038018_MPEG_CLAMP                          0xFFFFFFFC
#define   S_038018_MAX_ANISO(x)                        (((unsigned)(x) & 0x7) << 2)
#define   G_038018_MAX_ANISO(x)                        (((x) >> 2) & 0x7)
#define   C_038018_MAX_ANISO                           0xFFFFFFE3
#define   S_038018_PERF_MODULATION(x)                  (((unsigned)(x) & 0x7) << 5)
#define   G_038018_PERF_MODULATION(x)                  (((x) >> 5) & 0x7)
#define   C_038018_PERF_MODULATION                     0xFFFFFF1F
#define   S_038018_INTERLACED(x)                       (((unsigned)(x) & 0x1) << 8)
#define   G_038018_INTERLACED(x)                       (((x) >> 8) & 0x1)
#define   C_038018_INTERLACED                          0xFFFFFEFF
#define   S_038018_TYPE(x)                             (((unsigned)(x) & 0x3) << 30)
#define   G_038018_TYPE(x)                             (((x) >> 30) & 0x3)
#define   C_038018_TYPE                                0x3FFFFFFF
#define     V_038010_SQ_TEX_VTX_INVALID_TEXTURE        0x00000000
#define     V_038010_SQ_TEX_VTX_INVALID_BUFFER         0x00000001
#define     V_038010_SQ_TEX_VTX_VALID_TEXTURE          0x00000002
#define     V_038010_SQ_TEX_VTX_VALID_BUFFER           0x00000003
#define R_038008_SQ_VTX_CONSTANT_WORD2_0             0x038008
#define   S_038008_BASE_ADDRESS_HI(x)                  (((unsigned)(x) & 0xFF) << 0)
#define   G_038008_BASE_ADDRESS_HI(x)                  (((x) >> 0) & 0xFF)
#define   C_038008_BASE_ADDRESS_HI                     0xFFFFFF00
#define   S_038008_STRIDE(x)                           (((unsigned)(x) & 0x7FF) << 8)
#define   G_038008_STRIDE(x)                           (((x) >> 8) & 0x7FF)
#define   C_038008_STRIDE                              0xFFF800FF
#define   S_038008_CLAMP_X(x)                          (((unsigned)(x) & 0x1) << 19)
#define   G_038008_CLAMP_X(x)                          (((x) >> 19) & 0x1)
#define   C_038008_CLAMP_X                             0xFFF7FFFF
#define   S_038008_DATA_FORMAT(x)                      (((unsigned)(x) & 0x3F) << 20)
#define   G_038008_DATA_FORMAT(x)                      (((x) >> 20) & 0x3F)
#define   C_038008_DATA_FORMAT                         0xFC0FFFFF

#define   S_038008_NUM_FORMAT_ALL(x)                   (((unsigned)(x) & 0x3) << 26)
#define   G_038008_NUM_FORMAT_ALL(x)                   (((x) >> 26) & 0x3)
#define   C_038008_NUM_FORMAT_ALL                      0xF3FFFFFF
#define     V_038008_SQ_NUM_FORMAT_NORM                0x00000000
#define     V_038008_SQ_NUM_FORMAT_INT                 0x00000001
#define     V_038008_SQ_NUM_FORMAT_SCALED              0x00000002
#define   S_038008_FORMAT_COMP_ALL(x)                  (((unsigned)(x) & 0x1) << 28)
#define   G_038008_FORMAT_COMP_ALL(x)                  (((x) >> 28) & 0x1)
#define   C_038008_FORMAT_COMP_ALL                     0xEFFFFFFF
#define   S_038008_SRF_MODE_ALL(x)                     (((unsigned)(x) & 0x1) << 29)
#define   G_038008_SRF_MODE_ALL(x)                     (((x) >> 29) & 0x1)
#define   C_038008_SRF_MODE_ALL                        0xDFFFFFFF
#define   S_038008_ENDIAN_SWAP(x)                      (((unsigned)(x) & 0x3) << 30)
#define   G_038008_ENDIAN_SWAP(x)                      (((x) >> 30) & 0x3)
#define   C_038008_ENDIAN_SWAP                         0x3FFFFFFF
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
#define   S_03C000_XY_MAG_FILTER(x)                    (((unsigned)(x) & 0x7) << 9)
#define   G_03C000_XY_MAG_FILTER(x)                    (((x) >> 9) & 0x7)
#define   C_03C000_XY_MAG_FILTER                       0xFFFFF1FF
#define     V_03C000_SQ_TEX_XY_FILTER_POINT            0x00000000
#define     V_03C000_SQ_TEX_XY_FILTER_BILINEAR         0x00000001
#define     V_03C000_SQ_TEX_XY_FILTER_BICUBIC          0x00000002
#define     V_03C000_SQ_TEX_XY_FILTER_ANISO_POINT      0x00000004
#define     V_03C000_SQ_TEX_XY_FILTER_ANISO_BILINEAR   0x00000005
#define   S_03C000_XY_MIN_FILTER(x)                    (((unsigned)(x) & 0x7) << 12)
#define   G_03C000_XY_MIN_FILTER(x)                    (((x) >> 12) & 0x7)
#define   C_03C000_XY_MIN_FILTER                       0xFFFF8FFF
#define   S_03C000_Z_FILTER(x)                         (((unsigned)(x) & 0x3) << 15)
#define   G_03C000_Z_FILTER(x)                         (((x) >> 15) & 0x3)
#define   C_03C000_Z_FILTER                            0xFFFE7FFF
#define     V_03C000_SQ_TEX_Z_FILTER_NONE              0x00000000
#define     V_03C000_SQ_TEX_Z_FILTER_POINT             0x00000001
#define     V_03C000_SQ_TEX_Z_FILTER_LINEAR            0x00000002
#define   S_03C000_MIP_FILTER(x)                       (((unsigned)(x) & 0x3) << 17)
#define   G_03C000_MIP_FILTER(x)                       (((x) >> 17) & 0x3)
#define   C_03C000_MIP_FILTER                          0xFFF9FFFF
#define   S_03C000_MAX_ANISO_RATIO(x)                  (((unsigned)(x) & 0x7) << 19)
#define   G_03C000_MAX_ANISO_RATIO(x)                  (((x) >> 19) & 0x7)
#define   C_03C000_MAX_ANISO_RATIO                     0xFFB7FFFF
#define   S_03C000_BORDER_COLOR_TYPE(x)                (((unsigned)(x) & 0x3) << 22)
#define   G_03C000_BORDER_COLOR_TYPE(x)                (((x) >> 22) & 0x3)
#define   C_03C000_BORDER_COLOR_TYPE                   0xFF3FFFFF
#define     V_03C000_SQ_TEX_BORDER_COLOR_TRANS_BLACK   0x00000000
#define     V_03C000_SQ_TEX_BORDER_COLOR_OPAQUE_BLACK  0x00000001
#define     V_03C000_SQ_TEX_BORDER_COLOR_OPAQUE_WHITE  0x00000002
#define     V_03C000_SQ_TEX_BORDER_COLOR_REGISTER      0x00000003
#define   S_03C000_POINT_SAMPLING_CLAMP(x)             (((unsigned)(x) & 0x1) << 24)
#define   G_03C000_POINT_SAMPLING_CLAMP(x)             (((x) >> 24) & 0x1)
#define   C_03C000_POINT_SAMPLING_CLAMP                0xFEFFFFFF
#define   S_03C000_TEX_ARRAY_OVERRIDE(x)               (((unsigned)(x) & 0x1) << 25)
#define   G_03C000_TEX_ARRAY_OVERRIDE(x)               (((x) >> 25) & 0x1)
#define   C_03C000_TEX_ARRAY_OVERRIDE                  0xFDFFFFFF
#define   S_03C000_DEPTH_COMPARE_FUNCTION(x)           (((unsigned)(x) & 0x7) << 26)
#define   G_03C000_DEPTH_COMPARE_FUNCTION(x)           (((x) >> 26) & 0x7)
#define   C_03C000_DEPTH_COMPARE_FUNCTION              0xE3FFFFFF
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_NEVER        0x00000000
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_LESS         0x00000001
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_EQUAL        0x00000002
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_LESSEQUAL    0x00000003
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_GREATER      0x00000004
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_NOTEQUAL     0x00000005
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_GREATEREQUAL 0x00000006
#define     V_03C000_SQ_TEX_DEPTH_COMPARE_ALWAYS       0x00000007
#define   S_03C000_CHROMA_KEY(x)                       (((unsigned)(x) & 0x3) << 29)
#define   G_03C000_CHROMA_KEY(x)                       (((x) >> 29) & 0x3)
#define   C_03C000_CHROMA_KEY                          0x9FFFFFFF
#define     V_03C000_SQ_TEX_CHROMA_KEY_DISABLE         0x00000000
#define     V_03C000_SQ_TEX_CHROMA_KEY_KILL            0x00000001
#define     V_03C000_SQ_TEX_CHROMA_KEY_BLEND           0x00000002
#define   S_03C000_LOD_USES_MINOR_AXIS(x)              (((unsigned)(x) & 0x1) << 31)
#define   G_03C000_LOD_USES_MINOR_AXIS(x)              (((x) >> 31) & 0x1)
#define   C_03C000_LOD_USES_MINOR_AXIS                 0x7FFFFFFF
#define R_03C004_SQ_TEX_SAMPLER_WORD1_0              0x03C004
#define   S_03C004_MIN_LOD(x)                          (((unsigned)(x) & 0x3FF) << 0)
#define   G_03C004_MIN_LOD(x)                          (((x) >> 0) & 0x3FF)
#define   C_03C004_MIN_LOD                             0xFFFFFC00
#define   S_03C004_MAX_LOD(x)                          (((unsigned)(x) & 0x3FF) << 10)
#define   G_03C004_MAX_LOD(x)                          (((x) >> 10) & 0x3FF)
#define   C_03C004_MAX_LOD                             0xFFF003FF
#define   S_03C004_LOD_BIAS(x)                         (((unsigned)(x) & 0xFFF) << 20)
#define   G_03C004_LOD_BIAS(x)                         (((x) >> 20) & 0xFFF)
#define   C_03C004_LOD_BIAS                            0x000FFFFF
#define R_03C008_SQ_TEX_SAMPLER_WORD2_0              0x03C008
#define   S_03C008_LOD_BIAS_SEC(x)                     (((unsigned)(x) & 0xFFF) << 0)
#define   G_03C008_LOD_BIAS_SEC(x)                     (((x) >> 0) & 0xFFF)
#define   C_03C008_LOD_BIAS_SEC                        0xFFFFF000
#define   S_03C008_MC_COORD_TRUNCATE(x)                (((unsigned)(x) & 0x1) << 12)
#define   G_03C008_MC_COORD_TRUNCATE(x)                (((x) >> 12) & 0x1)
#define   C_03C008_MC_COORD_TRUNCATE                   0xFFFFEFFF
#define   S_03C008_FORCE_DEGAMMA(x)                    (((unsigned)(x) & 0x1) << 13)
#define   G_03C008_FORCE_DEGAMMA(x)                    (((x) >> 13) & 0x1)
#define   C_03C008_FORCE_DEGAMMA                       0xFFFFDFFF
#define   S_03C008_HIGH_PRECISION_FILTER(x)            (((unsigned)(x) & 0x1) << 14)
#define   G_03C008_HIGH_PRECISION_FILTER(x)            (((x) >> 14) & 0x1)
#define   C_03C008_HIGH_PRECISION_FILTER               0xFFFFBFFF
#define   S_03C008_PERF_MIP(x)                         (((unsigned)(x) & 0x7) << 15)
#define   G_03C008_PERF_MIP(x)                         (((x) >> 15) & 0x7)
#define   C_03C008_PERF_MIP                            0xFFFC7FFF
#define   S_03C008_PERF_Z(x)                           (((unsigned)(x) & 0x3) << 18)
#define   G_03C008_PERF_Z(x)                           (((x) >> 18) & 0x3)
#define   C_03C008_PERF_Z                              0xFFF3FFFF
#define   S_03C008_ANISO_BIAS(x)                       (((unsigned)(x) & 0x3f) << 22)
#define   G_03C008_ANISO_BIAS(x)                       (((x) >> 22) & 0x3f)
#define   C_03C008_ANISO_BIAS                          (~(0x3f << 22))
#define   S_03C008_FETCH_4(x)                          (((unsigned)(x) & 0x1) << 26)
#define   G_03C008_FETCH_4(x)                          (((x) >> 26) & 0x1)
#define   C_03C008_FETCH_4                             0xFBFFFFFF
#define   S_03C008_SAMPLE_IS_PCF(x)                    (((unsigned)(x) & 0x1) << 27)
#define   G_03C008_SAMPLE_IS_PCF(x)                    (((x) >> 27) & 0x1)
#define   C_03C008_SAMPLE_IS_PCF                       0xF7FFFFFF
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
#define R_028868_SQ_PGM_RESOURCES_VS                 0x028868
#define   S_028868_NUM_GPRS(x)                         (((unsigned)(x) & 0xFF) << 0)
#define   G_028868_NUM_GPRS(x)                         (((x) >> 0) & 0xFF)
#define   C_028868_NUM_GPRS                            0xFFFFFF00
#define   S_028868_STACK_SIZE(x)                       (((unsigned)(x) & 0xFF) << 8)
#define   G_028868_STACK_SIZE(x)                       (((x) >> 8) & 0xFF)
#define   C_028868_STACK_SIZE                          0xFFFF00FF
#define   S_028868_DX10_CLAMP(x)                       (((unsigned)(x) & 0x1) << 21)
#define   G_028868_DX10_CLAMP(x)                       (((x) >> 21) & 0x1)
#define   C_028868_DX10_CLAMP                          0xFFDFFFFF
#define   S_028868_FETCH_CACHE_LINES(x)                (((unsigned)(x) & 0x7) << 24)
#define   G_028868_FETCH_CACHE_LINES(x)                (((x) >> 24) & 0x7)
#define   C_028868_FETCH_CACHE_LINES                   0xF8FFFFFF
#define   S_028868_UNCACHED_FIRST_INST(x)              (((unsigned)(x) & 0x1) << 28)
#define   G_028868_UNCACHED_FIRST_INST(x)              (((x) >> 28) & 0x1)
#define   C_028868_UNCACHED_FIRST_INST                 0xEFFFFFFF
#define R_028850_SQ_PGM_RESOURCES_PS                 0x028850
#define   S_028850_NUM_GPRS(x)                         (((unsigned)(x) & 0xFF) << 0)
#define   G_028850_NUM_GPRS(x)                         (((x) >> 0) & 0xFF)
#define   C_028850_NUM_GPRS                            0xFFFFFF00
#define   S_028850_STACK_SIZE(x)                       (((unsigned)(x) & 0xFF) << 8)
#define   G_028850_STACK_SIZE(x)                       (((x) >> 8) & 0xFF)
#define   C_028850_STACK_SIZE                          0xFFFF00FF
#define   S_028850_DX10_CLAMP(x)                       (((unsigned)(x) & 0x1) << 21)
#define   G_028850_DX10_CLAMP(x)                       (((x) >> 21) & 0x1)
#define   C_028850_DX10_CLAMP                          0xFFDFFFFF
#define   S_028850_FETCH_CACHE_LINES(x)                (((unsigned)(x) & 0x7) << 24)
#define   G_028850_FETCH_CACHE_LINES(x)                (((x) >> 24) & 0x7)
#define   C_028850_FETCH_CACHE_LINES                   0xF8FFFFFF
#define   S_028850_UNCACHED_FIRST_INST(x)              (((unsigned)(x) & 0x1) << 28)
#define   G_028850_UNCACHED_FIRST_INST(x)              (((x) >> 28) & 0x1)
#define   C_028850_UNCACHED_FIRST_INST                 0xEFFFFFFF
#define   S_028850_CLAMP_CONSTS(x)                     (((unsigned)(x) & 0x1) << 31)
#define   G_028850_CLAMP_CONSTS(x)                     (((x) >> 31) & 0x1)
#define   C_028850_CLAMP_CONSTS                        0x7FFFFFFF
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
#define R_028084_CB_COLOR1_VIEW                      0x028084
#define R_028088_CB_COLOR2_VIEW                      0x028088
#define R_02808C_CB_COLOR3_VIEW                      0x02808C
#define R_028090_CB_COLOR4_VIEW                      0x028090
#define R_028094_CB_COLOR5_VIEW                      0x028094
#define R_028098_CB_COLOR6_VIEW                      0x028098
#define R_02809C_CB_COLOR7_VIEW                      0x02809C
#define R_028104_CB_COLOR1_MASK                      0x028104
#define R_028108_CB_COLOR2_MASK                      0x028108
#define R_02810C_CB_COLOR3_MASK                      0x02810C
#define R_028110_CB_COLOR4_MASK                      0x028110
#define R_028114_CB_COLOR5_MASK                      0x028114
#define R_028118_CB_COLOR6_MASK                      0x028118
#define R_02811C_CB_COLOR7_MASK                      0x02811C
#define R_0280E4_CB_COLOR1_FRAG                      0x0280E4
#define R_0280E8_CB_COLOR2_FRAG                      0x0280E8
#define R_0280EC_CB_COLOR3_FRAG                      0x0280EC
#define R_0280F0_CB_COLOR4_FRAG                      0x0280F0
#define R_0280F4_CB_COLOR5_FRAG                      0x0280F4
#define R_0280F8_CB_COLOR6_FRAG                      0x0280F8
#define R_0280FC_CB_COLOR7_FRAG                      0x0280FC
#define R_0280C4_CB_COLOR1_TILE                      0x0280C4
#define R_0280C8_CB_COLOR2_TILE                      0x0280C8
#define R_0280CC_CB_COLOR3_TILE                      0x0280CC
#define R_0280D0_CB_COLOR4_TILE                      0x0280D0
#define R_0280D4_CB_COLOR5_TILE                      0x0280D4
#define R_0280D8_CB_COLOR6_TILE                      0x0280D8
#define R_0280DC_CB_COLOR7_TILE                      0x0280DC
#define R_0280A4_CB_COLOR1_INFO                      0x0280A4
#define R_0280A8_CB_COLOR2_INFO                      0x0280A8
#define R_0280AC_CB_COLOR3_INFO                      0x0280AC
#define R_0280B0_CB_COLOR4_INFO                      0x0280B0
#define R_0280B4_CB_COLOR5_INFO                      0x0280B4
#define R_0280B8_CB_COLOR6_INFO                      0x0280B8
#define R_0280BC_CB_COLOR7_INFO                      0x0280BC
#define R_028C1C_PA_SC_AA_SAMPLE_LOCS_MCTX           0x028C1C
#define R_028C20_PA_SC_AA_SAMPLE_LOCS_8D_WD1_MCTX    0x028C20
#define R_028C30_CB_CLRCMP_CONTROL                   0x028C30
#define   S_028C30_CLRCMP_FCN_SRC(x)                   (((unsigned)(x) & 0x7) << 0)
#define   G_028C30_CLRCMP_FCN_SRC(x)                   (((x) >> 0) & 0x7)
#define   C_028C30_CLRCMP_FCN_SRC                      0xFFFFFFF8
#define   S_028C30_CLRCMP_FCN_DST(x)                   (((unsigned)(x) & 0x7) << 8)
#define   G_028C30_CLRCMP_FCN_DST(x)                   (((x) >> 8) & 0x7)
#define   C_028C30_CLRCMP_FCN_DST                      0xFFFFF8FF
#define   S_028C30_CLRCMP_FCN_SEL(x)                   (((unsigned)(x) & 0x3) << 24)
#define   G_028C30_CLRCMP_FCN_SEL(x)                   (((x) >> 24) & 0x3)
#define   C_028C30_CLRCMP_FCN_SEL                      0xFCFFFFFF
#define R_028C20_PA_SC_AA_SAMPLE_LOCS_8S_WD1_MCTX    0x028C20
#define   S_028C20_S4_X(x)                             (((unsigned)(x) & 0xF) << 0)
#define   G_028C20_S4_X(x)                             (((x) >> 0) & 0xF)
#define   C_028C20_S4_X                                0xFFFFFFF0
#define   S_028C20_S4_Y(x)                             (((unsigned)(x) & 0xF) << 4)
#define   G_028C20_S4_Y(x)                             (((x) >> 4) & 0xF)
#define   C_028C20_S4_Y                                0xFFFFFF0F
#define   S_028C20_S5_X(x)                             (((unsigned)(x) & 0xF) << 8)
#define   G_028C20_S5_X(x)                             (((x) >> 8) & 0xF)
#define   C_028C20_S5_X                                0xFFFFF0FF
#define   S_028C20_S5_Y(x)                             (((unsigned)(x) & 0xF) << 12)
#define   G_028C20_S5_Y(x)                             (((x) >> 12) & 0xF)
#define   C_028C20_S5_Y                                0xFFFF0FFF
#define   S_028C20_S6_X(x)                             (((unsigned)(x) & 0xF) << 16)
#define   G_028C20_S6_X(x)                             (((x) >> 16) & 0xF)
#define   C_028C20_S6_X                                0xFFF0FFFF
#define   S_028C20_S6_Y(x)                             (((unsigned)(x) & 0xF) << 20)
#define   G_028C20_S6_Y(x)                             (((x) >> 20) & 0xF)
#define   C_028C20_S6_Y                                0xFF0FFFFF
#define   S_028C20_S7_X(x)                             (((unsigned)(x) & 0xF) << 24)
#define   G_028C20_S7_X(x)                             (((x) >> 24) & 0xF)
#define   C_028C20_S7_X                                0xF0FFFFFF
#define   S_028C20_S7_Y(x)                             (((unsigned)(x) & 0xF) << 28)
#define   G_028C20_S7_Y(x)                             (((x) >> 28) & 0xF)
#define   C_028C20_S7_Y                                0x0FFFFFFF
#define R_0280A0_CB_COLOR0_INFO                      0x0280A0
#define   S_0280A0_ENDIAN(x)                           (((unsigned)(x) & 0x3) << 0)
#define   G_0280A0_ENDIAN(x)                           (((x) >> 0) & 0x3)
#define   C_0280A0_ENDIAN                              0xFFFFFFFC
#define   S_0280A0_FORMAT(x)                           (((unsigned)(x) & 0x3F) << 2)
#define   G_0280A0_FORMAT(x)                           (((x) >> 2) & 0x3F)
#define   C_0280A0_FORMAT                              0xFFFFFF03
#define     V_0280A0_COLOR_INVALID                     0x00000000
#define     V_0280A0_COLOR_8                           0x00000001
#define     V_0280A0_COLOR_4_4                         0x00000002
#define     V_0280A0_COLOR_3_3_2                       0x00000003
#define     V_0280A0_COLOR_16                          0x00000005
#define     V_0280A0_COLOR_16_FLOAT                    0x00000006
#define     V_0280A0_COLOR_8_8                         0x00000007
#define     V_0280A0_COLOR_5_6_5                       0x00000008
#define     V_0280A0_COLOR_6_5_5                       0x00000009
#define     V_0280A0_COLOR_1_5_5_5                     0x0000000A
#define     V_0280A0_COLOR_4_4_4_4                     0x0000000B
#define     V_0280A0_COLOR_5_5_5_1                     0x0000000C
#define     V_0280A0_COLOR_32                          0x0000000D
#define     V_0280A0_COLOR_32_FLOAT                    0x0000000E
#define     V_0280A0_COLOR_16_16                       0x0000000F
#define     V_0280A0_COLOR_16_16_FLOAT                 0x00000010
#define     V_0280A0_COLOR_8_24                        0x00000011
#define     V_0280A0_COLOR_8_24_FLOAT                  0x00000012
#define     V_0280A0_COLOR_24_8                        0x00000013
#define     V_0280A0_COLOR_24_8_FLOAT                  0x00000014
#define     V_0280A0_COLOR_10_11_11                    0x00000015
#define     V_0280A0_COLOR_10_11_11_FLOAT              0x00000016
#define     V_0280A0_COLOR_11_11_10                    0x00000017
#define     V_0280A0_COLOR_11_11_10_FLOAT              0x00000018
#define     V_0280A0_COLOR_2_10_10_10                  0x00000019
#define     V_0280A0_COLOR_8_8_8_8                     0x0000001A
#define     V_0280A0_COLOR_10_10_10_2                  0x0000001B
#define     V_0280A0_COLOR_X24_8_32_FLOAT              0x0000001C
#define     V_0280A0_COLOR_32_32                       0x0000001D
#define     V_0280A0_COLOR_32_32_FLOAT                 0x0000001E
#define     V_0280A0_COLOR_16_16_16_16                 0x0000001F
#define     V_0280A0_COLOR_16_16_16_16_FLOAT           0x00000020
#define     V_0280A0_COLOR_32_32_32_32                 0x00000022
#define     V_0280A0_COLOR_32_32_32_32_FLOAT           0x00000023
#define   S_0280A0_ARRAY_MODE(x)                       (((unsigned)(x) & 0xF) << 8)
#define   G_0280A0_ARRAY_MODE(x)                       (((x) >> 8) & 0xF)
#define   C_0280A0_ARRAY_MODE                          0xFFFFF0FF
#define     V_0280A0_ARRAY_LINEAR_GENERAL              0x00000000
#define     V_0280A0_ARRAY_LINEAR_ALIGNED              0x00000001
#define     V_0280A0_ARRAY_1D_TILED_THIN1              0x00000002
#define     V_0280A0_ARRAY_2D_TILED_THIN1              0x00000004
#define   S_0280A0_NUMBER_TYPE(x)                      (((unsigned)(x) & 0x7) << 12)
#define   G_0280A0_NUMBER_TYPE(x)                      (((x) >> 12) & 0x7)
#define   C_0280A0_NUMBER_TYPE                         0xFFFF8FFF
#define   S_0280A0_READ_SIZE(x)                        (((unsigned)(x) & 0x1) << 15)
#define   G_0280A0_READ_SIZE(x)                        (((x) >> 15) & 0x1)
#define   C_0280A0_READ_SIZE                           0xFFFF7FFF
#define   S_0280A0_COMP_SWAP(x)                        (((unsigned)(x) & 0x3) << 16)
#define   G_0280A0_COMP_SWAP(x)                        (((x) >> 16) & 0x3)
#define   C_0280A0_COMP_SWAP                           0xFFFCFFFF
#define   S_0280A0_TILE_MODE(x)                        (((unsigned)(x) & 0x3) << 18)
#define   G_0280A0_TILE_MODE(x)                        (((x) >> 18) & 0x3)
#define   C_0280A0_TILE_MODE                           0xFFF3FFFF
#define   S_0280A0_BLEND_CLAMP(x)                      (((unsigned)(x) & 0x1) << 20)
#define   G_0280A0_BLEND_CLAMP(x)                      (((x) >> 20) & 0x1)
#define   C_0280A0_BLEND_CLAMP                         0xFFEFFFFF
#define   S_0280A0_CLEAR_COLOR(x)                      (((unsigned)(x) & 0x1) << 21)
#define   G_0280A0_CLEAR_COLOR(x)                      (((x) >> 21) & 0x1)
#define   C_0280A0_CLEAR_COLOR                         0xFFDFFFFF
#define   S_0280A0_BLEND_BYPASS(x)                     (((unsigned)(x) & 0x1) << 22)
#define   G_0280A0_BLEND_BYPASS(x)                     (((x) >> 22) & 0x1)
#define   C_0280A0_BLEND_BYPASS                        0xFFBFFFFF
#define   S_0280A0_BLEND_FLOAT32(x)                    (((unsigned)(x) & 0x1) << 23)
#define   G_0280A0_BLEND_FLOAT32(x)                    (((x) >> 23) & 0x1)
#define   C_0280A0_BLEND_FLOAT32                       0xFF7FFFFF
#define   S_0280A0_SIMPLE_FLOAT(x)                     (((unsigned)(x) & 0x1) << 24)
#define   G_0280A0_SIMPLE_FLOAT(x)                     (((x) >> 24) & 0x1)
#define   C_0280A0_SIMPLE_FLOAT                        0xFEFFFFFF
#define   S_0280A0_ROUND_MODE(x)                       (((unsigned)(x) & 0x1) << 25)
#define   G_0280A0_ROUND_MODE(x)                       (((x) >> 25) & 0x1)
#define   C_0280A0_ROUND_MODE                          0xFDFFFFFF
#define   S_0280A0_TILE_COMPACT(x)                     (((unsigned)(x) & 0x1) << 26)
#define   G_0280A0_TILE_COMPACT(x)                     (((x) >> 26) & 0x1)
#define   C_0280A0_TILE_COMPACT                        0xFBFFFFFF
#define   S_0280A0_SOURCE_FORMAT(x)                    (((unsigned)(x) & 0x1) << 27)
#define   G_0280A0_SOURCE_FORMAT(x)                    (((x) >> 27) & 0x1)
#define   C_0280A0_SOURCE_FORMAT                       0xF7FFFFFF
#define R_028060_CB_COLOR0_SIZE                      0x028060
#define   S_028060_PITCH_TILE_MAX(x)                   (((unsigned)(x) & 0x3FF) << 0)
#define   G_028060_PITCH_TILE_MAX(x)                   (((x) >> 0) & 0x3FF)
#define   C_028060_PITCH_TILE_MAX                      0xFFFFFC00
#define   S_028060_SLICE_TILE_MAX(x)                   (((unsigned)(x) & 0xFFFFF) << 10)
#define   G_028060_SLICE_TILE_MAX(x)                   (((x) >> 10) & 0xFFFFF)
#define   C_028060_SLICE_TILE_MAX                      0xC00003FF
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
#define   S_028800_STENCILFAIL(x)                      (((unsigned)(x) & 0x7) << 11)
#define   G_028800_STENCILFAIL(x)                      (((x) >> 11) & 0x7)
#define   C_028800_STENCILFAIL                         0xFFFFC7FF
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
#define R_028010_DB_DEPTH_INFO                       0x028010
#define   S_028010_FORMAT(x)                           (((unsigned)(x) & 0x7) << 0)
#define   G_028010_FORMAT(x)                           (((x) >> 0) & 0x7)
#define   C_028010_FORMAT                              0xFFFFFFF8
#define     V_028010_DEPTH_INVALID                     0x00000000
#define     V_028010_DEPTH_16                          0x00000001
#define     V_028010_DEPTH_X8_24                       0x00000002
#define     V_028010_DEPTH_8_24                        0x00000003
#define     V_028010_DEPTH_X8_24_FLOAT                 0x00000004
#define     V_028010_DEPTH_8_24_FLOAT                  0x00000005
#define     V_028010_DEPTH_32_FLOAT                    0x00000006
#define     V_028010_DEPTH_X24_8_32_FLOAT              0x00000007
#define   S_028010_READ_SIZE(x)                        (((unsigned)(x) & 0x1) << 3)
#define   G_028010_READ_SIZE(x)                        (((x) >> 3) & 0x1)
#define   C_028010_READ_SIZE                           0xFFFFFFF7
#define   S_028010_ARRAY_MODE(x)                       (((unsigned)(x) & 0xF) << 15)
#define   G_028010_ARRAY_MODE(x)                       (((x) >> 15) & 0xF)
#define   C_028010_ARRAY_MODE                          0xFFF87FFF
#define   S_028010_TILE_SURFACE_ENABLE(x)              (((unsigned)(x) & 0x1) << 25)
#define   G_028010_TILE_SURFACE_ENABLE(x)              (((x) >> 25) & 0x1)
#define   C_028010_TILE_SURFACE_ENABLE                 0xFDFFFFFF
#define   S_028010_TILE_COMPACT(x)                     (((unsigned)(x) & 0x1) << 26)
#define   G_028010_TILE_COMPACT(x)                     (((x) >> 26) & 0x1)
#define   C_028010_TILE_COMPACT                        0xFBFFFFFF
#define   S_028010_ZRANGE_PRECISION(x)                 (((unsigned)(x) & 0x1) << 31)
#define   G_028010_ZRANGE_PRECISION(x)                 (((x) >> 31) & 0x1)
#define   C_028010_ZRANGE_PRECISION                    0x7FFFFFFF
#define R_028000_DB_DEPTH_SIZE                       0x028000
#define   S_028000_PITCH_TILE_MAX(x)                   (((unsigned)(x) & 0x3FF) << 0)
#define   G_028000_PITCH_TILE_MAX(x)                   (((x) >> 0) & 0x3FF)
#define   C_028000_PITCH_TILE_MAX                      0xFFFFFC00
#define   S_028000_SLICE_TILE_MAX(x)                   (((unsigned)(x) & 0xFFFFF) << 10)
#define   G_028000_SLICE_TILE_MAX(x)                   (((x) >> 10) & 0xFFFFF)
#define   C_028000_SLICE_TILE_MAX                      0xC00003FF
#define R_028004_DB_DEPTH_VIEW                       0x028004
#define   S_028004_SLICE_START(x)                      (((unsigned)(x) & 0x7FF) << 0)
#define   G_028004_SLICE_START(x)                      (((x) >> 0) & 0x7FF)
#define   C_028004_SLICE_START                         0xFFFFF800
#define   S_028004_SLICE_MAX(x)                        (((unsigned)(x) & 0x7FF) << 13)
#define   G_028004_SLICE_MAX(x)                        (((x) >> 13) & 0x7FF)
#define   C_028004_SLICE_MAX                           0xFF001FFF
#define R_028D24_DB_HTILE_SURFACE                    0x028D24
#define   S_028D24_HTILE_WIDTH(x)                      (((unsigned)(x) & 0x1) << 0)
#define   G_028D24_HTILE_WIDTH(x)                      (((x) >> 0) & 0x1)
#define   C_028D24_HTILE_WIDTH                         0xFFFFFFFE
#define   S_028D24_HTILE_HEIGHT(x)                     (((unsigned)(x) & 0x1) << 1)
#define   G_028D24_HTILE_HEIGHT(x)                     (((x) >> 1) & 0x1)
#define   C_028D24_HTILE_HEIGHT                        0xFFFFFFFD
#define   S_028D24_LINEAR(x)                           (((unsigned)(x) & 0x1) << 2)
#define   G_028D24_LINEAR(x)                           (((x) >> 2) & 0x1)
#define   C_028D24_LINEAR                              0xFFFFFFFB
#define   S_028D24_FULL_CACHE(x)                       (((unsigned)(x) & 0x1) << 3)
#define   G_028D24_FULL_CACHE(x)                       (((x) >> 3) & 0x1)
#define   C_028D24_FULL_CACHE                          0xFFFFFFF7
#define   S_028D24_HTILE_USES_PRELOAD_WIN(x)           (((unsigned)(x) & 0x1) << 4)
#define   G_028D24_HTILE_USES_PRELOAD_WIN(x)           (((x) >> 4) & 0x1)
#define   C_028D24_HTILE_USES_PRELOAD_WIN              0xFFFFFFEF
#define   S_028D24_PRELOAD(x)                          (((unsigned)(x) & 0x1) << 5)
#define   G_028D24_PRELOAD(x)                          (((x) >> 5) & 0x1)
#define   C_028D24_PRELOAD                             0xFFFFFFDF
#define   S_028D24_PREFETCH_WIDTH(x)                   (((unsigned)(x) & 0x3F) << 6)
#define   G_028D24_PREFETCH_WIDTH(x)                   (((x) >> 6) & 0x3F)
#define   C_028D24_PREFETCH_WIDTH                      0xFFFFF03F
#define   S_028D24_PREFETCH_HEIGHT(x)                  (((unsigned)(x) & 0x3F) << 12)
#define   G_028D24_PREFETCH_HEIGHT(x)                  (((x) >> 12) & 0x3F)
#define   C_028D24_PREFETCH_HEIGHT                     0xFFFC0FFF
#define R_028D34_DB_PREFETCH_LIMIT                   0x028D34
#define   S_028D34_DEPTH_HEIGHT_TILE_MAX(x)            (((unsigned)(x) & 0x3FF) << 0)
#define   G_028D34_DEPTH_HEIGHT_TILE_MAX(x)            (((x) >> 0) & 0x3FF)
#define   C_028D34_DEPTH_HEIGHT_TILE_MAX               0xFFFFFC00
#define R_028D10_DB_RENDER_OVERRIDE                  0x028D10
#define   S_028D10_FORCE_HIZ_ENABLE(x)                 (((unsigned)(x) & 0x3) << 0)
#define   G_028D10_FORCE_HIZ_ENABLE(x)                 (((x) >> 0) & 0x3)
#define   C_028D10_FORCE_HIZ_ENABLE                    0xFFFFFFFC
#define   S_028D10_FORCE_HIS_ENABLE0(x)                (((unsigned)(x) & 0x3) << 2)
#define   G_028D10_FORCE_HIS_ENABLE0(x)                (((x) >> 2) & 0x3)
#define   C_028D10_FORCE_HIS_ENABLE0                   0xFFFFFFF3
#define   S_028D10_FORCE_HIS_ENABLE1(x)                (((unsigned)(x) & 0x3) << 4)
#define   G_028D10_FORCE_HIS_ENABLE1(x)                (((x) >> 4) & 0x3)
#define   C_028D10_FORCE_HIS_ENABLE1                   0xFFFFFFCF
#define   S_028D10_FORCE_SHADER_Z_ORDER(x)             (((unsigned)(x) & 0x1) << 6)
#define   G_028D10_FORCE_SHADER_Z_ORDER(x)             (((x) >> 6) & 0x1)
#define   C_028D10_FORCE_SHADER_Z_ORDER                0xFFFFFFBF
#define   S_028D10_FAST_Z_DISABLE(x)                   (((unsigned)(x) & 0x1) << 7)
#define   G_028D10_FAST_Z_DISABLE(x)                   (((x) >> 7) & 0x1)
#define   C_028D10_FAST_Z_DISABLE                      0xFFFFFF7F
#define   S_028D10_FAST_STENCIL_DISABLE(x)             (((unsigned)(x) & 0x1) << 8)
#define   G_028D10_FAST_STENCIL_DISABLE(x)             (((x) >> 8) & 0x1)
#define   C_028D10_FAST_STENCIL_DISABLE                0xFFFFFEFF
#define   S_028D10_NOOP_CULL_DISABLE(x)                (((unsigned)(x) & 0x1) << 9)
#define   G_028D10_NOOP_CULL_DISABLE(x)                (((x) >> 9) & 0x1)
#define   C_028D10_NOOP_CULL_DISABLE                   0xFFFFFDFF
#define   S_028D10_FORCE_COLOR_KILL(x)                 (((unsigned)(x) & 0x1) << 10)
#define   G_028D10_FORCE_COLOR_KILL(x)                 (((x) >> 10) & 0x1)
#define   C_028D10_FORCE_COLOR_KILL                    0xFFFFFBFF
#define   S_028D10_FORCE_Z_READ(x)                     (((unsigned)(x) & 0x1) << 11)
#define   G_028D10_FORCE_Z_READ(x)                     (((x) >> 11) & 0x1)
#define   C_028D10_FORCE_Z_READ                        0xFFFFF7FF
#define   S_028D10_FORCE_STENCIL_READ(x)               (((unsigned)(x) & 0x1) << 12)
#define   G_028D10_FORCE_STENCIL_READ(x)               (((x) >> 12) & 0x1)
#define   C_028D10_FORCE_STENCIL_READ                  0xFFFFEFFF
#define   S_028D10_FORCE_FULL_Z_RANGE(x)               (((unsigned)(x) & 0x3) << 13)
#define   G_028D10_FORCE_FULL_Z_RANGE(x)               (((x) >> 13) & 0x3)
#define   C_028D10_FORCE_FULL_Z_RANGE                  0xFFFF9FFF
#define   S_028D10_FORCE_QC_SMASK_CONFLICT(x)          (((unsigned)(x) & 0x1) << 15)
#define   G_028D10_FORCE_QC_SMASK_CONFLICT(x)          (((x) >> 15) & 0x1)
#define   C_028D10_FORCE_QC_SMASK_CONFLICT             0xFFFF7FFF
#define   S_028D10_DISABLE_VIEWPORT_CLAMP(x)           (((unsigned)(x) & 0x1) << 16)
#define   G_028D10_DISABLE_VIEWPORT_CLAMP(x)           (((x) >> 16) & 0x1)
#define   C_028D10_DISABLE_VIEWPORT_CLAMP              0xFFFEFFFF
#define   S_028D10_IGNORE_SC_ZRANGE(x)                 (((unsigned)(x) & 0x1) << 17)
#define   G_028D10_IGNORE_SC_ZRANGE(x)                 (((x) >> 17) & 0x1)
#define   C_028D10_IGNORE_SC_ZRANGE                    0xFFFDFFFF
#define R_028A40_VGT_GS_MODE                         0x028A40
#define   S_028A40_MODE(x)                             (((unsigned)(x) & 0x3) << 0)
#define   G_028A40_MODE(x)                             (((x) >> 0) & 0x3)
#define   C_028A40_MODE                                0xFFFFFFFC
#define     V_028A40_GS_OFF                            0
#define     V_028A40_GS_SCENARIO_A                     1
#define     V_028A40_GS_SCENARIO_B                     2
#define     V_028A40_GS_SCENARIO_G                     3
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
#define R_008DFC_SQ_CF_WORD0                         0x008DFC
#define   S_008DFC_ADDR(x)                             (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_008DFC_ADDR(x)                             (((x) >> 0) & 0xFFFFFFFF)
#define   C_008DFC_ADDR                                0x00000000
#define R_008DFC_SQ_CF_WORD1                         0x008DFC
#define   S_008DFC_POP_COUNT(x)                        (((unsigned)(x) & 0x7) << 0)
#define   G_008DFC_POP_COUNT(x)                        (((x) >> 0) & 0x7)
#define   C_008DFC_POP_COUNT                           0xFFFFFFF8
#define   S_008DFC_CF_CONST(x)                         (((unsigned)(x) & 0x1F) << 3)
#define   G_008DFC_CF_CONST(x)                         (((x) >> 3) & 0x1F)
#define   C_008DFC_CF_CONST                            0xFFFFFF07
#define   S_008DFC_COND(x)                             (((unsigned)(x) & 0x3) << 8)
#define   G_008DFC_COND(x)                             (((x) >> 8) & 0x3)
#define   C_008DFC_COND                                0xFFFFFCFF
#define   S_008DFC_COUNT(x)                            (((unsigned)(x) & 0x7) << 10)
#define   G_008DFC_COUNT(x)                            (((x) >> 10) & 0x7)
#define   C_008DFC_COUNT                               0xFFFFE3FF
#define   S_008DFC_CALL_COUNT(x)                       (((unsigned)(x) & 0x3F) << 13)
#define   G_008DFC_CALL_COUNT(x)                       (((x) >> 13) & 0x3F)
#define   C_008DFC_CALL_COUNT                          0xFFF81FFF
#define   S_008DFC_END_OF_PROGRAM(x)                   (((unsigned)(x) & 0x1) << 21)
#define   G_008DFC_END_OF_PROGRAM(x)                   (((x) >> 21) & 0x1)
#define   C_008DFC_END_OF_PROGRAM                      0xFFDFFFFF
#define   S_008DFC_VALID_PIXEL_MODE(x)                 (((unsigned)(x) & 0x1) << 22)
#define   G_008DFC_VALID_PIXEL_MODE(x)                 (((x) >> 22) & 0x1)
#define   C_008DFC_VALID_PIXEL_MODE                    0xFFBFFFFF
#define   S_008DFC_CF_INST(x)                          (((unsigned)(x) & 0x7F) << 23)
#define   G_008DFC_CF_INST(x)                          (((x) >> 23) & 0x7F)
#define   C_008DFC_CF_INST                             0xC07FFFFF
#define   S_008DFC_WHOLE_QUAD_MODE(x)                  (((unsigned)(x) & 0x1) << 30)
#define   G_008DFC_WHOLE_QUAD_MODE(x)                  (((x) >> 30) & 0x1)
#define   C_008DFC_WHOLE_QUAD_MODE                     0xBFFFFFFF
#define   S_008DFC_BARRIER(x)                          (((unsigned)(x) & 0x1) << 31)
#define   G_008DFC_BARRIER(x)                          (((x) >> 31) & 0x1)
#define   C_008DFC_BARRIER                             0x7FFFFFFF
#define R_008DFC_SQ_CF_ALU_WORD0                     0x008DFC
#define   S_008DFC_ALU_ADDR(x)                         (((unsigned)(x) & 0x3FFFFF) << 0)
#define   G_008DFC_ALU_ADDR(x)                         (((x) >> 0) & 0x3FFFFF)
#define   C_008DFC_ALU_ADDR                            0xFFC00000
#define   S_008DFC_KCACHE_BANK0(x)                     (((unsigned)(x) & 0xF) << 22)
#define   G_008DFC_KCACHE_BANK0(x)                     (((x) >> 22) & 0xF)
#define   C_008DFC_KCACHE_BANK0                        0xFC3FFFFF
#define   S_008DFC_KCACHE_BANK1(x)                     (((unsigned)(x) & 0xF) << 26)
#define   G_008DFC_KCACHE_BANK1(x)                     (((x) >> 26) & 0xF)
#define   C_008DFC_KCACHE_BANK1                        0xC3FFFFFF
#define   S_008DFC_KCACHE_MODE0(x)                     (((unsigned)(x) & 0x3) << 30)
#define   G_008DFC_KCACHE_MODE0(x)                     (((x) >> 30) & 0x3)
#define   C_008DFC_KCACHE_MODE0                        0x3FFFFFFF
#define R_008DFC_SQ_CF_ALU_WORD1                     0x008DFC
#define   S_008DFC_KCACHE_MODE1(x)                     (((unsigned)(x) & 0x3) << 0)
#define   G_008DFC_KCACHE_MODE1(x)                     (((x) >> 0) & 0x3)
#define   C_008DFC_KCACHE_MODE1                        0xFFFFFFFC
#define   S_008DFC_KCACHE_ADDR0(x)                     (((unsigned)(x) & 0xFF) << 2)
#define   G_008DFC_KCACHE_ADDR0(x)                     (((x) >> 2) & 0xFF)
#define   C_008DFC_KCACHE_ADDR0                        0xFFFFFC03
#define   S_008DFC_KCACHE_ADDR1(x)                     (((unsigned)(x) & 0xFF) << 10)
#define   G_008DFC_KCACHE_ADDR1(x)                     (((x) >> 10) & 0xFF)
#define   C_008DFC_KCACHE_ADDR1                        0xFFFC03FF
#define   S_008DFC_ALU_COUNT(x)                        (((unsigned)(x) & 0x7F) << 18)
#define   G_008DFC_ALU_COUNT(x)                        (((x) >> 18) & 0x7F)
#define   C_008DFC_ALU_COUNT                           0xFE03FFFF
#define   S_008DFC_USES_WATERFALL(x)                   (((unsigned)(x) & 0x1) << 25)
#define   G_008DFC_USES_WATERFALL(x)                   (((x) >> 25) & 0x1)
#define   C_008DFC_USES_WATERFALL                      0xFDFFFFFF
#define   S_008DFC_CF_ALU_INST(x)                      (((unsigned)(x) & 0xF) << 26)
#define   G_008DFC_CF_ALU_INST(x)                      (((x) >> 26) & 0xF)
#define   C_008DFC_CF_ALU_INST                         0xC3FFFFFF
#define     V_008DFC_SQ_CF_INST_ALU                    0x00000008
#define     V_008DFC_SQ_CF_INST_ALU_PUSH_BEFORE        0x00000009
#define     V_008DFC_SQ_CF_INST_ALU_POP_AFTER          0x0000000A
#define     V_008DFC_SQ_CF_INST_ALU_POP2_AFTER         0x0000000B
#define     V_008DFC_SQ_CF_INST_ALU_CONTINUE           0x0000000D
#define     V_008DFC_SQ_CF_INST_ALU_BREAK              0x0000000E
#define     V_008DFC_SQ_CF_INST_ALU_ELSE_AFTER         0x0000000F
#define   S_008DFC_WHOLE_QUAD_MODE(x)                  (((unsigned)(x) & 0x1) << 30)
#define   G_008DFC_WHOLE_QUAD_MODE(x)                  (((x) >> 30) & 0x1)
#define   C_008DFC_WHOLE_QUAD_MODE                     0xBFFFFFFF
#define   S_008DFC_BARRIER(x)                          (((unsigned)(x) & 0x1) << 31)
#define   G_008DFC_BARRIER(x)                          (((x) >> 31) & 0x1)
#define   C_008DFC_BARRIER                             0x7FFFFFFF
#define R_008DFC_SQ_CF_ALLOC_EXPORT_WORD0            0x008DFC
#define   S_008DFC_ARRAY_BASE(x)                       (((unsigned)(x) & 0x1FFF) << 0)
#define   G_008DFC_ARRAY_BASE(x)                       (((x) >> 0) & 0x1FFF)
#define   C_008DFC_ARRAY_BASE                          0xFFFFE000
#define   S_008DFC_TYPE(x)                             (((unsigned)(x) & 0x3) << 13)
#define   G_008DFC_TYPE(x)                             (((x) >> 13) & 0x3)
#define   C_008DFC_TYPE                                0xFFFF9FFF
#define   S_008DFC_RW_GPR(x)                           (((unsigned)(x) & 0x7F) << 15)
#define   G_008DFC_RW_GPR(x)                           (((x) >> 15) & 0x7F)
#define   C_008DFC_RW_GPR                              0xFFC07FFF
#define   S_008DFC_RW_REL(x)                           (((unsigned)(x) & 0x1) << 22)
#define   G_008DFC_RW_REL(x)                           (((x) >> 22) & 0x1)
#define   C_008DFC_RW_REL                              0xFFBFFFFF
#define   S_008DFC_INDEX_GPR(x)                        (((unsigned)(x) & 0x7F) << 23)
#define   G_008DFC_INDEX_GPR(x)                        (((x) >> 23) & 0x7F)
#define   C_008DFC_INDEX_GPR                           0xC07FFFFF
#define   S_008DFC_ELEM_SIZE(x)                        (((unsigned)(x) & 0x3) << 30)
#define   G_008DFC_ELEM_SIZE(x)                        (((x) >> 30) & 0x3)
#define   C_008DFC_ELEM_SIZE                           0x3FFFFFFF
#define R_008DFC_SQ_CF_ALLOC_EXPORT_WORD1            0x008DFC
#define   S_008DFC_BURST_COUNT(x)                      (((unsigned)(x) & 0xF) << 17)
#define   G_008DFC_BURST_COUNT(x)                      (((x) >> 17) & 0xF)
#define   C_008DFC_BURST_COUNT                         0xFFE1FFFF
#define   S_008DFC_END_OF_PROGRAM(x)                   (((unsigned)(x) & 0x1) << 21)
#define   G_008DFC_END_OF_PROGRAM(x)                   (((x) >> 21) & 0x1)
#define   C_008DFC_END_OF_PROGRAM                      0xFFDFFFFF
#define   S_008DFC_VALID_PIXEL_MODE(x)                 (((unsigned)(x) & 0x1) << 22)
#define   G_008DFC_VALID_PIXEL_MODE(x)                 (((x) >> 22) & 0x1)
#define   C_008DFC_VALID_PIXEL_MODE                    0xFFBFFFFF
#define   S_008DFC_CF_INST(x)                          (((unsigned)(x) & 0x7F) << 23)
#define   G_008DFC_CF_INST(x)                          (((x) >> 23) & 0x7F)
#define   C_008DFC_CF_INST                             0xC07FFFFF
#define   S_008DFC_WHOLE_QUAD_MODE(x)                  (((unsigned)(x) & 0x1) << 30)
#define   G_008DFC_WHOLE_QUAD_MODE(x)                  (((x) >> 30) & 0x1)
#define   C_008DFC_WHOLE_QUAD_MODE                     0xBFFFFFFF
#define   S_008DFC_BARRIER(x)                          (((unsigned)(x) & 0x1) << 31)
#define   G_008DFC_BARRIER(x)                          (((x) >> 31) & 0x1)
#define   C_008DFC_BARRIER                             0x7FFFFFFF
#define R_008DFC_SQ_CF_ALLOC_EXPORT_WORD1_BUF        0x008DFC
#define   S_008DFC_ARRAY_SIZE(x)                       (((unsigned)(x) & 0xFFF) << 0)
#define   G_008DFC_ARRAY_SIZE(x)                       (((x) >> 0) & 0xFFF)
#define   C_008DFC_ARRAY_SIZE                          0xFFFFF000
#define   S_008DFC_COMP_MASK(x)                        (((unsigned)(x) & 0xF) << 12)
#define   G_008DFC_COMP_MASK(x)                        (((x) >> 12) & 0xF)
#define   C_008DFC_COMP_MASK                           0xFFFF0FFF
#define R_008DFC_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ       0x008DFC
#define   S_008DFC_SEL_X(x)                            (((unsigned)(x) & 0x7) << 0)
#define   G_008DFC_SEL_X(x)                            (((x) >> 0) & 0x7)
#define   C_008DFC_SEL_X                               0xFFFFFFF8
#define   S_008DFC_SEL_Y(x)                            (((unsigned)(x) & 0x7) << 3)
#define   G_008DFC_SEL_Y(x)                            (((x) >> 3) & 0x7)
#define   C_008DFC_SEL_Y                               0xFFFFFFC7
#define   S_008DFC_SEL_Z(x)                            (((unsigned)(x) & 0x7) << 6)
#define   G_008DFC_SEL_Z(x)                            (((x) >> 6) & 0x7)
#define   C_008DFC_SEL_Z                               0xFFFFFE3F
#define   S_008DFC_SEL_W(x)                            (((unsigned)(x) & 0x7) << 9)
#define   G_008DFC_SEL_W(x)                            (((x) >> 9) & 0x7)
#define   C_008DFC_SEL_W                               0xFFFFF1FF
#define R_008DFC_SQ_VTX_WORD0                        0x008DFC
#define   S_008DFC_VTX_INST(x)                         (((unsigned)(x) & 0x1F) << 0)
#define   G_008DFC_VTX_INST(x)                         (((x) >> 0) & 0x1F)
#define   C_008DFC_VTX_INST                            0xFFFFFFE0
#define   S_008DFC_FETCH_TYPE(x)                       (((unsigned)(x) & 0x3) << 5)
#define   G_008DFC_FETCH_TYPE(x)                       (((x) >> 5) & 0x3)
#define   C_008DFC_FETCH_TYPE                          0xFFFFFF9F
#define   S_008DFC_FETCH_WHOLE_QUAD(x)                 (((unsigned)(x) & 0x1) << 7)
#define   G_008DFC_FETCH_WHOLE_QUAD(x)                 (((x) >> 7) & 0x1)
#define   C_008DFC_FETCH_WHOLE_QUAD                    0xFFFFFF7F
#define   S_008DFC_BUFFER_ID(x)                        (((unsigned)(x) & 0xFF) << 8)
#define   G_008DFC_BUFFER_ID(x)                        (((x) >> 8) & 0xFF)
#define   C_008DFC_BUFFER_ID                           0xFFFF00FF
#define   S_008DFC_SRC_GPR(x)                          (((unsigned)(x) & 0x7F) << 16)
#define   G_008DFC_SRC_GPR(x)                          (((x) >> 16) & 0x7F)
#define   C_008DFC_SRC_GPR                             0xFF80FFFF
#define   S_008DFC_SRC_REL(x)                          (((unsigned)(x) & 0x1) << 23)
#define   G_008DFC_SRC_REL(x)                          (((x) >> 23) & 0x1)
#define   C_008DFC_SRC_REL                             0xFF7FFFFF
#define   S_008DFC_SRC_SEL_X(x)                        (((unsigned)(x) & 0x3) << 24)
#define   G_008DFC_SRC_SEL_X(x)                        (((x) >> 24) & 0x3)
#define   C_008DFC_SRC_SEL_X                           0xFCFFFFFF
#define   S_008DFC_MEGA_FETCH_COUNT(x)                 (((unsigned)(x) & 0x3F) << 26)
#define   G_008DFC_MEGA_FETCH_COUNT(x)                 (((x) >> 26) & 0x3F)
#define   C_008DFC_MEGA_FETCH_COUNT                    0x03FFFFFF
#define R_008DFC_SQ_VTX_WORD1                        0x008DFC
#define   S_008DFC_DST_SEL_X(x)                        (((unsigned)(x) & 0x7) << 9)
#define   G_008DFC_DST_SEL_X(x)                        (((x) >> 9) & 0x7)
#define   C_008DFC_DST_SEL_X                           0xFFFFF1FF
#define   S_008DFC_DST_SEL_Y(x)                        (((unsigned)(x) & 0x7) << 12)
#define   G_008DFC_DST_SEL_Y(x)                        (((x) >> 12) & 0x7)
#define   C_008DFC_DST_SEL_Y                           0xFFFF8FFF
#define   S_008DFC_DST_SEL_Z(x)                        (((unsigned)(x) & 0x7) << 15)
#define   G_008DFC_DST_SEL_Z(x)                        (((x) >> 15) & 0x7)
#define   C_008DFC_DST_SEL_Z                           0xFFFC7FFF
#define   S_008DFC_DST_SEL_W(x)                        (((unsigned)(x) & 0x7) << 18)
#define   G_008DFC_DST_SEL_W(x)                        (((x) >> 18) & 0x7)
#define   C_008DFC_DST_SEL_W                           0xFFE3FFFF
#define   S_008DFC_USE_CONST_FIELDS(x)                 (((unsigned)(x) & 0x1) << 21)
#define   G_008DFC_USE_CONST_FIELDS(x)                 (((x) >> 21) & 0x1)
#define   C_008DFC_USE_CONST_FIELDS                    0xFFDFFFFF
#define   S_008DFC_DATA_FORMAT(x)                      (((unsigned)(x) & 0x3F) << 22)
#define   G_008DFC_DATA_FORMAT(x)                      (((x) >> 22) & 0x3F)
#define   C_008DFC_DATA_FORMAT                         0xF03FFFFF
#define   S_008DFC_NUM_FORMAT_ALL(x)                   (((unsigned)(x) & 0x3) << 28)
#define   G_008DFC_NUM_FORMAT_ALL(x)                   (((x) >> 28) & 0x3)
#define   C_008DFC_NUM_FORMAT_ALL                      0xCFFFFFFF
#define   S_008DFC_FORMAT_COMP_ALL(x)                  (((unsigned)(x) & 0x1) << 30)
#define   G_008DFC_FORMAT_COMP_ALL(x)                  (((x) >> 30) & 0x1)
#define   C_008DFC_FORMAT_COMP_ALL                     0xBFFFFFFF
#define   S_008DFC_SRF_MODE_ALL(x)                     (((unsigned)(x) & 0x1) << 31)
#define   G_008DFC_SRF_MODE_ALL(x)                     (((x) >> 31) & 0x1)
#define   C_008DFC_SRF_MODE_ALL                        0x7FFFFFFF
#define R_008DFC_SQ_VTX_WORD1_GPR                    0x008DFC
#define   S_008DFC_DST_GPR(x)                          (((unsigned)(x) & 0x7F) << 0)
#define   G_008DFC_DST_GPR(x)                          (((x) >> 0) & 0x7F)
#define   C_008DFC_DST_GPR                             0xFFFFFF80
#define   S_008DFC_DST_REL(x)                          (((unsigned)(x) & 0x1) << 7)
#define   G_008DFC_DST_REL(x)                          (((x) >> 7) & 0x1)
#define   C_008DFC_DST_REL                             0xFFFFFF7F
#define R_008DFC_SQ_VTX_WORD2                        0x008DFC
#define   S_008DFC_OFFSET(x)                           (((unsigned)(x) & 0xFFFF) << 0)
#define   G_008DFC_OFFSET(x)                           (((x) >> 0) & 0xFFFF)
#define   C_008DFC_OFFSET                              0xFFFF0000
#define   S_008DFC_ENDIAN_SWAP(x)                      (((unsigned)(x) & 0x3) << 16)
#define   G_008DFC_ENDIAN_SWAP(x)                      (((x) >> 16) & 0x3)
#define   C_008DFC_ENDIAN_SWAP                         0xFFFCFFFF
#define   S_008DFC_CONST_BUF_NO_STRIDE(x)              (((unsigned)(x) & 0x1) << 18)
#define   G_008DFC_CONST_BUF_NO_STRIDE(x)              (((x) >> 18) & 0x1)
#define   C_008DFC_CONST_BUF_NO_STRIDE                 0xFFFBFFFF
#define   S_008DFC_MEGA_FETCH(x)                       (((unsigned)(x) & 0x1) << 19)
#define   G_008DFC_MEGA_FETCH(x)                       (((x) >> 19) & 0x1)
#define   C_008DFC_MEGA_FETCH                          0xFFF7FFFF
#define   S_008DFC_ALT_CONST(x)                        (((unsigned)(x) & 0x1) << 20)
#define   G_008DFC_ALT_CONST(x)                        (((x) >> 20) & 0x1)
#define   C_008DFC_ALT_CONST                           0xFFEFFFFF
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
#define   S_0286CC_PARAM_GEN_ADDR(x)                   (((unsigned)(x) & 0x7F) << 19)
#define   G_0286CC_PARAM_GEN_ADDR(x)                   (((x) >> 19) & 0x7F)
#define   C_0286CC_PARAM_GEN_ADDR                      0xFC07FFFF
#define   S_0286CC_BARYC_SAMPLE_CNTL(x)                (((unsigned)(x) & 0x3) << 26)
#define   G_0286CC_BARYC_SAMPLE_CNTL(x)                (((x) >> 26) & 0x3)
#define   C_0286CC_BARYC_SAMPLE_CNTL                   0xF3FFFFFF
#define   S_0286CC_PERSP_GRADIENT_ENA(x)               (((unsigned)(x) & 0x1) << 28)
#define   G_0286CC_PERSP_GRADIENT_ENA(x)               (((x) >> 28) & 0x1)
#define   C_0286CC_PERSP_GRADIENT_ENA                  0xEFFFFFFF
#define   S_0286CC_LINEAR_GRADIENT_ENA(x)              (((unsigned)(x) & 0x1) << 29)
#define   G_0286CC_LINEAR_GRADIENT_ENA(x)              (((x) >> 29) & 0x1)
#define   C_0286CC_LINEAR_GRADIENT_ENA                 0xDFFFFFFF
#define   S_0286CC_POSITION_SAMPLE(x)                  (((unsigned)(x) & 0x1) << 30)
#define   G_0286CC_POSITION_SAMPLE(x)                  (((x) >> 30) & 0x1)
#define   C_0286CC_POSITION_SAMPLE                     0xBFFFFFFF
#define   S_0286CC_BARYC_AT_SAMPLE_ENA(x)              (((unsigned)(x) & 0x1) << 31)
#define   G_0286CC_BARYC_AT_SAMPLE_ENA(x)              (((x) >> 31) & 0x1)
#define   C_0286CC_BARYC_AT_SAMPLE_ENA                 0x7FFFFFFF
#define R_0286D0_SPI_PS_IN_CONTROL_1                 0x0286D0
#define   S_0286D0_GEN_INDEX_PIX(x)                    (((unsigned)(x) & 0x1) << 0)
#define   G_0286D0_GEN_INDEX_PIX(x)                    (((x) >> 0) & 0x1)
#define   C_0286D0_GEN_INDEX_PIX                       0xFFFFFFFE
#define   S_0286D0_GEN_INDEX_PIX_ADDR(x)               (((unsigned)(x) & 0x7F) << 1)
#define   G_0286D0_GEN_INDEX_PIX_ADDR(x)               (((x) >> 1) & 0x7F)
#define   C_0286D0_GEN_INDEX_PIX_ADDR                  0xFFFFFF01
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
#define R_028240_PA_SC_GENERIC_SCISSOR_TL            0x028240
#define   S_028240_TL_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028240_TL_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028240_TL_X                                0xFFFFC000
#define   S_028240_TL_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028240_TL_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028240_TL_Y                                0xC000FFFF
#define   S_028240_WINDOW_OFFSET_DISABLE(x)            (((unsigned)(x) & 0x1) << 31)
#define   G_028240_WINDOW_OFFSET_DISABLE(x)            (((x) >> 31) & 0x1)
#define   C_028240_WINDOW_OFFSET_DISABLE               0x7FFFFFFF
#define R_028244_PA_SC_GENERIC_SCISSOR_BR            0x028244
#define   S_028244_BR_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028244_BR_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028244_BR_X                                0xFFFFC000
#define   S_028244_BR_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028244_BR_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028244_BR_Y                                0xC000FFFF
#define R_028030_PA_SC_SCREEN_SCISSOR_TL             0x028030
#define   S_028030_TL_X(x)                             (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028030_TL_X(x)                             (((x) >> 0) & 0x7FFF)
#define   C_028030_TL_X                                0xFFFF8000
#define   S_028030_TL_Y(x)                             (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028030_TL_Y(x)                             (((x) >> 16) & 0x7FFF)
#define   C_028030_TL_Y                                0x8000FFFF
#define R_028034_PA_SC_SCREEN_SCISSOR_BR             0x028034
#define   S_028034_BR_X(x)                             (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028034_BR_X(x)                             (((x) >> 0) & 0x7FFF)
#define   C_028034_BR_X                                0xFFFF8000
#define   S_028034_BR_Y(x)                             (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028034_BR_Y(x)                             (((x) >> 16) & 0x7FFF)
#define   C_028034_BR_Y                                0x8000FFFF
#define R_028204_PA_SC_WINDOW_SCISSOR_TL             0x028204
#define   S_028204_TL_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028204_TL_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028204_TL_X                                0xFFFFC000
#define   S_028204_TL_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028204_TL_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028204_TL_Y                                0xC000FFFF
#define   S_028204_WINDOW_OFFSET_DISABLE(x)            (((unsigned)(x) & 0x1) << 31)
#define   G_028204_WINDOW_OFFSET_DISABLE(x)            (((x) >> 31) & 0x1)
#define   C_028204_WINDOW_OFFSET_DISABLE               0x7FFFFFFF
#define R_028208_PA_SC_WINDOW_SCISSOR_BR             0x028208
#define   S_028208_BR_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028208_BR_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028208_BR_X                                0xFFFFC000
#define   S_028208_BR_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028208_BR_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028208_BR_Y                                0xC000FFFF
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
#define R_0280A0_CB_COLOR0_INFO                      0x0280A0
#define R_0280A4_CB_COLOR1_INFO                      0x0280A4
#define R_0280A8_CB_COLOR2_INFO                      0x0280A8
#define R_0280AC_CB_COLOR3_INFO                      0x0280AC
#define R_0280B0_CB_COLOR4_INFO                      0x0280B0
#define R_0280B4_CB_COLOR5_INFO                      0x0280B4
#define R_0280B8_CB_COLOR6_INFO                      0x0280B8
#define R_0280BC_CB_COLOR7_INFO                      0x0280BC
#define R_02800C_DB_DEPTH_BASE                       0x02800C
#define R_028000_DB_DEPTH_SIZE                       0x028000
#define R_028004_DB_DEPTH_VIEW                       0x028004
#define R_028010_DB_DEPTH_INFO                       0x028010
#define R_028D24_DB_HTILE_SURFACE                    0x028D24
#define R_028D34_DB_PREFETCH_LIMIT                   0x028D34
#define R_0286D4_SPI_INTERP_CONTROL_0                0x0286D4
#define R_028A48_PA_SC_MPASS_PS_CNTL                 0x028A48
#define R_028C00_PA_SC_LINE_CNTL                     0x028C00
#define   S_028C00_EXPAND_LINE_WIDTH(x)                (((unsigned)(x) & 0x1) << 9)
#define   G_028C00_EXPAND_LINE_WIDTH(x)                (((x) >> 9) & 0x1)
#define   C_028C00_EXPAND_LINE_WIDTH                   0xFFFFFDFF
#define   S_028C00_LAST_PIXEL(x)                       (((unsigned)(x) & 0x1) << 10)
#define   G_028C00_LAST_PIXEL(x)                       (((x) >> 10) & 0x1)
#define   C_028C00_LAST_PIXEL                          0xFFFFFBFF
#define R_028C04_PA_SC_AA_CONFIG                     0x028C04
#define R_028C08_PA_SU_VTX_CNTL                      0x028C08
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
#define R_028C1C_PA_SC_AA_SAMPLE_LOCS_MCTX           0x028C1C
#define R_028C48_PA_SC_AA_MASK                       0x028C48
#define R_028810_PA_CL_CLIP_CNTL                     0x028810
#define R_02881C_PA_CL_VS_OUT_CNTL                   0x02881C
#define R_028820_PA_CL_NANINF_CNTL                   0x028820
#define R_028C0C_PA_CL_GB_VERT_CLIP_ADJ              0x028C0C
#define R_028C10_PA_CL_GB_VERT_DISC_ADJ              0x028C10
#define R_028C14_PA_CL_GB_HORZ_CLIP_ADJ              0x028C14
#define R_028C18_PA_CL_GB_HORZ_DISC_ADJ              0x028C18
#define R_028814_PA_SU_SC_MODE_CNTL                  0x028814
#define R_028A00_PA_SU_POINT_SIZE                    0x028A00
#define R_028A04_PA_SU_POINT_MINMAX                  0x028A04
#define   S_028A04_MIN_SIZE(x)                         (((unsigned)(x) & 0xFFFF) << 0)
#define   G_028A04_MIN_SIZE(x)                         (((x) >> 0) & 0xFFFF)
#define   C_028A04_MIN_SIZE                            0xFFFF0000
#define   S_028A04_MAX_SIZE(x)                         (((unsigned)(x) & 0xFFFF) << 16)
#define   G_028A04_MAX_SIZE(x)                         (((x) >> 16) & 0xFFFF)
#define   C_028A04_MAX_SIZE                            0x0000FFFF
#define R_028A08_PA_SU_LINE_CNTL                     0x028A08
#define   S_028A08_WIDTH(x)                            (((unsigned)(x) & 0xFFFF) << 0)
#define   G_028A08_WIDTH(x)                            (((x) >> 0) & 0xFFFF)
#define   C_028A08_WIDTH                               0xFFFF0000
#define R_028A0C_PA_SC_LINE_STIPPLE                  0x028A0C
#define R_028DF8_PA_SU_POLY_OFFSET_DB_FMT_CNTL       0x028DF8
#define R_028DFC_PA_SU_POLY_OFFSET_CLAMP             0x028DFC
#define R_028E00_PA_SU_POLY_OFFSET_FRONT_SCALE       0x028E00
#define R_028E04_PA_SU_POLY_OFFSET_FRONT_OFFSET      0x028E04
#define R_028E08_PA_SU_POLY_OFFSET_BACK_SCALE        0x028E08
#define R_028E0C_PA_SU_POLY_OFFSET_BACK_OFFSET       0x028E0C
#define R_028818_PA_CL_VTE_CNTL                      0x028818
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
#define R_02843C_PA_CL_VPORT_XSCALE_0                0x02843C
#define R_028444_PA_CL_VPORT_YSCALE_0                0x028444
#define R_02844C_PA_CL_VPORT_ZSCALE_0                0x02844C
#define R_028440_PA_CL_VPORT_XOFFSET_0               0x028440
#define R_028448_PA_CL_VPORT_YOFFSET_0               0x028448
#define R_028450_PA_CL_VPORT_ZOFFSET_0               0x028450
#define R_028250_PA_SC_VPORT_SCISSOR_0_TL            0x028250
#define R_028254_PA_SC_VPORT_SCISSOR_0_BR            0x028254
#define R_028780_CB_BLEND0_CONTROL                   0x028780
#define R_028784_CB_BLEND1_CONTROL                   0x028784
#define R_028788_CB_BLEND2_CONTROL                   0x028788
#define R_02878C_CB_BLEND3_CONTROL                   0x02878C
#define R_028790_CB_BLEND4_CONTROL                   0x028790
#define R_028794_CB_BLEND5_CONTROL                   0x028794
#define R_028798_CB_BLEND6_CONTROL                   0x028798
#define R_02879C_CB_BLEND7_CONTROL                   0x02879C
#define R_028804_CB_BLEND_CONTROL                    0x028804
#define R_028028_DB_STENCIL_CLEAR                    0x028028
#define R_02802C_DB_DEPTH_CLEAR                      0x02802C
#define R_028430_DB_STENCILREFMASK                   0x028430
#define R_028434_DB_STENCILREFMASK_BF                0x028434
#define R_028800_DB_DEPTH_CONTROL                    0x028800
#define R_02880C_DB_SHADER_CONTROL                   0x02880C
#define R_028D0C_DB_RENDER_CONTROL                   0x028D0C
#define R_028D10_DB_RENDER_OVERRIDE                  0x028D10
#define R_028D28_DB_SRESULTS_COMPARE_STATE0          0x028D28
#define R_028D2C_DB_SRESULTS_COMPARE_STATE1          0x028D2C
#define R_028D30_DB_PRELOAD_CONTROL                  0x028D30
#define R_028D44_DB_ALPHA_TO_MASK                    0x028D44
#define   S_028D44_ALPHA_TO_MASK_ENABLE(x)		(((unsigned)(x) & 0x1) << 0)
#define   S_028D44_ALPHA_TO_MASK_OFFSET0(x)		(((unsigned)(x) & 0x3) << 8)
#define   S_028D44_ALPHA_TO_MASK_OFFSET1(x)		(((unsigned)(x) & 0x3) << 10)
#define   S_028D44_ALPHA_TO_MASK_OFFSET2(x)		(((unsigned)(x) & 0x3) << 12)
#define   S_028D44_ALPHA_TO_MASK_OFFSET3(x)		(((unsigned)(x) & 0x3) << 14)
#define   S_028D44_OFFSET_ROUND(x)			(((unsigned)(x) & 0x1) << 16)
#define R_028868_SQ_PGM_RESOURCES_VS                 0x028868
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
#define R_02887C_SQ_PGM_RESOURCES_GS                 0x02887C
#define   S_02887C_NUM_GPRS(x)                         (((unsigned)(x) & 0xFF) << 0)
#define   G_02887C_NUM_GPRS(x)                         (((x) >> 0) & 0xFF)
#define   C_02887C_NUM_GPRS                            0xFFFFFF00
#define   S_02887C_STACK_SIZE(x)                       (((unsigned)(x) & 0xFF) << 8)
#define   G_02887C_STACK_SIZE(x)                       (((x) >> 8) & 0xFF)
#define   C_02887C_STACK_SIZE                          0xFFFF00FF
#define   S_02887C_DX10_CLAMP(x)                       (((unsigned)(x) & 0x1) << 21)
#define   G_02887C_DX10_CLAMP(x)                       (((x) >> 21) & 0x1)
#define   C_02887C_DX10_CLAMP                          0xFFDFFFFF
#define R_0286CC_SPI_PS_IN_CONTROL_0                 0x0286CC
#define R_0286D0_SPI_PS_IN_CONTROL_1                 0x0286D0
#define R_028644_SPI_PS_INPUT_CNTL_0                 0x028644
#define R_028648_SPI_PS_INPUT_CNTL_1                 0x028648
#define R_02864C_SPI_PS_INPUT_CNTL_2                 0x02864C
#define R_028650_SPI_PS_INPUT_CNTL_3                 0x028650
#define R_028654_SPI_PS_INPUT_CNTL_4                 0x028654
#define R_028658_SPI_PS_INPUT_CNTL_5                 0x028658
#define R_02865C_SPI_PS_INPUT_CNTL_6                 0x02865C
#define R_028660_SPI_PS_INPUT_CNTL_7                 0x028660
#define R_028664_SPI_PS_INPUT_CNTL_8                 0x028664
#define R_028668_SPI_PS_INPUT_CNTL_9                 0x028668
#define R_02866C_SPI_PS_INPUT_CNTL_10                0x02866C
#define R_028670_SPI_PS_INPUT_CNTL_11                0x028670
#define R_028674_SPI_PS_INPUT_CNTL_12                0x028674
#define R_028678_SPI_PS_INPUT_CNTL_13                0x028678
#define R_02867C_SPI_PS_INPUT_CNTL_14                0x02867C
#define R_028680_SPI_PS_INPUT_CNTL_15                0x028680
#define R_028684_SPI_PS_INPUT_CNTL_16                0x028684
#define R_028688_SPI_PS_INPUT_CNTL_17                0x028688
#define R_02868C_SPI_PS_INPUT_CNTL_18                0x02868C
#define R_028690_SPI_PS_INPUT_CNTL_19                0x028690
#define R_028694_SPI_PS_INPUT_CNTL_20                0x028694
#define R_028698_SPI_PS_INPUT_CNTL_21                0x028698
#define R_02869C_SPI_PS_INPUT_CNTL_22                0x02869C
#define R_0286A0_SPI_PS_INPUT_CNTL_23                0x0286A0
#define R_0286A4_SPI_PS_INPUT_CNTL_24                0x0286A4
#define R_0286A8_SPI_PS_INPUT_CNTL_25                0x0286A8
#define R_0286AC_SPI_PS_INPUT_CNTL_26                0x0286AC
#define R_0286B0_SPI_PS_INPUT_CNTL_27                0x0286B0
#define R_0286B4_SPI_PS_INPUT_CNTL_28                0x0286B4
#define R_0286B8_SPI_PS_INPUT_CNTL_29                0x0286B8
#define R_0286BC_SPI_PS_INPUT_CNTL_30                0x0286BC
#define R_0286C0_SPI_PS_INPUT_CNTL_31                0x0286C0
#define R_028850_SQ_PGM_RESOURCES_PS                 0x028850
#define R_028854_SQ_PGM_EXPORTS_PS                   0x028854
#define   S_028854_EXPORT_COLORS(x)                    (((unsigned)(x) & 0xF) << 1)
#define   G_028854_EXPORT_COLORS(x)                    (((x) >> 1) & 0xF)
#define   C_028854_EXPORT_COLORS                       0xFFFFFFE1
#define   S_028854_EXPORT_Z(x)                         (((unsigned)(x) & 0x1) << 0)
#define   G_028854_EXPORT_Z(x)                         (((x) >> 0) & 0x1)
#define   C_028854_EXPORT_Z                            0xFFFFFFFE
#define R_008958_VGT_PRIMITIVE_TYPE                  0x008958
#define R_028A7C_VGT_DMA_INDEX_TYPE                  0x028A7C
#define R_028A88_VGT_DMA_NUM_INSTANCES               0x028A88
#define R_008970_VGT_NUM_INDICES                     0x008970
#define R_0287F0_VGT_DRAW_INITIATOR                  0x0287F0
#define R_028238_CB_TARGET_MASK                      0x028238
#define R_02823C_CB_SHADER_MASK                      0x02823C
#define R_028060_CB_COLOR0_SIZE                      0x028060
#define   S_028060_PITCH_TILE_MAX(x)                   (((unsigned)(x) & 0x3FF) << 0)
#define   G_028060_PITCH_TILE_MAX(x)                   (((x) >> 0) & 0x3FF)
#define   C_028060_PITCH_TILE_MAX                      0xFFFFFC00
#define   S_028060_SLICE_TILE_MAX(x)                   (((unsigned)(x) & 0xFFFFF) << 10)
#define   G_028060_SLICE_TILE_MAX(x)                   (((x) >> 10) & 0xFFFFF)
#define   C_028060_SLICE_TILE_MAX                      0xC00003FF
#define R_028064_CB_COLOR1_SIZE                      0x028064
#define R_028068_CB_COLOR2_SIZE                      0x028068
#define R_02806C_CB_COLOR3_SIZE                      0x02806C
#define R_028070_CB_COLOR4_SIZE                      0x028070
#define R_028074_CB_COLOR5_SIZE                      0x028074
#define R_028078_CB_COLOR6_SIZE                      0x028078
#define R_02807C_CB_COLOR7_SIZE                      0x02807C
#define R_028040_CB_COLOR0_BASE                      0x028040
#define R_028044_CB_COLOR1_BASE                      0x028044
#define R_028048_CB_COLOR2_BASE                      0x028048
#define R_02804C_CB_COLOR3_BASE                      0x02804C
#define R_028050_CB_COLOR4_BASE                      0x028050
#define R_028054_CB_COLOR5_BASE                      0x028054
#define R_028058_CB_COLOR6_BASE                      0x028058
#define R_02805C_CB_COLOR7_BASE                      0x02805C
#define R_028240_PA_SC_GENERIC_SCISSOR_TL            0x028240
#define   S_028240_TL_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028240_TL_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028240_TL_X                                0xFFFFC000
#define   S_028240_TL_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028240_TL_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028240_TL_Y                                0xC000FFFF
#define R_028C04_PA_SC_AA_CONFIG                     0x028C04
#define   S_028C04_MSAA_NUM_SAMPLES(x)                 (((unsigned)(x) & 0x3) << 0)
#define   G_028C04_MSAA_NUM_SAMPLES(x)                 (((x) >> 0) & 0x3)
#define   C_028C04_MSAA_NUM_SAMPLES                    0xFFFFFFFC
#define   S_028C04_AA_MASK_CENTROID_DTMN(x)            (((unsigned)(x) & 0x1) << 4)
#define   G_028C04_AA_MASK_CENTROID_DTMN(x)            (((x) >> 4) & 0x1)
#define   C_028C04_AA_MASK_CENTROID_DTMN               0xFFFFFFEF
#define   S_028C04_MAX_SAMPLE_DIST(x)                  (((unsigned)(x) & 0xF) << 13)
#define   G_028C04_MAX_SAMPLE_DIST(x)                  (((x) >> 13) & 0xF)
#define   C_028C04_MAX_SAMPLE_DIST                     0xFFFE1FFF
#define R_0288CC_SQ_PGM_CF_OFFSET_PS                 0x0288CC
#define R_0288D0_SQ_PGM_CF_OFFSET_VS                 0x0288D0
#define R_0288D4_SQ_PGM_CF_OFFSET_GS                 0x0288D4
#define R_0288D8_SQ_PGM_CF_OFFSET_ES                 0x0288D8
#define R_0288DC_SQ_PGM_CF_OFFSET_FS                 0x0288DC
#define R_028840_SQ_PGM_START_PS                     0x028840
#define R_028894_SQ_PGM_START_FS                     0x028894
#define R_028858_SQ_PGM_START_VS                     0x028858
#define R_02886C_SQ_PGM_START_GS                     0x02886C
#define R_028880_SQ_PGM_START_ES                     0x028880
#define R_028080_CB_COLOR0_VIEW                      0x028080
#define   S_028080_SLICE_START(x)                      (((unsigned)(x) & 0x7FF) << 0)
#define   G_028080_SLICE_START(x)                      (((x) >> 0) & 0x7FF)
#define   C_028080_SLICE_START                         0xFFFFF800
#define   S_028080_SLICE_MAX(x)                        (((unsigned)(x) & 0x7FF) << 13)
#define   G_028080_SLICE_MAX(x)                        (((x) >> 13) & 0x7FF)
#define   C_028080_SLICE_MAX                           0xFF001FFF
#define R_028084_CB_COLOR1_VIEW                      0x028084
#define R_028088_CB_COLOR2_VIEW                      0x028088
#define R_02808C_CB_COLOR3_VIEW                      0x02808C
#define R_028090_CB_COLOR4_VIEW                      0x028090
#define R_028094_CB_COLOR5_VIEW                      0x028094
#define R_028098_CB_COLOR6_VIEW                      0x028098
#define R_02809C_CB_COLOR7_VIEW                      0x02809C
#define R_028100_CB_COLOR0_MASK                      0x028100
#define   S_028100_CMASK_BLOCK_MAX(x)                  (((unsigned)(x) & 0xFFF) << 0)
#define   G_028100_CMASK_BLOCK_MAX(x)                  (((x) >> 0) & 0xFFF)
#define   C_028100_CMASK_BLOCK_MAX                     0xFFFFF000
#define   S_028100_FMASK_TILE_MAX(x)                   (((unsigned)(x) & 0xFFFFF) << 12)
#define   G_028100_FMASK_TILE_MAX(x)                   (((x) >> 12) & 0xFFFFF)
#define   C_028100_FMASK_TILE_MAX                      0x00000FFF
#define R_028104_CB_COLOR1_MASK                      0x028104
#define R_028108_CB_COLOR2_MASK                      0x028108
#define R_02810C_CB_COLOR3_MASK                      0x02810C
#define R_028110_CB_COLOR4_MASK                      0x028110
#define R_028114_CB_COLOR5_MASK                      0x028114
#define R_028118_CB_COLOR6_MASK                      0x028118
#define R_02811C_CB_COLOR7_MASK                      0x02811C
#define R_028040_CB_COLOR0_BASE                      0x028040
#define   S_028040_BASE_256B(x)                        (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028040_BASE_256B(x)                        (((x) >> 0) & 0xFFFFFFFF)
#define   C_028040_BASE_256B                           0x00000000
#define R_0280E0_CB_COLOR0_FRAG                      0x0280E0
#define   S_0280E0_BASE_256B(x)                        (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_0280E0_BASE_256B(x)                        (((x) >> 0) & 0xFFFFFFFF)
#define   C_0280E0_BASE_256B                           0x00000000
#define R_0280E4_CB_COLOR1_FRAG                      0x0280E4
#define R_0280E8_CB_COLOR2_FRAG                      0x0280E8
#define R_0280EC_CB_COLOR3_FRAG                      0x0280EC
#define R_0280F0_CB_COLOR4_FRAG                      0x0280F0
#define R_0280F4_CB_COLOR5_FRAG                      0x0280F4
#define R_0280F8_CB_COLOR6_FRAG                      0x0280F8
#define R_0280FC_CB_COLOR7_FRAG                      0x0280FC
#define R_0280C0_CB_COLOR0_TILE                      0x0280C0
#define   S_0280C0_BASE_256B(x)                        (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_0280C0_BASE_256B(x)                        (((x) >> 0) & 0xFFFFFFFF)
#define   C_0280C0_BASE_256B                           0x00000000
#define R_0280C4_CB_COLOR1_TILE                      0x0280C4
#define R_0280C8_CB_COLOR2_TILE                      0x0280C8
#define R_0280CC_CB_COLOR3_TILE                      0x0280CC
#define R_0280D0_CB_COLOR4_TILE                      0x0280D0
#define R_0280D4_CB_COLOR5_TILE                      0x0280D4
#define R_0280D8_CB_COLOR6_TILE                      0x0280D8
#define R_0280DC_CB_COLOR7_TILE                      0x0280DC
#define R_028614_SPI_VS_OUT_ID_0                     0x028614
#define   S_028614_SEMANTIC_0(x)                       (((unsigned)(x) & 0xFF) << 0)
#define   G_028614_SEMANTIC_0(x)                       (((x) >> 0) & 0xFF)
#define   C_028614_SEMANTIC_0                          0xFFFFFF00
#define   S_028614_SEMANTIC_1(x)                       (((unsigned)(x) & 0xFF) << 8)
#define   G_028614_SEMANTIC_1(x)                       (((x) >> 8) & 0xFF)
#define   C_028614_SEMANTIC_1                          0xFFFF00FF
#define   S_028614_SEMANTIC_2(x)                       (((unsigned)(x) & 0xFF) << 16)
#define   G_028614_SEMANTIC_2(x)                       (((x) >> 16) & 0xFF)
#define   C_028614_SEMANTIC_2                          0xFF00FFFF
#define   S_028614_SEMANTIC_3(x)                       (((unsigned)(x) & 0xFF) << 24)
#define   G_028614_SEMANTIC_3(x)                       (((x) >> 24) & 0xFF)
#define   C_028614_SEMANTIC_3                          0x00FFFFFF
#define R_028618_SPI_VS_OUT_ID_1                     0x028618
#define R_02861C_SPI_VS_OUT_ID_2                     0x02861C
#define R_028620_SPI_VS_OUT_ID_3                     0x028620
#define R_028624_SPI_VS_OUT_ID_4                     0x028624
#define R_028628_SPI_VS_OUT_ID_5                     0x028628
#define R_02862C_SPI_VS_OUT_ID_6                     0x02862C
#define R_028630_SPI_VS_OUT_ID_7                     0x028630
#define R_028634_SPI_VS_OUT_ID_8                     0x028634
#define R_028638_SPI_VS_OUT_ID_9                     0x028638
#define R_038000_SQ_TEX_RESOURCE_WORD0_0             0x038000
#define   S_038000_DIM(x)                              (((unsigned)(x) & 0x7) << 0)
#define   G_038000_DIM(x)                              (((x) >> 0) & 0x7)
#define   C_038000_DIM                                 0xFFFFFFF8
#define   S_038000_TILE_MODE(x)                        (((unsigned)(x) & 0xF) << 3)
#define   G_038000_TILE_MODE(x)                        (((x) >> 3) & 0xF)
#define   C_038000_TILE_MODE                           0xFFFFFF87
#define   S_038000_TILE_TYPE(x)                        (((unsigned)(x) & 0x1) << 7)
#define   G_038000_TILE_TYPE(x)                        (((x) >> 7) & 0x1)
#define   C_038000_TILE_TYPE                           0xFFFFFF7F
#define   S_038000_PITCH(x)                            (((unsigned)(x) & 0x7FF) << 8)
#define   G_038000_PITCH(x)                            (((x) >> 8) & 0x7FF)
#define   C_038000_PITCH                               0xFFF800FF
#define   S_038000_TEX_WIDTH(x)                        (((unsigned)(x) & 0x1FFF) << 19)
#define   G_038000_TEX_WIDTH(x)                        (((x) >> 19) & 0x1FFF)
#define   C_038000_TEX_WIDTH                           0x0007FFFF
#define R_038004_SQ_TEX_RESOURCE_WORD1_0             0x038004
#define   S_038004_TEX_HEIGHT(x)                       (((unsigned)(x) & 0x1FFF) << 0)
#define   G_038004_TEX_HEIGHT(x)                       (((x) >> 0) & 0x1FFF)
#define   C_038004_TEX_HEIGHT                          0xFFFFE000
#define   S_038004_TEX_DEPTH(x)                        (((unsigned)(x) & 0x1FFF) << 13)
#define   G_038004_TEX_DEPTH(x)                        (((x) >> 13) & 0x1FFF)
#define   C_038004_TEX_DEPTH                           0xFC001FFF
#define   S_038004_DATA_FORMAT(x)                      (((unsigned)(x) & 0x3F) << 26)
#define   G_038004_DATA_FORMAT(x)                      (((x) >> 26) & 0x3F)
#define   C_038004_DATA_FORMAT                         0x03FFFFFF
#define     V_038004_COLOR_INVALID                     0x00000000
#define     V_038004_COLOR_8                           0x00000001
#define     V_038004_COLOR_4_4                         0x00000002
#define     V_038004_COLOR_3_3_2                       0x00000003
#define     V_038004_COLOR_16                          0x00000005
#define     V_038004_COLOR_16_FLOAT                    0x00000006
#define     V_038004_COLOR_8_8                         0x00000007
#define     V_038004_COLOR_5_6_5                       0x00000008
#define     V_038004_COLOR_6_5_5                       0x00000009
#define     V_038004_COLOR_1_5_5_5                     0x0000000A
#define     V_038004_COLOR_4_4_4_4                     0x0000000B
#define     V_038004_COLOR_5_5_5_1                     0x0000000C
#define     V_038004_COLOR_32                          0x0000000D
#define     V_038004_COLOR_32_FLOAT                    0x0000000E
#define     V_038004_COLOR_16_16                       0x0000000F
#define     V_038004_COLOR_16_16_FLOAT                 0x00000010
#define     V_038004_COLOR_8_24                        0x00000011
#define     V_038004_COLOR_8_24_FLOAT                  0x00000012
#define     V_038004_COLOR_24_8                        0x00000013
#define     V_038004_COLOR_24_8_FLOAT                  0x00000014
#define     V_038004_COLOR_10_11_11                    0x00000015
#define     V_038004_COLOR_10_11_11_FLOAT              0x00000016
#define     V_038004_COLOR_11_11_10                    0x00000017
#define     V_038004_COLOR_11_11_10_FLOAT              0x00000018
#define     V_038004_COLOR_2_10_10_10                  0x00000019
#define     V_038004_COLOR_8_8_8_8                     0x0000001A
#define     V_038004_COLOR_10_10_10_2                  0x0000001B
#define     V_038004_COLOR_X24_8_32_FLOAT              0x0000001C
#define     V_038004_COLOR_32_32                       0x0000001D
#define     V_038004_COLOR_32_32_FLOAT                 0x0000001E
#define     V_038004_COLOR_16_16_16_16                 0x0000001F
#define     V_038004_COLOR_16_16_16_16_FLOAT           0x00000020
#define     V_038004_COLOR_32_32_32_32                 0x00000022
#define     V_038004_COLOR_32_32_32_32_FLOAT           0x00000023
#define R_038008_SQ_TEX_RESOURCE_WORD2_0             0x038008
#define   S_038008_BASE_ADDRESS(x)                     (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_038008_BASE_ADDRESS(x)                     (((x) >> 0) & 0xFFFFFFFF)
#define   C_038008_BASE_ADDRESS                        0x00000000
#define R_03800C_SQ_TEX_RESOURCE_WORD3_0             0x03800C
#define   S_03800C_MIP_ADDRESS(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_03800C_MIP_ADDRESS(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_03800C_MIP_ADDRESS                         0x00000000
#define R_038010_SQ_TEX_RESOURCE_WORD4_0             0x038010
#define   S_038010_FORMAT_COMP_X(x)                    (((unsigned)(x) & 0x3) << 0)
#define   G_038010_FORMAT_COMP_X(x)                    (((x) >> 0) & 0x3)
#define   C_038010_FORMAT_COMP_X                       0xFFFFFFFC
#define   S_038010_FORMAT_COMP_Y(x)                    (((unsigned)(x) & 0x3) << 2)
#define   G_038010_FORMAT_COMP_Y(x)                    (((x) >> 2) & 0x3)
#define   C_038010_FORMAT_COMP_Y                       0xFFFFFFF3
#define   S_038010_FORMAT_COMP_Z(x)                    (((unsigned)(x) & 0x3) << 4)
#define   G_038010_FORMAT_COMP_Z(x)                    (((x) >> 4) & 0x3)
#define   C_038010_FORMAT_COMP_Z                       0xFFFFFFCF
#define   S_038010_FORMAT_COMP_W(x)                    (((unsigned)(x) & 0x3) << 6)
#define   G_038010_FORMAT_COMP_W(x)                    (((x) >> 6) & 0x3)
#define   C_038010_FORMAT_COMP_W                       0xFFFFFF3F
#define   S_038010_NUM_FORMAT_ALL(x)                   (((unsigned)(x) & 0x3) << 8)
#define   G_038010_NUM_FORMAT_ALL(x)                   (((x) >> 8) & 0x3)
#define   C_038010_NUM_FORMAT_ALL                      0xFFFFFCFF
#define   S_038010_SRF_MODE_ALL(x)                     (((unsigned)(x) & 0x1) << 10)
#define   G_038010_SRF_MODE_ALL(x)                     (((x) >> 10) & 0x1)
#define   C_038010_SRF_MODE_ALL                        0xFFFFFBFF
#define   S_038010_FORCE_DEGAMMA(x)                    (((unsigned)(x) & 0x1) << 11)
#define   G_038010_FORCE_DEGAMMA(x)                    (((x) >> 11) & 0x1)
#define   C_038010_FORCE_DEGAMMA                       0xFFFFF7FF
#define   S_038010_ENDIAN_SWAP(x)                      (((unsigned)(x) & 0x3) << 12)
#define   G_038010_ENDIAN_SWAP(x)                      (((x) >> 12) & 0x3)
#define   C_038010_ENDIAN_SWAP                         0xFFFFCFFF
#define   S_038010_REQUEST_SIZE(x)                     (((unsigned)(x) & 0x3) << 14)
#define   G_038010_REQUEST_SIZE(x)                     (((x) >> 14) & 0x3)
#define   C_038010_REQUEST_SIZE                        0xFFFF3FFF
#define   S_038010_DST_SEL_X(x)                        (((unsigned)(x) & 0x7) << 16)
#define   G_038010_DST_SEL_X(x)                        (((x) >> 16) & 0x7)
#define   C_038010_DST_SEL_X                           0xFFF8FFFF
#define   S_038010_DST_SEL_Y(x)                        (((unsigned)(x) & 0x7) << 19)
#define   G_038010_DST_SEL_Y(x)                        (((x) >> 19) & 0x7)
#define   C_038010_DST_SEL_Y                           0xFFC7FFFF
#define   S_038010_DST_SEL_Z(x)                        (((unsigned)(x) & 0x7) << 22)
#define   G_038010_DST_SEL_Z(x)                        (((x) >> 22) & 0x7)
#define   C_038010_DST_SEL_Z                           0xFE3FFFFF
#define   S_038010_DST_SEL_W(x)                        (((unsigned)(x) & 0x7) << 25)
#define   G_038010_DST_SEL_W(x)                        (((x) >> 25) & 0x7)
#define   C_038010_DST_SEL_W                           0xF1FFFFFF
#define   S_038010_BASE_LEVEL(x)                       (((unsigned)(x) & 0xF) << 28)
#define   G_038010_BASE_LEVEL(x)                       (((x) >> 28) & 0xF)
#define   C_038010_BASE_LEVEL                          0x0FFFFFFF
#define R_038014_SQ_TEX_RESOURCE_WORD5_0             0x038014
#define   S_038014_LAST_LEVEL(x)                       (((unsigned)(x) & 0xF) << 0)
#define   G_038014_LAST_LEVEL(x)                       (((x) >> 0) & 0xF)
#define   C_038014_LAST_LEVEL                          0xFFFFFFF0
#define   S_038014_BASE_ARRAY(x)                       (((unsigned)(x) & 0x1FFF) << 4)
#define   G_038014_BASE_ARRAY(x)                       (((x) >> 4) & 0x1FFF)
#define   C_038014_BASE_ARRAY                          0xFFFE000F
#define   S_038014_LAST_ARRAY(x)                       (((unsigned)(x) & 0x1FFF) << 17)
#define   G_038014_LAST_ARRAY(x)                       (((x) >> 17) & 0x1FFF)
#define   C_038014_LAST_ARRAY                          0xC001FFFF
#define R_038018_SQ_TEX_RESOURCE_WORD6_0             0x038018
#define   S_038018_MPEG_CLAMP(x)                       (((unsigned)(x) & 0x3) << 0)
#define   G_038018_MPEG_CLAMP(x)                       (((x) >> 0) & 0x3)
#define   C_038018_MPEG_CLAMP                          0xFFFFFFFC
#define   S_038018_PERF_MODULATION(x)                  (((unsigned)(x) & 0x7) << 5)
#define   G_038018_PERF_MODULATION(x)                  (((x) >> 5) & 0x7)
#define   C_038018_PERF_MODULATION                     0xFFFFFF1F
#define   S_038018_INTERLACED(x)                       (((unsigned)(x) & 0x1) << 8)
#define   G_038018_INTERLACED(x)                       (((x) >> 8) & 0x1)
#define   C_038018_INTERLACED                          0xFFFFFEFF
#define   S_038018_TYPE(x)                             (((unsigned)(x) & 0x3) << 30)
#define   G_038018_TYPE(x)                             (((x) >> 30) & 0x3)
#define   C_038018_TYPE                                0x3FFFFFFF
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
#define R_008958_VGT_PRIMITIVE_TYPE                  0x008958
#define   S_008958_PRIM_TYPE(x)                        (((unsigned)(x) & 0x3F) << 0)
#define   G_008958_PRIM_TYPE(x)                        (((x) >> 0) & 0x3F)
#define   C_008958_PRIM_TYPE                           0xFFFFFFC0
#define R_008C08_SQ_GPR_RESOURCE_MGMT_2              0x008C08
#define   S_008C08_NUM_GS_GPRS(x)                      (((unsigned)(x) & 0xFF) << 0)
#define   G_008C08_NUM_GS_GPRS(x)                      (((x) >> 0) & 0xFF)
#define   C_008C08_NUM_GS_GPRS                         0xFFFFFF00
#define   S_008C08_NUM_ES_GPRS(x)                      (((unsigned)(x) & 0xFF) << 16)
#define   G_008C08_NUM_ES_GPRS(x)                      (((x) >> 16) & 0xFF)
#define   C_008C08_NUM_ES_GPRS                         0xFF00FFFF
#define R_008D8C_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ        0x008D8C
#define   S_008D8C_RING0_OFFSET(x)                     (((unsigned)(x) & 0xFF) << 0)
#define   G_008D8C_RING0_OFFSET(x)                     (((x) >> 0) & 0xFF)
#define   C_008D8C_RING0_OFFSET                        0xFFFFFF00
#define   S_008D8C_ISOLATE_ES_ENABLE(x)                (((unsigned)(x) & 0x1) << 12)
#define   G_008D8C_ISOLATE_ES_ENABLE(x)                (((x) >> 12) & 0x1)
#define   C_008D8C_ISOLATE_ES_ENABLE                   0xFFFFEFFF
#define   S_008D8C_ISOLATE_GS_ENABLE(x)                (((unsigned)(x) & 0x1) << 13)
#define   G_008D8C_ISOLATE_GS_ENABLE(x)                (((x) >> 13) & 0x1)
#define   C_008D8C_ISOLATE_GS_ENABLE                   0xFFFFDFFF
#define   S_008D8C_VS_PC_LIMIT_ENABLE(x)               (((unsigned)(x) & 0x1) << 14)
#define   G_008D8C_VS_PC_LIMIT_ENABLE(x)               (((x) >> 14) & 0x1)
#define   C_008D8C_VS_PC_LIMIT_ENABLE                  0xFFFFBFFF
#define R_009508_TA_CNTL_AUX                         0x009508
#define   S_009508_DISABLE_CUBE_WRAP(x)                (((unsigned)(x) & 0x1) << 0)
#define   G_009508_DISABLE_CUBE_WRAP(x)                (((x) >> 0) & 0x1)
#define   C_009508_DISABLE_CUBE_WRAP                   0xFFFFFFFE
#define   S_009508_DISABLE_CUBE_ANISO(x)               (((unsigned)(x) & 0x1) << 1)
#define   G_009508_DISABLE_CUBE_ANISO(x)               (((x) >> 1) & 0x1)
#define   C_009508_DISABLE_CUBE_ANISO                  (~(1 << 1))
#define   S_009508_SYNC_GRADIENT(x)                    (((unsigned)(x) & 0x1) << 24)
#define   G_009508_SYNC_GRADIENT(x)                    (((x) >> 24) & 0x1)
#define   C_009508_SYNC_GRADIENT                       0xFEFFFFFF
#define   S_009508_SYNC_WALKER(x)                      (((unsigned)(x) & 0x1) << 25)
#define   G_009508_SYNC_WALKER(x)                      (((x) >> 25) & 0x1)
#define   C_009508_SYNC_WALKER                         0xFDFFFFFF
#define   S_009508_SYNC_ALIGNER(x)                     (((unsigned)(x) & 0x1) << 26)
#define   G_009508_SYNC_ALIGNER(x)                     (((x) >> 26) & 0x1)
#define   C_009508_SYNC_ALIGNER                        0xFBFFFFFF
#define   S_009508_BILINEAR_PRECISION(x)               (((unsigned)(x) & 0x1) << 31)
#define   G_009508_BILINEAR_PRECISION(x)               (((x) >> 31) & 0x1)
#define   C_009508_BILINEAR_PRECISION                  0x7FFFFFFF
#define R_009714_VC_ENHANCE                          0x009714
#define R_009830_DB_DEBUG                            0x009830
#define R_009838_DB_WATERMARKS                       0x009838
#define   S_009838_DEPTH_FREE(x)                       (((unsigned)(x) & 0x1F) << 0)
#define   G_009838_DEPTH_FREE(x)                       (((x) >> 0) & 0x1F)
#define   C_009838_DEPTH_FREE                          0xFFFFFFE0
#define   S_009838_DEPTH_FLUSH(x)                      (((unsigned)(x) & 0x3F) << 5)
#define   G_009838_DEPTH_FLUSH(x)                      (((x) >> 5) & 0x3F)
#define   C_009838_DEPTH_FLUSH                         0xFFFFF81F
#define   S_009838_FORCE_SUMMARIZE(x)                  (((unsigned)(x) & 0xF) << 11)
#define   G_009838_FORCE_SUMMARIZE(x)                  (((x) >> 11) & 0xF)
#define   C_009838_FORCE_SUMMARIZE                     0xFFFF87FF
#define   S_009838_DEPTH_PENDING_FREE(x)               (((unsigned)(x) & 0x1F) << 15)
#define   G_009838_DEPTH_PENDING_FREE(x)               (((x) >> 15) & 0x1F)
#define   C_009838_DEPTH_PENDING_FREE                  0xFFF07FFF
#define   S_009838_DEPTH_CACHELINE_FREE(x)             (((unsigned)(x) & 0x1F) << 20)
#define   G_009838_DEPTH_CACHELINE_FREE(x)             (((x) >> 20) & 0x1F)
#define   C_009838_DEPTH_CACHELINE_FREE                0xFE0FFFFF
#define   S_009838_EARLY_Z_PANIC_DISABLE(x)            (((unsigned)(x) & 0x1) << 25)
#define   G_009838_EARLY_Z_PANIC_DISABLE(x)            (((x) >> 25) & 0x1)
#define   C_009838_EARLY_Z_PANIC_DISABLE               0xFDFFFFFF
#define   S_009838_LATE_Z_PANIC_DISABLE(x)             (((unsigned)(x) & 0x1) << 26)
#define   G_009838_LATE_Z_PANIC_DISABLE(x)             (((x) >> 26) & 0x1)
#define   C_009838_LATE_Z_PANIC_DISABLE                0xFBFFFFFF
#define   S_009838_RE_Z_PANIC_DISABLE(x)               (((unsigned)(x) & 0x1) << 27)
#define   G_009838_RE_Z_PANIC_DISABLE(x)               (((x) >> 27) & 0x1)
#define   C_009838_RE_Z_PANIC_DISABLE                  0xF7FFFFFF
#define   S_009838_DB_EXTRA_DEBUG(x)                   (((unsigned)(x) & 0xF) << 28)
#define   G_009838_DB_EXTRA_DEBUG(x)                   (((x) >> 28) & 0xF)
#define   C_009838_DB_EXTRA_DEBUG                      0x0FFFFFFF
#define R_028030_PA_SC_SCREEN_SCISSOR_TL             0x028030
#define   S_028030_TL_X(x)                             (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028030_TL_X(x)                             (((x) >> 0) & 0x7FFF)
#define   C_028030_TL_X                                0xFFFF8000
#define   S_028030_TL_Y(x)                             (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028030_TL_Y(x)                             (((x) >> 16) & 0x7FFF)
#define   C_028030_TL_Y                                0x8000FFFF
#define R_028034_PA_SC_SCREEN_SCISSOR_BR             0x028034
#define   S_028034_BR_X(x)                             (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028034_BR_X(x)                             (((x) >> 0) & 0x7FFF)
#define   C_028034_BR_X                                0xFFFF8000
#define   S_028034_BR_Y(x)                             (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028034_BR_Y(x)                             (((x) >> 16) & 0x7FFF)
#define   C_028034_BR_Y                                0x8000FFFF
#define R_028200_PA_SC_WINDOW_OFFSET                 0x028200
#define   S_028200_WINDOW_X_OFFSET(x)                  (((unsigned)(x) & 0x7FFF) << 0)
#define   G_028200_WINDOW_X_OFFSET(x)                  (((x) >> 0) & 0x7FFF)
#define   C_028200_WINDOW_X_OFFSET                     0xFFFF8000
#define   S_028200_WINDOW_Y_OFFSET(x)                  (((unsigned)(x) & 0x7FFF) << 16)
#define   G_028200_WINDOW_Y_OFFSET(x)                  (((x) >> 16) & 0x7FFF)
#define   C_028200_WINDOW_Y_OFFSET                     0x8000FFFF
#define R_028204_PA_SC_WINDOW_SCISSOR_TL             0x028204
#define   S_028204_TL_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028204_TL_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028204_TL_X                                0xFFFFC000
#define   S_028204_TL_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028204_TL_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028204_TL_Y                                0xC000FFFF
#define   S_028204_WINDOW_OFFSET_DISABLE(x)            (((unsigned)(x) & 0x1) << 31)
#define   G_028204_WINDOW_OFFSET_DISABLE(x)            (((x) >> 31) & 0x1)
#define   C_028204_WINDOW_OFFSET_DISABLE               0x7FFFFFFF
#define R_028208_PA_SC_WINDOW_SCISSOR_BR             0x028208
#define   S_028208_BR_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028208_BR_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028208_BR_X                                0xFFFFC000
#define   S_028208_BR_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028208_BR_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028208_BR_Y                                0xC000FFFF
#define R_02820C_PA_SC_CLIPRECT_RULE                 0x02820C
#define   S_02820C_CLIP_RULE(x)                        (((unsigned)(x) & 0xFFFF) << 0)
#define   G_02820C_CLIP_RULE(x)                        (((x) >> 0) & 0xFFFF)
#define   C_02820C_CLIP_RULE                           0xFFFF0000
#define R_028210_PA_SC_CLIPRECT_0_TL                 0x028210
#define   S_028210_TL_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028210_TL_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028210_TL_X                                0xFFFFC000
#define   S_028210_TL_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028210_TL_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028210_TL_Y                                0xC000FFFF
#define R_028214_PA_SC_CLIPRECT_0_BR                 0x028214
#define   S_028214_BR_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028214_BR_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028214_BR_X                                0xFFFFC000
#define   S_028214_BR_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028214_BR_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028214_BR_Y                                0xC000FFFF
#define R_028218_PA_SC_CLIPRECT_1_TL                 0x028218
#define R_02821C_PA_SC_CLIPRECT_1_BR                 0x02821C
#define R_028220_PA_SC_CLIPRECT_2_TL                 0x028220
#define R_028224_PA_SC_CLIPRECT_2_BR                 0x028224
#define R_028228_PA_SC_CLIPRECT_3_TL                 0x028228
#define R_02822C_PA_SC_CLIPRECT_3_BR                 0x02822C
#define R_028230_PA_SC_EDGERULE                      0x028230
#define R_028240_PA_SC_GENERIC_SCISSOR_TL            0x028240
#define   S_028240_TL_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028240_TL_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028240_TL_X                                0xFFFFC000
#define   S_028240_TL_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028240_TL_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028240_TL_Y                                0xC000FFFF
#define   S_028240_WINDOW_OFFSET_DISABLE(x)            (((unsigned)(x) & 0x1) << 31)
#define   G_028240_WINDOW_OFFSET_DISABLE(x)            (((x) >> 31) & 0x1)
#define   C_028240_WINDOW_OFFSET_DISABLE               0x7FFFFFFF
#define R_028244_PA_SC_GENERIC_SCISSOR_BR            0x028244
#define   S_028244_BR_X(x)                             (((unsigned)(x) & 0x3FFF) << 0)
#define   G_028244_BR_X(x)                             (((x) >> 0) & 0x3FFF)
#define   C_028244_BR_X                                0xFFFFC000
#define   S_028244_BR_Y(x)                             (((unsigned)(x) & 0x3FFF) << 16)
#define   G_028244_BR_Y(x)                             (((x) >> 16) & 0x3FFF)
#define   C_028244_BR_Y                                0xC000FFFF
#define R_0282D0_PA_SC_VPORT_ZMIN_0                  0x0282D0
#define   S_0282D0_VPORT_ZMIN(x)                       (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_0282D0_VPORT_ZMIN(x)                       (((x) >> 0) & 0xFFFFFFFF)
#define   C_0282D0_VPORT_ZMIN                          0x00000000
#define R_0282D4_PA_SC_VPORT_ZMAX_0                  0x0282D4
#define   S_0282D4_VPORT_ZMAX(x)                       (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_0282D4_VPORT_ZMAX(x)                       (((x) >> 0) & 0xFFFFFFFF)
#define   C_0282D4_VPORT_ZMAX                          0x00000000
#define R_028350_SX_MISC                             0x028350
#define   S_028350_MULTIPASS(x)                        (((unsigned)(x) & 0x1) << 0)
#define   G_028350_MULTIPASS(x)                        (((x) >> 0) & 0x1)
#define   C_028350_MULTIPASS                           0xFFFFFFFE
#define R_028354_SX_SURFACE_SYNC                     0x028354
#define   S_028354_SURFACE_SYNC_MASK(x)                (((unsigned)(x) & 0x1FF) << 0)
#define R_028380_SQ_VTX_SEMANTIC_0                   0x028380
#define   S_028380_SEMANTIC_ID(x)                      (((unsigned)(x) & 0xFF) << 0)
#define   G_028380_SEMANTIC_ID(x)                      (((x) >> 0) & 0xFF)
#define   C_028380_SEMANTIC_ID                         0xFFFFFF00
#define R_028384_SQ_VTX_SEMANTIC_1                   0x028384
#define R_028388_SQ_VTX_SEMANTIC_2                   0x028388
#define R_02838C_SQ_VTX_SEMANTIC_3                   0x02838C
#define R_028390_SQ_VTX_SEMANTIC_4                   0x028390
#define R_028394_SQ_VTX_SEMANTIC_5                   0x028394
#define R_028398_SQ_VTX_SEMANTIC_6                   0x028398
#define R_02839C_SQ_VTX_SEMANTIC_7                   0x02839C
#define R_0283A0_SQ_VTX_SEMANTIC_8                   0x0283A0
#define R_0283A4_SQ_VTX_SEMANTIC_9                   0x0283A4
#define R_0283A8_SQ_VTX_SEMANTIC_10                  0x0283A8
#define R_0283AC_SQ_VTX_SEMANTIC_11                  0x0283AC
#define R_0283B0_SQ_VTX_SEMANTIC_12                  0x0283B0
#define R_0283B4_SQ_VTX_SEMANTIC_13                  0x0283B4
#define R_0283B8_SQ_VTX_SEMANTIC_14                  0x0283B8
#define R_0283BC_SQ_VTX_SEMANTIC_15                  0x0283BC
#define R_0283C0_SQ_VTX_SEMANTIC_16                  0x0283C0
#define R_0283C4_SQ_VTX_SEMANTIC_17                  0x0283C4
#define R_0283C8_SQ_VTX_SEMANTIC_18                  0x0283C8
#define R_0283CC_SQ_VTX_SEMANTIC_19                  0x0283CC
#define R_0283D0_SQ_VTX_SEMANTIC_20                  0x0283D0
#define R_0283D4_SQ_VTX_SEMANTIC_21                  0x0283D4
#define R_0283D8_SQ_VTX_SEMANTIC_22                  0x0283D8
#define R_0283DC_SQ_VTX_SEMANTIC_23                  0x0283DC
#define R_0283E0_SQ_VTX_SEMANTIC_24                  0x0283E0
#define R_0283E4_SQ_VTX_SEMANTIC_25                  0x0283E4
#define R_0283E8_SQ_VTX_SEMANTIC_26                  0x0283E8
#define R_0283EC_SQ_VTX_SEMANTIC_27                  0x0283EC
#define R_0283F0_SQ_VTX_SEMANTIC_28                  0x0283F0
#define R_0283F4_SQ_VTX_SEMANTIC_29                  0x0283F4
#define R_0283F8_SQ_VTX_SEMANTIC_30                  0x0283F8
#define R_0283FC_SQ_VTX_SEMANTIC_31                  0x0283FC
#define R_0288C8_SQ_GS_VERT_ITEMSIZE                 0x0288C8
#define R_0288E0_SQ_VTX_SEMANTIC_CLEAR               0x0288E0
#define R_028400_VGT_MAX_VTX_INDX                    0x028400
#define   S_028400_MAX_INDX(x)                         (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028400_MAX_INDX(x)                         (((x) >> 0) & 0xFFFFFFFF)
#define   C_028400_MAX_INDX                            0x00000000
#define R_028404_VGT_MIN_VTX_INDX                    0x028404
#define   S_028404_MIN_INDX(x)                         (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028404_MIN_INDX(x)                         (((x) >> 0) & 0xFFFFFFFF)
#define   C_028404_MIN_INDX                            0x00000000
#define R_028408_VGT_INDX_OFFSET                     0x028408
#define   S_028408_INDX_OFFSET(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028408_INDX_OFFSET(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_028408_INDX_OFFSET                         0x00000000
#define R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX        0x02840C
#define   S_02840C_RESET_INDX(x)                       (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_02840C_RESET_INDX(x)                       (((x) >> 0) & 0xFFFFFFFF)
#define   C_02840C_RESET_INDX                          0x00000000
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
#define R_028414_CB_BLEND_RED                        0x028414
#define   S_028414_BLEND_RED(x)                        (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028414_BLEND_RED(x)                        (((x) >> 0) & 0xFFFFFFFF)
#define   C_028414_BLEND_RED                           0x00000000
#define R_028418_CB_BLEND_GREEN                      0x028418
#define   S_028418_BLEND_GREEN(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028418_BLEND_GREEN(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_028418_BLEND_GREEN                         0x00000000
#define R_02841C_CB_BLEND_BLUE                       0x02841C
#define   S_02841C_BLEND_BLUE(x)                       (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_02841C_BLEND_BLUE(x)                       (((x) >> 0) & 0xFFFFFFFF)
#define   C_02841C_BLEND_BLUE                          0x00000000
#define R_028420_CB_BLEND_ALPHA                      0x028420
#define   S_028420_BLEND_ALPHA(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028420_BLEND_ALPHA(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_028420_BLEND_ALPHA                         0x00000000
#define R_028438_SX_ALPHA_REF                        0x028438
#define   S_028438_ALPHA_REF(x)                        (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028438_ALPHA_REF(x)                        (((x) >> 0) & 0xFFFFFFFF)
#define   C_028438_ALPHA_REF                           0x00000000
#define R_0286C8_SPI_THREAD_GROUPING                 0x0286C8
#define   S_0286C8_PS_GROUPING(x)                      (((unsigned)(x) & 0x1F) << 0)
#define   G_0286C8_PS_GROUPING(x)                      (((x) >> 0) & 0x1F)
#define   C_0286C8_PS_GROUPING                         0xFFFFFFE0
#define   S_0286C8_VS_GROUPING(x)                      (((unsigned)(x) & 0x1F) << 8)
#define   G_0286C8_VS_GROUPING(x)                      (((x) >> 8) & 0x1F)
#define   C_0286C8_VS_GROUPING                         0xFFFFE0FF
#define   S_0286C8_GS_GROUPING(x)                      (((unsigned)(x) & 0x1F) << 16)
#define   G_0286C8_GS_GROUPING(x)                      (((x) >> 16) & 0x1F)
#define   C_0286C8_GS_GROUPING                         0xFFE0FFFF
#define   S_0286C8_ES_GROUPING(x)                      (((unsigned)(x) & 0x1F) << 24)
#define   G_0286C8_ES_GROUPING(x)                      (((x) >> 24) & 0x1F)
#define   C_0286C8_ES_GROUPING                         0xE0FFFFFF
#define R_0286D8_SPI_INPUT_Z                         0x0286D8
#define   S_0286D8_PROVIDE_Z_TO_SPI(x)                 (((unsigned)(x) & 0x1) << 0)
#define   G_0286D8_PROVIDE_Z_TO_SPI(x)                 (((x) >> 0) & 0x1)
#define   C_0286D8_PROVIDE_Z_TO_SPI                    0xFFFFFFFE
#define R_0286DC_SPI_FOG_CNTL                        0x0286DC
#define   S_0286DC_PASS_FOG_THROUGH_PS(x)              (((unsigned)(x) & 0x1) << 0)
#define   G_0286DC_PASS_FOG_THROUGH_PS(x)              (((x) >> 0) & 0x1)
#define   C_0286DC_PASS_FOG_THROUGH_PS                 0xFFFFFFFE
#define   S_0286DC_PIXEL_FOG_FUNC(x)                   (((unsigned)(x) & 0x3) << 1)
#define   G_0286DC_PIXEL_FOG_FUNC(x)                   (((x) >> 1) & 0x3)
#define   C_0286DC_PIXEL_FOG_FUNC                      0xFFFFFFF9
#define   S_0286DC_PIXEL_FOG_SRC_SEL(x)                (((unsigned)(x) & 0x1) << 3)
#define   G_0286DC_PIXEL_FOG_SRC_SEL(x)                (((x) >> 3) & 0x1)
#define   C_0286DC_PIXEL_FOG_SRC_SEL                   0xFFFFFFF7
#define   S_0286DC_VS_FOG_CLAMP_DISABLE(x)             (((unsigned)(x) & 0x1) << 4)
#define   G_0286DC_VS_FOG_CLAMP_DISABLE(x)             (((x) >> 4) & 0x1)
#define   C_0286DC_VS_FOG_CLAMP_DISABLE                0xFFFFFFEF
#define R_0286E0_SPI_FOG_FUNC_SCALE                  0x0286E0
#define   S_0286E0_VALUE(x)                            (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_0286E0_VALUE(x)                            (((x) >> 0) & 0xFFFFFFFF)
#define   C_0286E0_VALUE                               0x00000000
#define R_0286E4_SPI_FOG_FUNC_BIAS                   0x0286E4
#define   S_0286E4_VALUE(x)                            (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_0286E4_VALUE(x)                            (((x) >> 0) & 0xFFFFFFFF)
#define   C_0286E4_VALUE                               0x00000000
#define R_0287A0_CB_SHADER_CONTROL                   0x0287A0
#define   S_0287A0_RT0_ENABLE(x)                       (((unsigned)(x) & 0x1) << 0)
#define   G_0287A0_RT0_ENABLE(x)                       (((x) >> 0) & 0x1)
#define   C_0287A0_RT0_ENABLE                          0xFFFFFFFE
#define   S_0287A0_RT1_ENABLE(x)                       (((unsigned)(x) & 0x1) << 1)
#define   G_0287A0_RT1_ENABLE(x)                       (((x) >> 1) & 0x1)
#define   C_0287A0_RT1_ENABLE                          0xFFFFFFFD
#define   S_0287A0_RT2_ENABLE(x)                       (((unsigned)(x) & 0x1) << 2)
#define   G_0287A0_RT2_ENABLE(x)                       (((x) >> 2) & 0x1)
#define   C_0287A0_RT2_ENABLE                          0xFFFFFFFB
#define   S_0287A0_RT3_ENABLE(x)                       (((unsigned)(x) & 0x1) << 3)
#define   G_0287A0_RT3_ENABLE(x)                       (((x) >> 3) & 0x1)
#define   C_0287A0_RT3_ENABLE                          0xFFFFFFF7
#define   S_0287A0_RT4_ENABLE(x)                       (((unsigned)(x) & 0x1) << 4)
#define   G_0287A0_RT4_ENABLE(x)                       (((x) >> 4) & 0x1)
#define   C_0287A0_RT4_ENABLE                          0xFFFFFFEF
#define   S_0287A0_RT5_ENABLE(x)                       (((unsigned)(x) & 0x1) << 5)
#define   G_0287A0_RT5_ENABLE(x)                       (((x) >> 5) & 0x1)
#define   C_0287A0_RT5_ENABLE                          0xFFFFFFDF
#define   S_0287A0_RT6_ENABLE(x)                       (((unsigned)(x) & 0x1) << 6)
#define   G_0287A0_RT6_ENABLE(x)                       (((x) >> 6) & 0x1)
#define   C_0287A0_RT6_ENABLE                          0xFFFFFFBF
#define   S_0287A0_RT7_ENABLE(x)                       (((unsigned)(x) & 0x1) << 7)
#define   G_0287A0_RT7_ENABLE(x)                       (((x) >> 7) & 0x1)
#define   C_0287A0_RT7_ENABLE                          0xFFFFFF7F
#define R_028894_SQ_PGM_START_FS                     0x028894
#define   S_028894_PGM_START(x)                        (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028894_PGM_START(x)                        (((x) >> 0) & 0xFFFFFFFF)
#define   C_028894_PGM_START                           0x00000000
#define R_0288A4_SQ_PGM_RESOURCES_FS                 0x0288A4
#define   S_0288A4_NUM_GPRS(x)                         (((unsigned)(x) & 0xFF) << 0)
#define   G_0288A4_NUM_GPRS(x)                         (((x) >> 0) & 0xFF)
#define   C_0288A4_NUM_GPRS                            0xFFFFFF00
#define   S_0288A4_STACK_SIZE(x)                       (((unsigned)(x) & 0xFF) << 8)
#define   G_0288A4_STACK_SIZE(x)                       (((x) >> 8) & 0xFF)
#define   C_0288A4_STACK_SIZE                          0xFFFF00FF
#define   S_0288A4_DX10_CLAMP(x)                       (((unsigned)(x) & 0x1) << 21)
#define   G_0288A4_DX10_CLAMP(x)                       (((x) >> 21) & 0x1)
#define   C_0288A4_DX10_CLAMP                          0xFFDFFFFF
#define R_0288A8_SQ_ESGS_RING_ITEMSIZE               0x0288A8
#define   S_0288A8_ITEMSIZE(x)                         (((unsigned)(x) & 0x7FFF) << 0)
#define   G_0288A8_ITEMSIZE(x)                         (((x) >> 0) & 0x7FFF)
#define   C_0288A8_ITEMSIZE                            0xFFFF8000
#define R_0288AC_SQ_GSVS_RING_ITEMSIZE               0x0288AC
#define   S_0288AC_ITEMSIZE(x)                         (((unsigned)(x) & 0x7FFF) << 0)
#define   G_0288AC_ITEMSIZE(x)                         (((x) >> 0) & 0x7FFF)
#define   C_0288AC_ITEMSIZE                            0xFFFF8000
#define R_0288B0_SQ_ESTMP_RING_ITEMSIZE              0x0288B0
#define   S_0288B0_ITEMSIZE(x)                         (((unsigned)(x) & 0x7FFF) << 0)
#define   G_0288B0_ITEMSIZE(x)                         (((x) >> 0) & 0x7FFF)
#define   C_0288B0_ITEMSIZE                            0xFFFF8000
#define R_0288B4_SQ_GSTMP_RING_ITEMSIZE              0x0288B4
#define   S_0288B4_ITEMSIZE(x)                         (((unsigned)(x) & 0x7FFF) << 0)
#define   G_0288B4_ITEMSIZE(x)                         (((x) >> 0) & 0x7FFF)
#define   C_0288B4_ITEMSIZE                            0xFFFF8000
#define R_0288B8_SQ_VSTMP_RING_ITEMSIZE              0x0288B8
#define   S_0288B8_ITEMSIZE(x)                         (((unsigned)(x) & 0x7FFF) << 0)
#define   G_0288B8_ITEMSIZE(x)                         (((x) >> 0) & 0x7FFF)
#define   C_0288B8_ITEMSIZE                            0xFFFF8000
#define R_0288BC_SQ_PSTMP_RING_ITEMSIZE              0x0288BC
#define   S_0288BC_ITEMSIZE(x)                         (((unsigned)(x) & 0x7FFF) << 0)
#define   G_0288BC_ITEMSIZE(x)                         (((x) >> 0) & 0x7FFF)
#define   C_0288BC_ITEMSIZE                            0xFFFF8000
#define R_0288C0_SQ_FBUF_RING_ITEMSIZE               0x0288C0
#define   S_0288C0_ITEMSIZE(x)                         (((unsigned)(x) & 0x7FFF) << 0)
#define   G_0288C0_ITEMSIZE(x)                         (((x) >> 0) & 0x7FFF)
#define   C_0288C0_ITEMSIZE                            0xFFFF8000
#define R_0288C4_SQ_REDUC_RING_ITEMSIZE              0x0288C4
#define   S_0288C4_ITEMSIZE(x)                         (((unsigned)(x) & 0x7FFF) << 0)
#define   G_0288C4_ITEMSIZE(x)                         (((x) >> 0) & 0x7FFF)
#define   C_0288C4_ITEMSIZE                            0xFFFF8000
#define R_0288C8_SQ_GS_VERT_ITEMSIZE                 0x0288C8
#define   S_0288C8_ITEMSIZE(x)                         (((unsigned)(x) & 0x7FFF) << 0)
#define   G_0288C8_ITEMSIZE(x)                         (((x) >> 0) & 0x7FFF)
#define   C_0288C8_ITEMSIZE                            0xFFFF8000
#define R_0288DC_SQ_PGM_CF_OFFSET_FS                 0x0288DC
#define   S_0288DC_PGM_CF_OFFSET(x)                    (((unsigned)(x) & 0xFFFFF) << 0)
#define   G_0288DC_PGM_CF_OFFSET(x)                    (((x) >> 0) & 0xFFFFF)
#define   C_0288DC_PGM_CF_OFFSET                       0xFFF00000
#define R_028A10_VGT_OUTPUT_PATH_CNTL                0x028A10
#define   S_028A10_PATH_SELECT(x)                      (((unsigned)(x) & 0x3) << 0)
#define   G_028A10_PATH_SELECT(x)                      (((x) >> 0) & 0x3)
#define   C_028A10_PATH_SELECT                         0xFFFFFFFC
#define R_028A14_VGT_HOS_CNTL                        0x028A14
#define   S_028A14_TESS_MODE(x)                        (((unsigned)(x) & 0x3) << 0)
#define   G_028A14_TESS_MODE(x)                        (((x) >> 0) & 0x3)
#define   C_028A14_TESS_MODE                           0xFFFFFFFC
#define R_028A18_VGT_HOS_MAX_TESS_LEVEL              0x028A18
#define   S_028A18_MAX_TESS(x)                         (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028A18_MAX_TESS(x)                         (((x) >> 0) & 0xFFFFFFFF)
#define   C_028A18_MAX_TESS                            0x00000000
#define R_028A1C_VGT_HOS_MIN_TESS_LEVEL              0x028A1C
#define   S_028A1C_MIN_TESS(x)                         (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028A1C_MIN_TESS(x)                         (((x) >> 0) & 0xFFFFFFFF)
#define   C_028A1C_MIN_TESS                            0x00000000
#define R_028A20_VGT_HOS_REUSE_DEPTH                 0x028A20
#define   S_028A20_REUSE_DEPTH(x)                      (((unsigned)(x) & 0xFF) << 0)
#define   G_028A20_REUSE_DEPTH(x)                      (((x) >> 0) & 0xFF)
#define   C_028A20_REUSE_DEPTH                         0xFFFFFF00
#define R_028A24_VGT_GROUP_PRIM_TYPE                 0x028A24
#define   S_028A24_PRIM_TYPE(x)                        (((unsigned)(x) & 0x1F) << 0)
#define   G_028A24_PRIM_TYPE(x)                        (((x) >> 0) & 0x1F)
#define   C_028A24_PRIM_TYPE                           0xFFFFFFE0
#define   S_028A24_RETAIN_ORDER(x)                     (((unsigned)(x) & 0x1) << 14)
#define   G_028A24_RETAIN_ORDER(x)                     (((x) >> 14) & 0x1)
#define   C_028A24_RETAIN_ORDER                        0xFFFFBFFF
#define   S_028A24_RETAIN_QUADS(x)                     (((unsigned)(x) & 0x1) << 15)
#define   G_028A24_RETAIN_QUADS(x)                     (((x) >> 15) & 0x1)
#define   C_028A24_RETAIN_QUADS                        0xFFFF7FFF
#define   S_028A24_PRIM_ORDER(x)                       (((unsigned)(x) & 0x7) << 16)
#define   G_028A24_PRIM_ORDER(x)                       (((x) >> 16) & 0x7)
#define   C_028A24_PRIM_ORDER                          0xFFF8FFFF
#define R_028A28_VGT_GROUP_FIRST_DECR                0x028A28
#define   S_028A28_FIRST_DECR(x)                       (((unsigned)(x) & 0xF) << 0)
#define   G_028A28_FIRST_DECR(x)                       (((x) >> 0) & 0xF)
#define   C_028A28_FIRST_DECR                          0xFFFFFFF0
#define R_028A2C_VGT_GROUP_DECR                      0x028A2C
#define   S_028A2C_DECR(x)                             (((unsigned)(x) & 0xF) << 0)
#define   G_028A2C_DECR(x)                             (((x) >> 0) & 0xF)
#define   C_028A2C_DECR                                0xFFFFFFF0
#define R_028A30_VGT_GROUP_VECT_0_CNTL               0x028A30
#define   S_028A30_COMP_X_EN(x)                        (((unsigned)(x) & 0x1) << 0)
#define   G_028A30_COMP_X_EN(x)                        (((x) >> 0) & 0x1)
#define   C_028A30_COMP_X_EN                           0xFFFFFFFE
#define   S_028A30_COMP_Y_EN(x)                        (((unsigned)(x) & 0x1) << 1)
#define   G_028A30_COMP_Y_EN(x)                        (((x) >> 1) & 0x1)
#define   C_028A30_COMP_Y_EN                           0xFFFFFFFD
#define   S_028A30_COMP_Z_EN(x)                        (((unsigned)(x) & 0x1) << 2)
#define   G_028A30_COMP_Z_EN(x)                        (((x) >> 2) & 0x1)
#define   C_028A30_COMP_Z_EN                           0xFFFFFFFB
#define   S_028A30_COMP_W_EN(x)                        (((unsigned)(x) & 0x1) << 3)
#define   G_028A30_COMP_W_EN(x)                        (((x) >> 3) & 0x1)
#define   C_028A30_COMP_W_EN                           0xFFFFFFF7
#define   S_028A30_STRIDE(x)                           (((unsigned)(x) & 0xFF) << 8)
#define   G_028A30_STRIDE(x)                           (((x) >> 8) & 0xFF)
#define   C_028A30_STRIDE                              0xFFFF00FF
#define   S_028A30_SHIFT(x)                            (((unsigned)(x) & 0xFF) << 16)
#define   G_028A30_SHIFT(x)                            (((x) >> 16) & 0xFF)
#define   C_028A30_SHIFT                               0xFF00FFFF
#define R_028A34_VGT_GROUP_VECT_1_CNTL               0x028A34
#define   S_028A34_COMP_X_EN(x)                        (((unsigned)(x) & 0x1) << 0)
#define   G_028A34_COMP_X_EN(x)                        (((x) >> 0) & 0x1)
#define   C_028A34_COMP_X_EN                           0xFFFFFFFE
#define   S_028A34_COMP_Y_EN(x)                        (((unsigned)(x) & 0x1) << 1)
#define   G_028A34_COMP_Y_EN(x)                        (((x) >> 1) & 0x1)
#define   C_028A34_COMP_Y_EN                           0xFFFFFFFD
#define   S_028A34_COMP_Z_EN(x)                        (((unsigned)(x) & 0x1) << 2)
#define   G_028A34_COMP_Z_EN(x)                        (((x) >> 2) & 0x1)
#define   C_028A34_COMP_Z_EN                           0xFFFFFFFB
#define   S_028A34_COMP_W_EN(x)                        (((unsigned)(x) & 0x1) << 3)
#define   G_028A34_COMP_W_EN(x)                        (((x) >> 3) & 0x1)
#define   C_028A34_COMP_W_EN                           0xFFFFFFF7
#define   S_028A34_STRIDE(x)                           (((unsigned)(x) & 0xFF) << 8)
#define   G_028A34_STRIDE(x)                           (((x) >> 8) & 0xFF)
#define   C_028A34_STRIDE                              0xFFFF00FF
#define   S_028A34_SHIFT(x)                            (((unsigned)(x) & 0xFF) << 16)
#define   G_028A34_SHIFT(x)                            (((x) >> 16) & 0xFF)
#define   C_028A34_SHIFT                               0xFF00FFFF
#define R_028A38_VGT_GROUP_VECT_0_FMT_CNTL           0x028A38
#define   S_028A38_X_CONV(x)                           (((unsigned)(x) & 0xF) << 0)
#define   G_028A38_X_CONV(x)                           (((x) >> 0) & 0xF)
#define   C_028A38_X_CONV                              0xFFFFFFF0
#define   S_028A38_X_OFFSET(x)                         (((unsigned)(x) & 0xF) << 4)
#define   G_028A38_X_OFFSET(x)                         (((x) >> 4) & 0xF)
#define   C_028A38_X_OFFSET                            0xFFFFFF0F
#define   S_028A38_Y_CONV(x)                           (((unsigned)(x) & 0xF) << 8)
#define   G_028A38_Y_CONV(x)                           (((x) >> 8) & 0xF)
#define   C_028A38_Y_CONV                              0xFFFFF0FF
#define   S_028A38_Y_OFFSET(x)                         (((unsigned)(x) & 0xF) << 12)
#define   G_028A38_Y_OFFSET(x)                         (((x) >> 12) & 0xF)
#define   C_028A38_Y_OFFSET                            0xFFFF0FFF
#define   S_028A38_Z_CONV(x)                           (((unsigned)(x) & 0xF) << 16)
#define   G_028A38_Z_CONV(x)                           (((x) >> 16) & 0xF)
#define   C_028A38_Z_CONV                              0xFFF0FFFF
#define   S_028A38_Z_OFFSET(x)                         (((unsigned)(x) & 0xF) << 20)
#define   G_028A38_Z_OFFSET(x)                         (((x) >> 20) & 0xF)
#define   C_028A38_Z_OFFSET                            0xFF0FFFFF
#define   S_028A38_W_CONV(x)                           (((unsigned)(x) & 0xF) << 24)
#define   G_028A38_W_CONV(x)                           (((x) >> 24) & 0xF)
#define   C_028A38_W_CONV                              0xF0FFFFFF
#define   S_028A38_W_OFFSET(x)                         (((unsigned)(x) & 0xF) << 28)
#define   G_028A38_W_OFFSET(x)                         (((x) >> 28) & 0xF)
#define   C_028A38_W_OFFSET                            0x0FFFFFFF
#define R_028A3C_VGT_GROUP_VECT_1_FMT_CNTL           0x028A3C
#define   S_028A3C_X_CONV(x)                           (((unsigned)(x) & 0xF) << 0)
#define   G_028A3C_X_CONV(x)                           (((x) >> 0) & 0xF)
#define   C_028A3C_X_CONV                              0xFFFFFFF0
#define   S_028A3C_X_OFFSET(x)                         (((unsigned)(x) & 0xF) << 4)
#define   G_028A3C_X_OFFSET(x)                         (((x) >> 4) & 0xF)
#define   C_028A3C_X_OFFSET                            0xFFFFFF0F
#define   S_028A3C_Y_CONV(x)                           (((unsigned)(x) & 0xF) << 8)
#define   G_028A3C_Y_CONV(x)                           (((x) >> 8) & 0xF)
#define   C_028A3C_Y_CONV                              0xFFFFF0FF
#define   S_028A3C_Y_OFFSET(x)                         (((unsigned)(x) & 0xF) << 12)
#define   G_028A3C_Y_OFFSET(x)                         (((x) >> 12) & 0xF)
#define   C_028A3C_Y_OFFSET                            0xFFFF0FFF
#define   S_028A3C_Z_CONV(x)                           (((unsigned)(x) & 0xF) << 16)
#define   G_028A3C_Z_CONV(x)                           (((x) >> 16) & 0xF)
#define   C_028A3C_Z_CONV                              0xFFF0FFFF
#define   S_028A3C_Z_OFFSET(x)                         (((unsigned)(x) & 0xF) << 20)
#define   G_028A3C_Z_OFFSET(x)                         (((x) >> 20) & 0xF)
#define   C_028A3C_Z_OFFSET                            0xFF0FFFFF
#define   S_028A3C_W_CONV(x)                           (((unsigned)(x) & 0xF) << 24)
#define   G_028A3C_W_CONV(x)                           (((x) >> 24) & 0xF)
#define   C_028A3C_W_CONV                              0xF0FFFFFF
#define   S_028A3C_W_OFFSET(x)                         (((unsigned)(x) & 0xF) << 28)
#define   G_028A3C_W_OFFSET(x)                         (((x) >> 28) & 0xF)
#define   C_028A3C_W_OFFSET                            0x0FFFFFFF
#define R_028A40_VGT_GS_MODE                         0x028A40
#define   S_028A40_MODE(x)                             (((unsigned)(x) & 0x3) << 0)
#define   G_028A40_MODE(x)                             (((x) >> 0) & 0x3)
#define   C_028A40_MODE                                0xFFFFFFFC
#define   S_028A40_ES_PASSTHRU(x)                      (((unsigned)(x) & 0x1) << 2)
#define   G_028A40_ES_PASSTHRU(x)                      (((x) >> 2) & 0x1)
#define   C_028A40_ES_PASSTHRU                         0xFFFFFFFB
#define   S_028A40_CUT_MODE(x)                         (((unsigned)(x) & 0x3) << 3)
#define   G_028A40_CUT_MODE(x)                         (((x) >> 3) & 0x3)
#define   C_028A40_CUT_MODE                            0xFFFFFFE7
#define R_028A4C_PA_SC_MODE_CNTL                     0x028A4C
#define   S_028A4C_MSAA_ENABLE(x)                      (((unsigned)(x) & 0x1) << 0)
#define   G_028A4C_MSAA_ENABLE(x)                      (((x) >> 0) & 0x1)
#define   C_028A4C_MSAA_ENABLE                         0xFFFFFFFE
#define   S_028A4C_CLIPRECT_ENABLE(x)                  (((unsigned)(x) & 0x1) << 1)
#define   G_028A4C_CLIPRECT_ENABLE(x)                  (((x) >> 1) & 0x1)
#define   C_028A4C_CLIPRECT_ENABLE                     0xFFFFFFFD
#define   S_028A4C_LINE_STIPPLE_ENABLE(x)              (((unsigned)(x) & 0x1) << 2)
#define   G_028A4C_LINE_STIPPLE_ENABLE(x)              (((x) >> 2) & 0x1)
#define   C_028A4C_LINE_STIPPLE_ENABLE                 0xFFFFFFFB
#define   S_028A4C_MULTI_CHIP_PRIM_DISCARD_ENAB(x)     (((unsigned)(x) & 0x1) << 3)
#define   G_028A4C_MULTI_CHIP_PRIM_DISCARD_ENAB(x)     (((x) >> 3) & 0x1)
#define   C_028A4C_MULTI_CHIP_PRIM_DISCARD_ENAB        0xFFFFFFF7
#define   S_028A4C_WALK_ORDER_ENABLE(x)                (((unsigned)(x) & 0x1) << 4)
#define   G_028A4C_WALK_ORDER_ENABLE(x)                (((x) >> 4) & 0x1)
#define   C_028A4C_WALK_ORDER_ENABLE                   0xFFFFFFEF
#define   S_028A4C_HALVE_DETAIL_SAMPLE_PERF(x)         (((unsigned)(x) & 0x1) << 5)
#define   G_028A4C_HALVE_DETAIL_SAMPLE_PERF(x)         (((x) >> 5) & 0x1)
#define   C_028A4C_HALVE_DETAIL_SAMPLE_PERF            0xFFFFFFDF
#define   S_028A4C_WALK_SIZE(x)                        (((unsigned)(x) & 0x1) << 6)
#define   G_028A4C_WALK_SIZE(x)                        (((x) >> 6) & 0x1)
#define   C_028A4C_WALK_SIZE                           0xFFFFFFBF
#define   S_028A4C_WALK_ALIGNMENT(x)                   (((unsigned)(x) & 0x1) << 7)
#define   G_028A4C_WALK_ALIGNMENT(x)                   (((x) >> 7) & 0x1)
#define   C_028A4C_WALK_ALIGNMENT                      0xFFFFFF7F
#define   S_028A4C_WALK_ALIGN8_PRIM_FITS_ST(x)         (((unsigned)(x) & 0x1) << 8)
#define   G_028A4C_WALK_ALIGN8_PRIM_FITS_ST(x)         (((x) >> 8) & 0x1)
#define   C_028A4C_WALK_ALIGN8_PRIM_FITS_ST            0xFFFFFEFF
#define   S_028A4C_TILE_COVER_NO_SCISSOR(x)            (((unsigned)(x) & 0x1) << 9)
#define   G_028A4C_TILE_COVER_NO_SCISSOR(x)            (((x) >> 9) & 0x1)
#define   C_028A4C_TILE_COVER_NO_SCISSOR               0xFFFFFDFF
#define   S_028A4C_KILL_PIX_POST_HI_Z(x)               (((unsigned)(x) & 0x1) << 10)
#define   G_028A4C_KILL_PIX_POST_HI_Z(x)               (((x) >> 10) & 0x1)
#define   C_028A4C_KILL_PIX_POST_HI_Z                  0xFFFFFBFF
#define   S_028A4C_KILL_PIX_POST_DETAIL_MASK(x)        (((unsigned)(x) & 0x1) << 11)
#define   G_028A4C_KILL_PIX_POST_DETAIL_MASK(x)        (((x) >> 11) & 0x1)
#define   C_028A4C_KILL_PIX_POST_DETAIL_MASK           0xFFFFF7FF
#define   S_028A4C_MULTI_CHIP_SUPERTILE_ENABLE(x)      (((unsigned)(x) & 0x1) << 12)
#define   G_028A4C_MULTI_CHIP_SUPERTILE_ENABLE(x)      (((x) >> 12) & 0x1)
#define   C_028A4C_MULTI_CHIP_SUPERTILE_ENABLE         0xFFFFEFFF
#define   S_028A4C_TILE_COVER_DISABLE(x)               (((unsigned)(x) & 0x1) << 13)
#define   G_028A4C_TILE_COVER_DISABLE(x)               (((x) >> 13) & 0x1)
#define   C_028A4C_TILE_COVER_DISABLE                  0xFFFFDFFF
#define   S_028A4C_FORCE_EOV_CNTDWN_ENABLE(x)          (((unsigned)(x) & 0x1) << 14)
#define   G_028A4C_FORCE_EOV_CNTDWN_ENABLE(x)          (((x) >> 14) & 0x1)
#define   C_028A4C_FORCE_EOV_CNTDWN_ENABLE             0xFFFFBFFF
#define   S_028A4C_FORCE_EOV_TILE_ENABLE(x)            (((unsigned)(x) & 0x1) << 15)
#define   G_028A4C_FORCE_EOV_TILE_ENABLE(x)            (((x) >> 15) & 0x1)
#define   C_028A4C_FORCE_EOV_TILE_ENABLE               0xFFFF7FFF
#define   S_028A4C_FORCE_EOV_REZ_ENABLE(x)             (((unsigned)(x) & 0x1) << 16)
#define   G_028A4C_FORCE_EOV_REZ_ENABLE(x)             (((x) >> 16) & 0x1)
#define   C_028A4C_FORCE_EOV_REZ_ENABLE                0xFFFEFFFF
#define   S_028A4C_PS_ITER_SAMPLE(x)                   (((unsigned)(x) & 0x1) << 17)
#define   G_028A4C_PS_ITER_SAMPLE(x)                   (((x) >> 17) & 0x1)
#define   C_028A4C_PS_ITER_SAMPLE                      0xFFFDFFFF
#define   S_028A4C_R700_ZMM_LINE_OFFSET(x)             (((unsigned)(x) & 0x1) << 20)
#define   S_028A4C_R700_VPORT_SCISSOR_ENABLE(x)        (((unsigned)(x) & 0x1) << 22)
#define R_028A84_VGT_PRIMITIVEID_EN                  0x028A84
#define   S_028A84_PRIMITIVEID_EN(x)                   (((unsigned)(x) & 0x1) << 0)
#define   G_028A84_PRIMITIVEID_EN(x)                   (((x) >> 0) & 0x1)
#define   C_028A84_PRIMITIVEID_EN                      0xFFFFFFFE
#define R_028A94_VGT_MULTI_PRIM_IB_RESET_EN          0x028A94
#define   S_028A94_RESET_EN(x)                         (((unsigned)(x) & 0x1) << 0)
#define   G_028A94_RESET_EN(x)                         (((x) >> 0) & 0x1)
#define   C_028A94_RESET_EN                            0xFFFFFFFE
#define R_028AA0_VGT_INSTANCE_STEP_RATE_0            0x028AA0
#define   S_028AA0_STEP_RATE(x)                        (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028AA0_STEP_RATE(x)                        (((x) >> 0) & 0xFFFFFFFF)
#define   C_028AA0_STEP_RATE                           0x00000000
#define R_028AA4_VGT_INSTANCE_STEP_RATE_1            0x028AA4
#define   S_028AA4_STEP_RATE(x)                        (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028AA4_STEP_RATE(x)                        (((x) >> 0) & 0xFFFFFFFF)
#define   C_028AA4_STEP_RATE                           0x00000000
#define R_028AB0_VGT_STRMOUT_EN                      0x028AB0
#define   S_028AB0_STREAMOUT(x)                        (((unsigned)(x) & 0x1) << 0)
#define   G_028AB0_STREAMOUT(x)                        (((x) >> 0) & 0x1)
#define   C_028AB0_STREAMOUT                           0xFFFFFFFE
#define R_028AB4_VGT_REUSE_OFF                       0x028AB4
#define   S_028AB4_REUSE_OFF(x)                        (((unsigned)(x) & 0x1) << 0)
#define   G_028AB4_REUSE_OFF(x)                        (((x) >> 0) & 0x1)
#define   C_028AB4_REUSE_OFF                           0xFFFFFFFE
#define R_028AB8_VGT_VTX_CNT_EN                      0x028AB8
#define   S_028AB8_VTX_CNT_EN(x)                       (((unsigned)(x) & 0x1) << 0)
#define   G_028AB8_VTX_CNT_EN(x)                       (((x) >> 0) & 0x1)
#define   C_028AB8_VTX_CNT_EN                          0xFFFFFFFE
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
#define R_028B20_VGT_STRMOUT_BUFFER_EN               0x028B20
#define   S_028B20_BUFFER_0_EN(x)                      (((unsigned)(x) & 0x1) << 0)
#define   G_028B20_BUFFER_0_EN(x)                      (((x) >> 0) & 0x1)
#define   C_028B20_BUFFER_0_EN                         0xFFFFFFFE
#define   S_028B20_BUFFER_1_EN(x)                      (((unsigned)(x) & 0x1) << 1)
#define   G_028B20_BUFFER_1_EN(x)                      (((x) >> 1) & 0x1)
#define   C_028B20_BUFFER_1_EN                         0xFFFFFFFD
#define   S_028B20_BUFFER_2_EN(x)                      (((unsigned)(x) & 0x1) << 2)
#define   G_028B20_BUFFER_2_EN(x)                      (((x) >> 2) & 0x1)
#define   C_028B20_BUFFER_2_EN                         0xFFFFFFFB
#define   S_028B20_BUFFER_3_EN(x)                      (((unsigned)(x) & 0x1) << 3)
#define   G_028B20_BUFFER_3_EN(x)                      (((x) >> 3) & 0x1)
#define   C_028B20_BUFFER_3_EN                         0xFFFFFFF7
#define R_028B28_VGT_STRMOUT_DRAW_OPAQUE_OFFSET	     0x028B28
#define R_028B2C_VGT_STRMOUT_DRAW_OPAQUE_BUFFER_FILLED_SIZE 0x028B2C
#define R_028B30_VGT_STRMOUT_DRAW_OPAQUE_VERTEX_STRIDE 0x028B30
#define R_028B38_VGT_GS_MAX_VERT_OUT                 0x028B38 /* r7xx */
#define   S_028B38_MAX_VERT_OUT(x)                      (((unsigned)(x) & 0x7FF) << 0)
#define R_028B44_VGT_STRMOUT_BASE_OFFSET_HI_0	     0x028B44
#define R_028B48_VGT_STRMOUT_BASE_OFFSET_HI_1	     0x028B48
#define R_028B4C_VGT_STRMOUT_BASE_OFFSET_HI_2	     0x028B4C
#define R_028B50_VGT_STRMOUT_BASE_OFFSET_HI_3	     0x028B50
#define R_028C20_PA_SC_AA_SAMPLE_LOCS_8S_WD1_MCTX    0x028C20
#define   S_028C20_S4_X(x)                             (((unsigned)(x) & 0xF) << 0)
#define   G_028C20_S4_X(x)                             (((x) >> 0) & 0xF)
#define   C_028C20_S4_X                                0xFFFFFFF0
#define   S_028C20_S4_Y(x)                             (((unsigned)(x) & 0xF) << 4)
#define   G_028C20_S4_Y(x)                             (((x) >> 4) & 0xF)
#define   C_028C20_S4_Y                                0xFFFFFF0F
#define   S_028C20_S5_X(x)                             (((unsigned)(x) & 0xF) << 8)
#define   G_028C20_S5_X(x)                             (((x) >> 8) & 0xF)
#define   C_028C20_S5_X                                0xFFFFF0FF
#define   S_028C20_S5_Y(x)                             (((unsigned)(x) & 0xF) << 12)
#define   G_028C20_S5_Y(x)                             (((x) >> 12) & 0xF)
#define   C_028C20_S5_Y                                0xFFFF0FFF
#define   S_028C20_S6_X(x)                             (((unsigned)(x) & 0xF) << 16)
#define   G_028C20_S6_X(x)                             (((x) >> 16) & 0xF)
#define   C_028C20_S6_X                                0xFFF0FFFF
#define   S_028C20_S6_Y(x)                             (((unsigned)(x) & 0xF) << 20)
#define   G_028C20_S6_Y(x)                             (((x) >> 20) & 0xF)
#define   C_028C20_S6_Y                                0xFF0FFFFF
#define   S_028C20_S7_X(x)                             (((unsigned)(x) & 0xF) << 24)
#define   G_028C20_S7_X(x)                             (((x) >> 24) & 0xF)
#define   C_028C20_S7_X                                0xF0FFFFFF
#define   S_028C20_S7_Y(x)                             (((unsigned)(x) & 0xF) << 28)
#define   G_028C20_S7_Y(x)                             (((x) >> 28) & 0xF)
#define   C_028C20_S7_Y                                0x0FFFFFFF
#define R_028C30_CB_CLRCMP_CONTROL                   0x028C30
#define   S_028C30_CLRCMP_FCN_SRC(x)                   (((unsigned)(x) & 0x7) << 0)
#define   G_028C30_CLRCMP_FCN_SRC(x)                   (((x) >> 0) & 0x7)
#define   C_028C30_CLRCMP_FCN_SRC                      0xFFFFFFF8
#define   S_028C30_CLRCMP_FCN_DST(x)                   (((unsigned)(x) & 0x7) << 8)
#define   G_028C30_CLRCMP_FCN_DST(x)                   (((x) >> 8) & 0x7)
#define   C_028C30_CLRCMP_FCN_DST                      0xFFFFF8FF
#define   S_028C30_CLRCMP_FCN_SEL(x)                   (((unsigned)(x) & 0x3) << 24)
#define   G_028C30_CLRCMP_FCN_SEL(x)                   (((x) >> 24) & 0x3)
#define   C_028C30_CLRCMP_FCN_SEL                      0xFCFFFFFF
#define R_028C34_CB_CLRCMP_SRC                       0x028C34
#define   S_028C34_CLRCMP_SRC(x)                       (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028C34_CLRCMP_SRC(x)                       (((x) >> 0) & 0xFFFFFFFF)
#define   C_028C34_CLRCMP_SRC                          0x00000000
#define R_028C38_CB_CLRCMP_DST                       0x028C38
#define   S_028C38_CLRCMP_DST(x)                       (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028C38_CLRCMP_DST(x)                       (((x) >> 0) & 0xFFFFFFFF)
#define   C_028C38_CLRCMP_DST                          0x00000000
#define R_028C3C_CB_CLRCMP_MSK                       0x028C3C
#define   S_028C3C_CLRCMP_MSK(x)                       (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028C3C_CLRCMP_MSK(x)                       (((x) >> 0) & 0xFFFFFFFF)
#define   C_028C3C_CLRCMP_MSK                          0x00000000
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
/* r600 only start */
#define   S_0085F0_CR_DEST_BASE_ENA(x)                 (((unsigned)(x) & 0x1) << 15)
#define   G_0085F0_CR_DEST_BASE_ENA(x)                 (((x) >> 15) & 0x1)
#define   C_0085F0_CR_DEST_BASE_ENA                    0xFFFF7FFF
/* r600 only end */
/* evergreen only start */
#define   S_0085F0_CB8_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 15)
#define   G_0085F0_CB8_DEST_BASE_ENA(x)                (((x) >> 15) & 0x1)
#define   S_0085F0_CB9_DEST_BASE_ENA(x)                (((unsigned)(x) & 0x1) << 16)
#define   G_0085F0_CB9_DEST_BASE_ENA(x)                (((x) >> 16) & 0x1)
#define   S_0085F0_CB10_DEST_BASE_ENA(x)               (((unsigned)(x) & 0x1) << 17)
#define   G_0085F0_CB10_DEST_BASE_ENA(x)               (((x) >> 17) & 0x1)
#define   S_0085F0_CB11_DEST_BASE_ENA(x)               (((unsigned)(x) & 0x1) << 18)
#define   G_0085F0_CB11_DEST_BASE_ENA(x)               (((x) >> 18) & 0x1)
/* evergreen only end */
/* evergreen and r7xx only */
#define   S_0085F0_FULL_CACHE_ENA(x)                   (((unsigned)(x) & 0x1) << 20)
#define   G_0085F0_FULL_CACHE_ENA(x)                   (((x) >> 20) & 0x1)
/* evergreen and r7xx only end */
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
#define R_0085FC_CP_COHER_STATUS                     0x0085FC


#define R_02812C_CB_CLEAR_ALPHA                      0x02812C
#define   S_02812C_CLEAR_ALPHA(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_02812C_CLEAR_ALPHA(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_02812C_CLEAR_ALPHA                         0x00000000
#define R_028128_CB_CLEAR_BLUE                       0x028128
#define   S_028128_CLEAR_BLUE(x)                       (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028128_CLEAR_BLUE(x)                       (((x) >> 0) & 0xFFFFFFFF)
#define   C_028128_CLEAR_BLUE                          0x00000000
#define R_028124_CB_CLEAR_GREEN                      0x028124
#define   S_028124_CLEAR_GREEN(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028124_CLEAR_GREEN(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_028124_CLEAR_GREEN                         0x00000000
#define R_028120_CB_CLEAR_RED                        0x028120
#define   S_028120_CLEAR_RED(x)                        (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028120_CLEAR_RED(x)                        (((x) >> 0) & 0xFFFFFFFF)
#define   C_028120_CLEAR_RED                           0x00000000
#define R_02842C_CB_FOG_BLUE                         0x02842C
#define   S_02842C_FOG_BLUE(x)                         (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_02842C_FOG_BLUE(x)                         (((x) >> 0) & 0xFFFFFFFF)
#define   C_02842C_FOG_BLUE                            0x00000000
#define R_028428_CB_FOG_GREEN                        0x028428
#define   S_028428_FOG_GREEN(x)                        (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028428_FOG_GREEN(x)                        (((x) >> 0) & 0xFFFFFFFF)
#define   C_028428_FOG_GREEN                           0x00000000
#define R_028424_CB_FOG_RED                          0x028424
#define   S_028424_FOG_RED(x)                          (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_028424_FOG_RED(x)                          (((x) >> 0) & 0xFFFFFFFF)
#define   C_028424_FOG_RED                             0x00000000
#define R_03C000_SQ_TEX_SAMPLER_WORD0_0              0x03C000
#define   S_03C000_CLAMP_X(x)                          (((unsigned)(x) & 0x7) << 0)
#define   G_03C000_CLAMP_X(x)                          (((x) >> 0) & 0x7)
#define   C_03C000_CLAMP_X                             0xFFFFFFF8
#define   S_03C000_CLAMP_Y(x)                          (((unsigned)(x) & 0x7) << 3)
#define   G_03C000_CLAMP_Y(x)                          (((x) >> 3) & 0x7)
#define   C_03C000_CLAMP_Y                             0xFFFFFFC7
#define   S_03C000_CLAMP_Z(x)                          (((unsigned)(x) & 0x7) << 6)
#define   G_03C000_CLAMP_Z(x)                          (((x) >> 6) & 0x7)
#define   C_03C000_CLAMP_Z                             0xFFFFFE3F
#define   S_03C000_XY_MAG_FILTER(x)                    (((unsigned)(x) & 0x7) << 9)
#define   G_03C000_XY_MAG_FILTER(x)                    (((x) >> 9) & 0x7)
#define   C_03C000_XY_MAG_FILTER                       0xFFFFF1FF
#define   S_03C000_XY_MIN_FILTER(x)                    (((unsigned)(x) & 0x7) << 12)
#define   G_03C000_XY_MIN_FILTER(x)                    (((x) >> 12) & 0x7)
#define   C_03C000_XY_MIN_FILTER                       0xFFFF8FFF
#define   S_03C000_Z_FILTER(x)                         (((unsigned)(x) & 0x3) << 15)
#define   G_03C000_Z_FILTER(x)                         (((x) >> 15) & 0x3)
#define   C_03C000_Z_FILTER                            0xFFFE7FFF
#define   S_03C000_MIP_FILTER(x)                       (((unsigned)(x) & 0x3) << 17)
#define   G_03C000_MIP_FILTER(x)                       (((x) >> 17) & 0x3)
#define   C_03C000_MIP_FILTER                          0xFFF9FFFF
#define   S_03C000_BORDER_COLOR_TYPE(x)                (((unsigned)(x) & 0x3) << 22)
#define   G_03C000_BORDER_COLOR_TYPE(x)                (((x) >> 22) & 0x3)
#define   C_03C000_BORDER_COLOR_TYPE                   0xFF3FFFFF
#define   S_03C000_POINT_SAMPLING_CLAMP(x)             (((unsigned)(x) & 0x1) << 24)
#define   G_03C000_POINT_SAMPLING_CLAMP(x)             (((x) >> 24) & 0x1)
#define   C_03C000_POINT_SAMPLING_CLAMP                0xFEFFFFFF
#define   S_03C000_TEX_ARRAY_OVERRIDE(x)               (((unsigned)(x) & 0x1) << 25)
#define   G_03C000_TEX_ARRAY_OVERRIDE(x)               (((x) >> 25) & 0x1)
#define   C_03C000_TEX_ARRAY_OVERRIDE                  0xFDFFFFFF
#define   S_03C000_DEPTH_COMPARE_FUNCTION(x)           (((unsigned)(x) & 0x7) << 26)
#define   G_03C000_DEPTH_COMPARE_FUNCTION(x)           (((x) >> 26) & 0x7)
#define   C_03C000_DEPTH_COMPARE_FUNCTION              0xE3FFFFFF
#define   S_03C000_CHROMA_KEY(x)                       (((unsigned)(x) & 0x3) << 29)
#define   G_03C000_CHROMA_KEY(x)                       (((x) >> 29) & 0x3)
#define   C_03C000_CHROMA_KEY                          0x9FFFFFFF
#define   S_03C000_LOD_USES_MINOR_AXIS(x)              (((unsigned)(x) & 0x1) << 31)
#define   G_03C000_LOD_USES_MINOR_AXIS(x)              (((x) >> 31) & 0x1)
#define   C_03C000_LOD_USES_MINOR_AXIS                 0x7FFFFFFF
#define R_03C004_SQ_TEX_SAMPLER_WORD1_0              0x03C004
#define   S_03C004_MIN_LOD(x)                          (((unsigned)(x) & 0x3FF) << 0)
#define   G_03C004_MIN_LOD(x)                          (((x) >> 0) & 0x3FF)
#define   C_03C004_MIN_LOD                             0xFFFFFC00
#define   S_03C004_MAX_LOD(x)                          (((unsigned)(x) & 0x3FF) << 10)
#define   G_03C004_MAX_LOD(x)                          (((x) >> 10) & 0x3FF)
#define   C_03C004_MAX_LOD                             0xFFF003FF
#define   S_03C004_LOD_BIAS(x)                         (((unsigned)(x) & 0xFFF) << 20)
#define   G_03C004_LOD_BIAS(x)                         (((x) >> 20) & 0xFFF)
#define   C_03C004_LOD_BIAS                            0x000FFFFF
#define R_03C008_SQ_TEX_SAMPLER_WORD2_0              0x03C008
#define   S_03C008_LOD_BIAS_SEC(x)                     (((unsigned)(x) & 0xFFF) << 0)
#define   G_03C008_LOD_BIAS_SEC(x)                     (((x) >> 0) & 0xFFF)
#define   C_03C008_LOD_BIAS_SEC                        0xFFFFF000
#define   S_03C008_MC_COORD_TRUNCATE(x)                (((unsigned)(x) & 0x1) << 12)
#define   G_03C008_MC_COORD_TRUNCATE(x)                (((x) >> 12) & 0x1)
#define   C_03C008_MC_COORD_TRUNCATE                   0xFFFFEFFF
#define   S_03C008_FORCE_DEGAMMA(x)                    (((unsigned)(x) & 0x1) << 13)
#define   G_03C008_FORCE_DEGAMMA(x)                    (((x) >> 13) & 0x1)
#define   C_03C008_FORCE_DEGAMMA                       0xFFFFDFFF
#define   S_03C008_HIGH_PRECISION_FILTER(x)            (((unsigned)(x) & 0x1) << 14)
#define   G_03C008_HIGH_PRECISION_FILTER(x)            (((x) >> 14) & 0x1)
#define   C_03C008_HIGH_PRECISION_FILTER               0xFFFFBFFF
#define   S_03C008_PERF_MIP(x)                         (((unsigned)(x) & 0x7) << 15)
#define   G_03C008_PERF_MIP(x)                         (((x) >> 15) & 0x7)
#define   C_03C008_PERF_MIP                            0xFFFC7FFF
#define   S_03C008_PERF_Z(x)                           (((unsigned)(x) & 0x3) << 18)
#define   G_03C008_PERF_Z(x)                           (((x) >> 18) & 0x3)
#define   C_03C008_PERF_Z                              0xFFF3FFFF
#define   S_03C008_FETCH_4(x)                          (((unsigned)(x) & 0x1) << 26)
#define   G_03C008_FETCH_4(x)                          (((x) >> 26) & 0x1)
#define   C_03C008_FETCH_4                             0xFBFFFFFF
#define   S_03C008_SAMPLE_IS_PCF(x)                    (((unsigned)(x) & 0x1) << 27)
#define   G_03C008_SAMPLE_IS_PCF(x)                    (((x) >> 27) & 0x1)
#define   C_03C008_SAMPLE_IS_PCF                       0xF7FFFFFF
#define   S_03C008_TYPE(x)                             (((unsigned)(x) & 0x1) << 31)
#define   G_03C008_TYPE(x)                             (((x) >> 31) & 0x1)
#define   C_03C008_TYPE                                0x7FFFFFFF
#define R_00A40C_TD_PS_SAMPLER0_BORDER_ALPHA         0x00A40C
#define   S_00A40C_BORDER_ALPHA(x)                     (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_00A40C_BORDER_ALPHA(x)                     (((x) >> 0) & 0xFFFFFFFF)
#define   C_00A40C_BORDER_ALPHA                        0x00000000
#define R_00A408_TD_PS_SAMPLER0_BORDER_BLUE          0x00A408
#define   S_00A408_BORDER_BLUE(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_00A408_BORDER_BLUE(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_00A408_BORDER_BLUE                         0x00000000
#define R_00A404_TD_PS_SAMPLER0_BORDER_GREEN         0x00A404
#define   S_00A404_BORDER_GREEN(x)                     (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_00A404_BORDER_GREEN(x)                     (((x) >> 0) & 0xFFFFFFFF)
#define   C_00A404_BORDER_GREEN                        0x00000000
#define R_00A400_TD_PS_SAMPLER0_BORDER_RED           0x00A400
#define   S_00A400_BORDER_RED(x)                       (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_00A400_BORDER_RED(x)                       (((x) >> 0) & 0xFFFFFFFF)
#define   C_00A400_BORDER_RED                          0x00000000
#define R_00A60C_TD_VS_SAMPLER0_BORDER_ALPHA         0x00A60C
#define   S_00A60C_BORDER_ALPHA(x)                     (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_00A60C_BORDER_ALPHA(x)                     (((x) >> 0) & 0xFFFFFFFF)
#define   C_00A60C_BORDER_ALPHA                        0x00000000
#define R_00A608_TD_VS_SAMPLER0_BORDER_BLUE          0x00A608
#define   S_00A608_BORDER_BLUE(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_00A608_BORDER_BLUE(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_00A608_BORDER_BLUE                         0x00000000
#define R_00A604_TD_VS_SAMPLER0_BORDER_GREEN         0x00A604
#define   S_00A604_BORDER_GREEN(x)                     (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_00A604_BORDER_GREEN(x)                     (((x) >> 0) & 0xFFFFFFFF)
#define   C_00A604_BORDER_GREEN                        0x00000000
#define R_00A600_TD_VS_SAMPLER0_BORDER_RED           0x00A600
#define   S_00A600_BORDER_RED(x)                       (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_00A600_BORDER_RED(x)                       (((x) >> 0) & 0xFFFFFFFF)
#define   C_00A600_BORDER_RED                          0x00000000
#define R_00A80C_TD_GS_SAMPLER0_BORDER_ALPHA         0x00A80C
#define   S_00A80C_BORDER_ALPHA(x)                     (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_00A80C_BORDER_ALPHA(x)                     (((x) >> 0) & 0xFFFFFFFF)
#define   C_00A80C_BORDER_ALPHA                        0x00000000
#define R_00A808_TD_GS_SAMPLER0_BORDER_BLUE          0x00A808
#define   S_00A808_BORDER_BLUE(x)                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_00A808_BORDER_BLUE(x)                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_00A808_BORDER_BLUE                         0x00000000
#define R_00A804_TD_GS_SAMPLER0_BORDER_GREEN         0x00A804
#define   S_00A804_BORDER_GREEN(x)                     (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_00A804_BORDER_GREEN(x)                     (((x) >> 0) & 0xFFFFFFFF)
#define   C_00A804_BORDER_GREEN                        0x00000000
#define R_00A800_TD_GS_SAMPLER0_BORDER_RED           0x00A800
#define   S_00A800_BORDER_RED(x)                       (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_00A800_BORDER_RED(x)                       (((x) >> 0) & 0xFFFFFFFF)
#define   C_00A800_BORDER_RED                          0x00000000
#define R_030000_SQ_ALU_CONSTANT0_0                  0x030000
#define   S_030000_X(x)                                (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_030000_X(x)                                (((x) >> 0) & 0xFFFFFFFF)
#define   C_030000_X                                   0x00000000
#define R_030004_SQ_ALU_CONSTANT1_0                  0x030004
#define   S_030004_Y(x)                                (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_030004_Y(x)                                (((x) >> 0) & 0xFFFFFFFF)
#define   C_030004_Y                                   0x00000000
#define R_030008_SQ_ALU_CONSTANT2_0                  0x030008
#define   S_030008_Z(x)                                (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_030008_Z(x)                                (((x) >> 0) & 0xFFFFFFFF)
#define   C_030008_Z                                   0x00000000
#define R_03000C_SQ_ALU_CONSTANT3_0                  0x03000C
#define   S_03000C_W(x)                                (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_03000C_W(x)                                (((x) >> 0) & 0xFFFFFFFF)
#define   C_03000C_W                                   0x00000000
#define R_0287E4_VGT_DMA_BASE_HI                     0x0287E4
#define R_0287E8_VGT_DMA_BASE                        0x0287E8
#define R_028E20_PA_CL_UCP0_X                        0x028E20
#define R_028E24_PA_CL_UCP0_Y                        0x028E24
#define R_028E28_PA_CL_UCP0_Z                        0x028E28
#define R_028E2C_PA_CL_UCP0_W                        0x028E2C
#define R_028E30_PA_CL_UCP1_X                        0x028E30
#define R_028E34_PA_CL_UCP1_Y                        0x028E34
#define R_028E38_PA_CL_UCP1_Z                        0x028E38
#define R_028E3C_PA_CL_UCP1_W                        0x028E3C
#define R_028E40_PA_CL_UCP2_X                        0x028E40
#define R_028E44_PA_CL_UCP2_Y                        0x028E44
#define R_028E48_PA_CL_UCP2_Z                        0x028E48
#define R_028E4C_PA_CL_UCP2_W                        0x028E4C
#define R_028E50_PA_CL_UCP3_X                        0x028E50
#define R_028E54_PA_CL_UCP3_Y                        0x028E54
#define R_028E58_PA_CL_UCP3_Z                        0x028E58
#define R_028E5C_PA_CL_UCP3_W                        0x028E5C
#define R_028E60_PA_CL_UCP4_X                        0x028E60
#define R_028E64_PA_CL_UCP4_Y                        0x028E64
#define R_028E68_PA_CL_UCP4_Z                        0x028E68
#define R_028E6C_PA_CL_UCP4_W                        0x028E6C
#define R_028E70_PA_CL_UCP5_X                        0x028E70
#define R_028E74_PA_CL_UCP5_Y                        0x028E74
#define R_028E78_PA_CL_UCP5_Z                        0x028E78
#define R_028E7C_PA_CL_UCP5_W                        0x028E7C
#define R_038000_RESOURCE0_WORD0                     0x038000
#define R_038004_RESOURCE0_WORD1                     0x038004
#define R_038008_RESOURCE0_WORD2                     0x038008
#define R_03800C_RESOURCE0_WORD3                     0x03800C
#define R_038010_RESOURCE0_WORD4                     0x038010
#define R_038014_RESOURCE0_WORD5                     0x038014
#define R_038018_RESOURCE0_WORD6                     0x038018

#define R_028140_ALU_CONST_BUFFER_SIZE_PS_0          0x00028140
#define R_028144_ALU_CONST_BUFFER_SIZE_PS_1          0x00028144
#define R_028180_ALU_CONST_BUFFER_SIZE_VS_0          0x00028180
#define R_028184_ALU_CONST_BUFFER_SIZE_VS_1          0x00028184
#define R_0281C0_ALU_CONST_BUFFER_SIZE_GS_0          0x000281C0
#define R_028940_ALU_CONST_CACHE_PS_0                0x00028940
#define R_028944_ALU_CONST_CACHE_PS_1                0x00028944
#define R_028980_ALU_CONST_CACHE_VS_0                0x00028980
#define R_028984_ALU_CONST_CACHE_VS_1                0x00028984
#define R_0289C0_ALU_CONST_CACHE_GS_0                0x000289C0

#define R_03CFF0_SQ_VTX_BASE_VTX_LOC                 0x03CFF0
#define R_03CFF4_SQ_VTX_START_INST_LOC               0x03CFF4

#define R_03E200_SQ_LOOP_CONST_0                     0x3E200

#define SQ_TEX_INST_LD			0x03
#define SQ_TEX_INST_GET_TEXTURE_RESINFO	0x04
#define SQ_TEX_INST_GET_BORDER_COLOR_FRAC 0x05
#define SQ_TEX_INST_GET_COMP_TEX_LOD	0x06
#define SQ_TEX_INST_GET_GRADIENTS_H	0x07
#define SQ_TEX_INST_GET_GRADIENTS_V	0x08
#define SQ_TEX_INST_GET_LERP_FACTORS	0x09
#define SQ_TEX_INST_GET_WEIGHTS		0x0A
#define SQ_TEX_INST_SET_GRADIENTS_H	0x0B
#define SQ_TEX_INST_SET_GRADIENTS_V	0x0C
#define SQ_TEX_INST_PASS		0x0D
#define SQ_TEX_INST_SET_CUBEMAP_INDEX	0x0E

#define SQ_TEX_INST_SAMPLE		0x10
#define SQ_TEX_INST_SAMPLE_L		0x11
#define SQ_TEX_INST_SAMPLE_LB		0x12
#define SQ_TEX_INST_SAMPLE_LZ		0x13
#define SQ_TEX_INST_SAMPLE_G		0x14
#define SQ_TEX_INST_SAMPLE_G_L		0x15
#define SQ_TEX_INST_SAMPLE_G_LB		0x16
#define SQ_TEX_INST_SAMPLE_G_LZ		0x17
#define SQ_TEX_INST_SAMPLE_C		0x18 /* src.xyz = texcoord, src.z = array index (if needed), src.w = depth */
#define SQ_TEX_INST_SAMPLE_C_L		0x19 /* src.xy  = texcoord, src.y = array index (if needed), src.z = depth, src.w = lod */
#define SQ_TEX_INST_SAMPLE_C_LB		0x1A /* src.xy  = texcoord, src.y = array index (if needed), src.z = depth, src.w = bias */
#define SQ_TEX_INST_SAMPLE_C_LZ		0x1B
#define SQ_TEX_INST_SAMPLE_C_G		0x1C
#define SQ_TEX_INST_SAMPLE_C_G_L	0x1D
#define SQ_TEX_INST_SAMPLE_C_G_LB	0x1E
#define SQ_TEX_INST_SAMPLE_C_G_LZ	0x1F

#define EG_0802C_GRBM_GFX_INDEX          			0x802C
#define   S_0802C_INSTANCE_INDEX(x)					  (((x) & 0xffff) << 0)
#define   S_0802C_SE_INDEX(x)						  (((x) & 0x3fff) << 16)
#define   S_0802C_INSTANCE_BROADCAST_WRITES(x)		  (((x) & 0x1) << 30)
#define   S_0802C_SE_BROADCAST_WRITES(x)			  (((x) & 0x1) << 31)

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
#define DMA_PACKET(cmd, t, s, n)	((((unsigned)(cmd) & 0xF) << 28) |	\
					(((unsigned)(t) & 0x1) << 23) |		\
					(((unsigned)(s) & 0x1) << 22) |		\
					(((unsigned)(n) & 0xFFFF) << 0))
/* async DMA Packet types */
#define DMA_PACKET_WRITE		0x2
#define DMA_PACKET_COPY			0x3
#define R600_DMA_COPY_MAX_SIZE_DW		0xffff
#define DMA_PACKET_INDIRECT_BUFFER	0x4
#define DMA_PACKET_SEMAPHORE		0x5
#define DMA_PACKET_FENCE		0x6
#define DMA_PACKET_TRAP			0x7
#define DMA_PACKET_CONSTANT_FILL	0xd /* 7xx only */
#define DMA_PACKET_NOP			0xf


/* Resource IDs:
 *   PS: 0   .. +160
 *   VS: 160 .. +160
 *   FS: 320 .. +16
 *   GS: 336 .. +160
 */
#define R600_FETCH_CONSTANTS_OFFSET_PS 0
#define R600_FETCH_CONSTANTS_OFFSET_VS 160
#define R600_FETCH_CONSTANTS_OFFSET_FS 320
#define R600_FETCH_CONSTANTS_OFFSET_GS 336
#endif
