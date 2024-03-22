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
#ifndef R700_SQ_H
#define R700_SQ_H

#define P_SQ_CF_WORD0
#define   S_SQ_CF_WORD0_ADDR(x)                                      (((unsigned)(x) & 0xFFFFFFFF) << 0)
#define   G_SQ_CF_WORD0_ADDR(x)                                      (((x) >> 0) & 0xFFFFFFFF)
#define   C_SQ_CF_WORD0_ADDR                                         0x00000000
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
#define   S_SQ_CF_WORD1_COUNT(x)                                     (((unsigned)(x) & 0x7) << 10)
#define   G_SQ_CF_WORD1_COUNT(x)                                     (((x) >> 10) & 0x7)
#define   C_SQ_CF_WORD1_COUNT                                        0xFFFFE3FF
#define   S_SQ_CF_WORD1_CALL_COUNT(x)                                (((unsigned)(x) & 0x3F) << 13)
#define   G_SQ_CF_WORD1_CALL_COUNT(x)                                (((x) >> 13) & 0x3F)
#define   C_SQ_CF_WORD1_CALL_COUNT                                   0xFFF81FFF
#define   S_SQ_CF_WORD1_END_OF_PROGRAM(x)                            (((unsigned)(x) & 0x1) << 21)
#define   G_SQ_CF_WORD1_END_OF_PROGRAM(x)                            (((x) >> 21) & 0x1)
#define   C_SQ_CF_WORD1_END_OF_PROGRAM                               0xFFDFFFFF
#define   S_SQ_CF_WORD1_VALID_PIXEL_MODE(x)                          (((unsigned)(x) & 0x1) << 22)
#define   G_SQ_CF_WORD1_VALID_PIXEL_MODE(x)                          (((x) >> 22) & 0x1)
#define   C_SQ_CF_WORD1_VALID_PIXEL_MODE                             0xFFBFFFFF
#define   S_SQ_CF_WORD1_CF_INST(x)                                   (((unsigned)(x) & 0x7F) << 23)
#define   G_SQ_CF_WORD1_CF_INST(x)                                   (((x) >> 23) & 0x7F)
#define   C_SQ_CF_WORD1_CF_INST                                      0xC07FFFFF
#define   S_SQ_CF_WORD1_WHOLE_QUAD_MODE(x)                           (((unsigned)(x) & 0x1) << 30)
#define   G_SQ_CF_WORD1_WHOLE_QUAD_MODE(x)                           (((x) >> 30) & 0x1)
#define   C_SQ_CF_WORD1_WHOLE_QUAD_MODE                              0xBFFFFFFF
#define   S_SQ_CF_WORD1_BARRIER(x)                                   (((unsigned)(x) & 0x1) << 31)
#define   G_SQ_CF_WORD1_BARRIER(x)                                   (((x) >> 31) & 0x1)
#define   C_SQ_CF_WORD1_BARRIER                                      0x7FFFFFFF
#define   S_SQ_CF_WORD1_COUNT_3(x)                                   (((unsigned)(x) & 0x1) << 19)
#define   G_SQ_CF_WORD1_COUNT_3(x)                                   (((x) >> 19) & 0x1)
#define   C_SQ_CF_WORD1_COUNT_3                                      0xFFF7FFFF
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
#define   S_SQ_CF_ALU_WORD1_USES_WATERFALL(x)                        (((unsigned)(x) & 0x1) << 25)
#define   G_SQ_CF_ALU_WORD1_USES_WATERFALL(x)                        (((x) >> 25) & 0x1)
#define   C_SQ_CF_ALU_WORD1_USES_WATERFALL                           0xFDFFFFFF
#define   S_SQ_CF_ALU_WORD1_CF_INST(x)                               (((unsigned)(x) & 0xF) << 26)
#define   G_SQ_CF_ALU_WORD1_CF_INST(x)                               (((x) >> 26) & 0xF)
#define   C_SQ_CF_ALU_WORD1_CF_INST                                  0xC3FFFFFF
#define   S_SQ_CF_ALU_WORD1_WHOLE_QUAD_MODE(x)                       (((unsigned)(x) & 0x1) << 30)
#define   G_SQ_CF_ALU_WORD1_WHOLE_QUAD_MODE(x)                       (((x) >> 30) & 0x1)
#define   C_SQ_CF_ALU_WORD1_WHOLE_QUAD_MODE                          0xBFFFFFFF
#define   S_SQ_CF_ALU_WORD1_BARRIER(x)                               (((unsigned)(x) & 0x1) << 31)
#define   G_SQ_CF_ALU_WORD1_BARRIER(x)                               (((x) >> 31) & 0x1)
#define   C_SQ_CF_ALU_WORD1_BARRIER                                  0x7FFFFFFF
#define   S_SQ_CF_ALU_WORD1_ALT_CONST(x)                             (((unsigned)(x) & 0x1) << 25)
#define   G_SQ_CF_ALU_WORD1_ALT_CONST(x)                             (((x) >> 25) & 0x1)
#define   C_SQ_CF_ALU_WORD1_ALT_CONST                                0xFDFFFFFF
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
#define     V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_SX                  0x00000003
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
#define P_SQ_CF_ALLOC_EXPORT_WORD1
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_BURST_COUNT(x)                  (((unsigned)(x) & 0xF) << 17)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_BURST_COUNT(x)                  (((x) >> 17) & 0xF)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_BURST_COUNT                     0xFFE1FFFF
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM(x)               (((unsigned)(x) & 0x1) << 21)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM(x)               (((x) >> 21) & 0x1)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM                  0xFFDFFFFF
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_VALID_PIXEL_MODE(x)             (((unsigned)(x) & 0x1) << 22)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_VALID_PIXEL_MODE(x)             (((x) >> 22) & 0x1)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_VALID_PIXEL_MODE                0xFFBFFFFF
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST(x)                      (((unsigned)(x) & 0x7F) << 23)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST(x)                      (((x) >> 23) & 0x7F)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST                         0xC07FFFFF
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_WHOLE_QUAD_MODE(x)              (((unsigned)(x) & 0x1) << 30)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_WHOLE_QUAD_MODE(x)              (((x) >> 30) & 0x1)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_WHOLE_QUAD_MODE                 0xBFFFFFFF
#define   S_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER(x)                      (((unsigned)(x) & 0x1) << 31)
#define   G_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER(x)                      (((x) >> 31) & 0x1)
#define   C_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER                         0x7FFFFFFF
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
#define P_SQ_ALU_WORD0
#define   S_SQ_ALU_WORD0_SRC0_SEL(x)                                 (((unsigned)(x) & 0x1FF) << 0)
#define   G_SQ_ALU_WORD0_SRC0_SEL(x)                                 (((x) >> 0) & 0x1FF)
#define   C_SQ_ALU_WORD0_SRC0_SEL                                    0xFFFFFE00
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
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_ADD                       0x00000000
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MUL                       0x00000001
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MUL_IEEE                  0x00000002
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MAX                       0x00000003
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MIN                       0x00000004
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MAX_DX10                  0x00000005
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MIN_DX10                  0x00000006
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETE                      0x00000008
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETGT                     0x00000009
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETGE                     0x0000000A
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETNE                     0x0000000B
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETE_DX10                 0x0000000C
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETGT_DX10                0x0000000D
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETGE_DX10                0x0000000E
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETNE_DX10                0x0000000F
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_FRACT                     0x00000010
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_TRUNC                     0x00000011
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_CEIL                      0x00000012
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_RNDNE                     0x00000013
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_FLOOR                     0x00000014
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MOVA                      0x00000015
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MOVA_FLOOR                0x00000016
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MOVA_INT                  0x00000018
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MOV                       0x00000019
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_NOP                       0x0000001A
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETGT_UINT           0x0000001E
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETGE_UINT           0x0000001F
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETE                 0x00000020
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETGT                0x00000021
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETGE                0x00000022
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETNE                0x00000023
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SET_INV              0x00000024
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SET_POP              0x00000025
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SET_CLR              0x00000026
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SET_RESTORE          0x00000027
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETE_PUSH            0x00000028
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETGT_PUSH           0x00000029
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETGE_PUSH           0x0000002A
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETNE_PUSH           0x0000002B
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_KILLE                     0x0000002C
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_KILLGT                    0x0000002D
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_KILLGE                    0x0000002E
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_KILLNE                    0x0000002F
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_AND_INT                   0x00000030
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_OR_INT                    0x00000031
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_XOR_INT                   0x00000032
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_NOT_INT                   0x00000033
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_ADD_INT                   0x00000034
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SUB_INT                   0x00000035
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MAX_INT                   0x00000036
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MIN_INT                   0x00000037
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MAX_UINT                  0x00000038
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MIN_UINT                  0x00000039
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETE_INT                  0x0000003A
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETGT_INT                 0x0000003B
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETGE_INT                 0x0000003C
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETNE_INT                 0x0000003D
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETGT_UINT                0x0000003E
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SETGE_UINT                0x0000003F
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_KILLGT_UINT               0x00000040
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_KILLGE_UINT               0x00000041
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETE_INT             0x00000042
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETGT_INT            0x00000043
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETGE_INT            0x00000044
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETNE_INT            0x00000045
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_KILLE_INT                 0x00000046
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_KILLGT_INT                0x00000047
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_KILLGE_INT                0x00000048
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_KILLNE_INT                0x00000049
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETE_PUSH_INT        0x0000004A
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETGT_PUSH_INT       0x0000004B
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETGE_PUSH_INT       0x0000004C
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETNE_PUSH_INT       0x0000004D
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETLT_PUSH_INT       0x0000004E
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_PRED_SETLE_PUSH_INT       0x0000004F
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_DOT4                      0x00000050
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_DOT4_IEEE                 0x00000051
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_CUBE                      0x00000052
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MAX4                      0x00000053
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MOVA_GPR_INT              0x00000060
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_EXP_IEEE                  0x00000061
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_LOG_CLAMPED               0x00000062
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_LOG_IEEE                  0x00000063
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_RECIP_CLAMPED             0x00000064
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_RECIP_FF                  0x00000065
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_RECIP_IEEE                0x00000066
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_RECIPSQRT_CLAMPED         0x00000067
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_RECIPSQRT_FF              0x00000068
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_RECIPSQRT_IEEE            0x00000069
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SQRT_IEEE                 0x0000006A
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_FLT_TO_INT                0x0000006B
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_INT_TO_FLT                0x0000006C
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_UINT_TO_FLT               0x0000006D
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_SIN                       0x0000006E
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_COS                       0x0000006F
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_ASHR_INT                  0x00000070
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_LSHR_INT                  0x00000071
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_LSHL_INT                  0x00000072
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MULLO_INT                 0x00000073
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MULHI_INT                 0x00000074
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MULLO_UINT                0x00000075
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_MULHI_UINT                0x00000076
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_RECIP_INT                 0x00000077
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_RECIP_UINT                0x00000078
#define     V_SQ_ALU_WORD1_OP2_SQ_OP2_INST_FLT_TO_UINT               0x00000079
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
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_MUL_LIT                   0x0000000C
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_MUL_LIT_M2                0x0000000D
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_MUL_LIT_M4                0x0000000E
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_MUL_LIT_D2                0x0000000F
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_MULADD                    0x00000010
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_MULADD_M2                 0x00000011
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_MULADD_M4                 0x00000012
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_MULADD_D2                 0x00000013
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_MULADD_IEEE               0x00000014
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_MULADD_IEEE_M2            0x00000015
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_MULADD_IEEE_M4            0x00000016
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_MULADD_IEEE_D2            0x00000017
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_CNDE                      0x00000018
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_CNDGT                     0x00000019
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_CNDGE                     0x0000001A
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_CNDE_INT                  0x0000001C
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_CNDGT_INT                 0x0000001D
#define     V_SQ_ALU_WORD1_OP3_SQ_OP3_INST_CNDGE_INT                 0x0000001E
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
#define P_SQ_TEX_WORD0
#define   S_SQ_TEX_WORD0_TEX_INST(x)                                 (((unsigned)(x) & 0x1F) << 0)
#define   G_SQ_TEX_WORD0_TEX_INST(x)                                 (((x) >> 0) & 0x1F)
#define   C_SQ_TEX_WORD0_TEX_INST                                    0xFFFFFFE0
#define   S_SQ_TEX_WORD0_BC_FRAC_MODE(x)                             (((unsigned)(x) & 0x1) << 5)
#define   G_SQ_TEX_WORD0_BC_FRAC_MODE(x)                             (((x) >> 5) & 0x1)
#define   C_SQ_TEX_WORD0_BC_FRAC_MODE                                0xFFFFFFDF
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

