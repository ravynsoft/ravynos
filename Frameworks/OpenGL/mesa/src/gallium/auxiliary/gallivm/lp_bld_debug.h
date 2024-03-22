/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


#ifndef LP_BLD_DEBUG_H
#define LP_BLD_DEBUG_H


#include "gallivm/lp_bld.h"

#include "util/compiler.h"
#include "util/u_string.h"


#define GALLIVM_DEBUG_TGSI          (1 << 0)
#define GALLIVM_DEBUG_IR            (1 << 1)
#define GALLIVM_DEBUG_ASM           (1 << 2)
#define GALLIVM_DEBUG_PERF          (1 << 3)
#define GALLIVM_DEBUG_GC            (1 << 4)
#define GALLIVM_DEBUG_DUMP_BC       (1 << 5)

#define GALLIVM_PERF_BRILINEAR       (1 << 0)
#define GALLIVM_PERF_RHO_APPROX      (1 << 1)
#define GALLIVM_PERF_NO_QUAD_LOD     (1 << 2)
#define GALLIVM_PERF_NO_OPT          (1 << 3)
#define GALLIVM_PERF_NO_AOS_SAMPLING (1 << 4)

#ifdef __cplusplus
extern "C" {
#endif


extern unsigned gallivm_perf;

extern unsigned gallivm_debug;


static inline void
lp_build_name(LLVMValueRef val, const char *format, ...)
{
#ifdef DEBUG
   char name[32];
   va_list ap;
   va_start(ap, format);
   vsnprintf(name, sizeof name, format, ap);
   va_end(ap);
   LLVMSetValueName(val, name);
#else
   (void)val;
   (void)format;
#endif
}


void
lp_debug_dump_value(LLVMValueRef value);


bool
lp_check_alignment(const void *ptr, unsigned alignment);


void
lp_disassemble(LLVMValueRef func, const void *code);


void
lp_profile(LLVMValueRef func, const void *code);


#ifdef __cplusplus
}
#endif


#endif /* !LP_BLD_DEBUG_H */
