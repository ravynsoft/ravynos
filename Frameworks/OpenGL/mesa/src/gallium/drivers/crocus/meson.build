# Copyright © 2017-2019 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

files_libcrocus = files(
  'gen4_blorp_exec.h',
  'driinfo_crocus.h',
  'crocus_batch.c',
  'crocus_batch.h',
  'crocus_blit.c',
  'crocus_bufmgr.c',
  'crocus_bufmgr.h',
  'crocus_clear.c',
  'crocus_context.c',
  'crocus_context.h',
  'crocus_draw.c',
  'crocus_fence.c',
  'crocus_fence.h',
  'crocus_fine_fence.c',
  'crocus_fine_fence.h',
  'crocus_formats.c',
  'crocus_genx_macros.h',
  'crocus_genx_protos.h',
  'crocus_monitor.c',
  'crocus_perf.c',
  'crocus_perf.h',
  'crocus_performance_query.c',
  'crocus_pipe.h',
  'crocus_pipe_control.c',
  'crocus_program.c',
  'crocus_program_cache.c',
  'crocus_resolve.c',
  'crocus_resource.c',
  'crocus_resource.h',
  'crocus_screen.c',
  'crocus_screen.h',
  'crocus_disk_cache.c',
)

crocus_per_hw_ver_libs = []
foreach v : ['40', '45', '50', '60', '70', '75', '80']
  crocus_per_hw_ver_libs += static_library(
    'crocus_per_hw_ver@0@'.format(v),
    ['crocus_blorp.c', 'crocus_query.c', 'crocus_state.c', 'crocus_blt.c', gen_xml_pack],
    include_directories : [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_intel],
    c_args : [
      no_override_init_args, sse2_args,
      '-DGFX_VERx10=@0@'.format(v),
    ],
    gnu_symbol_visibility : 'hidden',
    dependencies : [dep_libdrm, dep_valgrind, idep_genxml, idep_nir_headers, idep_intel_dev],
  )
endforeach

libcrocus = static_library(
  'crocus',
  [files_libcrocus, gen_xml_pack],
  include_directories : [
    inc_src, inc_include, inc_gallium, inc_gallium_aux, inc_intel,
    inc_gallium_drivers,
    # these should not be necessary, but main/macros.h...
    inc_mesa, inc_mapi
  ],
  c_args : [sse2_args],
  cpp_args : [sse2_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : [dep_libdrm, dep_valgrind, idep_genxml, idep_libintel_common, idep_nir_headers, idep_intel_dev],
  link_with : [
    crocus_per_hw_ver_libs, libintel_compiler, libisl,
    libblorp, libintel_perf
  ],
)

driver_crocus = declare_dependency(
  compile_args : '-DGALLIUM_CROCUS',
  link_with : [libcrocus, libcrocuswinsys],
)
