# Copyright © 2022 Collabora Ltd. and Red Hat Inc.
# SPDX-License-Identifier: MIT
nvk_files = files(
  '../../gallium/auxiliary/tgsi/tgsi_from_mesa.c',
  'nvk_buffer.c',
  'nvk_buffer.h',
  'nvk_buffer_view.c',
  'nvk_buffer_view.h',
  'nvk_cmd_buffer.c',
  'nvk_cmd_buffer.h',
  'nvk_cmd_clear.c',
  'nvk_cmd_copy.c',
  'nvk_cmd_dispatch.c',
  'nvk_cmd_draw.c',
  'nvk_cmd_meta.c',
  'nvk_cmd_pool.c',
  'nvk_cmd_pool.h',
  'nvk_codegen.c',
  'nvk_compute_pipeline.c',
  'nvk_descriptor_set.h',
  'nvk_descriptor_set.c',
  'nvk_descriptor_set_layout.c',
  'nvk_descriptor_set_layout.h',
  'nvk_descriptor_table.c',
  'nvk_descriptor_table.h',
  'nvk_device.c',
  'nvk_device.h',
  'nvk_device_memory.c',
  'nvk_device_memory.h',
  'nvk_event.c',
  'nvk_event.h',
  'nvk_format.c',
  'nvk_format.h',
  'nvk_graphics_pipeline.c',
  'nvk_heap.c',
  'nvk_heap.h',
  'nvk_image.c',
  'nvk_image.h',
  'nvk_image_view.c',
  'nvk_image_view.h',
  'nvk_instance.c',
  'nvk_instance.h',
  'nvk_mme.c',
  'nvk_mme.h',
  'nvk_nir_lower_descriptors.c',
  'nvk_physical_device.c',
  'nvk_physical_device.h',
  'nvk_pipeline.c',
  'nvk_pipeline.h',
  'nvk_private.h',
  'nvk_query_pool.c',
  'nvk_query_pool.h',
  'nvk_queue.c',
  'nvk_queue.h',
  'nvk_queue_drm_nouveau.c',
  'nvk_sampler.c',
  'nvk_sampler.h',
  'nvk_shader.c',
  'nvk_shader.h',
  'nvk_wsi.c',
  'nvk_wsi.h'
)

nvk_entrypoints = custom_target(
  'nvk_entrypoints',
  input : [vk_entrypoints_gen, vk_api_xml],
  output : ['nvk_entrypoints.h', 'nvk_entrypoints.c'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@', '--proto', '--weak',
    '--out-h', '@OUTPUT0@', '--out-c', '@OUTPUT1@', '--prefix', 'nvk',
    '--beta', with_vulkan_beta.to_string(),
  ],
  depend_files : vk_entrypoints_gen_depend_files,
)

nvk_deps = [
  dep_libdrm,
  idep_nak,
  idep_nil,
  idep_nir,
  idep_nouveau_mme,
  idep_nouveau_ws,
  idep_nvidia_headers,
  idep_vulkan_runtime,
  idep_vulkan_util,
  idep_vulkan_wsi,
  idep_vulkan_wsi_headers,
]

libnvk = static_library(
  'nvk',
  [
    nvk_entrypoints,
    cl_generated,
    nvk_files,
  ],
  include_directories : [
    inc_gallium,
    inc_gallium_aux,
    inc_include,
    inc_src,
  ],
  link_with : [libnouveau_codegen],
  dependencies : [nvk_deps, idep_nouveau_codegen],
  gnu_symbol_visibility : 'hidden',
)

libvulkan_nouveau = shared_library(
  'vulkan_nouveau',
  link_whole : [libnvk],
  link_args: [ld_args_build_id],
  gnu_symbol_visibility : 'hidden',
  install : true,
)

icd_lib_path = join_paths(get_option('prefix'), get_option('libdir'))
icd_file_name = 'libvulkan_nouveau.so'
if with_platform_windows
  icd_lib_path = join_paths(get_option('prefix'), get_option('bindir'))
  icd_file_name = 'vulkan_nouveau.dll'
endif

nouveau_icd = custom_target(
  'nouveau_icd',
  input : [vk_icd_gen, vk_api_xml],
  output : 'nouveau_icd.@0@.json'.format(host_machine.cpu()),
  command : [
    prog_python, '@INPUT0@',
    '--api-version', '1.1', '--xml', '@INPUT1@',
    '--lib-path', join_paths(icd_lib_path, icd_file_name),
    '--out', '@OUTPUT@',
  ],
  build_by_default : true,
  install_dir : with_vulkan_icd_dir,
  install_tag : 'runtime',
  install : true,
)

if meson.version().version_compare('>= 0.58')
  _dev_icdname = 'nouveau_devenv_icd.@0@.json'.format(host_machine.cpu())
  custom_target(
    'nouveau_devenv_icd',
    input : [vk_icd_gen, vk_api_xml],
    output : _dev_icdname,
    command : [
      prog_python, '@INPUT0@',
      '--api-version', '1.1', '--xml', '@INPUT1@',
      '--lib-path', meson.current_build_dir() / icd_file_name,
      '--out', '@OUTPUT@',
    ],
    build_by_default : true,
  )

  devenv.append('VK_ICD_FILENAMES', meson.current_build_dir() / _dev_icdname)
endif

if with_tests and not with_platform_android
  test(
    'nvk_mme',
    executable(
      'test_nvk_mme',
      files('test_nvk_mme.c'),
      dependencies : [nvk_deps],
      link_with : [libnvk],
    )
  )
endif
