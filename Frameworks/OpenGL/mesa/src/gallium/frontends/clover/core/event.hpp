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

#ifndef CLOVER_CORE_EVENT_HPP
#define CLOVER_CORE_EVENT_HPP

#include <condition_variable>
#include <functional>

#include "core/object.hpp"
#include "core/queue.hpp"
#include "core/timestamp.hpp"
#include "util/lazy.hpp"

namespace clover {
   ///
   /// Class that represents a task that might be executed
   /// asynchronously at some point in the future.
   ///
   /// An event consists of a list of dependencies, a boolean
   /// signalled() flag, and an associated task.  An event is
   /// considered signalled as soon as all its dependencies (if any)
   /// are signalled as well, and the trigger() method is called; at
   /// that point the associated task will be started through the
   /// specified \a action_ok.  If the abort() method is called
   /// instead, the specified \a action_fail is executed and the
   /// associated task will never be started.  Dependent events will
   /// be aborted recursively.
   ///
   /// The execution status of the associated task can be queried
   /// using the status() method, and it can be waited for completion
   /// using the wait() method.
   ///
   class event : public ref_counter, public _cl_event {
   public:
      typedef std::function<void (event &)> action;

      event(clover::context &ctx, const ref_vector<event> &deps,
            action action_ok, action action_fail);
      virtual ~event();

      event(const event &ev) = delete;
      event &
      operator=(const event &ev) = delete;

      void trigger();
      void abort(cl_int status);
      bool signalled() const;

      virtual cl_int status() const;
      virtual command_queue *queue() const = 0;
      virtual cl_command_type command() const = 0;
      void wait_signalled() const;
      virtual void wait() const;

      virtual struct pipe_fence_handle *fence() const {
         return NULL;
      }

      const intrusive_ref<clover::context> context;

   protected:
      void chain(event &ev);

      mutable std::vector<intrusive_ref<event>> deps;

   private:
      std::vector<intrusive_ref<event>> trigger_self();
      std::vector<intrusive_ref<event>> abort_self(cl_int status);
      unsigned wait_count() const;

      unsigned _wait_count;
      cl_int _status;
      action action_ok;
      action action_fail;
      std::vector<intrusive_ref<event>> _chain;
      mutable std::condition_variable cv;
      mutable std::mutex mutex;
   };

   ///
   /// Class that represents a task executed by a command queue.
   ///
   /// Similar to a normal clover::event.  In addition it's associated
   /// with a given command queue \a q and a given OpenCL \a command.
   /// hard_event instances created for the same queue are implicitly
   /// ordered with respect to each other, and they are implicitly
   /// triggered on construction.
   ///
   /// A hard_event is considered complete when the associated
   /// hardware task finishes execution.
   ///
   class hard_event : public event {
   public:
      hard_event(command_queue &q, cl_command_type command,
                 const ref_vector<event> &deps,
                 action action = [](event &){});
      ~hard_event();

      virtual cl_int status() const;
      virtual command_queue *queue() const;
      virtual cl_command_type command() const;
      virtual void wait() const;

      const lazy<cl_ulong> &time_queued() const;
      const lazy<cl_ulong> &time_submit() const;
      const lazy<cl_ulong> &time_start() const;
      const lazy<cl_ulong> &time_end() const;

      friend class command_queue;

      virtual struct pipe_fence_handle *fence() const {
         return _fence;
      }

   private:
      virtual void fence(pipe_fence_handle *fence);
      action profile(command_queue &q, const action &action) const;

      const intrusive_ref<command_queue> _queue;
      cl_command_type _command;
      pipe_fence_handle *_fence;
      lazy<cl_ulong> _time_queued, _time_submit, _time_start, _time_end;
   };

   ///
   /// Class that represents a software event.
   ///
   /// A soft_event is not associated with any specific hardware task
   /// or command queue.  It's considered complete as soon as all its
   /// dependencies finish execution.
   ///
   class soft_event : public event {
   public:
      soft_event(clover::context &ctx, const ref_vector<event> &deps,
                 bool trigger, action action = [](event &){});

      virtual cl_int status() const;
      virtual command_queue *queue() const;
      virtual cl_command_type command() const;
      virtual void wait() const;
   };
}

#endif
