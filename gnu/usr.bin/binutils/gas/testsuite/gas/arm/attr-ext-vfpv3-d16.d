# name: attributes for architecture extension vfpv3-d16
# source: blank.s
# as: -march=armv7+fp
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "7"
  Tag_CPU_arch: v7
  Tag_THUMB_ISA_use: Thumb-2
  Tag_FP_arch: VFPv3-D16

