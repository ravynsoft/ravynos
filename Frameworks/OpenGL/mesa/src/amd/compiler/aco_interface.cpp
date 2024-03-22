/*
 * Copyright © 2018 Google
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

#include "aco_interface.h"

#include "aco_ir.h"

#include "util/memstream.h"

#include "ac_gpu_info.h"
#include <array>
#include <iostream>
#include <vector>

static const std::array<aco_compiler_statistic_info, aco_num_statistics> statistic_infos = []()
{
   std::array<aco_compiler_statistic_info, aco_num_statistics> ret{};
   ret[aco_statistic_hash] =
      aco_compiler_statistic_info{"Hash", "CRC32 hash of code and constant data"};
   ret[aco_statistic_instructions] =
      aco_compiler_statistic_info{"Instructions", "Instruction count"};
   ret[aco_statistic_copies] =
      aco_compiler_statistic_info{"Copies", "Copy instructions created for pseudo-instructions"};
   ret[aco_statistic_branches] = aco_compiler_statistic_info{"Branches", "Branch instructions"};
   ret[aco_statistic_latency] =
      aco_compiler_statistic_info{"Latency", "Issue cycles plus stall cycles"};
   ret[aco_statistic_inv_throughput] = aco_compiler_statistic_info{
      "Inverse Throughput", "Estimated busy cycles to execute one wave"};
   ret[aco_statistic_vmem_clauses] = aco_compiler_statistic_info{
      "VMEM Clause", "Number of VMEM clauses (includes 1-sized clauses)"};
   ret[aco_statistic_smem_clauses] = aco_compiler_statistic_info{
      "SMEM Clause", "Number of SMEM clauses (includes 1-sized clauses)"};
   ret[aco_statistic_sgpr_presched] =
      aco_compiler_statistic_info{"Pre-Sched SGPRs", "SGPR usage before scheduling"};
   ret[aco_statistic_vgpr_presched] =
      aco_compiler_statistic_info{"Pre-Sched VGPRs", "VGPR usage before scheduling"};
   ret[aco_statistic_valu] = aco_compiler_statistic_info{"VALU", "Number of VALU instructions"};
   ret[aco_statistic_salu] = aco_compiler_statistic_info{"SALU", "Number of SALU instructions"};
   ret[aco_statistic_vmem] = aco_compiler_statistic_info{"VMEM", "Number of VMEM instructions"};
   ret[aco_statistic_smem] = aco_compiler_statistic_info{"SMEM", "Number of SMEM instructions"};
   return ret;
}();

const aco_compiler_statistic_info* aco_statistic_infos = statistic_infos.data();

uint64_t
aco_get_codegen_flags()
{
   aco::init();
   /* Exclude flags which don't affect code generation. */
   uint64_t exclude = aco::DEBUG_VALIDATE_IR | aco::DEBUG_VALIDATE_RA | aco::DEBUG_PERFWARN |
                      aco::DEBUG_PERF_INFO | aco::DEBUG_LIVE_INFO;
   return aco::debug_flags & ~exclude;
}

static void
validate(aco::Program* program)
{
   if (!(aco::debug_flags & aco::DEBUG_VALIDATE_IR))
      return;

   ASSERTED bool is_valid = aco::validate_ir(program);
   assert(is_valid);
}

static std::string
get_disasm_string(aco::Program* program, std::vector<uint32_t>& code, unsigned exec_size)
{
   std::string disasm;

   char* data = NULL;
   size_t disasm_size = 0;
   struct u_memstream mem;
   if (u_memstream_open(&mem, &data, &disasm_size)) {
      FILE* const memf = u_memstream_get(&mem);
      if (check_print_asm_support(program)) {
         aco::print_asm(program, code, exec_size / 4u, memf);
      } else {
         fprintf(memf, "Shader disassembly is not supported in the current configuration"
#if !LLVM_AVAILABLE
                       " (LLVM not available)"
#endif
                       ", falling back to print_program.\n\n");
         aco::aco_print_program(program, memf);
      }
      fputc(0, memf);
      u_memstream_close(&mem);
      disasm = std::string(data, data + disasm_size);
      free(data);
   }

   return disasm;
}

