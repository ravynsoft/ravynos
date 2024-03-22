/* Copyright © 2023 Valve Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef MESA_BLAKE3_H
#define MESA_BLAKE3_H

#include "blake3/blake3.h"

#ifdef __cplusplus
extern "C" {
#endif

#define mesa_blake3 blake3_hasher

typedef uint8_t blake3_hash[BLAKE3_OUT_LEN];

static inline void
_mesa_blake3_init(struct mesa_blake3 *ctx)
{
  blake3_hasher_init(ctx);
}

static inline void
_mesa_blake3_update(struct mesa_blake3 *ctx, const void *data, size_t size)
{
   blake3_hasher_update(ctx, data, size);
}

static inline void
_mesa_blake3_final(struct mesa_blake3 *ctx, blake3_hash result)
{
   blake3_hasher_finalize(ctx, result, BLAKE3_OUT_LEN);
}

void
_mesa_blake3_format(char *buf, const unsigned char *blake3);

void
_mesa_blake3_hex_to_blake3(unsigned char *buf, const char *hex);

void
_mesa_blake3_compute(const void *data, size_t size, blake3_hash result);

#ifdef __cplusplus
} /* extern C */
#endif


#endif