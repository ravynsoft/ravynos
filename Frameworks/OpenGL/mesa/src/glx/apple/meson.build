# Copyright © 2017 Jon Turney

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

# library for native GL on macos
files_libappleglx = files(
  'apple_cgl.c',
  'apple_cgl.h',
  'appledri.c',
  'appledri.h',
  'appledristr.h',
  'apple_glapi.c',
  'apple_glx.c',
  'apple_glx_context.c',
  'apple_glx_context.h',
  'apple_glx_drawable.c',
  'apple_glx_drawable.h',
  'apple_glx.h',
  'apple_glx_log.c',
  'apple_glx_log.h',
  'apple_glx_pbuffer.c',
  'apple_glx_pixmap.c',
  'apple_glx_surface.c',
  'apple_visual.c',
  'apple_visual.h',
  'apple_xgl_api.h',
  'apple_xgl_api_read.c',
  'apple_xgl_api_stereo.c',
  'apple_xgl_api_viewport.c',
  'glx_empty.c',
)

dep_xplugin = null_dep
if with_dri_platform == 'apple'
  dep_xplugin = meson.get_compiler('c').find_library('Xplugin')
endif

libappleglx = static_library(
  'glxapple',
  [files_libappleglx, glapitable_h],
  include_directories: [inc_mesa, inc_glx, inc_src, inc_include, inc_glapi, inc_loader],
  dependencies: [dep_xext, dep_xplugin],
  gnu_symbol_visibility : 'hidden',
  build_by_default: false,
)