static std::string
aco_postprocess_shader(const struct aco_compiler_options* options,
                       const struct aco_shader_info* info, std::unique_ptr<aco::Program>& program)
{
   std::string llvm_ir;

   if (options->dump_preoptir)
      aco_print_program(program.get(), stderr);

   ASSERTED bool is_valid = aco::validate_cfg(program.get());
   assert(is_valid);

   aco::live live_vars;
   if (!info->is_trap_handler_shader) {
      aco::dominator_tree(program.get());
      aco::lower_phis(program.get());
      validate(program.get());

      /* Optimization */
      if (!options->optimisations_disabled) {
         if (!(aco::debug_flags & aco::DEBUG_NO_VN))
            aco::value_numbering(program.get());
         if (!(aco::debug_flags & aco::DEBUG_NO_OPT))
            aco::optimize(program.get());
      }

      /* cleanup and exec mask handling */
      aco::setup_reduce_temp(program.get());
      aco::insert_exec_mask(program.get());
      validate(program.get());

      /* spilling and scheduling */
      live_vars = aco::live_var_analysis(program.get());
      if (program->collect_statistics)
         aco::collect_presched_stats(program.get());
      aco::spill(program.get(), live_vars);
   }

   if (options->record_ir) {
      char* data = NULL;
      size_t size = 0;
      u_memstream mem;
      if (u_memstream_open(&mem, &data, &size)) {
         FILE* const memf = u_memstream_get(&mem);
         aco_print_program(program.get(), memf);
         fputc(0, memf);
         u_memstream_close(&mem);
      }

      llvm_ir = std::string(data, data + size);
      free(data);
   }

   if ((aco::debug_flags & aco::DEBUG_LIVE_INFO) && options->dump_shader)
      aco_print_program(program.get(), stderr, live_vars, aco::print_live_vars | aco::print_kill);

   if (!info->is_trap_handler_shader) {
      if (!options->optimisations_disabled && !(aco::debug_flags & aco::DEBUG_NO_SCHED))
         aco::schedule_program(program.get(), live_vars);
      validate(program.get());

      /* Register Allocation */
      aco::register_allocation(program.get(), live_vars.live_out);

      if (aco::validate_ra(program.get())) {
         aco_print_program(program.get(), stderr);
         abort();
      } else if (options->dump_shader) {
         aco_print_program(program.get(), stderr);
      }

      validate(program.get());

      /* Optimization */
      if (!options->optimisations_disabled && !(aco::debug_flags & aco::DEBUG_NO_OPT)) {
         aco::optimize_postRA(program.get());
         validate(program.get());
      }

      aco::ssa_elimination(program.get());
   }

   /* Lower to HW Instructions */
   aco::lower_to_hw_instr(program.get());
   validate(program.get());

   /* Schedule hardware instructions for ILP */
   if (!options->optimisations_disabled && !(aco::debug_flags & aco::DEBUG_NO_SCHED_ILP))
      aco::schedule_ilp(program.get());

   /* Insert Waitcnt */
   aco::insert_wait_states(program.get());
   aco::insert_NOPs(program.get());

   if (program->gfx_level >= GFX10)
      aco::form_hard_clauses(program.get());

   if (program->collect_statistics || (aco::debug_flags & aco::DEBUG_PERF_INFO))
      aco::collect_preasm_stats(program.get());

   return llvm_ir;
}

void
aco_compile_shader(const struct aco_compiler_options* options, const struct aco_shader_info* info,
                   unsigned shader_count, struct nir_shader* const* shaders,
                   const struct ac_shader_args* args, aco_callback* build_binary, void** binary)
{
   aco::init();

   ac_shader_config config = {0};
   std::unique_ptr<aco::Program> program{new aco::Program};

   program->collect_statistics = options->record_stats;
   if (program->collect_statistics)
      memset(program->statistics, 0, sizeof(program->statistics));

   program->debug.func = options->debug.func;
   program->debug.private_data = options->debug.private_data;

   /* Instruction Selection */
   if (info->is_trap_handler_shader)
      aco::select_trap_handler_shader(program.get(), shaders[0], &config, options, info, args);
   else
      aco::select_program(program.get(), shader_count, shaders, &config, options, info, args);

   std::string llvm_ir = aco_postprocess_shader(options, info, program);

   /* assembly */
   std::vector<uint32_t> code;
   std::vector<struct aco_symbol> symbols;
   /* OpenGL combine multi shader parts into one continous code block,
    * so only last part need the s_endpgm instruction.
    */
   bool append_endpgm = !(options->is_opengl && info->has_epilog);
   unsigned exec_size = aco::emit_program(program.get(), code, &symbols, append_endpgm);

   if (program->collect_statistics)
      aco::collect_postasm_stats(program.get(), code);

   bool get_disasm = options->dump_shader || options->record_ir;

   std::string disasm;
   if (get_disasm)
      disasm = get_disasm_string(program.get(), code, exec_size);

   size_t stats_size = 0;
   if (program->collect_statistics)
      stats_size = aco_num_statistics * sizeof(uint32_t);

   (*build_binary)(binary, &config, llvm_ir.c_str(), llvm_ir.size(), disasm.c_str(), disasm.size(),
                   program->statistics, stats_size, exec_size, code.data(), code.size(),
                   symbols.data(), symbols.size());
}

