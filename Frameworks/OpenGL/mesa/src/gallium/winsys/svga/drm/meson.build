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

files_svgadrm = files(
  'pb_buffer_simple_fenced.c',
  'vmw_buffer.c',
  'vmw_context.c',
  'vmw_fence.c',
  'vmw_msg.c',
  'vmw_screen.c',
  'vmw_screen_dri.c',
  'vmw_screen_ioctl.c',
  'vmw_screen_pools.c',
  'vmw_screen_svga.c',
  'vmw_surface.c',
  'vmw_shader.c',
  'vmw_query.c',
)

libsvgadrm = static_library(
  'svgadrm',
  files_svgadrm,
  c_args : [c_msvc_compat_args],
  gnu_symbol_visibility : 'hidden',
  include_directories : [
    inc_src, inc_include, inc_gallium, inc_gallium_aux,
    include_directories('../../../drivers/svga', '../../../drivers/svga/include'),
  ],
  dependencies : [dep_libdrm, idep_mesautil],
)
