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

#include "sfn_instr_fetch.h"

#include "sfn_alu_defines.h"
#include "sfn_defines.h"
#include "sfn_valuefactory.h"

#include <sstream>

namespace r600 {

using std::istringstream;
using std::string;

FetchInstr::FetchInstr(EVFetchInstr opcode,
                       const RegisterVec4& dst,
                       const RegisterVec4::Swizzle& dest_swizzle,
                       PRegister src,
                       uint32_t src_offset,
                       EVFetchType fetch_type,
                       EVTXDataFormat data_format,
                       EVFetchNumFormat num_format,
                       EVFetchEndianSwap endian_swap,
                       uint32_t resource_id,
                       PRegister resource_offset):
    InstrWithVectorResult(dst, dest_swizzle, resource_id, resource_offset),   
    m_opcode(opcode),
    m_src(src),
    m_src_offset(src_offset),
    m_fetch_type(fetch_type),
    m_data_format(data_format),
    m_num_format(num_format),
    m_endian_swap(endian_swap),
    m_mega_fetch_count(0),
    m_array_base(0),
    m_array_size(0),
    m_elm_size(0)
{
   switch (m_opcode) {
   case vc_fetch:
      m_opname = "VFETCH";
      break;
   case vc_semantic:
      m_opname = "FETCH_SEMANTIC";
      break;
   case vc_get_buf_resinfo:
      set_print_skip(mfc);
      set_print_skip(fmt);
      set_print_skip(ftype);
      m_opname = "GET_BUF_RESINFO";
      break;
   case vc_read_scratch:
      m_opname = "READ_SCRATCH";
      break;
   default:
      unreachable("Unknown fetch instruction");
   }

   if (m_src)
      m_src->add_use(this);
}

void
FetchInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
FetchInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

bool
FetchInstr::is_equal_to(const FetchInstr& rhs) const
{
   if (m_src) {
      if (rhs.m_src) {
         if (!m_src->equal_to(*rhs.m_src))
            return false;
      } else
         return false;
   } else if (rhs.m_src)
      return false;

   if (!comp_dest(rhs.dst(), rhs.all_dest_swizzle()))
      return false;

   if (m_tex_flags != rhs.m_tex_flags)
      return false;

   if (resource_offset() && rhs.resource_offset()) {
      if (!resource_offset()->equal_to(*rhs.resource_offset()))
         return false;
   } else if (!(!!resource_offset() == !!rhs.resource_offset()))
      return false;

   return m_opcode == rhs.m_opcode && m_src_offset == rhs.m_src_offset &&
          m_fetch_type == rhs.m_fetch_type && m_data_format == rhs.m_data_format &&
          m_num_format == rhs.m_num_format && m_endian_swap == rhs.m_endian_swap &&
          m_mega_fetch_count == rhs.m_mega_fetch_count &&
          m_array_base == rhs.m_array_base && m_array_size == rhs.m_array_size &&
          m_elm_size == rhs.m_elm_size && resource_id() == rhs.resource_id();
}

bool
FetchInstr::propagate_death()
{
   auto reg = m_src->as_register();
   if (reg)
      reg->del_use(this);
   return true;
}

bool
FetchInstr::replace_source(PRegister old_src, PVirtualValue new_src)
{
   bool success = false;
   auto new_reg = new_src->as_register();
   if (new_reg) {
      if (old_src->equal_to(*m_src)) {
         m_src->del_use(this);
         m_src = new_reg;
         new_reg->add_use(this);
         success = true;
      }
      success |= replace_resource_offset(old_src, new_reg);
   }
   return success;
}

bool
FetchInstr::do_ready() const
{
   for (auto i : required_instr()) {
      if (!i->is_scheduled())
         return false;
   }

   bool result = m_src && m_src->ready(block_id(), index());
   if (resource_offset())
      result &= resource_offset()->ready(block_id(), index());
   return result;
}

void
FetchInstr::do_print(std::ostream& os) const
{
   os << m_opname << ' ';

   print_dest(os);

   os << " :";

   if (m_opcode != vc_get_buf_resinfo) {

      if (m_src && m_src->chan() < 7) {
         os << " " << *m_src;
         if (m_src_offset)
            os << " + " << m_src_offset << "b";
      }
   }

   if (m_opcode != vc_read_scratch)
      os << " RID:" << resource_id();

   print_resource_offset(os);

   if (!m_skip_print.test(ftype)) {
      switch (m_fetch_type) {
      case vertex_data:
         os << " VERTEX";
         break;
      case instance_data:
         os << " INSTANCE_DATA";
         break;
      case no_index_offset:
         os << " NO_IDX_OFFSET";
         break;
      default:
         unreachable("Unknown fetch instruction type");
      }
   }

   if (!m_skip_print.test(fmt)) {
      os << " FMT(";
      auto fmt = s_data_format_map.find(m_data_format);
      if (fmt != s_data_format_map.end())
         os << fmt->second << ",";
      else
         unreachable("unknown data format");

      if (m_tex_flags.test(format_comp_signed))
         os << "S";
      else
         os << "U";

      switch (m_num_format) {
      case vtx_nf_norm:
         os << "NORM";
         break;
      case vtx_nf_int:
         os << "INT";
         break;
      case vtx_nf_scaled:
         os << "SCALED";
         break;
      default:
         unreachable("Unknown number format");
      }

      os << ")";
   }

   if (m_array_base) {
      if (m_opcode != vc_read_scratch)
         os << " BASE:" << m_array_base;
      else
         os << " L[0x" << std::uppercase << std::hex << m_array_base << std::dec << "]";
   }

   if (m_array_size)
      os << " SIZE:" << m_array_size + 1;

   if (m_tex_flags.test(is_mega_fetch) && !m_skip_print.test(mfc))
      os << " MFC:" << m_mega_fetch_count;

   if (m_elm_size)
      os << " ES:" << m_elm_size;

   if (m_tex_flags.test(fetch_whole_quad))
      os << " WQ";
   if (m_tex_flags.test(use_const_field))
      os << " UCF";
   if (m_tex_flags.test(srf_mode))
      os << " SRF";
   if (m_tex_flags.test(buf_no_stride))
      os << " BNS";
   if (m_tex_flags.test(alt_const))
      os << " AC";
   if (m_tex_flags.test(use_tc))
      os << " TC";
   if (m_tex_flags.test(vpm))
      os << " VPM";
   if (m_tex_flags.test(uncached) && m_opcode != vc_read_scratch)
      os << " UNCACHED";
   if (m_tex_flags.test(indexed) && m_opcode != vc_read_scratch)
      os << " INDEXED";
}

Instr::Pointer
FetchInstr::from_string(std::istream& is, ValueFactory& vf)
{
   return from_string_impl(is, vc_fetch, vf);
}

Instr::Pointer
FetchInstr::from_string_impl(std::istream& is, EVFetchInstr opcode, ValueFactory& vf)
{
   std::string deststr;
   is >> deststr;

   RegisterVec4::Swizzle dst_swz;
   auto dest_reg = vf.dest_vec4_from_string(deststr, dst_swz, pin_group);

   char help;
   is >> help;
   assert(help == ':');

   string srcstr;
   is >> srcstr;

   std::cerr << "Get source " << srcstr << "\n";

   auto src_reg = vf.src_from_string(srcstr)->as_register();
   assert(src_reg);

   string res_id_str;
   string next;
   is >> next;

   int src_offset_val = 0;

   if (next == "+") {
      is >> src_offset_val;
      is >> help;
      assert(help == 'b');
      is >> res_id_str;
   } else {
      res_id_str = next;
   }

   int res_id = int_from_string_with_prefix(res_id_str, "RID:");

   string fetch_type_str;
   is >> fetch_type_str;

   EVFetchType fetch_type = vertex_data;
   if (fetch_type_str == "VERTEX") {
      fetch_type = vertex_data;
   } else {
      assert("Fetch type not yet implemented");
   }

   string format_str;
   is >> format_str;

   assert(!strncmp(format_str.c_str(), "FMT(", 4));
   string data_format;
   string num_format_str;

   istringstream fmt_stream(format_str.substr(4));
   bool is_num_fmr = false;
   assert(!fmt_stream.eof());

   do {
      char c;
      fmt_stream >> c;

      if (c == ',') {
         is_num_fmr = true;
         continue;
      }

      if (!is_num_fmr)
         data_format.append(1, c);
      else
         num_format_str.append(1, c);
   } while (!fmt_stream.eof());

   EVTXDataFormat fmt = fmt_invalid;

   for (auto& [f, name] : s_data_format_map) {
      if (data_format == name) {
         fmt = f;
         break;
      }
   }

   assert(fmt != fmt_invalid);

   bool fmt_signed = num_format_str[0] == 'S';
   assert(fmt_signed || num_format_str[0] == 'U');

   size_t num_format_end = num_format_str.find(')');
   num_format_str = num_format_str.substr(1, num_format_end - 1);

   EVFetchNumFormat num_fmt;
   if (num_format_str == "NORM")
      num_fmt = vtx_nf_norm;
   else if (num_format_str == "INT")
      num_fmt = vtx_nf_int;
   else if (num_format_str == "SCALED")
      num_fmt = vtx_nf_scaled;
   else {
      std::cerr << "Number format: '" << num_format_str << "' : ";
      unreachable("Unknown number format");
   }

   auto fetch = new FetchInstr(opcode,
                               dest_reg,
                               dst_swz,
                               src_reg,
                               src_offset_val,
                               fetch_type,
                               fmt,
                               num_fmt,
                               vtx_es_none,
                               res_id,
                               nullptr);
   if (fmt_signed)
      fetch->set_fetch_flag(format_comp_signed);

   while (!is.eof() && is.good()) {
      std::string next_token;
      is >> next_token;

      if (next_token.empty())
         break;

      if (next_token.find(':') != string::npos) {
         fetch->set_param_from_string(next_token);
      } else {
         fetch->set_flag_from_string(next_token);
      }
   }

   return fetch;
}

void
FetchInstr::set_param_from_string(const std::string& token)
{
   if (token.substr(0, 4) == "MFC:")
      set_mfc(int_from_string_with_prefix(token, "MFC:"));
   else if (token.substr(0, 5) == "ARRB:")
      set_array_base(int_from_string_with_prefix(token, "ARRB:"));
   else if (token.substr(0, 5) == "ARRS:")
      set_array_size(int_from_string_with_prefix(token, "ARRS:"));
   else if (token.substr(0, 3) == "ES:")
      set_element_size(int_from_string_with_prefix(token, "ES:"));
   else {
      std::cerr << "Token '" << token << "': ";
      unreachable("Unknown token in fetch param list");
   }
}

void
FetchInstr::set_flag_from_string(const std::string& token)
{
   auto flag = s_flag_map.find(token.c_str());
   if (flag != s_flag_map.end())
      set_fetch_flag(flag->second);
   else {
      std::cerr << "Token: " << token << " : ";
      unreachable("Unknown token in fetch flag list");
   }
}

const std::map<const char *, FetchInstr::EFlags> FetchInstr::s_flag_map = {
   {"WQ",       fetch_whole_quad},
   {"UCF",      use_const_field },
   {"SRF",      srf_mode        },
   {"BNS",      buf_no_stride   },
   {"AC",       alt_const       },
   {"TC",       use_tc          },
   {"VPM",      vpm             },
   {"UNCACHED", uncached        },
   {"INDEXED",  indexed         }
};

const std::map<EVTXDataFormat, const char *> FetchInstr::s_data_format_map = {
   {fmt_invalid,           "INVALID"          },
   {fmt_8,                 "8"                },
   {fmt_4_4,               "4_4"              },
   {fmt_3_3_2,             "3_3_2"            },
   {fmt_reserved_4,        "RESERVED_4"       },
   {fmt_16,                "16"               },
   {fmt_16_float,          "16F"              },
   {fmt_8_8,               "8_8"              },
   {fmt_5_6_5,             "5_6_5"            },
   {fmt_6_5_5,             "6_5_5"            },
   {fmt_1_5_5_5,           "1_5_5_5"          },
   {fmt_4_4_4_4,           "4_4_4_4"          },
   {fmt_5_5_5_1,           "5_5_5_1"          },
   {fmt_32,                "32"               },
   {fmt_32_float,          "32F"              },
   {fmt_16_16,             "16_16"            },
   {fmt_16_16_float,       "16_16F"           },
   {fmt_8_24,              "8_24"             },
   {fmt_8_24_float,        "8_24F"            },
   {fmt_24_8,              "24_8"             },
   {fmt_24_8_float,        "24_8F"            },
   {fmt_10_11_11,          "10_11_11"         },
   {fmt_10_11_11_float,    "10_11_11F"        },
   {fmt_11_11_10,          "11_11_10"         },
   {fmt_10_11_11_float,    "11_11_10F"        },
   {fmt_2_10_10_10,        "2_10_10_10"       },
   {fmt_8_8_8_8,           "8_8_8_8"          },
   {fmt_10_10_10_2,        "10_10_10_2"       },
   {fmt_x24_8_32_float,    "X24_8_32F"        },
   {fmt_32_32,             "32_32"            },
   {fmt_32_32_float,       "32_32F"           },
   {fmt_16_16_16_16,       "16_16_16_16"      },
   {fmt_16_16_16_16_float, "16_16_16_16F"     },
   {fmt_reserved_33,       "RESERVED_33"      },
   {fmt_32_32_32_32,       "32_32_32_32"      },
   {fmt_32_32_32_32_float, "32_32_32_32F"     },
   {fmt_reserved_36,       "RESERVED_36"      },
   {fmt_1,                 "1"                },
   {fmt_1_reversed,        "1_REVERSED"       },
   {fmt_gb_gr,             "GB_GR"            },
   {fmt_bg_rg,             "BG_RG"            },
   {fmt_32_as_8,           "32_AS_8"          },
   {fmt_32_as_8_8,         "32_AS_8_8"        },
   {fmt_5_9_9_9_sharedexp, "5_9_9_9_SHAREDEXP"},
   {fmt_8_8_8,             "8_8_8"            },
   {fmt_16_16_16,          "16_16_16"         },
   {fmt_16_16_16_float,    "16_16_16F"        },
   {fmt_32_32_32,          "32_32_32"         },
   {fmt_32_32_32_float,    "32_32_32F"        },
   {fmt_bc1,               "BC1"              },
   {fmt_bc2,               "BC2"              },
   {fmt_bc3,               "BC3"              },
   {fmt_bc4,               "BC4"              },
   {fmt_bc5,               "BC5"              },
   {fmt_apc0,              "APC0"             },
   {fmt_apc1,              "APC1"             },
   {fmt_apc2,              "APC2"             },
   {fmt_apc3,              "APC3"             },
   {fmt_apc4,              "APC4"             },
   {fmt_apc5,              "APC5"             },
   {fmt_apc6,              "APC6"             },
   {fmt_apc7,              "APC7"             },
   {fmt_ctx1,              "CTX1"             },
   {fmt_reserved_63,       "RESERVED_63"      }
};

QueryBufferSizeInstr::QueryBufferSizeInstr(const RegisterVec4& dst,
                                           const RegisterVec4::Swizzle& dst_swz,
                                           uint32_t resid):
    FetchInstr(vc_get_buf_resinfo,
               dst,
               dst_swz,
               new Register(0, 7, pin_fully),
               0,
               no_index_offset,
               fmt_32_32_32_32,
               vtx_nf_norm,
               vtx_es_none,
               resid,
               nullptr)
{
   set_fetch_flag(format_comp_signed);
   set_print_skip(mfc);
   set_print_skip(fmt);
   set_print_skip(ftype);
}

Instr::Pointer
QueryBufferSizeInstr::from_string(std::istream& is, ValueFactory& vf)
{
   std::string deststr, res_id_str;
   is >> deststr;

   char help;
   is >> help;
   assert(help == ':');

   is >> res_id_str;

   RegisterVec4::Swizzle dst_swz;
   auto dst = vf.dest_vec4_from_string(deststr, dst_swz, pin_group);
   int res_id = int_from_string_with_prefix(res_id_str, "RID:");

   return new QueryBufferSizeInstr(dst, dst_swz, res_id);
}

LoadFromBuffer::LoadFromBuffer(const RegisterVec4& dst,
                               const RegisterVec4::Swizzle& dst_swizzle,
                               PRegister addr,
                               uint32_t addr_offset,
                               uint32_t resid,
                               PRegister res_offset,
                               EVTXDataFormat data_format):
    FetchInstr(vc_fetch,
               dst,
               dst_swizzle,
               addr,
               addr_offset,
               no_index_offset,
               data_format,
               vtx_nf_scaled,
               vtx_es_none,
               resid,
               res_offset)
{
   set_fetch_flag(format_comp_signed);
   set_mfc(16);
   override_opname("LOAD_BUF");
   set_print_skip(mfc);
   set_print_skip(fmt);
   set_print_skip(ftype);
}

Instr::Pointer
LoadFromBuffer::from_string(std::istream& is, ValueFactory& vf)
{
   std::string deststr;
   is >> deststr;

   RegisterVec4::Swizzle dst_swz;
   auto dst = vf.dest_vec4_from_string(deststr, dst_swz, pin_group);

   char help;
   is >> help;
   assert(help == ':');

   string addrstr;
   is >> addrstr;
   auto addr_reg = vf.src_from_string(addrstr)->as_register();

   string res_id_str;
   string next;
   is >> next;

   int addr_offset_val = 0;

   if (next == "+") {
      is >> addr_offset_val;
      is >> help;
      assert(help == 'b');
      is >> res_id_str;
   } else {
      res_id_str = next;
   }

   int res_id = int_from_string_with_prefix(res_id_str, "RID:");

   next.clear();
   is >> next;
   PRegister res_offset = nullptr;
   if (next == "+") {
      string res_offset_str;
      is >> res_offset_str;
      res_offset = vf.src_from_string(res_offset_str)->as_register();
   }

   auto fetch = new LoadFromBuffer(
      dst, dst_swz, addr_reg, addr_offset_val, res_id, res_offset, fmt_32_32_32_32_float);
   is >> next;
   if (next == "SRF")
      fetch->set_fetch_flag(srf_mode);

   return fetch;
}

class AddrResolver : public RegisterVisitor {
public:
   AddrResolver(LoadFromScratch *lfs):
       m_lfs(lfs)
   {
   }

