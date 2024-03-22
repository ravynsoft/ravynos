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

# inc_util only depends inc_include, this folder and nothing else under src
# so we use include_directories('..') instead of inc_src to tell the fact
# util is self contained.
inc_util = [inc_include, include_directories('..')]

if with_platform_android
subdir('u_gralloc')
endif

subdir('blake3')

files_mesa_util = files(
  'anon_file.h',
  'anon_file.c',
  'bigmath.h',
  'bitscan.c',
  'bitscan.h',
  'bitset.h',
  'blend.h',
  'blob.c',
  'blob.h',
  'build_id.c',
  'build_id.h',
  'cnd_monotonic.h',
  'compiler.h',
  'compress.c',
  'compress.h',
  'crc32.c',
  'crc32.h',
  'dag.c',
  'disk_cache.c',
  'disk_cache.h',
  'disk_cache_os.c',
  'disk_cache_os.h',
  'double.c',
  'double.h',
  'enum_operators.h',
  'fast_idiv_by_const.c',
  'fast_idiv_by_const.h',
  'format_r11g11b10f.h',
  'format_rgb9e5.h',
  'format_srgb.h',
  'fossilize_db.c',
  'fossilize_db.h',
  'futex.c',
  'futex.h',
  'glheader.h',
  'half_float.c',
  'half_float.h',
  'hash_table.c',
  'hash_table.h',
  'hex.h',
  'u_idalloc.c',
  'u_idalloc.h',
  'list.h',
  'log.c',
  'macros.h',
  'memstream.c',
  'memstream.h',
  'mesa-sha1.c',
  'mesa-sha1.h',
  'mesa-blake3.c',
  'mesa-blake3.h',
  'os_time.c',
  'os_time.h',
  'os_file.c',
  'os_memory_fd.c',
  'os_misc.c',
  'os_misc.h',
  'os_socket.c',
  'os_socket.h',
  'ptralloc.h',
  'perf/u_trace.h',
  'perf/u_trace.c',
  'perf/u_trace_priv.h',
  'u_process.c',
  'u_process.h',
  'u_qsort.cpp',
  'u_qsort.h',
  'rwlock.c',
  'rwlock.h',
  'sha1/sha1.c',
  'sha1/sha1.h',
  'ralloc.c',
  'ralloc.h',
  'rand_xor.c',
  'rand_xor.h',
  'rb_tree.c',
  'rb_tree.h',
  'register_allocate.c',
  'register_allocate.h',
  'rgtc.c',
  'rgtc.h',
  'rounding.h',
  'set.c',
  'set.h',
  'simple_mtx.c',
  'simple_mtx.h',
  'slab.c',
  'slab.h',
  'softfloat.c',
  'softfloat.h',
  'sparse_array.c',
  'sparse_array.h',
  'string_buffer.c',
  'string_buffer.h',
  'strndup.h',
  'strtod.c',
  'strtod.h',
  'texcompress_astc_luts.cpp',
  'texcompress_astc_luts.h',
  'texcompress_astc_luts_wrap.cpp',
  'texcompress_astc_luts_wrap.h',
  'timespec.h',
  'u_atomic.c',
  'u_atomic.h',
  'u_call_once.c',
  'u_call_once.h',
  'u_dl.c',
  'u_dl.h',
  'u_dynarray.c',
  'u_dynarray.h',
  'u_endian.h',
  'u_hash_table.c',
  'u_hash_table.h',
  'u_pointer.h',
  'u_queue.c',
  'u_queue.h',
  'u_string.h',
  'u_thread.c',
  'u_thread.h',
  'u_vector.c',
  'u_vector.h',
  'u_math.c',
  'u_math.h',
  'u_memset.h',
  'u_mm.c',
  'u_mm.h',
  'u_pack_color.h',
  'u_debug.c',
  'u_debug.h',
  'u_debug_memory.c',
  'u_cpu_detect.c',
  'u_cpu_detect.h',
  'u_printf.c',
  'u_printf.h',
  'u_worklist.c',
  'u_worklist.h',
  'vl_vlc.h',
  'vl_rbsp.h',
  'vl_zscan_data.h',
  'vl_zscan_data.c',
  'vma.c',
  'vma.h',
  'xxhash.h',
  'mesa_cache_db.c',
  'mesa_cache_db.h',
  'mesa_cache_db_multipart.c',
  'mesa_cache_db_multipart.h',
)

