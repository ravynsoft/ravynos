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

#include "nine_queue.h"
#include "util/u_thread.h"
#include "util/macros.h"
#include "nine_helpers.h"

#define NINE_CMD_BUF_INSTR (256)

#define NINE_CMD_BUFS (32)
#define NINE_CMD_BUFS_MASK (NINE_CMD_BUFS - 1)

#define NINE_QUEUE_SIZE (8192 * 16 + 128)

#define DBG_CHANNEL DBG_DEVICE

/*
 * Single producer - single consumer pool queue
 *
 * Producer:
 * Calls nine_queue_alloc to get a slice of memory in current cmdbuf.
 * Calls nine_queue_flush to flush the queue by request.
 * The queue is flushed automatically on insufficient space or once the
 * cmdbuf contains NINE_CMD_BUF_INSTR instructions.
 *
 * nine_queue_flush does block, while nine_queue_alloc doesn't block.
 *
 * nine_queue_alloc returns NULL on insufficent space.
 *
 * Consumer:
 * Calls nine_queue_wait_flush to wait for a cmdbuf.
 * After waiting for a cmdbuf it calls nine_queue_get until NULL is returned.
 *
 * nine_queue_wait_flush does block, while nine_queue_get doesn't block.
 *
 * Constrains:
 * Only a single consumer and a single producer are supported.
 *
 */

struct nine_cmdbuf {
    unsigned instr_size[NINE_CMD_BUF_INSTR];
    unsigned num_instr;
    unsigned offset;
    void *mem_pool;
    BOOL full;
};

struct nine_queue_pool {
    struct nine_cmdbuf pool[NINE_CMD_BUFS];
    unsigned head;
    unsigned tail;
    unsigned cur_instr;
    BOOL worker_wait;
    cnd_t event_pop;
    cnd_t event_push;
    mtx_t mutex_pop;
    mtx_t mutex_push;
};

/* Consumer functions: */
void
nine_queue_wait_flush(struct nine_queue_pool* ctx)
{
    struct nine_cmdbuf *cmdbuf = &ctx->pool[ctx->tail];

    /* wait for cmdbuf full */
    mtx_lock(&ctx->mutex_push);
    while (!cmdbuf->full)
    {
        DBG("waiting for full cmdbuf\n");
        cnd_wait(&ctx->event_push, &ctx->mutex_push);
    }
    DBG("got cmdbuf=%p\n", cmdbuf);
    mtx_unlock(&ctx->mutex_push);

    cmdbuf->offset = 0;
    ctx->cur_instr = 0;
}

/* Gets a pointer to the next memory slice.
 * Does not block.
 * Returns NULL on empty cmdbuf. */
void *
nine_queue_get(struct nine_queue_pool* ctx)
{
    struct nine_cmdbuf *cmdbuf = &ctx->pool[ctx->tail];
    unsigned offset;

    /* At this pointer there's always a cmdbuf. */

    if (ctx->cur_instr == cmdbuf->num_instr) {
        /* signal waiting producer */
        mtx_lock(&ctx->mutex_pop);
        DBG("freeing cmdbuf=%p\n", cmdbuf);
        cmdbuf->full = 0;
        cnd_signal(&ctx->event_pop);
        mtx_unlock(&ctx->mutex_pop);

        ctx->tail = (ctx->tail + 1) & NINE_CMD_BUFS_MASK;

        return NULL;
    }

    /* At this pointer there's always a cmdbuf with instruction to process. */
    offset = cmdbuf->offset;
    cmdbuf->offset += cmdbuf->instr_size[ctx->cur_instr];
    ctx->cur_instr ++;

    return cmdbuf->mem_pool + offset;
}

/* Producer functions: */

/* Flushes the queue.
 * Moves the current cmdbuf to worker thread.
 * Blocks until next cmdbuf is free. */
