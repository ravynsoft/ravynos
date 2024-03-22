# Copyright © 2019 Valve Corporation

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

amd_common_llvm_files = files(
  'ac_llvm_build.c',
  'ac_llvm_build.h',
  'ac_llvm_helper.cpp',
  'ac_llvm_util.c',
  'ac_llvm_util.h',
  'ac_nir_to_llvm.c',
  'ac_nir_to_llvm.h',
  'ac_shader_abi.h',
)

libamd_common_llvm = static_library(
  'amd_common_llvm',
  [amd_common_llvm_files],
  include_directories : [
    inc_include, inc_src, inc_amd, inc_amd_common
  ],
  link_with: [
    libamd_common
  ],
  dependencies : [
    dep_llvm, dep_thread, dep_elf, dep_libdrm_amdgpu, dep_valgrind,
    idep_nir_headers, idep_amdgfxregs_h,
  ],
  gnu_symbol_visibility : 'hidden',
)

