/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2022 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SFN_SHADER_H
#define SFN_SHADER_H

#include "amd_family.h"
#include "compiler/shader_enums.h"
#include "gallium/drivers/r600/r600_shader.h"
#include "sfn_instr.h"
#include "sfn_instr_controlflow.h"
#include "sfn_instrfactory.h"
#include "sfn_liverangeevaluator.h"

#include <bitset>
#include <memory>
#include <stack>
#include <vector>

struct nir_shader;
struct nir_cf_node;
struct nir_if;
struct nir_block;
struct nir_instr;

namespace r600 {

class ShaderIO {
public:
   void print(std::ostream& os) const;

   int location() const { return m_location; }
   void set_location(int location) { m_location = location; }

   gl_varying_slot varying_slot() const { return m_varying_slot; }
   void set_varying_slot(gl_varying_slot varying_slot) { m_varying_slot = varying_slot; }

   bool no_varying() const { return m_no_varying; }
   void set_no_varying(bool no_varying) { m_no_varying = no_varying; }

   int spi_sid() const;

   void set_gpr(int gpr) { m_gpr = gpr; }
   int gpr() const { return m_gpr; }

protected:
   ShaderIO(const char *type, int loc, gl_varying_slot varying_slot = NUM_TOTAL_VARYING_SLOTS);

private:
   virtual void do_print(std::ostream& os) const = 0;

   const char *m_type;
   int m_location{-1};
   gl_varying_slot m_varying_slot{NUM_TOTAL_VARYING_SLOTS};
   bool m_no_varying{false};
   int m_gpr{0};
};

class ShaderOutput : public ShaderIO {
public:
   ShaderOutput();
   ShaderOutput(int location, int writemask,
                gl_varying_slot varying_slot = NUM_TOTAL_VARYING_SLOTS);

   gl_frag_result frag_result() const { return m_frag_result; }
   void set_frag_result(gl_frag_result frag_result) { m_frag_result = frag_result; }

   int writemask() const { return m_writemask; }
   void set_writemask(int writemask) { m_writemask = writemask; }

   int export_param() const { return m_export_param; }
   void set_export_param(int export_param) { m_export_param = export_param; }

private:
   void do_print(std::ostream& os) const override;

   gl_frag_result m_frag_result{static_cast<gl_frag_result>(FRAG_RESULT_MAX)};
   int m_writemask{0};
   int m_export_param{-1};
};

class ShaderInput : public ShaderIO {
public:
   ShaderInput();
   ShaderInput(int location, gl_varying_slot varying_slot = NUM_TOTAL_VARYING_SLOTS);

   gl_system_value system_value() const { return m_system_value; }
   void set_system_value(gl_system_value system_value) { m_system_value = system_value; }

   void set_interpolator(int interp, int interp_loc, bool uses_interpolate_at_centroid);
   void set_uses_interpolate_at_centroid();
   void set_need_lds_pos() { m_need_lds_pos = true; }
   int ij_index() const { return m_ij_index; }

   int interpolator() const { return m_interpolator; }
   int interpolate_loc() const { return m_interpolate_loc; }
   bool need_lds_pos() const { return m_need_lds_pos; }
   int lds_pos() const { return m_lds_pos; }
   void set_lds_pos(int pos) { m_lds_pos = pos; }

   int ring_offset() const { return m_ring_offset; }
   void set_ring_offset(int offs) { m_ring_offset = offs; }
   bool uses_interpolate_at_centroid() const { return m_uses_interpolate_at_centroid; }

private:
   void do_print(std::ostream& os) const override;

   gl_system_value m_system_value{SYSTEM_VALUE_MAX};
   int m_interpolator{0};
   int m_interpolate_loc{0};
   int m_ij_index{0};
   bool m_uses_interpolate_at_centroid{false};
   bool m_need_lds_pos{false};
   int m_lds_pos{0};
   int m_ring_offset{0};
};

class Shader : public Allocate {
public:
   using InputIterator = std::map<int, ShaderInput>::iterator;
   using OutputIterator = std::map<int, ShaderOutput>::iterator;

   using ShaderBlocks = std::list<Block::Pointer, Allocator<Block::Pointer>>;

   Shader(const Shader& orig) = delete;

   virtual ~Shader() {}

   auto shader_id() const {return m_shader_id;}
   // Needed for testing
   void reset_shader_id() {m_shader_id = 0;}

   bool add_info_from_string(std::istream& is);

   static Shader *translate_from_nir(nir_shader *nir,
                                     const pipe_stream_output_info *so_info,
                                     r600_shader *gs_shader,
                                     const r600_shader_key& key,
                                     r600_chip_class chip_class,
                                     radeon_family family);

   bool process(nir_shader *nir);

