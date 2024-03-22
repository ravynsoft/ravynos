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

#ifndef D3D12_RESOURCE_STATE_H
#define D3D12_RESOURCE_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "d3d12_common.h"

#include "util/hash_table.h"

struct d3d12_context;

const D3D12_RESOURCE_STATES RESOURCE_STATE_ALL_WRITE_BITS =
   D3D12_RESOURCE_STATE_RENDER_TARGET | D3D12_RESOURCE_STATE_UNORDERED_ACCESS | D3D12_RESOURCE_STATE_DEPTH_WRITE |
   D3D12_RESOURCE_STATE_STREAM_OUT | D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_RESOLVE_DEST |
   D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE | D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE;

inline bool
d3d12_is_write_state(D3D12_RESOURCE_STATES state)
{
   return (state & RESOURCE_STATE_ALL_WRITE_BITS) != D3D12_RESOURCE_STATE_COMMON;
}

inline bool
d3d12_resource_supports_simultaneous_access(const D3D12_RESOURCE_DESC *desc)
{
   return desc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER ||
          (desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS) != D3D12_RESOURCE_FLAG_NONE;
}

struct d3d12_subresource_state
{
   D3D12_RESOURCE_STATES state;
   uint64_t execution_id;
   bool is_promoted;
   bool may_decay;
};

/* Stores the current state of either an entire resource, or each subresource */
struct d3d12_resource_state
{
   bool homogenous;
   bool supports_simultaneous_access;
   uint32_t num_subresources;
   d3d12_subresource_state *subresource_states;
};

/* Stores the current desired state of either an entire resource, or each subresource. */
struct d3d12_desired_resource_state
{
   bool homogenous;
   bool pending_memory_barrier;
   uint32_t num_subresources;
   D3D12_RESOURCE_STATES* subresource_states;
};

struct d3d12_context_state_table_entry
{
   struct d3d12_desired_resource_state desired;
   struct d3d12_resource_state batch_begin, batch_end;
};


bool
d3d12_resource_state_init(d3d12_resource_state *state, uint32_t subresource_count, bool simultaneous_access);

void
d3d12_resource_state_cleanup(d3d12_resource_state *state);

void
d3d12_context_state_table_init(struct d3d12_context *ctx);

void
d3d12_context_state_table_destroy(struct d3d12_context *ctx);

bool
d3d12_context_state_resolve_submission(struct d3d12_context *ctx, struct d3d12_batch *batch);

void
d3d12_destroy_context_state_table_entry(d3d12_context_state_table_entry* entry);

#endif // D3D12_RESOURCE_STATE_H
