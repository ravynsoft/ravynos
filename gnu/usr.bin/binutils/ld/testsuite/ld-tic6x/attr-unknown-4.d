#name: C6X unknown attribute merging 4
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-unknown-71-a.s
#source: attr-unknown-71-b.s
#warning: .*warning: unknown EABI object attribute 71
#readelf: -A

Attribute Section: c6xabi
File Attributes
  Tag_ISA: C674x
