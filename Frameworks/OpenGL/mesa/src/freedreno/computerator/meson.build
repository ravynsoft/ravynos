# Copyright © 2020 Google, Inc

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

computerator_files = [
  'a4xx.cc',
  'a6xx.cc',
  'ir3_asm.cc',
  'main.cc',
  freedreno_xml_header_files,
  ir3_parser[1],
]

computerator_cpp_args = cpp.get_supported_arguments([
  '-Wno-sign-compare',
  '-Wno-array-bounds',
])

if meson.is_cross_build()
  computerator_cpp_args += '-Wno-array-bounds'
endif

computerator = executable(
  'computerator',
  computerator_files,
  include_directories : [
    inc_freedreno,
    inc_include,
    inc_src,
  ],
  link_with : [
    libfreedreno_drm,
    libfreedreno_ir3,
    libfreedreno_perfcntrs,
    libfreedreno_common,
  ],
  dependencies : [
    dep_libdrm,
    idep_mesautil,
    # We don't actually use nir, but ir3 wants some nir headers:
    idep_nir,
  ],
  cpp_args : [computerator_cpp_args],
  build_by_default : with_tools.contains('freedreno'),
  install : with_tools.contains('freedreno'),
)
