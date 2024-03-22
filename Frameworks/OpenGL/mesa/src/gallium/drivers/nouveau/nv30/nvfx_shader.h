#ifndef __NVFX_SHADER_H__
#define __NVFX_SHADER_H__

#include <stdint.h>

#include "util/compiler.h"

#define NVFX_SWZ_IDENTITY ((3 << 6) | (2 << 4) | (1 << 2) | (0 << 0))

/* this will resolve to either the NV30 or the NV40 version
 * depending on the current hardware */
/* unusual, but very fast and compact method */
#define NVFX_VP(c) ((NV30_VP_##c) + (vpc->is_nv4x & ((NV40_VP_##c) - (NV30_VP_##c))))

#define NVFX_VP_INST_SLOT_VEC 0
#define NVFX_VP_INST_SLOT_SCA 1

#define NVFX_VP_INST_IN_POS  0    /* These seem to match the bindings specified in */
#define NVFX_VP_INST_IN_WEIGHT  1    /* the ARB_v_p spec (2.14.3.1) */
#define NVFX_VP_INST_IN_NORMAL  2
#define NVFX_VP_INST_IN_COL0  3    /* Should probably confirm them all though */
#define NVFX_VP_INST_IN_COL1  4
#define NVFX_VP_INST_IN_FOGC  5
#define NVFX_VP_INST_IN_TC0  8
#define NVFX_VP_INST_IN_TC(n)  (8+n)

#define NVFX_VP_INST_SCA_OP_NOP 0x00
#define NVFX_VP_INST_SCA_OP_MOV 0x01
#define NVFX_VP_INST_SCA_OP_RCP 0x02
#define NVFX_VP_INST_SCA_OP_RCC 0x03
#define NVFX_VP_INST_SCA_OP_RSQ 0x04
#define NVFX_VP_INST_SCA_OP_EXP 0x05
#define NVFX_VP_INST_SCA_OP_LOG 0x06
#define NVFX_VP_INST_SCA_OP_LIT 0x07
#define NVFX_VP_INST_SCA_OP_BRA 0x09
#define NVFX_VP_INST_SCA_OP_CAL 0x0B
#define NVFX_VP_INST_SCA_OP_RET 0x0C
#define NVFX_VP_INST_SCA_OP_LG2 0x0D
#define NVFX_VP_INST_SCA_OP_EX2 0x0E
#define NVFX_VP_INST_SCA_OP_SIN 0x0F
#define NVFX_VP_INST_SCA_OP_COS 0x10

#define NV40_VP_INST_SCA_OP_PUSHA 0x13
#define NV40_VP_INST_SCA_OP_POPA 0x14

#define NVFX_VP_INST_VEC_OP_NOP 0x00
#define NVFX_VP_INST_VEC_OP_MOV 0x01
#define NVFX_VP_INST_VEC_OP_MUL 0x02
#define NVFX_VP_INST_VEC_OP_ADD 0x03
#define NVFX_VP_INST_VEC_OP_MAD 0x04
#define NVFX_VP_INST_VEC_OP_DP3 0x05
#define NVFX_VP_INST_VEC_OP_DPH 0x06
#define NVFX_VP_INST_VEC_OP_DP4 0x07
#define NVFX_VP_INST_VEC_OP_DST 0x08
#define NVFX_VP_INST_VEC_OP_MIN 0x09
#define NVFX_VP_INST_VEC_OP_MAX 0x0A
#define NVFX_VP_INST_VEC_OP_SLT 0x0B
#define NVFX_VP_INST_VEC_OP_SGE 0x0C
#define NVFX_VP_INST_VEC_OP_ARL 0x0D
#define NVFX_VP_INST_VEC_OP_FRC 0x0E
#define NVFX_VP_INST_VEC_OP_FLR 0x0F
#define NVFX_VP_INST_VEC_OP_SEQ 0x10
#define NVFX_VP_INST_VEC_OP_SFL 0x11
#define NVFX_VP_INST_VEC_OP_SGT 0x12
#define NVFX_VP_INST_VEC_OP_SLE 0x13
#define NVFX_VP_INST_VEC_OP_SNE 0x14
#define NVFX_VP_INST_VEC_OP_STR 0x15
#define NVFX_VP_INST_VEC_OP_SSG 0x16
#define NVFX_VP_INST_VEC_OP_ARR 0x17
#define NVFX_VP_INST_VEC_OP_ARA 0x18

