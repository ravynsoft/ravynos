# Copyright © 2017 Rob Clark

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

files_libfreedreno = files(
  'driinfo_freedreno.h',
  'freedreno_autotune.c',
  'freedreno_autotune.h',
  'freedreno_batch.c',
  'freedreno_batch.h',
  'freedreno_batch_cache.c',
  'freedreno_batch_cache.h',
  'freedreno_blitter.c',
  'freedreno_blitter.h',
  'freedreno_context.c',
  'freedreno_context.h',
  'freedreno_draw.c',
  'freedreno_draw.h',
  'freedreno_fence.c',
  'freedreno_fence.h',
  'freedreno_gmem.c',
  'freedreno_gmem.h',
  'freedreno_program.c',
  'freedreno_program.h',
  'freedreno_query.c',
  'freedreno_query.h',
  'freedreno_query_acc.c',
  'freedreno_query_acc.h',
  'freedreno_query_hw.c',
  'freedreno_query_hw.h',
  'freedreno_query_sw.c',
  'freedreno_query_sw.h',
  'freedreno_resource.c',
  'freedreno_resource.h',
  'freedreno_screen.c',
  'freedreno_screen.h',
  'freedreno_state.c',
  'freedreno_state.h',
  'freedreno_surface.c',
  'freedreno_surface.h',
  'freedreno_texture.c',
  'freedreno_texture.h',
  'freedreno_util.c',
  'freedreno_util.h',
  'a2xx/fd2_blend.c',
  'a2xx/fd2_blend.h',
  'a2xx/fd2_context.c',
  'a2xx/fd2_context.h',
  'a2xx/fd2_draw.c',
  'a2xx/fd2_draw.h',
  'a2xx/fd2_emit.c',
  'a2xx/fd2_emit.h',
  'a2xx/fd2_gmem.c',
  'a2xx/fd2_gmem.h',
  'a2xx/fd2_program.c',
  'a2xx/fd2_program.h',
  'a2xx/fd2_query.c',
  'a2xx/fd2_query.h',
  'a2xx/fd2_rasterizer.c',
  'a2xx/fd2_rasterizer.h',
  'a2xx/fd2_resource.c',
  'a2xx/fd2_resource.h',
  'a2xx/fd2_screen.c',
  'a2xx/fd2_screen.h',
  'a2xx/fd2_texture.c',
  'a2xx/fd2_texture.h',
  'a2xx/fd2_util.c',
  'a2xx/fd2_util.h',
  'a2xx/fd2_zsa.c',
  'a2xx/fd2_zsa.h',
  'a2xx/ir2.c',
  'a2xx/ir2.h',
  'a2xx/ir2_assemble.c',
  'a2xx/ir2_cp.c',
  'a2xx/ir2_nir.c',
  'a2xx/ir2_private.h',
  'a2xx/ir2_ra.c',
  'a3xx/fd3_blend.c',
  'a3xx/fd3_blend.h',
  'a3xx/fd3_context.c',
  'a3xx/fd3_context.h',
  'a3xx/fd3_draw.c',
  'a3xx/fd3_draw.h',
  'a3xx/fd3_emit.c',
  'a3xx/fd3_emit.h',
  'a3xx/fd3_format.c',
  'a3xx/fd3_format.h',
  'a3xx/fd3_gmem.c',
  'a3xx/fd3_gmem.h',
  'a3xx/fd3_program.c',
  'a3xx/fd3_program.h',
  'a3xx/fd3_query.c',
  'a3xx/fd3_query.h',
  'a3xx/fd3_rasterizer.c',
  'a3xx/fd3_rasterizer.h',
  'a3xx/fd3_resource.c',
  'a3xx/fd3_resource.h',
  'a3xx/fd3_screen.c',
  'a3xx/fd3_screen.h',
  'a3xx/fd3_texture.c',
  'a3xx/fd3_texture.h',
  'a3xx/fd3_zsa.c',
  'a3xx/fd3_zsa.h',
  'a4xx/fd4_blend.c',
  'a4xx/fd4_blend.h',
  'a4xx/fd4_compute.c',
  'a4xx/fd4_compute.h',
  'a4xx/fd4_context.c',
  'a4xx/fd4_context.h',
  'a4xx/fd4_draw.c',
  'a4xx/fd4_draw.h',
  'a4xx/fd4_emit.c',
  'a4xx/fd4_emit.h',
  'a4xx/fd4_format.c',
  'a4xx/fd4_format.h',
  'a4xx/fd4_gmem.c',
  'a4xx/fd4_gmem.h',
  'a4xx/fd4_image.c',
  'a4xx/fd4_image.h',
  'a4xx/fd4_program.c',
  'a4xx/fd4_program.h',
  'a4xx/fd4_query.c',
  'a4xx/fd4_query.h',
  'a4xx/fd4_rasterizer.c',
  'a4xx/fd4_rasterizer.h',
  'a4xx/fd4_resource.c',
  'a4xx/fd4_resource.h',
  'a4xx/fd4_screen.c',
  'a4xx/fd4_screen.h',
  'a4xx/fd4_texture.c',
  'a4xx/fd4_texture.h',
  'a4xx/fd4_zsa.c',
  'a4xx/fd4_zsa.h',
  'a5xx/fd5_blend.c',
  'a5xx/fd5_blend.h',
  'a5xx/fd5_blitter.c',
  'a5xx/fd5_blitter.h',
  'a5xx/fd5_compute.c',
  'a5xx/fd5_compute.h',
  'a5xx/fd5_context.c',
  'a5xx/fd5_context.h',
  'a5xx/fd5_draw.c',
  'a5xx/fd5_draw.h',
  'a5xx/fd5_emit.c',
  'a5xx/fd5_emit.h',
  'a5xx/fd5_format.c',
  'a5xx/fd5_format.h',
  'a5xx/fd5_gmem.c',
  'a5xx/fd5_gmem.h',
  'a5xx/fd5_image.c',
  'a5xx/fd5_image.h',
  'a5xx/fd5_program.c',
  'a5xx/fd5_program.h',
  'a5xx/fd5_query.c',
  'a5xx/fd5_query.h',
  'a5xx/fd5_rasterizer.c',
  'a5xx/fd5_rasterizer.h',
  'a5xx/fd5_resource.c',
  'a5xx/fd5_resource.h',
  'a5xx/fd5_screen.c',
  'a5xx/fd5_screen.h',
  'a5xx/fd5_texture.c',
  'a5xx/fd5_texture.h',
  'a5xx/fd5_zsa.c',
  'a5xx/fd5_zsa.h',
  'a6xx/fd6_barrier.cc',
  'a6xx/fd6_barrier.h',
  'a6xx/fd6_blend.cc',
  'a6xx/fd6_blend.h',
  'a6xx/fd6_blitter.cc',
  'a6xx/fd6_blitter.h',
  'a6xx/fd6_compute.cc',
  'a6xx/fd6_compute.h',
  'a6xx/fd6_const.cc',
  'a6xx/fd6_const.h',
  'a6xx/fd6_context.cc',
  'a6xx/fd6_context.h',
  'a6xx/fd6_draw.cc',
  'a6xx/fd6_draw.h',
  'a6xx/fd6_emit.cc',
  'a6xx/fd6_emit.h',
  'a6xx/fd6_gmem.cc',
  'a6xx/fd6_gmem.h',
  'a6xx/fd6_image.cc',
  'a6xx/fd6_image.h',
  'a6xx/fd6_program.cc',
  'a6xx/fd6_program.h',
  'a6xx/fd6_query.cc',
  'a6xx/fd6_query.h',
  'a6xx/fd6_rasterizer.cc',
  'a6xx/fd6_rasterizer.h',
  'a6xx/fd6_resource.cc',
  'a6xx/fd6_resource.h',
  'a6xx/fd6_screen.cc',
  'a6xx/fd6_screen.h',
  'a6xx/fd6_texture.cc',
  'a6xx/fd6_texture.h',
  'a6xx/fd6_vsc.cc',
  'a6xx/fd6_vsc.h',
  'a6xx/fd6_zsa.cc',
  'a6xx/fd6_zsa.h',
  'ir3/ir3_cache.c',
  'ir3/ir3_cache.h',
  'ir3/ir3_const.h',
  'ir3/ir3_descriptor.c',
  'ir3/ir3_descriptor.h',
  'ir3/ir3_gallium.c',
  'ir3/ir3_gallium.h',
)

