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
#ifndef EG_SQ_H
#define EG_SQ_H

#define P_SQ_CF_WORD0
#define   S_SQ_CF_WORD0_ADDR(x)                                      (((unsigned)(x) & 0xFFFFFF) << 0)
#define   G_SQ_CF_WORD0_ADDR(x)                                      (((x) >> 0) & 0xFFFFFF)
#define   C_SQ_CF_WORD0_ADDR                                         0x00000000
#define   S_SQ_CF_WORD0_JUMPTABLE_SEL(x)                             (((unsigned)(x) & 0x7) << 24)
#define   G_SQ_CF_WORD0_JUMPTABLE_SEL(x)                             (((x) >> 24) & 0x7)
#define   C_SQ_CF_WORD0_JUMPTABLE_SEL                                0xF8FFFFFF
#define P_SQ_CF_WORD1
#define   S_SQ_CF_WORD1_POP_COUNT(x)                                 (((unsigned)(x) & 0x7) << 0)
#define   G_SQ_CF_WORD1_POP_COUNT(x)                                 (((x) >> 0) & 0x7)
#define   C_SQ_CF_WORD1_POP_COUNT                                    0xFFFFFFF8
#define   S_SQ_CF_WORD1_CF_CONST(x)                                  (((unsigned)(x) & 0x1F) << 3)
#define   G_SQ_CF_WORD1_CF_CONST(x)                                  (((x) >> 3) & 0x1F)
#define   C_SQ_CF_WORD1_CF_CONST                                     0xFFFFFF07
#define   S_SQ_CF_WORD1_COND(x)                                      (((unsigned)(x) & 0x3) << 8)
#define   G_SQ_CF_WORD1_COND(x)                                      (((x) >> 8) & 0x3)
#define   C_SQ_CF_WORD1_COND                                         0xFFFFFCFF
#define   S_SQ_CF_WORD1_COUNT(x)                                     (((unsigned)(x) & 0x3f) << 10)
#define   G_SQ_CF_WORD1_COUNT(x)                                     (((x) >> 10) & 0x3f)
#define   C_SQ_CF_WORD1_COUNT                                        0xFFFF03FF
#define   S_SQ_CF_WORD1_VALID_PIXEL_MODE(x)                          (((unsigned)(x) & 0x1) << 20)
#define   G_SQ_CF_WORD1_VALID_PIXEL_MODE(x)                          (((x) >> 20) & 0x1)
#define   C_SQ_CF_WORD1_VALID_PIXEL_MODE                             0xFFEFFFFF
#define   S_SQ_CF_WORD1_END_OF_PROGRAM(x)                            (((unsigned)(x) & 0x1) << 21)
#define   G_SQ_CF_WORD1_END_OF_PROGRAM(x)                            (((x) >> 21) & 0x1)
#define   C_SQ_CF_WORD1_END_OF_PROGRAM                               0xFFDFFFFF

#define   S_SQ_CF_WORD1_CF_INST(x)                                   (((unsigned)(x) & 0xFF) << 22)
#define   G_SQ_CF_WORD1_CF_INST(x)                                   (((x) >> 22) & 0xFF)
#define   C_SQ_CF_WORD1_CF_INST                                      0xC03FFFFF

#define   S_SQ_CF_WORD1_WHOLE_QUAD_MODE(x)                           (((unsigned)(x) & 0x1) << 30)
#define   G_SQ_CF_WORD1_WHOLE_QUAD_MODE(x)                           (((x) >> 30) & 0x1)
#define   C_SQ_CF_WORD1_WHOLE_QUAD_MODE                              0xBFFFFFFF
#define   S_SQ_CF_WORD1_BARRIER(x)                                   (((unsigned)(x) & 0x1) << 31)
#define   G_SQ_CF_WORD1_BARRIER(x)                                   (((x) >> 31) & 0x1)
#define   C_SQ_CF_WORD1_BARRIER                                      0x7FFFFFFF

/* done */
#define P_SQ_CF_ALU_WORD0
#define   S_SQ_CF_ALU_WORD0_ADDR(x)                                  (((unsigned)(x) & 0x3FFFFF) << 0)
#define   G_SQ_CF_ALU_WORD0_ADDR(x)                                  (((x) >> 0) & 0x3FFFFF)
#define   C_SQ_CF_ALU_WORD0_ADDR                                     0xFFC00000
#define   S_SQ_CF_ALU_WORD0_KCACHE_BANK0(x)                          (((unsigned)(x) & 0xF) << 22)
#define   G_SQ_CF_ALU_WORD0_KCACHE_BANK0(x)                          (((x) >> 22) & 0xF)
#define   C_SQ_CF_ALU_WORD0_KCACHE_BANK0                             0xFC3FFFFF
#define   S_SQ_CF_ALU_WORD0_KCACHE_BANK1(x)                          (((unsigned)(x) & 0xF) << 26)
#define   G_SQ_CF_ALU_WORD0_KCACHE_BANK1(x)                          (((x) >> 26) & 0xF)
#define   C_SQ_CF_ALU_WORD0_KCACHE_BANK1                             0xC3FFFFFF
#define   S_SQ_CF_ALU_WORD0_KCACHE_MODE0(x)                          (((unsigned)(x) & 0x3) << 30)
#define   G_SQ_CF_ALU_WORD0_KCACHE_MODE0(x)                          (((x) >> 30) & 0x3)
#define   C_SQ_CF_ALU_WORD0_KCACHE_MODE0                             0x3FFFFFFF
#define     V_SQ_CF_KCACHE_NOP                                       0x00000000
#define     V_SQ_CF_KCACHE_LOCK_1                                    0x00000001
#define     V_SQ_CF_KCACHE_LOCK_2                                    0x00000002
#define     V_SQ_CF_KCACHE_LOCK_LOOP_INDEX                           0x00000003
#define P_SQ_CF_ALU_WORD1
#define   S_SQ_CF_ALU_WORD1_KCACHE_MODE1(x)                          (((unsigned)(x) & 0x3) << 0)
#define   G_SQ_CF_ALU_WORD1_KCACHE_MODE1(x)                          (((x) >> 0) & 0x3)
#define   C_SQ_CF_ALU_WORD1_KCACHE_MODE1                             0xFFFFFFFC
#define   S_SQ_CF_ALU_WORD1_KCACHE_ADDR0(x)                          (((unsigned)(x) & 0xFF) << 2)
#define   G_SQ_CF_ALU_WORD1_KCACHE_ADDR0(x)                          (((x) >> 2) & 0xFF)
#define   C_SQ_CF_ALU_WORD1_KCACHE_ADDR0                             0xFFFFFC03
#define   S_SQ_CF_ALU_WORD1_KCACHE_ADDR1(x)                          (((unsigned)(x) & 0xFF) << 10)
#define   G_SQ_CF_ALU_WORD1_KCACHE_ADDR1(x)                          (((x) >> 10) & 0xFF)
#define   C_SQ_CF_ALU_WORD1_KCACHE_ADDR1                             0xFFFC03FF
#define   S_SQ_CF_ALU_WORD1_COUNT(x)                                 (((unsigned)(x) & 0x7F) << 18)
#define   G_SQ_CF_ALU_WORD1_COUNT(x)                                 (((x) >> 18) & 0x7F)
#define   C_SQ_CF_ALU_WORD1_COUNT                                    0xFE03FFFF
#define   S_SQ_CF_ALU_WORD1_ALT_CONST(x)                             (((unsigned)(x) & 0x1) << 25)
#define   G_SQ_CF_ALU_WORD1_ALT_CONST(x)                             (((x) >> 25) & 0x1)
#define   C_SQ_CF_ALU_WORD1_ALT_CONST                                0xFDFFFFFF
#define   S_SQ_CF_ALU_WORD1_CF_INST(x)                               (((unsigned)(x) & 0xF) << 26)
#define   G_SQ_CF_ALU_WORD1_CF_INST(x)                               (((x) >> 26) & 0xF)
#define   C_SQ_CF_ALU_WORD1_CF_INST                                  0xC3FFFFFF
#define   S_SQ_CF_ALU_WORD1_WHOLE_QUAD_MODE(x)                       (((unsigned)(x) & 0x1) << 30)
#define   G_SQ_CF_ALU_WORD1_WHOLE_QUAD_MODE(x)                       (((x) >> 30) & 0x1)
#define   C_SQ_CF_ALU_WORD1_WHOLE_QUAD_MODE                          0xBFFFFFFF
#define   S_SQ_CF_ALU_WORD1_BARRIER(x)                               (((unsigned)(x) & 0x1) << 31)
#define   G_SQ_CF_ALU_WORD1_BARRIER(x)                               (((x) >> 31) & 0x1)
#define   C_SQ_CF_ALU_WORD1_BARRIER                                  0x7FFFFFFF

