# Copyright © 2017, 2019 Intel Corporation

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

inc_st_omx = [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux]
dep_st_omx = [
  dep_omx, dep_x11_xcb, dep_xcb, dep_xcb_dri2, dep_xcb_dri3, dep_libdrm,
]

files_omx = files(
  'vid_dec_common.c',
  'vid_dec_h264_common.c',
  'vid_enc_common.c',
  'vid_omx_common.c'
)

if with_gallium_omx == 'bellagio'
  files_omx += files(
    'bellagio/entrypoint.c',
    'bellagio/vid_dec.c',
    'bellagio/vid_dec_mpeg12.c',
    'bellagio/vid_dec_h264.c',
    'bellagio/vid_dec_h265.c',
    'bellagio/vid_dec_av1.c',
    'bellagio/vid_enc.c'
  )
elif with_gallium_omx == 'tizonia'
  files_omx += files(
    'tizonia/entrypoint.c',
     'tizonia/h264d.c',
     'tizonia/h264dprc.c',
     'tizonia/h264dinport.c',
     'tizonia/h264e.c',
     'tizonia/h264eprc.c',
     'tizonia/h264einport.c',
     'tizonia/h264eoutport.c'
  )
  inc_st_omx = [
    inc_st_omx,
    inc_loader,
    inc_gbm,
    inc_st_dri,
    inc_egl,
    inc_egl_dri2,
  ]
  dep_st_omx = [dep_st_omx, dep_omx_other]
endif

libomx_st = static_library(
  'omx_st',
  files_omx,
  c_args : ['-fcommon'],
  gnu_symbol_visibility : 'hidden',
  include_directories : inc_st_omx,
  dependencies : [dep_st_omx, idep_mesautil],
)
