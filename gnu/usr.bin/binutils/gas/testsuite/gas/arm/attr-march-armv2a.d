# name: attributes for -march=armv2a
# source: blank.s
# as: -march=armv2a
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "2A"
  Tag_ARM_ISA_use: Yes
