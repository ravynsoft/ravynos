# Copyright © 2018 Rob Clark
# Copyright © 2019 Collabora

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

pan_packers = []
foreach packer : ['common', 'v4', 'v5', 'v6', 'v7', 'v9', 'v10']
  pan_packers += custom_target(
    packer + '_pack.h',
    input : ['gen_pack.py', packer + '.xml'],
    output : packer + '_pack.h',
    command : [prog_python, '@INPUT@'],
    capture : true,
  )
endforeach

idep_pan_packers = declare_dependency(
  sources : [pan_packers],
  include_directories : include_directories('.'),
)

libpanfrost_decode_per_arch = []

foreach ver : ['4', '5', '6', '7', '9', '10']
  libpanfrost_decode_per_arch += static_library(
    'pandecode-arch-v' + ver,
    ['decode.c', 'decode_jm.c', 'decode_csf.c', pan_packers],
    include_directories : [inc_include, inc_src, inc_panfrost],
    dependencies : [dep_libdrm, idep_pan_packers, idep_nir],
    c_args : [no_override_init_args, '-DPAN_ARCH=' + ver],
    gnu_symbol_visibility : 'hidden',
    build_by_default : false,
  )
endforeach

libpanfrost_decode = static_library(
  'panfrost_decode',
  [
    'decode_common.c',
    pan_packers
  ],
  include_directories : [inc_include, inc_src, inc_panfrost],
  dependencies : idep_mesautil,
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
  link_with: [libpanfrost_decode_per_arch],
)
