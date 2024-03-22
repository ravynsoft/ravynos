# Copyright © 2021 VMware, Inc
# Copyright © 2018 Intel Corporation

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

d3d10_sw_def = custom_target(
  'd3d10_sw.def',
  input: 'd3d10_sw.def.in',
  output : 'd3d10_sw.def',
  command : gen_vs_module_defs_normal_command,
)

libd3d10sw = shared_library(
  'd3d10sw',
  ['d3d10_gdi.c'],
  vs_module_defs : d3d10_sw_def,
  include_directories : [
    inc_include, inc_src, inc_gallium, inc_gallium_aux, inc_d3d10umd, inc_gallium_winsys, inc_gallium_winsys_sw, inc_gallium_drivers,
  ],
  link_whole : [libd3d10umd],
  link_with : [
    libgallium, libwsgdi
  ],
  dependencies : [
    dep_ws2_32, idep_nir, driver_swrast
  ],
  name_prefix : '',  # otherwise mingw will create libd3d10sw.dll
  install : true,
)

if with_tests
  test(
    'd3d10sw',
    executable(
      'test_d3d10sw',
      files('tests/tri.cpp'),
      cpp_args : [cpp_msvc_compat_args],
      dependencies : [cpp.find_library('d3d11')],
      link_with : [libd3d10sw],
    ),
    suite : ['d3d10sw'],
  )
endif
