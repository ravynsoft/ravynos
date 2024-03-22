/*
 * Copyright 2012 Vadim Girlin <vadimgirlin@gmail.com>
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
 *      Vadim Girlin
 */

#include "r600_isa.h"

#include "util/macros.h"

#include <assert.h>
#include <stdlib.h>

const struct alu_op_info r600_alu_op_table[] = {
		{"ADD",                       2, { 0x00, 0x00 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC },
		{"MUL",                       2, { 0x01, 0x01 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC },
		{"MUL_IEEE",                  2, { 0x02, 0x02 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC | AF_IEEE },
		{"MAX",                       2, { 0x03, 0x03 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC },
		{"MIN",                       2, { 0x04, 0x04 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC },
		{"MAX_DX10",                  2, { 0x05, 0x05 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC | AF_DX10 },
		{"MIN_DX10",                  2, { 0x06, 0x06 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC | AF_DX10 },
		{"SETE",                      2, { 0x08, 0x08 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_E },
		{"SETGT",                     2, { 0x09, 0x09 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_GT },
		{"SETGE",                     2, { 0x0A, 0x0A },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_GE },
		{"SETNE",                     2, { 0x0B, 0x0B },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_NE },
		{"SETE_DX10",                 2, { 0x0C, 0x0C },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_E | AF_DX10 | AF_INT_DST },
		{"SETGT_DX10",                2, { 0x0D, 0x0D },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_GT | AF_DX10 | AF_INT_DST },
		{"SETGE_DX10",                2, { 0x0E, 0x0E },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_GE | AF_DX10 | AF_INT_DST },
		{"SETNE_DX10",                2, { 0x0F, 0x0F },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_NE | AF_DX10 | AF_INT_DST },
		{"FRACT",                     1, { 0x10, 0x10 },{  AF_VS, AF_VS, AF_VS, AF_VS},  0 },
		{"TRUNC",                     1, { 0x11, 0x11 },{  AF_VS, AF_VS, AF_VS, AF_VS},  0 },
		{"CEIL",                      1, { 0x12, 0x12 },{  AF_VS, AF_VS, AF_VS, AF_VS},  0 },
		{"RNDNE",                     1, { 0x13, 0x13 },{  AF_VS, AF_VS, AF_VS, AF_VS},  0 },
		{"FLOOR",                     1, { 0x14, 0x14 },{  AF_VS, AF_VS, AF_VS, AF_VS},  0 },
		{"ASHR_INT",                  2, { 0x70, 0x15 },{   AF_S, AF_VS, AF_VS, AF_VS},  AF_INT_DST },
		{"LSHR_INT",                  2, { 0x71, 0x16 },{   AF_S, AF_VS, AF_VS, AF_VS},  AF_INT_DST },
		{"LSHL_INT",                  2, { 0x72, 0x17 },{   AF_S, AF_VS, AF_VS, AF_VS},  AF_INT_DST },
		{"MOV",                       1, { 0x19, 0x19 },{  AF_VS, AF_VS, AF_VS, AF_VS},  0 },
		{"ALU_NOP",                   0, { 0x1A, 0x1A },{  AF_VS, AF_VS, AF_VS, AF_VS},  0 },
		{"PRED_SETGT_UINT",           2, { 0x1E, 0x1E },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED | AF_CC_GT | AF_UINT_CMP },
		{"PRED_SETGE_UINT",           2, { 0x1F, 0x1F },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED | AF_CC_GE | AF_UINT_CMP },
		{"PRED_SETE",                 2, { 0x20, 0x20 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED | AF_CC_E },
		{"PRED_SETGT",                2, { 0x21, 0x21 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED | AF_CC_GT },
		{"PRED_SETGE",                2, { 0x22, 0x22 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED | AF_CC_GE },
		{"PRED_SETNE",                2, { 0x23, 0x23 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED | AF_CC_NE },
		{"PRED_SET_INV",              1, { 0x24, 0x24 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED },
		{"PRED_SET_POP",              2, { 0x25, 0x25 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED },
		{"PRED_SET_CLR",              0, { 0x26, 0x26 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED },
		{"PRED_SET_RESTORE",          1, { 0x27, 0x27 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED },
		{"PRED_SETE_PUSH",            2, { 0x28, 0x28 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED_PUSH | AF_CC_E },
		{"PRED_SETGT_PUSH",           2, { 0x29, 0x29 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED_PUSH | AF_CC_GT },
		{"PRED_SETGE_PUSH",           2, { 0x2A, 0x2A },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED_PUSH | AF_CC_GE },
		{"PRED_SETNE_PUSH",           2, { 0x2B, 0x2B },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED_PUSH | AF_CC_NE },
		{"KILLE",                     2, { 0x2C, 0x2C },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_KILL | AF_CC_E },
		{"KILLGT",                    2, { 0x2D, 0x2D },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_KILL | AF_CC_GT },
		{"KILLGE",                    2, { 0x2E, 0x2E },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_KILL | AF_CC_GE },
		{"KILLNE",                    2, { 0x2F, 0x2F },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_KILL | AF_CC_NE },
		{"AND_INT",                   2, { 0x30, 0x30 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC | AF_INT_DST },
		{"OR_INT",                    2, { 0x31, 0x31 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC | AF_INT_DST },
		{"XOR_INT",                   2, { 0x32, 0x32 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC | AF_INT_DST },
		{"NOT_INT",                   1, { 0x33, 0x33 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_INT_DST },
		{"ADD_INT",                   2, { 0x34, 0x34 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC | AF_INT_DST },
		{"SUB_INT",                   2, { 0x35, 0x35 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_INT_DST },
		{"MAX_INT",                   2, { 0x36, 0x36 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC | AF_INT_DST },
		{"MIN_INT",                   2, { 0x37, 0x37 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC | AF_INT_DST },
		{"MAX_UINT",                  2, { 0x38, 0x38 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC | AF_UINT_DST },
		{"MIN_UINT",                  2, { 0x39, 0x39 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC | AF_UINT_DST },
		{"SETE_INT",                  2, { 0x3A, 0x3A },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_E | AF_INT_DST | AF_INT_CMP },
		{"SETGT_INT",                 2, { 0x3B, 0x3B },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_GT | AF_INT_DST | AF_INT_CMP },
		{"SETGE_INT",                 2, { 0x3C, 0x3C },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_GE | AF_INT_DST | AF_INT_CMP },
		{"SETNE_INT",                 2, { 0x3D, 0x3D },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_NE | AF_INT_DST | AF_INT_CMP },
		{"SETGT_UINT",                2, { 0x3E, 0x3E },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_GT | AF_UINT_DST | AF_UINT_CMP },
		{"SETGE_UINT",                2, { 0x3F, 0x3F },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_SET | AF_CC_GE | AF_UINT_DST | AF_UINT_CMP },
		{"KILLGT_UINT",               2, { 0x40, 0x40 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_KILL | AF_CC_GT | AF_UINT_CMP },
		{"KILLGE_UINT",               2, { 0x41, 0x41 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_KILL | AF_CC_GE | AF_UINT_CMP },
		{"PRED_SETE_INT",             2, { 0x42, 0x42 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED | AF_CC_E | AF_INT_CMP },
		{"PRED_SETGT_INT",            2, { 0x43, 0x43 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED | AF_CC_GT | AF_INT_CMP },
		{"PRED_SETGE_INT",            2, { 0x44, 0x44 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED | AF_CC_GE | AF_INT_CMP },
		{"PRED_SETNE_INT",            2, { 0x45, 0x45 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED | AF_CC_NE | AF_INT_CMP },
		{"KILLE_INT",                 2, { 0x46, 0x46 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_KILL | AF_CC_E | AF_INT_CMP },
		{"KILLGT_INT",                2, { 0x47, 0x47 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_KILL | AF_CC_GT | AF_INT_CMP },
		{"KILLGE_INT",                2, { 0x48, 0x48 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_KILL | AF_CC_GE | AF_INT_CMP },
		{"KILLNE_INT",                2, { 0x49, 0x49 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_KILL | AF_CC_NE | AF_INT_CMP },
		{"PRED_SETE_PUSH_INT",        2, { 0x4A, 0x4A },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED_PUSH | AF_CC_E | AF_INT_CMP },
		{"PRED_SETGT_PUSH_INT",       2, { 0x4B, 0x4B },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED_PUSH | AF_CC_GT | AF_INT_CMP },
		{"PRED_SETGE_PUSH_INT",       2, { 0x4C, 0x4C },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED_PUSH | AF_CC_GE | AF_INT_CMP },
		{"PRED_SETNE_PUSH_INT",       2, { 0x4D, 0x4D },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED_PUSH | AF_CC_NE | AF_INT_CMP },
		{"PRED_SETLT_PUSH_INT",       2, { 0x4E, 0x4E },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED_PUSH | AF_CC_LT | AF_INT_CMP },
		{"PRED_SETLE_PUSH_INT",       2, { 0x4F, 0x4F },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_PRED_PUSH | AF_CC_LE | AF_INT_CMP },
		{"FLT_TO_INT",                1, { 0x6B, 0x50 },{   AF_S,  AF_S,  AF_V,  AF_V},  AF_INT_DST | AF_CVT },
		{"BFREV_INT",                 1, {   -1, 0x51 },{      0,     0, AF_VS, AF_VS},  AF_INT_DST },
		{"ADDC_UINT",                 2, {   -1, 0x52 },{      0,     0, AF_VS, AF_VS},  AF_UINT_DST },
		{"SUBB_UINT",                 2, {   -1, 0x53 },{      0,     0, AF_VS, AF_VS},  AF_UINT_DST },
		{"GROUP_BARRIER",             0, {   -1, 0x54 },{      0,     0, AF_VS, AF_VS},  0 },
		{"GROUP_SEQ_BEGIN",           0, {   -1, 0x55 },{      0,     0, AF_VS,     0},  0 },
		{"GROUP_SEQ_END",             0, {   -1, 0x56 },{      0,     0, AF_VS,     0},  0 },
		{"SET_MODE",                  2, {   -1, 0x57 },{      0,     0, AF_VS, AF_VS},  0 },
		{"SET_CF_IDX0",               0, {   -1, 0x58 },{      0,     0, AF_VS,     0},  0 },
		{"SET_CF_IDX1",               0, {   -1, 0x59 },{      0,     0, AF_VS,     0},  0 },
		{"SET_LDS_SIZE",              2, {   -1, 0x5A },{      0,     0, AF_VS, AF_VS},  0 },
		{"MUL_INT24",                 2, {   -1, 0x5B },{      0,     0,     0,  AF_V},  AF_INT_DST | AF_24 },
		{"MULHI_INT24",               2, {   -1, 0x5C },{      0,     0,     0,  AF_V},  AF_INT_DST | AF_24 },
		{"FLT_TO_INT_TRUNC",          1, {   -1, 0x5D },{      0,     0,     0,  AF_V},  AF_INT_DST | AF_CVT},
		{"EXP_IEEE",                  1, { 0x61, 0x81 },{   AF_S,  AF_S,  AF_S,  AF_S},  AF_IEEE },
		{"LOG_CLAMPED",               1, { 0x62, 0x82 },{   AF_S,  AF_S,  AF_S,  AF_S},  0 },
		{"LOG_IEEE",                  1, { 0x63, 0x83 },{   AF_S,  AF_S,  AF_S,  AF_S},  AF_IEEE },
		{"RECIP_CLAMPED",             1, { 0x64, 0x84 },{   AF_S,  AF_S,  AF_S,  AF_S},  0 },
		{"RECIP_FF",                  1, { 0x65, 0x85 },{   AF_S,  AF_S,  AF_S,  AF_S},  0 },
		{"RECIP_IEEE",                1, { 0x66, 0x86 },{   AF_S,  AF_S,  AF_S,  AF_S},  AF_IEEE },
		{"RECIPSQRT_CLAMPED",         1, { 0x67, 0x87 },{   AF_S,  AF_S,  AF_S,  AF_S},  0 },
		{"RECIPSQRT_FF",              1, { 0x68, 0x88 },{   AF_S,  AF_S,  AF_S,  AF_S},  0 },
		{"RECIPSQRT_IEEE",            1, { 0x69, 0x89 },{   AF_S,  AF_S,  AF_S,  AF_S},  AF_IEEE },
		{"SQRT_IEEE",                 1, { 0x6A, 0x8A },{   AF_S,  AF_S,  AF_S,  AF_S},  AF_IEEE },
		{"SIN",                       1, { 0x6E, 0x8D },{   AF_S,  AF_S,  AF_S,  AF_S},  0 },
		{"COS",                       1, { 0x6F, 0x8E },{   AF_S,  AF_S,  AF_S,  AF_S},  0 },
		{"MULLO_INT",                 2, { 0x73, 0x8F },{   AF_S,  AF_S,  AF_S,  AF_4V},  AF_M_COMM | AF_INT_DST | AF_REPL},
		{"MULHI_INT",                 2, { 0x74, 0x90 },{   AF_S,  AF_S,  AF_S,  AF_4V},  AF_M_COMM | AF_INT_DST | AF_REPL},
		{"MULLO_UINT",                2, { 0x75, 0x91 },{   AF_S,  AF_S,  AF_S,  AF_4V},  AF_M_COMM | AF_UINT_DST | AF_REPL},
		{"MULHI_UINT",                2, { 0x76, 0x92 },{   AF_S,  AF_S,  AF_S,  AF_4V},  AF_M_COMM | AF_UINT_DST | AF_REPL},
		{"RECIP_INT",                 1, { 0x77, 0x93 },{   AF_S,  AF_S,  AF_S,     0},  AF_INT_DST },
		{"RECIP_UINT",                1, { 0x78, 0x94 },{   AF_S,  AF_S,  AF_S,     0},  AF_UINT_DST },
		{"RECIP_64",                  2, {   -1, 0x95 },{      0,     0,  AF_S,  AF_S},  AF_64 },
		{"RECIP_CLAMPED_64",          2, {   -1, 0x96 },{      0,     0,  AF_S,  AF_S},  AF_64 },
		{"RECIPSQRT_64",              2, {   -1, 0x97 },{      0,     0,  AF_S,  AF_S},  AF_64 },
		{"RECIPSQRT_CLAMPED_64",      2, {   -1, 0x98 },{      0,     0,  AF_S,  AF_S},  AF_64 },
		{"SQRT_64",                   2, {   -1, 0x99 },{      0,     0,  AF_S,  AF_S},  AF_64 },
		{"FLT_TO_UINT",               1, { 0x79, 0x9A },{   AF_S,  AF_S,  AF_S,  AF_V},  AF_UINT_DST | AF_CVT},
		{"INT_TO_FLT",                1, { 0x6C, 0x9B },{   AF_S,  AF_S,  AF_S,  AF_V},  AF_CVT},
		{"UINT_TO_FLT",               1, { 0x6D, 0x9C },{   AF_S,  AF_S,  AF_S,  AF_V},  AF_CVT },
		{"BFM_INT",                   2, {   -1, 0xA0 },{      0,     0,  AF_V,  AF_V},  AF_INT_DST },
		{"FLT32_TO_FLT16",            1, {   -1, 0xA2 },{      0,     0,  AF_V,  AF_V},  0 },
		{"FLT16_TO_FLT32",            1, {   -1, 0xA3 },{      0,     0,  AF_V,  AF_V},  0 },
		{"UBYTE0_FLT",                1, {   -1, 0xA4 },{      0,     0,  AF_V,  AF_V},  0 },
		{"UBYTE1_FLT",                1, {   -1, 0xA5 },{      0,     0,  AF_V,  AF_V},  0 },
		{"UBYTE2_FLT",                1, {   -1, 0xA6 },{      0,     0,  AF_V,  AF_V},  0 },
		{"UBYTE3_FLT",                1, {   -1, 0xA7 },{      0,     0,  AF_V,  AF_V},  0 },
		{"BCNT_INT",                  1, {   -1, 0xAA },{      0,     0,  AF_V,  AF_V},  AF_INT_DST },
		{"FFBH_UINT",                 1, {   -1, 0xAB },{      0,     0,  AF_V,  AF_V},  AF_UINT_DST },
		{"FFBL_INT",                  1, {   -1, 0xAC },{      0,     0,  AF_V,  AF_V},  AF_INT_DST },
		{"FFBH_INT",                  1, {   -1, 0xAD },{      0,     0,  AF_V,  AF_V},  AF_INT_DST },
		{"FLT_TO_UINT4",              1, {   -1, 0xAE },{      0,     0,  AF_V,  AF_V},  AF_UINT_DST },
		{"DOT_IEEE",                  2, {   -1, 0xAF },{      0,     0,  AF_V,  AF_V},  AF_PREV_NEXT | AF_IEEE },
		{"FLT_TO_INT_RPI",            1, {   -1, 0xB0 },{      0,     0,  AF_V,  AF_V},  AF_INT_DST | AF_CVT},
		{"FLT_TO_INT_FLOOR",          1, {   -1, 0xB1 },{      0,     0,  AF_V,  AF_V},  AF_INT_DST | AF_CVT},
		{"MULHI_UINT24",              2, {   -1, 0xB2 },{      0,     0,  AF_V,  AF_V},  AF_UINT_DST | AF_24 },
		{"MBCNT_32HI_INT",            1, {   -1, 0xB3 },{      0,     0,  AF_V,  AF_V},  AF_INT_DST },
		{"OFFSET_TO_FLT",             1, {   -1, 0xB4 },{      0,     0,  AF_V,  AF_V},  0 },
		{"MUL_UINT24",                2, {   -1, 0xB5 },{      0,     0,  AF_V,  AF_V},  AF_UINT_DST | AF_24 },
		{"BCNT_ACCUM_PREV_INT",       1, {   -1, 0xB6 },{      0,     0,  AF_V,  AF_V},  AF_INT_DST | AF_PREV_NEXT },
		{"MBCNT_32LO_ACCUM_PREV_INT", 1, {   -1, 0xB7 },{      0,     0,  AF_V,  AF_V},  AF_INT_DST | AF_PREV_NEXT },
		{"SETE_64",                   2, {   -1, 0xB8 },{      0,     0,  AF_V,  AF_V},  AF_SET | AF_CC_E | AF_64 },
		{"SETNE_64",                  2, {   -1, 0xB9 },{      0,     0,  AF_V,  AF_V},  AF_SET | AF_CC_NE | AF_64 },
		{"SETGT_64",                  2, {   -1, 0xBA },{      0,     0,  AF_V,  AF_V},  AF_SET | AF_CC_GT | AF_64 },
		{"SETGE_64",                  2, {   -1, 0xBB },{      0,     0,  AF_V,  AF_V},  AF_SET | AF_CC_GE | AF_64 },
		{"MIN_64",                    2, {   -1, 0xBC },{      0,     0,  AF_V,  AF_V},  AF_64 },
		{"MAX_64",                    2, {   -1, 0xBD },{      0,     0,  AF_V,  AF_V},  AF_64 },
		{"DOT4",                      2, { 0x50, 0xBE },{  AF_4V, AF_4V, AF_4V, AF_4V},  AF_REPL },
		{"DOT4_IEEE",                 2, { 0x51, 0xBF },{  AF_4V, AF_4V, AF_4V, AF_4V},  AF_REPL | AF_IEEE  },
		{"CUBE",                      2, { 0x52, 0xC0 },{  AF_4V, AF_4V, AF_4V, AF_4V},  0 },
		{"MAX4",                      1, { 0x53, 0xC1 },{  AF_4V, AF_4V, AF_4V, AF_4V},  AF_REPL },
		{"FREXP_64",                  1, { 0x07, 0xC4 },{   AF_V,  AF_V,  AF_V,  AF_V},  AF_64 },
		{"LDEXP_64",                  2, { 0x7A, 0xC5 },{   AF_V,  AF_V,  AF_V,  AF_V},  AF_64 },
		{"FRACT_64",                  1, { 0x7B, 0xC6 },{   AF_V,  AF_V,  AF_V,  AF_V},  AF_64 },
		{"PRED_SETGT_64",             2, { 0x7C, 0xC7 },{   AF_V,  AF_V,  AF_V,  AF_V},  AF_PRED | AF_CC_GT | AF_64 },
		{"PRED_SETE_64",              2, { 0x7D, 0xC8 },{   AF_V,  AF_V,  AF_V,  AF_V},  AF_PRED | AF_CC_E | AF_64 },
		{"PRED_SETGE_64",             2, { 0x7E, 0xC9 },{   AF_V,  AF_V,  AF_V,  AF_V},  AF_PRED | AF_CC_GE | AF_64 },
		{"MUL_64",                    2, { 0x1B, 0xCA },{   AF_V,  AF_V,  AF_V,  AF_4V}, AF_64 },
		{"ADD_64",                    2, { 0x17, 0xCB },{   AF_V,  AF_V,  AF_V,  AF_V},  AF_64 },
		{"MOVA_INT",                  1, { 0x18, 0xCC },{   AF_V,  AF_V,  AF_V,  AF_V},  AF_MOVA },
		{"FLT64_TO_FLT32",            1, { 0x1C, 0xCD },{   AF_V,  AF_V,  AF_V,  AF_V},  AF_64 },
		{"FLT32_TO_FLT64",            1, { 0x1D, 0xCE },{   AF_V,  AF_V,  AF_V,  AF_V},  AF_64 },
		{"SAD_ACCUM_PREV_UINT",       2, {   -1, 0xCF },{      0,     0,  AF_V,  AF_V},  AF_UINT_DST | AF_PREV_NEXT },
		{"DOT",                       2, {   -1, 0xD0 },{      0,     0,  AF_V,  AF_V},  AF_PREV_NEXT },
		{"MUL_PREV",                  1, {   -1, 0xD1 },{      0,     0,  AF_V,  AF_V},  AF_PREV_INTERLEAVE },
		{"MUL_IEEE_PREV",             1, {   -1, 0xD2 },{      0,     0,  AF_V,  AF_V},  AF_PREV_INTERLEAVE | AF_IEEE },
		{"ADD_PREV",                  1, {   -1, 0xD3 },{      0,     0,  AF_V,  AF_V},  AF_PREV_INTERLEAVE },
		{"MULADD_PREV",               2, {   -1, 0xD4 },{      0,     0,  AF_V,  AF_V},  AF_PREV_INTERLEAVE },
		{"MULADD_IEEE_PREV",          2, {   -1, 0xD5 },{      0,     0,  AF_V,  AF_V},  AF_PREV_INTERLEAVE | AF_IEEE },
		{"INTERP_XY",                 2, {   -1, 0xD6 },{      0,     0, AF_4V, AF_4V},  AF_INTERP },
		{"INTERP_ZW",                 2, {   -1, 0xD7 },{      0,     0, AF_4V, AF_4V},  AF_INTERP },
		{"INTERP_X",                  2, {   -1, 0xD8 },{      0,     0, AF_2V, AF_2V},  AF_INTERP },
		{"INTERP_Z",                  2, {   -1, 0xD9 },{      0,     0, AF_2V, AF_2V},  AF_INTERP },
		{"STORE_FLAGS",               1, {   -1, 0xDA },{      0,     0,  AF_V,  AF_V},  0 },
		{"LOAD_STORE_FLAGS",          1, {   -1, 0xDB },{      0,     0,  AF_V,  AF_V},  0 },
		{"LDS_1A",                    2, {   -1, 0xDC },{      0,     0,  AF_V,  AF_V},  0 },
		{"LDS_1A1D",                  2, {   -1, 0xDD },{      0,     0,  AF_V,  AF_V},  0 },
		{"LDS_2A",                    2, {   -1, 0xDF },{      0,     0,  AF_V,  AF_V},  0 },
		{"INTERP_LOAD_P0",            1, {   -1, 0xE0 },{      0,     0,  AF_V,  AF_V},  AF_INTERP },
		{"INTERP_LOAD_P10",           1, {   -1, 0xE1 },{      0,     0,  AF_V,  AF_V},  AF_INTERP },
		{"INTERP_LOAD_P20",           1, {   -1, 0xE2 },{      0,     0,  AF_V,  AF_V},  AF_INTERP },
		{"BFE_UINT",                  3, {   -1, 0x04 },{      0,     0,  AF_V,  AF_V},  AF_UINT_DST },
		{"BFE_INT",                   3, {   -1, 0x05 },{      0,     0,  AF_V,  AF_V},  AF_INT_DST },
		{"BFI_INT",                   3, {   -1, 0x06 },{      0,     0,  AF_V,  AF_V},  AF_INT_DST },
		{"FMA",                       3, {   -1, 0x07 },{      0,     0,  AF_V,  AF_V},  0 },
		{"MULADD_INT24",              3, {   -1, 0x08 },{      0,     0,     0,  AF_V},  AF_INT_DST | AF_24 },
		{"CNDNE_64",                  3, {   -1, 0x09 },{      0,     0,  AF_V,  AF_V},  AF_CMOV | AF_64 },
		{"FMA_64",                    3, {   -1, 0x0A },{      0,     0,  AF_V,  AF_4V}, AF_64 },
		{"LERP_UINT",                 3, {   -1, 0x0B },{      0,     0,  AF_V,  AF_V},  AF_UINT_DST },
		{"BIT_ALIGN_INT",             3, {   -1, 0x0C },{      0,     0,  AF_V,  AF_V},  AF_INT_DST },
		{"BYTE_ALIGN_INT",            3, {   -1, 0x0D },{      0,     0,  AF_V,  AF_V},  AF_INT_DST },
		{"SAD_ACCUM_UINT",            3, {   -1, 0x0E },{      0,     0,  AF_V,  AF_V},  AF_UINT_DST },
		{"SAD_ACCUM_HI_UINT",         3, {   -1, 0x0F },{      0,     0,  AF_V,  AF_V},  AF_UINT_DST },
		{"MULADD_UINT24",             3, {   -1, 0x10 },{      0,     0,  AF_V,  AF_V},  AF_UINT_DST | AF_24 },
		{"LDS_IDX_OP",                3, {   -1, 0x11 },{      0,     0,  AF_V,  AF_V},  0 },
		{"MULADD",                    3, { 0x10, 0x14 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC },
		{"MULADD_M2",                 3, { 0x11, 0x15 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC },
		{"MULADD_M4",                 3, { 0x12, 0x16 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC },
		{"MULADD_D2",                 3, { 0x13, 0x17 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC },
		{"MULADD_IEEE",               3, { 0x14, 0x18 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_M_COMM | AF_M_ASSOC | AF_IEEE },
		{"CNDE",                      3, { 0x18, 0x19 },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_CMOV | AF_CC_E },
		{"CNDGT",                     3, { 0x19, 0x1A },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_CMOV | AF_CC_GT },
		{"CNDGE",                     3, { 0x1A, 0x1B },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_CMOV | AF_CC_GE },
		{"CNDE_INT",                  3, { 0x1C, 0x1C },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_CMOV | AF_CC_E | AF_INT_CMP },
		{"CNDGT_INT",                 3, { 0x1D, 0x1D },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_CMOV | AF_CC_GT | AF_INT_CMP },
		{"CNDGE_INT",                 3, { 0x1E, 0x1E },{  AF_VS, AF_VS, AF_VS, AF_VS},  AF_CMOV | AF_CC_GE | AF_INT_CMP },
		{"MUL_LIT",                   3, { 0x0C, 0x1F },{   AF_S,  AF_S,  AF_S,  AF_V},  0 },

		{"MOVA",                      1, { 0x15,   -1 },{   AF_V,  AF_V,     0,     0},  AF_MOVA },
		{"MOVA_FLOOR",                1, { 0x16,   -1 },{   AF_V,  AF_V,     0,     0},  AF_MOVA },
		{"MOVA_GPR_INT",              1, { 0x60,   -1 },{   AF_S,     0,     0,     0},  AF_MOVA },

		{"MULADD_64",                 3, { 0x08,   -1 },{   AF_V,  AF_V,     0,     0},  AF_64 },
		{"MULADD_64_M2",              3, { 0x09,   -1 },{   AF_V,  AF_V,     0,     0},  AF_64 },
		{"MULADD_64_M4",              3, { 0x0A,   -1 },{   AF_V,  AF_V,     0,     0},  AF_64 },
		{"MULADD_64_D2",              3, { 0x0B,   -1 },{   AF_V,  AF_V,     0,     0},  AF_64 },
		{"MUL_LIT_M2",                3, { 0x0D,   -1 },{  AF_VS, AF_VS,     0,     0},  0 },
		{"MUL_LIT_M4",                3, { 0x0E,   -1 },{  AF_VS, AF_VS,     0,     0},  0 },
		{"MUL_LIT_D2",                3, { 0x0F,   -1 },{  AF_VS, AF_VS,     0,     0},  0 },
		{"MULADD_IEEE_M2",            3, { 0x15,   -1 },{  AF_VS, AF_VS,     0,     0},  AF_M_COMM | AF_M_ASSOC | AF_IEEE },
		{"MULADD_IEEE_M4",            3, { 0x16,   -1 },{  AF_VS, AF_VS,     0,     0},  AF_M_COMM | AF_M_ASSOC | AF_IEEE },
		{"MULADD_IEEE_D2",            3, { 0x17,   -1 },{  AF_VS, AF_VS,     0,     0},  AF_M_COMM | AF_M_ASSOC | AF_IEEE },

		{"LDS_ADD",                   2, {   -1, 0x0011 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_SUB",                   2, {   -1, 0x0111 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_RSUB",                  2, {   -1, 0x0211 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_INC",                   2, {   -1, 0x0311 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_DEC",                   2, {   -1, 0x0411 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_MIN_INT",               2, {   -1, 0x0511 },{      0,     0,  AF_V,  AF_V},  AF_LDS | AF_INT_DST },
		{"LDS_MAX_INT",               2, {   -1, 0x0611 },{      0,     0,  AF_V,  AF_V},  AF_LDS | AF_INT_DST },
		{"LDS_MIN_UINT",              2, {   -1, 0x0711 },{      0,     0,  AF_V,  AF_V},  AF_LDS | AF_UINT_DST },
		{"LDS_MAX_UINT",              2, {   -1, 0x0811 },{      0,     0,  AF_V,  AF_V},  AF_LDS | AF_UINT_DST },
		{"LDS_AND",                   2, {   -1, 0x0911 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_OR",                    2, {   -1, 0x0A11 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_XOR",                   2, {   -1, 0x0B11 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_MSKOR",                 3, {   -1, 0x0C11 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_WRITE",                 2, {   -1, 0x0D11 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_WRITE_REL",             3, {   -1, 0x0E11 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_WRITE2",                3, {   -1, 0x0F11 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_CMP_STORE",             3, {   -1, 0x1011 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_CMP_STORE_SPF",         3, {   -1, 0x1111 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_BYTE_WRITE",            2, {   -1, 0x1211 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_SHORT_WRITE",           2, {   -1, 0x1311 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_ADD_RET",               2, {   -1, 0x2011 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_SUB_RET",               2, {   -1, 0x2111 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_RSUB_RET",              2, {   -1, 0x2211 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_INC_RET",               2, {   -1, 0x2311 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_DEC_RET",               2, {   -1, 0x2411 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_MIN_INT_RET",           2, {   -1, 0x2511 },{      0,     0,  AF_V,  AF_V},  AF_LDS | AF_INT_DST },
		{"LDS_MAX_INT_RET",           2, {   -1, 0x2611 },{      0,     0,  AF_V,  AF_V},  AF_LDS | AF_INT_DST },
		{"LDS_MIN_UINT_RET",          2, {   -1, 0x2711 },{      0,     0,  AF_V,  AF_V},  AF_LDS | AF_UINT_DST },
		{"LDS_MAX_UINT_RET",          2, {   -1, 0x2811 },{      0,     0,  AF_V,  AF_V},  AF_LDS | AF_UINT_DST },
		{"LDS_AND_RET",               2, {   -1, 0x2911 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_OR_RET",                2, {   -1, 0x2A11 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_XOR_RET",               2, {   -1, 0x2B11 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_MSKOR_RET",             3, {   -1, 0x2C11 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_XCHG_RET",              2, {   -1, 0x2D11 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_XCHG_REL_RET",          3, {   -1, 0x2E11 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_XCHG2_RET",             3, {   -1, 0x2F11 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_CMP_XCHG_RET",          3, {   -1, 0x3011 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_CMP_XCHG_SPF_RET",      3, {   -1, 0x3111 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_READ_RET",              1, {   -1, 0x3211 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_READ_REL_RET",          1, {   -1, 0x3311 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_READ2_RET",             2, {   -1, 0x3411 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_READWRITE_RET",         3, {   -1, 0x3511 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_BYTE_READ_RET",         1, {   -1, 0x3611 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_UBYTE_READ_RET",        1, {   -1, 0x3711 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_SHORT_READ_RET",        1, {   -1, 0x3811 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
		{"LDS_USHORT_READ_RET",       1, {   -1, 0x3911 },{      0,     0,  AF_V,  AF_V},  AF_LDS },
};

static const struct fetch_op_info fetch_op_table[] = {
		{"VFETCH",                        { 0x000000,  0x000000,  0x000000,  0x000000 }, FF_VTX },
		{"SEMFETCH",                      { 0x000001,  0x000001,  0x000001,  0x000001 }, FF_VTX },

		{"READ_SCRATCH",                  {       -1,  0x000002,  0x000002,  0x000002 }, FF_VTX | FF_MEM },
		{"READ_REDUCT",                   {       -1,  0x000102,        -1,        -1 }, FF_VTX | FF_MEM },
		{"READ_MEM",                      {       -1,  0x000202,  0x000202,  0x000202 }, FF_VTX | FF_MEM },
		{"DS_LOCAL_WRITE",                {       -1,  0x000402,        -1,        -1 }, FF_VTX | FF_MEM },
		{"DS_LOCAL_READ",                 {       -1,  0x000502,        -1,        -1 }, FF_VTX | FF_MEM },

		{"GDS_ADD",                       {       -1,        -1,  0x020002,  0x020002 }, FF_GDS },
		{"GDS_SUB",                       {       -1,        -1,  0x020102,  0x020102 }, FF_GDS },
		{"GDS_RSUB",                      {       -1,        -1,  0x020202,  0x020202 }, FF_GDS },
		{"GDS_INC",                       {       -1,        -1,  0x020302,  0x020302 }, FF_GDS },
		{"GDS_DEC",                       {       -1,        -1,  0x020402,  0x020402 }, FF_GDS },
		{"GDS_MIN_INT",                   {       -1,        -1,  0x020502,  0x020502 }, FF_GDS },
		{"GDS_MAX_INT",                   {       -1,        -1,  0x020602,  0x020602 }, FF_GDS },
		{"GDS_MIN_UINT",                  {       -1,        -1,  0x020702,  0x020702 }, FF_GDS },
		{"GDS_MAX_UINT",                  {       -1,        -1,  0x020802,  0x020802 }, FF_GDS },
		{"GDS_AND",                       {       -1,        -1,  0x020902,  0x020902 }, FF_GDS },
		{"GDS_OR",                        {       -1,        -1,  0x020A02,  0x020A02 }, FF_GDS },
		{"GDS_XOR",                       {       -1,        -1,  0x020B02,  0x020B02 }, FF_GDS },
		{"GDS_MSKOR",                     {       -1,        -1,  0x030C02,  0x030C02 }, FF_GDS },
		{"GDS_WRITE",                     {       -1,        -1,  0x020D02,  0x020D02 }, FF_GDS },
		{"GDS_WRITE_REL",                 {       -1,        -1,  0x030E02,  0x030E02 }, FF_GDS },
		{"GDS_WRITE2",                    {       -1,        -1,  0x030F02,  0x030F02 }, FF_GDS },
		{"GDS_CMP_STORE",                 {       -1,        -1,  0x031002,  0x031002 }, FF_GDS },
		{"GDS_CMP_STORE_SPF",             {       -1,        -1,  0x031102,  0x031102 }, FF_GDS },
		{"GDS_BYTE_WRITE",                {       -1,        -1,  0x021202,  0x021202 }, FF_GDS },
		{"GDS_SHORT_WRITE",               {       -1,        -1,  0x021302,  0x021302 }, FF_GDS },
		{"GDS_ADD_RET",                   {       -1,        -1,  0x122002,  0x122002 }, FF_GDS },
		{"GDS_SUB_RET",                   {       -1,        -1,  0x122102,  0x122102 }, FF_GDS },
		{"GDS_RSUB_RET",                  {       -1,        -1,  0x122202,  0x122202 }, FF_GDS },
		{"GDS_INC_RET",                   {       -1,        -1,  0x122302,  0x122302 }, FF_GDS },
		{"GDS_DEC_RET",                   {       -1,        -1,  0x122402,  0x122402 }, FF_GDS },
		{"GDS_MIN_INT_RET",               {       -1,        -1,  0x122502,  0x122502 }, FF_GDS },
		{"GDS_MAX_INT_RET",               {       -1,        -1,  0x122602,  0x122602 }, FF_GDS },
		{"GDS_MIN_UINT_RET",              {       -1,        -1,  0x122702,  0x122702 }, FF_GDS },
		{"GDS_MAX_UINT_RET",              {       -1,        -1,  0x122802,  0x122802 }, FF_GDS },
		{"GDS_AND_RET",                   {       -1,        -1,  0x122902,  0x122902 }, FF_GDS },
		{"GDS_OR_RET",                    {       -1,        -1,  0x122A02,  0x122A02 }, FF_GDS },
		{"GDS_XOR_RET",                   {       -1,        -1,  0x122B02,  0x122B02 }, FF_GDS },
		{"GDS_MSKOR_RET",                 {       -1,        -1,  0x132C02,  0x132C02 }, FF_GDS },
		{"GDS_XCHG_RET",                  {       -1,        -1,  0x122D02,  0x122D02 }, FF_GDS },
		{"GDS_XCHG_REL_RET",              {       -1,        -1,  0x232E02,  0x232E02 }, FF_GDS },
		{"GDS_XCHG2_RET",                 {       -1,        -1,  0x232F02,  0x232F02 }, FF_GDS },
		{"GDS_CMP_XCHG_RET",              {       -1,        -1,  0x133002,  0x133002 }, FF_GDS },
		{"GDS_CMP_XCHG_SPF_RET",          {       -1,        -1,  0x133102,  0x133102 }, FF_GDS },
		{"GDS_READ_RET",                  {       -1,        -1,  0x113202,  0x113202 }, FF_GDS },
		{"GDS_READ_REL_RET",              {       -1,        -1,  0x213302,  0x213302 }, FF_GDS },
		{"GDS_READ2_RET",                 {       -1,        -1,  0x223402,  0x223402 }, FF_GDS },
		{"GDS_READWRITE_RET",             {       -1,        -1,  0x133502,  0x133502 }, FF_GDS },
		{"GDS_BYTE_READ_RET",             {       -1,        -1,  0x113602,  0x113602 }, FF_GDS },
		{"GDS_UBYTE_READ_RET",            {       -1,        -1,  0x113702,  0x113702 }, FF_GDS },
		{"GDS_SHORT_READ_RET",            {       -1,        -1,  0x113802,  0x113802 }, FF_GDS },
		{"GDS_USHORT_READ_RET",           {       -1,        -1,  0x113902,  0x113902 }, FF_GDS },
		{"GDS_ATOMIC_ORDERED_ALLOC",      {       -1,        -1,  0x113F02,  0x113F02 }, FF_GDS },

		{"TF_WRITE",                      {       -1,        -1,  0x020502,  0x020502 }, FF_GDS },

		{"DS_GLOBAL_WRITE",               {       -1,  0x000602,        -1,        -1 }, 0 },
		{"DS_GLOBAL_READ",                {       -1,  0x000702,        -1,        -1 }, 0 },

		{"LD",                            { 0x000003,  0x000003,  0x000003,  0x000003 }, 0 },
		{"LDFPTR",                        {       -1,        -1,  0x000103,  0x000103 }, 0 },
		{"GET_TEXTURE_RESINFO",           { 0x000004,  0x000004,  0x000004,  0x000004 }, 0 },
		{"GET_NUMBER_OF_SAMPLES",         { 0x000005,  0x000005,  0x000005,  0x000005 }, 0 },
		{"GET_LOD",                       { 0x000006,  0x000006,  0x000006,  0x000006 }, 0 },
		{"GET_GRADIENTS_H",               { 0x000007,  0x000007,  0x000007,  0x000007 }, FF_GETGRAD },
		{"GET_GRADIENTS_V",               { 0x000008,  0x000008,  0x000008,  0x000008 }, FF_GETGRAD },
		{"GET_GRADIENTS_H_FINE",          {       -1,        -1,  0x000107,  0x000107 }, FF_GETGRAD },
		{"GET_GRADIENTS_V_FINE",          {       -1,        -1,  0x000108,  0x000108 }, FF_GETGRAD },
		{"GET_LERP",                      { 0x000009,  0x000009,        -1,        -1 }, 0 },
		{"SET_TEXTURE_OFFSETS",           {       -1,        -1,  0x000009,  0x000009 }, FF_SET_TEXTURE_OFFSETS },
		{"KEEP_GRADIENTS",                {       -1,  0x00000A,  0x00000A,  0x00000A }, 0 },
		{"SET_GRADIENTS_H",               { 0x00000B,  0x00000B,  0x00000B,  0x00000B }, FF_SETGRAD },
		{"SET_GRADIENTS_V",               { 0x00000C,  0x00000C,  0x00000C,  0x00000C }, FF_SETGRAD },
		{"SET_GRADIENTS_H_COARSE",        {       -1,        -1,        -1,  0x00010B }, FF_SETGRAD },
		{"SET_GRADIENTS_V_COARSE",        {       -1,        -1,        -1,  0x00010C }, FF_SETGRAD },
		{"SET_GRADIENTS_H_PACKED_FINE",   {       -1,        -1,        -1,  0x00020B }, FF_SETGRAD },
		{"SET_GRADIENTS_V_PACKED_FINE",   {       -1,        -1,        -1,  0x00020C }, FF_SETGRAD },
		{"SET_GRADIENTS_H_PACKED_COARSE", {       -1,        -1,        -1,  0x00030B }, FF_SETGRAD },
		{"SET_GRADIENTS_V_PACKED_COARSE", {       -1,        -1,        -1,  0x00030C }, FF_SETGRAD },
		{"PASS",                          { 0x00000D,  0x00000D,  0x00000D,  0x00000D }, 0 }, /* ???? 700, eg, cm docs - marked as reserved */
		{"PASS1",                         {       -1,        -1,  0x00010D,  0x00010D }, 0 },
		{"PASS2",                         {       -1,        -1,  0x00020D,  0x00020D }, 0 },
		{"PASS3",                         {       -1,        -1,  0x00030D,  0x00030D }, 0 },
		{"SET_CUBEMAP_INDEX",             { 0x00000E,  0x00000E,        -1,        -1 }, 0 },
		{"GET_BUFFER_RESINFO",            {       -1,        -1,  0x00000E,  0x00000E }, FF_VTX },
		{"FETCH4",                        { 0x00000F,  0x00000F,        -1,        -1 }, 0 },

		{"SAMPLE",                        { 0x000010,  0x000010,  0x000010,  0x000010 }, FF_TEX },
		{"SAMPLE_L",                      { 0x000011,  0x000011,  0x000011,  0x000011 }, FF_TEX },
		{"SAMPLE_LB",                     { 0x000012,  0x000012,  0x000012,  0x000012 }, FF_TEX },
		{"SAMPLE_LZ",                     { 0x000013,  0x000013,  0x000013,  0x000013 }, FF_TEX },
		{"SAMPLE_G",                      { 0x000014,  0x000014,  0x000014,  0x000014 }, FF_TEX | FF_USEGRAD },
		{"SAMPLE_G_L",                    { 0x000015,  0x000015,        -1,        -1 }, FF_TEX | FF_USEGRAD},
		{"GATHER4",                       {       -1,        -1,  0x000015,  0x000015 }, FF_TEX },
		{"SAMPLE_G_LB",                   { 0x000016,  0x000016,  0x000016,  0x000016 }, FF_TEX | FF_USEGRAD},
		{"SAMPLE_G_LZ",                   { 0x000017,  0x000017,        -1,        -1 }, FF_TEX | FF_USEGRAD},
		{"GATHER4_O",                     {       -1,        -1,  0x000017,  0x000017 }, FF_TEX | FF_USE_TEXTURE_OFFSETS},
		{"SAMPLE_C",                      { 0x000018,  0x000018,  0x000018,  0x000018 }, FF_TEX },
		{"SAMPLE_C_L",                    { 0x000019,  0x000019,  0x000019,  0x000019 }, FF_TEX },
		{"SAMPLE_C_LB",                   { 0x00001A,  0x00001A,  0x00001A,  0x00001A }, FF_TEX },
		{"SAMPLE_C_LZ",                   { 0x00001B,  0x00001B,  0x00001B,  0x00001B }, FF_TEX },
		{"SAMPLE_C_G",                    { 0x00001C,  0x00001C,  0x00001C,  0x00001C }, FF_TEX | FF_USEGRAD},
		{"SAMPLE_C_G_L",                  { 0x00001D,  0x00001D,        -1,        -1 }, FF_TEX | FF_USEGRAD},
		{"GATHER4_C",                     {       -1,        -1,  0x00001D,  0x00001D }, FF_TEX },
		{"SAMPLE_C_G_LB",                 { 0x00001E,  0x00001E,  0x00001E,  0x00001E }, FF_TEX | FF_USEGRAD},
		{"SAMPLE_C_G_LZ",                 { 0x00001F,  0x00001F,        -1,        -1 }, FF_TEX | FF_USEGRAD},
		{"GATHER4_C_O",                   {       -1,        -1,  0x00001F,  0x00001F }, FF_TEX | FF_USE_TEXTURE_OFFSETS}
};

static const struct cf_op_info cf_op_table[] = {
		{"NOP",                           { 0x00, 0x00, 0x00, 0x00 },  0  },

		{"TEX",                           { 0x01, 0x01, 0x01, 0x01 },  CF_CLAUSE | CF_FETCH | CF_UNCOND }, /* merged with "TC" entry */
		{"VTX",                           { 0x02, 0x02, 0x02,   -1 },  CF_CLAUSE | CF_FETCH | CF_UNCOND }, /* merged with "VC" entry */
		{"VTX_TC",                        { 0x03, 0x03,   -1,   -1 },  CF_CLAUSE | CF_FETCH | CF_UNCOND },
		{"GDS",                           {   -1,   -1, 0x03, 0x03 },  CF_CLAUSE | CF_FETCH | CF_UNCOND },

		{"LOOP_START",                    { 0x04, 0x04, 0x04, 0x04 },  CF_LOOP | CF_LOOP_START },
		{"LOOP_END",                      { 0x05, 0x05, 0x05, 0x05 },  CF_LOOP  },
		{"LOOP_START_DX10",               { 0x06, 0x06, 0x06, 0x06 },  CF_LOOP | CF_LOOP_START },
		{"LOOP_START_NO_AL",              { 0x07, 0x07, 0x07, 0x07 },  CF_LOOP | CF_LOOP_START },
		{"LOOP_CONTINUE",                 { 0x08, 0x08, 0x08, 0x08 },  CF_LOOP  },
		{"LOOP_BREAK",                    { 0x09, 0x09, 0x09, 0x09 },  CF_LOOP  },
		{"JUMP",                          { 0x0A, 0x0A, 0x0A, 0x0A },  CF_BRANCH  },
		{"PUSH",                          { 0x0B, 0x0B, 0x0B, 0x0B },  CF_BRANCH  },
		{"PUSH_ELSE",                     { 0x0C, 0x0C,   -1,   -1 },  CF_BRANCH  },
		{"ELSE",                          { 0x0D, 0x0D, 0x0D, 0x0D },  CF_BRANCH  },
		{"POP",                           { 0x0E, 0x0E, 0x0E, 0x0E },  CF_BRANCH  },
		{"POP_JUMP",                      { 0x0F, 0x0F,   -1,   -1 },  CF_BRANCH  },
		{"POP_PUSH",                      { 0x10, 0x10,   -1,   -1 },  CF_BRANCH  },
		{"POP_PUSH_ELSE",                 { 0x11, 0x11,   -1,   -1 },  CF_BRANCH  },
		{"CALL",                          { 0x12, 0x12, 0x12, 0x12 },  CF_CALL  },
		{"CALL_FS",                       { 0x13, 0x13, 0x13, 0x13 },  CF_CALL  },
		{"RET",                           { 0x14, 0x14, 0x14, 0x14 },  0 },
		{"EMIT_VERTEX",                   { 0x15, 0x15, 0x15, 0x15 },  CF_EMIT | CF_UNCOND },
		{"EMIT_CUT_VERTEX",               { 0x16, 0x16, 0x16, 0x16 },  CF_EMIT | CF_UNCOND  },
		{"CUT_VERTEX",                    { 0x17, 0x17, 0x17, 0x17 },  CF_EMIT | CF_UNCOND  },
		{"KILL",                          { 0x18, 0x18, 0x18, 0x18 },  CF_UNCOND  },
		{"END_PROGRAM",                   { 0x19, 0x19, 0x19, 0x19 },  0  },  /* ??? "reserved" in isa docs */
		{"WAIT_ACK",                      {   -1, 0x1A, 0x1A, 0x1A },  0  },
		{"TEX_ACK",                       {   -1, 0x1B, 0x1B, 0x1B },  CF_CLAUSE | CF_FETCH | CF_ACK | CF_UNCOND },
		{"VTX_ACK",                       {   -1, 0x1C, 0x1C,   -1 },  CF_CLAUSE | CF_FETCH | CF_ACK | CF_UNCOND },
		{"VTX_TC_ACK",                    {   -1, 0x1D,   -1,   -1 },  CF_CLAUSE | CF_FETCH | CF_ACK | CF_UNCOND },
		{"JUMPTABLE",                     {   -1,   -1, 0x1D, 0x1D },  CF_BRANCH  },
		{"WAVE_SYNC",                     {   -1,   -1, 0x1E, 0x1E },  0  },
		{"HALT",                          {   -1,   -1, 0x1F, 0x1F },  0  },
		{"CF_END",                        {   -1,   -1,   -1, 0x20 },  0  },
		{"LDS_DEALLOC",                   {   -1,   -1,   -1, 0x21 },  0  },
		{"PUSH_WQM",                      {   -1,   -1,   -1, 0x22 },  CF_BRANCH  },
		{"POP_WQM",                       {   -1,   -1,   -1, 0x23 },  CF_BRANCH  },
		{"ELSE_WQM",                      {   -1,   -1,   -1, 0x24 },  CF_BRANCH  },
		{"JUMP_ANY",                      {   -1,   -1,   -1, 0x25 },  CF_BRANCH  },

		/* ??? next 5 added from CAYMAN ISA doc, not in the original table */
		{"REACTIVATE",                    {   -1,   -1,   -1, 0x26 },  0  },
		{"REACTIVATE_WQM",                {   -1,   -1,   -1, 0x27 },  0  },
		{"INTERRUPT",                     {   -1,   -1,   -1, 0x28 },  0  },
		{"INTERRUPT_AND_SLEEP",           {   -1,   -1,   -1, 0x29 },  0  },
		{"SET_PRIORITY",                  {   -1,   -1,   -1, 0x2A },  0  },

		{"MEM_STREAM0_BUF0",              {   -1,   -1, 0x40, 0x40 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM0_BUF1",              {   -1,   -1, 0x41, 0x41 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM0_BUF2",              {   -1,   -1, 0x42, 0x42 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM0_BUF3",              {   -1,   -1, 0x43, 0x43 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM1_BUF0",              {   -1,   -1, 0x44, 0x44 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM1_BUF1",              {   -1,   -1, 0x45, 0x45 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM1_BUF2",              {   -1,   -1, 0x46, 0x46 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM1_BUF3",              {   -1,   -1, 0x47, 0x47 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM2_BUF0",              {   -1,   -1, 0x48, 0x48 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM2_BUF1",              {   -1,   -1, 0x49, 0x49 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM2_BUF2",              {   -1,   -1, 0x4A, 0x4A },  CF_MEM | CF_STRM  },
		{"MEM_STREAM2_BUF3",              {   -1,   -1, 0x4B, 0x4B },  CF_MEM | CF_STRM  },
		{"MEM_STREAM3_BUF0",              {   -1,   -1, 0x4C, 0x4C },  CF_MEM | CF_STRM  },
		{"MEM_STREAM3_BUF1",              {   -1,   -1, 0x4D, 0x4D },  CF_MEM | CF_STRM  },
		{"MEM_STREAM3_BUF2",              {   -1,   -1, 0x4E, 0x4E },  CF_MEM | CF_STRM  },
		{"MEM_STREAM3_BUF3",              {   -1,   -1, 0x4F, 0x4F },  CF_MEM | CF_STRM  },

		{"MEM_STREAM0",                   { 0x20, 0x20,   -1,   -1 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM1",                   { 0x21, 0x21,   -1,   -1 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM2",                   { 0x22, 0x22,   -1,   -1 },  CF_MEM | CF_STRM  },
		{"MEM_STREAM3",                   { 0x23, 0x23,   -1,   -1 },  CF_MEM | CF_STRM  },

		{"MEM_SCRATCH",                   { 0x24, 0x24, 0x50, 0x50 },  CF_MEM  },
		{"MEM_REDUCT",                    { 0x25, 0x25,   -1,   -1 },  CF_MEM  },
		{"MEM_RING",                      { 0x26, 0x26, 0x52, 0x52 },  CF_MEM | CF_EMIT },

		{"EXPORT",                        { 0x27, 0x27, 0x53, 0x53 },  CF_EXP  },
		{"EXPORT_DONE",                   { 0x28, 0x28, 0x54, 0x54 },  CF_EXP  },

		{"MEM_EXPORT",                    {   -1, 0x3A, 0x55, 0x55 },  CF_MEM  },
		{"MEM_RAT",                       {   -1,   -1, 0x56, 0x56 },  CF_MEM | CF_RAT },
		{"MEM_RAT_NOCACHE",               {   -1,   -1, 0x57, 0x57 },  CF_MEM | CF_RAT },
		{"MEM_RING1",                     {   -1,   -1, 0x58, 0x58 },  CF_MEM | CF_EMIT },
		{"MEM_RING2",                     {   -1,   -1, 0x59, 0x59 },  CF_MEM | CF_EMIT },
		{"MEM_RING3",                     {   -1,   -1, 0x5A, 0x5A },  CF_MEM | CF_EMIT },
		{"MEM_MEM_COMBINED",              {   -1,   -1, 0x5B, 0x5B },  CF_MEM  },
		{"MEM_RAT_COMBINED_NOCACHE",      {   -1,   -1, 0x5C, 0x5C },  CF_MEM | CF_RAT },
		{"MEM_RAT_COMBINED",              {   -1,   -1,   -1, 0x5D },  CF_MEM | CF_RAT }, /* ??? not in cayman isa doc */

		{"EXPORT_DONE_END",               {   -1,   -1,   -1, 0x5E },  CF_EXP  },   /* ??? not in cayman isa doc */

		{"ALU",                           { 0x08, 0x08, 0x08, 0x08 },  CF_CLAUSE | CF_ALU  },
		{"ALU_PUSH_BEFORE",               { 0x09, 0x09, 0x09, 0x09 },  CF_CLAUSE | CF_ALU  },
		{"ALU_POP_AFTER",                 { 0x0A, 0x0A, 0x0A, 0x0A },  CF_CLAUSE | CF_ALU  },
		{"ALU_POP2_AFTER",                { 0x0B, 0x0B, 0x0B, 0x0B },  CF_CLAUSE | CF_ALU  },
		{"ALU_EXT",                       {   -1,   -1, 0x0C, 0x0C },  CF_CLAUSE | CF_ALU | CF_ALU_EXT  },
		{"ALU_CONTINUE",                  { 0x0D, 0x0D, 0x0D,   -1 },  CF_CLAUSE | CF_ALU  },
		{"ALU_BREAK",                     { 0x0E, 0x0E, 0x0E,   -1 },  CF_CLAUSE | CF_ALU  },
		{"ALU_VALID_PIXEL_MODE",          {   -1,   -1,   -1, 0x0E },  CF_CLAUSE | CF_ALU  },
		{"ALU_ELSE_AFTER",                { 0x0F, 0x0F, 0x0F, 0x0F },  CF_CLAUSE | CF_ALU  },
		{"CF_NATIVE",                     { 0x00, 0x00, 0x00, 0x00 },  0  }
};

unsigned
r600_alu_op_table_size(void)
{
	return ARRAY_SIZE(r600_alu_op_table);
}

const struct alu_op_info *
r600_isa_alu(unsigned op) {
	assert (op < ARRAY_SIZE(r600_alu_op_table));
	return &r600_alu_op_table[op];
}

const struct fetch_op_info *
r600_isa_fetch(unsigned op) {
	assert (op < ARRAY_SIZE(fetch_op_table));
	return &fetch_op_table[op];
}

const struct cf_op_info *
r600_isa_cf(unsigned op) {
	assert (op < ARRAY_SIZE(cf_op_table));
	return &cf_op_table[op];
}

int r600_isa_init(enum amd_gfx_level gfx_level, struct r600_isa *isa) {
	unsigned i;

	assert(gfx_level >= R600 && gfx_level <= CAYMAN);
	isa->hw_class = gfx_level - R600;

	/* reverse lookup maps are required for bytecode parsing */

	isa->alu_op2_map = calloc(256, sizeof(unsigned));
	if (!isa->alu_op2_map)
		return -1;
	isa->alu_op3_map = calloc(256, sizeof(unsigned));
	if (!isa->alu_op3_map)
		return -1;
	isa->fetch_map = calloc(256, sizeof(unsigned));
	if (!isa->fetch_map)
		return -1;
	isa->cf_map = calloc(256, sizeof(unsigned));
	if (!isa->cf_map)
		return -1;

	for (i = 0; i < ARRAY_SIZE(r600_alu_op_table); ++i) {
		const struct alu_op_info *op = &r600_alu_op_table[i];
		int opc;
		if (op->flags & AF_LDS || op->slots[isa->hw_class] == 0)
			continue;
		opc = op->opcode[isa->hw_class >> 1];
		assert(opc != -1);
		if (op->src_count == 3)
			isa->alu_op3_map[opc] = i + 1;
		else
			isa->alu_op2_map[opc] = i + 1;
	}

	for (i = 0; i < ARRAY_SIZE(fetch_op_table); ++i) {
		const struct fetch_op_info *op = &fetch_op_table[i];
		int opc = op->opcode[isa->hw_class];
		if ((op->flags & FF_GDS) || ((opc & 0xFF) != opc))
			continue; /* ignore GDS ops and INST_MOD versions for now */
		isa->fetch_map[opc] = i + 1;
	}

	for (i = 0; i < ARRAY_SIZE(cf_op_table); ++i) {
		const struct cf_op_info *op = &cf_op_table[i];
		int opc = op->opcode[isa->hw_class];
		if (opc == -1)
			continue;
		/* using offset for CF_ALU_xxx opcodes because they overlap with other
		 * CF opcodes (they use different encoding in hw) */
		if (op->flags & CF_ALU)
			opc += 0x80;
		isa->cf_map[opc] = i + 1;
	}

	return 0;
}

int r600_isa_destroy(struct r600_isa *isa) {

	if (!isa)
		return 0;

	free(isa->alu_op2_map);
	free(isa->alu_op3_map);
	free(isa->fetch_map);
	free(isa->cf_map);

	free(isa);
	return 0;
}



