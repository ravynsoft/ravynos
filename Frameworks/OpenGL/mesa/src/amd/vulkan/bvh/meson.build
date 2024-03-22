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

# source file, output name, defines
bvh_shaders = [
  [
    'copy.comp',
    'copy',
    [],
  ],
  [
    'encode.comp',
    'encode',
    ['COMPACT=0'],
  ],
  [
    'encode.comp',
    'encode_compact',
    ['COMPACT=1'],
  ],
  [
    'header.comp',
    'header',
    [],
  ],
  [
    'lbvh_generate_ir.comp',
    'lbvh_generate_ir',
    [],
  ],
  [
    'lbvh_main.comp',
    'lbvh_main',
    [],
  ],
  [
    'leaf.comp',
    'leaf',
    ['ALWAYS_ACTIVE=0'],
  ],
  [
    'leaf.comp',
    'leaf_always_active',
    ['ALWAYS_ACTIVE=1'],
  ],
  [
    'morton.comp',
    'morton',
    [],
  ],
  [
    'ploc_internal.comp',
    'ploc_internal',
    [],
  ],
  [
    'update.comp',
    'update',
    [],
  ],
]

bvh_include_dir = dir_source_root + '/src/amd/vulkan/bvh'

bvh_includes = files(
  'build_helpers.h',
  'build_interface.h',
  'bvh.h',
)

bvh_spv = []
foreach s : bvh_shaders
  command = [
    prog_glslang, '-V', '-I' + bvh_include_dir, '--target-env', 'spirv1.5', '-x', '-o', '@OUTPUT@', '@INPUT@'
  ]
  command += glslang_quiet

  foreach define : s[2]
    command += '-D' + define
  endforeach

  bvh_spv += custom_target(
    s[1] + '.spv.h',
    input : s[0],
    output : s[1] + '.spv.h',
    command : command,
    depend_files: bvh_includes
  )
endforeach
