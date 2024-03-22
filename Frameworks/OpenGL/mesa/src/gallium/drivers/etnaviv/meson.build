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

files_etnaviv = files(
  'hw/cmdstream.xml.h',
  'hw/common.xml.h',
  'hw/common_3d.xml.h',
  'hw/isa.xml.h',
  'hw/state_3d.xml.h',
  'hw/state_blt.xml.h',
  'hw/state.xml.h',
  'hw/texdesc_3d.xml.h',
  'etnaviv_asm.c',
  'etnaviv_asm.h',
  'etnaviv_blend.c',
  'etnaviv_blend.h',
  'etnaviv_blt.c',
  'etnaviv_blt.h',
  'etnaviv_clear_blit.c',
  'etnaviv_clear_blit.h',
  'etnaviv_compiler.c',
  'etnaviv_compiler.h',
  'etnaviv_compiler_nir.c',
  'etnaviv_compiler_nir_emit.c',
  'etnaviv_compiler_nir_liveness.c',
  'etnaviv_compiler_nir_ra.c',
  'etnaviv_context.c',
  'etnaviv_context.h',
  'etnaviv_debug.h',
  'etnaviv_disasm.c',
  'etnaviv_disasm.h',
  'etnaviv_disk_cache.c',
  'etnaviv_disk_cache.h',
  'etnaviv_emit.c',
  'etnaviv_emit.h',
  'etnaviv_etc2.c',
  'etnaviv_etc2.h',
  'etnaviv_fence.c',
  'etnaviv_fence.h',
  'etnaviv_format.c',
  'etnaviv_format.h',
  'etnaviv_internal.h',
  'etnaviv_nir_lower_source_mods.c',
  'etnaviv_nir_lower_texture.c',
  'etnaviv_nir_lower_ubo_to_uniform.c',
  'etnaviv_nir.c',
  'etnaviv_nir.h',
  'etnaviv_perfmon.c',
  'etnaviv_perfmon.h',
  'etnaviv_query.c',
  'etnaviv_query.h',
  'etnaviv_query_acc_occlusion.c',
  'etnaviv_query_acc_perfmon.c',
  'etnaviv_query_acc.c',
  'etnaviv_query_acc.h',
  'etnaviv_query_sw.c',
  'etnaviv_query_sw.h',
  'etnaviv_rasterizer.c',
  'etnaviv_rasterizer.h',
  'etnaviv_resource.c',
  'etnaviv_resource.h',
  'etnaviv_rs.c',
  'etnaviv_rs.h',
  'etnaviv_screen.c',
  'etnaviv_screen.h',
  'etnaviv_shader.c',
  'etnaviv_shader.h',
  'etnaviv_state.c',
  'etnaviv_state.h',
  'etnaviv_surface.c',
  'etnaviv_surface.h',
  'etnaviv_texture.c',
  'etnaviv_texture.h',
  'etnaviv_texture_desc.c',
  'etnaviv_texture_desc.h',
  'etnaviv_texture_state.c',
  'etnaviv_texture_state.h',
  'etnaviv_tiling.c',
  'etnaviv_tiling.h',
  'etnaviv_transfer.c',
  'etnaviv_transfer.h',
  'etnaviv_translate.h',
  'etnaviv_uniforms.c',
  'etnaviv_uniforms.h',
  'etnaviv_util.h',
  'etnaviv_zsa.c',
  'etnaviv_zsa.h',
)

libetnaviv = static_library(
  'etnaviv',
  files_etnaviv,
  gnu_symbol_visibility : 'hidden',
  include_directories : [
    inc_include, inc_src, inc_gallium, inc_gallium_aux, inc_etnaviv,
  ],
  link_with: [libetnaviv_drm, libetnaviv_decode],
  dependencies : [dep_libdrm, idep_nir_headers, idep_mesautil],
)

etnaviv_compiler = executable(
  'etnaviv_compiler',
  'etnaviv_compiler_cmdline.c',
  include_directories : [
    inc_include, inc_src, inc_gallium, inc_gallium_aux, inc_etnaviv,
  ],
  link_with : [libgallium, libetnaviv, libetnaviv_drm],
  build_by_default : with_tools.contains('etnaviv'),
  install : with_tools.contains('etnaviv'),
  dependencies : [idep_mesautil, dep_libdrm, idep_nir],
)

driver_etnaviv = declare_dependency(
  compile_args : '-DGALLIUM_ETNAVIV',
  link_with : [libetnaviv, libetnavivdrm],
  dependencies : idep_nir,
)

if with_tests
  test(
    'lower_ubo',
    executable(
      'nir_lower_ubo_test',
      files('tests/lower_ubo_tests.cpp'),
      cpp_args : [cpp_msvc_compat_args],
      gnu_symbol_visibility : 'hidden',
      include_directories : [inc_include, inc_src, inc_gallium],
      link_with : [libetnaviv],
      dependencies : [idep_gtest, idep_nir],
    ),
    suite : ['compiler', 'etnaviv'],
    protocol : 'gtest',
  )
endif
