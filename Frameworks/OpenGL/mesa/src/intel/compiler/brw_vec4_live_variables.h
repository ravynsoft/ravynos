/*
 * Copyright © 2012 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

#ifndef BRW_VEC4_LIVE_VARIABLES_H
#define BRW_VEC4_LIVE_VARIABLES_H

#include "brw_ir_vec4.h"
#include "brw_ir_analysis.h"
#include "util/bitset.h"

struct backend_shader;

namespace brw {

class vec4_live_variables {
public:
   struct block_data {
      /**
       * Which variables are defined before being used in the block.
       *
       * Note that for our purposes, "defined" means unconditionally, completely
       * defined.
       */
      BITSET_WORD *def;

      /**
       * Which variables are used before being defined in the block.
       */
      BITSET_WORD *use;

      /** Which defs reach the entry point of the block. */
      BITSET_WORD *livein;

      /** Which defs reach the exit point of the block. */
      BITSET_WORD *liveout;

      BITSET_WORD flag_def[1];
      BITSET_WORD flag_use[1];
      BITSET_WORD flag_livein[1];
      BITSET_WORD flag_liveout[1];
   };

   vec4_live_variables(const backend_shader *s);
   ~vec4_live_variables();

   bool
   validate(const backend_shader *s) const;

   analysis_dependency_class
   dependency_class() const
   {
      return (DEPENDENCY_INSTRUCTION_IDENTITY |
              DEPENDENCY_INSTRUCTION_DATA_FLOW |
              DEPENDENCY_VARIABLES);
   }

   int num_vars;
   int bitset_words;

   const struct intel_device_info *devinfo;

   /** Per-basic-block information on live variables */
   struct block_data *block_data;

   /** @{
    * Final computed live ranges for each variable.
    */
   int *start;
   int *end;
   /** @} */

   int var_range_start(unsigned v, unsigned n) const;
   int var_range_end(unsigned v, unsigned n) const;
   bool vgrfs_interfere(int a, int b) const;

protected:
   void setup_def_use();
   void compute_live_variables();
   void compute_start_end();

   const simple_allocator &alloc;
   cfg_t *cfg;
   void *mem_ctx;
};

/* Returns the variable index for the k-th dword of the c-th component of
 * register reg.
 */
inline unsigned
var_from_reg(const simple_allocator &alloc, const src_reg &reg,
             unsigned c = 0, unsigned k = 0)
{
   assert(reg.file == VGRF && reg.nr < alloc.count && c < 4);
   const unsigned csize = DIV_ROUND_UP(type_sz(reg.type), 4);
   unsigned result =
      8 * alloc.offsets[reg.nr] + reg.offset / 4 +
      (BRW_GET_SWZ(reg.swizzle, c) + k / csize * 4) * csize + k % csize;
   /* Do not exceed the limit for this register */
   assert(result < 8 * (alloc.offsets[reg.nr] + alloc.sizes[reg.nr]));
   return result;
}

inline unsigned
var_from_reg(const simple_allocator &alloc, const dst_reg &reg,
             unsigned c = 0, unsigned k = 0)
{
   assert(reg.file == VGRF && reg.nr < alloc.count && c < 4);
   const unsigned csize = DIV_ROUND_UP(type_sz(reg.type), 4);
   unsigned result =
      8 * alloc.offsets[reg.nr] + reg.offset / 4 +
      (c + k / csize * 4) * csize + k % csize;
   /* Do not exceed the limit for this register */
   assert(result < 8 * (alloc.offsets[reg.nr] + alloc.sizes[reg.nr]));
   return result;
}

} /* namespace brw */

#endif /* BRW_VEC4_LIVE_VARIABLES_H */
