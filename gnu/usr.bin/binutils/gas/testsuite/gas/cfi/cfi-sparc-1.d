#readelf: -wf
#name: CFI on SPARC 32-bit
#as: -32

Contents of the .eh_frame section:

0+0000 0+0010 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 4
  Data alignment factor: -4
  Return address column: 15
  Augmentation data:     1b

  DW_CFA_def_cfa: r14 ofs 0

0+0014 0+0014 0+0018 FDE cie=0+0000 pc=0+0000..0+0024
  DW_CFA_advance_loc: 4 to 0+0004
  DW_CFA_def_cfa_register: r30
  DW_CFA_GNU_window_save
  DW_CFA_register: r15 in r31