void
aco_compile_rt_prolog(const struct aco_compiler_options* options,
                      const struct aco_shader_info* info, const struct ac_shader_args* in_args,
                      const struct ac_shader_args* out_args, aco_callback* build_prolog,
                      void** binary)
{
   aco::init();

   /* create program */
   ac_shader_config config = {0};
   std::unique_ptr<aco::Program> program{new aco::Program};
   program->collect_statistics = false;
   program->debug.func = NULL;
   program->debug.private_data = NULL;

   aco::select_rt_prolog(program.get(), &config, options, info, in_args, out_args);
   validate(program.get());
   aco::insert_wait_states(program.get());
   aco::insert_NOPs(program.get());
   if (program->gfx_level >= GFX10)
      aco::form_hard_clauses(program.get());

   if (options->dump_shader)
      aco_print_program(program.get(), stderr);

   /* assembly */
   std::vector<uint32_t> code;
   code.reserve(align(program->blocks[0].instructions.size() * 2, 16));
   unsigned exec_size = aco::emit_program(program.get(), code);

   bool get_disasm = options->dump_shader || options->record_ir;

   std::string disasm;
   if (get_disasm)
      disasm = get_disasm_string(program.get(), code, exec_size);

   (*build_prolog)(binary, &config, NULL, 0, disasm.c_str(), disasm.size(), program->statistics, 0,
                   exec_size, code.data(), code.size(), NULL, 0);
}

void
aco_compile_vs_prolog(const struct aco_compiler_options* options,
                      const struct aco_shader_info* info, const struct aco_vs_prolog_info* pinfo,
                      const struct ac_shader_args* args, aco_shader_part_callback* build_prolog,
                      void** binary)
{
   aco::init();

   /* create program */
   ac_shader_config config = {0};
   std::unique_ptr<aco::Program> program{new aco::Program};
   program->collect_statistics = false;
   program->debug.func = NULL;
   program->debug.private_data = NULL;

   /* create IR */
   aco::select_vs_prolog(program.get(), pinfo, &config, options, info, args);
   validate(program.get());
   aco::insert_NOPs(program.get());

   if (options->dump_shader)
      aco_print_program(program.get(), stderr);

   /* assembly */
   std::vector<uint32_t> code;
   code.reserve(align(program->blocks[0].instructions.size() * 2, 16));
   unsigned exec_size = aco::emit_program(program.get(), code);

   bool get_disasm = options->dump_shader || options->record_ir;

   std::string disasm;
   if (get_disasm)
      disasm = get_disasm_string(program.get(), code, exec_size);

   (*build_prolog)(binary, config.num_sgprs, config.num_vgprs, code.data(), code.size(),
                   disasm.data(), disasm.size());
}

typedef void(select_shader_part_callback)(aco::Program* program, void* pinfo,
                                          ac_shader_config* config,
                                          const struct aco_compiler_options* options,
                                          const struct aco_shader_info* info,
                                          const struct ac_shader_args* args);

