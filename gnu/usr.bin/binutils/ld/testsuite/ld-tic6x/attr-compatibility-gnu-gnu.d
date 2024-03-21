#name: C6X compatibility attribute merging, gnu gnu
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-compatibility-gnu.s
#source: attr-compatibility-gnu.s
#readelf: -A

Attribute Section: c6xabi
File Attributes
  Tag_ISA: C674x
  Tag_ABI_compatibility: flag = 1, vendor = gnu
