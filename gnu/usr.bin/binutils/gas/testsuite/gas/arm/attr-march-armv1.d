# name: attributes for -march=armv1
# source: blank.s
# as: -march=armv1
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "1"
  Tag_ARM_ISA_use: Yes