#define P_SQ_CF_ALU_WORD0_EXT
#define   S_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE0(x)           (((unsigned)(x) & 0x3) << 4)
#define   G_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE0(x)           (((x) >> 4) & 0x3)
#define   C_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE0              0xFFFFFFCF
#define	    V_SQ_CF_INDEX_NONE                                       0x00
#define	    V_SQ_CF_INDEX_0                                          0x01
#define	    V_SQ_CF_INDEX_1                                          0x02
#define   S_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE1(x)           (((unsigned)(x) & 0x3) << 6)
#define   G_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE1(x)           (((x) >> 6) & 0x3)
#define   C_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE1              0xFFFFFF3F
#define   S_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE2(x)           (((unsigned)(x) & 0x3) << 8)
#define   G_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE2(x)           (((x) >> 8) & 0x3)
#define   C_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE2              0xFFFFFCFF
#define   S_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE3(x)           (((unsigned)(x) & 0x3) << 10)
#define   G_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE3(x)           (((x) >> 10) & 0x3)
#define   C_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE3              0xFFFFF3FF
#define   S_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK2(x)                      (((unsigned)(x) & 0xF) << 22)
#define   G_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK2(x)                      (((x) >> 22) & 0xF)
#define   C_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK2                         0xFC3FFFFF
#define   S_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK3(x)                      (((unsigned)(x) & 0xF) << 26)
#define   G_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK3(x)                      (((x) >> 26) & 0xF)
#define   C_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK3                         0xC3FFFFFF
#define   S_SQ_CF_ALU_WORD0_EXT_KCACHE_MODE2(x)                      (((unsigned)(x) & 0x3) << 30)
#define   G_SQ_CF_ALU_WORD0_EXT_KCACHE_MODE2(x)                      (((x) >> 30) & 0x3)
#define   C_SQ_CF_ALU_WORD0_EXT_KCACHE_MODE2                         0x3FFFFFFF

#define P_SQ_CF_ALU_WORD1_EXT
#define   S_SQ_CF_ALU_WORD1_EXT_KCACHE_MODE3(x)                      (((unsigned)(x) & 0x3) << 0)
#define   G_SQ_CF_ALU_WORD1_EXT_KCACHE_MODE3(x)                      (((x) >> 0) & 0x3)
#define   C_SQ_CF_ALU_WORD1_EXT_KCACHE_MODE3                         0xFFFFFFFC
#define   S_SQ_CF_ALU_WORD1_EXT_KCACHE_ADDR2(x)                      (((unsigned)(x) & 0xFF) << 2)
#define   G_SQ_CF_ALU_WORD1_EXT_KCACHE_ADDR2(x)                      (((x) >> 2) & 0xFF)
#define   C_SQ_CF_ALU_WORD1_EXT_KCACHE_ADDR2                         0xFFFFFC03
#define   S_SQ_CF_ALU_WORD1_EXT_KCACHE_ADDR3(x)                      (((unsigned)(x) & 0xFF) << 10)
#define   G_SQ_CF_ALU_WORD1_EXT_KCACHE_ADDR3(x)                      (((x) >> 10) & 0xFF)
#define   C_SQ_CF_ALU_WORD1_EXT_KCACHE_ADDR3                         0xFFFC03FF
#define   S_SQ_CF_ALU_WORD1_EXT_CF_INST(x)                           (((unsigned)(x) & 0xF) << 26)
#define   G_SQ_CF_ALU_WORD1_EXT_CF_INST(x)                           (((x) >> 26) & 0xF)
#define   C_SQ_CF_ALU_WORD1_EXT_CF_INST                              0xC3FFFFFF
#define   S_SQ_CF_ALU_WORD1_EXT_BARRIER(x)                           (((unsigned)(x) & 0x1) << 31)
#define   G_SQ_CF_ALU_WORD1_EXT_BARRIER(x)                           (((x) >> 31) & 0x1)
#define   C_SQ_CF_ALU_WORD1_EXT_BARRIER                              0x7FFFFFFF

/* done */
#define P_SQ_CF_ALLOC_EXPORT_WORD0
#define   S_SQ_CF_ALLOC_EXPORT_WORD0_ARRAY_BASE(x)                   (((unsigned)(x) & 0x1FFF) << 0)
#define   G_SQ_CF_ALLOC_EXPORT_WORD0_ARRAY_BASE(x)                   (((x) >> 0) & 0x1FFF)
#define   C_SQ_CF_ALLOC_EXPORT_WORD0_ARRAY_BASE                      0xFFFFE000
#define   S_SQ_CF_ALLOC_EXPORT_WORD0_TYPE(x)                         (((unsigned)(x) & 0x3) << 13)
#define   G_SQ_CF_ALLOC_EXPORT_WORD0_TYPE(x)                         (((x) >> 13) & 0x3)
#define   C_SQ_CF_ALLOC_EXPORT_WORD0_TYPE                            0xFFFF9FFF
#define     V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_PIXEL               0x00000000
#define     V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_POS                 0x00000001
#define     V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_PARAM               0x00000002
#define     V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_WRITE               0x00000000
#define     V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_WRITE_IND           0x00000001
#define     V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_READ                0x00000002
#define     V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_WRITE_ACK           0x00000002
#define     V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_READ_IND            0x00000003
#define     V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_WRITE_IND_ACK       0x00000003

