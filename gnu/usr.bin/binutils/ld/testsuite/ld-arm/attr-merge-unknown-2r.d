#source: blank.s
#source: attr-merge-unknown-2.s
#as:
#ld:
#warning: unknown EABI object attribute 82
#readelf: -A
# This test is only valid on ELF based ports.
# not-target: *-*-pe *-*-wince

Attribute Section: aeabi
File Attributes
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
