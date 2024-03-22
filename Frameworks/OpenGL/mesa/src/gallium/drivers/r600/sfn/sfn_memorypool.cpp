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

#include "sfn_memorypool.h"

#include <cassert>
#include <iostream>

#ifdef HAVE_MEMORY_RESOURCE
#include <memory_resource>
#else
#include <list>
#include <stdlib.h>
#endif

namespace r600 {

#ifndef HAVE_MEMORY_RESOURCE
/* Fallback memory resource if the C++17 memory resource is not
 * available
 */
struct MemoryBacking {
   ~MemoryBacking();
   void *allocate(size_t size);
   void *allocate(size_t size, size_t align);
   std::list<void *> m_data;
};
#endif

struct MemoryPoolImpl {
public:
   MemoryPoolImpl();
   ~MemoryPoolImpl();
#ifdef HAVE_MEMORY_RESOURCE
   using MemoryBacking = ::std::pmr::monotonic_buffer_resource;
#endif
   MemoryBacking *pool;
};

MemoryPool::MemoryPool() noexcept:
    impl(nullptr)
{
}

MemoryPool&
MemoryPool::instance()
{
   static thread_local MemoryPool me;
   return me;
}

void
MemoryPool::free()
{
   delete impl;
   impl = nullptr;
}

void
MemoryPool::initialize()
{
   if (!impl)
      impl = new MemoryPoolImpl();
}

void *
MemoryPool::allocate(size_t size)
{
   assert(impl);
   return impl->pool->allocate(size);
}

void *
MemoryPool::allocate(size_t size, size_t align)
{
   assert(impl);
   return impl->pool->allocate(size, align);
}

void
MemoryPool::release_all()
{
   instance().free();
}

void
init_pool()
{
   MemoryPool::instance().initialize();
}

void
release_pool()
{
   MemoryPool::release_all();
}

void *
Allocate::operator new(size_t size)
{
   return MemoryPool::instance().allocate(size);
}

void
Allocate::operator delete(void *p, size_t size)
{
   // MemoryPool::instance().deallocate(p, size);
}

MemoryPoolImpl::MemoryPoolImpl() { pool = new MemoryBacking(); }

MemoryPoolImpl::~MemoryPoolImpl() { delete pool; }

#ifndef HAVE_MEMORY_RESOURCE
MemoryBacking::~MemoryBacking()
{
   for (auto p : m_data)
      free(p);
}

void *
MemoryBacking::allocate(size_t size)
{
   void *retval = malloc(size);
   m_data.push_back(retval);
   return retval;
}

void *
MemoryBacking::allocate(size_t size, size_t align)
{
   void *retval = aligned_alloc(align, size);
   m_data.push_back(retval);
   return retval;
}

#endif

} // namespace r600
