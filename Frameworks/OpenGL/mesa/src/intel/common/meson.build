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

fs = import('fs')

files_libintel_common = files(
  'i915/intel_engine.c',
  'i915/intel_engine.h',
  'i915/intel_gem.c',
  'i915/intel_gem.h',
  'xe/intel_device_query.c',
  'xe/intel_device_query.h',
  'xe/intel_engine.c',
  'xe/intel_engine.h',
  'xe/intel_gem.c',
  'xe/intel_gem.h',
  'intel_aux_map.c',
  'intel_aux_map.h',
  'intel_buffer_alloc.h',
  'intel_decoder.h',
  'intel_disasm.c',
  'intel_disasm.h',
  'intel_engine.c',
  'intel_engine.h',
  'intel_gem.c',
  'intel_gem.h',
  'intel_genX_state.h',
  'intel_guardband.h',
  'intel_l3_config.c',
  'intel_l3_config.h',
  'intel_tiled_render.h',
  'intel_urb_config.c',
  'intel_sample_positions.c',
  'intel_sample_positions.h',
  'intel_uuid.c',
  'intel_uuid.h',
  'intel_measure.c',
  'intel_measure.h',
  'intel_mem.c',
  'intel_mem.h',
  'intel_pixel_hash.h'
)

files_batch_decoder = files(
  'intel_batch_decoder.c',
  'intel_decoder.c',
)

libintel_common_links = [libisl]

if with_clflushopt
  libintel_clflushopt = static_library(
    'intel_clflushopt',
    ['intel_clflushopt.c'],
    include_directories : [inc_include, inc_src],
    c_args : [no_override_init_args] + clflushopt_args,
    gnu_symbol_visibility : 'hidden',
  )
  libintel_common_links += libintel_clflushopt
endif

batch_decoder_dependencies = []
if not dep_expat.found()
  files_libintel_common += 'intel_batch_decoder_stub.c'
else
  batch_decoder_dependencies += dep_expat
  files_libintel_common += files_batch_decoder
endif

libintel_common = static_library(
  'intel_common',
  [files_libintel_common, genX_xml_h, sha1_h],
  include_directories : [inc_include, inc_src, inc_intel],
  c_args : [no_override_init_args, sse2_args],
  gnu_symbol_visibility : 'hidden',
  link_with : libintel_common_links,
  dependencies : [batch_decoder_dependencies, dep_libdrm, dep_thread, idep_genxml, idep_mesautil, idep_intel_dev],
)

libintel_extra_dependencies = []
if with_platform_android
  libintel_extra_dependencies += dep_android
endif

idep_libintel_common = declare_dependency(
  link_with: [libintel_common],
  dependencies: [libintel_extra_dependencies],
)

install_intel_gpu_tests = get_option('install-intel-gpu-tests')
if install_intel_gpu_tests and not with_tests
  error('`install-intel-gpu-tests` requires `build-tests`')
endif

if with_tests and not with_platform_android
  gentest_xml = files('tests/gentest.xml')
  _name = 'gentest_pack.h'
  gentest_pack = custom_target(
    _name,
    input : [gen_pack_header_py, gentest_xml],
    output : _name,
    command : [prog_python, '@INPUT@'],
    capture : true,
    depend_files: gen_pack_header_deps
  )

  genxml_path = join_paths(dir_source_root,
                           '@0@'.format(gentest_xml[0]))

  test(
    'genxml_test',
    executable(
      'genxml_test',
      ['tests/genxml_test.c', gentest_pack],
      include_directories : [inc_include, inc_src, inc_intel],
      dependencies : [idep_mesautil, idep_intel_dev],
      link_with : libintel_common,
      c_args : [
        '-DGENXML_DIR="@0@"'.format(fs.parent(genxml_path)),
        '-DGENXML_FILE="@0@"'.format(fs.name(genxml_path)),
      ],
    ),
    args : ['-quiet'],
    suite : ['intel'],
  )

  foreach g : [['70', 'gfx7'], ['75', 'hsw'], ['80', 'gfx8'],
               ['90', 'gfx9'], ['110', 'gfx11'], ['120', 'gfx12'],
               ['125', 'gfx125']]
    executable(
      'intel_@0@_mi_builder_test'.format(g[1]),
      files('tests/mi_builder_test.cpp'),
      cpp_args : [
        cpp_msvc_compat_args,
        '-DGFX_VERx10=@0@'.format(g[0])
      ],
      gnu_symbol_visibility : 'hidden',
      include_directories : [inc_include, inc_src, inc_intel],
      link_with : [libintel_common],
      dependencies : [dep_libdrm, idep_gtest, idep_genxml, idep_mesautil, idep_intel_dev],
      install : install_intel_gpu_tests,
    )
  endforeach
endif
