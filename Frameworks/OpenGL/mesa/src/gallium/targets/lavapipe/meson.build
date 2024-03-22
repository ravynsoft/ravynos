

libvulkan_lvp = shared_library(
  'vulkan_lvp',
  [ 'target.c' ],
  vs_module_defs : vulkan_api_def,
  include_directories : [ inc_src, inc_util, inc_include, inc_gallium, inc_gallium_aux, inc_gallium_winsys, inc_gallium_drivers ],
  link_whole : [ liblavapipe_st ],
  link_with : [libpipe_loader_static, libgallium, libwsw, libswdri, libws_null, libswkmsdri ],
  gnu_symbol_visibility : 'hidden',
  link_args : [ld_args_bsymbolic, ld_args_gc_sections],
  dependencies : driver_swrast,
  name_prefix : host_machine.system() == 'windows' ? '' : [],
  install : true,
)

icd_lib_path = join_paths(get_option('prefix'), get_option('libdir'))
icd_file_name = 'libvulkan_lvp.so'
if with_platform_windows
  icd_lib_path = join_paths(get_option('prefix'), get_option('bindir'))
  icd_file_name = 'vulkan_lvp.dll'
endif

icd_command = [
  prog_python, '@INPUT0@',
  '--api-version', '1.1', '--xml', '@INPUT1@',
  '--lib-path', join_paths(icd_lib_path, icd_file_name),
  '--out', '@OUTPUT@',
]
if with_platform_windows
  icd_command += '--use-backslash'
endif

lvp_icd = custom_target(
  'lvp_icd',
  input : [vk_icd_gen, vk_api_xml],
  output : 'lvp_icd.@0@.json'.format(host_machine.cpu()),
  command : icd_command,
  build_by_default : true,
  install_dir : with_vulkan_icd_dir,
  install_tag : 'runtime',
  install : true,
)

_dev_icdname = 'lvp_devenv_icd.@0@.json'.format(host_machine.cpu())
_dev_icd = custom_target(
  'lvp_devenv_icd',
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

devenv.append('VK_ICD_FILENAMES', _dev_icd.full_path())