#define   S_SQ_CF_ALLOC_EXPORT_WORD0_RW_GPR(x)                       (((unsigned)(x) & 0x7F) << 15)
#define   G_SQ_CF_ALLOC_EXPORT_WORD0_RW_GPR(x)                       (((x) >> 15) & 0x7F)
#define   C_SQ_CF_ALLOC_EXPORT_WORD0_RW_GPR                          0xFFC07FFF
#define   S_SQ_CF_ALLOC_EXPORT_WORD0_RW_REL(x)                       (((unsigned)(x) & 0x1) << 22)
#define   G_SQ_CF_ALLOC_EXPORT_WORD0_RW_REL(x)                       (((x) >> 22) & 0x1)
#define   C_SQ_CF_ALLOC_EXPORT_WORD0_RW_REL                          0xFFBFFFFF
#define   S_SQ_CF_ALLOC_EXPORT_WORD0_INDEX_GPR(x)                    (((unsigned)(x) & 0x7F) << 23)
#define   G_SQ_CF_ALLOC_EXPORT_WORD0_INDEX_GPR(x)                    (((x) >> 23) & 0x7F)
#define   C_SQ_CF_ALLOC_EXPORT_WORD0_INDEX_GPR                       0xC07FFFFF
#define   S_SQ_CF_ALLOC_EXPORT_WORD0_ELEM_SIZE(x)                    (((unsigned)(x) & 0x3) << 30)
#define   G_SQ_CF_ALLOC_EXPORT_WORD0_ELEM_SIZE(x)                    (((x) >> 30) & 0x3)
#define   C_SQ_CF_ALLOC_EXPORT_WORD0_ELEM_SIZE                       0x3FFFFFFF
/* done */

#define P_SQ_CF_ALLOC_EXPORT_WORD0_RAT
#define   S_SQ_CF_ALLOC_EXPORT_WORD0_RAT_RAT_ID(x)                   (((x) & 0xF) << 0)
#define   G_SQ_CF_ALLOC_EXPORT_WORD0_RAT_RAT_ID(x)                   (((x) >> 0) & 0xF)
#define   C_SQ_CF_ALLOC_EXPORT_WORD0_RAT_RAT_ID                      0xFFFFFFF0
#define   S_SQ_CF_ALLOC_EXPORT_WORD0_RAT_RAT_INST(x)                 (((x) & 0x3F) << 4)
#define   G_SQ_CF_ALLOC_EXPORT_WORD0_RAT_RAT_INST(x)                 (((x) >> 4) & 0x3F)
#define   C_SQ_CF_ALLOC_EXPORT_WORD0_RAT_RAT_INST                    0xFFFFFC0F
#define   S_SQ_CF_ALLOC_EXPORT_WORD0_RAT_RAT_INDEX_MODE(x)           (((x) & 0x3) << 11)
#define   G_SQ_CF_ALLOC_EXPORT_WORD0_RAT_RAT_INDEX_MODE(x)           (((x) >> 11) & 0x3)
#define   C_SQ_CF_ALLOC_EXPORT_WORD0_RAT_RAT_INDEX_MODE              0xFFFFE7FF

#define P_SQ_CF_ALLOC_EXPORT_WORD1
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_BURST_COUNT(x)                  (((unsigned)(x) & 0xF) << 16)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_BURST_COUNT(x)                  (((x) >> 16) & 0xF)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_BURST_COUNT                     0xFFF0FFFF
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_VALID_PIXEL_MODE(x)             (((unsigned)(x) & 0x1) << 20)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_VALID_PIXEL_MODE(x)             (((x) >> 20) & 0x1)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_VALID_PIXEL_MODE                0xFFEFFFFF
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM(x)               (((unsigned)(x) & 0x1) << 21)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM(x)               (((x) >> 21) & 0x1)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM                  0xFFDFFFFF
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST(x)                      (((unsigned)(x) & 0xFF) << 22)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST(x)                      (((x) >> 22) & 0xFF)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST                         0xC03FFFFF

#define   S_SQ_CF_ALLOC_EXPORT_WORD1_MARK(x)                         (((unsigned)(x) & 0x1) << 30)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_MARK(x)                         (((x) >> 30) & 0x1)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_WHOLE_QUAD_MODE                 0xBFFFFFFF
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER(x)                      (((unsigned)(x) & 0x1) << 31)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER(x)                      (((x) >> 31) & 0x1)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER                         0x7FFFFFFF

/* done */
#define P_SQ_CF_ALLOC_EXPORT_WORD1_BUF
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_BUF_ARRAY_SIZE(x)               (((unsigned)(x) & 0xFFF) << 0)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_BUF_ARRAY_SIZE(x)               (((x) >> 0) & 0xFFF)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_BUF_ARRAY_SIZE                  0xFFFFF000
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_BUF_COMP_MASK(x)                (((unsigned)(x) & 0xF) << 12)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_BUF_COMP_MASK(x)                (((x) >> 12) & 0xF)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_BUF_COMP_MASK                   0xFFFF0FFF
#define P_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_X(x)                   (((unsigned)(x) & 0x7) << 0)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_X(x)                   (((x) >> 0) & 0x7)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_X                      0xFFFFFFF8
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_Y(x)                   (((unsigned)(x) & 0x7) << 3)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_Y(x)                   (((x) >> 3) & 0x7)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_Y                      0xFFFFFFC7
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_Z(x)                   (((unsigned)(x) & 0x7) << 6)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_Z(x)                   (((x) >> 6) & 0x7)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_Z                      0xFFFFFE3F
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_W(x)                   (((unsigned)(x) & 0x7) << 9)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_W(x)                   (((x) >> 9) & 0x7)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_W                      0xFFFFF1FF

/* done */
#define P_SQ_ALU_WORD0
#define   S_SQ_ALU_WORD0_SRC0_SEL(x)                                 (((unsigned)(x) & 0x1FF) << 0)
#define   G_SQ_ALU_WORD0_SRC0_SEL(x)                                 (((x) >> 0) & 0x1FF)
#define   C_SQ_ALU_WORD0_SRC0_SEL                                    0xFFFFFE00

/*
 * 244  ALU_SRC_1_DBL_L: special constant 1.0 double-float, LSW. (RV670+)
 * 245  ALU_SRC_1_DBL_M: special constant 1.0 double-float, MSW. (RV670+)
 * 246  ALU_SRC_0_5_DBL_L: special constant 0.5 double-float, LSW. (RV670+)
 * 247  ALU_SRC_0_5_DBL_M: special constant 0.5 double-float, MSW. (RV670+)
 * 248  SQ_ALU_SRC_0: special constant 0.0.
 * 249  SQ_ALU_SRC_1: special constant 1.0 float.
 * 250  SQ_ALU_SRC_1_INT: special constant 1 integer.
 * 251  SQ_ALU_SRC_M_1_INT: special constant -1 integer.
 * 252  SQ_ALU_SRC_0_5: special constant 0.5 float.
 * 253  SQ_ALU_SRC_LITERAL: literal constant.
 * 254  SQ_ALU_SRC_PV: previous vector result.
 * 255  SQ_ALU_SRC_PS: previous scalar result.
 */
