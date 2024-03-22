# Copyright © 2017 Broadcom
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

libbroadcom_compiler_files = files(
  'nir_to_vir.c',
  'vir.c',
  'vir_dump.c',
  'vir_live_variables.c',
  'vir_opt_constant_alu.c',
  'vir_opt_copy_propagate.c',
  'vir_opt_dead_code.c',
  'vir_opt_redundant_flags.c',
  'vir_opt_small_immediates.c',
  'vir_register_allocate.c',
  'vir_to_qpu.c',
  'qpu_schedule.c',
  'qpu_validate.c',
  'v3d_tex.c',
  'v3d_compiler.h',
  'v3d_nir_lower_io.c',
  'v3d_nir_lower_image_load_store.c',
  'v3d_nir_lower_line_smooth.c',
  'v3d_nir_lower_load_store_bitsize.c',
  'v3d_nir_lower_logic_ops.c',
  'v3d_nir_lower_scratch.c',
  'v3d_nir_lower_txf_ms.c',
  'v3d_packing.c',
)

libbroadcom_compiler = static_library(
  'broadcom_compiler',
  [libbroadcom_compiler_files, v3d_xml_pack],
  include_directories : [inc_include, inc_src, inc_gallium, inc_gallium_aux, inc_broadcom],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : [dep_libdrm, dep_valgrind, idep_nir_headers],
  build_by_default : false,
)

v3d_libs += libbroadcom_compiler
