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

files_r300 = files(
  'r300_blit.c',
  'r300_cb.h',
  'r300_chipset.c',
  'r300_chipset.h',
  'r300_context.c',
  'r300_context.h',
  'r300_cs.h',
  'r300_debug.c',
  'r300_defines.h',
  'r300_emit.c',
  'r300_emit.h',
  'r300_flush.c',
  'r300_fs.c',
  'r300_fs.h',
  'r300_hyperz.c',
  'r300_public.h',
  'r300_query.c',
  'r300_reg.h',
  'r300_render.c',
  'r300_render_stencilref.c',
  'r300_render_translate.c',
  'r300_resource.c',
  'r300_screen_buffer.c',
  'r300_screen_buffer.h',
  'r300_screen.c',
  'r300_screen.h',
  'r300_shader_semantics.h',
  'r300_state.c',
  'r300_state_derived.c',
  'r300_state_inlines.h',
  'r300_texture.c',
  'r300_texture_desc.c',
  'r300_texture_desc.h',
  'r300_texture.h',
  'r300_tgsi_to_rc.c',
  'r300_tgsi_to_rc.h',
  'r300_transfer.c',
  'r300_transfer.h',
  'r300_vs.c',
  'r300_vs_draw.c',
  'r300_vs.h',
  'compiler/memory_pool.c',
  'compiler/memory_pool.h',
  'compiler/nir_to_rc.c',
  'compiler/nir_to_rc.h',
  'compiler/r300_fragprog.c',
  'compiler/r300_fragprog_emit.c',
  'compiler/r300_fragprog.h',
  'compiler/r300_fragprog_swizzle.c',
  'compiler/r300_fragprog_swizzle.h',
  'compiler/r3xx_fragprog.c',
  'compiler/r3xx_vertprog.c',
  'compiler/r3xx_vertprog_dump.c',
  'compiler/r500_fragprog.c',
  'compiler/r500_fragprog_emit.c',
  'compiler/r500_fragprog.h',
  'compiler/r300_nir.c',
  'compiler/r300_nir.h',
  'compiler/r500_nir_lower_fcsel.c',
  'compiler/radeon_code.c',
  'compiler/radeon_code.h',
  'compiler/radeon_compiler.c',
  'compiler/radeon_compiler.h',
  'compiler/radeon_compiler_util.c',
  'compiler/radeon_compiler_util.h',
  'compiler/radeon_dataflow.c',
  'compiler/radeon_dataflow_deadcode.c',
  'compiler/radeon_dataflow.h',
  'compiler/radeon_dataflow_swizzles.c',
  'compiler/radeon_inline_literals.c',
  'compiler/radeon_list.c',
  'compiler/radeon_list.h',
  'compiler/radeon_opcodes.c',
  'compiler/radeon_opcodes.h',
  'compiler/radeon_optimize.c',
  'compiler/radeon_pair_dead_sources.c',
  'compiler/radeon_pair_regalloc.c',
  'compiler/radeon_pair_schedule.c',
  'compiler/radeon_pair_translate.c',
  'compiler/radeon_program_alu.c',
  'compiler/radeon_program_alu.h',
  'compiler/radeon_program.c',
  'compiler/radeon_program_constants.h',
  'compiler/radeon_program.h',
  'compiler/radeon_program_pair.c',
  'compiler/radeon_program_pair.h',
  'compiler/radeon_program_print.c',
  'compiler/radeon_program_tex.c',
  'compiler/radeon_program_tex.h',
  'compiler/radeon_regalloc.c',
  'compiler/radeon_regalloc.h',
  'compiler/radeon_remove_constants.c',
  'compiler/radeon_remove_constants.h',
  'compiler/radeon_rename_regs.c',
  'compiler/radeon_rename_regs.h',
  'compiler/radeon_swizzle.h',
  'compiler/radeon_variable.c',
  'compiler/radeon_variable.h',
  'compiler/radeon_vert_fc.c',
)

r300_nir_algebraic_c = custom_target(
  'r300_nir_algebraic.c',
  input : 'compiler/r300_nir_algebraic.py',
  output : 'r300_nir_algebraic.c',
  command : [
    prog_python, '@INPUT@', '-p', dir_compiler_nir, '@OUTPUT@',
  ],
  depend_files : nir_algebraic_depends,
)

libr300 = static_library(
  'r300',
  files_r300, r300_nir_algebraic_c,
  include_directories : [
    inc_src, inc_include, inc_gallium, inc_gallium_aux, inc_gallium_drivers,
    inc_mesa,
  ],
  gnu_symbol_visibility : 'hidden',
  dependencies : [dep_libdrm_radeon, dep_llvm, idep_mesautil, idep_nir],
)

driver_r300 = declare_dependency(
  compile_args : '-DGALLIUM_R300',
  link_with : [libr300, libradeonwinsys],
)

if with_tests
  test(
    'r300_compiler_test',
    executable(
      'r300_compiler_test',
      files(
        'compiler/tests/r300_compiler_tests.c',
        'compiler/tests/radeon_compiler_optimize_tests.c',
        'compiler/tests/radeon_compiler_regalloc_tests.c',
        'compiler/tests/radeon_compiler_util_tests.c',
        'compiler/tests/rc_test_helpers.c',
        'compiler/tests/unit_test.c',
      ),
      c_args : [
        '-DTEST_PATH="@0@"'.format(
          join_paths(meson.current_source_dir(), 'compiler', 'tests')
        )
      ],
      include_directories : [
        inc_src, inc_include, inc_gallium, inc_gallium_aux,
        include_directories('compiler'),
      ],
      link_with : [libr300, libgallium],
      dependencies : [dep_m, dep_clock, dep_dl, dep_unwind, idep_mesautil],
    ),
    suite : ['r300'],
  )
endif
