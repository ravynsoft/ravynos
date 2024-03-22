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

inc_wgl = include_directories('.')
_c_args_wgl = []
if not with_shared_glapi
  # prevent _glapi_* from being declared __declspec(dllimport)
  _c_args_wgl += '-D_GLAPI_NO_EXPORTS'
endif

_wgl_deps = [idep_xmlconfig, driver_zink]
if with_gallium_zink
  _wgl_deps += idep_vulkan_wsi_defines
endif

files_libwgl = files(
  'stw_context.c',
  'stw_device.c',
  'stw_ext_context.c',
  'stw_ext_extensionsstring.c',
  'stw_ext_interop.c',
  'stw_ext_pbuffer.c',
  'stw_ext_pixelformat.c',
  'stw_ext_rendertexture.c',
  'stw_ext_swapinterval.c',
  'stw_framebuffer.c',
  'stw_getprocaddress.c',
  'stw_image.c',
  'stw_nopfuncs.c',
  'stw_nopfuncs.h',
  'stw_pixelformat.c',
  'stw_st.c',
  'stw_tls.c',
)

if target_machine.system().startswith('Gaming.Xbox')
  files_libwgl += files('stw_gdishim.c')
endif

libwgl = static_library(
  'wgl',
  files_libwgl,
  c_args : [
    '-D_GDI32_',             # prevent wgl* being declared __declspec(dllimport)
    _c_args_wgl
  ],
  include_directories : [
    inc_include, inc_src, inc_gallium, inc_gallium_aux, inc_mapi, inc_mesa,
  ],
  dependencies : _wgl_deps,
)
