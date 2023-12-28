# name: attributes for vfpv3xd-fp16 using architecture extensions
# source: blank.s
# as: -march=armv7-r+vfpv3xd-fp16
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "7-R"
  Tag_CPU_arch: v7
  Tag_CPU_arch_profile: Realtime
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-2
  Tag_FP_arch: VFPv3-D16
  Tag_ABI_HardFP_use: SP only
  Tag_FP_HP_extension: Allowed
