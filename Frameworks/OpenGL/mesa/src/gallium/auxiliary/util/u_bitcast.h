/**************************************************************************
 *
 * Copyright © 2016 Intel Corporation
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
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef U_BITCAST_H_
#define U_BITCAST_H_

#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint32_t
u_bitcast_f2u(float f)
{
   unsigned u;
   memcpy(&u, &f, sizeof(u));
   return u;
}

static inline uint64_t
u_bitcast_d2u(double d)
{
   uint64_t u;
   memcpy(&u, &d, sizeof(u));
   return u;
}

static inline float
u_bitcast_u2f(uint32_t u)
{
   float f;
   memcpy(&f, &u, sizeof(f));
   return f;
}

static inline double
u_bitcast_u2d(uint64_t u)
{
   double d;
   memcpy(&d, &u, sizeof(d));
   return d;
}

#ifdef __cplusplus
}
#endif

#endif /* U_BITCAST_H_ */
