/*
 * Copyright 2016 Patrick Rudolph <siro@das-labor.org>
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef _NINE_QUEUE_H_
#define _NINE_QUEUE_H_

#include "util/compiler.h"

struct nine_queue_pool;

void
nine_queue_wait_flush(struct nine_queue_pool* ctx);

void *
nine_queue_get(struct nine_queue_pool* ctx);

void
nine_queue_flush(struct nine_queue_pool* ctx);

void *
nine_queue_alloc(struct nine_queue_pool* ctx, unsigned space);

bool
nine_queue_no_flushed_work(struct nine_queue_pool* ctx);

bool
nine_queue_isempty(struct nine_queue_pool* ctx);

struct nine_queue_pool*
nine_queue_create(void);

void
nine_queue_delete(struct nine_queue_pool *ctx);

#endif /* _NINE_QUEUE_H_ */
