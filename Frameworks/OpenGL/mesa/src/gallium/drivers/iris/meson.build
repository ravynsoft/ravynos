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

files_libiris = files(
  'i915/iris_batch.c',
  'i915/iris_batch.h',
  'i915/iris_bufmgr.c',
  'i915/iris_bufmgr.h',
  'i915/iris_kmd_backend.c',
  'xe/iris_batch.c',
  'xe/iris_batch.h',
  'xe/iris_bufmgr.c',
  'xe/iris_bufmgr.h',
  'xe/iris_kmd_backend.c',
  'driinfo_iris.h',
  'iris_batch.c',
  'iris_batch.h',
  'iris_binder.c',
  'iris_binder.h',
  'iris_blit.c',
  'iris_border_color.c',
  'iris_bufmgr.c',
  'iris_bufmgr.h',
  'iris_clear.c',
  'iris_context.c',
  'iris_context.h',
  'iris_draw.c',
  'iris_fence.c',
  'iris_fence.h',
  'iris_fine_fence.c',
  'iris_fine_fence.h',
  'iris_formats.c',
  'iris_genx_macros.h',
  'iris_genx_protos.h',
  'iris_kmd_backend.c',
  'iris_kmd_backend.h',
  'iris_measure.c',
  'iris_measure.h',
  'iris_monitor.c',
  'iris_perf.h',
  'iris_perf.c',
  'iris_performance_query.c',
  'iris_pipe.h',
  'iris_pipe_control.c',
  'iris_program.c',
  'iris_program_cache.c',
  'iris_resolve.c',
  'iris_resource.c',
  'iris_resource.h',
  'iris_screen.c',
  'iris_screen.h',
  'iris_disk_cache.c',
  'iris_utrace.c',
)

iris_per_hw_ver_libs = []
foreach v : ['80', '90', '110', '120', '125', '200']
  iris_per_hw_ver_libs += static_library(
    'iris_per_hw_ver@0@'.format(v),
    ['iris_blorp.c', 'iris_query.c', 'iris_state.c', gen_xml_pack],
    include_directories : [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_intel],
    c_args : [
      no_override_init_args, sse2_args,
      '-DGFX_VERx10=@0@'.format(v),
    ],
    gnu_symbol_visibility : 'hidden',
    dependencies : [dep_libdrm, dep_valgrind, idep_genxml, idep_nir_headers,
                    idep_intel_driver_ds_headers, ],
  )
endforeach

libiris = static_library(
  'iris',
  [files_libiris, gen_xml_pack],
  include_directories : [
    inc_src, inc_include, inc_gallium, inc_gallium_aux, inc_intel,
    inc_gallium_drivers,
    # these should not be necessary, but main/macros.h...
    inc_mesa, inc_mapi
  ],
  c_args : [sse2_args],
  cpp_args : [sse2_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : [dep_libdrm, dep_valgrind, idep_genxml, idep_nir_headers,
                  idep_libintel_common, idep_intel_driver_ds, idep_intel_dev],
  link_with : [
    iris_per_hw_ver_libs, libintel_compiler, libisl,
    libblorp, libintel_perf
  ],
)

driver_iris = declare_dependency(
  compile_args : '-DGALLIUM_IRIS',
  link_with : [libiris, libiriswinsys],
)
