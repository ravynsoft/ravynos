#source: ../../../cfi/cfi-common-3.s
#readelf: -wf
#name: CFI common 3
Contents of the .eh_frame section:

00000000 00000010 00000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     [01]b
#...
00000014 00000010 00000018 FDE cie=00000000 pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_remember_state
  DW_CFA_restore_state
#pass
