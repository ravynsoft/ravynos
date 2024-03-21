# name: EABI attributes .eabi_attribute overrides .cpu
# source: attr-override-cpu-directive.s
# as:
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "custom name"
  Tag_CPU_arch: v7
  Tag_THUMB_ISA_use: \?\?\? \(10\)