freedreno_tracepoints = custom_target(
  'freedreno_tracepoints.[ch]',
  input: 'freedreno_tracepoints.py',
  output: ['freedreno_tracepoints.c', 'freedreno_tracepoints.h'],
  command: [
    prog_python, '@INPUT@',
    '-p', join_paths(dir_source_root, 'src/util/perf/'),
    '-C', '@OUTPUT0@',
    '-H', '@OUTPUT1@',
  ],
  depend_files: u_trace_py,
)

files_libfreedreno += freedreno_tracepoints

freedreno_includes = [
  inc_mesa, inc_mapi,
  inc_src, inc_include, inc_gallium, inc_gallium_aux,
  inc_freedreno, include_directories('ir3'),
]

freedreno_c_args = [no_override_init_args]
freedreno_c_args += cc.get_supported_arguments([
  '-Wno-packed-bitfield-compat',
  '-Wno-array-bounds',
])

freedreno_cpp_args = []
freedreno_cpp_args += cpp.get_supported_arguments([
  '-fno-exceptions',
  '-fno-rtti',
  '-Wno-packed-bitfield-compat',
  '-Wno-c99-designator',
  '-Wno-reorder-init-list',
  '-Wno-address-of-temporary',
  '-Wno-c++11-narrowing',
  '-Wno-narrowing',
  '-Wno-sign-compare',
  '-Wno-missing-field-initializers',
  '-Wno-reorder',
  '-Wno-array-bounds',
  '-Wno-overflow',
  '-Wno-c++11-narrowing',
])

