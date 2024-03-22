#
# Copyright (c) 2018 Valve Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#

# Class that represents all the information we have about the opcode
# NOTE: this must be kept in sync with aco_op_info

import sys
from enum import Enum, IntEnum, auto

class InstrClass(Enum):
   Valu32 = "valu32"
   ValuConvert32 = "valu_convert32"
   Valu64 = "valu64"
   ValuQuarterRate32 = "valu_quarter_rate32"
   ValuFma = "valu_fma"
   ValuTranscendental32 = "valu_transcendental32"
   ValuDouble = "valu_double"
   ValuDoubleAdd = "valu_double_add"
   ValuDoubleConvert = "valu_double_convert"
   ValuDoubleTranscendental = "valu_double_transcendental"
   WMMA = "wmma"
   Salu = "salu"
   SMem = "smem"
   Barrier = "barrier"
   Branch = "branch"
   Sendmsg = "sendmsg"
   DS = "ds"
   Export = "exp"
   VMem = "vmem"
   Waitcnt = "waitcnt"
   Other = "other"

# Representation of the instruction's microcode encoding format
# Note: Some Vector ALU Formats can be combined, such that:
# - VOP2* | VOP3 represents a VOP2 instruction in VOP3 encoding
# - VOP2* | DPP represents a VOP2 instruction with data parallel primitive.
# - VOP2* | SDWA represents a VOP2 instruction with sub-dword addressing.
#
# (*) The same is applicable for VOP1 and VOPC instructions.
class Format(IntEnum):
   # Pseudo Instruction Formats
   PSEUDO = 0
   PSEUDO_BRANCH = auto()
   PSEUDO_BARRIER = auto()
   PSEUDO_REDUCTION = auto()
   # Scalar ALU & Control Formats
   SOP1 = auto()
   SOP2 = auto()
   SOPK = auto()
   SOPP = auto()
   SOPC = auto()
   # Scalar Memory Format
   SMEM = auto()
   # LDS/GDS Format
   DS = auto()
   LDSDIR = auto()
   # Vector Memory Buffer Formats
   MTBUF = auto()
   MUBUF = auto()
   # Vector Memory Image Format
   MIMG = auto()
   # Export Format
   EXP = auto()
   # Flat Formats
   FLAT = auto()
   GLOBAL = auto()
   SCRATCH = auto()
   # Vector Parameter Interpolation Formats
   VINTRP = auto()
   # Vector ALU Formats
   VINTERP_INREG = auto()
   VOP1 = 1 << 7
   VOP2 = 1 << 8
   VOPC = 1 << 9
   VOP3 = 1 << 10
   VOP3P = 1 << 11
   SDWA = 1 << 12
   DPP16 = 1 << 13
   DPP8 = 1 << 14

   def get_builder_fields(self):
      if self == Format.SOPK:
         return [('uint16_t', 'imm', None)]
      elif self == Format.SOPP:
         return [('uint32_t', 'block', '-1'),
                 ('uint32_t', 'imm', '0')]
      elif self == Format.SMEM:
         return [('memory_sync_info', 'sync', 'memory_sync_info()'),
                 ('bool', 'glc', 'false'),
                 ('bool', 'dlc', 'false'),
                 ('bool', 'nv', 'false')]
      elif self == Format.DS:
         return [('uint16_t', 'offset0', '0'),
                 ('uint8_t', 'offset1', '0'),
                 ('bool', 'gds', 'false')]
      elif self == Format.LDSDIR:
         return [('uint8_t', 'attr', 0),
                 ('uint8_t', 'attr_chan', 0),
                 ('memory_sync_info', 'sync', 'memory_sync_info()'),
                 ('uint8_t', 'wait_vdst', 15)]
      elif self == Format.MTBUF:
         return [('unsigned', 'dfmt', None),
                 ('unsigned', 'nfmt', None),
                 ('unsigned', 'offset', None),
                 ('bool', 'offen', None),
                 ('bool', 'idxen', 'false'),
                 ('bool', 'disable_wqm', 'false'),
                 ('bool', 'glc', 'false'),
                 ('bool', 'dlc', 'false'),
                 ('bool', 'slc', 'false'),
                 ('bool', 'tfe', 'false')]
      elif self == Format.MUBUF:
         return [('unsigned', 'offset', None),
                 ('bool', 'offen', None),
                 ('bool', 'swizzled', 'false'),
                 ('bool', 'idxen', 'false'),
                 ('bool', 'addr64', 'false'),
                 ('bool', 'disable_wqm', 'false'),
                 ('bool', 'glc', 'false'),
                 ('bool', 'dlc', 'false'),
                 ('bool', 'slc', 'false'),
                 ('bool', 'tfe', 'false'),
                 ('bool', 'lds', 'false')]
      elif self == Format.MIMG:
         return [('unsigned', 'dmask', '0xF'),
                 ('bool', 'da', 'false'),
                 ('bool', 'unrm', 'false'),
                 ('bool', 'disable_wqm', 'false'),
                 ('bool', 'glc', 'false'),
                 ('bool', 'dlc', 'false'),
                 ('bool', 'slc', 'false'),
                 ('bool', 'tfe', 'false'),
                 ('bool', 'lwe', 'false'),
                 ('bool', 'r128', 'false'),
                 ('bool', 'a16', 'false'),
                 ('bool', 'd16', 'false')]
         return [('unsigned', 'attribute', None),
                 ('unsigned', 'component', None)]
      elif self == Format.EXP:
         return [('unsigned', 'enabled_mask', None),
                 ('unsigned', 'dest', None),
                 ('bool', 'compr', 'false', 'compressed'),
                 ('bool', 'done', 'false'),
                 ('bool', 'vm', 'false', 'valid_mask')]
      elif self == Format.PSEUDO_BRANCH:
         return [('uint32_t', 'target0', '0', 'target[0]'),
                 ('uint32_t', 'target1', '0', 'target[1]')]
      elif self == Format.PSEUDO_REDUCTION:
         return [('ReduceOp', 'op', None, 'reduce_op'),
                 ('unsigned', 'cluster_size', '0')]
      elif self == Format.PSEUDO_BARRIER:
         return [('memory_sync_info', 'sync', None),
                 ('sync_scope', 'exec_scope', 'scope_invocation')]
      elif self == Format.VINTRP:
         return [('unsigned', 'attribute', None),
                 ('unsigned', 'component', None)]
      elif self == Format.DPP16:
         return [('uint16_t', 'dpp_ctrl', None),
                 ('uint8_t', 'row_mask', '0xF'),
                 ('uint8_t', 'bank_mask', '0xF'),
                 ('bool', 'bound_ctrl', 'true'),
                 ('bool', 'fetch_inactive', 'true')]
      elif self == Format.DPP8:
         return [('uint32_t', 'lane_sel', 0),
                 ('bool', 'fetch_inactive', 'true')]
      elif self == Format.VOP3P:
         return [('uint8_t', 'opsel_lo', None),
                 ('uint8_t', 'opsel_hi', None)]
      elif self == Format.VINTERP_INREG:
         return [('unsigned', 'wait_exp', 7),
                 ('uint8_t', 'opsel', 0)]
      elif self in [Format.FLAT, Format.GLOBAL, Format.SCRATCH]:
         return [('int16_t', 'offset', 0),
                 ('memory_sync_info', 'sync', 'memory_sync_info()'),
                 ('bool', 'glc', 'false'),
                 ('bool', 'slc', 'false'),
                 ('bool', 'lds', 'false'),
                 ('bool', 'nv', 'false')]
      else:
         return []

   def get_builder_field_names(self):
      return [f[1] for f in self.get_builder_fields()]

   def get_builder_field_dests(self):
      return [(f[3] if len(f) >= 4 else f[1]) for f in self.get_builder_fields()]

   def get_builder_field_decls(self):
      return [('%s %s=%s' % (f[0], f[1], f[2]) if f[2] != None else '%s %s' % (f[0], f[1])) for f in self.get_builder_fields()]

   def get_builder_initialization(self, num_operands):
      res = ''
      if self == Format.SDWA:
         for i in range(min(num_operands, 2)):
            res += 'instr->sel[{0}] = SubdwordSel(op{0}.op.bytes(), 0, false);'.format(i)
         res += 'instr->dst_sel = SubdwordSel(def0.bytes(), 0, false);\n'
      elif self in [Format.DPP16, Format.DPP8]:
         res += 'instr->fetch_inactive &= program->gfx_level >= GFX10;\n'
      return res


class Opcode(object):
   """Class that represents all the information we have about the opcode
   NOTE: this must be kept in sync with aco_op_info
   """
   def __init__(self, name, opcode_gfx7, opcode_gfx9, opcode_gfx10, opcode_gfx11, format, input_mod, output_mod, is_atomic, cls, definitions, operands):
      assert isinstance(name, str)
      assert isinstance(opcode_gfx7, int)
      assert isinstance(opcode_gfx9, int)
      assert isinstance(opcode_gfx10, int)
      assert isinstance(opcode_gfx11, int)
      assert isinstance(format, Format)
      assert isinstance(input_mod, bool)
      assert isinstance(output_mod, bool)
      assert isinstance(definitions, int)
      assert isinstance(operands, int)

      self.name = name
      self.opcode_gfx7 = opcode_gfx7
      self.opcode_gfx9 = opcode_gfx9
      self.opcode_gfx10 = opcode_gfx10
      self.opcode_gfx11 = opcode_gfx11
      self.input_mod = "1" if input_mod else "0"
      self.output_mod = "1" if output_mod else "0"
      self.is_atomic = "1" if is_atomic else "0"
      self.format = format
      self.cls = cls
      self.definitions = definitions
      self.operands = operands

      parts = name.replace('_e64', '').rsplit('_', 2)
      op_dtype = parts[-1]

      op_dtype_sizes = {'{}{}'.format(prefix, size) : size for prefix in 'biuf' for size in [64, 32, 24, 16]}
      # inline constants are 32-bit for 16-bit integer/typeless instructions: https://reviews.llvm.org/D81841
      op_dtype_sizes['b16'] = 32
      op_dtype_sizes['i16'] = 32
      op_dtype_sizes['u16'] = 32

      # If we can't tell the operand size, default to 32.
      self.operand_size = op_dtype_sizes.get(op_dtype, 32)

      # exceptions for operands:
      if 'qsad_' in name:
        self.operand_size = 0
      elif 'sad_' in name:
        self.operand_size = 32
      elif name in ['v_mad_u64_u32', 'v_mad_i64_i32']:
        self.operand_size = 0
      elif self.operand_size == 24:
        self.operand_size = 32
      elif op_dtype == 'u8' or op_dtype == 'i8':
        self.operand_size = 32
      elif name in ['v_cvt_f32_ubyte0', 'v_cvt_f32_ubyte1',
                    'v_cvt_f32_ubyte2', 'v_cvt_f32_ubyte3']:
        self.operand_size = 32


# Matches PhysReg
VCC = 106
M0 = 124
EXEC_LO = 126
EXEC = 127 # Some instructins only write lo, so use exec_hi encoding here
SCC = 253

def src(op1 = 0, op2 = 0, op3 = 0, op4 = 0):
   return op1 | (op2 << 8) | (op3 << 16) | (op4 << 24)

def dst(def1 = 0, def2 = 0, def3 = 0, def4 = 0):
   return def1 | (def2 << 8) | (def3 << 16) | (def4 << 24)

# global dictionary of opcodes
opcodes = {}

def opcode(name, opcode_gfx7 = -1, opcode_gfx9 = -1, opcode_gfx10 = -1, opcode_gfx11 = -1, format = Format.PSEUDO, cls = InstrClass.Other, input_mod = False, output_mod = False, is_atomic = False, definitions = 0, operands = 0):
   assert name not in opcodes
   opcodes[name] = Opcode(name, opcode_gfx7, opcode_gfx9, opcode_gfx10, opcode_gfx11, format, input_mod, output_mod, is_atomic, cls, definitions, operands)

def default_class(opcodes, cls):
   for op in opcodes:
      if isinstance(op[-1], InstrClass):
         yield op
      else:
         yield op + (cls,)

opcode("exp", 0, 0, 0, 0, format = Format.EXP, cls = InstrClass.Export)
opcode("p_parallelcopy")
opcode("p_startpgm")
opcode("p_return")
opcode("p_phi")
opcode("p_linear_phi")
opcode("p_as_uniform")
opcode("p_unit_test")

opcode("p_create_vector")
opcode("p_extract_vector")
opcode("p_split_vector")

# start/end the parts where we can use exec based instructions
# implicitly
opcode("p_logical_start")
opcode("p_logical_end")

# e.g. subgroupMin() in SPIR-V
opcode("p_reduce", format=Format.PSEUDO_REDUCTION)
# e.g. subgroupInclusiveMin()
opcode("p_inclusive_scan", format=Format.PSEUDO_REDUCTION)
# e.g. subgroupExclusiveMin()
opcode("p_exclusive_scan", format=Format.PSEUDO_REDUCTION)

opcode("p_branch", format=Format.PSEUDO_BRANCH)
opcode("p_cbranch", format=Format.PSEUDO_BRANCH)
opcode("p_cbranch_z", format=Format.PSEUDO_BRANCH)
opcode("p_cbranch_nz", format=Format.PSEUDO_BRANCH)

opcode("p_barrier", format=Format.PSEUDO_BARRIER)

# Primitive Ordered Pixel Shading pseudo-instructions.

# For querying whether the current wave can enter the ordered section on GFX9-10.3, doing
# s_add_i32(pops_exiting_wave_id, op0), but in a way that it's different from a usual SALU
# instruction so that it's easier to maintain the volatility of pops_exiting_wave_id and to handle
# the polling specially in scheduling.
# Definitions:
# - Result SGPR;
# - Clobbered SCC.
# Operands:
# - s1 value to add, usually -(current_wave_ID + 1) (or ~current_wave_ID) to remap the exiting wave
#   ID from wrapping [0, 0x3FF] to monotonic [0, 0xFFFFFFFF].
opcode("p_pops_gfx9_add_exiting_wave_id")

# Indicates that the wait for the completion of the ordered section in overlapped waves has been
# finished on GFX9-10.3. Not lowered to any hardware instructions.
opcode("p_pops_gfx9_overlapped_wave_wait_done")

# Indicates that a POPS ordered section has ended, hints that overlapping waves can possibly
# continue execution. The overlapping waves may actually be resumed by this instruction or anywhere
# later, however, especially taking into account the fact that there can be multiple ordered
# sections in a wave (for instance, if one is chosen in divergent control flow in the source
# shader), thus multiple p_pops_gfx9_ordered_section_done instructions. At least one must be present
# in the program if POPS is used, however, otherwise the location of the end of the ordered section
# will be undefined. Only needed on GFX9-10.3 (GFX11+ ordered section is until the last export,
# can't be exited early). Not lowered to any hardware instructions.
opcode("p_pops_gfx9_ordered_section_done")

opcode("p_spill")
opcode("p_reload")

# Start/end linear vgprs. p_start_linear_vgpr can take an operand to copy from, into the linear vgpr
opcode("p_start_linear_vgpr")
opcode("p_end_linear_vgpr")

opcode("p_end_wqm")
opcode("p_discard_if")
opcode("p_demote_to_helper")
opcode("p_is_helper")
opcode("p_exit_early_if")

# simulates proper bpermute behavior using v_readlane_b32
# definitions: result VGPR, temp EXEC, clobbered VCC
# operands: index, input data
opcode("p_bpermute_readlane")

# simulates proper wave64 bpermute behavior using shared vgprs (for GFX10/10.3)
# definitions: result VGPR, temp EXEC, clobbered SCC
# operands: index * 4, input data, same half (bool)
opcode("p_bpermute_shared_vgpr")

# simulates proper wave64 bpermute behavior using v_permlane64_b32 (for GFX11+)
# definitions: result VGPR, temp EXEC, clobbered SCC
# operands: linear VGPR, index * 4, input data, same half (bool)
opcode("p_bpermute_permlane")

# creates a lane mask where only the first active lane is selected
opcode("p_elect")

opcode("p_constaddr")
opcode("p_resume_shader_address")

# These don't have to be pseudo-ops, but it makes optimization easier to only
# have to consider two instructions.
# (src0 >> (index * bits)) & ((1 << bits) - 1) with optional sign extension
opcode("p_extract") # src1=index, src2=bits, src3=signext
# (src0 & ((1 << bits) - 1)) << (index * bits)
opcode("p_insert") # src1=index, src2=bits

opcode("p_init_scratch")

# jumps to a shader epilog
opcode("p_jump_to_epilog")

# loads and interpolates a fragment shader input with a correct exec mask
#dst0=result, src0=linear_vgpr, src1=attribute, src2=component, src3=coord1, src4=coord2, src5=m0
#dst0=result, src0=linear_vgpr, src1=attribute, src2=component, src3=dpp_ctrl, src4=m0
opcode("p_interp_gfx11")

# performs dual source MRTs swizzling and emits exports on GFX11
opcode("p_dual_src_export_gfx11")

# Let shader end with specific registers set to wanted value, used by multi part
# shader to pass arguments to next part.
opcode("p_end_with_regs")

