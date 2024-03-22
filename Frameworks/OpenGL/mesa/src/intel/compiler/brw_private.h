/* -*- c++ -*- */
/*
 * Copyright © 2021 Intel Corporation
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

#ifndef BRW_PRIVATE_H
#define BRW_PRIVATE_H

#include "brw_compiler.h"

#include <variant>

unsigned brw_required_dispatch_width(const struct shader_info *info);

static constexpr int SIMD_COUNT = 3;

struct brw_simd_selection_state {
   const struct intel_device_info *devinfo;

   std::variant<struct brw_cs_prog_data *,
                struct brw_bs_prog_data *> prog_data;

   unsigned required_width;

   const char *error[SIMD_COUNT];

   bool compiled[SIMD_COUNT];
   bool spilled[SIMD_COUNT];
};

inline int brw_simd_first_compiled(const brw_simd_selection_state &state)
{
   for (int i = 0; i < SIMD_COUNT; i++) {
      if (state.compiled[i])
         return i;
   }
   return -1;
}

inline bool brw_simd_any_compiled(const brw_simd_selection_state &state)
{
   return brw_simd_first_compiled(state) >= 0;
}

bool brw_simd_should_compile(brw_simd_selection_state &state, unsigned simd);

void brw_simd_mark_compiled(brw_simd_selection_state &state, unsigned simd, bool spilled);

int brw_simd_select(const brw_simd_selection_state &state);

int brw_simd_select_for_workgroup_size(const struct intel_device_info *devinfo,
                                       const struct brw_cs_prog_data *prog_data,
                                       const unsigned *sizes);

bool brw_should_print_shader(const nir_shader *shader, uint64_t debug_flag);

#endif // BRW_PRIVATE_H
