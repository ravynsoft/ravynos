# Copyright © 2017-2018 Intel Corporation

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

# TODO: support non-static targets
# Static targets are always enabled in autotools (unless you modify
# configure.ac)

xa_link_args = []
xa_link_depends = []

if with_ld_version_script
  xa_link_args += ['-Wl,--version-script', join_paths(meson.current_source_dir(), 'xa.sym')]
  xa_link_depends += files('xa.sym')
endif

_xa_version = '.'.join(xa_version)

libxatracker = shared_library(
  'xatracker',
  'target.c',
  gnu_symbol_visibility : 'hidden',
  link_args : [xa_link_args, ld_args_gc_sections],
  include_directories : [
    inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_util, inc_gallium_winsys, inc_gallium_drivers,
  ],
  link_whole : [libxa_st],
  link_with : [
    libgalliumvl_stub, libgallium, libpipe_loader_static,
    libws_null, libwsw, libswdri, libswkmsdri,
  ],
  link_depends : xa_link_depends,
  dependencies : [
    idep_mesautil,
    driver_nouveau, driver_i915, driver_svga, driver_freedreno,
  ],
  version : _xa_version,
  install : true,
)

pkg.generate(
  name : 'xatracker',
  description : 'Xorg gallium3D acceleration library',
  version : _xa_version,
  libraries : libxatracker,
)
