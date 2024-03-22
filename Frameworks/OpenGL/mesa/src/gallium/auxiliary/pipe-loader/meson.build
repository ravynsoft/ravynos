# Copyright © 2017 Dylan Baker

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

files_pipe_loader = files(
  'pipe_loader.c',
  'pipe_loader.h',
  'pipe_loader_priv.h',
  'pipe_loader_sw.c',
  'driinfo_gallium.h',
)

libpipe_loader_defines = []
libpipe_loader_links = []

if dep_libdrm.found()
  files_pipe_loader += files('pipe_loader_drm.c')
  libpipe_loader_links += libloader
endif

renderonly_drivers_c_args = []
if with_gallium_etnaviv
  renderonly_drivers_c_args += '-DGALLIUM_ETNAVIV'
endif
if with_gallium_lima
  renderonly_drivers_c_args += '-DGALLIUM_LIMA'
endif
if with_gallium_v3d
  renderonly_drivers_c_args += '-DGALLIUM_V3D'
endif
if with_gallium_vc4
  renderonly_drivers_c_args += '-DGALLIUM_VC4'
endif
if with_gallium_freedreno
  renderonly_drivers_c_args += '-DGALLIUM_FREEDRENO'
endif
if with_gallium_panfrost
  renderonly_drivers_c_args += '-DGALLIUM_PANFROST'
endif
if with_gallium_asahi
  renderonly_drivers_c_args += '-DGALLIUM_ASAHI'
endif

libpipe_loader_static = static_library(
  'pipe_loader_static',
  files_pipe_loader,
  include_directories : [
    inc_util, inc_loader, inc_gallium, inc_include, inc_src, inc_gallium_aux,
    inc_gallium_winsys, inc_gallium_drivers,
  ],
  c_args : [libpipe_loader_defines, '-DGALLIUM_STATIC_TARGETS=1', renderonly_drivers_c_args],
  gnu_symbol_visibility : 'hidden',
  link_with : [libpipe_loader_links],
  dependencies : [dep_libdrm, idep_xmlconfig, idep_mesautil],
  build_by_default : false,
)

libpipe_loader_dynamic = static_library(
  'pipe_loader_dynamic',
  files_pipe_loader,
  include_directories : [
    inc_util, inc_loader, inc_gallium, inc_include, inc_src, inc_gallium_aux,
    inc_gallium_winsys, inc_gallium_drivers,
  ],
  c_args : [
    libpipe_loader_defines,
    '-DPIPE_SEARCH_DIR="@0@"'.format(
      join_paths(get_option('prefix'), get_option('libdir'), 'gallium-pipe')
    )
  ],
  gnu_symbol_visibility : 'hidden',
  link_with : [libpipe_loader_links],
  dependencies : [dep_libdrm, idep_xmlconfig, idep_mesautil],
  build_by_default : false,
)
