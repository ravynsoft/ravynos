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

vdpau_link_args = []
vdpau_link_depends = []
vdpau_drivers = []

vdpau_sym = configure_file(input : 'vdpau.sym.in', output : 'vdpau.sym', configuration : sym_config)

if with_ld_version_script
  vdpau_link_args += ['-Wl,--version-script', join_paths(meson.current_build_dir(), 'vdpau.sym')]
  vdpau_link_depends += vdpau_sym
endif
if with_ld_dynamic_list
  vdpau_link_args += ['-Wl,--dynamic-list', join_paths(meson.current_source_dir(), '../dri.dyn')]
  vdpau_link_depends += files('../dri.dyn')
endif

link_with_libvdpau_gallium = [
  libgalliumvlwinsys, libgalliumvl, libgallium,
  libpipe_loader_static, libws_null, libwsw, libswdri, libswkmsdri,
]

if with_glx == 'xlib'
  link_with_libvdpau_gallium += [libws_xlib]
endif

libvdpau_gallium = shared_library(
  'vdpau_gallium',
  'target.c',
  gnu_symbol_visibility : 'hidden',
  link_args : [vdpau_link_args, ld_args_gc_sections],
  include_directories : [
    inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux, inc_util, inc_gallium_winsys, inc_gallium_drivers,
  ],
  link_whole : [libvdpau_st],
  link_with : link_with_libvdpau_gallium,
  dependencies : [
    idep_mesautil,
    driver_r600, driver_radeonsi, driver_nouveau, driver_d3d12, driver_virgl,
  ],
  link_depends : vdpau_link_depends,
  soversion : '@0@.@1@.0'.format(VDPAU_MAJOR, VDPAU_MINOR),
  # Will be deleted during installation, see install_megadrivers.py
  install : true,
  install_dir : vdpau_drivers_path,
  name_suffix : 'so',
)
foreach d : [[with_gallium_r600, 'r600'],
             [with_gallium_radeonsi, 'radeonsi'],
             [with_gallium_nouveau, 'nouveau'],
             [with_gallium_virgl, 'virtio_gpu'],
             [with_gallium_d3d12_video, 'd3d12']]
  if d[0]
    vdpau_drivers += 'libvdpau_@0@.so.@1@.@2@.0'.format(d[1], VDPAU_MAJOR, VDPAU_MINOR)
  endif
endforeach

meson.add_install_script(
  install_megadrivers_py.full_path(),
  libvdpau_gallium.full_path(),
  vdpau_drivers_path,
  vdpau_drivers,
  install_tag : 'runtime',
)
