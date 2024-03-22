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

#ifndef SFN_TESS_SHADER_H
#define SFN_TESS_SHADER_H

#include "sfn_shader_vs.h"

namespace r600 {

class VertexExportStage;

class TCSShader : public Shader {
public:
   TCSShader(const r600_shader_key& key);

private:
   bool do_scan_instruction(nir_instr *instr) override;
   int do_allocate_reserved_registers() override;

   bool process_stage_intrinsic(nir_intrinsic_instr *intr) override;
   void do_get_shader_info(r600_shader *sh_info) override;
   bool store_tess_factor(nir_intrinsic_instr *instr);

   bool load_input(UNUSED nir_intrinsic_instr *intr) override
   {
      unreachable("load_input must be lowered in TCS");
   };
   bool store_output(UNUSED nir_intrinsic_instr *intr) override
   {
      unreachable("load_output must be lowered in TCS");
   };

   bool read_prop(std::istream& is) override;
   void do_print_properties(std::ostream& os) const override;

   PRegister m_tess_factor_base;
   PRegister m_rel_patch_id;
   PRegister m_invocation_id;
   PRegister m_primitive_id;

   unsigned m_tcs_prim_mode{0};
};

class TESShader : public VertexStageShader {
public:
   TESShader(const pipe_stream_output_info *so_info,
             const r600_shader *gs_shader,
             const r600_shader_key& key);

private:
   bool do_scan_instruction(nir_instr *instr) override;
   int do_allocate_reserved_registers() override;

   bool process_stage_intrinsic(nir_intrinsic_instr *intr) override;
   void do_get_shader_info(r600_shader *sh_info) override;

   bool load_input(UNUSED nir_intrinsic_instr *intr) override
   {
      unreachable("load_input must be lowered in TES");
   };
   bool store_output(UNUSED nir_intrinsic_instr *intr) override
   {
      unreachable("load_output must be lowered in TES");
   };

   bool read_prop(std::istream& is) override;
   void do_print_properties(std::ostream& os) const override;

   void do_finalize() override;

   PRegister m_tess_coord[2] = {nullptr, nullptr};
   PRegister m_rel_patch_id{nullptr};
   PRegister m_primitive_id{nullptr};

   VertexExportStage *m_export_processor{nullptr};

   int m_tcs_vertices_out{0};
   bool m_vs_as_gs_a{false};
   bool m_tes_as_es{false};
};

} // namespace r600

#endif // TCS_H
