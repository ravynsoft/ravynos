# Copyright ©

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

rust = import('unstable-rust')

libmesa_rust_util_files = files(
  'util/lib.rs',
  'util/assert.rs',
  'util/bitset.rs',
  'util/feature.rs',
  'util/properties.rs',
  'util/ptr.rs',
  'util/string.rs',
)

libmesa_rust_files = files(
  'mesa/lib.rs',
  'mesa/compiler.rs',
  'mesa/compiler/clc.rs',
  'mesa/compiler/clc/spirv.rs',
  'mesa/compiler/nir.rs',
  'mesa/pipe.rs',
  'mesa/pipe/context.rs',
  'mesa/pipe/device.rs',
  'mesa/pipe/fence.rs',
  'mesa/pipe/screen.rs',
  'mesa/pipe/transfer.rs',
)

rusticl_proc_macros_files = files(
  'proc/lib.rs',
)

rusticl_files = files(
  'lib.rs',
  'api.rs',
  'api/context.rs',
  'api/device.rs',
  'api/event.rs',
  'api/icd.rs',
  'api/kernel.rs',
  'api/memory.rs',
  'api/platform.rs',
  'api/program.rs',
  'api/queue.rs',
  'api/types.rs',
  'api/util.rs',
  'core.rs',
  'core/context.rs',
  'core/device.rs',
  'core/format.rs',
  'core/kernel.rs',
  'core/memory.rs',
  'core/platform.rs',
  'core/program.rs',
  'core/queue.rs',
  'core/util.rs',
  'core/version.rs',
  'core/gl.rs',
)

rusticl_args = [
  # we want unsafe blocks inside unsafe functions
  '-Dunsafe_op_in_unsafe_fn',
  # we error on all clippy warnings unless they are disabled
  '-Dclippy::all',
  # we want to add asserts in control flow
  '-Aclippy::assertions_on_constants',
  # warns on Arc<_> as keys
  '-Aclippy::mutable_key_type',
  '-Aclippy::not_unsafe_ptr_arg_deref',
  # dunno, kind of looks nicier being explicit
  '-Aclippy::redundant_field_names',
  '-Aclippy::too_many_arguments',
  '-Aclippy::type_complexity',
]

if rustc.version().version_compare('>=1.72')
  rusticl_args += [
    # Needs to be fixed
    '-Aclippy::arc-with-non-send-sync'
  ]
endif

if with_platform_x11
  rusticl_args += [
    '--cfg', 'glx',
  ]
endif

rusticl_gen_args = [
  # can't do anything about it anyway
  '-Aclippy::all',
  '-Aimproper_ctypes',
  # Some bindgen versions assume `unsafe_op_in_unsafe_fn`
  '-Aunused_unsafe',
  '-Anon_camel_case_types',
  '-Anon_snake_case',
  '-Anon_upper_case_globals',
]

rusticl_bindgen_args = [
  '--no-convert-floats',
  '--use-array-pointers-in-arguments',
  '--default-enum-style', 'rust',
  '--with-derive-partialeq',
  '--with-derive-eq',
  '--with-derive-partialord',
  '--with-derive-ord',
  '--with-derive-hash',
  '--with-derive-default',
  '--anon-fields-prefix', 'anon_',
]

bindgen_version = find_program('bindgen').version()

if bindgen_version == 'unknown'
  error('Failed to detect bindgen version. If you are using bindgen 0.69.0, please either update to 0.69.1 or downgrade to 0.68.1. You can install the latest version for your user with `cargo install bindgen-cli`.')
endif

if bindgen_version.version_compare('< 0.62')
  error('rusticl requires bindgen 0.62 or newer. If your distribution does not ship a recent enough version, you can install the latest version for your user with `cargo install bindgen-cli`.')
endif

if bindgen_version.version_compare('< 0.65')
  rusticl_bindgen_args += [
    '--size_t-is-usize',
  ]
endif

rusticl_bindgen_c_args = [
  '-fno-builtin-malloc',
]

cl_c_args = [
  '-DCL_USE_DEPRECATED_OPENCL_1_0_APIS',
  '-DCL_USE_DEPRECATED_OPENCL_1_1_APIS',
  '-DCL_USE_DEPRECATED_OPENCL_1_2_APIS',
  '-DCL_USE_DEPRECATED_OPENCL_2_0_APIS',
  '-DCL_USE_DEPRECATED_OPENCL_2_1_APIS',
  '-DCL_USE_DEPRECATED_OPENCL_2_2_APIS',
  '-DCL_TARGET_OPENCL_VERSION=300',
]

