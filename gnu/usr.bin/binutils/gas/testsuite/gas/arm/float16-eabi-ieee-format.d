# name: Tag_ABI_FP_16bit_format written for IEEE float16 format.
# readelf: -A
# notarget: *-*pe *-*wince
# source: float16-eabi.s
# as: -mfp16-format=ieee
Attribute Section: aeabi
File Attributes
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
  Tag_ABI_FP_16bit_format: IEEE 754
