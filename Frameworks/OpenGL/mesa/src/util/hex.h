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

#ifndef UTIL_HEX_H
#define UTIL_HEX_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Convert a binary buffer of length `len` to a hexadecimal string of length
 * `len * 2 + 1` (including NUL terminator).
 */
static inline char *mesa_bytes_to_hex(char *buf, const unsigned char *binary,
                                      unsigned len) {
  static const char hex_digits[] = "0123456789abcdef";
  unsigned i;

  for (i = 0; i < len * 2; i += 2) {
    buf[i] = hex_digits[binary[i >> 1] >> 4];
    buf[i + 1] = hex_digits[binary[i >> 1] & 0x0f];
  }
  buf[i] = '\0';

  return buf;
}

/*
 * Read `len` pairs of hexadecimal digits from `hex` and write the values to
 * `binary` as `len` bytes.
 */
static inline void mesa_hex_to_bytes(unsigned char *buf, const char *hex,
                                     unsigned len) {
  for (unsigned i = 0; i < len; i++) {
    char tmp[3];
    tmp[0] = hex[i * 2];
    tmp[1] = hex[(i * 2) + 1];
    tmp[2] = '\0';
    buf[i] = strtol(tmp, NULL, 16);
  }
}

#ifdef __cplusplus
}
#endif

#endif