#define P_SQ_MEM_RD_WORD0
#define   S_SQ_MEM_RD_WORD0_MEM_INST(x)                              (((x) & 0x1F) << 0)
#define   S_SQ_MEM_RD_WORD0_ELEM_SIZE(x)                             (((x) & 0x3) << 5)
#define   S_SQ_MEM_RD_WORD0_FETCH_WHOLE_QUAD(x)                      (((x) & 0x1) << 7)
#define   S_SQ_MEM_RD_WORD0_MEM_OP(x)                                (((x) & 0x7) << 8)
#define   S_SQ_MEM_RD_WORD0_UNCACHED(x)                              (((x) & 0x1) << 11)
#define   S_SQ_MEM_RD_WORD0_INDEXED(x)                               (((x) & 0x1) << 12)
#define   S_SQ_MEM_RD_WORD0_SRC_SEL_Y(x)                             (((x) & 0x3) << 13)
#define   S_SQ_MEM_RD_WORD0_SRC_GPR(x)                               (((x) & 0x7F) << 16)
#define   S_SQ_MEM_RD_WORD0_SRC_REL(x)                               (((x) & 0x1) << 23)
#define   S_SQ_MEM_RD_WORD0_SRC_SEL_X(x)                             (((x) & 0x3) << 24)
#define   S_SQ_MEM_RD_WORD0_BURST_COUNT(x)                           (((x) & 0xF) << 26)
#define   S_SQ_MEM_RD_WORD0_LDS_REQ(x)                               (((x) & 0x1) << 30)
#define   S_SQ_MEM_RD_WORD0_COALESCED_READ(x)                        (((x) & 0x1) << 31)
#define P_SQ_MEM_RD_WORD1
#define   S_SQ_MEM_RD_WORD1_DST_GPR(x)                               (((x) & 0x7f) << 0)
#define   S_SQ_MEM_RD_WORD1_DST_REL(x)                               (((x) & 0x1) << 7)
#define   S_SQ_MEM_RD_WORD1_DST_SEL_X(x)                             (((x) & 0x7) << 9)
#define   S_SQ_MEM_RD_WORD1_DST_SEL_Y(x)                             (((x) & 0x7) << 12)
#define   S_SQ_MEM_RD_WORD1_DST_SEL_Z(x)                             (((x) & 0x7) << 15)
#define   S_SQ_MEM_RD_WORD1_DST_SEL_W(x)                             (((x) & 0x7) << 18)
#define   S_SQ_MEM_RD_WORD1_DATA_FORMAT(x)                           (((x) & 0x3F) << 22)
#define   S_SQ_MEM_RD_WORD1_NUM_FORMAT_ALL(x)                        (((x) & 0x3) << 28)
#define   S_SQ_MEM_RD_WORD1_FORMAT_COMP_ALL(x)                       (((x) & 0x1) << 30)
#define   S_SQ_MEM_RD_WORD1_SRF_MODE_ALL(x)                          (((x) & 0x1) << 31)
#define P_SQ_MEM_RD_WORD2
#define   S_SQ_MEM_RD_WORD2_ARRAY_BASE(x)                            (((x) & 0x1FFF) << 0)
#define   S_SQ_MEM_RD_WORD2_ENDIAN_SWAP(x)                           (((x) & 0x3) << 16)
#define   S_SQ_MEM_RD_WORD2_ARRAY_SIZE(x)                            (((x) & 0xFFF) << 20)

#endif
