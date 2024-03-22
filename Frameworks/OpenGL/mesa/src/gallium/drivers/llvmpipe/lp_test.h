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

/**
 * @file
 * Shared testing code.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#ifndef LP_TEST_H
#define LP_TEST_H


#include <stdlib.h>
#include <stdio.h>
#include <float.h>

#include "gallivm/lp_bld.h"

#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_dump.h"

#include "gallivm/lp_bld_type.h"


#define LP_TEST_NUM_SAMPLES 32


void
write_tsv_header(FILE *fp);


bool
test_some(unsigned verbose, FILE *fp,
          unsigned long n);

bool
test_single(unsigned verbose, FILE *fp);

bool
test_all(unsigned verbose, FILE *fp);


#if DETECT_CC_MSVC

#include <intrin.h>
#define rdtsc() __rdtsc()

#elif DETECT_CC_GCC && (DETECT_ARCH_X86 || DETECT_ARCH_X86_64)

static inline uint64_t
rdtsc(void)
{
   uint32_t hi, lo;
   __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
   return ((uint64_t)lo) | (((uint64_t)hi) << 32);
}

#else

#define rdtsc() 0

#endif



float
random_float(void);


void
dump_type(FILE *fp, struct lp_type type);


double
read_elem(struct lp_type type, const void *src, unsigned index);


void
write_elem(struct lp_type type, void *dst, unsigned index, double src);


void
random_elem(struct lp_type type, void *dst, unsigned index);


void
read_vec(struct lp_type type, const void *src, double *dst);


void
write_vec(struct lp_type type, void *dst, const double *src);


void
random_vec(struct lp_type type, void *dst);


bool
compare_vec_with_eps(struct lp_type type, const void *res, const void *ref, double eps);


bool
compare_vec(struct lp_type type, const void *res, const void *ref);


void
dump_vec(FILE *fp, struct lp_type type, const void *src);


#endif /* !LP_TEST_H */
