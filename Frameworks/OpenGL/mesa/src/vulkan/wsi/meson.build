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

files_vulkan_wsi = files('wsi_common.c')
platform_deps = []

if dep_libdrm.found()
  files_vulkan_wsi += files('wsi_common_drm.c')
endif

if with_platform_x11
  files_vulkan_wsi += files('wsi_common_x11.c')
endif

if with_platform_wayland
  files_vulkan_wsi += files('wsi_common_wayland.c')
  files_vulkan_wsi += wp_files['linux-dmabuf-unstable-v1']
  files_vulkan_wsi += wp_files['presentation-time']
  files_vulkan_wsi += wp_files['tearing-control-v1']
endif

if with_platform_windows
  files_vulkan_wsi += files('wsi_common_win32.cpp')
  platform_deps += dep_dxheaders
else
  files_vulkan_wsi += files('wsi_common_headless.c')
endif

if system_has_kms_drm and not with_platform_android
  files_vulkan_wsi += files('wsi_common_display.c')
endif

wsi_entrypoints = custom_target(
  'wsi_entrypoints',
  input : [vk_entrypoints_gen, vk_api_xml],
  output : ['wsi_common_entrypoints.h', 'wsi_common_entrypoints.c'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@', '--proto', '--weak',
    '--out-h', '@OUTPUT0@', '--out-c', '@OUTPUT1@', '--prefix', 'wsi',
    '--beta', with_vulkan_beta.to_string()
  ],
  depend_files : vk_entrypoints_gen_depend_files,
)

libvulkan_wsi = static_library(
  'vulkan_wsi',
  [files_vulkan_wsi, wsi_entrypoints],
  include_directories : [inc_include, inc_src],
  dependencies : [
    vulkan_wsi_deps, dep_libdrm, dep_libudev, idep_vulkan_util_headers,
    idep_vulkan_runtime_headers, idep_xmlconfig, idep_mesautil, platform_deps
  ],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
)

idep_vulkan_wsi_headers = declare_dependency(
  dependencies : idep_vulkan_wsi_defines,
  include_directories : include_directories('.')
)

idep_vulkan_wsi_entrypoints_h = declare_dependency(
  sources : [wsi_entrypoints[0]]
)

# This is likely a bug in the Meson VS backend, as MSVC with ninja works fine.
# See this discussion here:
# https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/10506
if get_option('backend').startswith('vs')
  idep_vulkan_wsi = declare_dependency(
    link_with : libvulkan_wsi,
    dependencies : idep_vulkan_wsi_headers
  )
else
  idep_vulkan_wsi = declare_dependency(
    # Instruct users of this library to link with --whole-archive.  Otherwise,
    # our weak function overloads may not resolve properly.
    link_whole : libvulkan_wsi,
    dependencies : [
      idep_vulkan_wsi_headers, dep_libudev
    ]
  )
endif
