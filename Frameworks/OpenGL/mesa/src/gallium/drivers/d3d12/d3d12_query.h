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

#ifndef D3D12_QUERY_H
#define D3D12_QUERY_H

#include "d3d12_common.h"
#include "d3d12_resource.h"

struct d3d12_context;

void
d3d12_suspend_queries(struct d3d12_context *ctx);

void
d3d12_resume_queries(struct d3d12_context *ctx);

void
d3d12_validate_queries(struct d3d12_context *ctx);

void
d3d12_enable_predication(struct d3d12_context *ctx);

constexpr unsigned MAX_SUBQUERIES = 4;

struct d3d12_query_impl {
   ID3D12QueryHeap* query_heap;
   unsigned curr_query, num_queries;
   size_t query_size;

   D3D12_QUERY_TYPE d3d12qtype;

   pipe_resource* buffer;
   unsigned buffer_offset;

   bool active;
};

struct d3d12_query {
   struct threaded_query base;
   struct pipe_reference reference;
   enum pipe_query_type type;
   unsigned index;

   struct d3d12_query_impl subqueries[MAX_SUBQUERIES];

   struct list_head active_list;
   struct d3d12_resource* predicate;

   /* 
   * Used to track if a query's results are ready to be read asynchronously
   * 
   * Initialized to 0, it is set to UINT64_MAX when the query is ended but before it is flushed
   * At flush, it is set to the fence_value associated with the work it was
   * submitted with
   */
   uint64_t fence_value;
};

void
d3d12_destroy_query(d3d12_query *query);

#endif
