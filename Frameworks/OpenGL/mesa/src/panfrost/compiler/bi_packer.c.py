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
from bifrost_isa import *
from mako.template import Template

# Consider pseudo instructions when getting the modifier list
instructions_with_pseudo = parse_instructions(sys.argv[1], include_pseudo = True)
ir_instructions_with_pseudo = partition_mnemonics(instructions_with_pseudo)
modifier_lists = order_modifiers(ir_instructions_with_pseudo)

# ...but strip for packing
instructions = parse_instructions(sys.argv[1])
ir_instructions = partition_mnemonics(instructions)

# Packs sources into an argument. Offset argument to work around a quirk of our
# compiler IR when dealing with staging registers (TODO: reorder in the IR to
# fix this)
def pack_sources(sources, body, pack_exprs, offset, is_fma):
    for i, src in enumerate(sources):
        # FMA first two args are restricted, but that's checked once for all
        # FMA so the compiler has less work to do
        expected = 0xFB if (is_fma and i < 2) else 0xFF

        # Validate the source
        if src[1] != expected:
            assert((src[1] & expected) == src[1])
            body.append('assert((1 << src{}) & {});'.format(i, hex(src[1])))

        # Sources are state-invariant
        for state in pack_exprs:
            state.append('(src{} << {})'.format(i, src[0]))

# Try to map from a modifier list `domain` to the list `target`
def map_modifier(body, prefix, mod, domain, target):
    # We don't want to map reserveds, that's invalid IR anyway
    def reserved_to_none(arr):
        return [None if x == 'reserved' else x for x in arr]

    # Trim out reserveds at the end
    noned_domain = reserved_to_none(domain)
    noned_target = reserved_to_none(target)
    none_indices = [i for i, x in enumerate(noned_target) if x != None]
    trimmed = noned_target[0: none_indices[-1] + 1]

    if trimmed == noned_domain[0:len(trimmed)]:
        # Identity map, possibly on the left subset
        return mod
    else:
        # Generate a table as a fallback
        table = ", ".join([str(target.index(x)) if x in target else "~0" for x in domain])
        body.append("static uint8_t {}_table[] = {{ {} }};".format(prefix, table))

        if len(domain) > 2:
            # no need to validate bools
            body.append("assert({} < {});".format(mod, len(domain)))

        return "{}_table[{}]".format(prefix, mod)

def pick_from_bucket(opts, bucket):
    intersection = set(opts) & bucket
    assert(len(intersection) <= 1)
    return intersection.pop() if len(intersection) == 1 else None

def pack_modifier(mod, width, default, opts, body, pack_exprs):
    # Destructure the modifier name
    (raw, arg) = (mod[0:-1], mod[-1]) if mod[-1] in "0123" else (mod, 0)

    SWIZZLES = ["lane", "lanes", "replicate", "swz", "widen", "swap"]

    ir_value = "bytes2" if mod == "bytes2" else "{}[{}]".format(raw, arg) if mod[-1] in "0123" else mod
    lists = modifier_lists[raw]

    # Swizzles need to be packed "specially"
    SWIZZLE_BUCKETS = [
            set(['h00', 'h0']),
            set(['h01', 'none', 'b0123', 'w0']), # Identity
            set(['h10']),
            set(['h11', 'h1']),
            set(['b0000', 'b00', 'b0']),
            set(['b1111', 'b11', 'b1']),
            set(['b2222', 'b22', 'b2']),
            set(['b3333', 'b33', 'b3']),
            set(['b0011', 'b01']),
            set(['b2233', 'b23']),
            set(['b1032']),
            set(['b3210']),
            set(['b0022', 'b02'])
    ]

    if raw in SWIZZLES:
        # Construct a list
        lists = [pick_from_bucket(opts, bucket) for bucket in SWIZZLE_BUCKETS]
        ir_value = "src[{}].swizzle".format(arg)
    elif raw == "lane_dest":
        lists = [pick_from_bucket(opts, bucket) for bucket in SWIZZLE_BUCKETS]
        ir_value = "dest->swizzle"
    elif raw in ["abs", "sign"]:
        ir_value = "src[{}].abs".format(arg)
    elif raw in ["neg", "not"]:
        ir_value = "src[{}].neg".format(arg)

    ir_value = "I->{}".format(ir_value)

    # We need to map from ir_opts to opts
    mapped = map_modifier(body, mod, ir_value, lists, opts)
    body.append('unsigned {} = {};'.format(mod, mapped))
    body.append('assert({} < {});'.format(mod, 1 << width))

