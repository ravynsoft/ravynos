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

#ifndef API_DISPATCH_HPP
#define API_DISPATCH_HPP

#include "CL/cl.h"
#include "CL/cl_ext.h"
#include "CL/cl_egl.h"
#include "CL/cl_gl.h"
#include "CL/cl_icd.h"

namespace clover {
   extern const cl_icd_dispatch _dispatch;

   cl_int CL_API_CALL
   GetPlatformInfo(cl_platform_id d_platform, cl_platform_info param,
                   size_t size, void *r_buf, size_t *r_size);

   void * CL_API_CALL
   GetExtensionFunctionAddress(const char *p_name);

   void * CL_API_CALL
   GetExtensionFunctionAddressForPlatform(cl_platform_id d_platform,
                                          const char *p_name);

   cl_int CL_API_CALL
   IcdGetPlatformIDsKHR(cl_uint num_entries, cl_platform_id *rd_platforms,
                        cl_uint *rnum_platforms);

   cl_int CL_API_CALL
   EnqueueSVMFree(cl_command_queue command_queue,
                  cl_uint num_svm_pointers,
                  void *svm_pointers[],
                  void (CL_CALLBACK *pfn_free_func) (
                     cl_command_queue queue, cl_uint num_svm_pointers,
                     void *svm_pointers[], void *user_data),
                  void *user_data,
                  cl_uint num_events_in_wait_list,
                  const cl_event *event_wait_list,
                  cl_event *event,
                  cl_int cmd);

   cl_int CL_API_CALL
   EnqueueSVMMemcpy(cl_command_queue command_queue,
                    cl_bool blocking_copy,
                    void *dst_ptr,
                    const void *src_ptr,
                    size_t size,
                    cl_uint num_events_in_wait_list,
                    const cl_event *event_wait_list,
                    cl_event *event,
                    cl_int cmd);

   cl_int CL_API_CALL
   EnqueueSVMMap(cl_command_queue command_queue,
                 cl_bool blocking_map,
                 cl_map_flags map_flags,
                 void *svm_ptr,
                 size_t size,
                 cl_uint num_events_in_wait_list,
                 const cl_event *event_wait_list,
                 cl_event *event,
                 cl_int cmd);

   cl_int CL_API_CALL
   EnqueueSVMMemFill(cl_command_queue command_queue,
                     void *svm_ptr,
                     const void *pattern,
                     size_t pattern_size,
                     size_t size,
                     cl_uint num_events_in_wait_list,
                     const cl_event *event_wait_list,
                     cl_event *event,
                     cl_int cmd);

   cl_int CL_API_CALL
   EnqueueSVMUnmap(cl_command_queue command_queue,
                   void *svm_ptr,
                   cl_uint num_events_in_wait_list,
                   const cl_event *event_wait_list,
                   cl_event *event,
                   cl_int cmd);

   cl_program CL_API_CALL
   CreateProgramWithILKHR(cl_context d_ctx, const void *il,
                          size_t length, cl_int *r_errcode);
}

#endif
