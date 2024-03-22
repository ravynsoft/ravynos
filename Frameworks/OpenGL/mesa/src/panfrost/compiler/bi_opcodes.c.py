#
# Copyright (C) 2020 Collabora, Ltd.
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

TEMPLATE = """#include "bi_opcodes.h"
<%
def hasmod(mods, name):
        return 1 if name in mods else 0
%>
struct bi_op_props bi_opcode_props[BI_NUM_OPCODES] = {
% for opcode in sorted(mnemonics):
    <%
        add = instructions["+" + opcode][0][1] if "+" + opcode in instructions else None
        size = typesize(opcode)
        message = add["message"].upper() if add else "NONE"
        sr_count = add["staging_count"].upper() if add else "0"
        sr_read = int(add["staging"] in ["r", "rw"] if add else False)
        sr_write = int(add["staging"] in ["w", "rw"] if add else False)
        last = int(bool(add["last"]) if add else False)
        table = int(bool(add["table"]) if add else False)
        branch = int(opcode.startswith('BRANCH'))
        has_fma = int("*" + opcode in instructions)
        has_add = int("+" + opcode in instructions)
        mods = ops[opcode]['modifiers']
        clamp = hasmod(mods, 'clamp')
        not_result = hasmod(mods, 'not_result')
        abs = hasmod(mods, 'abs0') | (hasmod(mods, 'abs1') << 1) | (hasmod(mods, 'abs2') << 2)
        neg = hasmod(mods, 'neg0') | (hasmod(mods, 'neg1') << 1) | (hasmod(mods, 'neg2') << 2)
        m_not = hasmod(mods, 'not1')
    %>
    [BI_OPCODE_${opcode.replace('.', '_').upper()}] = {
        "${opcode}", BIFROST_MESSAGE_${message}, BI_SIZE_${size},
        BI_SR_COUNT_${sr_count}, ${sr_read}, ${sr_write}, ${last}, ${branch},
        ${table}, ${has_fma}, ${has_add}, ${clamp}, ${not_result}, ${abs},
        ${neg}, ${m_not},
    },
% endfor
};"""

import sys
from bifrost_isa import *
from mako.template import Template

instructions = parse_instructions(sys.argv[1], include_pseudo = True)
ir_instructions = partition_mnemonics(instructions)
mnemonics = set(x[1:] for x in instructions.keys())

print(Template(COPYRIGHT + TEMPLATE).render(ops = ir_instructions, mnemonics = mnemonics, instructions = instructions, typesize = typesize))
