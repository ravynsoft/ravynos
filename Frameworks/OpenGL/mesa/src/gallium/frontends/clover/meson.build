# Copyright © 2017-2018 Intel Corporation

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

clover_cpp_args = []
clover_opencl_cpp_args = [
  '-DCL_TARGET_OPENCL_VERSION=300',
  '-DCL_USE_DEPRECATED_OPENCL_1_0_APIS',
  '-DCL_USE_DEPRECATED_OPENCL_1_1_APIS',
  '-DCL_USE_DEPRECATED_OPENCL_1_2_APIS',
  '-DCL_USE_DEPRECATED_OPENCL_2_0_APIS',
  '-DCL_USE_DEPRECATED_OPENCL_2_1_APIS',
  '-DCL_USE_DEPRECATED_OPENCL_2_2_APIS',
  '-DLIBCLC_INCLUDEDIR="@0@/"'.format(dep_clc.get_variable(pkgconfig : 'includedir')),
  '-DLIBCLC_LIBEXECDIR="@0@/"'.format(dep_clc.get_variable(pkgconfig : 'libexecdir'))
]
clover_spirv_cpp_args = []
clover_incs = [inc_include, inc_src, inc_gallium, inc_gallium_aux]

# the CL header files declare attributes on the CL types. Compilers warn if
# we use them as template arguments. Disable the warning as there isn't
# anything we can do about it
if cpp.has_argument('-Wno-ignored-attributes')
   clover_cpp_args += '-Wno-ignored-attributes'
endif

if with_opencl_icd
  clover_cpp_args += '-DHAVE_CLOVER_ICD'
endif

if with_opencl_spirv
  clover_spirv_cpp_args += '-DHAVE_CLOVER_SPIRV'
endif

libclllvm = static_library(
  'clllvm',
  files(
    'llvm/codegen/bitcode.cpp',
    'llvm/codegen/common.cpp',
    'llvm/codegen/native.cpp',
    'llvm/codegen.hpp',
    'llvm/compat.hpp',
    'llvm/invocation.cpp',
    'llvm/invocation.hpp',
    'llvm/metadata.hpp',
    'llvm/util.hpp',
  ),
  include_directories : clover_incs,
  cpp_args : [
    clover_cpp_args,
    clover_opencl_cpp_args,
    clover_spirv_cpp_args,
    '-DCLANG_RESOURCE_DIR="@0@"'.format(join_paths(
      dep_llvm.get_variable(cmake : 'LLVM_LIBRARY_DIR', configtool: 'libdir'), 'clang',
      dep_llvm.version(), 'include',
    )),
  ],
  gnu_symbol_visibility : 'hidden',
  dependencies : [dep_llvm, dep_elf, dep_llvmspirvlib, idep_mesautil],
)

idep_opencl_spirv = null_dep
if with_opencl_spirv
  libclspirv = static_library(
    'clspirv',
    files('spirv/invocation.cpp', 'spirv/invocation.hpp'),
    include_directories : clover_incs,
    cpp_args : [clover_opencl_cpp_args, clover_spirv_cpp_args],
    gnu_symbol_visibility : 'hidden',
    dependencies : [dep_spirv_tools, idep_mesautil],
  )

  libclnir = static_library(
    'clnir',
    files('nir/invocation.cpp', 'nir/invocation.hpp'),
    include_directories : [clover_incs, inc_mesa],
    dependencies : [idep_nir, idep_vtn, idep_mesaclc],
    cpp_args : [clover_opencl_cpp_args, clover_spirv_cpp_args],
    gnu_symbol_visibility : 'hidden',
  )

  idep_opencl_spirv = declare_dependency(
    dependencies : [idep_nir],
    link_with : [libclspirv, libclnir],
  )
endif

clover_files = files(
  'api/context.cpp',
  'api/device.cpp',
  'api/dispatch.cpp',
  'api/dispatch.hpp',
  'api/event.cpp',
  'api/interop.cpp',
  'api/invalid.cpp',
  'api/kernel.cpp',
  'api/memory.cpp',
  'api/platform.cpp',
  'api/program.cpp',
  'api/queue.cpp',
  'api/sampler.cpp',
  'api/transfer.cpp',
  'api/util.hpp',
  'core/binary.cpp',
  'core/binary.hpp',
  'core/compiler.hpp',
  'core/context.cpp',
  'core/context.hpp',
  'core/device.cpp',
  'core/device.hpp',
  'core/error.hpp',
  'core/event.cpp',
  'core/event.hpp',
  'core/format.cpp',
  'core/format.hpp',
  'core/kernel.cpp',
  'core/kernel.hpp',
  'core/memory.cpp',
  'core/memory.hpp',
  'core/object.hpp',
  'core/platform.cpp',
  'core/platform.hpp',
  'core/printf.cpp',
  'core/printf.hpp',
  'core/program.cpp',
  'core/program.hpp',
  'core/property.hpp',
  'core/queue.cpp',
  'core/queue.hpp',
  'core/resource.cpp',
  'core/resource.hpp',
  'core/sampler.cpp',
  'core/sampler.hpp',
  'core/timestamp.cpp',
  'core/timestamp.hpp',
  'util/adaptor.hpp',
  'util/algebra.hpp',
  'util/algorithm.hpp',
  'util/compat.hpp',
  'util/factor.hpp',
  'util/functional.hpp',
  'util/lazy.hpp',
  'util/pointer.hpp',
  'util/range.hpp',
  'util/tuple.hpp',
)

libclover = static_library(
  'clover',
  [clover_files, sha1_h],
  include_directories : clover_incs,
  cpp_args : [
    clover_opencl_cpp_args,
    clover_spirv_cpp_args,
    clover_cpp_args,
  ],
  gnu_symbol_visibility : 'hidden',
  link_with : [libclllvm],
  dependencies : [idep_mesautil, idep_nir, idep_opencl_spirv],
)
