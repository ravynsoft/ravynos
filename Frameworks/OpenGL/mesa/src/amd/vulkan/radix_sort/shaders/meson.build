# Copyright © 2022 Konstantin Seurer

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

radix_sort_shaders = [
  'init.comp',
  'fill.comp',
  'histogram.comp',
  'prefix.comp',
  'scatter_0_even.comp',
  'scatter_0_odd.comp',
  'scatter_1_even.comp',
  'scatter_1_odd.comp'
]

shader_include_dir = dir_source_root + '/src/amd/vulkan/radix_sort/targets/u64'

shader_include_files = files(
  'bufref.h',
  'prefix_limits.h',
  'prefix.h',
  'push.h',
  'scatter.glsl',
  dir_source_root + '/src/amd/vulkan/radix_sort/targets/u64/config.h'
)

radix_sort_spv = []
foreach s : radix_sort_shaders
  radix_sort_spv += custom_target(
    s + '.spv.h',
    input : s,
    output : s + '.spv.h',
    command : [prog_glslang, '-V', '-I' + shader_include_dir, '--target-env', 'spirv1.3', '-x', '-o', '@OUTPUT@', '@INPUT@'] + glslang_quiet,
    depend_files: shader_include_files)
endforeach
