#readelf: -wf
#name: CFI on mips, 1
Contents of the .eh_frame section:

0+0000 0+0014 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 31
  Augmentation data:     0[bc]

  DW_CFA_def_cfa_register: r29
  DW_CFA_def_cfa: r29 ofs 0
  DW_CFA_nop
  DW_CFA_nop

0+0018 0+00(1c|24) 0+001c FDE cie=0+0000 pc=0+0000..0+002c
  DW_CFA_advance_loc: 4 to 0+0004
  DW_CFA_def_cfa_offset: 8
  DW_CFA_advance_loc: 4 to 0+0008
  DW_CFA_offset: r30 at cfa-8
  DW_CFA_advance_loc: 4 to 0+000c
  DW_CFA_def_cfa: r30 ofs 8
  DW_CFA_advance_loc: 24 to 0+0024
  DW_CFA_def_cfa: r29 ofs 0
  DW_CFA_nop
