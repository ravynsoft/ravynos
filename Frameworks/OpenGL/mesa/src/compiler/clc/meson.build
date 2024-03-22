# Copyright © Microsoft Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.


files_libmesaclc = files(
  'clc.c',
  'clc_helpers.cpp',
  'nir_load_libclc.c',
)

_libmesaclc_c_args = []
_libmesaclc_cpp_args = ['-DLLVM_LIB_DIR="@0@"'.format(llvm_libdir)]
_libmesaclc_sources = []

if get_option('opencl-external-clang-headers') \
   .disable_auto_if(host_machine.system() == 'windows') \
   .disabled()
  # LLVM 16 moved clang header path from using full version to only major version
  if dep_llvm.version().version_compare('< 16')
    # Prior to LLVM 16, this path used a full version
    clang_version_dir = dep_llvm.version()
  else
    # LLVM 16 changed to only using a major version
    clang_version_dir = dep_llvm.version().split('.')[0]
  endif
  clang_resource_dir = join_paths(llvm_libdir, 'clang', clang_version_dir, 'include')

  if dep_llvm.version().version_compare('< 15.0')
    opencl_c_h = custom_target(
      'opencl-c.h',
      input : [files_xxd, join_paths(clang_resource_dir, 'opencl-c.h')],
      output : 'opencl-c.h.h',
      command : [prog_python, '@INPUT@', '@OUTPUT@', '-n', 'opencl_c_source'],
    )
  endif

  opencl_c_base_h = custom_target(
    'opencl-c-base.h',
    input : [files_xxd, join_paths(clang_resource_dir, 'opencl-c-base.h')],
    output : 'opencl-c-base.h.h',
    command : [prog_python, '@INPUT@', '@OUTPUT@', '-n', 'opencl_c_base_source'],
  )

  _libmesaclc_sources += [opencl_c_base_h]
  if dep_llvm.version().version_compare('< 15.0')
    _libmesaclc_sources += [opencl_c_h]
  endif

  _libmesaclc_cpp_args += ['-DUSE_STATIC_OPENCL_C_H=1']
endif

# Supported added for SPIRV 1.4 in a version that required LLVM 14.
if dep_llvm.version().version_compare('>= 14.0')
  _libmesaclc_cpp_args += ['-DHAS_SPIRV_1_4=1']
endif

_basedir = dep_clc.get_variable(pkgconfig : 'libexecdir')

_static_libclc = get_option('static-libclc')
if _static_libclc.length() > 0
  if _static_libclc.contains('all')
    _static_libclc = ['spirv', 'spirv64']
  endif
  prog_zstd = find_program('zstd', required : false, native : true)
  _zstd_static_libclc = dep_zstd.found() and prog_zstd.found()
  if _zstd_static_libclc
    _libmesaclc_c_args += '-DHAVE_STATIC_LIBCLC_ZSTD'
  endif
  foreach s : _static_libclc
    _libmesaclc_c_args += '-DHAVE_STATIC_LIBCLC_@0@'.format(s.to_upper())
    f = '@0@-mesa3d-.spv'.format(s)
    _libclc_file = _basedir / f
    if _zstd_static_libclc
      _libclc_file = custom_target(
        '@0@.zstd'.format(f),
        command : [prog_zstd, '-f', '@INPUT@', '-o', '@OUTPUT@'],
        input : [_libclc_file],
        output : '@0@.zstd'.format(f),
      )
    endif
    files_libmesaclc += custom_target(
      '@0@.h'.format(f),
      command : [
        prog_python, files_xxd, '-b', '@INPUT@', '@OUTPUT@',
        '-n', 'libclc_@0@_mesa3d_spv'.format(s),
      ],
      input : [_libclc_file],
      output : '@0@.h'.format(f),
      depend_files : files_xxd,
    )
  endforeach
else
  _libmesaclc_c_args += ['-DDYNAMIC_LIBCLC_PATH="@0@/"'.format(_basedir)]
  if not cc.has_function('mmap')
    error('mmap required for dynamic libCLC loading')
  endif
endif

_libmesaclc = static_library(
  'libmesaclc',
  files_libmesaclc,
  sources: _libmesaclc_sources,
  include_directories : [inc_include, inc_src, inc_spirv],
  c_args : _libmesaclc_c_args,
  cpp_args : [_libmesaclc_cpp_args, _libmesaclc_c_args],
  dependencies: [idep_nir, dep_clang, dep_llvm, dep_llvmspirvlib,
                 idep_mesautil, dep_spirv_tools, idep_vtn]
)

_idep_mesaclc_link_args = []
if _shared_llvm
  _idep_mesaclc_link_args += cc.get_supported_link_arguments('-fPIC')
endif

idep_mesaclc = declare_dependency(
  link_with : _libmesaclc,
  include_directories : include_directories('.'),
  link_args : _idep_mesaclc_link_args,
)
