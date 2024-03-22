"""
Copyright 2021 Alyssa Rosenzweig
# SPDX-License-Identifier: MIT
"""

opcodes = {}
immediates = {}
enums = {}

VARIABLE = ~0

class Opcode(object):
   def __init__(self, name, dests, srcs, imms, is_float, can_eliminate,
                can_reorder, schedule_class, encoding_16, encoding_32):
      self.name = name
      self.dests = dests if dests != VARIABLE else 0
      self.srcs = srcs if srcs != VARIABLE else 0
      self.variable_srcs = (srcs == VARIABLE)
      self.variable_dests = (dests == VARIABLE)
      self.imms = imms
      self.is_float = is_float
      self.can_eliminate = can_eliminate
      self.can_reorder = can_reorder
      self.schedule_class = schedule_class
      self.encoding_16 = encoding_16
      self.encoding_32 = encoding_32

class Immediate(object):
   def __init__(self, name, ctype):
      self.name = name
      self.ctype = ctype

class Encoding(object):
   def __init__(self, description):
      (exact, mask, length_short, length_long) = description

      # Convenience
      if length_long is None:
         length_long = length_short

      self.exact = exact
      self.mask = mask
      self.length_short = length_short
      self.extensible = length_short != length_long

      if self.extensible:
         assert(length_long == length_short + (4 if length_short > 8 else 2))

def op(name, encoding_32, dests = 1, srcs = 0, imms = [], is_float = False,
        can_eliminate = True, can_reorder = True, encoding_16 = None,
        schedule_class = "none"):
   encoding_16 = Encoding(encoding_16) if encoding_16 is not None else None
   encoding_32 = Encoding(encoding_32) if encoding_32 is not None else None

   opcodes[name] = Opcode(name, dests, srcs, imms, is_float, can_eliminate,
                          can_reorder, schedule_class,  encoding_16, encoding_32)

def immediate(name, ctype = "uint32_t"):
   imm = Immediate(name, ctype)
   immediates[name] = imm
   return imm

def enum(name, value_dict):
   enums[name] = value_dict
   return immediate(name, "enum agx_" + name)

L = (1 << 15)
_ = None

FORMAT = immediate("format", "enum agx_format")
IMM = immediate("imm", "uint64_t")
WRITEOUT = immediate("writeout")
INDEX = immediate("index")
COMPONENT = immediate("component")
CHANNELS = immediate("channels")
TRUTH_TABLE = immediate("truth_table")
ROUND = immediate("round", "enum agx_round")
SHIFT = immediate("shift")
MASK = immediate("mask")
BFI_MASK = immediate("bfi_mask")
LOD_MODE = immediate("lod_mode", "enum agx_lod_mode")
PIXEL_OFFSET = immediate("pixel_offset")
STACK_SIZE = immediate("stack_size", 'int16_t')

DIM = enum("dim", {
    0: '1d',
    1: '1d_array',
    2: '2d',
    3: '2d_array',
    4: '2d_ms',
    5: '3d',
    6: 'cube',
    7: 'cube_array',
    8: '2d_ms_array',
})

GATHER = enum("gather", {
	0b000: "none",
	0b001: "r",
	0b011: "g",
	0b101: "b",
	0b111: "a",
})

OFFSET = immediate("offset", "bool")
SHADOW = immediate("shadow", "bool")
QUERY_LOD = immediate("query_lod", "bool")
SCOREBOARD = immediate("scoreboard")
ICOND = immediate("icond", "enum agx_icond")
FCOND = immediate("fcond", "enum agx_fcond")
NEST = immediate("nest")
INVERT_COND = immediate("invert_cond")
NEST = immediate("nest")
TARGET = immediate("target", "agx_block *")
ZS = immediate("zs")
SR = enum("sr", {
   0:  'threadgroup_position_in_grid.x',
   1:  'threadgroup_position_in_grid.y',
   2:  'threadgroup_position_in_grid.z',
   4:  'threads_per_threadgroup.x',
   5:  'threads_per_threadgroup.y',
   6:  'threads_per_threadgroup.z',
   8:  'dispatch_threads_per_threadgroup.x',
   9:  'dispatch_threads_per_threadgroup.y',
   10: 'dispatch_threads_per_threadgroup.z',
   20: 'core_index',
   21: 'vm_slot',
   48: 'thread_position_in_threadgroup.x',
   49: 'thread_position_in_threadgroup.y',
   50: 'thread_position_in_threadgroup.z',
   51: 'thread_index_in_threadgroup',
   52: 'thread_index_in_subgroup',
   53: 'subgroup_index_in_threadgroup',
   56: 'active_thread_index_in_quad',
   58: 'active_thread_index_in_subgroup',
   60: 'coverage_mask',
   62: 'backfacing',
   63: 'is_active_thread',
   80: 'thread_position_in_grid.x',
   81: 'thread_position_in_grid.y',
   82: 'thread_position_in_grid.z',
   124: 'input_sample_mask',
   144: 'opfifo_cmd',
   146: 'opfifo_data_l',
   147: 'opfifo_data_h',
})

