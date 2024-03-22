//
// Copyright 2012 Francisco Jerez
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

#ifndef CLOVER_CORE_QUEUE_HPP
#define CLOVER_CORE_QUEUE_HPP

#include <deque>
#include <mutex>

#include "core/object.hpp"
#include "core/context.hpp"
#include "core/timestamp.hpp"
#include "pipe/p_context.h"

namespace clover {
   class resource;
   class mapping;
   class hard_event;

   class command_queue : public ref_counter, public _cl_command_queue {
   public:
      command_queue(clover::context &ctx, clover::device &dev,
                    std::vector<cl_queue_properties> properties);
      command_queue(clover::context &ctx, clover::device &dev,
                    cl_command_queue_properties props);
      ~command_queue();

      command_queue(const command_queue &q) = delete;
      command_queue &
      operator=(const command_queue &q) = delete;

      void flush();
      void svm_migrate(const std::vector<void const *> &svm_pointers,
                       const std::vector<size_t> &sizes, cl_mem_migration_flags flags);

      cl_command_queue_properties props() const;

      std::vector<cl_queue_properties> properties() const;
      bool profiling_enabled() const;

      const intrusive_ref<clover::context> context;
      const intrusive_ref<clover::device> device;

      friend class resource;
      friend class root_resource;
      friend class mapping;
      friend class hard_event;
      friend class sampler;
      friend class kernel;
      friend class clover::timestamp::query;
      friend class clover::timestamp::current;

   private:
      /// Serialize a hardware event with respect to the previous ones,
      /// and push it to the pending list.
      void sequence(hard_event &ev);
      // Use this instead of flush() if `queued_events_mutex` is acquired.
      void flush_unlocked();

      std::vector<cl_queue_properties> _properties;
      cl_command_queue_properties _props;
      pipe_context *pipe;
      std::mutex queued_events_mutex;
      std::deque<intrusive_ref<hard_event>> queued_events;
   };
}

#endif