#define NV40_VP_INST_VEC_OP_TXL 0x19

/* DWORD 3 */
#define NVFX_VP_INST_LAST                           (1 << 0)

/*
 * Each fragment program opcode appears to be comprised of 4 32-bit values.
 *
 * 0: OPDEST
 * 	0: program end
 * 	1-6: destination register
 * 	7: destination register is fp16?? (use for outputs)
 * 	8: set condition code
 * 	9: writemask x
 *  	10: writemask y
 *  	11: writemask z
 *  	12: writemask w
 *  	13-16: source attribute register number (e.g. COL0)
 *  	17-20: texture unit number
 *  	21: expand value on texture operation (x -> 2x - 1)
 *  	22-23: precision 0 = fp32, 1 = fp16, 2 = s1.10 fixed, 3 = s0.8 fixed (nv40-only))
 * 	24-29: opcode
 * 	30: no destination
 * 	31: saturate
 * 1 - SRC0
 * 	0-17: see common source fields
 * 	18: execute if condition code less
 * 	19: execute if condition code equal
 * 	20: execute if condition code greater
 * 	21-22: condition code swizzle x source component
 * 	23-24: condition code swizzle y source component
 * 	25-26: condition code swizzle z source component
 * 	27-28: condition code swizzle w source component
 * 	29: source 0 absolute
 * 	30: always 0 in renouveau tests
 * 	31: always 0 in renouveau tests
 * 2 - SRC1
 * 	0-17: see common source fields
 * 	18: source 1 absolute
 * 	19-20: input precision 0 = fp32, 1 = fp16, 2 = s1.10 fixed, 3 = ???
 * 	21-27: always 0 in renouveau tests
 * 	28-30: scale (0 = 1x, 1 = 2x, 2 = 4x, 3 = 8x, 4 = ???, 5, = 1/2, 6 = 1/4, 7 = 1/8)
 * 	31: opcode is branch
 * 3 - SRC2
 * 	0-17: see common source fields
 * 	18: source 2 absolute
 * 	19-29: address register displacement
 * 	30: use index register
 * 	31: disable perspective-correct interpolation?
 *
* Common fields of 0, 1, 2 - SRC
 * 	0-1: source register type (0 = temp, 1 = input, 2 = immediate, 3 = ???)
 * 	2-7: source temp register index
 * 	8: source register is fp16??
 * 	9-10: source swizzle x source component
 * 	11-12: source swizzle y source component
 * 	13-14: source swizzle z source component
 * 	15-16: source swizzle w source component
 *	17: negate

 * There appears to be no special difference between result regs and temp regs.
 *     result.color == R0.xyzw
 *     result.depth == R1.z
 * When the fragprog contains instructions to write depth, NV30_TCL_PRIMITIVE_3D_UNK1D78=0
 * otherwise it is set to 1.
 *
 * Constants are inserted directly after the instruction that uses them.
 *
 * It appears that it's not possible to use two input registers in one
 * instruction as the input sourcing is done in the instruction dword
 * and not the source selection dwords.  As such instructions such as:
 *
 *     ADD result.color, fragment.color, fragment.texcoord[0];
 *
 * must be split into two MOV's and then an ADD (nvidia does this) but
 * I'm not sure why it's not just one MOV and then source the second input
 * in the ADD instruction..
 *
 * Negation of the full source is done with NV30_FP_REG_NEGATE, arbitrary
 * negation requires multiplication with a const.
 *
 * Arbitrary swizzling is supported with the exception of SWIZZLE_ZERO/SWIZZLE_ONE
 * The temp/result regs appear to be initialised to (0.0, 0.0, 0.0, 0.0) as SWIZZLE_ZERO
 * is implemented simply by not writing to the relevant components of the destination.
 *
 * Conditional execution
 *   TODO
 *
 * Non-native instructions:
 *   LIT
 *   LRP - MAD+MAD
 *   SUB - ADD, negate second source
 *   RSQ - LG2 + EX2
 *   POW - LG2 + MUL + EX2
 *
 * NV40 Looping
 *   Loops appear to be fairly expensive on NV40 at least, the proprietary
 *   driver goes to a lot of effort to avoid using the native looping
 *   instructions.  If the total number of *executed* instructions between
 *   REP/ENDREP or LOOP/ENDLOOP is <=500, the driver will unroll the loop.
 *   The maximum loop count is 255.
 *
 */

