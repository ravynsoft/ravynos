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

#ifndef CLOVER_CORE_CONTEXT_HPP
#define CLOVER_CORE_CONTEXT_HPP

#include <map>
#include <stack>

#include "core/object.hpp"
#include "core/device.hpp"
#include "core/property.hpp"

namespace clover {
   class context : public ref_counter, public _cl_context {
   private:
      typedef adaptor_range<
            evals, const std::vector<intrusive_ref<device>> &
         > device_range;
      typedef clover::property_list<cl_context_properties> property_list;

   public:
      ~context();

      typedef std::function<void (const char *)> notify_action;
      typedef std::map<const void *, size_t> svm_pointer_map;

      context(const property_list &props, const ref_vector<device> &devs,
              const notify_action &notify);

      context(const context &ctx) = delete;
      context &
      operator=(const context &ctx) = delete;

      bool
      operator==(const context &ctx) const;
      bool
      operator!=(const context &ctx) const;

      void destroy_notify(std::function<void ()> f);

      const property_list &
      properties() const;

      device_range
      devices() const;

      void
      add_svm_allocation(const void *ptr, size_t size);

      void
      remove_svm_allocation(const void *ptr);

      svm_pointer_map::value_type
      find_svm_allocation(const void *ptr) const;

      const notify_action notify;

   private:
      property_list props;
      const std::vector<intrusive_ref<device>> devs;
      std::stack<std::function<void ()>> _destroy_notify;
      svm_pointer_map svm_ptrs;
   };
}

#endif
