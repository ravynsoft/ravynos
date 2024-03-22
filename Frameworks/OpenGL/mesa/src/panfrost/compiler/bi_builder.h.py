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

SKIP = set(["lane", "lane_dest", "lanes", "lanes", "replicate", "swz", "widen",
    "swap", "neg", "abs", "not", "sign", "extend", "divzero", "clamp", "sem",
    "not_result", "skip", "round", "ftz"])

TEMPLATE = """
#ifndef _BI_BUILDER_H_
#define _BI_BUILDER_H_

#include "compiler.h"

<%
# For <32-bit loads/stores, the default extend `none` with a natural sized
# input is not encodeable! To avoid a footgun, swap the default to `zext` which
# will work as expected
ZEXT_DEFAULT = set(["LOAD.i8", "LOAD.i16", "LOAD.i24", "STORE.i8", "STORE.i16", "STORE.i24"])

def nirtypes(opcode):
    split = opcode.split('.', 1)
    if len(split) < 2:
        split = opcode.split('_')

    if len(split) <= 1:
        return None

    assert len(split) > 1

    type = split[1]
    if type[0] == 'v':
        type = type[2:]

    if type[0] == 'f':
        return ['nir_type_float']
    elif type[0] == 's':
        return ['nir_type_int']
    elif type[0] == 'u':
        return ['nir_type_uint']
    elif type[0] == 'i':
        return ['nir_type_uint', 'nir_type_int']
    else:
        return None

def condition(opcode, typecheck, sizecheck):
    cond = ''
    if typecheck == True:
        cond += '('
        types = nirtypes(opcode)
        assert types != None
        for T in types:
            cond += "{}type == {}".format(' || ' if cond[-1] != '(' else '', T)
        cond += ')'

    if sizecheck == True:
        cond += "{}bitsize == {}".format(' && ' if cond != '' else '', typesize(opcode))

    cmpf_mods = ops[opcode]["modifiers"]["cmpf"] if "cmpf" in ops[opcode]["modifiers"] else None
    if "cmpf" in ops[opcode]["modifiers"]:
        cond += "{}(".format(' && ' if cond != '' else '')
        for cmpf in ops[opcode]["modifiers"]["cmpf"]:
            if cmpf != 'reserved':
                cond += "{}cmpf == BI_CMPF_{}".format(' || ' if cond[-1] != '(' else '', cmpf.upper())
        cond += ')'

    return 'true' if cond == '' else cond

def to_suffix(op):
    return "_to" if op["dests"] > 0 else ""

%>

% for opcode in ops:
static inline
bi_instr * bi_${opcode.replace('.', '_').lower()}${to_suffix(ops[opcode])}(${signature(ops[opcode], modifiers)})
{
<%
    op = ops[opcode]
    nr_dests = "nr_dests" if op["variable_dests"] else op["dests"]
    nr_srcs = "nr_srcs" if op["variable_srcs"] else src_count(op)
%>
    size_t size = sizeof(bi_instr) + sizeof(bi_index) * (${nr_dests} + ${nr_srcs});
    bi_instr *I = (bi_instr *) rzalloc_size(b->shader, size);

    I->op = BI_OPCODE_${opcode.replace('.', '_').upper()};
    I->nr_dests = ${nr_dests};
    I->nr_srcs = ${nr_srcs};
    I->dest = (bi_index *) (&I[1]);
    I->src = I->dest + ${nr_dests};

% if not op["variable_dests"]:
% for dest in range(op["dests"]):
    I->dest[${dest}] = dest${dest};
% endfor
%endif

% if not op["variable_srcs"]:
% for src in range(src_count(op)):
    I->src[${src}] = src${src};
% endfor
% endif

% for mod in ops[opcode]["modifiers"]:
% if not should_skip(mod, opcode):
    I->${mod} = ${mod};
% endif
% endfor
% if ops[opcode]["rtz"]:
    I->round = BI_ROUND_RTZ;
% endif
% for imm in ops[opcode]["immediates"]:
    I->${imm} = ${imm};
% endfor
% if opcode in ZEXT_DEFAULT:
    I->extend = BI_EXTEND_ZEXT;
% endif
    bi_builder_insert(&b->cursor, I);
    return I;
}

% if ops[opcode]["dests"] == 1 and not ops[opcode]["variable_dests"]:
static inline
bi_index bi_${opcode.replace('.', '_').lower()}(${signature(ops[opcode], modifiers, no_dests=True)})
{
    return (bi_${opcode.replace('.', '_').lower()}_to(${arguments(ops[opcode])}))->dest[0];
}

%endif
<%
    common_op = opcode.split('.')[0]
    variants = [a for a in ops.keys() if a.split('.')[0] == common_op]
    signatures = [signature(ops[op], modifiers, no_dests=True) for op in variants]
    homogenous = all([sig == signatures[0] for sig in signatures])
    types = [nirtypes(x) for x in variants]
    typeful = False
    for t in types:
        if t != types[0]:
            typeful = True

    sizes = [typesize(x) for x in variants]
    sized = False
    for size in sizes:
        if size != sizes[0]:
            sized = True

    last = opcode == variants[-1]
%>
% if homogenous and len(variants) > 1 and last:
% for (suffix, temp, dests, ret) in (('_to', False, 1, 'instr *'), ('', True, 0, 'index')):
% if not temp or ops[opcode]["dests"] > 0:
static inline
bi_${ret} bi_${common_op.replace('.', '_').lower()}${suffix if ops[opcode]['dests'] > 0 else ''}(${signature(ops[opcode], modifiers, typeful=typeful, sized=sized, no_dests=not dests)})
{
% for i, variant in enumerate(variants):
    ${"{}if ({})".format("else " if i > 0 else "", condition(variant, typeful, sized))}
        return (bi_${variant.replace('.', '_').lower()}${to_suffix(ops[opcode])}(${arguments(ops[opcode], temp_dest = temp)}))${"->dest[0]" if temp else ""};
% endfor
    else
        unreachable("Invalid parameters for ${common_op}");
}

%endif
%endfor
%endif
%endfor
#endif"""