//== Opcode / Destination selection ==
#define NVFX_FP_OP_PROGRAM_END          (1 << 0)
#define NVFX_FP_OP_OUT_REG_SHIFT        1
#define NV30_FP_OP_OUT_REG_MASK          (31 << 1)  /* uncertain */
#define NV40_FP_OP_OUT_REG_MASK          (63 << 1)
/* Needs to be set when writing outputs to get expected result.. */
#define NVFX_FP_OP_OUT_REG_HALF          (1 << 7)
#define NVFX_FP_OP_COND_WRITE_ENABLE        (1 << 8)
#define NVFX_FP_OP_OUTMASK_SHIFT        9
#define NVFX_FP_OP_OUTMASK_MASK          (0xF << 9)
#  define NVFX_FP_OP_OUT_X  (1<<9)
#  define NVFX_FP_OP_OUT_Y  (1<<10)
#  define NVFX_FP_OP_OUT_Z  (1<<11)
#  define NVFX_FP_OP_OUT_W  (1<<12)
/* Uncertain about these, especially the input_src values.. it's possible that
 * they can be dynamically changed.
 */
#define NVFX_FP_OP_INPUT_SRC_SHIFT        13
#define NVFX_FP_OP_INPUT_SRC_MASK        (15 << 13)
#  define NVFX_FP_OP_INPUT_SRC_POSITION  0x0
#  define NVFX_FP_OP_INPUT_SRC_COL0  0x1
#  define NVFX_FP_OP_INPUT_SRC_COL1  0x2
#  define NVFX_FP_OP_INPUT_SRC_FOGC  0x3
#  define NVFX_FP_OP_INPUT_SRC_TC0    0x4
#  define NVFX_FP_OP_INPUT_SRC_TC(n)  (0x4 + n)
#  define NV40_FP_OP_INPUT_SRC_FACING  0xE
#define NVFX_FP_OP_TEX_UNIT_SHIFT        17
#define NVFX_FP_OP_TEX_UNIT_MASK        (0xF << 17) /* guess */
#define NVFX_FP_OP_PRECISION_SHIFT        22
#define NVFX_FP_OP_PRECISION_MASK        (3 << 22)
#   define NVFX_FP_PRECISION_FP32  0
#   define NVFX_FP_PRECISION_FP16  1
#   define NVFX_FP_PRECISION_FX12  2
#define NVFX_FP_OP_OPCODE_SHIFT          24
#define NVFX_FP_OP_OPCODE_MASK          (0x3F << 24)
/* NV30/NV40 fragment program opcodes */
#define NVFX_FP_OP_OPCODE_NOP 0x00
#define NVFX_FP_OP_OPCODE_MOV 0x01
#define NVFX_FP_OP_OPCODE_MUL 0x02
#define NVFX_FP_OP_OPCODE_ADD 0x03
#define NVFX_FP_OP_OPCODE_MAD 0x04
#define NVFX_FP_OP_OPCODE_DP3 0x05
#define NVFX_FP_OP_OPCODE_DP4 0x06
#define NVFX_FP_OP_OPCODE_DST 0x07
#define NVFX_FP_OP_OPCODE_MIN 0x08
#define NVFX_FP_OP_OPCODE_MAX 0x09
#define NVFX_FP_OP_OPCODE_SLT 0x0A
#define NVFX_FP_OP_OPCODE_SGE 0x0B
#define NVFX_FP_OP_OPCODE_SLE 0x0C
#define NVFX_FP_OP_OPCODE_SGT 0x0D
#define NVFX_FP_OP_OPCODE_SNE 0x0E
#define NVFX_FP_OP_OPCODE_SEQ 0x0F
#define NVFX_FP_OP_OPCODE_FRC 0x10
#define NVFX_FP_OP_OPCODE_FLR 0x11
#define NVFX_FP_OP_OPCODE_KIL 0x12
#define NVFX_FP_OP_OPCODE_PK4B 0x13
#define NVFX_FP_OP_OPCODE_UP4B 0x14
#define NVFX_FP_OP_OPCODE_DDX 0x15 /* can only write XY */
#define NVFX_FP_OP_OPCODE_DDY 0x16 /* can only write XY */
#define NVFX_FP_OP_OPCODE_TEX 0x17
#define NVFX_FP_OP_OPCODE_TXP 0x18
#define NVFX_FP_OP_OPCODE_TXD 0x19
#define NVFX_FP_OP_OPCODE_RCP 0x1A
#define NVFX_FP_OP_OPCODE_EX2 0x1C
#define NVFX_FP_OP_OPCODE_LG2 0x1D
#define NVFX_FP_OP_OPCODE_STR 0x20
#define NVFX_FP_OP_OPCODE_SFL 0x21
#define NVFX_FP_OP_OPCODE_COS 0x22
#define NVFX_FP_OP_OPCODE_SIN 0x23
#define NVFX_FP_OP_OPCODE_PK2H 0x24
#define NVFX_FP_OP_OPCODE_UP2H 0x25
#define NVFX_FP_OP_OPCODE_PK4UB 0x27
#define NVFX_FP_OP_OPCODE_UP4UB 0x28
#define NVFX_FP_OP_OPCODE_PK2US 0x29
#define NVFX_FP_OP_OPCODE_UP2US 0x2A
#define NVFX_FP_OP_OPCODE_DP2A 0x2E
#define NVFX_FP_OP_OPCODE_TXB 0x31
#define NVFX_FP_OP_OPCODE_DIV 0x3A

