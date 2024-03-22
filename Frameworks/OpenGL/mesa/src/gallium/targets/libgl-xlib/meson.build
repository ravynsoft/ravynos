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

# TODO: support non-static targets
# Static targets are always enabled in autotools (unless you modify
# configure.ac)

gallium_xlib_c_args = [
  '-DGALLIUM_SOFTPIPE',
]
gallium_xlib_ld_args = []
gallium_xlib_link_with = []
gallium_xlib_link_depends = []

if with_ld_version_script
  gallium_xlib_ld_args += ['-Wl,--version-script', join_paths(meson.current_source_dir(), 'libgl-xlib.sym')]
  gallium_xlib_link_depends += files('libgl-xlib.sym')
endif
if with_shared_glapi
  gallium_xlib_link_with += libglapi
endif

libgl = shared_library(
  'GL',
  files('xlib.c'),
  include_directories : [
    inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_gallium_winsys, inc_gallium_drivers,
    include_directories('../../frontends/glx/xlib'),
  ],
  c_args : [gallium_xlib_c_args],
  gnu_symbol_visibility : 'hidden',
  link_args : [ld_args_bsymbolic, ld_args_gc_sections, gallium_xlib_ld_args],
  link_depends : gallium_xlib_link_depends,
  link_whole : [libxlib, libglapi_static],
  link_with : [
    libgalliumvl_stub, libws_xlib,
    libgallium, libmesa, gallium_xlib_link_with,
  ],
  dependencies : [dep_x11, dep_thread, dep_clock, dep_unwind, driver_swrast, driver_virgl, driver_asahi],
  install : true,
  version : '1.5.0',
  darwin_versions: '4.0.0',
)
