# Copyright 2018 Rob Clark
# Copyright 2019 Collabora, Ltd.
# SPDX-License-Identifier: MIT

inc_asahi = include_directories([
   '.', 'layout', 'lib', 'compiler'
])

if with_gallium_asahi
   subdir('layout')
   subdir('clc')
   subdir('lib')
   subdir('compiler')
endif

if with_tools.contains('drm-shim')
  subdir('drm-shim')
endif
