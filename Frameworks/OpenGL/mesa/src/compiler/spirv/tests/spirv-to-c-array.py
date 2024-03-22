#!/usr/bin/env python3

import argparse
import filecmp
import os
import struct
import subprocess
import tempfile
import sys

p = argparse.ArgumentParser(usage="%(prog)s SOURCE [-- [EXTRA_FLAGS ...]]")
p.add_argument('SOURCE',
               help='SPIR-V assembly source filename')
p.add_argument('EXTRA_FLAGS',
               help='extra flags passed to spirv-as, e.g. --target-env spv1.0',
               nargs=argparse.REMAINDER)
args = p.parse_args()
source = args.SOURCE

fd, generated = tempfile.mkstemp(prefix="spirv-to-c-array.", suffix=".spv")
os.close(fd)

assembler_cmd = ['spirv-as'] + args.EXTRA_FLAGS

ret = subprocess.run(assembler_cmd + ['-o', generated, source])

if ret.returncode != 0:
    print(f'Failed to assemble {source}, see error above.')
    sys.exit(1)

if os.path.getsize(generated) == 0:
    print(f'Failed to assemble {source}. Output {generated} is empty.')
    sys.exit(1)

if (os.path.getsize(generated) % 4) != 0:
    print(f'Failed to assemble {source}. Output {generated} size is not multiple of 4 bytes.')
    sys.exit(1)

ret = subprocess.run(['spirv-dis', '--raw-id', generated], capture_output=True)
if ret.returncode != 0:
    print(ret.stderr.decode('ascii'))
    print(f'Something is wrong: assembled binary {generated} ')
    print('failed to disassemble for checking.')
    sys.exit(1)

disassembled_source = ret.stdout

generated_check = os.path.splitext(generated)[0] + '.check.spv'

ret = subprocess.run(assembler_cmd + ['-o', generated_check],
                     input=disassembled_source, capture_output=True)
if ret.returncode != 0:
    print(ret.stderr.decode('ascii'))
    print(f'Something is wrong: assembled binary {generated} ')
    print('failed to reassemble for checking.')
    sys.exit(1)

if not filecmp.cmp(generated, generated_check, shallow=False):
    print('Something is wrong: assembled binary generated ')
    print('does not match after round trip of using disassembler ')
    print('and assembler again.  See files:')
    print()
    print(f'    {generated}')
    print(f'    {generated_check}')
    print()
    print(f'Extra arguments for spirv-as {" ".join(args.EXTRA_FLAGS)}')
    sys.exit(1)

with open(source, 'r') as f:
    source_lines = f.readlines()

words = []
with open(generated, 'rb') as f:
    while True:
        w = f.read(4)
        if not w:
            break
        v = struct.unpack('<I', w)[0]
        words.append(f'{v:#010x}')

os.remove(generated_check)
os.remove(generated)

print("   /*")

if args.EXTRA_FLAGS:
    print(f'    ; Extra arguments for spirv-as {" ".join(args.EXTRA_FLAGS)}')
    print()

for line in source_lines:
    print(f'   {line}', end='')

print('    */')
print('   static const uint32_t words[] = {')

while words:
    line, words = words[:6], words[6:]
    line = ', '.join(line)
    print(f'      {line},')

print('   };')
