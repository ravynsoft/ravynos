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

si_tracepoints = custom_target(
  'si_tracepoints.[ch]',
  input: 'si_tracepoints.py',
  output: ['si_tracepoints.c', 'si_tracepoints_perfetto.h', 'si_tracepoints.h'],
  command: [
    prog_python, '@INPUT@',
    '-p', join_paths(dir_source_root, 'src/util/perf/'),
    '-C', '@OUTPUT0@',
    '--perfetto-hdr', '@OUTPUT1@',
    '-H', '@OUTPUT2@'
  ],
  depend_files: u_trace_py,
)

idep_si_tracepoints = declare_dependency(
  sources: si_tracepoints,
)

files_libradeonsi = files(
  'driinfo_radeonsi.h',
  'gfx10_shader_ngg.c',
  'gfx11_query.c',
  'si_blit.c',
  'si_buffer.c',
  'si_build_pm4.h',
  'si_clear.c',
  'si_compute.c',
  'si_compute_blit.c',
  'si_cp_dma.c',
  'si_cp_reg_shadowing.c',
  'si_debug.c',
  'si_descriptors.c',
  'si_fence.c',
  'si_get.c',
  'si_gfx_cs.c',
  'si_gpu_load.c',
  'si_perfcounter.c',
  'si_pipe.c',
  'si_pipe.h',
  'si_pm4.c',
  'si_pm4.h',
  'si_public.h',
  'si_query.c',
  'si_query.h',
  'si_nir_lower_abi.c',
  'si_nir_lower_resource.c',
  'si_nir_lower_vs_inputs.c',
  'si_nir_optim.c',
  'si_sdma_copy_image.c',
  'si_shader.c',
  'si_shader.h',
  'si_shader_aco.c',
  'si_shader_info.c',
  'si_shader_internal.h',
  'si_shader_nir.c',
  'si_shaderlib_nir.c',
  'si_sqtt.c',
  'si_state.c',
  'si_state.h',
  'si_state_binning.c',
  'si_state_msaa.c',
  'si_state_shaders.cpp',
  'si_state_streamout.c',
  'si_state_viewport.c',
  'si_test_image_copy_region.c',
  'si_test_dma_perf.c',
  'si_texture.c',
  'si_utrace.c',
  'si_utrace.h',
  'si_uvd.c',
  'si_vpe.c',
  'si_vpe.h',
  'pspdecryptionparam.h',
  'radeon_temporal.h',
  'radeon_uvd.c',
  'radeon_uvd.h',
  'radeon_uvd_enc.c',
  'radeon_uvd_enc.h',
  'radeon_uvd_enc_1_1.c',
  'radeon_vce.c',
  'radeon_vce.h',
  'radeon_vce_40_2_2.c',
  'radeon_vce_50.c',
  'radeon_vce_52.c',
  'radeon_vcn.h',
  'radeon_vcn.c',
  'radeon_vcn_dec.c',
  'radeon_vcn_dec.h',
  'radeon_vcn_dec_jpeg.c',
  'radeon_vcn_enc.c',
  'radeon_vcn_enc.h',
  'radeon_vcn_enc_1_2.c',
  'radeon_vcn_enc_2_0.c',
  'radeon_vcn_enc_3_0.c',
  'radeon_vcn_enc_4_0.c',
  'radeon_video.c',
  'radeon_video.h',
)

radeonsi_include_dirs = [inc_src, inc_include, inc_gallium, inc_gallium_aux, inc_amd_common,
                         inc_amd_common_llvm, inc_gallium_drivers, inc_compiler, inc_amd, inc_amd_vpe]
radeonsi_deps = [dep_llvm, dep_clock, dep_libdrm_radeon, idep_nir_headers, idep_amdgfxregs_h, idep_mesautil, idep_aco, idep_u_tracepoints, idep_si_tracepoints]

if with_perfetto
  radeonsi_deps += dep_perfetto
endif

files_libradeonsi += ['si_perfetto.cpp', 'si_perfetto.h']

amd_common_libs = [libamd_common]

if with_llvm
  files_libradeonsi += files(
    'si_shader_llvm.c',
    'si_shader_llvm.h',
    'si_shader_llvm_gs.c',
    'si_shader_llvm_ps.c',
    'si_shader_llvm_tess.c',
    'si_shader_llvm_vs.c',
  )

  radeonsi_include_dirs += [inc_amd_common_llvm]
  radeonsi_deps += [dep_llvm]
  amd_common_libs += [libamd_common_llvm]
endif

radeonsi_gfx_libs = []
foreach ver : ['6', '7', '8', '9', '10', '103', '11', '115']
  radeonsi_gfx_libs += static_library(
    'radeonsi_gfx@0@'.format(ver),
    ['si_state_draw.cpp'],
    include_directories : radeonsi_include_dirs,
    cpp_args : ['-DGFX_VER=@0@'.format(ver)],
    gnu_symbol_visibility : 'hidden',
    dependencies : radeonsi_deps,
  )
endforeach

libradeonsi = static_library(
  'radeonsi',
  [files_libradeonsi, sid_tables_h],
  include_directories : radeonsi_include_dirs,
  gnu_symbol_visibility : 'hidden',
  dependencies : radeonsi_deps,
)

driver_radeonsi = declare_dependency(
  compile_args : '-DGALLIUM_RADEONSI',
  link_with : radeonsi_gfx_libs + [
    libradeonsi, libradeonwinsys, libamdgpuwinsys, libamd_common, libamd_common_llvm, libgalliumvl, libvpe
  ],
  dependencies : idep_nir,
)
