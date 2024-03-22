# Copyright (C) 2020-2021 Collabora
# Copyright (C) 2018 Rob Clark

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

valhall_c = custom_target(
  'valhall_c',
  input : ['valhall.c.py', 'ISA.xml'],
  output : 'valhall.c',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : files('valhall.py'),
)

valhall_enums_h = custom_target(
  'valhall_enums.h',
  input : ['valhall_enums.h.py', 'ISA.xml'],
  output : 'valhall_enums.h',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : files('valhall.py'),
)

idep_valhall_enums_h = declare_dependency(
  sources : [valhall_enums_h],
  include_directories : include_directories('.'),
)

valhall_disasm_c = custom_target(
  'valhall_disasm_c',
  input : ['disasm.py', 'ISA.xml'],
  output : 'valhall_disasm.c',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : files('valhall.py'),
)

libpanfrost_valhall_disasm = static_library(
  'panfrost_valhall_disasm',
  valhall_disasm_c,
  include_directories : [inc_include, inc_src, inc_mesa],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
)

if with_tests
  test(
    'valhall_disasm',
    executable(
      'valhall_disasm_test',
      files('test/test-disassembler.c'),
      c_args : [c_msvc_compat_args, no_override_init_args],
      gnu_symbol_visibility : 'hidden',
      include_directories : [inc_include, inc_src, inc_mesa],
      dependencies: [idep_valhall_enums_h],
      link_with : [libpanfrost_valhall_disasm],
    ),
    suite : ['panfrost'],
    args : files('test/assembler-cases.txt'),
  )

  test(
    'valhall_asm',
    prog_python,
    args : files('test-assembly.py', 'test/assembler-cases.txt', 'test/negative-cases.txt'),
    suite : ['panfrost'],
  )
endif
