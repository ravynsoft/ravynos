# name: attributes for -march=armv2s
# source: blank.s
# as: -march=armv2s
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "2S"
  Tag_ARM_ISA_use: Yes