ATOMIC_OPC = enum("atomic_opc", {
	0: 'add',
	1: 'sub',
	2: 'xchg',
	3: 'cmpxchg',
	4: 'umin',
	5: 'imin',
	6: 'umax',
	7: 'imax',
	8: 'and',
	9: 'or',
	10: 'xor',
})

INTERPOLATION = enum("interpolation", {
    0: 'center',
    1: 'sample',
    2: 'centroid',
    # We translate sample -> sample_register at pack time for simplicity
    3: 'sample_register',
})

FUNOP = lambda x: (x << 28)
FUNOP_MASK = FUNOP((1 << 14) - 1)

def funop(name, opcode, schedule_class = "none"):
   op(name, (0x0A | L | (opcode << 28),
      0x3F | L | (((1 << 14) - 1) << 28), 6, _),
      srcs = 1, is_float = True, schedule_class = schedule_class)

def iunop(name, opcode):
    assert(opcode < 4)
    op(name, (0x3E | (opcode << 26),
              0x7F | L | (((1 << 14) - 1) << 26),
              6, _),
       srcs = 1)

# Listing of opcodes
funop("floor",     0b000000)
funop("srsqrt",    0b000001)
funop("dfdx",      0b000100, schedule_class = "coverage")
funop("dfdy",      0b000110, schedule_class = "coverage")
funop("rcp",       0b001000)
funop("rsqrt",     0b001001)
funop("sin_pt_1",  0b001010)
funop("log2",      0b001100)
funop("exp2",      0b001101)
funop("sin_pt_2",  0b001110)
funop("ceil",      0b010000)
funop("trunc",     0b100000)
funop("roundeven", 0b110000)

iunop("bitrev",    0b01)
iunop("popcount",  0b10)
iunop("ffs",       0b11)

op("fadd",
      encoding_16 = (0x26, 0x3F, 4, 6),
      encoding_32 = (0x2A, 0x3F, 4, 6),
      srcs = 2, is_float = True)

op("fma",
      encoding_16 = (0x36, 0x3F, 6, 8),
      encoding_32 = (0x3A, 0x3F, 6, 8),
      srcs = 3, is_float = True)

op("fmul",
      encoding_16 = (0x16, 0x3F, 4, 6),
      encoding_32 = (0x1A, 0x3F, 4, 6),
      srcs = 2, is_float = True)

op("mov_imm",
      encoding_32 = (0x62, 0xFF, 6, 8),
      encoding_16 = (0x62, 0xFF, 4, 6),
      imms = [IMM])

op("iadd",
      encoding_32 = (0x0E, 0x3F | L, 8, _),
      srcs = 2, imms = [SHIFT])

op("imad",
      encoding_32 = (0x1E, 0x3F | L, 8, _),
      srcs = 3, imms = [SHIFT])

op("bfi",
      encoding_32 = (0x2E, 0x7F | (0x3 << 26), 8, _),
      srcs = 3, imms = [BFI_MASK])

op("bfeil",
      encoding_32 = (0x2E | L, 0x7F | L | (0x3 << 26), 8, _),
      srcs = 3, imms = [BFI_MASK])

op("extr",
      encoding_32 = (0x2E | (0x1 << 26), 0x7F | L | (0x3 << 26), 8, _),
      srcs = 3, imms = [BFI_MASK])

op("asr",
      encoding_32 = (0x2E | L | (0x1 << 26), 0x7F | L | (0x3 << 26), 8, _),
      srcs = 2)