# HACK compiler.get_supported_arguments()/has_argument() do not seem
# to be reliable for cross builds (!?!) so just jam in some args we
# need and hope for the best
if meson.is_cross_build()
  freedreno_cpp_args += '-Wno-array-bounds'
  freedreno_cpp_args += '-Wno-c++11-narrowing'
  freedreno_cpp_args += '-Wno-c99-designator'
endif

libfreedreno_dependencies = [
  dep_libdrm,
  idep_mesautil,
  idep_nir_headers,
  idep_libfreedreno_common,
  idep_u_tracepoints,
]

if with_perfetto
  libfreedreno_dependencies += dep_perfetto
  files_libfreedreno += 'freedreno_perfetto.cc'
endif

# The header file ends up part of the build (but just a stub)
# in either case:
files_libfreedreno += 'freedreno_perfetto.h'

libfreedreno = static_library(
  'freedreno',
  [files_libfreedreno, freedreno_xml_header_files],
  include_directories : freedreno_includes,
  c_args : [freedreno_c_args],
  cpp_args : [freedreno_cpp_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : libfreedreno_dependencies,
)

driver_freedreno = declare_dependency(
  compile_args : '-DGALLIUM_FREEDRENO',
  link_with : [
    libfreedrenowinsys,
    libfreedreno,
    libfreedreno_drm,
    libfreedreno_ir2,
    libfreedreno_ir3,
    libfreedreno_layout,
    libfreedreno_perfcntrs
  ],
  dependencies : [
    idep_nir,
    idep_xmlconfig,
  ],
)

ir3_compiler = executable(
  'ir3_compiler',
  ['ir3/ir3_cmdline.c', freedreno_xml_header_files],
  include_directories : freedreno_includes,
  dependencies : [
    idep_nir,
    idep_vtn,
    idep_mesautil,
  ],
  link_with : [
    libfreedreno,
    libfreedreno_drm,
    libfreedreno_ir3,
    libfreedreno_layout,
    libgallium,
    libglsl_standalone,
  ],
  build_by_default : with_tools.contains('freedreno'),
  install : false,
)

gmemtool = executable(
  'gmemtool',
  [
    'gmemtool.c',
    freedreno_tracepoints[1]
  ],
  include_directories : freedreno_includes,
  dependencies : [
    idep_nir,
    idep_mesautil,
    idep_u_tracepoints,
    idep_xmlconfig,
  ],
  link_with : [
    libfreedreno,
    libfreedreno_drm,
    libfreedreno_ir2,
    libfreedreno_ir3,
    libfreedreno_layout,
    libgallium,
    libglsl_standalone,
    libfreedreno_perfcntrs,
  ],
  build_by_default : with_tools.contains('freedreno'),
  install : false,
)
