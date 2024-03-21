#name: C6X array attribute merging, 816 16
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-array-816.s
#source: attr-array-16.s
#readelf: -A

Attribute Section: c6xabi
File Attributes
  Tag_ISA: C674x
  Tag_ABI_array_object_alignment: 16-byte
  Tag_ABI_array_object_align_expected: 16-byte
