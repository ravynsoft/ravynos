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

files_svga = files(
  'svga_cmd.c',
  'svga_cmd_vgpu10.c',
  'svga_context.c',
  'svga_draw_arrays.c',
  'svga_draw.c',
  'svga_draw_elements.c',
  'svga_format.c',
  'svga_image_view.c',
  'svga_link.c',
  'svga_pipe_blend.c',
  'svga_pipe_blit.c',
  'svga_pipe_clear.c',
  'svga_pipe_constants.c',
  'svga_pipe_cs.c',
  'svga_pipe_depthstencil.c',
  'svga_pipe_draw.c',
  'svga_pipe_flush.c',
  'svga_pipe_fs.c',
  'svga_pipe_gs.c',
  'svga_pipe_ts.c',
  'svga_pipe_misc.c',
  'svga_pipe_query.c',
  'svga_pipe_rasterizer.c',
  'svga_pipe_sampler.c',
  'svga_pipe_streamout.c',
  'svga_pipe_vertex.c',
  'svga_pipe_vs.c',
  'svga_resource_buffer.c',
  'svga_resource_buffer_upload.c',
  'svga_resource.c',
  'svga_resource_texture.c',
  'svga_sampler_view.c',
  'svga_screen.c',
  'svga_screen_cache.c',
  'svga_shader.c',
  'svga_shader_buffer.c',
  'svga_state.c',
  'svga_state_constants.c',
  'svga_state_cs.c',
  'svga_state_framebuffer.c',
  'svga_state_fs.c',
  'svga_state_gs.c',
  'svga_state_ts.c',
  'svga_state_need_swtnl.c',
  'svga_state_rss.c',
  'svga_state_sampler.c',
  'svga_state_tgsi_transform.c',
  'svga_state_tss.c',
  'svga_state_uav.c',
  'svga_state_vdecl.c',
  'svga_state_vs.c',
  'svga_surface.c',
  'svga_swtnl_backend.c',
  'svga_swtnl_draw.c',
  'svga_swtnl_state.c',
  'svga_tgsi.c',
  'svga_tgsi_decl_sm30.c',
  'svga_tgsi_insn.c',
  'svga_tgsi_vgpu10.c',
  'svgadump/svga_dump.c',
  'svgadump/svga_shader_dump.c',
  'svgadump/svga_shader_op.c',
)

libsvga = static_library(
  'svga',
  [files_svga, sha1_h],
  c_args : [c_msvc_compat_args],
  gnu_symbol_visibility : 'hidden',
  include_directories : [
    inc_src, inc_include, inc_gallium, inc_gallium_aux,
    include_directories('include')
  ],
  link_with : [libglsl],
  dependencies : [idep_mesautil, idep_nir],
)

svga_deps = [libsvga]
if not with_platform_windows
  svga_deps += libsvgadrm
endif

driver_svga = declare_dependency(
  compile_args : '-DGALLIUM_VMWGFX',
  link_with : svga_deps,
)
