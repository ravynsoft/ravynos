/**************************************************************************

Copyright (C) 2004-2005 Nicolai Haehnle et al.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/* *INDENT-OFF* */

#ifndef _R300_REG_H
#define _R300_REG_H

#define R300_MC_INIT_MISC_LAT_TIMER	0x180
#	define R300_MC_MISC__MC_CPR_INIT_LAT_SHIFT	0
#	define R300_MC_MISC__MC_VF_INIT_LAT_SHIFT	4
#	define R300_MC_MISC__MC_DISP0R_INIT_LAT_SHIFT	8
#	define R300_MC_MISC__MC_DISP1R_INIT_LAT_SHIFT	12
#	define R300_MC_MISC__MC_FIXED_INIT_LAT_SHIFT	16
#	define R300_MC_MISC__MC_E2R_INIT_LAT_SHIFT	20
#	define R300_MC_MISC__MC_SAME_PAGE_PRIO_SHIFT	24
#	define R300_MC_MISC__MC_GLOBW_INIT_LAT_SHIFT	28


#define R300_MC_INIT_GFX_LAT_TIMER	0x154
#	define R300_MC_MISC__MC_G3D0R_INIT_LAT_SHIFT	0
#	define R300_MC_MISC__MC_G3D1R_INIT_LAT_SHIFT	4
#	define R300_MC_MISC__MC_G3D2R_INIT_LAT_SHIFT	8
#	define R300_MC_MISC__MC_G3D3R_INIT_LAT_SHIFT	12
#	define R300_MC_MISC__MC_TX0R_INIT_LAT_SHIFT	16
#	define R300_MC_MISC__MC_TX1R_INIT_LAT_SHIFT	20
#	define R300_MC_MISC__MC_GLOBR_INIT_LAT_SHIFT	24
#	define R300_MC_MISC__MC_GLOBW_FULL_LAT_SHIFT	28

/*
 * This file contains registers and constants for the R300. They have been
 * found mostly by examining command buffers captured using glxtest, as well
 * as by extrapolating some known registers and constants from the R200.
 * I am fairly certain that they are correct unless stated otherwise
 * in comments.
 */

#define R300_SE_VPORT_XSCALE                0x1D98
#define R300_SE_VPORT_XOFFSET               0x1D9C
#define R300_SE_VPORT_YSCALE                0x1DA0
#define R300_SE_VPORT_YOFFSET               0x1DA4
#define R300_SE_VPORT_ZSCALE                0x1DA8
#define R300_SE_VPORT_ZOFFSET               0x1DAC

#define R300_VAP_PORT_IDX0		    0x2040
/*
 * Vertex Array Processing (VAP) Control
 */
#define R300_VAP_CNTL	0x2080
#       define R300_PVS_NUM_SLOTS_SHIFT                 0
#       define R300_PVS_NUM_CNTLRS_SHIFT                4
#       define R300_PVS_NUM_FPUS_SHIFT                  8
#       define R300_VF_MAX_VTX_NUM_SHIFT                18
#       define R300_PVS_NUM_SLOTS(x)                    ((x) << 0)
#       define R300_PVS_NUM_CNTLRS(x)                   ((x) << 4)
#       define R300_PVS_NUM_FPUS(x)                     ((x) << 8)
#       define R300_PVS_VF_MAX_VTX_NUM(x)               ((x) << 18)
#       define R300_GL_CLIP_SPACE_DEF                   (0 << 22)
#       define R300_DX_CLIP_SPACE_DEF                   (1 << 22)
#       define R500_TCL_STATE_OPTIMIZATION              (1 << 23)

/* This register is written directly and also starts data section
 * in many 3d CP_PACKET3's
 */
#define R300_VAP_VF_CNTL	0x2084
#	define	R300_VAP_VF_CNTL__PRIM_TYPE__SHIFT              0
#	define  R300_VAP_VF_CNTL__PRIM_NONE                     (0<<0)
#	define  R300_VAP_VF_CNTL__PRIM_POINTS                   (1<<0)
#	define  R300_VAP_VF_CNTL__PRIM_LINES                    (2<<0)
#	define  R300_VAP_VF_CNTL__PRIM_LINE_STRIP               (3<<0)
#	define  R300_VAP_VF_CNTL__PRIM_TRIANGLES                (4<<0)
#	define  R300_VAP_VF_CNTL__PRIM_TRIANGLE_FAN             (5<<0)
#	define  R300_VAP_VF_CNTL__PRIM_TRIANGLE_STRIP           (6<<0)
#	define  R300_VAP_VF_CNTL__PRIM_LINE_LOOP                (12<<0)
#	define  R300_VAP_VF_CNTL__PRIM_QUADS                    (13<<0)
#	define  R300_VAP_VF_CNTL__PRIM_QUAD_STRIP               (14<<0)
#	define  R300_VAP_VF_CNTL__PRIM_POLYGON                  (15<<0)

#	define	R300_VAP_VF_CNTL__PRIM_WALK__SHIFT              4
	/* State based - direct writes to registers trigger vertex
           generation */
#	define	R300_VAP_VF_CNTL__PRIM_WALK_STATE_BASED         (0<<4)
#	define	R300_VAP_VF_CNTL__PRIM_WALK_INDICES             (1<<4)
#	define	R300_VAP_VF_CNTL__PRIM_WALK_VERTEX_LIST         (2<<4)
#	define	R300_VAP_VF_CNTL__PRIM_WALK_VERTEX_EMBEDDED     (3<<4)

	/* I don't think I saw these three used.. */
#	define	R300_VAP_VF_CNTL__COLOR_ORDER__SHIFT            6
#	define	R300_VAP_VF_CNTL__TCL_OUTPUT_CTL_ENA__SHIFT     9
#	define	R300_VAP_VF_CNTL__PROG_STREAM_ENA__SHIFT        10

	/* index size - when not set the indices are assumed to be 16 bit */
#	define	R300_VAP_VF_CNTL__INDEX_SIZE_32bit              (1<<11)
#       define R500_VAP_VF_CNTL__USE_ALT_NUM_VERTS          (1<<14)
	/* number of vertices */
#	define	R300_VAP_VF_CNTL__NUM_VERTICES__SHIFT           16

#define R500_VAP_INDEX_OFFSET		    0x208c

#define R500_VAP_ALT_NUM_VERTICES                           0x2088

#define R300_VAP_OUTPUT_VTX_FMT_0           0x2090
#       define R300_VAP_OUTPUT_VTX_FMT_0__POS_PRESENT     (1<<0)
#       define R300_VAP_OUTPUT_VTX_FMT_0__COLOR_0_PRESENT (1<<1)
#       define R300_VAP_OUTPUT_VTX_FMT_0__COLOR_1_PRESENT (1<<2)
#       define R300_VAP_OUTPUT_VTX_FMT_0__COLOR_2_PRESENT (1<<3)
#       define R300_VAP_OUTPUT_VTX_FMT_0__COLOR_3_PRESENT (1<<4)
#       define R300_VAP_OUTPUT_VTX_FMT_0__PT_SIZE_PRESENT (1<<16)

#define R300_VAP_OUTPUT_VTX_FMT_1           0x2094
	/* each of the following is 3 bits wide, specifies number
	   of components */
#       define R300_VAP_OUTPUT_VTX_FMT_1__TEX_0_COMP_CNT_SHIFT 0
#       define R300_VAP_OUTPUT_VTX_FMT_1__TEX_1_COMP_CNT_SHIFT 3
#       define R300_VAP_OUTPUT_VTX_FMT_1__TEX_2_COMP_CNT_SHIFT 6
#       define R300_VAP_OUTPUT_VTX_FMT_1__TEX_3_COMP_CNT_SHIFT 9
#       define R300_VAP_OUTPUT_VTX_FMT_1__TEX_4_COMP_CNT_SHIFT 12
#       define R300_VAP_OUTPUT_VTX_FMT_1__TEX_5_COMP_CNT_SHIFT 15
#       define R300_VAP_OUTPUT_VTX_FMT_1__TEX_6_COMP_CNT_SHIFT 18
#       define R300_VAP_OUTPUT_VTX_FMT_1__TEX_7_COMP_CNT_SHIFT 21
#	define R300_VAP_OUTPUT_VTX_FMT_1__NOT_PRESENT  0
#	define R300_VAP_OUTPUT_VTX_FMT_1__1_COMPONENT  1
#	define R300_VAP_OUTPUT_VTX_FMT_1__2_COMPONENTS 2
#	define R300_VAP_OUTPUT_VTX_FMT_1__3_COMPONENTS 3
#	define R300_VAP_OUTPUT_VTX_FMT_1__4_COMPONENTS 4

#define R300_VAP_VPORT_XSCALE                     0x2098
#define R300_VAP_VPORT_XOFFSET                    0x209c
#define R300_VAP_VPORT_YSCALE                     0x20a0
#define R300_VAP_VPORT_YOFFSET                    0x20a4
#define R300_VAP_VPORT_ZSCALE                     0x20a8
#define R300_VAP_VPORT_ZOFFSET                    0x20ac

#define R300_VAP_VTE_CNTL                         0x20b0
#define R300_SE_VTE_CNTL R300_VAP_VTE_CNTL
#   define R300_VPORT_X_SCALE_ENA                           (1 << 0)
#   define R300_VPORT_X_OFFSET_ENA                          (1 << 1)
#   define R300_VPORT_Y_SCALE_ENA                           (1 << 2)
#   define R300_VPORT_Y_OFFSET_ENA                          (1 << 3)
#   define R300_VPORT_Z_SCALE_ENA                           (1 << 4)
#   define R300_VPORT_Z_OFFSET_ENA                          (1 << 5)
#   define R300_VTX_XY_FMT                                  (1 << 8)
#   define R300_VTX_Z_FMT                                   (1 << 9)
#   define R300_VTX_W0_FMT                                  (1 << 10)
#   define R300_SERIAL_PROC_ENA                             (1 << 11)

#define R300_VAP_VTX_SIZE               0x20b4

/* BEGIN: Vertex data assembly - lots of uncertainties */

/* gap */

/* Maximum Vertex Indx Clamp */
#define R300_VAP_VF_MAX_VTX_INDX         0x2134
/* Minimum Vertex Indx Clamp */
#define R300_VAP_VF_MIN_VTX_INDX         0x2138

/** Vertex assembler/processor control status */
#define R300_VAP_CNTL_STATUS              0x2140
/* No swap at all (default) */
#	define R300_VC_NO_SWAP                  (0 << 0)
/* 16-bit swap: 0xAABBCCDD becomes 0xBBAADDCC */
#	define R300_VC_16BIT_SWAP               (1 << 0)
/* 32-bit swap: 0xAABBCCDD becomes 0xDDCCBBAA */
#	define R300_VC_32BIT_SWAP               (2 << 0)
/* Half-dword swap: 0xAABBCCDD becomes 0xCCDDAABB */
#	define R300_VC_HALF_DWORD_SWAP          (3 << 0)
/* The TCL engine will not be used (as it is logically or even physically removed) */
#	define R300_VAP_TCL_BYPASS		(1 << 8)
/* Read only flag if TCL engine is busy. */
#	define R300_VAP_PVS_BUSY                (1 << 11)
/* Read only flag if the vertex store is busy. */
#	define R300_VAP_VS_BUSY                 (1 << 24)
/* Read only flag if the reciprocal engine is busy. */
#	define R300_VAP_RCP_BUSY                (1 << 25)
/* Read only flag if the viewport transform engine is busy. */
#	define R300_VAP_VTE_BUSY                (1 << 26)
/* Read only flag if the memory interface unit is busy. */
#	define R300_VAP_MUI_BUSY                (1 << 27)
/* Read only flag if the vertex cache is busy. */
#	define R300_VAP_VC_BUSY                 (1 << 28)
/* Read only flag if the vertex fetcher is busy. */
#	define R300_VAP_VF_BUSY                 (1 << 29)
/* Read only flag if the register pipeline is busy. */
#	define R300_VAP_REGPIPE_BUSY            (1 << 30)
/* Read only flag if the VAP engine is busy. */
#	define R300_VAP_VAP_BUSY                (1 << 31)

/* gap */

/* Where do we get our vertex data?
 *
 * Vertex data either comes either from immediate mode registers or from
 * vertex arrays.
 * There appears to be no mixed mode (though we can force the pitch of
 * vertex arrays to 0, effectively reusing the same element over and over
 * again).
 *
 * Immediate mode is controlled by the INPUT_CNTL registers. I am not sure
 * if these registers influence vertex array processing.
 *
 * Vertex arrays are controlled via the 3D_LOAD_VBPNTR packet3.
 *
 * In both cases, vertex attributes are then passed through INPUT_ROUTE.
 *
 * Beginning with INPUT_ROUTE_0_0 is a list of WORDs that route vertex data
 * into the vertex processor's input registers.
 * The first word routes the first input, the second word the second, etc.
 * The corresponding input is routed into the register with the given index.
 * The list is ended by a word with INPUT_ROUTE_END set.
 *
 * Always set COMPONENTS_4 in immediate mode.
 */

#define R300_VAP_PROG_STREAM_CNTL_0                     0x2150
#       define R300_DATA_TYPE_0_SHIFT                   0
#       define R300_DATA_TYPE_FLOAT_1                   0
#       define R300_DATA_TYPE_FLOAT_2                   1
#       define R300_DATA_TYPE_FLOAT_3                   2
#       define R300_DATA_TYPE_FLOAT_4                   3
#       define R300_DATA_TYPE_BYTE                      4
#       define R300_DATA_TYPE_D3DCOLOR                  5
#       define R300_DATA_TYPE_SHORT_2                   6
#       define R300_DATA_TYPE_SHORT_4                   7
#       define R300_DATA_TYPE_VECTOR_3_TTT              8
#       define R300_DATA_TYPE_VECTOR_3_EET              9
#       define R300_DATA_TYPE_FLOAT_8                   10
#       define R300_DATA_TYPE_FLT16_2                   11
#       define R300_DATA_TYPE_FLT16_4                   12
#       define R300_SKIP_DWORDS_SHIFT                   4
#       define R300_DST_VEC_LOC_SHIFT                   8
#       define R300_LAST_VEC                            (1 << 13)
#       define R300_SIGNED                              (1 << 14)
#       define R300_NORMALIZE                           (1 << 15)
#       define R300_DATA_TYPE_1_SHIFT                   16
#define R300_VAP_PROG_STREAM_CNTL_1                     0x2154
#define R300_VAP_PROG_STREAM_CNTL_2                     0x2158
#define R300_VAP_PROG_STREAM_CNTL_3                     0x215C
#define R300_VAP_PROG_STREAM_CNTL_4                     0x2160
#define R300_VAP_PROG_STREAM_CNTL_5                     0x2164
#define R300_VAP_PROG_STREAM_CNTL_6                     0x2168
#define R300_VAP_PROG_STREAM_CNTL_7                     0x216C
/* gap */

/* Notes:
 *  - always set up to produce at least two attributes:
 *    if vertex program uses only position, fglrx will set normal, too
 *  - INPUT_CNTL_0_COLOR and INPUT_CNTL_COLOR bits are always equal.
 */
#define R300_VAP_VTX_STATE_CNTL               0x2180
#       define R300_COLOR_0_ASSEMBLY_SHIFT    0
#       define R300_SEL_COLOR                 0
#       define R300_SEL_USER_COLOR_0          1
#       define R300_SEL_USER_COLOR_1          2
#       define R300_COLOR_1_ASSEMBLY_SHIFT    2
#       define R300_COLOR_2_ASSEMBLY_SHIFT    4
#       define R300_COLOR_3_ASSEMBLY_SHIFT    6
#       define R300_COLOR_4_ASSEMBLY_SHIFT    8
#       define R300_COLOR_5_ASSEMBLY_SHIFT    10
#       define R300_COLOR_6_ASSEMBLY_SHIFT    12
#       define R300_COLOR_7_ASSEMBLY_SHIFT    14
#       define R300_UPDATE_USER_COLOR_0_ENA   (1 << 16)

/*
 * Each bit in this field applies to the corresponding vector in the VSM
 * memory (i.e. Bit 0 applies to VECTOR_0 (POSITION), etc.). If the bit
 * is set, then the corresponding 4-Dword Vector is output into the Vertex Stream.
 */
#define R300_VAP_VSM_VTX_ASSM               0x2184
#       define R300_INPUT_CNTL_POS               0x00000001
#       define R300_INPUT_CNTL_NORMAL            0x00000002
#       define R300_INPUT_CNTL_COLOR             0x00000004
#       define R300_INPUT_CNTL_TC0               0x00000400
#       define R300_INPUT_CNTL_TC1               0x00000800
#       define R300_INPUT_CNTL_TC2               0x00001000 /* GUESS */
#       define R300_INPUT_CNTL_TC3               0x00002000 /* GUESS */
#       define R300_INPUT_CNTL_TC4               0x00004000 /* GUESS */
#       define R300_INPUT_CNTL_TC5               0x00008000 /* GUESS */
#       define R300_INPUT_CNTL_TC6               0x00010000 /* GUESS */
#       define R300_INPUT_CNTL_TC7               0x00020000 /* GUESS */

/* Programmable Stream Control Signed Normalize Control */
#define R300_VAP_PSC_SGN_NORM_CNTL                0x21dc
#   define SGN_NORM_ZERO                                    0
#   define SGN_NORM_ZERO_CLAMP_MINUS_ONE                    1
#   define SGN_NORM_NO_ZERO                                 2U
#   define R300_SGN_NORM_NO_ZERO (SGN_NORM_NO_ZERO | \
        (SGN_NORM_NO_ZERO << 2) | (SGN_NORM_NO_ZERO << 4) | \
        (SGN_NORM_NO_ZERO << 6) | (SGN_NORM_NO_ZERO << 8) | \
        (SGN_NORM_NO_ZERO << 10) | (SGN_NORM_NO_ZERO << 12) | \
        (SGN_NORM_NO_ZERO << 14) | (SGN_NORM_NO_ZERO << 16) | \
        (SGN_NORM_NO_ZERO << 18) | (SGN_NORM_NO_ZERO << 20) | \
        (SGN_NORM_NO_ZERO << 22) | (SGN_NORM_NO_ZERO << 24) | \
        (SGN_NORM_NO_ZERO << 26) | (SGN_NORM_NO_ZERO << 28) | \
        (SGN_NORM_NO_ZERO << 30))

/* gap */

/* Words parallel to INPUT_ROUTE_0; All words that are active in INPUT_ROUTE_0
 * are set to a swizzling bit pattern, other words are 0.
 *
 * In immediate mode, the pattern is always set to xyzw. In vertex array
 * mode, the swizzling pattern is e.g. used to set zw components in texture
 * coordinates with only two components.
 */
#define R300_VAP_PROG_STREAM_CNTL_EXT_0                 0x21e0
#       define R300_SWIZZLE0_SHIFT                      0
#       define R300_SWIZZLE_SELECT_X_SHIFT              0
#       define R300_SWIZZLE_SELECT_Y_SHIFT              3
#       define R300_SWIZZLE_SELECT_Z_SHIFT              6
#       define R300_SWIZZLE_SELECT_W_SHIFT              9

#       define R300_SWIZZLE_SELECT_X                    0
#       define R300_SWIZZLE_SELECT_Y                    1
#       define R300_SWIZZLE_SELECT_Z                    2
#       define R300_SWIZZLE_SELECT_W                    3
#       define R300_SWIZZLE_SELECT_FP_ZERO              4
#       define R300_SWIZZLE_SELECT_FP_ONE               5
/* alternate forms for r300_emit.c */
#       define R300_INPUT_ROUTE_SELECT_X    0
#       define R300_INPUT_ROUTE_SELECT_Y    1
#       define R300_INPUT_ROUTE_SELECT_Z    2
#       define R300_INPUT_ROUTE_SELECT_W    3
#       define R300_INPUT_ROUTE_SELECT_ZERO 4
#       define R300_INPUT_ROUTE_SELECT_ONE  5

#       define R300_WRITE_ENA_SHIFT                     12
#       define R300_WRITE_ENA_X                         1
#       define R300_WRITE_ENA_Y                         2
#       define R300_WRITE_ENA_Z                         4
#       define R300_WRITE_ENA_W                         8
#       define R300_SWIZZLE1_SHIFT                      16

#       define R300_VAP_SWIZZLE_X001 \
        ((R300_SWIZZLE_SELECT_X << R300_SWIZZLE_SELECT_X_SHIFT) | \
         (R300_SWIZZLE_SELECT_FP_ZERO << R300_SWIZZLE_SELECT_Y_SHIFT) | \
         (R300_SWIZZLE_SELECT_FP_ZERO << R300_SWIZZLE_SELECT_Z_SHIFT) | \
         (R300_SWIZZLE_SELECT_FP_ONE << R300_SWIZZLE_SELECT_W_SHIFT) | \
         (0xf << R300_WRITE_ENA_SHIFT))

#       define R300_VAP_SWIZZLE_XY01 \
        ((R300_SWIZZLE_SELECT_X << R300_SWIZZLE_SELECT_X_SHIFT) | \
         (R300_SWIZZLE_SELECT_Y << R300_SWIZZLE_SELECT_Y_SHIFT) | \
         (R300_SWIZZLE_SELECT_FP_ZERO << R300_SWIZZLE_SELECT_Z_SHIFT) | \
         (R300_SWIZZLE_SELECT_FP_ONE << R300_SWIZZLE_SELECT_W_SHIFT) | \
         (0xf << R300_WRITE_ENA_SHIFT))

#       define R300_VAP_SWIZZLE_XYZ1 \
        ((R300_SWIZZLE_SELECT_X << R300_SWIZZLE_SELECT_X_SHIFT) | \
         (R300_SWIZZLE_SELECT_Y << R300_SWIZZLE_SELECT_Y_SHIFT) | \
         (R300_SWIZZLE_SELECT_Z << R300_SWIZZLE_SELECT_Z_SHIFT) | \
         (R300_SWIZZLE_SELECT_FP_ONE << R300_SWIZZLE_SELECT_W_SHIFT) | \
         (0xf << R300_WRITE_ENA_SHIFT))

#       define R300_VAP_SWIZZLE_XYZW \
        ((R300_SWIZZLE_SELECT_X << R300_SWIZZLE_SELECT_X_SHIFT) | \
         (R300_SWIZZLE_SELECT_Y << R300_SWIZZLE_SELECT_Y_SHIFT) | \
         (R300_SWIZZLE_SELECT_Z << R300_SWIZZLE_SELECT_Z_SHIFT) | \
         (R300_SWIZZLE_SELECT_W << R300_SWIZZLE_SELECT_W_SHIFT) | \
         (0xf << R300_WRITE_ENA_SHIFT))

#define R300_VAP_PROG_STREAM_CNTL_EXT_1                 0x21e4
#define R300_VAP_PROG_STREAM_CNTL_EXT_2                 0x21e8
#define R300_VAP_PROG_STREAM_CNTL_EXT_3                 0x21ec
#define R300_VAP_PROG_STREAM_CNTL_EXT_4                 0x21f0
#define R300_VAP_PROG_STREAM_CNTL_EXT_5                 0x21f4
#define R300_VAP_PROG_STREAM_CNTL_EXT_6                 0x21f8
#define R300_VAP_PROG_STREAM_CNTL_EXT_7                 0x21fc

/* END: Vertex data assembly */

/* gap */

/* BEGIN: Upload vertex program and data */

/*
 * The programmable vertex shader unit has a memory bank of unknown size
 * that can be written to in 16 byte units by writing the address into
 * UPLOAD_ADDRESS, followed by data in UPLOAD_DATA (multiples of 4 DWORDs).
 *
 * Pointers into the memory bank are always in multiples of 16 bytes.
 *
 * The memory bank is divided into areas with fixed meaning.
 *
 * Starting at address UPLOAD_PROGRAM: Vertex program instructions.
 * Native limits reported by drivers from ATI suggest size 256 (i.e. 4KB),
 * whereas the difference between known addresses suggests size 512.
 *
 * Starting at address UPLOAD_PARAMETERS: Vertex program parameters.
 * Native reported limits and the VPI layout suggest size 256, whereas
 * difference between known addresses suggests size 512.
 *
 * At address UPLOAD_POINTSIZE is a vector (0, 0, ps, 0), where ps is the
 * floating point pointsize. The exact purpose of this state is uncertain,
 * as there is also the R300_RE_POINTSIZE register.
 *
 * Multiple vertex programs and parameter sets can be loaded at once,
 * which could explain the size discrepancy.
 */
#define R300_VAP_PVS_VECTOR_INDX_REG         0x2200
#       define R300_PVS_CODE_START           0
#       define R300_MAX_PVS_CODE_LINES       256
#       define R500_MAX_PVS_CODE_LINES       1024
#       define R300_PVS_CONST_START          512
#       define R500_PVS_CONST_START          1024
#       define R300_MAX_PVS_CONST_VECS       256
#       define R500_MAX_PVS_CONST_VECS       256
#       define R300_PVS_UCP_START            1024
#       define R500_PVS_UCP_START            1536
#       define R300_POINT_VPORT_SCALE_OFFSET 1030
#       define R500_POINT_VPORT_SCALE_OFFSET 1542
#       define R300_POINT_GEN_TEX_OFFSET     1031
#       define R500_POINT_GEN_TEX_OFFSET     1543

/*
 * These are obsolete defines form r300_context.h, but they might give some
 * clues when investigating the addresses further...
 */
#if 0
#define VSF_DEST_PROGRAM        0x0
#define VSF_DEST_MATRIX0        0x200
#define VSF_DEST_MATRIX1        0x204
#define VSF_DEST_MATRIX2        0x208
#define VSF_DEST_VECTOR0        0x20c
#define VSF_DEST_VECTOR1        0x20d
#define VSF_DEST_UNKNOWN1       0x400
#define VSF_DEST_UNKNOWN2       0x406
#endif

/* gap */

#define R300_VAP_PVS_UPLOAD_DATA            0x2208

/* END: Upload vertex program and data */

/* gap */

/* I do not know the purpose of this register. However, I do know that
 * it is set to 221C_CLEAR for clear operations and to 221C_NORMAL
 * for normal rendering.
 *
 * 2007-11-05: This register is the user clip plane control register, but there
 * also seems to be a rendering mode control; the NORMAL/CLEAR defines.
 *
 * See bug #9871. http://bugs.freedesktop.org/attachment.cgi?id=10672&action=view
 */
#define R500_VAP_TEX_TO_COLOR_CNTL		0x2218

#define R300_VAP_CLIP_CNTL                       0x221C
#       define R300_VAP_UCP_ENABLE_0             (1 << 0)
#       define R300_VAP_UCP_ENABLE_1             (1 << 1)
#       define R300_VAP_UCP_ENABLE_2             (1 << 2)
#       define R300_VAP_UCP_ENABLE_3             (1 << 3)
#       define R300_VAP_UCP_ENABLE_4             (1 << 4)
#       define R300_VAP_UCP_ENABLE_5             (1 << 5)
#       define R300_PS_UCP_MODE_DIST_COP         (0 << 14)
#       define R300_PS_UCP_MODE_RADIUS_COP       (1 << 14)
#       define R300_PS_UCP_MODE_RADIUS_COP_CLIP  (2 << 14)
#       define R300_PS_UCP_MODE_CLIP_AS_TRIFAN   (3 << 14)
#       define R300_CLIP_DISABLE                 (1 << 16)
#       define R300_UCP_CULL_ONLY_ENABLE         (1 << 17)
#       define R300_BOUNDARY_EDGE_FLAG_ENABLE    (1 << 18)
#       define R500_COLOR2_IS_TEXTURE            (1 << 20)
#       define R500_COLOR3_IS_TEXTURE            (1 << 21)

/* These seem to be per-pixel and per-vertex X and Y clipping planes. The first
 * plane is per-pixel and the second plane is per-vertex.
 *
 * This was determined by experimentation alone but I believe it is correct.
 *
 * These registers are called X_QUAD0_1_FL to X_QUAD0_4_FL by glxtest.
 */
