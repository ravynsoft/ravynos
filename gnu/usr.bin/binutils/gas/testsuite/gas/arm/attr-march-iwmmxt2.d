# name: attributes for -march=iwmmxt2
# source: blank.s
# as: -march=iwmmxt2
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "iwmmxt2"
  Tag_CPU_arch: v5TE
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
  Tag_WMMX_arch: WMMXv2
