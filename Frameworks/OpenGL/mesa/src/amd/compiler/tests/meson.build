# Copyright © 2020 Valve Corporation

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
aco_tests_files = files(
  'framework.h',
  'helpers.cpp',
  'helpers.h',
  'main.cpp',
  'test_assembler.cpp',
  'test_builder.cpp',
  'test_d3d11_derivs.cpp',
  'test_hard_clause.cpp',
  'test_insert_nops.cpp',
  'test_insert_waitcnt.cpp',
  'test_isel.cpp',
  'test_optimizer.cpp',
  'test_reduce_assign.cpp',
  'test_regalloc.cpp',
  'test_optimizer_postRA.cpp',
  'test_sdwa.cpp',
  'test_to_hw_instr.cpp',
  'test_tests.cpp',
)

spirv_files = files(
  'test_isel.cpp',
  'test_d3d11_derivs.cpp',
)

gen_spirv = generator(prog_python,
                      output    : '@BASENAME@-spirv.h',
                      arguments : [join_paths(meson.current_source_dir(), 'glsl_scraper.py'),
                                   '@INPUT@', '--with-glslang', prog_glslang.full_path(), '-o', '@OUTPUT@'])
gen_spirv_files = gen_spirv.process(spirv_files)

test(
  'aco_tests',
  executable(
    'aco_tests',
    [aco_tests_files, gen_spirv_files],
    cpp_args : ['-DACO_TEST_SOURCE_DIR="@0@"'.format(meson.current_source_dir()),
                '-DACO_TEST_BUILD_ROOT="@0@"'.format(meson.project_build_root()),
                '-DACO_TEST_PYTHON_BIN="@0@"'.format(prog_python.full_path())] +
               cpp_args_aco,
    include_directories : [
      inc_include, inc_src, inc_amd, inc_amd_common, inc_amd_common_llvm,
    ],
    link_with : [
      libamd_common, libamd_common_llvm, libvulkan_radeon,
    ],
    dependencies : [
      dep_llvm, dep_thread, idep_aco, idep_nir, idep_mesautil, idep_vulkan_util_headers, idep_amdgfxregs_h,
    ],
    gnu_symbol_visibility : 'hidden',
    build_by_default : true,
  ),
  suite : ['amd', 'compiler'],
)
