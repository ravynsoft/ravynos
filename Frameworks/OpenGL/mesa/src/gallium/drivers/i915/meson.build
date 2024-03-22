# Copyright © 2017 Intel Corporation

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

files_i915 = files(
  'i915_batchbuffer.h',
  'i915_batch.h',
  'i915_blit.c',
  'i915_blit.h',
  'i915_clear.c',
  'i915_context.c',
  'i915_context.h',
  'i915_debug.c',
  'i915_debug_fp.c',
  'i915_debug.h',
  'i915_debug_private.h',
  'i915_flush.c',
  'i915_fpc_emit.c',
  'i915_fpc.h',
  'i915_fpc_optimize.c',
  'i915_fpc_translate.c',
  'i915_prim_emit.c',
  'i915_prim_vbuf.c',
  'i915_public.h',
  'i915_query.c',
  'i915_query.h',
  'i915_reg.h',
  'i915_resource_buffer.c',
  'i915_resource.c',
  'i915_resource.h',
  'i915_resource_texture.c',
  'i915_screen.c',
  'i915_screen.h',
  'i915_state.c',
  'i915_state_derived.c',
  'i915_state_dynamic.c',
  'i915_state_emit.c',
  'i915_state_fpc.c',
  'i915_state.h',
  'i915_state_immediate.c',
  'i915_state_inlines.h',
  'i915_state_sampler.c',
  'i915_state_static.c',
  'i915_surface.c',
  'i915_surface.h',
  'i915_winsys.h',
)

libi915 = static_library(
  'i915',
  files_i915,
  gnu_symbol_visibility : 'hidden',
  include_directories : [inc_include, inc_src, inc_gallium, inc_gallium_aux],
  dependencies : idep_nir,
)

driver_i915 = declare_dependency(
  compile_args : '-DGALLIUM_I915',
  link_with : [libi915, libi915drm],
)