#define R300_VAP_GB_VERT_CLIP_ADJ                   0x2220
#define R300_VAP_GB_VERT_DISC_ADJ                   0x2224
#define R300_VAP_GB_HORZ_CLIP_ADJ                   0x2228
#define R300_VAP_GB_HORZ_DISC_ADJ                   0x222c

#define R300_VAP_PVS_FLOW_CNTL_ADDRS_0      0x2230
#define R300_PVS_FC_ACT_ADRS(x)             ((x) << 0)
#define R300_PVS_FC_LOOP_CNT_JMP_INST(x)    ((x) << 8)
#define R300_PVS_FC_LAST_INST(x)            ((x) << 16)
#define R300_PVS_FC_RTN_INST(x)             ((x) << 24)

/* gap */

/* Sometimes, END_OF_PKT and 0x2284=0 are the only commands sent between
 * rendering commands and overwriting vertex program parameters.
 * Therefore, I suspect writing zero to 0x2284 synchronizes the engine and
 * avoids bugs caused by still running shaders reading bad data from memory.
 */
#define R300_VAP_PVS_STATE_FLUSH_REG        0x2284

/* This register is used to define the number of core clocks to wait for a
 * vertex to be received by the VAP input controller (while the primitive
 * path is backed up) before forcing any accumulated vertices to be submitted
 * to the vertex processing path.
 */
#define VAP_PVS_VTX_TIMEOUT_REG             0x2288
#       define R300_2288_R300                    0x00750000 /* -- nh */
#       define R300_2288_RV350                   0x0000FFFF /* -- Vladimir */

#define R300_VAP_PVS_FLOW_CNTL_LOOP_INDEX_0 0x2290
#define R300_PVS_FC_LOOP_INIT_VAL(x)        ((x) << 0)
#define R300_PVS_FC_LOOP_STEP_VAL(x)        ((x) << 8)

/* gap */

/* Addresses are relative to the vertex program instruction area of the
 * memory bank. PROGRAM_END points to the last instruction of the active
 * program
 *
 * The meaning of the two UNKNOWN fields is obviously not known. However,
 * experiments so far have shown that both *must* point to an instruction
 * inside the vertex program, otherwise the GPU locks up.
 *
 * fglrx usually sets CNTL_3_UNKNOWN to the end of the program and
 * R300_PVS_CNTL_1_POS_END_SHIFT points to instruction where last write to
 * position takes place.
 *
 * Most likely this is used to ignore rest of the program in cases
 * where group of verts aren't visible. For some reason this "section"
 * is sometimes accepted other instruction that have no relationship with
 * position calculations.
 */
#define R300_VAP_PVS_CODE_CNTL_0            0x22D0
#       define R300_PVS_FIRST_INST_SHIFT         0
#       define R300_PVS_XYZW_VALID_INST_SHIFT    10
#       define R300_PVS_LAST_INST_SHIFT          20
#       define R300_PVS_FIRST_INST(x)            ((x) << 0)
#       define R300_PVS_XYZW_VALID_INST(x)       ((x) << 10)
#       define R300_PVS_LAST_INST(x)             ((x) << 20)
/* Addresses are relative to the vertex program parameters area. */
#define R300_VAP_PVS_CONST_CNTL             0x22D4
#       define R300_PVS_CONST_BASE_OFFSET_SHIFT  0
#       define R300_PVS_CONST_BASE_OFFSET(x)     (x)
#       define R300_PVS_MAX_CONST_ADDR_SHIFT     16
#       define R300_PVS_MAX_CONST_ADDR(x)        ((x) << 16)
#define R300_VAP_PVS_CODE_CNTL_1	    0x22D8
#       define R300_PVS_LAST_VTX_SRC_INST_SHIFT  0
#define R300_VAP_PVS_FLOW_CNTL_OPC          0x22DC
#define R300_VAP_PVS_FC_OPC_JUMP(x)         (1 << (2 * (x)))
#define R300_VAP_PVS_FC_OPC_LOOP(x)         (2 << (2 * (x)))
#define R300_VAP_PVS_FC_OPC_JSR(x)          (3 << (2 * (x)))

/* The entire range from 0x2300 to 0x2AC inclusive seems to be used for
 * immediate vertices
 */
#define R300_VAP_VTX_COLOR_R                0x2464
#define R300_VAP_VTX_COLOR_G                0x2468
#define R300_VAP_VTX_COLOR_B                0x246C
#define R300_VAP_VTX_POS_0_X_1              0x2490 /* used for glVertex2*() */
#define R300_VAP_VTX_POS_0_Y_1              0x2494
#define R300_VAP_VTX_COLOR_PKD              0x249C /* RGBA */
#define R300_VAP_VTX_POS_0_X_2              0x24A0 /* used for glVertex3*() */
#define R300_VAP_VTX_POS_0_Y_2              0x24A4
#define R300_VAP_VTX_POS_0_Z_2              0x24A8
/* write 0 to indicate end of packet? */
#define R300_VAP_VTX_END_OF_PKT             0x24AC

#define R500_VAP_PVS_FLOW_CNTL_ADDRS_LW_0   0x2500
#define R500_PVS_FC_ACT_ADRS(x)             ((x) << 0)
#define R500_PVS_FC_LOOP_CNT_JMP_INST(x)    ((x) << 16)

#define R500_VAP_PVS_FLOW_CNTL_ADDRS_UW_0   0x2504
#define R500_PVS_FC_LAST_INST(x)            ((x) << 0)
#define R500_PVS_FC_RTN_INST(x)             ((x) << 16)

/* gap */

/* These are values from r300_reg/r300_reg.h - they are known to be correct
 * and are here so we can use one register file instead of several
 * - Vladimir
 */
#define R300_GB_VAP_RASTER_VTX_FMT_0	0x4000
#	define R300_GB_VAP_RASTER_VTX_FMT_0__POS_PRESENT	(1<<0)
#	define R300_GB_VAP_RASTER_VTX_FMT_0__COLOR_0_PRESENT	(1<<1)
#	define R300_GB_VAP_RASTER_VTX_FMT_0__COLOR_1_PRESENT	(1<<2)
#	define R300_GB_VAP_RASTER_VTX_FMT_0__COLOR_2_PRESENT	(1<<3)
#	define R300_GB_VAP_RASTER_VTX_FMT_0__COLOR_3_PRESENT	(1<<4)
#	define R300_GB_VAP_RASTER_VTX_FMT_0__COLOR_SPACE	(0xf<<5)
#	define R300_GB_VAP_RASTER_VTX_FMT_0__PT_SIZE_PRESENT	(0x1<<16)

#define R300_GB_VAP_RASTER_VTX_FMT_1	0x4004
	/* each of the following is 3 bits wide, specifies number
	   of components */
#	define R300_GB_VAP_RASTER_VTX_FMT_1__TEX_0_COMP_CNT_SHIFT	0
#	define R300_GB_VAP_RASTER_VTX_FMT_1__TEX_1_COMP_CNT_SHIFT	3
#	define R300_GB_VAP_RASTER_VTX_FMT_1__TEX_2_COMP_CNT_SHIFT	6
#	define R300_GB_VAP_RASTER_VTX_FMT_1__TEX_3_COMP_CNT_SHIFT	9
#	define R300_GB_VAP_RASTER_VTX_FMT_1__TEX_4_COMP_CNT_SHIFT	12
#	define R300_GB_VAP_RASTER_VTX_FMT_1__TEX_5_COMP_CNT_SHIFT	15
#	define R300_GB_VAP_RASTER_VTX_FMT_1__TEX_6_COMP_CNT_SHIFT	18
#	define R300_GB_VAP_RASTER_VTX_FMT_1__TEX_7_COMP_CNT_SHIFT	21

/* UNK30 seems to enables point to quad transformation on textures
 * (or something closely related to that).
 * This bit is rather fatal at the time being due to lackings at pixel
 * shader side
 * Specifies top of Raster pipe specific enable controls.
 */
#define R300_GB_ENABLE	0x4008
#	define R300_GB_POINT_STUFF_DISABLE     (0 << 0)
#	define R300_GB_POINT_STUFF_ENABLE      (1 << 0) /* Specifies if points will have stuffed texture coordinates. */
#	define R300_GB_LINE_STUFF_DISABLE      (0 << 1)
#	define R300_GB_LINE_STUFF_ENABLE       (1 << 1) /* Specifies if lines will have stuffed texture coordinates. */
#	define R300_GB_TRIANGLE_STUFF_DISABLE  (0 << 2)
#	define R300_GB_TRIANGLE_STUFF_ENABLE   (1 << 2) /* Specifies if triangles will have stuffed texture coordinates. */
#	define R300_GB_STENCIL_AUTO_DISABLE    (0 << 4)
#	define R300_GB_STENCIL_AUTO_ENABLE     (1 << 4) /* Enable stencil auto inc/dec based on triangle cw/ccw, force into dzy low bit. */
#	define R300_GB_STENCIL_AUTO_FORCE      (2 << 4) /* Force 0 into dzy low bit. */

	/* each of the following is 2 bits wide */
#define R300_GB_TEX_REPLICATE	0 /* Replicate VAP source texture coordinates (S,T,[R,Q]). */
#define R300_GB_TEX_ST		1 /* Stuff with source texture coordinates (S,T). */
#define R300_GB_TEX_STR		2 /* Stuff with source texture coordinates (S,T,R). */
#	define R300_GB_TEX0_SOURCE_SHIFT	16
#	define R300_GB_TEX1_SOURCE_SHIFT	18
#	define R300_GB_TEX2_SOURCE_SHIFT	20
#	define R300_GB_TEX3_SOURCE_SHIFT	22
#	define R300_GB_TEX4_SOURCE_SHIFT	24
#	define R300_GB_TEX5_SOURCE_SHIFT	26
#	define R300_GB_TEX6_SOURCE_SHIFT	28
#	define R300_GB_TEX7_SOURCE_SHIFT	30

/* MSPOS - positions for multisample antialiasing (?) */
#define R300_GB_MSPOS0                           0x4010
	/* shifts - each of the fields is 4 bits */
#	define R300_GB_MSPOS0__MS_X0_SHIFT	0
#	define R300_GB_MSPOS0__MS_Y0_SHIFT	4
#	define R300_GB_MSPOS0__MS_X1_SHIFT	8
#	define R300_GB_MSPOS0__MS_Y1_SHIFT	12
#	define R300_GB_MSPOS0__MS_X2_SHIFT	16
#	define R300_GB_MSPOS0__MS_Y2_SHIFT	20
#	define R300_GB_MSPOS0__MSBD0_Y		24
#	define R300_GB_MSPOS0__MSBD0_X		28

#define R300_GB_MSPOS1                           0x4014
#	define R300_GB_MSPOS1__MS_X3_SHIFT	0
#	define R300_GB_MSPOS1__MS_Y3_SHIFT	4
#	define R300_GB_MSPOS1__MS_X4_SHIFT	8
#	define R300_GB_MSPOS1__MS_Y4_SHIFT	12
#	define R300_GB_MSPOS1__MS_X5_SHIFT	16
#	define R300_GB_MSPOS1__MS_Y5_SHIFT	20
#	define R300_GB_MSPOS1__MSBD1		24

/* Specifies the graphics pipeline configuration for rasterization. */
#define R300_GB_TILE_CONFIG                      0x4018
#	define R300_GB_TILE_DISABLE             (0 << 0)
#	define R300_GB_TILE_ENABLE              (1 << 0)
#	define R300_GB_TILE_PIPE_COUNT_RV300	(0 << 1) /* RV350 (1 pipe, 1 ctx) */
#	define R300_GB_TILE_PIPE_COUNT_R300	(3 << 1) /* R300 (2 pipes, 1 ctx) */
#	define R300_GB_TILE_PIPE_COUNT_R420_3P  (6 << 1) /* R420-3P (3 pipes, 1 ctx) */
#	define R300_GB_TILE_PIPE_COUNT_R420	(7 << 1) /* R420 (4 pipes, 1 ctx) */
#	define R300_GB_TILE_SIZE_8		(0 << 4)
#	define R300_GB_TILE_SIZE_16		(1 << 4)
#	define R300_GB_TILE_SIZE_32		(2 << 4)
#	define R300_GB_SUPER_SIZE_1		(0 << 6)
#	define R300_GB_SUPER_SIZE_2		(1 << 6)
#	define R300_GB_SUPER_SIZE_4		(2 << 6)
#	define R300_GB_SUPER_SIZE_8		(3 << 6)
#	define R300_GB_SUPER_SIZE_16		(4 << 6)
#	define R300_GB_SUPER_SIZE_32		(5 << 6)
#	define R300_GB_SUPER_SIZE_64		(6 << 6)
#	define R300_GB_SUPER_SIZE_128		(7 << 6)
#	define R300_GB_SUPER_X_SHIFT		9	/* 3 bits wide */
#	define R300_GB_SUPER_Y_SHIFT		12	/* 3 bits wide */
#	define R300_GB_SUPER_TILE_A		(0 << 15)
#	define R300_GB_SUPER_TILE_B		(1 << 15)
#	define R300_GB_SUBPIXEL_1_12		(0 << 16)
#	define R300_GB_SUBPIXEL_1_16		(1 << 16)
#	define R300_GB_TILE_CONFIG_QUADS_PER_RAS_4   (0 << 17)
#	define R300_GB_TILE_CONFIG_QUADS_PER_RAS_8   (1 << 17)
#	define R300_GB_TILE_CONFIG_QUADS_PER_RAS_16  (2 << 17)
#	define R300_GB_TILE_CONFIG_QUADS_PER_RAS_32  (3 << 17)
#	define R300_GB_TILE_CONFIG_BB_SCAN_INTERCEPT (0 << 19)
#	define R300_GB_TILE_CONFIG_BB_SCAN_BOUND_BOX (1 << 19)
#	define R300_GB_TILE_CONFIG_ALT_SCAN_EN_LR    (0 << 20)
#	define R300_GB_TILE_CONFIG_ALT_SCAN_EN_LRL   (1 << 20)
#	define R300_GB_TILE_CONFIG_ALT_OFFSET        (0 << 21)
#	define R300_GB_TILE_CONFIG_SUBPRECISION      (0 << 22)
#	define R300_GB_TILE_CONFIG_ALT_TILING_DEF    (0 << 23)
#	define R300_GB_TILE_CONFIG_ALT_TILING_3_2    (1 << 23)
#	define R300_GB_TILE_CONFIG_Z_EXTENDED_24_1   (0 << 24)
#	define R300_GB_TILE_CONFIG_Z_EXTENDED_S25_1  (1 << 24)

/* Specifies the sizes of the various FIFO`s in the sc/rs/us. This register must be the first one written */
#define R300_GB_FIFO_SIZE	0x4024
	/* each of the following is 2 bits wide */
#define R300_GB_FIFO_SIZE_32	0
#define R300_GB_FIFO_SIZE_64	1
#define R300_GB_FIFO_SIZE_128	2
#define R300_GB_FIFO_SIZE_256	3
#	define R300_SC_IFIFO_SIZE_SHIFT	0
#	define R300_SC_TZFIFO_SIZE_SHIFT	2
#	define R300_SC_BFIFO_SIZE_SHIFT	4

#	define R300_US_OFIFO_SIZE_SHIFT	12
#	define R300_US_WFIFO_SIZE_SHIFT	14
	/* the following use the same constants as above, but meaning is
	   is times 2 (i.e. instead of 32 words it means 64 */
#	define R300_RS_TFIFO_SIZE_SHIFT	6
#	define R300_RS_CFIFO_SIZE_SHIFT	8
#	define R300_US_RAM_SIZE_SHIFT		10
	/* watermarks, 3 bits wide */
#	define R300_RS_HIGHWATER_COL_SHIFT	16
#	define R300_RS_HIGHWATER_TEX_SHIFT	19
#	define R300_OFIFO_HIGHWATER_SHIFT	22	/* two bits only */
#	define R300_CUBE_FIFO_HIGHWATER_COL_SHIFT	24

#define R300_GB_Z_PEQ_CONFIG                          0x4028
#	define R300_GB_Z_PEQ_CONFIG_Z_PEQ_SIZE_4_4    (0 << 0)
#	define R300_GB_Z_PEQ_CONFIG_Z_PEQ_SIZE_8_8    (1 << 0)

/* Specifies various polygon specific selects (fog, depth, perspective). */
#define R300_GB_SELECT                           0x401c
#	define R300_GB_FOG_SELECT_C0A		(0 << 0)
#	define R300_GB_FOG_SELECT_C1A           (1 << 0)
#	define R300_GB_FOG_SELECT_C2A           (2 << 0)
#	define R300_GB_FOG_SELECT_C3A           (3 << 0)
#	define R300_GB_FOG_SELECT_1_1_W         (4 << 0)
#	define R300_GB_FOG_SELECT_Z		(5 << 0)
#	define R300_GB_DEPTH_SELECT_Z		(0 << 3)
#	define R300_GB_DEPTH_SELECT_1_1_W	(1 << 3)
#	define R300_GB_W_SELECT_1_W		(0 << 4)
#	define R300_GB_W_SELECT_1		(1 << 4)
#	define R300_GB_FOG_STUFF_DISABLE        (0 << 5)
#	define R300_GB_FOG_STUFF_ENABLE         (1 << 5)
#	define R300_GB_FOG_STUFF_TEX_SHIFT      6
#	define R300_GB_FOG_STUFF_TEX_MASK       0x000003c0
#	define R300_GB_FOG_STUFF_COMP_SHIFT     10
#	define R300_GB_FOG_STUFF_COMP_MASK      0x00000c00

/* Specifies the graphics pipeline configuration for antialiasing. */
#define R300_GB_AA_CONFIG                         0x4020
#	define R300_GB_AA_CONFIG_AA_DISABLE           (0 << 0)
#	define R300_GB_AA_CONFIG_AA_ENABLE            (1 << 0)
#	define R300_GB_AA_CONFIG_NUM_AA_SUBSAMPLES_2  (0 << 1)
#	define R300_GB_AA_CONFIG_NUM_AA_SUBSAMPLES_3  (1 << 1)
#	define R300_GB_AA_CONFIG_NUM_AA_SUBSAMPLES_4  (2 << 1)
#	define R300_GB_AA_CONFIG_NUM_AA_SUBSAMPLES_6  (3 << 1)

/* Selects which of 4 pipes are active. */
#define R300_GB_PIPE_SELECT                           0x402c
#	define R300_GB_PIPE_SELECT_PIPE0_ID_SHIFT  0
#	define R300_GB_PIPE_SELECT_PIPE1_ID_SHIFT  2
#	define R300_GB_PIPE_SELECT_PIPE2_ID_SHIFT  4
#	define R300_GB_PIPE_SELECT_PIPE3_ID_SHIFT  6
#	define R300_GB_PIPE_SELECT_PIPE_MASK_SHIFT 8
#	define R300_GB_PIPE_SELECT_MAX_PIPE        12
#	define R300_GB_PIPE_SELECT_BAD_PIPES       14
#	define R300_GB_PIPE_SELECT_CONFIG_PIPES    18


/* Specifies the sizes of the various FIFO`s in the sc/rs. */
#define R300_GB_FIFO_SIZE1                            0x4070
/* High water mark for SC input fifo */
#	define R300_GB_FIFO_SIZE1_SC_HIGHWATER_IFIFO_SHIFT 0
#	define R300_GB_FIFO_SIZE1_SC_HIGHWATER_IFIFO_MASK  0x0000003f
/* High water mark for SC input fifo (B) */
#	define R300_GB_FIFO_SIZE1_SC_HIGHWATER_BFIFO_SHIFT 6
#	define R300_GB_FIFO_SIZE1_SC_HIGHWATER_BFIFO_MASK  0x00000fc0
/* High water mark for RS colors' fifo */
#	define R300_GB_FIFO_SIZE1_SC_HIGHWATER_COL_SHIFT   12
#	define R300_GB_FIFO_SIZE1_SC_HIGHWATER_COL_MASK    0x0003f000
/* High water mark for RS textures' fifo */
#	define R300_GB_FIFO_SIZE1_SC_HIGHWATER_TEX_SHIFT   18
#	define R300_GB_FIFO_SIZE1_SC_HIGHWATER_TEX_MASK    0x00fc0000

/* This table specifies the source location and format for up to 16 texture
 * addresses (i[0]:i[15]) and four colors (c[0]:c[3])
 */
#define R500_RS_IP_0					0x4074
#define R500_RS_IP_1					0x4078
#define R500_RS_IP_2					0x407C
#define R500_RS_IP_3					0x4080
#define R500_RS_IP_4					0x4084
#define R500_RS_IP_5					0x4088
#define R500_RS_IP_6					0x408C
#define R500_RS_IP_7					0x4090
#define R500_RS_IP_8					0x4094
#define R500_RS_IP_9					0x4098
#define R500_RS_IP_10					0x409C
#define R500_RS_IP_11					0x40A0
#define R500_RS_IP_12					0x40A4
#define R500_RS_IP_13					0x40A8
#define R500_RS_IP_14					0x40AC
#define R500_RS_IP_15					0x40B0
#define R500_RS_IP_PTR_K0                               62
#define R500_RS_IP_PTR_K1                               63
#define R500_RS_IP_TEX_PTR_S_SHIFT 			0
#define R500_RS_IP_TEX_PTR_T_SHIFT 			6
#define R500_RS_IP_TEX_PTR_R_SHIFT 			12
#define R500_RS_IP_TEX_PTR_Q_SHIFT 			18
#define R500_RS_IP_COL_PTR_SHIFT 			24
#define R500_RS_IP_COL_FMT_SHIFT 			27
#       define R500_RS_SEL_S(x)                         ((x) << 0)
#       define R500_RS_SEL_T(x)                         ((x) << 6)
#       define R500_RS_SEL_R(x)                         ((x) << 12)
#       define R500_RS_SEL_Q(x)                         ((x) << 18)
#	define R500_RS_COL_PTR(x)		        ((x) << 24)
#       define R500_RS_COL_FMT(x)                       ((x) << 27)
/* gap */
#define R500_RS_IP_OFFSET_DIS 				(0U << 31)
#define R500_RS_IP_OFFSET_EN 				(1U << 31)

/* gap */

/* Zero to flush caches. */
#define R300_TX_INVALTAGS                   0x4100
#define R300_TX_FLUSH                       0x0

/* The upper enable bits are guessed, based on fglrx reported limits. */
#define R300_TX_ENABLE                      0x4104
#       define R300_TX_ENABLE_0                  (1 << 0)
#       define R300_TX_ENABLE_1                  (1 << 1)
#       define R300_TX_ENABLE_2                  (1 << 2)
#       define R300_TX_ENABLE_3                  (1 << 3)
#       define R300_TX_ENABLE_4                  (1 << 4)
#       define R300_TX_ENABLE_5                  (1 << 5)
#       define R300_TX_ENABLE_6                  (1 << 6)
#       define R300_TX_ENABLE_7                  (1 << 7)
#       define R300_TX_ENABLE_8                  (1 << 8)
#       define R300_TX_ENABLE_9                  (1 << 9)
#       define R300_TX_ENABLE_10                 (1 << 10)
#       define R300_TX_ENABLE_11                 (1 << 11)
#       define R300_TX_ENABLE_12                 (1 << 12)
#       define R300_TX_ENABLE_13                 (1 << 13)
#       define R300_TX_ENABLE_14                 (1 << 14)
#       define R300_TX_ENABLE_15                 (1 << 15)

#define R500_TX_FILTER_4		    0x4110
#	define R500_TX_WEIGHT_1_SHIFT            (0)
#	define R500_TX_WEIGHT_0_SHIFT            (11)
#	define R500_TX_WEIGHT_PAIR               (1<<22)
#	define R500_TX_PHASE_SHIFT               (23)
#	define R500_TX_DIRECTION_HORIZONTAL	 (0<<27)
#	define R500_TX_DIRECTION_VERTICAL	 (1<<27)

#define R500_SU_TEX_WRAP_PS3		    0x4114

/* S Texture Coordinate of Vertex 0 for Point texture stuffing (LLC) */
#define R300_GA_POINT_S0                              0x4200

/* T Texture Coordinate of Vertex 0 for Point texture stuffing (LLC) */
#define R300_GA_POINT_T0                              0x4204

/* S Texture Coordinate of Vertex 2 for Point texture stuffing (URC) */
#define R300_GA_POINT_S1                              0x4208

/* T Texture Coordinate of Vertex 2 for Point texture stuffing (URC) */
#define R300_GA_POINT_T1                              0x420c

/* Specifies amount to shift integer position of vertex (screen space) before
 * converting to float for triangle stipple.
 */
#define R300_GA_TRIANGLE_STIPPLE            0x4214
#	define R300_GA_TRIANGLE_STIPPLE_X_SHIFT_SHIFT 0
#	define R300_GA_TRIANGLE_STIPPLE_X_SHIFT_MASK  0x0000000f
#	define R300_GA_TRIANGLE_STIPPLE_Y_SHIFT_SHIFT 16
#	define R300_GA_TRIANGLE_STIPPLE_Y_SHIFT_MASK  0x000f0000

/* The pointsize is given in multiples of 6. The pointsize can be enormous:
 * Clear() renders a single point that fills the entire framebuffer.
 * 1/2 Height of point; fixed (16.0), subpixel format (1/12 or 1/16, even if in
 * 8b precision).
 */
#define R300_GA_POINT_SIZE                   0x421C
#       define R300_POINTSIZE_Y_SHIFT         0
#       define R300_POINTSIZE_Y_MASK          0x0000ffff
#       define R300_POINTSIZE_X_SHIFT         16
#       define R300_POINTSIZE_X_MASK          0xffff0000
#       define R300_POINTSIZE_MAX             (R300_POINTSIZE_Y_MASK / 6)

/* Red fill color */
#define R500_GA_FILL_R                                0x4220

/* Green fill color */
#define R500_GA_FILL_G                                0x4224

/* Blue fill color */
#define R500_GA_FILL_B                                0x4228

/* Alpha fill color */
#define R500_GA_FILL_A                                0x422c


/* Specifies maximum and minimum point & sprite sizes for per vertex size
 * specification. The lower part (15:0) is MIN and (31:16) is max.
 */
#define R300_GA_POINT_MINMAX                0x4230
#       define R300_GA_POINT_MINMAX_MIN_SHIFT          0
#       define R300_GA_POINT_MINMAX_MIN_MASK           (0xFFFF << 0)
#       define R300_GA_POINT_MINMAX_MAX_SHIFT          16
#       define R300_GA_POINT_MINMAX_MAX_MASK           (0xFFFF << 16)

/* 1/2 width of line, in subpixels (1/12 or 1/16 only, even in 8b
 * subprecision); (16.0) fixed format.
 *
 * The line width is given in multiples of 6.
 * In default mode lines are classified as vertical lines.
 */
#define R300_GA_LINE_CNTL                             0x4234
#       define R300_GA_LINE_CNTL_WIDTH_SHIFT       0
#       define R300_GA_LINE_CNTL_WIDTH_MASK        0x0000ffff
#	define R300_GA_LINE_CNTL_END_TYPE_HOR      (0 << 16)
#	define R300_GA_LINE_CNTL_END_TYPE_VER      (1 << 16)
#	define R300_GA_LINE_CNTL_END_TYPE_SQR      (2 << 16) /* horizontal or vertical depending upon slope */
#	define R300_GA_LINE_CNTL_END_TYPE_COMP     (3 << 16) /* Computed (perpendicular to slope) */
#	define R500_GA_LINE_CNTL_SORT_NO           (0 << 18)
#	define R500_GA_LINE_CNTL_SORT_MINX_MINY    (1 << 18)

