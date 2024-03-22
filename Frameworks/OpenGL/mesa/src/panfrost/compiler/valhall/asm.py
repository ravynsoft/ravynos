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

import argparse
import sys
import struct
from valhall import instructions, enums, immediates, typesize

LINE = ''

class ParseError(Exception):
    def __init__(self, error):
        self.error = error

class FAUState:
    def __init__(self, message = False):
        self.message = message
        self.page = None
        self.words = set()
        self.buffer = set()

    def set_page(self, page):
        assert(page <= 3)
        die_if(self.page is not None and self.page != page, 'Mismatched pages')
        self.page = page

    def push(self, source):
        if not (source & (1 << 7)):
            # Skip registers
            return

        self.buffer.add(source)
        die_if(len(self.buffer) > 2, "Overflowed FAU buffer")

        if (source >> 5) == 0b110:
            # Small constants need to check if the buffer overflows but no else
            return

        slot = (source >> 1)

        self.words.add(source)

        # Check the encoded slots
        slots = set([(x >> 1) for x in self.words])
        die_if(len(slots) > (2 if self.message else 1), 'Too many FAU slots')
        die_if(len(self.words) > (3 if self.message else 2), 'Too many FAU words')

# When running standalone, exit with the error since we're dealing with a
# human. Otherwise raise a Python exception so the test harness can handle it.
def die(s):
    if __name__ == "__main__":
        print(LINE)
        print(s)
        sys.exit(1)
    else:
        raise ParseError(s)

def die_if(cond, s):
    if cond:
        die(s)

def parse_int(s, minimum, maximum):
    try:
        number = int(s, base = 0)
    except ValueError:
        die(f"Expected number {s}")

    if number > maximum or number < minimum:
        die(f"Range error on {s}")

    return number

def encode_source(op, fau):
    if op[0] == '^':
        die_if(op[1] != 'r', f"Expected register after discard {op}")
        return parse_int(op[2:], 0, 63) | 0x40
    elif op[0] == 'r':
        return parse_int(op[1:], 0, 63)
    elif op[0] == 'u':
        val = parse_int(op[1:], 0, 127)
        fau.set_page(val >> 6)
        return (val & 0x3F) | 0x80
    elif op[0] == 'i':
        return int(op[3:]) | 0xC0
    elif op.startswith('0x'):
        try:
            val = int(op, base=0)
        except ValueError:
            die('Expected value')

        die_if(val not in immediates, 'Unexpected immediate value')
        return immediates.index(val) | 0xC0
    else:
        for i in [0, 1, 3]:
            if op in enums[f'fau_special_page_{i}'].bare_values:
                idx = 32 + (enums[f'fau_special_page_{i}'].bare_values.index(op) << 1)
                fau.set_page(i)
                return idx | 0xC0

        die('Invalid operand')


def encode_dest(op):
    die_if(op[0] != 'r', f"Expected register destination {op}")

    parts = op.split(".")
    reg = parts[0]

    # Default to writing in full
    wrmask = 0x3

    if len(parts) > 1:
        WMASKS = ["h0", "h1"]
        die_if(len(parts) > 2, "Too many modifiers")
        mask = parts[1];
        die_if(mask not in WMASKS, "Expected a write mask")
        wrmask = 1 << WMASKS.index(mask)

    return parse_int(reg[1:], 0, 63) | (wrmask << 6)

