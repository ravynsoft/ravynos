# Copyright © Microsoft Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

files_libd3d12winsys = files('d3d12_wgl_winsys.c')
if target_machine.system().startswith('Gaming.Xbox')
  files_libd3d12winsys += files('d3d12_wgl_framebuffer_xbox.cpp')
else
  files_libd3d12winsys += files('d3d12_wgl_framebuffer.cpp')
endif

libd3d12winsys = static_library(
  'd3d12winsys',
  files_libd3d12winsys,
  include_directories : [inc_src, inc_wgl, inc_include, inc_gallium, inc_gallium_aux, inc_gallium_drivers],
  dependencies : [dep_dxheaders, idep_nir_headers, idep_libdxil_compiler],
  gnu_symbol_visibility : 'hidden',
)

winsys_d3d12_wgl = declare_dependency(
  link_with : [libd3d12winsys],
)
