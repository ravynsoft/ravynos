# name: attributes for -mfpu=vfpxd
# source: blank.s
# as: -mfpu=vfpxd
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
  Tag_FP_arch: VFPv1
  Tag_ABI_HardFP_use: SP only
