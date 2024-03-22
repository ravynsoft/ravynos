# Copyright © 2020 Igalia S.L

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

freedreno_devices_h = custom_target(
  'freedreno_devices.h',
  input: 'freedreno_devices.py',
  output: 'freedreno_devices.h',
  command: [
    prog_python, '@INPUT@',
    '-p', join_paths(meson.project_build_root(), 'src/freedreno/registers/adreno/')
  ],
  depends: freedreno_py_header_files,
  capture: true,
)

libfreedreno_common = static_library(
  'freedreno_common',
  [
    'disasm.h',
    'fd6_pack.h',
    'freedreno_dev_info.c',
    'freedreno_dev_info.h',
    'freedreno_pm4.h',
    'freedreno_uuid.c',
    'freedreno_uuid.h',
    'freedreno_guardband.h',
    freedreno_devices_h,
    sha1_h,
    'redump.h'
  ],
  include_directories : [inc_freedreno, inc_include, inc_src],
  c_args : [no_override_init_args],
  build_by_default : true,
  dependencies: [idep_mesautil],
  gnu_symbol_visibility : 'hidden',
)

idep_libfreedreno_common = declare_dependency(
  link_with: [libfreedreno_common],
)