# SOP2 instructions: 2 scalar inputs, 1 scalar output (+optional scc)
SOP2 = {
  # GFX6, GFX7, GFX8, GFX9, GFX10,GFX11,name
   (0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "s_add_u32", dst(1, SCC), src(1, 1)),
   (0x01, 0x01, 0x01, 0x01, 0x01, 0x01, "s_sub_u32", dst(1, SCC), src(1, 1)),
   (0x02, 0x02, 0x02, 0x02, 0x02, 0x02, "s_add_i32", dst(1, SCC), src(1, 1)),
   (0x03, 0x03, 0x03, 0x03, 0x03, 0x03, "s_sub_i32", dst(1, SCC), src(1, 1)),
   (0x04, 0x04, 0x04, 0x04, 0x04, 0x04, "s_addc_u32", dst(1, SCC), src(1, 1, SCC)),
   (0x05, 0x05, 0x05, 0x05, 0x05, 0x05, "s_subb_u32", dst(1, SCC), src(1, 1, SCC)),
   (0x06, 0x06, 0x06, 0x06, 0x06, 0x12, "s_min_i32", dst(1, SCC), src(1, 1)),
   (0x07, 0x07, 0x07, 0x07, 0x07, 0x13, "s_min_u32", dst(1, SCC), src(1, 1)),
   (0x08, 0x08, 0x08, 0x08, 0x08, 0x14, "s_max_i32", dst(1, SCC), src(1, 1)),
   (0x09, 0x09, 0x09, 0x09, 0x09, 0x15, "s_max_u32", dst(1, SCC), src(1, 1)),
   (0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x30, "s_cselect_b32", dst(1), src(1, 1, SCC)),
   (0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x31, "s_cselect_b64", dst(2), src(2, 2, SCC)),
   (0x0e, 0x0e, 0x0c, 0x0c, 0x0e, 0x16, "s_and_b32", dst(1, SCC), src(1, 1)),
   (0x0f, 0x0f, 0x0d, 0x0d, 0x0f, 0x17, "s_and_b64", dst(2, SCC), src(2, 2)),
   (0x10, 0x10, 0x0e, 0x0e, 0x10, 0x18, "s_or_b32", dst(1, SCC), src(1, 1)),
   (0x11, 0x11, 0x0f, 0x0f, 0x11, 0x19, "s_or_b64", dst(2, SCC), src(2, 2)),
   (0x12, 0x12, 0x10, 0x10, 0x12, 0x1a, "s_xor_b32", dst(1, SCC), src(1, 1)),
   (0x13, 0x13, 0x11, 0x11, 0x13, 0x1b, "s_xor_b64", dst(2, SCC), src(2, 2)),
   (0x14, 0x14, 0x12, 0x12, 0x14, 0x22, "s_andn2_b32", dst(1, SCC), src(1, 1)), #s_and_not1_b32 in GFX11
   (0x15, 0x15, 0x13, 0x13, 0x15, 0x23, "s_andn2_b64", dst(2, SCC), src(2, 2)), #s_and_not1_b64 in GFX11
   (0x16, 0x16, 0x14, 0x14, 0x16, 0x24, "s_orn2_b32", dst(1, SCC), src(1, 1)), #s_or_not1_b32 in GFX11
   (0x17, 0x17, 0x15, 0x15, 0x17, 0x25, "s_orn2_b64", dst(2, SCC), src(2, 2)), #s_or_not1_b64 in GFX11
   (0x18, 0x18, 0x16, 0x16, 0x18, 0x1c, "s_nand_b32", dst(1, SCC), src(1, 1)),
   (0x19, 0x19, 0x17, 0x17, 0x19, 0x1d, "s_nand_b64", dst(2, SCC), src(2, 2)),
   (0x1a, 0x1a, 0x18, 0x18, 0x1a, 0x1e, "s_nor_b32", dst(1, SCC), src(1, 1)),
   (0x1b, 0x1b, 0x19, 0x19, 0x1b, 0x1f, "s_nor_b64", dst(2, SCC), src(2, 2)),
   (0x1c, 0x1c, 0x1a, 0x1a, 0x1c, 0x20, "s_xnor_b32", dst(1, SCC), src(1, 1)),
   (0x1d, 0x1d, 0x1b, 0x1b, 0x1d, 0x21, "s_xnor_b64", dst(2, SCC), src(2, 2)),
   (0x1e, 0x1e, 0x1c, 0x1c, 0x1e, 0x08, "s_lshl_b32", dst(1, SCC), src(1, 1)),
   (0x1f, 0x1f, 0x1d, 0x1d, 0x1f, 0x09, "s_lshl_b64", dst(2, SCC), src(2, 1)),
   (0x20, 0x20, 0x1e, 0x1e, 0x20, 0x0a, "s_lshr_b32", dst(1, SCC), src(1, 1)),
   (0x21, 0x21, 0x1f, 0x1f, 0x21, 0x0b, "s_lshr_b64", dst(2, SCC), src(2, 1)),
   (0x22, 0x22, 0x20, 0x20, 0x22, 0x0c, "s_ashr_i32", dst(1, SCC), src(1, 1)),
   (0x23, 0x23, 0x21, 0x21, 0x23, 0x0d, "s_ashr_i64", dst(2, SCC), src(2, 1)),
   (0x24, 0x24, 0x22, 0x22, 0x24, 0x2a, "s_bfm_b32", dst(1), src(1, 1)),
   (0x25, 0x25, 0x23, 0x23, 0x25, 0x2b, "s_bfm_b64", dst(2), src(1, 1)),
   (0x26, 0x26, 0x24, 0x24, 0x26, 0x2c, "s_mul_i32", dst(1), src(1, 1)),
   (0x27, 0x27, 0x25, 0x25, 0x27, 0x26, "s_bfe_u32", dst(1, SCC), src(1, 1)),
   (0x28, 0x28, 0x26, 0x26, 0x28, 0x27, "s_bfe_i32", dst(1, SCC), src(1, 1)),
   (0x29, 0x29, 0x27, 0x27, 0x29, 0x28, "s_bfe_u64", dst(2, SCC), src(2, 1)),
   (0x2a, 0x2a, 0x28, 0x28, 0x2a, 0x29, "s_bfe_i64", dst(2, SCC), src(2, 1)),
   (0x2b, 0x2b, 0x29, 0x29,   -1,   -1, "s_cbranch_g_fork", dst(), src(), InstrClass.Branch),
   (0x2c, 0x2c, 0x2a, 0x2a, 0x2c, 0x06, "s_absdiff_i32", dst(1, SCC), src(1, 1)),
   (  -1,   -1, 0x2b, 0x2b,   -1,   -1, "s_rfe_restore_b64", dst(), src(), InstrClass.Branch),
   (  -1,   -1,   -1, 0x2e, 0x2e, 0x0e, "s_lshl1_add_u32", dst(1, SCC), src(1, 1)),
   (  -1,   -1,   -1, 0x2f, 0x2f, 0x0f, "s_lshl2_add_u32", dst(1, SCC), src(1, 1)),
   (  -1,   -1,   -1, 0x30, 0x30, 0x10, "s_lshl3_add_u32", dst(1, SCC), src(1, 1)),
   (  -1,   -1,   -1, 0x31, 0x31, 0x11, "s_lshl4_add_u32", dst(1, SCC), src(1, 1)),
   (  -1,   -1,   -1, 0x32, 0x32, 0x32, "s_pack_ll_b32_b16", dst(1), src(1, 1)),
   (  -1,   -1,   -1, 0x33, 0x33, 0x33, "s_pack_lh_b32_b16", dst(1), src(1, 1)),
   (  -1,   -1,   -1, 0x34, 0x34, 0x34, "s_pack_hh_b32_b16", dst(1), src(1, 1)),
   (  -1,   -1,   -1,   -1,   -1, 0x35, "s_pack_hl_b32_b16", dst(1), src(1, 1)),
   (  -1,   -1,   -1, 0x2c, 0x35, 0x2d, "s_mul_hi_u32", dst(1), src(1, 1)),
   (  -1,   -1,   -1, 0x2d, 0x36, 0x2e, "s_mul_hi_i32", dst(1), src(1, 1)),
   # actually a pseudo-instruction. it's lowered to SALU during assembly though, so it's useful to identify it as a SOP2.
   (  -1,   -1,   -1,   -1,   -1,   -1, "p_constaddr_addlo", dst(1, SCC), src(1, 1, 1)),
   (  -1,   -1,   -1,   -1,   -1,   -1, "p_resumeaddr_addlo", dst(1, SCC), src(1, 1, 1)),
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name, defs, ops, cls) in default_class(SOP2, InstrClass.Salu):
    opcode(name, gfx7, gfx9, gfx10, gfx11, Format.SOP2, cls, definitions = defs, operands = ops)


# SOPK instructions: 0 input (+ imm), 1 output + optional scc
SOPK = {
  # GFX6, GFX7, GFX8, GFX9, GFX10,GFX11,name
   (0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "s_movk_i32", dst(1), src()),
   (  -1,   -1,   -1,   -1, 0x01, 0x01, "s_version", dst(), src()),
   (0x02, 0x02, 0x01, 0x01, 0x02, 0x02, "s_cmovk_i32", dst(1), src(1, SCC)),
   (0x03, 0x03, 0x02, 0x02, 0x03, 0x03, "s_cmpk_eq_i32", dst(SCC), src(1)),
   (0x04, 0x04, 0x03, 0x03, 0x04, 0x04, "s_cmpk_lg_i32", dst(SCC), src(1)),
   (0x05, 0x05, 0x04, 0x04, 0x05, 0x05, "s_cmpk_gt_i32", dst(SCC), src(1)),
   (0x06, 0x06, 0x05, 0x05, 0x06, 0x06, "s_cmpk_ge_i32", dst(SCC), src(1)),
   (0x07, 0x07, 0x06, 0x06, 0x07, 0x07, "s_cmpk_lt_i32", dst(SCC), src(1)),
   (0x08, 0x08, 0x07, 0x07, 0x08, 0x08, "s_cmpk_le_i32", dst(SCC), src(1)),
   (0x09, 0x09, 0x08, 0x08, 0x09, 0x09, "s_cmpk_eq_u32", dst(SCC), src(1)),
   (0x0a, 0x0a, 0x09, 0x09, 0x0a, 0x0a, "s_cmpk_lg_u32", dst(SCC), src(1)),
   (0x0b, 0x0b, 0x0a, 0x0a, 0x0b, 0x0b, "s_cmpk_gt_u32", dst(SCC), src(1)),
   (0x0c, 0x0c, 0x0b, 0x0b, 0x0c, 0x0c, "s_cmpk_ge_u32", dst(SCC), src(1)),
   (0x0d, 0x0d, 0x0c, 0x0c, 0x0d, 0x0d, "s_cmpk_lt_u32", dst(SCC), src(1)),
   (0x0e, 0x0e, 0x0d, 0x0d, 0x0e, 0x0e, "s_cmpk_le_u32", dst(SCC), src(1)),
   (0x0f, 0x0f, 0x0e, 0x0e, 0x0f, 0x0f, "s_addk_i32", dst(1, SCC), src(1)),
   (0x10, 0x10, 0x0f, 0x0f, 0x10, 0x10, "s_mulk_i32", dst(1), src(1)),
   (0x11, 0x11, 0x10, 0x10,   -1,   -1, "s_cbranch_i_fork", dst(), src(), InstrClass.Branch),
   (0x12, 0x12, 0x11, 0x11, 0x12, 0x11, "s_getreg_b32", dst(1), src()),
   (0x13, 0x13, 0x12, 0x12, 0x13, 0x12, "s_setreg_b32", dst(), src(1)),
   (0x15, 0x15, 0x14, 0x14, 0x15, 0x13, "s_setreg_imm32_b32", dst(), src(1)), # requires 32bit literal
   (  -1,   -1, 0x15, 0x15, 0x16, 0x14, "s_call_b64", dst(2), src(), InstrClass.Branch),
   (  -1,   -1,   -1,   -1, 0x17, 0x18, "s_waitcnt_vscnt", dst(), src(1), InstrClass.Waitcnt),
   (  -1,   -1,   -1,   -1, 0x18, 0x19, "s_waitcnt_vmcnt", dst(), src(1), InstrClass.Waitcnt),
   (  -1,   -1,   -1,   -1, 0x19, 0x1a, "s_waitcnt_expcnt", dst(), src(1), InstrClass.Waitcnt),
   (  -1,   -1,   -1,   -1, 0x1a, 0x1b, "s_waitcnt_lgkmcnt", dst(), src(1), InstrClass.Waitcnt),
   (  -1,   -1,   -1,   -1, 0x1b, 0x16, "s_subvector_loop_begin", dst(), src(), InstrClass.Branch),
   (  -1,   -1,   -1,   -1, 0x1c, 0x17, "s_subvector_loop_end", dst(), src(), InstrClass.Branch),
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name, defs, ops, cls) in default_class(SOPK, InstrClass.Salu):
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.SOPK, cls, definitions = defs, operands = ops)


# SOP1 instructions: 1 input, 1 output (+optional SCC)
SOP1 = {
  # GFX6, GFX7, GFX8, GFX9, GFX10,GFX11,name
   (0x03, 0x03, 0x00, 0x00, 0x03, 0x00, "s_mov_b32", dst(1), src(1)),
   (0x04, 0x04, 0x01, 0x01, 0x04, 0x01, "s_mov_b64", dst(2), src(2)),
   (0x05, 0x05, 0x02, 0x02, 0x05, 0x02, "s_cmov_b32", dst(1), src(1, 1, SCC)),
   (0x06, 0x06, 0x03, 0x03, 0x06, 0x03, "s_cmov_b64", dst(2), src(2, 2, SCC)),
   (0x07, 0x07, 0x04, 0x04, 0x07, 0x1e, "s_not_b32", dst(1, SCC), src(1)),
   (0x08, 0x08, 0x05, 0x05, 0x08, 0x1f, "s_not_b64", dst(2, SCC), src(2)),
   (0x09, 0x09, 0x06, 0x06, 0x09, 0x1c, "s_wqm_b32", dst(1, SCC), src(1)),
   (0x0a, 0x0a, 0x07, 0x07, 0x0a, 0x1d, "s_wqm_b64", dst(2, SCC), src(2)),
   (0x0b, 0x0b, 0x08, 0x08, 0x0b, 0x04, "s_brev_b32", dst(1), src(1)),
   (0x0c, 0x0c, 0x09, 0x09, 0x0c, 0x05, "s_brev_b64", dst(2), src(2)),
   (0x0d, 0x0d, 0x0a, 0x0a, 0x0d, 0x16, "s_bcnt0_i32_b32", dst(1, SCC), src(1)),
   (0x0e, 0x0e, 0x0b, 0x0b, 0x0e, 0x17, "s_bcnt0_i32_b64", dst(1, SCC), src(2)),
   (0x0f, 0x0f, 0x0c, 0x0c, 0x0f, 0x18, "s_bcnt1_i32_b32", dst(1, SCC), src(1)),
   (0x10, 0x10, 0x0d, 0x0d, 0x10, 0x19, "s_bcnt1_i32_b64", dst(1, SCC), src(2)),
   (0x11, 0x11, 0x0e, 0x0e, 0x11,   -1, "s_ff0_i32_b32", dst(1), src(1)),
   (0x12, 0x12, 0x0f, 0x0f, 0x12,   -1, "s_ff0_i32_b64", dst(1), src(2)),
   (0x13, 0x13, 0x10, 0x10, 0x13, 0x08, "s_ff1_i32_b32", dst(1), src(1)), #s_ctz_i32_b32 in GFX11
   (0x14, 0x14, 0x11, 0x11, 0x14, 0x09, "s_ff1_i32_b64", dst(1), src(2)), #s_ctz_i32_b64 in GFX11
   (0x15, 0x15, 0x12, 0x12, 0x15, 0x0a, "s_flbit_i32_b32", dst(1), src(1)), #s_clz_i32_u32 in GFX11
   (0x16, 0x16, 0x13, 0x13, 0x16, 0x0b, "s_flbit_i32_b64", dst(1), src(2)), #s_clz_i32_u64 in GFX11
   (0x17, 0x17, 0x14, 0x14, 0x17, 0x0c, "s_flbit_i32", dst(1), src(1)), #s_cls_i32 in GFX11
   (0x18, 0x18, 0x15, 0x15, 0x18, 0x0d, "s_flbit_i32_i64", dst(1), src(2)), #s_cls_i32_i64 in GFX11
   (0x19, 0x19, 0x16, 0x16, 0x19, 0x0e, "s_sext_i32_i8", dst(1), src(1)),
   (0x1a, 0x1a, 0x17, 0x17, 0x1a, 0x0f, "s_sext_i32_i16", dst(1), src(1)),
   (0x1b, 0x1b, 0x18, 0x18, 0x1b, 0x10, "s_bitset0_b32", dst(1), src(1, 1)),
   (0x1c, 0x1c, 0x19, 0x19, 0x1c, 0x11, "s_bitset0_b64", dst(2), src(1, 2)),
   (0x1d, 0x1d, 0x1a, 0x1a, 0x1d, 0x12, "s_bitset1_b32", dst(1), src(1, 1)),
   (0x1e, 0x1e, 0x1b, 0x1b, 0x1e, 0x13, "s_bitset1_b64", dst(2), src(1, 2)),
   (0x1f, 0x1f, 0x1c, 0x1c, 0x1f, 0x47, "s_getpc_b64", dst(2), src()),
   (0x20, 0x20, 0x1d, 0x1d, 0x20, 0x48, "s_setpc_b64", dst(), src(2), InstrClass.Branch),
   (0x21, 0x21, 0x1e, 0x1e, 0x21, 0x49, "s_swappc_b64", dst(2), src(2), InstrClass.Branch),
   (0x22, 0x22, 0x1f, 0x1f, 0x22, 0x4a, "s_rfe_b64", dst(), src(2), InstrClass.Branch),
   (0x24, 0x24, 0x20, 0x20, 0x24, 0x21, "s_and_saveexec_b64", dst(2, SCC, EXEC), src(2, EXEC)),
   (0x25, 0x25, 0x21, 0x21, 0x25, 0x23, "s_or_saveexec_b64", dst(2, SCC, EXEC), src(2, EXEC)),
   (0x26, 0x26, 0x22, 0x22, 0x26, 0x25, "s_xor_saveexec_b64", dst(2, SCC, EXEC), src(2, EXEC)),
   (0x27, 0x27, 0x23, 0x23, 0x27, 0x31, "s_andn2_saveexec_b64", dst(2, SCC, EXEC), src(2, EXEC)), #s_and_not1_saveexec_b64 in GFX11
   (0x28, 0x28, 0x24, 0x24, 0x28, 0x33, "s_orn2_saveexec_b64", dst(2, SCC, EXEC), src(2, EXEC)), #s_or_not1_saveexec_b64 in GFX11
   (0x29, 0x29, 0x25, 0x25, 0x29, 0x27, "s_nand_saveexec_b64", dst(2, SCC, EXEC), src(2, EXEC)),
   (0x2a, 0x2a, 0x26, 0x26, 0x2a, 0x29, "s_nor_saveexec_b64", dst(2, SCC, EXEC), src(2, EXEC)),
   (0x2b, 0x2b, 0x27, 0x27, 0x2b, 0x2b, "s_xnor_saveexec_b64", dst(2, SCC, EXEC), src(2, EXEC)),
   (0x2c, 0x2c, 0x28, 0x28, 0x2c, 0x1a, "s_quadmask_b32", dst(1, SCC), src(1)),
   (0x2d, 0x2d, 0x29, 0x29, 0x2d, 0x1b, "s_quadmask_b64", dst(2, SCC), src(2)), # Always writes 0 to the second SGPR
   (0x2e, 0x2e, 0x2a, 0x2a, 0x2e, 0x40, "s_movrels_b32", dst(1), src(1, M0)),
   (0x2f, 0x2f, 0x2b, 0x2b, 0x2f, 0x41, "s_movrels_b64", dst(2), src(2, M0)),
   (0x30, 0x30, 0x2c, 0x2c, 0x30, 0x42, "s_movreld_b32", dst(1), src(1, M0)),
   (0x31, 0x31, 0x2d, 0x2d, 0x31, 0x43, "s_movreld_b64", dst(2), src(2, M0)),
   (0x32, 0x32, 0x2e, 0x2e,   -1,   -1, "s_cbranch_join", dst(), src(), InstrClass.Branch),
   (0x34, 0x34, 0x30, 0x30, 0x34, 0x15, "s_abs_i32", dst(1, SCC), src(1)),
   (0x35, 0x35,   -1,   -1, 0x35,   -1, "s_mov_fed_b32", dst(), src()),
   (  -1,   -1, 0x32, 0x32,   -1,   -1, "s_set_gpr_idx_idx", dst(M0), src(1, M0)),
   (  -1,   -1,   -1, 0x33, 0x37, 0x2d, "s_andn1_saveexec_b64", dst(2, SCC, EXEC), src(2, EXEC)), #s_and_not0_savexec_b64 in GFX11
   (  -1,   -1,   -1, 0x34, 0x38, 0x2f, "s_orn1_saveexec_b64", dst(2, SCC, EXEC), src(2, EXEC)), #s_or_not0_savexec_b64 in GFX11
   (  -1,   -1,   -1, 0x35, 0x39, 0x35, "s_andn1_wrexec_b64", dst(2, SCC, EXEC), src(2, EXEC)), #s_and_not0_wrexec_b64 in GFX11
   (  -1,   -1,   -1, 0x36, 0x3a, 0x37, "s_andn2_wrexec_b64", dst(2, SCC, EXEC), src(2, EXEC)), #s_and_not1_wrexec_b64 in GFX11
   (  -1,   -1,   -1, 0x37, 0x3b, 0x14, "s_bitreplicate_b64_b32", dst(2), src(1)),
   (  -1,   -1,   -1,   -1, 0x3c, 0x20, "s_and_saveexec_b32", dst(1, SCC, EXEC_LO), src(1, EXEC_LO)),
   (  -1,   -1,   -1,   -1, 0x3d, 0x22, "s_or_saveexec_b32", dst(1, SCC, EXEC_LO), src(1, EXEC_LO)),
   (  -1,   -1,   -1,   -1, 0x3e, 0x24, "s_xor_saveexec_b32", dst(1, SCC, EXEC_LO), src(1, EXEC_LO)),
   (  -1,   -1,   -1,   -1, 0x3f, 0x30, "s_andn2_saveexec_b32", dst(1, SCC, EXEC_LO), src(1, EXEC_LO)), #s_and_not1_saveexec_b32 in GFX11
   (  -1,   -1,   -1,   -1, 0x40, 0x32, "s_orn2_saveexec_b32", dst(1, SCC, EXEC_LO), src(1, EXEC_LO)), #s_or_not1_saveexec_b32 in GFX11
   (  -1,   -1,   -1,   -1, 0x41, 0x26, "s_nand_saveexec_b32", dst(1, SCC, EXEC_LO), src(1, EXEC_LO)),
   (  -1,   -1,   -1,   -1, 0x42, 0x28, "s_nor_saveexec_b32", dst(1, SCC, EXEC_LO), src(1, EXEC_LO)),
   (  -1,   -1,   -1,   -1, 0x43, 0x2a, "s_xnor_saveexec_b32", dst(1, SCC, EXEC_LO), src(1, EXEC_LO)),
   (  -1,   -1,   -1,   -1, 0x44, 0x2c, "s_andn1_saveexec_b32", dst(1, SCC, EXEC_LO), src(1, EXEC_LO)), #s_and_not0_savexec_b32 in GFX11
   (  -1,   -1,   -1,   -1, 0x45, 0x2e, "s_orn1_saveexec_b32", dst(1, SCC, EXEC_LO), src(1, EXEC_LO)), #s_or_not0_savexec_b32 in GFX11
   (  -1,   -1,   -1,   -1, 0x46, 0x34, "s_andn1_wrexec_b32", dst(1, SCC, EXEC_LO), src(1, EXEC_LO)), #s_and_not0_wrexec_b32 in GFX11
   (  -1,   -1,   -1,   -1, 0x47, 0x36, "s_andn2_wrexec_b32", dst(1, SCC, EXEC_LO), src(1, EXEC_LO)), #s_and_not1_wrexec_b32 in GFX11
   (  -1,   -1,   -1,   -1, 0x49, 0x44, "s_movrelsd_2_b32", dst(1), src(1, M0)),
   (  -1,   -1,   -1,   -1,   -1, 0x4c, "s_sendmsg_rtn_b32", dst(1), src(1)),
   (  -1,   -1,   -1,   -1,   -1, 0x4d, "s_sendmsg_rtn_b64", dst(2), src(1)),
   # actually a pseudo-instruction. it's lowered to SALU during assembly though, so it's useful to identify it as a SOP1.
   (  -1,   -1,   -1,   -1,   -1,   -1, "p_constaddr_getpc", dst(2), src(1)),
   (  -1,   -1,   -1,   -1,   -1,   -1, "p_resumeaddr_getpc", dst(2), src(1)),
   (  -1,   -1,   -1,   -1,   -1,   -1, "p_load_symbol", dst(1), src(1)),
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name, defs, ops, cls) in default_class(SOP1, InstrClass.Salu):
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.SOP1, cls, definitions = defs, operands = ops)


