# name: attributes for -march=armv5
# source: blank.s
# as: -march=armv5
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "5"
  Tag_CPU_arch: v5T
  Tag_ARM_ISA_use: Yes
