# name: EABI attribute Tag_ABI_HardFP_use with value 0
# source: attr-abi-hardfp-use-0.s
# as:
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "Cortex-M7"
  Tag_CPU_arch: v7E-M
  Tag_CPU_arch_profile: Microcontroller
  Tag_THUMB_ISA_use: Thumb-2
  Tag_FP_arch: FPv5/FP-D16 for ARMv8
