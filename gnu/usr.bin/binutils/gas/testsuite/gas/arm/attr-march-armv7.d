# name: attributes for -march=armv7
# source: blank.s
# as: -march=armv7
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "7"
  Tag_CPU_arch: v7
  Tag_THUMB_ISA_use: Thumb-2
