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

#ifndef CLOVER_CORE_TIMESTAMP_HPP
#define CLOVER_CORE_TIMESTAMP_HPP

#include "core/object.hpp"

struct pipe_query;

namespace clover {
   class command_queue;

   namespace timestamp {
      ///
      /// Emit a timestamp query that is executed asynchronously by
      /// the command queue \a q.
      ///
      class query {
      public:
         query(command_queue &q);
         query(query &&other);
         ~query();

         query &operator=(const query &) = delete;

         ///
         /// Retrieve the query results.
         ///
         cl_ulong operator()() const;

      private:
         const intrusive_ref<command_queue> q;
         pipe_query *_query;
      };

      ///
      /// Get the current timestamp value.
      ///
      class current {
      public:
         current(command_queue &q);

         ///
         /// Retrieve the query results.
         ///
         cl_ulong operator()() const;

      private:
         cl_ulong result;
      };
   }
}

#endif
