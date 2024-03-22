#ifndef __NV30_SHADER_H__
#define __NV30_SHADER_H__

/* Vertex programs instruction set
 *
 * 128bit opcodes, split into 4 32-bit ones for ease of use.
 *
 * Non-native instructions
 *   ABS - MOV + NV40_VP_INST0_DEST_ABS
 *   POW - EX2 + MUL + LG2
 *   SUB - ADD, second source negated
 *   SWZ - MOV
 *
 * Register access
 *   - Only one INPUT can be accessed per-instruction (move extras into TEMPs)
 *   - Only one CONST can be accessed per-instruction (move extras into TEMPs)
 *
 * Relative Addressing
 *   According to the value returned for
 *   MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB
 *
 *   there are only two address registers available.  The destination in the
 *   ARL instruction is set to TEMP <n> (The temp isn't actually written).
 *
 *   When using vanilla ARB_v_p, the proprietary driver will squish both the
 *   available ADDRESS regs into the first hardware reg in the X and Y
 *   components.
 *
 *   To use an address reg as an index into consts, the CONST_SRC is set to
 *   (const_base + offset) and INDEX_CONST is set.
 *
 *   To access the second address reg use ADDR_REG_SELECT_1. A particular
 *   component of the address regs is selected with ADDR_SWZ.
 *
 *   Only one address register can be accessed per instruction.
 *
 * Conditional execution (see NV_vertex_program{2,3} for details) Conditional
 * execution of an instruction is enabled by setting COND_TEST_ENABLE, and
 * selecting the condition which will allow the test to pass with
 * COND_{FL,LT,...}.  It is possible to swizzle the values in the condition
 * register, which allows for testing against an individual component.
 *
 * Branching:
 *
 *   The BRA/CAL instructions seem to follow a slightly different opcode
 *   layout.  The destination instruction ID (IADDR) overlaps a source field.
 *   Instruction ID's seem to be numbered based on the UPLOAD_FROM_ID FIFO
 *   command, and is incremented automatically on each UPLOAD_INST FIFO
 *   command.
 *
 *   Conditional branching is achieved by using the condition tests described
 *   above.  There doesn't appear to be dedicated looping instructions, but
 *   this can be done using a temp reg + conditional branching.
 *
 *   Subroutines may be uploaded before the main program itself, but the first
 *   executed instruction is determined by the PROGRAM_START_ID FIFO command.
 *
 */

/* DWORD 0 */

/* guess that this is the same as nv40 */
#define NV30_VP_INST_INDEX_INPUT                                        (1 << 27)

#define NV30_VP_INST_ADDR_REG_SELECT_1        (1 << 24)
#define NV30_VP_INST_SRC2_ABS           (1 << 23) /* guess */
#define NV30_VP_INST_SRC1_ABS           (1 << 22) /* guess */
#define NV30_VP_INST_SRC0_ABS           (1 << 21) /* guess */
#define NV30_VP_INST_VEC_RESULT         (1 << 20)
#define NV30_VP_INST_DEST_TEMP_ID_SHIFT        16
#define NV30_VP_INST_DEST_TEMP_ID_MASK        (0x0F << 16)
#define NV30_VP_INST_COND_UPDATE_ENABLE        (1<<15)
#define NV30_VP_INST_VEC_DEST_TEMP_MASK      (0x1F << 16)
#define NV30_VP_INST_COND_TEST_ENABLE        (1<<14)
#define NV30_VP_INST_COND_SHIFT          11
#define NV30_VP_INST_COND_MASK          (0x07 << 11)
#define NV30_VP_INST_COND_SWZ_X_SHIFT        9
#define NV30_VP_INST_COND_SWZ_X_MASK        (0x03 <<  9)
#define NV30_VP_INST_COND_SWZ_Y_SHIFT        7
#define NV30_VP_INST_COND_SWZ_Y_MASK        (0x03 <<  7)
#define NV30_VP_INST_COND_SWZ_Z_SHIFT        5
#define NV30_VP_INST_COND_SWZ_Z_MASK        (0x03 <<  5)
#define NV30_VP_INST_COND_SWZ_W_SHIFT        3
#define NV30_VP_INST_COND_SWZ_W_MASK        (0x03 <<  3)
#define NV30_VP_INST_COND_SWZ_ALL_SHIFT        3
#define NV30_VP_INST_COND_SWZ_ALL_MASK        (0xFF <<  3)
#define NV30_VP_INST_ADDR_SWZ_SHIFT        1
#define NV30_VP_INST_ADDR_SWZ_MASK        (0x03 <<  1)
#define NV30_VP_INST_SCA_OPCODEH_SHIFT        0
#define NV30_VP_INST_SCA_OPCODEH_MASK        (0x01 <<  0)

/* DWORD 1 */
#define NV30_VP_INST_SCA_OPCODEL_SHIFT        28
#define NV30_VP_INST_SCA_OPCODEL_MASK        (0x0F << 28)
#define NV30_VP_INST_VEC_OPCODE_SHIFT        23
#define NV30_VP_INST_VEC_OPCODE_MASK        (0x1F << 23)
#define NV30_VP_INST_CONST_SRC_SHIFT        14
#define NV30_VP_INST_CONST_SRC_MASK        (0xFF << 14)
#define NV30_VP_INST_INPUT_SRC_SHIFT        9    /*NV20*/
#define NV30_VP_INST_INPUT_SRC_MASK        (0x0F <<  9)  /*NV20*/
#define NV30_VP_INST_SRC0H_SHIFT        0    /*NV20*/
#define NV30_VP_INST_SRC0H_MASK          (0x1FF << 0)  /*NV20*/

