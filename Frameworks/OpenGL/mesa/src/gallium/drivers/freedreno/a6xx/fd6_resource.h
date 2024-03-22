/*
 * Copyright (C) 2018 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
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

#ifndef FD6_RESOURCE_H_
#define FD6_RESOURCE_H_

#include "freedreno_resource.h"

enum fd6_format_status {
   FORMAT_OK,
   DEMOTE_TO_LINEAR,
   DEMOTE_TO_TILED,
};

BEGINC;

enum fd6_format_status fd6_check_valid_format(struct fd_resource *rsc,
                                              enum pipe_format format);
void fd6_validate_format(struct fd_context *ctx, struct fd_resource *rsc,
                         enum pipe_format format) assert_dt;

static inline void
fd6_assert_valid_format(struct fd_resource *rsc, enum pipe_format format)
{
   assert(fd6_check_valid_format(rsc, format) == FORMAT_OK);
}

void fd6_emit_flag_reference(struct fd_ringbuffer *ring,
                             struct fd_resource *rsc, int level, int layer);
void fd6_resource_screen_init(struct pipe_screen *pscreen);

ENDC;

#endif /* FD6_RESOURCE_H_ */
