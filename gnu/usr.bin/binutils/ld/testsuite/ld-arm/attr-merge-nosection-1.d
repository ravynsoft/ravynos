#source: attr-merge-nosection-1a.s RUN_OBJCOPY
#source: attr-merge-nosection-1b.s
#as:
#objcopy_objects: -R '.ARM.attributes'
#ld:
#readelf: -A
# This test is only valid on ELF based ports.
# not-target: *-*-pe *-*-wince

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "Cortex-M4"
  Tag_CPU_arch: v7E-M
  Tag_CPU_arch_profile: Microcontroller
  Tag_THUMB_ISA_use: Thumb-2
  Tag_FP_arch: VFPv4-D16
  Tag_ABI_HardFP_use: SP only