#define     V_SQ_ALU_SRC_0                                           0x000000F8
#define     V_SQ_ALU_SRC_1                                           0x000000F9
#define     V_SQ_ALU_SRC_1_INT                                       0x000000FA
#define     V_SQ_ALU_SRC_M_1_INT                                     0x000000FB
#define     V_SQ_ALU_SRC_0_5                                         0x000000FC
#define     V_SQ_ALU_SRC_LITERAL                                     0x000000FD
#define   S_SQ_ALU_WORD0_SRC0_REL(x)                                 (((unsigned)(x) & 0x1) << 9)
#define   G_SQ_ALU_WORD0_SRC0_REL(x)                                 (((x) >> 9) & 0x1)
#define   C_SQ_ALU_WORD0_SRC0_REL                                    0xFFFFFDFF
#define   S_SQ_ALU_WORD0_SRC0_CHAN(x)                                (((unsigned)(x) & 0x3) << 10)
#define   G_SQ_ALU_WORD0_SRC0_CHAN(x)                                (((x) >> 10) & 0x3)
#define   C_SQ_ALU_WORD0_SRC0_CHAN                                   0xFFFFF3FF
#define   S_SQ_ALU_WORD0_SRC0_NEG(x)                                 (((unsigned)(x) & 0x1) << 12)
#define   G_SQ_ALU_WORD0_SRC0_NEG(x)                                 (((x) >> 12) & 0x1)
#define   C_SQ_ALU_WORD0_SRC0_NEG                                    0xFFFFEFFF
#define   S_SQ_ALU_WORD0_SRC1_SEL(x)                                 (((unsigned)(x) & 0x1FF) << 13)
#define   G_SQ_ALU_WORD0_SRC1_SEL(x)                                 (((x) >> 13) & 0x1FF)
#define   C_SQ_ALU_WORD0_SRC1_SEL                                    0xFFC01FFF
#define   S_SQ_ALU_WORD0_SRC1_REL(x)                                 (((unsigned)(x) & 0x1) << 22)
#define   G_SQ_ALU_WORD0_SRC1_REL(x)                                 (((x) >> 22) & 0x1)
#define   C_SQ_ALU_WORD0_SRC1_REL                                    0xFFBFFFFF
#define   S_SQ_ALU_WORD0_SRC1_CHAN(x)                                (((unsigned)(x) & 0x3) << 23)
#define   G_SQ_ALU_WORD0_SRC1_CHAN(x)                                (((x) >> 23) & 0x3)
#define   C_SQ_ALU_WORD0_SRC1_CHAN                                   0xFE7FFFFF
#define   S_SQ_ALU_WORD0_SRC1_NEG(x)                                 (((unsigned)(x) & 0x1) << 25)
#define   G_SQ_ALU_WORD0_SRC1_NEG(x)                                 (((x) >> 25) & 0x1)
#define   C_SQ_ALU_WORD0_SRC1_NEG                                    0xFDFFFFFF
#define   S_SQ_ALU_WORD0_INDEX_MODE(x)                               (((unsigned)(x) & 0x7) << 26)
#define   G_SQ_ALU_WORD0_INDEX_MODE(x)                               (((x) >> 26) & 0x7)
#define   C_SQ_ALU_WORD0_INDEX_MODE                                  0xE3FFFFFF
#define   S_SQ_ALU_WORD0_PRED_SEL(x)                                 (((unsigned)(x) & 0x3) << 29)
#define   G_SQ_ALU_WORD0_PRED_SEL(x)                                 (((x) >> 29) & 0x3)
#define   C_SQ_ALU_WORD0_PRED_SEL                                    0x9FFFFFFF
#define   S_SQ_ALU_WORD0_LAST(x)                                     (((unsigned)(x) & 0x1) << 31)
#define   G_SQ_ALU_WORD0_LAST(x)                                     (((x) >> 31) & 0x1)
#define   C_SQ_ALU_WORD0_LAST                                        0x7FFFFFFF
/* same */
#define P_SQ_ALU_WORD1
#define   S_SQ_ALU_WORD1_ENCODING(x)                                 (((unsigned)(x) & 0x7) << 15)
#define   G_SQ_ALU_WORD1_ENCODING(x)                                 (((x) >> 15) & 0x7)
#define   C_SQ_ALU_WORD1_ENCODING                                    0xFFFC7FFF
#define   S_SQ_ALU_WORD1_BANK_SWIZZLE(x)                             (((unsigned)(x) & 0x7) << 18)
#define   G_SQ_ALU_WORD1_BANK_SWIZZLE(x)                             (((x) >> 18) & 0x7)
#define   C_SQ_ALU_WORD1_BANK_SWIZZLE                                0xFFE3FFFF
#define   S_SQ_ALU_WORD1_DST_GPR(x)                                  (((unsigned)(x) & 0x7F) << 21)
#define   G_SQ_ALU_WORD1_DST_GPR(x)                                  (((x) >> 21) & 0x7F)
#define   C_SQ_ALU_WORD1_DST_GPR                                     0xF01FFFFF
#define   S_SQ_ALU_WORD1_DST_REL(x)                                  (((unsigned)(x) & 0x1) << 28)
#define   G_SQ_ALU_WORD1_DST_REL(x)                                  (((x) >> 28) & 0x1)
#define   C_SQ_ALU_WORD1_DST_REL                                     0xEFFFFFFF
#define   S_SQ_ALU_WORD1_DST_CHAN(x)                                 (((unsigned)(x) & 0x3) << 29)
#define   G_SQ_ALU_WORD1_DST_CHAN(x)                                 (((x) >> 29) & 0x3)
#define   C_SQ_ALU_WORD1_DST_CHAN                                    0x9FFFFFFF
#define   S_SQ_ALU_WORD1_CLAMP(x)                                    (((unsigned)(x) & 0x1) << 31)
#define   G_SQ_ALU_WORD1_CLAMP(x)                                    (((x) >> 31) & 0x1)
#define   C_SQ_ALU_WORD1_CLAMP                                       0x7FFFFFFF
/* same except maybe encoding */
#define P_SQ_ALU_WORD1_OP2
#define   S_SQ_ALU_WORD1_OP2_SRC0_ABS(x)                             (((unsigned)(x) & 0x1) << 0)
#define   G_SQ_ALU_WORD1_OP2_SRC0_ABS(x)                             (((x) >> 0) & 0x1)
#define   C_SQ_ALU_WORD1_OP2_SRC0_ABS                                0xFFFFFFFE
#define   S_SQ_ALU_WORD1_OP2_SRC1_ABS(x)                             (((unsigned)(x) & 0x1) << 1)
#define   G_SQ_ALU_WORD1_OP2_SRC1_ABS(x)                             (((x) >> 1) & 0x1)
#define   C_SQ_ALU_WORD1_OP2_SRC1_ABS                                0xFFFFFFFD
#define   S_SQ_ALU_WORD1_OP2_UPDATE_EXECUTE_MASK(x)                  (((unsigned)(x) & 0x1) << 2)
#define   G_SQ_ALU_WORD1_OP2_UPDATE_EXECUTE_MASK(x)                  (((x) >> 2) & 0x1)
#define   C_SQ_ALU_WORD1_OP2_UPDATE_EXECUTE_MASK                     0xFFFFFFFB
#define   S_SQ_ALU_WORD1_OP2_UPDATE_PRED(x)                          (((unsigned)(x) & 0x1) << 3)
#define   G_SQ_ALU_WORD1_OP2_UPDATE_PRED(x)                          (((x) >> 3) & 0x1)
#define   C_SQ_ALU_WORD1_OP2_UPDATE_PRED                             0xFFFFFFF7
#define   S_SQ_ALU_WORD1_OP2_WRITE_MASK(x)                           (((unsigned)(x) & 0x1) << 4)
#define   G_SQ_ALU_WORD1_OP2_WRITE_MASK(x)                           (((x) >> 4) & 0x1)
#define   C_SQ_ALU_WORD1_OP2_WRITE_MASK                              0xFFFFFFEF
#define   S_SQ_ALU_WORD1_OP2_OMOD(x)                                 (((unsigned)(x) & 0x3) << 5)
#define   G_SQ_ALU_WORD1_OP2_OMOD(x)                                 (((x) >> 5) & 0x3)
#define   C_SQ_ALU_WORD1_OP2_OMOD                                    0xFFFFFF9F
#define   S_SQ_ALU_WORD1_OP2_ALU_INST(x)                             (((unsigned)(x) & 0x7FF) << 7)
#define   G_SQ_ALU_WORD1_OP2_ALU_INST(x)                             (((x) >> 7) & 0x7FF)
#define   C_SQ_ALU_WORD1_OP2_ALU_INST                                0xFFFC007F