   bool process_cf_node(nir_cf_node *node);
   bool process_if(nir_if *node);
   bool process_loop(nir_loop *node);
   bool process_block(nir_block *node);
   bool process_instr(nir_instr *instr);
   void emit_instruction(PInst instr);
   bool emit_atomic_local_shared(nir_intrinsic_instr *instr);

   void print(std::ostream& os) const;
   void print_header(std::ostream& os) const;

   bool process_intrinsic(nir_intrinsic_instr *intr);

   virtual bool load_input(nir_intrinsic_instr *intr) = 0;
   virtual bool store_output(nir_intrinsic_instr *intr) = 0;

   bool load_ubo(nir_intrinsic_instr *intr);

   ValueFactory& value_factory();

   void add_output(const ShaderOutput& output) { m_outputs[output.location()] = output; }

   void add_input(const ShaderInput& input) { m_inputs[input.location()] = input; }

   void set_input_gpr(int driver_lcation, int gpr);

   InputIterator find_input(int location) { return m_inputs.find(location); }

   InputIterator input_not_found() { return m_inputs.end(); }

   OutputIterator find_output(int location);
   OutputIterator output_not_found() { return m_outputs.end(); }

   ShaderBlocks& func() { return m_root; }
   void reset_function(ShaderBlocks& new_root);

   void emit_instruction_from_string(const std::string& s);

   void set_info(nir_shader *nir);
   void get_shader_info(r600_shader *sh_info);

   r600_chip_class chip_class() const { return m_chip_class; }
   void set_chip_class(r600_chip_class cls) { m_chip_class = cls; }

   radeon_family chip_family() const { return m_chip_family; }
   void set_chip_family(radeon_family family) { m_chip_family = family; }

   void start_new_block(int nesting_depth);

   const ShaderOutput& output(int base) const;

   LiveRangeMap prepare_live_range_map();

   void set_last_txd(Instr *txd) { m_last_txd = txd; }
   Instr *last_txd() { return m_last_txd; }

   // Needed for keeping the memory access in order
   void chain_scratch_read(Instr *instr);
   void chain_ssbo_read(Instr *instr);

   virtual uint32_t enabled_stream_buffers_mask() const { return 0; }

   size_t noutputs() const { return m_outputs.size(); }
   size_t ninputs() const { return m_inputs.size(); }

   enum Flags {
      sh_indirect_const_file,
      sh_needs_scratch_space,
      sh_needs_sbo_ret_address,
      sh_uses_atomics,
      sh_uses_images,
      sh_uses_tex_buffer,
      sh_writes_memory,
      sh_txs_cube_array_comp,
      sh_indirect_atomic,
      sh_mem_barrier,
      sh_legacy_math_rules,
      sh_disble_sb,
      sh_flags_count
   };

   void set_flag(Flags f) { m_flags.set(f); }
   bool has_flag(Flags f) const { return m_flags.test(f); }

   int atomic_file_count() const { return m_atomic_file_count; }

   PRegister atomic_update();
   int remap_atomic_base(int base);
   auto evaluate_resource_offset(nir_intrinsic_instr *instr, int src_id)
      -> std::pair<int, PRegister>;
   int ssbo_image_offset() const { return m_ssbo_image_offset; }
   PRegister rat_return_address()
   {
      assert(m_rat_return_address);
      return m_rat_return_address;
   }

   PRegister emit_load_to_register(PVirtualValue src);

   virtual unsigned image_size_const_offset() { return 0;}

   auto required_registers() const { return m_required_registers;}

protected:
   enum ESlots {
      es_face,
      es_instanceid,
      es_invocation_id,
      es_patch_id,
      es_pos,
      es_rel_patch_id,
      es_sample_mask_in,
      es_sample_id,
      es_sample_pos,
      es_tess_factor_base,
      es_vertexid,
      es_tess_coord,
      es_primitive_id,
      es_helper_invocation,
      es_last
   };

   std::bitset<es_last> m_sv_values;

   Shader(const char *type_id, unsigned atomic_base);

   const ShaderInput& input(int base) const;

   bool emit_simple_mov(nir_def& def, int chan, PVirtualValue src, Pin pin = pin_free);

   template <typename T>
   using IOMap = std::map<int, T, std::less<int>, Allocator<std::pair<const int, T>>>;

   IOMap<ShaderInput>& inputs() { return m_inputs; }

private:
   virtual bool process_stage_intrinsic(nir_intrinsic_instr *intr) = 0;

   bool allocate_registers_from_string(std::istream& is, Pin pin);
   bool allocate_arrays_from_string(std::istream& is);

   bool read_chipclass(std::istream& is);
   bool read_family(std::istream& is);

   bool scan_shader(const nir_function *impl);
   bool scan_uniforms(nir_variable *uniform);
   void allocate_reserved_registers();

