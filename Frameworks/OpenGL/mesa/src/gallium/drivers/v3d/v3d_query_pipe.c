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

/**
 * Gallium query object support.
 *
 * The HW has native support for occlusion queries, with the query result
 * being loaded and stored by the TLB unit. From a SW perspective, we have to
 * be careful to make sure that the jobs that need to be tracking queries are
 * bracketed by the start and end of counting, even across FBO transitions.
 *
 * For the transform feedback PRIMITIVES_GENERATED/WRITTEN queries, we have to
 * do the calculations in software at draw time.
 */

#include "v3d_query.h"

struct v3d_query_pipe
{
        struct v3d_query base;

        enum pipe_query_type type;
        struct v3d_bo *bo;

        uint32_t start, end;
        uint32_t result;
};

static void
v3d_destroy_query_pipe(struct v3d_context *v3d, struct v3d_query *query)
{
        struct v3d_query_pipe *pquery = (struct v3d_query_pipe *)query;

        v3d_bo_unreference(&pquery->bo);
        free(pquery);
}

static bool
v3d_begin_query_pipe(struct v3d_context *v3d, struct v3d_query *query)
{
        struct v3d_query_pipe *pquery = (struct v3d_query_pipe *)query;

        switch (pquery->type) {
        case PIPE_QUERY_PRIMITIVES_GENERATED:
                /* If we are using PRIMITIVE_COUNTS_FEEDBACK to retrieve
                 * primitive counts from the GPU (which we need when a GS
                 * is present), then we need to update our counters now
                 * to discard any primitives generated before this.
                 */
                if (v3d->prog.gs)
                        v3d_update_primitive_counters(v3d);
                pquery->start = v3d->prims_generated;
                v3d->n_primitives_generated_queries_in_flight++;
                break;
        case PIPE_QUERY_PRIMITIVES_EMITTED:
                /* If we are inside transform feedback we need to update the
                 * primitive counts to skip primitives recorded before this.
                 */
                if (v3d->streamout.num_targets > 0)
                        v3d_update_primitive_counters(v3d);
                pquery->start = v3d->tf_prims_generated;
                break;
        case PIPE_QUERY_OCCLUSION_COUNTER:
        case PIPE_QUERY_OCCLUSION_PREDICATE:
        case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
                v3d_bo_unreference(&pquery->bo);
                pquery->bo = v3d_bo_alloc(v3d->screen, 4096, "query");
                uint32_t *map = v3d_bo_map(pquery->bo);
                *map = 0;

                v3d->current_oq = pquery->bo;
                v3d->dirty |= V3D_DIRTY_OQ;
                break;
        default:
                unreachable("unsupported query type");
        }

        return true;
}

static bool
v3d_end_query_pipe(struct v3d_context *v3d, struct v3d_query *query)
{
        struct v3d_query_pipe *pquery = (struct v3d_query_pipe *)query;

        switch (pquery->type) {
        case PIPE_QUERY_PRIMITIVES_GENERATED:
                /* If we are using PRIMITIVE_COUNTS_FEEDBACK to retrieve
                 * primitive counts from the GPU (which we need when a GS
                 * is present), then we need to update our counters now.
                 */
                if (v3d->prog.gs)
                        v3d_update_primitive_counters(v3d);
                pquery->end = v3d->prims_generated;
                v3d->n_primitives_generated_queries_in_flight--;
                break;
        case PIPE_QUERY_PRIMITIVES_EMITTED:
                /* If transform feedback has ended, then we have already
                 * updated the primitive counts at glEndTransformFeedback()
                 * time. Otherwise, we have to do it now.
                 */
                if (v3d->streamout.num_targets > 0)
                        v3d_update_primitive_counters(v3d);
                pquery->end = v3d->tf_prims_generated;
                break;
        case PIPE_QUERY_OCCLUSION_COUNTER:
        case PIPE_QUERY_OCCLUSION_PREDICATE:
        case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
                v3d->current_oq = NULL;
                v3d->dirty |= V3D_DIRTY_OQ;
                break;
        default:
                unreachable("unsupported query type");
        }

        return true;
}

static bool
v3d_get_query_result_pipe(struct v3d_context *v3d, struct v3d_query *query,
                          bool wait, union pipe_query_result *vresult)
{
        struct v3d_query_pipe *pquery = (struct v3d_query_pipe *)query;

        if (pquery->bo) {
                v3d_flush_jobs_using_bo(v3d, pquery->bo);

                if (wait) {
                        if (!v3d_bo_wait(pquery->bo, ~0ull, "query"))
                                return false;
                } else {
                        if (!v3d_bo_wait(pquery->bo, 0, "query"))
                                return false;
                }

                /* XXX: Sum up per-core values. */
                uint32_t *map = v3d_bo_map(pquery->bo);
                pquery->result = *map;

                v3d_bo_unreference(&pquery->bo);
        }

        switch (pquery->type) {
        case PIPE_QUERY_OCCLUSION_COUNTER:
                vresult->u64 = pquery->result;
                break;
        case PIPE_QUERY_OCCLUSION_PREDICATE:
        case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
                vresult->b = pquery->result != 0;
                break;
        case PIPE_QUERY_PRIMITIVES_GENERATED:
        case PIPE_QUERY_PRIMITIVES_EMITTED:
                vresult->u64 = pquery->end - pquery->start;
                break;
        default:
                unreachable("unsupported query type");
        }

        return true;
}

static const struct v3d_query_funcs pipe_query_funcs = {
        .destroy_query = v3d_destroy_query_pipe,
        .begin_query = v3d_begin_query_pipe,
        .end_query = v3d_end_query_pipe,
        .get_query_result = v3d_get_query_result_pipe,
};

struct pipe_query *
v3d_create_query_pipe(struct v3d_context *v3d, unsigned query_type, unsigned index)
{
        if (query_type >= PIPE_QUERY_DRIVER_SPECIFIC)
                return NULL;

        struct v3d_query_pipe *pquery = calloc(1, sizeof(*pquery));
        struct v3d_query *query = &pquery->base;

        pquery->type = query_type;
        query->funcs = &pipe_query_funcs;

        /* Note that struct pipe_query isn't actually defined anywhere. */
        return (struct pipe_query *)query;
}
