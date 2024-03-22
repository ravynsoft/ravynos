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

amd_json_files = [
  # Generated:
  '../registers/gfx6.json',
  '../registers/gfx7.json',
  '../registers/gfx8.json',
  '../registers/gfx81.json',
  '../registers/gfx9.json',
  '../registers/gfx940.json',
  '../registers/gfx10.json',
  '../registers/gfx103.json',
  '../registers/gfx11.json',
  '../registers/gfx115.json',

  # Manually written:
  '../registers/pkt3.json',
  '../registers/gfx10-rsrc.json',
  '../registers/gfx11-rsrc.json',
  '../registers/registers-manually-defined.json',
]

sid_tables_h = custom_target(
  'sid_tables_h',
  input : ['sid_tables.py', 'sid.h'] + amd_json_files,
  output : 'sid_tables.h',
  command : [prog_python, '@INPUT@'],
  capture : true,
)

amdgfxregs_h = custom_target(
  'amdgfxregs_h',
  input : ['../registers/makeregheader.py'] + amd_json_files,
  output : 'amdgfxregs.h',
  command : [prog_python, '@INPUT@', '--sort', 'address', '--guard', 'AMDGFXREGS_H'],
  capture : true,
)

gfx10_format_table_c = custom_target(
  'gfx10_format_table.c',
  input : files(
    'gfx10_format_table.py',
    '../../util/format/u_format.csv', '../registers/gfx10-rsrc.json', '../registers/gfx11-rsrc.json'
  ),
  output : 'gfx10_format_table.c',
  command : [prog_python, '@INPUT@'],
  capture : true,
  depend_files : ['../registers/regdb.py']
)

amd_common_files = files(
  'ac_binary.c',
  'ac_binary.h',
  'ac_shader_args.c',
  'ac_shader_args.h',
  'ac_shader_util.c',
  'ac_shader_util.h',
  'ac_gather_context_rolls.c',
  'ac_gpu_info.c',
  'ac_gpu_info.h',
  'ac_surface.c',
  'ac_surface.h',
  'ac_debug.c',
  'ac_debug.h',
  'ac_shadowed_regs.c',
  'ac_shadowed_regs.h',
  'ac_spm.c',
  'ac_spm.h',
  'ac_sqtt.c',
  'ac_sqtt.h',
  'ac_rgp.c',
  'ac_rgp.h',
  'ac_msgpack.c',
  'ac_msgpack.h',
  'ac_nir.c',
  'ac_nir.h',
  'ac_nir_opt_outputs.c',
  'ac_nir_cull.c',
  'ac_nir_lower_esgs_io_to_mem.c',
  'ac_nir_lower_global_access.c',
  'ac_nir_lower_image_opcodes_cdna.c',
  'ac_nir_lower_resinfo.c',
  'ac_nir_lower_subdword_loads.c',
  'ac_nir_lower_taskmesh_io_to_mem.c',
  'ac_nir_lower_tess_io_to_mem.c',
  'ac_nir_lower_tex.c',
  'ac_nir_lower_ngg.c',
  'ac_nir_lower_ps.c',
  'amd_family.c',
  'ac_parse_ib.c',
  'ac_perfcounter.c',
  'ac_perfcounter.h',
  'ac_vcn_av1_default.h',
)

if dep_elf.found()
  amd_common_files += files(
    'ac_rtld.c',
    'ac_rtld.h',
    'ac_rgp_elf_object_pack.c',
  )
endif

libamd_common = static_library(
  'amd_common',
  [amd_common_files, sid_tables_h, amdgfxregs_h, gfx10_format_table_c],
  include_directories : [
    inc_include, inc_src, inc_amd,
  ],
  dependencies : [
    dep_thread, dep_elf, dep_libdrm_amdgpu, dep_valgrind,
    idep_mesautil, idep_nir_headers, idep_nir
  ],
  gnu_symbol_visibility : 'hidden',
  c_args : ['-DADDR_FASTCALL=']
)

idep_amdgfxregs_h = declare_dependency(sources : [amdgfxregs_h])

executable(
  'ac_ib_parser',
  ['ac_ib_parser.c'],
  link_with: [libamd_common],
  include_directories : [
    inc_amd, inc_include, inc_src,
  ],
  dependencies : [idep_amdgfxregs_h, idep_mesautil, idep_nir_headers],
)

if with_tests and not with_platform_windows
  test(
    'ac_surface_modifier_test',
    executable(
      'ac_surface_modifier_test',
      ['ac_surface_modifier_test.c'],
      link_with: [libamd_common, libamdgpu_addrlib],
      include_directories : [
        inc_amd, inc_include, inc_src,
      ],
      dependencies: [idep_amdgfxregs_h, dep_libdrm_amdgpu, idep_mesautil],
    ),
    suite: ['amd']
  )

  # Limit this to only a few architectures for the Gitlab CI.
  if ['x86', 'x86_64', 'aarch64'].contains(host_machine.cpu_family())
    test(
      'ac_surface_meta_address_test',
      executable(
        'ac_surface_meta_address_test',
        ['ac_surface_meta_address_test.c'],
        link_with: [libamd_common, libamdgpu_addrlib],
        include_directories : [
          inc_amd, inc_include, inc_src,
        ],
        dependencies: [idep_amdgfxregs_h, dep_libdrm_amdgpu, idep_mesautil, dep_openmp],
      ),
      suite: ['amd']
    )
  endif
endif
