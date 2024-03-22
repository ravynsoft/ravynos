/*
 * Copyright (c) 2019 Etnaviv Project
 * Copyright (C) 2019 Zodiac Inflight Innovations
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
 *
 * Authors:
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#ifndef H_ETNA_ETC2
#define H_ETNA_ETC2

#include <stdbool.h>
#include <stdint.h>

#include "util/format/u_formats.h"
#include "util/u_dynarray.h"

struct pipe_resource;

bool
etna_etc2_needs_patching(const struct pipe_resource *prsc);

void
etna_etc2_calculate_blocks(uint8_t *buffer, unsigned stride,
                           unsigned width, unsigned height,
                           enum pipe_format format,
                           struct util_dynarray *offsets);

void
etna_etc2_patch(uint8_t *buffer, const struct util_dynarray *offsets);

#endif
