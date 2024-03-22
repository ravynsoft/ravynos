# Copyright © 2017-2019 Intel Corporation

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

inc_glx = include_directories('.')

subdir('apple')
if with_dri_platform == 'windows'
  subdir('windows')
endif

files_libglx = files(
  'clientattrib.c',
  'clientinfo.c',
  'compsize.c',
  'create_context.c',
  'dri_common.c',
  'dri_common.h',
  'dri_common_query_renderer.c',
  'dri_common_interop.c',
  'drisw_glx.c',
  'drisw_priv.h',
  'eval.c',
  'glxclient.h',
  'glxcmds.c',
  'glxconfig.c',
  'glxconfig.h',
  'glxcurrent.c',
  'glx_error.c',
  'glx_error.h',
  'glxext.c',
  'glxextensions.c',
  'glxextensions.h',
  'glxhash.c',
  'glxhash.h',
  'glx_pbuffer.c',
  'glx_query.c',
  'indirect_glx.c',
  'indirect_init.h',
  'indirect_texture_compression.c',
  'indirect_transpose_matrix.c',
  'indirect_vertex_array.c',
  'indirect_vertex_array.h',
  'indirect_vertex_array_priv.h',
  'indirect_vertex_program.c',
  'indirect_window_pos.c',
  'packrender.h',
  'packsingle.h',
  'pixel.c',
  'pixelstore.c',
  'query_renderer.c',
  'render2.c',
  'renderpix.c',
  'single2.c',
  'singlepix.c',
  'vertarr.c',
  'xfont.c',
)

extra_libs_libglx = []
extra_deps_libgl = []
extra_ld_args_libgl = []

# dri2
if with_dri_platform == 'drm' and dep_libdrm.found()
  files_libglx += files(
    'dri2.c',
    'dri2_glx.c',
    'dri2.h',
    'dri2_priv.h',
  )
endif

if with_dri3
  files_libglx += files('dri3_glx.c', 'dri3_priv.h')
endif

if with_dri_platform == 'apple'
  files_libglx += files('applegl_glx.c')
  extra_libs_libglx += libappleglx
elif with_dri_platform == 'windows'
  files_libglx += files('driwindows_glx.c')
  extra_libs_libglx += [
    libwindowsdri,
    libwindowsglx,
  ]
  extra_deps_libgl = [
    meson.get_compiler('c').find_library('gdi32'),
    meson.get_compiler('c').find_library('opengl32')
  ]
  extra_ld_args_libgl = '-Wl,--disable-stdcall-fixup'
endif

if not with_glvnd
  gl_lib_name = 'GL'
  gl_lib_version = '1.2.0'
else
  gl_lib_name = 'GLX_@0@'.format(glvnd_vendor_name)
  gl_lib_version = '0.0.0'
  files_libglx += files(
    'g_glxglvnddispatchfuncs.c',
    'g_glxglvnddispatchindices.h',
    'glxglvnd.c',
    'glxglvnd.h',
    'glxglvnddispatchfuncs.h',
  )
endif

libglx = static_library(
  'glx',
  [files_libglx, glx_generated],
  include_directories : [inc_include, inc_src, inc_glapi, inc_loader],
  c_args : [
    '-DGL_LIB_NAME="lib@0@.so.@1@"'.format(gl_lib_name, gl_lib_version.split('.')[0]),
  ],
  gnu_symbol_visibility : 'hidden',
  link_with : [
    libloader, libloader_dri3_helper,
    extra_libs_libglx,
  ],
  dependencies : [
    idep_mesautil, idep_xmlconfig,
    dep_libdrm, dep_dri2proto, dep_glproto, dep_x11, dep_xext, dep_glvnd, dep_xxf86vm, dep_xshmfence
  ],
)

libgl = shared_library(
  gl_lib_name,
  [],
  link_with : [libglapi_static, libglapi],
  link_whole : libglx,
  link_args : [ld_args_bsymbolic, ld_args_gc_sections, extra_ld_args_libgl],
  dependencies : [
    dep_libdrm, dep_dl, dep_m, dep_thread, dep_x11, dep_xcb_glx, dep_xcb,
    dep_x11_xcb, dep_xcb_dri2, dep_xext, dep_xfixes, dep_xxf86vm,
    dep_xcb_shm, extra_deps_libgl,
  ],
  version : gl_lib_version,
  darwin_versions : '4.0.0',
  install : true,
)

if with_tests
  subdir('tests')
endif
