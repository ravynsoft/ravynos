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

libspirv_to_dxil = both_libraries(
  'spirv_to_dxil',
  [
    files(
      'dxil_spirv_nir.c',
      'dxil_spirv_nir.h',
      'dxil_spirv_nir_lower_bindless.c',
      'spirv_to_dxil.c',
      'spirv_to_dxil.h',
    ),
    sha1_h,
  ],
  vs_module_defs : 'spirv_to_dxil.def',
  dependencies : [idep_nir, idep_vtn, idep_libdxil_compiler],
  include_directories : [inc_include, inc_src],
  install : true,
)

idep_libspirv_to_dxil = declare_dependency(
   link_with : libspirv_to_dxil.get_static_lib(),
   include_directories : include_directories('.')
)

spirv_to_dxil = executable(
  'spirv2dxil',
  files(
    'spirv2dxil.c',
  ),
  dependencies : [idep_nir, idep_libdxil_compiler, idep_getopt, idep_libspirv_to_dxil],
  include_directories : [inc_include, inc_src],
  build_by_default : true,
  install : true,
)
