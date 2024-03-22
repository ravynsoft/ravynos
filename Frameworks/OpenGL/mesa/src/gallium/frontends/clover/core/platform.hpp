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

#ifndef CLOVER_CORE_PLATFORM_HPP
#define CLOVER_CORE_PLATFORM_HPP

#include <vector>

#include "core/object.hpp"
#include "core/device.hpp"
#include "util/range.hpp"

namespace clover {
   class platform : public _cl_platform_id,
                    public adaptor_range<
      evals, std::vector<intrusive_ref<device>> &> {
   public:
      platform();

      platform(const platform &platform) = delete;
      platform &
      operator=(const platform &platform) = delete;

      std::string supported_extensions_as_string() const;
      std::vector<cl_name_version> supported_extensions() const;

      std::string platform_version_as_string() const;
      cl_version platform_version() const;

   protected:
      cl_version version;
      std::vector<intrusive_ref<device>> devs;
   };

   platform &find_platform(cl_platform_id d_platform);
}

#endif
