#name: PR ld/12570
#as: --32
#ld: -melf_i386 -shared
#readelf: -wf --wide

#...
  DW_CFA_def_cfa_expression \(DW_OP_breg4 \(esp\): 4; DW_OP_breg8 \(eip\): 0; DW_OP_lit15; DW_OP_and; DW_OP_lit11; DW_OP_ge; DW_OP_lit2; DW_OP_shl; DW_OP_plus\)
#...
