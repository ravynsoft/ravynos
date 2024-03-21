# name: attributes for -march=armv4
# source: blank.s
# as: -march=armv4
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "4"
  Tag_CPU_arch: v4
  Tag_ARM_ISA_use: Yes
