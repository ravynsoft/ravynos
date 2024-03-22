/*
 * Copyright © 2011 Intel Corporation
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

#ifndef BRW_VEC4_H
#define BRW_VEC4_H

#include "brw_shader.h"

#ifdef __cplusplus
#include "brw_ir_vec4.h"
#include "brw_ir_performance.h"
#include "brw_vec4_builder.h"
#include "brw_vec4_live_variables.h"
#endif

#include "compiler/glsl/ir.h"
#include "compiler/nir/nir.h"


#ifdef __cplusplus
extern "C" {
#endif

const unsigned *
brw_vec4_generate_assembly(const struct brw_compiler *compiler,
                           const struct brw_compile_params *params,
                           const nir_shader *nir,
                           struct brw_vue_prog_data *prog_data,
                           const struct cfg_t *cfg,
                           const brw::performance &perf,
                           bool debug_enabled);

#ifdef __cplusplus
} /* extern "C" */

namespace brw {
/**
 * The vertex shader front-end.
 *
 * Translates either GLSL IR or Mesa IR (for ARB_vertex_program and
 * fixed-function) into VS IR.
 */
class vec4_visitor : public backend_shader
{
public:
   vec4_visitor(const struct brw_compiler *compiler,
                const struct brw_compile_params *params,
                const struct brw_sampler_prog_key_data *key,
                struct brw_vue_prog_data *prog_data,
                const nir_shader *shader,
                bool no_spills,
                bool debug_enabled);

   dst_reg dst_null_f()
   {
      return dst_reg(brw_null_reg());
   }

   dst_reg dst_null_df()
   {
      return dst_reg(retype(brw_null_reg(), BRW_REGISTER_TYPE_DF));
   }

   dst_reg dst_null_d()
   {
      return dst_reg(retype(brw_null_reg(), BRW_REGISTER_TYPE_D));
   }

   dst_reg dst_null_ud()
   {
      return dst_reg(retype(brw_null_reg(), BRW_REGISTER_TYPE_UD));
   }

   const struct brw_sampler_prog_key_data * const key_tex;
   struct brw_vue_prog_data * const prog_data;
   char *fail_msg;
   bool failed;

   /**
    * GLSL IR currently being processed, which is associated with our
    * driver IR instructions for debugging purposes.
    */
   const void *base_ir;
   const char *current_annotation;

   int first_non_payload_grf;
   unsigned ubo_push_start[4];
   unsigned push_length;
   unsigned int max_grf;
   brw_analysis<brw::vec4_live_variables, backend_shader> live_analysis;
   brw_analysis<brw::performance, vec4_visitor> performance_analysis;

   /* Regs for vertex results.  Generated at ir_variable visiting time
    * for the ir->location's used.
    */
   dst_reg output_reg[VARYING_SLOT_TESS_MAX][4];
   unsigned output_num_components[VARYING_SLOT_TESS_MAX][4];
   const char *output_reg_annotation[VARYING_SLOT_TESS_MAX];
   int uniforms;

   bool run();
   void fail(const char *msg, ...);

   int setup_uniforms(int payload_reg);

   bool reg_allocate_trivial();
   bool reg_allocate();
   void evaluate_spill_costs(float *spill_costs, bool *no_spill);
   int choose_spill_reg(struct ra_graph *g);
   void spill_reg(unsigned spill_reg);
   void move_grf_array_access_to_scratch();
   void split_uniform_registers();
   void setup_push_ranges();
   virtual void invalidate_analysis(brw::analysis_dependency_class c);
   void split_virtual_grfs();
   bool opt_vector_float();
   bool opt_reduce_swizzle();
   bool dead_code_eliminate();
   bool opt_cmod_propagation();
   bool opt_copy_propagation(bool do_constant_prop = true);
   bool opt_cse_local(bblock_t *block, const vec4_live_variables &live);
   bool opt_cse();
   bool opt_algebraic();
   bool opt_register_coalesce();
   bool eliminate_find_live_channel();
   bool is_dep_ctrl_unsafe(const vec4_instruction *inst);
   void opt_set_dependency_control();
   void opt_schedule_instructions();
   void convert_to_hw_regs();
   void fixup_3src_null_dest();

