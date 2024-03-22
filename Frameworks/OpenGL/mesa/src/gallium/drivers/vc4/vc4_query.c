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
 * Expose V3D HW perf counters.
 *
 * We also have code to fake support for occlusion queries.
 * Since we expose support for GL 2.0, we have to expose occlusion queries,
 * but the spec allows you to expose 0 query counter bits, so we just return 0
 * as the result of all our queries.
 */
#include "vc4_context.h"

struct vc4_query
{
        unsigned num_queries;
        struct vc4_hwperfmon *hwperfmon;
};

static const char *v3d_counter_names[] = {
        "FEP-valid-primitives-no-rendered-pixels",
        "FEP-valid-primitives-rendered-pixels",
        "FEP-clipped-quads",
        "FEP-valid-quads",
        "TLB-quads-not-passing-stencil-test",
        "TLB-quads-not-passing-z-and-stencil-test",
        "TLB-quads-passing-z-and-stencil-test",
        "TLB-quads-with-zero-coverage",
        "TLB-quads-with-non-zero-coverage",
        "TLB-quads-written-to-color-buffer",
        "PTB-primitives-discarded-outside-viewport",
        "PTB-primitives-need-clipping",
        "PTB-primitives-discarded-reversed",
        "QPU-total-idle-clk-cycles",
        "QPU-total-clk-cycles-vertex-coord-shading",
        "QPU-total-clk-cycles-fragment-shading",
        "QPU-total-clk-cycles-executing-valid-instr",
        "QPU-total-clk-cycles-waiting-TMU",
        "QPU-total-clk-cycles-waiting-scoreboard",
        "QPU-total-clk-cycles-waiting-varyings",
        "QPU-total-instr-cache-hit",
        "QPU-total-instr-cache-miss",
        "QPU-total-uniform-cache-hit",
        "QPU-total-uniform-cache-miss",
        "TMU-total-text-quads-processed",
        "TMU-total-text-cache-miss",
        "VPM-total-clk-cycles-VDW-stalled",
        "VPM-total-clk-cycles-VCD-stalled",
        "L2C-total-cache-hit",
        "L2C-total-cache-miss",
};

int vc4_get_driver_query_group_info(struct pipe_screen *pscreen,
                                    unsigned index,
                                    struct pipe_driver_query_group_info *info)
{
        struct vc4_screen *screen = vc4_screen(pscreen);

        if (!screen->has_perfmon_ioctl)
                return 0;

        if (!info)
                return 1;

        if (index > 0)
                return 0;

        info->name = "V3D counters";
        info->max_active_queries = DRM_VC4_MAX_PERF_COUNTERS;
        info->num_queries = ARRAY_SIZE(v3d_counter_names);
        return 1;
}

int vc4_get_driver_query_info(struct pipe_screen *pscreen, unsigned index,
                              struct pipe_driver_query_info *info)
{
        struct vc4_screen *screen = vc4_screen(pscreen);

        if (!screen->has_perfmon_ioctl)
                return 0;

        if (!info)
                return ARRAY_SIZE(v3d_counter_names);

        if (index >= ARRAY_SIZE(v3d_counter_names))
                return 0;

        info->group_id = 0;
        info->name = v3d_counter_names[index];
        info->query_type = PIPE_QUERY_DRIVER_SPECIFIC + index;
        info->result_type = PIPE_DRIVER_QUERY_RESULT_TYPE_CUMULATIVE;
        info->type = PIPE_DRIVER_QUERY_TYPE_UINT64;
        info->flags = PIPE_DRIVER_QUERY_FLAG_BATCH;
        return 1;
}

static struct pipe_query *
vc4_create_batch_query(struct pipe_context *pctx, unsigned num_queries,
                       unsigned *query_types)
{
        struct vc4_query *query = calloc(1, sizeof(*query));
        struct vc4_hwperfmon *hwperfmon;
        unsigned i, nhwqueries = 0;

        if (!query)
                return NULL;

        for (i = 0; i < num_queries; i++) {
                if (query_types[i] >= PIPE_QUERY_DRIVER_SPECIFIC)
                        nhwqueries++;
        }

        /* We can't mix HW and non-HW queries. */
        if (nhwqueries && nhwqueries != num_queries)
                goto err_free_query;

        if (!nhwqueries)
                return (struct pipe_query *)query;

        hwperfmon = calloc(1, sizeof(*hwperfmon));
        if (!hwperfmon)
                goto err_free_query;

        for (i = 0; i < num_queries; i++)
                hwperfmon->events[i] = query_types[i] -
                                       PIPE_QUERY_DRIVER_SPECIFIC;

        query->hwperfmon = hwperfmon;
        query->num_queries = num_queries;

        /* Note that struct pipe_query isn't actually defined anywhere. */
        return (struct pipe_query *)query;

err_free_query:
        free(query);

        return NULL;
}

