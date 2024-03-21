# name: attributes for -march=armv8-m.main
# source: blank.s
# as: -march=armv8-m.main
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "8-M.MAIN"
  Tag_CPU_arch: v8-M.mainline
  Tag_CPU_arch_profile: Microcontroller
  Tag_THUMB_ISA_use: Yes
