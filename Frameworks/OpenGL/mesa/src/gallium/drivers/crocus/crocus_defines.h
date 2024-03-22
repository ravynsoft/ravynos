/*
 * Copyright © 2018 Intel Corporation
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
 */
#ifndef CROCUS_DEFINES_H
#define CROCUS_DEFINES_H

/**
 * @file crocus_defines.h
 *
 * Random hardware #defines that we're not using GENXML for.
 */

#define MI_PREDICATE                         (0xC << 23)
# define MI_PREDICATE_LOADOP_KEEP            (0 << 6)
# define MI_PREDICATE_LOADOP_LOAD            (2 << 6)
# define MI_PREDICATE_LOADOP_LOADINV         (3 << 6)
# define MI_PREDICATE_COMBINEOP_SET          (0 << 3)
# define MI_PREDICATE_COMBINEOP_AND          (1 << 3)
# define MI_PREDICATE_COMBINEOP_OR           (2 << 3)
# define MI_PREDICATE_COMBINEOP_XOR          (3 << 3)
# define MI_PREDICATE_COMPAREOP_TRUE         (0 << 0)
# define MI_PREDICATE_COMPAREOP_FALSE        (1 << 0)
# define MI_PREDICATE_COMPAREOP_SRCS_EQUAL   (2 << 0)
# define MI_PREDICATE_COMPAREOP_DELTAS_EQUAL (3 << 0)

/* Predicate registers */
#define MI_PREDICATE_SRC0                    0x2400
#define MI_PREDICATE_SRC1                    0x2408
#define MI_PREDICATE_DATA                    0x2410
#define MI_PREDICATE_RESULT                  0x2418
#define MI_PREDICATE_RESULT_1                0x241C
#define MI_PREDICATE_RESULT_2                0x2214

#define CS_GPR(n) (0x2600 + (n) * 8)

/* The number of bits in our TIMESTAMP queries. */
#define TIMESTAMP_BITS 36

#endif