static struct pipe_query *
vc4_create_query(struct pipe_context *ctx, unsigned query_type, unsigned index)
{
        return vc4_create_batch_query(ctx, 1, &query_type);
}

static void
vc4_destroy_query(struct pipe_context *pctx, struct pipe_query *pquery)
{
        struct vc4_context *ctx = vc4_context(pctx);
        struct vc4_query *query = (struct vc4_query *)pquery;

        if (query->hwperfmon && query->hwperfmon->id) {
                if (query->hwperfmon->id) {
                        struct drm_vc4_perfmon_destroy req = { };

                        req.id = query->hwperfmon->id;
                        vc4_ioctl(ctx->fd, DRM_IOCTL_VC4_PERFMON_DESTROY,
                                  &req);
                }

                free(query->hwperfmon);
        }

        free(query);
}

static bool
vc4_begin_query(struct pipe_context *pctx, struct pipe_query *pquery)
{
        struct vc4_query *query = (struct vc4_query *)pquery;
        struct vc4_context *ctx = vc4_context(pctx);
        struct drm_vc4_perfmon_create req = { };
        unsigned i;
        int ret;

        if (!query->hwperfmon)
                return true;

        /* Only one perfmon can be activated per context. */
        if (ctx->perfmon)
                return false;

        /* Reset the counters by destroying the previously allocated perfmon */
        if (query->hwperfmon->id) {
                struct drm_vc4_perfmon_destroy destroyreq = { };

                destroyreq.id = query->hwperfmon->id;
                vc4_ioctl(ctx->fd, DRM_IOCTL_VC4_PERFMON_DESTROY, &destroyreq);
        }

        for (i = 0; i < query->num_queries; i++)
                req.events[i] = query->hwperfmon->events[i];

        req.ncounters = query->num_queries;
        ret = vc4_ioctl(ctx->fd, DRM_IOCTL_VC4_PERFMON_CREATE, &req);
        if (ret)
                return false;

        query->hwperfmon->id = req.id;

        /* Make sure all pendings jobs are flushed before activating the
         * perfmon.
         */
        vc4_flush(pctx);
        ctx->perfmon = query->hwperfmon;
        return true;
}

static bool
vc4_end_query(struct pipe_context *pctx, struct pipe_query *pquery)
{
        struct vc4_query *query = (struct vc4_query *)pquery;
        struct vc4_context *ctx = vc4_context(pctx);

        if (!query->hwperfmon)
                return true;

        if (ctx->perfmon != query->hwperfmon)
                return false;

        /* Make sure all pendings jobs are flushed before deactivating the
         * perfmon.
         */
        vc4_flush(pctx);
        ctx->perfmon = NULL;
        return true;
}

static bool
vc4_get_query_result(struct pipe_context *pctx, struct pipe_query *pquery,
                     bool wait, union pipe_query_result *vresult)
{
        struct vc4_context *ctx = vc4_context(pctx);
        struct vc4_query *query = (struct vc4_query *)pquery;
        struct drm_vc4_perfmon_get_values req;
        unsigned i;
        int ret;

        if (!query->hwperfmon) {
                vresult->u64 = 0;
                return true;
        }

        if (!vc4_wait_seqno(ctx->screen, query->hwperfmon->last_seqno,
                            wait ? OS_TIMEOUT_INFINITE : 0, "perfmon"))
                return false;

        req.id = query->hwperfmon->id;
        req.values_ptr = (uintptr_t)query->hwperfmon->counters;
        ret = vc4_ioctl(ctx->fd, DRM_IOCTL_VC4_PERFMON_GET_VALUES, &req);
        if (ret)
                return false;

        for (i = 0; i < query->num_queries; i++)
                vresult->batch[i].u64 = query->hwperfmon->counters[i];

        return true;
}

static void
vc4_set_active_query_state(struct pipe_context *pctx, bool enable)
{
}

void
vc4_query_init(struct pipe_context *pctx)
{
        pctx->create_query = vc4_create_query;
        pctx->create_batch_query = vc4_create_batch_query;
        pctx->destroy_query = vc4_destroy_query;
        pctx->begin_query = vc4_begin_query;
        pctx->end_query = vc4_end_query;
        pctx->get_query_result = vc4_get_query_result;
        pctx->set_active_query_state = vc4_set_active_query_state;
}
