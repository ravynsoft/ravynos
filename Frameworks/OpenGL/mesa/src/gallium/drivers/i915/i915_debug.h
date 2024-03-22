/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/* Authors:  Keith Whitwell <keithw@vmware.com>
 *           Jakob Bornecrantz <wallbraker@gmail.com>
 */

#ifndef I915_DEBUG_H
#define I915_DEBUG_H

#include "util/u_debug.h"

struct i915_screen;
struct i915_context;
struct i915_winsys_batchbuffer;

#define DBG_BLIT      0x1
#define DBG_EMIT      0x2
#define DBG_ATOMS     0x4
#define DBG_FLUSH     0x8
#define DBG_TEXTURE   0x10
#define DBG_CONSTANTS 0x20
#define DBG_FS        0x40
#define DBG_VBUF      0x80

extern unsigned i915_debug;

static inline bool
I915_DBG_ON(unsigned flags)
{
   return i915_debug & flags;
}

static inline void
I915_DBG(unsigned flags, const char *fmt, ...)
{
   if (I915_DBG_ON(flags)) {
      va_list args;

      va_start(args, fmt);
      debug_vprintf(fmt, args);
      va_end(args);
   }
}

void i915_debug_init(struct i915_screen *i915);

void i915_dump_batchbuffer(struct i915_winsys_batchbuffer *i915);

void i915_dump_dirty(struct i915_context *i915, const char *func);

void i915_dump_hardware_dirty(struct i915_context *i915, const char *func);

#endif
