/*
 * © Copyright 2017-2018 Alyssa Rosenzweig
 * © Copyright 2017-2018 Connor Abbott
 * © Copyright 2017-2018 Lyude Paul
 * © Copyright2019 Collabora, Ltd.
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

#ifndef __PANFROST_JOB_H__
#define __PANFROST_JOB_H__

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint64_t mali_ptr;

#define MALI_FORMAT_COMPRESSED     (0 << 5)
#define MALI_EXTRACT_TYPE(fmt)     ((fmt)&0xe0)
#define MALI_EXTRACT_INDEX(pixfmt) (((pixfmt) >> 12) & 0xFF)

/* Mali hardware can texture up to 65536 x 65536 x 65536 and render up to 16384
 * x 16384, but 8192 x 8192 should be enough for anyone.  The OpenGL game
 * "Cathedral" requires a texture of width 8192 to start.
 */
#define MAX_MIP_LEVELS (14)

#define MAX_IMAGE_PLANES (3)

#endif /* __PANFROST_JOB_H__ */