rusticl_opencl_bindings_rs = rust.bindgen(
  input : [
    'rusticl_opencl_bindings.h',
    opencl_headers,
  ],
  output : 'rusticl_opencl_bindings.rs',
  include_directories : [
    inc_include,
  ],
  dependencies : [
    dep_x11,
  ],
  c_args : [
    rusticl_bindgen_c_args,
    pre_args,
    cl_c_args,
  ],
  args : [
    rusticl_bindgen_args,
    '--disable-header-comment',
    '--ignore-functions',
    # needed because bindgen adds *mut void fields...
    '--raw-line', 'unsafe impl std::marker::Sync for _cl_icd_dispatch {}',
    # _cl_image_desc contains a pointer to _cl_mem
    '--raw-line', 'unsafe impl std::marker::Send for _cl_image_desc {}',
    '--raw-line', 'unsafe impl std::marker::Sync for _cl_image_desc {}',
    '--allowlist-type', 'cl_.*',
    '--allowlist-var', 'CL_.*',
    # needed for gl_sharing extension
    '--allowlist-var', 'GL_.*',
    '--allowlist-var', 'MESA_GLINTEROP_.*',
    '--allowlist-type', 'PFNEGLGETPROCADDRESSPROC',
    '--allowlist-type', 'PFNGLXGETPROCADDRESSPROC',
    '--allowlist-type', 'PFNMESAGLINTEROP.*',
    # some info types need to be strongly typed so we can implement various get_infos
    '--new-type-alias-deref', 'cl_(mem|image|pipe|gl_texture)_info',
    '--new-type-alias-deref', 'cl_kernel_(arg|work_group)_info',
    '--new-type-alias-deref', 'cl_(event|profiling)_info',
    # turn gl interop enums into constfields so we can compare with rust types
    '--constified-enum', 'MESA_GLINTEROP_.*',
  ],
)

rusticl_opencl_gen = static_library(
  'rusticl_opencl_gen',
  rusticl_opencl_bindings_rs,
  gnu_symbol_visibility : 'hidden',
  rust_crate_type : 'rlib',
  rust_args : [
    rusticl_gen_args,
  ],
)

rusticl_llvm_bindings_rs = rust.bindgen(
  input : 'rusticl_llvm_bindings.hpp',
  output : 'rusticl_llvm_bindings.rs',
  c_args : [
    rusticl_bindgen_c_args,
    pre_args,
  ],
  dependencies : [
    dep_clang,
    dep_llvm,
    dep_llvmspirvlib,
  ],
  args : [
    # we want to limit what to generate bindings for
    '--generate', 'constructors,functions,types',
    # and all types will be opaque
    '--opaque-type', '.*',
    # LLVM/Clang/Translator stuff, only used for build-id
    # also only use functions from very basic header files, otherwise bindgen might crash :')
    '--allowlist-function', 'clang::getClangFullVersion',
    '--allowlist-function', 'llvm::LLVMContext::LLVMContext',
    '--allowlist-function', 'llvm::writeSpirv',
  ],
)

rusticl_llvm_gen = static_library(
  'rusticl_llvm_gen',
  rusticl_llvm_bindings_rs,
  gnu_symbol_visibility : 'hidden',
  rust_crate_type : 'rlib',
  rust_args : [
    rusticl_gen_args,
  ],
)

rusticl_system_bindings_wrapper = static_library(
  'system_bindings',
  [
    'rusticl_system_bindings.c',
    'rusticl_system_bindings.h',
  ],
  gnu_symbol_visibility : 'hidden',
  include_directories : [
    inc_include,
  ],
  c_args : [
    pre_args,
  ],
)

rusticl_mesa_bindings_inline_wrapper = static_library(
  'mesa_bindings_inline_wrapper',
  [
    'rusticl_mesa_inline_bindings_wrapper.c',
    'rusticl_mesa_inline_bindings_wrapper.h',
    'rusticl_nir.c',
    'rusticl_nir.h',
    sha1_h,
  ],
  gnu_symbol_visibility : 'hidden',
  include_directories : [
    inc_gallium,
    inc_gallium_aux,
    inc_include,
    inc_nir,
    inc_src,
  ],
  c_args : [
    pre_args,
    cl_c_args,
  ],
  dependencies: [
    idep_nir_headers,
    dep_valgrind,
  ],
)

rusticl_libc_bindings_rs = rust.bindgen(
  input : 'rusticl_libc_bindings.h',
  output : 'rusticl_libc_bindings.rs',
  dependencies: [
    dep_valgrind,
  ],
  c_args : [
    rusticl_bindgen_c_args,
    pre_args,
  ],
  args : [
    rusticl_bindgen_args,
    '--allowlist-function',     'close',
    '--allowlist-function',     'dlsym',
    '--allowlist-function',     'free',
    '--allowlist-function',     'malloc',
  ]
)

