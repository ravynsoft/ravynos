# Copyright © 2018 Rob Clark

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

libfreedreno_perfcntrs_files = files(
  'fd2_perfcntr.c',
  'fd5_perfcntr.c',
  'fd6_perfcntr.c',
  'freedreno_dt.c',
  'freedreno_dt.h',
  'freedreno_perfcntr.c',
  'freedreno_perfcntr.h',
)

libfreedreno_perfcntrs = static_library(
  'freedreno_perfcntrs',
  [libfreedreno_perfcntrs_files, freedreno_xml_header_files],
  include_directories : [inc_freedreno, inc_include, inc_src],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  link_with : [libfreedreno_common],
  dependencies : idep_nir_headers,
  build_by_default : false,
)

dep_libconfig = dependency('libconfig', required : false)
dep_curses = dependency('curses', required : false)

if dep_libconfig.found() and dep_curses.found()
  fdperf = executable(
    'fdperf',
    ['fdperf.c', freedreno_xml_header_files],
    include_directories : [
      inc_freedreno,
      inc_include,
      inc_src,
    ],
    link_with : [
      libfreedreno_common,
      libfreedreno_drm,
      libfreedreno_perfcntrs,
    ],
    dependencies : [
      dep_libconfig,
      dep_libdrm,
      dep_curses,
      idep_mesautil,
    ],
    build_by_default : with_tools.contains('freedreno'),
    install : with_tools.contains('freedreno'),
  )
endif
