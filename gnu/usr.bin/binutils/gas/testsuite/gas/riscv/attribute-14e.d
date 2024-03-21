#as: -march-attr --defsym priv_insn_e=1
#readelf: -A
#source: attribute-14.s
Attribute Section: riscv
File Attributes
  Tag_RISCV_arch: [a-zA-Z0-9_\"].*
  Tag_RISCV_priv_spec: [0-9_\"].*
#...
