# Copyright © 2017 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

vtn_gather_types_c = custom_target(
  'vtn_gather_types.c',
  input : files('vtn_gather_types_c.py', 'spirv.core.grammar.json'),
  output : 'vtn_gather_types.c',
  command : [prog_python, '@INPUT0@', '@INPUT1@', '@OUTPUT@'],
)

spirv_info_c = custom_target(
  'spirv_info.c',
  input : files('spirv_info_c.py', 'spirv.core.grammar.json'),
  output : 'spirv_info.c',
  command : [prog_python, '@INPUT0@', '@INPUT1@', '@OUTPUT@'],
)

vtn_generator_ids_h = custom_target(
  'vtn_generator_ids.h',
  input : files('vtn_generator_ids_h.py', 'spir-v.xml'),
  output : 'vtn_generator_ids.h',
  command : [prog_python, '@INPUT0@', '@INPUT1@', '@OUTPUT@'],
)

files_libvtn = files(
  'GLSL.ext.AMD.h',
  'GLSL.std.450.h',
  'OpenCL.std.h',
  'gl_spirv.c',
  'nir_spirv.h',
  'spirv.h',
  'spirv_info.h',
  'spirv_to_nir.c',
  'vtn_alu.c',
  'vtn_amd.c',
  'vtn_cfg.c',
  'vtn_cmat.c',
  'vtn_glsl450.c',
  'vtn_opencl.c',
  'vtn_private.h',
  'vtn_structured_cfg.c',
  'vtn_subgroup.c',
  'vtn_variables.c',
)

libvtn = static_library(
  'vtn',
  [files_libvtn,
   spirv_info_c,
   vtn_gather_types_c,
   vtn_generator_ids_h,
  ],
  include_directories : [inc_include, inc_src, inc_mapi, inc_mesa],
  c_args : [c_msvc_compat_args, no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : [idep_nir, dep_valgrind],
  build_by_default : false,
)

idep_vtn = declare_dependency(
  dependencies : [idep_nir, idep_mesautil],
  link_with : libvtn,
)

spirv2nir = executable(
  'spirv2nir',
  files('spirv2nir.c') + [
   vtn_generator_ids_h,
  ],
  dependencies : [dep_m, idep_vtn, idep_mesautil],
  include_directories : [inc_include, inc_src],
  c_args : [c_msvc_compat_args, no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : with_tools.contains('nir'),
  install : with_tools.contains('nir'),
)

if with_tests
  test(
    'spirv_tests',
    executable(
      'spirv_tests',
      files(
        'tests/helpers.h',
        'tests/avail_vis.cpp',
        'tests/volatile.cpp',
        'tests/control_flow_tests.cpp',
      ),
      c_args : [c_msvc_compat_args, no_override_init_args],
      gnu_symbol_visibility : 'hidden',
      include_directories : [inc_include, inc_src],
      dependencies : [idep_vtn, dep_thread, idep_gtest, idep_nir, idep_mesautil],
    ),
    suite : ['compiler', 'spirv'],
    protocol : 'gtest',
  )
endif
