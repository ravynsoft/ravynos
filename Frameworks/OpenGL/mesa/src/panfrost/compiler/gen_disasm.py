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

import sys
import itertools
from bifrost_isa import parse_instructions, opname_to_c, expand_states
from mako.template import Template

instructions = parse_instructions(sys.argv[1], include_unused = True)

# Constructs a reserved mask for a derived to cull impossible encodings

def reserved_mask(derived):
    ((pos, width), opts) = derived
    reserved = [x is None for x in opts]
    mask = sum([(y << x) for x, y in enumerate(reserved)])
    return (pos, width, mask)

def reserved_masks(op):
    masks = [reserved_mask(m) for m in op[2].get("derived", [])]
    return [m for m in masks if m[2] != 0]

# To decode instructions, pattern match based on the rules:
#
# 1. Execution unit (FMA or ADD) must line up.
# 2. All exact bits must match.
# 3. No fields should be reserved in a legal encoding.
# 4. Tiebreaker: Longer exact masks (greater unsigned bitwise inverses) win.
#
# To implement, filter the execution unit and check for exact bits in
# descending order of exact mask length.  Check for reserved fields per
# candidate and succeed if it matches.
# found.

def decode_op(instructions, is_fma):
    # Filter out the desired execution unit
    options = [n for n in instructions.keys() if (n[0] == '*') == is_fma]

    # Sort by exact masks, descending
    MAX_MASK = (1 << (23 if is_fma else 20)) - 1
    options.sort(key = lambda n: (MAX_MASK ^ instructions[n][2]["exact"][0]))

    # Map to what we need to template
    mapped = [(opname_to_c(op), instructions[op][2]["exact"], reserved_masks(instructions[op])) for op in options]

    # Generate checks in order
    template = """void
bi_disasm_${unit}(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("    ", fp);

% for (i, (name, (emask, ebits), derived)) in enumerate(options):
% if len(derived) > 0:
    ${"else " if i > 0 else ""}if (unlikely(((bits & ${hex(emask)}) == ${hex(ebits)})
% for (pos, width, reserved) in derived:
        && !(${hex(reserved)} & (1 << _BITS(bits, ${pos}, ${width})))
% endfor
    ))
% else:
    ${"else " if i > 0 else ""}if (unlikely(((bits & ${hex(emask)}) == ${hex(ebits)})))
% endif
        bi_disasm_${name}(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
% endfor
    else
        fprintf(fp, "INSTR_INVALID_ENC ${unit} %X", bits);

    fputs("\\n", fp);
}"""

    return Template(template).render(options = mapped, unit = "fma" if is_fma else "add")

# Decoding emits a series of function calls to e.g. `fma_fadd_v2f16`. We need to
# emit functions to disassemble a single decoded instruction in a particular
# state. Sync prototypes to avoid moves when calling.

disasm_op_template = Template("""static void
bi_disasm_${c_name}(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    ${body.strip()}
}
""")

lut_template_only = Template("""    static const char *${field}[] = {
        ${", ".join(['"' + x + '"' for x in table])}
    };
""")

# Given a lookup table written logically, generate an accessor
lut_template = Template("""    static const char *${field}_table[] = {
        ${", ".join(['"' + x + '"' for x in table])}
    };

    const char *${field} = ${field}_table[_BITS(bits, ${pos}, ${width})];
""")

# Helpers for decoding follow. pretty_mods applies dot syntax

def pretty_mods(opts, default):
    return [('.' + (opt or 'reserved') if opt != default else '') for opt in opts]

# Recursively searches for the set of free variables required by an expression

def find_context_keys_expr(expr):
    if isinstance(expr, list):
        return set.union(*[find_context_keys_expr(x) for x in expr[1:]])
    elif expr[0] == '#':
        return set()
    else:
        return set([expr])

def find_context_keys(desc, test):
    keys = set()

    if len(test) > 0:
        keys |= find_context_keys_expr(test)

    for i, (_, vals) in enumerate(desc.get('derived', [])):
        for j, val in enumerate(vals):
            if val is not None:
                keys |= find_context_keys_expr(val)

    return keys
 
