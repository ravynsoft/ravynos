#readelf: -wf
#name: CFI on SPARC 64-bit
#as: -64

Contents of the .eh_frame section:

0+0000 0+0014 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 4
  Data alignment factor: -8
  Return address column: 15
  Augmentation data:     1b

  DW_CFA_def_cfa: r14 ofs 2047
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0018 0+0014 0+001c FDE cie=0+0000 pc=0+0000..0+0030
  DW_CFA_advance_loc: 4 to 0+0004
  DW_CFA_def_cfa_register: r30
  DW_CFA_GNU_window_save
  DW_CFA_register: r15 in r31