# SOPC instructions: 2 inputs and 0 outputs (+SCC)
SOPC = {
  # GFX6, GFX7, GFX8, GFX9, GFX10,GFX11,name
   (0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "s_cmp_eq_i32", dst(SCC), src(1, 1)),
   (0x01, 0x01, 0x01, 0x01, 0x01, 0x01, "s_cmp_lg_i32", dst(SCC), src(1, 1)),
   (0x02, 0x02, 0x02, 0x02, 0x02, 0x02, "s_cmp_gt_i32", dst(SCC), src(1, 1)),
   (0x03, 0x03, 0x03, 0x03, 0x03, 0x03, "s_cmp_ge_i32", dst(SCC), src(1, 1)),
   (0x04, 0x04, 0x04, 0x04, 0x04, 0x04, "s_cmp_lt_i32", dst(SCC), src(1, 1)),
   (0x05, 0x05, 0x05, 0x05, 0x05, 0x05, "s_cmp_le_i32", dst(SCC), src(1, 1)),
   (0x06, 0x06, 0x06, 0x06, 0x06, 0x06, "s_cmp_eq_u32", dst(SCC), src(1, 1)),
   (0x07, 0x07, 0x07, 0x07, 0x07, 0x07, "s_cmp_lg_u32", dst(SCC), src(1, 1)),
   (0x08, 0x08, 0x08, 0x08, 0x08, 0x08, "s_cmp_gt_u32", dst(SCC), src(1, 1)),
   (0x09, 0x09, 0x09, 0x09, 0x09, 0x09, "s_cmp_ge_u32", dst(SCC), src(1, 1)),
   (0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, "s_cmp_lt_u32", dst(SCC), src(1, 1)),
   (0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, "s_cmp_le_u32", dst(SCC), src(1, 1)),
   (0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, "s_bitcmp0_b32", dst(SCC), src(1, 1)),
   (0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, "s_bitcmp1_b32", dst(SCC), src(1, 1)),
   (0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, "s_bitcmp0_b64", dst(SCC), src(2, 1)),
   (0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, "s_bitcmp1_b64", dst(SCC), src(2, 1)),
   (0x10, 0x10, 0x10, 0x10,   -1,   -1, "s_setvskip", dst(), src(1, 1)),
   (  -1,   -1, 0x11, 0x11,   -1,   -1, "s_set_gpr_idx_on", dst(M0), src(1, 1, M0)),
   (  -1,   -1, 0x12, 0x12, 0x12, 0x10, "s_cmp_eq_u64", dst(SCC), src(2, 2)),
   (  -1,   -1, 0x13, 0x13, 0x13, 0x11, "s_cmp_lg_u64", dst(SCC), src(2, 2)),
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name, defs, ops) in SOPC:
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.SOPC, InstrClass.Salu, definitions = defs, operands = ops)


# SOPP instructions: 0 inputs (+optional scc/vcc), 0 outputs
SOPP = {
  # GFX6, GFX7, GFX8, GFX9, GFX10,GFX11,name
   (0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "s_nop", dst(), src()),
   (0x01, 0x01, 0x01, 0x01, 0x01, 0x30, "s_endpgm", dst(), src()),
   (0x02, 0x02, 0x02, 0x02, 0x02, 0x20, "s_branch", dst(), src(), InstrClass.Branch),
   (  -1,   -1, 0x03, 0x03, 0x03, 0x34, "s_wakeup", dst(), src()),
   (0x04, 0x04, 0x04, 0x04, 0x04, 0x21, "s_cbranch_scc0", dst(), src(), InstrClass.Branch),
   (0x05, 0x05, 0x05, 0x05, 0x05, 0x22, "s_cbranch_scc1", dst(), src(), InstrClass.Branch),
   (0x06, 0x06, 0x06, 0x06, 0x06, 0x23, "s_cbranch_vccz", dst(), src(), InstrClass.Branch),
   (0x07, 0x07, 0x07, 0x07, 0x07, 0x24, "s_cbranch_vccnz", dst(), src(), InstrClass.Branch),
   (0x08, 0x08, 0x08, 0x08, 0x08, 0x25, "s_cbranch_execz", dst(), src(), InstrClass.Branch),
   (0x09, 0x09, 0x09, 0x09, 0x09, 0x26, "s_cbranch_execnz", dst(), src(), InstrClass.Branch),
   (0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x3d, "s_barrier", dst(), src(), InstrClass.Barrier),
   (  -1, 0x0b, 0x0b, 0x0b, 0x0b, 0x01, "s_setkill", dst(), src()),
   (0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x09, "s_waitcnt", dst(), src(), InstrClass.Waitcnt),
   (0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x02, "s_sethalt", dst(), src()),
   (0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x03, "s_sleep", dst(), src()),
   (0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x35, "s_setprio", dst(), src()),
   (0x10, 0x10, 0x10, 0x10, 0x10, 0x36, "s_sendmsg", dst(), src(), InstrClass.Sendmsg),
   (0x11, 0x11, 0x11, 0x11, 0x11, 0x37, "s_sendmsghalt", dst(), src(), InstrClass.Sendmsg),
   (0x12, 0x12, 0x12, 0x12, 0x12, 0x10, "s_trap", dst(), src(), InstrClass.Branch),
   (0x13, 0x13, 0x13, 0x13, 0x13, 0x3c, "s_icache_inv", dst(), src()),
   (0x14, 0x14, 0x14, 0x14, 0x14, 0x38, "s_incperflevel", dst(), src()),
   (0x15, 0x15, 0x15, 0x15, 0x15, 0x39, "s_decperflevel", dst(), src()),
   (0x16, 0x16, 0x16, 0x16, 0x16, 0x3a, "s_ttracedata", dst(), src(M0)),
   (  -1, 0x17, 0x17, 0x17, 0x17, 0x27, "s_cbranch_cdbgsys", dst(), src(), InstrClass.Branch),
   (  -1, 0x18, 0x18, 0x18, 0x18, 0x28, "s_cbranch_cdbguser", dst(), src(), InstrClass.Branch),
   (  -1, 0x19, 0x19, 0x19, 0x19, 0x29, "s_cbranch_cdbgsys_or_user", dst(), src(), InstrClass.Branch),
   (  -1, 0x1a, 0x1a, 0x1a, 0x1a, 0x2a, "s_cbranch_cdbgsys_and_user", dst(), src(), InstrClass.Branch),
   (  -1,   -1, 0x1b, 0x1b, 0x1b, 0x31, "s_endpgm_saved", dst(), src()),
   (  -1,   -1, 0x1c, 0x1c,   -1,   -1, "s_set_gpr_idx_off", dst(), src()),
   (  -1,   -1, 0x1d, 0x1d,   -1,   -1, "s_set_gpr_idx_mode", dst(M0), src(M0)),
   (  -1,   -1,   -1, 0x1e, 0x1e,   -1, "s_endpgm_ordered_ps_done", dst(), src()),
   (  -1,   -1,   -1,   -1, 0x1f, 0x1f, "s_code_end", dst(), src()),
   (  -1,   -1,   -1,   -1, 0x20, 0x04, "s_inst_prefetch", dst(), src()), #s_set_inst_prefetch_distance in GFX11
   (  -1,   -1,   -1,   -1, 0x21, 0x05, "s_clause", dst(), src()),
   (  -1,   -1,   -1,   -1, 0x22, 0x0a, "s_wait_idle", dst(), src(), InstrClass.Waitcnt),
   (  -1,   -1,   -1,   -1, 0x23, 0x08, "s_waitcnt_depctr", dst(), src(), InstrClass.Waitcnt),
   (  -1,   -1,   -1,   -1, 0x24, 0x11, "s_round_mode", dst(), src()),
   (  -1,   -1,   -1,   -1, 0x25, 0x12, "s_denorm_mode", dst(), src()),
   (  -1,   -1,   -1,   -1, 0x26, 0x3b, "s_ttracedata_imm", dst(), src()),
   (  -1,   -1,   -1,   -1,   -1, 0x07, "s_delay_alu", dst(), src(), InstrClass.Waitcnt),
   (  -1,   -1,   -1,   -1,   -1, 0x0b, "s_wait_event", dst(), src()),
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name, defs, ops, cls) in default_class(SOPP, InstrClass.Salu):
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.SOPP, cls, definitions = defs, operands = ops)


# SMEM instructions: sbase input (2 sgpr), potentially 2 offset inputs, 1 sdata input/output
# Unlike GFX10, GFX10.3 does not have SMEM store, atomic or scratch instructions
SMEM = {
  # GFX6, GFX7, GFX8, GFX9, GFX10,GFX11,name
   (0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "s_load_dword"), #s_load_b32 in GFX11
   (0x01, 0x01, 0x01, 0x01, 0x01, 0x01, "s_load_dwordx2"), #s_load_b64 in GFX11
   (0x02, 0x02, 0x02, 0x02, 0x02, 0x02, "s_load_dwordx4"), #s_load_b128 in GFX11
   (0x03, 0x03, 0x03, 0x03, 0x03, 0x03, "s_load_dwordx8"), #s_load_b256 in GFX11
   (0x04, 0x04, 0x04, 0x04, 0x04, 0x04, "s_load_dwordx16"), #s_load_b512 in GFX11
   (  -1,   -1,   -1, 0x05, 0x05,   -1, "s_scratch_load_dword"),
   (  -1,   -1,   -1, 0x06, 0x06,   -1, "s_scratch_load_dwordx2"),
   (  -1,   -1,   -1, 0x07, 0x07,   -1, "s_scratch_load_dwordx4"),
   (0x08, 0x08, 0x08, 0x08, 0x08, 0x08, "s_buffer_load_dword"), #s_buffer_load_b32 in GFX11
   (0x09, 0x09, 0x09, 0x09, 0x09, 0x09, "s_buffer_load_dwordx2"), #s_buffer_load_b64 in GFX11
   (0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, "s_buffer_load_dwordx4"), #s_buffer_load_b128 in GFX11
   (0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, "s_buffer_load_dwordx8"), #s_buffer_load_b256 in GFX11
   (0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, "s_buffer_load_dwordx16"), #s_buffer_load_b512 in GFX11
   (  -1,   -1, 0x10, 0x10, 0x10,   -1, "s_store_dword"),
   (  -1,   -1, 0x11, 0x11, 0x11,   -1, "s_store_dwordx2"),
   (  -1,   -1, 0x12, 0x12, 0x12,   -1, "s_store_dwordx4"),
   (  -1,   -1,   -1, 0x15, 0x15,   -1, "s_scratch_store_dword"),
   (  -1,   -1,   -1, 0x16, 0x16,   -1, "s_scratch_store_dwordx2"),
   (  -1,   -1,   -1, 0x17, 0x17,   -1, "s_scratch_store_dwordx4"),
   (  -1,   -1, 0x18, 0x18, 0x18,   -1, "s_buffer_store_dword"),
   (  -1,   -1, 0x19, 0x19, 0x19,   -1, "s_buffer_store_dwordx2"),
   (  -1,   -1, 0x1a, 0x1a, 0x1a,   -1, "s_buffer_store_dwordx4"),
   (  -1,   -1, 0x1f, 0x1f, 0x1f, 0x20, "s_gl1_inv"),
   (0x1f, 0x1f, 0x20, 0x20, 0x20, 0x21, "s_dcache_inv"),
   (  -1,   -1, 0x21, 0x21, 0x21,   -1, "s_dcache_wb"),
   (  -1, 0x1d, 0x22, 0x22,   -1,   -1, "s_dcache_inv_vol"),
   (  -1,   -1, 0x23, 0x23,   -1,   -1, "s_dcache_wb_vol"),
   (0x1e, 0x1e, 0x24, 0x24, 0x24,   -1, "s_memtime"), #GFX6-GFX10
   (  -1,   -1, 0x25, 0x25, 0x25,   -1, "s_memrealtime"),
   (  -1,   -1, 0x26, 0x26, 0x26, 0x22, "s_atc_probe"),
   (  -1,   -1, 0x27, 0x27, 0x27, 0x23, "s_atc_probe_buffer"),
   (  -1,   -1,   -1, 0x28, 0x28,   -1, "s_dcache_discard"),
   (  -1,   -1,   -1, 0x29, 0x29,   -1, "s_dcache_discard_x2"),
   (  -1,   -1,   -1,   -1, 0x2a,   -1, "s_get_waveid_in_workgroup"),
   (  -1,   -1,   -1, 0x40, 0x40,   -1, "s_buffer_atomic_swap"),
   (  -1,   -1,   -1, 0x41, 0x41,   -1, "s_buffer_atomic_cmpswap"),
   (  -1,   -1,   -1, 0x42, 0x42,   -1, "s_buffer_atomic_add"),
   (  -1,   -1,   -1, 0x43, 0x43,   -1, "s_buffer_atomic_sub"),
   (  -1,   -1,   -1, 0x44, 0x44,   -1, "s_buffer_atomic_smin"),
   (  -1,   -1,   -1, 0x45, 0x45,   -1, "s_buffer_atomic_umin"),
   (  -1,   -1,   -1, 0x46, 0x46,   -1, "s_buffer_atomic_smax"),
   (  -1,   -1,   -1, 0x47, 0x47,   -1, "s_buffer_atomic_umax"),
   (  -1,   -1,   -1, 0x48, 0x48,   -1, "s_buffer_atomic_and"),
   (  -1,   -1,   -1, 0x49, 0x49,   -1, "s_buffer_atomic_or"),
   (  -1,   -1,   -1, 0x4a, 0x4a,   -1, "s_buffer_atomic_xor"),
   (  -1,   -1,   -1, 0x4b, 0x4b,   -1, "s_buffer_atomic_inc"),
   (  -1,   -1,   -1, 0x4c, 0x4c,   -1, "s_buffer_atomic_dec"),
   (  -1,   -1,   -1, 0x60, 0x60,   -1, "s_buffer_atomic_swap_x2"),
   (  -1,   -1,   -1, 0x61, 0x61,   -1, "s_buffer_atomic_cmpswap_x2"),
   (  -1,   -1,   -1, 0x62, 0x62,   -1, "s_buffer_atomic_add_x2"),
   (  -1,   -1,   -1, 0x63, 0x63,   -1, "s_buffer_atomic_sub_x2"),
   (  -1,   -1,   -1, 0x64, 0x64,   -1, "s_buffer_atomic_smin_x2"),
   (  -1,   -1,   -1, 0x65, 0x65,   -1, "s_buffer_atomic_umin_x2"),
   (  -1,   -1,   -1, 0x66, 0x66,   -1, "s_buffer_atomic_smax_x2"),
   (  -1,   -1,   -1, 0x67, 0x67,   -1, "s_buffer_atomic_umax_x2"),
   (  -1,   -1,   -1, 0x68, 0x68,   -1, "s_buffer_atomic_and_x2"),
   (  -1,   -1,   -1, 0x69, 0x69,   -1, "s_buffer_atomic_or_x2"),
   (  -1,   -1,   -1, 0x6a, 0x6a,   -1, "s_buffer_atomic_xor_x2"),
   (  -1,   -1,   -1, 0x6b, 0x6b,   -1, "s_buffer_atomic_inc_x2"),
   (  -1,   -1,   -1, 0x6c, 0x6c,   -1, "s_buffer_atomic_dec_x2"),
   (  -1,   -1,   -1, 0x80, 0x80,   -1, "s_atomic_swap"),
   (  -1,   -1,   -1, 0x81, 0x81,   -1, "s_atomic_cmpswap"),
   (  -1,   -1,   -1, 0x82, 0x82,   -1, "s_atomic_add"),
   (  -1,   -1,   -1, 0x83, 0x83,   -1, "s_atomic_sub"),
   (  -1,   -1,   -1, 0x84, 0x84,   -1, "s_atomic_smin"),
   (  -1,   -1,   -1, 0x85, 0x85,   -1, "s_atomic_umin"),
   (  -1,   -1,   -1, 0x86, 0x86,   -1, "s_atomic_smax"),
   (  -1,   -1,   -1, 0x87, 0x87,   -1, "s_atomic_umax"),
   (  -1,   -1,   -1, 0x88, 0x88,   -1, "s_atomic_and"),
   (  -1,   -1,   -1, 0x89, 0x89,   -1, "s_atomic_or"),
   (  -1,   -1,   -1, 0x8a, 0x8a,   -1, "s_atomic_xor"),
   (  -1,   -1,   -1, 0x8b, 0x8b,   -1, "s_atomic_inc"),
   (  -1,   -1,   -1, 0x8c, 0x8c,   -1, "s_atomic_dec"),
   (  -1,   -1,   -1, 0xa0, 0xa0,   -1, "s_atomic_swap_x2"),
   (  -1,   -1,   -1, 0xa1, 0xa1,   -1, "s_atomic_cmpswap_x2"),
   (  -1,   -1,   -1, 0xa2, 0xa2,   -1, "s_atomic_add_x2"),
   (  -1,   -1,   -1, 0xa3, 0xa3,   -1, "s_atomic_sub_x2"),
   (  -1,   -1,   -1, 0xa4, 0xa4,   -1, "s_atomic_smin_x2"),
   (  -1,   -1,   -1, 0xa5, 0xa5,   -1, "s_atomic_umin_x2"),
   (  -1,   -1,   -1, 0xa6, 0xa6,   -1, "s_atomic_smax_x2"),
   (  -1,   -1,   -1, 0xa7, 0xa7,   -1, "s_atomic_umax_x2"),
   (  -1,   -1,   -1, 0xa8, 0xa8,   -1, "s_atomic_and_x2"),
   (  -1,   -1,   -1, 0xa9, 0xa9,   -1, "s_atomic_or_x2"),
   (  -1,   -1,   -1, 0xaa, 0xaa,   -1, "s_atomic_xor_x2"),
   (  -1,   -1,   -1, 0xab, 0xab,   -1, "s_atomic_inc_x2"),
   (  -1,   -1,   -1, 0xac, 0xac,   -1, "s_atomic_dec_x2"),
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) in SMEM:
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.SMEM, InstrClass.SMem, is_atomic = "atomic" in name)


