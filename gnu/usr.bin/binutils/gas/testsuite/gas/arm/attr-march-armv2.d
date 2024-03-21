# name: attributes for -march=armv2
# source: blank.s
# as: -march=armv2
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "2"
  Tag_ARM_ISA_use: Yes
