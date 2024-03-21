# name: arch and isa entries in elf attribute section
# source: attr-arch-assumption.s
# as:
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_arch: v4T
  Tag_THUMB_ISA_use: Thumb-1