def parse_asm(line):
    global LINE
    LINE = line # For better errors
    encoded = 0

    # Figure out mnemonic
    head = line.split(" ")[0]
    opts = [ins for ins in instructions if head.startswith(ins.name)]
    opts = sorted(opts, key=lambda x: len(x.name), reverse=True)

    if len(opts) == 0:
        die(f"No known mnemonic for {head}")

    if len(opts) > 1 and len(opts[0].name) == len(opts[1].name):
        print(f"Ambiguous mnemonic for {head}")
        print(f"Options:")
        for ins in opts:
            print(f"  {ins}")
        sys.exit(1)

    ins = opts[0]

    # Split off modifiers
    if len(head) > len(ins.name) and head[len(ins.name)] != '.':
        die(f"Expected . after instruction in {head}")

    mods = head[len(ins.name) + 1:].split(".")
    modifier_map = {}

    tail = line[(len(head) + 1):]
    operands = [x.strip() for x in tail.split(",") if len(x.strip()) > 0]
    expected_op_count = len(ins.srcs) + len(ins.dests) + len(ins.immediates) + len(ins.staging)
    if len(operands) != expected_op_count:
        die(f"Wrong number of operands in {line}, expected {expected_op_count}, got {len(operands)} {operands}")

    # Encode each operand
    for i, (op, sr) in enumerate(zip(operands, ins.staging)):
        die_if(op[0] != '@', f'Expected staging register, got {op}')
        parts = op[1:].split(':')

        if op == '@':
            parts = []

        die_if(any([x[0] != 'r' for x in parts]), f'Expected registers, got {op}')
        regs = [parse_int(x[1:], 0, 63) for x in parts]

        extended_write = "staging_register_write_count" in [x.name for x in ins.modifiers] and sr.write
        max_sr_count = 8 if extended_write else 7

        sr_count = len(regs)
        die_if(sr_count > max_sr_count, f'Too many staging registers {sr_count}')

        base = regs[0] if len(regs) > 0 else 0
        die_if(any([reg != (base + i) for i, reg in enumerate(regs)]),
                'Expected consecutive staging registers, got {op}')
        die_if(sr_count > 1 and (base % 2) != 0,
                'Consecutive staging registers must be aligned to a register pair')

        if sr.count == 0:
            if "staging_register_write_count" in [x.name for x in ins.modifiers] and sr.write:
                modifier_map["staging_register_write_count"] = sr_count - 1
            else:
                assert "staging_register_count" in [x.name for x in ins.modifiers]
                modifier_map["staging_register_count"] = sr_count
        else:
            die_if(sr_count != sr.count, f"Expected {sr.count} staging registers, got {sr_count}")

        encoded |= ((sr.encoded_flags | base) << sr.start)
    operands = operands[len(ins.staging):]

    for op, dest in zip(operands, ins.dests):
        encoded |= encode_dest(op) << 40
    operands = operands[len(ins.dests):]

    if len(ins.dests) == 0 and len(ins.staging) == 0:
        # Set a placeholder writemask to prevent encoding faults
        encoded |= (0xC0 << 40)

    fau = FAUState(message = ins.message)

    for i, (op, src) in enumerate(zip(operands, ins.srcs)):
        parts = op.split('.')
        encoded_src = encode_source(parts[0], fau)

        # Require a word selection for special FAU values
        needs_word_select = ((encoded_src >> 5) == 0b111)

        # Has a swizzle been applied yet?
        swizzled = False

        for mod in parts[1:]:
            # Encode the modifier
            if mod in src.offset and src.bits[mod] == 1:
                encoded |= (1 << src.offset[mod])
            elif src.halfswizzle and mod in enums[f'half_swizzles_{src.size}_bit'].bare_values:
                die_if(swizzled, "Multiple swizzles specified")
                swizzled = True
                val = enums[f'half_swizzles_{src.size}_bit'].bare_values.index(mod)
                encoded |= (val << src.offset['widen'])
            elif mod in enums[f'swizzles_{src.size}_bit'].bare_values and (src.widen or src.lanes):
                die_if(swizzled, "Multiple swizzles specified")
                swizzled = True
                val = enums[f'swizzles_{src.size}_bit'].bare_values.index(mod)
                encoded |= (val << src.offset['widen'])
            elif src.lane and mod in enums[f'lane_{src.size}_bit'].bare_values:
                die_if(swizzled, "Multiple swizzles specified")
                swizzled = True
                val = enums[f'lane_{src.size}_bit'].bare_values.index(mod)
                encoded |= (val << src.offset['lane'])
            elif src.combine and mod in enums['combine'].bare_values:
                die_if(swizzled, "Multiple swizzles specified")
                swizzled = True
                val = enums['combine'].bare_values.index(mod)
                encoded |= (val << src.offset['combine'])
            elif src.size == 32 and mod in enums['widen'].bare_values:
                die_if(not src.swizzle, "Instruction doesn't take widens")
                die_if(swizzled, "Multiple swizzles specified")
                swizzled = True
                val = enums['widen'].bare_values.index(mod)
                encoded |= (val << src.offset['swizzle'])
            elif src.size == 16 and mod in enums['swizzles_16_bit'].bare_values:
                die_if(not src.swizzle, "Instruction doesn't take swizzles")
                die_if(swizzled, "Multiple swizzles specified")
                swizzled = True
                val = enums['swizzles_16_bit'].bare_values.index(mod)
                encoded |= (val << src.offset['swizzle'])
            elif mod in enums['lane_8_bit'].bare_values:
                die_if(not src.lane, "Instruction doesn't take a lane")
                die_if(swizzled, "Multiple swizzles specified")
                swizzled = True
                val = enums['lane_8_bit'].bare_values.index(mod)
                encoded |= (val << src.lane)
            elif mod in enums['lanes_8_bit'].bare_values:
                die_if(not src.lanes, "Instruction doesn't take a lane")
                die_if(swizzled, "Multiple swizzles specified")
                swizzled = True
                val = enums['lanes_8_bit'].bare_values.index(mod)
                encoded |= (val << src.offset['widen'])
            elif mod in ['w0', 'w1']:
                # Chck for special
                die_if(not needs_word_select, 'Unexpected word select')

                if mod == 'w1':
                    encoded_src |= 0x1

                needs_word_select = False
            else:
                die(f"Unknown modifier {mod}")

        # Encode the identity if a swizzle is required but not specified
        if src.swizzle and not swizzled and src.size == 16:
            mod = enums['swizzles_16_bit'].default
            val = enums['swizzles_16_bit'].bare_values.index(mod)
            encoded |= (val << src.offset['swizzle'])
        elif src.widen and not swizzled and src.size == 16:
            die_if(swizzled, "Multiple swizzles specified")
            mod = enums['swizzles_16_bit'].default
            val = enums['swizzles_16_bit'].bare_values.index(mod)
            encoded |= (val << src.offset['widen'])

        encoded |= encoded_src << src.start
        fau.push(encoded_src)

    operands = operands[len(ins.srcs):]

    for i, (op, imm) in enumerate(zip(operands, ins.immediates)):
        if op[0] == '#':
            die_if(imm.name != 'constant', "Wrong syntax for immediate")
            parts = [imm.name, op[1:]]
        else:
            parts = op.split(':')
            die_if(len(parts) != 2, f"Wrong syntax for immediate, wrong number of colons in {op}")
            die_if(parts[0] != imm.name, f"Wrong immediate, expected {imm.name}, got {parts[0]}")

        if imm.signed:
            minimum = -(1 << (imm.size - 1))
            maximum = +(1 << (imm.size - 1)) - 1
        else:
            minimum = 0
            maximum = (1 << imm.size) - 1

        val = parse_int(parts[1], minimum, maximum)

        if val < 0:
            # Sign extends
            val = (1 << imm.size) + val

        encoded |= (val << imm.start)

    operands = operands[len(ins.immediates):]

    # Encode the operation itself
    encoded |= (ins.opcode << 48)
    encoded |= (ins.opcode2 << ins.secondary_shift)

    # Encode FAU page
    if fau.page:
        encoded |= (fau.page << 57)

    # Encode modifiers
    has_flow = False
    for mod in mods:
        if len(mod) == 0:
            continue

        if mod in enums['flow'].bare_values:
            die_if(has_flow, "Multiple flow control modifiers specified")
            has_flow = True
            encoded |= (enums['flow'].bare_values.index(mod) << 59)
        else:
            candidates = [c for c in ins.modifiers if mod in c.bare_values]

            die_if(len(candidates) == 0, f"Invalid modifier {mod} used")
            assert(len(candidates) == 1) # No ambiguous modifiers
            opts = candidates[0]

            value = opts.bare_values.index(mod)
            assert(value is not None)

            die_if(opts.name in modifier_map, f"{opts.name} specified twice")
            modifier_map[opts.name] = value

    for mod in ins.modifiers:
        value = modifier_map.get(mod.name, mod.default)
        die_if(value is None, f"Missing required modifier {mod.name}")

        assert(value < (1 << mod.size))
        encoded |= (value << mod.start)

    return encoded

if __name__ == "__main__":
    # Provide commandline interface
    parser = argparse.ArgumentParser(description='Assemble Valhall shaders')
    parser.add_argument('infile', nargs='?', type=argparse.FileType('r'),
                        default=sys.stdin)
    parser.add_argument('outfile', type=argparse.FileType('wb'))
    args = parser.parse_args()

    lines = args.infile.read().strip().split('\n')
    lines = [l for l in lines if len(l) > 0 and l[0] != '#']

    packed = b''.join([struct.pack('<Q', parse_asm(ln)) for ln in lines])
    args.outfile.write(packed)
