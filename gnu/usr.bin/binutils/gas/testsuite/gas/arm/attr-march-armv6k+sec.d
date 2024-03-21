# name: attributes for -march=armv6k+sec
# source: blank.s
# as: -march=armv6k+sec
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_name: "6K"
  Tag_CPU_arch: v6KZ
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
  Tag_Virtualization_use: TrustZone