# Compiles a logic expression to Python expression, ctx -> { T, F }

EVALUATORS = {
        'and': ' and ',
        'or': ' or ',
        'eq': ' == ',
        'neq': ' != ',
}

def compile_derived_inner(expr, keys):
    if expr == []:
        return 'True'
    elif expr is None or expr[0] == 'alias':
        return 'False'
    elif isinstance(expr, list):
        args = [compile_derived_inner(arg, keys) for arg in expr[1:]]
        return '(' + EVALUATORS[expr[0]].join(args) + ')'
    elif expr[0] == '#':
        return "'{}'".format(expr[1:])
    elif expr == 'ordering':
        return expr
    else:
        return "ctx[{}]".format(keys.index(expr))

def compile_derived(expr, keys):
    return eval('lambda ctx, ordering: ' + compile_derived_inner(expr, keys))

# Generate all possible combinations of values and evaluate the derived values
# by bruteforce evaluation to generate a forward mapping (values -> deriveds)

def evaluate_forward_derived(vals, ctx, ordering):
    for j, expr in enumerate(vals):
        if expr(ctx, ordering):
            return j

    return None

def evaluate_forward(keys, derivf, testf, ctx, ordering):
    if not testf(ctx, ordering):
        return None

    deriv = []

    for vals in derivf:
        evaled = evaluate_forward_derived(vals, ctx, ordering) 

        if evaled is None:
            return None

        deriv.append(evaled)

    return deriv

def evaluate_forwards(keys, derivf, testf, mod_vals, ordered):
    orderings = ["lt", "gt"] if ordered else [None]
    return [[evaluate_forward(keys, derivf, testf, i, order) for i in itertools.product(*mod_vals)] for order in orderings]

# Invert the forward mapping (values -> deriveds) of finite sets to produce a
# backwards mapping (deriveds -> values), suitable for disassembly. This is
# possible since the encoding is unambiguous, so this mapping is a bijection
# (after reserved/impossible encodings)

def invert_lut(value_size, forward, derived, mod_map, keys, mod_vals):
    backwards = [None] * (1 << value_size)
    for (i, deriveds), ctx in zip(enumerate(forward), itertools.product(*mod_vals)):
        # Skip reserved
        if deriveds == None:
            continue

        shift = 0
        param = 0
        for j, ((x, width), y) in enumerate(derived):
            param += (deriveds[j] << shift)
            shift += width

        assert(param not in backwards)
        backwards[param] = ctx

    return backwards

# Compute the value of all indirectly specified modifiers by using the
# backwards mapping (deriveds -> values) as a run-time lookup table.

def build_lut(mnemonic, desc, test):
    # Construct the system
    facts = []

    mod_map = {}

    for ((name, pos, width), default, values) in desc.get('modifiers', []):
        mod_map[name] = (width, values, pos, default)

    derived = desc.get('derived', [])

    # Find the keys and impose an order
    key_set = find_context_keys(desc, test)
    ordered = 'ordering' in key_set
    key_set.discard('ordering')
    keys = sorted(list(key_set))

    # Evaluate the deriveds for every possible state, forming a (state -> deriveds) map
    testf = compile_derived(test, keys)
    derivf = [[compile_derived(expr, keys) for expr in v] for (_, v) in derived]
    mod_vals = [mod_map[k][1] for k in keys]
    forward = evaluate_forwards(keys, derivf, testf, mod_vals, ordered)

    # Now invert that map to get a (deriveds -> state) map
    value_size = sum([width for ((x, width), y) in derived])
    backwards = [invert_lut(value_size, f, derived, mod_map, keys, mod_vals) for f in forward]

    # From that map, we can generate LUTs
    output = ""

    if ordered:
        output += "bool ordering = (_BITS(bits, {}, 3) > _BITS(bits, {}, 3));\n".format(desc["srcs"][0][0], desc["srcs"][1][0])

    for j, key in enumerate(keys):
        # Only generate tables for indirect specifiers
        if mod_map[key][2] is not None:
            continue

        idx_parts = []
        shift = 0

        for ((pos, width), _) in derived:
            idx_parts.append("(_BITS(bits, {}, {}) << {})".format(pos, width, shift))
            shift += width

        built_idx = (" | ".join(idx_parts)) if len(idx_parts) > 0 else "0"

        default = mod_map[key][3]

        if ordered:
            for i, order in enumerate(backwards):
                options = [ctx[j] if ctx is not None and ctx[j] is not None else "reserved" for ctx in order]
                output += lut_template_only.render(field = key + "_" + str(i), table = pretty_mods(options, default))

            output += "    const char *{} = ordering ? {}_1[{}] : {}_0[{}];\n".format(key, key, built_idx, key, built_idx)
        else:
            options = [ctx[j] if ctx is not None and ctx[j] is not None else "reserved" for ctx in backwards[0]]
            output += lut_template_only.render(field = key + "_table", table = pretty_mods(options, default))
            output += "    const char *{} = {}_table[{}];\n".format(key, key, built_idx)

    return output

