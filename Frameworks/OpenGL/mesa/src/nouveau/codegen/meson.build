# Copyright © 2017 Dylan Baker

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

files_libnouveau_codegen = files(
  'nv50_ir.cpp',
  'nv50_ir_bb.cpp',
  'nv50_ir_build_util.cpp',
  'nv50_ir_build_util.h',
  'nv50_ir_driver.h',
  'nv50_ir_emit_nv50.cpp',
  'nv50_ir_from_nir.cpp',
  'nv50_ir_graph.cpp',
  'nv50_ir_graph.h',
  'nv50_ir.h',
  'nv50_ir_inlines.h',
  'nv50_ir_lowering_helper.cpp',
  'nv50_ir_lowering_helper.h',
  'nv50_ir_lowering_nv50.cpp',
  'nv50_ir_peephole.cpp',
  'nv50_ir_print.cpp',
  'nv50_ir_ra.cpp',
  'nv50_ir_serialize.cpp',
  'nv50_ir_ssa.cpp',
  'nv50_ir_target.cpp',
  'nv50_ir_target.h',
  'nv50_ir_target_nv50.cpp',
  'nv50_ir_target_nv50.h',
  'nv50_ir_util.cpp',
  'nv50_ir_util.h',
  'nv50_ir_emit_gv100.cpp',
  'nv50_ir_emit_gv100.h',
  'nv50_ir_emit_gk110.cpp',
  'nv50_ir_emit_gm107.cpp',
  'nv50_ir_emit_nvc0.cpp',
  'nv50_ir_lowering_gv100.cpp',
  'nv50_ir_lowering_gv100.h',
  'nv50_ir_lowering_gm107.cpp',
  'nv50_ir_lowering_gm107.h',
  'nv50_ir_lowering_nvc0.cpp',
  'nv50_ir_lowering_nvc0.h',
  'nv50_ir_target_gv100.cpp',
  'nv50_ir_target_gv100.h',
  'nv50_ir_target_gm107.cpp',
  'nv50_ir_target_gm107.h',
  'nv50_ir_target_nvc0.cpp',
  'nv50_ir_target_nvc0.h',
)

libnouveau_codegen = static_library(
  'nouveau_codegen',
  files_libnouveau_codegen,
  include_directories : [
    inc_src, inc_include, inc_gallium, inc_gallium_aux, inc_mapi, inc_mesa,
  ],
  gnu_symbol_visibility : 'hidden',
  dependencies : [dep_libdrm, idep_mesautil, idep_nir_headers],
)

idep_nouveau_codegen = declare_dependency(
  sources : [ 'nv50_ir_driver.h' ],
  include_directories : include_directories('.'),
)