# Compiles an S-expression (and/or/eq/neq, modifiers, `ordering`, immediates)
# into a C boolean expression suitable to stick in an if-statement. Takes an
# imm_map to map modifiers to immediate values, parametrized by the ctx that
# we're looking up in (the first, non-immediate argument of the equality)

SEXPR_BINARY = {
        "and": "&&",
        "or": "||",
        "eq": "==",
        "neq": "!="
}

def compile_s_expr(expr, imm_map, ctx):
    if expr[0] == 'alias':
        return compile_s_expr(expr[1], imm_map, ctx)
    elif expr == ['eq', 'ordering', '#gt']:
        return '(src0 > src1)'
    elif expr == ['neq', 'ordering', '#lt']:
        return '(src0 >= src1)'
    elif expr == ['neq', 'ordering', '#gt']:
        return '(src0 <= src1)'
    elif expr == ['eq', 'ordering', '#lt']:
        return '(src0 < src1)'
    elif expr == ['eq', 'ordering', '#eq']:
        return '(src0 == src1)'
    elif isinstance(expr, list):
        sep = " {} ".format(SEXPR_BINARY[expr[0]])
        return "(" + sep.join([compile_s_expr(s, imm_map, expr[1]) for s in expr[1:]]) + ")"
    elif expr[0] == '#':
        return str(imm_map[ctx][expr[1:]])
    else:
        return expr

# Packs a derived value. We just iterate through the possible choices and test
# whether the encoding matches, and if so we use it.

def pack_derived(pos, exprs, imm_map, body, pack_exprs):
    body.append('unsigned derived_{} = 0;'.format(pos))

    first = True
    for i, expr in enumerate(exprs):
        if expr is not None:
            cond = compile_s_expr(expr, imm_map, None)
            body.append('{}if {} derived_{} = {};'.format('' if first else 'else ', cond, pos, i))
            first = False

    assert (not first)
    body.append('else unreachable("No pattern match at pos {}");'.format(pos))
    body.append('')

    assert(pos is not None)
    pack_exprs.append('(derived_{} << {})'.format(pos, pos))

# Generates a routine to pack a single variant of a single- instruction.
# Template applies the needed formatting and combine to OR together all the
# pack_exprs to avoid bit fields.
#
# Argument swapping is sensitive to the order of operations. Dependencies:
# sources (RW), modifiers (RW), derived values (W). Hence we emit sources and
# modifiers first, then perform a swap if necessary overwriting
# sources/modifiers, and last calculate derived values and pack.

variant_template = Template("""static inline unsigned
bi_pack_${name}(${", ".join(["bi_instr *I"] + ["enum bifrost_packed_src src{}".format(i) for i in range(srcs)])})
{
${"\\n".join([("    " + x) for x in common_body])}
% if single_state:
% for (pack_exprs, s_body, _) in states:
${"\\n".join(["    " + x for x in s_body + ["return {};".format( " | ".join(pack_exprs))]])}
% endfor
% else:
% for i, (pack_exprs, s_body, cond) in enumerate(states):
    ${'} else ' if i > 0 else ''}if ${cond} {
${"\\n".join(["        " + x for x in s_body + ["return {};".format(" | ".join(pack_exprs))]])}
% endfor
    } else {
        unreachable("No matching state found in ${name}");
    }
% endif
}
""")

