/**************************************************************************
 *
 * Copyright 2013 Marek Olšák <maraeo@gmail.com>
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
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef HUD_CONTEXT_H
#define HUD_CONTEXT_H

struct hud_context;
struct cso_context;
struct pipe_context;
struct pipe_resource;
struct util_queue_monitoring;
struct st_context;

typedef void (*hud_st_invalidate_state_func)(struct st_context *st,
                                             unsigned flags);

struct hud_context *
hud_create(struct cso_context *cso, struct hud_context *share,
           struct st_context *st,
           hud_st_invalidate_state_func st_invalidate_state);

void
hud_destroy(struct hud_context *hud, struct cso_context *cso);

void
hud_run(struct hud_context *hud, struct cso_context *cso,
        struct pipe_resource *tex);

void
hud_record_only(struct hud_context *hud, struct pipe_context *pipe);

void
hud_add_queue_for_monitoring(struct hud_context *hud,
                             struct util_queue_monitoring *queue_info);

#endif
