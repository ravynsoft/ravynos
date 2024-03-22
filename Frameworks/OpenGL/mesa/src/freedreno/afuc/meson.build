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

if with_tests
  diff = find_program('diff')
endif

afuc_parser = custom_target(
  'parser.[ch]',
  input: 'parser.y',
  output: ['parser.c', 'parser.h'],
  command: [
    prog_bison, '@INPUT@', '--defines=@OUTPUT1@', '--output=@OUTPUT0@'
  ]
)

afuc_lexer = custom_target(
  'lexer.c',
  input: 'lexer.l',
  output: 'lexer.c',
  command: [
    prog_flex, '-o', '@OUTPUT@', '@INPUT@'
  ]
)

encode_h = custom_target(
  'encode.h',
  input: ['afuc.xml'],
  output: 'encode.h',
  command: [
    prog_isaspec_encode, '--xml', '@INPUT@', '--out-h', '@OUTPUT@'
  ],
)

asm = executable(
  'afuc-asm',
  [
    'asm.c',
    'util.c',
    'util.h',
    afuc_lexer,
    afuc_parser,
    encode_h,
  ],
  include_directories: [
    inc_freedreno_rnn, inc_include, inc_src, inc_util,
  ],
  link_with: [
    libfreedreno_rnn,
  ],
  dependencies: [],
  build_by_default : with_tools.contains('freedreno'),
  install: install_fd_decode_tools,
)
if with_tests
  asm_fw = custom_target('afuc_test.fw',
    output: 'afuc_test.fw',
    command: [asm, '-g', '6', files('../.gitlab-ci/traces/afuc_test.asm'), '@OUTPUT@'],
  )
  test('afuc-asm',
    diff,
    args: ['-u', files('../.gitlab-ci/reference/afuc_test.fw'), asm_fw],
    suite: 'freedreno',
    workdir: dir_source_root
  )
endif

afuc_isa = custom_target(
  'afuc-isa',
  input: ['afuc.xml'],
  output: ['afuc-isa.c', 'afuc-isa.h'],
  command: [
    prog_isaspec_decode, '--xml', '@INPUT@',
    '--out-c', '@OUTPUT0@', '--out-h', '@OUTPUT1@',
  ],
)

# Disasm requires mmaping >4GB
if cc.sizeof('size_t') > 4
  disasm = executable(
    'afuc-disasm',
    [
      'disasm.c',
      'emu.c',
      'emu.h',
      'emu-ds.c',
      'emu-regs.c',
      'emu-ui.c',
      'util.c',
      'util.h',
      afuc_isa,
    ],
    include_directories: [
      inc_freedreno,
      inc_freedreno_rnn,
      inc_include,
      inc_src,
      inc_util,
    ],
    link_with: [
      libfreedreno_rnn,
    ],
    dependencies: [idep_mesautil, idep_isaspec_decode],
    build_by_default : with_tools.contains('freedreno'),
    install: install_fd_decode_tools,
  )

  if with_tests
    disasm_fw = custom_target('afuc_test.asm',
      output: 'afuc_test.asm',
      command: [disasm, '-u', files('../.gitlab-ci/reference/afuc_test.fw'), '-g', '630'],
      capture: true
    )
    test('afuc-disasm',
      diff,
      args: ['-u', files('../.gitlab-ci/reference/afuc_test.asm'), disasm_fw],
      suite: 'freedreno',
      workdir: dir_source_root
    )
  endif
endif