import sys
from bifrost_isa import *
from mako.template import Template

instructions = parse_instructions(sys.argv[1], include_pseudo = True)
ir_instructions = partition_mnemonics(instructions)
modifier_lists = order_modifiers(ir_instructions)

# Generate type signature for a builder routine

def should_skip(mod, op):
    # FROUND and HADD only make sense in context of a round mode, so override
    # the usual skip
    if mod == "round" and ("FROUND" in op or "HADD" in op):
        return False

    return mod in SKIP or mod[0:-1] in SKIP

def modifier_signature(op):
    return sorted([m for m in op["modifiers"].keys() if not should_skip(m, op["key"])])

def signature(op, modifiers, typeful = False, sized = False, no_dests = False):
    return ", ".join(
        ["bi_builder *b"] +
        (["nir_alu_type type"] if typeful == True else []) +
        (["unsigned bitsize"] if sized == True else []) +
        (["unsigned nr_dests"] if op["variable_dests"] else
            ["bi_index dest{}".format(i) for i in range(0 if no_dests else op["dests"])]) +
        (["unsigned nr_srcs"] if op["variable_srcs"] else
            ["bi_index src{}".format(i) for i in range(src_count(op))]) +
        ["{} {}".format(
        "bool" if len(modifiers[T[0:-1]] if T[-1] in "0123" else modifiers[T]) == 2 else
        "enum bi_" + T[0:-1] if T[-1] in "0123" else
        "enum bi_" + T,
        T) for T in modifier_signature(op)] +
        ["uint32_t {}".format(imm) for imm in op["immediates"]])

def arguments(op, temp_dest = True):
    dest_pattern = "bi_temp(b->shader)" if temp_dest else 'dest{}'
    dests = [dest_pattern.format(i) for i in range(op["dests"])]
    srcs = ["src{}".format(i) for i in range(src_count(op))]

    # Variable source/destinations just pass in the count
    if op["variable_dests"]:
        dests = ["nr_dests"]

    if op["variable_srcs"]:
        srcs = ["nr_srcs"]

    return ", ".join(["b"] + dests + srcs + modifier_signature(op) + op["immediates"])

print(Template(COPYRIGHT + TEMPLATE).render(ops = ir_instructions, modifiers =
    modifier_lists, signature = signature, arguments = arguments, src_count =
    src_count, typesize = typesize, should_skip = should_skip))
