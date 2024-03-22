/*
 * Copyright 2017 Advanced Micro Devices, Inc.
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
 */

/**
 * @file u_log.h
 * @brief Context logging facilities
 *
 * Provides a means of logging context events (draw calls, command streams, ...)
 * into files.
 *
 * Log entries start their life cycle as "chunks". Chunks can be plain text
 * written by \ref u_log_printf or custom internal representations added by
 * \ref u_log_chunk that are only converted to text on-demand (e.g. for higher
 * performance pipelined hang-debugging).
 *
 * Chunks are accumulated into "pages". The manager of the log can periodically
 * take out the current page using \ref u_log_new_page and dump it to a file.
 *
 * Furthermore, "auto loggers" can be added to a context, which are callbacks
 * that are given the opportunity to add their own logging each time a chunk is
 * added. Drivers can use this to lazily log chunks of their command stream.
 * Lazy loggers don't need to be re-entrant.
 */

#ifndef U_LOG_H
#define U_LOG_H

#include <stdio.h>

#include "util/u_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

struct u_log_page;
struct u_log_auto_logger;

struct u_log_chunk_type {
   void (*destroy)(void *data);
   void (*print)(void *data, FILE *stream);
};

struct u_log_context {
   struct u_log_page *cur;
   struct u_log_auto_logger *auto_loggers;
   unsigned num_auto_loggers;
};

typedef void (u_auto_log_fn)(void *data, struct u_log_context *ctx);

void
u_log_context_init(struct u_log_context *ctx);

void
u_log_context_destroy(struct u_log_context *ctx);

void
u_log_add_auto_logger(struct u_log_context *ctx, u_auto_log_fn *callback,
                      void *data);

void
u_log_flush(struct u_log_context *ctx);

void
u_log_printf(struct u_log_context *ctx, const char *fmt, ...) _util_printf_format(2,3);

void
u_log_chunk(struct u_log_context *ctx, const struct u_log_chunk_type *type,
            void *data);

void
u_log_new_page_print(struct u_log_context *ctx, FILE *stream);

struct u_log_page *
u_log_new_page(struct u_log_context *ctx);

void
u_log_page_destroy(struct u_log_page *page);

void
u_log_page_print(struct u_log_page *page, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif /* U_LOG_H */
