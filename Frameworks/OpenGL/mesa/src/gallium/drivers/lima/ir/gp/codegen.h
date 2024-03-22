/*
 * Copyright (c) 2017 Lima Project
 * Copyright (c) 2013 Ben Brewer (ben.brewer@codethink.co.uk)
 * Copyright (c) 2013 Connor Abbott (connor@abbott.cx)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef LIMA_IR_GP_CODEGEN_H
#define LIMA_IR_GP_CODEGEN_H

typedef enum {
   gpir_codegen_src_attrib_x     =  0,
   gpir_codegen_src_attrib_y     =  1,
   gpir_codegen_src_attrib_z     =  2,
   gpir_codegen_src_attrib_w     =  3,
   gpir_codegen_src_register_x   =  4,
   gpir_codegen_src_register_y   =  5,
   gpir_codegen_src_register_z   =  6,
   gpir_codegen_src_register_w   =  7,
   gpir_codegen_src_unknown_0    =  8,
   gpir_codegen_src_unknown_1    =  9,
   gpir_codegen_src_unknown_2    = 10,
   gpir_codegen_src_unknown_3    = 11,
   gpir_codegen_src_load_x       = 12,
   gpir_codegen_src_load_y       = 13,
   gpir_codegen_src_load_z       = 14,
   gpir_codegen_src_load_w       = 15,
   gpir_codegen_src_p1_acc_0     = 16,
   gpir_codegen_src_p1_acc_1     = 17,
   gpir_codegen_src_p1_mul_0     = 18,
   gpir_codegen_src_p1_mul_1     = 19,
   gpir_codegen_src_p1_pass      = 20,
   gpir_codegen_src_unused       = 21,
   gpir_codegen_src_ident        = 22,
   gpir_codegen_src_p1_complex   = 22,
   gpir_codegen_src_p2_pass      = 23,
   gpir_codegen_src_p2_acc_0     = 24,
   gpir_codegen_src_p2_acc_1     = 25,
   gpir_codegen_src_p2_mul_0     = 26,
   gpir_codegen_src_p2_mul_1     = 27,
   gpir_codegen_src_p1_attrib_x  = 28,
   gpir_codegen_src_p1_attrib_y  = 29,
   gpir_codegen_src_p1_attrib_z  = 30,
   gpir_codegen_src_p1_attrib_w  = 31,
} gpir_codegen_src;

typedef enum {
   gpir_codegen_load_off_ld_addr_0 = 1,
   gpir_codegen_load_off_ld_addr_1 = 2,
   gpir_codegen_load_off_ld_addr_2 = 3,
   gpir_codegen_load_off_none      = 7,
} gpir_codegen_load_off;

typedef enum {
   gpir_codegen_store_src_acc_0   = 0,
   gpir_codegen_store_src_acc_1   = 1,
   gpir_codegen_store_src_mul_0   = 2,
   gpir_codegen_store_src_mul_1   = 3,
   gpir_codegen_store_src_pass    = 4,
   gpir_codegen_store_src_unknown = 5,
   gpir_codegen_store_src_complex = 6,
   gpir_codegen_store_src_none    = 7,
} gpir_codegen_store_src;

typedef enum {
   gpir_codegen_acc_op_add   = 0,
   gpir_codegen_acc_op_floor = 1,
   gpir_codegen_acc_op_sign  = 2,
   gpir_codegen_acc_op_ge    = 4,
   gpir_codegen_acc_op_lt    = 5,
   gpir_codegen_acc_op_min   = 6,
   gpir_codegen_acc_op_max   = 7,
} gpir_codegen_acc_op;

typedef enum {
   gpir_codegen_complex_op_nop	 = 0,
   gpir_codegen_complex_op_exp2	 = 2,
   gpir_codegen_complex_op_log2	 = 3,
   gpir_codegen_complex_op_rsqrt = 4,
   gpir_codegen_complex_op_rcp	 = 5,
   gpir_codegen_complex_op_pass	 = 9,
   gpir_codegen_complex_op_temp_store_addr  = 12,
   gpir_codegen_complex_op_temp_load_addr_0 = 13,
   gpir_codegen_complex_op_temp_load_addr_1 = 14,
   gpir_codegen_complex_op_temp_load_addr_2 = 15,
} gpir_codegen_complex_op;

typedef enum {
   gpir_codegen_mul_op_mul	= 0,
   gpir_codegen_mul_op_complex1 = 1,
   gpir_codegen_mul_op_complex2 = 3,
   gpir_codegen_mul_op_select	= 4,
} gpir_codegen_mul_op;

typedef enum {
   gpir_codegen_pass_op_pass	 = 2,
   gpir_codegen_pass_op_preexp2	 = 4,
   gpir_codegen_pass_op_postlog2 = 5,
   gpir_codegen_pass_op_clamp	 = 6,
} gpir_codegen_pass_op;


typedef struct __attribute__((__packed__)) {
   gpir_codegen_src        mul0_src0           : 5;
   gpir_codegen_src        mul0_src1           : 5;
   gpir_codegen_src        mul1_src0           : 5;
   gpir_codegen_src        mul1_src1           : 5;
   bool                    mul0_neg            : 1;
   bool                    mul1_neg            : 1;
   gpir_codegen_src        acc0_src0           : 5;
   gpir_codegen_src        acc0_src1           : 5;
   gpir_codegen_src        acc1_src0           : 5;
   gpir_codegen_src        acc1_src1           : 5;
   bool                    acc0_src0_neg       : 1;
   bool                    acc0_src1_neg       : 1;
   bool                    acc1_src0_neg       : 1;
   bool                    acc1_src1_neg       : 1;
   unsigned                load_addr           : 9;
   gpir_codegen_load_off   load_offset         : 3;
   unsigned                register0_addr      : 4;
   bool                    register0_attribute : 1;
   unsigned                register1_addr      : 4;
   bool                    store0_temporary    : 1;
   bool                    store1_temporary    : 1;
   bool                    branch              : 1;
   bool                    branch_target_lo    : 1;
   gpir_codegen_store_src  store0_src_x        : 3;
   gpir_codegen_store_src  store0_src_y        : 3;
   gpir_codegen_store_src  store1_src_z        : 3;
   gpir_codegen_store_src  store1_src_w        : 3;
   gpir_codegen_acc_op     acc_op              : 3;
   gpir_codegen_complex_op complex_op          : 4;
   unsigned                store0_addr         : 4;
   bool                    store0_varying      : 1;
   unsigned                store1_addr         : 4;
   bool                    store1_varying      : 1;
   gpir_codegen_mul_op     mul_op              : 3;
   gpir_codegen_pass_op    pass_op             : 3;
   gpir_codegen_src        complex_src         : 5;
   gpir_codegen_src        pass_src            : 5;
   unsigned                unknown_1           : 4; /* 12: tmp_st, 13: branch */
   unsigned                branch_target       : 8;
} gpir_codegen_instr;

void gpir_disassemble_program(gpir_codegen_instr *code, unsigned num_instr, FILE *fp);

#endif
