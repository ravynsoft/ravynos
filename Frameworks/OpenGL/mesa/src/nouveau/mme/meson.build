# Copyright © 2022 Collabora, Ltd.
# SPDX-License-Identifier: MIT

mme_tu104_isa_depend_files = [
  'mme_tu104.xml',
  isaspec_py_deps
]

mme_tu104_isa = custom_target(
  'mme_isa',
  input: ['mme_tu104.xml'],
  output: ['mme_tu104_isa.c', 'mme_tu104_isa.h'],
  command: [
    prog_isaspec_decode, '--xml', '@INPUT@',
    '--out-c', '@OUTPUT0@', '--out-h', '@OUTPUT1@',
  ],
  depend_files: mme_tu104_isa_depend_files,
)

mme_tu104_encode_h = custom_target(
  'mme-tu104-encode.h',
  input: ['mme_tu104.xml'],
  output: 'mme_tu104_encode.h',
  command: [
    prog_isaspec_encode, '--xml', '@INPUT@', '--out-h', '@OUTPUT@'
  ],
  depend_files: mme_tu104_isa_depend_files,
)

mme_fermi_isa_depend_files = [
  'mme_fermi.xml',
  isaspec_py_deps
]

mme_fermi_isa = custom_target(
  'mme_fermi_isa',
  input: ['mme_fermi.xml'],
  output: ['mme_fermi_isa.c', 'mme_fermi_isa.h'],
  command: [
    prog_isaspec_decode, '--xml', '@INPUT@',
    '--out-c', '@OUTPUT0@', '--out-h', '@OUTPUT1@',
  ],
  depend_files: mme_fermi_isa_depend_files,
)

mme_fermi_encode_h = custom_target(
  'mme-fermi-encode.h',
  input: ['mme_fermi.xml'],
  output: 'mme_fermi_encode.h',
  command: [
    prog_isaspec_encode, '--xml', '@INPUT@', '--out-h', '@OUTPUT@'
  ],
  depend_files: mme_fermi_isa_depend_files,
)

libnouveau_mme_files = files(
  'mme_builder.h',
  'mme_fermi.c',
  'mme_fermi.h',
  'mme_fermi_builder.c',
  'mme_fermi_dump.c',
  'mme_fermi_sim.c',
  'mme_tu104.c',
  'mme_tu104.h',
  'mme_tu104_builder.c',
  'mme_tu104_dump.c',
  'mme_tu104_sim.c',
  'mme_tu104_sim.h',
)

_libnouveau_mme = static_library(
  'nouveau_mme',
  [
    libnouveau_mme_files,
    mme_fermi_isa,
    mme_fermi_encode_h,
    mme_tu104_isa,
    mme_tu104_encode_h,
  ],
  include_directories : [inc_include, inc_src],
  gnu_symbol_visibility : 'hidden',
  dependencies : [
    idep_mesautil,
    idep_isaspec_decode,
    idep_nvidia_headers,
  ],
)

idep_nouveau_mme = declare_dependency(
  include_directories : include_directories('.'),
  link_with : _libnouveau_mme,
)

if with_tests and not with_platform_android
  test('mme_builder',
    executable(
      'mme_builder_test',
      files('tests/mme_runner.cpp', 'tests/mme_builder_test.cpp'),
      gnu_symbol_visibility : 'hidden',
      include_directories : [inc_include, inc_src],
      dependencies : [
        dep_libdrm,
        idep_gtest,
        idep_mesautil,
        idep_nvidia_headers,
        idep_nouveau_mme,
        idep_nouveau_ws
      ],
    ),
    suite : ['nouveau'],
  )

  executable(
    'mme_fermi_sim_hw_test',
    files('tests/mme_runner.cpp', 'tests/mme_fermi_sim_hw_test.cpp'),
    gnu_symbol_visibility : 'hidden',
    include_directories : [inc_include, inc_src],
    dependencies : [
      dep_libdrm,
      idep_gtest,
      idep_mesautil,
      idep_nvidia_headers,
      idep_nouveau_mme,
      idep_nouveau_ws
    ],
    install : true,
  )

  executable(
    'mme_tu104_sim_hw_test',
    files('tests/mme_runner.cpp', 'tests/mme_tu104_sim_hw_test.cpp'),
    gnu_symbol_visibility : 'hidden',
    include_directories : [inc_include, inc_src],
    dependencies : [
      dep_libdrm,
      idep_gtest,
      idep_mesautil,
      idep_nvidia_headers,
      idep_nouveau_mme,
      idep_nouveau_ws
    ],
    install : true,
  )
endif
