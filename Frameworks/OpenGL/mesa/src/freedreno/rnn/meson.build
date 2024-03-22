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

libfreedreno_rnn_files = files(
  'aprintf.c',
  'colors.c',
  'colors.h',
  'path.c',
  'rnn.c',
  'rnn.h',
  'rnndec.c',
  'rnndec.h',
  'util.h',
)

libfreedreno_rnn = static_library(
  'freedreno_rnn',
  libfreedreno_rnn_files,
  include_directories: [
    inc_src,
    inc_include,
  ],
  c_args : [
    no_override_init_args,
    '-DRNN_DEF_PATH="' + rnn_path + '"',
  ],
  gnu_symbol_visibility: 'hidden',
  dependencies: [ dep_libxml2, idep_mesautil ],
  build_by_default: false,
)

headergen2 = executable(
  'headergen2',
  'headergen2.c',
  include_directories: [],
  c_args : [no_override_init_args],
  gnu_symbol_visibility: 'hidden',
  dependencies: [],
  link_with: libfreedreno_rnn,
  build_by_default: with_tools.contains('freedreno'),
  install: false
)

if with_tests
  foreach xml : ['adreno.xml', 'msm.xml']
    test('headergen2-' + xml,
      headergen2,
      args: [xml],
      suite: 'freedreno',
    )
  endforeach
endif
