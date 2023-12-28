# name: EABI attributes .cpu overrides -mcpu
# source: attr-override-mcpu.s
# as: -mcpu=cortex-a8
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "ARM7TDMI"
  Tag_CPU_arch: v4T
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
