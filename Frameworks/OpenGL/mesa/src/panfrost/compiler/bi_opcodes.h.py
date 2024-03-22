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

TEMPLATE = """
#ifndef _BI_OPCODES_H_
#define _BI_OPCODES_H_

#include "bifrost.h"
 
% for mod in sorted(modifiers):
% if len(modifiers[mod]) > 2: # otherwise just boolean
enum bi_${mod.lower()} {
% for i, state in enumerate(modifiers[mod]):
% if state != "reserved":
    BI_${mod.upper()}_${state.upper()} = ${i},
% endif
% endfor
};

% endif
% endfor
enum bi_opcode {
% for opcode in sorted(mnemonics):
    BI_OPCODE_${opcode.replace('.', '_').upper()},
% endfor
    BI_NUM_OPCODES
};

/* Number of staging registers accessed, note this fits into 3-bits */

enum bi_sr_count {
    /* fixed counts */
    BI_SR_COUNT_0 = 0,
    BI_SR_COUNT_1 = 1,
    BI_SR_COUNT_2 = 2,
    BI_SR_COUNT_3 = 3,
    BI_SR_COUNT_4 = 4,

    /* derived from register_format and vecsize */
    BI_SR_COUNT_FORMAT = 5,

    /* equal to vecsize alone */
    BI_SR_COUNT_VECSIZE = 6,

    /* specified directly as the sr_count immediate */
    BI_SR_COUNT_SR_COUNT = 7
};

enum bi_size {
   BI_SIZE_8 = 0,
   BI_SIZE_16,
   BI_SIZE_24,
   BI_SIZE_32,
   BI_SIZE_48,
   BI_SIZE_64,
   BI_SIZE_96,
   BI_SIZE_128,
};

/* Description of an opcode in the IR */
struct bi_op_props {
        const char *name;

        enum bifrost_message_type message : 4;
        enum bi_size size : 3;
        enum bi_sr_count sr_count : 3;
        bool sr_read : 1;
        bool sr_write : 1;
        bool last : 1;
        bool branch : 1;
        bool table : 1;
        bool fma : 1;
        bool add : 1;

        /* Supported propagable modifiers */
        bool clamp : 1;
        bool not_result : 1;
        unsigned abs : 3;
        unsigned neg : 3;
        bool not_mod : 1;
};

/* Generated in bi_opcodes.c.py */
extern struct bi_op_props bi_opcode_props[BI_NUM_OPCODES];

#endif
"""

import sys
from bifrost_isa import *
from mako.template import Template

instructions = parse_instructions(sys.argv[1], include_pseudo = True)
ir_instructions = partition_mnemonics(instructions)
modifier_lists = order_modifiers(ir_instructions)

# Generate sorted list of mnemonics without regard to unit
mnemonics = set(x[1:] for x in instructions.keys())

print(Template(COPYRIGHT + TEMPLATE).render(mnemonics = mnemonics, modifiers = modifier_lists))