def disasm_mod(mod, skip_mods):
    if mod[0][0] in skip_mods:
        return ''
    else:
        return '    fputs({}, fp);\n'.format(mod[0][0])

def disasm_op(name, op):
    (mnemonic, test, desc) = op
    is_fma = mnemonic[0] == '*'

    # Modifiers may be either direct (pos is not None) or indirect (pos is
    # None). If direct, we just do the bit lookup. If indirect, we use a LUT.

    body = ""
    skip_mods = []

    body += build_lut(mnemonic, desc, test)

    for ((mod, pos, width), default, opts) in desc.get('modifiers', []):
        if pos is not None:
            body += lut_template.render(field = mod, table = pretty_mods(opts, default), pos = pos, width = width) + "\n"

    # Mnemonic, followed by modifiers
    body += '    fputs("{}", fp);\n'.format(mnemonic)

    srcs = desc.get('srcs', [])

    for mod in desc.get('modifiers', []):
        # Skip per-source until next block
        if mod[0][0][-1] in "0123" and int(mod[0][0][-1]) < len(srcs):
            continue

        body += disasm_mod(mod, skip_mods)

    body += '    fputs(" ", fp);\n'
    body += '    bi_disasm_dest_{}(fp, next_regs, last);\n'.format('fma' if is_fma else 'add')

    # Next up, each source. Source modifiers are inserterd here

    for i, (pos, mask) in enumerate(srcs):
        body += '    fputs(", ", fp);\n'
        body += '    dump_src(fp, _BITS(bits, {}, 3), *srcs, branch_offset, consts, {});\n'.format(pos, "true" if is_fma else "false")

        # Error check if needed
        if (mask != 0xFF):
            body += '    if (!({} & (1 << _BITS(bits, {}, 3)))) fputs("(INVALID)", fp);\n'.format(hex(mask), pos, 3)

        # Print modifiers suffixed with this src number (e.g. abs0 for src0)
        for mod in desc.get('modifiers', []):
            if mod[0][0][-1] == str(i):
                body += disasm_mod(mod, skip_mods)

    # And each immediate
    for (imm, pos, width) in desc.get('immediates', []):
        body += '    fprintf(fp, ", {}:%u", _BITS(bits, {}, {}));\n'.format(imm, pos, width)

    # Attach a staging register if one is used
    if desc.get('staging'):
        body += '    fprintf(fp, ", @r%u", staging_register);\n'

    return disasm_op_template.render(c_name = opname_to_c(name), body = body)

print('#include "util/macros.h"')
print('#include "bifrost/disassemble.h"')

states = expand_states(instructions)
print('#define _BITS(bits, pos, width) (((bits) >> (pos)) & ((1 << (width)) - 1))')

for st in states:
    print(disasm_op(st, states[st]))

print(decode_op(states, True))
print(decode_op(states, False))
