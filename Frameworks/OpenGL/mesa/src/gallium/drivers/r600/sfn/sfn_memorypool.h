/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2022 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <cstdlib>
#include <memory>
#include <stack>

#define R600_POINTER_TYPE(X) X *

namespace r600 {

void
init_pool();
void
release_pool();

class Allocate {
public:
   void *operator new(size_t size);
   void operator delete(void *p, size_t size);
};

class MemoryPool {
public:
   static MemoryPool& instance();
   static void release_all();

   void free();
   void initialize();

   void *allocate(size_t size);
   void *allocate(size_t size, size_t align);

private:
   MemoryPool() noexcept;

   struct MemoryPoolImpl *impl;
};

template <typename T> struct Allocator {
   using value_type = T;

   Allocator() = default;
   Allocator(const Allocator& other) = default;

   template <typename U> Allocator(const Allocator<U>& other) { (void)other; }

   T *allocate(size_t n)
   {
      return (T *)MemoryPool::instance().allocate(n * sizeof(T), alignof(T));
   }

   void deallocate(void *p, size_t n)
   {
      (void)p;
      (void)n;
      // MemoryPool::instance().deallocate(p, n * sizeof(T), alignof(T));
   }

   friend bool operator==(const Allocator<T>& lhs, const Allocator<T>& rhs)
   {
      (void)lhs;
      (void)rhs;
      return true;
   }
};

} // namespace r600

#endif // MEMORYPOOL_H
