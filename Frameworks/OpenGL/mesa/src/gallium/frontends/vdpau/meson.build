# Copyright © 2017, 2019 Intel Corproration

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

VDPAU_MAJOR = 1
VDPAU_MINOR = 0

libvdpau_st = static_library(
  'vdpau_st',
  files(
    'bitmap.c', 'decode.c', 'device.c', 'ftab.c', 'htab.c', 'mixer.c',
    'output.c', 'preemption.c', 'presentation.c', 'query.c', 'surface.c',
  ),
  c_args : [
    '-DVER_MAJOR=@0@'.format(VDPAU_MAJOR),
    '-DVER_MINOR=@0@'.format(VDPAU_MINOR),
  ],
  gnu_symbol_visibility : 'hidden',
  include_directories : [
    inc_include, inc_src, inc_util, inc_gallium, inc_gallium_aux,
  ],
  dependencies : [dep_vdpau, dep_xcb, dep_x11_xcb, dep_xcb_dri2, dep_libdrm, idep_mesautil],
)