#define P_SQ_ALU_WORD1_OP3
#define   S_SQ_ALU_WORD1_OP3_SRC2_SEL(x)                             (((unsigned)(x) & 0x1FF) << 0)
#define   G_SQ_ALU_WORD1_OP3_SRC2_SEL(x)                             (((x) >> 0) & 0x1FF)
#define   C_SQ_ALU_WORD1_OP3_SRC2_SEL                                0xFFFFFE00
#define   S_SQ_ALU_WORD1_OP3_SRC2_REL(x)                             (((unsigned)(x) & 0x1) << 9)
#define   G_SQ_ALU_WORD1_OP3_SRC2_REL(x)                             (((x) >> 9) & 0x1)
#define   C_SQ_ALU_WORD1_OP3_SRC2_REL                                0xFFFFFDFF
#define   S_SQ_ALU_WORD1_OP3_SRC2_CHAN(x)                            (((unsigned)(x) & 0x3) << 10)
#define   G_SQ_ALU_WORD1_OP3_SRC2_CHAN(x)                            (((x) >> 10) & 0x3)
#define   C_SQ_ALU_WORD1_OP3_SRC2_CHAN                               0xFFFFF3FF
#define   S_SQ_ALU_WORD1_OP3_SRC2_NEG(x)                             (((unsigned)(x) & 0x1) << 12)
#define   G_SQ_ALU_WORD1_OP3_SRC2_NEG(x)                             (((x) >> 12) & 0x1)
#define   C_SQ_ALU_WORD1_OP3_SRC2_NEG                                0xFFFFEFFF
#define   S_SQ_ALU_WORD1_OP3_ALU_INST(x)                             (((unsigned)(x) & 0x1F) << 13)
#define   G_SQ_ALU_WORD1_OP3_ALU_INST(x)                             (((x) >> 13) & 0x1F)
#define   C_SQ_ALU_WORD1_OP3_ALU_INST                                0xFFFC1FFF
/* XXX ADD OTHER OP3 */
/* done */
#define P_SQ_VTX_WORD0
#define   S_SQ_VTX_WORD0_VTX_INST(x)                                 (((unsigned)(x) & 0x1F) << 0)
#define   G_SQ_VTX_WORD0_VTX_INST(x)                                 (((x) >> 0) & 0x1F)
#define   C_SQ_VTX_WORD0_VTX_INST                                    0xFFFFFFE0
#define   S_SQ_VTX_WORD0_FETCH_TYPE(x)                               (((unsigned)(x) & 0x3) << 5)
#define   G_SQ_VTX_WORD0_FETCH_TYPE(x)                               (((x) >> 5) & 0x3)
#define   C_SQ_VTX_WORD0_FETCH_TYPE                                  0xFFFFFF9F
#define   S_SQ_VTX_WORD0_FETCH_WHOLE_QUAD(x)                         (((unsigned)(x) & 0x1) << 7)
#define   G_SQ_VTX_WORD0_FETCH_WHOLE_QUAD(x)                         (((x) >> 7) & 0x1)
#define   C_SQ_VTX_WORD0_FETCH_WHOLE_QUAD                            0xFFFFFF7F
#define   S_SQ_VTX_WORD0_BUFFER_ID(x)                                (((unsigned)(x) & 0xFF) << 8)
#define   G_SQ_VTX_WORD0_BUFFER_ID(x)                                (((x) >> 8) & 0xFF)
#define   C_SQ_VTX_WORD0_BUFFER_ID                                   0xFFFF00FF
#define   S_SQ_VTX_WORD0_SRC_GPR(x)                                  (((unsigned)(x) & 0x7F) << 16)
#define   G_SQ_VTX_WORD0_SRC_GPR(x)                                  (((x) >> 16) & 0x7F)
#define   C_SQ_VTX_WORD0_SRC_GPR                                     0xFF80FFFF
#define   S_SQ_VTX_WORD0_SRC_REL(x)                                  (((unsigned)(x) & 0x1) << 23)
#define   G_SQ_VTX_WORD0_SRC_REL(x)                                  (((x) >> 23) & 0x1)
#define   C_SQ_VTX_WORD0_SRC_REL                                     0xFF7FFFFF
#define   S_SQ_VTX_WORD0_SRC_SEL_X(x)                                (((unsigned)(x) & 0x3) << 24)
#define   G_SQ_VTX_WORD0_SRC_SEL_X(x)                                (((x) >> 24) & 0x3)
#define   C_SQ_VTX_WORD0_SRC_SEL_X                                   0xFCFFFFFF
#define   S_SQ_VTX_WORD0_MEGA_FETCH_COUNT(x)                         (((unsigned)(x) & 0x3F) << 26)
#define   G_SQ_VTX_WORD0_MEGA_FETCH_COUNT(x)                         (((x) >> 26) & 0x3F)
#define   C_SQ_VTX_WORD0_MEGA_FETCH_COUNT                            0x03FFFFFF
/* same WORD 0 */
#define P_SQ_VTX_WORD1
#define   S_SQ_VTX_WORD1_DST_SEL_X(x)                                (((unsigned)(x) & 0x7) << 9)
#define   G_SQ_VTX_WORD1_DST_SEL_X(x)                                (((x) >> 9) & 0x7)
#define   C_SQ_VTX_WORD1_DST_SEL_X                                   0xFFFFF1FF
#define   S_SQ_VTX_WORD1_DST_SEL_Y(x)                                (((unsigned)(x) & 0x7) << 12)
#define   G_SQ_VTX_WORD1_DST_SEL_Y(x)                                (((x) >> 12) & 0x7)
#define   C_SQ_VTX_WORD1_DST_SEL_Y                                   0xFFFF8FFF
#define   S_SQ_VTX_WORD1_DST_SEL_Z(x)                                (((unsigned)(x) & 0x7) << 15)
#define   G_SQ_VTX_WORD1_DST_SEL_Z(x)                                (((x) >> 15) & 0x7)
#define   C_SQ_VTX_WORD1_DST_SEL_Z                                   0xFFFC7FFF
#define   S_SQ_VTX_WORD1_DST_SEL_W(x)                                (((unsigned)(x) & 0x7) << 18)
#define   G_SQ_VTX_WORD1_DST_SEL_W(x)                                (((x) >> 18) & 0x7)
#define   C_SQ_VTX_WORD1_DST_SEL_W                                   0xFFE3FFFF
#define   S_SQ_VTX_WORD1_USE_CONST_FIELDS(x)                         (((unsigned)(x) & 0x1) << 21)
#define   G_SQ_VTX_WORD1_USE_CONST_FIELDS(x)                         (((x) >> 21) & 0x1)
#define   C_SQ_VTX_WORD1_USE_CONST_FIELDS                            0xFFDFFFFF
#define   S_SQ_VTX_WORD1_DATA_FORMAT(x)                              (((unsigned)(x) & 0x3F) << 22)
#define   G_SQ_VTX_WORD1_DATA_FORMAT(x)                              (((x) >> 22) & 0x3F)
#define   C_SQ_VTX_WORD1_DATA_FORMAT                                 0xF03FFFFF
#define   S_SQ_VTX_WORD1_NUM_FORMAT_ALL(x)                           (((unsigned)(x) & 0x3) << 28)
#define   G_SQ_VTX_WORD1_NUM_FORMAT_ALL(x)                           (((x) >> 28) & 0x3)
#define   C_SQ_VTX_WORD1_NUM_FORMAT_ALL                              0xCFFFFFFF
#define   S_SQ_VTX_WORD1_FORMAT_COMP_ALL(x)                          (((unsigned)(x) & 0x1) << 30)
#define   G_SQ_VTX_WORD1_FORMAT_COMP_ALL(x)                          (((x) >> 30) & 0x1)
#define   C_SQ_VTX_WORD1_FORMAT_COMP_ALL                             0xBFFFFFFF
#define   S_SQ_VTX_WORD1_SRF_MODE_ALL(x)                             (((unsigned)(x) & 0x1) << 31)
#define   G_SQ_VTX_WORD1_SRF_MODE_ALL(x)                             (((x) >> 31) & 0x1)
#define   C_SQ_VTX_WORD1_SRF_MODE_ALL                                0x7FFFFFFF
/* same WORD1 generic */
#define P_SQ_VTX_WORD1_GPR
#define   S_SQ_VTX_WORD1_GPR_DST_GPR(x)                              (((unsigned)(x) & 0x7F) << 0)
#define   G_SQ_VTX_WORD1_GPR_DST_GPR(x)                              (((x) >> 0) & 0x7F)
#define   C_SQ_VTX_WORD1_GPR_DST_GPR                                 0xFFFFFF80
#define   S_SQ_VTX_WORD1_GPR_DST_REL(x)                              (((unsigned)(x) & 0x1) << 7)
#define   G_SQ_VTX_WORD1_GPR_DST_REL(x)                              (((x) >> 7) & 0x1)
#define   C_SQ_VTX_WORD1_GPR_DST_REL                                 0xFFFFFF7F
#define P_SQ_VTX_WORD1_SEM
#define   S_SQ_VTX_WORD1_SEM_SEMANTIC_ID(x)                          (((unsigned)(x) & 0xFF) << 0)
#define   G_SQ_VTX_WORD1_SEM_SEMANTIC_ID(x)                          (((x) >> 0) & 0xFF)
#define   C_SQ_VTX_WORD1_SEM_SEMANTIC_ID                             0xFFFFFF00
#define P_SQ_VTX_WORD2
#define   S_SQ_VTX_WORD2_OFFSET(x)                                   (((unsigned)(x) & 0xFFFF) << 0)
#define   G_SQ_VTX_WORD2_OFFSET(x)                                   (((x) >> 0) & 0xFFFF)
#define   C_SQ_VTX_WORD2_OFFSET                                      0xFFFF0000
#define   S_SQ_VTX_WORD2_ENDIAN_SWAP(x)                              (((unsigned)(x) & 0x3) << 16)
#define   G_SQ_VTX_WORD2_ENDIAN_SWAP(x)                              (((x) >> 16) & 0x3)
#define   C_SQ_VTX_WORD2_ENDIAN_SWAP                                 0xFFFCFFFF
#define   S_SQ_VTX_WORD2_CONST_BUF_NO_STRIDE(x)                      (((unsigned)(x) & 0x1) << 18)
#define   G_SQ_VTX_WORD2_CONST_BUF_NO_STRIDE(x)                      (((x) >> 18) & 0x1)
#define   C_SQ_VTX_WORD2_CONST_BUF_NO_STRIDE                         0xFFFBFFFF
#define   S_SQ_VTX_WORD2_MEGA_FETCH(x)                               (((unsigned)(x) & 0x1) << 19)
#define   G_SQ_VTX_WORD2_MEGA_FETCH(x)                               (((x) >> 19) & 0x1)
#define   C_SQ_VTX_WORD2_MEGA_FETCH                                  0xFFF7FFFF
#define   S_SQ_VTX_WORD2_ALT_CONST(x)                                (((unsigned)(x) & 0x1) << 20)
#define   G_SQ_VTX_WORD2_ALT_CONST(x)                                (((x) >> 20) & 0x1)
#define   C_SQ_VTX_WORD2_ALT_CONST                                   0xFFEFFFFF
#define   S_SQ_VTX_WORD2_BIM(x)                                      (((unsigned)(x) & 0x3) << 21)
#define   G_SQ_VTX_WORD2_BIM(x)                                      (((x) >> 21) & 0x3)
#define   C_SQ_VTX_WORD2_BIM                                         0xFF9FFFFF
/* done */

