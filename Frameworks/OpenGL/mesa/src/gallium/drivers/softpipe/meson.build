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

files_softpipe = files(
  'sp_buffer.c',
  'sp_buffer.h',
  'sp_clear.c',
  'sp_clear.h',
  'sp_context.c',
  'sp_context.h',
  'sp_compute.c',
  'sp_draw_arrays.c',
  'sp_fence.c',
  'sp_fence.h',
  'sp_flush.c',
  'sp_flush.h',
  'sp_fs_exec.c',
  'sp_fs.h',
  'sp_image.c',
  'sp_image.h',
  'sp_limits.h',
  'sp_prim_vbuf.c',
  'sp_prim_vbuf.h',
  'sp_public.h',
  'sp_quad_blend.c',
  'sp_quad_depth_test.c',
  'sp_quad_depth_test_tmp.h',
  'sp_quad_fs.c',
  'sp_quad.h',
  'sp_quad_pipe.c',
  'sp_quad_pipe.h',
  'sp_query.c',
  'sp_query.h',
  'sp_screen.c',
  'sp_screen.h',
  'sp_setup.c',
  'sp_setup.h',
  'sp_state_blend.c',
  'sp_state_clip.c',
  'sp_state_derived.c',
  'sp_state_image.c',
  'sp_state.h',
  'sp_state_rasterizer.c',
  'sp_state_sampler.c',
  'sp_state_shader.c',
  'sp_state_so.c',
  'sp_state_surface.c',
  'sp_state_vertex.c',
  'sp_surface.c',
  'sp_surface.h',
  'sp_tex_sample.c',
  'sp_tex_sample.h',
  'sp_tex_tile_cache.c',
  'sp_tex_tile_cache.h',
  'sp_texture.c',
  'sp_texture.h',
  'sp_tile_cache.c',
  'sp_tile_cache.h',
)

libsoftpipe = static_library(
  'softpipe',
  files_softpipe,
  include_directories : [inc_gallium_aux, inc_gallium, inc_include, inc_src],
  c_args : [c_msvc_compat_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : idep_nir,
)

driver_swrast = declare_dependency(
  compile_args : '-DGALLIUM_SOFTPIPE',
  link_with : libsoftpipe
)
