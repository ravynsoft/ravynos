# name: attributes for 'any' cpu v6 thumb insn
# source: attr-any-thumbv6.s
# as:
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_arch: v6
  Tag_THUMB_ISA_use: Thumb-1