   bool is_supported_64bit_region(vec4_instruction *inst, unsigned arg);
   bool lower_simd_width();
   bool scalarize_df();
   bool lower_64bit_mad_to_mul_add();
   void apply_logical_swizzle(struct brw_reg *hw_reg,
                              vec4_instruction *inst, int arg);

   vec4_instruction *emit(vec4_instruction *inst);

   vec4_instruction *emit(enum opcode opcode);
   vec4_instruction *emit(enum opcode opcode, const dst_reg &dst);
   vec4_instruction *emit(enum opcode opcode, const dst_reg &dst,
                          const src_reg &src0);
   vec4_instruction *emit(enum opcode opcode, const dst_reg &dst,
                          const src_reg &src0, const src_reg &src1);
   vec4_instruction *emit(enum opcode opcode, const dst_reg &dst,
                          const src_reg &src0, const src_reg &src1,
                          const src_reg &src2);

   vec4_instruction *emit_before(bblock_t *block,
                                 vec4_instruction *inst,
				 vec4_instruction *new_inst);

#define EMIT1(op) vec4_instruction *op(const dst_reg &, const src_reg &);
#define EMIT2(op) vec4_instruction *op(const dst_reg &, const src_reg &, const src_reg &);
#define EMIT3(op) vec4_instruction *op(const dst_reg &, const src_reg &, const src_reg &, const src_reg &);
   EMIT1(MOV)
   EMIT1(NOT)
   EMIT1(RNDD)
   EMIT1(RNDE)
   EMIT1(RNDZ)
   EMIT1(FRC)
   EMIT1(F32TO16)
   EMIT1(F16TO32)
   EMIT2(ADD)
   EMIT2(MUL)
   EMIT2(MACH)
   EMIT2(MAC)
   EMIT2(AND)
   EMIT2(OR)
   EMIT2(XOR)
   EMIT2(DP3)
   EMIT2(DP4)
   EMIT2(DPH)
   EMIT2(SHL)
   EMIT2(SHR)
   EMIT2(ASR)
   vec4_instruction *CMP(dst_reg dst, src_reg src0, src_reg src1,
			 enum brw_conditional_mod condition);
   vec4_instruction *IF(src_reg src0, src_reg src1,
                        enum brw_conditional_mod condition);
   vec4_instruction *IF(enum brw_predicate predicate);
   EMIT1(SCRATCH_READ)
   EMIT2(SCRATCH_WRITE)
   EMIT3(LRP)
   EMIT1(BFREV)
   EMIT3(BFE)
   EMIT2(BFI1)
   EMIT3(BFI2)
   EMIT1(FBH)
   EMIT1(FBL)
   EMIT1(CBIT)
   EMIT1(LZD)
   EMIT3(MAD)
   EMIT2(ADDC)
   EMIT2(SUBB)
   EMIT1(DIM)

#undef EMIT1
#undef EMIT2
#undef EMIT3

   vec4_instruction *emit_minmax(enum brw_conditional_mod conditionalmod, dst_reg dst,
                                 src_reg src0, src_reg src1);

   /**
    * Copy any live channel from \p src to the first channel of the
    * result.
    */
   src_reg emit_uniformize(const src_reg &src);

   /** Fix all float operands of a 3-source instruction. */
   void fix_float_operands(src_reg op[3], nir_alu_instr *instr);

   src_reg fix_3src_operand(const src_reg &src);

   vec4_instruction *emit_math(enum opcode opcode, const dst_reg &dst, const src_reg &src0,
                               const src_reg &src1 = src_reg());

   src_reg fix_math_operand(const src_reg &src);

   void emit_pack_half_2x16(dst_reg dst, src_reg src0);
   void emit_unpack_half_2x16(dst_reg dst, src_reg src0);
   void emit_unpack_unorm_4x8(const dst_reg &dst, src_reg src0);
   void emit_unpack_snorm_4x8(const dst_reg &dst, src_reg src0);
   void emit_pack_unorm_4x8(const dst_reg &dst, const src_reg &src0);
   void emit_pack_snorm_4x8(const dst_reg &dst, const src_reg &src0);

   src_reg emit_mcs_fetch(const glsl_type *coordinate_type, src_reg coordinate,
                          src_reg surface);