def subgroup_op(name, op1, op2):
    exact      = 0b01101111 | L | (op1 << 47) | (op2 << 26)
    exact_mask = 0b11111111 | L | (1   << 47) | (0xFFFF << 26)

    op(name, encoding_32 = (exact, exact_mask, 6, _), srcs = 1)

subgroup_op("simd_prefix_iadd", 1, 0b0000000000011000)
subgroup_op("simd_iadd", 1, 0b0000000000001000)

op("simd_shuffle",
    encoding_32 = (0b01101111 | (1 << 26),
                   0xFF | L | (1 << 47) | (3 << 38) | (3 << 26), 6, _),
    srcs = 2)

for T, T_bit, cond in [('f', 0, FCOND), ('i', 1, ICOND)]:
    for window, w_bit in [('quad_', 0), ('', 1)]:
        op(f"{T}cmp_{window}ballot",
           encoding_32 = (0b0100010 | (T_bit << 4) | (w_bit << 48), 0, 8, _),
           srcs = 2, imms = [cond, INVERT_COND])

op("icmpsel",
      encoding_32 = (0x12, 0x7F, 8, 10),
      srcs = 4, imms = [ICOND])

op("fcmpsel",
      encoding_32 = (0x02, 0x7F, 8, 10),
      srcs = 4, imms = [FCOND])

# Pseudo-instructions for compares returning 1/0
op("icmp", _, srcs = 2, imms = [ICOND, INVERT_COND])
op("fcmp", _, srcs = 2, imms = [FCOND, INVERT_COND])

# sources are coordinates, LOD, texture bindless base (zero for texture state
# registers), texture, sampler, shadow/offset
# TODO: anything else?
op("texture_sample",
      encoding_32 = (0x31, 0x7F, 8, 10), # XXX WRONG SIZE
      srcs = 6, imms = [DIM, LOD_MODE, MASK, SCOREBOARD, OFFSET, SHADOW,
                        QUERY_LOD, GATHER])
for memory, can_reorder in [("texture", True), ("image", False)]:
    op(f"{memory}_load", encoding_32 = (0x71, 0x7F, 8, 10), # XXX WRONG SIZE
       srcs = 6, imms = [DIM, LOD_MODE, MASK, SCOREBOARD, OFFSET],
       can_reorder = can_reorder,
       schedule_class = "none" if can_reorder else "load")

# sources are base, index
op("device_load",
      encoding_32 = (0x05, 0x7F, 6, 8),
      srcs = 2, imms = [FORMAT, MASK, SHIFT, SCOREBOARD], can_reorder = False,
      schedule_class = "load")

# sources are base (relative to workgroup memory), index
op("local_load",
      encoding_32 = (0b1101001, 0, 6, 8),
      srcs = 2, imms = [FORMAT, MASK], can_reorder = False,
      schedule_class = "load")

# sources are value, base, index
# TODO: Consider permitting the short form
op("device_store",
      encoding_32 = (0x45 | (1 << 47), 0, 8, _),
      dests = 0, srcs = 3, imms = [FORMAT, MASK, SHIFT, SCOREBOARD], can_eliminate = False,
      schedule_class = "store")

# sources are value, base, index
op("local_store",
      encoding_32 = (0b0101001, 0, 6, 8),
      dests = 0, srcs = 3, imms = [FORMAT, MASK],
      can_eliminate=False, schedule_class = "store")

# sources are value, index
# TODO: Consider permitting the short form
op("uniform_store",
      encoding_32 = ((0b111 << 27) | 0b1000101 | (1 << 47), 0, 8, _),
      dests = 0, srcs = 2, can_eliminate = False)

# sources are value, base, index
op("atomic",
      encoding_32 = (0x15 | (1 << 26) | (1 << 31) | (5 << 44), 0x3F | (1 << 26) | (1 << 31) | (5 << 44), 8, _),
      dests = 1, srcs = 3, imms = [ATOMIC_OPC, SCOREBOARD],
      can_eliminate = False, schedule_class = "atomic")

# XXX: stop hardcoding the long form
op("local_atomic",
      encoding_32 = (0x19 | (1 << 15) | (1 << 36) | (1 << 47), 0x3F | (1 << 36) | (1 << 47), 10, _),
      dests = 1, srcs = 3, imms = [ATOMIC_OPC], schedule_class = "atomic",
      can_eliminate = False)

