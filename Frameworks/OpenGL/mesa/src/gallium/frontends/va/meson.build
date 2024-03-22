# Copyright © 2017-2019 Intel Corporation

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

libva_version = dep_va.version().split('.')

libva_st = static_library(
  'va_st',
  files(
    'buffer.c', 'config.c', 'context.c', 'display.c', 'image.c', 'picture.c',
    'picture_mpeg12.c', 'picture_mpeg4.c', 'picture_h264.c', 'picture_hevc.c',
    'picture_vc1.c', 'picture_mjpeg.c', 'picture_vp9.c','picture_av1.c','postproc.c',
    'subpicture.c', 'surface.c', 'picture_h264_enc.c', 'picture_hevc_enc.c',
    'picture_av1_enc.c',
  ),
  c_args : [
    '-DVA_DRIVER_INIT_FUNC=__vaDriverInit_@0@_@1@'.format(
      libva_version[0], libva_version[1]
    ),
  ],
  gnu_symbol_visibility : 'hidden',
  include_directories : [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux],
  dependencies : [
    dep_va_headers, dep_x11_xcb, dep_xcb, dep_xcb_dri2, dep_xcb_dri3,
    dep_libdrm, idep_mesautil,
  ],
)