files_drirc = files('00-mesa-defaults.conf')

if with_amd_vk
  files_drirc += files('00-radv-defaults.conf')
endif

if with_tests
  prog_xmllint = find_program('xmllint', required : false, native : true)
  if prog_xmllint.found()
    test(
      'drirc xml validation',
      prog_xmllint,
      args : ['--noout', '--valid', files_drirc],
      suite : ['util'],
    )
  endif
endif

files_xmlconfig = files(
  'xmlconfig.c',
  'xmlconfig.h',
)

files_xmlconfig += custom_target(
  'driconf_static.h',
  input: ['driconf_static.py'] + files_drirc,
  output: 'driconf_static.h',
  command: [
    prog_python, '@INPUT@', '@OUTPUT@',
  ],
)

format_srgb = custom_target(
  'format_srgb',
  input : ['format_srgb.py'],
  output : 'format_srgb.c',
  command : [prog_python, '@INPUT0@'],
  capture : true,
)

deps_for_libmesa_util = [
  dep_zlib,
  dep_clock,
  dep_thread,
  dep_atomic,
  dep_m,
  dep_valgrind,
  dep_zstd,
  dep_dl,
  dep_unwind,
  dep_futex,
  idep_blake3,
  idep_mesautilc11
]

if with_platform_android
  deps_for_libmesa_util += dep_android
  files_debug_stack = files('u_debug_stack_android.cpp')
else
  files_debug_stack = files(
    'dbghelp.h',
    'u_debug_stack.c',
    'u_debug_symbol.c',
    'u_debug_symbol.h',
  )
endif

if with_platform_haiku
  deps_for_libmesa_util += dep_network
endif

if with_perfetto
  files_mesa_util += files(
    'perf/u_perfetto.cc',
    'perf/u_perfetto.h',
  )
  deps_for_libmesa_util += dep_perfetto
endif

if with_gpuvis
  files_mesa_util += files(
    'perf/u_gpuvis.c',
    'perf/u_gpuvis.h',
    'perf/gpuvis_trace_utils.h',
  )
endif

u_trace_py = files('perf/u_trace.py')

libmesa_util_sse41 = static_library(
  'mesa_util_sse41',
  files('streaming-load-memcpy.c'),
  c_args : [c_msvc_compat_args, sse41_args],
  include_directories : [inc_util],
  gnu_symbol_visibility : 'hidden',
)

# subdir format provide files_mesa_format
subdir('format')
files_mesa_util += files_mesa_format

