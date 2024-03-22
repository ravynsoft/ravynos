COPYRIGHT = """\
/*
 * Copyright (C) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
"""

import argparse
import json
from sys import stdout
from mako.template import Template

def collect_data(spirv, kind):
    for x in spirv["operand_kinds"]:
        if x["kind"] == kind:
            operands = x
            break

    # There are some duplicate values in some of the tables (thanks guys!), so
    # filter them out.
    seen = set()
    values = []
    for x in operands["enumerants"]:
        if x["value"] not in seen:
            seen.add(x["value"])
            values.append(x["enumerant"])

    return (kind, values, operands["category"])

def collect_opcodes(spirv):
    seen = set()
    values = []
    for x in spirv["instructions"]:
        # Handle aliases by choosing the first one in the grammar.
        # E.g. OpDecorateString and OpDecorateStringGOOGLE share same opcode.
        if x["opcode"] in seen:
            continue
        opcode = x["opcode"]
        name = x["opname"]
        assert name.startswith("Op")
        values.append(name[2:])
        seen.add(opcode)

    return ("Op", values, None)

def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument("json")
    p.add_argument("out")
    return p.parse_args()

TEMPLATE  = Template("""\
/* DO NOT EDIT - This file is generated automatically by spirv_info_c.py script */

""" + COPYRIGHT + """\
#include "spirv_info.h"
% for kind,values,category in info:

% if category == "BitEnum":
const char *
spirv_${kind.lower()}_to_string(Spv${kind}Mask v)
{
   switch (v) {
    % for name in values:
    %if name != "None":
   case Spv${kind}${name}Mask: return "Spv${kind}${name}";
    % else:
   case Spv${kind}MaskNone: return "Spv${kind}${name}";
    % endif
    % endfor
   }

   return "unknown";
}
% else:
const char *
spirv_${kind.lower()}_to_string(Spv${kind} v)
{
   switch (v) {
    % for name in values:
   case Spv${kind}${name}: return "Spv${kind}${name}";
    % endfor
   case Spv${kind}Max: break; /* silence warnings about unhandled enums. */
   }

   return "unknown";
}
% endif
% endfor
""")

if __name__ == "__main__":
    pargs = parse_args()

    spirv_info = json.JSONDecoder().decode(open(pargs.json, "r").read())

    info = [
        collect_data(spirv_info, "AddressingModel"),
        collect_data(spirv_info, "BuiltIn"),
        collect_data(spirv_info, "Capability"),
        collect_data(spirv_info, "Decoration"),
        collect_data(spirv_info, "Dim"),
        collect_data(spirv_info, "ExecutionMode"),
        collect_data(spirv_info, "ExecutionModel"),
        collect_data(spirv_info, "ImageFormat"),
        collect_data(spirv_info, "MemoryModel"),
        collect_data(spirv_info, "StorageClass"),
        collect_data(spirv_info, "ImageOperands"),
        collect_data(spirv_info, "FPRoundingMode"),
        collect_opcodes(spirv_info),
    ]

    with open(pargs.out, 'w', encoding='utf-8') as f:
        f.write(TEMPLATE.render(info=info))
