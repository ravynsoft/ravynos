#source: attr-merge-strict-align-05a.s
#source: attr-merge-strict-align-05b.s
#as: -march-attr
#ld: -r
#readelf: -A

Attribute Section: riscv
File Attributes
  Tag_RISCV_arch: [a-zA-Z0-9_\"].*
  Tag_RISCV_unaligned_access: Unaligned access
