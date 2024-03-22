#
# Copyright © 2023 Igalia S.L.
# SPDX-License-Identifier: MIT
#

test(
  'etnaviv_isa_disasm',
  executable(
    'etnaviv_disasm', 'disasm.cpp',
    link_with: [libetnaviv_decode],
    include_directories: [inc_etnaviv, inc_src],
    dependencies : [idep_gtest],
  )
)
