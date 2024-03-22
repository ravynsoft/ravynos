# Copyright © 2018 Collabora Ltd

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

files_libzink = files(
  'zink_lower_cubemap_to_array.c',
  'nir_to_spirv/nir_to_spirv.c',
  'nir_to_spirv/spirv_builder.c',
  'zink_batch.c',
  'zink_blit.c',
  'zink_bo.c',
  'zink_clear.c',
  'zink_compiler.c',
  'zink_context.c',
  'zink_kopper.c',
  'zink_descriptors.c',
  'zink_draw.cpp',
  'zink_fence.c',
  'zink_format.c',
  'zink_framebuffer.c',
  'zink_pipeline.c',
  'zink_program.c',
  'zink_query.c',
  'zink_render_pass.c',
  'zink_resource.c',
  'zink_screen.c',
  'zink_state.c',
  'zink_surface.c',
  'zink_synchronization.cpp',
)

zink_device_info = custom_target(
  'zink_device_info.c',
  input : ['zink_device_info.py'],
  output : ['zink_device_info.h', 'zink_device_info.c'],
  command : [
    prog_python, '@INPUT@', '@OUTPUT@', vk_api_xml
  ]
)

zink_instance = custom_target(
  'zink_instance.c',
  input : ['zink_instance.py'],
  output : ['zink_instance.h', 'zink_instance.c'],
  command : [
    prog_python, '@INPUT@', '@OUTPUT@', vk_api_xml
  ]
)

zink_nir_algebraic_c = custom_target(
  'zink_nir_algebraic.c',
  input : 'nir_to_spirv/zink_nir_algebraic.py',
  output : 'zink_nir_algebraic.c',
  command : [
    prog_python, '@INPUT@', '-p', dir_compiler_nir,
  ],
  capture : true,
  depend_files : nir_algebraic_depends,
)

zink_c_args = []
inc_zink_vk = []

if host_machine.system() == 'darwin'

  # MoltenVK options
  if with_moltenvk_dir != ''
    inc_zink_vk += include_directories(join_paths(with_moltenvk_dir, 'include'))
  else
    error('moltenvk-dir is required but not set.')
  endif

  zink_c_args += ['-x','objective-c'] # Put compiler into objective-C mode to allow for MacOS types, like IOSurface and CAMetalLayer
  zink_c_args += ['-iframework' , 'Foundation']

endif

libzink = static_library(
  'zink',
  [files_libzink, zink_device_info, zink_instance, zink_nir_algebraic_c, vk_dispatch_table],
  gnu_symbol_visibility : 'hidden',
  include_directories : [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_vulkan_util, inc_zink_vk],
  link_args : [ld_args_build_id],
  dependencies: [
    idep_nir_headers, idep_mesautil, idep_vulkan_util_headers,
    idep_vulkan_wsi_defines, idep_vulkan_util, dep_libdrm, vulkan_wsi_deps
  ],
  c_args: zink_c_args,
)

driver_zink = declare_dependency(
  compile_args : '-DGALLIUM_ZINK',
  link_with : [libzink],
)

if with_tests
  test(
    'zink_format_test',
    executable(
      'zink_format_test',
      ['zink_format_test.c', 'zink_format.c'],
      dependencies : [idep_mesautil, idep_vulkan_util],
      include_directories : [inc_gallium, inc_gallium_aux, inc_include, inc_src],
    ),
    suite : ['zink'],
  )
endif
