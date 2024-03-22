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

#include "u_async_debug.h"

#include <stdio.h>
#include <stdarg.h>

#include "util/u_debug.h"
#include "util/u_string.h"

static void
u_async_debug_message(void *data, unsigned *id, enum util_debug_type type,
                      const char *fmt, va_list args)
{
   struct util_async_debug_callback *adbg = data;
   struct util_debug_message *msg;
   char *text;
   int r;

   r = vasprintf(&text, fmt, args);
   if (r < 0)
      return;

   simple_mtx_lock(&adbg->lock);
   if (adbg->count >= adbg->max) {
      size_t new_max = MAX2(16, adbg->max * 2);

      if (new_max < adbg->max ||
          new_max > SIZE_MAX / sizeof(*adbg->messages)) {
         free(text);
         goto out;
      }

      struct util_debug_message *new_msg =
         realloc(adbg->messages, new_max * sizeof(*adbg->messages));
      if (!new_msg) {
         free(text);
         goto out;
      }

      adbg->max = new_max;
      adbg->messages = new_msg;
   }

   msg = &adbg->messages[adbg->count++];
   msg->id = id;
   msg->type = type;
   msg->msg = text;

out:
   simple_mtx_unlock(&adbg->lock);
}

void
u_async_debug_init(struct util_async_debug_callback *adbg)
{
   memset(adbg, 0, sizeof(*adbg));

   simple_mtx_init(&adbg->lock, mtx_plain);
   adbg->base.async = true;
   adbg->base.debug_message = u_async_debug_message;
   adbg->base.data = adbg;
}

void
u_async_debug_cleanup(struct util_async_debug_callback *adbg)
{
   simple_mtx_destroy(&adbg->lock);

   for (unsigned i = 0; i < adbg->count; ++i)
      free(adbg->messages[i].msg);
   free(adbg->messages);
}

void
_u_async_debug_drain(struct util_async_debug_callback *adbg,
                     struct util_debug_callback *dst)
{
   simple_mtx_lock(&adbg->lock);
   for (unsigned i = 0; i < adbg->count; ++i) {
      const struct util_debug_message *msg = &adbg->messages[i];

      _util_debug_message(dst, msg->id, msg->type, "%s", msg->msg);

      free(msg->msg);
   }

   adbg->count = 0;
   simple_mtx_unlock(&adbg->lock);
}