   void emit_ndc_computation();
   void emit_psiz_and_flags(dst_reg reg);
   vec4_instruction *emit_generic_urb_slot(dst_reg reg, int varying, int comp);
   virtual void emit_urb_slot(dst_reg reg, int varying);

   src_reg get_scratch_offset(bblock_t *block, vec4_instruction *inst,
			      src_reg *reladdr, int reg_offset);
   void emit_scratch_read(bblock_t *block, vec4_instruction *inst,
			  dst_reg dst,
			  src_reg orig_src,
			  int base_offset);
   void emit_scratch_write(bblock_t *block, vec4_instruction *inst,
			   int base_offset);
   void emit_pull_constant_load_reg(dst_reg dst,
                                    src_reg surf_index,
                                    src_reg offset,
                                    bblock_t *before_block,
                                    vec4_instruction *before_inst);
   src_reg emit_resolve_reladdr(int scratch_loc[], bblock_t *block,
                                vec4_instruction *inst, src_reg src);

   void resolve_ud_negate(src_reg *reg);

   void emit_shader_float_controls_execution_mode();

   bool lower_minmax();

   src_reg get_timestamp();

   virtual void dump_instruction_to_file(const backend_instruction *inst, FILE *file) const;

   bool optimize_predicate(nir_alu_instr *instr, enum brw_predicate *predicate);

   void emit_conversion_from_double(dst_reg dst, src_reg src);
   void emit_conversion_to_double(dst_reg dst, src_reg src);

   vec4_instruction *shuffle_64bit_data(dst_reg dst, src_reg src,
                                        bool for_write,
                                        bool for_scratch = false,
                                        bblock_t *block = NULL,
                                        vec4_instruction *ref = NULL);

   virtual void emit_nir_code();
   virtual void nir_setup_uniforms();
   virtual void nir_emit_impl(nir_function_impl *impl);
   virtual void nir_emit_cf_list(exec_list *list);
   virtual void nir_emit_if(nir_if *if_stmt);
   virtual void nir_emit_loop(nir_loop *loop);
   virtual void nir_emit_block(nir_block *block);
   virtual void nir_emit_instr(nir_instr *instr);
   virtual void nir_emit_load_const(nir_load_const_instr *instr);
   src_reg get_nir_ssbo_intrinsic_index(nir_intrinsic_instr *instr);
   virtual void nir_emit_intrinsic(nir_intrinsic_instr *instr);
   virtual void nir_emit_alu(nir_alu_instr *instr);
   virtual void nir_emit_jump(nir_jump_instr *instr);
   virtual void nir_emit_texture(nir_tex_instr *instr);
   virtual void nir_emit_undef(nir_undef_instr *instr);
   virtual void nir_emit_ssbo_atomic(int op, nir_intrinsic_instr *instr);

   dst_reg get_nir_def(const nir_def &def, enum brw_reg_type type);
   dst_reg get_nir_def(const nir_def &def, nir_alu_type type);
   dst_reg get_nir_def(const nir_def &def);
   src_reg get_nir_src(const nir_src &src, enum brw_reg_type type,
                       unsigned num_components = 4);
   src_reg get_nir_src(const nir_src &src, nir_alu_type type,
                       unsigned num_components = 4);
   src_reg get_nir_src(const nir_src &src,
                       unsigned num_components = 4);
   src_reg get_nir_src_imm(const nir_src &src);
   src_reg get_indirect_offset(nir_intrinsic_instr *instr);

   dst_reg *nir_ssa_values;

protected:
   void emit_vertex();
   void setup_payload_interference(struct ra_graph *g, int first_payload_node,
                                   int reg_node_count);
   virtual void setup_payload() = 0;
   virtual void emit_prolog() = 0;
   virtual void emit_thread_end() = 0;
   virtual void emit_urb_write_header(int mrf) = 0;
   virtual vec4_instruction *emit_urb_write_opcode(bool complete) = 0;
   virtual void gs_emit_vertex(int stream_id);
   virtual void gs_end_primitive();

private:
   /**
    * If true, then register allocation should fail instead of spilling.
    */
   const bool no_spills;

   unsigned last_scratch; /**< measured in 32-byte (register size) units */
};

} /* namespace brw */
#endif /* __cplusplus */

#endif /* BRW_VEC4_H */
