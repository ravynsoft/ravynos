# Copyright 2022 Alyssa Rosenzweig
# SPDX-License-Identifier: MIT

libasahi_layout_files = files(
  'layout.c',
  'tiling.c',
)

libasahi_layout = static_library(
  'asahi_layout',
  [libasahi_layout_files],
  include_directories : [inc_include, inc_src],
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
  build_by_default : false,
)

if with_tests
  test(
    'asahi_layout_tests',
    executable(
      'asahi_layout_tests',
      files(
        'tests/test-layout.cpp',
        'tests/test-generated.cpp',
      ),
      c_args : [c_msvc_compat_args, no_override_init_args],
      gnu_symbol_visibility : 'hidden',
      dependencies: [idep_gtest, idep_mesautil],
      link_with : [libasahi_layout],
    ),
    suite : ['asahi'],
    protocol : 'gtest',
  )
endif
