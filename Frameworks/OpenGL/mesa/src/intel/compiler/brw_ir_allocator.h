/* -*- c++ -*- */
/*
 * Copyright © 2010-2014 Intel Corporation
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

#ifndef BRW_IR_ALLOCATOR_H
#define BRW_IR_ALLOCATOR_H

#include "util/compiler.h"
#include "util/glheader.h"
#include "util/macros.h"
#include "util/rounding.h"
#include "util/u_math.h"

namespace brw {
   /**
    * Simple allocator used to keep track of virtual GRFs.
    */
   class simple_allocator {
   public:
      simple_allocator() :
         sizes(NULL), offsets(NULL), count(0), total_size(0), capacity(0)
      {
      }

      ~simple_allocator()
      {
         free(offsets);
         free(sizes);
      }

      unsigned
      allocate(unsigned size)
      {
         assert(size > 0);
         if (capacity <= count) {
            capacity = MAX2(16, capacity * 2);
            sizes = (unsigned *)realloc(sizes, capacity * sizeof(unsigned));
            offsets = (unsigned *)realloc(offsets, capacity * sizeof(unsigned));
         }

         sizes[count] = size;
         offsets[count] = total_size;
         total_size += size;

         return count++;
      }

      /**
       * Array of sizes for each allocation.  The allocation unit is up to the
       * back-end, but it's expected to be one scalar value in the FS back-end
       * and one vec4 in the VEC4 back-end.
       */
      unsigned *sizes;

      /**
       * Array of offsets from the start of the VGRF space in allocation
       * units.
       */
      unsigned *offsets;

      /** Total number of VGRFs allocated. */
      unsigned count;

      /** Cumulative size in allocation units. */
      unsigned total_size;

   private:
      unsigned capacity;
   };
}

#endif
