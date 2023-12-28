#source: ../../../cfi/cfi-common-7.s
#readelf: -wf
#name: CFI common 7
Contents of the .eh_frame section:

00000000 00000010 00000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     [01]b

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000014 000000(18|1c|20) 00000018 FDE cie=00000000 pc=.*
  DW_CFA_advance_loc: 16 to .*
  DW_CFA_def_cfa: r0( \([er]ax\)|) ofs 16
  DW_CFA_advance_loc[24]: 75040 to .*
  DW_CFA_def_cfa: r0( \([er]ax\)|) ofs 64
#...
