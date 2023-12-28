#name: C6X PID attribute merging, 2 1
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-pid-2.s
#source: attr-pid-1.s
#readelf: -A

Attribute Section: c6xabi
File Attributes
  Tag_ISA: C674x
  Tag_ABI_PID: Data addressing position-independent, GOT near DP