   virtual int do_allocate_reserved_registers() = 0;

   bool scan_instruction(nir_instr *instr);
   virtual bool do_scan_instruction(nir_instr *instr) = 0;

   void print_properties(std::ostream& os) const;
   virtual void do_print_properties(std::ostream& os) const = 0;

   bool read_output(std::istream& is);
   bool read_input(std::istream& is);
   virtual bool read_prop(std::istream& is) = 0;

   bool emit_control_flow(ControlFlowInstr::CFType type);
   bool emit_store_scratch(nir_intrinsic_instr *intr);
   bool emit_load_scratch(nir_intrinsic_instr *intr);
   bool emit_load_global(nir_intrinsic_instr *intr);
   bool emit_local_store(nir_intrinsic_instr *intr);
   bool emit_local_load(nir_intrinsic_instr *instr);
   bool emit_load_tcs_param_base(nir_intrinsic_instr *instr, int offset);
   bool emit_group_barrier(nir_intrinsic_instr *intr);
   bool emit_shader_clock(nir_intrinsic_instr *instr);
   bool emit_wait_ack();
   bool emit_barrier(nir_intrinsic_instr *instr);
   bool emit_load_reg(nir_intrinsic_instr *intr);
   bool emit_load_reg_indirect(nir_intrinsic_instr *intr);
   bool emit_store_reg(nir_intrinsic_instr *intr);
   bool emit_store_reg_indirect(nir_intrinsic_instr *intr);

   bool equal_to(const Shader& other) const;
   void finalize();
   virtual void do_finalize();

   virtual void do_get_shader_info(r600_shader *sh_info);

   ShaderBlocks m_root;
   Block::Pointer m_current_block;

   InstrFactory *m_instr_factory;
   const char *m_type_id;

   IOMap<ShaderOutput> m_outputs;
   IOMap<ShaderInput> m_inputs;
   r600_chip_class m_chip_class;
   radeon_family m_chip_family{CHIP_CEDAR};

   int m_scratch_size;
   int m_next_block;
   bool m_indirect_const_file{false};

   Instr *m_last_txd{nullptr};

   uint32_t m_indirect_files{0};
   std::bitset<sh_flags_count> m_flags;
   uint32_t nhwatomic_ranges{0};
   std::vector<r600_shader_atomic, Allocator<r600_shader_atomic>> m_atomics;

   uint32_t m_nhwatomic{0};
   uint32_t m_atomic_base{0};
   uint32_t m_next_hwatomic_loc{0};
   std::unordered_map<int, int,
                      std::hash<int>,  std::equal_to<int>,
                      Allocator<std::pair<const int, int>>> m_atomic_base_map;
   uint32_t m_atomic_file_count{0};
   PRegister m_atomic_update{nullptr};
   PRegister m_rat_return_address{nullptr};

   int32_t m_ssbo_image_offset{0};
   uint32_t m_nloops{0};
   uint32_t m_required_registers{0};

   int64_t m_shader_id;
   static int64_t s_next_shader_id;

   class InstructionChain : public InstrVisitor {
   public:
      void visit(AluGroup *instr) override { (void)instr; }
      void visit(TexInstr *instr) override { (void)instr; }
      void visit(ExportInstr *instr) override { (void)instr; }
      void visit(FetchInstr *instr) override { (void)instr; }
      void visit(Block *instr) override { (void)instr; }
      void visit(ControlFlowInstr *instr) override { (void)instr; }
      void visit(IfInstr *instr) override { (void)instr; }
      void visit(StreamOutInstr *instr) override { (void)instr; }
      void visit(MemRingOutInstr *instr) override { (void)instr; }
      void visit(EmitVertexInstr *instr) override { (void)instr; }
      void visit(WriteTFInstr *instr) override { (void)instr; }
      void visit(LDSAtomicInstr *instr) override { (void)instr; }
      void visit(LDSReadInstr *instr) override { (void)instr; }

      void visit(AluInstr *instr) override;
      void visit(ScratchIOInstr *instr) override;
      void visit(GDSInstr *instr) override;
      void visit(RatInstr *instr) override;

      void apply(Instr *current, Instr **last);

      Shader *this_shader{nullptr};
      Instr *last_scratch_instr{nullptr};
      Instr *last_gds_instr{nullptr};
      Instr *last_ssbo_instr{nullptr};
      Instr *last_kill_instr{nullptr};
      std::unordered_map<int, Instr * > last_alu_with_indirect_reg;
      bool prepare_mem_barrier{false};
   };

   InstructionChain m_chain_instr;
   std::list<Instr *, Allocator<Instr *>> m_loops;
   int m_control_flow_depth{0};
   std::list<nir_intrinsic_instr*> m_register_allocations;
};

} // namespace r600

#endif // SHADER_H
