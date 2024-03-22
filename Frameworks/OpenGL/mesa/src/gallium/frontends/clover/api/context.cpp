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
#include "core/context.hpp"
#include "core/platform.hpp"

using namespace clover;

CLOVER_API cl_context
clCreateContext(const cl_context_properties *d_props, cl_uint num_devs,
                const cl_device_id *d_devs,
                void (CL_CALLBACK *pfn_notify)(const char *, const void *,
                                               size_t, void *),
                void *user_data, cl_int *r_errcode) try {
   auto props = obj<property_list_tag>(d_props);
   auto devs = objs(d_devs, num_devs);

   if (!pfn_notify && user_data)
      throw error(CL_INVALID_VALUE);

   for (auto &prop : props) {
      if (prop.first == CL_CONTEXT_PLATFORM)
         find_platform(prop.second.as<cl_platform_id>());
      else
         throw error(CL_INVALID_PROPERTY);
   }

   const auto notify = (!pfn_notify ? context::notify_action() :
                        [=](const char *s) {
                           pfn_notify(s, NULL, 0, user_data);
                        });

   ret_error(r_errcode, CL_SUCCESS);
   return desc(new context(props, devs, notify));

} catch (error &e) {
   ret_error(r_errcode, e);
   return NULL;
}

CLOVER_API cl_context
clCreateContextFromType(const cl_context_properties *d_props,
                        cl_device_type type,
                        void (CL_CALLBACK *pfn_notify)(
                           const char *, const void *, size_t, void *),
                        void *user_data, cl_int *r_errcode) try {
   cl_platform_id d_platform;
   cl_uint num_platforms;
   cl_int ret;
   std::vector<cl_device_id> devs;
   cl_uint num_devices;

   ret = clGetPlatformIDs(1, &d_platform, &num_platforms);
   if (ret || !num_platforms)
      throw error(CL_INVALID_PLATFORM);

   ret = clGetDeviceIDs(d_platform, type, 0, NULL, &num_devices);
   if (ret)
      throw error(CL_DEVICE_NOT_FOUND);
   devs.resize(num_devices);
   ret = clGetDeviceIDs(d_platform, type, num_devices, devs.data(), 0);
   if (ret)
      throw error(CL_DEVICE_NOT_FOUND);

   return clCreateContext(d_props, num_devices, devs.data(), pfn_notify,
                          user_data, r_errcode);

} catch (error &e) {
   ret_error(r_errcode, e);
   return NULL;
}

CLOVER_API cl_int
clRetainContext(cl_context d_ctx) try {
   obj(d_ctx).retain();
   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clReleaseContext(cl_context d_ctx) try {
   if (obj(d_ctx).release())
      delete pobj(d_ctx);

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clGetContextInfo(cl_context d_ctx, cl_context_info param,
                 size_t size, void *r_buf, size_t *r_size) try {
   property_buffer buf { r_buf, size, r_size };
   auto &ctx = obj(d_ctx);

   switch (param) {
   case CL_CONTEXT_REFERENCE_COUNT:
      buf.as_scalar<cl_uint>() = ctx.ref_count();
      break;

   case CL_CONTEXT_NUM_DEVICES:
      buf.as_scalar<cl_uint>() = ctx.devices().size();
      break;

   case CL_CONTEXT_DEVICES:
      buf.as_vector<cl_device_id>() = descs(ctx.devices());
      break;

   case CL_CONTEXT_PROPERTIES:
      buf.as_vector<cl_context_properties>() = desc(ctx.properties());
      break;

   default:
      throw error(CL_INVALID_VALUE);
   }

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clSetContextDestructorCallback(cl_context d_ctx,
                               void (CL_CALLBACK *pfn_notify)(cl_context, void *),
                               void *user_data) try {
   CLOVER_NOT_SUPPORTED_UNTIL("3.0");
   auto &ctx = obj(d_ctx);

   if (!pfn_notify)
      return CL_INVALID_VALUE;

   ctx.destroy_notify([=]{ pfn_notify(d_ctx, user_data); });

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}
