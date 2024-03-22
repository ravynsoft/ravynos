# Copyright © 2022 Collabora Ltd
# SPDX-License-Identifier: MIT

libnil_files = files(
  'nil_format.c',
  'nil_format.h',
  'nil_image.c',
  'nil_image.h',
  'nil_image_tic.c',
)

libnil_deps = [
  idep_mesautil,
  idep_nouveau_ws,
  idep_nvidia_headers,
]

_libnil = static_library(
  'nil',
  libnil_files,
  include_directories : [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium],
  dependencies : libnil_deps,
  c_args : [no_override_init_args],
  gnu_symbol_visibility : 'hidden',
)

idep_nil = declare_dependency(
  include_directories : include_directories('.'),
  link_with : _libnil,
)