/* NV30 only fragment program opcodes */
#define NVFX_FP_OP_OPCODE_RSQ_NV30 0x1B
#define NVFX_FP_OP_OPCODE_LIT_NV30 0x1E
#define NVFX_FP_OP_OPCODE_LRP_NV30 0x1F
#define NVFX_FP_OP_OPCODE_POW_NV30 0x26
#define NVFX_FP_OP_OPCODE_RFL_NV30 0x36

/* NV40 only fragment program opcodes */
#define NVFX_FP_OP_OPCODE_TXL_NV40 0x2F
#define NVFX_FP_OP_OPCODE_LITEX2_NV40 0x3C

/* The use of these instructions appears to be indicated by bit 31 of DWORD 2.*/
#define NV40_FP_OP_BRA_OPCODE_BRK                                    0x0
#define NV40_FP_OP_BRA_OPCODE_CAL                                    0x1
#define NV40_FP_OP_BRA_OPCODE_IF                                     0x2
#define NV40_FP_OP_BRA_OPCODE_LOOP                                   0x3
#define NV40_FP_OP_BRA_OPCODE_REP                                    0x4
#define NV40_FP_OP_BRA_OPCODE_RET                                    0x5

#define NV40_FP_OP_OUT_NONE         (1 << 30)
#define NVFX_FP_OP_OUT_SAT          (1 << 31)

/* high order bits of SRC0 */
#define NVFX_FP_OP_SRC0_ABS          (1 << 29)
#define NVFX_FP_OP_COND_SWZ_W_SHIFT        27
#define NVFX_FP_OP_COND_SWZ_W_MASK        (3 << 27)
#define NVFX_FP_OP_COND_SWZ_Z_SHIFT        25
#define NVFX_FP_OP_COND_SWZ_Z_MASK        (3 << 25)
#define NVFX_FP_OP_COND_SWZ_Y_SHIFT        23
#define NVFX_FP_OP_COND_SWZ_Y_MASK        (3 << 23)
#define NVFX_FP_OP_COND_SWZ_X_SHIFT        21
#define NVFX_FP_OP_COND_SWZ_X_MASK        (3 << 21)
#define NVFX_FP_OP_COND_SWZ_ALL_SHIFT        21
#define NVFX_FP_OP_COND_SWZ_ALL_MASK        (0xFF << 21)
#define NVFX_FP_OP_COND_SHIFT          18
#define NVFX_FP_OP_COND_MASK          (0x07 << 18)
#  define NVFX_FP_OP_COND_FL  0
#  define NVFX_FP_OP_COND_LT  1
#  define NVFX_FP_OP_COND_EQ  2
#  define NVFX_FP_OP_COND_LE  3
#  define NVFX_FP_OP_COND_GT  4
#  define NVFX_FP_OP_COND_NE  5
#  define NVFX_FP_OP_COND_GE  6
#  define NVFX_FP_OP_COND_TR  7

/* high order bits of SRC1 */
#define NV40_FP_OP_OPCODE_IS_BRANCH                                      (1<<31)
#define NVFX_FP_OP_DST_SCALE_SHIFT        28
#define NVFX_FP_OP_DST_SCALE_MASK        (3 << 28)
#define NVFX_FP_OP_DST_SCALE_1X                                                0
#define NVFX_FP_OP_DST_SCALE_2X                                                1
#define NVFX_FP_OP_DST_SCALE_4X                                                2
#define NVFX_FP_OP_DST_SCALE_8X                                                3
#define NVFX_FP_OP_DST_SCALE_INV_2X                                            5
#define NVFX_FP_OP_DST_SCALE_INV_4X                                            6
#define NVFX_FP_OP_DST_SCALE_INV_8X                                            7
#define NVFX_FP_OP_SRC1_ABS          (1 << 18)

