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

inc_broadcom = include_directories('.', 'cle')

subdir('cle')

v3d_versions = ['42', '71']
v3d_libs = []

if with_gallium_v3d or with_broadcom_vk
  subdir('compiler')
  subdir('qpu')
  subdir('simulator')
endif

if with_tools.contains('drm-shim')
  subdir('drm-shim')
endif

per_version_libs = []
foreach ver : v3d_versions
  per_version_libs += static_library(
    'broadcom-v' + ver,
    [
      files('clif/v3dx_dump.c'),
      v3d_xml_pack
    ],
    include_directories : [inc_include, inc_src, inc_broadcom],
    c_args : [no_override_init_args, '-DV3D_VERSION=' + ver],
    gnu_symbol_visibility : 'hidden',
    dependencies: [dep_valgrind, dep_thread],
  )
endforeach

v3d_args = ['-DV3D_BUILD_NEON']

v3d_neon_c_args = []
if host_machine.cpu_family() == 'arm'
    v3d_neon_c_args = '-mfpu=neon'
endif

libv3d_neon = static_library(
  'v3d_neon',
  'common/v3d_tiling.c',
  include_directories : [
    inc_src, inc_include, inc_gallium, inc_gallium_aux, inc_broadcom,
  ],
  c_args : [v3d_args, v3d_neon_c_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : [dep_libdrm, dep_valgrind, idep_nir_headers],
)

libbroadcom_v3d = static_library(
  'broadcom_v3d',
  [
    files('common/v3d_debug.c', 'common/v3d_device_info.c', 'clif/clif_dump.c', 'common/v3d_util.c'),
    v3d_xml_pack,
  ],
  include_directories : [inc_include, inc_src, inc_broadcom],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  link_whole : v3d_libs + per_version_libs,
  link_with: [libv3d_neon],
  build_by_default : false,
  dependencies: [dep_valgrind, dep_thread],
)

if with_broadcom_vk
  subdir('vulkan')
endif