#define P_SQ_TEX_WORD0
#define   S_SQ_TEX_WORD0_TEX_INST(x)                                 (((unsigned)(x) & 0x1F) << 0)
#define   G_SQ_TEX_WORD0_TEX_INST(x)                                 (((x) >> 0) & 0x1F)
#define   C_SQ_TEX_WORD0_TEX_INST                                    0xFFFFFFE0
#define   S_SQ_TEX_WORD0_INST_MOD(x)                                 (((unsigned)(x) & 0x3) << 5)
#define   G_SQ_TEX_WORD0_INST_MOD(x)                                 (((x) >> 5) & 0x3)
#define   C_SQ_TEX_WORD0_INST_MOD                                    0xFFFFFF9F
#define   S_SQ_TEX_WORD0_FETCH_WHOLE_QUAD(x)                         (((unsigned)(x) & 0x1) << 7)
#define   G_SQ_TEX_WORD0_FETCH_WHOLE_QUAD(x)                         (((x) >> 7) & 0x1)
#define   C_SQ_TEX_WORD0_FETCH_WHOLE_QUAD                            0xFFFFFF7F
#define   S_SQ_TEX_WORD0_RESOURCE_ID(x)                              (((unsigned)(x) & 0xFF) << 8)
#define   G_SQ_TEX_WORD0_RESOURCE_ID(x)                              (((x) >> 8) & 0xFF)
#define   C_SQ_TEX_WORD0_RESOURCE_ID                                 0xFFFF00FF
#define   S_SQ_TEX_WORD0_SRC_GPR(x)                                  (((unsigned)(x) & 0x7F) << 16)
#define   G_SQ_TEX_WORD0_SRC_GPR(x)                                  (((x) >> 16) & 0x7F)
#define   C_SQ_TEX_WORD0_SRC_GPR                                     0xFF80FFFF
#define   S_SQ_TEX_WORD0_SRC_REL(x)                                  (((unsigned)(x) & 0x1) << 23)
#define   G_SQ_TEX_WORD0_SRC_REL(x)                                  (((x) >> 23) & 0x1)
#define   C_SQ_TEX_WORD0_SRC_REL                                     0xFF7FFFFF
#define   S_SQ_TEX_WORD0_ALT_CONST(x)                                (((unsigned)(x) & 0x1) << 24)
#define   G_SQ_TEX_WORD0_ALT_CONST(x)                                (((x) >> 24) & 0x1)
#define   C_SQ_TEX_WORD0_ALT_CONST                                   0xFEFFFFFF
#define   S_SQ_TEX_WORD0_RIM(x)                                      (((unsigned)(x) & 0x3) << 25)
#define   G_SQ_TEX_WORD0_RIM(x)                                      (((x) >> 25) & 0x3)
#define   C_SQ_TEX_WORD0_RIM                                         0xF9FFFFFF
#define   S_SQ_TEX_WORD0_SIM(x)                                      (((unsigned)(x) & 0x3) << 27)
#define   G_SQ_TEX_WORD0_SIM(x)                                      (((x) >> 27) & 0x3)
#define   C_SQ_TEX_WORD0_SIM                                         0xE7FFFFFF
#define P_SQ_TEX_WORD1
#define   S_SQ_TEX_WORD1_DST_GPR(x)                                  (((unsigned)(x) & 0x7F) << 0)
#define   G_SQ_TEX_WORD1_DST_GPR(x)                                  (((x) >> 0) & 0x7F)
#define   C_SQ_TEX_WORD1_DST_GPR                                     0xFFFFFF80
#define   S_SQ_TEX_WORD1_DST_REL(x)                                  (((unsigned)(x) & 0x1) << 7)
#define   G_SQ_TEX_WORD1_DST_REL(x)                                  (((x) >> 7) & 0x1)
#define   C_SQ_TEX_WORD1_DST_REL                                     0xFFFFFF7F
#define   S_SQ_TEX_WORD1_DST_SEL_X(x)                                (((unsigned)(x) & 0x7) << 9)
#define   G_SQ_TEX_WORD1_DST_SEL_X(x)                                (((x) >> 9) & 0x7)
#define   C_SQ_TEX_WORD1_DST_SEL_X                                   0xFFFFF1FF
#define   S_SQ_TEX_WORD1_DST_SEL_Y(x)                                (((unsigned)(x) & 0x7) << 12)
#define   G_SQ_TEX_WORD1_DST_SEL_Y(x)                                (((x) >> 12) & 0x7)
#define   C_SQ_TEX_WORD1_DST_SEL_Y                                   0xFFFF8FFF
#define   S_SQ_TEX_WORD1_DST_SEL_Z(x)                                (((unsigned)(x) & 0x7) << 15)
#define   G_SQ_TEX_WORD1_DST_SEL_Z(x)                                (((x) >> 15) & 0x7)
#define   C_SQ_TEX_WORD1_DST_SEL_Z                                   0xFFFC7FFF
#define   S_SQ_TEX_WORD1_DST_SEL_W(x)                                (((unsigned)(x) & 0x7) << 18)
#define   G_SQ_TEX_WORD1_DST_SEL_W(x)                                (((x) >> 18) & 0x7)
#define   C_SQ_TEX_WORD1_DST_SEL_W                                   0xFFE3FFFF
#define   S_SQ_TEX_WORD1_LOD_BIAS(x)                                 (((unsigned)(x) & 0x7F) << 21)
#define   G_SQ_TEX_WORD1_LOD_BIAS(x)                                 (((x) >> 21) & 0x7F)
#define   C_SQ_TEX_WORD1_LOD_BIAS                                    0xF01FFFFF
#define   S_SQ_TEX_WORD1_COORD_TYPE_X(x)                             (((unsigned)(x) & 0x1) << 28)
#define   G_SQ_TEX_WORD1_COORD_TYPE_X(x)                             (((x) >> 28) & 0x1)
#define   C_SQ_TEX_WORD1_COORD_TYPE_X                                0xEFFFFFFF
#define     V_SQ_TEX_WORD1_COORD_UNNORMALIZED                        0x00000000
#define     V_SQ_TEX_WORD1_COORD_NORMALIZED                          0x00000001
#define   S_SQ_TEX_WORD1_COORD_TYPE_Y(x)                             (((unsigned)(x) & 0x1) << 29)
#define   G_SQ_TEX_WORD1_COORD_TYPE_Y(x)                             (((x) >> 29) & 0x1)
#define   C_SQ_TEX_WORD1_COORD_TYPE_Y                                0xDFFFFFFF
#define   S_SQ_TEX_WORD1_COORD_TYPE_Z(x)                             (((unsigned)(x) & 0x1) << 30)
#define   G_SQ_TEX_WORD1_COORD_TYPE_Z(x)                             (((x) >> 30) & 0x1)
#define   C_SQ_TEX_WORD1_COORD_TYPE_Z                                0xBFFFFFFF
#define   S_SQ_TEX_WORD1_COORD_TYPE_W(x)                             (((unsigned)(x) & 0x1) << 31)
#define   G_SQ_TEX_WORD1_COORD_TYPE_W(x)                             (((x) >> 31) & 0x1)
#define   C_SQ_TEX_WORD1_COORD_TYPE_W                                0x7FFFFFFF
#define P_SQ_TEX_WORD2
#define   S_SQ_TEX_WORD2_OFFSET_X(x)                                 (((unsigned)(x) & 0x1F) << 0)
#define   G_SQ_TEX_WORD2_OFFSET_X(x)                                 (((x) >> 0) & 0x1F)
#define   C_SQ_TEX_WORD2_OFFSET_X                                    0xFFFFFFE0
#define   S_SQ_TEX_WORD2_OFFSET_Y(x)                                 (((unsigned)(x) & 0x1F) << 5)
#define   G_SQ_TEX_WORD2_OFFSET_Y(x)                                 (((x) >> 5) & 0x1F)
#define   C_SQ_TEX_WORD2_OFFSET_Y                                    0xFFFFFC1F
#define   S_SQ_TEX_WORD2_OFFSET_Z(x)                                 (((unsigned)(x) & 0x1F) << 10)
#define   G_SQ_TEX_WORD2_OFFSET_Z(x)                                 (((x) >> 10) & 0x1F)
#define   C_SQ_TEX_WORD2_OFFSET_Z                                    0xFFFF83FF
#define   S_SQ_TEX_WORD2_SAMPLER_ID(x)                               (((unsigned)(x) & 0x1F) << 15)
#define   G_SQ_TEX_WORD2_SAMPLER_ID(x)                               (((x) >> 15) & 0x1F)
#define   C_SQ_TEX_WORD2_SAMPLER_ID                                  0xFFF07FFF
#define   S_SQ_TEX_WORD2_SRC_SEL_X(x)                                (((unsigned)(x) & 0x7) << 20)
#define   G_SQ_TEX_WORD2_SRC_SEL_X(x)                                (((x) >> 20) & 0x7)
#define   C_SQ_TEX_WORD2_SRC_SEL_X                                   0xFF8FFFFF
#define   S_SQ_TEX_WORD2_SRC_SEL_Y(x)                                (((unsigned)(x) & 0x7) << 23)
#define   G_SQ_TEX_WORD2_SRC_SEL_Y(x)                                (((x) >> 23) & 0x7)
#define   C_SQ_TEX_WORD2_SRC_SEL_Y                                   0xFC7FFFFF
#define   S_SQ_TEX_WORD2_SRC_SEL_Z(x)                                (((unsigned)(x) & 0x7) << 26)
#define   G_SQ_TEX_WORD2_SRC_SEL_Z(x)                                (((x) >> 26) & 0x7)
#define   C_SQ_TEX_WORD2_SRC_SEL_Z                                   0xE3FFFFFF
#define   S_SQ_TEX_WORD2_SRC_SEL_W(x)                                (((unsigned)(x) & 0x7) << 29)
#define   G_SQ_TEX_WORD2_SRC_SEL_W(x)                                (((x) >> 29) & 0x7)
#define   C_SQ_TEX_WORD2_SRC_SEL_W                                   0x1FFFFFFF

