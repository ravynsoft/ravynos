# name: attributes for 'armv8-m.base' CPU with Thumb integer divide
# source: any-idiv.s
# as: -march=armv8-m.base
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "8-M.BASE"
  Tag_CPU_arch: v8-M.baseline
  Tag_CPU_arch_profile: Microcontroller
  Tag_THUMB_ISA_use: Yes
