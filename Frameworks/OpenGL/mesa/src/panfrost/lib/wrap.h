
/*
 * Copyright (C) 2017-2019 Lyude Paul
 * Copyright (C) 2017-2019 Alyssa Rosenzweig
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

#ifndef __PAN_DECODE_PUBLIC_H__
#define __PAN_DECODE_PUBLIC_H__

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Public entrypoints for the tracing infrastructure. This API should be kept
 * more or less stable. Don't feel bad if you have to change it; just feel
 * slightly guilty about creating more work for me later. -Alyssa <3
 *
 * I'm joking. Mostly. panwrap (out-of-tree) includes this, so update that if
 * you need to change something here. panwrap is open-source but cannot be
 * included in-tree.
 */

// TODO: update panwrap

struct pandecode_context;

struct pandecode_context *pandecode_create_context(bool to_stderr);

void pandecode_next_frame(struct pandecode_context *ctx);

void pandecode_destroy_context(struct pandecode_context *ctx);

void pandecode_inject_mmap(struct pandecode_context *ctx, uint64_t gpu_va,
                           void *cpu, unsigned sz, const char *name);

void pandecode_inject_free(struct pandecode_context *ctx, uint64_t gpu_va,
                           unsigned sz);

void pandecode_jc(struct pandecode_context *ctx, uint64_t jc_gpu_va,
                  unsigned gpu_id);

void pandecode_cs(struct pandecode_context *ctx, mali_ptr queue_gpu_va,
                  uint32_t size, unsigned gpu_id, uint32_t *regs);

void pandecode_abort_on_fault(struct pandecode_context *ctx, uint64_t jc_gpu_va,
                              unsigned gpu_id);

#endif /* __MMAP_TRACE_H__ */