/* SRC1 LOOP */
#define NV40_FP_OP_LOOP_INCR_SHIFT                                            19
#define NV40_FP_OP_LOOP_INCR_MASK                                   (0xFF << 19)
#define NV40_FP_OP_LOOP_INDEX_SHIFT                                           10
#define NV40_FP_OP_LOOP_INDEX_MASK                                  (0xFF << 10)
#define NV40_FP_OP_LOOP_COUNT_SHIFT                                            2
#define NV40_FP_OP_LOOP_COUNT_MASK                                   (0xFF << 2)

/* SRC1 IF: absolute offset in dwords */
#define NV40_FP_OP_ELSE_OFFSET_SHIFT                                           0
#define NV40_FP_OP_ELSE_OFFSET_MASK                             (0x7FFFFFFF << 0)

/* SRC1 CAL */
#define NV40_FP_OP_SUB_OFFSET_SHIFT                                                 0
#define NV40_FP_OP_SUB_OFFSET_MASK                                   (0x7FFFFFFF << 0)

/* SRC1 REP
 *   I have no idea why there are 3 count values here..  but they
 *   have always been filled with the same value in my tests so
 *   far..
 */
#define NV40_FP_OP_REP_COUNT1_SHIFT                                            2
#define NV40_FP_OP_REP_COUNT1_MASK                                   (0xFF << 2)
#define NV40_FP_OP_REP_COUNT2_SHIFT                                           10
#define NV40_FP_OP_REP_COUNT2_MASK                                  (0xFF << 10)
#define NV40_FP_OP_REP_COUNT3_SHIFT                                           19
#define NV40_FP_OP_REP_COUNT3_MASK                                  (0xFF << 19)

/* SRC2 REP/IF: absolute offset in dwords */
#define NV40_FP_OP_END_OFFSET_SHIFT                                            0
#define NV40_FP_OP_END_OFFSET_MASK                              (0x7FFFFFFF << 0)

/* high order bits of SRC2 */
#define NVFX_FP_OP_INDEX_INPUT          (1 << 30)
#define NV40_FP_OP_ADDR_INDEX_SHIFT        19
#define NV40_FP_OP_ADDR_INDEX_MASK        (0xF << 19)

//== Register selection ==
#define NVFX_FP_REG_TYPE_SHIFT           0
#define NVFX_FP_REG_TYPE_MASK           (3 << 0)
#  define NVFX_FP_REG_TYPE_TEMP   0
#  define NVFX_FP_REG_TYPE_INPUT  1
#  define NVFX_FP_REG_TYPE_CONST  2
#define NVFX_FP_REG_SRC_SHIFT            2
#define NV30_FP_REG_SRC_MASK              (31 << 2)
#define NV40_FP_REG_SRC_MASK              (63 << 2)
#define NVFX_FP_REG_SRC_HALF            (1 << 8)
#define NVFX_FP_REG_SWZ_ALL_SHIFT        9
#define NVFX_FP_REG_SWZ_ALL_MASK        (255 << 9)
#define NVFX_FP_REG_SWZ_X_SHIFT          9
#define NVFX_FP_REG_SWZ_X_MASK          (3 << 9)
#define NVFX_FP_REG_SWZ_Y_SHIFT          11
#define NVFX_FP_REG_SWZ_Y_MASK          (3 << 11)
#define NVFX_FP_REG_SWZ_Z_SHIFT          13
#define NVFX_FP_REG_SWZ_Z_MASK          (3 << 13)
#define NVFX_FP_REG_SWZ_W_SHIFT          15
#define NVFX_FP_REG_SWZ_W_MASK          (3 << 15)
#  define NVFX_FP_SWIZZLE_X  0
#  define NVFX_FP_SWIZZLE_Y  1
#  define NVFX_FP_SWIZZLE_Z  2
#  define NVFX_FP_SWIZZLE_W  3
#define NVFX_FP_REG_NEGATE          (1 << 17)

#define NVFXSR_NONE	0
#define NVFXSR_OUTPUT	1
#define NVFXSR_INPUT	2
#define NVFXSR_TEMP	3
#define NVFXSR_CONST	5
#define NVFXSR_IMM	6

