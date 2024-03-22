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

#ifndef CLOVER_CORE_ERROR_HPP
#define CLOVER_CORE_ERROR_HPP

#include "CL/cl.h"
#if defined(__ALTIVEC__) && !defined(__APPLE_ALTIVEC__)
   #undef vector
   #undef pixel
   #undef bool
#endif

#include <stdexcept>
#include <string>

namespace clover {
   class command_queue;
   class context;
   class device;
   class event;
   class hard_event;
   class soft_event;
   class kernel;
   class memory_obj;
   class buffer;
   class root_buffer;
   class sub_buffer;
   class image;
   class image2d;
   class image3d;
   class platform;
   class program;
   class sampler;

   ///
   /// Class that represents an error that can be converted to an
   /// OpenCL status code.
   ///
   class error : public std::runtime_error {
   public:
      error(cl_int code, std::string what = "") :
         std::runtime_error(what), code(code) {
      }

      cl_int get() const {
         return code;
      }

   protected:
      cl_int code;
   };

   class invalid_build_options_error : public error {
   public:
      invalid_build_options_error(const std::string &what = "") :
         error(CL_INVALID_BUILD_OPTIONS, what) {}
   };

   class build_error : public error {
   public:
      build_error(const std::string &what = "") :
         error(CL_BUILD_PROGRAM_FAILURE, what) {}
   };

   template<typename O>
   class invalid_object_error;

   template<>
   class invalid_object_error<command_queue> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_COMMAND_QUEUE, what) {}
   };

   template<>
   class invalid_object_error<context> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_CONTEXT, what) {}
   };

   template<>
   class invalid_object_error<device> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_DEVICE, what) {}
   };

   template<>
   class invalid_object_error<event> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_EVENT, what) {}
   };

   template<>
   class invalid_object_error<soft_event> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_EVENT, what) {}
   };

   template<>
   class invalid_object_error<kernel> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_KERNEL, what) {}
   };

   template<>
   class invalid_object_error<memory_obj> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_MEM_OBJECT, what) {}
   };

   template<>
   class invalid_object_error<buffer> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_MEM_OBJECT, what) {}
   };

   template<>
   class invalid_object_error<root_buffer> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_MEM_OBJECT, what) {}
   };

   template<>
   class invalid_object_error<sub_buffer> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_MEM_OBJECT, what) {}
   };

   template<>
   class invalid_object_error<image> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_MEM_OBJECT, what) {}
   };

   template<>
   class invalid_object_error<image2d> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_MEM_OBJECT, what) {}
   };

   template<>
   class invalid_object_error<image3d> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_MEM_OBJECT, what) {}
   };

   template<>
   class invalid_object_error<platform> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_PLATFORM, what) {}
   };

   template<>
   class invalid_object_error<program> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_PROGRAM, what) {}
   };

   template<>
   class invalid_object_error<sampler> : public error {
   public:
      invalid_object_error(std::string what = "") :
         error(CL_INVALID_SAMPLER, what) {}
   };

   class invalid_wait_list_error : public error {
   public:
      invalid_wait_list_error(std::string what = "") :
         error(CL_INVALID_EVENT_WAIT_LIST, what) {}
   };
}

#endif
