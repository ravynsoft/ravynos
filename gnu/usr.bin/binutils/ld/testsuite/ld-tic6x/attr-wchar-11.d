#name: C6X wchar_t attribute merging, 1 1
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-wchar-1.s
#source: attr-wchar-1.s
#readelf: -A

Attribute Section: c6xabi
File Attributes
  Tag_ISA: C674x
  Tag_ABI_wchar_t: 2 bytes
