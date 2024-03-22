# Copyright © 2017-2019 Intel Corporation

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

subdir('shaders')

inc_anv = include_directories('.')

anv_flags = [
  no_override_init_args,
  sse2_args,
]

anv_cpp_flags = []

anv_entrypoints = custom_target(
  'anv_entrypoints',
  input : [vk_entrypoints_gen, vk_api_xml],
  output : ['anv_entrypoints.h', 'anv_entrypoints.c'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@', '--proto', '--weak',
    '--out-h', '@OUTPUT0@', '--out-c', '@OUTPUT1@', '--prefix', 'anv',
    '--beta', with_vulkan_beta.to_string(),
    '--device-prefix', 'gfx9',
    '--device-prefix', 'gfx11',
    '--device-prefix', 'gfx12',
    '--device-prefix', 'gfx125',
    '--device-prefix', 'gfx20',
    '--device-prefix', 'doom64',
    '--device-prefix', 'hitman3',
    '--device-prefix', 'android'
  ],
  depend_files : vk_entrypoints_gen_depend_files,
)

idep_anv_headers = declare_dependency(
  sources : [anv_entrypoints[0]],
  include_directories : inc_anv,
)

if with_intel_vk_rt
  subdir('grl')
  optional_libgrl = [libgrl]
  anv_flags += '-DANV_SUPPORT_RT=1'
else
  idep_grl = null_dep
  optional_libgrl = []
  anv_flags += '-DANV_SUPPORT_RT=0'
endif

intel_icd = custom_target(
  'intel_icd',
  input : [vk_icd_gen, vk_api_xml],
  output : 'intel_icd.@0@.json'.format(host_machine.cpu()),
  command : [
    prog_python, '@INPUT0@',
    '--api-version', '1.3', '--xml', '@INPUT1@',
    '--lib-path', join_paths(get_option('prefix'), get_option('libdir'),
                             'libvulkan_intel.so'),
    '--out', '@OUTPUT@',
  ],
  build_by_default : true,
  install_dir : with_vulkan_icd_dir,
  install_tag : 'runtime',
  install : true,
)

_dev_icdname = 'intel_devenv_icd.@0@.json'.format(host_machine.cpu())
_dev_icd = custom_target(
  'intel_devenv_icd',
  input : [vk_icd_gen, vk_api_xml],
  output : _dev_icdname,
  command : [
    prog_python, '@INPUT0@',
    '--api-version', '1.3', '--xml', '@INPUT1@',
    '--lib-path', meson.current_build_dir() / 'libvulkan_intel.so',
    '--out', '@OUTPUT@',
  ],
  build_by_default : true,
)

devenv.append('VK_ICD_FILENAMES', _dev_icd.full_path())

libanv_per_hw_ver_libs = []
anv_per_hw_ver_files = files(
  'genX_blorp_exec.c',
  'genX_cmd_buffer.c',
  'genX_gfx_state.c',
  'genX_gpu_memcpy.c',
  'genX_init_state.c',
  'genX_pipeline.c',
  'genX_query.c',
  'genX_simple_shader.c',
  'genX_video.c',
)
if with_intel_vk_rt
  anv_per_hw_ver_files += files('genX_acceleration_structure.c',)
endif

foreach _gfx_ver : ['90', '110', '120', '125', '200']
  libanv_per_hw_ver_libs += static_library(
    'anv_per_hw_ver@0@'.format(_gfx_ver),
    [anv_per_hw_ver_files, anv_entrypoints[0], anv_internal_spvs, ],
    include_directories : [
      inc_include, inc_src, inc_intel,
    ],
    c_args : anv_flags + ['-DGFX_VERx10=@0@'.format(_gfx_ver)],
    gnu_symbol_visibility : 'hidden',
    dependencies : [
      dep_libdrm, dep_valgrind, idep_nir_headers, idep_genxml,
      idep_vulkan_util_headers, idep_vulkan_wsi_headers,
      idep_vulkan_runtime_headers, idep_intel_driver_ds_headers,
      idep_grl,
    ],
  )
endforeach

libanv_files = files(
  'i915/anv_batch_chain.c',
  'i915/anv_batch_chain.h',
  'i915/anv_device.c',
  'i915/anv_device.h',
  'i915/anv_gem.c',
  'i915/anv_gem.h',
  'i915/anv_kmd_backend.c',
  'i915/anv_queue.c',
  'i915/anv_queue.h',
  'layers/anv_doom64.c',
  'layers/anv_hitman3.c',
  'layers/anv_android_layer.c',
  'xe/anv_batch_chain.c',
  'xe/anv_batch_chain.h',
  'xe/anv_kmd_backend.c',
  'xe/anv_device.c',
  'xe/anv_device.h',
  'xe/anv_queue.c',
  'xe/anv_queue.h',
  'anv_allocator.c',
  'anv_android.h',
  'anv_batch_chain.c',
  'anv_blorp.c',
  'anv_bo_sync.c',
  'anv_cmd_buffer.c',
  'anv_descriptor_set.c',
  'anv_device.c',
  'anv_formats.c',
  'anv_genX.h',
  'anv_image.c',
  'anv_internal_kernels.c',
  'anv_internal_kernels.h',
  'anv_kmd_backend.c',
  'anv_kmd_backend.h',
  'anv_measure.c',
  'anv_measure.h',
  'anv_mesh_perprim_wa.c',
  'anv_nir.h',
  'anv_nir_apply_pipeline_layout.c',
  'anv_nir_compute_push_layout.c',
  'anv_nir_lower_multiview.c',
  'anv_nir_lower_load_patch_vertices_in.c',
  'anv_nir_lower_ubo_loads.c',
  'anv_nir_lower_resource_intel.c',
  'anv_nir_push_descriptor_analysis.c',
  'anv_perf.c',
  'anv_pipeline.c',
  'anv_pipeline_cache.c',
  'anv_private.h',
  'anv_queue.c',
  'anv_sparse.c',
  'anv_astc_emu.c',
  'anv_util.c',
  'anv_utrace.c',
  'anv_va.c',
  'anv_video.c',
  'anv_wsi.c',
)

