#ifndef __NV40_SHADER_H__
#define __NV40_SHADER_H__

/* Vertex programs instruction set
 *
 * The NV40 instruction set is very similar to NV30.  Most fields are in
 * a slightly different position in the instruction however.
 *
 * Merged instructions
 *     In some cases it is possible to put two instructions into one opcode
 *     slot.  The rules for when this is OK is not entirely clear to me yet.
 *
 *     There are separate writemasks and dest temp register fields for each
 *     grouping of instructions.  There is however only one field with the
 *     ID of a result register.  Writing to temp/result regs is selected by
 *     setting VEC_RESULT/SCA_RESULT.
 *
 * Temporary registers
 *     The source/dest temp register fields have been extended by 1 bit, to
 *     give a total of 32 temporary registers.
 *
 * Relative Addressing
 *     NV40 can use an address register to index into vertex attribute regs.
 *     This is done by putting the offset value into INPUT_SRC and setting
 *     the INDEX_INPUT flag.
 *
 * Conditional execution (see NV_vertex_program{2,3} for details)
 *     There is a second condition code register on NV40, it's use is enabled
 *     by setting the COND_REG_SELECT_1 flag.
 *
 * Texture lookup
 *     TODO
 */

/* ---- OPCODE BITS 127:96 / data DWORD 0 --- */
#define NV40_VP_INST_VEC_RESULT                                        (1 << 30)
/* uncertain.. */
#define NV40_VP_INST_COND_UPDATE_ENABLE                        ((1 << 14)|1<<29)
/* use address reg as index into attribs */
#define NV40_VP_INST_INDEX_INPUT                                       (1 << 27)
#define NV40_VP_INST_SATURATE                                          (1 << 26)
#define NV40_VP_INST_COND_REG_SELECT_1                                 (1 << 25)
#define NV40_VP_INST_ADDR_REG_SELECT_1                                 (1 << 24)
#define NV40_VP_INST_SRC2_ABS                                          (1 << 23)
#define NV40_VP_INST_SRC1_ABS                                          (1 << 22)
#define NV40_VP_INST_SRC0_ABS                                          (1 << 21)
#define NV40_VP_INST_VEC_DEST_TEMP_SHIFT                                      15
#define NV40_VP_INST_VEC_DEST_TEMP_MASK                             (0x3F << 15)
#define NV40_VP_INST_COND_TEST_ENABLE                                  (1 << 13)
#define NV40_VP_INST_COND_SHIFT                                               10
#define NV40_VP_INST_COND_MASK                                       (0x7 << 10)
#define NV40_VP_INST_COND_SWZ_X_SHIFT                                          8
#define NV40_VP_INST_COND_SWZ_X_MASK                                    (3 << 8)
#define NV40_VP_INST_COND_SWZ_Y_SHIFT                                          6
#define NV40_VP_INST_COND_SWZ_Y_MASK                                    (3 << 6)
#define NV40_VP_INST_COND_SWZ_Z_SHIFT                                          4
#define NV40_VP_INST_COND_SWZ_Z_MASK                                    (3 << 4)
#define NV40_VP_INST_COND_SWZ_W_SHIFT                                          2
#define NV40_VP_INST_COND_SWZ_W_MASK                                    (3 << 2)
#define NV40_VP_INST_COND_SWZ_ALL_SHIFT                                        2
#define NV40_VP_INST_COND_SWZ_ALL_MASK                               (0xFF << 2)
#define NV40_VP_INST_ADDR_SWZ_SHIFT                                            0
#define NV40_VP_INST_ADDR_SWZ_MASK                                   (0x03 << 0)
#define NV40_VP_INST0_KNOWN ( \
                NV40_VP_INST_INDEX_INPUT | \
                NV40_VP_INST_COND_REG_SELECT_1 | \
                NV40_VP_INST_ADDR_REG_SELECT_1 | \
                NV40_VP_INST_SRC2_ABS | \
                NV40_VP_INST_SRC1_ABS | \
                NV40_VP_INST_SRC0_ABS | \
                NV40_VP_INST_VEC_DEST_TEMP_MASK | \
                NV40_VP_INST_COND_TEST_ENABLE | \
                NV40_VP_INST_COND_MASK | \
                NV40_VP_INST_COND_SWZ_ALL_MASK | \
                NV40_VP_INST_ADDR_SWZ_MASK)

