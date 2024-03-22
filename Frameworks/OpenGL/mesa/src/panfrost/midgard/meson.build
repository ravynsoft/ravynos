# Copyright © 2018 Rob Clark
# Copyright © 2019 Collabora

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

libpanfrost_midgard_files = files(
  'midgard_compile.c',
  'mir.c',
  'midgard_address.c',
  'midgard_print.c',
  'midgard_schedule.c',
  'midgard_derivatives.c',
  'midgard_emit.c',
  'midgard_helper_invocations.c',
  'midgard_ra.c',
  'midgard_ra_pipeline.c',
  'midgard_liveness.c',
  'midgard_ops.c',
  'mir_promote_uniforms.c',
  'mir_squeeze.c',
  'midgard_nir_lower_image_bitsize.c',
  'midgard_nir_type_csel.c',
  'midgard_opt_copy_prop.c',
  'midgard_opt_dce.c',
  'midgard_opt_perspective.c',
  'midgard_opt_prop.c',
  'midgard_errata_lod.c',
  'nir_fuse_io_16.c',
)

midgard_nir_algebraic_c = custom_target(
  'midgard_nir_algebraic.c',
  input : 'midgard_nir_algebraic.py',
  output : 'midgard_nir_algebraic.c',
  command : [
    prog_python, '@INPUT@', '-p', dir_compiler_nir,
  ],
  capture : true,
  depend_files : nir_algebraic_depends,
)

libpanfrost_midgard_disasm = static_library(
  'panfrost_midgard_disasm',
  ['disassemble.c', 'midgard_ops.c', 'midgard_print_constant.c'],
  include_directories : [
   inc_mapi,
   inc_mesa,
   inc_gallium,
   inc_gallium_aux,
   inc_include,
   inc_src,
   inc_panfrost_hw,
  ],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
)

libpanfrost_midgard = static_library(
  'panfrost_midgard',
  [libpanfrost_midgard_files, midgard_nir_algebraic_c],
  include_directories : [
   inc_mapi,
   inc_mesa,
   inc_gallium,
   inc_gallium_aux,
   inc_include,
   inc_src,
   inc_panfrost_hw,
  ],
  dependencies: [
   idep_nir
  ],
  link_with: [libpanfrost_util, libpanfrost_midgard_disasm],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
)