# VOP2 instructions: 2 inputs, 1 output (+ optional vcc)
# TODO: misses some GFX6_7 opcodes which were shifted to VOP3 in GFX8
VOP2 = {
  # GFX6, GFX7, GFX8, GFX9, GFX10,GFX11,name, input modifiers, output modifiers
   (0x00, 0x00, 0x00, 0x00, 0x01, 0x01, "v_cndmask_b32", True, False, dst(1), src(1, 1, VCC)),
   (0x01, 0x01,   -1,   -1,   -1,   -1, "v_readlane_b32", False, False, dst(1), src(1, 1)),
   (0x02, 0x02,   -1,   -1,   -1,   -1, "v_writelane_b32", False, False, dst(1), src(1, 1, 1)),
   (0x03, 0x03, 0x01, 0x01, 0x03, 0x03, "v_add_f32", True, True, dst(1), src(1, 1)),
   (0x04, 0x04, 0x02, 0x02, 0x04, 0x04, "v_sub_f32", True, True, dst(1), src(1, 1)),
   (0x05, 0x05, 0x03, 0x03, 0x05, 0x05, "v_subrev_f32", True, True, dst(1), src(1, 1)),
   (0x06, 0x06,   -1,   -1, 0x06,   -1, "v_mac_legacy_f32", True, True, dst(1), src(1, 1, 1)), #GFX6,7,10
   (  -1,   -1,   -1,   -1, 0x06, 0x06, "v_fmac_legacy_f32", True, True, dst(1), src(1, 1, 1)), #GFX10.3+, v_fmac_dx9_zero_f32 in GFX11
   (0x07, 0x07, 0x04, 0x04, 0x07, 0x07, "v_mul_legacy_f32", True, True, dst(1), src(1, 1)), #v_mul_dx9_zero_f32 in GFX11
   (0x08, 0x08, 0x05, 0x05, 0x08, 0x08, "v_mul_f32", True, True, dst(1), src(1, 1)),
   (0x09, 0x09, 0x06, 0x06, 0x09, 0x09, "v_mul_i32_i24", False, False, dst(1), src(1, 1)),
   (0x0a, 0x0a, 0x07, 0x07, 0x0a, 0x0a, "v_mul_hi_i32_i24", False, False, dst(1), src(1, 1)),
   (0x0b, 0x0b, 0x08, 0x08, 0x0b, 0x0b, "v_mul_u32_u24", False, False, dst(1), src(1, 1)),
   (0x0c, 0x0c, 0x09, 0x09, 0x0c, 0x0c, "v_mul_hi_u32_u24", False, False, dst(1), src(1, 1)),
   (  -1,   -1,   -1, 0x39, 0x0d,   -1, "v_dot4c_i32_i8", False, False, dst(1), src(1, 1, 1)),
   (0x0d, 0x0d,   -1,   -1,   -1,   -1, "v_min_legacy_f32", True, True, dst(1), src(1, 1)),
   (0x0e, 0x0e,   -1,   -1,   -1,   -1, "v_max_legacy_f32", True, True, dst(1), src(1, 1)),
   (0x0f, 0x0f, 0x0a, 0x0a, 0x0f, 0x0f, "v_min_f32", True, True, dst(1), src(1, 1)),
   (0x10, 0x10, 0x0b, 0x0b, 0x10, 0x10, "v_max_f32", True, True, dst(1), src(1, 1)),
   (0x11, 0x11, 0x0c, 0x0c, 0x11, 0x11, "v_min_i32", False, False, dst(1), src(1, 1)),
   (0x12, 0x12, 0x0d, 0x0d, 0x12, 0x12, "v_max_i32", False, False, dst(1), src(1, 1)),
   (0x13, 0x13, 0x0e, 0x0e, 0x13, 0x13, "v_min_u32", False, False, dst(1), src(1, 1)),
   (0x14, 0x14, 0x0f, 0x0f, 0x14, 0x14, "v_max_u32", False, False, dst(1), src(1, 1)),
   (0x15, 0x15,   -1,   -1,   -1,   -1, "v_lshr_b32", False, False, dst(1), src(1, 1)),
   (0x16, 0x16, 0x10, 0x10, 0x16, 0x19, "v_lshrrev_b32", False, False, dst(1), src(1, 1)),
   (0x17, 0x17,   -1,   -1,   -1,   -1, "v_ashr_i32", False, False, dst(1), src(1, 1)),
   (0x18, 0x18, 0x11, 0x11, 0x18, 0x1a, "v_ashrrev_i32", False, False, dst(1), src(1, 1)),
   (0x19, 0x19,   -1,   -1,   -1,   -1, "v_lshl_b32", False, False, dst(1), src(1, 1)),
   (0x1a, 0x1a, 0x12, 0x12, 0x1a, 0x18, "v_lshlrev_b32", False, False, dst(1), src(1, 1)),
   (0x1b, 0x1b, 0x13, 0x13, 0x1b, 0x1b, "v_and_b32", False, False, dst(1), src(1, 1)),
   (0x1c, 0x1c, 0x14, 0x14, 0x1c, 0x1c, "v_or_b32", False, False, dst(1), src(1, 1)),
   (0x1d, 0x1d, 0x15, 0x15, 0x1d, 0x1d, "v_xor_b32", False, False, dst(1), src(1, 1)),
   (  -1,   -1,   -1,   -1, 0x1e, 0x1e, "v_xnor_b32", False, False, dst(1), src(1, 1)),
   (0x1f, 0x1f, 0x16, 0x16, 0x1f,   -1, "v_mac_f32", True, True, dst(1), src(1, 1, 1)),
   (0x20, 0x20, 0x17, 0x17, 0x20,   -1, "v_madmk_f32", False, False, dst(1), src(1, 1, 1)),
   (0x21, 0x21, 0x18, 0x18, 0x21,   -1, "v_madak_f32", False, False, dst(1), src(1, 1, 1)),
   (0x24, 0x24,   -1,   -1,   -1,   -1, "v_mbcnt_hi_u32_b32", False, False, dst(1), src(1, 1)),
   (0x25, 0x25, 0x19, 0x19,   -1,   -1, "v_add_co_u32", False, False, dst(1, VCC), src(1, 1)), # VOP3B only in RDNA
   (0x26, 0x26, 0x1a, 0x1a,   -1,   -1, "v_sub_co_u32", False, False, dst(1, VCC), src(1, 1)), # VOP3B only in RDNA
   (0x27, 0x27, 0x1b, 0x1b,   -1,   -1, "v_subrev_co_u32", False, False, dst(1, VCC), src(1, 1)), # VOP3B only in RDNA
   (0x28, 0x28, 0x1c, 0x1c, 0x28, 0x20, "v_addc_co_u32", False, False, dst(1, VCC), src(1, 1, VCC)), # v_add_co_ci_u32 in RDNA
   (0x29, 0x29, 0x1d, 0x1d, 0x29, 0x21, "v_subb_co_u32", False, False, dst(1, VCC), src(1, 1, VCC)), # v_sub_co_ci_u32 in RDNA
   (0x2a, 0x2a, 0x1e, 0x1e, 0x2a, 0x22, "v_subbrev_co_u32", False, False, dst(1, VCC), src(1, 1, VCC)), # v_subrev_co_ci_u32 in RDNA
   (  -1,   -1,   -1,   -1, 0x2b, 0x2b, "v_fmac_f32", True, True, dst(1), src(1, 1, 1)),
   (  -1,   -1,   -1,   -1, 0x2c, 0x2c, "v_fmamk_f32", False, False, dst(1), src(1, 1, 1)),
   (  -1,   -1,   -1,   -1, 0x2d, 0x2d, "v_fmaak_f32", False, False, dst(1), src(1, 1, 1)),
   (0x2f, 0x2f,   -1,   -1, 0x2f, 0x2f, "v_cvt_pkrtz_f16_f32", True, False, dst(1), src(1, 1)), #v_cvt_pk_rtz_f16_f32 in GFX11
   (  -1,   -1, 0x1f, 0x1f, 0x32, 0x32, "v_add_f16", True, True, dst(1), src(1, 1)),
   (  -1,   -1, 0x20, 0x20, 0x33, 0x33, "v_sub_f16", True, True, dst(1), src(1, 1)),
   (  -1,   -1, 0x21, 0x21, 0x34, 0x34, "v_subrev_f16", True, True, dst(1), src(1, 1)),
   (  -1,   -1, 0x22, 0x22, 0x35, 0x35, "v_mul_f16", True, True, dst(1), src(1, 1)),
   (  -1,   -1, 0x23, 0x23,   -1,   -1, "v_mac_f16", True, True, dst(1), src(1, 1, 1)),
   (  -1,   -1, 0x24, 0x24,   -1,   -1, "v_madmk_f16", False, False, dst(1), src(1, 1, 1)),
   (  -1,   -1, 0x25, 0x25,   -1,   -1, "v_madak_f16", False, False, dst(1), src(1, 1, 1)),
   (  -1,   -1, 0x26, 0x26,   -1,   -1, "v_add_u16", False, False, dst(1), src(1, 1)),
   (  -1,   -1, 0x27, 0x27,   -1,   -1, "v_sub_u16", False, False, dst(1), src(1, 1)),
   (  -1,   -1, 0x28, 0x28,   -1,   -1, "v_subrev_u16", False, False, dst(1), src(1, 1)),
   (  -1,   -1, 0x29, 0x29,   -1,   -1, "v_mul_lo_u16", False, False, dst(1), src(1, 1)),
   (  -1,   -1, 0x2a, 0x2a,   -1,   -1, "v_lshlrev_b16", False, False, dst(1), src(1, 1)),
   (  -1,   -1, 0x2b, 0x2b,   -1,   -1, "v_lshrrev_b16", False, False, dst(1), src(1, 1)),
   (  -1,   -1, 0x2c, 0x2c,   -1,   -1, "v_ashrrev_i16", False, False, dst(1), src(1, 1)),
   (  -1,   -1, 0x2d, 0x2d, 0x39, 0x39, "v_max_f16", True, True, dst(1), src(1, 1)),
   (  -1,   -1, 0x2e, 0x2e, 0x3a, 0x3a, "v_min_f16", True, True, dst(1), src(1, 1)),
   (  -1,   -1, 0x2f, 0x2f,   -1,   -1, "v_max_u16", False, False, dst(1), src(1, 1)),
   (  -1,   -1, 0x30, 0x30,   -1,   -1, "v_max_i16", False, False, dst(1), src(1, 1)),
   (  -1,   -1, 0x31, 0x31,   -1,   -1, "v_min_u16", False, False, dst(1), src(1, 1)),
   (  -1,   -1, 0x32, 0x32,   -1,   -1, "v_min_i16", False, False, dst(1), src(1, 1)),
   (  -1,   -1, 0x33, 0x33, 0x3b, 0x3b, "v_ldexp_f16", False, True, dst(1), src(1, 1)),
   (  -1,   -1,   -1, 0x34, 0x25, 0x25, "v_add_u32", False, False, dst(1), src(1, 1)), # called v_add_nc_u32 in RDNA
   (  -1,   -1,   -1, 0x35, 0x26, 0x26, "v_sub_u32", False, False, dst(1), src(1, 1)), # called v_sub_nc_u32 in RDNA
   (  -1,   -1,   -1, 0x36, 0x27, 0x27, "v_subrev_u32", False, False, dst(1), src(1, 1)), # called v_subrev_nc_u32 in RDNA
   (  -1,   -1,   -1,   -1, 0x36, 0x36, "v_fmac_f16", True, True, dst(1), src(1, 1, 1)),
   (  -1,   -1,   -1,   -1, 0x37, 0x37, "v_fmamk_f16", False, False, dst(1), src(1, 1, 1)),
   (  -1,   -1,   -1,   -1, 0x38, 0x38, "v_fmaak_f16", False, False, dst(1), src(1, 1, 1)),
   (  -1,   -1,   -1,   -1, 0x3c, 0x3c, "v_pk_fmac_f16", False, False, dst(1), src(1, 1, 1)),
   (  -1,   -1,   -1, 0x37, 0x02, 0x02, "v_dot2c_f32_f16", False, False, dst(1), src(1, 1, 1)), #v_dot2acc_f32_f16 in GFX11
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name, in_mod, out_mod, defs, ops) in VOP2:
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOP2, InstrClass.Valu32, in_mod, out_mod, definitions = defs, operands = ops)


# VOP1 instructions: instructions with 1 input and 1 output
VOP1 = {
  # GFX6, GFX7, GFX8, GFX9, GFX10,GFX11,name, input_modifiers, output_modifiers
   (0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "v_nop", False, False, dst(), src()),
   (0x01, 0x01, 0x01, 0x01, 0x01, 0x01, "v_mov_b32", False, False, dst(1), src(1)),
   (0x02, 0x02, 0x02, 0x02, 0x02, 0x02, "v_readfirstlane_b32", False, False, dst(1), src(1)),
   (0x03, 0x03, 0x03, 0x03, 0x03, 0x03, "v_cvt_i32_f64", True, False, dst(1), src(2), InstrClass.ValuDoubleConvert),
   (0x04, 0x04, 0x04, 0x04, 0x04, 0x04, "v_cvt_f64_i32", False, True, dst(2), src(1), InstrClass.ValuDoubleConvert),
   (0x05, 0x05, 0x05, 0x05, 0x05, 0x05, "v_cvt_f32_i32", False, True, dst(1), src(1)),
   (0x06, 0x06, 0x06, 0x06, 0x06, 0x06, "v_cvt_f32_u32", False, True, dst(1), src(1)),
   (0x07, 0x07, 0x07, 0x07, 0x07, 0x07, "v_cvt_u32_f32", True, False, dst(1), src(1)),
   (0x08, 0x08, 0x08, 0x08, 0x08, 0x08, "v_cvt_i32_f32", True, False, dst(1), src(1)),
   (0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, "v_cvt_f16_f32", True, True, dst(1), src(1)),
   (  -1,   -1,   -1,   -1,   -1,   -1, "p_cvt_f16_f32_rtne", True, True, dst(1), src(1)),
   (0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, "v_cvt_f32_f16", True, True, dst(1), src(1)),
   (0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, "v_cvt_rpi_i32_f32", True, False, dst(1), src(1)), #v_cvt_nearest_i32_f32 in GFX11
   (0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, "v_cvt_flr_i32_f32", True, False, dst(1), src(1)),#v_cvt_floor_i32_f32 in GFX11
   (0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, "v_cvt_off_f32_i4", False, True, dst(1), src(1)),
   (0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, "v_cvt_f32_f64", True, True, dst(1), src(2), InstrClass.ValuDoubleConvert),
   (0x10, 0x10, 0x10, 0x10, 0x10, 0x10, "v_cvt_f64_f32", True, True, dst(2), src(1), InstrClass.ValuDoubleConvert),
   (0x11, 0x11, 0x11, 0x11, 0x11, 0x11, "v_cvt_f32_ubyte0", False, True, dst(1), src(1)),
   (0x12, 0x12, 0x12, 0x12, 0x12, 0x12, "v_cvt_f32_ubyte1", False, True, dst(1), src(1)),
   (0x13, 0x13, 0x13, 0x13, 0x13, 0x13, "v_cvt_f32_ubyte2", False, True, dst(1), src(1)),
   (0x14, 0x14, 0x14, 0x14, 0x14, 0x14, "v_cvt_f32_ubyte3", False, True, dst(1), src(1)),
   (0x15, 0x15, 0x15, 0x15, 0x15, 0x15, "v_cvt_u32_f64", True, False, dst(1), src(2), InstrClass.ValuDoubleConvert),
   (0x16, 0x16, 0x16, 0x16, 0x16, 0x16, "v_cvt_f64_u32", False, True, dst(2), src(1), InstrClass.ValuDoubleConvert),
   (  -1, 0x17, 0x17, 0x17, 0x17, 0x17, "v_trunc_f64", True, True, dst(2), src(2), InstrClass.ValuDouble),
   (  -1, 0x18, 0x18, 0x18, 0x18, 0x18, "v_ceil_f64", True, True, dst(2), src(2), InstrClass.ValuDouble),
   (  -1, 0x19, 0x19, 0x19, 0x19, 0x19, "v_rndne_f64", True, True, dst(2), src(2), InstrClass.ValuDouble),
   (  -1, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, "v_floor_f64", True, True, dst(2), src(2), InstrClass.ValuDouble),
   (  -1,   -1,   -1,   -1, 0x1b, 0x1b, "v_pipeflush", False, False, dst(), src()),
   (0x20, 0x20, 0x1b, 0x1b, 0x20, 0x20, "v_fract_f32", True, True, dst(1), src(1)),
   (0x21, 0x21, 0x1c, 0x1c, 0x21, 0x21, "v_trunc_f32", True, True, dst(1), src(1)),
   (0x22, 0x22, 0x1d, 0x1d, 0x22, 0x22, "v_ceil_f32", True, True, dst(1), src(1)),
   (0x23, 0x23, 0x1e, 0x1e, 0x23, 0x23, "v_rndne_f32", True, True, dst(1), src(1)),
   (0x24, 0x24, 0x1f, 0x1f, 0x24, 0x24, "v_floor_f32", True, True, dst(1), src(1)),
   (0x25, 0x25, 0x20, 0x20, 0x25, 0x25, "v_exp_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x26, 0x26,   -1,   -1,   -1,   -1, "v_log_clamp_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x27, 0x27, 0x21, 0x21, 0x27, 0x27, "v_log_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x28, 0x28,   -1,   -1,   -1,   -1, "v_rcp_clamp_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x29, 0x29,   -1,   -1,   -1,   -1, "v_rcp_legacy_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x2a, 0x2a, 0x22, 0x22, 0x2a, 0x2a, "v_rcp_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x2b, 0x2b, 0x23, 0x23, 0x2b, 0x2b, "v_rcp_iflag_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x2c, 0x2c,   -1,   -1,   -1,   -1, "v_rsq_clamp_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x2d, 0x2d,   -1,   -1,   -1,   -1, "v_rsq_legacy_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x2e, 0x2e, 0x24, 0x24, 0x2e, 0x2e, "v_rsq_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x2f, 0x2f, 0x25, 0x25, 0x2f, 0x2f, "v_rcp_f64", True, True, dst(2), src(2), InstrClass.ValuDoubleTranscendental),
   (0x30, 0x30,   -1,   -1,   -1,   -1, "v_rcp_clamp_f64", True, True, dst(2), src(2), InstrClass.ValuDoubleTranscendental),
   (0x31, 0x31, 0x26, 0x26, 0x31, 0x31, "v_rsq_f64", True, True, dst(2), src(2), InstrClass.ValuDoubleTranscendental),
   (0x32, 0x32,   -1,   -1,   -1,   -1, "v_rsq_clamp_f64", True, True, dst(2), src(2), InstrClass.ValuDoubleTranscendental),
   (0x33, 0x33, 0x27, 0x27, 0x33, 0x33, "v_sqrt_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x34, 0x34, 0x28, 0x28, 0x34, 0x34, "v_sqrt_f64", True, True, dst(2), src(2), InstrClass.ValuDoubleTranscendental),
   (0x35, 0x35, 0x29, 0x29, 0x35, 0x35, "v_sin_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x36, 0x36, 0x2a, 0x2a, 0x36, 0x36, "v_cos_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (0x37, 0x37, 0x2b, 0x2b, 0x37, 0x37, "v_not_b32", False, False, dst(1), src(1)),
   (0x38, 0x38, 0x2c, 0x2c, 0x38, 0x38, "v_bfrev_b32", False, False, dst(1), src(1)),
   (0x39, 0x39, 0x2d, 0x2d, 0x39, 0x39, "v_ffbh_u32", False, False, dst(1), src(1)), #v_clz_i32_u32 in GFX11
   (0x3a, 0x3a, 0x2e, 0x2e, 0x3a, 0x3a, "v_ffbl_b32", False, False, dst(1), src(1)), #v_ctz_i32_b32 in GFX11
   (0x3b, 0x3b, 0x2f, 0x2f, 0x3b, 0x3b, "v_ffbh_i32", False, False, dst(1), src(1)), #v_cls_i32 in GFX11
   (0x3c, 0x3c, 0x30, 0x30, 0x3c, 0x3c, "v_frexp_exp_i32_f64", True, False, dst(1), src(2), InstrClass.ValuDouble),
   (0x3d, 0x3d, 0x31, 0x31, 0x3d, 0x3d, "v_frexp_mant_f64", True, False, dst(2), src(2), InstrClass.ValuDouble),
   (0x3e, 0x3e, 0x32, 0x32, 0x3e, 0x3e, "v_fract_f64", True, True, dst(2), src(2), InstrClass.ValuDouble),
   (0x3f, 0x3f, 0x33, 0x33, 0x3f, 0x3f, "v_frexp_exp_i32_f32", True, False, dst(1), src(1)),
   (0x40, 0x40, 0x34, 0x34, 0x40, 0x40, "v_frexp_mant_f32", True, False, dst(1), src(1)),
   (0x41, 0x41, 0x35, 0x35, 0x41,   -1, "v_clrexcp", False, False, dst(), src()),
   (0x42, 0x42, 0x36,   -1, 0x42, 0x42, "v_movreld_b32", False, False, dst(1), src(1, M0)),
   (0x43, 0x43, 0x37,   -1, 0x43, 0x43, "v_movrels_b32", False, False, dst(1), src(1, M0)),
   (0x44, 0x44, 0x38,   -1, 0x44, 0x44, "v_movrelsd_b32", False, False, dst(1), src(1, M0)),
   (  -1,   -1,   -1,   -1, 0x48, 0x48, "v_movrelsd_2_b32", False, False, dst(1), src(1, M0)),
   (  -1,   -1,   -1, 0x37,   -1,   -1, "v_screen_partition_4se_b32", False, False, dst(1), src(1)),
   (  -1,   -1, 0x39, 0x39, 0x50, 0x50, "v_cvt_f16_u16", False, True, dst(1), src(1)),
   (  -1,   -1, 0x3a, 0x3a, 0x51, 0x51, "v_cvt_f16_i16", False, True, dst(1), src(1)),
   (  -1,   -1, 0x3b, 0x3b, 0x52, 0x52, "v_cvt_u16_f16", True, False, dst(1), src(1)),
   (  -1,   -1, 0x3c, 0x3c, 0x53, 0x53, "v_cvt_i16_f16", True, False, dst(1), src(1)),
   (  -1,   -1, 0x3d, 0x3d, 0x54, 0x54, "v_rcp_f16", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (  -1,   -1, 0x3e, 0x3e, 0x55, 0x55, "v_sqrt_f16", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (  -1,   -1, 0x3f, 0x3f, 0x56, 0x56, "v_rsq_f16", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (  -1,   -1, 0x40, 0x40, 0x57, 0x57, "v_log_f16", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (  -1,   -1, 0x41, 0x41, 0x58, 0x58, "v_exp_f16", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (  -1,   -1, 0x42, 0x42, 0x59, 0x59, "v_frexp_mant_f16", True, False, dst(1), src(1)),
   (  -1,   -1, 0x43, 0x43, 0x5a, 0x5a, "v_frexp_exp_i16_f16", True, False, dst(1), src(1)),
   (  -1,   -1, 0x44, 0x44, 0x5b, 0x5b, "v_floor_f16", True, True, dst(1), src(1)),
   (  -1,   -1, 0x45, 0x45, 0x5c, 0x5c, "v_ceil_f16", True, True, dst(1), src(1)),
   (  -1,   -1, 0x46, 0x46, 0x5d, 0x5d, "v_trunc_f16", True, True, dst(1), src(1)),
   (  -1,   -1, 0x47, 0x47, 0x5e, 0x5e, "v_rndne_f16", True, True, dst(1), src(1)),
   (  -1,   -1, 0x48, 0x48, 0x5f, 0x5f, "v_fract_f16", True, True, dst(1), src(1)),
   (  -1,   -1, 0x49, 0x49, 0x60, 0x60, "v_sin_f16", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (  -1,   -1, 0x4a, 0x4a, 0x61, 0x61, "v_cos_f16", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (  -1, 0x46, 0x4b, 0x4b,   -1,   -1, "v_exp_legacy_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (  -1, 0x45, 0x4c, 0x4c,   -1,   -1, "v_log_legacy_f32", True, True, dst(1), src(1), InstrClass.ValuTranscendental32),
   (  -1,   -1,   -1, 0x4f, 0x62, 0x62, "v_sat_pk_u8_i16", False, False, dst(1), src(1)),
   (  -1,   -1,   -1, 0x4d, 0x63, 0x63, "v_cvt_norm_i16_f16", True, False, dst(1), src(1)),
   (  -1,   -1,   -1, 0x4e, 0x64, 0x64, "v_cvt_norm_u16_f16", True, False, dst(1), src(1)),
   (  -1,   -1,   -1, 0x51, 0x65, 0x65, "v_swap_b32", False, False, dst(1, 1), src(1, 1)),
   (  -1,   -1,   -1,   -1, 0x68, 0x68, "v_swaprel_b32", False, False, dst(1, 1), src(1, 1, M0)),
   (  -1,   -1,   -1,   -1,   -1, 0x67, "v_permlane64_b32", False, False, dst(1), src(1)), #cannot use VOP3
   (  -1,   -1,   -1,   -1,   -1, 0x69, "v_not_b16", False, False, dst(1), src(1)),
   (  -1,   -1,   -1,   -1,   -1, 0x6a, "v_cvt_i32_i16", False, False, dst(1), src(1)),
   (  -1,   -1,   -1,   -1,   -1, 0x6b, "v_cvt_u32_u16", False, False, dst(1), src(1)),
   (  -1,   -1,   -1,   -1,   -1, 0x1c, "v_mov_b16", True, False, dst(1), src(1)),
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name, in_mod, out_mod, defs, ops, cls) in default_class(VOP1, InstrClass.Valu32):
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOP1, cls, in_mod, out_mod, definitions = defs, operands = ops)


# VOPC instructions:

VOPC_CLASS = {
   (0x88, 0x88, 0x10, 0x10, 0x88, 0x7e, "v_cmp_class_f32", dst(VCC), src(1, 1)),
   (  -1,   -1, 0x14, 0x14, 0x8f, 0x7d, "v_cmp_class_f16", dst(VCC), src(1, 1)),
   (0x98, 0x98, 0x11, 0x11, 0x98, 0xfe, "v_cmpx_class_f32", dst(EXEC), src(1, 1)),
   (  -1,   -1, 0x15, 0x15, 0x9f, 0xfd, "v_cmpx_class_f16", dst(EXEC), src(1, 1)),
   (0xa8, 0xa8, 0x12, 0x12, 0xa8, 0x7f, "v_cmp_class_f64", dst(VCC), src(2, 1), InstrClass.ValuDouble),
   (0xb8, 0xb8, 0x13, 0x13, 0xb8, 0xff, "v_cmpx_class_f64", dst(EXEC), src(2, 1), InstrClass.ValuDouble),
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name, defs, ops, cls) in default_class(VOPC_CLASS, InstrClass.Valu32):
    opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, cls, True, False, definitions = defs, operands = ops)

COMPF = ["f", "lt", "eq", "le", "gt", "lg", "ge", "o", "u", "nge", "nlg", "ngt", "nle", "neq", "nlt", "tru"]

for i in range(8):
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, 0x20+i, 0x20+i, 0xc8+i, 0x00+i, "v_cmp_"+COMPF[i]+"_f16")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, True, False, definitions = dst(VCC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, 0x30+i, 0x30+i, 0xd8+i, 0x80+i, "v_cmpx_"+COMPF[i]+"_f16")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, True, False, definitions = dst(EXEC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, 0x28+i, 0x28+i, 0xe8+i, 0x08+i, "v_cmp_"+COMPF[i+8]+"_f16")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, True, False, definitions = dst(VCC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, 0x38+i, 0x38+i, 0xf8+i, 0x88+i, "v_cmpx_"+COMPF[i+8]+"_f16")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, True, False, definitions = dst(EXEC), operands = src(1, 1))

for i in range(16):
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0x00+i, 0x00+i, 0x40+i, 0x40+i, 0x00+i, 0x10+i, "v_cmp_"+COMPF[i]+"_f32")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, True, False, definitions = dst(VCC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0x10+i, 0x10+i, 0x50+i, 0x50+i, 0x10+i, 0x90+i, "v_cmpx_"+COMPF[i]+"_f32")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, True, False, definitions = dst(EXEC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0x20+i, 0x20+i, 0x60+i, 0x60+i, 0x20+i, 0x20+i, "v_cmp_"+COMPF[i]+"_f64")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.ValuDouble, True, False, definitions = dst(VCC), operands = src(2, 2))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0x30+i, 0x30+i, 0x70+i, 0x70+i, 0x30+i, 0xa0+i, "v_cmpx_"+COMPF[i]+"_f64")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.ValuDouble, True, False, definitions = dst(EXEC), operands = src(2, 2))
   # GFX_6_7
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0x40+i, 0x40+i, -1, -1, -1, -1, "v_cmps_"+COMPF[i]+"_f32")
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0x50+i, 0x50+i, -1, -1, -1, -1, "v_cmpsx_"+COMPF[i]+"_f32")
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0x60+i, 0x60+i, -1, -1, -1, -1, "v_cmps_"+COMPF[i]+"_f64")
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0x70+i, 0x70+i, -1, -1, -1, -1, "v_cmpsx_"+COMPF[i]+"_f64")