op("wait", (0x38, 0xFF, 2, _), dests = 0,
      can_eliminate = False, imms = [SCOREBOARD], schedule_class = "invalid")

for (suffix, schedule_class) in [("", "none"), ("_coverage", "coverage")]:
    op(f"get_sr{suffix}", (0x72, 0x7F | L, 4, _), dests = 1, imms = [SR],
       schedule_class = schedule_class, can_reorder = schedule_class == "none")

op("sample_mask", (0x7fc1, 0xffff, 6, _), dests = 0, srcs = 2,
    can_eliminate = False, schedule_class = "coverage")

# Sources: sample mask, combined depth/stencil
op("zs_emit", (0x41, 0xFF | L, 4, _), dests = 0, srcs = 2,
              can_eliminate = False, imms = [ZS], schedule_class = "coverage")

# Essentially same encoding. Last source is the sample mask
op("ld_tile", (0x49, 0x7F, 8, _), dests = 1, srcs = 1,
        imms = [FORMAT, MASK, PIXEL_OFFSET], can_reorder = False,
        schedule_class = "coverage")

op("st_tile", (0x09, 0x7F, 8, _), dests = 0, srcs = 2,
      can_eliminate = False, imms = [FORMAT, MASK, PIXEL_OFFSET],
      schedule_class = "coverage")

for (name, exact) in [("any", 0xC000), ("none", 0xC020), ("none_after", 0xC020)]:
   op("jmp_exec_" + name, (exact, (1 << 16) - 1, 6, _), dests = 0, srcs = 0,
         can_eliminate = False, schedule_class = "invalid", imms = [TARGET])

# TODO: model implicit r0l destinations
op("pop_exec", (0x52 | (0x3 << 9), ((1 << 48) - 1) ^ (0x3 << 7) ^ (0x3 << 11), 6, _),
      dests = 0, srcs = 0, can_eliminate = False, schedule_class = "invalid",
      imms = [NEST])

for is_float in [False, True]:
   mod_mask = 0 if is_float else (0x3 << 26) | (0x3 << 38)

   for (cf, cf_op) in [("if", 0), ("else", 1), ("while", 2)]:
      name = "{}_{}cmp".format(cf, "f" if is_float else "i")
      exact = 0x42 | (0x0 if is_float else 0x10) | (cf_op << 9)
      mask = 0x7F | (0x3 << 9) | mod_mask | (0x3 << 44)
      imms = [NEST, FCOND if is_float else ICOND, INVERT_COND, TARGET]

      op(name, (exact, mask, 6, _), dests = 0, srcs = 2, can_eliminate = False,
            imms = imms, is_float = is_float,
            schedule_class = "preload" if cf == "else" else "invalid")

op("bitop", (0x7E, 0x7F, 6, _), srcs = 2, imms = [TRUTH_TABLE])
op("intl", (0x3E, 0x7F, 6, _), srcs = 2, imms = [])
op("convert", (0x3E | L, 0x7F | L | (0x3 << 38), 6, _), srcs = 2, imms = [ROUND]) 

# Sources are the coeffient register and the sample index (if applicable)
op("iter", (0x21, 0xBF, 8, _), srcs = 2, imms = [CHANNELS, INTERPOLATION])

# Sources are the coeffient register for the varying, the coefficient register
# for W, and the sample index (if applicable)
op("iterproj", (0x21, 0xBF, 8, _), srcs = 3, imms = [CHANNELS, INTERPOLATION])

op("ldcf", (0xA1, 0xBF, 8, _), srcs = 1, imms = [CHANNELS])
op("st_vary", None, dests = 0, srcs = 2, can_eliminate = False)
op("no_varyings", (0x80000051, 0xFFFFFFFF, 4, _), dests = 0, can_eliminate = False)
op("stop", (0x88, 0xFFFF, 2, _), dests = 0, can_eliminate = False,
   schedule_class = "invalid")
op("trap", (0x08, 0xFFFF, 2, _), dests = 0, can_eliminate = False,
   schedule_class = "invalid")
op("wait_pix", (0x48, 0xFF, 4, _), dests = 0, imms = [WRITEOUT],
   can_eliminate = False, schedule_class = "coverage")
op("signal_pix", (0x58, 0xFF, 4, _), dests = 0, imms = [WRITEOUT],
   can_eliminate = False, schedule_class = "coverage")

