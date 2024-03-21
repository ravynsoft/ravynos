# name: attributes for idiv using architecture extensions
# source: blank.s
# as: -march=armv7-r+idiv
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "7-R"
  Tag_CPU_arch: v7
  Tag_CPU_arch_profile: Realtime
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-2
  Tag_DIV_use: Allowed in v7-A with integer division extension