COMPI = ["f", "lt", "eq", "le", "gt", "lg", "ge", "tru"]

# GFX_8_9
for i in [0,7]: # only 0 and 7
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, 0xa0+i, 0xa0+i, -1, -1, "v_cmp_"+COMPI[i]+"_i16")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, definitions = dst(VCC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, 0xb0+i, 0xb0+i, -1, -1, "v_cmpx_"+COMPI[i]+"_i16")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, definitions = dst(EXEC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, 0xa8+i, 0xa8+i, -1, -1, "v_cmp_"+COMPI[i]+"_u16")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, definitions = dst(VCC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, 0xb8+i, 0xb8+i, -1, -1, "v_cmpx_"+COMPI[i]+"_u16")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, definitions = dst(EXEC), operands = src(1, 1))

for i in range(1, 7): # [1..6]
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, 0xa0+i, 0xa0+i, 0x88+i, 0x30+i, "v_cmp_"+COMPI[i]+"_i16")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, definitions = dst(VCC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, 0xb0+i, 0xb0+i, 0x98+i, 0xb0+i, "v_cmpx_"+COMPI[i]+"_i16")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, definitions = dst(EXEC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, 0xa8+i, 0xa8+i, 0xa8+i, 0x38+i, "v_cmp_"+COMPI[i]+"_u16")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, definitions = dst(VCC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, 0xb8+i, 0xb8+i, 0xb8+i, 0xb8+i, "v_cmpx_"+COMPI[i]+"_u16")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, definitions = dst(EXEC), operands = src(1, 1))

for i in range(8):
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0x80+i, 0x80+i, 0xc0+i, 0xc0+i, 0x80+i, 0x40+i, "v_cmp_"+COMPI[i]+"_i32")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, definitions = dst(VCC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0x90+i, 0x90+i, 0xd0+i, 0xd0+i, 0x90+i, 0xc0+i, "v_cmpx_"+COMPI[i]+"_i32")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, definitions = dst(EXEC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0xa0+i, 0xa0+i, 0xe0+i, 0xe0+i, 0xa0+i, 0x50+i, "v_cmp_"+COMPI[i]+"_i64")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu64, definitions = dst(VCC), operands = src(2, 2))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0xb0+i, 0xb0+i, 0xf0+i, 0xf0+i, 0xb0+i, 0xd0+i, "v_cmpx_"+COMPI[i]+"_i64")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu64, definitions = dst(EXEC), operands = src(2, 2))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0xc0+i, 0xc0+i, 0xc8+i, 0xc8+i, 0xc0+i, 0x48+i, "v_cmp_"+COMPI[i]+"_u32")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, definitions = dst(VCC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0xd0+i, 0xd0+i, 0xd8+i, 0xd8+i, 0xd0+i, 0xc8+i, "v_cmpx_"+COMPI[i]+"_u32")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu32, definitions = dst(EXEC), operands = src(1, 1))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0xe0+i, 0xe0+i, 0xe8+i, 0xe8+i, 0xe0+i, 0x58+i, "v_cmp_"+COMPI[i]+"_u64")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu64, definitions = dst(VCC), operands = src(2, 2))
   (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (0xf0+i, 0xf0+i, 0xf8+i, 0xf8+i, 0xf0+i, 0xd8+i, "v_cmpx_"+COMPI[i]+"_u64")
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOPC, InstrClass.Valu64, definitions = dst(EXEC), operands = src(2, 2))


# VOPP instructions: packed 16bit instructions - 2 or 3 inputs and 1 output
VOPP = {
   # opcode, name, input/output modifiers
   (0x00, "v_pk_mad_i16", False, dst(1), src(1, 1, 1)),
   (0x01, "v_pk_mul_lo_u16", False, dst(1), src(1, 1)),
   (0x02, "v_pk_add_i16", False, dst(1), src(1, 1)),
   (0x03, "v_pk_sub_i16", False, dst(1), src(1, 1)),
   (0x04, "v_pk_lshlrev_b16", False, dst(1), src(1, 1)),
   (0x05, "v_pk_lshrrev_b16", False, dst(1), src(1, 1)),
   (0x06, "v_pk_ashrrev_i16", False, dst(1), src(1, 1)),
   (0x07, "v_pk_max_i16", False, dst(1), src(1, 1)),
   (0x08, "v_pk_min_i16", False, dst(1), src(1, 1)),
   (0x09, "v_pk_mad_u16", False, dst(1), src(1, 1, 1)),
   (0x0a, "v_pk_add_u16", False, dst(1), src(1, 1)),
   (0x0b, "v_pk_sub_u16", False, dst(1), src(1, 1)),
   (0x0c, "v_pk_max_u16", False, dst(1), src(1, 1)),
   (0x0d, "v_pk_min_u16", False, dst(1), src(1, 1)),
   (0x0e, "v_pk_fma_f16", True, dst(1), src(1, 1, 1)),
   (0x0f, "v_pk_add_f16", True, dst(1), src(1, 1)),
   (0x10, "v_pk_mul_f16", True, dst(1), src(1, 1)),
   (0x11, "v_pk_min_f16", True, dst(1), src(1, 1)),
   (0x12, "v_pk_max_f16", True, dst(1), src(1, 1)),
   (0x20, "v_fma_mix_f32", True, dst(1), src(1, 1, 1)), # v_mad_mix_f32 in VEGA ISA, v_fma_mix_f32 in RDNA ISA
   (0x21, "v_fma_mixlo_f16", True, dst(1), src(1, 1, 1)), # v_mad_mixlo_f16 in VEGA ISA, v_fma_mixlo_f16 in RDNA ISA
   (0x22, "v_fma_mixhi_f16", True, dst(1), src(1, 1, 1)), # v_mad_mixhi_f16 in VEGA ISA, v_fma_mixhi_f16 in RDNA ISA
}
# note that these are only supported on gfx9+ so we'll need to distinguish between gfx8 and gfx9 here
# (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, -1, code, code, code, name)
for (code, name, modifiers, defs, ops) in VOPP:
   opcode(name, -1, code, code, code, Format.VOP3P, InstrClass.Valu32, modifiers, modifiers, definitions = defs, operands = ops)
opcode("v_dot2_i32_i16", -1, 0x26, 0x14, -1, Format.VOP3P, InstrClass.Valu32, definitions = dst(1), operands = src(1, 1, 1))
opcode("v_dot2_u32_u16", -1, 0x27, 0x15, -1, Format.VOP3P, InstrClass.Valu32, definitions = dst(1), operands = src(1, 1, 1))
opcode("v_dot4_i32_iu8", -1, -1, -1, 0x16, Format.VOP3P, InstrClass.Valu32, definitions = dst(1), operands = src(1, 1, 1))
opcode("v_dot4_i32_i8", -1, 0x28, 0x16, -1, Format.VOP3P, InstrClass.Valu32, definitions = dst(1), operands = src(1, 1, 1))
opcode("v_dot4_u32_u8", -1, 0x29, 0x17, 0x17, Format.VOP3P, InstrClass.Valu32, definitions = dst(1), operands = src(1, 1, 1))
opcode("v_dot8_i32_iu4", -1, -1, -1, 0x18, Format.VOP3P, InstrClass.Valu32, definitions = dst(1), operands = src(1, 1, 1))
opcode("v_dot8_u32_u4", -1, 0x2b, 0x19, 0x19, Format.VOP3P, InstrClass.Valu32, definitions = dst(1), operands = src(1, 1, 1))
opcode("v_dot2_f32_f16", -1, 0x23, 0x13, 0x13, Format.VOP3P, InstrClass.Valu32, definitions = dst(1), operands = src(1, 1, 1))
opcode("v_dot2_f32_bf16", -1, -1, -1, 0x1a, Format.VOP3P, InstrClass.Valu32, definitions = dst(1), operands = src(1, 1, 1))
opcode("v_wmma_f32_16x16x16_f16", -1, -1, -1, 0x40, Format.VOP3P, InstrClass.WMMA, False, False)
opcode("v_wmma_f32_16x16x16_bf16", -1, -1, -1, 0x41, Format.VOP3P, InstrClass.WMMA, False, False)
opcode("v_wmma_f16_16x16x16_f16", -1, -1, -1, 0x42, Format.VOP3P, InstrClass.WMMA, False, False)
opcode("v_wmma_bf16_16x16x16_bf16", -1, -1, -1, 0x43, Format.VOP3P, InstrClass.WMMA, False, False)
opcode("v_wmma_i32_16x16x16_iu8", -1, -1, -1, 0x44, Format.VOP3P, InstrClass.WMMA, False, False)
opcode("v_wmma_i32_16x16x16_iu4", -1, -1, -1, 0x45, Format.VOP3P, InstrClass.WMMA, False, False)


# VINTRP (GFX6 - GFX10.3) instructions:
VINTRP = {
   (0x00, "v_interp_p1_f32", dst(1), src(1, M0)),
   (0x01, "v_interp_p2_f32", dst(1), src(1, M0, 1)),
   (0x02, "v_interp_mov_f32", dst(1), src(1, M0)),
}
# (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (code, code, code, code, code, code, name)
for (code, name, defs, ops) in VINTRP:
   opcode(name, code, code, code, -1, Format.VINTRP, InstrClass.Valu32, definitions = defs, operands = ops)


# VINTERP (GFX11+) instructions:
VINTERP = {
   (0x00, "v_interp_p10_f32_inreg"),
   (0x01, "v_interp_p2_f32_inreg"),
   (0x02, "v_interp_p10_f16_f32_inreg"),
   (0x03, "v_interp_p2_f16_f32_inreg"),
   (0x04, "v_interp_p10_rtz_f16_f32_inreg"),
   (0x05, "v_interp_p2_rtz_f16_f32_inreg"),
}
for (code, name) in VINTERP:
   opcode(name, -1, -1, -1, code, Format.VINTERP_INREG, InstrClass.Valu32, False, True, definitions = dst(1), operands = src(1, 1, 1))