#define P_SQ_MEM_GDS_WORD0
#define   S_SQ_MEM_GDS_WORD0_MEM_INST(x)                             (((unsigned)(x) & 0x1f) << 0)
#define   S_SQ_MEM_GDS_WORD0_MEM_OP(x)                               (((unsigned)(x) & 0x7) << 8)
#define   S_SQ_MEM_GDS_WORD0_SRC_GPR(x)                              (((unsigned)(x) & 0x7f) << 11)
#define   S_SQ_MEM_GDS_WORD0_SRC_REL(x)                              (((unsigned)(x) & 0x3) << 18)
#define   S_SQ_MEM_GDS_WORD0_SRC_SEL_X(x)                            (((unsigned)(x) & 0x7) << 20)
#define   S_SQ_MEM_GDS_WORD0_SRC_SEL_Y(x)                            (((unsigned)(x) & 0x7) << 23)
#define   S_SQ_MEM_GDS_WORD0_SRC_SEL_Z(x)                            (((unsigned)(x) & 0x7) << 26)

#define P_SQ_MEM_GDS_WORD1
#define   S_SQ_MEM_GDS_WORD1_DST_GPR(x)                              (((unsigned)(x) & 0x7f) << 0)
#define   S_SQ_MEM_GDS_WORD1_DST_REL(x)                              (((unsigned)(x) & 0x3) << 7)
#define   S_SQ_MEM_GDS_WORD1_GDS_OP(x)                               (((unsigned)(x) & 0x3f) << 9)
#define   S_SQ_MEM_GDS_WORD1_SRC_GPR(x)                              (((unsigned)(x) & 0x7f) << 16)
#define   S_SQ_MEM_GDS_WORD1_UAV_INDEX_MODE(x)                       (((unsigned)(x) & 0x3) << 24)
#define   S_SQ_MEM_GDS_WORD1_UAV_ID(x)                               (((unsigned)(x) & 0xf) << 26)
#define   S_SQ_MEM_GDS_WORD1_ALLOC_CONSUME(x)                        (((unsigned)(x) & 0x1) << 30)
#define   S_SQ_MEM_GDS_WORD1_BCAST_FIRST_REQ(x)                      (((unsigned)(x) & 0x1) << 31)

