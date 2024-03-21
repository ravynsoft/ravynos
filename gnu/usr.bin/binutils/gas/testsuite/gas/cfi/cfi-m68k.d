#readelf: -wf
#name: CFI on m68k
Contents of the .eh_frame section:

0+0000 0+0014 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 2
  Data alignment factor: -4
  Return address column: 24
  Augmentation data:     1b

  DW_CFA_def_cfa: r15 ofs 4
  DW_CFA_offset: r24 at cfa-4
  DW_CFA_nop
  DW_CFA_nop

0+0018 0+0014 0+001c FDE cie=0+0000 pc=0+0000..0+000c
  DW_CFA_advance_loc: 4 to 0+0004
  DW_CFA_def_cfa_offset: 4664
  DW_CFA_advance_loc: 6 to 0+000a
  DW_CFA_def_cfa_offset: 4

0+0030 0+0018 0+0034 FDE cie=0+0000 pc=0+000c..0+0018
  DW_CFA_advance_loc: 4 to 0+0010
  DW_CFA_def_cfa_offset: 8
  DW_CFA_offset: r14 at cfa-8
  DW_CFA_def_cfa_register: r14
  DW_CFA_advance_loc: 6 to 0+0016
  DW_CFA_def_cfa_register: r15
  DW_CFA_nop

0+004c 0+0010 0+0050 FDE cie=0+0000 pc=0+0018..0+001c

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

