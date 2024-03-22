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
#include "core/queue.hpp"

using namespace clover;

CLOVER_API cl_command_queue
clCreateCommandQueue(cl_context d_ctx, cl_device_id d_dev,
                     cl_command_queue_properties props,
                     cl_int *r_errcode) try {
   auto &ctx = obj(d_ctx);
   auto &dev = obj(d_dev);

   if (!count(dev, ctx.devices()))
      throw error(CL_INVALID_DEVICE);

   if (props & ~(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                 CL_QUEUE_PROFILING_ENABLE))
      throw error(CL_INVALID_VALUE);

   ret_error(r_errcode, CL_SUCCESS);
   return new command_queue(ctx, dev, props);

} catch (error &e) {
   ret_error(r_errcode, e);
   return NULL;
}

CLOVER_API cl_int
clRetainCommandQueue(cl_command_queue d_q) try {
   obj(d_q).retain();
   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clReleaseCommandQueue(cl_command_queue d_q) try {
   auto &q = obj(d_q);

   q.flush();

   if (q.release())
      delete pobj(d_q);

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clGetCommandQueueInfo(cl_command_queue d_q, cl_command_queue_info param,
                      size_t size, void *r_buf, size_t *r_size) try {
   property_buffer buf { r_buf, size, r_size };
   auto &q = obj(d_q);

   switch (param) {
   case CL_QUEUE_CONTEXT:
      buf.as_scalar<cl_context>() = desc(q.context());
      break;

   case CL_QUEUE_DEVICE:
      buf.as_scalar<cl_device_id>() = desc(q.device());
      break;

   case CL_QUEUE_REFERENCE_COUNT:
      buf.as_scalar<cl_uint>() = q.ref_count();
      break;

   case CL_QUEUE_PROPERTIES:
      buf.as_scalar<cl_command_queue_properties>() = q.props();
      break;

   case CL_QUEUE_PROPERTIES_ARRAY:
      buf.as_vector<cl_queue_properties>() = q.properties();
      break;

   case CL_QUEUE_DEVICE_DEFAULT:
      if (r_size)
	 *r_size = 0;
      break;

   case CL_QUEUE_SIZE:
      throw error(CL_INVALID_COMMAND_QUEUE);
      break;

   default:
      throw error(CL_INVALID_VALUE);
   }

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clFlush(cl_command_queue d_q) try {
   obj(d_q).flush();
   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_command_queue
clCreateCommandQueueWithProperties(cl_context d_ctx, cl_device_id d_dev,
                                   const cl_queue_properties *d_properties,
                                   cl_int *r_errcode) try {
   auto &ctx = obj(d_ctx);
   auto &dev = obj(d_dev);

   if (!count(dev, ctx.devices()))
      throw error(CL_INVALID_DEVICE);

   ret_error(r_errcode, CL_SUCCESS);
   std::vector<cl_queue_properties> properties;

   if (d_properties) {
      int idx = -1;
      /* these come in pairs, bail if the first is 0 */
      do {
         idx++;
         properties.push_back(d_properties[idx]);
      } while (d_properties[idx & ~1]);
   }
   return new command_queue(ctx, dev, properties);

} catch (error &e) {
   ret_error(r_errcode, e);
   return NULL;
}
