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

opencl_link_args = []
opencl_link_deps = []
opencl_version = '1'

if with_ld_version_script
  opencl_link_args += [
    '-Wl,--version-script', join_paths(meson.current_source_dir(), 'opencl.sym')
  ]
  opencl_link_deps += files('opencl.sym')
endif

llvm_libdir = dep_llvm.get_variable(cmake : 'LLVM_LIBRARY_DIR', configtool: 'libdir')
opencl_libname = with_opencl_icd ? 'MesaOpenCL' : 'OpenCL'

polly_dep = null_dep
polly_isl_dep = null_dep
if dep_llvm.version().version_compare('>=10.0.0')
  polly_dep = cpp.find_library('Polly', dirs : llvm_libdir, required : false)
  polly_isl_dep = cpp.find_library('PollyISL', dirs : llvm_libdir, required : false)
endif

dep_clang = cpp.find_library('clang-cpp', dirs : llvm_libdir, required : false)

# meson will return clang-cpp from system dirs if it's not found in llvm_libdir
linker_rpath_arg = '-Wl,--rpath=@0@'.format(llvm_libdir)
clang_test_code = '''
  #include <clang/Basic/Version.h>
  int main (void) {
    size_t found_pos = clang::getClangFullVersion().find(CLANG_VERSION_STRING);
    return found_pos == ::std::string::npos ? 1 : 0;
  }
'''
can_check_clang = (not meson.is_cross_build() or meson.can_run_host_binaries()) and cpp.has_link_argument(linker_rpath_arg)
if can_check_clang
  test_run = cpp.run(clang_test_code, name : 'dep-clang-usable',
                     dependencies : [dep_llvm, dep_clang], args : linker_rpath_arg)
  dep_clang_usable = test_run.compiled() and test_run.returncode() == 0
else
  dep_clang_usable = true
endif
if not _shared_llvm or not (dep_clang.found() and dep_clang_usable)
  dep_clang = [
    cpp.find_library('clangCodeGen', dirs : llvm_libdir),
    cpp.find_library('clangFrontendTool', dirs : llvm_libdir),
    cpp.find_library('clangFrontend', dirs : llvm_libdir),
    cpp.find_library('clangDriver', dirs : llvm_libdir),
    cpp.find_library('clangSerialization', dirs : llvm_libdir),
    cpp.find_library('clangParse', dirs : llvm_libdir),
    cpp.find_library('clangSema', dirs : llvm_libdir),
    cpp.find_library('clangAnalysis', dirs : llvm_libdir),
    cpp.find_library('clangAST', dirs : llvm_libdir),
    cpp.find_library('clangASTMatchers', dirs : llvm_libdir),
    cpp.find_library('clangEdit', dirs : llvm_libdir),
    cpp.find_library('clangLex', dirs : llvm_libdir),
    cpp.find_library('clangBasic', dirs : llvm_libdir),
    polly_dep, polly_isl_dep,
  ]
  if dep_llvm.version().version_compare('>= 15.0')
    dep_clang += cpp.find_library('clangSupport', dirs : llvm_libdir)
  endif

  # check clang once more
  if can_check_clang
    test_run = cpp.run(clang_test_code, name : 'dep-clang-usable',
                       dependencies : [dep_llvm, dep_clang], args : linker_rpath_arg)
    if not test_run.compiled() or test_run.returncode() != 0
      error('No usable clang found!')
    endif
  endif
endif

ocldef_in = files(opencl_libname + '.def.in')[0]
ocldef = custom_target(
  'ocldef.def',
  input: ocldef_in,
  output : 'ocldef.def',
  command : gen_vs_module_defs_normal_command,
)

libopencl = shared_library(
  opencl_libname,
  [],
  vs_module_defs : ocldef,
  link_args : [ld_args_gc_sections, opencl_link_args],
  link_depends : opencl_link_deps,
  link_whole : libclover,
  link_with : [libpipe_loader_dynamic, libgallium],
  dependencies : [
    idep_mesautil,
    dep_clock, dep_dl, dep_unwind, dep_elf, dep_clang, dep_version
  ],
  name_prefix : host_machine.system() == 'windows' ? '' : [],  # otherwise mingw will create libOpenCL-1.dll or libMesaOpenCL-1.dll
  version : '@0@.0.0'.format(opencl_version),
  soversion : host_machine.system() == 'windows' ? '' : opencl_version,
  install : true,
)

if with_opencl_icd
  _config = configuration_data()
  _config.set('OPENCL_LIBNAME', 'MesaOpenCL')
  _config.set('OPENCL_VERSION', opencl_version)
  configure_file(
    configuration : _config,
    input : 'mesa.icd.in',
    output : 'mesa.icd',
    install : true,
    install_tag : 'runtime',
    install_dir : join_paths(get_option('sysconfdir'), 'OpenCL', 'vendors'),
  )

  # .so is hardcoded in the icd as well
  devenv.prepend(
    'OCL_ICD_FILENAMES',
    meson.current_build_dir() / 'libMesaOpenCL.so.@0@'.format(opencl_version)
  )
endif
