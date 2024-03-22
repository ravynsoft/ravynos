# Copyright © 2019 Raspberry Pi Ltd
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
inc_simulator = include_directories('.')

libbroadcom_simulator_files = files(
  'v3d_simulator.c',
  'v3d_simulator_wrapper.cpp',
)

files_per_version = files(
    'v3dx_simulator.c',
)

v3d_args = []
dep_v3dv3 = dependency('v3dv3', required: false)
if dep_v3dv3.found()
  v3d_args += '-DUSE_V3D_SIMULATOR'
endif

per_version_libs = []
foreach ver : v3d_versions
  per_version_libs += static_library(
    'v3d-simulator-v' + ver,
    [files_per_version, v3d_xml_pack],
    include_directories : [
      inc_src, inc_include, inc_broadcom,
    ],
    c_args : [v3d_args, '-DV3D_VERSION=' + ver],
    gnu_symbol_visibility: 'hidden',
    dependencies : [dep_v3dv3, dep_libdrm, dep_valgrind],
)
endforeach

libbroadcom_simulator = static_library(
  'broadcom_simulator',
  [libbroadcom_simulator_files],
  include_directories : [inc_src, inc_include],
  c_args : [v3d_args, no_override_init_args],
  cpp_args : [v3d_args],
  gnu_symbol_visibility : 'hidden', 
  dependencies : [dep_v3dv3, dep_libdrm, dep_valgrind],
  link_with : [per_version_libs],
  build_by_default : false,
)

v3d_libs += libbroadcom_simulator
