# Copyright 2022 Advanced Micro Devices, Inc.
# All Rights Reserved.

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

c_args_vpe = cc.get_supported_arguments([
  '-Wall',
  '-Wextra',
  '-Wno-unused',
  '-Wno-unused-parameter',
  '-Wno-unused-command-line-argument',
  '-Wno-ignored-qualifiers',
  '-Wno-missing-field-initializers',
  '-Wno-self-assign',
  '-Wno-implicit-fallthrough',
  '-Werror=comment',
  '-Werror=missing-braces',
  '-Werror=override-init',
  '-Werror=enum-conversion',
  '-Werror=enum-compare',
  '-Werror=maybe-uninitialized',
])

c_args_vpe += [
  '-DLITTLEENDIAN_CPU',
  '-DVPE_BUILD_1_0',
  '-DVPE_BUILD_1_X',
]

vpe_files = files(
  'src/core/inc/reg_helper.h',
  'src/core/inc/vpe_priv.h',
  'src/core/inc/vpe_command.h',
  'src/core/inc/color_pwl.h',
  'src/core/inc/color_gamut.h',
  'src/core/inc/vpe_assert.h',
  'src/core/inc/vpec.h',
  'src/core/inc/plane_desc_writer.h',
  'src/core/inc/color_table.h',
  'src/core/inc/hw_shared.h',
  'src/core/inc/cdc.h',
  'src/core/inc/dpp.h',
  'src/core/inc/color_test_values.h',
  'src/core/inc/vpe_visual_confirm.h',
  'src/core/inc/color_cs.h',
  'src/core/inc/diag_reg_helper.h',
  'src/core/inc/shaper_builder.h',
  'src/core/inc/color_bg.h',
  'src/core/inc/transform.h',
  'src/core/inc/common.h',
  'src/core/inc/color.h',
  'src/core/inc/mpc.h',
  'src/core/inc/3dlut_builder.h',
  'src/core/inc/cmd_builder.h',
  'src/core/inc/background.h',
  'src/core/inc/color_gamma.h',
  'src/core/inc/opp.h',
  'src/core/inc/resource.h',
  'src/core/inc/vpe_desc_writer.h',
  'src/core/inc/config_writer.h',
  'src/core/color_gamma.c',
  'src/core/color_bg.c',
  'src/core/vpe_scl_filters.c',
  'src/core/background.c',
  'src/core/vpe_visual_confirm.c',
  'src/core/mpc.c',
  'src/core/config_writer.c',
  'src/core/plane_desc_writer.c',
  'src/core/color_gamut.c',
  'src/core/vpelib.c',
  'src/core/vpe_desc_writer.c',
  'src/core/3dlut_builder.c',
  'src/core/color_test_values.c',
  'src/core/resource.c',
  'src/core/color_table.c',
  'src/core/color.c',
  'src/core/color_cs.c',
  'src/core/common.c',
  'src/core/shaper_builder.c',
  'src/utils/inc/custom_fp16.h',
  'src/utils/inc/custom_float.h',
  'src/utils/inc/fixed31_32.h',
  'src/utils/inc/conversion.h',
  'src/utils/inc/calc_u64.h',
  'src/utils/custom_fp16.c',
  'src/utils/custom_float.c',
  'src/utils/conversion.c',
  'src/utils/fixpt31_32.c',
  'src/chip/vpe10/inc/vpe10_background.h',
  'src/chip/vpe10/inc/vpe10_cm_common.h',
  'src/chip/vpe10/inc/vpe10_vpec.h',
  'src/chip/vpe10/inc/vpe10_mpc.h',
  'src/chip/vpe10/inc/vpe10_cmd_builder.h',
  'src/chip/vpe10/inc/vpe10_opp.h',
  'src/chip/vpe10/inc/asic/bringup_vpe_6_1_0_default.h',
  'src/chip/vpe10/inc/asic/bringup_vpe_6_1_0_offset.h',
  'src/chip/vpe10/inc/asic/bringup_vpe_6_1_0_sh_mask.h',
  'src/chip/vpe10/inc/asic/vpe_1_0_offset.h',
  'src/chip/vpe10/inc/vpe10_resource.h',
  'src/chip/vpe10/inc/vpe10_cdc.h',
  'src/chip/vpe10/inc/vpe10_dpp.h',
  'src/chip/vpe10/vpe10_cm_common.c',
  'src/chip/vpe10/vpe10_dpp.c',
  'src/chip/vpe10/vpe10_resource.c',
  'src/chip/vpe10/vpe10_mpc.c',
  'src/chip/vpe10/vpe10_cmd_builder.c',
  'src/chip/vpe10/vpe10_dpp_dscl.c',
  'src/chip/vpe10/vpe10_dpp_cm.c',
  'src/chip/vpe10/vpe10_opp.c',
  'src/chip/vpe10/vpe10_background.c',
  'src/chip/vpe10/vpe10_cdc.c',
  'src/chip/vpe10/vpe10_vpec.c',
)

inc_amd_vpe = include_directories(
  'inc',
  'src',
  'src/core/inc',
  'src/chip',
  'src/utils/inc',
  'src/chip/vpe10/inc',
)

libvpe = static_library(
  'libvpe.a',
  vpe_files,
  install : false,
  c_args : c_args_vpe,
  include_directories : inc_amd_vpe
)
