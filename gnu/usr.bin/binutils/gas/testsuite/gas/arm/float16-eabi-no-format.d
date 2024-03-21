# name: Tag_ABI_FP_16bit_format EABI attribute not written when format not specified
# readelf: -A
# notarget: *-*pe *-*-wince
# source: float16-eabi.s
Attribute Section: aeabi
File Attributes
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
