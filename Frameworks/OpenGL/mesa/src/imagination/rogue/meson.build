# Copyright © 2022 Imagination Technologies Ltd.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

with_imagination_tools = with_tools.contains('imagination')

inc_rogue = include_directories([
  '.',
])

libpowervr_rogue_files = files(
  'nir/rogue_nir_lower_io.c',
  'nir/rogue_nir_pfo.c',

  'rogue.c',
  'rogue_build_data.c',
  'rogue_builder.c',
  'rogue_compile.c',
  'rogue_constreg.c',
  'rogue_debug.c',
  'rogue_encode.c',
  'rogue_info.c',
  'rogue_nir.c',
  'rogue_print.c',
  'rogue_validate.c',

  'passes/rogue_constreg.c',
  'passes/rogue_copy_prop.c',
  'passes/rogue_dce.c',
  'passes/rogue_lower_late_ops.c',
  'passes/rogue_lower_pseudo_ops.c',
  'passes/rogue_regalloc.c',
  'passes/rogue_schedule_instr_groups.c',
  'passes/rogue_schedule_uvsw.c',
  'passes/rogue_schedule_wdf.c',
  'passes/rogue_trim.c',
)

# FIXME: Remove idep_vulkan_runtime once
# the compiler/driver interface is finalised.
libpowervr_rogue = shared_library(
  'powervr_rogue',
  libpowervr_rogue_files,
  include_directories : [
    inc_imagination,
    inc_include,
    inc_src,
  ],
  c_args : [imagination_c_args, no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : [idep_mesautil, idep_nir, idep_vulkan_runtime, dep_csbgen],
  install : true,
)

subdir('tools')
