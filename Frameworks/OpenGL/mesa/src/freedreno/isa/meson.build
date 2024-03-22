# Copyright © 2020 Google, Inc

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

isa_depend_files = [
  'ir3-common.xml',
  'ir3-cat0.xml',
  'ir3-cat1.xml',
  'ir3-cat2.xml',
  'ir3-cat3.xml',
  'ir3-cat4.xml',
  'ir3-cat5.xml',
  'ir3-cat6.xml',
  'ir3-cat7.xml',
  isaspec_py_deps
]

ir3_isa = custom_target(
  'ir3-isa',
  input: ['ir3.xml'],
  output: ['ir3-isa.c', 'ir3-isa.h'],
  command: [
    prog_isaspec_decode, '--xml', '@INPUT@',
    '--out-c', '@OUTPUT0@', '--out-h', '@OUTPUT1@',
  ],
  depend_files: isa_depend_files,
)

decode_files = [
  ir3_isa,
  'isa.h',
]

libir3decode = static_library(
  'ir3decode',
  decode_files,
  dependencies: [idep_mesautil, idep_isaspec_decode],
  include_directories: [
    inc_include,
    inc_src,
  ],
  gnu_symbol_visibility: 'hidden',
)

ir3disasm = executable(
  'ir3-disasm',
  ['ir3-disasm.c'],
  link_with: libir3decode,
  build_by_default: with_tools.contains('freedreno'),
  include_directories: [
    inc_src,
  ],
  install: false,
)

encode_h = custom_target(
  'encode.h',
  input: ['ir3.xml'],
  output: 'encode.h',
  command: [
    prog_isaspec_encode, '--xml', '@INPUT@', '--out-h', '@OUTPUT@'
  ],
  depend_files: isa_depend_files,
)

encode_files = [
  encode_h,
  'encode.c',
  'isa.h',
]

libir3encode = static_library(
  'ir3encode',
  encode_files,
  dependencies: [idep_mesautil, idep_nir],
  include_directories: [
    inc_src,
    inc_include,
    inc_freedreno,
  ],
  gnu_symbol_visibility: 'hidden',
)
