
lvp_entrypoints = custom_target(
  'lvp_entrypoints',
  input : [vk_entrypoints_gen, vk_api_xml],
  output : ['lvp_entrypoints.h', 'lvp_entrypoints.c'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@', '--proto', '--weak',
    '--out-h', '@OUTPUT0@', '--out-c', '@OUTPUT1@', '--prefix', 'lvp',
    '--beta', with_vulkan_beta.to_string(),
  ],
  depend_files : vk_entrypoints_gen_depend_files,
)

liblvp_files = files(
    'lvp_device.c',
    'lvp_cmd_buffer.c',
    'lvp_descriptor_set.c',
    'lvp_execute.c',
    'lvp_util.c',
    'lvp_image.c',
    'lvp_formats.c',
    'lvp_inline_uniforms.c',
    'lvp_lower_exec_graph.c',
    'lvp_lower_vulkan_resource.c',
    'lvp_lower_vulkan_resource.h',
    'lvp_lower_input_attachments.c',
    'lvp_pipe_sync.c',
    'lvp_pipeline.c',
    'lvp_pipeline_cache.c',
    'lvp_query.c',
    'lvp_wsi.c') + [vk_cmd_enqueue_entrypoints[0]]

lvp_deps = []
lvp_flags = []

if with_platform_x11
  lvp_deps += dep_xcb_dri3
endif

if with_platform_wayland
  lvp_deps += dep_wayland_client
endif

liblavapipe_st = static_library(
  'lavapipe_st',
  [liblvp_files, lvp_entrypoints, sha1_h],
  c_args : [ c_msvc_compat_args, lvp_flags],
  gnu_symbol_visibility : 'hidden',
  include_directories : [ inc_include, inc_src, inc_util, inc_gallium, inc_gallium_aux, inc_llvmpipe ],
  dependencies : [ dep_llvm, idep_nir, idep_mesautil, idep_vulkan_util, idep_vulkan_wsi,
                   idep_vulkan_runtime, lvp_deps ]
)
