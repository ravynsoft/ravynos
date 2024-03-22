/*
 * Copyright © 2018 Valve Corporation
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
 */

#ifndef ACO_INSTRUCTION_SELECTION_H
#define ACO_INSTRUCTION_SELECTION_H

#include "aco_ir.h"

#include "nir.h"

#include <array>
#include <unordered_map>
#include <vector>

namespace aco {

enum aco_color_output_type {
   ACO_TYPE_ANY32,
   ACO_TYPE_FLOAT16,
   ACO_TYPE_INT16,
   ACO_TYPE_UINT16,
};

struct shader_io_state {
   uint8_t mask[VARYING_SLOT_MAX];
   Temp temps[VARYING_SLOT_MAX * 4u];

   shader_io_state()
   {
      memset(mask, 0, sizeof(mask));
      std::fill_n(temps, VARYING_SLOT_MAX * 4u, Temp(0, RegClass::v1));
   }
};

struct isel_context {
   const struct aco_compiler_options* options;
   const struct ac_shader_args* args;
   Program* program;
   nir_shader* shader;
   uint32_t constant_data_offset;
   Block* block;
   uint32_t first_temp_id;
   std::unordered_map<unsigned, std::array<Temp, NIR_MAX_VEC_COMPONENTS>> allocated_vec;
   std::vector<Temp> unended_linear_vgprs;
   Stage stage;
   struct {
      bool has_branch;
      struct {
         unsigned header_idx;
         Block* exit;
         bool has_divergent_continue = false;
         bool has_divergent_branch = false;
      } parent_loop;
      struct {
         bool is_divergent = false;
      } parent_if;
      bool had_divergent_discard = false;
      bool exec_potentially_empty_discard =
         false; /* set to false when loop_nest_depth==0 && parent_if.is_divergent==false */
      uint16_t exec_potentially_empty_break_depth = UINT16_MAX;
      /* Set to false when loop_nest_depth==exec_potentially_empty_break_depth
       * and parent_if.is_divergent==false. Called _break but it's also used for
       * loop continues. */
      bool exec_potentially_empty_break = false;
      std::unique_ptr<unsigned[]> nir_to_aco; /* NIR block index to ACO block index */
   } cf_info;

   /* NIR range analysis. */
   struct hash_table* range_ht;
   nir_unsigned_upper_bound_config ub_config;

   Temp arg_temps[AC_MAX_ARGS];

   /* tessellation information */
   uint64_t tcs_temp_only_inputs;
   bool tcs_in_out_eq = false;

   /* Fragment color output information */
   uint16_t output_color_types;

   /* I/O information */
   shader_io_state inputs;
   shader_io_state outputs;

   /* WQM information */
   uint32_t wqm_block_idx;
   uint32_t wqm_instruction_idx;
};

inline Temp
get_arg(isel_context* ctx, struct ac_arg arg)
{
   assert(arg.used);
   return ctx->arg_temps[arg.arg_index];
}

void init_context(isel_context* ctx, nir_shader* shader);
void cleanup_context(isel_context* ctx);

isel_context setup_isel_context(Program* program, unsigned shader_count,
                                struct nir_shader* const* shaders, ac_shader_config* config,
                                const struct aco_compiler_options* options,
                                const struct aco_shader_info* info,
                                const struct ac_shader_args* args,
                                SWStage sw_stage = SWStage::None);

} // namespace aco

#endif /* ACO_INSTRUCTION_SELECTION_H */