   void visit(Register& value)
   {
      m_lfs->set_fetch_flag(FetchInstr::indexed);
      m_lfs->set_src(&value);
      value.add_use(m_lfs);
   }
   void visit(LocalArray& value)
   {
      unreachable("An array can't be a direct source for scratch reads");
      (void)value;
   }
   void visit(LocalArrayValue& value)
   {
      unreachable("An array value can't be a direct source for scratch reads");
      // TODO: an array element with constant offset could be used here
      (void)value;
   }
   void visit(UniformValue& value)
   {
      unreachable("A uniform can't be a direct source for scratch reads");
      (void)value;
   }
   void visit(LiteralConstant& value)
   {
      m_lfs->set_array_base(value.value());
      m_lfs->set_src(new Register(0, 7, pin_none));
   }
   void visit(InlineConstant& value)
   {
      if (value.sel() == ALU_SRC_1_INT)
         m_lfs->set_array_base(1);
      else if (value.sel() != ALU_SRC_0)
         unreachable("Scratch array base is an impossible inline constant");

      m_lfs->set_src(new Register(0, 7, pin_none));
   }

   LoadFromScratch *m_lfs;
};

LoadFromScratch::LoadFromScratch(const RegisterVec4& dst,
                                 const RegisterVec4::Swizzle& dst_swz,
                                 PVirtualValue addr,
                                 uint32_t scratch_size):
    FetchInstr(vc_read_scratch,
               dst,
               dst_swz,
               nullptr,
               0,
               no_index_offset,
               fmt_32_32_32_32,
               vtx_nf_int,
               vtx_es_none,
               0,
               nullptr)
{
   set_fetch_flag(uncached);
   set_fetch_flag(wait_ack);

   assert(scratch_size >= 1);
   set_array_size(scratch_size - 1);
   set_array_base(0);
   AddrResolver ar(this);
   addr->accept(ar);

   set_print_skip(mfc);
   set_print_skip(fmt);
   set_print_skip(ftype);
   set_element_size(3);
}

Instr::Pointer
LoadFromScratch::from_string(std::istream& is, ValueFactory& vf)
{
   std::string deststr;
   is >> deststr;

   RegisterVec4::Swizzle dst_swz;
   auto dest = vf.dest_vec4_from_string(deststr, dst_swz, pin_group);

   char help;
   is >> help;
   assert(help == ':');

   string addrstr;
   is >> addrstr;
   auto addr_reg = vf.src_from_string(addrstr);

   string offsetstr;
   is >> offsetstr;
   int size = int_from_string_with_prefix(offsetstr, "SIZE:");
   assert(size >= 1);

   return new LoadFromScratch(dest, dst_swz, addr_reg, size);
}

} // namespace r600
