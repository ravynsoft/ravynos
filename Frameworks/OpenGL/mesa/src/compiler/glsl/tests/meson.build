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

general_ir_test_files = files(
  'array_refcount_test.cpp',
  'builtin_variable_test.cpp',
  'general_ir_test.cpp',
  'opt_add_neg_to_sub_test.cpp',
)
general_ir_test_files += ir_expression_operation_h

if with_gles2
  general_ir_test_files += files('test_gl_lower_mediump.cpp')
endif

test(
  'general_ir_test',
  executable(
    'general_ir_test',
    general_ir_test_files,
    cpp_args : [cpp_msvc_compat_args],
    gnu_symbol_visibility : 'hidden',
    include_directories : [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_glsl],
    link_with : [libglsl, libglsl_standalone, libglsl_util],
    dependencies : [dep_clock, dep_thread, idep_gtest, idep_mesautil, idep_nir],
  ),
  suite : ['compiler', 'glsl'],
  protocol : 'gtest',
)

test(
  'sampler_types_test',
  executable(
    'sampler_types_test',
    ['sampler_types_test.cpp', ir_expression_operation_h],
    cpp_args : [cpp_msvc_compat_args],
    gnu_symbol_visibility : 'hidden',
    include_directories : [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_glsl],
    link_with : [libglsl, libglsl_util],
    dependencies : [dep_thread, idep_gtest, idep_mesautil, idep_compiler],
  ),
  suite : ['compiler', 'glsl'],
  protocol : 'gtest',
)

test(
  'list_iterators',
  executable(
    'list_iterators',
    ['list_iterators.cpp'],
    cpp_args : [cpp_msvc_compat_args],
    gnu_symbol_visibility : 'hidden',
    include_directories : [inc_include, inc_src, inc_glsl],
    link_with : [libglsl, libglsl_util],
    dependencies : [dep_thread, idep_gtest],
  ),
  suite : ['compiler', 'glsl'],
  protocol : 'gtest',
)

# Meson can't auto-skip these on cross builds because of the python wrapper
if meson.can_run_host_binaries()
  test(
    'glsl compiler warnings',
    prog_python,
    args : [
      files('warnings_test.py'),
      '--glsl-compiler', glsl_compiler,
      '--test-directory', join_paths(
        dir_source_root, 'src', 'compiler', 'glsl', 'tests', 'warnings'
      ),
    ],
    suite : ['compiler', 'glsl'],
    timeout: 60,
  )

  test(
    'glsl optimization',
    prog_python,
    args : [
      files('optimization_test.py'),
      '--test-runner', glsl_test
    ],
    suite : ['compiler', 'glsl'],
  )
endif

if with_tools.contains('glsl')
  test(
    'glsl dump-builder test',
    glsl_compiler,
    args : ['--version', '110', '--dump-builder',
            files('standalone_dump-builder.frag'),
           ],
    suite : ['compiler', 'glsl'],
  )
  if with_gles2
    test(
      'glsl lower-precision test',
      prog_python,
      args : [files('lower_precision_test.py'),
              glsl_compiler
            ],
      suite : ['compiler', 'glsl'],
    )
  endif
endif
