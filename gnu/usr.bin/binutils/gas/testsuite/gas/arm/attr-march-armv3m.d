# name: attributes for -march=armv3m
# source: blank.s
# as: -march=armv3m
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "3M"
  Tag_ARM_ISA_use: Yes
