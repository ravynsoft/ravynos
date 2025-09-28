#source: attr-merge-div-1.s
#source: attr-merge-div-2.s
#source: attr-merge-div-0.s
#as: -mcpu=cortex-m3
#ld: -r
#readelf: -A
# This test is only valid on ELF based ports.
# not-target: *-*-pe *-*-wince

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "Cortex-M3"
  Tag_CPU_arch: v7
  Tag_CPU_arch_profile: Microcontroller
  Tag_THUMB_ISA_use: Thumb-2
  Tag_DIV_use: Allowed in v7-A with integer division extension
