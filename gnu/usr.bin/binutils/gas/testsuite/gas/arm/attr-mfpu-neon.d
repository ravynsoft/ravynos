# name: attributes for -mfpu=neon
# source: blank.s
# as: -mfpu=neon
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
  Tag_FP_arch: VFPv3
  Tag_Advanced_SIMD_arch: NEONv1
