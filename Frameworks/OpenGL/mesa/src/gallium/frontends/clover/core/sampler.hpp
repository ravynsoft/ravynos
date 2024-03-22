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

#ifndef CLOVER_CORE_SAMPLER_HPP
#define CLOVER_CORE_SAMPLER_HPP

#include "core/object.hpp"
#include "core/queue.hpp"

namespace clover {
   class sampler : public ref_counter, public _cl_sampler {
   public:
      sampler(clover::context &ctx, bool norm_mode,
              cl_addressing_mode addr_mode,
              cl_filter_mode filter_mode);

      sampler(const sampler &s) = delete;
      sampler &
      operator=(const sampler &s) = delete;

      bool norm_mode();
      cl_addressing_mode addr_mode();
      cl_filter_mode filter_mode();

      const intrusive_ref<clover::context> context;

      friend class kernel;

   private:
      void *bind(command_queue &q);
      void unbind(command_queue &q, void *st);

      bool _norm_mode;
      cl_addressing_mode _addr_mode;
      cl_filter_mode _filter_mode;
   };
}

#endif
