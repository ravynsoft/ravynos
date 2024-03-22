/*
 * Copyright © 2010 Intel Corporation
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

#ifndef BRW_SHADER_H
#define BRW_SHADER_H

#include <stdint.h>
#include "brw_cfg.h"
#include "brw_compiler.h"
#include "compiler/nir/nir.h"

#ifdef __cplusplus
#include "brw_ir_analysis.h"
#include "brw_ir_allocator.h"

enum instruction_scheduler_mode {
   SCHEDULE_PRE,
   SCHEDULE_PRE_NON_LIFO,
   SCHEDULE_PRE_LIFO,
   SCHEDULE_POST,
   SCHEDULE_NONE,
};

#define UBO_START ((1 << 16) - 4)

struct backend_shader {
protected:

   backend_shader(const struct brw_compiler *compiler,
                  const struct brw_compile_params *params,
                  const nir_shader *shader,
                  struct brw_stage_prog_data *stage_prog_data,
                  bool debug_enabled);

public:
   virtual ~backend_shader();

   const struct brw_compiler *compiler;
   void *log_data; /* Passed to compiler->*_log functions */

   const struct intel_device_info * const devinfo;
   const nir_shader *nir;
   struct brw_stage_prog_data * const stage_prog_data;

   /** ralloc context for temporary data used during compile */
   void *mem_ctx;

   /**
    * List of either fs_inst or vec4_instruction (inheriting from
    * backend_instruction)
    */
   exec_list instructions;

   cfg_t *cfg;
   brw_analysis<brw::idom_tree, backend_shader> idom_analysis;

   gl_shader_stage stage;
   bool debug_enabled;

   brw::simple_allocator alloc;

   virtual void dump_instruction_to_file(const backend_instruction *inst, FILE *file) const = 0;
   virtual void dump_instructions_to_file(FILE *file) const;

   /* Convenience functions based on the above. */
   void dump_instruction(const backend_instruction *inst, FILE *file = stderr) const {
      dump_instruction_to_file(inst, file);
   }
   void dump_instructions(const char *name = nullptr) const;

   void calculate_cfg();

   virtual void invalidate_analysis(brw::analysis_dependency_class c);
};

#else
struct backend_shader;
#endif /* __cplusplus */

enum brw_reg_type brw_type_for_base_type(const struct glsl_type *type);
uint32_t brw_math_function(enum opcode op);
const char *brw_instruction_name(const struct brw_isa_info *isa,
                                 enum opcode op);
bool brw_saturate_immediate(enum brw_reg_type type, struct brw_reg *reg);
bool brw_negate_immediate(enum brw_reg_type type, struct brw_reg *reg);
bool brw_abs_immediate(enum brw_reg_type type, struct brw_reg *reg);

bool opt_predicated_break(struct backend_shader *s);

#ifdef __cplusplus
extern "C" {
#endif

/* brw_fs_reg_allocate.cpp */
void brw_fs_alloc_reg_sets(struct brw_compiler *compiler);

/* brw_vec4_reg_allocate.cpp */
void brw_vec4_alloc_reg_set(struct brw_compiler *compiler);

/* brw_disasm.c */
extern const char *const conditional_modifier[16];
extern const char *const pred_ctrl_align16[16];

/* Per-thread scratch space is a power-of-two multiple of 1KB. */
static inline unsigned
brw_get_scratch_size(int size)
{
   return MAX2(1024, util_next_power_of_two(size));
}


static inline nir_variable_mode
brw_nir_no_indirect_mask(const struct brw_compiler *compiler,
                         gl_shader_stage stage)
{
   const struct intel_device_info *devinfo = compiler->devinfo;
   const bool is_scalar = compiler->scalar_stage[stage];
   nir_variable_mode indirect_mask = (nir_variable_mode) 0;

   switch (stage) {
   case MESA_SHADER_VERTEX:
   case MESA_SHADER_FRAGMENT:
      indirect_mask |= nir_var_shader_in;
      break;

   case MESA_SHADER_GEOMETRY:
      if (!is_scalar)
         indirect_mask |= nir_var_shader_in;
      break;

   default:
      /* Everything else can handle indirect inputs */
      break;
   }

   if (is_scalar && stage != MESA_SHADER_TESS_CTRL &&
                    stage != MESA_SHADER_TASK &&
                    stage != MESA_SHADER_MESH)
      indirect_mask |= nir_var_shader_out;

   /* On HSW+, we allow indirects in scalar shaders.  They get implemented
    * using nir_lower_vars_to_explicit_types and nir_lower_explicit_io in
    * brw_postprocess_nir.
    *
    * We haven't plumbed through the indirect scratch messages on gfx6 or
    * earlier so doing indirects via scratch doesn't work there. On gfx7 and
    * earlier the scratch space size is limited to 12kB.  If we allowed
    * indirects as scratch all the time, we may easily exceed this limit
    * without having any fallback.
    */
   if (is_scalar && devinfo->verx10 <= 70)
      indirect_mask |= nir_var_function_temp;

   return indirect_mask;
}

bool brw_texture_offset(const nir_tex_instr *tex, unsigned src,
                        uint32_t *offset_bits);

/**
 * Scratch data used when compiling a GLSL geometry shader.
 */
struct brw_gs_compile
{
   struct brw_gs_prog_key key;
   struct brw_vue_map input_vue_map;

   unsigned control_data_bits_per_vertex;
   unsigned control_data_header_size_bits;
};

#ifdef __cplusplus
}
#endif

#endif /* BRW_SHADER_H */
