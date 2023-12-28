#name: C6X PIC attribute merging, 1 1
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-pic-1.s
#source: attr-pic-1.s
#readelf: -A

Attribute Section: c6xabi
File Attributes
  Tag_ISA: C674x
  Tag_ABI_PIC: Code addressing position-independent
