# Copyright © 2017 Intel Corporation
# SPDX-License-Identifier: MIT

tu_entrypoints = custom_target(
  'tu_entrypoints',
  input : [vk_entrypoints_gen, vk_api_xml],
  output : ['tu_entrypoints.h', 'tu_entrypoints.cc'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@', '--proto', '--weak',
    '--out-h', '@OUTPUT0@', '--out-c', '@OUTPUT1@', '--prefix', 'tu',
    '--include', 'adreno_common.xml.h',
    '--tmpl-prefix', 'tu', '--tmpl-param', 'chip CHIP',
    '--tmpl-variants', '<A6XX>', '<A7XX>',
    '--beta', with_vulkan_beta.to_string(),
  ],
  depend_files : vk_entrypoints_gen_depend_files,
)


libtu_files = files(
  'tu_autotune.cc',
  'tu_clear_blit.cc',
  'tu_cmd_buffer.cc',
  'tu_cs_breadcrumbs.cc',
  'tu_cs.cc',
  'tu_device.cc',
  'tu_descriptor_set.cc',
  'tu_dynamic_rendering.cc',
  'tu_formats.cc',
  'tu_image.cc',
  'tu_knl.cc',
  'tu_lrz.cc',
  'tu_nir_lower_multiview.cc',
  'tu_pass.cc',
  'tu_pipeline.cc',
  'tu_query.cc',
  'tu_shader.cc',
  'tu_suballoc.cc',
  'tu_util.cc',
)

libtu_includes = [
    inc_include,
    inc_src,
    inc_freedreno,
]

tu_link_with = []
tu_deps = []
tu_flags = []

tu_wsi = false

if with_platform_x11
  tu_deps += dep_xcb_dri3
  tu_wsi = true
endif

if with_platform_wayland
  tu_deps += dep_wayland_client
  tu_wsi = true
endif

if system_has_kms_drm and not with_platform_android
  tu_wsi = true
endif

if tu_wsi
  libtu_files += 'tu_wsi.cc'
endif

if with_platform_android
  libtu_files += files('tu_android.cc')
  tu_deps += [dep_android]
endif

if with_xlib_lease
  tu_deps += [dep_xlib_xrandr]
endif

if freedreno_kmds.contains('kgsl')
  tu_flags += '-DTU_HAS_KGSL'
  libtu_files += files('tu_knl_kgsl.cc')
endif

if freedreno_kmds.contains('msm')
  tu_flags += '-DTU_HAS_MSM'
  libtu_files += files('tu_knl_drm_msm.cc', 'tu_knl_drm.cc')
  tu_deps += dep_libdrm
endif

if freedreno_kmds.contains('virtio')
  tu_flags += '-DTU_HAS_VIRTIO'
  libtu_files += files('tu_knl_drm_virtio.cc', 'tu_knl_drm.cc')
  libtu_includes += [
    inc_virtio_gpu,
    inc_virtio_vdrm,
  ]
  tu_link_with += libvdrm
  tu_deps += dep_libdrm
endif

tu_tracepoints = custom_target(
  'tu_tracepoints.[ch]',
  input: 'tu_tracepoints.py',
  output: ['tu_tracepoints.cc', 'tu_tracepoints.h', 'tu_tracepoints_perfetto.h'],
  command: [
    prog_python, '@INPUT@',
    '-p', join_paths(dir_source_root, 'src/util/perf/'),
    '--utrace-src', '@OUTPUT0@',
    '--utrace-hdr', '@OUTPUT1@',
    '--perfetto-hdr', '@OUTPUT2@',
  ],
  depend_files: u_trace_py,
)

if with_perfetto
  libtu_files += ['tu_perfetto.cc']
  tu_deps += dep_perfetto
endif

tu_cpp_args = []
tu_cpp_args += cpp.get_supported_arguments([
  '-fno-exceptions',
  '-fno-rtti',
  '-Wno-address-of-temporary',
  '-Wno-array-bounds',
  '-Wno-c99-designator',
  '-Wno-class-memaccess',
  '-Wno-missing-braces',
  '-Wno-narrowing',
  '-Wno-pointer-arith',
  '-Wno-reorder-init-list',
  '-Wno-sign-compare',
  '-Wno-switch',
  '-Wno-unused-function',
  '-Wno-writable-strings',
  '-Wno-write-strings',
])

# HACK compiler.get_supported_arguments()/has_argument() do not seem
# to be reliable for cross builds (!?!) so just jam in some args we
# need and hope for the best
if meson.is_cross_build()
  tu_cpp_args += '-Wno-array-bounds'
  tu_cpp_args += '-Wno-c++11-narrowing'
  tu_cpp_args += '-Wno-missing-braces'
  tu_cpp_args += '-Wno-unused-function'
  tu_cpp_args += '-Wno-writable-strings'
endif

libvulkan_freedreno = shared_library(
  'vulkan_freedreno',
  [libtu_files, tu_entrypoints, tu_tracepoints, freedreno_xml_header_files, sha1_h, u_format_pack_h],
  include_directories : libtu_includes,
  link_with : [
    libfreedreno_ir3,
    libfreedreno_layout,
    libfreedreno_perfcntrs,
    tu_link_with,
  ],
  dependencies : [
    idep_libfreedreno_common,
    dep_dl,
    dep_elf,
    dep_m,
    dep_thread,
    dep_valgrind,
    idep_nir,
    tu_deps,
    idep_vulkan_util,
    idep_vulkan_runtime,
    idep_vulkan_wsi,
    idep_mesautil,
  ],
  c_args : [no_override_init_args, tu_flags],
  cpp_args : [tu_cpp_args, tu_flags],
  gnu_symbol_visibility : 'hidden',
  link_args : [vulkan_icd_link_args, ld_args_bsymbolic, ld_args_gc_sections, ld_args_build_id],
  link_depends : vulkan_icd_link_depends,
  install : true,
)

if with_symbols_check
  test(
    'tu symbols check',
    symbols_check,
    args : [
      '--lib', libvulkan_freedreno,
      '--symbols-file', vulkan_icd_symbols,
      symbols_check_args,
    ],
    suite : ['freedreno'],
  )
endif

freedreno_icd = custom_target(
  'freedreno_icd',
  input : [vk_icd_gen, vk_api_xml],
  output : 'freedreno_icd.@0@.json'.format(host_machine.cpu()),
  command : [
    prog_python, '@INPUT0@',
    '--api-version', '1.1', '--xml', '@INPUT1@',
    '--lib-path', join_paths(get_option('prefix'), get_option('libdir'),
                             'libvulkan_freedreno.so'),
    '--out', '@OUTPUT@',
  ],
  build_by_default : true,
  install_dir : with_vulkan_icd_dir,
  install_tag : 'runtime',
  install : true,
)

_dev_icdname = 'freedreno_devenv_icd.@0@.json'.format(host_machine.cpu())
_dev_icd = custom_target(
  'freedreno_devenv_icd',
  input : [vk_icd_gen, vk_api_xml],
  output : _dev_icdname,
  command : [
    prog_python, '@INPUT0@',
    '--api-version', '1.1', '--xml', '@INPUT1@',
    '--lib-path', meson.current_build_dir() / 'libvulkan_freedreno.so',
    '--out', '@OUTPUT@',
  ],
  build_by_default : true,
)

devenv.append('VK_ICD_FILENAMES', _dev_icd.full_path())
