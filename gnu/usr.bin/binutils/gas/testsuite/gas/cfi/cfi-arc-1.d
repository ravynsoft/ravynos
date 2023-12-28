#readelf: -wf
#name: CFI on ARC

Contents of the .eh_frame section:

00000000 0+10 00000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 31
  Augmentation data:     1b

  DW_CFA_def_cfa: r28 ofs 0

00000014 0+20 00000018 FDE cie=00000000 pc=0+..[0-9a-f]+
  DW_CFA_advance_loc: 4 to 0+4
  DW_CFA_def_cfa_offset: 48
  DW_CFA_offset: r13 at cfa-48
  DW_CFA_advance_loc: 8 to 0+c
  DW_CFA_def_cfa_offset: 52
  DW_CFA_offset: r14 at cfa-44
  DW_CFA_offset: r15 at cfa-40
  DW_CFA_advance_loc: 4 to 0+10
  DW_CFA_offset: r27 at cfa-52
  DW_CFA_advance_loc: 4 to 0+14
  DW_CFA_def_cfa_register: r27
  DW_CFA_nop