/* ---- OPCODE BITS 95:64 / data DWORD 1 --- */
#define NV40_VP_INST_VEC_OPCODE_SHIFT                                         22
#define NV40_VP_INST_VEC_OPCODE_MASK                                (0x1F << 22)
#define NV40_VP_INST_SCA_OPCODE_SHIFT                                         27
#define NV40_VP_INST_SCA_OPCODE_MASK                                (0x1F << 27)
#define NV40_VP_INST_CONST_SRC_SHIFT                                          12
#define NV40_VP_INST_CONST_SRC_MASK                                 (0xFF << 12)
#define NV40_VP_INST_INPUT_SRC_SHIFT                                           8
#define NV40_VP_INST_INPUT_SRC_MASK                                  (0x0F << 8)
#define NV40_VP_INST_SRC0H_SHIFT                                               0
#define NV40_VP_INST_SRC0H_MASK                                      (0xFF << 0)
#define NV40_VP_INST1_KNOWN ( \
                NV40_VP_INST_VEC_OPCODE_MASK | \
                NV40_VP_INST_SCA_OPCODE_MASK | \
                NV40_VP_INST_CONST_SRC_MASK  | \
                NV40_VP_INST_INPUT_SRC_MASK  | \
                NV40_VP_INST_SRC0H_MASK \
                )

/* ---- OPCODE BITS 63:32 / data DWORD 2 --- */
#define NV40_VP_INST_SRC0L_SHIFT                                              23
#define NV40_VP_INST_SRC0L_MASK                                    (0x1FF << 23)
#define NV40_VP_INST_SRC1_SHIFT                                                6
#define NV40_VP_INST_SRC1_MASK                                    (0x1FFFF << 6)
#define NV40_VP_INST_SRC2H_SHIFT                                               0
#define NV40_VP_INST_SRC2H_MASK                                      (0x3F << 0)
#define NV40_VP_INST_IADDRH_SHIFT                                              0
#define NV40_VP_INST_IADDRH_MASK                                     (0x3F << 0)

