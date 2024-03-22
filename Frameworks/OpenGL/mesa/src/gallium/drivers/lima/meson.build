# Copyright © 2018 Lima Project
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

files_lima = files(
  'ir/gp/gpir.h',
  'ir/gp/nir.c',
  'ir/gp/node.c',
  'ir/gp/lower.c',
  'ir/gp/scheduler.c',
  'ir/gp/instr.c',
  'ir/gp/codegen.h',
  'ir/gp/codegen.c',
  'ir/gp/reduce_scheduler.c',
  'ir/gp/regalloc.c',
  'ir/gp/disasm.c',
  'ir/gp/optimize.c',

  'ir/pp/ppir.h',
  'ir/pp/nir.c',
  'ir/pp/node.c',
  'ir/pp/lower.c',
  'ir/pp/scheduler.c',
  'ir/pp/instr.c',
  'ir/pp/regalloc.c',
  'ir/pp/liveness.c',
  'ir/pp/codegen.h',
  'ir/pp/codegen.c',
  'ir/pp/node_to_instr.c',
  'ir/pp/disasm.c',

  'ir/lima_nir_duplicate_consts.c',
  'ir/lima_nir_duplicate_intrinsic.c',
  'ir/lima_nir_lower_uniform_to_scalar.c',
  'ir/lima_nir_split_load_input.c',
  'ir/lima_nir_split_loads.c',
  'ir/lima_nir_lower_txp.c',

  'ir/lima_ir.h',

  'lima_screen.c',
  'lima_screen.h',
  'lima_context.c',
  'lima_context.h',
  'lima_resource.c',
  'lima_resource.h',
  'lima_state.c',
  'lima_disk_cache.c',
  'lima_disk_cache.h',
  'lima_draw.c',
  'lima_program.c',
  'lima_query.c',
  'lima_bo.c',
  'lima_bo.h',
  'lima_job.c',
  'lima_job.h',
  'lima_parser.c',
  'lima_parser.h',
  'lima_util.c',
  'lima_util.h',
  'lima_texture.c',
  'lima_texture.h',
  'lima_fence.c',
  'lima_fence.h',
  'lima_format.h',
  'lima_format.c',
  'lima_gpu.h',
  'lima_blit.c',
  'lima_blit.h',
)

lima_nir_algebraic_c = custom_target(
  'lima_nir_algebraic.c',
  input : 'ir/lima_nir_algebraic.py',
  output : 'lima_nir_algebraic.c',
  command : [
    prog_python, '@INPUT@', '-p', dir_compiler_nir,
  ],
  capture : true,
  depend_files : nir_algebraic_depends,
)

liblima = static_library(
  'lima',
  files_lima, lima_nir_algebraic_c,
  include_directories : [
    inc_src, inc_include, inc_gallium, inc_gallium_aux, inc_gallium_drivers,
    inc_panfrost
  ],
  dependencies : [dep_libdrm, idep_nir_headers, idep_mesautil],
)

driver_lima = declare_dependency(
  compile_args : '-DGALLIUM_LIMA',
  link_with : [liblima, liblimawinsys, libpanfrost_shared],
  dependencies : idep_nir,
)


lima_compiler = executable(
  'lima_compiler',
  files(
    'standalone/lima_compiler_cmdline.c',
    'standalone/glsl.cpp'
  ),
  include_directories : [
    inc_src, inc_include, inc_gallium, inc_gallium_aux, inc_gallium_drivers, inc_mesa, inc_mapi,
  ],
  dependencies : [
    idep_nir,
    idep_mesautil,
  ],
  link_with : [
    liblima,
    libgallium,
    libglsl_standalone,
    libpanfrost_shared,
  ],
  build_by_default : with_tools.contains('lima'),
  install : with_tools.contains('lima'),
)

lima_disasm = executable(
  'lima_disasm',
  files(
    'standalone/lima_disasm.c',
  ),
  include_directories : [
    inc_src, inc_include, inc_gallium, inc_gallium_aux, inc_gallium_drivers, inc_mesa, inc_mapi,
  ],
  dependencies : [
    idep_mesautil,
  ],
  link_with : [
    liblima,
    libpanfrost_shared,
  ],
  build_by_default : with_tools.contains('lima'),
  install : with_tools.contains('lima'),
)

if with_tools.contains('drm-shim')
  subdir('drm-shim')
endif
