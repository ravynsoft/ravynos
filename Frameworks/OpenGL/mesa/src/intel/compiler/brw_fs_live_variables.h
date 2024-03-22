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

#ifndef BRW_FS_LIVE_VARIABLES_H
#define BRW_FS_LIVE_VARIABLES_H

#include "brw_ir_analysis.h"
#include "brw_ir_fs.h"
#include "util/bitset.h"

struct cfg_t;
struct backend_shader;

namespace brw {

class fs_live_variables {
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

      /**
       * Variables such that the entry point of the block may be reached from any
       * of their definitions.
       */
      BITSET_WORD *defin;

      /**
       * Variables such that the exit point of the block may be reached from any
       * of their definitions.
       */
      BITSET_WORD *defout;

      BITSET_WORD flag_def[1];
      BITSET_WORD flag_use[1];
      BITSET_WORD flag_livein[1];
      BITSET_WORD flag_liveout[1];
   };

   fs_live_variables(const backend_shader *s);
   ~fs_live_variables();

   bool validate(const backend_shader *s) const;

   analysis_dependency_class
   dependency_class() const
   {
      return (DEPENDENCY_INSTRUCTION_IDENTITY |
              DEPENDENCY_INSTRUCTION_DATA_FLOW |
              DEPENDENCY_VARIABLES);
   }

   bool vars_interfere(int a, int b) const;
   bool vgrfs_interfere(int a, int b) const;
   int var_from_reg(const fs_reg &reg) const
   {
      return var_from_vgrf[reg.nr] + reg.offset / REG_SIZE;
   }

   /** Map from virtual GRF number to index in block_data arrays. */
   int *var_from_vgrf;

   /**
    * Map from any index in block_data to the virtual GRF containing it.
    *
    * For alloc.sizes of [1, 2, 3], vgrf_from_var would contain
    * [0, 1, 1, 2, 2, 2].
    */
   int *vgrf_from_var;

   int num_vars;
   int num_vgrfs;
   int bitset_words;

   /** @{
    * Final computed live ranges for each var (each component of each virtual
    * GRF).
    */
   int *start;
   int *end;
   /** @} */

   /** @{
    * Final computed live ranges for each VGRF.
    */
   int *vgrf_start;
   int *vgrf_end;
   /** @} */

   /** Per-basic-block information on live variables */
   struct block_data *block_data;

protected:
   void setup_def_use();
   void setup_one_read(struct block_data *bd, int ip, const fs_reg &reg);
   void setup_one_write(struct block_data *bd, fs_inst *inst, int ip,
                        const fs_reg &reg);
   void compute_live_variables();
   void compute_start_end();

   const struct intel_device_info *devinfo;
   const cfg_t *cfg;
   void *mem_ctx;
};

} /* namespace brw */

#endif /* BRW_FS_LIVE_VARIABLES_H */
