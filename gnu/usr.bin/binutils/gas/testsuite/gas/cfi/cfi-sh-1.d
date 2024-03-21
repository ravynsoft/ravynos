#readelf: -wf
#name: CFI on SH
Contents of the .eh_frame section:

0+0000 0+0010 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 2
  Data alignment factor: -4
  Return address column: 17
  Augmentation data:     1b

  DW_CFA_def_cfa: r15 ofs 0

0+0014 0+0020 0+0018 FDE cie=0+0000 pc=0+0000..0+002c
  DW_CFA_advance_loc: 2 to 0+0002
  DW_CFA_def_cfa_offset: 4
  DW_CFA_advance_loc: 2 to 0+0004
  DW_CFA_def_cfa_offset: 8
  DW_CFA_offset: r15 at cfa-4
  DW_CFA_offset: r17 at cfa-8
  DW_CFA_advance_loc: 6 to 0+000a
  DW_CFA_def_cfa_register: r14
  DW_CFA_advance_loc: 2 to 0+000c
  DW_CFA_def_cfa_offset: 40
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

