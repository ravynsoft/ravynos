//
// Copyright 2013 Francisco Jerez
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

#include "core/timestamp.hpp"
#include "core/queue.hpp"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"

using namespace clover;

timestamp::query::query(command_queue &q) :
   q(q),
   _query(q.pipe->create_query(q.pipe, PIPE_QUERY_TIMESTAMP, 0)) {
   q.pipe->end_query(q.pipe, _query);
}

timestamp::query::query(query &&other) :
   q(other.q),
   _query(other._query) {
   other._query = NULL;
}

timestamp::query::~query() {
   if (_query)
      q().pipe->destroy_query(q().pipe, _query);
}

cl_ulong
timestamp::query::operator()() const {
   pipe_query_result result;

   if (!q().pipe->get_query_result(q().pipe, _query, false, &result))
      throw error(CL_PROFILING_INFO_NOT_AVAILABLE);

   return result.u64;
}

timestamp::current::current(command_queue &q) :
   result(q.pipe->screen->get_timestamp(q.pipe->screen)) {
}

cl_ulong
timestamp::current::operator()() const {
   return result;
}
