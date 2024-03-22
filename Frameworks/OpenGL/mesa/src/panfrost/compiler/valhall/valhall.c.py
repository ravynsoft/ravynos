#encoding=utf-8

# Copyright (C) 2021 Collabora, Ltd.
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

from valhall import immediates, instructions, typesize
from mako.template import Template
from mako import exceptions

SKIP = set([
        # Extra conversions
        "S8_TO_S16",
        "S8_TO_F16",
        "U8_TO_U16",
        "U8_TO_F16",

        # Saturating multiplies
        "IMUL.s32",
        "IMUL.v2s16",
        "IMUL.v4s8",

        # 64-bit support
        "IADD.u64",
        "IADD.s64",
        "ISUB.u64",
        "ISUB.s64",
        "IMULD.u64",
        "SHADDX.u64",
        "SHADDX.s64",
        "IMULD.u64",
        "CLPER.s64",
        "CLPER.u64",
        "LSHIFT_AND.i64",
        "RSHIFT_AND.i64",
        "LSHIFT_OR.i64",
        "RSHIFT_OR.i64",
        "LSHIFT_XOR.i64",
        "RSHIFT_XOR.i64",
        "ATOM.i64",
        "ATOM_RETURN.i64",
        "ATOM1_RETURN.i64",

        # CLPER widens
        "CLPER.s32",
        "CLPER.v2s16",
        "CLPER.v4s8",
        "CLPER.v2u16",
        "CLPER.v4u8",

        # VAR_TEX
        "VAR_TEX_SINGLE",
        "VAR_TEX_GATHER",
        "VAR_TEX_GRADIENT",
        "VAR_TEX_DUAL",
        "VAR_TEX_BUF_SINGLE",
        "VAR_TEX_BUF_GATHER",
        "VAR_TEX_BUF_GRADIENT",
        "VAR_TEX_BUF_DUAL",

        # Special cased
        "FMA_RSCALE_N.f32",
        "FMA_RSCALE_LEFT.f32",
        "FMA_RSCALE_SCALE16.f32",

        # Deprecated instruction
        "NOT_OLD.i32",
        "NOT_OLD.i64",

        # TODO
        "IDP.v4s8",
        "IDP.v4u8",
        "FATAN_ASSIST.f32",
        "SEG_ADD.u64",
        "TEX_DUAL",
    ])

template = """
#include "valhall.h"
#include "bi_opcodes.h"

const uint32_t valhall_immediates[32] = {
% for imm in immediates:
    ${hex(imm)},
% endfor
};

<%
def ibool(x):
    return '1' if x else '0'

def hasmod(x, mod):
    return ibool(any([x.name == mod for x in op.modifiers]))

%>
const struct va_opcode_info
valhall_opcodes[BI_NUM_OPCODES] = {
% for op in instructions:
% if op.name not in skip:
<%
    name = op.name
    if name == 'BRANCHZ':
        name = 'BRANCHZ.i16'
    elif name == 'CUBEFACE2':
        name = 'CUBEFACE2_V9'

    sr_control = 0

    if len(op.staging) > 0:
        sr_control = op.staging[0].encoded_flags >> 6
%>
    [BI_OPCODE_${name.replace('.', '_').upper()}] = {
        .exact = ${hex(exact(op))}ULL,
        .srcs = {
% for src in ([sr for sr in op.staging if sr.read] + op.srcs):
            {
                .absneg = ${ibool(src.absneg)},
                .swizzle = ${ibool(src.swizzle)},
                .notted = ${ibool(src.notted)},
                .widen = ${ibool(src.widen)},
                .lanes = ${ibool(src.lanes)},
                .halfswizzle = ${ibool(src.halfswizzle)},
                .lane = ${ibool(src.lane)},
                .combine = ${ibool(src.combine)},
% if src.size in [8, 16, 32, 64]:
                .size = VA_SIZE_${src.size},
% endif
            },
% endfor
        },
        .type_size = ${typesize(op.name)},
        .has_dest = ${ibool(len(op.dests) > 0)},
        .is_signed = ${ibool(op.is_signed)},
        .unit = VA_UNIT_${op.unit},
        .nr_srcs = ${len(op.srcs)},
        .nr_staging_srcs = ${sum([sr.read for sr in op.staging])},
        .nr_staging_dests = ${sum([sr.write for sr in op.staging])},
        .clamp = ${hasmod(x, 'clamp')},
        .saturate = ${hasmod(x, 'saturate')},
        .rhadd = ${hasmod(x, 'rhadd')},
        .round_mode = ${hasmod(x, 'round_mode')},
        .condition = ${hasmod(x, 'condition')},
        .result_type = ${hasmod(x, 'result_type')},
        .vecsize = ${hasmod(x, 'vector_size')},
        .register_format = ${hasmod(x, 'register_format')},
        .slot = ${hasmod(x, 'slot')},
        .sr_count = ${hasmod(x, 'staging_register_count')},
        .sr_write_count = ${hasmod(x, 'staging_register_write_count')},
        .sr_control = ${sr_control},
    },
% endif
% endfor
};
"""

# Exact value to be ORed in to every opcode
def exact_op(op):
    return (op.opcode << 48) | (op.opcode2 << op.secondary_shift)

try:
    print(Template(template).render(immediates = immediates, instructions = instructions, skip = SKIP, exact = exact_op, typesize = typesize))
except:
    print(exceptions.text_error_template().render())
