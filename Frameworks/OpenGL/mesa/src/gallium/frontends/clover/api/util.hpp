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

#ifndef CLOVER_API_UTIL_HPP
#define CLOVER_API_UTIL_HPP

#include <cassert>
#include <iostream>

#include "core/error.hpp"
#include "core/property.hpp"
#include "util/algorithm.hpp"
#include "util/detect_os.h"

#if DETECT_OS_WINDOWS
#define CLOVER_API
#define CLOVER_ICD_API
#elif HAVE_CLOVER_ICD
#define CLOVER_API
#define CLOVER_ICD_API PUBLIC
#else
#define CLOVER_API PUBLIC
#define CLOVER_ICD_API PUBLIC
#endif

#define CLOVER_NOT_SUPPORTED_UNTIL(version)                    \
   do {                                                        \
      std::cerr << "CL user error: " << __func__               \
                << "() requires OpenCL version " << (version)  \
                << " or greater." << std::endl;                \
   } while (0)

namespace clover {
   ///
   /// Return an error code in \a p if non-zero.
   ///
   inline void
   ret_error(cl_int *p, const clover::error &e) {
      if (p)
         *p = e.get();
   }

   ///
   /// Return a clover object in \a p if non-zero incrementing the
   /// reference count of the object.
   ///
   template<typename T>
   void
   ret_object(typename T::descriptor_type **p,
              const intrusive_ref<T> &v) {
      if (p) {
         v().retain();
         *p = desc(v());
      }
   }

   ///
   /// Return an API object from an intrusive reference to a Clover object,
   /// incrementing the reference count of the object.
   ///
   template<typename T>
   typename T::descriptor_type *
   ret_object(const intrusive_ref<T> &v) {
      v().retain();
      return desc(v());
   }
}

#endif
