/*
 * Copyright © 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef __I965_ASM_H__
#define __I965_ASM_H__

#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>

#include "compiler/brw_reg.h"
#include "compiler/brw_reg_type.h"
#include "compiler/brw_eu_defines.h"
#include "compiler/brw_inst.h"
#include "compiler/brw_eu.h"
#include "dev/intel_device_info.h"
#include "util/list.h"

/* glibc < 2.27 defines OVERFLOW in /usr/include/math.h. */
#undef OVERFLOW

int yyparse(void);
int yylex(void);
char *lex_text(void);

extern struct brw_codegen *p;
extern int errors;
extern char *input_filename;

extern struct list_head instr_labels;
extern struct list_head target_labels;

struct condition {
   unsigned cond_modifier:4;
   unsigned flag_reg_nr:1;
   unsigned flag_subreg_nr:1;
};

struct predicate {
   unsigned pred_control:4;
   unsigned pred_inv:1;
   unsigned flag_reg_nr:1;
   unsigned flag_subreg_nr:1;
};

enum instoption_type {
   INSTOPTION_FLAG,
   INSTOPTION_DEP_INFO,
};

struct instoption {
   enum instoption_type type;
   union {
      unsigned uint_value;
      struct tgl_swsb depinfo_value;
   };
};

struct options {
   unsigned access_mode:1;
   unsigned compression_control:2;
   unsigned thread_control:2;
   unsigned no_dd_check:1; // Dependency control
   unsigned no_dd_clear:1; // Dependency control
   unsigned mask_control:1;
   unsigned debug_control:1;
   unsigned acc_wr_control:1;
   unsigned end_of_thread:1;
   unsigned compaction:1;
   unsigned qtr_ctrl:2;
   unsigned nib_ctrl:1;
   unsigned is_compr:1;
   struct tgl_swsb depinfo;
};

struct msgdesc {
   unsigned ex_bso:1;
   unsigned src1_len:5;
};

enum instr_label_type {
   INSTR_LABEL_JIP,
   INSTR_LABEL_UIP,
};

struct instr_label {
   struct list_head link;

   char *name;
   int offset;
   enum instr_label_type type;
};

struct target_label {
   struct list_head link;

   char *name;
   int offset;
};

#endif /* __I965_ASM_H__ */