# VOP3 instructions: 3 inputs, 1 output
# VOP3b instructions: have a unique scalar output, e.g. VOP2 with vcc out
VOP3 = {
   (0x140, 0x140, 0x1c0, 0x1c0, 0x140,    -1, "v_mad_legacy_f32", True, True, dst(1), src(1, 1, 1)), # GFX6-GFX10
   (0x141, 0x141, 0x1c1, 0x1c1, 0x141,    -1, "v_mad_f32", True, True, dst(1), src(1, 1, 1)),
   (0x142, 0x142, 0x1c2, 0x1c2, 0x142, 0x20a, "v_mad_i32_i24", False, False, dst(1), src(1, 1, 1)),
   (0x143, 0x143, 0x1c3, 0x1c3, 0x143, 0x20b, "v_mad_u32_u24", False, False, dst(1), src(1, 1, 1)),
   (0x144, 0x144, 0x1c4, 0x1c4, 0x144, 0x20c, "v_cubeid_f32", True, True, dst(1), src(1, 1, 1)),
   (0x145, 0x145, 0x1c5, 0x1c5, 0x145, 0x20d, "v_cubesc_f32", True, True, dst(1), src(1, 1, 1)),
   (0x146, 0x146, 0x1c6, 0x1c6, 0x146, 0x20e, "v_cubetc_f32", True, True, dst(1), src(1, 1, 1)),
   (0x147, 0x147, 0x1c7, 0x1c7, 0x147, 0x20f, "v_cubema_f32", True, True, dst(1), src(1, 1, 1)),
   (0x148, 0x148, 0x1c8, 0x1c8, 0x148, 0x210, "v_bfe_u32", False, False, dst(1), src(1, 1, 1)),
   (0x149, 0x149, 0x1c9, 0x1c9, 0x149, 0x211, "v_bfe_i32", False, False, dst(1), src(1, 1, 1)),
   (0x14a, 0x14a, 0x1ca, 0x1ca, 0x14a, 0x212, "v_bfi_b32", False, False, dst(1), src(1, 1, 1)),
   (0x14b, 0x14b, 0x1cb, 0x1cb, 0x14b, 0x213, "v_fma_f32", True, True, dst(1), src(1, 1, 1), InstrClass.ValuFma),
   (0x14c, 0x14c, 0x1cc, 0x1cc, 0x14c, 0x214, "v_fma_f64", True, True, dst(2), src(2, 2, 2), InstrClass.ValuDouble),
   (0x14d, 0x14d, 0x1cd, 0x1cd, 0x14d, 0x215, "v_lerp_u8", False, False, dst(1), src(1, 1, 1)),
   (0x14e, 0x14e, 0x1ce, 0x1ce, 0x14e, 0x216, "v_alignbit_b32", False, False, dst(1), src(1, 1, 1)),
   (0x14f, 0x14f, 0x1cf, 0x1cf, 0x14f, 0x217, "v_alignbyte_b32", False, False, dst(1), src(1, 1, 1)),
   (0x150, 0x150,    -1,    -1, 0x150, 0x218, "v_mullit_f32", True, True, dst(1), src(1, 1, 1)),
   (0x151, 0x151, 0x1d0, 0x1d0, 0x151, 0x219, "v_min3_f32", True, True, dst(1), src(1, 1, 1)),
   (0x152, 0x152, 0x1d1, 0x1d1, 0x152, 0x21a, "v_min3_i32", False, False, dst(1), src(1, 1, 1)),
   (0x153, 0x153, 0x1d2, 0x1d2, 0x153, 0x21b, "v_min3_u32", False, False, dst(1), src(1, 1, 1)),
   (0x154, 0x154, 0x1d3, 0x1d3, 0x154, 0x21c, "v_max3_f32", True, True, dst(1), src(1, 1, 1)),
   (0x155, 0x155, 0x1d4, 0x1d4, 0x155, 0x21d, "v_max3_i32", False, False, dst(1), src(1, 1, 1)),
   (0x156, 0x156, 0x1d5, 0x1d5, 0x156, 0x21e, "v_max3_u32", False, False, dst(1), src(1, 1, 1)),
   (0x157, 0x157, 0x1d6, 0x1d6, 0x157, 0x21f, "v_med3_f32", True, True, dst(1), src(1, 1, 1)),
   (0x158, 0x158, 0x1d7, 0x1d7, 0x158, 0x220, "v_med3_i32", False, False, dst(1), src(1, 1, 1)),
   (0x159, 0x159, 0x1d8, 0x1d8, 0x159, 0x221, "v_med3_u32", False, False, dst(1), src(1, 1, 1)),
   (0x15a, 0x15a, 0x1d9, 0x1d9, 0x15a, 0x222, "v_sad_u8", False, False, dst(1), src(1, 1, 1)),
   (0x15b, 0x15b, 0x1da, 0x1da, 0x15b, 0x223, "v_sad_hi_u8", False, False, dst(1), src(1, 1, 1)),
   (0x15c, 0x15c, 0x1db, 0x1db, 0x15c, 0x224, "v_sad_u16", False, False, dst(1), src(1, 1, 1)),
   (0x15d, 0x15d, 0x1dc, 0x1dc, 0x15d, 0x225, "v_sad_u32", False, False, dst(1), src(1, 1, 1)),
   (0x15e, 0x15e, 0x1dd, 0x1dd, 0x15e, 0x226, "v_cvt_pk_u8_f32", True, False, dst(1), src(1, 1, 1)),
   (0x15f, 0x15f, 0x1de, 0x1de, 0x15f, 0x227, "v_div_fixup_f32", True, True, dst(1), src(1, 1, 1)),
   (0x160, 0x160, 0x1df, 0x1df, 0x160, 0x228, "v_div_fixup_f64", True, True, dst(2), src(2, 2, 2)),
   (0x161, 0x161,    -1,    -1,    -1,    -1, "v_lshl_b64", False, False, dst(2), src(2, 1), InstrClass.Valu64),
   (0x162, 0x162,    -1,    -1,    -1,    -1, "v_lshr_b64", False, False, dst(2), src(2, 1), InstrClass.Valu64),
   (0x163, 0x163,    -1,    -1,    -1,    -1, "v_ashr_i64", False, False, dst(2), src(2, 1), InstrClass.Valu64),
   (0x164, 0x164, 0x280, 0x280, 0x164, 0x327, "v_add_f64", True, True, dst(2), src(2, 2), InstrClass.ValuDoubleAdd),
   (0x165, 0x165, 0x281, 0x281, 0x165, 0x328, "v_mul_f64", True, True, dst(2), src(2, 2), InstrClass.ValuDouble),
   (0x166, 0x166, 0x282, 0x282, 0x166, 0x329, "v_min_f64", True, True, dst(2), src(2, 2), InstrClass.ValuDouble),
   (0x167, 0x167, 0x283, 0x283, 0x167, 0x32a, "v_max_f64", True, True, dst(2), src(2, 2), InstrClass.ValuDouble),
   (0x168, 0x168, 0x284, 0x284, 0x168, 0x32b, "v_ldexp_f64", False, True, dst(2), src(2, 1), InstrClass.ValuDouble), # src1 can take input modifiers
   (0x169, 0x169, 0x285, 0x285, 0x169, 0x32c, "v_mul_lo_u32", False, False, dst(1), src(1, 1), InstrClass.ValuQuarterRate32),
   (0x16a, 0x16a, 0x286, 0x286, 0x16a, 0x32d, "v_mul_hi_u32", False, False, dst(1), src(1, 1), InstrClass.ValuQuarterRate32),
   (0x16b, 0x16b, 0x285, 0x285, 0x16b, 0x32c, "v_mul_lo_i32", False, False, dst(1), src(1, 1), InstrClass.ValuQuarterRate32), # identical to v_mul_lo_u32
   (0x16c, 0x16c, 0x287, 0x287, 0x16c, 0x32e, "v_mul_hi_i32", False, False, dst(1), src(1, 1), InstrClass.ValuQuarterRate32),
   (0x16d, 0x16d, 0x1e0, 0x1e0, 0x16d, 0x2fc, "v_div_scale_f32", True, True, dst(1, VCC), src(1, 1, 1)),
   (0x16e, 0x16e, 0x1e1, 0x1e1, 0x16e, 0x2fd, "v_div_scale_f64", True, True, dst(2, VCC), src(2, 2, 2), InstrClass.ValuDouble),
   (0x16f, 0x16f, 0x1e2, 0x1e2, 0x16f, 0x237, "v_div_fmas_f32", True, True, dst(1), src(1, 1, 1, VCC)),
   (0x170, 0x170, 0x1e3, 0x1e3, 0x170, 0x238, "v_div_fmas_f64", True, True, dst(2), src(2, 2, 2, VCC), InstrClass.ValuDouble),
   (0x171, 0x171, 0x1e4, 0x1e4, 0x171, 0x239, "v_msad_u8", False, False, dst(1), src(1, 1, 1)),
   (0x172, 0x172, 0x1e5, 0x1e5, 0x172, 0x23a, "v_qsad_pk_u16_u8", False, False, dst(2), src(2, 1, 2)),
   (0x173, 0x173, 0x1e6, 0x1e6, 0x173, 0x23b, "v_mqsad_pk_u16_u8", False, False, dst(2), src(2, 1, 2)),
   (0x174, 0x174, 0x292, 0x292, 0x174, 0x32f, "v_trig_preop_f64", False, False, dst(2), src(2, 2), InstrClass.ValuDouble),
   (   -1, 0x175, 0x1e7, 0x1e7, 0x175, 0x23d, "v_mqsad_u32_u8", False, False, dst(4), src(2, 1, 4)),
   (   -1, 0x176, 0x1e8, 0x1e8, 0x176, 0x2fe, "v_mad_u64_u32", False, False, dst(2, VCC), src(1, 1, 2), InstrClass.Valu64),
   (   -1, 0x177, 0x1e9, 0x1e9, 0x177, 0x2ff, "v_mad_i64_i32", False, False, dst(2, VCC), src(1, 1, 2), InstrClass.Valu64),
   (   -1,    -1, 0x1ea, 0x1ea,    -1,    -1, "v_mad_legacy_f16", True, True, dst(1), src(1, 1, 1)),
   (   -1,    -1, 0x1eb, 0x1eb,    -1,    -1, "v_mad_legacy_u16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1, 0x1ec, 0x1ec,    -1,    -1, "v_mad_legacy_i16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1, 0x1ed, 0x1ed, 0x344, 0x244, "v_perm_b32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1, 0x1ee, 0x1ee,    -1,    -1, "v_fma_legacy_f16", True, True, dst(1), src(1, 1, 1), InstrClass.ValuFma),
   (   -1,    -1, 0x1ef, 0x1ef,    -1,    -1, "v_div_fixup_legacy_f16", True, True, dst(1), src(1, 1, 1)),
   (0x12c, 0x12c, 0x1f0, 0x1f0,    -1,    -1, "v_cvt_pkaccum_u8_f32", True, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1f1, 0x373, 0x259, "v_mad_u32_u16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1f2, 0x375, 0x25a, "v_mad_i32_i16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1f3, 0x345, 0x245, "v_xad_u32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1f4, 0x351, 0x249, "v_min3_f16", True, True, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1f5, 0x352, 0x24a, "v_min3_i16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1f6, 0x353, 0x24b, "v_min3_u16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1f7, 0x354, 0x24c, "v_max3_f16", True, True, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1f8, 0x355, 0x24d, "v_max3_i16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1f9, 0x356, 0x24e, "v_max3_u16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1fa, 0x357, 0x24f, "v_med3_f16", True, True, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1fb, 0x358, 0x250, "v_med3_i16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1fc, 0x359, 0x251, "v_med3_u16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1fd, 0x346, 0x246, "v_lshl_add_u32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1fe, 0x347, 0x247, "v_add_lshl_u32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x1ff, 0x36d, 0x255, "v_add3_u32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x200, 0x36f, 0x256, "v_lshl_or_b32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x201, 0x371, 0x257, "v_and_or_b32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x202, 0x372, 0x258, "v_or3_b32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x203,    -1,    -1, "v_mad_f16", True, True, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x204, 0x340, 0x241, "v_mad_u16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x205, 0x35e, 0x253, "v_mad_i16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x206, 0x34b, 0x248, "v_fma_f16", True, True, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1, 0x207, 0x35f, 0x254, "v_div_fixup_f16", True, True, dst(1), src(1, 1, 1)),
   (   -1,    -1, 0x274, 0x274, 0x342,    -1, "v_interp_p1ll_f16", True, True, dst(1), src(1, M0)),
   (   -1,    -1, 0x275, 0x275, 0x343,    -1, "v_interp_p1lv_f16", True, True, dst(1), src(1, M0, 1)),
   (   -1,    -1, 0x276, 0x276,    -1,    -1, "v_interp_p2_legacy_f16", True, True, dst(1), src(1, M0, 1)),
   (   -1,    -1,    -1, 0x277, 0x35a,    -1, "v_interp_p2_f16", True, True, dst(1), src(1, M0, 1)),
   (0x12b, 0x12b, 0x288, 0x288, 0x362, 0x31c, "v_ldexp_f32", False, True, dst(1), src(1, 1)),
   (   -1,    -1, 0x289, 0x289, 0x360, 0x360, "v_readlane_b32_e64", False, False, dst(1), src(1, 1)),
   (   -1,    -1, 0x28a, 0x28a, 0x361, 0x361, "v_writelane_b32_e64", False, False, dst(1), src(1, 1, 1)),
   (0x122, 0x122, 0x28b, 0x28b, 0x364, 0x31e, "v_bcnt_u32_b32", False, False, dst(1), src(1, 1)),
   (0x123, 0x123, 0x28c, 0x28c, 0x365, 0x31f, "v_mbcnt_lo_u32_b32", False, False, dst(1), src(1, 1)),
   (   -1,    -1, 0x28d, 0x28d, 0x366, 0x320, "v_mbcnt_hi_u32_b32_e64", False, False, dst(1), src(1, 1)),
   (   -1,    -1, 0x28f, 0x28f, 0x2ff, 0x33c, "v_lshlrev_b64", False, False, dst(2), src(1, 2), InstrClass.Valu64),
   (   -1,    -1, 0x290, 0x290, 0x300, 0x33d, "v_lshrrev_b64", False, False, dst(2), src(1, 2), InstrClass.Valu64),
   (   -1,    -1, 0x291, 0x291, 0x301, 0x33e, "v_ashrrev_i64", False, False, dst(2), src(1, 2), InstrClass.Valu64),
   (0x11e, 0x11e, 0x293, 0x293, 0x363, 0x31d, "v_bfm_b32", False, False, dst(1), src(1, 1)),
   (0x12d, 0x12d, 0x294, 0x294, 0x368, 0x321, "v_cvt_pknorm_i16_f32", True, False, dst(1), src(1, 1)),
   (0x12e, 0x12e, 0x295, 0x295, 0x369, 0x322, "v_cvt_pknorm_u16_f32", True, False, dst(1), src(1, 1)),
   (   -1,    -1, 0x296, 0x296,    -1,    -1, "v_cvt_pkrtz_f16_f32_e64", True, False, dst(1), src(1, 1)),
   (0x130, 0x130, 0x297, 0x297, 0x36a, 0x323, "v_cvt_pk_u16_u32", False, False, dst(1), src(1, 1)),
   (0x131, 0x131, 0x298, 0x298, 0x36b, 0x324, "v_cvt_pk_i16_i32", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1, 0x299, 0x312, 0x312, "v_cvt_pknorm_i16_f16", True, False, dst(1), src(1, 1)), #v_cvt_pk_norm_i16_f32 in GFX11
   (   -1,    -1,    -1, 0x29a, 0x313, 0x313, "v_cvt_pknorm_u16_f16", True, False, dst(1), src(1, 1)), #v_cvt_pk_norm_u16_f32 in GFX11
   (   -1,    -1,    -1, 0x29c, 0x37f, 0x326, "v_add_i32", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1, 0x29d, 0x376, 0x325, "v_sub_i32", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1, 0x29e, 0x30d, 0x30d, "v_add_i16", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1, 0x29f, 0x30e, 0x30e, "v_sub_i16", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1, 0x2a0, 0x311, 0x311, "v_pack_b32_f16", True, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x178, 0x240, "v_xor3_b32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1, 0x377, 0x25b, "v_permlane16_b32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1, 0x378, 0x25c, "v_permlanex16_b32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1, 0x30f, 0x300, "v_add_co_u32_e64", False, False, dst(1, VCC), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x310, 0x301, "v_sub_co_u32_e64", False, False, dst(1, VCC), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x319, 0x302, "v_subrev_co_u32_e64", False, False, dst(1, VCC), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x303, 0x303, "v_add_u16_e64", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x304, 0x304, "v_sub_u16_e64", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x305, 0x305, "v_mul_lo_u16_e64", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x309, 0x309, "v_max_u16_e64", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x30a, 0x30a, "v_max_i16_e64", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x30b, 0x30b, "v_min_u16_e64", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x30c, 0x30c, "v_min_i16_e64", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x307, 0x339, "v_lshrrev_b16_e64", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x308, 0x33a, "v_ashrrev_i16_e64", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x314, 0x338, "v_lshlrev_b16_e64", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1, 0x140, 0x209, "v_fma_legacy_f32", True, True, dst(1), src(1, 1, 1), InstrClass.ValuFma), #GFX10.3+, v_fma_dx9_zero_f32 in GFX11
   (   -1,    -1,    -1,    -1,    -1, 0x25e, "v_maxmin_f32", True, True, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x25f, "v_minmax_f32", True, True, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x260, "v_maxmin_f16", True, True, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x261, "v_minmax_f16", True, True, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x262, "v_maxmin_u32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x263, "v_minmax_u32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x264, "v_maxmin_i32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x265, "v_minmax_i32", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x266, "v_dot2_f16_f16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x267, "v_dot2_bf16_bf16", False, False, dst(1), src(1, 1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x306, "v_cvt_pk_i16_f32", True, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x307, "v_cvt_pk_u16_f32", True, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x362, "v_and_b16", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x363, "v_or_b16", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x364, "v_xor_b16", False, False, dst(1), src(1, 1)),
   (   -1,    -1,    -1,    -1,    -1, 0x25d, "v_cndmask_b16", True, False, dst(1), src(1, 1, VCC)),
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name, in_mod, out_mod, defs, ops, cls) in default_class(VOP3, InstrClass.Valu32):
   opcode(name, gfx7, gfx9, gfx10, gfx11, Format.VOP3, cls, in_mod, out_mod, definitions = defs, operands = ops)


# DS instructions: 3 inputs (1 addr, 2 data), 1 output
DS = {
   (0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "ds_add_u32"),
   (0x01, 0x01, 0x01, 0x01, 0x01, 0x01, "ds_sub_u32"),
   (0x02, 0x02, 0x02, 0x02, 0x02, 0x02, "ds_rsub_u32"),
   (0x03, 0x03, 0x03, 0x03, 0x03, 0x03, "ds_inc_u32"),
   (0x04, 0x04, 0x04, 0x04, 0x04, 0x04, "ds_dec_u32"),
   (0x05, 0x05, 0x05, 0x05, 0x05, 0x05, "ds_min_i32"),
   (0x06, 0x06, 0x06, 0x06, 0x06, 0x06, "ds_max_i32"),
   (0x07, 0x07, 0x07, 0x07, 0x07, 0x07, "ds_min_u32"),
   (0x08, 0x08, 0x08, 0x08, 0x08, 0x08, "ds_max_u32"),
   (0x09, 0x09, 0x09, 0x09, 0x09, 0x09, "ds_and_b32"),
   (0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, "ds_or_b32"),
   (0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, "ds_xor_b32"),
   (0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, "ds_mskor_b32"),
   (0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, "ds_write_b32"), #ds_store_b32 in GFX11
   (0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, "ds_write2_b32"), #ds_store_2addr_b32 in GFX11
   (0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, "ds_write2st64_b32"), #ds_store_2addr_stride64_b32 in GFX11
   (0x10, 0x10, 0x10, 0x10, 0x10, 0x10, "ds_cmpst_b32"), #ds_cmpstore_b32 in GFX11
   (0x11, 0x11, 0x11, 0x11, 0x11, 0x11, "ds_cmpst_f32"), #ds_cmpstore_f32 in GFX11
   (0x12, 0x12, 0x12, 0x12, 0x12, 0x12, "ds_min_f32"),
   (0x13, 0x13, 0x13, 0x13, 0x13, 0x13, "ds_max_f32"),
   (  -1, 0x14, 0x14, 0x14, 0x14, 0x14, "ds_nop"),
   (  -1,   -1, 0x15, 0x15, 0x15, 0x15, "ds_add_f32"),
   (  -1,   -1, 0x1d, 0x1d, 0xb0, 0xb0, "ds_write_addtid_b32"), #ds_store_addtid_b32 in GFX11
   (0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, "ds_write_b8"), #ds_store_b8 in GFX11
   (0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, "ds_write_b16"), #ds_store_b16 in GFX11
   (0x20, 0x20, 0x20, 0x20, 0x20, 0x20, "ds_add_rtn_u32"),
   (0x21, 0x21, 0x21, 0x21, 0x21, 0x21, "ds_sub_rtn_u32"),
   (0x22, 0x22, 0x22, 0x22, 0x22, 0x22, "ds_rsub_rtn_u32"),
   (0x23, 0x23, 0x23, 0x23, 0x23, 0x23, "ds_inc_rtn_u32"),
   (0x24, 0x24, 0x24, 0x24, 0x24, 0x24, "ds_dec_rtn_u32"),
   (0x25, 0x25, 0x25, 0x25, 0x25, 0x25, "ds_min_rtn_i32"),
   (0x26, 0x26, 0x26, 0x26, 0x26, 0x26, "ds_max_rtn_i32"),
   (0x27, 0x27, 0x27, 0x27, 0x27, 0x27, "ds_min_rtn_u32"),
   (0x28, 0x28, 0x28, 0x28, 0x28, 0x28, "ds_max_rtn_u32"),
   (0x29, 0x29, 0x29, 0x29, 0x29, 0x29, "ds_and_rtn_b32"),
   (0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, "ds_or_rtn_b32"),
   (0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b, "ds_xor_rtn_b32"),
   (0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, "ds_mskor_rtn_b32"),
   (0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, "ds_wrxchg_rtn_b32"), #ds_storexchg_rtn_b32 in GFX11
   (0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, "ds_wrxchg2_rtn_b32"), #ds_storexchg_2addr_rtn_b32 in GFX11
   (0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, "ds_wrxchg2st64_rtn_b32"), #ds_storexchg_2addr_stride64_rtn_b32 in GFX11
   (0x30, 0x30, 0x30, 0x30, 0x30, 0x30, "ds_cmpst_rtn_b32"), #ds_cmpstore_rtn_b32 in GFX11
   (0x31, 0x31, 0x31, 0x31, 0x31, 0x31, "ds_cmpst_rtn_f32"), #ds_cmpstore_rtn_f32 in GFX11
   (0x32, 0x32, 0x32, 0x32, 0x32, 0x32, "ds_min_rtn_f32"),
   (0x33, 0x33, 0x33, 0x33, 0x33, 0x33, "ds_max_rtn_f32"),
   (  -1, 0x34, 0x34, 0x34, 0x34, 0x34, "ds_wrap_rtn_b32"),
   (  -1,   -1, 0x35, 0x35, 0x55, 0x79, "ds_add_rtn_f32"),
   (0x36, 0x36, 0x36, 0x36, 0x36, 0x36, "ds_read_b32"), #ds_load_b32 in GFX11
   (0x37, 0x37, 0x37, 0x37, 0x37, 0x37, "ds_read2_b32"), #ds_load_2addr_b32 in GFX11
   (0x38, 0x38, 0x38, 0x38, 0x38, 0x38, "ds_read2st64_b32"), #ds_load_2addr_stride64_b32 in GFX11
   (0x39, 0x39, 0x39, 0x39, 0x39, 0x39, "ds_read_i8"), #ds_load_i8 in GFX11
   (0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, "ds_read_u8"), #ds_load_u8 in GFX11
   (0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3b, "ds_read_i16"), #ds_load_i16 in GFX11
   (0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, "ds_read_u16"), #ds_load_u16 in GFX11
   (0x35, 0x35, 0x3d, 0x3d, 0x35, 0x35, "ds_swizzle_b32"), #data1 & offset, no addr/data2
   (  -1,   -1, 0x3e, 0x3e, 0xb2, 0xb2, "ds_permute_b32"),
   (  -1,   -1, 0x3f, 0x3f, 0xb3, 0xb3, "ds_bpermute_b32"),
   (0x40, 0x40, 0x40, 0x40, 0x40, 0x40, "ds_add_u64"),
   (0x41, 0x41, 0x41, 0x41, 0x41, 0x41, "ds_sub_u64"),
   (0x42, 0x42, 0x42, 0x42, 0x42, 0x42, "ds_rsub_u64"),
   (0x43, 0x43, 0x43, 0x43, 0x43, 0x43, "ds_inc_u64"),
   (0x44, 0x44, 0x44, 0x44, 0x44, 0x44, "ds_dec_u64"),
   (0x45, 0x45, 0x45, 0x45, 0x45, 0x45, "ds_min_i64"),
   (0x46, 0x46, 0x46, 0x46, 0x46, 0x46, "ds_max_i64"),
   (0x47, 0x47, 0x47, 0x47, 0x47, 0x47, "ds_min_u64"),
   (0x48, 0x48, 0x48, 0x48, 0x48, 0x48, "ds_max_u64"),
   (0x49, 0x49, 0x49, 0x49, 0x49, 0x49, "ds_and_b64"),
   (0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, "ds_or_b64"),
   (0x4b, 0x4b, 0x4b, 0x4b, 0x4b, 0x4b, "ds_xor_b64"),
   (0x4c, 0x4c, 0x4c, 0x4c, 0x4c, 0x4c, "ds_mskor_b64"),
   (0x4d, 0x4d, 0x4d, 0x4d, 0x4d, 0x4d, "ds_write_b64"), #ds_store_b64 in GFX11
   (0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, "ds_write2_b64"), #ds_store_2addr_b64 in GFX11
   (0x4f, 0x4f, 0x4f, 0x4f, 0x4f, 0x4f, "ds_write2st64_b64"), #ds_store_2addr_stride64_b64 in GFX11
   (0x50, 0x50, 0x50, 0x50, 0x50, 0x50, "ds_cmpst_b64"), #ds_cmpstore_b64 in GFX11
   (0x51, 0x51, 0x51, 0x51, 0x51, 0x51, "ds_cmpst_f64"), #ds_cmpstore_f64 in GFX11
   (0x52, 0x52, 0x52, 0x52, 0x52, 0x52, "ds_min_f64"),
   (0x53, 0x53, 0x53, 0x53, 0x53, 0x53, "ds_max_f64"),
   (  -1,   -1,   -1, 0x54, 0xa0, 0xa0, "ds_write_b8_d16_hi"), #ds_store_b8_d16_hi in GFX11
   (  -1,   -1,   -1, 0x55, 0xa1, 0xa1, "ds_write_b16_d16_hi"), #ds_store_b16_d16_hi in GFX11
   (  -1,   -1,   -1, 0x56, 0xa2, 0xa2, "ds_read_u8_d16"), #ds_load_u8_d16 in GFX11
   (  -1,   -1,   -1, 0x57, 0xa3, 0xa3, "ds_read_u8_d16_hi"), #ds_load_u8_d16_hi in GFX11
   (  -1,   -1,   -1, 0x58, 0xa4, 0xa4, "ds_read_i8_d16"), #ds_load_i8_d16 in GFX11
   (  -1,   -1,   -1, 0x59, 0xa5, 0xa5, "ds_read_i8_d16_hi"), #ds_load_i8_d16_hi in GFX11
   (  -1,   -1,   -1, 0x5a, 0xa6, 0xa6, "ds_read_u16_d16"), #ds_load_u16_d16 in GFX11
   (  -1,   -1,   -1, 0x5b, 0xa7, 0xa7, "ds_read_u16_d16_hi"), #ds_load_u16_d16_hi in GFX11
   (0x60, 0x60, 0x60, 0x60, 0x60, 0x60, "ds_add_rtn_u64"),
   (0x61, 0x61, 0x61, 0x61, 0x61, 0x61, "ds_sub_rtn_u64"),
   (0x62, 0x62, 0x62, 0x62, 0x62, 0x62, "ds_rsub_rtn_u64"),
   (0x63, 0x63, 0x63, 0x63, 0x63, 0x63, "ds_inc_rtn_u64"),
   (0x64, 0x64, 0x64, 0x64, 0x64, 0x64, "ds_dec_rtn_u64"),
   (0x65, 0x65, 0x65, 0x65, 0x65, 0x65, "ds_min_rtn_i64"),
   (0x66, 0x66, 0x66, 0x66, 0x66, 0x66, "ds_max_rtn_i64"),
   (0x67, 0x67, 0x67, 0x67, 0x67, 0x67, "ds_min_rtn_u64"),
   (0x68, 0x68, 0x68, 0x68, 0x68, 0x68, "ds_max_rtn_u64"),
   (0x69, 0x69, 0x69, 0x69, 0x69, 0x69, "ds_and_rtn_b64"),
   (0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, "ds_or_rtn_b64"),
   (0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, "ds_xor_rtn_b64"),
   (0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, "ds_mskor_rtn_b64"),
   (0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, "ds_wrxchg_rtn_b64"), #ds_storexchg_rtn_b64 in GFX11
   (0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, "ds_wrxchg2_rtn_b64"), #ds_storexchg_2addr_rtn_b64 in GFX11
   (0x6f, 0x6f, 0x6f, 0x6f, 0x6f, 0x6f, "ds_wrxchg2st64_rtn_b64"), #ds_storexchg_2addr_stride64_rtn_b64 in GFX11
   (0x70, 0x70, 0x70, 0x70, 0x70, 0x70, "ds_cmpst_rtn_b64"), #ds_cmpstore_rtn_b64 in GFX11
   (0x71, 0x71, 0x71, 0x71, 0x71, 0x71, "ds_cmpst_rtn_f64"), #ds_cmpstore_rtn_f64 in GFX11
   (0x72, 0x72, 0x72, 0x72, 0x72, 0x72, "ds_min_rtn_f64"),
   (0x73, 0x73, 0x73, 0x73, 0x73, 0x73, "ds_max_rtn_f64"),
   (0x76, 0x76, 0x76, 0x76, 0x76, 0x76, "ds_read_b64"), #ds_load_b64 in GFX11
   (0x77, 0x77, 0x77, 0x77, 0x77, 0x77, "ds_read2_b64"), #ds_load_2addr_b64 in GFX11
   (0x78, 0x78, 0x78, 0x78, 0x78, 0x78, "ds_read2st64_b64"), #ds_load_2addr_stride64_b64 in GFX11
   (  -1, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, "ds_condxchg32_rtn_b64"),
   (0x80, 0x80, 0x80, 0x80, 0x80,   -1, "ds_add_src2_u32"),
   (0x81, 0x81, 0x81, 0x81, 0x81,   -1, "ds_sub_src2_u32"),
   (0x82, 0x82, 0x82, 0x82, 0x82,   -1, "ds_rsub_src2_u32"),
   (0x83, 0x83, 0x83, 0x83, 0x83,   -1, "ds_inc_src2_u32"),
   (0x84, 0x84, 0x84, 0x84, 0x84,   -1, "ds_dec_src2_u32"),
   (0x85, 0x85, 0x85, 0x85, 0x85,   -1, "ds_min_src2_i32"),
   (0x86, 0x86, 0x86, 0x86, 0x86,   -1, "ds_max_src2_i32"),
   (0x87, 0x87, 0x87, 0x87, 0x87,   -1, "ds_min_src2_u32"),
   (0x88, 0x88, 0x88, 0x88, 0x88,   -1, "ds_max_src2_u32"),
   (0x89, 0x89, 0x89, 0x89, 0x89,   -1, "ds_and_src2_b32"),
   (0x8a, 0x8a, 0x8a, 0x8a, 0x8a,   -1, "ds_or_src2_b32"),
   (0x8b, 0x8b, 0x8b, 0x8b, 0x8b,   -1, "ds_xor_src2_b32"),
   (0x8d, 0x8d, 0x8d, 0x8d, 0x8d,   -1, "ds_write_src2_b32"),
   (0x92, 0x92, 0x92, 0x92, 0x92,   -1, "ds_min_src2_f32"),
   (0x93, 0x93, 0x93, 0x93, 0x93,   -1, "ds_max_src2_f32"),
   (  -1,   -1, 0x95, 0x95, 0x95,   -1, "ds_add_src2_f32"),
   (  -1, 0x18, 0x98, 0x98, 0x18, 0x18, "ds_gws_sema_release_all"),
   (0x19, 0x19, 0x99, 0x99, 0x19, 0x19, "ds_gws_init"),
   (0x1a, 0x1a, 0x9a, 0x9a, 0x1a, 0x1a, "ds_gws_sema_v"),
   (0x1b, 0x1b, 0x9b, 0x9b, 0x1b, 0x1b, "ds_gws_sema_br"),
   (0x1c, 0x1c, 0x9c, 0x9c, 0x1c, 0x1c, "ds_gws_sema_p"),
   (0x1d, 0x1d, 0x9d, 0x9d, 0x1d, 0x1d, "ds_gws_barrier"),
   (  -1,   -1, 0xb6, 0xb6, 0xb1, 0xb1, "ds_read_addtid_b32"), #ds_load_addtid_b32 in GFX11
   (0x3d, 0x3d, 0xbd, 0xbd, 0x3d, 0x3d, "ds_consume"),
   (0x3e, 0x3e, 0xbe, 0xbe, 0x3e, 0x3e, "ds_append"),
   (0x3f, 0x3f, 0xbf, 0xbf, 0x3f, 0x3f, "ds_ordered_count"),
   (0xc0, 0xc0, 0xc0, 0xc0, 0xc0,   -1, "ds_add_src2_u64"),
   (0xc1, 0xc1, 0xc1, 0xc1, 0xc1,   -1, "ds_sub_src2_u64"),
   (0xc2, 0xc2, 0xc2, 0xc2, 0xc2,   -1, "ds_rsub_src2_u64"),
   (0xc3, 0xc3, 0xc3, 0xc3, 0xc3,   -1, "ds_inc_src2_u64"),
   (0xc4, 0xc4, 0xc4, 0xc4, 0xc4,   -1, "ds_dec_src2_u64"),
   (0xc5, 0xc5, 0xc5, 0xc5, 0xc5,   -1, "ds_min_src2_i64"),
   (0xc6, 0xc6, 0xc6, 0xc6, 0xc6,   -1, "ds_max_src2_i64"),
   (0xc7, 0xc7, 0xc7, 0xc7, 0xc7,   -1, "ds_min_src2_u64"),
   (0xc8, 0xc8, 0xc8, 0xc8, 0xc8,   -1, "ds_max_src2_u64"),
   (0xc9, 0xc9, 0xc9, 0xc9, 0xc9,   -1, "ds_and_src2_b64"),
   (0xca, 0xca, 0xca, 0xca, 0xca,   -1, "ds_or_src2_b64"),
   (0xcb, 0xcb, 0xcb, 0xcb, 0xcb,   -1, "ds_xor_src2_b64"),
   (0xcd, 0xcd, 0xcd, 0xcd, 0xcd,   -1, "ds_write_src2_b64"),
   (0xd2, 0xd2, 0xd2, 0xd2, 0xd2,   -1, "ds_min_src2_f64"),
   (0xd3, 0xd3, 0xd3, 0xd3, 0xd3,   -1, "ds_max_src2_f64"),
   (  -1, 0xde, 0xde, 0xde, 0xde, 0xde, "ds_write_b96"), #ds_store_b96 in GFX11
   (  -1, 0xdf, 0xdf, 0xdf, 0xdf, 0xdf, "ds_write_b128"), #ds_store_b128 in GFX11
   (  -1, 0xfd, 0xfd,   -1,   -1,   -1, "ds_condxchg32_rtn_b128"),
   (  -1, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, "ds_read_b96"), #ds_load_b96 in GFX11
   (  -1, 0xff, 0xff, 0xff, 0xff, 0xff, "ds_read_b128"), #ds_load_b128 in GFX11
   (  -1,   -1,   -1,   -1,   -1, 0x7a, "ds_add_gs_reg_rtn"),
   (  -1,   -1,   -1,   -1,   -1, 0x7b, "ds_sub_gs_reg_rtn"),
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) in DS:
    opcode(name, gfx7, gfx9, gfx10, gfx11, Format.DS, InstrClass.DS)


# LDSDIR instructions:
LDSDIR = {
   (0x00, "lds_param_load"),
   (0x01, "lds_direct_load"),
}
for (code, name) in LDSDIR:
    opcode(name, -1, -1, -1, code, Format.LDSDIR, InstrClass.DS)

# MUBUF instructions:
MUBUF = {
   (0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "buffer_load_format_x"),
   (0x01, 0x01, 0x01, 0x01, 0x01, 0x01, "buffer_load_format_xy"),
   (0x02, 0x02, 0x02, 0x02, 0x02, 0x02, "buffer_load_format_xyz"),
   (0x03, 0x03, 0x03, 0x03, 0x03, 0x03, "buffer_load_format_xyzw"),
   (0x04, 0x04, 0x04, 0x04, 0x04, 0x04, "buffer_store_format_x"),
   (0x05, 0x05, 0x05, 0x05, 0x05, 0x05, "buffer_store_format_xy"),
   (0x06, 0x06, 0x06, 0x06, 0x06, 0x06, "buffer_store_format_xyz"),
   (0x07, 0x07, 0x07, 0x07, 0x07, 0x07, "buffer_store_format_xyzw"),
   (  -1,   -1, 0x08, 0x08, 0x80, 0x08, "buffer_load_format_d16_x"),
   (  -1,   -1, 0x09, 0x09, 0x81, 0x09, "buffer_load_format_d16_xy"),
   (  -1,   -1, 0x0a, 0x0a, 0x82, 0x0a, "buffer_load_format_d16_xyz"),
   (  -1,   -1, 0x0b, 0x0b, 0x83, 0x0b, "buffer_load_format_d16_xyzw"),
   (  -1,   -1, 0x0c, 0x0c, 0x84, 0x0c, "buffer_store_format_d16_x"),
   (  -1,   -1, 0x0d, 0x0d, 0x85, 0x0d, "buffer_store_format_d16_xy"),
   (  -1,   -1, 0x0e, 0x0e, 0x86, 0x0e, "buffer_store_format_d16_xyz"),
   (  -1,   -1, 0x0f, 0x0f, 0x87, 0x0f, "buffer_store_format_d16_xyzw"),
   (0x08, 0x08, 0x10, 0x10, 0x08, 0x10, "buffer_load_ubyte"),
   (0x09, 0x09, 0x11, 0x11, 0x09, 0x11, "buffer_load_sbyte"),
   (0x0a, 0x0a, 0x12, 0x12, 0x0a, 0x12, "buffer_load_ushort"),
   (0x0b, 0x0b, 0x13, 0x13, 0x0b, 0x13, "buffer_load_sshort"),
   (0x0c, 0x0c, 0x14, 0x14, 0x0c, 0x14, "buffer_load_dword"),
   (0x0d, 0x0d, 0x15, 0x15, 0x0d, 0x15, "buffer_load_dwordx2"),
   (  -1, 0x0f, 0x16, 0x16, 0x0f, 0x16, "buffer_load_dwordx3"),
   (0x0f, 0x0e, 0x17, 0x17, 0x0e, 0x17, "buffer_load_dwordx4"),
   (0x18, 0x18, 0x18, 0x18, 0x18, 0x18, "buffer_store_byte"),
   (  -1,   -1,   -1, 0x19, 0x19, 0x24, "buffer_store_byte_d16_hi"),
   (0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x19, "buffer_store_short"),
   (  -1,   -1,   -1, 0x1b, 0x1b, 0x25, "buffer_store_short_d16_hi"),
   (0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1a, "buffer_store_dword"),
   (0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1b, "buffer_store_dwordx2"),
   (  -1, 0x1f, 0x1e, 0x1e, 0x1f, 0x1c, "buffer_store_dwordx3"),
   (0x1e, 0x1e, 0x1f, 0x1f, 0x1e, 0x1d, "buffer_store_dwordx4"),
   (  -1,   -1,   -1, 0x20, 0x20, 0x1e, "buffer_load_ubyte_d16"),
   (  -1,   -1,   -1, 0x21, 0x21, 0x21, "buffer_load_ubyte_d16_hi"),
   (  -1,   -1,   -1, 0x22, 0x22, 0x1f, "buffer_load_sbyte_d16"),
   (  -1,   -1,   -1, 0x23, 0x23, 0x22, "buffer_load_sbyte_d16_hi"),
   (  -1,   -1,   -1, 0x24, 0x24, 0x20, "buffer_load_short_d16"),
   (  -1,   -1,   -1, 0x25, 0x25, 0x23, "buffer_load_short_d16_hi"),
   (  -1,   -1,   -1, 0x26, 0x26, 0x26, "buffer_load_format_d16_hi_x"),
   (  -1,   -1,   -1, 0x27, 0x27, 0x27, "buffer_store_format_d16_hi_x"),
   (  -1,   -1, 0x3d, 0x3d,   -1,   -1, "buffer_store_lds_dword"),
   (0x71, 0x71, 0x3e, 0x3e,   -1,   -1, "buffer_wbinvl1"),
   (0x70, 0x70, 0x3f, 0x3f,   -1,   -1, "buffer_wbinvl1_vol"),
   (0x30, 0x30, 0x40, 0x40, 0x30, 0x33, "buffer_atomic_swap"),
   (0x31, 0x31, 0x41, 0x41, 0x31, 0x34, "buffer_atomic_cmpswap"),
   (0x32, 0x32, 0x42, 0x42, 0x32, 0x35, "buffer_atomic_add"),
   (0x33, 0x33, 0x43, 0x43, 0x33, 0x36, "buffer_atomic_sub"),
   (0x34,   -1,   -1,   -1,   -1,   -1, "buffer_atomic_rsub"),
   (0x35, 0x35, 0x44, 0x44, 0x35, 0x38, "buffer_atomic_smin"),
   (0x36, 0x36, 0x45, 0x45, 0x36, 0x39, "buffer_atomic_umin"),
   (0x37, 0x37, 0x46, 0x46, 0x37, 0x3a, "buffer_atomic_smax"),
   (0x38, 0x38, 0x47, 0x47, 0x38, 0x3b, "buffer_atomic_umax"),
   (0x39, 0x39, 0x48, 0x48, 0x39, 0x3c, "buffer_atomic_and"),
   (0x3a, 0x3a, 0x49, 0x49, 0x3a, 0x3d, "buffer_atomic_or"),
   (0x3b, 0x3b, 0x4a, 0x4a, 0x3b, 0x3e, "buffer_atomic_xor"),
   (0x3c, 0x3c, 0x4b, 0x4b, 0x3c, 0x3f, "buffer_atomic_inc"),
   (0x3d, 0x3d, 0x4c, 0x4c, 0x3d, 0x40, "buffer_atomic_dec"),
   (0x3e, 0x3e,   -1,   -1, 0x3e, 0x50, "buffer_atomic_fcmpswap"),
   (0x3f, 0x3f,   -1,   -1, 0x3f, 0x51, "buffer_atomic_fmin"),
   (0x40, 0x40,   -1,   -1, 0x40, 0x52, "buffer_atomic_fmax"),
   (0x50, 0x50, 0x60, 0x60, 0x50, 0x41, "buffer_atomic_swap_x2"),
   (0x51, 0x51, 0x61, 0x61, 0x51, 0x42, "buffer_atomic_cmpswap_x2"),
   (0x52, 0x52, 0x62, 0x62, 0x52, 0x43, "buffer_atomic_add_x2"),
   (0x53, 0x53, 0x63, 0x63, 0x53, 0x44, "buffer_atomic_sub_x2"),
   (0x54,   -1,   -1,   -1,   -1,   -1, "buffer_atomic_rsub_x2"),
   (0x55, 0x55, 0x64, 0x64, 0x55, 0x45, "buffer_atomic_smin_x2"),
   (0x56, 0x56, 0x65, 0x65, 0x56, 0x46, "buffer_atomic_umin_x2"),
   (0x57, 0x57, 0x66, 0x66, 0x57, 0x47, "buffer_atomic_smax_x2"),
   (0x58, 0x58, 0x67, 0x67, 0x58, 0x48, "buffer_atomic_umax_x2"),
   (0x59, 0x59, 0x68, 0x68, 0x59, 0x49, "buffer_atomic_and_x2"),
   (0x5a, 0x5a, 0x69, 0x69, 0x5a, 0x4a, "buffer_atomic_or_x2"),
   (0x5b, 0x5b, 0x6a, 0x6a, 0x5b, 0x4b, "buffer_atomic_xor_x2"),
   (0x5c, 0x5c, 0x6b, 0x6b, 0x5c, 0x4c, "buffer_atomic_inc_x2"),
   (0x5d, 0x5d, 0x6c, 0x6c, 0x5d, 0x4d, "buffer_atomic_dec_x2"),
   (0x5e, 0x5e,   -1,   -1, 0x5e,   -1, "buffer_atomic_fcmpswap_x2"),
   (0x5f, 0x5f,   -1,   -1, 0x5f,   -1, "buffer_atomic_fmin_x2"),
   (0x60, 0x60,   -1,   -1, 0x60,   -1, "buffer_atomic_fmax_x2"),
   (  -1,   -1,   -1,   -1, 0x71, 0x2b, "buffer_gl0_inv"),
   (  -1,   -1,   -1,   -1, 0x72, 0x2c, "buffer_gl1_inv"),
   (  -1,   -1,   -1,   -1, 0x34, 0x37, "buffer_atomic_csub"), #GFX10.3+. seems glc must be set. buffer_atomic_csub_u32 in GFX11
   (  -1,   -1,   -1,   -1,   -1, 0x31, "buffer_load_lds_b32"),
   (  -1,   -1,   -1,   -1,   -1, 0x32, "buffer_load_lds_format_x"),
   (  -1,   -1,   -1,   -1,   -1, 0x2e, "buffer_load_lds_i8"),
   (  -1,   -1,   -1,   -1,   -1, 0x30, "buffer_load_lds_i16"),
   (  -1,   -1,   -1,   -1,   -1, 0x2d, "buffer_load_lds_u8"),
   (  -1,   -1,   -1,   -1,   -1, 0x2f, "buffer_load_lds_u16"),
   (  -1,   -1,   -1,   -1,   -1, 0x56, "buffer_atomic_add_f32"),
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) in MUBUF:
    opcode(name, gfx7, gfx9, gfx10, gfx11, Format.MUBUF, InstrClass.VMem, is_atomic = "atomic" in name)

MTBUF = {
   (0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "tbuffer_load_format_x"),
   (0x01, 0x01, 0x01, 0x01, 0x01, 0x01, "tbuffer_load_format_xy"),
   (0x02, 0x02, 0x02, 0x02, 0x02, 0x02, "tbuffer_load_format_xyz"),
   (0x03, 0x03, 0x03, 0x03, 0x03, 0x03, "tbuffer_load_format_xyzw"),
   (0x04, 0x04, 0x04, 0x04, 0x04, 0x04, "tbuffer_store_format_x"),
   (0x05, 0x05, 0x05, 0x05, 0x05, 0x05, "tbuffer_store_format_xy"),
   (0x06, 0x06, 0x06, 0x06, 0x06, 0x06, "tbuffer_store_format_xyz"),
   (0x07, 0x07, 0x07, 0x07, 0x07, 0x07, "tbuffer_store_format_xyzw"),
   (  -1,   -1, 0x08, 0x08, 0x08, 0x08, "tbuffer_load_format_d16_x"),
   (  -1,   -1, 0x09, 0x09, 0x09, 0x09, "tbuffer_load_format_d16_xy"),
   (  -1,   -1, 0x0a, 0x0a, 0x0a, 0x0a, "tbuffer_load_format_d16_xyz"),
   (  -1,   -1, 0x0b, 0x0b, 0x0b, 0x0b, "tbuffer_load_format_d16_xyzw"),
   (  -1,   -1, 0x0c, 0x0c, 0x0c, 0x0c, "tbuffer_store_format_d16_x"),
   (  -1,   -1, 0x0d, 0x0d, 0x0d, 0x0d, "tbuffer_store_format_d16_xy"),
   (  -1,   -1, 0x0e, 0x0e, 0x0e, 0x0e, "tbuffer_store_format_d16_xyz"),
   (  -1,   -1, 0x0f, 0x0f, 0x0f, 0x0f, "tbuffer_store_format_d16_xyzw"),
}
for (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) in MTBUF:
    opcode(name, gfx7, gfx9, gfx10, gfx11, Format.MTBUF, InstrClass.VMem)


IMAGE = {
   (0x00, 0x00, "image_load"),
   (0x01, 0x01, "image_load_mip"),
   (0x02, 0x02, "image_load_pck"),
   (0x03, 0x03, "image_load_pck_sgn"),
   (0x04, 0x04, "image_load_mip_pck"),
   (0x05, 0x05, "image_load_mip_pck_sgn"),
   (0x08, 0x06, "image_store"),
   (0x09, 0x07, "image_store_mip"),
   (0x0a, 0x08, "image_store_pck"),
   (0x0b, 0x09, "image_store_mip_pck"),
   (0x0e, 0x17, "image_get_resinfo"),
   (0x60, 0x38, "image_get_lod"),
}
# (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (code, code, code, code, code, name)
for (code, gfx11, name) in IMAGE:
   opcode(name, code, code, code, gfx11, Format.MIMG, InstrClass.VMem)

opcode("image_msaa_load", -1, -1, 0x80, 0x18, Format.MIMG, InstrClass.VMem) #GFX10.3+

IMAGE_ATOMIC = {
   (0x0f, 0x0f, 0x10, 0x0a, "image_atomic_swap"),
   (0x10, 0x10, 0x11, 0x0b, "image_atomic_cmpswap"),
   (0x11, 0x11, 0x12, 0x0c, "image_atomic_add"),
   (0x12, 0x12, 0x13, 0x0d, "image_atomic_sub"),
   (0x13,   -1,   -1,   -1, "image_atomic_rsub"),
   (0x14, 0x14, 0x14, 0x0e, "image_atomic_smin"),
   (0x15, 0x15, 0x15, 0x0f, "image_atomic_umin"),
   (0x16, 0x16, 0x16, 0x10, "image_atomic_smax"),
   (0x17, 0x17, 0x17, 0x11, "image_atomic_umax"),
   (0x18, 0x18, 0x18, 0x12, "image_atomic_and"),
   (0x19, 0x19, 0x19, 0x13, "image_atomic_or"),
   (0x1a, 0x1a, 0x1a, 0x14, "image_atomic_xor"),
   (0x1b, 0x1b, 0x1b, 0x15, "image_atomic_inc"),
   (0x1c, 0x1c, 0x1c, 0x16, "image_atomic_dec"),
   (0x1d, 0x1d,   -1,   -1, "image_atomic_fcmpswap"),
   (0x1e, 0x1e,   -1,   -1, "image_atomic_fmin"),
   (0x1f, 0x1f,   -1,   -1, "image_atomic_fmax"),
}
# (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (gfx6, gfx7, gfx89, gfx89, ???, gfx11, name)
# gfx7 and gfx10 opcodes are the same here
for (gfx6, gfx7, gfx89, gfx11, name) in IMAGE_ATOMIC:
   opcode(name, gfx7, gfx89, gfx7, gfx11, Format.MIMG, InstrClass.VMem, is_atomic = True)

IMAGE_SAMPLE = {
   (0x20, 0x1b, "image_sample"),
   (0x21, 0x40, "image_sample_cl"),
   (0x22, 0x1c, "image_sample_d"),
   (0x23, 0x41, "image_sample_d_cl"),
   (0x24, 0x1d, "image_sample_l"),
   (0x25, 0x1e, "image_sample_b"),
   (0x26, 0x42, "image_sample_b_cl"),
   (0x27, 0x1f, "image_sample_lz"),
   (0x28, 0x20, "image_sample_c"),
   (0x29, 0x43, "image_sample_c_cl"),
   (0x2a, 0x21, "image_sample_c_d"),
   (0x2b, 0x44, "image_sample_c_d_cl"),
   (0x2c, 0x22, "image_sample_c_l"),
   (0x2d, 0x23, "image_sample_c_b"),
   (0x2e, 0x45, "image_sample_c_b_cl"),
   (0x2f, 0x24, "image_sample_c_lz"),
   (0x30, 0x25, "image_sample_o"),
   (0x31, 0x46, "image_sample_cl_o"),
   (0x32, 0x26, "image_sample_d_o"),
   (0x33, 0x47, "image_sample_d_cl_o"),
   (0x34, 0x27, "image_sample_l_o"),
   (0x35, 0x28, "image_sample_b_o"),
   (0x36, 0x48, "image_sample_b_cl_o"),
   (0x37, 0x29, "image_sample_lz_o"),
   (0x38, 0x2a, "image_sample_c_o"),
   (0x39, 0x49, "image_sample_c_cl_o"),
   (0x3a, 0x2b, "image_sample_c_d_o"),
   (0x3b, 0x4a, "image_sample_c_d_cl_o"),
   (0x3c, 0x2c, "image_sample_c_l_o"),
   (0x3d, 0x2d, "image_sample_c_b_o"),
   (0x3e, 0x4b, "image_sample_c_b_cl_o"),
   (0x3f, 0x2e, "image_sample_c_lz_o"),
   (0x68,   -1, "image_sample_cd"),
   (0x69,   -1, "image_sample_cd_cl"),
   (0x6a,   -1, "image_sample_c_cd"),
   (0x6b,   -1, "image_sample_c_cd_cl"),
   (0x6c,   -1, "image_sample_cd_o"),
   (0x6d,   -1, "image_sample_cd_cl_o"),
   (0x6e,   -1, "image_sample_c_cd_o"),
   (0x6f,   -1, "image_sample_c_cd_cl_o"),
}
# (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (code, code, code, code, code, gfx11, name)
for (code, gfx11, name) in IMAGE_SAMPLE:
   opcode(name, code, code, code, gfx11, Format.MIMG, InstrClass.VMem)

IMAGE_SAMPLE_G16 = {
   (0xa2, 0x39, "image_sample_d_g16"),
   (0xa3, 0x5f, "image_sample_d_cl_g16"),
   (0xaa, 0x3a, "image_sample_c_d_g16"),
   (0xab, 0x54, "image_sample_c_d_cl_g16"),
   (0xb2, 0x3b, "image_sample_d_o_g16"),
   (0xb3, 0x55, "image_sample_d_cl_o_g16"),
   (0xba, 0x3c, "image_sample_c_d_o_g16"),
   (0xbb, 0x56, "image_sample_c_d_cl_o_g16"),
}

# (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (-1, -1, -1, -1, code, gfx11, name)
for (code, gfx11, name) in IMAGE_SAMPLE_G16:
   opcode(name, -1, -1, code, gfx11, Format.MIMG, InstrClass.VMem)

IMAGE_GATHER4 = {
   (0x40, 0x2f, "image_gather4"),
   (0x41, 0x60, "image_gather4_cl"),
   #(0x42, "image_gather4h"), VEGA only?
   (0x44, 0x30, "image_gather4_l"), # following instructions have different opcodes according to ISA sheet.
   (0x45, 0x31, "image_gather4_b"),
   (0x46, 0x61, "image_gather4_b_cl"),
   (0x47, 0x32, "image_gather4_lz"),
   (0x48, 0x33, "image_gather4_c"),
   (0x49, 0x62, "image_gather4_c_cl"), # previous instructions have different opcodes according to ISA sheet.
   #(0x4a, "image_gather4h_pck"), VEGA only?
   #(0x4b, "image_gather8h_pck"), VGEA only?
   (0x4c, 0x63, "image_gather4_c_l"),
   (0x4d, 0x64, "image_gather4_c_b"),
   (0x4e, 0x65, "image_gather4_c_b_cl"),
   (0x4f, 0x34, "image_gather4_c_lz"),
   (0x50, 0x35, "image_gather4_o"),
   (0x51,   -1, "image_gather4_cl_o"),
   (0x54,   -1, "image_gather4_l_o"),
   (0x55,   -1, "image_gather4_b_o"),
   (0x56,   -1, "image_gather4_b_cl_o"),
   (0x57, 0x36, "image_gather4_lz_o"),
   (0x58,   -1, "image_gather4_c_o"),
   (0x59,   -1, "image_gather4_c_cl_o"),
   (0x5c,   -1, "image_gather4_c_l_o"),
   (0x5d,   -1, "image_gather4_c_b_o"),
   (0x5e,   -1, "image_gather4_c_b_cl_o"),
   (0x5f, 0x37, "image_gather4_c_lz_o"),
}
# (gfx6, gfx7, gfx8, gfx9, gfx10, gfx11, name) = (code, code, code, code, code, gfx11, name)
for (code, gfx11, name) in IMAGE_GATHER4:
   opcode(name, code, code, code, gfx11, Format.MIMG, InstrClass.VMem)

opcode("image_bvh_intersect_ray", -1, -1, 0xe6, 0x19, Format.MIMG, InstrClass.VMem)
opcode("image_bvh64_intersect_ray", -1, -1, 0xe7, 0x1a, Format.MIMG, InstrClass.VMem)

FLAT = {
   #GFX7, GFX89,GFX10,GFX11
   (0x08, 0x10, 0x08, 0x10, "flat_load_ubyte"),
   (0x09, 0x11, 0x09, 0x11, "flat_load_sbyte"),
   (0x0a, 0x12, 0x0a, 0x12, "flat_load_ushort"),
   (0x0b, 0x13, 0x0b, 0x13, "flat_load_sshort"),
   (0x0c, 0x14, 0x0c, 0x14, "flat_load_dword"),
   (0x0d, 0x15, 0x0d, 0x15, "flat_load_dwordx2"),
   (0x0f, 0x16, 0x0f, 0x16, "flat_load_dwordx3"),
   (0x0e, 0x17, 0x0e, 0x17, "flat_load_dwordx4"),
   (0x18, 0x18, 0x18, 0x18, "flat_store_byte"),
   (  -1, 0x19, 0x19, 0x24, "flat_store_byte_d16_hi"),
   (0x1a, 0x1a, 0x1a, 0x19, "flat_store_short"),
   (  -1, 0x1b, 0x1b, 0x25, "flat_store_short_d16_hi"),
   (0x1c, 0x1c, 0x1c, 0x1a, "flat_store_dword"),
   (0x1d, 0x1d, 0x1d, 0x1b, "flat_store_dwordx2"),
   (0x1f, 0x1e, 0x1f, 0x1c, "flat_store_dwordx3"),
   (0x1e, 0x1f, 0x1e, 0x1d, "flat_store_dwordx4"),
   (  -1, 0x20, 0x20, 0x1e, "flat_load_ubyte_d16"),
   (  -1, 0x21, 0x21, 0x21, "flat_load_ubyte_d16_hi"),
   (  -1, 0x22, 0x22, 0x1f, "flat_load_sbyte_d16"),
   (  -1, 0x23, 0x23, 0x22, "flat_load_sbyte_d16_hi"),
   (  -1, 0x24, 0x24, 0x20, "flat_load_short_d16"),
   (  -1, 0x25, 0x25, 0x23, "flat_load_short_d16_hi"),
   (0x30, 0x40, 0x30, 0x33, "flat_atomic_swap"),
   (0x31, 0x41, 0x31, 0x34, "flat_atomic_cmpswap"),
   (0x32, 0x42, 0x32, 0x35, "flat_atomic_add"),
   (0x33, 0x43, 0x33, 0x36, "flat_atomic_sub"),
   (0x35, 0x44, 0x35, 0x38, "flat_atomic_smin"),
   (0x36, 0x45, 0x36, 0x39, "flat_atomic_umin"),
   (0x37, 0x46, 0x37, 0x3a, "flat_atomic_smax"),
   (0x38, 0x47, 0x38, 0x3b, "flat_atomic_umax"),
   (0x39, 0x48, 0x39, 0x3c, "flat_atomic_and"),
   (0x3a, 0x49, 0x3a, 0x3d, "flat_atomic_or"),
   (0x3b, 0x4a, 0x3b, 0x3e, "flat_atomic_xor"),
   (0x3c, 0x4b, 0x3c, 0x3f, "flat_atomic_inc"),
   (0x3d, 0x4c, 0x3d, 0x40, "flat_atomic_dec"),
   (0x3e,   -1, 0x3e, 0x50, "flat_atomic_fcmpswap"),
   (0x3f,   -1, 0x3f, 0x51, "flat_atomic_fmin"),
   (0x40,   -1, 0x40, 0x52, "flat_atomic_fmax"),
   (0x50, 0x60, 0x50, 0x41, "flat_atomic_swap_x2"),
   (0x51, 0x61, 0x51, 0x42, "flat_atomic_cmpswap_x2"),
   (0x52, 0x62, 0x52, 0x43, "flat_atomic_add_x2"),
   (0x53, 0x63, 0x53, 0x44, "flat_atomic_sub_x2"),
   (0x55, 0x64, 0x55, 0x45, "flat_atomic_smin_x2"),
   (0x56, 0x65, 0x56, 0x46, "flat_atomic_umin_x2"),
   (0x57, 0x66, 0x57, 0x47, "flat_atomic_smax_x2"),
   (0x58, 0x67, 0x58, 0x48, "flat_atomic_umax_x2"),
   (0x59, 0x68, 0x59, 0x49, "flat_atomic_and_x2"),
   (0x5a, 0x69, 0x5a, 0x4a, "flat_atomic_or_x2"),
   (0x5b, 0x6a, 0x5b, 0x4b, "flat_atomic_xor_x2"),
   (0x5c, 0x6b, 0x5c, 0x4c, "flat_atomic_inc_x2"),
   (0x5d, 0x6c, 0x5d, 0x4d, "flat_atomic_dec_x2"),
   (0x5e,   -1, 0x5e,   -1, "flat_atomic_fcmpswap_x2"),
   (0x5f,   -1, 0x5f,   -1, "flat_atomic_fmin_x2"),
   (0x60,   -1, 0x60,   -1, "flat_atomic_fmax_x2"),
   (  -1,   -1,   -1, 0x56, "flat_atomic_add_f32"),
}
for (gfx7, gfx8, gfx10, gfx11, name) in FLAT:
    opcode(name, gfx7, gfx8, gfx10, gfx11, Format.FLAT, InstrClass.VMem, is_atomic = "atomic" in name) #TODO: also LDS?

GLOBAL = {
   #GFX89,GFX10,GFX11
   (0x10, 0x08, 0x10, "global_load_ubyte"),
   (0x11, 0x09, 0x11, "global_load_sbyte"),
   (0x12, 0x0a, 0x12, "global_load_ushort"),
   (0x13, 0x0b, 0x13, "global_load_sshort"),
   (0x14, 0x0c, 0x14, "global_load_dword"),
   (0x15, 0x0d, 0x15, "global_load_dwordx2"),
   (0x16, 0x0f, 0x16, "global_load_dwordx3"),
   (0x17, 0x0e, 0x17, "global_load_dwordx4"),
   (0x18, 0x18, 0x18, "global_store_byte"),
   (0x19, 0x19, 0x24, "global_store_byte_d16_hi"),
   (0x1a, 0x1a, 0x19, "global_store_short"),
   (0x1b, 0x1b, 0x25, "global_store_short_d16_hi"),
   (0x1c, 0x1c, 0x1a, "global_store_dword"),
   (0x1d, 0x1d, 0x1b, "global_store_dwordx2"),
   (0x1e, 0x1f, 0x1c, "global_store_dwordx3"),
   (0x1f, 0x1e, 0x1d, "global_store_dwordx4"),
   (0x20, 0x20, 0x1e, "global_load_ubyte_d16"),
   (0x21, 0x21, 0x21, "global_load_ubyte_d16_hi"),
   (0x22, 0x22, 0x1f, "global_load_sbyte_d16"),
   (0x23, 0x23, 0x22, "global_load_sbyte_d16_hi"),
   (0x24, 0x24, 0x20, "global_load_short_d16"),
   (0x25, 0x25, 0x23, "global_load_short_d16_hi"),
   (0x40, 0x30, 0x33, "global_atomic_swap"),
   (0x41, 0x31, 0x34, "global_atomic_cmpswap"),
   (0x42, 0x32, 0x35, "global_atomic_add"),
   (0x43, 0x33, 0x36, "global_atomic_sub"),
   (0x44, 0x35, 0x38, "global_atomic_smin"),
   (0x45, 0x36, 0x39, "global_atomic_umin"),
   (0x46, 0x37, 0x3a, "global_atomic_smax"),
   (0x47, 0x38, 0x3b, "global_atomic_umax"),
   (0x48, 0x39, 0x3c, "global_atomic_and"),
   (0x49, 0x3a, 0x3d, "global_atomic_or"),
   (0x4a, 0x3b, 0x3e, "global_atomic_xor"),
   (0x4b, 0x3c, 0x3f, "global_atomic_inc"),
   (0x4c, 0x3d, 0x40, "global_atomic_dec"),
   (  -1, 0x3e, 0x50, "global_atomic_fcmpswap"),
   (  -1, 0x3f, 0x51, "global_atomic_fmin"),
   (  -1, 0x40, 0x52, "global_atomic_fmax"),
   (0x60, 0x50, 0x41, "global_atomic_swap_x2"),
   (0x61, 0x51, 0x42, "global_atomic_cmpswap_x2"),
   (0x62, 0x52, 0x43, "global_atomic_add_x2"),
   (0x63, 0x53, 0x44, "global_atomic_sub_x2"),
   (0x64, 0x55, 0x45, "global_atomic_smin_x2"),
   (0x65, 0x56, 0x46, "global_atomic_umin_x2"),
   (0x66, 0x57, 0x47, "global_atomic_smax_x2"),
   (0x67, 0x58, 0x48, "global_atomic_umax_x2"),
   (0x68, 0x59, 0x49, "global_atomic_and_x2"),
   (0x69, 0x5a, 0x4a, "global_atomic_or_x2"),
   (0x6a, 0x5b, 0x4b, "global_atomic_xor_x2"),
   (0x6b, 0x5c, 0x4c, "global_atomic_inc_x2"),
   (0x6c, 0x5d, 0x4d, "global_atomic_dec_x2"),
   (  -1, 0x5e,   -1, "global_atomic_fcmpswap_x2"),
   (  -1, 0x5f,   -1, "global_atomic_fmin_x2"),
   (  -1, 0x60,   -1, "global_atomic_fmax_x2"),
   (  -1, 0x16, 0x28, "global_load_dword_addtid"), #GFX10.3+
   (  -1, 0x17, 0x29, "global_store_dword_addtid"), #GFX10.3+
   (  -1, 0x34, 0x37, "global_atomic_csub"), #GFX10.3+. seems glc must be set
   (  -1,   -1, 0x56, "global_atomic_add_f32"),
}
for (gfx8, gfx10, gfx11, name) in GLOBAL:
    opcode(name, -1, gfx8, gfx10, gfx11, Format.GLOBAL, InstrClass.VMem, is_atomic = "atomic" in name)

SCRATCH = {
   #GFX89,GFX10,GFX11
   (0x10, 0x08, 0x10, "scratch_load_ubyte"),
   (0x11, 0x09, 0x11, "scratch_load_sbyte"),
   (0x12, 0x0a, 0x12, "scratch_load_ushort"),
   (0x13, 0x0b, 0x13, "scratch_load_sshort"),
   (0x14, 0x0c, 0x14, "scratch_load_dword"),
   (0x15, 0x0d, 0x15, "scratch_load_dwordx2"),
   (0x16, 0x0f, 0x16, "scratch_load_dwordx3"),
   (0x17, 0x0e, 0x17, "scratch_load_dwordx4"),
   (0x18, 0x18, 0x18, "scratch_store_byte"),
   (0x19, 0x19, 0x24, "scratch_store_byte_d16_hi"),
   (0x1a, 0x1a, 0x19, "scratch_store_short"),
   (0x1b, 0x1b, 0x25, "scratch_store_short_d16_hi"),
   (0x1c, 0x1c, 0x1a, "scratch_store_dword"),
   (0x1d, 0x1d, 0x1b, "scratch_store_dwordx2"),
   (0x1e, 0x1f, 0x1c, "scratch_store_dwordx3"),
   (0x1f, 0x1e, 0x1d, "scratch_store_dwordx4"),
   (0x20, 0x20, 0x1e, "scratch_load_ubyte_d16"),
   (0x21, 0x21, 0x21, "scratch_load_ubyte_d16_hi"),
   (0x22, 0x22, 0x1f, "scratch_load_sbyte_d16"),
   (0x23, 0x23, 0x22, "scratch_load_sbyte_d16_hi"),
   (0x24, 0x24, 0x20, "scratch_load_short_d16"),
   (0x25, 0x25, 0x23, "scratch_load_short_d16_hi"),
}
for (gfx8, gfx10, gfx11, name) in SCRATCH:
    opcode(name, -1, gfx8, gfx10, gfx11, Format.SCRATCH, InstrClass.VMem)

# check for duplicate opcode numbers
for ver in ['gfx9', 'gfx10', 'gfx11']:
    op_to_name = {}
    for op in opcodes.values():
        if op.format in [Format.PSEUDO, Format.PSEUDO_BRANCH, Format.PSEUDO_BARRIER, Format.PSEUDO_REDUCTION]:
            continue

        num = getattr(op, 'opcode_' + ver)
        if num == -1:
            continue

        key = (op.format, num)

        if key in op_to_name:
            # exceptions
            names = set([op_to_name[key], op.name])
            if ver in ['gfx8', 'gfx9', 'gfx11'] and names == set(['v_mul_lo_i32', 'v_mul_lo_u32']):
                continue
            # v_mad_legacy_f32 is replaced with v_fma_legacy_f32 on GFX10.3
            if ver == 'gfx10' and names == set(['v_mad_legacy_f32', 'v_fma_legacy_f32']):
                continue
            # v_mac_legacy_f32 is replaced with v_fmac_legacy_f32 on GFX10.3
            if ver == 'gfx10' and names == set(['v_mac_legacy_f32', 'v_fmac_legacy_f32']):
                continue

            print('%s and %s share the same opcode number (%s)' % (op_to_name[key], op.name, ver))
            sys.exit(1)
        else:
            op_to_name[key] = op.name
