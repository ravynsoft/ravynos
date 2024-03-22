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

TEMPLATE = """#include <stdio.h>
#include "compiler.h"

static const char *
bi_swizzle_as_str(enum bi_swizzle swz)
{
        switch (swz) {
        case BI_SWIZZLE_H00: return ".h00";
        case BI_SWIZZLE_H01: return "";
        case BI_SWIZZLE_H10: return ".h10";
        case BI_SWIZZLE_H11: return ".h11";
        case BI_SWIZZLE_B0000: return ".b0";
        case BI_SWIZZLE_B1111: return ".b1";
        case BI_SWIZZLE_B2222: return ".b2";
        case BI_SWIZZLE_B3333: return ".b3";
        case BI_SWIZZLE_B0011: return ".b0011";
        case BI_SWIZZLE_B2233: return ".b2233";
        case BI_SWIZZLE_B1032: return ".b1032";
        case BI_SWIZZLE_B3210: return ".b3210";
        case BI_SWIZZLE_B0022: return ".b0022";
        }

        unreachable("Invalid swizzle");
}

static const char *
bir_fau_name(unsigned fau_idx)
{
    const char *names[] = {
            "zero", "lane-id", "wrap-id", "core-id", "fb-extent",
            "atest-param", "sample-pos", "reserved",
            "blend_descriptor_0", "blend_descriptor_1",
            "blend_descriptor_2", "blend_descriptor_3",
            "blend_descriptor_4", "blend_descriptor_5",
            "blend_descriptor_6", "blend_descriptor_7",
            "tls_ptr", "wls_ptr", "program_counter",
    };

    assert(fau_idx < ARRAY_SIZE(names));
    return names[fau_idx];
}

static const char *
bir_passthrough_name(unsigned idx)
{
    const char *names[] = {
            "s0", "s1", "s2", "t", "fau.x", "fau.y", "t0", "t1"
    };

    assert(idx < ARRAY_SIZE(names));
    return names[idx];
}

static void
bi_print_index(FILE *fp, bi_index index)
{
    if (index.discard)
        fputs("^", fp);

    if (bi_is_null(index))
        fprintf(fp, "_");
    else if (index.type == BI_INDEX_CONSTANT)
        fprintf(fp, "#0x%x", index.value);
    else if (index.type == BI_INDEX_FAU && index.value >= BIR_FAU_UNIFORM)
        fprintf(fp, "u%u", index.value & ~BIR_FAU_UNIFORM);
    else if (index.type == BI_INDEX_FAU)
        fprintf(fp, "%s", bir_fau_name(index.value));
    else if (index.type == BI_INDEX_PASS)
        fprintf(fp, "%s", bir_passthrough_name(index.value));
    else if (index.type == BI_INDEX_REGISTER)
        fprintf(fp, "r%u", index.value);
    else if (index.type == BI_INDEX_NORMAL)
        fprintf(fp, "%u", index.value);
    else
        unreachable("Invalid index");

    if (index.offset)
        fprintf(fp, "[%u]", index.offset);

    if (index.abs)
        fputs(".abs", fp);

    if (index.neg)
        fputs(".neg", fp);

    fputs(bi_swizzle_as_str(index.swizzle), fp);
}

% for mod in sorted(modifiers):
% if len(modifiers[mod]) > 2: # otherwise just boolean

UNUSED static inline const char *
bi_${mod}_as_str(enum bi_${mod} ${mod})
{
    switch (${mod}) {
% for i, state in enumerate(sorted(modifiers[mod])):
% if state == "none":
    case BI_${mod.upper()}_NONE: return "";
% elif state != "reserved":
    case BI_${mod.upper()}_${state.upper()}: return ".${state.lower()}";
% endif
% endfor
    }

    unreachable("Invalid ${mod}");
};
% endif
% endfor

<%def name="print_modifiers(mods, table)">
    % for mod in mods:
    % if mod not in ["lane_dest"]:
    % if len(table[mod]) > 2:
        fputs(bi_${mod}_as_str(I->${mod}), fp);
    % else:
        if (I->${mod}) fputs(".${mod}", fp);
    % endif
    % endif
    % endfor
</%def>

<%def name="print_source_modifiers(mods, src, table)">
    % for mod in mods:
    % if mod[0:-1] not in ["lane", "lanes", "replicate", "swz", "widen", "swap", "abs", "neg", "sign", "not"]:
    % if len(table[mod[0:-1]]) > 2:
        fputs(bi_${mod[0:-1]}_as_str(I->${mod[0:-1]}[${src}]), fp);
    % elif mod == "bytes2":
        if (I->bytes2) fputs(".bytes", fp);
    % else:
        if (I->${mod[0:-1]}[${src}]) fputs(".${mod[0:-1]}", fp);
    % endif
    %endif
    % endfor
</%def>

void
bi_print_instr(const bi_instr *I, FILE *fp)
{
    fputs("   ", fp);

    bi_foreach_dest(I, d) {
        if (d > 0) fprintf(fp, ", ");

        bi_print_index(fp, I->dest[d]);
    }

    if (I->nr_dests > 0)
        fputs(" = ", fp);

    fprintf(fp, "%s", bi_opcode_props[I->op].name);

    if (I->table)
        fprintf(fp, ".table%u", I->table);

    if (I->flow)
        fprintf(fp, ".flow%u", I->flow);

    if (I->op == BI_OPCODE_COLLECT_I32 || I->op == BI_OPCODE_PHI) {
        for (unsigned i = 0; i < I->nr_srcs; ++i) {
            if (i > 0)
                fputs(", ", fp);
            else
                fputs(" ", fp);

            bi_print_index(fp, I->src[i]);
        }
    }

    switch (I->op) {
% for opcode in ops:
<%
    # Extract modifiers that are not per-source
    root_modifiers = [x for x in ops[opcode]["modifiers"] if x[-1] not in "0123"]
%>
    case BI_OPCODE_${opcode.replace('.', '_').upper()}:
        ${print_modifiers(root_modifiers, modifiers)}
        fputs(" ", fp);
    % for src in range(src_count(ops[opcode])):
    % if src > 0:
        fputs(", ", fp);
    % endif
        bi_print_index(fp, I->src[${src}]);
        ${print_source_modifiers([m for m in ops[opcode]["modifiers"] if m[-1] == str(src)], src, modifiers)}
    % endfor
    % for imm in ops[opcode]["immediates"]:
        fprintf(fp, ", ${imm}:%u", I->${imm});
    % endfor
        break;
% endfor
    default:
        unreachable("Invalid opcode");
    }

    if (I->branch_target)
            fprintf(fp, " -> block%u", I->branch_target->index);

    fputs("\\n", fp);

}"""

import sys
from bifrost_isa import *
from mako.template import Template

instructions = parse_instructions(sys.argv[1], include_pseudo = True)
ir_instructions = partition_mnemonics(instructions)
modifier_lists = order_modifiers(ir_instructions)

print(Template(COPYRIGHT + TEMPLATE).render(ops = ir_instructions, modifiers = modifier_lists, src_count = src_count))
