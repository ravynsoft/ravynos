# name: attributes for -march=armv5t
# source: blank.s
# as: -march=armv5t
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "5T"
  Tag_CPU_arch: v5T
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
