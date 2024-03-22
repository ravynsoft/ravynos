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

#include "sfn_instr_tex.h"

#include "nir_builder.h"
#include "sfn_debug.h"
#include "sfn_instr_alu.h"
#include "sfn_instr_fetch.h"
#include "sfn_nir.h"

namespace r600 {

using std::string;

TexInstr::TexInstr(Opcode op,
                   const RegisterVec4& dest,
                   const RegisterVec4::Swizzle& dest_swizzle,
                   const RegisterVec4& src,
                   unsigned resource_id,
                   PRegister resource_offs,
                   int sampler_id, PRegister sampler_offset):
    InstrWithVectorResult(dest, dest_swizzle, resource_id, resource_offs),
    m_opcode(op),
    m_src(src),
    m_inst_mode(0),
    m_sampler(this, sampler_id, sampler_offset)
{
   memset(m_coord_offset, 0, sizeof(m_coord_offset));
   m_src.add_use(this);
}

void
TexInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
TexInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

void
TexInstr::set_offset(unsigned index, int32_t val)
{
   assert(index < 3);
   m_coord_offset[index] = val;
}

int
TexInstr::get_offset(unsigned index) const
{
   assert(index < 3);
   return m_coord_offset[index] << 1;
}

void
TexInstr::set_gather_comp(int cmp)
{
   m_inst_mode = cmp;
}

bool
TexInstr::is_equal_to(const TexInstr& lhs) const
{
   if (m_opcode != lhs.m_opcode)
      return false;

   if (!comp_dest(lhs.dst(), lhs.all_dest_swizzle()))
      return false;

   if (m_src != lhs.m_src)
      return false;

   if (resource_offset() && lhs.resource_offset()) {
      if (!resource_offset()->equal_to(*lhs.resource_offset()))
         return false;
   } else if ((resource_offset() && !lhs.resource_offset()) ||
              (!resource_offset() && lhs.resource_offset()))
      return false;

   if (sampler_offset() && lhs.sampler_offset()) {
      if (!sampler_offset()->equal_to(*lhs.sampler_offset()))
         return false;
   } else if ((sampler_offset() && !lhs.sampler_offset()) ||
              (!sampler_offset() && lhs.sampler_offset()))
      return false;

   if (m_tex_flags != lhs.m_tex_flags)
      return false;

   for (int i = 0; i < 3; ++i) {
      if (m_coord_offset[i] != lhs.m_coord_offset[i])
         return false;
   }

   return m_inst_mode == lhs.m_inst_mode &&
         resource_id() == lhs.resource_id() &&
         resource_index_mode() == lhs.resource_index_mode() &&
         sampler_id() == lhs.sampler_id() &&
         sampler_index_mode() == lhs.sampler_index_mode();
}

bool
TexInstr::propagate_death()
{
   m_src.del_use(this);
   return true;
}

void TexInstr::forward_set_blockid(int id, int index)
{
   for (auto p : m_prepare_instr)
      p->set_blockid(id, index);
}

bool
TexInstr::do_ready() const
{
   for (auto p : m_prepare_instr)
      if (!p->ready())
         return false;

   for (auto p : required_instr())
      if (!p->is_scheduled() && !p->is_dead()) {
         return false;
      }

   if (resource_offset() && !resource_offset()->ready(block_id(), index()))
      return false;
   return m_src.ready(block_id(), index());
}

void
TexInstr::do_print(std::ostream& os) const
{

   for (auto& p : prepare_instr()) {
      os << *p << "\n";
   }

   os << "TEX " << opname(m_opcode) << " ";
   print_dest(os);

   os << " : ";
   m_src.print(os);

   os << " RID:" << resource_id();
   if (resource_offset())
      os << " RO:" << *resource_offset();

   os << " SID:" << sampler_id();
   if (sampler_offset())
      os << " SO:" << *sampler_offset();

   if (m_coord_offset[0])
      os << " OX:" << m_coord_offset[0];
   if (m_coord_offset[1])
      os << " OY:" << m_coord_offset[1];
   if (m_coord_offset[2])
      os << " OZ:" << m_coord_offset[2];

   if (m_inst_mode || is_gather(m_opcode))
      os << " MODE:" << m_inst_mode;

   os << " ";
   os << (m_tex_flags.test(x_unnormalized) ? "U" : "N");
   os << (m_tex_flags.test(y_unnormalized) ? "U" : "N");
   os << (m_tex_flags.test(z_unnormalized) ? "U" : "N");
   os << (m_tex_flags.test(w_unnormalized) ? "U" : "N");
}

const char *
TexInstr::opname(Opcode op)
{
   switch (op) {
   case ld:
      return "LD";
   case get_resinfo:
      return "GET_TEXTURE_RESINFO";
   case get_nsamples:
      return "GET_NUMBER_OF_SAMPLES";
   case get_tex_lod:
      return "GET_LOD";
   case get_gradient_h:
      return "GET_GRADIENTS_H";
   case get_gradient_v:
      return "GET_GRADIENTS_V";
   case set_offsets:
      return "SET_TEXTURE_OFFSETS";
   case keep_gradients:
      return "KEEP_GRADIENTS";
   case set_gradient_h:
      return "SET_GRADIENTS_H";
   case set_gradient_v:
      return "SET_GRADIENTS_V";
   case sample:
      return "SAMPLE";
   case sample_l:
      return "SAMPLE_L";
   case sample_lb:
      return "SAMPLE_LB";
   case sample_lz:
      return "SAMPLE_LZ";
   case sample_g:
      return "SAMPLE_G";
   case sample_g_lb:
      return "SAMPLE_G_L";
   case gather4:
      return "GATHER4";
   case gather4_o:
      return "GATHER4_O";
   case sample_c:
      return "SAMPLE_C";
   case sample_c_l:
      return "SAMPLE_C_L";
   case sample_c_lb:
      return "SAMPLE_C_LB";
   case sample_c_lz:
      return "SAMPLE_C_LZ";
   case sample_c_g:
      return "SAMPLE_C_G";
   case sample_c_g_lb:
      return "SAMPLE_C_G_L";
   case gather4_c:
      return "GATHER4_C";
   case gather4_c_o:
      return "OP_GATHER4_C_O";
   default:
      return "ERROR";
   }
}

const std::map<TexInstr::Opcode, std::string> TexInstr::s_opcode_map = {
   {ld,             "LD"                   },
   {get_resinfo,    "GET_TEXTURE_RESINFO"  },
   {get_nsamples,   "GET_NUMBER_OF_SAMPLES"},
   {get_tex_lod,    "GET_LOD"              },
   {get_gradient_h, "GET_GRADIENTS_H"      },
   {get_gradient_v, "GET_GRADIENTS_V"      },
   {set_offsets,    "SET_TEXTURE_OFFSETS"  },
   {keep_gradients, "KEEP_GRADIENTS"       },
   {set_gradient_h, "SET_GRADIENTS_H"      },
   {set_gradient_v, "SET_GRADIENTS_V"      },
   {sample,         "SAMPLE"               },
   {sample_l,       "SAMPLE_L"             },
   {sample_lb,      "SAMPLE_LB"            },
   {sample_lz,      "SAMPLE_LZ"            },
   {sample_g,       "SAMPLE_G"             },
   {sample_g_lb,    "SAMPLE_G_L"           },
   {gather4,        "GATHER4"              },
   {gather4_o,      "GATHER4_O"            },
   {sample_c,       "SAMPLE_C"             },
   {sample_c_l,     "SAMPLE_C_L"           },
   {sample_c_lb,    "SAMPLE_C_LB"          },
   {sample_c_lz,    "SAMPLE_C_LZ"          },
   {sample_c_g,     "SAMPLE_C_G"           },
   {sample_c_g_lb,  "SAMPLE_C_G_L"         },
   {gather4_c,      "GATHER4_C"            },
   {gather4_c_o,    "OP_GATHER4_C_O"       },
   {unknown,        "ERROR"                }
};

bool
TexInstr::is_gather(Opcode op)
{
   return op == gather4 || op == gather4_c || op == gather4_o || op == gather4_c_o;
}

TexInstr::Opcode
TexInstr::op_from_string(const std::string& s)
{
   for (auto& [op, str] : s_opcode_map) {
      if (s == str)
         return op;
   }
   return unknown;
}

Instr::Pointer
TexInstr::from_string(std::istream& is, ValueFactory& value_fctory)
{
   string opstr;
   string deststr;
   is >> opstr >> deststr;

   auto opcode = TexInstr::op_from_string(opstr);

   RegisterVec4::Swizzle dest_swz;

   auto dest = value_fctory.dest_vec4_from_string(deststr, dest_swz, pin_group);

   char dummy;
   is >> dummy;
   assert(dummy == ':');

   string srcstr;
   is >> srcstr;

   auto src = value_fctory.src_vec4_from_string(srcstr);

   string res_id_str;
   string sampler_id_str;

   is >> res_id_str >> sampler_id_str;

   int res_id = int_from_string_with_prefix(res_id_str, "RID:");
   int sampler_id = int_from_string_with_prefix(sampler_id_str, "SID:");

   auto tex = new TexInstr(opcode, dest, dest_swz, src, res_id, nullptr,
                           sampler_id, nullptr);

   while (!is.eof() && is.good()) {
      std::string next_token;
      is >> next_token;

      if (next_token.empty())
         break;

      if (next_token[0] == 'U' || next_token[0] == 'N') {
         tex->read_tex_coord_normalitazion(next_token);
      } else {
         tex->set_tex_param(next_token);
      }
   }

   return tex;
}

void
TexInstr::read_tex_coord_normalitazion(const std::string& flags)
{
   assert(flags.length() == 4);
   if (flags[0] == 'U')
      set_tex_flag(x_unnormalized);
   if (flags[1] == 'U')
      set_tex_flag(y_unnormalized);
   if (flags[2] == 'U')
      set_tex_flag(z_unnormalized);
   if (flags[3] == 'U')
      set_tex_flag(w_unnormalized);
}

void
TexInstr::set_tex_param(const std::string& token)
{
   if (token.substr(0, 3) == "OX:")
      set_offset(0, int_from_string_with_prefix(token, "OX:"));
   else if (token.substr(0, 3) == "OY:")
      set_offset(1, int_from_string_with_prefix(token, "OY:"));
   else if (token.substr(0, 3) == "OZ:")
      set_offset(2, int_from_string_with_prefix(token, "OZ:"));
   else if (token.substr(0, 5) == "MODE:")
      set_inst_mode(int_from_string_with_prefix(token, "MODE:"));
   else if (token.substr(0, 3) == "SO:")
      set_sampler_offset(VirtualValue::from_string(token.substr(3))->as_register());
   else if (token.substr(0, 3) == "RO:")
      set_resource_offset(VirtualValue::from_string(token.substr(3))->as_register());
   else {
      std::cerr << "Token '" << token << "': ";
      unreachable("Unknown token in tex param");
   }
}

bool
TexInstr::from_nir(nir_tex_instr *tex, Shader& shader)
{
   Inputs src(*tex, shader.value_factory());

   if (nir_tex_instr_src_index(tex, nir_tex_src_backend1) != -1)
      return emit_lowered_tex(tex, src, shader);

   if (tex->sampler_dim == GLSL_SAMPLER_DIM_BUF) {
      switch (tex->op) {
      case nir_texop_txs:
         return emit_tex_txs(tex, src, {0, 1, 2, 3}, shader);
      case nir_texop_txf:
         return emit_buf_txf(tex, src, shader);
      default:
         return false;
      }
   } else {
      switch (tex->op) {
      case nir_texop_txs:
         return emit_tex_txs(tex, src, {0, 1, 2, 3}, shader);
      case nir_texop_lod:
         return emit_tex_lod(tex, src, shader);
      case nir_texop_query_levels:
         return emit_tex_txs(tex, src, {3, 7, 7, 7}, shader);
      case nir_texop_texture_samples:
         return emit_tex_texture_samples(tex, src, shader);
      default:
         return false;
      }
   }
   return true;
}

bool
TexInstr::replace_source(PRegister old_src, PVirtualValue new_src)
{
   if (old_src->pin() != pin_free)
      return false;

   if (!new_src->as_register())
      return false;

   bool success = false;
   for (int i = 0; i < 4; ++i) {
      if (m_src[i]->equal_to(*old_src)) {
         m_src.set_value(i, new_src->as_register());
         success = true;
      }
   }
   m_src.validate();
   if (success) {
      old_src->del_use(this);
      new_src->as_register()->add_use(this);
   }
   return success;
}

void TexInstr::update_indirect_addr(PRegister old_reg, PRegister addr)
{
   if (resource_offset() && old_reg->equal_to(*resource_offset()))
      set_resource_offset(addr);
   else if (sampler_offset() && old_reg->equal_to(*sampler_offset()))
      set_sampler_offset(addr);

   for (auto& p : m_prepare_instr)
      p->update_indirect_addr(old_reg, addr);
}

uint8_t
TexInstr::allowed_src_chan_mask() const
{
   return m_src.free_chan_mask();
}

struct SamplerId {
   int id;
   bool indirect;
};

SamplerId
get_sampler_id(int sampler_id, const nir_variable *deref)
{
   SamplerId result = {sampler_id, false};

   if (deref) {
      assert(glsl_type_is_sampler(deref->type));
      result.id = deref->data.binding;
   }
   return result;
}

void
TexInstr::emit_set_gradients(
   nir_tex_instr *tex, int texture_id, Inputs& src, TexInstr *irt, Shader& shader)
{
   TexInstr *grad[2] = {nullptr, nullptr};
   RegisterVec4 empty_dst(0, false, {0, 0, 0, 0}, pin_group);
   grad[0] = new TexInstr(set_gradient_h,
                          empty_dst,
                          {7, 7, 7, 7},
                          src.ddx,
                          texture_id,
                          src.texture_offset);
   grad[0]->set_rect_coordinate_flags(tex);
   grad[0]->set_always_keep();

   grad[1] = new TexInstr(set_gradient_v,
                          empty_dst,
                          {7, 7, 7, 7},
                          src.ddy,
                          texture_id,
                          src.texture_offset);
   grad[1]->set_rect_coordinate_flags(tex);
   grad[1]->set_always_keep();
   irt->add_prepare_instr(grad[0]);
   irt->add_prepare_instr(grad[1]);
   if (shader.last_txd())
      irt->add_required_instr(shader.last_txd());
   shader.set_last_txd(irt);
}

void
TexInstr::emit_set_offsets(nir_tex_instr *tex, int texture_id, Inputs& src, TexInstr *irt, Shader& shader)
{
   RegisterVec4::Swizzle swizzle = {4, 4, 4, 4};
   int src_components = tex->coord_components;
   if (tex->is_array)
      --src_components;

   for (int i = 0; i < src_components; ++i)
      swizzle[i] = i;

   auto ofs = shader.value_factory().src_vec4(*src.offset, pin_group, swizzle);
   RegisterVec4 empty_dst(0, false, {0, 0, 0, 0}, pin_group);

   auto set_ofs = new TexInstr(TexInstr::set_offsets,
                               empty_dst,
                               {7, 7, 7, 7},
                               ofs,
                               texture_id + R600_MAX_CONST_BUFFERS,
                               src.texture_offset);
   set_ofs->set_always_keep();
   irt->add_prepare_instr(set_ofs);
}

bool
TexInstr::emit_lowered_tex(nir_tex_instr *tex, Inputs& src, Shader& shader)
{
   assert(src.backend1);
   assert(src.backend2);

   auto& vf = shader.value_factory();
   sfn_log << SfnLog::instr << "emit '" << *reinterpret_cast<nir_instr *>(tex) << "' ("
           << __func__ << ")\n";

   auto params = nir_src_as_const_value(*src.backend2);
   int32_t coord_mask = params[0].i32;
   int32_t flags = params[1].i32;
   int32_t inst_mode = params[2].i32;
   uint32_t dst_swz_packed = params[3].u32;

   auto dst = vf.dest_vec4(tex->def, pin_group);

   RegisterVec4::Swizzle src_swizzle = {0};
   for (int i = 0; i < 4; ++i)
      src_swizzle[i] = (coord_mask & (1 << i)) ? i : 7;

   auto src_coord = vf.src_vec4(*src.backend1, pin_group, src_swizzle);

   RegisterVec4::Swizzle dst_swz = {0, 1, 2, 3};
   if (dst_swz_packed) {
      for (int i = 0; i < 4; ++i) {
         dst_swz[i] = (dst_swz_packed >> (8 * i)) & 0xff;
      }
   }

   int texture_id = tex->texture_index + R600_MAX_CONST_BUFFERS;
   auto irt = new TexInstr(src.opcode,
                           dst,
                           dst_swz,
                           src_coord,
                           texture_id,
                           src.texture_offset,
                           tex->sampler_index,
                           src.sampler_offset);

   if (tex->op == nir_texop_txd)
      emit_set_gradients(tex, texture_id, src, irt, shader);

   if (!irt->set_coord_offsets(src.offset)) {
      assert(tex->op == nir_texop_tg4);
      emit_set_offsets(tex, texture_id, src, irt, shader);
   }

   for (const auto f : TexFlags) {
      if (flags & (1 << f))
         irt->set_tex_flag(f);
   }

   irt->set_inst_mode(inst_mode);

   shader.emit_instruction(irt);
   return true;
}

bool
TexInstr::emit_buf_txf(nir_tex_instr *tex, Inputs& src, Shader& shader)
{
   auto& vf = shader.value_factory();
   auto dst = vf.dest_vec4(tex->def, pin_group);

   PRegister tex_offset = nullptr;
   if (src.sampler_offset)
      tex_offset = shader.emit_load_to_register(src.sampler_offset);

   auto *real_dst = &dst;
   RegisterVec4 tmp = vf.temp_vec4(pin_group);

   if (shader.chip_class() < ISA_CC_EVERGREEN) {
      real_dst = &tmp;
   }

   auto ir = new LoadFromBuffer(*real_dst,
                                {0, 1, 2, 3},
                                src.coord[0],
                                0,
                                tex->texture_index + R600_MAX_CONST_BUFFERS,
                                tex_offset,
                                fmt_invalid);
   ir->set_fetch_flag(FetchInstr::use_const_field);
   shader.emit_instruction(ir);
   shader.set_flag(Shader::sh_uses_tex_buffer);

   if (shader.chip_class() < ISA_CC_EVERGREEN) {
      auto tmp_w = vf.temp_register();
      int buf_sel = (512 + R600_BUFFER_INFO_OFFSET / 16) + 2 * tex->texture_index;
      AluInstr *ir = nullptr;
      for (int i = 0; i < 4; ++i) {
         auto d = i < 3 ? dst[i] : tmp_w;
         ir = new AluInstr(op2_and_int,
                           d,
                           tmp[i],
                           vf.uniform(buf_sel, i, R600_BUFFER_INFO_CONST_BUFFER),
                           AluInstr::write);
         shader.emit_instruction(ir);
      }

      ir->set_alu_flag(alu_last_instr);
      shader.emit_instruction(
         new AluInstr(op2_or_int,
                      dst[3],
                      tmp_w,
                      vf.uniform(buf_sel + 1, 0, R600_BUFFER_INFO_CONST_BUFFER),
                      AluInstr::last_write));
   }

   return true;
}

bool
TexInstr::emit_tex_texture_samples(nir_tex_instr *instr, Inputs& src, Shader& shader)
{
   RegisterVec4 dest = shader.value_factory().dest_vec4(instr->def, pin_chan);
   RegisterVec4 help{
      0, true, {4, 4, 4, 4}
   };

   int res_id = R600_MAX_CONST_BUFFERS + instr->texture_index;

   // Fishy: should the zero be instr->sampler_index?
   auto ir =
      new TexInstr(src.opcode, dest, {3, 7, 7, 7}, help, res_id, src.texture_offset);
   shader.emit_instruction(ir);
   return true;
}

bool
TexInstr::emit_tex_txs(nir_tex_instr *tex,
                       Inputs& src,
                       RegisterVec4::Swizzle dest_swz,
                       Shader& shader)
{
   auto& vf = shader.value_factory();

   auto dest = vf.dest_vec4(tex->def, pin_group);

   if (tex->sampler_dim == GLSL_SAMPLER_DIM_BUF) {
      if (shader.chip_class() >= ISA_CC_EVERGREEN) {
         shader.emit_instruction(new QueryBufferSizeInstr(
            dest, {0, 7, 7, 7}, tex->texture_index + R600_MAX_CONST_BUFFERS));
      } else {
         int id = 2 * tex->texture_index + (512 + R600_BUFFER_INFO_OFFSET / 16) + 1;
         auto src = vf.uniform(id, 1, R600_BUFFER_INFO_CONST_BUFFER);
         shader.emit_instruction(
            new AluInstr(op1_mov, dest[0], src, AluInstr::last_write));
         shader.set_flag(Shader::sh_uses_tex_buffer);
      }
   } else {

      auto src_lod = vf.temp_register();
      shader.emit_instruction(
         new AluInstr(op1_mov, src_lod, src.lod, AluInstr::last_write));

      RegisterVec4 src_coord(src_lod, src_lod, src_lod, src_lod, pin_free);

      if (tex->is_array && tex->sampler_dim == GLSL_SAMPLER_DIM_CUBE)
         dest_swz[2] = 7;

      auto ir = new TexInstr(get_resinfo,
                             dest,
                             dest_swz,
                             src_coord,
                             tex->texture_index + R600_MAX_CONST_BUFFERS,
                             src.texture_offset);

      ir->set_dest_swizzle(dest_swz);
      shader.emit_instruction(ir);

      if (tex->is_array && tex->sampler_dim == GLSL_SAMPLER_DIM_CUBE) {
         auto src_loc = vf.uniform(512 + R600_BUFFER_INFO_OFFSET / 16 + (tex->texture_index >> 2),
                                   tex->texture_index & 3,
                                   R600_BUFFER_INFO_CONST_BUFFER);

         auto alu = new AluInstr(op1_mov, dest[2], src_loc, AluInstr::last_write);
         shader.emit_instruction(alu);
         shader.set_flag(Shader::sh_txs_cube_array_comp);
      }
   }

   return true;
}

auto
TexInstr::prepare_source(nir_tex_instr *tex, const Inputs& inputs, Shader& shader)
   -> RegisterVec4
{
   RegisterVec4::Swizzle target{7, 7, 7, 7};
   PVirtualValue src[4]{nullptr, nullptr, nullptr, nullptr};

   for (unsigned i = 0; i < tex->coord_components; ++i) {
      target[i] = i;
      src[i] = inputs.coord[i];
   }

   // array index always goes into z
   if (tex->is_array && tex->sampler_dim == GLSL_SAMPLER_DIM_1D) {
      target[2] = 1;
      target[1] = 7;
      src[2] = inputs.coord[1];
   }

   /* With txl and txb shadow goes into z and lod or bias go into w */
   if (tex->op == nir_texop_txl || tex->op == nir_texop_txb) {
      target[3] = 3;
      src[3] = tex->op == nir_texop_txl ? inputs.lod : inputs.bias;
      if (tex->is_shadow) {
         target[2] = 2;
         src[2] = inputs.comperator;
      }
   } else if (tex->is_shadow) {
      /* Other ops have shadow in w */
      target[3] = 3;
      src[3] = inputs.comperator;
   }

   auto src_coord = shader.value_factory().temp_vec4(pin_group, target);

   AluInstr *ir = nullptr;
   for (int i = 0; i < 4; ++i) {
      if (target[i] > 3)
         continue;

      auto op = tex->is_array && i == 2 ? op1_rndne : op1_mov;

      ir = new AluInstr(op, src_coord[i], src[i], AluInstr::write);
      shader.emit_instruction(ir);
   }

   if (ir)
      ir->set_alu_flag(alu_last_instr);

   return src_coord;
}

TexInstr::Inputs::Inputs(const nir_tex_instr& instr, ValueFactory& vf):
    sampler_deref(nullptr),
    texture_deref(nullptr),
    bias(nullptr),
    comperator(nullptr),
    lod(nullptr),
    offset(nullptr),
    gather_comp(nullptr),
    ms_index(nullptr),
    texture_offset(nullptr),
    sampler_offset(nullptr),
    backend1(nullptr),
    backend2(nullptr),
    opcode(ld)
{
   // sfn_log << SfnLog::tex << "Get Inputs with " << instr.coord_components
   // << " components\n";

   unsigned grad_components = instr.coord_components;
   if (instr.is_array && !instr.array_is_lowered_cube)
      --grad_components;

   for (unsigned i = 0; i < instr.num_srcs; ++i) {
      switch (instr.src[i].src_type) {
      case nir_tex_src_bias:
         bias = vf.src(instr.src[i], 0);
         break;

      case nir_tex_src_coord: {
         coord = vf.src_vec4(instr.src[i].src,
                             pin_none,
                             swizzle_from_ncomps(instr.coord_components));
      } break;
      case nir_tex_src_comparator:
         comperator = vf.src(instr.src[i], 0);
         break;
      case nir_tex_src_ddx:
         ddx = vf.src_vec4(instr.src[i].src,
                           pin_group,
                           swizzle_from_ncomps(grad_components));
         break;
      case nir_tex_src_ddy:
         ddy = vf.src_vec4(instr.src[i].src,
                           pin_group,
                           swizzle_from_ncomps(grad_components));
         break;
      case nir_tex_src_lod:
         lod = vf.src(instr.src[i].src, 0);
         break;
      case nir_tex_src_offset:
         offset = &instr.src[i].src;
         break;
         /* case nir_tex_src_sampler_deref:
         sampler_deref = get_deref_location(instr.src[i].src);
         break;
      case nir_tex_src_texture_deref:
         texture_deref = get_deref_location(instr.src[i].src);
         break;
      */
      case nir_tex_src_ms_index:
         ms_index = vf.src(instr.src[i], 0);
         break;
      case nir_tex_src_texture_offset:
         texture_offset = vf.src(instr.src[i], 0)->as_register();
         break;
      case nir_tex_src_sampler_offset:
         sampler_offset = vf.src(instr.src[i], 0)->as_register();
         break;
      case nir_tex_src_backend1:
         backend1 = &instr.src[i].src;
         break;
      case nir_tex_src_backend2:
         backend2 = &instr.src[i].src;
         break;
      case nir_tex_src_plane:
      case nir_tex_src_projector:
      case nir_tex_src_min_lod:
      default:
         unreachable("unsupported texture input type");
      }
   }

   opcode = get_opcode(instr);
}

auto
TexInstr::Inputs::get_opcode(const nir_tex_instr& instr) -> Opcode
{
   switch (instr.op) {
   case nir_texop_tex:
      return instr.is_shadow ? sample_c : sample;
   case nir_texop_txf:
      return ld;
   case nir_texop_txb:
      return instr.is_shadow ? sample_c_lb : sample_lb;
   case nir_texop_txl:
      return instr.is_shadow ? sample_c_l : sample_l;
   case nir_texop_txs:
      return get_resinfo;
   case nir_texop_lod:
      return get_resinfo;
   case nir_texop_txd:
      return instr.is_shadow ? sample_c_g : sample_g;
   case nir_texop_tg4: {
      auto var_offset = offset && nir_src_as_const_value(*offset) == nullptr;
      return instr.is_shadow ? (var_offset ? gather4_c_o : gather4_c)
                             : (var_offset ? gather4_o : gather4);
   }
   case nir_texop_txf_ms:
      return ld;
   case nir_texop_query_levels:
      return get_resinfo;
   case nir_texop_texture_samples:
      return TexInstr::get_nsamples;
   default:
      unreachable("unsupported texture input opcode");
   }
}

bool
TexInstr::emit_tex_lod(nir_tex_instr *tex, Inputs& src, Shader& shader)
{
   auto& vf = shader.value_factory();

   auto dst = shader.value_factory().dest_vec4(tex->def, pin_group);

   auto swizzle = src.swizzle_from_ncomps(tex->coord_components);

   auto src_coord = vf.temp_vec4(pin_group, swizzle);

   AluInstr *ir = nullptr;
   for (unsigned i = 0; i < tex->coord_components; ++i) {
      ir = new AluInstr(op1_mov, src_coord[i], src.coord[i], AluInstr::write);
      shader.emit_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);

   auto irt = new TexInstr(TexInstr::get_tex_lod,
                           dst,
                           {1, 0, 7, 7},
                           src_coord,
                           tex->texture_index + R600_MAX_CONST_BUFFERS,
                           src.texture_offset);

   shader.emit_instruction(irt);
   return true;
}

RegisterVec4::Swizzle
TexInstr::Inputs::swizzle_from_ncomps(int comps) const
{
   RegisterVec4::Swizzle swz;
   for (int i = 0; i < 4; ++i)
      swz[i] = i < comps ? i : 7;
   return swz;
}

bool
TexInstr::set_coord_offsets(nir_src *offset)
{
   if (!offset)
      return true;

   auto literal = nir_src_as_const_value(*offset);
   if (!literal)
      return false;

   for (int i = 0; i < offset->ssa->num_components; ++i)
      set_offset(i, literal[i].i32);
   return true;
}

void
TexInstr::set_rect_coordinate_flags(nir_tex_instr *instr)
{
   if (instr->sampler_dim == GLSL_SAMPLER_DIM_RECT) {
      set_tex_flag(x_unnormalized);
      set_tex_flag(y_unnormalized);
   }
}

class LowerTexToBackend : public NirLowerInstruction {
public:
   LowerTexToBackend(amd_gfx_level chip_class);

private:
   bool filter(const nir_instr *instr) const override;
   nir_def *lower(nir_instr *instr) override;

