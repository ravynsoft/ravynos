//
// Copyright 2015 Advanced Micro Devices, Inc.
// All Rights Reserved.
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

#include "core/event.hpp"
#include "api/util.hpp"

using namespace clover;

extern "C" {

PUBLIC bool
opencl_dri_event_add_ref(cl_event event)
{
   /* This should fail if the event hasn't been created by
    * clEnqueueReleaseGLObjects or clEnqueueReleaseEGLObjects.
    *
    * TODO: implement the CL functions
    */
   return false; /*return clRetainEvent(event) == CL_SUCCESS;*/
}

PUBLIC bool
opencl_dri_event_release(cl_event event)
{
   return clReleaseEvent(event) == CL_SUCCESS;
}

PUBLIC bool
opencl_dri_event_wait(cl_event event, uint64_t timeout) try {
   if (!timeout) {
      return obj(event).status() == CL_COMPLETE;
   }

   obj(event).wait();
   return true;

} catch (error &) {
   return false;
}

PUBLIC struct pipe_fence_handle *
opencl_dri_event_get_fence(cl_event event) try {
   return obj(event).fence();

} catch (error &) {
   return NULL;
}

}
