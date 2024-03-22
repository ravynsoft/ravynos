/* -*- c++ -*- */
/*
 * Copyright © 2020 Intel Corporation
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

#ifndef BRW_IR_PERFORMANCE_H
#define BRW_IR_PERFORMANCE_H

class fs_visitor;

namespace brw {
   class vec4_visitor;

   /**
    * Various estimates of the performance of a shader based on static
    * analysis.
    */
   struct performance {
      performance(const fs_visitor *v);
      performance(const vec4_visitor *v);
      ~performance();

      analysis_dependency_class
      dependency_class() const
      {
         return (DEPENDENCY_INSTRUCTIONS |
                 DEPENDENCY_BLOCKS);
      }

      bool
      validate(const backend_shader *) const
      {
         return true;
      }

      /**
       * Array containing estimates of the runtime of each basic block of the
       * program in cycle units.
       */
      unsigned *block_latency;

      /**
       * Estimate of the runtime of the whole program in cycle units assuming
       * uncontended execution.
       */
      unsigned latency;

      /**
       * Estimate of the throughput of the whole program in
       * invocations-per-cycle units.
       *
       * Note that this might be lower than the ratio between the dispatch
       * width of the program and its latency estimate in cases where
       * performance doesn't scale without limits as a function of its thread
       * parallelism, e.g. due to the existence of a bottleneck in a shared
       * function.
       */
      float throughput;

   private:
      performance(const performance &perf);
      performance &
      operator=(performance u);
   };
}

#endif
