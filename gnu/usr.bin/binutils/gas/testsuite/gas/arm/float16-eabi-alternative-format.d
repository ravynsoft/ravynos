# name: Tag_ABI_FP_16bit_format EABI attribute written for Arm alternative format.
# readelf: -A
# notarget: *-*pe *-*-wince
# source: float16-eabi.s
# as: -mfp16-format=alternative
Attribute Section: aeabi
File Attributes
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
  Tag_ABI_FP_16bit_format: Alternative Format
