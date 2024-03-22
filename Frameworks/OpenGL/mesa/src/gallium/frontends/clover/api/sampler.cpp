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
#include "core/sampler.hpp"

using namespace clover;

CLOVER_API cl_sampler
clCreateSampler(cl_context d_ctx, cl_bool norm_mode,
                cl_addressing_mode addr_mode, cl_filter_mode filter_mode,
                cl_int *r_errcode) try {
   auto &ctx = obj(d_ctx);

   if (!any_of(std::mem_fn(&device::image_support), ctx.devices()))
      throw error(CL_INVALID_OPERATION);

   ret_error(r_errcode, CL_SUCCESS);
   return new sampler(ctx, norm_mode, addr_mode, filter_mode);

} catch (error &e) {
   ret_error(r_errcode, e);
   return NULL;
}

CLOVER_API cl_int
clRetainSampler(cl_sampler d_s) try {
   obj(d_s).retain();
   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clReleaseSampler(cl_sampler d_s) try {
   if (obj(d_s).release())
      delete pobj(d_s);

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clGetSamplerInfo(cl_sampler d_s, cl_sampler_info param,
                 size_t size, void *r_buf, size_t *r_size) try {
   property_buffer buf { r_buf, size, r_size };
   auto &s = obj(d_s);

   switch (param) {
   case CL_SAMPLER_REFERENCE_COUNT:
      buf.as_scalar<cl_uint>() = s.ref_count();
      break;

   case CL_SAMPLER_CONTEXT:
      buf.as_scalar<cl_context>() = desc(s.context());
      break;

   case CL_SAMPLER_NORMALIZED_COORDS:
      buf.as_scalar<cl_bool>() = s.norm_mode();
      break;

   case CL_SAMPLER_ADDRESSING_MODE:
      buf.as_scalar<cl_addressing_mode>() = s.addr_mode();
      break;

   case CL_SAMPLER_FILTER_MODE:
      buf.as_scalar<cl_filter_mode>() = s.filter_mode();
      break;

   default:
      throw error(CL_INVALID_VALUE);
   }

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}
