#source: attr-merge-hardfp-use-2a.s
#source: attr-merge-hardfp-use-2b.s
#as:
#ld: -e main
#readelf: -A
# This test is only valid on ELF based ports.
# not-target: *-*-pe *-*-wince

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "Cortex-M4"
  Tag_CPU_arch: v7E-M
  Tag_CPU_arch_profile: Microcontroller
  Tag_THUMB_ISA_use: Thumb-2
  Tag_FP_arch: FPv5/FP-D16 for ARMv8