/* Line Stipple configuration information. */
#define R300_GA_LINE_STIPPLE_CONFIG                   0x4238
#	define R300_GA_LINE_STIPPLE_CONFIG_LINE_RESET_NO     (0 << 0)
#	define R300_GA_LINE_STIPPLE_CONFIG_LINE_RESET_LINE   (1 << 0)
#	define R300_GA_LINE_STIPPLE_CONFIG_LINE_RESET_PACKET (2 << 0)
#	define R300_GA_LINE_STIPPLE_CONFIG_STIPPLE_SCALE_SHIFT 2
#	define R300_GA_LINE_STIPPLE_CONFIG_STIPPLE_SCALE_MASK  0xfffffffc

/* Used to load US instructions and constants */
#define R500_GA_US_VECTOR_INDEX               0x4250
#	define R500_GA_US_VECTOR_INDEX_SHIFT       0
#	define R500_GA_US_VECTOR_INDEX_MASK        0x000000ff
#	define R500_GA_US_VECTOR_INDEX_TYPE_INSTR  (0 << 16)
#	define R500_GA_US_VECTOR_INDEX_TYPE_CONST  (1 << 16)
#	define R500_GA_US_VECTOR_INDEX_CLAMP_NO    (0 << 17)
#	define R500_GA_US_VECTOR_INDEX_CLAMP_CONST (1 << 17)

/* Data register for loading US instructions and constants */
#define R500_GA_US_VECTOR_DATA                0x4254

/* Specifies color properties and mappings of textures. */
#define R500_GA_COLOR_CONTROL_PS3                     0x4258
#	define R500_TEX0_SHADING_PS3_SOLID       (0 << 0)
#	define R500_TEX0_SHADING_PS3_FLAT        (1 << 0)
#	define R500_TEX0_SHADING_PS3_GOURAUD     (2 << 0)
#	define R500_TEX1_SHADING_PS3_SOLID       (0 << 2)
#	define R500_TEX1_SHADING_PS3_FLAT        (1 << 2)
#	define R500_TEX1_SHADING_PS3_GOURAUD     (2 << 2)
#	define R500_TEX2_SHADING_PS3_SOLID       (0 << 4)
#	define R500_TEX2_SHADING_PS3_FLAT        (1 << 4)
#	define R500_TEX2_SHADING_PS3_GOURAUD     (2 << 4)
#	define R500_TEX3_SHADING_PS3_SOLID       (0 << 6)
#	define R500_TEX3_SHADING_PS3_FLAT        (1 << 6)
#	define R500_TEX3_SHADING_PS3_GOURAUD     (2 << 6)
#	define R500_TEX4_SHADING_PS3_SOLID       (0 << 8)
#	define R500_TEX4_SHADING_PS3_FLAT        (1 << 8)
#	define R500_TEX4_SHADING_PS3_GOURAUD     (2 << 8)
#	define R500_TEX5_SHADING_PS3_SOLID       (0 << 10)
#	define R500_TEX5_SHADING_PS3_FLAT        (1 << 10)
#	define R500_TEX5_SHADING_PS3_GOURAUD     (2 << 10)
#	define R500_TEX6_SHADING_PS3_SOLID       (0 << 12)
#	define R500_TEX6_SHADING_PS3_FLAT        (1 << 12)
#	define R500_TEX6_SHADING_PS3_GOURAUD     (2 << 12)
#	define R500_TEX7_SHADING_PS3_SOLID       (0 << 14)
#	define R500_TEX7_SHADING_PS3_FLAT        (1 << 14)
#	define R500_TEX7_SHADING_PS3_GOURAUD     (2 << 14)
#	define R500_TEX8_SHADING_PS3_SOLID       (0 << 16)
#	define R500_TEX8_SHADING_PS3_FLAT        (1 << 16)
#	define R500_TEX8_SHADING_PS3_GOURAUD     (2 << 16)
#	define R500_TEX9_SHADING_PS3_SOLID       (0 << 18)
#	define R500_TEX9_SHADING_PS3_FLAT        (1 << 18)
#	define R500_TEX9_SHADING_PS3_GOURAUD     (2 << 18)
#	define R500_TEX10_SHADING_PS3_SOLID      (0 << 20)
#	define R500_TEX10_SHADING_PS3_FLAT       (1 << 20)
#	define R500_TEX10_SHADING_PS3_GOURAUD    (2 << 20)
#	define R500_COLOR0_TEX_OVERRIDE_NO       (0 << 22)
#	define R500_COLOR0_TEX_OVERRIDE_TEX_0    (1 << 22)
#	define R500_COLOR0_TEX_OVERRIDE_TEX_1    (2 << 22)
#	define R500_COLOR0_TEX_OVERRIDE_TEX_2    (3 << 22)
#	define R500_COLOR0_TEX_OVERRIDE_TEX_3    (4 << 22)
#	define R500_COLOR0_TEX_OVERRIDE_TEX_4    (5 << 22)
#	define R500_COLOR0_TEX_OVERRIDE_TEX_5    (6 << 22)
#	define R500_COLOR0_TEX_OVERRIDE_TEX_6    (7 << 22)
#	define R500_COLOR0_TEX_OVERRIDE_TEX_7    (8 << 22)
#	define R500_COLOR0_TEX_OVERRIDE_TEX_8_C2 (9 << 22)
#	define R500_COLOR0_TEX_OVERRIDE_TEX_9_C3 (10 << 22)
#	define R500_COLOR1_TEX_OVERRIDE_NO       (0 << 26)
#	define R500_COLOR1_TEX_OVERRIDE_TEX_0    (1 << 26)
#	define R500_COLOR1_TEX_OVERRIDE_TEX_1    (2 << 26)
#	define R500_COLOR1_TEX_OVERRIDE_TEX_2    (3 << 26)
#	define R500_COLOR1_TEX_OVERRIDE_TEX_3    (4 << 26)
#	define R500_COLOR1_TEX_OVERRIDE_TEX_4    (5 << 26)
#	define R500_COLOR1_TEX_OVERRIDE_TEX_5    (6 << 26)
#	define R500_COLOR1_TEX_OVERRIDE_TEX_6    (7 << 26)
#	define R500_COLOR1_TEX_OVERRIDE_TEX_7    (8 << 26)
#	define R500_COLOR1_TEX_OVERRIDE_TEX_8_C2 (9 << 26)
#	define R500_COLOR1_TEX_OVERRIDE_TEX_9_C3 (10 << 26)

/* Returns idle status of various G3D block, captured when GA_IDLE written or
 * when hard or soft reset asserted.
 */
#define R500_GA_IDLE                                  0x425c
#	define R500_GA_IDLE_PIPE3_Z_IDLE  (0 << 0)
#	define R500_GA_IDLE_PIPE2_Z_IDLE  (0 << 1)
#	define R500_GA_IDLE_PIPE3_CD_IDLE (0 << 2)
#	define R500_GA_IDLE_PIPE2_CD_IDLE (0 << 3)
#	define R500_GA_IDLE_PIPE3_FG_IDLE (0 << 4)
#	define R500_GA_IDLE_PIPE2_FG_IDLE (0 << 5)
#	define R500_GA_IDLE_PIPE3_US_IDLE (0 << 6)
#	define R500_GA_IDLE_PIPE2_US_IDLE (0 << 7)
#	define R500_GA_IDLE_PIPE3_SC_IDLE (0 << 8)
#	define R500_GA_IDLE_PIPE2_SC_IDLE (0 << 9)
#	define R500_GA_IDLE_PIPE3_RS_IDLE (0 << 10)
#	define R500_GA_IDLE_PIPE2_RS_IDLE (0 << 11)
#	define R500_GA_IDLE_PIPE1_Z_IDLE  (0 << 12)
#	define R500_GA_IDLE_PIPE0_Z_IDLE  (0 << 13)
#	define R500_GA_IDLE_PIPE1_CD_IDLE (0 << 14)
#	define R500_GA_IDLE_PIPE0_CD_IDLE (0 << 15)
#	define R500_GA_IDLE_PIPE1_FG_IDLE (0 << 16)
#	define R500_GA_IDLE_PIPE0_FG_IDLE (0 << 17)
#	define R500_GA_IDLE_PIPE1_US_IDLE (0 << 18)
#	define R500_GA_IDLE_PIPE0_US_IDLE (0 << 19)
#	define R500_GA_IDLE_PIPE1_SC_IDLE (0 << 20)
#	define R500_GA_IDLE_PIPE0_SC_IDLE (0 << 21)
#	define R500_GA_IDLE_PIPE1_RS_IDLE (0 << 22)
#	define R500_GA_IDLE_PIPE0_RS_IDLE (0 << 23)
#	define R500_GA_IDLE_SU_IDLE       (0 << 24)
#	define R500_GA_IDLE_GA_IDLE       (0 << 25)
#	define R500_GA_IDLE_GA_UNIT2_IDLE (0 << 26)

/* Current value of stipple accumulator. */
#define R300_GA_LINE_STIPPLE_VALUE            0x4260

/* S Texture Coordinate Value for Vertex 0 of Line (stuff textures -- i.e. AA) */
#define R300_GA_LINE_S0                               0x4264
/* S Texture Coordinate Value for Vertex 1 of Lines (V2 of parallelogram -- stuff textures -- i.e. AA) */
#define R300_GA_LINE_S1                               0x4268

/* GA Input fifo high water marks */
#define R500_GA_FIFO_CNTL                             0x4270
#	define R500_GA_FIFO_CNTL_VERTEX_FIFO_MASK   0x00000007
#	define R500_GA_FIFO_CNTL_VERTEX_FIFO_SHIFT  0
#	define R500_GA_FIFO_CNTL_VERTEX_INDEX_MASK  0x00000038
#	define R500_GA_FIFO_CNTL_VERTEX_INDEX_SHIFT 3
#	define R500_GA_FIFO_CNTL_VERTEX_REG_MASK    0x00003fc0
#	define R500_GA_FIFO_CNTL_VERTEX_REG_SHIFT   6

/* GA enhance/tweaks */
#define R300_GA_ENHANCE                               0x4274
#	define R300_GA_ENHANCE_DEADLOCK_CNTL_NO_EFFECT   (0 << 0)
#	define R300_GA_ENHANCE_DEADLOCK_CNTL_PREVENT_TCL (1 << 0) /* Prevents TCL interface from deadlocking on GA side. */
#	define R300_GA_ENHANCE_FASTSYNC_CNTL_NO_EFFECT   (0 << 1)
#	define R300_GA_ENHANCE_FASTSYNC_CNTL_ENABLE      (1 << 1) /* Enables high-performance register/primitive switching. */
#	define R500_GA_ENHANCE_REG_READWRITE_NO_EFFECT   (0 << 2) /* R520+ only */
#	define R500_GA_ENHANCE_REG_READWRITE_ENABLE      (1 << 2) /* R520+ only, Enables GA support of simultaneous register reads and writes. */
#	define R500_GA_ENHANCE_REG_NOSTALL_NO_EFFECT     (0 << 3)
#	define R500_GA_ENHANCE_REG_NOSTALL_ENABLE        (1 << 3) /* Enables GA support of no-stall reads for register read back. */

#define R300_GA_COLOR_CONTROL                   0x4278
#	define R300_GA_COLOR_CONTROL_RGB0_SHADING_SOLID      (0 << 0)
#	define R300_GA_COLOR_CONTROL_RGB0_SHADING_FLAT       (1 << 0)
#	define R300_GA_COLOR_CONTROL_RGB0_SHADING_GOURAUD    (2 << 0)
#	define R300_GA_COLOR_CONTROL_ALPHA0_SHADING_SOLID    (0 << 2)
#	define R300_GA_COLOR_CONTROL_ALPHA0_SHADING_FLAT     (1 << 2)
#	define R300_GA_COLOR_CONTROL_ALPHA0_SHADING_GOURAUD  (2 << 2)
#	define R300_GA_COLOR_CONTROL_RGB1_SHADING_SOLID      (0 << 4)
#	define R300_GA_COLOR_CONTROL_RGB1_SHADING_FLAT       (1 << 4)
#	define R300_GA_COLOR_CONTROL_RGB1_SHADING_GOURAUD    (2 << 4)
#	define R300_GA_COLOR_CONTROL_ALPHA1_SHADING_SOLID    (0 << 6)
#	define R300_GA_COLOR_CONTROL_ALPHA1_SHADING_FLAT     (1 << 6)
#	define R300_GA_COLOR_CONTROL_ALPHA1_SHADING_GOURAUD  (2 << 6)
#	define R300_GA_COLOR_CONTROL_RGB2_SHADING_SOLID      (0 << 8)
#	define R300_GA_COLOR_CONTROL_RGB2_SHADING_FLAT       (1 << 8)
#	define R300_GA_COLOR_CONTROL_RGB2_SHADING_GOURAUD    (2 << 8)
#	define R300_GA_COLOR_CONTROL_ALPHA2_SHADING_SOLID    (0 << 10)
#	define R300_GA_COLOR_CONTROL_ALPHA2_SHADING_FLAT     (1 << 10)
#	define R300_GA_COLOR_CONTROL_ALPHA2_SHADING_GOURAUD  (2 << 10)
#	define R300_GA_COLOR_CONTROL_RGB3_SHADING_SOLID      (0 << 12)
#	define R300_GA_COLOR_CONTROL_RGB3_SHADING_FLAT       (1 << 12)
#	define R300_GA_COLOR_CONTROL_RGB3_SHADING_GOURAUD    (2 << 12)
#	define R300_GA_COLOR_CONTROL_ALPHA3_SHADING_SOLID    (0 << 14)
#	define R300_GA_COLOR_CONTROL_ALPHA3_SHADING_FLAT     (1 << 14)
#	define R300_GA_COLOR_CONTROL_ALPHA3_SHADING_GOURAUD  (2 << 14)
#	define R300_GA_COLOR_CONTROL_PROVOKING_VERTEX_FIRST  (0 << 16)
#	define R300_GA_COLOR_CONTROL_PROVOKING_VERTEX_SECOND (1 << 16)
#	define R300_GA_COLOR_CONTROL_PROVOKING_VERTEX_THIRD  (2 << 16)
#	define R300_GA_COLOR_CONTROL_PROVOKING_VERTEX_LAST   (3 << 16)

#       define R300_SHADE_MODEL_FLAT ( \
        R300_GA_COLOR_CONTROL_RGB0_SHADING_FLAT | \
        R300_GA_COLOR_CONTROL_ALPHA0_SHADING_FLAT | \
        R300_GA_COLOR_CONTROL_RGB1_SHADING_FLAT | \
        R300_GA_COLOR_CONTROL_ALPHA1_SHADING_FLAT | \
        R300_GA_COLOR_CONTROL_RGB2_SHADING_FLAT | \
        R300_GA_COLOR_CONTROL_ALPHA2_SHADING_FLAT | \
        R300_GA_COLOR_CONTROL_RGB3_SHADING_FLAT | \
        R300_GA_COLOR_CONTROL_ALPHA3_SHADING_FLAT )

#       define R300_SHADE_MODEL_SMOOTH ( \
        R300_GA_COLOR_CONTROL_RGB0_SHADING_GOURAUD | \
        R300_GA_COLOR_CONTROL_ALPHA0_SHADING_GOURAUD | \
        R300_GA_COLOR_CONTROL_RGB1_SHADING_GOURAUD | \
        R300_GA_COLOR_CONTROL_ALPHA1_SHADING_GOURAUD | \
        R300_GA_COLOR_CONTROL_RGB2_SHADING_GOURAUD | \
        R300_GA_COLOR_CONTROL_ALPHA2_SHADING_GOURAUD | \
        R300_GA_COLOR_CONTROL_RGB3_SHADING_GOURAUD | \
        R300_GA_COLOR_CONTROL_ALPHA3_SHADING_GOURAUD )

/* Specifies red & green components of fill color -- S312 format -- Backwards comp. */
#define R300_GA_SOLID_RG                         0x427c
#	define GA_SOLID_RG_COLOR_GREEN_SHIFT 0
#	define GA_SOLID_RG_COLOR_GREEN_MASK  0x0000ffff
#	define GA_SOLID_RG_COLOR_RED_SHIFT   16
#	define GA_SOLID_RG_COLOR_RED_MASK    0xffff0000
/* Specifies blue & alpha components of fill color -- S312 format -- Backwards comp. */
#define R300_GA_SOLID_BA                         0x4280
#	define GA_SOLID_BA_COLOR_ALPHA_SHIFT 0
#	define GA_SOLID_BA_COLOR_ALPHA_MASK  0x0000ffff
#	define GA_SOLID_BA_COLOR_BLUE_SHIFT  16
#	define GA_SOLID_BA_COLOR_BLUE_MASK   0xffff0000

/* Polygon Mode
 * Dangerous
 */
#define R300_GA_POLY_MODE                             0x4288
#	define R300_GA_POLY_MODE_DISABLE           (0 << 0)
#	define R300_GA_POLY_MODE_DUAL              (1 << 0) /* send 2 sets of 3 polys with specified poly type */
/* reserved */
#	define R300_GA_POLY_MODE_FRONT_PTYPE_POINT (0 << 4)
#	define R300_GA_POLY_MODE_FRONT_PTYPE_LINE  (1 << 4)
#	define R300_GA_POLY_MODE_FRONT_PTYPE_TRI   (2 << 4)
/* reserved */
#	define R300_GA_POLY_MODE_BACK_PTYPE_POINT  (0 << 7)
#	define R300_GA_POLY_MODE_BACK_PTYPE_LINE   (1 << 7)
#	define R300_GA_POLY_MODE_BACK_PTYPE_TRI    (2 << 7)
/* reserved */

/* Specifies the rounding mode for geometry & color SPFP to FP conversions. */
#define R300_GA_ROUND_MODE                            0x428c
#	define R300_GA_ROUND_MODE_GEOMETRY_ROUND_TRUNC   (0 << 0)
#	define R300_GA_ROUND_MODE_GEOMETRY_ROUND_NEAREST (1 << 0)
#	define R300_GA_ROUND_MODE_COLOR_ROUND_TRUNC      (0 << 2)
#	define R300_GA_ROUND_MODE_COLOR_ROUND_NEAREST    (1 << 2)
#	define R300_GA_ROUND_MODE_RGB_CLAMP_RGB          (0 << 4)
#	define R300_GA_ROUND_MODE_RGB_CLAMP_FP20         (1 << 4)
#	define R300_GA_ROUND_MODE_ALPHA_CLAMP_RGB        (0 << 5)
#	define R300_GA_ROUND_MODE_ALPHA_CLAMP_FP20       (1 << 5)
#	define R500_GA_ROUND_MODE_GEOMETRY_MASK_SHIFT    6
#	define R500_GA_ROUND_MODE_GEOMETRY_MASK_MASK     0x000003c0

/* Specifies x & y offsets for vertex data after conversion to FP.
 * Offsets are in S15 format (subpixels -- 1/12 or 1/16, even in 8b
 * subprecision).
 */
#define R300_GA_OFFSET                                0x4290
#	define R300_GA_OFFSET_X_OFFSET_SHIFT 0
#	define R300_GA_OFFSET_X_OFFSET_MASK  0x0000ffff
#	define R300_GA_OFFSET_Y_OFFSET_SHIFT 16
#	define R300_GA_OFFSET_Y_OFFSET_MASK  0xffff0000

/* Specifies the scale to apply to fog. */
#define R300_GA_FOG_SCALE                     0x4294
/* Specifies the offset to apply to fog. */
#define R300_GA_FOG_OFFSET                    0x4298
/* Specifies number of cycles to assert reset, and also causes RB3D soft reset to assert. */
#define R300_GA_SOFT_RESET                    0x429c

/* Not sure why there are duplicate of factor and constant values.
 * My best guess so far is that there are separate zbiases for test and write.
 * Ordering might be wrong.
 * Some of the tests indicate that fgl has a fallback implementation of zbias
 * via pixel shaders.
 */
#define R300_SU_TEX_WRAP                      0x42A0
#define R300_SU_POLY_OFFSET_FRONT_SCALE       0x42A4
#define R300_SU_POLY_OFFSET_FRONT_OFFSET      0x42A8
#define R300_SU_POLY_OFFSET_BACK_SCALE        0x42AC
#define R300_SU_POLY_OFFSET_BACK_OFFSET       0x42B0

/* This register needs to be set to (1<<1) for RV350 to correctly
 * perform depth test (see --vb-triangles in r300_demo)
 * Don't know about other chips. - Vladimir
 * This is set to 3 when GL_POLYGON_OFFSET_FILL is on.
 * My guess is that there are two bits for each zbias primitive
 * (FILL, LINE, POINT).
 *  One to enable depth test and one for depth write.
 * Yet this doesn't explain why depth writes work ...
 */
#define R300_SU_POLY_OFFSET_ENABLE	       0x42B4
#	define R300_FRONT_ENABLE	       (1 << 0)
#	define R300_BACK_ENABLE 	       (1 << 1)
#	define R300_PARA_ENABLE 	       (1 << 2)

#define R300_SU_CULL_MODE                      0x42B8
#       define R300_CULL_FRONT                   (1 << 0)
#       define R300_CULL_BACK                    (1 << 1)
#       define R300_FRONT_FACE_CCW               (0 << 2)
#       define R300_FRONT_FACE_CW                (1 << 2)

/* SU Depth Scale value */
#define R300_SU_DEPTH_SCALE                 0x42c0
/* SU Depth Offset value */
#define R300_SU_DEPTH_OFFSET                0x42c4

#define R300_SU_REG_DEST		    0x42c8
#	define R300_RASTER_PIPE_SELECT_0	(1 << 0)
#	define R300_RASTER_PIPE_SELECT_1	(1 << 1)
#	define R300_RASTER_PIPE_SELECT_2	(1 << 2)
#	define R300_RASTER_PIPE_SELECT_3	(1 << 3)
#	define R300_RASTER_PIPE_SELECT_ALL	0xf


/* BEGIN: Rasterization / Interpolators - many guesses */

/*
 * TC_CNT is the number of incoming texture coordinate sets (i.e. it depends
 * on the vertex program, *not* the fragment program)
 */
#define R300_RS_COUNT                      0x4300
#       define R300_IT_COUNT_SHIFT               0
#       define R300_IT_COUNT_MASK                0x0000007f
#       define R300_IC_COUNT_SHIFT               7
#       define R300_IC_COUNT_MASK                0x00000780
#       define R300_W_ADDR_SHIFT                 12
#       define R300_W_ADDR_MASK                  0x0003f000
#       define R300_HIRES_DIS                    (0 << 18)
#       define R300_HIRES_EN                     (1 << 18)
#       define R300_IT_COUNT(x)                  ((x) << 0)
#       define R300_IC_COUNT(x)                  ((x) << 7)
#       define R300_W_COUNT(x)                   ((x) << 12)

#define R300_RS_INST_COUNT                       0x4304
#       define R300_RS_INST_COUNT_SHIFT          0
#       define R300_RS_INST_COUNT_MASK           0x0000000f
#       define R300_RS_TX_OFFSET_SHIFT           5
#	define R300_RS_TX_OFFSET_MASK            0x000000e0
#       define R300_RS_TX_OFFSET(x)              ((x) << 5)

/* gap */

/* Only used for texture coordinates.
 * Use the source field to route texture coordinate input from the
 * vertex program to the desired interpolator. Note that the source
 * field is relative to the outputs the vertex program *actually*
 * writes. If a vertex program only writes texcoord[1], this will
 * be source index 0.
 * Set INTERP_USED on all interpolators that produce data used by
 * the fragment program. INTERP_USED looks like a swizzling mask,
 * but I haven't seen it used that way.
 *
 * Note: The _UNKNOWN constants are always set in their respective
 * register. I don't know if this is necessary.
 */
#define R300_RS_IP_0				        0x4310
#define R300_RS_IP_1				        0x4314
#define R300_RS_IP_2				        0x4318
#define R300_RS_IP_3				        0x431C
#	define R300_RS_TEX_PTR(x)		        (x << 0)
#	define R300_RS_COL_PTR(x)		        ((x) << 6)
#	define R300_RS_COL_FMT(x)		        ((x) << 9)
#	define R300_RS_COL_FMT_RGBA		        0
#	define R300_RS_COL_FMT_RGB0		        1
#	define R300_RS_COL_FMT_RGB1		        2
#	define R300_RS_COL_FMT_000A		        4
#	define R300_RS_COL_FMT_0000		        5
#	define R300_RS_COL_FMT_0001		        6
#	define R300_RS_COL_FMT_111A		        8
#	define R300_RS_COL_FMT_1110		        9
#	define R300_RS_COL_FMT_1111		        10
#	define R300_RS_SEL_S(x)		                ((x) << 13)
#	define R300_RS_SEL_T(x)		                ((x) << 16)
#	define R300_RS_SEL_R(x)		                ((x) << 19)
#	define R300_RS_SEL_Q(x)		                ((x) << 22)
#	define R300_RS_SEL_C0		                0
#	define R300_RS_SEL_C1		                1
#	define R300_RS_SEL_C2		                2
#	define R300_RS_SEL_C3		                3
#	define R300_RS_SEL_K0		                4
#	define R300_RS_SEL_K1		                5


/*  */
#define R500_RS_INST_0					0x4320
#define R500_RS_INST_1					0x4324
#define R500_RS_INST_2					0x4328
#define R500_RS_INST_3					0x432c
#define R500_RS_INST_4					0x4330
#define R500_RS_INST_5					0x4334
#define R500_RS_INST_6					0x4338
#define R500_RS_INST_7					0x433c
#define R500_RS_INST_8					0x4340
#define R500_RS_INST_9					0x4344
#define R500_RS_INST_10					0x4348
#define R500_RS_INST_11					0x434c
#define R500_RS_INST_12					0x4350
#define R500_RS_INST_13					0x4354
#define R500_RS_INST_14					0x4358
#define R500_RS_INST_15					0x435c
#define R500_RS_INST_TEX_ID_SHIFT			0
#        define R500_RS_INST_TEX_ID(x)                  ((x) << 0)
#define R500_RS_INST_TEX_CN_WRITE			(1 << 4)
#define R500_RS_INST_TEX_ADDR_SHIFT			5
#        define R500_RS_INST_TEX_ADDR(x)                ((x) << 5)
#define R500_RS_INST_COL_ID_SHIFT			12
#        define R500_RS_INST_COL_ID(x)                  ((x) << 12)
#define R500_RS_INST_COL_CN_NO_WRITE			(0 << 16)
#define R500_RS_INST_COL_CN_WRITE			(1 << 16)
#define R500_RS_INST_COL_CN_WRITE_FBUFFER		(2 << 16)
#define R500_RS_INST_COL_CN_WRITE_BACKFACE		(3 << 16)
#define R500_RS_INST_COL_ADDR_SHIFT			18
#        define R500_RS_INST_COL_ADDR(x)                ((x) << 18)
#define R500_RS_INST_TEX_ADJ				(1 << 25)
#define R500_RS_INST_W_CN				(1 << 26)

/* These DWORDs control how vertex data is routed into fragment program
 * registers, after interpolators.
 */
#define R300_RS_INST_0                     0x4330
#define R300_RS_INST_1                     0x4334
#define R300_RS_INST_2                     0x4338
#define R300_RS_INST_3                     0x433C
#define R300_RS_INST_4                     0x4340
#define R300_RS_INST_5                     0x4344
#define R300_RS_INST_6                     0x4348
#define R300_RS_INST_7                     0x434C
#	define R300_RS_INST_TEX_ID(x)  		((x) << 0)
#	define R300_RS_INST_TEX_CN_WRITE 	(1 << 3)
#	define R300_RS_INST_TEX_ADDR(x)		((x) << 6)
#	define R300_RS_INST_TEX_ADDR_SHIFT 	6
#	define R300_RS_INST_COL_ID(x)		((x) << 11)
#	define R300_RS_INST_COL_CN_WRITE	(1 << 14)
#	define R300_RS_INST_COL_ADDR(x)		((x) << 17)
#	define R300_RS_INST_COL_ADDR_SHIFT	17
#	define R300_RS_INST_TEX_ADJ		(1 << 22)
#	define R300_RS_COL_BIAS_UNUSED_SHIFT    23

