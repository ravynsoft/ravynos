# Copyright © 2017 Dylan Baker
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

sym_config = configuration_data()

foreach d : [[with_gallium_r300 or with_gallium_radeonsi or with_gallium_r600, 'radeon_drm_winsys_create'],
             [with_gallium_radeonsi, 'amdgpu_winsys_create'],
             [with_gallium_nouveau, 'nouveau_drm_screen_create'],
             [with_gallium_freedreno, 'fd_drm_screen_create_renderonly'],
             [with_llvm and with_gallium_radeonsi, 'ac_init_shared_llvm_once']]
  if d[0]
    sym_config.set(d[1], d[1] + ';')
  else
    sym_config.set(d[1], '')
  endif
endforeach

inc_gallium_drivers = include_directories('drivers')
inc_gallium_winsys = include_directories('winsys')
inc_gallium_winsys_sw = include_directories('winsys/sw')

subdir('auxiliary')
subdir('auxiliary/pipe-loader')
subdir('winsys/sw/null')
if with_dri
  subdir('winsys/sw/dri')
else
  libswdri = []
endif
if with_gallium_drisw_kms
  subdir('winsys/sw/kms-dri')
else
  libswkmsdri = []
endif
if with_platform_windows
  subdir('winsys/sw/gdi')
else
  libwsgdi = null_dep
endif
subdir('winsys/sw/wrapper')
if with_platform_haiku
  subdir('winsys/sw/hgl')
endif
if with_gallium_softpipe
  subdir('drivers/softpipe')
  if draw_with_llvm
    subdir('drivers/llvmpipe')
  endif
else
  driver_swrast = declare_dependency()
endif
if with_gallium_asahi
  subdir('winsys/asahi/drm')
  subdir('drivers/asahi')
else
  driver_asahi = declare_dependency()
endif
if with_gallium_r300 or with_gallium_radeonsi or with_gallium_r600
  subdir('winsys/radeon/drm')
endif
if with_gallium_r300
  subdir('drivers/r300')
else
  driver_r300 = declare_dependency()
endif
if with_gallium_r600
  subdir('drivers/r600')
else
  driver_r600 = declare_dependency()
endif
if with_gallium_radeonsi
  subdir('winsys/amdgpu/drm')
  subdir('drivers/radeonsi')
else
  driver_radeonsi = declare_dependency()
endif
if with_gallium_nouveau
  subdir('winsys/nouveau/drm')
  subdir('drivers/nouveau')
else
  driver_nouveau = declare_dependency()
endif
if with_gallium_freedreno
  subdir('winsys/freedreno/drm')
  subdir('drivers/freedreno')
else
  driver_freedreno = declare_dependency()
endif
if with_gallium_vc4
  subdir('winsys/vc4/drm')
  subdir('drivers/vc4')
else
  driver_vc4 = declare_dependency()
endif
if with_gallium_panfrost
  subdir('winsys/panfrost/drm')
  subdir('drivers/panfrost')
else
  driver_panfrost = declare_dependency()
endif
if with_gallium_etnaviv
  subdir('winsys/etnaviv/drm')
  subdir('drivers/etnaviv')
else
  driver_etnaviv = declare_dependency()
endif
if with_gallium_kmsro
  subdir('winsys/kmsro/drm')
else
  driver_kmsro = declare_dependency()
endif
if with_gallium_v3d
  subdir('winsys/v3d/drm')
  subdir('drivers/v3d')
else
  driver_v3d = declare_dependency()
endif
if with_gallium_tegra
  subdir('winsys/tegra/drm')
  subdir('drivers/tegra')
else
  driver_tegra = declare_dependency()
endif
if with_gallium_crocus
  subdir('winsys/crocus/drm')
  subdir('drivers/crocus')
else
  driver_crocus = declare_dependency()
endif
if with_gallium_iris
  subdir('winsys/iris/drm')
  subdir('drivers/iris')
else
  driver_iris = declare_dependency()
endif
if with_gallium_i915
  subdir('winsys/i915/drm')
  subdir('drivers/i915')
else
  driver_i915 = declare_dependency()
endif
if with_gallium_svga
  if not with_platform_windows
     subdir('winsys/svga/drm')
  endif
  subdir('drivers/svga')
else
  driver_svga = declare_dependency()
endif
if with_gallium_virgl
  subdir('winsys/virgl/common')
  subdir('winsys/virgl/drm')
  subdir('winsys/virgl/vtest')
  subdir('drivers/virgl')
else
  driver_virgl = declare_dependency()
endif
if with_gallium_lima
  subdir('winsys/lima/drm')
  subdir('drivers/lima')
else
  driver_lima = declare_dependency()
endif
if with_gallium_zink
  subdir('drivers/zink')
else
  driver_zink = declare_dependency()
endif
if with_gallium_d3d12
  subdir('drivers/d3d12')
else
  driver_d3d12 = declare_dependency()
endif
if with_gallium_opencl
  # TODO: this isn't really clover specific, but ATM clover is the only
  # consumer
  subdir('targets/pipe-loader')

  subdir('frontends/clover')
  subdir('targets/opencl')
endif
if with_gallium_rusticl
  subdir('frontends/rusticl')
  subdir('targets/rusticl')
endif
if with_dri
  subdir('frontends/dri')
  subdir('targets/dri')
endif
if with_osmesa
  subdir('frontends/osmesa')
  subdir('targets/osmesa')
endif
if with_glx == 'xlib'
  subdir('winsys/sw/xlib')
  subdir('frontends/glx/xlib')
  subdir('targets/libgl-xlib')
endif
if with_gallium_vdpau
  subdir('frontends/vdpau')
  subdir('targets/vdpau')
endif
if with_gallium_omx != 'disabled'
  subdir('frontends/omx')
  subdir('targets/omx')
endif
if with_gallium_va
  subdir('frontends/va')
  subdir('targets/va')
endif
if with_gallium_xa
  subdir('frontends/xa')
  subdir('targets/xa')
endif
if with_platform_haiku
  subdir('frontends/hgl')
endif
if with_gallium_st_nine
  subdir('frontends/nine')
  subdir('targets/d3dadapter9')
endif
if with_gallium_st_d3d10umd
  subdir('frontends/d3d10umd')
  subdir('targets/d3d10sw')
endif
if with_platform_windows
  if with_opengl
    subdir('frontends/wgl')
    if with_gallium_d3d12
      subdir('winsys/d3d12/wgl')
    else
      winsys_d3d12_wgl = declare_dependency()
    endif
    subdir('targets/wgl')
    subdir('targets/libgl-gdi')
  endif
endif
if with_tests
  subdir('tests')
endif
if with_swrast_vk
  subdir('frontends/lavapipe')
  subdir('targets/lavapipe')
endif
