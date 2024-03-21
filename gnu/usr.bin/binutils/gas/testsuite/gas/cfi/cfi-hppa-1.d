#readelf: -wf
#name: CFI on hppa
Contents of the .eh_frame section:

00000000 0+0010 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 4
  Data alignment factor: [48]
  Return address column: 2
  Augmentation data:     1[bc]

  DW_CFA_def_cfa: r30 ofs 0

00000014 0+00(18|20) 0+0018 FDE cie=0+0000 pc=0+0000..0+0018
  DW_CFA_advance_loc: 8 to 0+0008
  DW_CFA_def_cfa_register: r3
  DW_CFA_advance_loc: 4 to 0+000c
  DW_CFA_def_cfa_offset: 4660
  DW_CFA_advance_loc: 8 to 0+0014
  DW_CFA_def_cfa_register: r30
  DW_CFA_nop

0000003[08] 0+00(18|20) 0+003[4c] FDE cie=0+0000 pc=0+0018..0+0040
  DW_CFA_advance_loc: 12 to 0+0024
  DW_CFA_def_cfa_register: r3
  DW_CFA_offset_extended_sf: r2 at cfa-24
  DW_CFA_advance_loc: 24 to 0+003c
  DW_CFA_def_cfa_register: r30
  DW_CFA_nop
  DW_CFA_nop

000000[45]c 0+001[08] 0+00[56]0 FDE cie=0+0000 pc=0+0040..0+0048
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

