# Copyright © 2017 Broadcom
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

files_libv3d = files(
  'v3d_blit.c',
  'v3d_bufmgr.c',
  'v3d_bufmgr.h',
  'v3d_cl.c',
  'v3d_cl.h',
  'v3d_context.c',
  'v3d_context.h',
  'v3d_disk_cache.c',
  'v3d_fence.c',
  'v3d_formats.c',
  'v3d_job.c',
  'v3d_program.c',
  'v3d_query.c',
  'v3d_query.h',
  'v3d_query_pipe.c',
  'v3d_resource.c',
  'v3d_resource.h',
  'v3d_screen.c',
  'v3d_screen.h',
  'v3d_uniforms.c',
)

files_per_version = files(
  'v3dx_draw.c',
  'v3dx_emit.c',
  'v3dx_format_table.c',
  'v3dx_job.c',
  'v3dx_query_perfcnt.c',
  'v3dx_rcl.c',
  'v3dx_state.c',
  'v3dx_tfu.c',
)

v3d_args = ['-DV3D_BUILD_NEON']

dep_v3dv3 = dependency('v3dv3', required: false)
if dep_v3dv3.found()
  v3d_args += '-DUSE_V3D_SIMULATOR'
endif

v3d_versions = ['42', '71']

per_version_libs = []
foreach ver : v3d_versions
  per_version_libs += static_library(
    'v3d-v' + ver,
    [files_per_version, v3d_xml_pack],
    include_directories : [
      inc_src, inc_include, inc_gallium, inc_gallium_aux, inc_broadcom,
      inc_gallium_drivers,
    ],
    c_args : [v3d_args, '-DV3D_VERSION=' + ver],
    gnu_symbol_visibility : 'hidden',
    dependencies : [dep_v3dv3, dep_libdrm, dep_valgrind, idep_nir_headers],
)

endforeach

libv3d = static_library(
  'v3d',
  [
    files_libv3d,
    v3d_xml_pack,
  ],
  include_directories : [
    inc_src, inc_include, inc_gallium, inc_gallium_aux, inc_broadcom,
    inc_simulator, inc_gallium_drivers,
  ],
  c_args : [v3d_args],
  cpp_args : [v3d_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : [
    dep_v3dv3, dep_libdrm, dep_valgrind,
    idep_nir_headers, idep_mesautil,
  ],
  link_with: [per_version_libs],
)

driver_v3d = declare_dependency(
  compile_args : '-DGALLIUM_V3D',
  link_with : [libv3d, libv3dwinsys, libbroadcom_cle, libbroadcom_v3d],
  dependencies : idep_nir,
)
