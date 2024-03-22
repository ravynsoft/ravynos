/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
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
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FD5_SCREEN_H_
#define FD5_SCREEN_H_

#include "pipe/p_screen.h"

#include "freedreno_util.h"

#include "a5xx.xml.h"

void fd5_screen_init(struct pipe_screen *pscreen);

static inline void
emit_marker5(struct fd_ringbuffer *ring, int scratch_idx)
{
   extern int32_t marker_cnt;
   unsigned reg = REG_A5XX_CP_SCRATCH_REG(scratch_idx);
   if (__EMIT_MARKER) {
      OUT_WFI5(ring);
      OUT_PKT4(ring, reg, 1);
      OUT_RING(ring, p_atomic_inc_return(&marker_cnt));
   }
}

#endif /* FD5_SCREEN_H_ */
