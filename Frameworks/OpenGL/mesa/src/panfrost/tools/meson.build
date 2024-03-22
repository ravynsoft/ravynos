# Copyright © 2021 Collabora

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

coredumpdec = executable(
  'panfrostdump',
  files('panfrostdump.c'),
  c_args : [c_msvc_compat_args, no_override_init_args, compile_args_panfrost],
  gnu_symbol_visibility : 'hidden',
  include_directories : [inc_include, inc_src, inc_mesa],
  dependencies: [libpanfrost_dep],
  build_by_default : true,
  install: true
)

panfrost_texfeatures = executable(
  'panfrost_texfeatures',
  files('panfrost_texfeatures.c'),
  c_args : [c_msvc_compat_args, no_override_init_args, compile_args_panfrost],
  gnu_symbol_visibility : 'hidden',
  include_directories : [inc_include, inc_src, inc_mesa],
  dependencies: [libpanfrost_dep],
  build_by_default : true,
  install: true
)
