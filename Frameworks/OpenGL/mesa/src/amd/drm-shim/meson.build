# Copyright © 2021 Emma Anholt
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

libradeon_noop_drm_shim = shared_library(
  'radeon_noop_drm_shim',
  'radeon_noop_drm_shim.c',
  include_directories: [inc_include, inc_src, inc_amd],
  dependencies: dep_drm_shim,
  gnu_symbol_visibility : 'hidden',
  install : true,
)

libamdgpu_noop_drm_shim = shared_library(
  'amdgpu_noop_drm_shim',
  ['amdgpu_noop_drm_shim.c', 'amdgpu_devices.c'],
  include_directories: [inc_include, inc_src, inc_amd],
  dependencies: dep_drm_shim,
  gnu_symbol_visibility : 'hidden',
  install : true,
)

amdgpu_dump = executable(
  'amdgpu_dump_states',
  'amdgpu_dump_states.c',
  include_directories: [inc_include, inc_src, inc_amd],
  gnu_symbol_visibility: 'hidden',
  dependencies: [dep_libdrm],
  install: false,
)