/* ---- OPCODE BITS 31:0 / data DWORD 3 --- */
#define NV40_VP_INST_IADDRL_SHIFT                                             29
#define NV40_VP_INST_IADDRL_MASK                                       (7 << 29)
#define NV40_VP_INST_SRC2L_SHIFT                                              21
#define NV40_VP_INST_SRC2L_MASK                                    (0x7FF << 21)
#define NV40_VP_INST_SCA_WRITEMASK_SHIFT                                      17
#define NV40_VP_INST_SCA_WRITEMASK_MASK                              (0xF << 17)
#    define NV40_VP_INST_SCA_WRITEMASK_X                               (1 << 20)
#    define NV40_VP_INST_SCA_WRITEMASK_Y                               (1 << 19)
#    define NV40_VP_INST_SCA_WRITEMASK_Z                               (1 << 18)
#    define NV40_VP_INST_SCA_WRITEMASK_W                               (1 << 17)
#define NV40_VP_INST_VEC_WRITEMASK_SHIFT                                      13
#define NV40_VP_INST_VEC_WRITEMASK_MASK                              (0xF << 13)
#    define NV40_VP_INST_VEC_WRITEMASK_X                               (1 << 16)
#    define NV40_VP_INST_VEC_WRITEMASK_Y                               (1 << 15)
#    define NV40_VP_INST_VEC_WRITEMASK_Z                               (1 << 14)
#    define NV40_VP_INST_VEC_WRITEMASK_W                               (1 << 13)
#define NV40_VP_INST_SCA_RESULT                                        (1 << 12)
#define NV40_VP_INST_SCA_DEST_TEMP_SHIFT                                       7
#define NV40_VP_INST_SCA_DEST_TEMP_MASK                              (0x1F << 7)
#define NV40_VP_INST_DEST_SHIFT                                                2
#define NV40_VP_INST_DEST_MASK                                         (31 << 2)
#    define NV40_VP_INST_DEST_POS                                              0
#    define NV40_VP_INST_DEST_COL0                                             1
#    define NV40_VP_INST_DEST_COL1                                             2
#    define NV40_VP_INST_DEST_BFC0                                             3
#    define NV40_VP_INST_DEST_BFC1                                             4
#    define NV40_VP_INST_DEST_FOGC                                             5
#    define NV40_VP_INST_DEST_PSZ                                              6
#    define NV40_VP_INST_DEST_TC0                                              7
#    define NV40_VP_INST_DEST_TC(n)                                        (7+n)
#    define NV40_VP_INST_DEST_TEMP                                          0x1F
#define NV40_VP_INST_INDEX_CONST                                        (1 << 1)
#define NV40_VP_INST3_KNOWN ( \
                NV40_VP_INST_SRC2L_MASK |\
                NV40_VP_INST_SCA_WRITEMASK_MASK |\
                NV40_VP_INST_VEC_WRITEMASK_MASK |\
                NV40_VP_INST_SCA_DEST_TEMP_MASK |\
                NV40_VP_INST_DEST_MASK |\
                NV40_VP_INST_INDEX_CONST)

/* Useful to split the source selection regs into their pieces */
#define NV40_VP_SRC0_HIGH_SHIFT                                                9
#define NV40_VP_SRC0_HIGH_MASK                                        0x0001FE00
#define NV40_VP_SRC0_LOW_MASK                                         0x000001FF
#define NV40_VP_SRC2_HIGH_SHIFT                                               11
#define NV40_VP_SRC2_HIGH_MASK                                        0x0001F800
#define NV40_VP_SRC2_LOW_MASK                                         0x000007FF

/* Source selection - these are the bits you fill NV40_VP_INST_SRCn with */
#define NV40_VP_SRC_NEGATE                                             (1 << 16)
#define NV40_VP_SRC_SWZ_X_SHIFT                                               14
#define NV40_VP_SRC_SWZ_X_MASK                                         (3 << 14)
#define NV40_VP_SRC_SWZ_Y_SHIFT                                               12
#define NV40_VP_SRC_SWZ_Y_MASK                                         (3 << 12)
#define NV40_VP_SRC_SWZ_Z_SHIFT                                               10
#define NV40_VP_SRC_SWZ_Z_MASK                                         (3 << 10)
#define NV40_VP_SRC_SWZ_W_SHIFT                                                8
#define NV40_VP_SRC_SWZ_W_MASK                                          (3 << 8)
#define NV40_VP_SRC_SWZ_ALL_SHIFT                                              8
#define NV40_VP_SRC_SWZ_ALL_MASK                                     (0xFF << 8)
#define NV40_VP_SRC_TEMP_SRC_SHIFT                                             2
#define NV40_VP_SRC_TEMP_SRC_MASK                                    (0x1F << 2)
#define NV40_VP_SRC_REG_TYPE_SHIFT                                             0
#define NV40_VP_SRC_REG_TYPE_MASK                                       (3 << 0)
#    define NV40_VP_SRC_REG_TYPE_UNK0                                          0
#    define NV40_VP_SRC_REG_TYPE_TEMP                                          1
#    define NV40_VP_SRC_REG_TYPE_INPUT                                         2
#    define NV40_VP_SRC_REG_TYPE_CONST                                         3

#include "nv30/nvfx_shader.h"

#endif