void
nine_queue_flush(struct nine_queue_pool* ctx)
{
    struct nine_cmdbuf *cmdbuf = &ctx->pool[ctx->head];

    DBG("flushing cmdbuf=%p instr=%d size=%d\n",
           cmdbuf, cmdbuf->num_instr, cmdbuf->offset);

    /* Nothing to flush */
    if (!cmdbuf->num_instr)
        return;

    /* signal waiting worker */
    mtx_lock(&ctx->mutex_push);
    cmdbuf->full = 1;
    cnd_signal(&ctx->event_push);
    mtx_unlock(&ctx->mutex_push);

    ctx->head = (ctx->head + 1) & NINE_CMD_BUFS_MASK;

    cmdbuf = &ctx->pool[ctx->head];

    /* wait for queue empty */
    mtx_lock(&ctx->mutex_pop);
    while (cmdbuf->full)
    {
        DBG("waiting for empty cmdbuf\n");
        cnd_wait(&ctx->event_pop, &ctx->mutex_pop);
    }
    DBG("got empty cmdbuf=%p\n", cmdbuf);
    mtx_unlock(&ctx->mutex_pop);
    cmdbuf->offset = 0;
    cmdbuf->num_instr = 0;
}

/* Gets a a pointer to slice of memory with size @space.
 * Does block if queue is full.
 * Returns NULL on @space > NINE_QUEUE_SIZE. */
void *
nine_queue_alloc(struct nine_queue_pool* ctx, unsigned space)
{
    unsigned offset;
    struct nine_cmdbuf *cmdbuf = &ctx->pool[ctx->head];

    if (space > NINE_QUEUE_SIZE)
        return NULL;

    /* at this pointer there's always a free queue available */

    if ((cmdbuf->offset + space > NINE_QUEUE_SIZE) ||
        (cmdbuf->num_instr == NINE_CMD_BUF_INSTR)) {

        nine_queue_flush(ctx);

        cmdbuf = &ctx->pool[ctx->head];
    }

    DBG("cmdbuf=%p space=%d\n", cmdbuf, space);

    /* at this pointer there's always a free queue with sufficient space available */

    offset = cmdbuf->offset;
    cmdbuf->offset += space;
    cmdbuf->instr_size[cmdbuf->num_instr] = space;
    cmdbuf->num_instr ++;

    return cmdbuf->mem_pool + offset;
}

/* Returns the current queue flush state.
 * TRUE nothing flushed
 * FALSE one ore more instructions queued flushed. */
bool
nine_queue_no_flushed_work(struct nine_queue_pool* ctx)
{
    return (ctx->tail == ctx->head);
}

/* Returns the current queue empty state.
 * TRUE no instructions queued.
 * FALSE one ore more instructions queued. */
bool
nine_queue_isempty(struct nine_queue_pool* ctx)
{
    struct nine_cmdbuf *cmdbuf = &ctx->pool[ctx->head];

    return (ctx->tail == ctx->head) && !cmdbuf->num_instr;
}

struct nine_queue_pool*
nine_queue_create(void)
{
    unsigned i;
    struct nine_queue_pool *ctx;

    ctx = CALLOC_STRUCT(nine_queue_pool);
    if (!ctx)
        goto failed;

    for (i = 0; i < NINE_CMD_BUFS; i++) {
        ctx->pool[i].mem_pool = MALLOC(NINE_QUEUE_SIZE);
        if (!ctx->pool[i].mem_pool)
            goto failed;
    }

    cnd_init(&ctx->event_pop);
    (void) mtx_init(&ctx->mutex_pop, mtx_plain);

    cnd_init(&ctx->event_push);
    (void) mtx_init(&ctx->mutex_push, mtx_plain);

    /* Block until first cmdbuf has been flushed. */
    ctx->worker_wait = true;

    return ctx;
failed:
    if (ctx) {
        for (i = 0; i < NINE_CMD_BUFS; i++) {
            if (ctx->pool[i].mem_pool)
                FREE(ctx->pool[i].mem_pool);
        }
        FREE(ctx);
    }
    return NULL;
}

void
nine_queue_delete(struct nine_queue_pool *ctx)
{
    unsigned i;

    mtx_destroy(&ctx->mutex_pop);
    cnd_destroy(&ctx->event_pop);

    mtx_destroy(&ctx->mutex_push);
    cnd_destroy(&ctx->event_push);

    for (i = 0; i < NINE_CMD_BUFS; i++)
        FREE(ctx->pool[i].mem_pool);

    FREE(ctx);
}
