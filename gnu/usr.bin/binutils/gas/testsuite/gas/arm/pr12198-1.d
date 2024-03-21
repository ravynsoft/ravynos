# name: PR12198 - Only select v6S-M when v6-M is selected (1)
# source: pr12198-1.s
# as:
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_arch: v4T
  Tag_THUMB_ISA_use: Thumb-1
