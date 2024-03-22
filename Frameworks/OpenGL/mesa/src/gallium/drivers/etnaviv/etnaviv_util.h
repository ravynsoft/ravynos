/*
 * Copyright (c) 2012-2015 Etnaviv Project
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
 */

/* Misc util */
#ifndef H_ETNA_UTIL
#define H_ETNA_UTIL

#include <math.h>
#include "mesa/main/macros.h"

/* for conditionally setting boolean flag(s): */
#define COND(bool, val) ((bool) ? (val) : 0)

/* float to fixp 5.5 */
static inline uint32_t
etna_float_to_fixp55(float f)
{
   return U_FIXED(f, 5);
}

/* float to fixp 8.8 */
static inline uint32_t
etna_float_to_fixp88(float f)
{
   return U_FIXED(f, 8);
}

/* texture size to log2 in fixp 5.5 format */
static inline uint32_t
etna_log2_fixp55(unsigned width)
{
   return etna_float_to_fixp55(log2f((float)width));
}

/* texture size to log2 in fixp 8.8 format */
static inline uint32_t
etna_log2_fixp88(unsigned width)
{
   return etna_float_to_fixp88(log2f((float)width));
}

/* float to fixp 16.16 */
static inline uint32_t
etna_f32_to_fixp16(float f)
{
   return S_FIXED(f, 16);
}

#endif
