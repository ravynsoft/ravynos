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

subdir('gtest')
subdir('c11/impl')
subdir('android_stub')
subdir('util')

inc_mesa = include_directories('mesa')
inc_mapi = include_directories('mapi')
inc_src = include_directories('.')
inc_gallium = include_directories('gallium/include')
inc_gallium_aux = include_directories('gallium/auxiliary')
inc_amd_common = include_directories('amd/common')
inc_tool = include_directories('tool')
inc_virtio_gpu = include_directories('virtio/virtio-gpu')
inc_frontends = include_directories('gallium/frontends')
pps_datasources = []
pps_includes = []

inc_llvmpipe = include_directories('gallium/drivers/llvmpipe')

if with_llvm
  inc_amd_common_llvm = include_directories('amd/llvm')
else
  inc_amd_common_llvm = []
endif

glsl_util_files = files(
    'mesa/main/extensions_table.c',
    'mesa/program/prog_parameter.c',
    'mesa/program/symbol_table.c',
    'mesa/program/dummy_errors.c',
)

sha1_h = custom_target(
  'git_sha1.h',
  output : 'git_sha1.h',
  command : [prog_python, git_sha1_gen_py, '--output', '@OUTPUT@'],
  build_by_default : true,
  build_always_stale : true, # commit sha1 can change without having touched these files
)

if cc.get_argument_syntax() == 'msvc'
  subdir('getopt')
else
  idep_getopt = null_dep
endif
if with_gallium
  subdir('mapi')
endif
subdir('compiler')
if with_tools.contains('drm-shim')
  subdir('drm-shim')
endif
if with_imgui
  subdir('imgui')
endif
if with_platform_wayland
  subdir('egl/wayland/wayland-drm')
endif
if with_any_vk or with_gallium_zink
  subdir('vulkan')
endif
if with_any_intel
  subdir('intel')
endif
if with_gallium_radeonsi or with_amd_vk or with_gallium_r300 or with_gallium_r600
  subdir('amd')
endif
if with_any_broadcom
  subdir('broadcom')
endif
if with_gallium_etnaviv
  subdir('etnaviv')
endif
if system_has_kms_drm
  subdir('virtio')
endif
if with_gallium_freedreno or with_freedreno_vk or with_tools.contains('freedreno')
  subdir('freedreno')
endif
if with_imagination_vk
  subdir('imagination')
endif
if with_gallium_panfrost or with_gallium_lima or with_panfrost_vk or with_tools.contains('panfrost')
  subdir('panfrost')
endif
#if with_gallium_virgl or with_virtio_vk
#  subdir('virtio')
#endif
if with_microsoft_clc or with_gallium_d3d12 or with_spirv_to_dxil or with_microsoft_vk
  subdir('microsoft')
endif
if with_any_nouveau
  subdir('nouveau')
endif
if with_gallium_asahi or with_tools.contains('asahi')
  subdir('asahi')
endif
if with_gallium or with_gbm
  subdir('loader')
endif
if with_gbm
  subdir('gbm')
else
  inc_gbm = []
endif
if with_gallium
  subdir('mesa')
  subdir('gallium')
  if with_glx == 'dri'
     subdir('glx')
  endif
  # This has to be here since it requires libgallium, and subdir cannot
  # contain ..
  if with_tests
    subdir('mesa/main/tests')
  endif
  if with_tests and with_shared_glapi
    subdir('mesa/state_tracker/tests')
  endif
endif
if with_egl
  subdir('egl')
endif

# This must be after at least mesa, glx, and gallium, since libgl will be
# defined in one of those subdirs depending on the glx provider.
if with_glx != 'disabled' and not with_glvnd
  pkg.generate(
    name : 'gl',
    description : 'Mesa OpenGL Library',
    version : meson.project_version(),
    libraries : libgl,
    libraries_private : gl_priv_libs,
    requires_private : gl_priv_reqs,
    variables : ['glx_tls=yes'],
  )
endif

subdir('tool')
