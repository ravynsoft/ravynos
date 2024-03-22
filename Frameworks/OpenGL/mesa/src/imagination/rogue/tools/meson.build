# Copyright © 2022 Imagination Technologies Ltd.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# FIXME: Remove idep_vulkan_runtime once
# the compiler/driver interface is finalised.
rogue_compiler = executable(
  'rogue_vk_compiler',
  'vk_compiler.c',
  link_with : [libpowervr_rogue],
  dependencies : [idep_mesautil, idep_nir, idep_vulkan_runtime, dep_csbgen],
  include_directories : [
    inc_imagination,
    inc_include,
    inc_src,
    inc_rogue,
  ],
  c_args : [imagination_c_args],
  build_by_default : with_imagination_tools,
  install : false,
)
