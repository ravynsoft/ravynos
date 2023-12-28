#readelf: -wf
#name: CFI on alpha
Contents of the .eh_frame section:

00000000 0+0010 0+00 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 4
  Data alignment factor: -8
  Return address column: 26
  Augmentation data:     1b

  DW_CFA_def_cfa_register: r30
  DW_CFA_nop

00000014 0+0020 0+0018 FDE cie=00000000 pc=0+0000..0+0034
  DW_CFA_advance_loc: 24 to 0+0018
  DW_CFA_def_cfa: r15 ofs 32
  DW_CFA_offset: r26 at cfa-32
  DW_CFA_offset: r9 at cfa-24
  DW_CFA_offset: r15 at cfa-16
  DW_CFA_offset: r34 at cfa-8
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