#define P_SQ_MEM_GDS_WORD2
#define   S_SQ_MEM_GDS_WORD2_DST_SEL_X(x)                            (((unsigned)(x) & 0x7) << 0)
#define   S_SQ_MEM_GDS_WORD2_DST_SEL_Y(x)                            (((unsigned)(x) & 0x7) << 3)
#define   S_SQ_MEM_GDS_WORD2_DST_SEL_Z(x)                            (((unsigned)(x) & 0x7) << 6)
#define   S_SQ_MEM_GDS_WORD2_DST_SEL_W(x)                            (((unsigned)(x) & 0x7) << 9)

/* LDS IDX redefines the neg bits on op3 */
#define S_SQ_ALU_WORD0_LDS_IDX_OP_IDX_OFFSET_4(x)                    (((unsigned)(x) & 0x1) << 12)
#define S_SQ_ALU_WORD0_LDS_IDX_OP_IDX_OFFSET_5(x)                    (((unsigned)(x) & 0x1) << 25)

/* src2 neg */
#define S_SQ_ALU_WORD1_LDS_IDX_OP_IDX_OFFSET_1(x)                    (((unsigned)(x) & 0x1) << 12)

/* this was dst gpr */
#define S_SQ_ALU_WORD1_LDS_IDX_OP_LDS_OP(x)                    (((unsigned)(x) & 0x3f) << 21)
#define S_SQ_ALU_WORD1_LDS_IDX_OP_IDX_OFFSET_0(x)                    (((unsigned)(x) & 0x1) << 27)

/* this was dst rel */
#define S_SQ_ALU_WORD1_LDS_IDX_OP_IDX_OFFSET_2(x)                    (((unsigned)(x) & 0x1) << 28)
/* this was clamp */
#define S_SQ_ALU_WORD1_LDS_IDX_OP_IDX_OFFSET_3(x)                    (((unsigned)(x) & 0x1) << 31)

#define     EG_V_SQ_ALU_SRC_LDS_OQ_A_POP                             0x000000DD
#define     EG_V_SQ_ALU_SRC_LDS_OQ_B_POP                             0x000000DE


#define V_SQ_LDS_INST_ADD                               0x00
#define V_SQ_LDS_INST_SUB                               0x01
#define V_SQ_LDS_INST_RSUB                              0x02

#define V_SQ_LDS_INST_INC                               0x03
#define V_SQ_LDS_INST_DEC                               0x04
#define V_SQ_LDS_INST_MIN_INT                           0x05
#define V_SQ_LDS_INST_MAX_INT                           0x06
#define V_SQ_LDS_INST_MIN_UINT                          0x07
#define V_SQ_LDS_INST_MAX_UINT                          0x08
#define V_SQ_LDS_INST_AND                               0x09
#define V_SQ_LDS_INST_OR                                0x0a
#define V_SQ_LDS_INST_XOR                               0x0b
#define V_SQ_LDS_INST_MSKOR                             0x0c
#define V_SQ_LDS_INST_WRITE                             0x0d
#define V_SQ_LDS_INST_WRITE_REL                         0x0e
#define V_SQ_LDS_INST_WRITE2                            0x0f

#define V_SQ_LDS_INST_READ_RET                          0x32
#define V_SQ_LDS_INST_READ_REL_RET                      0x33
#define V_SQ_LDS_INST_READ2_RET                         0x34
#define V_SQ_LDS_INST_READWRITE_RET                     0x35

#define V_SQ_CF_COND_ACTIVE                             0x00
#define V_SQ_CF_COND_FALSE                              0x01
#define V_SQ_CF_COND_BOOL                               0x02
#define V_SQ_CF_COND_NOT_BOOL                           0x03

#define V_SQ_REL_ABSOLUTE 0
#define V_SQ_REL_RELATIVE 1

/* CAYMAN has special encoding for MOVA_INT destination */
#define CM_V_SQ_MOVA_DST_AR_X 0
#define CM_V_SQ_MOVA_DST_CF_PC 1
#define CM_V_SQ_MOVA_DST_CF_IDX0 2
#define CM_V_SQ_MOVA_DST_CF_IDX1 3

#endif
