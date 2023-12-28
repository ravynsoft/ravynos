#source: attr-merge-vfp-4-sp.s
#source: attr-merge-vfp-5-sp.s
#as:
#ld: -r
#readelf: -A
# This test is only valid on ELF based ports.
# not-target: *-*-pe *-*-wince

Attribute Section: aeabi
File Attributes
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
  Tag_FP_arch: FPv5/FP-D16 for ARMv8
  Tag_ABI_HardFP_use: SP only
