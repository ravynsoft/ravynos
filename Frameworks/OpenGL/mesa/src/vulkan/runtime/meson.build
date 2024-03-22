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

# Mesa-local imports in the Python files must be declared here for correct
# dependency tracking.

vulkan_runtime_files = files(
  'rmv/vk_rmv_common.c',
  'rmv/vk_rmv_common.h',
  'rmv/vk_rmv_exporter.c',
  'rmv/vk_rmv_tokens.h',
  'vk_acceleration_structure.c',
  'vk_acceleration_structure.h',
  'vk_blend.c',
  'vk_blend.h',
  'vk_buffer.c',
  'vk_buffer.h',
  'vk_buffer_view.c',
  'vk_buffer_view.h',
  'vk_cmd_copy.c',
  'vk_cmd_enqueue.c',
  'vk_command_buffer.c',
  'vk_command_buffer.h',
  'vk_command_pool.c',
  'vk_command_pool.h',
  'vk_debug_report.c',
  'vk_debug_report.h',
  'vk_debug_utils.c',
  'vk_debug_utils.h',
  'vk_deferred_operation.c',
  'vk_deferred_operation.h',
  'vk_descriptor_set_layout.c',
  'vk_descriptor_set_layout.h',
  'vk_descriptors.c',
  'vk_descriptors.h',
  'vk_descriptor_update_template.c',
  'vk_descriptor_update_template.h',
  'vk_device.c',
  'vk_device.h',
  'vk_device_memory.c',
  'vk_device_memory.h',
  'vk_fence.c',
  'vk_fence.h',
  'vk_framebuffer.c',
  'vk_framebuffer.h',
  'vk_graphics_state.c',
  'vk_graphics_state.h',
  'vk_image.c',
  'vk_image.h',
  'vk_instance.c',
  'vk_instance.h',
  'vk_log.c',
  'vk_log.h',
  'vk_meta.c',
  'vk_meta.h',
  'vk_meta_blit_resolve.c',
  'vk_meta_draw_rects.c',
  'vk_meta_clear.c',
  'vk_nir.c',
  'vk_nir.h',
  'vk_nir_convert_ycbcr.c',
  'vk_nir_convert_ycbcr.h',
  'vk_object.c',
  'vk_object.h',
  'vk_pipeline.c',
  'vk_pipeline.h',
  'vk_pipeline_cache.c',
  'vk_pipeline_cache.h',
  'vk_physical_device.c',
  'vk_physical_device.h',
  'vk_pipeline_layout.c',
  'vk_pipeline_layout.h',
  'vk_query_pool.c',
  'vk_query_pool.h',
  'vk_queue.c',
  'vk_queue.h',
  'vk_render_pass.c',
  'vk_sampler.c',
  'vk_sampler.h',
  'vk_semaphore.c',
  'vk_semaphore.h',
  'vk_shader_module.c',
  'vk_shader_module.h',
  'vk_standard_sample_locations.c',
  'vk_standard_sample_locations.h',
  'vk_sync.c',
  'vk_sync.h',
  'vk_sync_binary.c',
  'vk_sync_binary.h',
  'vk_sync_dummy.c',
  'vk_sync_dummy.h',
  'vk_sync_timeline.c',
  'vk_sync_timeline.h',
  'vk_synchronization.c',
  'vk_synchronization.h',
  'vk_texcompress_etc2.c',
  'vk_texcompress_etc2.h',
  'vk_video.c',
  'vk_video.h',
  'vk_ycbcr_conversion.c',
  'vk_ycbcr_conversion.h',
)

vulkan_runtime_deps = [
  vulkan_wsi_deps,
  idep_mesautil,
  idep_nir_headers,
  idep_vulkan_util,
]

if with_vk_compiler
  vulkan_runtime_deps += [idep_nir, idep_vtn]
endif

if dep_libdrm.found()
  vulkan_runtime_files += files('vk_drm_syncobj.c', 'vk_drm_syncobj.h')
  vulkan_runtime_deps += dep_libdrm
endif

if with_platform_android
  vulkan_runtime_files += files('vk_android.c', 'vk_android.h')
  vulkan_runtime_deps += dep_android
endif

if prog_glslang.found()
  vulkan_runtime_files += files('vk_texcompress_astc.c', 'vk_texcompress_astc.h')
  vulkan_runtime_files += custom_target(
    'astc_spv.h',
    input : astc_decoder_glsl_file,
    output : 'astc_spv.h',
    command : [prog_glslang, '-V', '-S', 'comp', '-x', '-o', '@OUTPUT@', '@INPUT@'] + glslang_quiet,
    )
endif

vk_common_entrypoints = custom_target(
  'vk_common_entrypoints',
  input : [vk_entrypoints_gen, vk_api_xml],
  output : ['vk_common_entrypoints.h', 'vk_common_entrypoints.c'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@', '--proto', '--weak',
    '--out-h', '@OUTPUT0@', '--out-c', '@OUTPUT1@', '--prefix', 'vk_common',
    '--beta', with_vulkan_beta.to_string()
  ],
  depend_files : vk_entrypoints_gen_depend_files,
)

