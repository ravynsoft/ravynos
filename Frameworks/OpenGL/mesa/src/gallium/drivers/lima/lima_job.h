/*
 * Copyright (C) 2018-2019 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef H_LIMA_JOB
#define H_LIMA_JOB

#include <stdbool.h>
#include <stdint.h>

#include <util/u_dynarray.h>

#include <pipe/p_state.h>

#define MAX_DRAWS_PER_JOB 2500

struct lima_context;
struct lima_bo;
struct lima_dump;
struct pipe_surface;

struct lima_job_key {
   struct pipe_surface *cbuf;
   struct pipe_surface *zsbuf;
};

struct lima_job_clear {
   unsigned buffers;
   uint32_t color_8pc;
   uint32_t depth;
   uint32_t stencil;
   uint64_t color_16pc;
};

struct lima_job_fb_info {
   int width, height;
   int tiled_w, tiled_h;
   int shift_w, shift_h;
   int block_w, block_h;
   int shift_min;
};

struct lima_job {
   int fd;
   struct lima_context *ctx;

   struct util_dynarray gem_bos[2];
   struct util_dynarray bos[2];

   struct lima_job_key key;

   struct util_dynarray vs_cmd_array;
   struct util_dynarray plbu_cmd_array;
   struct util_dynarray plbu_cmd_head;

   unsigned resolve;

   int pp_max_stack_size;

   struct pipe_scissor_state damage_rect;

   struct lima_job_clear clear;

   struct lima_job_fb_info fb;

   int draws;

   /* for dump command stream */
   struct lima_dump *dump;
};

static inline bool
lima_job_has_draw_pending(struct lima_job *job)
{
   return !!job->plbu_cmd_array.size;
}

struct lima_job *lima_job_get(struct lima_context *ctx);
struct lima_job * lima_job_get_with_fb(struct lima_context *ctx,
                                       struct pipe_surface *cbuf,
                                       struct pipe_surface *zsbuf);

bool lima_job_add_bo(struct lima_job *job, int pipe,
                     struct lima_bo *bo, uint32_t flags);
void *lima_job_create_stream_bo(struct lima_job *job, int pipe,
                                unsigned size, uint32_t *va);

void lima_do_job(struct lima_job *job);

bool lima_job_init(struct lima_context *ctx);
void lima_job_fini(struct lima_context *ctx);

#endif