#define NVFX_COND_FL  0
#define NVFX_COND_LT  1
#define NVFX_COND_EQ  2
#define NVFX_COND_LE  3
#define NVFX_COND_GT  4
#define NVFX_COND_NE  5
#define NVFX_COND_GE  6
#define NVFX_COND_TR  7

/* Yes, this are ordered differently... */

#define NVFX_VP_MASK_X 8
#define NVFX_VP_MASK_Y 4
#define NVFX_VP_MASK_Z 2
#define NVFX_VP_MASK_W 1
#define NVFX_VP_MASK_ALL 0xf

#define NVFX_FP_MASK_X 1
#define NVFX_FP_MASK_Y 2
#define NVFX_FP_MASK_Z 4
#define NVFX_FP_MASK_W 8
#define NVFX_FP_MASK_ALL 0xf

#define NVFX_SWZ_X 0
#define NVFX_SWZ_Y 1
#define NVFX_SWZ_Z 2
#define NVFX_SWZ_W 3

#define swz(s,x,y,z,w) nvfx_src_swz((s), NVFX_SWZ_##x, NVFX_SWZ_##y, NVFX_SWZ_##z, NVFX_SWZ_##w)
#define neg(s) nvfx_src_neg((s))
#define abs(s) nvfx_src_abs((s))

struct nvfx_reg {
	int8_t type;
	int32_t index;
};

struct nvfx_src {
	struct nvfx_reg reg;

	uint8_t indirect : 1;
	uint8_t indirect_reg : 1;
	uint8_t indirect_swz : 2;
	uint8_t negate : 1;
	uint8_t abs : 1;
	uint8_t swz[4];
};

struct nvfx_insn
{
	uint8_t op;
	char scale;
	int8_t unit;
	uint8_t mask;
	uint8_t cc_swz[4];

	uint8_t sat : 1;
	uint8_t cc_update : 1;
	uint8_t cc_update_reg : 1;
	uint8_t cc_test : 3;
	uint8_t cc_test_reg : 1;

	struct nvfx_reg dst;
	struct nvfx_src src[3];
};

static inline struct nvfx_insn
nvfx_insn(bool sat, unsigned op, int unit, struct nvfx_reg dst, unsigned mask, struct nvfx_src s0, struct nvfx_src s1, struct nvfx_src s2)
{
	struct nvfx_insn insn = {
		.op = op,
		.scale = 0,
		.unit = unit,
		.sat = sat,
		.mask = mask,
		.cc_update = 0,
		.cc_update_reg = 0,
		.cc_test = NVFX_COND_TR,
		.cc_test_reg = 0,
		.cc_swz = { 0, 1, 2, 3 },
		.dst = dst,
		.src = {s0, s1, s2}
	};
	return insn;
}

static inline struct nvfx_reg
nvfx_reg(int type, int index)
{
	struct nvfx_reg temp = {
		.type = type,
		.index = index,
	};
	return temp;
}

static inline struct nvfx_src
nvfx_src(struct nvfx_reg reg)
{
	struct nvfx_src temp = {
		.reg = reg,
		.abs = 0,
		.negate = 0,
		.swz = { 0, 1, 2, 3 },
		.indirect = 0,
	};
	return temp;
}

static inline struct nvfx_src
nvfx_src_swz(struct nvfx_src src, int x, int y, int z, int w)
{
	struct nvfx_src dst = src;

	dst.swz[NVFX_SWZ_X] = src.swz[x];
	dst.swz[NVFX_SWZ_Y] = src.swz[y];
	dst.swz[NVFX_SWZ_Z] = src.swz[z];
	dst.swz[NVFX_SWZ_W] = src.swz[w];
	return dst;
}

static inline struct nvfx_src
nvfx_src_neg(struct nvfx_src src)
{
	src.negate = !src.negate;
	return src;
}

static inline struct nvfx_src
nvfx_src_abs(struct nvfx_src src)
{
	src.abs = 1;
	return src;
}

struct nvfx_relocation {
        unsigned location;
        unsigned target;
};

struct nv30_fragprog;
struct nv30_vertprog;

//XXX: needed to make it build, clean this up!
void
_nvfx_fragprog_translate(uint16_t oclass, struct nv30_fragprog *fp);

bool
_nvfx_vertprog_translate(uint16_t oclass, struct nv30_vertprog *vp);

#endif
