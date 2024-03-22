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

#include "mesa-blake3.h"

#include "hex.h"

void _mesa_blake3_format(char *buf, const unsigned char *blake3)
{
  mesa_bytes_to_hex(buf, blake3, BLAKE3_OUT_LEN);
}

void _mesa_blake3_hex_to_blake3(unsigned char *buf, const char *hex)
{
  mesa_hex_to_bytes(buf, hex, BLAKE3_OUT_LEN);
}

void _mesa_blake3_compute(const void *data, size_t size, blake3_hash result)
{
  struct mesa_blake3 ctx;
  _mesa_blake3_init(&ctx);
  _mesa_blake3_update(&ctx, data, size);
  _mesa_blake3_final(&ctx, result);
}