# Sources are the data vector, the coordinate vector, the LOD, the bindless
# table if present (zero for texture state registers), and texture index.
op("image_write", (0xF1 | (1 << 23) | (9 << 43), 0xFF, 6, 8), dests = 0, srcs = 5, imms
   = [DIM], can_eliminate = False, schedule_class = "store")

# Sources are the image, the offset within shared memory, and the layer.
# TODO: Do we need the short encoding?
op("block_image_store", (0xB1, 0xFF, 10, _), dests = 0, srcs = 3,
   imms = [FORMAT, DIM], can_eliminate = False, schedule_class = "store")

# Barriers
op("threadgroup_barrier", (0x0068, 0xFFFF, 2, _), dests = 0, srcs = 0,
   can_eliminate = False, schedule_class = "barrier")

def memory_barrier(name, a, b, c):
    op(name, (0xF5 | (a << 10) | (b << 8) | (c << 12), 0xFFFF, 2, _), dests = 0, srcs = 0,
       can_eliminate = False, schedule_class = "barrier")

memory_barrier("memory_barrier", 1, 2, 9)

# TODO: Not clear what these individually are. Some might be cache flushes?
memory_barrier("image_barrier_1", 2, 2, 10)
memory_barrier("image_barrier_2", 3, 2, 10)
memory_barrier("image_barrier_3", 2, 1, 10)
memory_barrier("image_barrier_4", 3, 1, 10)

memory_barrier("flush_memory_to_texture", 0, 0, 4)

memory_barrier("memory_barrier_2", 2, 2, 9)
memory_barrier("memory_barrier_3", 2, 1, 9)
memory_barrier("unknown_barrier_1", 0, 3, 3)
memory_barrier("unknown_barrier_2", 0, 3, 0)

op("doorbell", (0x60020 | 0x28 << 32, (1 << 48) - 1, 6, _), dests = 0,
      can_eliminate = False, can_reorder = False, imms = [IMM])

op("stack_unmap", (0x00075, (1 << 24) - 1, 8, _), dests = 1, srcs = 0, can_eliminate = False, can_reorder = False, imms = [IMM])
op("stack_map",   (0x10075, (1 << 24) - 1, 8, _), dests = 0, srcs = 1, can_eliminate = False, can_reorder = False, imms = [IMM])

op("stack_adjust",
      encoding_32 = (0x10100b5, (1 << 26) - 1, 8, _),
      dests = 0, srcs = 0, can_eliminate = False, can_reorder = False,
      imms = [STACK_SIZE], schedule_class = "store")

# source is offset
op("stack_load",
      encoding_32 = (0x35, (1 << 20) - 1, 6, 8),
      srcs = 1, imms = [FORMAT, MASK, SCOREBOARD], can_reorder = False,
      schedule_class = "load")

# sources are value and offset
op("stack_store",
      encoding_32 = (0xb5, (1 << 20) - 1, 6, 8),
      dests = 0, srcs = 2, imms = [FORMAT, MASK, SCOREBOARD],
      can_eliminate=False, schedule_class = "store")

# Convenient aliases.
op("mov", _, srcs = 1)
op("not", _, srcs = 1)
op("xor", _, srcs = 2)
op("and", _, srcs = 2)
op("or", _, srcs = 2)

op("collect", _, srcs = VARIABLE)
op("split", _, srcs = 1, dests = VARIABLE)
op("phi", _, srcs = VARIABLE, schedule_class = "preload")

op("unit_test", _, dests = 0, srcs = 1, can_eliminate = False)

# Like mov, but takes a register and can only appear at the start. Guaranteed
# to be coalesced during RA, rather than lowered to a real move. 
op("preload", _, srcs = 1, schedule_class = "preload")

# Pseudo-instructions to set the nesting counter. Lowers to r0l writes after RA.
op("begin_cf", _, dests = 0, can_eliminate = False)
op("break", _, dests = 0, imms = [NEST, TARGET], can_eliminate = False,
   schedule_class = "invalid")

for (name, is_float) in [("break_if_icmp", False), ("break_if_fcmp", True)]:
    op(name, _, dests = 0, srcs = 2,
       imms = [NEST, INVERT_COND, FCOND if is_float else ICOND, TARGET],
       can_eliminate = False, schedule_class = "invalid")
