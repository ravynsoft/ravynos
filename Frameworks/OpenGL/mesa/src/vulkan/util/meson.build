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
vk_extensions_depend_files = [
]
vk_entrypoints_depend_files = [
  files('vk_extensions.py'),
  vk_extensions_depend_files,
]
vk_extensions_gen_depend_files = [
  files('vk_extensions.py'),
  vk_extensions_depend_files,
]
vk_dispatch_table_gen_depend_files = [
  files('vk_entrypoints.py'),
  vk_entrypoints_depend_files,
]
vk_dispatch_trampolines_gen_depend_files = [
  files('vk_entrypoints.py'),
  vk_entrypoints_depend_files,
]
vk_entrypoints_gen_depend_files = [
  files('vk_entrypoints.py'),
  vk_entrypoints_depend_files,
]
vk_cmd_queue_gen_depend_files = [
  files('vk_entrypoints.py'),
  vk_entrypoints_depend_files,
]
vk_physical_device_features_gen_depend_files = [
  files('vk_extensions.py'),
]
vk_physical_device_properties_gen_depend_files = [
  files('vk_extensions.py'),
]
vk_synchronization_helpers_gen_depend_files = [
  files('vk_extensions.py'),
]

vk_entrypoints_gen = files('vk_entrypoints_gen.py')
vk_extensions_gen = files('vk_extensions_gen.py')
vk_icd_gen = files('vk_icd_gen.py')
vk_cmd_queue_gen = files('vk_cmd_queue_gen.py')
vk_dispatch_trampolines_gen = files('vk_dispatch_trampolines_gen.py')
vk_physical_device_features_gen = files('vk_physical_device_features_gen.py')
vk_physical_device_properties_gen = files('vk_physical_device_properties_gen.py')
vk_synchronization_helpers_gen = files('vk_synchronization_helpers_gen.py')

files_vulkan_util = files(
  'vk_alloc.c',
  'vk_alloc.h',
  'vk_format.c',
  'vk_util.c',
  'vk_util.h',
)

vk_dispatch_table = custom_target(
  'vk_dispatch_table',
  input : ['vk_dispatch_table_gen.py', vk_api_xml],
  output : ['vk_dispatch_table.c', 'vk_dispatch_table.h'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@',
    '--out-c', '@OUTPUT0@', '--out-h', '@OUTPUT1@',
    '--beta', with_vulkan_beta.to_string()
  ],
  depend_files : vk_dispatch_table_gen_depend_files,
)

vk_enum_to_str = custom_target(
  'vk_enum_to_str',
  input : ['gen_enum_to_str.py', vk_api_xml],
  output : ['vk_enum_to_str.c', 'vk_enum_to_str.h', 'vk_enum_defines.h'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@',
    '--outdir', meson.current_build_dir(),
    '--beta', with_vulkan_beta.to_string()
  ],
)

vk_struct_type_cast = custom_target(
  'vk_struct_type_cast',
  input : ['vk_struct_type_cast_gen.py', vk_api_xml],
  output : ['vk_struct_type_cast.h'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@',
    '--outdir', meson.current_build_dir(),
    '--beta', with_vulkan_beta.to_string()
  ],
)

vk_extensions = custom_target(
  'vk_extensions',
  input : ['vk_extensions_gen.py', vk_api_xml],
  output : ['vk_extensions.c', 'vk_extensions.h'],
  command : [
    prog_python, '@INPUT0@', '--xml', '@INPUT1@',
    '--out-c', '@OUTPUT0@', '--out-h', '@OUTPUT1@'
  ],
  depend_files : vk_extensions_gen_depend_files,
)

libvulkan_util = static_library(
  'vulkan_util',
  [files_vulkan_util, vk_dispatch_table, vk_enum_to_str,
   vk_struct_type_cast, vk_extensions],
  include_directories : [inc_include, inc_src],
  dependencies : [vulkan_wsi_deps, idep_mesautil, idep_nir_headers],
  c_args : [c_msvc_compat_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
)

idep_vulkan_util_headers = declare_dependency(
  sources : [vk_dispatch_table[1], vk_enum_to_str[1],
             vk_struct_type_cast[0], vk_extensions[1]],
  include_directories : include_directories('.')
)

idep_vulkan_util = declare_dependency(
  link_with : libvulkan_util,
  dependencies : idep_vulkan_util_headers
)