/* END: Rasterization / Interpolators - many guesses */

/* Hierarchical Z Enable */
#define R300_SC_HYPERZ                   0x43a4
#	define R300_SC_HYPERZ_DISABLE     (0 << 0)
#	define R300_SC_HYPERZ_ENABLE      (1 << 0)
#	define R300_SC_HYPERZ_MIN         (0 << 1)
#	define R300_SC_HYPERZ_MAX         (1 << 1)
#	define R300_SC_HYPERZ_ADJ_256     (0 << 2)
#	define R300_SC_HYPERZ_ADJ_128     (1 << 2)
#	define R300_SC_HYPERZ_ADJ_64      (2 << 2)
#	define R300_SC_HYPERZ_ADJ_32      (3 << 2)
#	define R300_SC_HYPERZ_ADJ_16      (4 << 2)
#	define R300_SC_HYPERZ_ADJ_8       (5 << 2)
#	define R300_SC_HYPERZ_ADJ_4       (6 << 2)
#	define R300_SC_HYPERZ_ADJ_2       (7 << 2)
#	define R300_SC_HYPERZ_HZ_Z0MIN_NO (0 << 5)
#	define R300_SC_HYPERZ_HZ_Z0MIN    (1 << 5)
#	define R300_SC_HYPERZ_HZ_Z0MAX_NO (0 << 6)
#	define R300_SC_HYPERZ_HZ_Z0MAX    (1 << 6)

#define R300_SC_EDGERULE                 0x43a8

/* BEGIN: Scissors and cliprects */

/* There are four clipping rectangles. Their corner coordinates are inclusive.
 * Every pixel is assigned a number from 0 and 15 by setting bits 0-3 depending
 * on whether the pixel is inside cliprects 0-3, respectively. For example,
 * if a pixel is inside cliprects 0 and 1, but outside 2 and 3, it is assigned
 * the number 3 (binary 0011).
 * Iff the bit corresponding to the pixel's number in RE_CLIPRECT_CNTL is set,
 * the pixel is rasterized.
 *
 * In addition to this, there is a scissors rectangle. Only pixels inside the
 * scissors rectangle are drawn. (coordinates are inclusive)
 *
 * For some reason, the top-left corner of the framebuffer is at (1440, 1440)
 * for the purpose of clipping and scissors.
 */
#define R300_SC_CLIPRECT_TL_0               0x43B0
#define R300_SC_CLIPRECT_BR_0               0x43B4
#define R300_SC_CLIPRECT_TL_1               0x43B8
#define R300_SC_CLIPRECT_BR_1               0x43BC
#define R300_SC_CLIPRECT_TL_2               0x43C0
#define R300_SC_CLIPRECT_BR_2               0x43C4
#define R300_SC_CLIPRECT_TL_3               0x43C8
#define R300_SC_CLIPRECT_BR_3               0x43CC
#       define R300_CLIPRECT_OFFSET              1440
#       define R300_CLIPRECT_MASK                0x1FFF
#       define R300_CLIPRECT_X_SHIFT             0
#       define R300_CLIPRECT_X_MASK              (0x1FFF << 0)
#       define R300_CLIPRECT_Y_SHIFT             13
#       define R300_CLIPRECT_Y_MASK              (0x1FFF << 13)
#define R300_SC_CLIP_RULE                   0x43D0
#       define R300_CLIP_OUT                     (1 << 0)
#       define R300_CLIP_0                       (1 << 1)
#       define R300_CLIP_1                       (1 << 2)
#       define R300_CLIP_10                      (1 << 3)
#       define R300_CLIP_2                       (1 << 4)
#       define R300_CLIP_20                      (1 << 5)
#       define R300_CLIP_21                      (1 << 6)
#       define R300_CLIP_210                     (1 << 7)
#       define R300_CLIP_3                       (1 << 8)
#       define R300_CLIP_30                      (1 << 9)
#       define R300_CLIP_31                      (1 << 10)
#       define R300_CLIP_310                     (1 << 11)
#       define R300_CLIP_32                      (1 << 12)
#       define R300_CLIP_320                     (1 << 13)
#       define R300_CLIP_321                     (1 << 14)
#       define R300_CLIP_3210                    (1 << 15)

/* gap */

#define R300_SC_SCISSORS_TL                 0x43E0
#define R300_SC_SCISSORS_BR                 0x43E4
#       define R300_SCISSORS_OFFSET              1440
#       define R300_SCISSORS_X_SHIFT             0
#       define R300_SCISSORS_X_MASK              (0x1FFF << 0)
#       define R300_SCISSORS_Y_SHIFT             13
#       define R300_SCISSORS_Y_MASK              (0x1FFF << 13)

/* Screen door sample mask */
#define R300_SC_SCREENDOOR                 0x43e8

/* END: Scissors and cliprects */

/* BEGIN: Texture specification */

/*
 * The texture specification dwords are grouped by meaning and not by texture
 * unit. This means that e.g. the offset for texture image unit N is found in
 * register TX_OFFSET_0 + (4*N)
 */
#define R300_TX_FILTER0_0                        0x4400
#define R300_TX_FILTER0_1                        0x4404
#define R300_TX_FILTER0_2                        0x4408
#define R300_TX_FILTER0_3                        0x440c
#define R300_TX_FILTER0_4                        0x4410
#define R300_TX_FILTER0_5                        0x4414
#define R300_TX_FILTER0_6                        0x4418
#define R300_TX_FILTER0_7                        0x441c
#define R300_TX_FILTER0_8                        0x4420
#define R300_TX_FILTER0_9                        0x4424
#define R300_TX_FILTER0_10                       0x4428
#define R300_TX_FILTER0_11                       0x442c
#define R300_TX_FILTER0_12                       0x4430
#define R300_TX_FILTER0_13                       0x4434
#define R300_TX_FILTER0_14                       0x4438
#define R300_TX_FILTER0_15                       0x443c
#       define R300_TX_REPEAT                    0
#       define R300_TX_MIRRORED                  1
#       define R300_TX_CLAMP_TO_EDGE             2
#	define R300_TX_MIRROR_ONCE_TO_EDGE       3
#       define R300_TX_CLAMP                     4
#	define R300_TX_MIRROR_ONCE               5
#       define R300_TX_CLAMP_TO_BORDER           6
#	define R300_TX_MIRROR_ONCE_TO_BORDER     7
#       define R300_TX_WRAP_S_SHIFT              0
#       define R300_TX_WRAP_S_MASK               (7 << 0)
#       define R300_TX_WRAP_T_SHIFT              3
#       define R300_TX_WRAP_T_MASK               (7 << 3)
#       define R300_TX_WRAP_R_SHIFT              6
#       define R300_TX_WRAP_R_MASK               (7 << 6)
#	define R300_TX_MAG_FILTER_4              (0 << 9)
#       define R300_TX_MAG_FILTER_NEAREST        (1 << 9)
#       define R300_TX_MAG_FILTER_LINEAR         (2 << 9)
#       define R300_TX_MAG_FILTER_ANISO          (3 << 9)
#       define R300_TX_MAG_FILTER_MASK           (3 << 9)
#       define R300_TX_MIN_FILTER_NEAREST        (1 << 11)
#       define R300_TX_MIN_FILTER_LINEAR         (2 << 11)
#	define R300_TX_MIN_FILTER_ANISO          (3 << 11)
#	define R300_TX_MIN_FILTER_MASK           (3 << 11)
#	define R300_TX_MIN_FILTER_MIP_NONE       (0 << 13)
#	define R300_TX_MIN_FILTER_MIP_NEAREST    (1 << 13)
#	define R300_TX_MIN_FILTER_MIP_LINEAR     (2 << 13)
#	define R300_TX_MIN_FILTER_MIP_MASK       (3 << 13)
#       define R300_TX_MAX_MIP_LEVEL_SHIFT       17
#       define R300_TX_MAX_MIP_LEVEL_MASK        (0xf << 17)
#	define R300_TX_MAX_ANISO_1_TO_1          (0 << 21)
#	define R300_TX_MAX_ANISO_2_TO_1          (1 << 21)
#	define R300_TX_MAX_ANISO_4_TO_1          (2 << 21)
#	define R300_TX_MAX_ANISO_8_TO_1          (3 << 21)
#	define R300_TX_MAX_ANISO_16_TO_1         (4 << 21)
#	define R300_TX_MAX_ANISO_MASK            (7 << 21)
#       define R300_TX_WRAP_S(x)                 ((x) << 0)
#       define R300_TX_WRAP_T(x)                 ((x) << 3)
#       define R300_TX_MAX_MIP_LEVEL(x)          ((x) << 17)

#define R300_TX_FILTER1_0                      0x4440
#	define R300_CHROMA_KEY_MODE_DISABLE    0
#	define R300_CHROMA_KEY_FORCE	       1
#	define R300_CHROMA_KEY_BLEND           2
#	define R300_MC_ROUND_NORMAL            (0<<2)
#	define R300_MC_ROUND_MPEG4             (1<<2)
#	define R300_LOD_BIAS_SHIFT             3
#	define R300_LOD_BIAS_MASK	       0x1ff8
#	define R300_EDGE_ANISO_EDGE_DIAG       (0<<13)
#	define R300_EDGE_ANISO_EDGE_ONLY       (1<<13)
#	define R300_MC_COORD_TRUNCATE_DISABLE  (0<<14)
#	define R300_MC_COORD_TRUNCATE_MPEG     (1<<14)
#	define R300_TX_TRI_PERF_0_8            (0<<15)
#	define R300_TX_TRI_PERF_1_8            (1<<15)
#	define R300_TX_TRI_PERF_1_4            (2<<15)
#	define R300_TX_TRI_PERF_3_8            (3<<15)
#	define R300_ANISO_THRESHOLD_MASK       (7<<17)

#       define R400_DXTC_SWIZZLE_ENABLE        (1<<21)
#	define R500_MACRO_SWITCH               (1<<22)
#       define R500_TX_MAX_ANISO(x)            ((x) << 23)
#       define R500_TX_MAX_ANISO_MASK          (63 << 23)
#       define R500_TX_ANISO_HIGH_QUALITY      (1 << 30)
#	define R500_BORDER_FIX                 (1<<31)

#define R300_TX_FORMAT0_0                   0x4480
#       define R300_TX_WIDTHMASK_SHIFT           0
#       define R300_TX_WIDTHMASK_MASK            (2047 << 0)
#       define R300_TX_HEIGHTMASK_SHIFT          11
#       define R300_TX_HEIGHTMASK_MASK           (2047 << 11)
#	define R300_TX_DEPTHMASK_SHIFT           22
#	define R300_TX_DEPTHMASK_MASK            (0xf << 22)
#       define R300_TX_SIZE_PROJECTED            (1 << 30)
#       define R300_TX_PITCH_EN                  (1 << 31)
#       define R300_TX_WIDTH(x)                  ((x) << 0)
#       define R300_TX_HEIGHT(x)                 ((x) << 11)
#       define R300_TX_DEPTH(x)                  ((x) << 22)
#       define R300_TX_NUM_LEVELS(x)             ((x) << 26)

#define R300_TX_FORMAT1_0                   0x44C0
	/* The interpretation of the format word by Wladimir van der Laan */
	/* The X, Y, Z and W refer to the layout of the components.
	   They are given meanings as R, G, B and Alpha by the swizzle
	   specification */
#	define R300_TX_FORMAT_X8		    0x0
#	define R300_TX_FORMAT_X16		    0x1
#	define R300_TX_FORMAT_Y4X4		    0x2
#	define R300_TX_FORMAT_Y8X8		    0x3
#	define R300_TX_FORMAT_Y16X16		    0x4
#	define R300_TX_FORMAT_Z3Y3X2		    0x5
#	define R300_TX_FORMAT_Z5Y6X5		    0x6
#	define R300_TX_FORMAT_Z6Y5X5		    0x7
#	define R300_TX_FORMAT_Z11Y11X10		    0x8
#	define R300_TX_FORMAT_Z10Y11X11		    0x9
#	define R300_TX_FORMAT_W4Z4Y4X4		    0xA
#	define R300_TX_FORMAT_W1Z5Y5X5		    0xB
#	define R300_TX_FORMAT_W8Z8Y8X8		    0xC
#	define R300_TX_FORMAT_W2Z10Y10X10	    0xD
#	define R300_TX_FORMAT_W16Z16Y16X16	    0xE
#	define R300_TX_FORMAT_DXT1	    	    0xF
#	define R300_TX_FORMAT_DXT3	    	    0x10
#	define R300_TX_FORMAT_DXT5	    	    0x11
#	define R300_TX_FORMAT_CxV8U8           	    0x12
#	define R300_TX_FORMAT_AVYU444 	    	    0x13
#	define R300_TX_FORMAT_VYUY422  	    	    0x14
#	define R300_TX_FORMAT_YVYU422  	    	    0x15
#	define R300_TX_FORMAT_16_MPEG  	    	    0x16
#	define R300_TX_FORMAT_16_16_MPEG    	    0x17
#	define R300_TX_FORMAT_16F     	    	    0x18
#	define R300_TX_FORMAT_16F_16F 	    	    0x19
#	define R300_TX_FORMAT_16F_16F_16F_16F  	    0x1A
#	define R300_TX_FORMAT_32F     	    	    0x1B
#	define R300_TX_FORMAT_32F_32F 	    	    0x1C
#	define R300_TX_FORMAT_32F_32F_32F_32F  	    0x1D
#       define R300_TX_FORMAT_W24_FP                0x1E
#       define R400_TX_FORMAT_ATI2N                 0x1F

/* These need TX_FORMAT2_[0-15].TXFORMAT_MSB set.

   My guess is the 10-bit formats are the 8-bit ones but with filtering being
   performed with the precision of 10 bits per channel. This makes sense
   with sRGB textures since the conversion to linear space reduces the precision
   significantly so the shader gets approximately the 8-bit precision
   in the end. It might also improve the quality of HDR rendering where
   high-precision filtering is desirable.

   Again, this is guessed, the formats might mean something entirely else.
   The others should be fine. */
#       define R500_TX_FORMAT_X1                    0x0
#       define R500_TX_FORMAT_X1_REV                0x1
#       define R500_TX_FORMAT_X10                   0x2
#       define R500_TX_FORMAT_Y10X10                0x3
#       define R500_TX_FORMAT_W10Z10Y10X10          0x4
#       define R500_TX_FORMAT_ATI1N                 0x5
#       define R500_TX_FORMAT_Y8X24                 0x6


#       define R300_TX_FORMAT_SIGNED_W             (1 << 5)
#       define R300_TX_FORMAT_SIGNED_Z             (1 << 6)
#       define R300_TX_FORMAT_SIGNED_Y             (1 << 7)
#       define R300_TX_FORMAT_SIGNED_X             (1 << 8)
#       define R300_TX_FORMAT_SIGNED               (0xf << 5)

#	define R300_TX_FORMAT_3D		   (1 << 25)
#	define R300_TX_FORMAT_CUBIC_MAP		   (2 << 25)
#	define R300_TX_FORMAT_TEX_COORD_TYPE_MASK  (0x3 << 25)

	/* alpha modes, convenience mostly */
	/* if you have alpha, pick constant appropriate to the
	   number of channels (1 for I8, 2 for I8A8, 4 for R8G8B8A8, etc */
# 	define R300_TX_FORMAT_ALPHA_1CH		    0x000
# 	define R300_TX_FORMAT_ALPHA_2CH		    0x200
# 	define R300_TX_FORMAT_ALPHA_4CH		    0x600
# 	define R300_TX_FORMAT_ALPHA_NONE	    0xA00
	/* Swizzling */
	/* constants */
#	define R300_TX_FORMAT_X		0
#	define R300_TX_FORMAT_Y		1
#	define R300_TX_FORMAT_Z		2
#	define R300_TX_FORMAT_W		3
#	define R300_TX_FORMAT_ZERO	4
#	define R300_TX_FORMAT_ONE	5
	/* 2.0*Z, everything above 1.0 is set to 0.0 */
#	define R300_TX_FORMAT_CUT_Z	6
	/* 2.0*W, everything above 1.0 is set to 0.0 */
#	define R300_TX_FORMAT_CUT_W	7

#	define R300_TX_FORMAT_B_SHIFT	18
#	define R300_TX_FORMAT_G_SHIFT	15
#	define R300_TX_FORMAT_R_SHIFT	12
#	define R300_TX_FORMAT_A_SHIFT	9
	/* Convenience macro to take care of layout and swizzling */
