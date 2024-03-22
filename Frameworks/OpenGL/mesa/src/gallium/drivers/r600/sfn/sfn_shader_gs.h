#ifndef SFN_GEOMETRYSHADER_H
#define SFN_GEOMETRYSHADER_H

#include "sfn_instr_export.h"
#include "sfn_shader.h"

namespace r600 {

class GeometryShader : public Shader {
public:
   GeometryShader(const r600_shader_key& key);

private:
   bool do_scan_instruction(nir_instr *instr) override;
   int do_allocate_reserved_registers() override;

   bool process_stage_intrinsic(nir_intrinsic_instr *intr) override;

   bool process_store_output(nir_intrinsic_instr *intr);
   bool process_load_input(nir_intrinsic_instr *intr);

   void do_finalize() override;

   void do_get_shader_info(r600_shader *sh_info) override;

   bool read_prop(std::istream& is) override;
   void do_print_properties(std::ostream& os) const override;

   void emit_adj_fix();

   bool emit_load_per_vertex_input(nir_intrinsic_instr *instr);

   bool load_input(UNUSED nir_intrinsic_instr *intr) override
   {
      unreachable("load_input must be lowered in GS");
   };
   bool store_output(nir_intrinsic_instr *instr) override;
   bool emit_vertex(nir_intrinsic_instr *instr, bool cut);

   std::array<PRegister, 6> m_per_vertex_offsets{nullptr};
   PRegister m_primitive_id{nullptr};
   PRegister m_invocation_id{nullptr};
   std::array<PRegister, 4> m_export_base{nullptr};

   unsigned m_ring_item_sizes[4]{0};

   bool m_tri_strip_adj_fix{false};
   bool m_first_vertex_emitted{false};
   int m_offset{0};
   int m_next_input_ring_offset{0};
   int m_cc_dist_mask{0};
   int m_clip_dist_write{0};
   int m_cur_ring_output{0};
   bool m_gs_tri_strip_adj_fix{false};
   uint64_t m_input_mask{0};
   unsigned m_noutputs{0};
   bool m_out_viewport{false};
   bool m_out_misc_write{false};

   std::map<int, MemRingOutInstr *> m_streamout_data;
};

} // namespace r600

#endif // GEOMETRYSHADER_H