_libmesa_util = static_library(
  'mesa_util',
  [files_mesa_util, files_debug_stack, format_srgb],
  include_directories : [inc_util, include_directories('format')],
  dependencies : deps_for_libmesa_util,
  link_with: [libmesa_util_sse41],
  c_args : [c_msvc_compat_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false
)

idep_mesautil = declare_dependency(
  link_with : _libmesa_util,
  include_directories : [inc_util],
  dependencies : deps_for_libmesa_util,
)

# Only install the drirc file if we build with support for parsing drirc files
if use_xmlconfig
   install_data(files_drirc, install_dir : join_paths(get_option('datadir'), 'drirc.d'), install_tag : 'runtime')
endif

xmlconfig_deps = []
if use_xmlconfig
  xmlconfig_deps += dep_expat
endif
xmlconfig_deps += dep_regex

c_xmlconfig_arg = '-DWITH_XMLCONFIG=@0@'.format(use_xmlconfig.to_int())

_libxmlconfig = static_library(
  'xmlconfig',
  files_xmlconfig,
  dependencies : [idep_mesautil, dep_m, xmlconfig_deps],
  c_args : [
    c_msvc_compat_args,
    c_xmlconfig_arg,
    '-DSYSCONFDIR="@0@"'.format(
      join_paths(get_option('prefix'), get_option('sysconfdir'))
    ),
    '-DDATADIR="@0@"'.format(
      join_paths(get_option('prefix'), get_option('datadir'))
    ),
  ],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
)

idep_xmlconfig = declare_dependency(
  dependencies : xmlconfig_deps,
  link_with : _libxmlconfig,
)

files_xxd = files('xxd.py')
glsl2spirv = files('glsl2spirv.py')

devenv.set('DRIRC_CONFIGDIR', meson.current_source_dir())

if with_tests
  # DRI_CONF macros use designated initializers (required for union
  # initializaiton), so we need c++2a since gtest forces us to use c++
  if host_machine.system() != 'windows' and cpp.has_argument('-std=c++2a')
    test('xmlconfig',
      executable('xmlconfig_test',
        files('tests/xmlconfig.cpp'),
        dependencies : [idep_mesautil, idep_xmlconfig, idep_gtest],
        override_options : ['cpp_std=c++2a'],
        cpp_args: ['-Wno-write-strings', c_xmlconfig_arg]
      ),
      suite : ['util'],
      env: ['HOME=' + join_paths(meson.current_source_dir(),
                                 'tests', 'drirc_home'),
            'DRIRC_CONFIGDIR=' + join_paths(meson.current_source_dir(),
                                            'tests', 'drirc_configdir')],
      protocol : 'gtest',
    )
  endif

  files_util_tests = files(
    'tests/bitset_test.cpp',
    'tests/blob_test.cpp',
    'tests/dag_test.cpp',
    'tests/fast_idiv_by_const_test.cpp',
    'tests/fast_urem_by_const_test.cpp',
    'tests/gc_alloc_tests.cpp',
    'tests/half_float_test.cpp',
    'tests/int_min_max.cpp',
    'tests/linear_test.cpp',
    'tests/mesa-sha1_test.cpp',
    'tests/os_mman_test.cpp',
    'tests/perf/u_trace_test.cpp',
    'tests/rb_tree_test.cpp',
    'tests/register_allocate_test.cpp',
    'tests/roundeven_test.cpp',
    'tests/set_test.cpp',
    'tests/string_buffer_test.cpp',
    'tests/timespec_test.cpp',
    'tests/u_atomic_test.cpp',
    'tests/u_call_once_test.cpp',
    'tests/u_debug_stack_test.cpp',
    'tests/u_debug_test.cpp',
    'tests/u_printf_test.cpp',
    'tests/u_qsort_test.cpp',
    'tests/vector_test.cpp',
  )

  # FIXME: this test cause a big timeout on MacOS
  if host_machine.system() != 'darwin'
    files_util_tests += files(
      'tests/sparse_array_test.cpp',
    )
  endif

  if with_shader_cache
    files_util_tests += files(
      'tests/cache_test.cpp',
    )
  endif

  test(
    'util_tests',
    executable(
      'util_tests',
      files_util_tests,
      dependencies : [idep_mesautil, idep_gtest],
    ),
    suite : ['util'],
    protocol : 'gtest',
    timeout : 180,
  )

  process_test_exe = executable(
    'process_test',
    files('tests/process_test.c'),
    dependencies : idep_mesautil,
    c_args : [c_msvc_compat_args],
  )
  if (host_machine.system() == 'windows' and build_machine.system() != 'windows')
    # This conversion is only required on mingw crosscompilers, otherwise we hit at least one of these issues
    # https://gitlab.freedesktop.org/mesa/mesa/-/issues/2690
    # https://gitlab.freedesktop.org/mesa/mesa/-/issues/2788
    prog_winepath = find_program('winepath')
    process_test_exe_full_path = run_command(
      prog_winepath, '-w', process_test_exe.full_path(),
      check : true
    ).stdout().strip()
  else
    process_test_exe_full_path = process_test_exe.full_path()
  endif

  test(
    'process',
    process_test_exe,
    suite : ['util'],
    env: ['BUILD_FULL_PATH='+process_test_exe_full_path]
  )

  test(
    'process_with_overrides',
    process_test_exe,
    suite : ['util'],
    env: [
      'BUILD_FULL_PATH='+process_test_exe_full_path,
      'MESA_PROCESS_NAME=hello',
    ]
  )

  subdir('tests/hash_table')
  subdir('tests/vma')
  subdir('tests/format')
endif
