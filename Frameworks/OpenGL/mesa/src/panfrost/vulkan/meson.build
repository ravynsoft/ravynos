# Copyright © 2021 Collabora Ltd.
#
# Derived from the freedreno driver which is:
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

panvk_entrypoints = custom_target(
  'panvk_entrypoints.[ch]',
  input : [vk_entrypoints_gen, vk_api_xml],
  output : ['panvk_entrypoints.h', 'panvk_entrypoints.c'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@', '--proto', '--weak',
    '--out-h', '@OUTPUT0@', '--out-c', '@OUTPUT1@', '--prefix', 'panvk',
    '--device-prefix', 'panvk_v6', '--device-prefix', 'panvk_v7',
    '--beta', with_vulkan_beta.to_string()
  ],
  depend_files : vk_entrypoints_gen_depend_files,
)

libpanvk_files = files(
  'panvk_cmd_buffer.c',
  'panvk_cs.c',
  'panvk_device.c',
  'panvk_descriptor_set.c',
  'panvk_formats.c',
  'panvk_image.c',
  'panvk_mempool.c',
  'panvk_pass.c',
  'panvk_pipeline.c',
  'panvk_pipeline_cache.c',
  'panvk_private.h',
  'panvk_query.c',
  'panvk_shader.c',
  'panvk_wsi.c',
) + [vk_cmd_enqueue_entrypoints[0], vk_common_entrypoints[0]]

panvk_deps = []
panvk_flags = []
panvk_per_arch_libs = []

foreach arch : ['6', '7']
  panvk_per_arch_libs += static_library(
    'panvk_v@0@'.format(arch),
    [
      panvk_entrypoints[0],
      'panvk_vX_cmd_buffer.c',
      'panvk_vX_cs.c',
      'panvk_vX_descriptor_set.c',
      'panvk_vX_device.c',
      'panvk_vX_image.c',
      'panvk_vX_meta.c',
      'panvk_vX_meta_blit.c',
      'panvk_vX_meta_copy.c',
      'panvk_vX_meta_clear.c',
      'panvk_vX_nir_lower_descriptors.c',
      'panvk_vX_pipeline.c',
      'panvk_vX_shader.c',
    ],
    include_directories : [
      inc_include,
      inc_src,
      inc_gallium, # XXX: util/format/u_formats.h
      inc_gallium_aux, # XXX: renderonly
      inc_panfrost,
    ],
    dependencies : [
      idep_nir_headers,
      idep_pan_packers,
      idep_vulkan_util_headers,
      idep_vulkan_runtime_headers,
      idep_vulkan_wsi_headers,
      dep_libdrm,
      dep_valgrind,
    ],
    c_args : [no_override_init_args, panvk_flags, '-DPAN_ARCH=@0@'.format(arch)],
  )
endforeach

if with_platform_wayland
  panvk_deps += dep_wayland_client
  libpanvk_files += [wayland_drm_client_protocol_h, wayland_drm_protocol_c]
endif

if with_platform_android
  panvk_deps += [dep_android]
  panvk_flags += '-DVK_USE_PLATFORM_ANDROID_KHR'
  libpanvk_files += files('panvk_android.c')
endif

libvulkan_panfrost = shared_library(
  'vulkan_panfrost',
  [libpanvk_files, panvk_entrypoints],
  include_directories : [
    inc_include,
    inc_src,
    inc_gallium, # XXX: util/format/u_formats.h
    inc_gallium_aux, # XXX: renderonly
    inc_panfrost,
  ],
  link_whole : [panvk_per_arch_libs],
  link_with : [
    libpanfrost_shared,
    libpanfrost_midgard,
    libpanfrost_bifrost,
    libpanfrost_decode,
    libpanfrost_lib,
    libpanfrost_util,
  ],
  dependencies : [
    dep_dl,
    dep_elf,
    dep_libdrm,
    dep_m,
    dep_thread,
    dep_valgrind,
    idep_nir,
    idep_pan_packers,
    panvk_deps,
    idep_vulkan_util,
    idep_vulkan_runtime,
    idep_vulkan_wsi,
    idep_mesautil,
  ],
  c_args : [no_override_init_args, panvk_flags],
  link_args : [ld_args_bsymbolic, ld_args_gc_sections],
  install : true,
)

panfrost_icd = custom_target(
  'panfrost_icd',
  input : [vk_icd_gen, vk_api_xml],
  output : 'panfrost_icd.@0@.json'.format(host_machine.cpu()),
  command : [
    prog_python, '@INPUT0@',
    '--api-version', '1.0', '--xml', '@INPUT1@',
    '--lib-path', join_paths(get_option('prefix'), get_option('libdir'),
                             'libvulkan_panfrost.so'),
    '--out', '@OUTPUT@',
  ],
  build_by_default : true,
  install_dir : with_vulkan_icd_dir,
  install_tag : 'runtime',
  install : true,
)

_dev_icdname = 'panfrost_devenv_icd.@0@.json'.format(host_machine.cpu())
_dev_icd = custom_target(
  'panfrost_devenv_icd',
  input : [vk_icd_gen, vk_api_xml],
  output : _dev_icdname,
  command : [
    prog_python, '@INPUT0@',
    '--api-version', '1.0', '--xml', '@INPUT1@',
    '--lib-path', meson.current_build_dir() / 'libvulkan_panfrost.so',
    '--out', '@OUTPUT@',
  ],
  build_by_default : true,
)

devenv.append('VK_ICD_FILENAMES', _dev_icd.full_path())
