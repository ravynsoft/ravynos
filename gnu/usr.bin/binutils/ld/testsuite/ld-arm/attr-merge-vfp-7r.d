#source: attr-merge-vfp-armv8.s
#source: attr-merge-vfp-armv8-hard.s
#as:
#ld: -r
#readelf: -A
# This test is only valid on ELF based ports.
# not-target: *-*-pe *-*-wince

Attribute Section: aeabi
File Attributes
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
  Tag_FP_arch: FP for ARMv8
