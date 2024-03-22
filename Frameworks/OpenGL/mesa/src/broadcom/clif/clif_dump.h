/*
 * Copyright © 2016 Broadcom
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

#ifndef CLIF_DUMP_H
#define CLIF_DUMP_H

#include <stdbool.h>
#include <stdint.h>

struct v3d_device_info;
struct clif_dump;
struct drm_v3d_submit_cl;

struct clif_dump *clif_dump_init(const struct v3d_device_info *devinfo,
                                 FILE *output, bool pretty, bool nobin);
void clif_dump(struct clif_dump *clif, const struct drm_v3d_submit_cl *submit);
void clif_dump_destroy(struct clif_dump *clif);

void clif_dump_add_bo(struct clif_dump *clif, const char *name,
                      uint32_t offset, uint32_t size, void *vaddr);
void clif_dump_add_cl(struct clif_dump *clif, uint32_t start, uint32_t end);

#endif
