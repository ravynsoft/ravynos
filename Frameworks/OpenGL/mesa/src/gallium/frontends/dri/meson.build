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

inc_st_dri = include_directories('.')
deps_for_libdri = []

files_libdri = files(
  'dri_context.c',
  'dri_context.h',
  'dri_drawable.c',
  'dri_drawable.h',
  'dri_helpers.c',
  'dri_helpers.h',
  'dri_query_renderer.c',
  'dri_query_renderer.h',
  'dri_screen.c',
  'dri_screen.h',
  'dri_util.c',
  'dri_util.h',
)

if with_dri
  files_libdri += files('drisw.c')
endif

if with_dri2
  files_libdri += files('dri2.c')
endif

libdri_c_args = []
if with_gallium_zink and not with_platform_android
  if get_option('glvnd')
    libdri_c_args += '-DKOPPER_LIB_NAMES="libEGL_@0@ and libGLX_@0@"'.format(get_option('glvnd-vendor-name'))
  else
    libdri_c_args += '-DKOPPER_LIB_NAMES="libEGL and libGLX"'
  endif
  files_libdri += files('kopper.c')
  deps_for_libdri += vulkan_wsi_deps
endif

if with_gallium_softpipe
  libdri_c_args += '-DGALLIUM_SOFTPIPE'
endif

libdri = static_library(
  'dri',
  [ files_libdri, sha1_h ],
  include_directories : [
    inc_include, inc_util, inc_mesa, inc_mapi, inc_src, inc_gallium,
    inc_gallium_aux, inc_util,  inc_gallium_drivers,
  ],
  c_args : [libdri_c_args],
  gnu_symbol_visibility : 'hidden',
  dependencies : [
    dep_libdrm,
    idep_mesautil,
    deps_for_libdri,
  ],
)

if with_dri
  dri_req_private = []
  if dep_libdrm.found()
    dri_req_private = ['libdrm >= @0@'.format(_drm_ver)]
  endif

  pkg.generate(
    name : 'dri',
    filebase : 'dri',
    description : 'Direct Rendering Infrastructure',
    version : meson.project_version(),
    variables : ['dridriverdir=@0@'.format(dri_drivers_path)],
    requires_private : dri_req_private,
  )
endif
