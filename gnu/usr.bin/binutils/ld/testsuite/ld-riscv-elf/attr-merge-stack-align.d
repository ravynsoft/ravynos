#source: attr-merge-stack-align-a.s
#source: attr-merge-stack-align-b.s
#as: -march-attr
#ld: -r
#readelf: -A

Attribute Section: riscv
File Attributes
  Tag_RISCV_stack_align: 16-bytes
  Tag_RISCV_arch: [a-zA-Z0-9_\"].*
#...
