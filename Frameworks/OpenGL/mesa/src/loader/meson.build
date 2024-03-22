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

inc_loader = include_directories('.')

if with_platform_x11 and with_dri3
  libloader_dri3_helper = static_library(
    'loader_dri3_helper',
    'loader_dri3_helper.c',
    gnu_symbol_visibility : 'hidden',
    include_directories : [inc_include, inc_src],
    dependencies : [
      idep_mesautil,
      dep_libdrm, dep_xcb_dri3, dep_xcb_present, dep_xcb_sync, dep_xshmfence,
      dep_xcb_xfixes,
    ],
    build_by_default : false,
  )
else
  libloader_dri3_helper = []
endif

loader_c_args = [
  '-DUSE_DRICONF',
  '-DDEFAULT_DRIVER_DIR="@0@"'.format(dri_search_path),
]
libloader = static_library(
  'loader',
  ['loader_dri_helper.c', 'loader.c', sha1_h],
  c_args : loader_c_args,
  gnu_symbol_visibility : 'hidden',
  include_directories : [inc_include, inc_src, inc_util],
  dependencies : [dep_libdrm, dep_thread, dep_xcb, dep_xcb_xrandr],
  build_by_default : false,
)
