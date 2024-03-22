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

#include "api/util.hpp"
#include "core/event.hpp"

using namespace clover;

CLOVER_API cl_event
clCreateUserEvent(cl_context d_ctx, cl_int *r_errcode) try {
   auto &ctx = obj(d_ctx);

   ret_error(r_errcode, CL_SUCCESS);
   return desc(new soft_event(ctx, {}, false));

} catch (error &e) {
   ret_error(r_errcode, e);
   return NULL;
}

CLOVER_API cl_int
clSetUserEventStatus(cl_event d_ev, cl_int status) try {
   auto &sev = obj<soft_event>(d_ev);

   if (status > 0)
      return CL_INVALID_VALUE;

   if (sev.status() <= 0)
      return CL_INVALID_OPERATION;

   if (status)
      sev.abort(status);
   else
      sev.trigger();

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clWaitForEvents(cl_uint num_evs, const cl_event *d_evs) try {
   auto evs = objs(d_evs, num_evs);

   for (auto &ev : evs) {
      if (ev.context() != evs.front().context())
         throw error(CL_INVALID_CONTEXT);

      if (ev.status() < 0)
         throw error(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
   }

   // Create a temporary soft event that depends on all the events in
   // the wait list
   auto sev = create<soft_event>(evs.front().context(), evs, true);

   // ...and wait on it.
   sev().wait();

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clGetEventInfo(cl_event d_ev, cl_event_info param,
               size_t size, void *r_buf, size_t *r_size) try {
   property_buffer buf { r_buf, size, r_size };
   auto &ev = obj(d_ev);

   switch (param) {
   case CL_EVENT_COMMAND_QUEUE:
      buf.as_scalar<cl_command_queue>() = desc(ev.queue());
      break;

   case CL_EVENT_CONTEXT:
      buf.as_scalar<cl_context>() = desc(ev.context());
      break;

   case CL_EVENT_COMMAND_TYPE:
      buf.as_scalar<cl_command_type>() = ev.command();
      break;

   case CL_EVENT_COMMAND_EXECUTION_STATUS:
      buf.as_scalar<cl_int>() = ev.status();
      break;

   case CL_EVENT_REFERENCE_COUNT:
      buf.as_scalar<cl_uint>() = ev.ref_count();
      break;

   default:
      throw error(CL_INVALID_VALUE);
   }

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clSetEventCallback(cl_event d_ev, cl_int type,
                   void (CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *),
                   void *user_data) try {
   auto &ev = obj(d_ev);

   if (!pfn_notify ||
       (type != CL_COMPLETE && type != CL_SUBMITTED && type != CL_RUNNING))
      throw error(CL_INVALID_VALUE);

   // Create a temporary soft event that depends on ev, with
   // pfn_notify as completion action.
   create<soft_event>(ev.context(), ref_vector<event> { ev }, true,
                      [=, &ev](event &) {
                         ev.wait();
                         pfn_notify(desc(ev), ev.status(), user_data);
                      });

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clRetainEvent(cl_event d_ev) try {
   obj(d_ev).retain();
   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clReleaseEvent(cl_event d_ev) try {
   if (obj(d_ev).release())
      delete pobj(d_ev);

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clEnqueueMarker(cl_command_queue d_q, cl_event *rd_ev) try {
   auto &q = obj(d_q);

   if (!rd_ev)
      throw error(CL_INVALID_VALUE);

   *rd_ev = desc(new hard_event(q, CL_COMMAND_MARKER, {}));

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clEnqueueMarkerWithWaitList(cl_command_queue d_q, cl_uint num_deps,
                            const cl_event *d_deps, cl_event *rd_ev) try {
   auto &q = obj(d_q);
   auto deps = objs<wait_list_tag>(d_deps, num_deps);

   for (auto &ev : deps) {
      if (ev.context() != q.context())
         throw error(CL_INVALID_CONTEXT);
   }

   // Create a hard event that depends on the events in the wait list:
   // previous commands in the same queue are implicitly serialized
   // with respect to it -- hard events always are.
   auto hev = create<hard_event>(q, CL_COMMAND_MARKER, deps);

   ret_object(rd_ev, hev);
   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clEnqueueBarrier(cl_command_queue d_q) try {
   obj(d_q);

   // No need to do anything, q preserves data ordering strictly.

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clEnqueueBarrierWithWaitList(cl_command_queue d_q, cl_uint num_deps,
                             const cl_event *d_deps, cl_event *rd_ev) try {
   auto &q = obj(d_q);
   auto deps = objs<wait_list_tag>(d_deps, num_deps);

   for (auto &ev : deps) {
      if (ev.context() != q.context())
         throw error(CL_INVALID_CONTEXT);
   }

   // Create a hard event that depends on the events in the wait list:
   // subsequent commands in the same queue will be implicitly
   // serialized with respect to it -- hard events always are.
   auto hev = create<hard_event>(q, CL_COMMAND_BARRIER, deps);

   ret_object(rd_ev, hev);
   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clEnqueueWaitForEvents(cl_command_queue d_q, cl_uint num_evs,
                       const cl_event *d_evs) try {
   // The wait list is mandatory for clEnqueueWaitForEvents().
   objs(d_evs, num_evs);

   return clEnqueueBarrierWithWaitList(d_q, num_evs, d_evs, NULL);

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clGetEventProfilingInfo(cl_event d_ev, cl_profiling_info param,
                        size_t size, void *r_buf, size_t *r_size) try {
   property_buffer buf { r_buf, size, r_size };
   hard_event &hev = dynamic_cast<hard_event &>(obj(d_ev));

   if (hev.status() != CL_COMPLETE)
      throw error(CL_PROFILING_INFO_NOT_AVAILABLE);

   switch (param) {
   case CL_PROFILING_COMMAND_QUEUED:
      buf.as_scalar<cl_ulong>() = hev.time_queued();
      break;

   case CL_PROFILING_COMMAND_SUBMIT:
      buf.as_scalar<cl_ulong>() = hev.time_submit();
      break;

   case CL_PROFILING_COMMAND_START:
      buf.as_scalar<cl_ulong>() = hev.time_start();
      break;

   case CL_PROFILING_COMMAND_END:
   case CL_PROFILING_COMMAND_COMPLETE:
      buf.as_scalar<cl_ulong>() = hev.time_end();
      break;

   default:
      throw error(CL_INVALID_VALUE);
   }

   return CL_SUCCESS;

} catch (std::bad_cast &) {
   return CL_PROFILING_INFO_NOT_AVAILABLE;

} catch (lazy<cl_ulong>::undefined_error &) {
   return CL_PROFILING_INFO_NOT_AVAILABLE;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clFinish(cl_command_queue d_q) try {
   auto &q = obj(d_q);

   // Create a temporary hard event -- it implicitly depends on all
   // the previously queued hard events.
   auto hev = create<hard_event>(q, 0, ref_vector<event> {});

   // And wait on it.
   hev().wait();

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}
