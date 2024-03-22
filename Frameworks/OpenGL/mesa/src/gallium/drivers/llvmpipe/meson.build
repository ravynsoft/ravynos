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

files_llvmpipe = files(
  'lp_bld_alpha.c',
  'lp_bld_alpha.h',
  'lp_bld_blend_aos.c',
  'lp_bld_blend.c',
  'lp_bld_blend.h',
  'lp_bld_blend_logicop.c',
  'lp_bld_depth.c',
  'lp_bld_depth.h',
  'lp_bld_interp.c',
  'lp_bld_interp.h',
  'lp_clear.c',
  'lp_clear.h',
  'lp_context.c',
  'lp_context.h',
  'lp_cs_tpool.h',
  'lp_cs_tpool.c',
  'lp_debug.h',
  'lp_draw_arrays.c',
  'lp_fence.c',
  'lp_fence.h',
  'lp_flush.c',
  'lp_flush.h',
  'lp_jit.c',
  'lp_jit.h',
  'lp_limits.h',
  'lp_linear.c',
  'lp_linear_fastpath.c',
  'lp_linear_interp.c',
  'lp_linear_sampler.c',
  'lp_linear_sampler_tmp.h',
  'lp_memory.c',
  'lp_memory.h',
  'lp_perf.c',
  'lp_perf.h',
  'lp_public.h',
  'lp_query.c',
  'lp_query.h',
  'lp_rast.c',
  'lp_rast_debug.c',
  'lp_rast.h',
  'lp_rast_linear.c',
  'lp_rast_linear_fallback.c',
  'lp_rast_priv.h',
  'lp_rast_rect.c',
  'lp_rast_tri.c',
  'lp_rast_tri_tmp.h',
  'lp_scene.c',
  'lp_scene.h',
  'lp_scene_queue.c',
  'lp_scene_queue.h',
  'lp_screen.c',
  'lp_screen.h',
  'lp_setup.c',
  'lp_setup_analysis.c',
  'lp_setup_context.h',
  'lp_setup.h',
  'lp_setup_line.c',
  'lp_setup_point.c',
  'lp_setup_rect.c',
  'lp_setup_tri.c',
  'lp_setup_vbuf.c',
  'lp_state_blend.c',
  'lp_state_clip.c',
  'lp_state_derived.c',
  'lp_state_cs.c',
  'lp_state_cs.h',
  'lp_state_fs.c',
  'lp_state_fs.h',
  'lp_state_fs_analysis.c',
  'lp_state_fs_fastpath.c',
  'lp_state_fs_linear.c',
  'lp_state_fs_linear_llvm.c',
  'lp_state_gs.c',
  'lp_state.h',
  'lp_state_rasterizer.c',
  'lp_state_sampler.c',
  'lp_state_setup.c',
  'lp_state_setup.h',
  'lp_state_so.c',
  'lp_state_surface.c',
  'lp_state_tess.c',
  'lp_state_vertex.c',
  'lp_state_vs.c',
  'lp_surface.c',
  'lp_surface.h',
  'lp_tex_sample.c',
  'lp_tex_sample.h',
  'lp_texture.c',
  'lp_texture.h',
  'lp_texture_handle.c',
  'lp_texture_handle.h',
)

libllvmpipe = static_library(
  'llvmpipe',
  [files_llvmpipe, sha1_h],
  c_args : [c_msvc_compat_args],
  cpp_args : [cpp_msvc_compat_args],
  gnu_symbol_visibility : 'hidden',
  include_directories : [inc_gallium, inc_gallium_aux, inc_include, inc_src],
  dependencies : [ dep_llvm, idep_nir_headers, idep_mesautil ],
)

# This overwrites the softpipe driver dependency, but itself depends on the
# softpipe dependency.
driver_swrast = declare_dependency(
  compile_args : '-DGALLIUM_LLVMPIPE',
  link_with : libllvmpipe,
  dependencies : [driver_swrast, dep_llvm, idep_mesautil],
)

if with_tests and with_gallium_softpipe and draw_with_llvm
  foreach t : ['lp_test_format', 'lp_test_arit', 'lp_test_blend',
               'lp_test_conv', 'lp_test_printf']
    test(
      t,
      executable(
        t,
        ['@0@.c'.format(t), 'lp_test_main.c', sha1_h],
        dependencies : [dep_llvm, dep_dl, dep_clock, idep_mesautil],
        include_directories : [inc_gallium, inc_gallium_aux, inc_include, inc_src],
        link_with : [libllvmpipe, libgallium],
      ),
      suite : ['llvmpipe'],
      should_fail : meson.get_external_property('xfail', '').contains(t),
      timeout: 240,
    )
  endforeach
endif
