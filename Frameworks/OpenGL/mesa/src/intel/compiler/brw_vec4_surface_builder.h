/* -*- c++ -*- */
/*
 * Copyright © 2013-2015 Intel Corporation
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

#ifndef BRW_VEC4_SURFACE_BUILDER_H
#define BRW_VEC4_SURFACE_BUILDER_H

#include "brw_vec4_builder.h"

namespace brw {
   namespace surface_access {
      src_reg
      emit_untyped_read(const vec4_builder &bld,
                        const src_reg &surface, const src_reg &addr,
                        unsigned dims, unsigned size,
                        brw_predicate pred = BRW_PREDICATE_NONE);

      void
      emit_untyped_write(const vec4_builder &bld, const src_reg &surface,
                         const src_reg &addr, const src_reg &src,
                         unsigned dims, unsigned size,
                         brw_predicate pred = BRW_PREDICATE_NONE);

      src_reg
      emit_untyped_atomic(const vec4_builder &bld,
                          const src_reg &surface, const src_reg &addr,
                          const src_reg &src0, const src_reg &src1,
                          unsigned dims, unsigned rsize, unsigned op,
                          brw_predicate pred = BRW_PREDICATE_NONE);
   }
}

#endif
