# Copyright © Microsoft Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

files_libdxil_compiler = files(
  'dxil_buffer.c',
  'dxil_container.c',
  'dxil_dump.c',
  'dxil_enums.c',
  'dxil_function.c',
  'dxil_module.c',
  'dxil_nir.c',
  'dxil_nir_lower_int_cubemaps.c',
  'dxil_nir_lower_int_samplers.c',
  'dxil_nir_lower_vs_vertex_conversion.c',
  'dxil_signature.c',
  'dxil_nir_tess.c',
  'nir_to_dxil.c',
)

if host_machine.system() == 'windows'
  files_libdxil_compiler += files('dxil_validator.cpp')
endif

dxil_nir_algebraic_c = custom_target(
  'dxil_nir_algebraic.c',
  input : 'dxil_nir_algebraic.py',
  output : 'dxil_nir_algebraic.c',
  command : [
    prog_python, '@INPUT@', '-p', dir_compiler_nir,
  ],
  capture : true,
  depend_files : nir_algebraic_depends,
)

libdxil_compiler = static_library(
  'dxil_compiler',
  [files_libdxil_compiler, dxil_nir_algebraic_c, sha1_h],
  include_directories : [inc_include, inc_src],
  dependencies: [idep_nir_headers, dep_valgrind],
  gnu_symbol_visibility : 'hidden',
)

idep_libdxil_compiler = declare_dependency(
  link_with : libdxil_compiler,
  dependencies : [idep_mesautil, dep_version],
  include_directories : include_directories('.')
)

if with_tests
  test(
    'dxil_buffer',
    executable(
      'dxil_buffer_test',
      files('dxil_buffer_test.c'),
      include_directories : [inc_src],
      dependencies : [idep_libdxil_compiler]
    ),
    suite: 'd3d12'
  )
endif