#	define R300_EASY_TX_FORMAT(B, G, R, A, FMT)	(		\
		((R300_TX_FORMAT_##B)<<R300_TX_FORMAT_B_SHIFT)		\
		| ((R300_TX_FORMAT_##G)<<R300_TX_FORMAT_G_SHIFT)	\
		| ((R300_TX_FORMAT_##R)<<R300_TX_FORMAT_R_SHIFT)	\
		| ((R300_TX_FORMAT_##A)<<R300_TX_FORMAT_A_SHIFT)	\
		| (R300_TX_FORMAT_##FMT)				\
		)
	/* These can be ORed with result of R300_EASY_TX_FORMAT()
	   We don't really know what they do. Take values from a
           constant color ? */
#	define R300_TX_FORMAT_CONST_X		(1<<5)
#	define R300_TX_FORMAT_CONST_Y		(2<<5)
#	define R300_TX_FORMAT_CONST_Z		(4<<5)
#	define R300_TX_FORMAT_CONST_W		(8<<5)

#       define R300_TX_FORMAT_GAMMA               (1 << 21)
#       define R300_TX_FORMAT_YUV_TO_RGB          (1 << 22)

#       define R300_TX_CACHE(x)                 ((x) << 27)
#       define R300_TX_CACHE_WHOLE              0
/* reserved */
#       define R300_TX_CACHE_HALF_0             2
#       define R300_TX_CACHE_HALF_1             3
#       define R300_TX_CACHE_FOURTH_0           4
#       define R300_TX_CACHE_FOURTH_1           5
#       define R300_TX_CACHE_FOURTH_2           6
#       define R300_TX_CACHE_FOURTH_3           7
#       define R300_TX_CACHE_EIGHTH_0           8
#       define R300_TX_CACHE_EIGHTH_1           9
#       define R300_TX_CACHE_EIGHTH_2           10
#       define R300_TX_CACHE_EIGHTH_3           11
#       define R300_TX_CACHE_EIGHTH_4           12
#       define R300_TX_CACHE_EIGHTH_5           13
#       define R300_TX_CACHE_EIGHTH_6           14
#       define R300_TX_CACHE_EIGHTH_7           15
#       define R300_TX_CACHE_SIXTEENTH_0        16
#       define R300_TX_CACHE_SIXTEENTH_1        17
#       define R300_TX_CACHE_SIXTEENTH_2        18
#       define R300_TX_CACHE_SIXTEENTH_3        19
#       define R300_TX_CACHE_SIXTEENTH_4        20
#       define R300_TX_CACHE_SIXTEENTH_5        21
#       define R300_TX_CACHE_SIXTEENTH_6        22
#       define R300_TX_CACHE_SIXTEENTH_7        23
#       define R300_TX_CACHE_SIXTEENTH_8        24
#       define R300_TX_CACHE_SIXTEENTH_9        25
#       define R300_TX_CACHE_SIXTEENTH_10       26
#       define R300_TX_CACHE_SIXTEENTH_11       27
#       define R300_TX_CACHE_SIXTEENTH_12       28
#       define R300_TX_CACHE_SIXTEENTH_13       29
#       define R300_TX_CACHE_SIXTEENTH_14       30
#       define R300_TX_CACHE_SIXTEENTH_15       31

#define R300_TX_FORMAT2_0		    0x4500 /* obvious missing in gap */
#       define R300_TX_PITCHMASK_SHIFT           0
#       define R300_TX_PITCHMASK_MASK            (2047 << 0)
#	define R500_TXFORMAT_MSB		 (1 << 14)
#	define R500_TXWIDTH_BIT11	         (1 << 15)
#	define R500_TXHEIGHT_BIT11	         (1 << 16)
#	define R500_POW2FIX2FLT			 (1 << 17)
#	define R500_SEL_FILTER4_TC0		 (0 << 18)
#	define R500_SEL_FILTER4_TC1		 (1 << 18)
#	define R500_SEL_FILTER4_TC2		 (2 << 18)
#	define R500_SEL_FILTER4_TC3		 (3 << 18)

#define R300_TX_OFFSET_0                    0x4540
#define R300_TX_OFFSET_1                    0x4544
#define R300_TX_OFFSET_2                    0x4548
#define R300_TX_OFFSET_3                    0x454C
#define R300_TX_OFFSET_4                    0x4550
#define R300_TX_OFFSET_5                    0x4554
#define R300_TX_OFFSET_6                    0x4558
#define R300_TX_OFFSET_7                    0x455C

#       define R300_TXO_ENDIAN(x)                ((x) << 0)
#       define R300_TXO_MACRO_TILE_LINEAR        (0 << 2)
#       define R300_TXO_MACRO_TILE_TILED         (1 << 2)
#       define R300_TXO_MACRO_TILE(x)            ((x) << 2)
#       define R300_TXO_MICRO_TILE_LINEAR        (0 << 3)
#       define R300_TXO_MICRO_TILE_TILED         (1 << 3)
#       define R300_TXO_MICRO_TILE_TILED_SQUARE  (2 << 3)
#       define R300_TXO_MICRO_TILE(x)            ((x) << 3)
#       define R300_TXO_OFFSET_MASK              0xffffffe0
#       define R300_TXO_OFFSET_SHIFT             5

/* 32 bit chroma key */
#define R300_TX_CHROMA_KEY_0                      0x4580
#define R300_TX_CHROMA_KEY_1                      0x4584
#define R300_TX_CHROMA_KEY_2                      0x4588
#define R300_TX_CHROMA_KEY_3                      0x458c
#define R300_TX_CHROMA_KEY_4                      0x4590
#define R300_TX_CHROMA_KEY_5                      0x4594
#define R300_TX_CHROMA_KEY_6                      0x4598
#define R300_TX_CHROMA_KEY_7                      0x459c
#define R300_TX_CHROMA_KEY_8                      0x45a0
#define R300_TX_CHROMA_KEY_9                      0x45a4
#define R300_TX_CHROMA_KEY_10                     0x45a8
#define R300_TX_CHROMA_KEY_11                     0x45ac
#define R300_TX_CHROMA_KEY_12                     0x45b0
#define R300_TX_CHROMA_KEY_13                     0x45b4
#define R300_TX_CHROMA_KEY_14                     0x45b8
#define R300_TX_CHROMA_KEY_15                     0x45bc
/* ff00ff00 == { 0, 1.0, 0, 1.0 } */

/* Border Color */
#define R300_TX_BORDER_COLOR_0              0x45c0
#define R300_TX_BORDER_COLOR_1              0x45c4
#define R300_TX_BORDER_COLOR_2              0x45c8
#define R300_TX_BORDER_COLOR_3              0x45cc
#define R300_TX_BORDER_COLOR_4              0x45d0
#define R300_TX_BORDER_COLOR_5              0x45d4
#define R300_TX_BORDER_COLOR_6              0x45d8
#define R300_TX_BORDER_COLOR_7              0x45dc
#define R300_TX_BORDER_COLOR_8              0x45e0
#define R300_TX_BORDER_COLOR_9              0x45e4
#define R300_TX_BORDER_COLOR_10             0x45e8
#define R300_TX_BORDER_COLOR_11             0x45ec
#define R300_TX_BORDER_COLOR_12             0x45f0
#define R300_TX_BORDER_COLOR_13             0x45f4
#define R300_TX_BORDER_COLOR_14             0x45f8
#define R300_TX_BORDER_COLOR_15             0x45fc


/* END: Texture specification */

/* BEGIN: Fragment program instruction set */

/* Fragment programs are written directly into register space.
 * There are separate instruction streams for texture instructions and ALU
 * instructions.
 * In order to synchronize these streams, the program is divided into up
 * to 4 nodes. Each node begins with a number of TEX operations, followed
 * by a number of ALU operations.
 * The first node can have zero TEX ops, all subsequent nodes must have at
 * least
 * one TEX ops.
 * All nodes must have at least one ALU op.
 *
 * The index of the last node is stored in PFS_CNTL_0: A value of 0 means
 * 1 node, a value of 3 means 4 nodes.
 * The total amount of instructions is defined in PFS_CNTL_2. The offsets are
 * offsets into the respective instruction streams, while *_END points to the
 * last instruction relative to this offset.
 */
#define R300_US_CONFIG                      0x4600
#       define R300_PFS_CNTL_LAST_NODES_SHIFT    0
#       define R300_PFS_CNTL_LAST_NODES_MASK     (3 << 0)
#       define R300_PFS_CNTL_FIRST_NODE_HAS_TEX  (1 << 3)
#define R300_US_PIXSIZE                     0x4604
/* There is an unshifted value here which has so far always been equal to the
 * index of the highest used temporary register.
 */
#define R300_US_CODE_OFFSET                 0x4608
#       define R300_PFS_CNTL_ALU_OFFSET_SHIFT    0
#       define R300_PFS_CNTL_ALU_OFFSET_MASK     (63 << 0)
#       define R300_PFS_CNTL_ALU_END_SHIFT       6
#       define R300_PFS_CNTL_ALU_END_MASK        (63 << 6)
#       define R300_PFS_CNTL_TEX_OFFSET_SHIFT    13
#       define R300_PFS_CNTL_TEX_OFFSET_MASK     (31 << 13)
#       define R300_PFS_CNTL_TEX_END_SHIFT       18
#       define R300_PFS_CNTL_TEX_END_MASK        (31 << 18)
#       define R400_PFS_CNTL_TEX_OFFSET_MSB_SHIFT 24
#       define R400_PFS_CNTL_TEX_OFFSET_MSB_MASK (0xf << 24)
#       define R400_PFS_CNTL_TEX_END_MSB_SHIFT   28
#       define R400_PFS_CNTL_TEX_END_MSB_MASK    (0xf << 28)

/* gap */

/* Nodes are stored backwards. The last active node is always stored in
 * PFS_NODE_3.
 * Example: In a 2-node program, NODE_0 and NODE_1 are set to 0. The
 * first node is stored in NODE_2, the second node is stored in NODE_3.
 *
 * Offsets are relative to the master offset from PFS_CNTL_2.
 */
#define R300_US_CODE_ADDR_0                 0x4610
#define R300_US_CODE_ADDR_1                 0x4614
#define R300_US_CODE_ADDR_2                 0x4618
#define R300_US_CODE_ADDR_3                 0x461C
#       define R300_ALU_START_SHIFT         0
#       define R300_ALU_START_MASK          (63 << 0)
#       define R300_ALU_SIZE_SHIFT          6
#       define R300_ALU_SIZE_MASK           (63 << 6)
#       define R300_TEX_START_SHIFT         12
#       define R300_TEX_START_MASK          (31 << 12)
#       define R300_TEX_SIZE_SHIFT          17
#       define R300_TEX_SIZE_MASK           (31 << 17)
#	define R300_RGBA_OUT                (1 << 22)
#	define R300_W_OUT                   (1 << 23)
#       define R400_TEX_START_MSB_SHIFT     24
#       define R400_TEX_START_MSG_MASK      (0xf << 24)
#       define R400_TEX_SIZE_MSB_SHIFT      28
#       define R400_TEX_SIZE_MSG_MASK       (0xf << 28)

/* TEX
 * As far as I can tell, texture instructions cannot write into output
 * registers directly. A subsequent ALU instruction is always necessary,
 * even if it's just MAD o0, r0, 1, 0
 */
#define R300_US_TEX_INST_0                  0x4620
#	define R300_SRC_ADDR_SHIFT          0
#	define R300_SRC_ADDR_MASK           (31 << 0)
#	define R300_DST_ADDR_SHIFT          6
#	define R300_DST_ADDR_MASK           (31 << 6)
#	define R300_TEX_ID_SHIFT            11
#       define R300_TEX_ID_MASK             (15 << 11)
#	define R300_TEX_INST_SHIFT		15
#		define R300_TEX_OP_NOP	        0
#		define R300_TEX_OP_LD	        1
#		define R300_TEX_OP_KIL	        2
#		define R300_TEX_OP_TXP	        3
#		define R300_TEX_OP_TXB	        4
#	define R300_TEX_INST_MASK               (7 << 15)
#      define R400_SRC_ADDR_EXT_BIT         (1 << 19)
#      define R400_DST_ADDR_EXT_BIT         (1 << 20)

/* Output format from the unified shader */
#define R300_US_OUT_FMT_0                   0x46A4
#	define R300_US_OUT_FMT_C4_8         (0 << 0)
#	define R300_US_OUT_FMT_C4_10        (1 << 0)
#	define R300_US_OUT_FMT_C4_10_GAMMA  (2 << 0)
#	define R300_US_OUT_FMT_C_16         (3 << 0)
#	define R300_US_OUT_FMT_C2_16        (4 << 0)
#	define R300_US_OUT_FMT_C4_16        (5 << 0)
#	define R300_US_OUT_FMT_C_16_MPEG    (6 << 0)
#	define R300_US_OUT_FMT_C2_16_MPEG   (7 << 0)
#	define R300_US_OUT_FMT_C2_4         (8 << 0)
#	define R300_US_OUT_FMT_C_3_3_2      (9 << 0)
#	define R300_US_OUT_FMT_C_6_5_6      (10 << 0)
#	define R300_US_OUT_FMT_C_11_11_10   (11 << 0)
#	define R300_US_OUT_FMT_C_10_11_11   (12 << 0)
#	define R300_US_OUT_FMT_C_2_10_10_10 (13 << 0)
/* reserved */
#	define R300_US_OUT_FMT_UNUSED       (15 << 0)
#	define R300_US_OUT_FMT_C_16_FP      (16 << 0)
#	define R300_US_OUT_FMT_C2_16_FP     (17 << 0)
#	define R300_US_OUT_FMT_C4_16_FP     (18 << 0)
#	define R300_US_OUT_FMT_C_32_FP      (19 << 0)
#	define R300_US_OUT_FMT_C2_32_FP     (20 << 0)
#	define R300_US_OUT_FMT_C4_32_FP     (21 << 0)
#   define R300_C0_SEL_A				(0 << 8)
#   define R300_C0_SEL_R				(1 << 8)
#   define R300_C0_SEL_G				(2 << 8)
#   define R300_C0_SEL_B				(3 << 8)
#   define R300_C1_SEL_A				(0 << 10)
#   define R300_C1_SEL_R				(1 << 10)
#   define R300_C1_SEL_G				(2 << 10)
#   define R300_C1_SEL_B				(3 << 10)
#   define R300_C2_SEL_A				(0 << 12)
#   define R300_C2_SEL_R				(1 << 12)
#   define R300_C2_SEL_G				(2 << 12)
#   define R300_C2_SEL_B				(3 << 12)
#   define R300_C3_SEL_A				(0 << 14)
#   define R300_C3_SEL_R				(1 << 14)
#   define R300_C3_SEL_G				(2 << 14)
#   define R300_C3_SEL_B				(3 << 14)
#   define R300_OUT_SIGN(x)				((x) << 16)
#   define R500_ROUND_ADJ				(1 << 20)

/* ALU
 * The ALU instructions register blocks are enumerated according to the order
 * in which fglrx. I assume there is space for 64 instructions, since
 * each block has space for a maximum of 64 DWORDs, and this matches reported
 * native limits.
 *
 * The basic functional block seems to be one MAD for each color and alpha,
 * and an adder that adds all components after the MUL.
 *  - ADD, MUL, MAD etc.: use MAD with appropriate neutral operands
 *  - DP4: Use OUTC_DP4, OUTA_DP4
 *  - DP3: Use OUTC_DP3, OUTA_DP4, appropriate alpha operands
 *  - DPH: Use OUTC_DP4, OUTA_DP4, appropriate alpha operands
 *  - CMPH: If ARG2 > 0.5, return ARG0, else return ARG1
 *  - CMP: If ARG2 < 0, return ARG1, else return ARG0
 *  - FLR: use FRC+MAD
 *  - SGE, SLT: use MAD+CMP
 *  - RSQ: use ABS modifier for argument
 *  - Use OUTC_REPL_ALPHA to write results of an alpha-only operation
 *    (e.g. RCP) into color register
 *  - apparently, there's no quick DST operation
 *  - fglrx set FPI2_UNKNOWN_31 on a "MAD fragment.color, tmp0, tmp1, tmp2"
 *  - fglrx set FPI2_UNKNOWN_31 on a "MAX r2, r1, c0"
 *  - fglrx once set FPI0_UNKNOWN_31 on a "FRC r1, r1"
 *
 * Operand selection
 * First stage selects three sources from the available registers and
 * constant parameters. This is defined in INSTR1 (color) and INSTR3 (alpha).
 * fglrx sorts the three source fields: Registers before constants,
 * lower indices before higher indices; I do not know whether this is
 * necessary.
 *
 * fglrx fills unused sources with "read constant 0"
 * According to specs, you cannot select more than two different constants.
 *
 * Second stage selects the operands from the sources. This is defined in
 * INSTR0 (color) and INSTR2 (alpha). You can also select the special constants
 * zero and one.
 * Swizzling and negation happens in this stage, as well.
 *
 * Important: Color and alpha seem to be mostly separate, i.e. their sources
 * selection appears to be fully independent (the register storage is probably
 * physically split into a color and an alpha section).
 * However (because of the apparent physical split), there is some interaction
 * WRT swizzling. If, for example, you want to load an R component into an
 * Alpha operand, this R component is taken from a *color* source, not from
 * an alpha source. The corresponding register doesn't even have to appear in
 * the alpha sources list. (I hope this all makes sense to you)
 *
 * Destination selection
 * The destination register index is in FPI1 (color) and FPI3 (alpha)
 * together with enable bits.
 * There are separate enable bits for writing into temporary registers
 * (DSTC_REG_* /DSTA_REG) and program output registers (DSTC_OUTPUT_*
 * /DSTA_OUTPUT). You can write to both at once, or not write at all (the
 * same index must be used for both).
 *
 * Note: There is a special form for LRP
 *  - Argument order is the same as in ARB_fragment_program.
 *  - Operation is MAD
 *  - ARG1 is set to ARGC_SRC1C_LRP/ARGC_SRC1A_LRP
 *  - Set FPI0/FPI2_SPECIAL_LRP
 * Arbitrary LRP (including support for swizzling) requires vanilla MAD+MAD
 */
#define R300_US_ALU_RGB_ADDR_0                   0x46C0
#       define R300_ALU_SRC0C_SHIFT             0
#       define R300_ALU_SRC0C_MASK              (31 << 0)
#       define R300_ALU_SRC0C_CONST             (1 << 5)
#       define R300_ALU_SRC1C_SHIFT             6
#       define R300_ALU_SRC1C_MASK              (31 << 6)
#       define R300_ALU_SRC1C_CONST             (1 << 11)
#       define R300_ALU_SRC2C_SHIFT             12
#       define R300_ALU_SRC2C_MASK              (31 << 12)
#       define R300_ALU_SRC2C_CONST             (1 << 17)
#       define R300_ALU_SRC_MASK                0x0003ffff
#       define R300_ALU_DSTC_SHIFT              18
#       define R300_ALU_DSTC_MASK               (31 << 18)
#		define R300_ALU_DSTC_REG_MASK_SHIFT     23
#       define R300_ALU_DSTC_REG_X              (1 << 23)
#       define R300_ALU_DSTC_REG_Y              (1 << 24)
#       define R300_ALU_DSTC_REG_Z              (1 << 25)
#		define R300_ALU_DSTC_OUTPUT_MASK_SHIFT  26
#       define R300_ALU_DSTC_OUTPUT_X           (1 << 26)
#       define R300_ALU_DSTC_OUTPUT_Y           (1 << 27)
#       define R300_ALU_DSTC_OUTPUT_Z           (1 << 28)
#       define R300_ALU_DSTC_OUTPUT_XYZ         (7 << 26)
#       define R300_RGB_ADDR0(x)                ((x) << 0)
#       define R300_RGB_ADDR1(x)                ((x) << 6)
#       define R300_RGB_ADDR2(x)                ((x) << 12)
#       define R300_RGB_TARGET(x)               ((x) << 29)

#define R300_US_ALU_ALPHA_ADDR_0                 0x47C0
#       define R300_ALU_SRC0A_SHIFT             0
#       define R300_ALU_SRC0A_MASK              (31 << 0)
#       define R300_ALU_SRC0A_CONST             (1 << 5)
#       define R300_ALU_SRC1A_SHIFT             6
#       define R300_ALU_SRC1A_MASK              (31 << 6)
#       define R300_ALU_SRC1A_CONST             (1 << 11)
#       define R300_ALU_SRC2A_SHIFT             12
#       define R300_ALU_SRC2A_MASK              (31 << 12)
#       define R300_ALU_SRC2A_CONST             (1 << 17)
#       define R300_ALU_SRC_MASK                0x0003ffff
#       define R300_ALU_DSTA_SHIFT              18
#       define R300_ALU_DSTA_MASK               (31 << 18)
#       define R300_ALU_DSTA_REG                (1 << 23)
#       define R300_ALU_DSTA_OUTPUT             (1 << 24)
#		define R300_ALU_DSTA_DEPTH              (1 << 27)
#       define R300_ALPHA_ADDR0(x)              ((x) << 0)
#       define R300_ALPHA_ADDR1(x)              ((x) << 6)
#       define R300_ALPHA_ADDR2(x)              ((x) << 12)
#       define R300_ALPHA_TARGET(x)             ((x) << 25)

#define R300_US_ALU_RGB_INST_0                   0x48C0
#       define R300_ALU_ARGC_SRC0C_XYZ          0
#       define R300_ALU_ARGC_SRC0C_XXX          1
#       define R300_ALU_ARGC_SRC0C_YYY          2
#       define R300_ALU_ARGC_SRC0C_ZZZ          3
#       define R300_ALU_ARGC_SRC1C_XYZ          4
#       define R300_ALU_ARGC_SRC1C_XXX          5
#       define R300_ALU_ARGC_SRC1C_YYY          6
#       define R300_ALU_ARGC_SRC1C_ZZZ          7
#       define R300_ALU_ARGC_SRC2C_XYZ          8
#       define R300_ALU_ARGC_SRC2C_XXX          9
#       define R300_ALU_ARGC_SRC2C_YYY          10
#       define R300_ALU_ARGC_SRC2C_ZZZ          11
#       define R300_ALU_ARGC_SRC0A              12
#       define R300_ALU_ARGC_SRC1A              13
#       define R300_ALU_ARGC_SRC2A              14
#       define R300_ALU_ARGC_SRCP_XYZ           15
#       define R300_ALU_ARGC_SRCP_XXX           16
#       define R300_ALU_ARGC_SRCP_YYY           17
#       define R300_ALU_ARGC_SRCP_ZZZ           18
#       define R300_ALU_ARGC_SRCP_WWW           19
#       define R300_ALU_ARGC_ZERO               20
#       define R300_ALU_ARGC_ONE                21
#       define R300_ALU_ARGC_HALF               22
#       define R300_ALU_ARGC_SRC0C_YZX          23
#       define R300_ALU_ARGC_SRC1C_YZX          24
#       define R300_ALU_ARGC_SRC2C_YZX          25
#       define R300_ALU_ARGC_SRC0C_ZXY          26
#       define R300_ALU_ARGC_SRC1C_ZXY          27
#       define R300_ALU_ARGC_SRC2C_ZXY          28
#       define R300_ALU_ARGC_SRC0CA_WZY         29
#       define R300_ALU_ARGC_SRC1CA_WZY         30
#       define R300_ALU_ARGC_SRC2CA_WZY         31
#       define R300_RGB_SWIZA(x)                ((x) << 0)
#       define R300_RGB_SWIZB(x)                ((x) << 7)
#       define R300_RGB_SWIZC(x)                ((x) << 14)

#       define R300_ALU_ARG0C_SHIFT             0
#       define R300_ALU_ARG0C_MASK              (31 << 0)
#       define R300_ALU_ARG0C_NOP               (0 << 5)
#       define R300_ALU_ARG0C_NEG               (1 << 5)
#       define R300_ALU_ARG0C_ABS               (2 << 5)
#       define R300_ALU_ARG0C_NAB               (3 << 5)
#       define R300_ALU_ARG1C_SHIFT             7
#       define R300_ALU_ARG1C_MASK              (31 << 7)
#       define R300_ALU_ARG1C_NOP               (0 << 12)
#       define R300_ALU_ARG1C_NEG               (1 << 12)
#       define R300_ALU_ARG1C_ABS               (2 << 12)
#       define R300_ALU_ARG1C_NAB               (3 << 12)
#       define R300_ALU_ARG2C_SHIFT             14
#       define R300_ALU_ARG2C_MASK              (31 << 14)
#       define R300_ALU_ARG2C_NOP               (0 << 19)
#       define R300_ALU_ARG2C_NEG               (1 << 19)
#       define R300_ALU_ARG2C_ABS               (2 << 19)
#       define R300_ALU_ARG2C_NAB               (3 << 19)
#       define R300_ALU_SRCP_1_MINUS_2_SRC0     (0 << 21)
#       define R300_ALU_SRCP_SRC1_MINUS_SRC0    (1 << 21)
#       define R300_ALU_SRCP_SRC1_PLUS_SRC0     (2 << 21)
#       define R300_ALU_SRCP_1_MINUS_SRC0       (3 << 21)

#       define R300_ALU_OUTC_MAD                (0 << 23)
#       define R300_ALU_OUTC_DP3                (1 << 23)
#       define R300_ALU_OUTC_DP4                (2 << 23)
#       define R300_ALU_OUTC_D2A                (3 << 23)
#       define R300_ALU_OUTC_MIN                (4 << 23)
#       define R300_ALU_OUTC_MAX                (5 << 23)
#       define R300_ALU_OUTC_CND                (7 << 23)
#       define R300_ALU_OUTC_CMP                (8 << 23)
#       define R300_ALU_OUTC_FRC                (9 << 23)
#       define R300_ALU_OUTC_REPL_ALPHA         (10 << 23)

#	define R300_ALU_OUTC_MOD_SHIFT		27
#       define R300_ALU_OUTC_MOD_NOP            (0 << R300_ALU_OUTC_MOD_SHIFT)
#       define R300_ALU_OUTC_MOD_MUL2           (1 << R300_ALU_OUTC_MOD_SHIFT)
#       define R300_ALU_OUTC_MOD_MUL4           (2 << R300_ALU_OUTC_MOD_SHIFT)
#       define R300_ALU_OUTC_MOD_MUL8           (3 << R300_ALU_OUTC_MOD_SHIFT)
#       define R300_ALU_OUTC_MOD_DIV2           (4 << R300_ALU_OUTC_MOD_SHIFT)
#       define R300_ALU_OUTC_MOD_DIV4           (5 << R300_ALU_OUTC_MOD_SHIFT)
#       define R300_ALU_OUTC_MOD_DIV8           (6 << R300_ALU_OUTC_MOD_SHIFT)

#       define R300_ALU_OUTC_CLAMP              (1 << 30)
#       define R300_ALU_INSERT_NOP              (1U << 31)

#define R300_US_ALU_ALPHA_INST_0                 0x49C0
#       define R300_ALU_ARGA_SRC0C_X            0
#       define R300_ALU_ARGA_SRC0C_Y            1
#       define R300_ALU_ARGA_SRC0C_Z            2
#       define R300_ALU_ARGA_SRC1C_X            3
#       define R300_ALU_ARGA_SRC1C_Y            4
#       define R300_ALU_ARGA_SRC1C_Z            5
#       define R300_ALU_ARGA_SRC2C_X            6
#       define R300_ALU_ARGA_SRC2C_Y            7
#       define R300_ALU_ARGA_SRC2C_Z            8
#       define R300_ALU_ARGA_SRC0A              9
#       define R300_ALU_ARGA_SRC1A              10
#       define R300_ALU_ARGA_SRC2A              11
#       define R300_ALU_ARGA_SRCP_X             12
#       define R300_ALU_ARGA_SRCP_Y             13
#       define R300_ALU_ARGA_SRCP_Z             14
#       define R300_ALU_ARGA_SRCP_W             15
#       define R300_ALU_ARGA_ZERO               16
#       define R300_ALU_ARGA_ONE                17
#       define R300_ALU_ARGA_HALF               18
#       define R300_ALPHA_SWIZA(x)              ((x) << 0)
#       define R300_ALPHA_SWIZB(x)              ((x) << 7)
#       define R300_ALPHA_SWIZC(x)              ((x) << 14)

#       define R300_ALU_ARG0A_SHIFT             0
#       define R300_ALU_ARG0A_MASK              (31 << 0)
#       define R300_ALU_ARG0A_NOP               (0 << 5)
#       define R300_ALU_ARG0A_NEG               (1 << 5)
#	define R300_ALU_ARG0A_ABS		 (2 << 5)
#	define R300_ALU_ARG0A_NAB		 (3 << 5)
#       define R300_ALU_ARG1A_SHIFT             7
#       define R300_ALU_ARG1A_MASK              (31 << 7)
#       define R300_ALU_ARG1A_NOP               (0 << 12)
#       define R300_ALU_ARG1A_NEG               (1 << 12)
#	define R300_ALU_ARG1A_ABS		 (2 << 12)
#	define R300_ALU_ARG1A_NAB		 (3 << 12)
#       define R300_ALU_ARG2A_SHIFT             14
#       define R300_ALU_ARG2A_MASK              (31 << 14)
#       define R300_ALU_ARG2A_NOP               (0 << 19)
#       define R300_ALU_ARG2A_NEG               (1 << 19)
#	define R300_ALU_ARG2A_ABS		 (2 << 19)
#	define R300_ALU_ARG2A_NAB		 (3 << 19)
#       define R300_ALU_SRCP_1_MINUS_2_SRC0     (0 << 21)
#       define R300_ALU_SRCP_SRC1_MINUS_SRC0    (1 << 21)
#       define R300_ALU_SRCP_SRC1_PLUS_SRC0     (2 << 21)
#       define R300_ALU_SRCP_1_MINUS_SRC0       (3 << 21)

#       define R300_ALU_OUTA_MAD                (0 << 23)
#       define R300_ALU_OUTA_DP4                (1 << 23)
#       define R300_ALU_OUTA_MIN                (2 << 23)
#       define R300_ALU_OUTA_MAX                (3 << 23)
#       define R300_ALU_OUTA_CND                (5 << 23)
#       define R300_ALU_OUTA_CMP                (6 << 23)
#       define R300_ALU_OUTA_FRC                (7 << 23)
#       define R300_ALU_OUTA_EX2                (8 << 23)
#       define R300_ALU_OUTA_LG2                (9 << 23)
#       define R300_ALU_OUTA_RCP                (10 << 23)
#       define R300_ALU_OUTA_RSQ                (11 << 23)

#       define R300_ALU_OUTA_MOD_NOP            (0 << 27)
#       define R300_ALU_OUTA_MOD_MUL2           (1 << 27)
#       define R300_ALU_OUTA_MOD_MUL4           (2 << 27)
#       define R300_ALU_OUTA_MOD_MUL8           (3 << 27)
#       define R300_ALU_OUTA_MOD_DIV2           (4 << 27)
#       define R300_ALU_OUTA_MOD_DIV4           (5 << 27)
#       define R300_ALU_OUTA_MOD_DIV8           (6 << 27)

#       define R300_ALU_OUTA_CLAMP              (1 << 30)
/* END: Fragment program instruction set */

/* R4xx extended fragment shader registers. */
#define R400_US_ALU_EXT_ADDR_0              0x4ac0 /* up to 63 (0x4bbc) */
#   define R400_ADDR_EXT_RGB_MSB_BIT(x)     (1 << (x))
#   define R400_ADDRD_EXT_RGB_MSB_BIT       0x08
#   define R400_ADDR_EXT_A_MSB_BIT(x)       (1 << ((x) + 4))
#   define R400_ADDRD_EXT_A_MSB_BIT         0x80

#define R400_US_CODE_BANK                   0x46b8
#   define R400_BANK_SHIFT                  0
#   define R400_BANK_MASK                   0xf
#   define R400_R390_MODE_ENABLE            (1 << 4)
#define R400_US_CODE_EXT                    0x46bc
#   define R400_ALU_OFFSET_MSB_SHIFT        0
#   define R400_ALU_OFFSET_MSB_MASK         (0x7 << 0)
#   define R400_ALU_SIZE_MSB_SHIFT          3
#   define R400_ALU_SIZE_MSB_MASK           (0x7 << 3)
#   define R400_ALU_START0_MSB_SHIFT        6
#   define R400_ALU_START0_MSB_MASK         (0x7 << 6)
#   define R400_ALU_SIZE0_MSB_SHIFT         9
#   define R400_ALU_SIZE0_MSB_MASK          (0x7 << 9)
#   define R400_ALU_START1_MSB_SHIFT        12
#   define R400_ALU_START1_MSB_MASK         (0x7 << 12)
#   define R400_ALU_SIZE1_MSB_SHIFT         15
#   define R400_ALU_SIZE1_MSB_MASK          (0x7 << 15)
#   define R400_ALU_START2_MSB_SHIFT        18
#   define R400_ALU_START2_MSB_MASK         (0x7 << 18)
#   define R400_ALU_SIZE2_MSB_SHIFT         21
#   define R400_ALU_SIZE2_MSB_MASK          (0x7 << 21)
#   define R400_ALU_START3_MSB_SHIFT        24
#   define R400_ALU_START3_MSB_MASK         (0x7 << 24)
#   define R400_ALU_SIZE3_MSB_SHIFT         27
#   define R400_ALU_SIZE3_MSB_MASK          (0x7 << 27)
/* END: R4xx extended fragment shader registers. */

/* Fog: Fog Blending Enable */
#define R300_FG_FOG_BLEND                             0x4bc0
#       define R300_FG_FOG_BLEND_DISABLE              (0 << 0)
#       define R300_FG_FOG_BLEND_ENABLE               (1 << 0)
#	define R300_FG_FOG_BLEND_FN_LINEAR            (0 << 1)
#	define R300_FG_FOG_BLEND_FN_EXP               (1 << 1)
#	define R300_FG_FOG_BLEND_FN_EXP2              (2 << 1)
#	define R300_FG_FOG_BLEND_FN_CONSTANT          (3 << 1)
#	define R300_FG_FOG_BLEND_FN_MASK              (3 << 1)

/* Fog: Red Component of Fog Color */
#define R300_FG_FOG_COLOR_R                           0x4bc8
/* Fog: Green Component of Fog Color */
#define R300_FG_FOG_COLOR_G                           0x4bcc
/* Fog: Blue Component of Fog Color */
#define R300_FG_FOG_COLOR_B                           0x4bd0
#	define R300_FG_FOG_COLOR_MASK 0x000003ff

/* Fog: Constant Factor for Fog Blending */
#define R300_FG_FOG_FACTOR                            0x4bc4
#	define FG_FOG_FACTOR_MASK 0x000003ff

/* Fog: Alpha function */
#define R300_FG_ALPHA_FUNC                            0x4bd4
#       define R300_FG_ALPHA_FUNC_VAL_MASK               0x000000ff
#       define R300_FG_ALPHA_FUNC_NEVER                     (0 << 8)
#       define R300_FG_ALPHA_FUNC_LESS                      (1 << 8)
#       define R300_FG_ALPHA_FUNC_EQUAL                     (2 << 8)
#       define R300_FG_ALPHA_FUNC_LE                        (3 << 8)
#       define R300_FG_ALPHA_FUNC_GREATER                   (4 << 8)
#       define R300_FG_ALPHA_FUNC_NOTEQUAL                  (5 << 8)
#       define R300_FG_ALPHA_FUNC_GE                        (6 << 8)
#       define R300_FG_ALPHA_FUNC_ALWAYS                    (7 << 8)
#       define R300_ALPHA_TEST_OP_MASK                      (7 << 8)
#       define R300_FG_ALPHA_FUNC_DISABLE                   (0 << 11)
#       define R300_FG_ALPHA_FUNC_ENABLE                    (1 << 11)

#       define R500_FG_ALPHA_FUNC_10BIT                     (0 << 12)
#       define R500_FG_ALPHA_FUNC_8BIT                      (1 << 12)

#       define R300_FG_ALPHA_FUNC_MASK_DISABLE              (0 << 16)
#       define R300_FG_ALPHA_FUNC_MASK_ENABLE               (1 << 16)
#       define R300_FG_ALPHA_FUNC_CFG_2_OF_4                (0 << 17)
#       define R300_FG_ALPHA_FUNC_CFG_3_OF_6                (1 << 17)

#       define R300_FG_ALPHA_FUNC_DITH_DISABLE              (0 << 20)
#       define R300_FG_ALPHA_FUNC_DITH_ENABLE               (1 << 20)

#       define R500_FG_ALPHA_FUNC_OFFSET_DISABLE            (0 << 24)
#       define R500_FG_ALPHA_FUNC_OFFSET_ENABLE             (1 << 24) /* Not supported in R520 */
#       define R500_FG_ALPHA_FUNC_DISC_ZERO_MASK_DISABLE    (0 << 25)
#       define R500_FG_ALPHA_FUNC_DISC_ZERO_MASK_ENABLE     (1 << 25)

#       define R500_FG_ALPHA_FUNC_FP16_DISABLE              (0 << 28)
#       define R500_FG_ALPHA_FUNC_FP16_ENABLE               (1 << 28)


/* Fog: Where does the depth come from? */
#define R300_FG_DEPTH_SRC                  0x4bd8
#	define R300_FG_DEPTH_SRC_SCAN   (0 << 0)
#	define R300_FG_DEPTH_SRC_SHADER (1 << 0)

/* Fog: Alpha Compare Value */
#define R500_FG_ALPHA_VALUE                0x4be0
#	define R500_FG_ALPHA_VALUE_MASK 0x0000ffff

#define RV530_FG_ZBREG_DEST                 0x4be8
#	define RV530_FG_ZBREG_DEST_PIPE_SELECT_0             (1 << 0)
#	define RV530_FG_ZBREG_DEST_PIPE_SELECT_1             (1 << 1)
#	define RV530_FG_ZBREG_DEST_PIPE_SELECT_ALL           (3 << 0)
/* gap */

/* Fragment program parameters in 7.16 floating point */
#define R300_PFS_PARAM_0_X                  0x4C00
#define R300_PFS_PARAM_0_Y                  0x4C04
#define R300_PFS_PARAM_0_Z                  0x4C08
#define R300_PFS_PARAM_0_W                  0x4C0C
/* last consts */
#define R300_PFS_PARAM_31_X                 0x4DF0
#define R300_PFS_PARAM_31_Y                 0x4DF4
#define R300_PFS_PARAM_31_Z                 0x4DF8
#define R300_PFS_PARAM_31_W                 0x4DFC

/* Unpipelined. */
#define R300_RB3D_CCTL                      0x4e00
#	define R300_RB3D_CCTL_NUM_MULTIWRITES(x)       (MAX2(((x)-1), 0) << 5)
#	define R300_RB3D_CCTL_NUM_MULTIWRITES_1_BUFFER                (0 << 5)
#	define R300_RB3D_CCTL_NUM_MULTIWRITES_2_BUFFERS               (1 << 5)
#	define R300_RB3D_CCTL_NUM_MULTIWRITES_3_BUFFERS               (2 << 5)
#	define R300_RB3D_CCTL_NUM_MULTIWRITES_4_BUFFERS               (3 << 5)
#	define R300_RB3D_CCTL_CLRCMP_FLIPE_DISABLE                    (0 << 7)
#	define R300_RB3D_CCTL_CLRCMP_FLIPE_ENABLE                     (1 << 7)
#	define R300_RB3D_CCTL_AA_COMPRESSION_DISABLE                  (0 << 9)
#	define R300_RB3D_CCTL_AA_COMPRESSION_ENABLE                   (1 << 9)
#	define R300_RB3D_CCTL_CMASK_DISABLE                           (0 << 10)
#	define R300_RB3D_CCTL_CMASK_ENABLE                            (1 << 10)
/* reserved */
#	define R300_RB3D_CCTL_INDEPENDENT_COLOR_CHANNEL_MASK_DISABLE  (0 << 12)
#	define R300_RB3D_CCTL_INDEPENDENT_COLOR_CHANNEL_MASK_ENABLE   (1 << 12)
#	define R300_RB3D_CCTL_WRITE_COMPRESSION_ENABLE                (0 << 13)
#	define R300_RB3D_CCTL_WRITE_COMPRESSION_DISABLE               (1 << 13)
#	define R300_RB3D_CCTL_INDEPENDENT_COLORFORMAT_ENABLE_DISABLE  (0 << 14)
#	define R300_RB3D_CCTL_INDEPENDENT_COLORFORMAT_ENABLE_ENABLE   (1 << 14)


/* Notes:
 * - AFAIK fglrx always sets BLEND_UNKNOWN when blending is used in
 *   the application
 * - AFAIK fglrx always sets BLEND_NO_SEPARATE when CBLEND and ABLEND
 *    are set to the same
 *   function (both registers are always set up completely in any case)
 * - Most blend flags are simply copied from R200 and not tested yet
 */
#define R300_RB3D_CBLEND                    0x4E04
#define R300_RB3D_ABLEND                    0x4E08
/* the following only appear in CBLEND */
#       define R300_ALPHA_BLEND_ENABLE         (1 << 0)
#       define R300_SEPARATE_ALPHA_ENABLE      (1 << 1)
#       define R300_READ_ENABLE                (1 << 2)
#       define R300_DISCARD_SRC_PIXELS_DIS     (0 << 3)
#       define R300_DISCARD_SRC_PIXELS_SRC_ALPHA_0     (1 << 3)
#       define R300_DISCARD_SRC_PIXELS_SRC_COLOR_0     (2 << 3)
#       define R300_DISCARD_SRC_PIXELS_SRC_ALPHA_COLOR_0     (3 << 3)
#       define R300_DISCARD_SRC_PIXELS_SRC_ALPHA_1     (4 << 3)
#       define R300_DISCARD_SRC_PIXELS_SRC_COLOR_1     (5 << 3)
#       define R300_DISCARD_SRC_PIXELS_SRC_ALPHA_COLOR_1     (6 << 3)
#       define R500_SRC_ALPHA_0_NO_READ                (1 << 30)
#       define R500_SRC_ALPHA_1_NO_READ                (1U << 31)

/* the following are shared between CBLEND and ABLEND */
#       define R300_FCN_MASK                         (3  << 12)
#       define R300_COMB_FCN_ADD_CLAMP               (0  << 12)
#       define R300_COMB_FCN_ADD_NOCLAMP             (1  << 12)
#       define R300_COMB_FCN_SUB_CLAMP               (2  << 12)
#       define R300_COMB_FCN_SUB_NOCLAMP             (3  << 12)
#       define R300_COMB_FCN_MIN                     (4  << 12)
#       define R300_COMB_FCN_MAX                     (5  << 12)
#       define R300_COMB_FCN_RSUB_CLAMP              (6  << 12)
#       define R300_COMB_FCN_RSUB_NOCLAMP            (7  << 12)
#       define R300_BLEND_GL_ZERO                    (32)
#       define R300_BLEND_GL_ONE                     (33)
#       define R300_BLEND_GL_SRC_COLOR               (34)
#       define R300_BLEND_GL_ONE_MINUS_SRC_COLOR     (35)
#       define R300_BLEND_GL_DST_COLOR               (36)
#       define R300_BLEND_GL_ONE_MINUS_DST_COLOR     (37)
#       define R300_BLEND_GL_SRC_ALPHA               (38)
#       define R300_BLEND_GL_ONE_MINUS_SRC_ALPHA     (39)
#       define R300_BLEND_GL_DST_ALPHA               (40)
#       define R300_BLEND_GL_ONE_MINUS_DST_ALPHA     (41)
#       define R300_BLEND_GL_SRC_ALPHA_SATURATE      (42)
#       define R300_BLEND_GL_CONST_COLOR             (43)
#       define R300_BLEND_GL_ONE_MINUS_CONST_COLOR   (44)
#       define R300_BLEND_GL_CONST_ALPHA             (45)
#       define R300_BLEND_GL_ONE_MINUS_CONST_ALPHA   (46)
#       define R300_BLEND_MASK                       (63)
#       define R300_SRC_BLEND_SHIFT                  (16)
#       define R300_DST_BLEND_SHIFT                  (24)

/* Constant color used by the blender. Pipelined through the blender.
 * Note: For R520, this field is ignored, use RB3D_CONSTANT_COLOR_GB__BLUE,
 * RB3D_CONSTANT_COLOR_GB__GREEN, etc. instead.
 */
#define R300_RB3D_BLEND_COLOR               0x4E10


/* 3D Color Channel Mask. If all the channels used in the current color format
 * are disabled, then the cb will discard all the incoming quads. Pipelined
 * through the blender.
 */
#define RB3D_COLOR_CHANNEL_MASK                  0x4E0C
#	define RB3D_COLOR_CHANNEL_MASK_BLUE_MASK0  (1 << 0)
#	define RB3D_COLOR_CHANNEL_MASK_GREEN_MASK0 (1 << 1)
#	define RB3D_COLOR_CHANNEL_MASK_RED_MASK0   (1 << 2)
#	define RB3D_COLOR_CHANNEL_MASK_ALPHA_MASK0 (1 << 3)
#	define RB3D_COLOR_CHANNEL_MASK_BLUE_MASK1  (1 << 4)
#	define RB3D_COLOR_CHANNEL_MASK_GREEN_MASK1 (1 << 5)
#	define RB3D_COLOR_CHANNEL_MASK_RED_MASK1   (1 << 6)
#	define RB3D_COLOR_CHANNEL_MASK_ALPHA_MASK1 (1 << 7)
#	define RB3D_COLOR_CHANNEL_MASK_BLUE_MASK2  (1 << 8)
#	define RB3D_COLOR_CHANNEL_MASK_GREEN_MASK2 (1 << 9)
#	define RB3D_COLOR_CHANNEL_MASK_RED_MASK2   (1 << 10)
#	define RB3D_COLOR_CHANNEL_MASK_ALPHA_MASK2 (1 << 11)
#	define RB3D_COLOR_CHANNEL_MASK_BLUE_MASK3  (1 << 12)
#	define RB3D_COLOR_CHANNEL_MASK_GREEN_MASK3 (1 << 13)
#	define RB3D_COLOR_CHANNEL_MASK_RED_MASK3   (1 << 14)
#	define RB3D_COLOR_CHANNEL_MASK_ALPHA_MASK3 (1 << 15)

/* Clear color that is used when the color mask is set to 00. Unpipelined.
 * Program this register with a 32-bit value in ARGB8888 or ARGB2101010
 * formats, ignoring the fields.
 */
#define R300_RB3D_COLOR_CLEAR_VALUE                   0x4E14
/* For FP16 AA. */
#define R500_RB3D_COLOR_CLEAR_VALUE_AR                0x46C0
#define R500_RB3D_COLOR_CLEAR_VALUE_GB                0x46C4

/* gap */

/* Color Compare Color. Stalls the 2d/3d datapath until it is idle. */
#define RB3D_CLRCMP_CLR                     0x4e20

/* Color Compare Mask. Stalls the 2d/3d datapath until it is idle. */
#define RB3D_CLRCMP_MSK                     0x4e24

/* Color Buffer Address Offset of multibuffer 0. Unpipelined. */
#define R300_RB3D_COLOROFFSET0              0x4E28
#       define R300_COLOROFFSET_MASK             0xFFFFFFE0
/* Color Buffer Address Offset of multibuffer 1. Unpipelined. */
#define R300_RB3D_COLOROFFSET1              0x4E2C
/* Color Buffer Address Offset of multibuffer 2. Unpipelined. */
#define R300_RB3D_COLOROFFSET2              0x4E30
/* Color Buffer Address Offset of multibuffer 3. Unpipelined. */
#define R300_RB3D_COLOROFFSET3              0x4E34

/* Color buffer format and tiling control for all the multibuffers and the
 * pitch of multibuffer 0 to 3. Unpipelined. The cache must be empty before any
 * of the registers are changed.
 *
 * Bit 16: Larger tiles
 * Bit 17: 4x2 tiles
 * Bit 18: Extremely weird tile like, but some pixels duplicated?
 */
#define R300_RB3D_COLORPITCH0               0x4E38
#       define R300_COLORPITCH_MASK              0x00003FFE
#       define R300_COLOR_TILE_DISABLE            (0 << 16)
#       define R300_COLOR_TILE_ENABLE             (1 << 16)
#       define R300_COLOR_TILE(x)                 ((x) << 16)
#       define R300_COLOR_MICROTILE_DISABLE       (0 << 17)
#       define R300_COLOR_MICROTILE_ENABLE        (1 << 17)
#       define R300_COLOR_MICROTILE_ENABLE_SQUARE (2 << 17) /* Only available in 16-bit */
#       define R300_COLOR_MICROTILE(x)            ((x) << 17)
#       define R300_COLOR_ENDIAN(x)               ((x) << 19)
#	define R500_COLOR_FORMAT_ARGB10101010     (0 << 21)
#	define R500_COLOR_FORMAT_UV1010           (1 << 21)
#	define R500_COLOR_FORMAT_CI8              (2 << 21) /* 2D only */
#	define R300_COLOR_FORMAT_ARGB1555         (3 << 21)
#       define R300_COLOR_FORMAT_RGB565           (4 << 21)
#       define R500_COLOR_FORMAT_ARGB2101010      (5 << 21)
#       define R300_COLOR_FORMAT_ARGB8888         (6 << 21)
#       define R300_COLOR_FORMAT_ARGB32323232     (7 << 21)
/* reserved */
#       define R300_COLOR_FORMAT_I8               (9 << 21)
#       define R300_COLOR_FORMAT_ARGB16161616     (10 << 21)
#       define R300_COLOR_FORMAT_VYUY             (11 << 21)
#       define R300_COLOR_FORMAT_YVYU             (12 << 21)
#       define R300_COLOR_FORMAT_UV88             (13 << 21)
#       define R500_COLOR_FORMAT_I10              (14 << 21)
#       define R300_COLOR_FORMAT_ARGB4444         (15 << 21)
#define R300_RB3D_COLORPITCH1               0x4E3C
#define R300_RB3D_COLORPITCH2               0x4E40
#define R300_RB3D_COLORPITCH3               0x4E44

/* gap */

/* Destination Color Buffer Cache Control/Status. If the cb is in e2 mode, then
 * a flush or free will not occur upon a write to this register, but a sync
 * will be immediately sent if one is requested. If both DC_FLUSH and DC_FREE
 * are zero but DC_FINISH is one, then a sync will be sent immediately -- the
 * cb will not wait for all the previous operations to complete before sending
 * the sync. Unpipelined except when DC_FINISH and DC_FREE are both set to
 * zero.
 *
 * Set to 0A before 3D operations, set to 02 afterwards.
 */
#define R300_RB3D_DSTCACHE_CTLSTAT               0x4e4c
#	define R300_RB3D_DSTCACHE_CTLSTAT_DC_FLUSH_NO_EFFECT         (0 << 0)
#	define R300_RB3D_DSTCACHE_CTLSTAT_DC_FLUSH_NO_EFFECT_1       (1 << 0)
#	define R300_RB3D_DSTCACHE_CTLSTAT_DC_FLUSH_FLUSH_DIRTY_3D    (2 << 0)
#	define R300_RB3D_DSTCACHE_CTLSTAT_DC_FLUSH_FLUSH_DIRTY_3D_1  (3 << 0)
#	define R300_RB3D_DSTCACHE_CTLSTAT_DC_FREE_NO_EFFECT          (0 << 2)
#	define R300_RB3D_DSTCACHE_CTLSTAT_DC_FREE_NO_EFFECT_1        (1 << 2)
#	define R300_RB3D_DSTCACHE_CTLSTAT_DC_FREE_FREE_3D_TAGS       (2 << 2)
#	define R300_RB3D_DSTCACHE_CTLSTAT_DC_FREE_FREE_3D_TAGS_1     (3 << 2)
#	define R300_RB3D_DSTCACHE_CTLSTAT_DC_FINISH_NO_SIGNAL        (0 << 4)
#	define R300_RB3D_DSTCACHE_CTLSTAT_DC_FINISH_SIGNAL           (1 << 4)

#define R300_RB3D_DITHER_CTL 0x4E50
#	define R300_RB3D_DITHER_CTL_DITHER_MODE_TRUNCATE         (0 << 0)
#	define R300_RB3D_DITHER_CTL_DITHER_MODE_ROUND            (1 << 0)
#	define R300_RB3D_DITHER_CTL_DITHER_MODE_LUT              (2 << 0)
/* reserved */
#	define R300_RB3D_DITHER_CTL_ALPHA_DITHER_MODE_TRUNCATE   (0 << 2)
#	define R300_RB3D_DITHER_CTL_ALPHA_DITHER_MODE_ROUND      (1 << 2)
#	define R300_RB3D_DITHER_CTL_ALPHA_DITHER_MODE_LUT        (2 << 2)
/* reserved */

#define R300_RB3D_CMASK_OFFSET0 0x4E54
#define R300_RB3D_CMASK_OFFSET1 0x4E58
#define R300_RB3D_CMASK_OFFSET2 0x4E5C
#define R300_RB3D_CMASK_OFFSET3 0x4E60
#define R300_RB3D_CMASK_PITCH0  0x4E64
#define R300_RB3D_CMASK_PITCH1  0x4E68
#define R300_RB3D_CMASK_PITCH2  0x4E6C
#define R300_RB3D_CMASK_PITCH3  0x4E70
#define R300_RB3D_CMASK_WRINDEX 0x4E74
#define R300_RB3D_CMASK_DWORD   0x4E78
#define R300_RB3D_CMASK_RDINDEX 0x4E7C

/* Resolve buffer destination address. The cache must be empty before changing
 * this register if the cb is in resolve mode. Unpipelined
 */
#define R300_RB3D_AARESOLVE_OFFSET        0x4e80
#	define R300_RB3D_AARESOLVE_OFFSET_SHIFT 5
#	define R300_RB3D_AARESOLVE_OFFSET_MASK 0xffffffe0 /* At least according to the calculations of Christoph Brill */

/* Resolve Buffer Pitch and Tiling Control. The cache must be empty before
 * changing this register if the cb is in resolve mode. Unpipelined
 */
#define R300_RB3D_AARESOLVE_PITCH         0x4e84
#	define R300_RB3D_AARESOLVE_PITCH_SHIFT 1
#	define R300_RB3D_AARESOLVE_PITCH_MASK  0x00003ffe /* At least according to the calculations of Christoph Brill */

/* Resolve Buffer Control. Unpipelined */
#define R300_RB3D_AARESOLVE_CTL           0x4e88
#	define R300_RB3D_AARESOLVE_CTL_AARESOLVE_MODE_NORMAL   (0 << 0)
#	define R300_RB3D_AARESOLVE_CTL_AARESOLVE_MODE_RESOLVE  (1 << 0)
#	define R300_RB3D_AARESOLVE_CTL_AARESOLVE_GAMMA_10      (0 << 1)
#	define R300_RB3D_AARESOLVE_CTL_AARESOLVE_GAMMA_22      (1 << 1)
#	define R300_RB3D_AARESOLVE_CTL_AARESOLVE_ALPHA_SAMPLE0 (0 << 2)
#	define R300_RB3D_AARESOLVE_CTL_AARESOLVE_ALPHA_AVERAGE (1 << 2)


/* Discard src pixels less than or equal to threshold. */
#define R500_RB3D_DISCARD_SRC_PIXEL_LTE_THRESHOLD 0x4ea0
/* Discard src pixels greater than or equal to threshold. */
#define R500_RB3D_DISCARD_SRC_PIXEL_GTE_THRESHOLD 0x4ea4
#	define R500_RB3D_DISCARD_SRC_PIXEL_THRESHOLD_BLUE_SHIFT 0
#	define R500_RB3D_DISCARD_SRC_PIXEL_THRESHOLD_BLUE_MASK 0x000000ff
#	define R500_RB3D_DISCARD_SRC_PIXEL_THRESHOLD_GREEN_SHIFT 8
#	define R500_RB3D_DISCARD_SRC_PIXEL_THRESHOLD_GREEN_MASK 0x0000ff00
#	define R500_RB3D_DISCARD_SRC_PIXEL_THRESHOLD_RED_SHIFT 16
#	define R500_RB3D_DISCARD_SRC_PIXEL_THRESHOLD_RED_MASK 0x00ff0000
#	define R500_RB3D_DISCARD_SRC_PIXEL_THRESHOLD_ALPHA_SHIFT 24
#	define R500_RB3D_DISCARD_SRC_PIXEL_THRESHOLD_ALPHA_MASK 0xff000000

/* 3D ROP Control. Stalls the 2d/3d datapath until it is idle. */
#define R300_RB3D_ROPCNTL                             0x4e18
#	define R300_RB3D_ROPCNTL_ROP_ENABLE            0x00000004
#	define R300_RB3D_ROPCNTL_ROP_MASK              (15 << 8)
#	define R300_RB3D_ROPCNTL_ROP_SHIFT             8

/* Color Compare Flip. Stalls the 2d/3d datapath until it is idle. */
#define R300_RB3D_CLRCMP_FLIPE                        0x4e1c

/* Sets the fifo sizes */
#define R500_RB3D_FIFO_SIZE                           0x4ef4
#	define R500_RB3D_FIFO_SIZE_OP_FIFO_SIZE_FULL   (0 << 0)
#	define R500_RB3D_FIFO_SIZE_OP_FIFO_SIZE_HALF   (1 << 0)
#	define R500_RB3D_FIFO_SIZE_OP_FIFO_SIZE_QUATER (2 << 0)
#	define R500_RB3D_FIFO_SIZE_OP_FIFO_SIZE_EIGTHS (3 << 0)

/* Constant color used by the blender. Pipelined through the blender. */
#define R500_RB3D_CONSTANT_COLOR_AR                   0x4ef8
#	define R500_RB3D_CONSTANT_COLOR_AR_RED_MASK    0x0000ffff
#	define R500_RB3D_CONSTANT_COLOR_AR_RED_SHIFT   0
#	define R500_RB3D_CONSTANT_COLOR_AR_ALPHA_MASK  0xffff0000
#	define R500_RB3D_CONSTANT_COLOR_AR_ALPHA_SHIFT 16

/* Constant color used by the blender. Pipelined through the blender. */
#define R500_RB3D_CONSTANT_COLOR_GB                   0x4efc
#	define R500_RB3D_CONSTANT_COLOR_AR_BLUE_MASK   0x0000ffff
#	define R500_RB3D_CONSTANT_COLOR_AR_BLUE_SHIFT  0
#	define R500_RB3D_CONSTANT_COLOR_AR_GREEN_MASK  0xffff0000
#	define R500_RB3D_CONSTANT_COLOR_AR_GREEN_SHIFT 16

/* gap */
/* There seems to be no "write only" setting, so use Z-test = ALWAYS
 * for this.
 * Bit (1<<8) is the "test" bit. so plain write is 6  - vd
 */
#define R300_ZB_CNTL                             0x4F00
#	define R300_STENCIL_ENABLE		 (1 << 0)
#	define R300_Z_ENABLE		         (1 << 1)
#	define R300_Z_WRITE_ENABLE		 (1 << 2)
#	define R300_Z_SIGNED_COMPARE		 (1 << 3)
#	define R300_STENCIL_FRONT_BACK		 (1 << 4)
#   define R500_STENCIL_ZSIGNED_MAGNITUDE (1 << 5)
#   define R500_STENCIL_REFMASK_FRONT_BACK (1 << 6)

#define R300_ZB_ZSTENCILCNTL                   0x4f04
	/* functions */
#	define R300_ZS_NEVER			0
#	define R300_ZS_LESS			1
#	define R300_ZS_LEQUAL			2
#	define R300_ZS_EQUAL			3
#	define R300_ZS_GEQUAL			4
#	define R300_ZS_GREATER			5
#	define R300_ZS_NOTEQUAL			6
#	define R300_ZS_ALWAYS			7
#       define R300_ZS_MASK                     7
	/* operations */
#	define R300_ZS_KEEP			0
#	define R300_ZS_ZERO			1
#	define R300_ZS_REPLACE			2
#	define R300_ZS_INCR			3
#	define R300_ZS_DECR			4
#	define R300_ZS_INVERT			5
#	define R300_ZS_INCR_WRAP		6
#	define R300_ZS_DECR_WRAP		7
#	define R300_Z_FUNC_SHIFT		0
	/* front and back refer to operations done for front
	   and back faces, i.e. separate stencil function support */
#	define R300_S_FRONT_FUNC_SHIFT	        3
#	define R300_S_FRONT_SFAIL_OP_SHIFT	6
#	define R300_S_FRONT_ZPASS_OP_SHIFT	9
#	define R300_S_FRONT_ZFAIL_OP_SHIFT      12
#	define R300_S_BACK_FUNC_SHIFT           15
#	define R300_S_BACK_SFAIL_OP_SHIFT       18
#	define R300_S_BACK_ZPASS_OP_SHIFT       21
#	define R300_S_BACK_ZFAIL_OP_SHIFT       24

#define R300_ZB_STENCILREFMASK                        0x4f08
#	define R300_STENCILREF_SHIFT       0
#	define R300_STENCILREF_MASK        0x000000ff
#	define R300_STENCILMASK_SHIFT      8
#	define R300_STENCILMASK_MASK       0x0000ff00
#	define R300_STENCILWRITEMASK_SHIFT 16
#	define R300_STENCILWRITEMASK_MASK  0x00ff0000

/* gap */

#define R300_ZB_FORMAT                             0x4f10
#	define R300_DEPTHFORMAT_16BIT_INT_Z   (0 << 0)
#	define R300_DEPTHFORMAT_16BIT_13E3    (1 << 0)
#	define R300_DEPTHFORMAT_24BIT_INT_Z_8BIT_STENCIL   (2 << 0)
/* reserved up to (15 << 0) */
#	define R300_INVERT_13E3_LEADING_ONES  (0 << 4)
#	define R300_INVERT_13E3_LEADING_ZEROS (1 << 4)

#define R300_ZB_ZTOP                             0x4F14
#	define R300_ZTOP_DISABLE                 (0 << 0)
#	define R300_ZTOP_ENABLE                  (1 << 0)

/* gap */

#define R300_ZB_ZCACHE_CTLSTAT            0x4f18
#       define R300_ZB_ZCACHE_CTLSTAT_ZC_FLUSH_NO_EFFECT      (0 << 0)
#       define R300_ZB_ZCACHE_CTLSTAT_ZC_FLUSH_FLUSH_AND_FREE (1 << 0)
#       define R300_ZB_ZCACHE_CTLSTAT_ZC_FREE_NO_EFFECT       (0 << 1)
#       define R300_ZB_ZCACHE_CTLSTAT_ZC_FREE_FREE            (1 << 1)
#       define R300_ZB_ZCACHE_CTLSTAT_ZC_BUSY_IDLE            (0 << 31)
#       define R300_ZB_ZCACHE_CTLSTAT_ZC_BUSY_BUSY            (1 << 31)

#define R300_ZB_BW_CNTL                     0x4f1c
#	define R300_HIZ_DISABLE                              (0 << 0)
#	define R300_HIZ_ENABLE                               (1 << 0)
#	define R300_HIZ_MAX                                  (0 << 1)
#	define R300_HIZ_MIN                                  (1 << 1)
#	define R300_FAST_FILL_DISABLE                        (0 << 2)
#	define R300_FAST_FILL_ENABLE                         (1 << 2)
#	define R300_RD_COMP_DISABLE                          (0 << 3)
#	define R300_RD_COMP_ENABLE                           (1 << 3)
#	define R300_WR_COMP_DISABLE                          (0 << 4)
#	define R300_WR_COMP_ENABLE                           (1 << 4)
#	define R300_ZB_CB_CLEAR_RMW                          (0 << 5)
#	define R300_ZB_CB_CLEAR_CACHE_LINE_WRITE_ONLY        (1 << 5)
#	define R300_FORCE_COMPRESSED_STENCIL_VALUE_DISABLE   (0 << 6)
#	define R300_FORCE_COMPRESSED_STENCIL_VALUE_ENABLE    (1 << 6)

#	define R500_ZEQUAL_OPTIMIZE_ENABLE                   (0 << 7)
#	define R500_ZEQUAL_OPTIMIZE_DISABLE                  (1 << 7)
#	define R500_SEQUAL_OPTIMIZE_ENABLE                   (0 << 8)
#	define R500_SEQUAL_OPTIMIZE_DISABLE                  (1 << 8)

#	define R500_BMASK_ENABLE                             (0 << 10)
#	define R500_BMASK_DISABLE                            (1 << 10)
#	define R500_HIZ_EQUAL_REJECT_DISABLE                 (0 << 11)
#	define R500_HIZ_EQUAL_REJECT_ENABLE                  (1 << 11)
#	define R500_HIZ_FP_EXP_BITS_DISABLE                  (0 << 12)
#	define R500_HIZ_FP_EXP_BITS_1                        (1 << 12)
#	define R500_HIZ_FP_EXP_BITS_2                        (2 << 12)
#	define R500_HIZ_FP_EXP_BITS_3                        (3 << 12)
#	define R500_HIZ_FP_EXP_BITS_4                        (4 << 12)
#	define R500_HIZ_FP_EXP_BITS_5                        (5 << 12)
#	define R500_HIZ_FP_INVERT_LEADING_ONES               (0 << 15)
#	define R500_HIZ_FP_INVERT_LEADING_ZEROS              (1 << 15)
#	define R500_TILE_OVERWRITE_RECOMPRESSION_ENABLE      (0 << 16)
#	define R500_TILE_OVERWRITE_RECOMPRESSION_DISABLE     (1 << 16)
#	define R500_CONTIGUOUS_6XAA_SAMPLES_ENABLE           (0 << 17)
#	define R500_CONTIGUOUS_6XAA_SAMPLES_DISABLE          (1 << 17)
#	define R500_PEQ_PACKING_DISABLE                      (0 << 18)
#	define R500_PEQ_PACKING_ENABLE                       (1 << 18)
#	define R500_COVERED_PTR_MASKING_DISABLE              (0 << 19)
#	define R500_COVERED_PTR_MASKING_ENABLE               (1 << 19)


/* gap */

/* Z Buffer Address Offset.
 * Bits 31 to 5 are used for aligned Z buffer address offset for macro tiles.
 */
#define R300_ZB_DEPTHOFFSET               0x4f20

/* Z Buffer Pitch and Endian Control */
#define R300_ZB_DEPTHPITCH                0x4f24
#       define R300_DEPTHPITCH_MASK              0x00003FFC
#       define R300_DEPTHMACROTILE_DISABLE      (0 << 16)
#       define R300_DEPTHMACROTILE_ENABLE       (1 << 16)
#       define R300_DEPTHMACROTILE(x)           ((x) << 16)
#       define R300_DEPTHMICROTILE_LINEAR       (0 << 17)
#       define R300_DEPTHMICROTILE_TILED        (1 << 17)
#       define R300_DEPTHMICROTILE_TILED_SQUARE (2 << 17)
#       define R300_DEPTHMICROTILE(x)           ((x) << 17)
#       define R300_DEPTHENDIAN(x)              ((x) << 19)

#define R300_SURF_NO_SWAP         0
#define R300_SURF_WORD_SWAP       1
#define R300_SURF_DWORD_SWAP      2
#define R300_SURF_HALF_DWORD_SWAP 3

/* Z Buffer Clear Value */
#define R300_ZB_DEPTHCLEARVALUE                  0x4f28

/* Z Mask RAM is a Z compression buffer.
 * Each dword of the Z Mask contains compression info for 16 4x4 pixel blocks,
 * that is 2 bits for each block.
 * On chips with 2 Z pipes, every other dword maps to a different pipe.
 */

/* The dword offset into Z mask RAM (bits 18:4) */
#define R300_ZB_ZMASK_OFFSET                     0x4f30

/* Z Mask Pitch. */
#define R300_ZB_ZMASK_PITCH                      0x4f34

/* Access to Z Mask RAM in a manner similar to HiZ RAM.
 * The indices are autoincrementing. */
#define R300_ZB_ZMASK_WRINDEX                    0x4f38
#define R300_ZB_ZMASK_DWORD                      0x4f3c
#define R300_ZB_ZMASK_RDINDEX                    0x4f40

/* Hierarchical Z Memory Offset */
#define R300_ZB_HIZ_OFFSET                       0x4f44

/* Hierarchical Z Write Index */
#define R300_ZB_HIZ_WRINDEX                      0x4f48

/* Hierarchical Z Data */
#define R300_ZB_HIZ_DWORD                        0x4f4c

/* Hierarchical Z Read Index */
#define R300_ZB_HIZ_RDINDEX                      0x4f50

/* Hierarchical Z Pitch */
#define R300_ZB_HIZ_PITCH                        0x4f54

/* Z Buffer Z Pass Counter Data */
#define R300_ZB_ZPASS_DATA                       0x4f58

/* Z Buffer Z Pass Counter Address */
#define R300_ZB_ZPASS_ADDR                       0x4f5c

/* Depth buffer X and Y coordinate offset */
#define R300_ZB_DEPTHXY_OFFSET                   0x4f60
#	define R300_DEPTHX_OFFSET_SHIFT  1
#	define R300_DEPTHX_OFFSET_MASK   0x000007FE
#	define R300_DEPTHY_OFFSET_SHIFT  17
#	define R300_DEPTHY_OFFSET_MASK   0x07FE0000

/* Sets the fifo sizes */
#define R500_ZB_FIFO_SIZE                        0x4fd0
#	define R500_OP_FIFO_SIZE_FULL   (0 << 0)
#	define R500_OP_FIFO_SIZE_HALF   (1 << 0)
#	define R500_OP_FIFO_SIZE_QUATER (2 << 0)
#	define R500_OP_FIFO_SIZE_EIGTHS (4 << 0)

/* Stencil Reference Value and Mask for backfacing quads */
/* R300_ZB_STENCILREFMASK handles front face */
#define R500_ZB_STENCILREFMASK_BF                0x4fd4
#	define R500_STENCILREF_SHIFT       0
#	define R500_STENCILREF_MASK        0x000000ff
#	define R500_STENCILMASK_SHIFT      8
#	define R500_STENCILMASK_MASK       0x0000ff00
#	define R500_STENCILWRITEMASK_SHIFT 16
#	define R500_STENCILWRITEMASK_MASK  0x00ff0000

/**
 * \defgroup R3XX_R5XX_PROGRAMMABLE_VERTEX_SHADER_DESCRIPTION R3XX-R5XX PROGRAMMABLE VERTEX SHADER DESCRIPTION
 *
 * The PVS_DST_MATH_INST is used to identify whether the instruction is a Vector
 * Engine instruction or a Math Engine instruction.
 */

/*\{*/

enum {
	/* R3XX */
	VECTOR_NO_OP			= 0,
	VE_DOT_PRODUCT			= 1,
	VE_MULTIPLY			= 2,
	VE_ADD				= 3,
	VE_MULTIPLY_ADD			= 4,
	VE_DISTANCE_VECTOR		= 5,
	VE_FRACTION			= 6,
	VE_MAXIMUM			= 7,
	VE_MINIMUM			= 8,
	VE_SET_GREATER_THAN_EQUAL	= 9,
	VE_SET_LESS_THAN		= 10,
	VE_MULTIPLYX2_ADD		= 11,
	VE_MULTIPLY_CLAMP		= 12,
	VE_FLT2FIX_DX			= 13,
	VE_FLT2FIX_DX_RND		= 14,
	/* R5XX */
	VE_PRED_SET_EQ_PUSH		= 15,
	VE_PRED_SET_GT_PUSH		= 16,
	VE_PRED_SET_GTE_PUSH		= 17,
	VE_PRED_SET_NEQ_PUSH		= 18,
	VE_COND_WRITE_EQ		= 19,
	VE_COND_WRITE_GT		= 20,
	VE_COND_WRITE_GTE		= 21,
	VE_COND_WRITE_NEQ		= 22,
	VE_COND_MUX_EQ			= 23,
	VE_COND_MUX_GT			= 24,
	VE_COND_MUX_GTE			= 25,
	VE_SET_GREATER_THAN		= 26,
	VE_SET_EQUAL			= 27,
	VE_SET_NOT_EQUAL		= 28
};

enum {
	/* R3XX */
	MATH_NO_OP			= 0,
	ME_EXP_BASE2_DX			= 1,
	ME_LOG_BASE2_DX			= 2,
	ME_EXP_BASEE_FF			= 3,
	ME_LIGHT_COEFF_DX		= 4,
	ME_POWER_FUNC_FF		= 5,
	ME_RECIP_DX			= 6,
	ME_RECIP_FF			= 7,
	ME_RECIP_SQRT_DX		= 8,
	ME_RECIP_SQRT_FF		= 9,
	ME_MULTIPLY			= 10,
	ME_EXP_BASE2_FULL_DX		= 11,
	ME_LOG_BASE2_FULL_DX		= 12,
	ME_POWER_FUNC_FF_CLAMP_B	= 13,
	ME_POWER_FUNC_FF_CLAMP_B1	= 14,
	ME_POWER_FUNC_FF_CLAMP_01	= 15,
	ME_SIN				= 16,
	ME_COS				= 17,
	/* R5XX */
	ME_LOG_BASE2_IEEE		= 18,
	ME_RECIP_IEEE			= 19,
	ME_RECIP_SQRT_IEEE		= 20,
	ME_PRED_SET_EQ			= 21,
	ME_PRED_SET_GT			= 22,
	ME_PRED_SET_GTE			= 23,
	ME_PRED_SET_NEQ			= 24,
	ME_PRED_SET_CLR			= 25,
	ME_PRED_SET_INV			= 26,
	ME_PRED_SET_POP			= 27,
	ME_PRED_SET_RESTORE		= 28
};

enum {
	/* R3XX */
	PVS_MACRO_OP_2CLK_MADD		= 0,
	PVS_MACRO_OP_2CLK_M2X_ADD	= 1
};

enum {
	PVS_SRC_REG_TEMPORARY		= 0,	/* Intermediate Storage */
	PVS_SRC_REG_INPUT		= 1,	/* Input Vertex Storage */
	PVS_SRC_REG_CONSTANT		= 2,	/* Constant State Storage */
	PVS_SRC_REG_ALT_TEMPORARY	= 3	/* Alternate Intermediate Storage */
};

enum {
	PVS_DST_REG_TEMPORARY		= 0,	/* Intermediate Storage */
	PVS_DST_REG_A0			= 1,	/* Address Register Storage */
	PVS_DST_REG_OUT			= 2,	/* Output Memory. Used for all outputs */
	PVS_DST_REG_OUT_REPL_X		= 3,	/* Output Memory & Replicate X to all channels */
	PVS_DST_REG_ALT_TEMPORARY	= 4,	/* Alternate Intermediate Storage */
	PVS_DST_REG_INPUT		= 5	/* Output Memory & Replicate X to all channels */
};

enum {
	PVS_SRC_SELECT_X		= 0,	/* Select X Component */
	PVS_SRC_SELECT_Y		= 1,	/* Select Y Component */
	PVS_SRC_SELECT_Z		= 2,	/* Select Z Component */
	PVS_SRC_SELECT_W		= 3,	/* Select W Component */
	PVS_SRC_SELECT_FORCE_0		= 4,	/* Force Component to 0.0 */
	PVS_SRC_SELECT_FORCE_1		= 5	/* Force Component to 1.0 */
};

/* PVS Opcode & Destination Operand Description */

enum {
	PVS_DST_OPCODE_MASK		= 0x3f,
	PVS_DST_OPCODE_SHIFT		= 0,
	PVS_DST_MATH_INST_MASK		= 0x1,
	PVS_DST_MATH_INST_SHIFT		= 6,
	PVS_DST_MACRO_INST_MASK		= 0x1,
	PVS_DST_MACRO_INST_SHIFT	= 7,
	PVS_DST_REG_TYPE_MASK		= 0xf,
	PVS_DST_REG_TYPE_SHIFT		= 8,
	PVS_DST_ADDR_MODE_1_MASK	= 0x1,
	PVS_DST_ADDR_MODE_1_SHIFT	= 12,
	PVS_DST_OFFSET_MASK		= 0x7f,
	PVS_DST_OFFSET_SHIFT		= 13,
	PVS_DST_WE_X_MASK		= 0x1,
	PVS_DST_WE_X_SHIFT		= 20,
	PVS_DST_WE_Y_MASK		= 0x1,
	PVS_DST_WE_Y_SHIFT		= 21,
	PVS_DST_WE_Z_MASK		= 0x1,
	PVS_DST_WE_Z_SHIFT		= 22,
	PVS_DST_WE_W_MASK		= 0x1,
	PVS_DST_WE_W_SHIFT		= 23,
	PVS_DST_VE_SAT_MASK		= 0x1,
	PVS_DST_VE_SAT_SHIFT		= 24,
	PVS_DST_ME_SAT_MASK		= 0x1,
	PVS_DST_ME_SAT_SHIFT		= 25,
	PVS_DST_PRED_ENABLE_MASK	= 0x1,
	PVS_DST_PRED_ENABLE_SHIFT	= 26,
	PVS_DST_PRED_SENSE_MASK		= 0x1,
	PVS_DST_PRED_SENSE_SHIFT	= 27,
	PVS_DST_DUAL_MATH_OP_MASK	= 0x3,
	PVS_DST_DUAL_MATH_OP_SHIFT	= 27,
	PVS_DST_ADDR_SEL_MASK		= 0x3,
	PVS_DST_ADDR_SEL_SHIFT		= 29,
	PVS_DST_ADDR_MODE_0_MASK	= 0x1,
	PVS_DST_ADDR_MODE_0_SHIFT	= 31
};

/* PVS Source Operand Description */

enum {
	PVS_SRC_REG_TYPE_MASK		= 0x3,
	PVS_SRC_REG_TYPE_SHIFT		= 0,
	SPARE_0_MASK			= 0x1,
	SPARE_0_SHIFT			= 2,
	PVS_SRC_ABS_XYZW_MASK		= 0x1,
	PVS_SRC_ABS_XYZW_SHIFT		= 3,
	PVS_SRC_ADDR_MODE_0_MASK	= 0x1,
	PVS_SRC_ADDR_MODE_0_SHIFT	= 4,
	PVS_SRC_OFFSET_MASK		= 0xff,
	PVS_SRC_OFFSET_SHIFT		= 5,
	PVS_SRC_SWIZZLE_X_MASK		= 0x7,
	PVS_SRC_SWIZZLE_X_SHIFT		= 13,
	PVS_SRC_SWIZZLE_Y_MASK		= 0x7,
	PVS_SRC_SWIZZLE_Y_SHIFT		= 16,
	PVS_SRC_SWIZZLE_Z_MASK		= 0x7,
	PVS_SRC_SWIZZLE_Z_SHIFT		= 19,
	PVS_SRC_SWIZZLE_W_MASK		= 0x7,
	PVS_SRC_SWIZZLE_W_SHIFT		= 22,
	PVS_SRC_MODIFIER_X_MASK		= 0x1,
	PVS_SRC_MODIFIER_X_SHIFT	= 25,
	PVS_SRC_MODIFIER_Y_MASK		= 0x1,
	PVS_SRC_MODIFIER_Y_SHIFT	= 26,
	PVS_SRC_MODIFIER_Z_MASK		= 0x1,
	PVS_SRC_MODIFIER_Z_SHIFT	= 27,
	PVS_SRC_MODIFIER_W_MASK		= 0x1,
	PVS_SRC_MODIFIER_W_SHIFT	= 28,
	PVS_SRC_ADDR_SEL_MASK		= 0x3,
	PVS_SRC_ADDR_SEL_SHIFT		= 29,
	PVS_SRC_ADDR_MODE_1_MASK	= 0x0,
	PVS_SRC_ADDR_MODE_1_SHIFT	= 32
};

/*\}*/

#define PVS_OP_DST_OPERAND(opcode, math_inst, macro_inst, reg_index, reg_writemask, reg_class, saturate)	\
	 (((opcode & PVS_DST_OPCODE_MASK) << PVS_DST_OPCODE_SHIFT)	\
	 | ((math_inst & PVS_DST_MATH_INST_MASK) << PVS_DST_MATH_INST_SHIFT)	\
	 | ((macro_inst & PVS_DST_MACRO_INST_MASK) << PVS_DST_MACRO_INST_SHIFT)	\
	 | ((reg_index & PVS_DST_OFFSET_MASK) << PVS_DST_OFFSET_SHIFT)	\
	 | ((reg_writemask & 0xf) << PVS_DST_WE_X_SHIFT)	/* X Y Z W */	\
	 | ((reg_class & PVS_DST_REG_TYPE_MASK) << PVS_DST_REG_TYPE_SHIFT)) \
         | ((math_inst) ? (((saturate) & PVS_DST_ME_SAT_MASK) << PVS_DST_ME_SAT_SHIFT) : \
                          (((saturate) & PVS_DST_VE_SAT_MASK) << PVS_DST_VE_SAT_SHIFT))

#define PVS_SRC_OPERAND(in_reg_index, comp_x, comp_y, comp_z, comp_w, reg_class, negate)	\
	(((in_reg_index & PVS_SRC_OFFSET_MASK) << PVS_SRC_OFFSET_SHIFT)				\
	 | ((comp_x & PVS_SRC_SWIZZLE_X_MASK) << PVS_SRC_SWIZZLE_X_SHIFT)			\
	 | ((comp_y & PVS_SRC_SWIZZLE_Y_MASK) << PVS_SRC_SWIZZLE_Y_SHIFT)			\
	 | ((comp_z & PVS_SRC_SWIZZLE_Z_MASK) << PVS_SRC_SWIZZLE_Z_SHIFT)			\
	 | ((comp_w & PVS_SRC_SWIZZLE_W_MASK) << PVS_SRC_SWIZZLE_W_SHIFT)			\
	 | ((negate & 0xf) << PVS_SRC_MODIFIER_X_SHIFT)	/* X Y Z W */				\
	 | ((reg_class & PVS_SRC_REG_TYPE_MASK) << PVS_SRC_REG_TYPE_SHIFT))

/* BEGIN: Packet 3 commands */

/* A primitive emission dword. */
#define R300_PRIM_TYPE_NONE                     (0 << 0)
#define R300_PRIM_TYPE_POINT                    (1 << 0)
#define R300_PRIM_TYPE_LINE                     (2 << 0)
#define R300_PRIM_TYPE_LINE_STRIP               (3 << 0)
#define R300_PRIM_TYPE_TRI_LIST                 (4 << 0)
#define R300_PRIM_TYPE_TRI_FAN                  (5 << 0)
#define R300_PRIM_TYPE_TRI_STRIP                (6 << 0)
#define R300_PRIM_TYPE_TRI_TYPE2                (7 << 0)
#define R300_PRIM_TYPE_RECT_LIST                (8 << 0)
#define R300_PRIM_TYPE_3VRT_POINT_LIST          (9 << 0)
#define R300_PRIM_TYPE_3VRT_LINE_LIST           (10 << 0)
	/* GUESS (based on r200) */
#define R300_PRIM_TYPE_POINT_SPRITES            (11 << 0)
#define R300_PRIM_TYPE_LINE_LOOP                (12 << 0)
#define R300_PRIM_TYPE_QUADS                    (13 << 0)
#define R300_PRIM_TYPE_QUAD_STRIP               (14 << 0)
#define R300_PRIM_TYPE_POLYGON                  (15 << 0)
#define R300_PRIM_TYPE_MASK                     0xF
#define R300_PRIM_WALK_IND                      (1 << 4)
#define R300_PRIM_WALK_LIST                     (2 << 4)
#define R300_PRIM_WALK_RING                     (3 << 4)
#define R300_PRIM_WALK_MASK                     (3 << 4)
	/* GUESS (based on r200) */
#define R300_PRIM_COLOR_ORDER_BGRA              (0 << 6)
#define R300_PRIM_COLOR_ORDER_RGBA              (1 << 6)
#define R300_PRIM_NUM_VERTICES_SHIFT            16
#define R300_PRIM_NUM_VERTICES_MASK             0xffff



/*
 * The R500 unified shader (US) registers come in banks of 512 each, one
 * for each instruction slot in the shader.  You can't touch them directly.
 * R500_US_VECTOR_INDEX() sets the base instruction to modify; successive
 * writes to R500_GA_US_VECTOR_DATA autoincrement the index after the
 * instruction is fully specified.
 */
#define R500_US_ALU_ALPHA_INST_0			0xa800
#   define R500_ALPHA_OP_MAD				0
#   define R500_ALPHA_OP_DP				1
#   define R500_ALPHA_OP_MIN				2
#   define R500_ALPHA_OP_MAX				3
/* #define R500_ALPHA_OP_RESERVED			4 */
#   define R500_ALPHA_OP_CND				5
#   define R500_ALPHA_OP_CMP				6
#   define R500_ALPHA_OP_FRC				7
#   define R500_ALPHA_OP_EX2				8
#   define R500_ALPHA_OP_LN2				9
#   define R500_ALPHA_OP_RCP				10
#   define R500_ALPHA_OP_RSQ				11
#   define R500_ALPHA_OP_SIN				12
#   define R500_ALPHA_OP_COS				13
#   define R500_ALPHA_OP_MDH				14
#   define R500_ALPHA_OP_MDV				15
#   define R500_ALPHA_ADDRD(x)				((x) << 4)
#   define R500_ALPHA_ADDRD_REL				(1 << 11)
#  define R500_ALPHA_SEL_A_SHIFT			12
#   define R500_ALPHA_SEL_A_SRC0			(0 << 12)
#   define R500_ALPHA_SEL_A_SRC1			(1 << 12)
#   define R500_ALPHA_SEL_A_SRC2			(2 << 12)
#   define R500_ALPHA_SEL_A_SRCP			(3 << 12)
#   define R500_ALPHA_SWIZ_A_R				(0 << 14)
#   define R500_ALPHA_SWIZ_A_G				(1 << 14)
#   define R500_ALPHA_SWIZ_A_B				(2 << 14)
#   define R500_ALPHA_SWIZ_A_A				(3 << 14)
#   define R500_ALPHA_SWIZ_A_0				(4 << 14)
#   define R500_ALPHA_SWIZ_A_HALF			(5 << 14)
#   define R500_ALPHA_SWIZ_A_1				(6 << 14)
/* #define R500_ALPHA_SWIZ_A_UNUSED			(7 << 14) */
#   define R500_ALPHA_MOD_A_NOP				(0 << 17)
#   define R500_ALPHA_MOD_A_NEG				(1 << 17)
#   define R500_ALPHA_MOD_A_ABS				(2 << 17)
#   define R500_ALPHA_MOD_A_NAB				(3 << 17)
#  define R500_ALPHA_SEL_B_SHIFT			19
#   define R500_ALPHA_SEL_B_SRC0			(0 << 19)
#   define R500_ALPHA_SEL_B_SRC1			(1 << 19)
#   define R500_ALPHA_SEL_B_SRC2			(2 << 19)
#   define R500_ALPHA_SEL_B_SRCP			(3 << 19)
#   define R500_ALPHA_SWIZ_B_R				(0 << 21)
#   define R500_ALPHA_SWIZ_B_G				(1 << 21)
#   define R500_ALPHA_SWIZ_B_B				(2 << 21)
#   define R500_ALPHA_SWIZ_B_A				(3 << 21)
#   define R500_ALPHA_SWIZ_B_0				(4 << 21)
#   define R500_ALPHA_SWIZ_B_HALF			(5 << 21)
#   define R500_ALPHA_SWIZ_B_1				(6 << 21)
/* #define R500_ALPHA_SWIZ_B_UNUSED			(7 << 21) */
#   define R500_ALPHA_MOD_B_NOP				(0 << 24)
#   define R500_ALPHA_MOD_B_NEG				(1 << 24)
#   define R500_ALPHA_MOD_B_ABS				(2 << 24)
#   define R500_ALPHA_MOD_B_NAB				(3 << 24)
#   define R500_ALPHA_OMOD_SHIFT		26
#   define R500_ALPHA_OMOD_IDENTITY		(0 << R500_ALPHA_OMOD_SHIFT)
#   define R500_ALPHA_OMOD_MUL_2		(1 << R500_ALPHA_OMOD_SHIFT)
#   define R500_ALPHA_OMOD_MUL_4		(2 << R500_ALPHA_OMOD_SHIFT)
#   define R500_ALPHA_OMOD_MUL_8		(3 << R500_ALPHA_OMOD_SHIFT)
#   define R500_ALPHA_OMOD_DIV_2		(4 << R500_ALPHA_OMOD_SHIFT)
#   define R500_ALPHA_OMOD_DIV_4		(5 << R500_ALPHA_OMOD_SHIFT)
#   define R500_ALPHA_OMOD_DIV_8		(6 << R500_ALPHA_OMOD_SHIFT)
#   define R500_ALPHA_OMOD_DISABLE		(7 << R500_ALPHA_OMOD_SHIFT)
#   define R500_ALPHA_TARGET(x)				((x) << 29)
#   define R500_ALPHA_W_OMASK				(1 << 31)
#define R500_US_ALU_ALPHA_ADDR_0			0x9800
#   define R500_ALPHA_ADDR0(x)				((x) << 0)
#   define R500_ALPHA_ADDR0_CONST			(1 << 8)
#   define R500_ALPHA_ADDR0_REL				(1 << 9)
#   define R500_ALPHA_ADDR1(x)				((x) << 10)
#   define R500_ALPHA_ADDR1_CONST			(1 << 18)
#   define R500_ALPHA_ADDR1_REL				(1 << 19)
#   define R500_ALPHA_ADDR2(x)				((x) << 20)
#   define R500_ALPHA_ADDR2_CONST			(1 << 28)
#   define R500_ALPHA_ADDR2_REL				(1 << 29)
#   define R500_ALPHA_SRCP_OP_1_MINUS_2A0		(0 << 30)
#   define R500_ALPHA_SRCP_OP_A1_MINUS_A0		(1 << 30)
#   define R500_ALPHA_SRCP_OP_A1_PLUS_A0		(2 << 30)
#   define R500_ALPHA_SRCP_OP_1_MINUS_A0		(3 << 30)
#define R500_US_ALU_RGBA_INST_0				0xb000
#   define R500_ALU_RGBA_OP_MAD				(0 << 0)
#   define R500_ALU_RGBA_OP_DP3				(1 << 0)
#   define R500_ALU_RGBA_OP_DP4				(2 << 0)
#   define R500_ALU_RGBA_OP_D2A				(3 << 0)
#   define R500_ALU_RGBA_OP_MIN				(4 << 0)
#   define R500_ALU_RGBA_OP_MAX				(5 << 0)
/* #define R500_ALU_RGBA_OP_RESERVED			(6 << 0) */
#   define R500_ALU_RGBA_OP_CND				(7 << 0)
#   define R500_ALU_RGBA_OP_CMP				(8 << 0)
#   define R500_ALU_RGBA_OP_FRC				(9 << 0)
#   define R500_ALU_RGBA_OP_SOP				(10 << 0)
#   define R500_ALU_RGBA_OP_MDH				(11 << 0)
#   define R500_ALU_RGBA_OP_MDV				(12 << 0)
#   define R500_ALU_RGBA_ADDRD(x)			((x) << 4)
#   define R500_ALU_RGBA_ADDRD_REL			(1 << 11)
#  define R500_ALU_RGBA_SEL_C_SHIFT			12
#   define R500_ALU_RGBA_SEL_C_SRC0			(0 << 12)
#   define R500_ALU_RGBA_SEL_C_SRC1			(1 << 12)
#   define R500_ALU_RGBA_SEL_C_SRC2			(2 << 12)
#   define R500_ALU_RGBA_SEL_C_SRCP			(3 << 12)
#   define R500_ALU_RGBA_R_SWIZ_R			(0 << 14)
#   define R500_ALU_RGBA_R_SWIZ_G			(1 << 14)
#   define R500_ALU_RGBA_R_SWIZ_B			(2 << 14)
#   define R500_ALU_RGBA_R_SWIZ_A			(3 << 14)
#   define R500_ALU_RGBA_R_SWIZ_0			(4 << 14)
#   define R500_ALU_RGBA_R_SWIZ_HALF			(5 << 14)
#   define R500_ALU_RGBA_R_SWIZ_1			(6 << 14)
/* #define R500_ALU_RGBA_R_SWIZ_UNUSED			(7 << 14) */
#   define R500_ALU_RGBA_G_SWIZ_R			(0 << 17)
#   define R500_ALU_RGBA_G_SWIZ_G			(1 << 17)
#   define R500_ALU_RGBA_G_SWIZ_B			(2 << 17)
#   define R500_ALU_RGBA_G_SWIZ_A			(3 << 17)
#   define R500_ALU_RGBA_G_SWIZ_0			(4 << 17)
#   define R500_ALU_RGBA_G_SWIZ_HALF			(5 << 17)
#   define R500_ALU_RGBA_G_SWIZ_1			(6 << 17)
/* #define R500_ALU_RGBA_G_SWIZ_UNUSED			(7 << 17) */
#   define R500_ALU_RGBA_B_SWIZ_R			(0 << 20)
#   define R500_ALU_RGBA_B_SWIZ_G			(1 << 20)
#   define R500_ALU_RGBA_B_SWIZ_B			(2 << 20)
#   define R500_ALU_RGBA_B_SWIZ_A			(3 << 20)
#   define R500_ALU_RGBA_B_SWIZ_0			(4 << 20)
#   define R500_ALU_RGBA_B_SWIZ_HALF			(5 << 20)
#   define R500_ALU_RGBA_B_SWIZ_1			(6 << 20)
/* #define R500_ALU_RGBA_B_SWIZ_UNUSED			(7 << 20) */
#   define R500_ALU_RGBA_MOD_C_NOP			(0 << 23)
#   define R500_ALU_RGBA_MOD_C_NEG			(1 << 23)
#   define R500_ALU_RGBA_MOD_C_ABS			(2 << 23)
#   define R500_ALU_RGBA_MOD_C_NAB			(3 << 23)
#  define R500_ALU_RGBA_ALPHA_SEL_C_SHIFT		25
#   define R500_ALU_RGBA_ALPHA_SEL_C_SRC0		(0 << 25)
#   define R500_ALU_RGBA_ALPHA_SEL_C_SRC1		(1 << 25)
#   define R500_ALU_RGBA_ALPHA_SEL_C_SRC2		(2 << 25)
#   define R500_ALU_RGBA_ALPHA_SEL_C_SRCP		(3 << 25)
#   define R500_ALU_RGBA_A_SWIZ_R			(0 << 27)
#   define R500_ALU_RGBA_A_SWIZ_G			(1 << 27)
#   define R500_ALU_RGBA_A_SWIZ_B			(2 << 27)
#   define R500_ALU_RGBA_A_SWIZ_A			(3 << 27)
#   define R500_ALU_RGBA_A_SWIZ_0			(4 << 27)
#   define R500_ALU_RGBA_A_SWIZ_HALF			(5 << 27)
#   define R500_ALU_RGBA_A_SWIZ_1			(6 << 27)
/* #define R500_ALU_RGBA_A_SWIZ_UNUSED			(7 << 27) */
#   define R500_ALU_RGBA_ALPHA_MOD_C_NOP		(0 << 30)
#   define R500_ALU_RGBA_ALPHA_MOD_C_NEG		(1 << 30)
#   define R500_ALU_RGBA_ALPHA_MOD_C_ABS		(2 << 30)
#   define R500_ALU_RGBA_ALPHA_MOD_C_NAB		(3 << 30)
#define R500_US_ALU_RGB_INST_0				0xa000
#  define R500_ALU_RGB_SEL_A_SHIFT			0
#   define R500_ALU_RGB_SEL_A_SRC0			(0 << 0)
#   define R500_ALU_RGB_SEL_A_SRC1			(1 << 0)
#   define R500_ALU_RGB_SEL_A_SRC2			(2 << 0)
#   define R500_ALU_RGB_SEL_A_SRCP			(3 << 0)
#   define R500_ALU_RGB_R_SWIZ_A_R			(0 << 2)
#   define R500_ALU_RGB_R_SWIZ_A_G			(1 << 2)
#   define R500_ALU_RGB_R_SWIZ_A_B			(2 << 2)
#   define R500_ALU_RGB_R_SWIZ_A_A			(3 << 2)
#   define R500_ALU_RGB_R_SWIZ_A_0			(4 << 2)
#   define R500_ALU_RGB_R_SWIZ_A_HALF			(5 << 2)
#   define R500_ALU_RGB_R_SWIZ_A_1			(6 << 2)
/* #define R500_ALU_RGB_R_SWIZ_A_UNUSED			(7 << 2) */
#   define R500_ALU_RGB_G_SWIZ_A_R			(0 << 5)
#   define R500_ALU_RGB_G_SWIZ_A_G			(1 << 5)
#   define R500_ALU_RGB_G_SWIZ_A_B			(2 << 5)
#   define R500_ALU_RGB_G_SWIZ_A_A			(3 << 5)
#   define R500_ALU_RGB_G_SWIZ_A_0			(4 << 5)
#   define R500_ALU_RGB_G_SWIZ_A_HALF			(5 << 5)
#   define R500_ALU_RGB_G_SWIZ_A_1			(6 << 5)
/* #define R500_ALU_RGB_G_SWIZ_A_UNUSED			(7 << 5) */
#   define R500_ALU_RGB_B_SWIZ_A_R			(0 << 8)
#   define R500_ALU_RGB_B_SWIZ_A_G			(1 << 8)
#   define R500_ALU_RGB_B_SWIZ_A_B			(2 << 8)
#   define R500_ALU_RGB_B_SWIZ_A_A			(3 << 8)
#   define R500_ALU_RGB_B_SWIZ_A_0			(4 << 8)
#   define R500_ALU_RGB_B_SWIZ_A_HALF			(5 << 8)
#   define R500_ALU_RGB_B_SWIZ_A_1			(6 << 8)
/* #define R500_ALU_RGB_B_SWIZ_A_UNUSED			(7 << 8) */
#   define R500_ALU_RGB_MOD_A_NOP			(0 << 11)
#   define R500_ALU_RGB_MOD_A_NEG			(1 << 11)
#   define R500_ALU_RGB_MOD_A_ABS			(2 << 11)
#   define R500_ALU_RGB_MOD_A_NAB			(3 << 11)
#  define R500_ALU_RGB_SEL_B_SHIFT			13
#   define R500_ALU_RGB_SEL_B_SRC0			(0 << 13)
#   define R500_ALU_RGB_SEL_B_SRC1			(1 << 13)
#   define R500_ALU_RGB_SEL_B_SRC2			(2 << 13)
#   define R500_ALU_RGB_SEL_B_SRCP			(3 << 13)
#   define R500_ALU_RGB_R_SWIZ_B_R			(0 << 15)
#   define R500_ALU_RGB_R_SWIZ_B_G			(1 << 15)
#   define R500_ALU_RGB_R_SWIZ_B_B			(2 << 15)
#   define R500_ALU_RGB_R_SWIZ_B_A			(3 << 15)
#   define R500_ALU_RGB_R_SWIZ_B_0			(4 << 15)
#   define R500_ALU_RGB_R_SWIZ_B_HALF			(5 << 15)
#   define R500_ALU_RGB_R_SWIZ_B_1			(6 << 15)
/* #define R500_ALU_RGB_R_SWIZ_B_UNUSED			(7 << 15) */
#   define R500_ALU_RGB_G_SWIZ_B_R			(0 << 18)
#   define R500_ALU_RGB_G_SWIZ_B_G			(1 << 18)
#   define R500_ALU_RGB_G_SWIZ_B_B			(2 << 18)
#   define R500_ALU_RGB_G_SWIZ_B_A			(3 << 18)
#   define R500_ALU_RGB_G_SWIZ_B_0			(4 << 18)
#   define R500_ALU_RGB_G_SWIZ_B_HALF			(5 << 18)
#   define R500_ALU_RGB_G_SWIZ_B_1			(6 << 18)
/* #define R500_ALU_RGB_G_SWIZ_B_UNUSED			(7 << 18) */
#   define R500_ALU_RGB_B_SWIZ_B_R			(0 << 21)
#   define R500_ALU_RGB_B_SWIZ_B_G			(1 << 21)
#   define R500_ALU_RGB_B_SWIZ_B_B			(2 << 21)
#   define R500_ALU_RGB_B_SWIZ_B_A			(3 << 21)
#   define R500_ALU_RGB_B_SWIZ_B_0			(4 << 21)
#   define R500_ALU_RGB_B_SWIZ_B_HALF			(5 << 21)
#   define R500_ALU_RGB_B_SWIZ_B_1			(6 << 21)
/* #define R500_ALU_RGB_B_SWIZ_B_UNUSED			(7 << 21) */
#   define R500_ALU_RGB_MOD_B_NOP			(0 << 24)
#   define R500_ALU_RGB_MOD_B_NEG			(1 << 24)
#   define R500_ALU_RGB_MOD_B_ABS			(2 << 24)
#   define R500_ALU_RGB_MOD_B_NAB			(3 << 24)
#   define R500_ALU_RGB_OMOD_SHIFT		26
#   define R500_ALU_RGB_OMOD_IDENTITY		(0 << R500_ALU_RGB_OMOD_SHIFT)
#   define R500_ALU_RGB_OMOD_MUL_2		(1 << R500_ALU_RGB_OMOD_SHIFT)
#   define R500_ALU_RGB_OMOD_MUL_4		(2 << R500_ALU_RGB_OMOD_SHIFT)
#   define R500_ALU_RGB_OMOD_MUL_8		(3 << R500_ALU_RGB_OMOD_SHIFT)
#   define R500_ALU_RGB_OMOD_DIV_2		(4 << R500_ALU_RGB_OMOD_SHIFT)
#   define R500_ALU_RGB_OMOD_DIV_4		(5 << R500_ALU_RGB_OMOD_SHIFT)
#   define R500_ALU_RGB_OMOD_DIV_8		(6 << R500_ALU_RGB_OMOD_SHIFT)
#   define R500_ALU_RGB_OMOD_DISABLE		(7 << R500_ALU_RGB_OMOD_SHIFT)
#   define R500_ALU_RGB_TARGET(x)			((x) << 29)
#   define R500_ALU_RGB_WMASK				(1 << 31)
#define R500_US_ALU_RGB_ADDR_0				0x9000
#   define R500_RGB_ADDR0(x)				((x) << 0)
#   define R500_RGB_ADDR0_CONST				(1 << 8)
#   define R500_RGB_ADDR0_REL				(1 << 9)
#   define R500_RGB_ADDR1(x)				((x) << 10)
#   define R500_RGB_ADDR1_CONST				(1 << 18)
#   define R500_RGB_ADDR1_REL				(1 << 19)
#   define R500_RGB_ADDR2(x)				((x) << 20)
#   define R500_RGB_ADDR2_CONST				(1 << 28)
#   define R500_RGB_ADDR2_REL				(1 << 29)
#   define R500_RGB_SRCP_OP_1_MINUS_2RGB0		(0 << 30)
#   define R500_RGB_SRCP_OP_RGB1_MINUS_RGB0		(1 << 30)
#   define R500_RGB_SRCP_OP_RGB1_PLUS_RGB0		(2 << 30)
#   define R500_RGB_SRCP_OP_1_MINUS_RGB0		(3 << 30)
#define R500_US_CMN_INST_0				0xb800
#  define R500_INST_TYPE_MASK				(3 << 0)
#   define R500_INST_TYPE_ALU				(0 << 0)
#   define R500_INST_TYPE_OUT				(1 << 0)
#   define R500_INST_TYPE_FC				(2 << 0)
#   define R500_INST_TYPE_TEX				(3 << 0)
#   define R500_INST_TEX_SEM_WAIT_SHIFT			2
#   define R500_INST_TEX_SEM_WAIT			(1 << R500_INST_TEX_SEM_WAIT_SHIFT)
#   define R500_INST_RGB_PRED_SEL_NONE			(0 << 3)
#   define R500_INST_RGB_PRED_SEL_RGBA			(1 << 3)
#   define R500_INST_RGB_PRED_SEL_RRRR			(2 << 3)
#   define R500_INST_RGB_PRED_SEL_GGGG			(3 << 3)
#   define R500_INST_RGB_PRED_SEL_BBBB			(4 << 3)
#   define R500_INST_RGB_PRED_SEL_AAAA			(5 << 3)
#   define R500_INST_RGB_PRED_INV			(1 << 6)
#   define R500_INST_WRITE_INACTIVE			(1 << 7)
#   define R500_INST_LAST				(1 << 8)
#   define R500_INST_NOP				(1 << 9)
#   define R500_INST_ALU_WAIT				(1 << 10)
#   define R500_INST_RGB_WMASK_R			(1 << 11)
#   define R500_INST_RGB_WMASK_G			(1 << 12)
#   define R500_INST_RGB_WMASK_B			(1 << 13)
#   define R500_INST_RGB_WMASK_RGB			(7 << 11)
#   define R500_INST_ALPHA_WMASK			(1 << 14)
#   define R500_INST_RGB_OMASK_R			(1 << 15)
#   define R500_INST_RGB_OMASK_G			(1 << 16)
#   define R500_INST_RGB_OMASK_B			(1 << 17)
#   define R500_INST_RGB_OMASK_RGB			(7 << 15)
#   define R500_INST_ALPHA_OMASK			(1 << 18)
#   define R500_INST_RGB_CLAMP				(1 << 19)
#   define R500_INST_ALPHA_CLAMP			(1 << 20)
#   define R500_INST_ALU_RESULT_SEL			(1 << 21)
#   define R500_INST_ALU_RESULT_SEL_RED			(0 << 21)
#   define R500_INST_ALU_RESULT_SEL_ALPHA		(1 << 21)
#   define R500_INST_ALPHA_PRED_INV			(1 << 22)
#   define R500_INST_ALU_RESULT_OP_EQ			(0 << 23)
#   define R500_INST_ALU_RESULT_OP_LT			(1 << 23)
#   define R500_INST_ALU_RESULT_OP_GE			(2 << 23)
#   define R500_INST_ALU_RESULT_OP_NE			(3 << 23)
#   define R500_INST_ALPHA_PRED_SEL_NONE		(0 << 25)
#   define R500_INST_ALPHA_PRED_SEL_RGBA		(1 << 25)
#   define R500_INST_ALPHA_PRED_SEL_RRRR		(2 << 25)
#   define R500_INST_ALPHA_PRED_SEL_GGGG		(3 << 25)
#   define R500_INST_ALPHA_PRED_SEL_BBBB		(4 << 25)
#   define R500_INST_ALPHA_PRED_SEL_AAAA		(5 << 25)
/* Next four are guessed, documentation doesn't mention order. */
#   define R500_INST_STAT_WE_R				(1 << 28)
#   define R500_INST_STAT_WE_G				(1 << 29)
#   define R500_INST_STAT_WE_B				(1 << 30)
#   define R500_INST_STAT_WE_A				(1 << 31)

/* note that these are 8 bit lengths, despite the offsets, at least for R500 */
#define R500_US_CODE_ADDR				0x4630
#   define R500_US_CODE_START_ADDR(x)			((x) << 0)
#   define R500_US_CODE_END_ADDR(x)			((x) << 16)
#define R500_US_CODE_OFFSET				0x4638
#   define R500_US_CODE_OFFSET_ADDR(x)			((x) << 0)
#define R500_US_CODE_RANGE				0x4634
#   define R500_US_CODE_RANGE_ADDR(x)			((x) << 0)
#   define R500_US_CODE_RANGE_SIZE(x)			((x) << 16)
#define R500_US_CONFIG					0x4600
#   define R500_ZERO_TIMES_ANYTHING_EQUALS_ZERO		(1 << 1)
#define R500_US_FC_ADDR_0				0xa000
#   define R500_FC_BOOL_ADDR(x)				((x) << 0)
#   define R500_FC_INT_ADDR(x)				((x) << 8)
#   define R500_FC_JUMP_ADDR(x)				((x) << 16)
#   define R500_FC_JUMP_GLOBAL				(1 << 31)
#define R500_US_FC_BOOL_CONST				0x4620
#   define R500_FC_KBOOL(x)				(x)
#define R500_US_FC_CTRL					0x4624
#   define R500_FC_TEST_EN				(1 << 30)
#   define R500_FC_FULL_FC_EN				(1U << 31)
#define R500_US_FC_INST_0				0x9800
#   define R500_FC_OP_JUMP				(0 << 0)
#   define R500_FC_OP_LOOP				(1 << 0)
#   define R500_FC_OP_ENDLOOP				(2 << 0)
#   define R500_FC_OP_REP				(3 << 0)
#   define R500_FC_OP_ENDREP				(4 << 0)
#   define R500_FC_OP_BREAKLOOP				(5 << 0)
#   define R500_FC_OP_BREAKREP				(6 << 0)
#   define R500_FC_OP_CONTINUE				(7 << 0)
#   define R500_FC_B_ELSE				(1 << 4)
#   define R500_FC_JUMP_ANY				(1 << 5)
#   define R500_FC_A_OP_NONE				(0 << 6)
#   define R500_FC_A_OP_POP				(1 << 6)
#   define R500_FC_A_OP_PUSH				(2 << 6)
#   define R500_FC_JUMP_FUNC(x)				((x) << 8)
#   define R500_FC_B_POP_CNT(x)				((x) << 16)
#   define R500_FC_B_OP0_NONE				(0 << 24)
#   define R500_FC_B_OP0_DECR				(1 << 24)
#   define R500_FC_B_OP0_INCR				(2 << 24)
#   define R500_FC_B_OP1_NONE				(0 << 26)
#   define R500_FC_B_OP1_DECR				(1 << 26)
#   define R500_FC_B_OP1_INCR				(2 << 26)
#   define R500_FC_IGNORE_UNCOVERED			(1 << 28)
#define R500_US_FC_INT_CONST_0				0x4c00
#   define R500_FC_INT_CONST_KR(x)			((x) << 0)
#   define R500_FC_INT_CONST_KG(x)			((x) << 8)
#   define R500_FC_INT_CONST_KB(x)			((x) << 16)
/* _0 through _15 */
#define R500_US_FORMAT0_0				0x4640
#   define R500_FORMAT_TXWIDTH(x)			((x) << 0)
#   define R500_FORMAT_TXHEIGHT(x)			((x) << 11)
#   define R500_FORMAT_TXDEPTH(x)			((x) << 22)
#define R500_US_PIXSIZE					0x4604
#   define R500_PIX_SIZE(x)				(x)
#define R500_US_TEX_ADDR_0				0x9800
#   define R500_TEX_SRC_ADDR(x)				((x) << 0)
#   define R500_TEX_SRC_ADDR_REL			(1 << 7)
#   define R500_TEX_SRC_S_SWIZ_R			(0 << 8)
#   define R500_TEX_SRC_S_SWIZ_G			(1 << 8)
#   define R500_TEX_SRC_S_SWIZ_B			(2 << 8)
#   define R500_TEX_SRC_S_SWIZ_A			(3 << 8)
#   define R500_TEX_SRC_T_SWIZ_R			(0 << 10)
#   define R500_TEX_SRC_T_SWIZ_G			(1 << 10)
#   define R500_TEX_SRC_T_SWIZ_B			(2 << 10)
#   define R500_TEX_SRC_T_SWIZ_A			(3 << 10)
#   define R500_TEX_SRC_R_SWIZ_R			(0 << 12)
#   define R500_TEX_SRC_R_SWIZ_G			(1 << 12)
#   define R500_TEX_SRC_R_SWIZ_B			(2 << 12)
#   define R500_TEX_SRC_R_SWIZ_A			(3 << 12)
#   define R500_TEX_SRC_Q_SWIZ_R			(0 << 14)
#   define R500_TEX_SRC_Q_SWIZ_G			(1 << 14)
#   define R500_TEX_SRC_Q_SWIZ_B			(2 << 14)
#   define R500_TEX_SRC_Q_SWIZ_A			(3 << 14)
#   define R500_TEX_DST_ADDR(x)				((x) << 16)
#   define R500_TEX_DST_ADDR_REL			(1 << 23)
#   define R500_TEX_DST_R_SWIZ_R			(0 << 24)
#   define R500_TEX_DST_R_SWIZ_G			(1 << 24)
#   define R500_TEX_DST_R_SWIZ_B			(2 << 24)
#   define R500_TEX_DST_R_SWIZ_A			(3 << 24)
#   define R500_TEX_DST_G_SWIZ_R			(0 << 26)
#   define R500_TEX_DST_G_SWIZ_G			(1 << 26)
#   define R500_TEX_DST_G_SWIZ_B			(2 << 26)
#   define R500_TEX_DST_G_SWIZ_A			(3 << 26)
#   define R500_TEX_DST_B_SWIZ_R			(0 << 28)
#   define R500_TEX_DST_B_SWIZ_G			(1 << 28)
#   define R500_TEX_DST_B_SWIZ_B			(2 << 28)
#   define R500_TEX_DST_B_SWIZ_A			(3 << 28)
#   define R500_TEX_DST_A_SWIZ_R			(0 << 30)
#   define R500_TEX_DST_A_SWIZ_G			(1 << 30)
#   define R500_TEX_DST_A_SWIZ_B			(2 << 30)
#   define R500_TEX_DST_A_SWIZ_A			(3 << 30)
#define R500_US_TEX_ADDR_DXDY_0				0xa000
#   define R500_DX_ADDR(x)				((x) << 0)
#   define R500_DX_ADDR_REL				(1 << 7)
#   define R500_DX_S_SWIZ_R				(0 << 8)
#   define R500_DX_S_SWIZ_G				(1 << 8)
#   define R500_DX_S_SWIZ_B				(2 << 8)
#   define R500_DX_S_SWIZ_A				(3 << 8)
#   define R500_DX_T_SWIZ_R				(0 << 10)
#   define R500_DX_T_SWIZ_G				(1 << 10)
#   define R500_DX_T_SWIZ_B				(2 << 10)
#   define R500_DX_T_SWIZ_A				(3 << 10)
#   define R500_DX_R_SWIZ_R				(0 << 12)
#   define R500_DX_R_SWIZ_G				(1 << 12)
#   define R500_DX_R_SWIZ_B				(2 << 12)
#   define R500_DX_R_SWIZ_A				(3 << 12)
#   define R500_DX_Q_SWIZ_R				(0 << 14)
#   define R500_DX_Q_SWIZ_G				(1 << 14)
#   define R500_DX_Q_SWIZ_B				(2 << 14)
#   define R500_DX_Q_SWIZ_A				(3 << 14)
#   define R500_DY_ADDR(x)				((x) << 16)
#   define R500_DY_ADDR_REL				(1 << 17)
#   define R500_DY_S_SWIZ_R				(0 << 24)
#   define R500_DY_S_SWIZ_G				(1 << 24)
#   define R500_DY_S_SWIZ_B				(2 << 24)
#   define R500_DY_S_SWIZ_A				(3 << 24)
#   define R500_DY_T_SWIZ_R				(0 << 26)
#   define R500_DY_T_SWIZ_G				(1 << 26)
#   define R500_DY_T_SWIZ_B				(2 << 26)
#   define R500_DY_T_SWIZ_A				(3 << 26)
#   define R500_DY_R_SWIZ_R				(0 << 28)
#   define R500_DY_R_SWIZ_G				(1 << 28)
#   define R500_DY_R_SWIZ_B				(2 << 28)
#   define R500_DY_R_SWIZ_A				(3 << 28)
#   define R500_DY_Q_SWIZ_R				(0 << 30)
#   define R500_DY_Q_SWIZ_G				(1 << 30)
#   define R500_DY_Q_SWIZ_B				(2 << 30)
#   define R500_DY_Q_SWIZ_A				(3 << 30)
#define R500_US_TEX_INST_0				0x9000
#   define R500_TEX_ID(x)				((x) << 16)
#   define R500_TEX_INST_NOP				(0 << 22)
#   define R500_TEX_INST_LD				(1 << 22)
#   define R500_TEX_INST_TEXKILL			(2 << 22)
#   define R500_TEX_INST_PROJ				(3 << 22)
#   define R500_TEX_INST_LODBIAS			(4 << 22)
#   define R500_TEX_INST_LOD				(5 << 22)
#   define R500_TEX_INST_DXDY				(6 << 22)
#   define R500_TEX_SEM_ACQUIRE_SHIFT			25
#   define R500_TEX_SEM_ACQUIRE				(1 << R500_TEX_SEM_ACQUIRE_SHIFT)
#   define R500_TEX_IGNORE_UNCOVERED			(1 << 26)
#   define R500_TEX_UNSCALED				(1 << 27)
#define R300_US_W_FMT					0x46b4
#   define R300_W_FMT_W0				(0 << 0)
#   define R300_W_FMT_W24				(1 << 0)
#   define R300_W_FMT_W24FP				(2 << 0)
#   define R300_W_SRC_US				(0 << 2)
#   define R300_W_SRC_RAS				(1 << 2)

/* Draw a primitive from vertex data in arrays loaded via 3D_LOAD_VBPNTR.
 * Two parameter dwords:
 * 0. VAP_VTX_FMT: The first parameter is not written to hardware
 * 1. VAP_VF_CTL: The second parameter is a standard primitive emission dword.
 */
#define R300_PACKET3_3D_DRAW_VBUF           0x00002800

/* Draw a primitive from immediate vertices in this packet
 * Up to 16382 dwords:
 * 0. VAP_VTX_FMT: The first parameter is not written to hardware
 * 1. VAP_VF_CTL: The second parameter is a standard primitive emission dword.
 * 2 to end: Up to 16380 dwords of vertex data.
 */
#define R300_PACKET3_3D_DRAW_IMMD           0x00002900

/* Draw a primitive from vertex data in arrays loaded via 3D_LOAD_VBPNTR and
 * immediate vertices in this packet
 * Up to 16382 dwords:
 * 0. VAP_VTX_FMT: The first parameter is not written to hardware
 * 1. VAP_VF_CTL: The second parameter is a standard primitive emission dword.
 * 2 to end: Up to 16380 dwords of vertex data.
 */
#define R300_PACKET3_3D_DRAW_INDX           0x00002A00


/* Specify the full set of vertex arrays as (address, stride).
 * The first parameter is the number of vertex arrays specified.
 * The rest of the command is a variable length list of blocks, where
 * each block is three dwords long and specifies two arrays.
 * The first dword of a block is split into two words, the lower significant
 * word refers to the first array, the more significant word to the second
 * array in the block.
 * The low byte of each word contains the size of an array entry in dwords,
 * the high byte contains the stride of the array.
 * The second dword of a block contains the pointer to the first array,
 * the third dword of a block contains the pointer to the second array.
 * Note that if the total number of arrays is odd, the third dword of
 * the last block is omitted.
 */
#define R300_PACKET3_3D_LOAD_VBPNTR         0x00002F00
#   define R300_VC_FORCE_PREFETCH  (1 << 5)
#   define R300_VBPNTR_SIZE0(x)    ((x) >> 2)
#   define R300_VBPNTR_STRIDE0(x)  (((x) >> 2) << 8)
#   define R300_VBPNTR_SIZE1(x)    (((x) >> 2) << 16)
#   define R300_VBPNTR_STRIDE1(x)  (((x) >> 2) << 24)

#define R300_PACKET3_3D_CLEAR_ZMASK         0x00003200
#define R300_PACKET3_INDX_BUFFER            0x00003300
#    define R300_INDX_BUFFER_DST_SHIFT          0
#    define R300_INDX_BUFFER_SKIP_SHIFT         16
#    define R300_INDX_BUFFER_ONE_REG_WR		(1U << 31)

/* Same as R300_PACKET3_3D_DRAW_VBUF but without VAP_VTX_FMT */
#define R300_PACKET3_3D_DRAW_VBUF_2         0x00003400
/* Same as R300_PACKET3_3D_DRAW_IMMD but without VAP_VTX_FMT */
#define R300_PACKET3_3D_DRAW_IMMD_2         0x00003500
/* Same as R300_PACKET3_3D_DRAW_INDX but without VAP_VTX_FMT */
#define R300_PACKET3_3D_DRAW_INDX_2         0x00003600

/* Clears a portion of hierarchical Z RAM
 * 3 dword parameters
 * 0. START
 * 1. COUNT: 13:0 (max is 0x3FFF)
 * 2. CLEAR_VALUE: Value to write into HIZ RAM.
 */
#define R300_PACKET3_3D_CLEAR_HIZ           0x00003700
#define R300_PACKET3_3D_CLEAR_CMASK         0x00003800

/* Draws a set of primitives using vertex buffers pointed by the state data.
 * At least 2 Parameters:
 * 0. VAP_VF_CNTL: The first parameter is a standard primitive emission dword.
 * 2 to end: Data or indices (see other 3D_DRAW_* packets for details)
 */
#define R300_PACKET3_3D_DRAW_128            0x00003900

/* END: Packet 3 commands */


/* Color formats for 2d packets
 */
#define R300_CP_COLOR_FORMAT_CI8	2
#define R300_CP_COLOR_FORMAT_ARGB1555	3
#define R300_CP_COLOR_FORMAT_RGB565	4
#define R300_CP_COLOR_FORMAT_ARGB8888	6
#define R300_CP_COLOR_FORMAT_RGB332	7
#define R300_CP_COLOR_FORMAT_RGB8	9
#define R300_CP_COLOR_FORMAT_ARGB4444	15

/*
 * CP type-3 packets
 */
#define RADEON_WAIT_UNTIL                   0x1720
#       define RADEON_WAIT_CRTC_PFLIP       (1 << 0)
#       define RADEON_WAIT_2D_IDLECLEAN     (1 << 16)
#       define RADEON_WAIT_3D_IDLECLEAN     (1 << 17)
#       define RADEON_WAIT_HOST_IDLECLEAN   (1 << 18)

#define RADEON_CP_PACKET0                           0x00000000
#define RADEON_CP_PACKET3                           0xC0000000

#define RADEON_ONE_REG_WR        (1 << 15)

#define CP_PACKET0(register, count) \
    (RADEON_CP_PACKET0 | ((count) << 16) | ((register) >> 2))

#define CP_PACKET3(op, count) \
    (RADEON_CP_PACKET3 | (op) | ((count) << 16))

#endif /* _R300_REG_H */

/* *INDENT-ON* */

/* vim: set foldenable foldmarker=\\{,\\} foldmethod=marker : */