static void
aco_compile_shader_part(const struct aco_compiler_options* options,
                        const struct aco_shader_info* info, const struct ac_shader_args* args,
                        select_shader_part_callback select_shader_part, void* pinfo,
                        aco_shader_part_callback* build_binary, void** binary,
                        bool is_prolog = false)
{
   aco::init();

   ac_shader_config config = {0};
   std::unique_ptr<aco::Program> program{new aco::Program};

   program->collect_statistics = options->record_stats;
   if (program->collect_statistics)
      memset(program->statistics, 0, sizeof(program->statistics));

   program->debug.func = options->debug.func;
   program->debug.private_data = options->debug.private_data;

   program->is_prolog = is_prolog;

   /* Instruction selection */
   select_shader_part(program.get(), pinfo, &config, options, info, args);

   aco_postprocess_shader(options, info, program);

   /* assembly */
   std::vector<uint32_t> code;
   bool append_endpgm = !(options->is_opengl && is_prolog);
   unsigned exec_size = aco::emit_program(program.get(), code, NULL, append_endpgm);

   bool get_disasm = options->dump_shader || options->record_ir;

   std::string disasm;
   if (get_disasm)
      disasm = get_disasm_string(program.get(), code, exec_size);

   (*build_binary)(binary, config.num_sgprs, config.num_vgprs, code.data(), code.size(),
                   disasm.data(), disasm.size());
}

void
aco_compile_ps_epilog(const struct aco_compiler_options* options,
                      const struct aco_shader_info* info, const struct aco_ps_epilog_info* pinfo,
                      const struct ac_shader_args* args, aco_shader_part_callback* build_epilog,
                      void** binary)
{
   aco_compile_shader_part(options, info, args, aco::select_ps_epilog, (void*)pinfo, build_epilog,
                           binary);
}

void
aco_compile_tcs_epilog(const struct aco_compiler_options* options,
                       const struct aco_shader_info* info, const struct aco_tcs_epilog_info* pinfo,
                       const struct ac_shader_args* args, aco_shader_part_callback* build_epilog,
                       void** binary)
{
   aco_compile_shader_part(options, info, args, aco::select_tcs_epilog, (void*)pinfo, build_epilog,
                           binary);
}

void
aco_compile_gl_vs_prolog(const struct aco_compiler_options* options,
                         const struct aco_shader_info* info,
                         const struct aco_gl_vs_prolog_info* pinfo,
                         const struct ac_shader_args* args, aco_shader_part_callback* build_prolog,
                         void** binary)
{
   aco_compile_shader_part(options, info, args, aco::select_gl_vs_prolog, (void*)pinfo,
                           build_prolog, binary, true);
}

void
aco_compile_ps_prolog(const struct aco_compiler_options* options,
                      const struct aco_shader_info* info, const struct aco_ps_prolog_info* pinfo,
                      const struct ac_shader_args* args, aco_shader_part_callback* build_prolog,
                      void** binary)
{
   aco_compile_shader_part(options, info, args, aco::select_ps_prolog, (void*)pinfo, build_prolog,
                           binary, true);
}

bool
aco_is_gpu_supported(const struct radeon_info* info)
{
   /* Does not support compute only cards yet. */
   return info->gfx_level >= GFX6 && info->has_graphics;
}

bool
aco_nir_op_supports_packed_math_16bit(const nir_alu_instr* alu)
{
   switch (alu->op) {
   case nir_op_f2f16: {
      nir_shader* shader = nir_cf_node_get_function(&alu->instr.block->cf_node)->function->shader;
      unsigned execution_mode = shader->info.float_controls_execution_mode;
      return (shader->options->force_f2f16_rtz && !nir_is_rounding_mode_rtne(execution_mode, 16)) ||
             nir_is_rounding_mode_rtz(execution_mode, 16);
   }
   case nir_op_fadd:
   case nir_op_fsub:
   case nir_op_fmul:
   case nir_op_ffma:
   case nir_op_fdiv:
   case nir_op_flrp:
   case nir_op_fabs:
   case nir_op_fneg:
   case nir_op_fsat:
   case nir_op_fmin:
   case nir_op_fmax:
   case nir_op_f2f16_rtz:
   case nir_op_iabs:
   case nir_op_iadd:
   case nir_op_iadd_sat:
   case nir_op_uadd_sat:
   case nir_op_isub:
   case nir_op_isub_sat:
   case nir_op_usub_sat:
   case nir_op_ineg:
   case nir_op_imul:
   case nir_op_imin:
   case nir_op_imax:
   case nir_op_umin:
   case nir_op_umax:
   case nir_op_fddx:
   case nir_op_fddy:
   case nir_op_fddx_fine:
   case nir_op_fddy_fine:
   case nir_op_fddx_coarse:
   case nir_op_fddy_coarse: return true;
   case nir_op_ishl: /* TODO: in NIR, these have 32bit shift operands */
   case nir_op_ishr: /* while Radeon needs 16bit operands when vectorized */
   case nir_op_ushr:
   default: return false;
   }
}
