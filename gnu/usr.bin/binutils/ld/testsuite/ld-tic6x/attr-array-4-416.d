#name: C6X array attribute merging, 4 416
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-array-4.s
#source: attr-array-416.s
#readelf: -A

Attribute Section: c6xabi
File Attributes
  Tag_ISA: C674x
  Tag_ABI_array_object_alignment: 4-byte
  Tag_ABI_array_object_align_expected: 4-byte
