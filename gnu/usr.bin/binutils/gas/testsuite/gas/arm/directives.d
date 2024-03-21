# name: multiple directives on one line
# source: directives.s
# as: --fatal-warnings
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "custom_name"
  Tag_CPU_arch: v8
  Tag_CPU_arch_profile: Application
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-2
  Tag_FP_arch: FP for ARMv8
  Tag_ABI_align_preserved: 8-byte, except leaf SP
  Tag_MPextension_use: Allowed
  Tag_Virtualization_use: TrustZone and Virtualization Extensions
