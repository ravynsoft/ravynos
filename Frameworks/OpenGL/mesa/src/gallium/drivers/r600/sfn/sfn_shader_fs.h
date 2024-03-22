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

#ifndef R600_SFN_SHADER_FS_H
#define R600_SFN_SHADER_FS_H

#include "sfn_shader.h"

namespace r600 {

class FragmentShader : public Shader {
public:
   FragmentShader(const r600_shader_key& key);
   bool load_input(nir_intrinsic_instr *intr) override;
   bool store_output(nir_intrinsic_instr *intr) override;

   bool process_stage_intrinsic(nir_intrinsic_instr *intr) override;

   unsigned image_size_const_offset() override { return m_image_size_const_offset;}

protected:
   static const int s_max_interpolators = 6;
   bool interpolators_used(int i) const { return m_interpolators_used.test(i); }

private:
   bool load_interpolated_input(nir_intrinsic_instr *intr);

   virtual int allocate_interpolators_or_inputs() = 0;
   virtual bool load_input_hw(nir_intrinsic_instr *intr) = 0;
   virtual bool process_stage_intrinsic_hw(nir_intrinsic_instr *intr) = 0;
   virtual bool load_interpolated_input_hw(nir_intrinsic_instr *intr) = 0;

   bool do_scan_instruction(nir_instr *instr) override;
   int do_allocate_reserved_registers() override;

   void do_get_shader_info(r600_shader *sh_info) override;

   bool scan_input(nir_intrinsic_instr *instr, int index_src_id);

   bool emit_export_pixel(nir_intrinsic_instr& intr);
   bool emit_load_sample_mask_in(nir_intrinsic_instr *instr);
   bool emit_load_helper_invocation(nir_intrinsic_instr *instr);
   bool emit_load_sample_pos(nir_intrinsic_instr *instr);
   void do_finalize() override;

   bool read_prop(std::istream& is) override;

   void do_print_properties(std::ostream& os) const override;

   bool m_dual_source_blend{false};
   unsigned m_max_color_exports{0};
   unsigned m_export_highest{0};
   unsigned m_num_color_exports{0};
   unsigned m_color_export_mask{0};
   unsigned m_color_export_written_mask{0};
   ExportInstr *m_last_pixel_export{nullptr};

   std::bitset<s_max_interpolators> m_interpolators_used;
   RegisterVec4 m_pos_input;
   Register *m_face_input{nullptr};
   bool m_fs_write_all{false};
   bool m_uses_discard{false};
   bool m_gs_prim_id_input{false};
   Register *m_sample_id_reg{nullptr};
   Register *m_sample_mask_reg{nullptr};
   Register *m_helper_invocation{nullptr};
   int m_nsys_inputs{0};
   bool m_apply_sample_mask{false};
   int m_rat_base{0};
   int m_pos_driver_loc{0};
   int m_face_driver_loc{0};
   int m_image_size_const_offset{0};
};

class FragmentShaderR600 : public FragmentShader {
public:
   using FragmentShader::FragmentShader;

private:
   int allocate_interpolators_or_inputs() override;
   bool load_input_hw(nir_intrinsic_instr *intr) override;
   bool process_stage_intrinsic_hw(nir_intrinsic_instr *intr) override;
   bool load_interpolated_input_hw(nir_intrinsic_instr *intr) override;

   IOMap<RegisterVec4> m_interpolated_inputs;
};

class FragmentShaderEG : public FragmentShader {
public:
   using FragmentShader::FragmentShader;

private:
   class Interpolator {
   public:
      Interpolator();
      bool enabled : 1;
      unsigned ij_index : 4;
      PRegister i;
      PRegister j;
   };

   struct InterpolateParams {
      PVirtualValue i, j;
      int base;
   };

   int allocate_interpolators_or_inputs() override;
   bool load_input_hw(nir_intrinsic_instr *intr) override;
   bool process_stage_intrinsic_hw(nir_intrinsic_instr *intr) override;
   bool load_interpolated_input_hw(nir_intrinsic_instr *intr) override;

   bool load_barycentric_pixel(nir_intrinsic_instr *intr);
   bool load_barycentric_at_sample(nir_intrinsic_instr *instr);
   bool load_barycentric_at_offset(nir_intrinsic_instr *instr);
   bool load_interpolated(RegisterVec4& dest,
                          const InterpolateParams& params,
                          int num_dest_comp,
                          int start_comp);

   bool load_interpolated_one_comp(RegisterVec4& dest,
                                   const InterpolateParams& params,
                                   EAluOp op);
   bool load_interpolated_two_comp(RegisterVec4& dest,
                                   const InterpolateParams& params,
                                   EAluOp op,
                                   int writemask);
   bool load_interpolated_two_comp_for_one(RegisterVec4& dest,
                                           const InterpolateParams& params,
                                           EAluOp op,
                                           int dest_slot);

   std::array<Interpolator, s_max_interpolators> m_interpolator;
};

} // namespace r600

#endif
