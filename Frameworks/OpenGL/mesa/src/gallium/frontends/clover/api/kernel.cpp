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
#include "core/kernel.hpp"
#include "core/event.hpp"

using namespace clover;

CLOVER_API cl_kernel
clCreateKernel(cl_program d_prog, const char *name, cl_int *r_errcode) try {
   auto &prog = obj(d_prog);

   if (!name)
      throw error(CL_INVALID_VALUE);

   auto &sym = find(name_equals(name), prog.symbols());

   ret_error(r_errcode, CL_SUCCESS);
   return new kernel(prog, name, range(sym.args));

} catch (std::out_of_range &) {
   ret_error(r_errcode, CL_INVALID_KERNEL_NAME);
   return NULL;

} catch (error &e) {
   ret_error(r_errcode, e);
   return NULL;
}

CLOVER_API cl_int
clCreateKernelsInProgram(cl_program d_prog, cl_uint count,
                         cl_kernel *rd_kerns, cl_uint *r_count) try {
   auto &prog = obj(d_prog);
   auto &syms = prog.symbols();

   if (rd_kerns && count < syms.size())
      throw error(CL_INVALID_VALUE);

   if (rd_kerns)
      copy(map([&](const binary::symbol &sym) {
               return desc(new kernel(prog,
                                      std::string(sym.name.begin(),
                                                  sym.name.end()),
                                      range(sym.args)));
            }, syms),
         rd_kerns);

   if (r_count)
      *r_count = syms.size();

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clRetainKernel(cl_kernel d_kern) try {
   obj(d_kern).retain();
   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clReleaseKernel(cl_kernel d_kern) try {
   if (obj(d_kern).release())
      delete pobj(d_kern);

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clSetKernelArg(cl_kernel d_kern, cl_uint idx, size_t size,
               const void *value) try {
   obj(d_kern).args().at(idx).set(size, value);
   return CL_SUCCESS;

} catch (std::out_of_range &) {
   return CL_INVALID_ARG_INDEX;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clGetKernelInfo(cl_kernel d_kern, cl_kernel_info param,
                size_t size, void *r_buf, size_t *r_size) try {
   property_buffer buf { r_buf, size, r_size };
   auto &kern = obj(d_kern);

   switch (param) {
   case CL_KERNEL_FUNCTION_NAME:
      buf.as_string() = kern.name();
      break;

   case CL_KERNEL_NUM_ARGS:
      buf.as_scalar<cl_uint>() = kern.args().size();
      break;

   case CL_KERNEL_REFERENCE_COUNT:
      buf.as_scalar<cl_uint>() = kern.ref_count();
      break;

   case CL_KERNEL_CONTEXT:
      buf.as_scalar<cl_context>() = desc(kern.program().context());
      break;

   case CL_KERNEL_PROGRAM:
      buf.as_scalar<cl_program>() = desc(kern.program());
      break;

   case CL_KERNEL_ATTRIBUTES:
      buf.as_string() = find(name_equals(kern.name()), kern.program().symbols()).attributes;
      break;

   default:
      throw error(CL_INVALID_VALUE);
   }

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clGetKernelWorkGroupInfo(cl_kernel d_kern, cl_device_id d_dev,
                         cl_kernel_work_group_info param,
                         size_t size, void *r_buf, size_t *r_size) try {
   property_buffer buf { r_buf, size, r_size };
   auto &kern = obj(d_kern);
   auto &dev = (d_dev ? *pobj(d_dev) : unique(kern.program().devices()));

   if (!count(dev, kern.program().devices()))
      throw error(CL_INVALID_DEVICE);

   switch (param) {
   case CL_KERNEL_WORK_GROUP_SIZE:
      buf.as_scalar<size_t>() = dev.max_threads_per_block();
      break;

   case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
      buf.as_vector<size_t>() = kern.required_block_size();
      break;

   case CL_KERNEL_LOCAL_MEM_SIZE:
      buf.as_scalar<cl_ulong>() = kern.mem_local();
      break;

   case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
      buf.as_scalar<size_t>() = dev.subgroup_size();
      break;

   case CL_KERNEL_PRIVATE_MEM_SIZE:
      buf.as_scalar<cl_ulong>() = kern.mem_private();
      break;

   default:
      throw error(CL_INVALID_VALUE);
   }

   return CL_SUCCESS;

} catch (error &e) {
   return e.get();

} catch (std::out_of_range &) {
   return CL_INVALID_DEVICE;
}

CLOVER_API cl_int
clGetKernelArgInfo(cl_kernel d_kern,
                   cl_uint idx, cl_kernel_arg_info param,
                   size_t size, void *r_buf, size_t *r_size) try {
   property_buffer buf { r_buf, size, r_size };

   auto info = obj(d_kern).args_infos().at(idx);

   if (info.arg_name.empty())
      return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;

   switch (param) {
   case CL_KERNEL_ARG_ADDRESS_QUALIFIER:
      buf.as_scalar<cl_kernel_arg_address_qualifier>() = info.address_qualifier;
      break;

   case CL_KERNEL_ARG_ACCESS_QUALIFIER:
      buf.as_scalar<cl_kernel_arg_access_qualifier>() = info.access_qualifier;
      break;

   case CL_KERNEL_ARG_TYPE_NAME:
      buf.as_string() = info.type_name;
      break;

   case CL_KERNEL_ARG_TYPE_QUALIFIER:
      buf.as_scalar<cl_kernel_arg_type_qualifier>() = info.type_qualifier;
      break;

   case CL_KERNEL_ARG_NAME:
      buf.as_string() = info.arg_name;
      break;

   default:
      throw error(CL_INVALID_VALUE);
   }

   return CL_SUCCESS;

} catch (std::out_of_range &) {
   return CL_INVALID_ARG_INDEX;

} catch (error &e) {
   return e.get();
}

namespace {
   ///
   /// Common argument checking shared by kernel invocation commands.
   ///
   void
   validate_common(const command_queue &q, kernel &kern,
                   const ref_vector<event> &deps) {
      if (kern.program().context() != q.context() ||
          any_of([&](const event &ev) {
                return ev.context() != q.context();
             }, deps))
         throw error(CL_INVALID_CONTEXT);

      if (any_of([](kernel::argument &arg) {
               return !arg.set();
            }, kern.args()))
         throw error(CL_INVALID_KERNEL_ARGS);

      // If the command queue's device is not associated to the program, we get
      // a binary, with no sections, which will also fail the following test.
      auto &b = kern.program().build(q.device()).bin;
      if (!any_of(type_equals(binary::section::text_executable), b.secs))
         throw error(CL_INVALID_PROGRAM_EXECUTABLE);
   }

   std::vector<size_t>
   validate_grid_size(const command_queue &q, cl_uint dims,
                      const size_t *d_grid_size) {
      auto grid_size = range(d_grid_size, dims);

      if (dims < 1 || dims > q.device().max_block_size().size())
         throw error(CL_INVALID_WORK_DIMENSION);

      return grid_size;
   }

   std::vector<size_t>
   validate_grid_offset(const command_queue &q, cl_uint dims,
                        const size_t *d_grid_offset) {
      if (d_grid_offset)
         return range(d_grid_offset, dims);
      else
         return std::vector<size_t>(dims, 0);
   }

   std::vector<size_t>
   validate_block_size(const command_queue &q, const kernel &kern,
                       cl_uint dims, const size_t *d_grid_size,
                       const size_t *d_block_size) {
      auto grid_size = range(d_grid_size, dims);

      if (d_block_size) {
         auto block_size = range(d_block_size, dims);

         if (any_of(is_zero(), block_size) ||
             any_of(greater(), block_size, q.device().max_block_size()))
            throw error(CL_INVALID_WORK_ITEM_SIZE);

         if (any_of(modulus(), grid_size, block_size))
            throw error(CL_INVALID_WORK_GROUP_SIZE);

         if (fold(multiplies(), 1u, block_size) >
             q.device().max_threads_per_block())
            throw error(CL_INVALID_WORK_GROUP_SIZE);

         return block_size;

      } else {
         return kern.optimal_block_size(q, grid_size);
      }
   }
}

CLOVER_API cl_int
clEnqueueNDRangeKernel(cl_command_queue d_q, cl_kernel d_kern,
                       cl_uint dims, const size_t *d_grid_offset,
                       const size_t *d_grid_size, const size_t *d_block_size,
                       cl_uint num_deps, const cl_event *d_deps,
                       cl_event *rd_ev) try {
   auto &q = obj(d_q);
   auto &kern = obj(d_kern);
   auto deps = objs<wait_list_tag>(d_deps, num_deps);
   auto grid_size = validate_grid_size(q, dims, d_grid_size);
   auto grid_offset = validate_grid_offset(q, dims, d_grid_offset);
   auto block_size = validate_block_size(q, kern, dims,
                                         d_grid_size, d_block_size);

   validate_common(q, kern, deps);

   auto hev = create<hard_event>(
      q, CL_COMMAND_NDRANGE_KERNEL, deps,
      [=, &kern, &q](event &) {
         kern.launch(q, grid_offset, grid_size, block_size);
      });

   ret_object(rd_ev, hev);
   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clEnqueueTask(cl_command_queue d_q, cl_kernel d_kern,
              cl_uint num_deps, const cl_event *d_deps,
              cl_event *rd_ev) try {
   auto &q = obj(d_q);
   auto &kern = obj(d_kern);
   auto deps = objs<wait_list_tag>(d_deps, num_deps);

   validate_common(q, kern, deps);

   auto hev = create<hard_event>(
      q, CL_COMMAND_TASK, deps,
      [=, &kern, &q](event &) {
         kern.launch(q, { 0 }, { 1 }, { 1 });
      });

   ret_object(rd_ev, hev);
   return CL_SUCCESS;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clEnqueueNativeKernel(cl_command_queue d_q,
                      void (CL_CALLBACK * func)(void *),
                      void *args, size_t args_size,
                      cl_uint num_mems, const cl_mem *d_mems,
                      const void **mem_handles, cl_uint num_deps,
                      const cl_event *d_deps, cl_event *rd_ev) {
   return CL_INVALID_OPERATION;
}

CLOVER_API cl_int
clSetKernelArgSVMPointer(cl_kernel d_kern,
                         cl_uint arg_index,
                         const void *arg_value) try {
  if (!any_of(std::mem_fn(&device::svm_support), obj(d_kern).program().devices()))
      return CL_INVALID_OPERATION;
   obj(d_kern).args().at(arg_index).set_svm(arg_value);
   return CL_SUCCESS;

} catch (std::out_of_range &) {
   return CL_INVALID_ARG_INDEX;

} catch (error &e) {
   return e.get();
}

CLOVER_API cl_int
clSetKernelExecInfo(cl_kernel d_kern,
                    cl_kernel_exec_info param_name,
                    size_t param_value_size,
                    const void *param_value) try {

   if (!any_of(std::mem_fn(&device::svm_support), obj(d_kern).program().devices()))
      return CL_INVALID_OPERATION;

   auto &kern = obj(d_kern);

   const bool has_system_svm = all_of(std::mem_fn(&device::has_system_svm),
                                      kern.program().context().devices());

   if (!param_value)
      return CL_INVALID_VALUE;

   switch (param_name) {
   case CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM:
   case CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM_ARM: {
      if (param_value_size != sizeof(cl_bool))
         return CL_INVALID_VALUE;

      cl_bool val = *static_cast<const cl_bool*>(param_value);
      if (val == CL_TRUE && !has_system_svm)
         return CL_INVALID_OPERATION;
      else
         return CL_SUCCESS;
   }

   case CL_KERNEL_EXEC_INFO_SVM_PTRS:
   case CL_KERNEL_EXEC_INFO_SVM_PTRS_ARM:
      if (has_system_svm)
         return CL_SUCCESS;

      CLOVER_NOT_SUPPORTED_UNTIL("2.0");
      return CL_INVALID_VALUE;

   default:
      return CL_INVALID_VALUE;
   }

} catch (error &e) {
   return e.get();
}
