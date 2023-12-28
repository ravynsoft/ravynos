#name: C6X wchar_t attribute merging, 0 2
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-wchar-0.s
#source: attr-wchar-2.s
#readelf: -A

Attribute Section: c6xabi
File Attributes
  Tag_ISA: C674x
  Tag_ABI_wchar_t: 4 bytes
