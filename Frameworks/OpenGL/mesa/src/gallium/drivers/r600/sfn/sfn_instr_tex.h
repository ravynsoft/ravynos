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

#ifndef INSTR_TEX_H
#define INSTR_TEX_H

#include "sfn_instr.h"
#include "sfn_shader.h"
#include "sfn_valuefactory.h"

namespace r600 {

class TexInstr : public InstrWithVectorResult {
public:
   enum Opcode {
      ld = FETCH_OP_LD,
      get_resinfo = FETCH_OP_GET_TEXTURE_RESINFO,
      get_nsamples = FETCH_OP_GET_NUMBER_OF_SAMPLES,
      get_tex_lod = FETCH_OP_GET_LOD,
      get_gradient_h = FETCH_OP_GET_GRADIENTS_H,
      get_gradient_v = FETCH_OP_GET_GRADIENTS_V,
      set_offsets = FETCH_OP_SET_TEXTURE_OFFSETS,
      keep_gradients = FETCH_OP_KEEP_GRADIENTS,
      set_gradient_h = FETCH_OP_SET_GRADIENTS_H,
      set_gradient_v = FETCH_OP_SET_GRADIENTS_V,
      sample = FETCH_OP_SAMPLE,
      sample_l = FETCH_OP_SAMPLE_L,
      sample_lb = FETCH_OP_SAMPLE_LB,
      sample_lz = FETCH_OP_SAMPLE_LZ,
      sample_g = FETCH_OP_SAMPLE_G,
      sample_g_lb = FETCH_OP_SAMPLE_G_L,
      gather4 = FETCH_OP_GATHER4,
      gather4_o = FETCH_OP_GATHER4_O,

      sample_c = FETCH_OP_SAMPLE_C,
      sample_c_l = FETCH_OP_SAMPLE_C_L,
      sample_c_lb = FETCH_OP_SAMPLE_C_LB,
      sample_c_lz = FETCH_OP_SAMPLE_C_LZ,
      sample_c_g = FETCH_OP_SAMPLE_C_G,
      sample_c_g_lb = FETCH_OP_SAMPLE_C_G_L,
      gather4_c = FETCH_OP_GATHER4_C,
      gather4_c_o = FETCH_OP_GATHER4_C_O,
      unknown = 255
   };

   enum Flags {
      x_unnormalized,
      y_unnormalized,
      z_unnormalized,
      w_unnormalized,
      grad_fine,
      num_tex_flag
   };

   static constexpr Flags TexFlags[] = {x_unnormalized,
                                        y_unnormalized,
                                        z_unnormalized,
                                        w_unnormalized,
                                        grad_fine,
                                        num_tex_flag};

   struct Inputs {
      Inputs(const nir_tex_instr& instr, ValueFactory& vf);
      const nir_variable *sampler_deref;
      const nir_variable *texture_deref;
      RegisterVec4 coord;
      PVirtualValue bias;
      PVirtualValue comperator;
      PVirtualValue lod;
      RegisterVec4 ddx;
      RegisterVec4 ddy;
      nir_src *offset;
      PVirtualValue gather_comp;
      PVirtualValue ms_index;
      PRegister texture_offset;
      PRegister sampler_offset;
      nir_src *backend1;
      nir_src *backend2;

      RegisterVec4::Swizzle swizzle_from_ncomps(int comps) const;

      Opcode opcode;

   private:
      auto get_opcode(const nir_tex_instr& instr) -> Opcode;
   };

   TexInstr(Opcode op,
            const RegisterVec4& dest,
            const RegisterVec4::Swizzle& dest_swizzle,
            const RegisterVec4& src,
            unsigned resource_id,
            PRegister resource_offs,
            int sampler_id = 0,
            PRegister sampler_offset = nullptr);

   TexInstr(const TexInstr& orig) = delete;
   TexInstr(const TexInstr&& orig) = delete;
   TexInstr& operator=(const TexInstr& orig) = delete;
   TexInstr& operator=(const TexInstr&& orig) = delete;

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   const auto& src() const { return m_src; }
   auto& src() { return m_src; }

   unsigned opcode() const { return m_opcode; }

   unsigned sampler_id() const { return m_sampler.resource_id(); }
   auto sampler_offset() const { return m_sampler.resource_offset(); }
   void set_sampler_offset(PRegister offs) { m_sampler.set_resource_offset(offs); }
   auto sampler_index_mode() const { return m_sampler.resource_index_mode(); }

   void set_offset(unsigned index, int32_t val);
   int get_offset(unsigned index) const;

   void set_inst_mode(int inst_mode) { m_inst_mode = inst_mode; }
   int inst_mode() const { return m_inst_mode; }

   void set_tex_flag(Flags flag) { m_tex_flags.set(flag); }
   bool has_tex_flag(Flags flag) const { return m_tex_flags.test(flag); }

   void set_gather_comp(int cmp);
   bool is_equal_to(const TexInstr& lhs) const;

   static Opcode op_from_string(const std::string& s);
   static Instr::Pointer from_string(std::istream& is, ValueFactory& value_fctory);

   static bool from_nir(nir_tex_instr *tex, Shader& shader);

   uint32_t slots() const override { return 1; };

   auto prepare_instr() const { return m_prepare_instr; }

   bool replace_source(PRegister old_src, PVirtualValue new_src) override;
   void update_indirect_addr(PRegister old_reg, PRegister addr) override;

   uint8_t allowed_src_chan_mask() const override;

private:
   bool do_ready() const override;
   void do_print(std::ostream& os) const override;
   bool propagate_death() override;

   static const char *opname(Opcode code);
   static bool is_gather(Opcode op);

   void read_tex_coord_normalitazion(const std::string& next_token);
   void set_tex_param(const std::string& next_token);

   static auto prepare_source(nir_tex_instr *tex, const Inputs& inputs, Shader& shader)
      -> RegisterVec4;

   static bool emit_buf_txf(nir_tex_instr *tex, Inputs& src, Shader& shader);
   static bool emit_tex_txs(nir_tex_instr *tex,
                            Inputs& src,
                            RegisterVec4::Swizzle dest_swz,
                            Shader& shader);
   static bool emit_tex_lod(nir_tex_instr *tex, Inputs& src, Shader& shader);
   static bool
   emit_tex_texture_samples(nir_tex_instr *instr, Inputs& src, Shader& shader);
   static bool emit_lowered_tex(nir_tex_instr *instr, Inputs& src, Shader& shader);
   static void emit_set_gradients(
      nir_tex_instr *tex, int texture_id, Inputs& src, TexInstr *irt, Shader& shader);
   static void emit_set_offsets(
      nir_tex_instr *tex, int texture_id, Inputs& src, TexInstr *irt, Shader& shader);

   bool set_coord_offsets(nir_src *offset);
   void set_rect_coordinate_flags(nir_tex_instr *instr);
   void add_prepare_instr(TexInstr *ir) { m_prepare_instr.push_back(ir); };
   void forward_set_blockid(int id, int index) override;


   Opcode m_opcode;

   RegisterVec4 m_src;
   std::bitset<num_tex_flag> m_tex_flags;
   int m_coord_offset[3];
   int m_inst_mode;

   static const std::map<Opcode, std::string> s_opcode_map;
   std::list<TexInstr *, Allocator<TexInstr *>> m_prepare_instr;

   Resource m_sampler;
};

bool
r600_nir_lower_tex_to_backend(nir_shader *shader, amd_gfx_level chip_class);

} // namespace r600

#endif // INSTR_TEX_H
