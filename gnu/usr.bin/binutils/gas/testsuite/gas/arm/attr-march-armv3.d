# name: attributes for -march=armv3
# source: blank.s
# as: -march=armv3
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "3"
  Tag_ARM_ISA_use: Yes