def pack_variant(opname, states):
    # Expressions to be ORed together for the final pack, an array per state
    pack_exprs = [[hex(state[1]["exact"][1])] for state in states]

    # Computations which need to be done to encode first, across states
    common_body = []

    # Map from modifier names to a map from modifier values to encoded values
    # String -> { String -> Uint }. This can be shared across states since
    # modifiers are (except the pos values) constant across state.
    imm_map = {}

    # Pack sources. Offset over to deal with staging/immediate weirdness in our
    # IR (TODO: reorder sources upstream so this goes away). Note sources are
    # constant across states.
    staging = states[0][1].get("staging", "")
    offset = 0
    if staging in ["r", "rw"]:
        offset += 1

    pack_sources(states[0][1].get("srcs", []), common_body, pack_exprs, offset, opname[0] == '*')

    modifiers_handled = []
    for st in states:
        for ((mod, _, width), default, opts) in st[1].get("modifiers", []):
            if mod in modifiers_handled:
                continue

            modifiers_handled.append(mod)
            pack_modifier(mod, width, default, opts, common_body, pack_exprs)

            imm_map[mod] = { x: y for y, x in enumerate(opts) }

    for i, st in enumerate(states):
        for ((mod, pos, width), default, opts) in st[1].get("modifiers", []):
            if pos is not None:
                pack_exprs[i].append('({} << {})'.format(mod, pos))

    for ((src_a, src_b), cond, remap) in st[1].get("swaps", []):
        # Figure out which vars to swap, in order to swap the arguments. This
        # always includes the sources themselves, and may include source
        # modifiers (with the same source indices). We swap based on which
        # matches A, this is arbitrary but if we swapped both nothing would end
        # up swapping at all since it would swap back.

        vars_to_swap = ['src']
        for ((mod, _, width), default, opts) in st[1].get("modifiers", []):
            if mod[-1] in str(src_a):
                vars_to_swap.append(mod[0:-1])

        common_body.append('if {}'.format(compile_s_expr(cond, imm_map, None)) + ' {')

        # Emit the swaps. We use a temp, and wrap in a block to avoid naming
        # collisions with multiple swaps. {{Doubling}} to escape the format.

        for v in vars_to_swap:
            common_body.append('    {{ unsigned temp = {}{}; {}{} = {}{}; {}{} = temp; }}'.format(v, src_a, v, src_a, v, src_b, v, src_b))

        # Also, remap. Bidrectional swaps are explicit in the XML.
        for v in remap:
            maps = remap[v]
            imm = imm_map[v]

            for i, l in enumerate(maps):
                common_body.append('    {}if ({} == {}) {} = {};'.format('' if i == 0 else 'else ', v, imm[l], v, imm[maps[l]]))

        common_body.append('}')
        common_body.append('')

    for (name, pos, width) in st[1].get("immediates", []):
        common_body.append('unsigned {} = I->{};'.format(name, name))
        common_body.append('assert({} < {});'.format(name, hex(1 << width)))

        for st in pack_exprs:
            st.append('({} << {})'.format(name, pos))

    # After this, we have to branch off, since deriveds *do* vary based on state.
    state_body = [[] for s in states]

    for i, (_, st) in enumerate(states):
        for ((pos, width), exprs) in st.get("derived", []):
            pack_derived(pos, exprs, imm_map, state_body[i], pack_exprs[i])

    # How do we pick a state? Accumulate the conditions
    state_conds = [compile_s_expr(st[0], imm_map, None) for st in states] if len(states) > 1 else [None]

    if state_conds == None:
        assert (states[0][0] == None)

    # Finally, we'll collect everything together
    return variant_template.render(name = opname_to_c(opname), states = zip(pack_exprs, state_body, state_conds), common_body = common_body, single_state = (len(states) == 1), srcs = 4)

print(COPYRIGHT + '#include "compiler.h"')

packs = [pack_variant(e, instructions[e]) for e in instructions]
for p in packs:
    print(p)

top_pack = Template("""unsigned
bi_pack_${'fma' if unit == '*' else 'add'}(bi_instr *I,
    enum bifrost_packed_src src0,
    enum bifrost_packed_src src1,
    enum bifrost_packed_src src2,
    enum bifrost_packed_src src3)
{
    if (!I)
        return bi_pack_${opname_to_c(unit + 'NOP')}(I, src0, src1, src2, src3);

% if unit == '*':
    assert((1 << src0) & 0xfb);
    assert((1 << src1) & 0xfb);

% endif
    switch (I->op) {
% for opcode in ops:
% if unit + opcode in instructions:
    case BI_OPCODE_${opcode.replace('.', '_').upper()}:
        return bi_pack_${opname_to_c(unit + opcode)}(I, src0, src1, src2, src3);
% endif
% endfor
    default:
#ifndef NDEBUG
        bi_print_instr(I, stderr);
#endif
        unreachable("Cannot pack instruction as ${unit}");
    }
}
""")

for unit in ['*', '+']:
    print(top_pack.render(ops = ir_instructions, instructions = instructions, opname_to_c = opname_to_c, unit = unit))
