# Copyright © 2022 Intel Corporation

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

float64_spv_h = custom_target(
  'float64_spv.h',
  input : [glsl2spirv, float64_glsl_file],
  output : 'float64_spv.h',
  command : [
    prog_python, '@INPUT@', '@OUTPUT@',
    prog_glslang,
    '--create-entry', 'main',
    '--vn', 'float64_spv_source',
    '--glsl-version', '450',
    '-Olib',
  ]
)

anv_internal_shaders = [
  [ 'generated_draws.glsl',       'frag' ],
  [ 'query_copy_compute.glsl',    'comp' ],
  [ 'query_copy_fragment.glsl',   'frag' ],
  [ 'memcpy_compute.glsl',        'comp' ],
]

anv_internal_spvs = []
foreach item : anv_internal_shaders
  f = item[0]
  stage = item[1]
  spv_filename = f.replace('.glsl', '_spv.h')
  src_name = f.replace('.glsl', '_spv_source')
  anv_internal_spvs += custom_target(
    spv_filename,
    input : [glsl2spirv, f, files('common_generated_draws.glsl',
                                  'common_query_copy.glsl',
                                  'interface.h')],
    output : spv_filename,
    command : [
      prog_python, '@INPUT0@', '@INPUT1@', '@OUTPUT@',
      prog_glslang,
      '--vn', src_name,
      '--glsl-version', '450',
      '--stage', stage,
      '-I' + meson.current_source_dir(),
      '-D_IN_SHADER_=1',
    ])
endforeach
