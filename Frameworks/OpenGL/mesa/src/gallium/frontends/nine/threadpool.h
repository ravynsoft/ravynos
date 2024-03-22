/*
 * Copyright © 2012 Intel Corporation
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

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <pthread.h>

struct NineSwapChain9;

#define MAXTHREADS 1

struct threadpool {
    pthread_mutex_t m;
    pthread_cond_t new_work;

    HANDLE wthread;
    pthread_t pthread;
    struct threadpool_task *workqueue;
    BOOL shutdown;
};

typedef void (*threadpool_task_func)(void *data);

struct threadpool_task {
    threadpool_task_func work;
    void *data;
    struct threadpool_task *next;
    pthread_cond_t finish;
    BOOL finished;
};

struct threadpool *_mesa_threadpool_create(struct NineSwapChain9 *swapchain);
void _mesa_threadpool_destroy(struct NineSwapChain9 *swapchain, struct threadpool *pool);
struct threadpool_task *_mesa_threadpool_queue_task(struct threadpool *pool,
                                                    threadpool_task_func func,
                                                    void *data);
void _mesa_threadpool_wait_for_task(struct threadpool *pool,
                                    struct threadpool_task **task);
#endif
