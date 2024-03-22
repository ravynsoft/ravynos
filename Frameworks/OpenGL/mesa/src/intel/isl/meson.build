# Copyright © 2017 Intel Corporation

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

isl_per_hw_ver_files = files(
  'isl_emit_cpb.c',
  'isl_emit_depth_stencil.c',
  'isl_surface_state.c',
  'isl_genX_helpers.h',
)

isl_gfx4_files = files(
  'isl_gfx4.c',
  'isl_gfx4.h',
)

isl_gfx6_files = files(
  'isl_gfx6.c',
  'isl_gfx6.h',
)

isl_gfx7_files = files(
  'isl_gfx7.c',
  'isl_gfx7.h',
)

isl_gfx8_files = files(
  'isl_gfx8.c',
  'isl_gfx8.h',
)

isl_gfx9_files = files(
  'isl_gfx9.c',
  'isl_gfx9.h',
)

isl_gfx12_files = files(
  'isl_gfx12.c',
  'isl_gfx12.h',
)

isl_gfx125_files = files(
  'isl_gfx12.c',
  'isl_gfx12.h',
)

isl_per_hw_ver_libs = []
foreach g : [['40', isl_gfx4_files], ['50', []], ['60', isl_gfx6_files],
             ['70', isl_gfx7_files], ['75', []], ['80', isl_gfx8_files],
             ['90', isl_gfx9_files], ['110', []], ['120', isl_gfx12_files],
             ['125', isl_gfx12_files], ['200', isl_gfx12_files]]
  _gfx_ver = g[0]
  isl_per_hw_ver_libs += static_library(
    'isl_per_hw_ver@0@'.format(_gfx_ver),
    [g[1], isl_per_hw_ver_files, gen_xml_pack],
    include_directories : [inc_include, inc_src, inc_intel],
    dependencies : [idep_mesautil, idep_intel_dev],
    c_args : [
      no_override_init_args,
      '-DGFX_VERx10=@0@'.format(_gfx_ver)
    ],
    gnu_symbol_visibility : 'hidden',
  )
endforeach

isl_format_layout_c = custom_target(
  'isl_format_layout.c',
  input : ['gen_format_layout.py', 'isl_format_layout.csv'],
  output : 'isl_format_layout.c',
  command : [prog_python, '@INPUT0@', '--csv', '@INPUT1@', '--out', '@OUTPUT@'],
)

files_isl_tiled_memcpy = files(
  'isl_tiled_memcpy_normal.c'
)

files_isl_tiled_memcpy_sse41 = files(
  'isl_tiled_memcpy_sse41.c',
)

isl_tiled_memcpy = static_library(
  'isl_tiled_memcpy',
  [files_isl_tiled_memcpy],
  include_directories : [
    inc_include, inc_src, inc_intel,
  ],

  dependencies : [idep_mesautil, idep_intel_dev],
  c_args : [no_override_init_args, sse2_arg],
  gnu_symbol_visibility : 'hidden',
  extra_files : ['isl_tiled_memcpy.c']
)

if with_sse41
  isl_tiled_memcpy_sse41 = static_library(
    'isl_tiled_memcpy_sse41',
    [files_isl_tiled_memcpy_sse41],
    include_directories : [
      inc_include, inc_src, inc_intel,
    ],
    dependencies : [idep_mesautil, idep_intel_dev],
    link_args : ['-Wl,--exclude-libs=ALL'],
    c_args : [no_override_init_args, sse2_arg, sse41_args],
    gnu_symbol_visibility : 'hidden',
    extra_files : ['isl_tiled_memcpy.c']
  )
else
  isl_tiled_memcpy_sse41 = []
endif

libisl_files = files(
  'isl.c',
  'isl.h',
  'isl_aux_info.c',
  'isl_drm.c',
  'isl_genX_priv.h',
  'isl_format.c',
  'isl_priv.h',
  'isl_storage_image.c',
)

libisl = static_library(
  'isl',
  [libisl_files, isl_format_layout_c, genX_bits_h],
  include_directories : [inc_include, inc_src, inc_intel],
  link_with : [isl_per_hw_ver_libs, isl_tiled_memcpy, isl_tiled_memcpy_sse41],
  dependencies : [idep_mesautil, idep_intel_dev],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
)

if with_intel_tools
isl_query = executable(
  'isl_query',
  files('isl_query.c'),
  dependencies : [idep_mesautil, dep_m, idep_intel_dev],
  include_directories : [inc_include, inc_src, inc_intel],
  link_with : [libisl],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  install : false
)
endif

if with_tests
  test(
    'isl_surf_get_image_offset',
    executable(
      'isl_surf_get_image_offset_test',
      'tests/isl_surf_get_image_offset_test.c',
      dependencies : [dep_m, idep_mesautil, idep_intel_dev],
      link_with:  libisl,
      include_directories : [inc_include, inc_src, inc_intel],
    ),
    suite : ['intel'],
  )
  test(
    'isl_aux_info',
    executable(
      'isl_aux_info_test',
      [
        'tests/isl_aux_info_test.cpp',
        'isl_aux_info.c',
      ],
      dependencies : [dep_m, idep_gtest, idep_mesautil, idep_intel_dev],
      include_directories : [inc_include, inc_src, inc_intel],
      c_args : '-DIN_UNIT_TEST',
      cpp_args : '-DIN_UNIT_TEST',
    ),
    suite : ['intel'],
    protocol : 'gtest',
  )
  test(
    'isl_tile_std_y',
    executable(
      'isl_tile_std_y_test',
      'tests/isl_tile_std_y_test.c',
      dependencies : dep_m,
      include_directories : [inc_include, inc_src, inc_gallium, inc_intel],
      link_with : [libisl, libintel_dev],
    ),
    suite : ['intel'],
  )
  test(
    'isl_tilememcpy',
    executable(
      'isl_tilememcpy_test',
      [
        'tests/isl_tilememcpy_tiled_unittest.cpp',
      ],
      dependencies : [dep_m, idep_gtest, idep_mesautil, idep_intel_dev],
      include_directories : [inc_include, inc_src, inc_gallium, inc_intel],
      link_with : libisl,
      c_args : '-DIN_UNIT_TEST',
      cpp_args : ['-std=c++17', '-DIN_UNIT_TEST'],
    ),
    suite : ['intel'],
  )
endif
