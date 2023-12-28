#name: C6X conformance attribute merging, 10 10
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-conformance-10.s
#source: attr-conformance-10.s
#readelf: -A

Attribute Section: c6xabi
File Attributes
  Tag_ABI_conformance: "1.0"
  Tag_ISA: C674x
