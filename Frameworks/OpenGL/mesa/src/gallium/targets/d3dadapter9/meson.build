# Copyright © 2017 Dylan Baker
# Copyright © 2018 Intel Corporation

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

nine_version = ['1', '0', '0']

gallium_nine_c_args = []
gallium_nine_ld_args = []
gallium_nine_link_depends = []
gallium_nine_link_with = [
    libgallium, libnine_st,
    libpipe_loader_static, libws_null, libwsw, libswdri,
    libswkmsdri,
]

if with_ld_version_script
  gallium_nine_ld_args += ['-Wl,--version-script', join_paths(meson.current_source_dir(), 'd3dadapter9.sym')]
  gallium_nine_link_depends += files('d3dadapter9.sym')
endif

if (with_gallium_va or with_gallium_vdpau or with_gallium_omx != 'disabled' or
    with_dri)
  gallium_nine_link_with += libgalliumvl
else
  gallium_nine_link_with += libgalliumvl_stub
endif

libgallium_nine = shared_library(
  'd3dadapter9',
  files('description.c', 'getproc.c', 'drm.c'),
  include_directories : [
    inc_include, inc_src, inc_loader, inc_mapi, inc_mesa, inc_util,
    inc_gallium, inc_gallium_aux, inc_gallium_winsys,
    inc_gallium_drivers, inc_d3d9,
    include_directories('../../frontends/nine'),
  ],
  c_args : [gallium_nine_c_args],
  gnu_symbol_visibility : 'hidden',
  link_args : [ld_args_build_id, ld_args_gc_sections, gallium_nine_ld_args],
  link_depends : gallium_nine_link_depends,
  link_with : gallium_nine_link_with,
  dependencies : [
    dep_selinux, dep_libdrm, dep_llvm, dep_thread,
    idep_xmlconfig, idep_mesautil, idep_nir,
    driver_asahi,
    driver_v3d,
    driver_crocus,
    driver_etnaviv,
    driver_freedreno,
    driver_kmsro,
    driver_lima,
    driver_nouveau,
    driver_panfrost,
    driver_r300,
    driver_r600,
    driver_radeonsi,
    driver_svga,
    driver_swrast,
    driver_i915,
    driver_iris,
    driver_v3d,
    driver_vc4,
    driver_zink
  ],
  name_prefix : '',
  version : '.'.join(nine_version),
  install : true,
  install_dir : d3d_drivers_path,
)

pkg.generate(
  name : 'd3d',
  description : 'Native D3D driver modules',
  version : '.'.join(nine_version),
  requires_private : 'libdrm >= ' + dep_libdrm.version(),
  variables : ['moduledir=@0@'.format(d3d_drivers_path)],
)
