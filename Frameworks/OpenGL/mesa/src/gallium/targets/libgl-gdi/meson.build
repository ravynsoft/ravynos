# Copyright © 2018 Intel Corporation

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


opengl32_link_args = []

opengl32_def = custom_target(
  'opengl32.def',
  input: 'opengl32.def.in',
  output : 'opengl32.def',
  command : gen_vs_module_defs_normal_command,
)

libopengl32 = shared_library(
  'opengl32',
  ['stw_wgl.c'],
  vs_module_defs : opengl32_def,
  include_directories : [
    inc_include, inc_wgl, inc_src, inc_gallium,
  ],
  link_with : [
    libgallium_wgl, libglapi_bridge, libglapi
  ],
  dependencies : [
    idep_mesautil
  ],
  c_args : ['-D_GDI32_'],
  link_args : opengl32_link_args,
  name_prefix : '',  # otherwise mingw will create libopengl32.dll
  install : true,
)

if with_tests
  extra_test_defs = []

  # The generated MinGW 32-bits import libraries are always broken due to missing @nn suffix on
  # symbols, no matter what we do.  So instead we use the builtin libopengl32.a
  extra_test_deps = [cpp.find_library('opengl32')]

  if with_gallium_d3d12
    extra_test_defs += ['-DGALLIUM_D3D12']
    extra_test_deps += [cpp.find_library('d3d12')]
  endif

  test_wgl = executable(
    'test_wgl',
    files('tests/wgl_tests.cpp'),
    cpp_args : [cpp_msvc_compat_args, extra_test_defs],
    dependencies : [idep_gtest, dep_dxheaders, extra_test_deps],
  )

  wgl_test_env = environment()
  wgl_test_env.append('PATH', libgallium_wgl_build_dir)
  if with_shared_glapi
    wgl_test_env.append('PATH', libglapi_build_dir)
  endif

  test(
    'wgl',
    test_wgl,
    suite : ['wgl'],
    env : wgl_test_env,
    depends : [libopengl32],
    protocol : 'gtest',
  )
endif
