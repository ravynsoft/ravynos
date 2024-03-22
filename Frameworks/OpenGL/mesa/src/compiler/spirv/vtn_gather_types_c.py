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

def find_result_types(spirv):
    seen = set()
    for inst in spirv['instructions']:
        # Handle aliases by choosing the first one in the grammar.
        if inst['opcode'] in seen:
            continue
        seen.add(inst['opcode'])

        name = inst['opname']

        if 'operands' not in inst:
            continue

        res_arg_idx = -1
        res_type_arg_idx = -1
        for idx, arg in enumerate(inst['operands']):
            if arg['kind'] == 'IdResult':
                res_arg_idx = idx
            elif arg['kind'] == 'IdResultType':
                res_type_arg_idx = idx

        if res_type_arg_idx >= 0:
            assert res_arg_idx >= 0
        elif res_arg_idx >= 0:
            untyped_insts = [
                'OpString',
                'OpExtInstImport',
                'OpDecorationGroup',
                'OpLabel',
            ]
            assert name.startswith('OpType') or name.startswith('OpAlias') or name in untyped_insts

        if res_arg_idx >= 0 or res_type_arg_idx >= 0:
            yield (name, res_arg_idx, res_type_arg_idx)

TEMPLATE  = Template(COPYRIGHT + """\

/* DO NOT EDIT - This file is generated automatically by the
 * vtn_gather_types_c.py script
 */

#include "vtn_private.h"

struct type_args {
    int res_idx;
    int res_type_idx;
};

static struct type_args
result_type_args_for_opcode(SpvOp opcode)
{
   switch (opcode) {
% for opcode in opcodes:
   case Spv${opcode[0]}: return (struct type_args){ ${opcode[1]}, ${opcode[2]} };
% endfor
   default: return (struct type_args){ -1, -1 };
   }
}

bool
vtn_set_instruction_result_type(struct vtn_builder *b, SpvOp opcode,
                                const uint32_t *w, unsigned count)
{
   struct type_args args = result_type_args_for_opcode(opcode);

   if (args.res_idx >= 0 && args.res_type_idx >= 0) {
      struct vtn_value *val = vtn_untyped_value(b, w[1 + args.res_idx]);
      val->type = vtn_value(b, w[1 + args.res_type_idx],
                            vtn_value_type_type)->type;
   }

   return true;
}

""")

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("json")
    p.add_argument("out")
    args = p.parse_args()

    spirv_info = json.JSONDecoder().decode(open(args.json, "r").read())

    opcodes = list(find_result_types(spirv_info))

    try:
        with open(args.out, 'w', encoding='utf-8') as f:
            f.write(TEMPLATE.render(opcodes=opcodes))
    except Exception:
        # In the even there's an error this imports some helpers from mako
        # to print a useful stack trace and prints it, then exits with
        # status 1, if python is run with debug; otherwise it just raises
        # the exception
        if __debug__:
            import sys
            from mako import exceptions
            sys.stderr.write(exceptions.text_error_template().render() + '\n')
            sys.exit(1)
        raise