anv_deps = [
  dep_libdrm,
  dep_valgrind,
  idep_genxml,
  idep_nir_headers,
  idep_vulkan_util_headers,
  idep_vulkan_runtime_headers,
  idep_vulkan_wsi_headers,
]

if with_platform_x11
  anv_deps += dep_xcb_dri3
endif

if with_platform_wayland
  anv_deps += dep_wayland_client
endif

if with_xlib_lease
  anv_deps += [dep_xlib_xrandr]
endif

if with_platform_android
  anv_deps += idep_u_gralloc
  libanv_files += files('anv_android.c')
else
  libanv_files += files('anv_android_stubs.c')
endif

anv_deps += idep_intel_driver_ds_headers

libanv_common = static_library(
  'anv_common',
  [
    libanv_files, anv_entrypoints, sha1_h,
    gen_xml_pack, float64_spv_h,
    anv_internal_spvs,
  ],
  include_directories : [
    inc_include, inc_src, inc_intel,
    inc_util,
  ],
  c_args : anv_flags,
  cpp_args : anv_cpp_flags,
  gnu_symbol_visibility : 'hidden',
  dependencies : anv_deps
)

libvulkan_intel = shared_library(
  'vulkan_intel',
  [files('anv_gem.c'), anv_entrypoints[0]],
  include_directories : [
    inc_include, inc_src, inc_intel,
  ],
  link_whole : [libanv_common, libanv_per_hw_ver_libs] + optional_libgrl,
  link_with : [
    libintel_compiler, libisl, libblorp, libintel_perf,
  ],
  dependencies : [
    dep_thread, dep_dl, dep_m, anv_deps, idep_libintel_common,
    idep_nir, idep_genxml, idep_vulkan_util, idep_vulkan_wsi,
    idep_vulkan_runtime, idep_mesautil, idep_xmlconfig,
    idep_intel_driver_ds, idep_intel_dev,
  ],
  c_args : anv_flags,
  gnu_symbol_visibility : 'hidden',
  link_args : [vulkan_icd_link_args, ld_args_build_id, ld_args_bsymbolic, ld_args_gc_sections],
  link_depends : vulkan_icd_link_depends,
  install : true,
)

if with_symbols_check
  test(
    'anv symbols check',
    symbols_check,
    args : [
      '--lib', libvulkan_intel,
      '--symbols-file', vulkan_icd_symbols,
      symbols_check_args,
    ],
    suite : ['intel'],
  )
endif

if with_tests
  libvulkan_intel_test = static_library(
    'vulkan_intel_test',
    [files('anv_gem_stubs.c'), anv_entrypoints[0]],
    include_directories : [
      inc_include, inc_src, inc_intel,
    ],
    link_whole : libanv_common,
    link_with : [
      libanv_per_hw_ver_libs, libintel_compiler, libintel_common,
      libisl, libblorp, libintel_perf,
    ] + optional_libgrl,
    dependencies : [
      dep_thread, dep_dl, dep_m, anv_deps,
      idep_nir, idep_vulkan_util, idep_vulkan_wsi, idep_vulkan_runtime,
      idep_mesautil, idep_intel_dev,
    ],
    c_args : anv_flags,
    gnu_symbol_visibility : 'hidden',
  )

  files_anv_tests = files(
    'tests/anv_tests.cpp',

    'tests/state_pool.c',
    'tests/state_pool_free_list_only.c',
    'tests/state_pool_max_size.c',
    'tests/state_pool_no_free.c',
    'tests/state_pool_padding.c',
    'tests/block_pool_no_free.c',
    'tests/block_pool_grow_first.c',
    'tests/block_pool_max_size.c',
  )

  test(
    'anv_tests',
    executable(
      'anv_tests',
      [files_anv_tests, anv_entrypoints[0]],
      c_args : [ sse2_args ],
      link_with : libvulkan_intel_test,
      dependencies : [
        idep_gtest, dep_libdrm, dep_thread, dep_m, dep_valgrind,
        idep_vulkan_util, idep_vulkan_wsi_headers,
        idep_vulkan_runtime, idep_intel_driver_ds, idep_intel_dev,
      ],
      include_directories : [
        inc_include, inc_src, inc_intel,
      ],
    ),
    suite : ['intel'],
    protocol : 'gtest',
  )
endif
