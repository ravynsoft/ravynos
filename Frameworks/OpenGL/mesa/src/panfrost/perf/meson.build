# Copyright © 2021 Collabora, Ltd.
# Author: Antonio Caggiano <antonio.caggiano@collabora.com>

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
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

pan_hw_metrics = [
  'G31', 'G51', 'G52', 'G57', 'G68', 'G71', 'G72', 'G76', 'G77',
  'G78', 'T72x', 'T76x', 'T82x', 'T83x', 'T86x', 'T88x',
]

pan_hw_metrics_xml_files = []
foreach hw : pan_hw_metrics
  pan_hw_metrics_xml_files += '@0@.xml'.format(hw)
endforeach

pan_perf_sources = [
  'pan_perf.c',
]

pan_perf_sources += custom_target(
  'pan-perf-sources',
  input : pan_hw_metrics_xml_files,
  output : [ 'pan_perf_metrics.c', 'pan_perf_metrics.h' ],
  command : [
    prog_python, files('pan_gen_perf.py'),
    '--code', '@OUTPUT0@', '--header', '@OUTPUT1@',
    '@INPUT@',
  ],
)

libpanfrost_perf = static_library(
  'panfrost_perf',
  pan_perf_sources,
  include_directories : [inc_include, inc_src, inc_panfrost],
  dependencies: libpanfrost_dep
)

dep_panfrost_perf = declare_dependency(
  link_with: libpanfrost_perf,
  include_directories: [inc_panfrost, inc_src, inc_include]
)

panfrost_quick = executable(
  'panquick',
  'quick.c',
  include_directories : [
    inc_mapi,
    inc_mesa,
    inc_gallium,
    inc_gallium_aux,
    inc_include,
    inc_src,
    inc_panfrost,
    inc_panfrost_hw,
 ],
  dependencies : [
    dep_libdrm,
    libpanfrost_dep,
    dep_panfrost_perf,
  ],
  build_by_default : with_tools.contains('panfrost')
)
