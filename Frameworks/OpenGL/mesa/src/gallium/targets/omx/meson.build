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

omx_link_args = []
omx_link_depends = []

omx_sym = configure_file(input : 'omx.sym.in', output : 'omx.sym', configuration : sym_config)

if with_ld_version_script
  omx_link_args += ['-Wl,--version-script', join_paths(meson.current_build_dir(), 'omx.sym')]
  omx_link_depends += omx_sym
endif

libomx_gallium = shared_library(
  'omx_mesa',
  'target.c',
  gnu_symbol_visibility : 'hidden',
  link_args : [omx_link_args, ld_args_gc_sections],
  include_directories : [
    inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_util, inc_gallium_winsys, inc_gallium_drivers,
  ],
  link_whole : [libomx_st],
  link_with : [
    libgalliumvlwinsys, libgalliumvl, libgallium,
    libpipe_loader_static, libws_null, libwsw, libswdri, libswkmsdri,
  ],
  link_depends : omx_link_depends,
  dependencies : [idep_mesautil, driver_r600, driver_radeonsi, driver_nouveau],
  install : true,
  install_dir : omx_drivers_path,
)
