# Copyright © 2018 Rob Clark
# Copyright © 2019 Collabora

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

subdir('valhall')

inc_valhall = include_directories(['.', 'valhall'])

libpanfrost_bifrost_files = files(
  'bi_helper_invocations.c',
  'bi_layout.c',
  'bi_liveness.c',
  'bi_lower_divergent_indirects.c',
  'bi_lower_swizzle.c',
  'bi_print.c',
  'bi_opt_constant_fold.c',
  'bi_opt_copy_prop.c',
  'bi_opt_dce.c',
  'bi_opt_cse.c',
  'bi_opt_push_ubo.c',
  'bi_opt_mod_props.c',
  'bi_opt_dual_tex.c',
  'bi_pressure_schedule.c',
  'bi_ra.c',
  'bi_validate.c',
  'bir.c',
  'bifrost_compile.c',
  'bifrost/bi_opt_message_preload.c',
  'bifrost/bi_pack.c',
  'bifrost/bi_schedule.c',
  'bifrost/bi_scoreboard.c',
  'valhall/va_insert_flow.c',
  'valhall/va_lower_constants.c',
  'valhall/va_lower_isel.c',
  'valhall/va_lower_split_64bit.c',
  'valhall/va_optimize.c',
  'valhall/va_mark_last.c',
  'valhall/va_merge_flow.c',
  'valhall/va_pack.c',
  'valhall/va_perf.c',
  'valhall/va_validate.c',
)

bifrost_gen_disasm_c = custom_target(
  'bifrost_gen_disasm.c',
  input : ['gen_disasm.py', 'ISA.xml'],
  output : 'bifrost_gen_disasm.c',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : files('bifrost_isa.py'),
)

bi_opcodes_c = custom_target(
  'bi_opcodes.c',
  input : ['bi_opcodes.c.py', 'ISA.xml'],
  output : 'bi_opcodes.c',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : files('bifrost_isa.py'),
)

bi_printer_c = custom_target(
  'bi_printer.c',
  input : ['bi_printer.c.py', 'ISA.xml'],
  output : 'bi_printer.c',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : files('bifrost_isa.py'),
)

bi_packer_c = custom_target(
  'bi_packer.c',
  input : ['bi_packer.c.py', 'ISA.xml'],
  output : 'bi_packer.c',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : files('bifrost_isa.py'),
)

bi_opcodes_h = custom_target(
  'bi_opcodes.h',
  input : ['bi_opcodes.h.py', 'ISA.xml'],
  output : 'bi_opcodes.h',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : files('bifrost_isa.py'),
)

idep_bi_opcodes_h = declare_dependency(
  sources : [bi_opcodes_h],
  include_directories : include_directories('.'),
)

bi_builder_h = custom_target(
  'bi_builder.h',
  input : ['bi_builder.h.py', 'ISA.xml'],
  output : 'bi_builder.h',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : files('bifrost_isa.py'),
)

idep_bi_builder_h = declare_dependency(
  sources : [bi_builder_h],
  include_directories : include_directories('.'),
)

bifrost_nir_algebraic_c = custom_target(
  'bifrost_nir_algebraic.c',
  input : 'bifrost_nir_algebraic.py',
  output : 'bifrost_nir_algebraic.c',
  command : [
    prog_python, '@INPUT@', '-p', dir_compiler_nir,
  ],
  capture : true,
  depend_files : nir_algebraic_depends,
)

libpanfrost_bifrost_disasm = static_library(
  'panfrost_bifrost_disasm',
  ['bifrost/disassemble.c', 'bi_print_common.c', bifrost_gen_disasm_c],
  include_directories : [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_panfrost_hw],
  dependencies: [idep_nir],
  link_with: [libpanfrost_util],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
)

libpanfrost_bifrost = static_library(
  'panfrost_bifrost',
  [libpanfrost_bifrost_files, bi_opcodes_c, bi_printer_c, bi_packer_c, bifrost_nir_algebraic_c, valhall_c],
  include_directories : [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_panfrost_hw, inc_valhall],
  dependencies: [idep_nir, idep_bi_opcodes_h, idep_bi_builder_h, idep_valhall_enums_h],
  link_with: [libpanfrost_util, libpanfrost_bifrost_disasm, libpanfrost_valhall_disasm],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
)

if with_tests
  test(
    'bifrost_tests',
    executable(
      'bifrost_tests',
      files(
        'test/test-constant-fold.cpp',
        'test/test-dual-texture.cpp',
        'test/test-lower-swizzle.cpp',
        'test/test-message-preload.cpp',
	'test/test-optimizer.cpp',
	'test/test-pack-formats.cpp',
	'test/test-packing.cpp',
	'test/test-scheduler-predicates.cpp',
        'valhall/test/test-add-imm.cpp',
        'valhall/test/test-validate-fau.cpp',
        'valhall/test/test-insert-flow.cpp',
        'valhall/test/test-lower-isel.cpp',
        'valhall/test/test-lower-constants.cpp',
        'valhall/test/test-mark-last.cpp',
        'valhall/test/test-merge-flow.cpp',
        'valhall/test/test-packing.cpp',
      ),
      c_args : [c_msvc_compat_args, no_override_init_args],
      gnu_symbol_visibility : 'hidden',
      include_directories : [inc_include, inc_src, inc_mesa, inc_valhall],
      dependencies: [idep_gtest, idep_nir, idep_bi_opcodes_h, idep_bi_builder_h],
      link_with : [libpanfrost_bifrost],
    ),
    suite : ['panfrost'],
    protocol : 'gtest',
  )
endif
