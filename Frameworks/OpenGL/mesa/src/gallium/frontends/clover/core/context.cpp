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

#include "core/context.hpp"

using namespace clover;

context::context(const property_list &props,
                 const ref_vector<device> &devs,
                 const notify_action &notify) :
   notify(notify), props(props), devs(devs) {
}

context::~context() {
   while (_destroy_notify.size()) {
      _destroy_notify.top()();
      _destroy_notify.pop();
   }
}

bool
context::operator==(const context &ctx) const {
   return this == &ctx;
}

bool
context::operator!=(const context &ctx) const {
   return this != &ctx;
}

void
context::destroy_notify(std::function<void ()> f) {
   _destroy_notify.push(f);
}

const context::property_list &
context::properties() const {
   return props;
}

context::device_range
context::devices() const {
   return map(evals(), devs);
}

void
context::add_svm_allocation(const void *ptr, size_t size) {
   svm_ptrs.emplace(ptr, size);
}

void
context::remove_svm_allocation(const void *ptr) {
   svm_ptrs.erase(ptr);
}

context::svm_pointer_map::value_type
context::find_svm_allocation(const void *ptr) const {
   // std::prev on an iterator of an empty container causes SIGSEGVs
   if (svm_ptrs.empty())
      return { nullptr, 0 };

   auto it = std::prev(svm_ptrs.upper_bound(ptr));
   if (it == svm_ptrs.end())
      return { nullptr, 0 };

   uintptr_t base = reinterpret_cast<uintptr_t>((*it).first);
   uintptr_t end  = (*it).second + base;
   uintptr_t ptrv = reinterpret_cast<uintptr_t>(ptr);
   if (ptrv >= base && ptrv < end)
      return *it;

   return { nullptr, 0 };
}
