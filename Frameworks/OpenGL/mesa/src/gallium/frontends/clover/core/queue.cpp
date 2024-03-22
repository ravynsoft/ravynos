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

#include "core/queue.hpp"
#include "core/event.hpp"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "util/u_debug.h"

using namespace clover;

namespace {
   void
   debug_notify_callback(void *data,
                         unsigned *id,
                         enum util_debug_type type,
                         const char *fmt,
                         va_list args) {
      const command_queue *queue = (const command_queue *)data;
      char buffer[1024];
      vsnprintf(buffer, sizeof(buffer), fmt, args);
      queue->context().notify(buffer);
   }
}

command_queue::command_queue(clover::context &ctx, clover::device &dev,
                             cl_command_queue_properties props) :
   context(ctx), device(dev), _props(props) {
   pipe = dev.pipe->context_create(dev.pipe, NULL, PIPE_CONTEXT_COMPUTE_ONLY);
   if (!pipe)
      throw error(CL_INVALID_DEVICE);

   if (ctx.notify) {
      struct util_debug_callback cb;
      memset(&cb, 0, sizeof(cb));
      cb.debug_message = &debug_notify_callback;
      cb.data = this;
      if (pipe->set_debug_callback)
         pipe->set_debug_callback(pipe, &cb);
   }
}
command_queue::command_queue(clover::context &ctx, clover::device &dev,
                             std::vector<cl_queue_properties> properties) :
   context(ctx), device(dev), _properties(properties), _props(0) {

   for(std::vector<cl_queue_properties>::size_type i = 0; i != properties.size(); i += 2) {
      if (properties[i] == 0)
         break;
      if (properties[i] == CL_QUEUE_PROPERTIES)
         _props |= properties[i + 1];
      else if (properties[i] != CL_QUEUE_SIZE)
         throw error(CL_INVALID_VALUE);
   }

   pipe = dev.pipe->context_create(dev.pipe, NULL, PIPE_CONTEXT_COMPUTE_ONLY);
   if (!pipe)
      throw error(CL_INVALID_DEVICE);

   if (ctx.notify) {
      struct util_debug_callback cb;
      memset(&cb, 0, sizeof(cb));
      cb.debug_message = &debug_notify_callback;
      cb.data = this;
      if (pipe->set_debug_callback)
         pipe->set_debug_callback(pipe, &cb);
   }
}

command_queue::~command_queue() {
   pipe->destroy(pipe);
}

void
command_queue::flush() {
   std::lock_guard<std::mutex> lock(queued_events_mutex);
   flush_unlocked();
}

void
command_queue::flush_unlocked() {
   pipe_screen *screen = device().pipe;
   pipe_fence_handle *fence = NULL;

   if (!queued_events.empty()) {
      pipe->flush(pipe, &fence, 0);

      while (!queued_events.empty() &&
             queued_events.front()().signalled()) {
         queued_events.front()().fence(fence);
         queued_events.pop_front();
      }

      screen->fence_reference(screen, &fence, NULL);
   }
}

void
command_queue::svm_migrate(const std::vector<void const*> &svm_pointers,
                           const std::vector<size_t> &sizes,
                           cl_mem_migration_flags flags) {
   if (!pipe->svm_migrate)
      return;

   bool to_device = !(flags & CL_MIGRATE_MEM_OBJECT_HOST);
   bool mem_undefined = flags & CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED;
   pipe->svm_migrate(pipe, svm_pointers.size(), svm_pointers.data(),
                     sizes.data(), to_device, mem_undefined);
}

cl_command_queue_properties
command_queue::props() const {
   return _props;
}

std::vector<cl_queue_properties>
command_queue::properties() const {
   return _properties;
}

bool
command_queue::profiling_enabled() const {
   return _props & CL_QUEUE_PROFILING_ENABLE;
}

void
command_queue::sequence(hard_event &ev) {
   std::lock_guard<std::mutex> lock(queued_events_mutex);
   if (!queued_events.empty())
      queued_events.back()().chain(ev);

   queued_events.push_back(ev);

   // Arbitrary threshold.
   // The CTS tends to run a lot of subtests without flushing with the image
   // tests, so flush regularly to prevent stack overflows.
   if (queued_events.size() > 1000)
      flush_unlocked();
}