/* Please note: the IADDR fields overlap other fields because they are used
 * only for branch instructions.  See Branching: label above
 *
 * DWORD 2
 */
#define NV30_VP_INST_SRC0L_SHIFT        26    /*NV20*/
#define NV30_VP_INST_SRC0L_MASK         (0x3F  <<26)  /* NV30_VP_SRC0_LOW_MASK << 26 */
#define NV30_VP_INST_SRC1_SHIFT         11    /*NV20*/
#define NV30_VP_INST_SRC1_MASK          (0x7FFF<<11)  /*NV20*/
#define NV30_VP_INST_SRC2H_SHIFT        0    /*NV20*/
#define NV30_VP_INST_SRC2H_MASK          (0x7FF << 0)  /* NV30_VP_SRC2_HIGH_MASK >> 4*/
#define NV30_VP_INST_IADDR_SHIFT        2
#define NV30_VP_INST_IADDR_MASK          (0x1FF <<  2)   /* NV30_VP_SRC2_LOW_MASK << 28 */

/* DWORD 3 */
#define NV30_VP_INST_SRC2L_SHIFT        28    /*NV20*/
#define NV30_VP_INST_SRC2L_MASK          (0x0F  <<28)  /*NV20*/
#define NV30_VP_INST_STEMP_WRITEMASK_SHIFT      24
#define NV30_VP_INST_STEMP_WRITEMASK_MASK      (0x0F << 24)
#define NV30_VP_INST_VTEMP_WRITEMASK_SHIFT      20
#define NV30_VP_INST_VTEMP_WRITEMASK_MASK      (0x0F << 20)
#define NV30_VP_INST_SDEST_WRITEMASK_SHIFT      16
#define NV30_VP_INST_SDEST_WRITEMASK_MASK      (0x0F << 16)
#define NV30_VP_INST_VDEST_WRITEMASK_SHIFT      12    /*NV20*/
#define NV30_VP_INST_VDEST_WRITEMASK_MASK      (0x0F << 12)  /*NV20*/
#define NV30_VP_INST_DEST_SHIFT        2
#define NV30_VP_INST_DEST_MASK        (0x1F <<  2)
#  define NV30_VP_INST_DEST_POS  0
#  define NV30_VP_INST_DEST_BFC0  1
#  define NV30_VP_INST_DEST_BFC1  2
#  define NV30_VP_INST_DEST_COL0  3
#  define NV30_VP_INST_DEST_COL1  4
#  define NV30_VP_INST_DEST_FOGC  5
#  define NV30_VP_INST_DEST_PSZ   6
#  define NV30_VP_INST_DEST_TC(n)  (8+(n))
#  define NV30_VP_INST_DEST_CLP(n) (17 + (n))

/* guess that this is the same as nv40 */
#define NV30_VP_INST_INDEX_CONST                                        (1 << 1)

/* Useful to split the source selection regs into their pieces */
#define NV30_VP_SRC0_HIGH_SHIFT                                                6
#define NV30_VP_SRC0_HIGH_MASK                                        0x00007FC0
#define NV30_VP_SRC0_LOW_MASK                                         0x0000003F
#define NV30_VP_SRC2_HIGH_SHIFT                                                4
#define NV30_VP_SRC2_HIGH_MASK                                        0x00007FF0
#define NV30_VP_SRC2_LOW_MASK                                         0x0000000F


/* Source-register definition - matches NV20 exactly */
#define NV30_VP_SRC_NEGATE          (1<<14)
#define NV30_VP_SRC_SWZ_X_SHIFT        12
#define NV30_VP_SRC_REG_SWZ_X_MASK        (0x03  <<12)
#define NV30_VP_SRC_SWZ_Y_SHIFT        10
#define NV30_VP_SRC_REG_SWZ_Y_MASK        (0x03  <<10)
#define NV30_VP_SRC_SWZ_Z_SHIFT        8
#define NV30_VP_SRC_REG_SWZ_Z_MASK        (0x03  << 8)
#define NV30_VP_SRC_SWZ_W_SHIFT        6
#define NV30_VP_SRC_REG_SWZ_W_MASK        (0x03  << 6)
#define NV30_VP_SRC_REG_SWZ_ALL_SHIFT        6
#define NV30_VP_SRC_REG_SWZ_ALL_MASK        (0xFF  << 6)
#define NV30_VP_SRC_TEMP_SRC_SHIFT        2
#define NV30_VP_SRC_REG_TEMP_ID_MASK        (0x0F  << 0)
#define NV30_VP_SRC_REG_TYPE_SHIFT        0
#define NV30_VP_SRC_REG_TYPE_MASK        (0x03  << 0)
#define NV30_VP_SRC_REG_TYPE_TEMP  1
#define NV30_VP_SRC_REG_TYPE_INPUT  2
#define NV30_VP_SRC_REG_TYPE_CONST  3 /* guess */

#include "nv30/nvfx_shader.h"

#endif