rusticl_mesa_bindings_rs = rust.bindgen(
  input : 'rusticl_mesa_bindings.h',
  output : 'rusticl_mesa_bindings.rs',
  include_directories : [
    inc_gallium,
    inc_gallium_aux,
    inc_include,
    inc_src,
  ],
  dependencies: [
    idep_nir_headers,
    dep_valgrind,
  ],
  c_args : [
    rusticl_bindgen_c_args,
    pre_args,
  ],
  args : [
    rusticl_bindgen_args,
    # mesa utils
    '--allowlist-function',     'blob_.*',
    '--allowlist-function',     'disk_cache_.*',
    '--allowlist-type',         'float_controls',
    '--allowlist-function',     'mesa_.*',
    '--allowlist-var',          'OS_.*',
    '--allowlist-function',     'rz?alloc_.*',
    '--allowlist-function',     'SHA1.*',
    '--allowlist-var',          'SHA1_.*',
    '--allowlist-function',     'u_.*',
    '--allowlist-function',     'util_format_.*',

    # CL API
    '--allowlist-type',         'cl_sampler_.*_mode',
    '--constified-enum-module', 'cl_sampler_.*_mode',

    # clc
    '--allowlist-function',     'clc_.*',
    '--allowlist-type',         'clc_kernel_arg_access_qualifier',
    '--bitfield-enum',          'clc_kernel_arg_access_qualifier',
    '--allowlist-type',         'clc_kernel_arg_type_qualifier',
    '--bitfield-enum',          'clc_kernel_arg_type_qualifier',

    # gl
    '--allowlist-type',         'gl_access_qualifier',
    '--bitfield-enum',          'gl_access_qualifier',
    '--allowlist-function',     'glsl_.*',

    # nir and spirv
    '--allowlist-function',     'nir_.*',
    '--allowlist-var',          'nir_debug',
    '--allowlist-var',          'NIR_DEBUG_.*',
    '--bitfield-enum',          'nir_lower_int64_options',
    '--bitfield-enum',          'nir_opt_if_options',
    '--bitfield-enum',          'nir_variable_mode',
    '--allowlist-function',     'should_.*_nir',
    '--allowlist-function',     'spirv_.*',

    # gallium
    '--allowlist-function',     'pipe_.*',
    '--allowlist-var',          'PIPE_.*',
    '--allowlist-type',         'pipe_endian',
    '--bitfield-enum',          'pipe_map_flags',
    '--allowlist-type',         'pipe_query_type',
    '--constified-enum-module', 'pipe_query_type',
    '--allowlist-type',         'pipe_resource_usage',
    '--bitfield-enum',          'pipe_resource_usage',
    '--allowlist-type',         'pipe_tex_filter',
    '--constified-enum-module', 'pipe_tex_filter',
    '--allowlist-type',         'pipe_tex_wrap',
    '--constified-enum-module', 'pipe_tex_wrap',

    # rusticl C functions
    '--allowlist-function',     'rusticl_.*',
    '--allowlist-function',     'std(err|out)_ptr',

    # winsys
    '--allowlist-var',          'WINSYS_HANDLE_TYPE_.*',
  ],
)

idep_rusticl_gen = declare_dependency(
  sources: [
    rusticl_opencl_bindings_rs,
  ],
)

libmesa_rust_gen = static_library(
  'mesa_rust_gen',
  rusticl_mesa_bindings_rs,
  gnu_symbol_visibility : 'hidden',
  link_with: [
    libgallium,
  ],
  dependencies: [
    idep_mesaclc,
  ],
  rust_crate_type : 'rlib',
  rust_args : [
    rusticl_gen_args,
  ],
)

libc_rust_gen = static_library(
  'libc_rust_gen',
  rusticl_libc_bindings_rs,
  gnu_symbol_visibility : 'hidden',
  rust_crate_type : 'rlib',
  rust_args : [
    rusticl_gen_args,
  ],
)

libmesa_rust_util = static_library(
  'mesa_rust_util',
  [libmesa_rust_util_files],
  gnu_symbol_visibility : 'hidden',
  rust_crate_type : 'rlib',
  rust_args : [
    rusticl_args,
  ],
)

libmesa_rust = static_library(
  'mesa_rust',
  [libmesa_rust_files],
  gnu_symbol_visibility : 'hidden',
  rust_crate_type : 'rlib',
  rust_args : [
    rusticl_args,
  ],
  link_with : [
    libc_rust_gen,
    libmesa_rust_gen,
    libmesa_rust_util,
    rusticl_mesa_bindings_inline_wrapper,
    rusticl_system_bindings_wrapper,
  ]
)

rusticl_proc_macros = rust.proc_macro(
  'rusticl_proc_macros',
  [rusticl_proc_macros_files],
  rust_args : [
    rusticl_args,
  ],
)

librusticl = static_library(
  'rusticl',
  [rusticl_files],
  gnu_symbol_visibility : 'hidden',
  rust_crate_type : 'staticlib',
  rust_args : [
    rusticl_args,
  ],
  link_with : [
    libc_rust_gen,
    libmesa_rust,
    libmesa_rust_gen,
    libmesa_rust_util,
    rusticl_llvm_gen,
    rusticl_opencl_gen,
    rusticl_proc_macros,
  ],
  dependencies : [
    idep_rusticl_gen,
  ],
)
