/*
 * Copyright © 2019 Intel Corporation
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

#ifndef INTEL_PERF_QUERY_H
#define INTEL_PERF_QUERY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct intel_device_info;

struct intel_perf_config;
struct intel_perf_context;
struct intel_perf_query_object;

bool
intel_perf_open(struct intel_perf_context *perf_ctx,
                int metrics_set_id,
                int report_format,
                int period_exponent,
                int drm_fd,
                uint32_t ctx_id,
                bool enable);

void
intel_perf_close(struct intel_perf_context *perfquery,
                 const struct intel_perf_query_info *query);

bool intel_perf_oa_stream_ready(struct intel_perf_context *perf_ctx);

ssize_t
intel_perf_read_oa_stream(struct intel_perf_context *perf_ctx,
                          void* buf,
                          size_t nbytes);

struct intel_perf_context *intel_perf_new_context(void *parent);

void intel_perf_init_context(struct intel_perf_context *perf_ctx,
                             struct intel_perf_config *perf_cfg,
                             void * mem_ctx, /* ralloc context */
                             void * ctx,  /* driver context (eg, brw_context) */
                             void * bufmgr,  /* eg brw_bufmgr */
                             const struct intel_device_info *devinfo,
                             uint32_t hw_ctx,
                             int drm_fd);

const struct intel_perf_query_info* intel_perf_query_info(const struct intel_perf_query_object *);

struct intel_perf_config *intel_perf_config(struct intel_perf_context *ctx);

int intel_perf_active_queries(struct intel_perf_context *perf_ctx,
                            const struct intel_perf_query_info *query);

struct intel_perf_query_object *
intel_perf_new_query(struct intel_perf_context *, unsigned query_index);


bool intel_perf_begin_query(struct intel_perf_context *perf_ctx,
                            struct intel_perf_query_object *query);
void intel_perf_end_query(struct intel_perf_context *perf_ctx,
                          struct intel_perf_query_object *query);
void intel_perf_wait_query(struct intel_perf_context *perf_ctx,
                           struct intel_perf_query_object *query,
                           void *current_batch);
bool intel_perf_is_query_ready(struct intel_perf_context *perf_ctx,
                               struct intel_perf_query_object *query,
                               void *current_batch);
void intel_perf_delete_query(struct intel_perf_context *perf_ctx,
                             struct intel_perf_query_object *query);
void intel_perf_get_query_data(struct intel_perf_context *perf_ctx,
                               struct intel_perf_query_object *query,
                               void *current_batch,
                               int data_size,
                               unsigned *data,
                               unsigned *bytes_written);

void intel_perf_dump_query_count(struct intel_perf_context *perf_ctx);
void intel_perf_dump_query(struct intel_perf_context *perf_ctx,
                           struct intel_perf_query_object *obj,
                           void *current_batch);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INTEL_PERF_QUERY_H */
