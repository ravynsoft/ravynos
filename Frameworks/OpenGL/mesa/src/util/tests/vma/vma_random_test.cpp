/*
 * Copyright © 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* it is a test after all */
#undef NDEBUG

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <set>
#include <vector>

#ifndef _WIN32
#include <err.h>
#else
#define errx(code, msg, ...)             \
   do {                                  \
      fprintf(stderr, msg, __VA_ARGS__); \
      exit(code);                        \
   } while (0);
#endif

#include "util/vma.h"

namespace {

static const uint64_t MEM_PAGE_SIZE = 4096;

struct allocation {
   uint64_t start_page;
   uint64_t num_pages;
};

struct allocation_less {
   bool operator()(const allocation& lhs, const allocation& rhs) const
   {
      assert(lhs.start_page + lhs.num_pages > lhs.start_page);
      return lhs.start_page + lhs.num_pages <= rhs.start_page;
   }
};

constexpr uint64_t allocation_end_page(const allocation& a) {
   return a.start_page + a.num_pages;
}

struct random_test {
   static const uint64_t MEM_START_PAGE = 1;
   static const uint64_t MEM_SIZE = 0xfffffffffffff000;
   static const uint64_t MEM_PAGES = MEM_SIZE / MEM_PAGE_SIZE;

   random_test(uint_fast32_t seed)
      : heap_holes{allocation{MEM_START_PAGE, MEM_PAGES}}, rand{seed}
   {
      util_vma_heap_init(&heap, MEM_START_PAGE * MEM_PAGE_SIZE, MEM_SIZE);
   }

   ~random_test()
   {
      util_vma_heap_finish(&heap);
   }

   void test(unsigned long count)
   {
      std::uniform_int_distribution<> one_to_thousand(1, 1000);
      while (count-- > 0) {
         int action = one_to_thousand(rand);
         if (action == 1)          fill();
         else if (action == 2)     empty();
         else if (action < 374)    dealloc();
         else                      alloc();
      }
   }

   bool alloc(uint64_t size_order=52, uint64_t align_order=52)
   {
      std::geometric_distribution<> dist;

      if (align_order > 51)
         align_order = std::min(dist(rand), 51);
      uint64_t align_pages = 1ULL << align_order;
      uint64_t align = align_pages * MEM_PAGE_SIZE;

      if (size_order > 51)
         size_order = std::min(dist(rand), 51);
      uint64_t size_pages = 1ULL << size_order;
      uint64_t size = size_pages * MEM_PAGE_SIZE;

      uint64_t addr = util_vma_heap_alloc(&heap, size, align);

      if (addr == 0) {
         /* assert no gaps are present in the tracker that could satisfy this
          * allocation.
          */
         for (const auto& hole : heap_holes) {
            uint64_t hole_alignment_pages =
               (align_pages - (hole.start_page % align_pages)) % align_pages;
            assert(hole.num_pages < size_pages + hole_alignment_pages);
         }
         return false;
      } else {
         assert(addr % align == 0);
         uint64_t addr_page = addr / MEM_PAGE_SIZE;
         allocation a{addr_page, size_pages};
         auto i = heap_holes.find(a);
         assert(i != end(heap_holes));
         allocation hole = *i;

         assert(hole.start_page <= addr_page);
         assert(hole.num_pages >= size_pages + addr_page - hole.start_page);

         heap_holes.erase(i);
         if (hole.start_page < a.start_page) {
            heap_holes.emplace(allocation{hole.start_page,
                     a.start_page - hole.start_page});
         }
         if (allocation_end_page(hole) > allocation_end_page(a)) {
            heap_holes.emplace(allocation{allocation_end_page(a),
                     allocation_end_page(hole) - allocation_end_page(a)});
         }

         allocations.push_back(a);
         return true;
      }
   }

   void dealloc()
   {
      if (allocations.size() == 0)
         return;

      std::uniform_int_distribution<> dist(0, allocations.size() - 1);
      int to_dealloc = dist(rand);

      std::swap(allocations.at(to_dealloc), allocations.back());
      allocation a = allocations.back();
      allocations.pop_back();

      util_vma_heap_free(&heap, a.start_page * MEM_PAGE_SIZE,
                         a.num_pages * MEM_PAGE_SIZE);

      assert(heap_holes.find(a) == end(heap_holes));
      auto next = heap_holes.upper_bound(a);
      if (next != end(heap_holes)) {
         if (next->start_page == allocation_end_page(a)) {
            allocation x {a.start_page, a.num_pages + next->num_pages};
            next = heap_holes.erase(next);
            next = heap_holes.insert(next, x);

            if (next != begin(heap_holes)) {
               auto prev = next;
               prev--;
               if (allocation_end_page(*prev) == next->start_page) {
                  allocation x {prev->start_page,
                        prev->num_pages + next->num_pages};

                  heap_holes.erase(prev);
                  next = heap_holes.erase(next);
                  heap_holes.insert(next, x);
               }
            }

            return;
         }
      }

      if (next != begin(heap_holes)) {
         auto prev = next;
         prev--;
         if (allocation_end_page(*prev) == a.start_page) {
            allocation x {prev->start_page, prev->num_pages + a.num_pages};
            next = heap_holes.erase(prev);
            heap_holes.insert(next, x);

            return;
         }
      }

      heap_holes.emplace(a);
   }

   void fill()
   {
      for (int sz = 51; sz >= 0; sz--) {
         while (alloc(sz, 0))
            ;
      }
      assert(heap_holes.empty());
   }

   void empty()
   {
      while (allocations.size() != 0)
         dealloc();
      assert(heap_holes.size() == 1);
      auto& hole = *begin(heap_holes);
      assert(hole.start_page == MEM_START_PAGE && hole.num_pages == MEM_PAGES);
   }

   struct util_vma_heap heap;
   std::set<allocation, allocation_less> heap_holes;
   std::default_random_engine rand;
   std::vector<allocation> allocations;
};

}

int main(int argc, char **argv)
{
   unsigned long seed, count;
   if (argc == 3) {
      char *arg_end = NULL;
      seed = strtoul(argv[1], &arg_end, 0);
      if (!arg_end || *arg_end || seed == ULONG_MAX)
         errx(1, "invalid seed \"%s\"", argv[1]);

      arg_end = NULL;
      count = strtoul(argv[2], &arg_end, 0);
      if (!arg_end || *arg_end || count == ULONG_MAX)
         errx(1, "invalid count \"%s\"", argv[2]);
   } else if (argc == 1) {
      /* importantly chosen prime numbers. */
      seed = 8675309;
      count = 2459;
   } else {
      errx(1, "USAGE: %s seed iter_count\n", argv[0]);
   }

   random_test r{(uint_fast32_t)seed};
   r.test(count);

   printf("ok\n");
   return 0;
}
