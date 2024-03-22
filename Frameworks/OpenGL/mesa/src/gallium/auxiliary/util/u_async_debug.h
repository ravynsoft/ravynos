/*
 * Copyright 2017 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/**
 * \file u_async_debug.h
 * Provides a helper implementation of util_debug_callback which allows debug
 * messages from non-application threads to be passed back to the application
 * thread.
 */

#ifndef UTIL_ASYNC_DEBUG_H
#define UTIL_ASYNC_DEBUG_H

#include "pipe/p_state.h"
#include "util/u_debug.h"
#include "util/simple_mtx.h"

#ifdef __cplusplus
extern "C" {
#endif

struct util_debug_message {
   unsigned *id;
   enum util_debug_type type;
   char *msg;
};

struct util_async_debug_callback {
   struct util_debug_callback base;

   simple_mtx_t lock;
   unsigned count;
   unsigned max;
   struct util_debug_message *messages;
};

void
u_async_debug_init(struct util_async_debug_callback *adbg);
void
u_async_debug_cleanup(struct util_async_debug_callback *adbg);

void
_u_async_debug_drain(struct util_async_debug_callback *adbg,
                     struct util_debug_callback *dst);

static inline void
u_async_debug_drain(struct util_async_debug_callback *adbg,
                    struct util_debug_callback *dst)
{
   /* Read the count without taking the lock to avoid atomics in the fast path.
    * We'll re-read the count after taking the lock. */
   if (adbg->count)
      _u_async_debug_drain(adbg, dst);
}

#ifdef __cplusplus
}
#endif

#endif /* UTIL_ASYNC_DEBUG_H */
