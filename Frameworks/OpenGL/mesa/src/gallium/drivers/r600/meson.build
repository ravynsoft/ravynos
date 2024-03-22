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

files_r600 = files(
  'r600d_common.h',
  'compute_memory_pool.c',
  'compute_memory_pool.h',
  'eg_asm.c',
  'eg_debug.c',
  'eg_sq.h',
  'evergreen_compute.c',
  'evergreen_compute.h',
  'evergreen_compute_internal.h',
  'evergreend.h',
  'evergreen_hw_context.c',
  'evergreen_state.c',
  'r600_asm.c',
  'r600_asm.h',
  'r600_blit.c',
  'r600d.h',
  'r600_dump.c',
  'r600_dump.h',
  'r600_formats.h',
  'r600_hw_context.c',
  'r600_isa.c',
  'r600_isa.h',
  'r600_opcodes.h',
  'r600_pipe.c',
  'r600_pipe.h',
  'r600_public.h',
  'r600_sfn.cpp',
  'r600_sfn.h',
  'r600_shader.c',
  'r600_shader.h',
  'r600_shader_common.h',
  'r600_sq.h',
  'r600_state.c',
  'r600_state_common.c',
  'r600_uvd.c',
  'r700_asm.c',
  'r700_sq.h',
  'cayman_msaa.c',
  'r600_buffer_common.c',
  'r600_cs.h',
  'r600_gpu_load.c',
  'r600_perfcounter.c',
  'r600_pipe_common.c',
  'r600_pipe_common.h',
  'r600_query.c',
  'r600_query.h',
  'r600_streamout.c',
  'r600_test_dma.c',
  'r600_texture.c',
  'r600_viewport.c',
  'radeon_uvd.c',
  'radeon_uvd.h',
  'radeon_vce.c',
  'radeon_vce.h',
  'radeon_video.c',
  'radeon_video.h',
  'sfn/sfn_alu_defines.cpp',
  'sfn/sfn_alu_defines.h',
  'sfn/sfn_alu_readport_validation.cpp',
  'sfn/sfn_alu_readport_validation.h',
  'sfn/sfn_assembler.cpp',
  'sfn/sfn_assembler.h',
  'sfn/sfn_callstack.cpp',
  'sfn/sfn_callstack.h',
  'sfn/sfn_conditionaljumptracker.cpp',
  'sfn/sfn_conditionaljumptracker.h',
  'sfn/sfn_defines.h',
  'sfn/sfn_debug.cpp',
  'sfn/sfn_debug.h',
  'sfn/sfn_instr.cpp',
  'sfn/sfn_instr.h',
  'sfn/sfn_instr_alu.cpp',
  'sfn/sfn_instr_alu.h',
  'sfn/sfn_instr_alugroup.cpp',
  'sfn/sfn_instr_alugroup.h',
  'sfn/sfn_instr_controlflow.cpp',
  'sfn/sfn_instr_controlflow.h',
  'sfn/sfn_instr_export.cpp',
  'sfn/sfn_instr_export.h',
  'sfn/sfn_instr_fetch.cpp',
  'sfn/sfn_instr_fetch.h',
  'sfn/sfn_instr_mem.cpp',
  'sfn/sfn_instr_mem.h',
  'sfn/sfn_instr_lds.cpp',
  'sfn/sfn_instr_lds.h',
  'sfn/sfn_instr_tex.cpp',
  'sfn/sfn_instr_tex.h',
  'sfn/sfn_instrfactory.cpp',
  'sfn/sfn_instrfactory.h',
  'sfn/sfn_liverangeevaluator.cpp',
  'sfn/sfn_liverangeevaluator.h',
  'sfn/sfn_liverangeevaluator_helpers.cpp',
  'sfn/sfn_liverangeevaluator_helpers.h',
  'sfn/sfn_memorypool.cpp',
  'sfn/sfn_memorypool.h',
  'sfn/sfn_nir.cpp',
  'sfn/sfn_nir.h',
  'sfn/sfn_nir_legalize_image_load_store.cpp',
  'sfn/sfn_nir_lower_64bit.cpp',
  'sfn/sfn_nir_lower_alu.cpp',
  'sfn/sfn_nir_lower_alu.h',
  'sfn/sfn_nir_lower_tex.cpp',
  'sfn/sfn_nir_lower_tex.h',
  'sfn/sfn_nir_lower_fs_out_to_vector.cpp',
  'sfn/sfn_nir_lower_fs_out_to_vector.h',
  'sfn/sfn_nir_lower_tess_io.cpp',
  'sfn/sfn_nir_vectorize_vs_inputs.c',
  'sfn/sfn_optimizer.cpp',
  'sfn/sfn_peephole.cpp',
  'sfn/sfn_ra.cpp',
  'sfn/sfn_ra.h',
  'sfn/sfn_scheduler.cpp',
  'sfn/sfn_scheduler.h',
  'sfn/sfn_shader.cpp',
  'sfn/sfn_shader.h',
  'sfn/sfn_shader_cs.cpp',
  'sfn/sfn_shader_cs.h',
  'sfn/sfn_shader_fs.cpp',
  'sfn/sfn_shader_fs.h',
  'sfn/sfn_shader_gs.cpp',
  'sfn/sfn_shader_gs.h',
  'sfn/sfn_shader_tess.cpp',
  'sfn/sfn_shader_tess.h',
  'sfn/sfn_shader_vs.cpp',
  'sfn/sfn_shader_vs.h',
  'sfn/sfn_split_address_loads.cpp',
  'sfn/sfn_split_address_loads.h',
  'sfn/sfn_valuefactory.cpp',
  'sfn/sfn_valuefactory.h',
  'sfn/sfn_virtualvalues.cpp',
  'sfn/sfn_virtualvalues.h',
  )

egd_tables_h = custom_target(
  'egd_tables.h',
  input : ['egd_tables.py', 'evergreend.h'],
  output : 'egd_tables.h',
  command : [prog_python, '@INPUT@'],
  capture : true,
)

r600_c_args = []
if with_gallium_opencl
  if dep_elf.found()
    r600_c_args += '-DHAVE_OPENCL'
  else
    warning('r600 requires libelf to support opencl.')
  endif
endif

r600_cpp_args = []
if cpp.has_type('std::pmr::monotonic_buffer_resource',
                 prefix : '#include <memory_resource>')
   r600_cpp_args += '-DHAVE_MEMORY_RESOURCE'
endif

libr600 = static_library(
  'r600',
  [files_r600, egd_tables_h, sha1_h],
  c_args : [r600_c_args, '-Wstrict-overflow=0'],
  cpp_args: r600_cpp_args,
  gnu_symbol_visibility : 'hidden',
  include_directories : [
    inc_src, inc_mapi, inc_mesa, inc_include, inc_gallium, inc_gallium_aux, inc_amd_common,
    inc_gallium_drivers,
  ],
  link_with : [ libgalliumvl ],
  dependencies: [dep_libdrm_radeon, dep_elf, dep_llvm, idep_nir, idep_nir_headers],
)

driver_r600 = declare_dependency(
  compile_args : '-DGALLIUM_R600',
  link_with : [libr600, libmesa, libradeonwinsys],
)

if with_tests
  subdir('sfn/tests')
endif


