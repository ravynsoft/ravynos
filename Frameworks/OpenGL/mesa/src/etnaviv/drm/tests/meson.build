# Copyright © 2019 Purism SPC

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

etnaviv_cmd_stream_test = executable(
  'etnaviv_cmd_stream_test',
  'etnaviv_cmd_stream_test.c',
  include_directories : [
    inc_etnaviv,
    inc_include,
    inc_src,
  ],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : [
    dep_libdrm,
    dep_valgrind,
    idep_mesautil,
  ],
  link_with : libetnaviv_drm,
  install: false,
)

etnaviv_bo_cache_test = executable(
  'etnaviv_bo_cache_test',
  'etnaviv_bo_cache_test.c',
  include_directories : [
    inc_etnaviv,
    inc_include,
    inc_src,
  ],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : [
    dep_libdrm,
    dep_valgrind,
    idep_mesautil,
  ],
  link_with : libetnaviv_drm,
  install: false,
)
