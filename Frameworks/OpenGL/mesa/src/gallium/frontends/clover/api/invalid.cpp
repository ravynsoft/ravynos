//
// Copyright 2020 Red Hat
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

// This contains all the CL 2.x API entrypoints that return INVALID_OPERATON
// on CL 3.0. If these are implemented they should be moved out of this file.

CLOVER_API cl_mem
clCreatePipe(cl_context d_ctx,
	     cl_mem_flags flags,
	     cl_uint pipe_packet_size,
	     cl_uint pipe_max_packets,
	     const cl_pipe_properties *properties,
	     cl_int *r_errorcode) {
   *r_errorcode = CL_INVALID_OPERATION;
   return nullptr;
}


CLOVER_API cl_int
clGetPipeInfo(cl_mem pipe,
	      cl_pipe_info param_name,
	      size_t param_value_size,
	      void *param_value,
	      size_t *param_value_size_ret) {
   return CL_INVALID_MEM_OBJECT;
}

CLOVER_API cl_int
clGetDeviceAndHostTimer(cl_device_id device,
			cl_ulong *device_timestamp,
			cl_ulong *host_timestamp) {
   return CL_INVALID_OPERATION;
}

CLOVER_API cl_int
clGetHostTimer(cl_device_id device,
	       cl_ulong *host_timestamp) {
   return CL_INVALID_OPERATION;
}


CLOVER_API cl_int
clGetKernelSubGroupInfo(cl_kernel d_kern,
			cl_device_id device,
			cl_kernel_sub_group_info param_name,
			size_t input_value_size,
			const void *input_value,
			size_t param_size_value,
			void *param_value,
			size_t *param_value_size_ret) {
   return CL_INVALID_OPERATION;
}


CLOVER_API cl_int
clSetDefaultDeviceCommandQueue(cl_context context,
			       cl_device_id device,
			       cl_command_queue command_queue) {
   return CL_INVALID_OPERATION;
}

CLOVER_API cl_int
clSetProgramReleaseCallback(cl_program d_prog,
			    void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
			    void *user_data) {
   return CL_INVALID_OPERATION;
}

CLOVER_API cl_int
clSetProgramSpecializationConstant(cl_program program,
				   cl_uint spec_id,
				   size_t spec_size,
				   const void* spec_value) {
   return CL_INVALID_OPERATION;
}
