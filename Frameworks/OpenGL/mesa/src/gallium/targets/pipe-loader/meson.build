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

pipe_loader_link_args = [ld_args_gc_sections, ld_args_build_id]
pipe_loader_link_deps = []
pipe_loader_link_with = [libgallium]
pipe_loader_comp_args = []
pipe_loader_incs = [
  inc_include, inc_src, inc_util, inc_gallium, inc_gallium_drivers,
  inc_gallium_winsys, inc_gallium_aux,
]

if (with_gallium_va or with_gallium_vdpau or with_gallium_omx != 'disabled' or
    with_dri or with_gallium_radeonsi)
  pipe_loader_link_with += libgalliumvl
else
  pipe_loader_link_with += libgalliumvl_stub
endif
if (with_gallium_va or with_gallium_vdpau or with_gallium_omx != 'disabled')
  pipe_loader_link_with += libgalliumvlwinsys
endif

pipe_loader_install_dir = join_paths(get_option('libdir'), 'gallium-pipe')

_kmsro_targets = [
   driver_kmsro, driver_v3d, driver_vc4, driver_freedreno, driver_etnaviv,
   driver_panfrost, driver_lima, driver_asahi,
]

if with_gallium_v3d
   _kmsro_targets += [idep_xmlconfig, dep_expat]
endif

pipe_loaders = [
  [with_gallium_i915, 'i915', driver_i915, []],
  [with_gallium_crocus, 'crocus', [driver_crocus, idep_xmlconfig], []],
  [with_gallium_iris, 'iris', [driver_iris, idep_xmlconfig], []],
  [with_gallium_nouveau, 'nouveau', driver_nouveau, []],
  [with_gallium_r300, 'r300', driver_r300, []],
  [with_gallium_r600, 'r600', driver_r600, []],
  [with_gallium_radeonsi, 'radeonsi', [driver_radeonsi, idep_xmlconfig], []],
  [with_gallium_freedreno, 'msm', driver_freedreno, []],
  [with_gallium_kmsro, 'kmsro', _kmsro_targets, [libpipe_loader_dynamic]],
  [with_gallium_svga, 'vmwgfx', driver_svga, []],
  [with_gallium_softpipe, 'swrast', driver_swrast, [libwsw, libws_null, libswdri, libswkmsdri]],
]

foreach x : pipe_loaders
  if not x[0]
    continue
  endif

  pipe_sym_config = configuration_data()

  foreach d : [[x[1] in ['r300', 'r600', 'radeonsi'], 'radeon_drm_winsys_create'],
               [x[1] == 'radeonsi', 'amdgpu_winsys_create'],
               [x[1] == 'radeonsi' and with_llvm, 'ac_init_shared_llvm_once'],
               [x[1] != 'swrast', 'driver_descriptor'],
               [x[1] == 'swrast', 'swrast_driver_descriptor']]
    if d[0]
      pipe_sym_config.set(d[1], d[1] + ';')
    else
      pipe_sym_config.set(d[1], '')
    endif
  endforeach

  pipe_sym = configure_file(input : 'pipe.sym.in', output : 'pipe_@0@.sym'.format(x[1]), configuration : pipe_sym_config)

  cur_pipe_loader_link_args = pipe_loader_link_args
  cur_pipe_loader_link_deps = pipe_loader_link_deps
  if with_ld_version_script
    cur_pipe_loader_link_args += [
      '-Wl,--version-script', join_paths(meson.current_build_dir(), 'pipe_@0@.sym'.format(x[1]))
    ]
    cur_pipe_loader_link_deps += pipe_sym
  endif

  shared_library(
    'pipe_@0@'.format(x[1]),
    'pipe_@0@.c'.format(x[1]),
    c_args : [pipe_loader_comp_args, '-DPIPE_LOADER_DYNAMIC=1'],
    cpp_args : [pipe_loader_comp_args],
    gnu_symbol_visibility : 'hidden',
    link_args : cur_pipe_loader_link_args,
    link_depends : cur_pipe_loader_link_deps,
    include_directories : pipe_loader_incs,
    link_with : [pipe_loader_link_with, x[3]],
    dependencies : [idep_mesautil, idep_nir, dep_thread, x[2]],
    name_prefix : '',
    install : true,
    install_dir : pipe_loader_install_dir,
  )
endforeach
