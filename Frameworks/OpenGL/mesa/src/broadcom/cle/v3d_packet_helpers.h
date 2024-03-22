/*
 * Copyright (C) 2016 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef MESA_V3D_PACKET_HELPERS_H
#define MESA_V3D_PACKET_HELPERS_H

#include "util/bitpack_helpers.h"

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#define VG(x) x
#else
#define VG(x) ((void)0)
#endif

static inline uint64_t
__gen_offset(uint64_t v, uint32_t start, uint32_t end)
{
   util_bitpack_validate_value(v);
#ifndef NDEBUG
   uint64_t mask = (~0ull >> (64 - (end - start + 1))) << start;

   assert((v & ~mask) == 0);
#endif

   return v;
}

static inline uint64_t
__gen_unpack_uint(const uint8_t *restrict cl, uint32_t start, uint32_t end)
{
   uint64_t val = 0;
   const int width = end - start + 1;
   const uint32_t mask = (width == 32 ? ~0 : (1 << width) - 1 );

   for (uint32_t byte = start / 8; byte <= end / 8; byte++) {
      val |= cl[byte] << ((byte - start / 8) * 8);
   }

   return (val >> (start % 8)) & mask;
}

static inline uint64_t
__gen_unpack_sint(const uint8_t *restrict cl, uint32_t start, uint32_t end)
{
   int size = end - start + 1;
   int64_t val = __gen_unpack_uint(cl, start, end);

   /* Get the sign bit extended. */
   return (val << (64 - size)) >> (64 - size);
}

static inline float
__gen_unpack_sfixed(const uint8_t *restrict cl, uint32_t start, uint32_t end,
                    uint32_t fractional_size)
{
        int32_t bits = __gen_unpack_sint(cl, start, end);
        return (float)bits / (1 << fractional_size);
}

static inline float
__gen_unpack_ufixed(const uint8_t *restrict cl, uint32_t start, uint32_t end,
                    uint32_t fractional_size)
{
        int32_t bits = __gen_unpack_uint(cl, start, end);
        return (float)bits / (1 << fractional_size);
}

static inline float
__gen_unpack_float(const uint8_t *restrict cl, uint32_t start, uint32_t end)
{
   assert(start % 8 == 0);
   assert(end - start == 31);

   struct PACKED { float f; } *f = (void *)(cl + (start / 8));

   return f->f;
}

static inline float
__gen_unpack_f187(const uint8_t *restrict cl, uint32_t start, uint32_t end)
{
   assert(end - start == 15);
   uint32_t bits = __gen_unpack_uint(cl, start, end);
   return uif(bits << 16);
}

#endif
