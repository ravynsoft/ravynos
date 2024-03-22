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

#ifndef SFN_SHADER_VS_H
#define SFN_SHADER_VS_H

#include "sfn_shader.h"

namespace r600 {

class VertexStageShader : public Shader {
protected:
   using Shader::Shader;

public:
   PRegister primitive_id() const { return m_primitive_id; }
   void set_primitive_id(PRegister prim_id) { m_primitive_id = prim_id; }

   void combine_enabled_stream_buffers_mask(uint32_t mask);
   uint32_t enabled_stream_buffers_mask() const override;

private:
   PRegister m_primitive_id{nullptr};
   uint32_t m_enabled_stream_buffers_mask{0};
};

class VertexExportStage : public Allocate {
public:
   VertexExportStage(VertexStageShader *parent);

   bool store_output(nir_intrinsic_instr& intr);

   virtual void finalize() = 0;

   virtual void get_shader_info(r600_shader *sh_info) const = 0;

protected:
   struct store_loc {
      unsigned frac;
      unsigned location;
      unsigned driver_location;
      int data_loc;
   };

   virtual bool do_store_output(const store_loc& store_info,
                                nir_intrinsic_instr& intr) = 0;

   VertexStageShader *m_parent;

private:
};

class VertexExportForFs : public VertexExportStage {
   friend VertexExportStage;

public:
   VertexExportForFs(VertexStageShader *parent,
                     const pipe_stream_output_info *so_info,
                     const r600_shader_key& key);

   void finalize() override;

   void get_shader_info(r600_shader *sh_info) const override;

private:
   bool do_store_output(const store_loc& store_info, nir_intrinsic_instr& intr) override;

   bool emit_varying_pos(const store_loc& store_info,
                         nir_intrinsic_instr& intr,
                         std::array<uint8_t, 4> *swizzle_override = nullptr);
   bool emit_varying_param(const store_loc& store_info, nir_intrinsic_instr& intr);

   bool emit_clip_vertices(const store_loc& store_info, const nir_intrinsic_instr& instr);

   bool emit_stream(int stream);

   const RegisterVec4 *output_register(int loc) const;

   ExportInstr *m_last_param_export{nullptr};
   ExportInstr *m_last_pos_export{nullptr};

   int m_num_clip_dist{0};
   int m_next_param{0};
   uint8_t m_cc_dist_mask{0};
   uint8_t m_clip_dist_write{0};
   int m_cur_clip_pos{1};
   bool m_writes_point_size{false};
   bool m_out_misc_write{false};
   bool m_vs_out_layer{false};
   bool m_vs_as_gs_a{false};
   bool m_out_edgeflag{false};
   bool m_out_viewport{false};
   bool m_out_point_size{false};
   RegisterVec4 m_clip_vertex;

   const pipe_stream_output_info *m_so_info{nullptr};

   template <typename Key, typename T>
   using unordered_map_alloc = std::unordered_map<Key,
                                                  T,
                                                  std::hash<Key>,
                                                  std::equal_to<Key>,
                                                  Allocator<std::pair<const Key, T>>>;

   unordered_map_alloc<int, RegisterVec4 *> m_output_registers;
};

class VertexExportForGS : public VertexExportStage {
public:
   VertexExportForGS(VertexStageShader *parent, const r600_shader *gs_shader);
   void finalize() override;

   void get_shader_info(r600_shader *sh_info) const override;

private:
   bool do_store_output(const store_loc& store_info, nir_intrinsic_instr& intr) override;
   unsigned m_num_clip_dist{0};
   bool m_vs_out_viewport{false};
   bool m_vs_out_misc_write{false};

   const r600_shader *m_gs_shader;
};

class VertexExportForTCS : public VertexExportStage {
public:
   VertexExportForTCS(VertexStageShader *parent);
   void finalize() override;
   void get_shader_info(r600_shader *sh_info) const override;

private:
   bool do_store_output(const store_loc& store_info, nir_intrinsic_instr& intr) override;
};

class VertexShader : public VertexStageShader {
public:
   VertexShader(const pipe_stream_output_info *so_info,
                r600_shader *gs_shader,
                const r600_shader_key& key);

   bool load_input(nir_intrinsic_instr *intr) override;
   bool store_output(nir_intrinsic_instr *intr) override;

   bool process_stage_intrinsic(nir_intrinsic_instr *intr) override;

private:
   bool do_scan_instruction(nir_instr *instr) override;
   int do_allocate_reserved_registers() override;

   void do_finalize() override;

   bool read_prop(std::istream& is) override;

   void do_print_properties(std::ostream& os) const override;
   void do_get_shader_info(r600_shader *sh_info) override;

   VertexExportStage *m_export_stage{nullptr};
   int m_last_vertex_attribute_register{0};
   PRegister m_vertex_id{nullptr};
   PRegister m_instance_id{nullptr};
   PRegister m_rel_vertex_id{nullptr};
   bool m_vs_as_gs_a;
};

} // namespace r600

#endif
