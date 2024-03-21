# name: attributes for 'any' CPU with ARMv8-M security extension
# source: any-armv8m.s
# as: -mthumb
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_arch: v8-M.baseline
  Tag_CPU_arch_profile: Microcontroller
  Tag_THUMB_ISA_use: Yes