vk_cmd_queue = custom_target(
  'vk_cmd_queue',
  input : [vk_cmd_queue_gen, vk_api_xml],
  output : ['vk_cmd_queue.c', 'vk_cmd_queue.h'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@',
    '--out-c', '@OUTPUT0@', '--out-h', '@OUTPUT1@',
    '--beta', with_vulkan_beta.to_string()
  ],
  depend_files : vk_cmd_queue_gen_depend_files,
)

vk_cmd_enqueue_entrypoints = custom_target(
  'vk_cmd_enqueue_entrypoints',
  input : [vk_entrypoints_gen, vk_api_xml],
  output : ['vk_cmd_enqueue_entrypoints.h', 'vk_cmd_enqueue_entrypoints.c'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@', '--proto', '--weak',
    '--out-h', '@OUTPUT0@', '--out-c', '@OUTPUT1@',
    '--prefix', 'vk_cmd_enqueue', '--prefix', 'vk_cmd_enqueue_unless_primary',
    '--beta', with_vulkan_beta.to_string()
  ],
  depend_files : vk_entrypoints_gen_depend_files,
)

vk_dispatch_trampolines = custom_target(
  'vk_dispatch_trampolines',
  input : [vk_dispatch_trampolines_gen, vk_api_xml],
  output : ['vk_dispatch_trampolines.c', 'vk_dispatch_trampolines.h'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@',
    '--out-c', '@OUTPUT0@', '--out-h', '@OUTPUT1@',
    '--beta', with_vulkan_beta.to_string()
  ],
  depend_files : vk_dispatch_trampolines_gen_depend_files,
)

vk_physical_device_features = custom_target(
  'vk_physical_device_features',
  input : [vk_physical_device_features_gen, vk_api_xml],
  output : ['vk_physical_device_features.c', 'vk_physical_device_features.h'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@',
    '--out-c', '@OUTPUT0@', '--out-h', '@OUTPUT1@',
    '--beta', with_vulkan_beta.to_string()
  ],
  depend_files : vk_physical_device_features_gen_depend_files,
)

vk_physical_device_properties = custom_target(
  'vk_physical_device_properties',
  input : [vk_physical_device_properties_gen, vk_api_xml],
  output : ['vk_physical_device_properties.c', 'vk_physical_device_properties.h'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@',
    '--out-c', '@OUTPUT0@', '--out-h', '@OUTPUT1@',
    '--beta', with_vulkan_beta.to_string()
  ],
  depend_files : vk_physical_device_properties_gen_depend_files,
)

vk_synchronization_helpers = custom_target(
  'vk_synchronization_helpers',
  input : [vk_synchronization_helpers_gen, vk_api_xml],
  output : 'vk_synchronization_helpers.c',
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@',
    '--out-c', '@OUTPUT0@',
    '--beta', with_vulkan_beta.to_string()
  ],
  depend_files : vk_synchronization_helpers_gen_depend_files,
)

vk_format_info = custom_target(
  'vk_format_info',
  input : ['vk_format_info_gen.py', vk_api_xml],
  output : ['vk_format_info.c', 'vk_format_info.h'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@',
    '--out-c', '@OUTPUT0@', '--out-h', '@OUTPUT1@'
  ],
)

libvulkan_runtime = static_library(
  'vulkan_runtime',
  [vulkan_runtime_files, vk_common_entrypoints,
   vk_cmd_queue, vk_cmd_enqueue_entrypoints,
   vk_dispatch_trampolines, vk_physical_device_features,
   vk_physical_device_properties, vk_synchronization_helpers,
   vk_format_info],
  include_directories : [inc_include, inc_src],
  dependencies : vulkan_runtime_deps,
  c_args : c_msvc_compat_args,
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
)

idep_vulkan_runtime_headers = declare_dependency(
  sources : [vk_cmd_queue[1], vk_physical_device_features[1], vk_physical_device_properties[1]],
  include_directories : include_directories('.'),
)

idep_vulkan_common_entrypoints_h = declare_dependency(
  sources : [vk_common_entrypoints[0]]
)

# This is likely a bug in the Meson VS backend, as MSVC with ninja works fine.
# See this discussion here:
# https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/10506
if get_option('backend').startswith('vs')
  idep_vulkan_runtime = declare_dependency(
    link_with : libvulkan_runtime,
    dependencies : idep_vulkan_runtime_headers
  )
else
  idep_vulkan_runtime = declare_dependency(
    # Instruct users of this library to link with --whole-archive.  Otherwise,
    # our weak function overloads may not resolve properly.
    link_whole : libvulkan_runtime,
    dependencies : idep_vulkan_runtime_headers
  )
endif
