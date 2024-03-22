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

libetnaviv_drm_files = files(
  'etnaviv_device.c',
  'etnaviv_gpu.c',
  'etnaviv_bo.c',
  'etnaviv_bo_cache.c',
  'etnaviv_perfmon.c',
  'etnaviv_pipe.c',
  'etnaviv_cmd_stream.c',
  'etnaviv_drmif.h',
  'etnaviv_priv.h',
)

libetnaviv_drm = static_library(
  'etnaviv_drm',
  libetnaviv_drm_files,
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
  ],
  build_by_default : false,
)

if with_tools.contains('etnaviv')
  subdir('tests')
endif
