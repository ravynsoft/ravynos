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

xa_version = ['2', '5', '0']

xa_conf = configuration_data()
xa_conf.set('XA_MAJOR', xa_version[0])
xa_conf.set('XA_MINOR', xa_version[1])
xa_conf.set('XA_PATCH', xa_version[2])

xa_tracker_h = configure_file(
  configuration : xa_conf,
  input : 'xa_tracker.h.in',
  output : 'xa_tracker.h',
  install_dir : get_option('includedir'),
)

libxa_st = static_library(
  'xa_st',
  [xa_tracker_h, files(
    'xa_composite.c', 'xa_context.c', 'xa_renderer.c', 'xa_tgsi.c',
    'xa_tracker.c', 'xa_yuv.c',
  )],
  c_args : ['-pedantic'],
  gnu_symbol_visibility : 'hidden',
  dependencies : idep_mesautil,
  include_directories : [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux],
)

install_headers('xa_composite.h', 'xa_context.h')
