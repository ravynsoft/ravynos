/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef D3D12_BATCH_H
#define D3D12_BATCH_H

#include "util/u_dynarray.h"
#include "util/hash_table.h"
#include "pipe/p_state.h"
#include <stdint.h>

#include "d3d12_common.h"

struct d3d12_bo;
struct d3d12_descriptor_heap;
struct d3d12_fence;


struct d3d12_sampler_desc_table_key
{
   D3D12_CPU_DESCRIPTOR_HANDLE descs[PIPE_MAX_SHADER_SAMPLER_VIEWS];
   unsigned count;
};

struct d3d12_batch {
   struct d3d12_fence *fence;

   struct hash_table *bos;
   struct util_dynarray local_bos;
   struct hash_table *sampler_tables;
   struct set *sampler_views;
   struct set *surfaces;
   struct set *objects;
   struct set *queries;

   struct util_dynarray zombie_samplers;

   ID3D12CommandAllocator *cmdalloc;
   struct d3d12_descriptor_heap *sampler_heap;
   struct d3d12_descriptor_heap *view_heap;
   bool has_errors;
   bool pending_memory_barrier;

   uint64_t submit_id;
   uint32_t ctx_id, ctx_index;
};

bool
d3d12_init_batch(struct d3d12_context *ctx, struct d3d12_batch *batch);

void
d3d12_destroy_batch(struct d3d12_context *ctx, struct d3d12_batch *batch);

void
d3d12_start_batch(struct d3d12_context *ctx, struct d3d12_batch *batch);

void
d3d12_end_batch(struct d3d12_context *ctx, struct d3d12_batch *batch);

bool
d3d12_reset_batch(struct d3d12_context *ctx, struct d3d12_batch *batch, uint64_t timeout_ns);

bool
d3d12_batch_has_references(struct d3d12_batch *batch,
                           struct d3d12_bo *bo,
                           bool want_to_write);

void
d3d12_batch_reference_resource(struct d3d12_batch *batch,
                               struct d3d12_resource *res,
                               bool write);

void
d3d12_batch_reference_sampler_view(struct d3d12_batch *batch,
                                   struct d3d12_sampler_view *sv);

void
d3d12_batch_reference_surface_texture(struct d3d12_batch *batch,
                              struct d3d12_surface *surf);

void
d3d12_batch_reference_object(struct d3d12_batch *batch,
                             ID3D12Object *object);

void
d3d12_batch_reference_query(struct d3d12_batch *batch,
                            struct d3d12_query *query);

#endif