   nir_def *lower_tex(nir_tex_instr *tex);
   nir_def *lower_txf(nir_tex_instr *tex);
   nir_def *lower_tg4(nir_tex_instr *tex);
   nir_def *lower_txf_ms(nir_tex_instr *tex);
   nir_def *lower_txf_ms_direct(nir_tex_instr *tex);

   nir_def *
   prepare_coord(nir_tex_instr *tex, int& unnormalized_mask, int& used_coord_mask);
   int get_src_coords(nir_tex_instr *tex,
                      std::array<nir_def *, 4>& coord,
                      bool round_array_index);
   nir_def *prep_src(std::array<nir_def *, 4>& coord, int& used_coord_mask);
   nir_def *
   finalize(nir_tex_instr *tex, nir_def *backend1, nir_def *backend2);

   nir_def *get_undef();

   amd_gfx_level m_chip_class;
   nir_def *m_undef {nullptr};
};

bool
r600_nir_lower_tex_to_backend(nir_shader *shader, amd_gfx_level chip_class)
{
   return LowerTexToBackend(chip_class).run(shader);
}

LowerTexToBackend::LowerTexToBackend(amd_gfx_level chip_class):
    m_chip_class(chip_class)
{
}

bool
LowerTexToBackend::filter(const nir_instr *instr) const
{
   if (instr->type != nir_instr_type_tex)
      return false;

   auto tex = nir_instr_as_tex(instr);
   if (tex->sampler_dim == GLSL_SAMPLER_DIM_BUF)
      return false;
   switch (tex->op) {
   case nir_texop_tex:
   case nir_texop_txb:
   case nir_texop_txl:
   case nir_texop_txf:
   case nir_texop_txd:
   case nir_texop_tg4:
   case nir_texop_txf_ms:
      break;
   default:
      return false;
   }

   return nir_tex_instr_src_index(tex, nir_tex_src_backend1) == -1;
}

nir_def *LowerTexToBackend::get_undef()
{
   if (!m_undef)
      m_undef = nir_undef(b, 1, 32);
   return m_undef;
}

nir_def *
LowerTexToBackend::lower(nir_instr *instr)
{
   b->cursor = nir_before_instr(instr);

   auto tex = nir_instr_as_tex(instr);
   switch (tex->op) {
   case nir_texop_tex:
   case nir_texop_txb:
   case nir_texop_txl:
   case nir_texop_txd:
      return lower_tex(tex);
   case nir_texop_txf:
      return lower_txf(tex);
   case nir_texop_tg4:
      return lower_tg4(tex);
   case nir_texop_txf_ms:
      if (m_chip_class < EVERGREEN)
         return lower_txf_ms_direct(tex);
      else
         return lower_txf_ms(tex);
   default:
      return nullptr;
   }
}

nir_def *
LowerTexToBackend::lower_tex(nir_tex_instr *tex)
{
   int unnormalized_mask = 0;
   int used_coord_mask = 0;

   nir_def *backend1 = prepare_coord(tex, unnormalized_mask, used_coord_mask);

   nir_def *backend2 = nir_imm_ivec4(b, used_coord_mask, unnormalized_mask, 0, 0);

   return finalize(tex, backend1, backend2);
}

nir_def *
LowerTexToBackend::lower_txf(nir_tex_instr *tex)
{
   std::array<nir_def *, 4> new_coord = {nullptr, nullptr, nullptr, nullptr};

   get_src_coords(tex, new_coord, false);

   int lod_idx = nir_tex_instr_src_index(tex, nir_tex_src_lod);
   new_coord[3] = tex->src[lod_idx].src.ssa;

   int used_coord_mask = 0;
   nir_def *backend1 = prep_src(new_coord, used_coord_mask);
   nir_def *backend2 =
      nir_imm_ivec4(b, used_coord_mask, tex->is_array ? 0x4 : 0, 0, 0);

   return finalize(tex, backend1, backend2);
}

nir_def *
LowerTexToBackend::lower_tg4(nir_tex_instr *tex)
{
   std::array<nir_def *, 4> new_coord = {nullptr, nullptr, nullptr, nullptr};

   get_src_coords(tex, new_coord, false);
   uint32_t dest_swizzle =
      m_chip_class <= EVERGREEN ? 1 | (2 << 8) | (0 << 16) | (3 << 24) : 0;

   int used_coord_mask = 0;
   int unnormalized_mask = 0;
   nir_def *backend1 = prepare_coord(tex, unnormalized_mask, used_coord_mask);

   nir_def *backend2 =
      nir_imm_ivec4(b, used_coord_mask, unnormalized_mask, tex->component, dest_swizzle);
   return finalize(tex, backend1, backend2);
}

nir_def *
LowerTexToBackend::lower_txf_ms(nir_tex_instr *tex)
{
   std::array<nir_def *, 4> new_coord = {nullptr, nullptr, nullptr, nullptr};

   get_src_coords(tex, new_coord, false);

   int ms_index = nir_tex_instr_src_index(tex, nir_tex_src_ms_index);
   new_coord[3] = tex->src[ms_index].src.ssa;

   int offset_index = nir_tex_instr_src_index(tex, nir_tex_src_offset);
   if (offset_index >= 0) {
      auto offset = tex->src[offset_index].src.ssa;
      for (int i = 0; i < offset->num_components; ++i) {
         new_coord[i] = nir_iadd(b, new_coord[i], nir_channel(b, offset, i));
      }
   }

   auto fetch_sample = nir_instr_as_tex(nir_instr_clone(b->shader, &tex->instr));
   nir_def_init(&fetch_sample->instr, &fetch_sample->def, 4, 32);

   int used_coord_mask = 0;
   nir_def *backend1 = prep_src(new_coord, used_coord_mask);
   nir_def *backend2 = nir_imm_ivec4(b, used_coord_mask, 0xf, 1, 0);

   nir_builder_instr_insert(b, &fetch_sample->instr);
   finalize(fetch_sample, backend1, backend2);

   new_coord[3] = nir_iand_imm(b,
                               nir_ushr(b,
                                        nir_channel(b, &fetch_sample->def, 0),
                                        nir_ishl_imm(b, new_coord[3], 2)),
                               15);

   nir_def *backend1b = prep_src(new_coord, used_coord_mask);
   nir_def *backend2b = nir_imm_ivec4(b, used_coord_mask, 0, 0, 0);
   return finalize(tex, backend1b, backend2b);
}

nir_def *
LowerTexToBackend::lower_txf_ms_direct(nir_tex_instr *tex)
{
   std::array<nir_def *, 4> new_coord = {nullptr, nullptr, nullptr, nullptr};

   get_src_coords(tex, new_coord, false);

   int ms_index = nir_tex_instr_src_index(tex, nir_tex_src_ms_index);
   new_coord[3] = tex->src[ms_index].src.ssa;

   int used_coord_mask = 0;
   nir_def *backend1 = prep_src(new_coord, used_coord_mask);
   nir_def *backend2 = nir_imm_ivec4(b, used_coord_mask, 0, 0, 0);

   return finalize(tex, backend1, backend2);
}

nir_def *
LowerTexToBackend::finalize(nir_tex_instr *tex,
                            nir_def *backend1,
                            nir_def *backend2)
{
   nir_tex_instr_add_src(tex, nir_tex_src_backend1, backend1);
   nir_tex_instr_add_src(tex, nir_tex_src_backend2, backend2);

   static const nir_tex_src_type cleanup[] = {nir_tex_src_coord,
                                              nir_tex_src_lod,
                                              nir_tex_src_bias,
                                              nir_tex_src_comparator,
                                              nir_tex_src_ms_index};

   for (const auto type : cleanup) {
      int pos = nir_tex_instr_src_index(tex, type);
      if (pos >= 0)
         nir_tex_instr_remove_src(tex, pos);
   }
   return NIR_LOWER_INSTR_PROGRESS;
}

nir_def *
LowerTexToBackend::prep_src(std::array<nir_def *, 4>& coord, int& used_coord_mask)
{
   int max_coord = 0;
   for (int i = 0; i < 4; ++i) {
      if (coord[i]) {
         used_coord_mask |= 1 << i;
         max_coord = i;
      } else
         coord[i] = get_undef();
   }

   return nir_vec(b, coord.data(), max_coord + 1);
}

nir_def *
LowerTexToBackend::prepare_coord(nir_tex_instr *tex,
                                 int& unnormalized_mask,
                                 int& used_coord_mask)
{
   std::array<nir_def *, 4> new_coord = {nullptr, nullptr, nullptr, nullptr};

   unnormalized_mask = get_src_coords(tex, new_coord, true);
   used_coord_mask = 0;

   int comp_idx =
      tex->is_shadow ? nir_tex_instr_src_index(tex, nir_tex_src_comparator) : -1;

   if (tex->op == nir_texop_txl || tex->op == nir_texop_txb) {
      int idx = tex->op == nir_texop_txl ? nir_tex_instr_src_index(tex, nir_tex_src_lod)
                                         : nir_tex_instr_src_index(tex, nir_tex_src_bias);
      assert(idx != -1);
      new_coord[3] = tex->src[idx].src.ssa;

      if (comp_idx >= 0)
         new_coord[2] = tex->src[comp_idx].src.ssa;
   } else if (comp_idx >= 0) {
      new_coord[3] = tex->src[comp_idx].src.ssa;
   }
   return prep_src(new_coord, used_coord_mask);
}

int
LowerTexToBackend::get_src_coords(nir_tex_instr *tex,
                                  std::array<nir_def *, 4>& coord,
                                  bool round_array_index)
{
   int unnormalized_mask = 0;
   auto coord_idx = nir_tex_instr_src_index(tex, nir_tex_src_coord);
   assert(coord_idx != -1);
   auto old_coord = tex->src[coord_idx];

   coord = {nir_channel(b, old_coord.src.ssa, 0), nullptr, nullptr, nullptr};

   if (tex->coord_components > 1) {
      if (tex->is_array && tex->sampler_dim == GLSL_SAMPLER_DIM_1D)
         coord[2] = nir_channel(b, old_coord.src.ssa, 1);
      else
         coord[1] = nir_channel(b, old_coord.src.ssa, 1);
   }

   if (tex->coord_components > 2) {
      coord[2] = nir_channel(b, old_coord.src.ssa, 2);
   }
   if (tex->is_array) {
      unnormalized_mask |= 0x4;
      if (round_array_index)
         coord[2] = nir_fround_even(b, coord[2]);
   }

   if (tex->sampler_dim == GLSL_SAMPLER_DIM_RECT) {
      unnormalized_mask |= 0x3;
   }

   return unnormalized_mask;
}

} // namespace r600
