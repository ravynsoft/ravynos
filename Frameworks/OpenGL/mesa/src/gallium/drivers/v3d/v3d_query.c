/*
 * Copyright © 2014 Broadcom
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

#include "v3d_query.h"

int
v3d_get_driver_query_group_info(struct pipe_screen *pscreen, unsigned index,
                                struct pipe_driver_query_group_info *info)
{
        struct v3d_screen *screen = v3d_screen(pscreen);
        struct v3d_device_info *devinfo = &screen->devinfo;

        return v3d_X(devinfo, get_driver_query_group_info_perfcnt)(screen,
                                                                   index,
                                                                   info);
}

int
v3d_get_driver_query_info(struct pipe_screen *pscreen, unsigned index,
                          struct pipe_driver_query_info *info)
{
        struct v3d_screen *screen = v3d_screen(pscreen);
        struct v3d_device_info *devinfo = &screen->devinfo;

        return v3d_X(devinfo, get_driver_query_info_perfcnt)(screen,
                                                             index,
                                                             info);
}

static struct pipe_query *
v3d_create_query(struct pipe_context *pctx, unsigned query_type, unsigned index)
{
        struct v3d_context *v3d = v3d_context(pctx);

        return v3d_create_query_pipe(v3d, query_type, index);
}

static struct pipe_query *
v3d_create_batch_query(struct pipe_context *pctx, unsigned num_queries,
                       unsigned *query_types)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_screen *screen = v3d->screen;
        struct v3d_device_info *devinfo = &screen->devinfo;

        return v3d_X(devinfo, create_batch_query_perfcnt)(v3d_context(pctx),
                                                          num_queries,
                                                          query_types);
}

static void
v3d_destroy_query(struct pipe_context *pctx, struct pipe_query *query)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_query *q = (struct v3d_query *)query;

        q->funcs->destroy_query(v3d, q);
}

static bool
v3d_begin_query(struct pipe_context *pctx, struct pipe_query *query)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_query *q = (struct v3d_query *)query;

        return q->funcs->begin_query(v3d, q);
}

static bool
v3d_end_query(struct pipe_context *pctx, struct pipe_query *query)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_query *q = (struct v3d_query *)query;

        return q->funcs->end_query(v3d, q);
}

static bool
v3d_get_query_result(struct pipe_context *pctx, struct pipe_query *query,
                     bool wait, union pipe_query_result *vresult)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_query *q = (struct v3d_query *)query;

        return q->funcs->get_query_result(v3d, q, wait, vresult);
}

static void
v3d_set_active_query_state(struct pipe_context *pctx, bool enable)
{
        struct v3d_context *v3d = v3d_context(pctx);

        v3d->active_queries = enable;
        v3d->dirty |= V3D_DIRTY_OQ;
        v3d->dirty |= V3D_DIRTY_STREAMOUT;
}

static void
v3d_render_condition(struct pipe_context *pipe,
                     struct pipe_query *query,
                     bool condition,
                     enum pipe_render_cond_flag mode)
{
        struct v3d_context *v3d = v3d_context(pipe);

        v3d->cond_query = query;
        v3d->cond_cond = condition;
        v3d->cond_mode = mode;
}

void
v3d_query_init(struct pipe_context *pctx)
{
        pctx->create_query = v3d_create_query;
        pctx->create_batch_query = v3d_create_batch_query;
        pctx->destroy_query = v3d_destroy_query;
        pctx->begin_query = v3d_begin_query;
        pctx->end_query = v3d_end_query;
        pctx->get_query_result = v3d_get_query_result;
        pctx->set_active_query_state = v3d_set_active_query_state;
        pctx->render_condition = v3d_render_condition;
}
