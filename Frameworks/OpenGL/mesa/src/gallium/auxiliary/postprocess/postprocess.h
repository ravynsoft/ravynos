/**************************************************************************
 *
 * Copyright 2011 Lauri Kasanen
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
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include "pipe/p_state.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cso_context;
struct st_context;

struct pp_queue_t;              /* Forward definition */
struct pp_program;

/* Less typing later on */
typedef void (*pp_func) (struct pp_queue_t *, struct pipe_resource *,
                         struct pipe_resource *, unsigned int);

typedef void (*pp_st_invalidate_state_func)(struct st_context *st,
                                            unsigned flags);

/* Main functions */

/**
 * Note enabled is an array of values, one per filter stage.
 * Zero indicates the stage is disabled.  Non-zero indicates the
 * stage is enabled.  For some stages, the value controls quality.
 */
struct pp_queue_t *pp_init(struct pipe_context *pipe,
                           const unsigned int *enabled,
                           struct cso_context *,
                           struct st_context *st,
                           pp_st_invalidate_state_func st_invalidate_state);

void pp_run(struct pp_queue_t *, struct pipe_resource *,
            struct pipe_resource *, struct pipe_resource *);
void pp_free(struct pp_queue_t *);

void pp_init_fbos(struct pp_queue_t *, unsigned int, unsigned int);


/* The filters */

void pp_nocolor(struct pp_queue_t *, struct pipe_resource *,
                struct pipe_resource *, unsigned int);

void pp_jimenezmlaa(struct pp_queue_t *, struct pipe_resource *,
                    struct pipe_resource *, unsigned int);
void pp_jimenezmlaa_color(struct pp_queue_t *, struct pipe_resource *,
                          struct pipe_resource *, unsigned int);

/* The filter init functions */

bool pp_celshade_init(struct pp_queue_t *, unsigned int, unsigned int);

bool pp_nored_init(struct pp_queue_t *, unsigned int, unsigned int);
bool pp_nogreen_init(struct pp_queue_t *, unsigned int, unsigned int);
bool pp_noblue_init(struct pp_queue_t *, unsigned int, unsigned int);

bool pp_jimenezmlaa_init(struct pp_queue_t *, unsigned int, unsigned int);
bool pp_jimenezmlaa_init_color(struct pp_queue_t *, unsigned int,
                               unsigned int);

/* The filter free functions */

void pp_celshade_free(struct pp_queue_t *, unsigned int);
void pp_nocolor_free(struct pp_queue_t *, unsigned int);
void pp_jimenezmlaa_free(struct pp_queue_t *, unsigned int);


#ifdef __cplusplus
}
#endif

#endif
