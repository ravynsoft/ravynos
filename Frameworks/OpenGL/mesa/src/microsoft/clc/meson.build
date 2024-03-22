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

files_libclc_compiler = files(
  'clc_compiler.c',
  'clc_nir.c',
)

libclc_compiler = shared_library(
  'clon12compiler',
  [files_libclc_compiler, sha1_h],
  vs_module_defs : 'clon12compiler.def',
  include_directories : [inc_include, inc_src, inc_spirv],
  dependencies: [idep_mesaclc, idep_nir_headers, dep_version, idep_mesautil,
                 idep_libdxil_compiler, idep_nir, idep_vtn],
  name_prefix : ''  # otherwise mingw will create libclon12compiler.dll
)

if dep_dxheaders.found() and host_machine.system() == 'windows' and with_tests
  clc_compiler_test = executable('clc_compiler_test',
    ['clc_compiler_test.cpp', 'compute_test.cpp'],
    link_with : [libclc_compiler],
    dependencies : [idep_gtest, idep_mesautil, idep_libdxil_compiler, dep_dxheaders,
                    dep_spirv_tools, idep_compiler],
    include_directories : [inc_include, inc_src, inc_spirv],
  )

  if build_machine.system() == 'windows'
    test('clc_compiler_test', clc_compiler_test, timeout: 180, protocol : 'gtest')
  endif

endif
